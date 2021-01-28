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

#include "mode_graph_show.cpp"
#include "mode_rubiks.cpp"
#include "mode_test_game.cpp"

GLOBAL_VARIABLE const char* MetaScene_Names[] = 
{
	"TestGame",
	"GraphShow",
	"RubiksCube",
};

GLOBAL_VARIABLE scene_init* MetaScene_InitFunctions[] = 
{
	TestGame_Init,
	GraphShow_Init,
	RubiksCube_Init,
};

GLOBAL_VARIABLE scene_update* MetaScene_UpdateFunctions[] = 
{
	TestGame_Update,
	GraphShow_Update,
	RubiksCube_Update,
};

GLOBAL_VARIABLE scene_ongui* MetaScene_OnGUIFunctions[] = 
{
	TestGame_OnGUI,
	0,
	0,
};

#endif //META_GAME_MODES_H
