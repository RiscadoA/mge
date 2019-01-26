#include <mge/scene/manager.h>
#include <mge/scene/component.h>
#include <mge/scene/node.h>
#include <mge/log.h>

#include <mgl/string/manipulation.h>

mge_scene_manager_t * mge_init_scene_manager(void * allocator, mgl_u64_t max_node_count)
{
	MGL_DEBUG_ASSERT(allocator != NULL && max_node_count > 0);

	mge_scene_manager_t* manager;

	// Allocate manager
	mgl_error_t err = mgl_allocate(allocator, sizeof(mge_scene_manager_t), (void**)&manager);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate scene manager", err);

	// Allocate nodes
	err = mgl_allocate(allocator, max_node_count * sizeof(mge_scene_node_t), (void**)&manager->nodes);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate nodes array on scene manager", err);

	manager->allocator = allocator;
	manager->max_node_count = max_node_count;

	// Init nodes
	for (mgl_u64_t i = 0; i < manager->max_node_count; ++i)
		manager->nodes[i].trash = MGL_TRUE;

	// Init root
	manager->root = &manager->nodes[0];
	manager->root->active = MGL_TRUE;
	manager->root->trash = MGL_FALSE;
	manager->root->first_child = NULL;
	manager->root->first_component = NULL;
	mgl_f32m4x4_identity(&manager->root->transform.local);
	mgl_f32m4x4_identity(&manager->root->transform.global);
	manager->root->transform.dirty = MGL_FALSE;
	manager->root->manager = manager;
	mgl_str_copy(u8"[root]", manager->root->name, MGE_MAX_SCENE_NODE_NAME_SIZE);

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully initialized scene manager\n");

	return manager;
}

void mge_terminate_scene_manager(mge_scene_manager_t * manager)
{
	MGL_DEBUG_ASSERT(manager != NULL);

	// Destroy nodes and components
	mge_clear_children_scene_node(manager->root);
	mge_clear_components_scene_node(manager->root);

	// Deallocate nodes
	mgl_error_t err = mgl_deallocate(manager->allocator, manager->nodes);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate nodes array on resource manager", err);

	// Deallocate manager
	err = mgl_deallocate(manager->allocator, manager);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate scene manager", err);

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully terminated scene manager\n");
}

mge_scene_node_t* mge_create_scene_node(mge_scene_node_t * parent, const mgl_chr8_t * name)
{
	MGL_DEBUG_ASSERT(parent != NULL);
	if (name == NULL)
		name = u8"[unnamed]";

	mge_scene_node_t* node = NULL;
	for (mgl_u32_t i = 0; i < parent->manager->max_node_count; ++i)
		if (parent->manager->nodes[i].trash)
		{
			node = &parent->manager->nodes[i];
			break;
		}
	if (node == NULL)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to create scene node, scene node limit surpassed");

	node->trash = MGL_FALSE;
	node->active = MGL_TRUE;
	node->first_child = NULL;
	node->first_component = NULL;
	mgl_f32m4x4_identity(&node->transform.local);
	node->transform.dirty = MGL_TRUE;
	node->manager = parent->manager;
	mgl_str_copy(name, node->name, MGE_MAX_SCENE_NODE_NAME_SIZE);

	// Add to parent and update transform
	mge_scene_add_child(parent, node);
	mge_scene_node_update_transform(node);

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Created scene node '");
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, node->name);
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"'\n");

	return node;
}

void mge_destroy_scene_node(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);

	if (node->parent == NULL)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to destroy scene node, the root scene node cannot be destroyed");
	if (node->trash)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to destroy scene node, this scene node was already destroyed");

	mge_clear_children_scene_node(node);
	mge_clear_components_scene_node(node);

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Destroyed scene node '");
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, node->name);
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"'\n");

	mge_scene_remove_child(node->parent, node);
	node->trash = MGL_TRUE;
}

void mge_clear_children_scene_node(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);

	// Destroy all of the children on the node
	mge_scene_node_t* c = node->first_child;
	while (c != NULL)
	{
		mge_scene_node_t* n = c->next;
		mge_destroy_scene_node(c);
		c = n;
	}
	node->first_child = NULL;
}

void mge_clear_components_scene_node(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);

	// Destroy all of the components on the node
	while (node->first_component != NULL)
	{
		mge_scene_component_t* c = node->first_component;
		node->first_component = c->next;
		c->destroy_func(c);
	}
}
