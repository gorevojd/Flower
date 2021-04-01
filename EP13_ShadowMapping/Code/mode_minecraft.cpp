#include "prj_minecraft.cpp"

struct minecraft_state
{
    minecraft Minecraft;
    
    game_camera Camera;
};

SCENE_INIT(Minecraft)
{
    minecraft_state* State = GetSceneState(minecraft_state);
    
    InitCamera(&State->Camera, Camera_FlyAround, 0.5f, 1500.0f);
    
    CreateMinecraft(Scene->Game->Arena, &State->Minecraft);
}

SCENE_UPDATE(Minecraft)
{
    FUNCTION_TIMING();
    minecraft_state* State = GetSceneState(minecraft_state);
    
    PushClear(V3(0.8f, 0.85f, 0.95f));
    
    // NOTE(Dima): Speed multiplyer
    float SpeedMultiplier = 5.0f;
    if(GetKey(Key_LeftShift))
    {
        SpeedMultiplier *= 5.0f;
    }
    if(GetKey(Key_Space))
    {
        SpeedMultiplier *= 20.0f;
    }
    
    render_pass* RenderPass = AddRenderPass();
    
    PushSky(&Global_Assets->Sky);
    //PushImage(&Global_Assets->VoxelAtlas, V2(0.0f, 0.0f), 400);
    //PushFullscreenRect(ColorRed());
    
    directional_light* DirLit = &Global_RenderCommands->Lighting.DirLit;
    
#if 0    
    DirLit->Dir = Lerp(NOZ(V3(-0.5f, -0.5f, -10.8f)), 
                       NOZ(V3(-0.5f, -0.5f, -0.8f)), 
                       Cos(Global_Time->Time) * 0.5f + 0.5f);
#else
    DirLit->Dir = NOZ(V3(-0.5f, -0.5f, -0.8f)); 
#endif
    
    // NOTE(Dima): Updating camera
    UpdateCamera(&State->Camera, SpeedMultiplier);
    UpdateMinecraft(&State->Minecraft, State->Camera.P);
    UpdateShadowCascades(RenderPass);
    
    SetMatrices(&State->Camera, RenderPass);
    
    
    render_water_params WaterParams = DefaultWaterParams();
    WaterParams.Height = 35.5f;
    PushWater(WaterParams, RenderPass);
}