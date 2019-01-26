#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

void mge_game_get_config(mge_engine_config_t* config)
{
	config->debug_mode = MGL_TRUE;

	MGE_LOG_VERBOSE_0(MGE_LOG_GAME_CLIENT, u8"Engine config changed (LOG TEST)\n");
}

void mge_game_load(mge_game_locator_t* locator)
{
	MGE_LOG_VERBOSE_0(MGE_LOG_GAME_CLIENT, u8"Game loaded (LOG TEST)\n");
}

void mge_game_unload(mge_game_locator_t* locator)
{
	MGE_LOG_VERBOSE_0(MGE_LOG_GAME_CLIENT, u8"Game unloaded (LOG TEST)\n");

}
