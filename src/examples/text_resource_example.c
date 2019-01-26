#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

#include <mgl/stream/stream.h>

#include <mge/resource/manager.h>
#include <mge/resource/text.h>

#include <mgl/file/windows_standard_archive.h>

mgl_windows_standard_archive_t archive;

void mge_game_get_config(mge_engine_config_t* config)
{
	config->debug_mode = MGL_TRUE;
}

void mge_game_load(mge_game_locator_t* locator)
{
	// Register archive
	mgl_error_t e = mgl_init_windows_standard_archive(&archive, mgl_standard_allocator, MGE_EXAMPLES_DATA_DIRECTORY);
	if (e != MGL_ERROR_NONE)
		mge_fatal_error(MGE_LOG_GAME_CLIENT, u8"Failed to init windows archive");
	mgl_register_archive(u8"data", &archive);

	// Add info file
	mge_add_resource_info_file(locator->resource_manager, u8"data/text_resource.mri");

	// Find resource
	mge_resource_t* rsc = mge_find_resource(locator->resource_manager, u8"text_resource");

	// Open resource
	mge_text_resource_access_t access;
	mge_open_resource(rsc, &access, MGE_RESOURCE_TEXT);

	// Print data
	mgl_print(mgl_stdout_stream, u8"Text size: ");
	mgl_print_u64(mgl_stdout_stream, access.data->size, 10);
	mgl_print(mgl_stdout_stream, u8"\nText:\n");
	mgl_print(mgl_stdout_stream, access.data->text);

	// Close resource
	mge_close_resource(&access);

	// Wait for input
	mgl_read_chars_until(mgl_stdin_stream, NULL, 0, NULL, u8"\n");
}

void mge_game_unload(mge_game_locator_t* locator)
{


	mgl_unregister_archive(&archive);
	mgl_terminate_windows_standard_archive(&archive);
}
