#include "flower_rubiks.cpp"

struct rubiks_state
{
    game_camera Camera;
    
    rubiks_cube Cube3;
};

SCENE_INIT(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    InitCamera(&State->Camera, Camera_RotateAround);
    
    State->Cube3 = CreateCube(Scene->Arena, 50, 1.0f, false);
}

SCENE_UPDATE(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    u32 CameraStates[2] = 
    {
        Camera_RotateAround,
        Camera_ShowcaseRotateZ,
    };
    
    LOCAL_PERSIST b32 CameraBehaviourIndex = 0;
    if(GetKeyDown(Key_V))
    {
        CameraBehaviourIndex = !CameraBehaviourIndex;
    }
    State->Camera.State = CameraStates[CameraBehaviourIndex];
    
    UpdateCamera(&State->Camera);
    
    rubiks_cube* Cube = &State->Cube3;
    
    b32 ShiftIsPressed = GetKey(Key_LeftShift);
    
    if(GetKeyDown(Key_G))
    {
        GenerateScrubmle(Cube, Global_Time->Time * 1000.0f);
    }
    
    if(GetKeyDown(Key_Q))
    {
        ResetCubies(Cube);
    }
    
    if(GetKeyDown(Key_R))
    {
        AddCommandToCube(Cube, RubiksAxis_X, 0, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_L))
    {
        AddCommandToCube(Cube, RubiksAxis_X, Cube->Dim - 1, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_D))
    {
        AddCommandToCube(Cube, RubiksAxis_Y, 0, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_U))
    {
        AddCommandToCube(Cube, RubiksAxis_Y, Cube->Dim - 1, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_F))
    {
        AddCommandToCube(Cube, RubiksAxis_Z, 0, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_B))
    {
        AddCommandToCube(Cube, RubiksAxis_Z, Cube->Dim - 1, !ShiftIsPressed);
    }
    
    // NOTE(Dima): CenterTests
    if(GetKeyDown(Key_I))
    {
        AddCommandToCube(Cube, RubiksAxis_X, Cube->Dim / 2, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_O))
    {
        AddCommandToCube(Cube, RubiksAxis_Y, Cube->Dim / 2, !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_P))
    {
        AddCommandToCube(Cube, RubiksAxis_Z, Cube->Dim / 2, !ShiftIsPressed);
    }
    
    UpdateCube(&State->Cube3, V3(0.0f), 2.0f);
    
    //UpdateCube(&State->Cube3, V3(4.0f, 0.0f, 0.0f), 1.0f, true);
    
    SetMatrices(GetViewMatrix(&State->Camera));
}