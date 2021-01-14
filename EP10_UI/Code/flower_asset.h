#ifndef FLOWER_ASSET_H
#define FLOWER_ASSET_H

#include <unordered_map>

enum asset_type
{
    Asset_Font,
    Asset_Image,
    Asset_Mesh,
    Asset_Animation,
    Asset_NodeAnimation,
    Asset_Model,
    Asset_Material,
};

struct asset
{
    char Name[64];
    
    u32 Type;
    
    void* Ptr;
};

typedef u32 asset_id;


struct asset_system
{
    memory_arena* Arena;
    
    std::unordered_map<std::string, asset_id>* NameToAssetID;
    std::vector<asset> Assets;
    
    image FontsAtlas;
    v2 FontAtlasAtP;
    int FontAtlasMaxRowY;
    
    // NOTE(Dima): Assets
    mesh Cube;
    mesh Plane;
    
    image Mouse;
    image Cheese;
    
    image BoxTexture;
    image PlaneTexture;
    image Palette; 
    
    font TimesNewRoman;
    font LifeIsGoofy;
    font Arial;
    
    image BearDiffuse;
    image BearNormal;
    image BearEyesDiffuse;
    image BearEyesShine;
    
    image FoxDiffuse;
    image FoxNormal;
    image FoxEyesDiffuse;
    image FoxEyesShine;
    
    model* Bear;
    model* Fox;
    model* Supra;
    
    animation* BearSuccess;
    animation* BearIdle;
    animation* FoxTalk;
    
    material BearMaterial;
    material BearEyesMaterial;
    material BearEyesShineMaterial;
    
    material FoxMaterial;
    material FoxEyesMaterial;
    material FoxEyesShineMaterial;
    
    material PaletteMaterial;
    material GroundMaterial;
};

#endif //FLOWER_ASSET_H
