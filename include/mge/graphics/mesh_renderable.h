#ifndef MGE_GRAPHICS_STATIC_MESH_H
#define MGE_GRAPHICS_STATIC_MESH_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mge/scene/component.h>

	typedef struct mge_mesh_renderable_t mge_mesh_renderable_t;

	/// <summary>
	///		Mesh renderable component.
	///		Renders an instance of a mesh.
	/// </summary>
	struct mge_mesh_renderable_t
	{
		mge_scene_component_t base;
		mgl_u32_t mesh_id;
		mgl_u32_t material_id;
	};

#ifdef __cplusplus
}
#endif
#endif
