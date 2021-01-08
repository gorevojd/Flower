struct rubiks_state
{
    game_camera Camera;
    
    int CubeSize;
    
    f32 CubeSideLen;
};

SCENE_INIT(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    InitCamera(&State->Camera, Camera_RotateAround);
}

SCENE_UPDATE(RubiksCube)
{
    rubiks_state* State = GetSceneState(rubiks_state);
    
    UpdateCamera(&State->Camera);
    
    PushMesh(Global_RenderCommands,
             &Global_Assets->Cube,
             &Global_Assets->GroundMaterial,
             IdentityMatrix4(),
             V3(1.0f));
    
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    Global_RenderCommands->View = GetViewMatrix(&State->Camera);
    Global_RenderCommands->Projection = PerspectiveProjection(WndDims->Width, 
                                                              WndDims->Height,
                                                              500.0f, 0.5f);
    Global_RenderCommands->ViewProjection = 
        Global_RenderCommands->View * 
        Global_RenderCommands->Projection;
}