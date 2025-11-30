#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"


// Externally-defined function to create a game.
extern b8 create_game(game* out_game);


/**
 * The main entry point of the application.
 */
int main(void)
{
	//Request the game instance from the application
	game game_inst;
	if (!create_game(&game_inst))
	{
		KFATAL("Failed to create game instance.");
		return -1;
	}

	// Ensure the function pointers are not NULL
	if (!game_inst.initialize || !game_inst.update || !game_inst.render || !game_inst.on_resize)
	{
		KFATAL("Game function pointers are NULL.");
		return -2;
	}


	// Initialize the application
	if (!application_create(&game_inst))
	{
		KINFO("Failed to create application.");
		return 1;
	}

	// Beginning of the game loop
	if (!application_run())
	{
		KINFO("Failed to run application.");
		return 2;
	}

	// Shutdown the application

	return 0;
}