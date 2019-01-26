#include <mge/config.h>
#include <mge/game.h>
#include <mge/log.h>

#include <mgl/error.h>
#include <mgl/string/manipulation.h>
#include <mgl/string/conversion.h>

static mgl_bool_t mge_config_parse_boolean(char* option_name, char* value)
{
	if (value == NULL)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"WARNING: Failed to parse option boolean value on option '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, option_name);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"', option is missing\n");
		return MGL_FALSE;
	}

	if (mgl_str_equal(value, u8"on") || mgl_str_equal(value, u8"true") || mgl_str_equal(value, u8"1"))
		return MGL_TRUE;
	else if (mgl_str_equal(value, u8"off") || mgl_str_equal(value, u8"false") || mgl_str_equal(value, u8"0"))
		return MGL_FALSE;

	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"WARNING: Failed to parse option boolean value '");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, value);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"', on option '");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, option_name);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"', boolean values can only be 'on', 'true', '1', 'off', 'false' and '0'\n");

	return MGL_FALSE;
}

static mgl_u64_t mge_config_parse_u64(char* option_name, char* value)
{
	if (value == NULL)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"WARNING: Failed to parse option U64 value on option '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, option_name);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"', option is missing\n");
		return MGL_FALSE;
	}

	mgl_u64_t ret;
	mgl_error_t err = mgl_u64_from_str(value, mgl_str_size(value), &ret, 10, NULL);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"WARNING: Failed to parse option U64 value '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, value);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"', on option '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, option_name);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"' (");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, mgl_get_error_string(err));
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, ")\n");
		return 1;
	}

	return ret;
}

void mge_load_config(int argc, char ** argv, mge_engine_config_t * config)
{
	MGL_DEBUG_ASSERT(config != NULL);
	*config = MGE_DEFAULT_ENGINE_CONFIG;
	mge_game_get_config(config);
	if (argc == 0 || argv == NULL)
		return;

	// Check all arguments
	for (int i = 0; i < argc; ++i)
	{
		// Check if it is a MGE option
		if (argv[i][0] == '-' && argv[i][1] == 'm' && argv[i][2] == 'g' && argv[i][3] == 'e' && argv[i][4] == '-')
		{
			// Get option name
			char* option = &argv[i][5];

			if (mgl_str_equal(option, u8"debug-mode"))
			{
				config->debug_mode = mge_config_parse_boolean(option, argv[i + 1]);
				if (config->debug_mode)
					MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"The option debug-mode was activated\n");
				else
					MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"The option debug-mode was deactivated\n");
				i += 1;
				continue;
			}
			else if (mgl_str_equal(option, u8"max-resource-count"))
			{
				config->max_resource_count = mge_config_parse_u64(option, argv[i + 1]);
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"The option max-resource-count was set to '");
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, argv[i + 1]);
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
				i += 1;
				continue;
			}
			else if (mgl_str_equal(option, u8"max-scene-node-count"))
			{
				config->max_scene_node_count = mge_config_parse_u64(option, argv[i + 1]);
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"The option max-scene-node-count was set to '");
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, argv[i + 1]);
				MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
				i += 1;
				continue;
			}
		}
	}
}
