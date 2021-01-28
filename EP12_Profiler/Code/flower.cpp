GLOBAL_VARIABLE input_system* Global_Input;
GLOBAL_VARIABLE time* Global_Time;
GLOBAL_VARIABLE asset_system* Global_Assets;
GLOBAL_VARIABLE ui_state* Global_UI;
GLOBAL_VARIABLE render_commands* Global_RenderCommands;

#if defined(INTERNAL_BUILD)
debug_global_table* Global_DebugTable;
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "flower_random.cpp"
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
    
#ifdef INTERNAL_BUILD
    Game->ShowOverlays = true;
#endif
}

INTERNAL_FUNCTION void ShowUI(game* Game)
{
    // TODO(Dima): Start new frame here
    
    if(Game->ShowOverlays){
        if(BeginLayout("MainLayout"))
        {
            ShowText("Test format string %d, %f", 100, 123.0f);
            ShowText("Hello Twitch and YouTube!");
            ShowText("Time: %.2f", Global_Time->Time);
            ShowText("FPS: %.0f", 1.0f / Global_Time->DeltaTime);
            ShowText("FrameTime ms: %.2f", Global_Time->DeltaTime * 1000.0f);
            
            EndLayout();
        }
    }
}

INTERNAL_FUNCTION void UpdateGame(game* Game)
{
    FUNCTION_TIMING();
    
    // NOTE(Dima): Processing Input
    Platform.ProcessInput();
    
    UIBeginFrame();
    
    
    // NOTE(Dima): Updating game
    scene* Scene = Game->Scenes + Game->CurrentSceneIndex;
    Scene->Update();
    
    if(GetKeyDown(Key_Backquote))
    {
        Game->ShowOverlays = !Game->ShowOverlays;
    }
    
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
        
        if(GetMod(KeyMod_LeftAlt))
        {
            if(GetKeyDown(Key_C))
            {
                Platform.SetCapturingMouse(!Global_Input->CapturingMouse);
            }
            if(GetKeyDown(Key_D))
            {
                DEBUGToggleShowMenus();
            }
        }
    }
    
    if(Game->NextSceneIndex != Game->CurrentSceneIndex)
    {
        Game->CurrentSceneIndex = Game->NextSceneIndex;
    }
    
    // NOTE(Dima): Debug update
    DEBUGUpdate();
    
    // NOTE(Dima): Render everything
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    Global_RenderCommands->FontAtlas = &Global_Assets->FontsAtlas;
    Global_RenderCommands->ScreenOrthoProjection = OrthographicProjection(WndDims->Width, 
                                                                          WndDims->Height);
    
    BeginRender();
    Platform.Render();
    EndRender();
    
    // NOTE(Dima): Swapping buffers
    Platform.SwapBuffers();
}