enum test_game_mode
{
    TestGame_Animals,
    TestGame_CubeField,
    
    TestGame_Count,
};

struct testgame_state
{
    game_camera Camera;
    
    u32 Mode;
    
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
    
    // NOTE(Dima): Updating camera
    UpdateCamera(&State->Camera, SpeedMultiplier);
    
    // NOTE(Dima): Updating game mode
    
    if(GetKeyDown(Key_M))
    {
        State->Mode = (State->Mode + 1) % TestGame_Count;
    }
    
    switch(State->Mode)
    {
        case TestGame_Animals:
        {
            RenderModel(&Global_Assets->Bear,
                        V3(1.0f, 0.0f, 0.0f),
                        Global_Time->Time,
                        &Global_Assets->BearSuccess);
            
            RenderModel(&Global_Assets->Bear,
                        V3(-1.0f, 0.0f, 0.0f),
                        Global_Time->Time,
                        &Global_Assets->BearIdle);
            
            RenderModel(&Global_Assets->Fox,
                        V3(-3.0f, 0.0f, 0.0f),
                        Global_Time->Time,
                        &Global_Assets->FoxTalk);
            
#if 1    
            RenderModel(&Global_Assets->Supra,
                        V3(4.0f, 0.0f, 0.0f),
                        Global_Time->Time);
#endif
            
            PushMesh(&Global_Assets->Plane,
                     &Global_Assets->GroundMaterial,
                     ScalingMatrix(10.0f),
                     V3(1.0f));
        }break;
        
        case TestGame_CubeField:
        {
            PushMesh(&Global_Assets->Plane,
                     0,
                     ScalingMatrix(20.0f),
                     V3(0.7f));
            
            float CubeSpacing = 1.5f;
            
            v4 Color00 = ColorFromHex("#29FFC9");
            v4 Color01 = ColorFromHex("#7E42F5");
            v4 Color10 = ColorFromHex("#FF5114");
            v4 Color11 = ColorRed();
            
            int SideLen = 10;
            f32 DimCubes = CubeSpacing * (f32)SideLen;
            
            v3 CubeAddOffset = V3(-DimCubes, 0.0f, -DimCubes) * 0.5f;
            
            for(int y = 0; y < SideLen; y++)
            {
                for(int x = 0; x < SideLen; x++)
                {
                    float HorzPercentage = (float)x / (float)(SideLen - 1);
                    float VertPercentage = (float)y / (float)(SideLen - 1);
                    
                    v3 ColorHorzBottom = Lerp(Color00, Color10, HorzPercentage).rgb;
                    v3 ColorHorzTop = Lerp(Color01, Color11, HorzPercentage).rgb;
                    v3 VertColor = Lerp(ColorHorzBottom, ColorHorzTop, VertPercentage);
                    
                    v3 P = V3((f32)x * CubeSpacing, 
                              0.5f, 
                              (f32)y * CubeSpacing) + CubeAddOffset;
                    
                    PushMesh(&Global_Assets->Cube, 
                             0,
                             TranslationMatrix(P), 
                             VertColor);
                }
            }
        }break;
    }
    
    SetMatrices(GetViewMatrix(&State->Camera));
}


SCENE_ONGUI(TestGame)
{
    
}