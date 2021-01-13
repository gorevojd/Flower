/*
	 This file was generated automatically.
	 Do not modify it until you know what you're doing. :)
	 It contains information about a game modes, which.
	 will be used by the engine. This data was extracted by.
	 parser (preprocessor.cpp). modes.json was parsed.
	 Modes.json contains an info about the game modes.
	 You can add and modify your game modes there.
	 I wish you all the best! 07 Jan 2021
*/
#ifndef META_SCENES_H
#define META_SCENES_H

#include "graph_show.cpp"
#include "mode_rubiks.cpp"
#include "test_game.cpp"

GLOBAL_VARIABLE const char* MetaScene_Names[] = 
{
	"TestGame",
	"GraphShow",
	"RubiksCube",
};

GLOBAL_VARIABLE scene_init* MetaScene_InitFunctions[] = 
{
	TestGameInit,
	GraphShowInit,
	RubiksCubeInit,
};

GLOBAL_VARIABLE scene_update* MetaScene_UpdateFunctions[] = 
{
	TestGameUpdate,
	GraphShowUpdate,
	RubiksCubeUpdate,
};

#endif //META_GAME_MODES_H
