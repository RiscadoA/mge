#ifndef MGE_GAME_H
#define MGE_GAME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <mgl/input/input_manager.h>

typedef struct mge_engine_config_t mge_engine_config_t;
typedef struct mge_resource_manager_t mge_resource_manager_t;
typedef struct mge_scene_manager_t mge_scene_manager_t;
typedef struct mge_graphics_manager_t mge_graphics_manager_t;
typedef struct mrl_render_device_t mrl_render_device_t;
typedef struct mge_game_locator_t mge_game_locator_t;

struct mge_game_locator_t
{
	mge_resource_manager_t* resource_manager;
	mge_scene_manager_t* scene_manager;
	mge_graphics_manager_t* graphics_manager;

	mgl_input_manager_t* input_manager;
	void* window;

	mrl_render_device_t* render_device;
};

extern void mge_game_load(mge_game_locator_t* locator);

extern void mge_game_unload(mge_game_locator_t* locator);

extern void mge_game_get_config(mge_engine_config_t* config);

#ifdef __cplusplus
}
#endif
#endif