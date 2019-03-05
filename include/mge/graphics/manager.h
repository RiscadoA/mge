#ifndef MGE_GRAPHICS_MANAGER_H
#define MGE_GRAPHICS_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>
#include <mrl/render_device.h>

	/*
		Each graphical entity is assigned to a shader effect.
		A shader effect specifies how should a collection of data (a graphical entity) shall be rendered.
		A graphical entity oontains  (optionally) a constant buffer for each shader effect pass.

		Shader effect properties:
			- Technique count (number of supported techniques)
			[For each technique]
				- Technique name (specifies the name of the technique described here).
				- Pass count (number of passes).
				[For each pass]
					- Shader pipeline (the shader pipeline used on this pass).
					- Global and local constant buffer binding points.
					- Layer name (specifies the layer this pass operates on).

		Supported techniques:
			"deferred":
				Performs deferred rendering.
				Supported layers:
					- "shadow": the passes on this layer are run for each light to generate its shadows.
						The shaders on this layer shall output a f32 containing the depth of the shadow.
						Parameters:
							- Constant buffer "light":
								- "light_vp": light view projection f32m4x4 matrix.

					- "gbuffer": renders the albedo, normal, specular and position of a fragment to the gbuffer.
						The shaders on this layer shall output:
							- (target 0) A f32v4 containing the albedo (xyz) and specular (w) of the fragment;
							- (target 1) A f32v4 containing the normal (xyz) of the fragment;
							- (target 2) A f32v4 containing the position (xyz) of the fragment;
						Parameters:
							- Constant buffer "camera":
								- "camera_view": camera view f32m4x4 matrix.
								- "camera_proj": camera projection f32m4x4 matrix.

					- "transparent": used to render transparent objects. These are not affected by lights and shadows (this is forward rendered).
						The shaders on this layer shall output a f32v4 containing the color of the fragment (alpha blending is supported).
						Parameters:
							- Constant buffer "camera":
								- "camera_view": camera view f32m4x4 matrix.
								- "camera_proj": camera projection f32m4x4 matrix.

					- "gui": usedd to render the graphical user interface.
						The shaders on this layer shall output a f32v4 containing the color of the fragment (alpha blending is supported).
						Parameters:
							- Constant buffer "screen":
								- "screen_view": screen view f32m4x4 matrix.
	*/


	typedef struct mge_shader_effect_t mge_shader_effect_t;
	typedef struct mge_shader_effect_desc_t mge_shader_effect_desc_t;
	typedef struct mge_shader_effect_params_t mge_shader_effect_params_t;
	typedef struct mge_graphics_frame_entity_t mge_graphics_frame_entity_t;
	typedef struct mge_graphics_frame_effect_t mge_graphics_frame_effect_t;
	typedef struct mge_graphics_frame_t mge_graphics_frame_t;
	typedef struct mge_graphics_manager_t mge_graphics_manager_t;
	typedef struct mge_graphics_manager_desc_t mge_graphics_manager_desc_t;

#define MGE_SHADER_EFFECT_MAX_TECHNIQUE_COUNT 4
#define MGE_SHADER_EFFECT_MAX_PASS_COUNT 8
#define MGE_MAX_SHADER_EFFECT_COUNT 256

	struct mge_shader_effect_desc_t
	{
		/// <summary>
		///		Number of available techniques.
		/// </summary>
		mgl_u32_t technique_count;

		struct
		{
			/// <summary>
			///		Technique name.
			/// </summary>
			const mgl_chr8_t* name;

			/// <summary>
			///		Number of passes (max is MGE_SHADER_EFFECT_MAX_PASS_COUNT).
			/// </summary>
			mgl_u32_t pass_count;

			/// <summary>
			///		Passes array.
			/// </summary>
			struct
			{
				/// <summary>
				///		Name of the layer.
				/// </summary>
				const mgl_chr8_t* layer;

				/// <summary>
				///		Shader pipeline used on this pass.
				/// </summary>
				mrl_shader_pipeline_t* pipeline;

				/// <summary>
				///		Constant buffer binding point.
				///		Optional, can be set to NULL.
				/// </summary>
				mrl_shader_binding_point_t* cb_bp;

			} passes[MGE_SHADER_EFFECT_MAX_PASS_COUNT];
		} techniques[MGE_SHADER_EFFECT_MAX_TECHNIQUE_COUNT];
	};

	// Parameters passed to a shader effect when rendering
	struct mge_shader_effect_params_t
	{
		struct
		{
			/// <summary>
			///		Constant buffer handle.
			/// </summary>
			mrl_constant_buffer_t* cb;
		} passes[MGE_SHADER_EFFECT_MAX_PASS_COUNT];
	};

	struct mge_graphics_frame_entity_t
	{
		/// <summary>
		///		Optional index buffer, can be NULL.
		/// </summary>
		mrl_index_buffer_t* index_buffer;

		/// <summary>
		///		Vertex array containing the entity's data.
		/// </summary>
		mrl_vertex_array_t* vertex_array;

		/// <summary>
		///		Parameters passed to mrl_draw_arrays or mrl_draw_arrays_indexed.
		/// </summary>
		mgl_u64_t offset, count;

		/// <summary>
		///		Number of instances that should be rendered.
		///		If this is set to 0, instancing is disabled.
		/// </summary>
		mgl_u32_t instance_count;

		/// <summary>
		///		Shader effect params.
		/// </summary>
		mge_shader_effect_params_t params;
	};

	// Contains data used to render all entities using an effect.
	struct mge_graphics_frame_effect_t
	{
		/// <summary>
		///		Effect handle.
		/// </summary>
		mge_shader_effect_t* effect;

		/// <summary>
		///		Number of entities.
		/// </summary>
		mgl_u32_t entity_count;

		/// <summary>
		///		Entity array.
		/// </summary>
		mge_graphics_frame_entity_t* entities;
	};

	struct mge_graphics_frame_t
	{
		/// <summary>
		///		Number of effects.
		/// </summary>
		mgl_u32_t effect_count;

		/// <summary>
		///		Effect array.
		/// </summary>
		mge_graphics_frame_effect_t* effects;
	};

	struct mge_graphics_manager_desc_t
	{
		/// <summary>
		///		Allocator used.
		/// </summary>
		void* allocator;

		/// <summary>
		///		Render device used on graphics operations.
		/// </summary>
		mrl_render_device_t* rd;

		/// <summary>
		///		Graphics manager viewport width.
		/// </summary>
		mgl_u32_t width;

		/// <summary>
		///		Graphics manager viewport height.
		/// </summary>
		mgl_u32_t height;
	};

	/// <summary>
	///		Initializes a graphics manager
	/// </summary>
	/// <param name="desc">Manager description</param>
	/// <returns>Manager handle</returns>
	mge_graphics_manager_t* mge_init_graphics_manager(const mge_graphics_manager_desc_t* desc);

	/// <summary>
	///		Terminates a graphics manager.
	/// </summary>
	/// <param name="manager">Manager handle</param>
	void mge_terminate_graphics_manager(mge_graphics_manager_t* manager);

	/// <summary>
	///		Resizes the graphics manager viewport.
	/// </summary>
	/// <param name="manager">Manager handle</param>
	/// <param name="width">New viewport width</param>
	/// <param name="height">New viewport height</param>
	void mge_resize_graphics_manager(mge_graphics_manager_t* manager, mgl_u32_t width, mgl_u32_t height);

	/// <summary>
	///		Creates a new shader effect.
	/// </summary>
	/// <param name="manager">Manager handle</param>
	/// <param name="desc">Effect description</param>
	/// <returns>Effect handle</returns>
	mge_shader_effect_t* mge_create_shader_effect(mge_graphics_manager_t* manager, const mge_shader_effect_desc_t* desc);

	/// <summary>
	///		Destroys a shader effect.
	/// </summary>
	/// <param name="manager">Manager handle</param>
	/// <param name="effect">Effect handle</param>
	void mge_destroy_shader_effect(mge_graphics_manager_t* manager, mge_shader_effect_t* effect);

	/// <summary>
	///		Renders a frame using the graphics manager.
	/// </summary>
	/// <param name="manager">Manager handle</param>
	/// <param name="frame">Frame data</param>
	void mge_graphics_manager_render(mge_graphics_manager_t* manager, const mge_graphics_frame_t* frame);

#ifdef __cplusplus
}
#endif
#endif
