struct testgame_state
{
    game_camera Camera;
    
    v3 PlayerP;
    v3 BearP;
};

SCENE_INIT(TestGame)
{
    testgame_state* State = GetSceneState(testgame_state);
    
    State->Camera.P = V3(0.0f, 0.0f, 5.0f);
    quat InitRot = LookRotation(V3_Back(), V3_Up());
    State->Camera.EulerAngles = QuatToEuler(InitRot);
    
    InitCamera(&State->Camera, Camera_FlyAround);
}

SCENE_UPDATE(TestGame)
{
    testgame_state* State = GetSceneState(testgame_state);
    
    // NOTE(Dima): Speed multiplyer
    float SpeedMultiplier = 5.0f;
    if(GetKey(Key_LeftShift))
    {
        SpeedMultiplier *= 5.0f;
    }
    if(GetKey(Key_Space))
    {
        SpeedMultiplier *= 5.0f;
    }
    
    UpdateCamera(&State->Camera, SpeedMultiplier);
    
#if 1    
    RenderModel(Global_RenderCommands,
                Global_Assets->Bear,
                V3(0.0f, 0.0f, 0.0f),
                Global_Time->Time,
                Global_Assets->BearSuccess);
    
    RenderModel(Global_RenderCommands,
                Global_Assets->Fox,
                V3(-3.0f, 0.0f, 0.0f),
                Global_Time->Time,
                Global_Assets->FoxTalk);
    
    RenderModel(Global_RenderCommands,
                Global_Assets->Supra,
                V3(4.0f, 0.0f, 0.0f),
                Global_Time->Time);
    
    PushMesh(Global_RenderCommands,
             &Global_Assets->Plane,
             &Global_Assets->GroundMaterial,
             ScalingMatrix(10.0f),
             V3(1.0f));
#endif
    
#if 0   
    // NOTE(Dima): Pushing cubes
    v3 Color00 = V3(1.0f, 1.0f, 1.0f);
    v3 Color01 = V3(1.0f, 0.0f, 0.0f);
    v3 Color10 = V3(1.0f, 1.0f, 0.0f);
    v3 Color11 = V3(0.3f, 0.1f, 0.9f);
    
    float CubeSpacing = 4.0f;
    
    int SideLen = 10;
    for(int VerticalIndex = 0; VerticalIndex < SideLen; VerticalIndex++)
    {
        for(int HorizontalIndex = 0; HorizontalIndex < SideLen; HorizontalIndex++)
        {
            float HorzPercentage = (float)HorizontalIndex / (float)(SideLen - 1);
            float VertPercentage = (float)VerticalIndex / (float)(SideLen - 1);
            
            v3 ColorHorzBottom = Lerp(Color00, Color10, HorzPercentage);
            v3 ColorHorzTop = Lerp(Color01, Color11, HorzPercentage);
            v3 VertColor = Lerp(ColorHorzBottom, ColorHorzTop, VertPercentage);
            
            v3 P = V3((f32)HorizontalIndex * CubeSpacing, 
                      Sin(Global_Time->Time + (HorizontalIndex + VerticalIndex) * 12.0f), 
                      (f32)VerticalIndex * CubeSpacing);
            
            PushMesh(Global_RenderCommands, 
                     &Global_Assets->Cube, 
                     &Global_Assets->GroundMaterial, 
                     TranslationMatrix(P), 
                     VertColor);
        }
    }
#endif
    
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    
    Global_RenderCommands->View = GetViewMatrix(&State->Camera);
    
    Global_RenderCommands->Projection = PerspectiveProjection(WndDims->Width, 
                                                              WndDims->Height,
                                                              500.0f, 0.5f);
    
    Global_RenderCommands->ViewProjection = 
        Global_RenderCommands->View * 
        Global_RenderCommands->Projection;
}