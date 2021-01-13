#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

GLOBAL_VARIABLE input_system* Global_Input;
GLOBAL_VARIABLE time* Global_Time;
GLOBAL_VARIABLE asset_system* Global_Assets;
GLOBAL_VARIABLE ui_state* Global_UI;
GLOBAL_VARIABLE render_commands* Global_RenderCommands;

#if defined(INTERNAL_BUILD)
debug_global_table* Global_DebugTable;
#endif

#include "flower_input.cpp"
#include "flower_asset.cpp"
#include "flower_render.cpp"
#include "flower_ui.cpp"
#include "flower_animation.cpp"
#include "flower_gameplay.cpp"
#include "flower_debug.cpp"

#include "flower_scene.cpp"
#include "meta_scenes.cpp"

INTERNAL_FUNCTION inline void InitScene(game* Game, int SceneIndex)
{
    scene* Scene = &Game->Scenes[SceneIndex];
    
    CopyStringsSafe(Scene->Name, ArrayCount(Scene->Name), (char*)MetaScene_Names[SceneIndex]);
    Scene->Init_ = MetaScene_InitFunctions[SceneIndex];
    Scene->Update_ = MetaScene_UpdateFunctions[SceneIndex];
    
    Scene->Arena = Game->Arena;
    Scene->SceneState = 0;
    Scene->Initialized = false;
}

INTERNAL_FUNCTION int FindSceneByName(char* Name)
{
    int Result = 0;
    
    for(int SceneIndex = 0;
        SceneIndex < ArrayCount(MetaScene_Names);
        SceneIndex++)
    {
        if(StringsAreEqual(Name, (char*)MetaScene_Names[SceneIndex]))
        {
            Result = SceneIndex;
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION void InitGameModes(game* Game)
{
    int NumNames = ArrayCount(MetaScene_Names);
    int NumInitFunctions = ArrayCount(MetaScene_InitFunctions);
    int NumUpdateFunctions = ArrayCount(MetaScene_UpdateFunctions);
    
    Assert(NumNames == NumInitFunctions &&
           NumNames == NumUpdateFunctions);
    
    Game->Scenes = PushArray(Game->Arena, scene, NumNames);
    Game->NumScenes = NumNames;
    
    for(int SceneIndex = 0;
        SceneIndex < Game->NumScenes;
        SceneIndex++)
    {
        InitScene(Game, SceneIndex);
    }
}

INTERNAL_FUNCTION void InitGame(game* Game, memory_arena* Arena)
{
    Game->Arena = Arena;
    
    Global_Time = PushStruct(Arena, time);
    InitInput(Arena);
    InitAssetSystem(Arena);
    InitUI(Arena);
    InitRender(Arena);
    DEBUGInit(Arena);
    
    InitGameModes(Game);
    
    Game->CurrentSceneIndex = FindSceneByName("RubiksCube");
    Game->NextSceneIndex = Game->CurrentSceneIndex;
}

INTERNAL_FUNCTION void UpdateGame(game* Game)
{
    // NOTE(Dima): Processing Input
    Platform.ProcessInput();
    
    // NOTE(Dima): Push UI
    char CharBuffer[64];
    stbsp_sprintf(CharBuffer, "Time: %.2f", Global_Time->Time);
    
    char FPSBuffer[64];
    stbsp_sprintf(FPSBuffer, "FPS: %.0f", 1.0f / Global_Time->DeltaTime);
    
    char FrameTimeBuffer[64];
    stbsp_sprintf(FrameTimeBuffer, "FrameTime ms: %.2f", Global_Time->DeltaTime * 1000.0f);
    
    
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    ui_params ParamsUI = {};
    ParamsUI.Commands = Global_RenderCommands;
    ParamsUI.Font = &Global_Assets->TimesNewRoman;
    ParamsUI.Scale = 1.0f;
    ParamsUI.WindowDims = WndDims;
    SetParamsUI(ParamsUI);
    
#if 1        
    BeginLayout();
    Text("Hello Twitch and YouTube!");
    Text(CharBuffer);
    Text(FPSBuffer);
    Text(FrameTimeBuffer);
    EndLayout();
#endif
    
    // NOTE(Dima): Updating game
    scene* Scene = Game->Scenes + Game->CurrentSceneIndex;
    Scene->Update();
    
    if(GetKey(Key_LeftShift) && GetKey(Key_LeftControl))
    {
        
        if(GetKeyDown(Key_N))
        {
            ChangeScene(Game, (Game->CurrentSceneIndex + 1) % Game->NumScenes);
        }
        else if(GetKeyDown(Key_P))
        {
            int NewSceneIndex = Game->CurrentSceneIndex - 1;
            if(NewSceneIndex < 0)
            {
                NewSceneIndex = Game->NumScenes - 1;
            }
            ChangeScene(Game, NewSceneIndex);
        }
    }
    
    if(Game->NextSceneIndex != Game->CurrentSceneIndex)
    {
        Game->CurrentSceneIndex = Game->NextSceneIndex;
    }
    
    // NOTE(Dima): Debug update
    DEBUGUpdate();
    
    // NOTE(Dima): Render everything
    Global_RenderCommands->FontAtlas = &Global_Assets->FontsAtlas;
    Global_RenderCommands->ScreenOrthoProjection = OrthographicProjection(WndDims->Width, 
                                                                          WndDims->Height);
    
    BeginRender();
    Platform.Render();
    EndRender();
    
    // NOTE(Dima): Swapping buffers
    Platform.SwapBuffers();
}