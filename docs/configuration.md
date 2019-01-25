# Configuration

The game-specific engine configuration is loaded by the function `void mge_game_get_config(mge_engine_configuration_t* config)` which is defined in the game code.

Each option value in the engine configuration can be represented by a string.

## Command Line Arguments

The options in the configuration loaded by the game can be overridden in the command line arguments in the following way: `game.exe --mge-[option] [value]<`

## Options

- `-mge-debug-mode [boolean]` - Sets debug mode to `boolean` (on|true|1 or off|false|0).

