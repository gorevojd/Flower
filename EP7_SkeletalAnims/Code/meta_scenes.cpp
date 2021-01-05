#ifndef META_SCENES_H
#define META_SCENES_H

#include "test_game.cpp"

GLOBAL_VARIABLE const char* MetaScene_Names[] = 
{
    "TestGame",
};

GLOBAL_VARIABLE scene_init* MetaScene_InitFunctions[] = 
{
    TestGameInit,
};

GLOBAL_VARIABLE scene_update* MetaScene_UpdateFunctions[] = 
{
    TestGameUpdate,
};

#endif //META_GAME_MODES_H
