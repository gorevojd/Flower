struct asset_inspector
{
    game_camera Camera;
    
};

SCENE_INIT(AssetInspector)
{
    asset_inspector* Inspector = GetSceneState(asset_inspector);
    
    InitCamera(&Inspector->Camera, Camera_RotateAround);
}

SCENE_UPDATE(AssetInspector)
{
    asset_inspector* Inspector = GetSceneState(asset_inspector);
    
    Inspector->Camera.ViewCenterP = V3_Up();
    UpdateCamera(&Inspector->Camera);
    
    RenderModel(Global_RenderCommands,
                Global_Assets->Bear,
                V3(0.0f, 0.0f, 0.0f),
                Global_Time->Time,
                Global_Assets->BearSuccess);
    
    PushMesh(Global_RenderCommands,
             &Global_Assets->Plane,
             &Global_Assets->GroundMaterial,
             ScalingMatrix(2.0f),
             V3(1.0f));
    
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    Global_RenderCommands->View = GetViewMatrix(&Inspector->Camera);
    Global_RenderCommands->Projection = PerspectiveProjection(WndDims->Width, 
                                                              WndDims->Height,
                                                              500.0f, 0.5f);
    Global_RenderCommands->ViewProjection = 
        Global_RenderCommands->View * 
        Global_RenderCommands->Projection;
}