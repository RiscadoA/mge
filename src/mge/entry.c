#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

#include <mgl/entry.h>

int main(int argc, char** argv)
{
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
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Initialized engine successfully\n");

	// Fill game locator
	mge_game_locator_t locator;
	locator.resource_manager = NULL;
	locator.scene_manager = NULL;

	// Load game
	mge_game_load(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Loaded game successfully\n");

	// Run engine
	 

	// Unload game
	mge_game_unload(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Unloaded game successfully\n");

	// Terminate engine
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated engine successfully\n");

	// Terminate MGL
	mgl_terminate();
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated MGL successfully\n");

	mge_internal_terminate_log();
	return 0;
}