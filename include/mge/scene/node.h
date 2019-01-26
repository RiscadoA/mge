#ifndef MGE_SCENE_NODE_H
#define MGE_SCENE_NODE_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>
#include <mgl/math/matrix4x4.h>

#define MGE_MAX_SCENE_NODE_NAME_SIZE 32
	
	typedef struct mge_scene_node_t mge_scene_node_t;
	typedef struct mge_scene_component_t mge_scene_component_t;
	typedef struct mge_scene_manager_t mge_scene_manager_t;

	struct mge_scene_node_t
	{
		/// <summary>
		///		Scene manager which manages this scene node.
		/// </summary>
		mge_scene_manager_t* manager;

		/// <summary>
		///		Is this scene node active?
		///		If this is set to MGL_FALSE, this node and all of its children stop havung their components updated.
		/// </summary>
		mgl_bool_t active;

		/// <summary>
		///		Is this scene node trash?
		///		If this is set to MGL_TRUE, the node is set as trash and cannot be used anymore.
		///		WARNING: This should not be set manually.
		/// </summary>
		mgl_bool_t trash;

		/// <summary>
		///		Parent node.
		///		WARNING: This should not be set manually.
		/// </summary>
		mge_scene_node_t* parent;

		/// <summary>
		///		First child node.
		///		WARNING: This should not be set manually.
		/// </summary>
		mge_scene_node_t* first_child;

		/// <summary>
		///		Next sibling node.
		///		WARNING: This should not be set manually.
		/// </summary>
		mge_scene_node_t* next;

		/// <summary>
		///		First component in this node.
		///		WARNING: This should not be set manually.
		/// </summary>
		mge_scene_component_t* first_component;

		/// <summary>
		///		Scene node name.
		/// </summary>
		mgl_chr8_t name[MGE_MAX_SCENE_NODE_NAME_SIZE];

		/// <summary>
		///		Node transform.
		/// </summary>
		struct
		{
			/// <summary>
			///		Node local transform matrix.
			///		When changes are made to this value, transform.dirty should be set to MGL_TRUE.
			/// </summary>
			mgl_f32m4x4_t local;

			/// <summary>
			///		Node global transform matrix.
			///		This should only be accessed through mge_scene_node_get_global_transform.
			/// </summary>
			mgl_f32m4x4_t global;

			/// <summary>
			///		Node dirty flag.
			///		This should be set to MGL_TRUE after changes are made to 'transform.local'.
			///		WARNING: This should not be set manually, instead, call mge_scene_node_set_dirty
			/// </summary>
			mgl_bool_t dirty;
		} transform;
	};

	/// <summary>
	///		Gets the local transform of a scene node.
	/// </summary>
	/// <param name="node">Node</param>
	/// <returns>Pointer to local transform</returns>
	mgl_f32m4x4_t* mge_scene_node_get_local_transform(mge_scene_node_t* node);

	/// <summary>
	///		Gets the global transform of a scene node.
	///		This function updates the global transform if the dirty flag is set.
	/// </summary>
	/// <param name="node">Node</param>
	/// <returns>Pointer to global transform</returns>
	mgl_f32m4x4_t* mge_scene_node_get_global_transform(mge_scene_node_t* node);

	/// <summary>
	///		Updates a scene node transform, clearing the dirty flag.
	/// </summary>
	/// <param name="node">Node</param>
	void mge_scene_node_update_transform(mge_scene_node_t* node);

	/// <summary>
	///		Sets a node's transform dirty flag.
	/// </summary>
	/// <param name="node">Node</param>
	void mge_scene_node_set_dirty(mge_scene_node_t* node);

	/// <summary>
	///		Adds a component to a node.
	///		WARNING: This function shouldn't be used directly.
	/// </summary>
	/// <param name="node">Node</param>
	/// <param name="component">Component</param>
	void mge_scene_add_component(mge_scene_node_t* node, mge_scene_component_t* component);

	/// <summary>
	///		Removes a component from a node.
	///		WARNING: This function shouldn't be used directly.
	/// </summary>
	/// <param name="node">Node</param>
	/// <param name="component">Component</param>
	void mge_scene_remove_component(mge_scene_node_t* node, mge_scene_component_t* component);

	/// <summary>
	///		Adds a child node to a parent node.
	///		WARNING: This function shouldn't be used directly.
	/// </summary>
	/// <param name="parent">Parent node</param>
	/// <param name="child">Child node</param>
	void mge_scene_add_child(mge_scene_node_t* parent, mge_scene_node_t* child);

	/// <summary>
	///		Removes a child node from a parent node.
	///		WARNING: This function shouldn't be used directly.
	///		This does not delete the child node.
	/// </summary>
	/// <param name="parent">Parent node</param>
	/// <param name="child">Child node</param>
	void mge_scene_remove_child(mge_scene_node_t* parent, mge_scene_node_t* child);

#ifdef __cplusplus
}
#endif
#endif
