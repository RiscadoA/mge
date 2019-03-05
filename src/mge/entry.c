#include <mge/game.h>
#include <mge/config.h>
#include <mge/log.h>

#include <mge/resource/manager.h>
#include <mge/scene/manager.h>
#include <mge/graphics/manager.h>

#include <mgl/entry.h>
#include <mgl/memory/allocator.h>
#include <mgl/input/windows_window.h>
#include <mrl/ogl_330_render_device.h>

static mgl_bool_t running;

static void mrl_render_device_warning(mrl_error_t error, const mgl_chr8_t* msg)
{
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Render device warning callback called:\n");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, msg);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"\n");
}

static void mrl_render_device_error(mrl_error_t error, const mgl_chr8_t* msg)
{
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Render device error callback called:\n");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, msg);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"\n");
}

static void close_callback(const mgl_action_t* action, mgl_enum_t state)
{
	running = MGL_FALSE;
}

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

	// Init input manager
	mgl_input_manager_t input_manager;
	mgl_init_input_manager(&input_manager);
	locator.input_manager = &input_manager;

	// Init window
	mgl_windows_window_t window;
	{
		mgl_windows_window_settings_t settings;
		settings.width = config.window_width;
		settings.height = config.window_height;
		settings.title = u8"Magma Game Engine " MGE_VERSION;
		settings.mode = config.window_fullscreen ? MGL_WINDOW_MODE_FULLSCREEN : MGL_WINDOW_MODE_WINDOWED;
		settings.input_manager = locator.input_manager;
		err = mgl_open_windows_window(&window, &settings);
	}
	locator.window = &window;

	// Init render device
	{
		mrl_render_device_desc_t desc = MRL_DEFAULT_RENDER_DEVICE_DESC;
		desc.allocator = mgl_standard_allocator;
		desc.window = locator.window;

		mrl_render_device_hint_error_callback_t error_callback = &mrl_render_device_error;
		mrl_render_device_hint_warning_callback_t warning_callback = &mrl_render_device_warning;

		mrl_hint_t warning_hint = MRL_DEFAULT_HINT;
		warning_hint.data = &warning_callback;
		warning_hint.type = MRL_HINT_RENDER_DEVICE_WARNING_CALLBACK;

		mrl_hint_t error_hint = MRL_DEFAULT_HINT;	
		error_hint.data = &error_callback;
		error_hint.type = MRL_HINT_RENDER_DEVICE_ERROR_CALLBACK;
		error_hint.next = &warning_hint;

		desc.hints = &error_hint;

		mrl_error_t err = mrl_init_ogl_330_render_device(&desc, &locator.render_device);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mrl_error(MGE_LOG_ENGINE, u8"Failed to initialize MRL render device", err);
	}

	// Init engine
	{
		// Init resource manager
		locator.resource_manager = mge_init_resource_manager(mgl_standard_allocator, config.max_resource_count);

		// Init scene manager
		locator.scene_manager = mge_init_scene_manager(mgl_standard_allocator, config.max_scene_node_count);

		// Init graphics manager
		{
			mge_graphics_manager_desc_t desc;
			desc.allocator = mgl_standard_allocator;
			desc.width = config.window_width;
			desc.height = config.window_height;
			desc.rd = locator.render_device;
			locator.graphics_manager = mge_init_graphics_manager(&desc);
		}

		MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Initialized engine successfully\n");
	}
	
	// Load game
	mge_game_load(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Loaded game successfully\n");

	// Set engine as running
	running = MGL_TRUE;

	// Add window close action callback
	err = mgl_add_action_callback(
		locator.input_manager,
		mgl_get_window_action(locator.window, MGL_WINDOW_CLOSE),
		&close_callback);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to add window close action callback", err);

	while (running)
	{
		mgl_poll_window_events(locator.window);
	}

	// Unload game
	mge_game_unload(&locator);
	MGE_LOG_VERBOSE_1(MGE_LOG_GAME_CLIENT, u8"Unloaded game successfully\n");

	// Terminate engine
	{
		// Terminate graphics manager
		mge_terminate_graphics_manager(locator.graphics_manager);

		// Terminate scene manager
		mge_terminate_scene_manager(locator.scene_manager);

		// Terminate resource manager
		mge_terminate_resource_manager(locator.resource_manager);

		MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated engine successfully\n");
	}

	// Terminate render device
	mrl_terminate_ogl_330_render_device(locator.render_device);

	// Terminate window
	mgl_close_windows_window(&window);

	// Terminate MGL
	mgl_terminate();
	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Terminated MGL successfully\n");

	mge_internal_terminate_log();
	return 0;
}