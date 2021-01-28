struct atool_asset
{
    u32 Type;
    
};

struct atool_source
{
    std::string GUID;
    std::string FilePath;
    
    u32 Type;
    
    loading_params LoadingParams;
    
    union
    {
        model* Model;
        image* Image;
        animation* Animation;
        font* Font;
    }Data;
};

struct atool
{
    game_camera Camera;
    
    std::vector<atool_asset>* Assets;
};

INTERNAL_FUNCTION void LoadAsset(atool_source* Source, memory_arena* Arena)
{
    char* FilePath = (char*)Source->FilePath.c_str();
    loading_params& Params = Source->LoadingParams;
    
    switch(Source->Type)
    {
        case Asset_Image:
        {
            Source->Data.Image = malloc(sizeof(image));
            *Source->Data.Image = LoadImageFile(FilePath, Params);
        }break;
        
        case Asset_Model:
        {
            Source->Data.Model = LoadModel(FilePath, Params);
        }break;
        
        case Asset_Animation:
        {
            Source->Data.Animation = LoadSkeletalAnimation();
        }break;
    }
}

INTERNAL_FUNCTION void FreeAsset(atool_source* Source)
{
    switch(Source->Type)
    {
        
    }
}

SCENE_INIT(Atool)
{
    atool* Inspector = GetSceneState(atool);
    
    InitCamera(&Inspector->Camera, Camera_RotateAround);
}

SCENE_UPDATE(Atool)
{
    atool* Inspector = GetSceneState(atool);
    
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
    
    SetMatrices(GetViewMatrix(&Inspector->Camera));
}