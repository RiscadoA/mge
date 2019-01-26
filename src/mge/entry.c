#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

#include <mge/resource/manager.h>
#include <mge/scene/manager.h>

#include <mgl/entry.h>
#include <mgl/memory/allocator.h>

int main(int argc, char** argv)
{
	mge_game_locator_t locator;
	
	mge_internal_init_log();

	// Init MGL
	mgl_error_t err = mgl_init();
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to initialize MGL", err);
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Initialized MGL successfully\n");

	// Get config
	mge_engine_config_t config;
	mge_load_config(argc, argv, &config);
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Loaded engine configuration successfully\n");

	// Init engine
	{
		// Init resource manager
		locator.resource_manager = mge_init_resource_manager(mgl_standard_allocator, config.max_resource_count);

		// Init scene manager
		locator.scene_manager = mge_init_scene_manager(mgl_standard_allocator, config.max_scene_node_count);

		MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Initialized engine successfully\n");
	}
	
	// Load game
	mge_game_load(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Loaded game successfully\n");

	// Run engine
	// TO DO

	// Unload game
	mge_game_unload(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Unloaded game successfully\n");

	// Terminate engine
	{
		// Terminate scene manager
		mge_terminate_scene_manager(locator.scene_manager);

		// Terminate resource manager
		mge_terminate_resource_manager(locator.resource_manager);

		MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated engine successfully\n");
	}

	// Terminate MGL
	mgl_terminate();
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated MGL successfully\n");

	mge_internal_terminate_log();
	return 0;
}