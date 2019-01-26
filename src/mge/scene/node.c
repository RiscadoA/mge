#include <mge/scene/node.h>
#include <mge/scene/component.h>

#include <mge/log.h>

mgl_f32m4x4_t * mge_scene_node_get_local_transform(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);
	return &node->transform.local;
}

mgl_f32m4x4_t * mge_scene_node_get_global_transform(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);
	if (node->transform.dirty)
		mge_scene_node_update_transform(node);
	return &node->transform.global;
}

void mge_scene_node_update_transform(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);

	// Update parent transform
	if (node->parent != NULL && node->parent->transform.dirty)
		mge_scene_node_update_transform(node->parent);

	// Update the global transform matrix
	mgl_f32m4x4_mul(&node->parent->transform.global, &node->transform.local, &node->transform.global);
	node->transform.dirty = MGL_FALSE;
}

void mge_scene_node_set_dirty(mge_scene_node_t * node)
{
	MGL_DEBUG_ASSERT(node != NULL);

	mge_scene_node_t* c = node->first_child;
	while (c != NULL)
	{
		mge_scene_node_set_dirty(c);
		c = c->next;
	}

	node->transform.dirty = MGL_TRUE;
}

void mge_scene_add_component(mge_scene_node_t * node, mge_scene_component_t * component)
{
	MGL_DEBUG_ASSERT(node != NULL && component != NULL);
	component->next = node->first_component;
	component->node = node;
	node->first_component = component;
}

void mge_scene_remove_component(mge_scene_node_t * node, mge_scene_component_t * component)
{
	MGL_DEBUG_ASSERT(node != NULL && component != NULL);
	mge_scene_component_t* c = node->first_component;

	if (c == component)
		node->first_component = component->next;
	else
	{
		while (c->next != component)
		{
			c = c->next;
			if (c == NULL)
				mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to remove component from node, component not found");
		}
		c->next = component->next;
	}

	component->node = NULL;
	component->active = MGL_FALSE;
}

void mge_scene_add_child(mge_scene_node_t * parent, mge_scene_node_t * child)
{
	MGL_DEBUG_ASSERT(parent != NULL && child != NULL);
	child->next = parent->first_child;
	child->parent = parent;
	parent->first_child = child;
}

void mge_scene_remove_child(mge_scene_node_t * parent, mge_scene_node_t * child)
{
	MGL_DEBUG_ASSERT(parent != NULL && child != NULL);
	mge_scene_node_t* c = parent->first_child;

	if (c == child)
		parent->first_child = child->next;
	else
	{
		while (c->next != child)
		{
			c = c->next;
			if (c == NULL)
				mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to remove child from parent node, child node not found");
		}
		c->next = child->next;
	}

	child->parent = NULL;
	child->active = MGL_FALSE;
}
