#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

#include <mge/scene/manager.h>

void mge_game_get_config(mge_engine_config_t* config)
{
	config->debug_mode = MGL_TRUE;
}

void mge_game_load(mge_game_locator_t* locator)
{
	mge_scene_node_t* node1 = mge_create_scene_node(locator->scene_manager->root, u8"my_node");
	mge_scene_node_t* node2 = mge_create_scene_node(node1, u8"my_child_node");
	mge_scene_node_t* node3 = mge_create_scene_node(node2, u8"my_child_node_1");
	mge_scene_node_t* node4 = mge_create_scene_node(node2, u8"my_child_node_2");
	mge_scene_node_t* node5 = mge_create_scene_node(node1, u8"my_child_node_3");
	mge_destroy_scene_node(node2);
}

void mge_game_unload(mge_game_locator_t* locator)
{
	
}
