#include "flower_rubiks.cpp"

struct rubiks_state
{
    game_camera Camera;
    
    rubiks_cube Cube3;
};

INTERNAL_FUNCTION void UpdateCube(rubiks_cube* Cube, v3 P, f32 Speed = 1.0f, b32 DebugMode = false)
{
    b32 ShiftIsPressed = GetKey(Key_LeftShift);
    
    if(GetKeyDown(Key_Q))
    {
        ResetCubies(Cube);
    }
    
    if(GetKeyDown(Key_R))
    {
        BeginRotateFace(Cube, 
                        RotationMatrixX,
                        GetFaceX, 0,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_L))
    {
        BeginRotateFace(Cube,
                        RotationMatrixX,
                        GetFaceX, Cube->Dim - 1,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_D))
    {
        BeginRotateFace(Cube,
                        RotationMatrixY,
                        GetFaceY, 0,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_U))
    {
        BeginRotateFace(Cube,
                        RotationMatrixY,
                        GetFaceY, Cube->Dim - 1,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_F))
    {
        BeginRotateFace(Cube,
                        RotationMatrixZ,
                        GetFaceZ, 0,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_B))
    {
        BeginRotateFace(Cube,
                        RotationMatrixZ,
                        GetFaceZ, Cube->Dim - 1,
                        Speed,
                        !ShiftIsPressed);
    }
    
    // NOTE(Dima): CenterTests
    if(GetKeyDown(Key_I))
    {
        BeginRotateFace(Cube, 
                        RotationMatrixX,
                        GetFaceX, Cube->Dim / 2,
                        Speed,
                        !ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_O))
    {
        BeginRotateFace(Cube, 
                        RotationMatrixY,
                        GetFaceY, Cube->Dim / 2,
                        Speed,
                        !ShiftIsPressed);
    }
    
    
    if(GetKeyDown(Key_P))
    {
        BeginRotateFace(Cube, 
                        RotationMatrixZ,
                        GetFaceZ, Cube->Dim / 2,
                        Speed,
                        !ShiftIsPressed);
    }
    
    
    UpdateBeginnedRotation(Cube);
    
    ShowCube(Cube, P, DebugMode);
}

SCENE_INIT(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    InitCamera(&State->Camera, Camera_RotateAround);
    
    State->Cube3 = CreateCube(Scene->Arena, 2, 2.0f, V3(0.0f));
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
    if(GetKeyDown(Key_C))
    {
        CameraBehaviourIndex = !CameraBehaviourIndex;
    }
    State->Camera.State = CameraStates[CameraBehaviourIndex];
    
    UpdateCamera(&State->Camera);
    
    UpdateCube(&State->Cube3, V3(0.0f));
    //UpdateCube(&State->Cube3, V3(0.0f, 0.0f, 0.0f), 1.0f, true);
    
    SetMatrices(GetViewMatrix(&State->Camera));
}