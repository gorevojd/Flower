#ifndef FLOWER_GAME_H
#define FLOWER_GAME_H

#include "flower_defines.h"
#include "flower_math.h"
#include "flower_strings.h"
#include "flower_platform.h"
#include "flower_mem.h"

#include "flower_input.h"
#include "flower_render_primitives.h"
#include "flower_asset.h"
#include "flower_opengl.h"
#include "flower_render.h"
#include "flower_ui.h"

#include "flower_scene.h"

struct game
{
    scene* Scenes;
    int NumScenes;
    
    int CurrentSceneIndex;
    
    memory_arena* Arena;
};

#endif //FLOWER_GAME_MODE_H
