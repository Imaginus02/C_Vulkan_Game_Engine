#include "game.h"

#include <entry.h>

// TODO: remove this
#include <core/kmemory.h>

// Externally-defined function to create a game.
b8 create_game(game* out_game)
{
	out_game->app_config.start_pos_x = 100;
	out_game->app_config.start_pos_y = 100;
	out_game->app_config.width = 1200;
	out_game->app_config.height = 600;
	out_game->app_config.name = "Testbed";
	out_game->update = game_update;
	out_game->render = game_render;
	out_game->on_resize = game_on_resize;
	out_game->initialize = game_initialize;

	// Create the game state
	out_game->state = kallocate(sizeof(game_state), MEMORY_TAG_GAME);
	

    out_game->application_state = 0;

    return true;
}