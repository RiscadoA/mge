#ifndef MGE_SCENE_MANAGER_H
#define MGE_SCENE_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>

	typedef struct mge_scene_node_t mge_scene_node_t;
	typedef struct mge_scene_component_t mge_scene_component_t;
	typedef struct mge_scene_manager_t mge_scene_manager_t;

	struct mge_scene_manager_t
	{
		void* allocator;

		mgl_u64_t max_node_count;
		mge_scene_node_t* nodes;
		mge_scene_node_t* root;
	};

	/// <summary>
	///		Initializes a scene manager
	/// </summary>
	/// <param name="allocator">Allocator used</param>
	/// <param name="max_node_count">Maximum scene node count</param>
	/// <returns>Pointer to manager</returns>
	mge_scene_manager_t* mge_init_scene_manager(void* allocator, mgl_u64_t max_node_count);

	/// <summary>
	///		Terminates a scene manager.
	/// </summary>
	/// <param name="manager">Pointer to manager</param>
	void mge_terminate_scene_manager(mge_scene_manager_t* manager);

	/// <summary>
	///		Creates a new scene node.
	/// </summary>
	/// <param name="parent">Pointer to parent node</param>
	/// <param name="name">Node name (can be NULL)</param>
	mge_scene_node_t* mge_create_scene_node(mge_scene_node_t* parent, const mgl_chr8_t* name);

	/// <summary>
	///		Destroys a scene node and all of its children.
	/// </summary>
	/// <param name="node">Pointer to node</param>
	void mge_destroy_scene_node(mge_scene_node_t* node);

	/// <summary>
	///		Destroys every child in the scene node, without destroying the node itself.
	/// </summary>
	/// <param name="node">Node</param>
	void mge_clear_children_scene_node(mge_scene_node_t* node);

	/// <summary>
	///		Destroys every component in the scene node, without destroying the node itself.
	/// </summary>
	/// <param name="node">Node</param>
	void mge_clear_components_scene_node(mge_scene_node_t* node);

#ifdef __cplusplus
}
#endif
#endif
