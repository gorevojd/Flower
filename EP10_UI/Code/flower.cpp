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

#include "flower_input.cpp"
#include "flower_asset.cpp"
#include "flower_render.cpp"
#include "flower_ui.cpp"
#include "flower_animation.cpp"
#include "flower_gameplay.cpp"
#include "flower_debug.cpp"

#include "flower_scene.cpp"
#include "meta_scenes.cpp"

#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#include "imgui.h"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_tables.cpp"
#include "imgui_demo.cpp"

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

INTERNAL_FUNCTION void ShowOverlays(game* Game)
{
    ImGui::NewFrame();
    
    // NOTE(Dima): Push UI
    if(Game->ShowOverlays)
    {
        LOCAL_PERSIST bool ShowDemo = false;
        if(ShowDemo)
        {
            ImGui::ShowDemoWindow(&ShowDemo);
        }
        
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("First Window"))
        {
#if 1
            ImGui::BeginChild("left pane", ImVec2(150, 0), true);
            
            ImGui::EndChild();
            
            
            ImGui::SameLine();
            
            ImGui::BeginChild("right pane");
            ImGui::Text("This is my first text with ImGUI");
            ImGui::BulletText("This is the bullet text");
            ImGui::BulletText("This is another one followed by separator");
            
            ImGui::Checkbox("Show Demo Window", &ShowDemo);
            
            ImGui::Separator();
            
            if(ImGui::CollapsingHeader("Collapsing Header"))
            {
                static v4 Color1;
                static v3 Color2;
                
                ImGui::ColorEdit4("Color 1", Color1.e);
                ImGui::SameLine(); HelpMarker("Click on the color square to open a color picker.\n"
                                              "Click and hold to use drag and drop.\n"
                                              "Right-click on the color square to show options.\n"
                                              "CTRL+click on individual component to input value.\n");
                
                ImGui::ColorEdit3("Color 2", Color2.e);
                
                if(ImGui::TreeNode("Tree1"))
                {
                    if(ImGui::TreeNode("Tree1"))
                    {
                        
                        
                        ImGui::TreePop();
                    }
                    
                    if(ImGui::TreeNode("Tree2"))
                    {
                        
                        
                        ImGui::TreePop();
                    }
                    
                    ImGui::TreePop();
                }
                
                if(ImGui::TreeNode("Tree2"))
                {
                    
                    
                    ImGui::TreePop();
                }
            }
            
            ImGui::EndChild();
#endif
        }
        ImGui::End();
        
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
        
    }
}

INTERNAL_FUNCTION void UpdateGame(game* Game)
{
    // NOTE(Dima): Processing Input
    Platform.ProcessInput();
    
    ShowOverlays(Game);
    
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
    ImGui::Render();
    
    BeginRender();
    Platform.Render();
    EndRender();
    
    // NOTE(Dima): Swapping buffers
    Platform.SwapBuffers();
}