#ifndef FLOWER_GAMEPLAY_H
#define FLOWER_GAMEPLAY_H

enum camera_state
{
    Camera_FlyAround,
    Camera_RotateAround,
    Camera_ShowcaseRotateZ,
};

struct game_camera
{
    v3 P;
    v3 EulerAngles;
    
    m33 Transform;
    
    f32 NearClipPlane;
    f32 FarClipPlane;
    
    // NOTE(Dima): Staff for rotating around
    f32 ViewRadius;
    f32 ViewTargetRadius;
    f32 ViewRadiusMin;
    f32 ViewRadiusMax;
    v3 ViewCenterP;
    f32 ShowcaseRotateTime;
    
    u32 State;
};

enum game_object_type
{
    GameObject_Root,
    GameObject_Sentinel,
    
    GameObject_Object,
};

#include "flower_component.h"
struct game_object
{
    b32 IsActive;
    u32 Type;
    
    v3 P;
    quat R;
    v3 S;
    
    game_object* NextAlloc;
    game_object* PrevAlloc;
    int CreationIndex;
    
    game_object* Parent;
    game_object* ChildSentinel;
    game_object* Next;
    game_object* Prev;
    
    char Name[128];
    
    // NOTE(Dima): Model related stuff
    u32 ComponentsMask;
    component Components[8];
};

struct game_object_pool
{
    game_object UseSentinel;
    game_object FreeSentinel;
    
    int CreatedObjectsCount;
    
    game_object* Root;
};

struct game
{
    scene* Scenes;
    int NumScenes;
    
    int CurrentSceneIndex;
    int NextSceneIndex;
    
    memory_arena* Arena;
    
    // NOTE(Dima): Subsystems
    input_system* Input;
    time_system* Time;
    asset_system* Assets;
    ui_state* UI;
    render_commands* RenderCommands;
    job_system* JobSystem;
    platform_api* PlatformAPI;
    
#if defined(INTERNAL_BUILD)
    debug_state* Debug;
    debug_global_table* DebugTable;
#endif
    
    game_object_pool GameObjectPool;
    
    window_dimensions WindowDimensions;
    
    b32 ShouldReloadGameCode;
    b32 GameCodeWasJustReloaded;
};

#endif //FLOWER_GAMEPLAY_H
