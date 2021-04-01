#include "prj_rubiks.cpp"

struct rubiks_state
{
    game_camera Camera;
    
    int NumCubes;
    rubiks_cube* Cubes;
    v3* CubesP;
};

SCENE_INIT(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    InitCamera(&State->Camera, Camera_RotateAround);
    State->Camera.ShowcaseRotateTime = 20.0f;
    
    f32 CubeWidth = 5.0f;
    int CubeDim = 3;
    int CountOfCubes = 1;
    
    State->Cubes = PushArray(Scene->Arena, rubiks_cube, CountOfCubes);
    State->CubesP = PushArray(Scene->Arena, v3, CountOfCubes);
    State->NumCubes = CountOfCubes;
    
#if 1    
    for(int i = 0;
        i < CountOfCubes;
        i++)
    {
        State->Cubes[i] = CreateCube(Scene->Arena, CubeDim, CubeWidth);
        State->CubesP[i] = V3(i, 0.0f, 0.0f) * CubeWidth * 1.1f;
    }
#else
    State->Cubes[0] = CreateCube(Scene->Arena, 3, 3.0f);
    State->Cubes[1] = CreateCube(Scene->Arena, 7, 3.0f);
    
    State->CubesP[0] = V3(-3, 0.0f, 0.0f);
    State->CubesP[1] = V3(3, 0.0f, 0.0f);
#endif
    
}

void ApplyTempFormulas(rubiks_cube* Cube)
{
    RubCom_R(Cube);
    RubCom_U(Cube);
    RubCom_R(Cube, true);
    RubCom_U(Cube, true);
    
    RubCom_L(Cube, true);
    RubCom_F(Cube, true);
    RubCom_L(Cube);
    RubCom_B(Cube);
    
    RubCom_D(Cube);
    RubCom_D(Cube);
}

void ModeUpdateCube(rubiks_state* State, int CubeIndex)
{
    rubiks_cube* Cube = &State->Cubes[CubeIndex];
    v3 P = State->CubesP[CubeIndex];
    
    b32 ShiftIsPressed = GetKey(Key_LeftShift);
    b32 CtrlIsPressed = GetKey(Key_LeftControl);
    
    if(GetKeyDown(Key_G))
    {
        GenerateScrubmle(Cube, Global_Time->Time * 100000.0f + CubeIndex);
        Cube->SolvingState = RubState_Disassembled;
    }
    
    if(GetKeyDown(Key_Q))
    {
        ResetCubies(Cube);
    }
    
    if(CtrlIsPressed)
    {
        if(GetKeyDown(Key_X))
        {
            RubCom_X(Cube, ShiftIsPressed);
        }
        
        if(GetKeyDown(Key_Y))
        {
            RubCom_Y(Cube, ShiftIsPressed);
        }
        
        if(GetKeyDown(Key_Z))
        {
            RubCom_Z(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_A))
    {
        ApplyTempFormulas(Cube);
    }
    
    if(GetKeyDown(Key_R))
    {
        if(CtrlIsPressed)
        {
            RubCom_RR(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_R(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_L))
    {
        if(CtrlIsPressed)
        {
            RubCom_LL(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_L(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_D))
    {
        if(CtrlIsPressed)
        {
            RubCom_DD(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_D(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_U))
    {
        if(CtrlIsPressed)
        {
            RubCom_UU(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_U(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_F))
    {
        if(CtrlIsPressed)
        {
            RubCom_FF(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_F(Cube, ShiftIsPressed);
        }
    }
    
    if(GetKeyDown(Key_B))
    {
        if(CtrlIsPressed)
        {
            RubCom_BB(Cube, ShiftIsPressed);
        }
        else
        {
            RubCom_B(Cube, ShiftIsPressed);
        }
    }
    
    // NOTE(Dima): CenterTests
    if(GetKeyDown(Key_M) && Cube->Dim >= 3)
    {
        RubCom_M(Cube, ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_E) && Cube->Dim >= 3)
    {
        RubCom_E(Cube, ShiftIsPressed);
    }
    
    if(GetKeyDown(Key_S) && Cube->Dim >= 3)
    {
        RubCom_S(Cube, ShiftIsPressed);
    }
    
    // NOTE(Dima): StartSolve
    if(GetKeyDown(Key_Return))
    {
        if(!CubeIsSolved(Cube))
        {
            Cube->SolvingState = RubState_SolvingCenters;
        }
    }
    
    static b32 DebugMode = false;
    if(GetKeyDown(Key_Space))
    {
        DebugMode = !DebugMode;
    }
    b32 Smooth = true;
    UpdateCube(Cube, P, 1.0f, DebugMode, Smooth);
}

SCENE_UPDATE(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    PushClear(V3(0.7f, 0.75f, 0.95f));
    
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
    
    
    // NOTE(Dima): Rendering
    render_pass* RenderPass = AddRenderPass();
    
    for(int i = 0;
        i < State->NumCubes;
        i++)
    {
        ModeUpdateCube(State, i);
    }
    
    
    
#if 0    
    ShowSides(&State->Cubes[0], V2(10), 240);
    
    ShowLabel3D(&State->Camera, 
                "Hello world this is the cube",
                V3_Up() * 1.5f,
                0.25f,
                ColorRed());
    PushImage(&Global_Assets->FontsAtlas, V2(0.0f), 1300);
#endif
    
    SetMatrices(&State->Camera, RenderPass);
}