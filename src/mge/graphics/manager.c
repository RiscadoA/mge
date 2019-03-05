#include <mge/graphics/manager.h>
#include <mge/log.h>
#include <mgl/memory/allocator.h>
#include <mgl/memory/pool_allocator.h>
#include <mgl/string/manipulation.h>

enum
{
	MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_SHADOW,
	MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_GBUFFER,
	MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_TRANSPARENT,
	MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_GUI,
};

struct mge_shader_effect_t
{
	mgl_u32_t pass_count;
	struct
	{
		mgl_u32_t index;
		mrl_shader_pipeline_t* pipeline;
		mrl_shader_binding_point_t* cb_bp;
	} passes[MGE_SHADER_EFFECT_MAX_PASS_COUNT];
};

struct mge_graphics_manager_t
{
	void* allocator;
	mrl_render_device_t* rd;
	mgl_u32_t width;
	mgl_u32_t height;

	mgl_u8_t effect_allocator_memory[MGL_POOL_ALLOCATOR_SIZE(MGE_MAX_SHADER_EFFECT_COUNT, sizeof(mge_shader_effect_t))];
	mgl_pool_allocator_t effect_allocator;

	struct
	{
		mrl_framebuffer_t* fb;
		mrl_texture_2d_t* albedo_specular;
		mrl_texture_2d_t* normal;
		mrl_texture_2d_t* position;
	} gbuffer;
	
	struct
	{
		mrl_vertex_array_t* va;
		mrl_vertex_buffer_t* vb;
		mrl_shader_stage_t* vertex;
		mrl_shader_stage_t* pixel;
		mrl_shader_pipeline_t* pp;

		mrl_shader_binding_point_t* albedo_specular_bp;
		mrl_shader_binding_point_t* normal_bp;
		mrl_shader_binding_point_t* position_bp;
	} screen_quad;
};

mge_graphics_manager_t * mge_init_graphics_manager(mge_graphics_manager_desc_t * desc)
{
	MGL_DEBUG_ASSERT(desc != NULL);

	mge_graphics_manager_t* manager;

	// Allocate manager
	{
		mgl_error_t err = mgl_allocate(desc->allocator, sizeof(mge_graphics_manager_t), (void**)&manager);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate graphics manager", err);
	}

	manager->allocator = desc->allocator;
	manager->rd = desc->rd;
	manager->width = desc->width;
	manager->height = desc->height;

	// Initialize effect pool
	mgl_init_pool_allocator(&manager->effect_allocator, MGE_MAX_SHADER_EFFECT_COUNT, sizeof(mge_shader_effect_t), manager->effect_allocator_memory, sizeof(manager->effect_allocator_memory));

	// Initialize framebuffers
	manager->gbuffer.fb = NULL;
	manager->gbuffer.albedo_specular = NULL;
	manager->gbuffer.normal = NULL;
	manager->gbuffer.position = NULL;
	mge_resize_graphics_manager(manager, manager->width, manager->height);

	// Initialize screen quad
	{
		// Create vertex shader stage
		{
			mrl_shader_stage_desc_t desc = MRL_DEFAULT_SHADER_STAGE_DESC;

			desc.src_type = MRL_SHADER_SOURCE_GLSL;
			desc.stage = MRL_SHADER_STAGE_VERTEX;
			desc.src =
				"#version 330 core\n"
				""
				"in vec2 position;"
				"in vec2 uvs;"
				""
				"out vec2 frag_uvs;"
				""
				"void main() {"
				"	gl_Position = vec4(position, 0.0, 1.0);"
				"	frag_uvs = uvs;"
				"}";

			mrl_error_t err = mrl_create_shader_stage(manager->rd, &manager->screen_quad.vertex, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create screen quad vertex shader stage", err);
		}

		// Create pixel shader stage
		{
			mrl_shader_stage_desc_t desc = MRL_DEFAULT_SHADER_STAGE_DESC;

			desc.src_type = MRL_SHADER_SOURCE_GLSL;
			desc.stage = MRL_SHADER_STAGE_PIXEL;
			desc.src =
				"#version 330 core\n"
				""
				"in vec2 frag_uvs;"
				""
				"uniform sampler2D albedo_specular_tex;"
				"uniform sampler2D normal_tex;"
				"uniform sampler2D position_tex;"
				""
				"out vec4 gl_FragColor;"
				""
				"void main() {"
				"	gl_FragColor = vec4(frag_uvs, 0.0, 1.0);"
				"}";

			mrl_error_t err = mrl_create_shader_stage(manager->rd, &manager->screen_quad.pixel, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create screen quad pixel shader stage", err);
		}

		// Create shader pipeline
		{
			mrl_shader_pipeline_desc_t desc = MRL_DEFAULT_SHADER_PIPELINE_DESC;

			desc.vertex = manager->screen_quad.vertex;
			desc.pixel = manager->screen_quad.pixel;

			mrl_error_t err = mrl_create_shader_pipeline(manager->rd, &manager->screen_quad.pp, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create screen quad pipeline", err);
		}

		// Get binding points
		manager->screen_quad.albedo_specular_bp = mrl_get_shader_binding_point(manager->rd, manager->screen_quad.pp, u8"albedo_specular_tex");
		manager->screen_quad.normal_bp = mrl_get_shader_binding_point(manager->rd, manager->screen_quad.pp, u8"normal_tex");
		manager->screen_quad.position_bp = mrl_get_shader_binding_point(manager->rd, manager->screen_quad.pp, u8"position_tex");

		// Create vertex buffer
		{
			mgl_f32_t data[] =
			{
				-1.0f, -1.0f, 0.0f, 0.0f,
				+1.0f, -1.0f, 1.0f, 0.0f,
				+1.0f, +1.0f, 1.0f, 1.0f,

				-1.0f, -1.0f, 0.0f, 0.0f,
				-1.0f, +1.0f, 0.0f, 1.0f,
				+1.0f, +1.0f, 1.0f, 1.0f,
			};

			mrl_vertex_buffer_desc_t desc = MRL_DEFAULT_VERTEX_BUFFER_DESC;
			desc.data = data;
			desc.size = sizeof(data);
			desc.usage = MRL_VERTEX_BUFFER_USAGE_STATIC;

			mrl_error_t err = mrl_create_vertex_buffer(manager->rd, &manager->screen_quad.vb, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create screen quad vertex buffer", err);
		}

		// Create vertex array
		{
			mrl_vertex_array_desc_t desc = MRL_DEFAULT_VERTEX_ARRAY_DESC;

			desc.buffer_count = 1;
			desc.buffers[0] = manager->screen_quad.vb;
			desc.element_count = 2;

			desc.elements[0].buffer.index = 0;
			desc.elements[0].buffer.offset = sizeof(mgl_f32_t) * 0;
			desc.elements[0].buffer.stride = sizeof(mgl_f32_t) * 2;
			mgl_str_copy(u8"position", desc.elements[0].name, sizeof(desc.elements[0].name));
			desc.elements[0].type = MRL_VERTEX_ELEMENT_TYPE_F32;
			desc.elements[0].size = 2;

			desc.elements[1].buffer.index = 0;
			desc.elements[1].buffer.offset = sizeof(mgl_f32_t) * 2;
			desc.elements[1].buffer.stride = sizeof(mgl_f32_t) * 4;
			mgl_str_copy(u8"uvs", desc.elements[1].name, sizeof(desc.elements[1].name));
			desc.elements[1].type = MRL_VERTEX_ELEMENT_TYPE_F32;
			desc.elements[1].size = 2;

			desc.shader_pipeline = manager->screen_quad.pp;

			mrl_error_t err = mrl_create_vertex_array(manager->rd, &manager->screen_quad.va, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create screen quad vertex array", err);
		}
	}

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully initialized graphics manager\n");

	return manager;
}

void mge_terminate_graphics_manager(mge_graphics_manager_t * manager)
{
	MGL_DEBUG_ASSERT(manager != NULL);

	// Destroy screen quad
	mrl_destroy_vertex_array(manager->rd, manager->screen_quad.va);
	mrl_destroy_vertex_buffer(manager->rd, manager->screen_quad.vb);
	mrl_destroy_shader_pipeline(manager->rd, manager->screen_quad.pp);
	mrl_destroy_shader_stage(manager->rd, manager->screen_quad.vertex);
	mrl_destroy_shader_stage(manager->rd, manager->screen_quad.pixel);

	// Destroy GBuffer
	mrl_destroy_framebuffer(manager->rd, manager->gbuffer.fb);
	mrl_destroy_texture_2d(manager->rd, manager->gbuffer.albedo_specular);
	mrl_destroy_texture_2d(manager->rd, manager->gbuffer.normal);
	mrl_destroy_texture_2d(manager->rd, manager->gbuffer.position);

	// Deallocate manager
	mgl_error_t err = mgl_deallocate(manager->allocator, manager);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate graphics manager", err);

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully terminated graphics manager\n");
}

void mge_resize_graphics_manager(mge_graphics_manager_t * manager, mgl_u32_t width, mgl_u32_t height)
{
	MGL_DEBUG_ASSERT(manager != NULL);

	manager->width = width;
	manager->height = height;

	// Initialize GBuffer
	{
		// Destroy old
		if (manager->gbuffer.fb != NULL)
			mrl_destroy_framebuffer(manager->rd, manager->gbuffer.fb);
		if (manager->gbuffer.albedo_specular != NULL)
			mrl_destroy_texture_2d(manager->rd, manager->gbuffer.albedo_specular);
		if (manager->gbuffer.normal != NULL)
			mrl_destroy_texture_2d(manager->rd, manager->gbuffer.normal);
		if (manager->gbuffer.position != NULL)
			mrl_destroy_texture_2d(manager->rd, manager->gbuffer.position);

		// Textures
		{
			mrl_texture_2d_desc_t desc = MRL_DEFAULT_TEXTURE_2D_DESC;
			desc.format = MRL_TEXTURE_FORMAT_RGBA32_F;
			desc.width = manager->width;
			desc.height = manager->height;
			desc.usage = MRL_TEXTURE_USAGE_RENDER_TARGET;

			mrl_error_t err = mrl_create_texture_2d(manager->rd, &manager->gbuffer.albedo_specular, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create GBuffer albedo-specular texture", err);
			err = mrl_create_texture_2d(manager->rd, &manager->gbuffer.normal, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create GBuffer normal texture", err);
			err = mrl_create_texture_2d(manager->rd, &manager->gbuffer.position, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create GBuffer position texture", err);
		}

		// Framebuffer
		{
			mrl_framebuffer_desc_t desc = MRL_DEFAULT_FRAMEBUFFER_DESC;
			desc.target_count = 3;
			desc.targets[0].type = MRL_RENDER_TARGET_TYPE_TEXTURE_2D;
			desc.targets[0].tex_2d.handle = manager->gbuffer.albedo_specular;
			desc.targets[1].type = MRL_RENDER_TARGET_TYPE_TEXTURE_2D;
			desc.targets[1].tex_2d.handle = manager->gbuffer.normal;
			desc.targets[2].type = MRL_RENDER_TARGET_TYPE_TEXTURE_2D;
			desc.targets[2].tex_2d.handle = manager->gbuffer.position;

			mrl_error_t err = mrl_create_framebuffer(manager->rd, &manager->gbuffer, &desc);
			if (err != MRL_ERROR_NONE)
				mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to create GBuffer framebuffer", err);
		}
	}
}

mge_shader_effect_t * mge_create_shader_effect(mge_graphics_manager_t * manager, const mge_shader_effect_desc_t * desc)
{
	MGL_DEBUG_ASSERT(manager != NULL && desc != NULL);

	// Check description (get correct technique)
	MGL_DEBUG_ASSERT(desc->technique_count <= MGE_SHADER_EFFECT_MAX_TECHNIQUE_COUNT);
	mgl_u32_t index = MGE_SHADER_EFFECT_MAX_TECHNIQUE_COUNT;

	for (mgl_u32_t i = 0; i < desc->technique_count; ++i)
		if (mgl_str_equal(u8"deferred", desc->techniques[i].name))
		{
			index = i;
			break;
		}

	if (index == MGE_SHADER_EFFECT_MAX_TECHNIQUE_COUNT)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to create shader effect, no 'deferred' technique found (only deferred rendering is supported for now");
	MGL_DEBUG_ASSERT(desc->techniques[index].pass_count < MGE_SHADER_EFFECT_MAX_PASS_COUNT);

	mge_shader_effect_t* effect;

	// Allocate shader effect
	mgl_error_t err = mgl_allocate(&manager->effect_allocator, sizeof(*effect), (void**)&effect);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate shader effect", err);
	
	// Store info
	effect->pass_count = desc->techniques[index].pass_count;
	for (mgl_u32_t i = 0; i < desc->techniques[index].pass_count; ++i)
	{
		const mgl_chr8_t* layer = desc->techniques[index].passes[i].layer;

		if (mgl_str_equal(u8"shadow", layer))
			effect->passes[i].index = MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_SHADOW;
		else if (mgl_str_equal(u8"gbuffer", layer))
			effect->passes[i].index = MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_GBUFFER;
		else if (mgl_str_equal(u8"transparent", layer))
			effect->passes[i].index = MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_TRANSPARENT;
		else if (mgl_str_equal(u8"gui", layer))
			effect->passes[i].index = MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_GUI;
		else
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to create shader effect, unsupported shader effect pass layer", err);

		effect->passes[i].pipeline = desc->techniques[index].passes[i].pipeline;
		effect->passes[i].cb_bp = desc->techniques[index].passes[i].cb_bp;
		MGL_DEBUG_ASSERT(effect->passes[i].pipeline != NULL);
	}

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Created shader effect\n");
	
	return effect;
}

void mge_destroy_shader_effect(mge_graphics_manager_t * manager, mge_shader_effect_t * effect)
{
	MGL_DEBUG_ASSERT(manager != NULL && effect != NULL);

	// Deallocate shader effect
	mgl_error_t err = mgl_deallocate(&manager->effect_allocator, effect);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate shader effect", err);

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Destroyed shader effect\n");
}

void mge_graphics_manager_render(mge_graphics_manager_t * manager, const mge_graphics_frame_t * frame)
{
	MGL_DEBUG_ASSERT(manager != NULL && frame != NULL);

	// GBuffer pass
	mrl_set_framebuffer(manager->rd, manager->gbuffer.fb);
	mrl_set_viewport(manager->rd, 0, 0, manager->width, manager->height);

	for (mgl_u32_t i = 0; i < frame->effect_count; ++i)
	{
		// Get pass index
		mgl_u32_t p;
		for (p = 0; p < frame->effects[i].effect->pass_count; ++p)
			if (frame->effects[i].effect->passes[p].index == MGE_GRAPHICS_MANAGER_DEFERRED_LAYER_GBUFFER)
				break;
		if (p == frame->effects[i].effect->pass_count)
			continue;

		// Set pipeline
		mrl_set_shader_pipeline(manager->rd, frame->effects[i].effect->passes[p].pipeline);

		// Draw entities
		for (mgl_u32_t j = 0; j < frame->effects[i].entity_count; ++j)
		{
			mrl_set_vertex_array(manager->rd, frame->effects[i].entities[j].vertex_array);

			if (frame->effects[i].entities[j].index_buffer == NULL)
			{
				if (frame->effects[i].entities[j].instance_count == 0)
					mrl_draw_triangles(
						manager->rd,
						frame->effects[i].entities[j].offset,
						frame->effects[i].entities[j].count);
				else
					mrl_draw_triangles_instanced(
						manager->rd,
						frame->effects[i].entities[j].offset,
						frame->effects[i].entities[j].count,
						frame->effects[i].entities[j].instance_count);
			}
			else
			{
				mrl_set_index_buffer(manager->rd, frame->effects[i].entities[j].index_buffer);
				if (frame->effects[i].entities[j].instance_count == 0)
					mrl_draw_triangles_indexed(
						manager->rd,
						frame->effects[i].entities[j].offset,
						frame->effects[i].entities[j].count);
				else
					mrl_draw_triangles_indexed_instanced(
						manager->rd,
						frame->effects[i].entities[j].offset,
						frame->effects[i].entities[j].count,
						frame->effects[i].entities[j].instance_count);
			}
		}
	}

	// Render screen space quad
	mrl_set_framebuffer(manager->rd, NULL);
	mrl_set_viewport(manager->rd, 0, 0, manager->width, manager->height);

	mrl_set_shader_pipeline(manager->rd, manager->screen_quad.pp);
	mrl_bind_texture_2d(manager->rd, manager->screen_quad.albedo_specular_bp, manager->gbuffer.albedo_specular);
	mrl_bind_texture_2d(manager->rd, manager->screen_quad.normal_bp, manager->gbuffer.normal);
	mrl_bind_texture_2d(manager->rd, manager->screen_quad.position_bp, manager->gbuffer.position);
	mrl_set_vertex_array(manager->rd, manager->screen_quad.va);
	mrl_draw_triangles(manager->rd, 0, 6);
}
