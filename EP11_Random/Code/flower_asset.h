#ifndef FLOWER_ASSET_H
#define FLOWER_ASSET_H

// TODO(Dima): Text segment in asset file to hold all names of various assets

#include <unordered_map>

enum asset_type
{
    Asset_None,
    
    Asset_Font,
    Asset_Image,
    Asset_Mesh,
    Asset_Animation,
    Asset_Material,
    Asset_Model,
    
    Asset_Count,
};

GLOBAL_VARIABLE mi Global_AssetTypeSize[Asset_Count] = 
{
    0,
    sizeof(font),
    sizeof(image),
    sizeof(mesh),
    sizeof(animation),
    sizeof(material),
    sizeof(model),
};

struct asset
{
    char GUID[64];
    
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
    
    image CoinTail;
    image CoinHead;
    
    image Mouse;
    image Cheese;
    
    image BoxTexture;
    image PlaneTexture;
    image Palette; 
    
    font TimesNewRoman;
    font LifeIsGoofy;
    font Arial;
    font BerlinSans;
    
    image BearDiffuse;
    image BearNormal;
    image BearEyesDiffuse;
    image BearEyesShine;
    
    image FoxDiffuse;
    image FoxNormal;
    image FoxEyesDiffuse;
    image FoxEyesShine;
    
    model Bear;
    model Fox;
    model Supra;
    
    animation BearSuccess;
    animation BearIdle;
    animation FoxTalk;
    
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
