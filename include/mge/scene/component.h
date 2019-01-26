#ifndef MGE_SCENE_COMPONENT_H
#define MGE_SCENE_COMPONENT_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>

#include <mgl/type.h>
#include <mgl/math/matrix4x4.h>

#define MGE_MAX_SCENE_NODE_NAME_SIZE 32

	typedef struct mge_scene_node_t mge_scene_node_t;
	typedef struct mge_scene_component_t mge_scene_component_t;
	typedef struct mge_scene_manager_t mge_scene_manager_t;

	/// <summary>
	///		Base struct for scene components.
	///		Components should be structured as:
	///		<code>
	///			struct my_component
	///			{
	///				mge_scene_component_t base;
	///				(...) // Properties
	///			};
	///		</code>
	/// 
	///		Each component type should be created and handled by its own manager.
	/// </summary>
	struct mge_scene_component_t
	{
		/// <summary>
		///		Component type.
		/// </summary>
		mgl_enum_u32_t type;

		/// <summary>
		///		Is this scene component active?
		///		If this is set to MGL_FALSE, this component will not be updated.
		/// </summary>
		mgl_bool_t active;

		/// <summary>
		///		Parent node.
		/// </summary>
		mge_scene_node_t* node;

		/// <summary>
		///		Next sibling component.
		/// </summary>
		mge_scene_component_t* next;

		/// <summary>
		///		Function called when the component is destroyed.
		/// </summary>
		void(*destroy_func)(void* component);
	};

#ifdef __cplusplus
}
#endif
#endif
