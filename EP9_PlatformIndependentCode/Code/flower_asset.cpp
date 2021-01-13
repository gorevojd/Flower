#include "flower_asset_load.cpp"

INTERNAL_FUNCTION void AddGlyphToAtlas(glyph* Glyph)
{
    image* Src = &Glyph->Image;
    image* Dst = &Global_Assets->FontsAtlas;
    
    int SrcW = Src->Width;
    int SrcH = Src->Height;
    
    int DstSize = Global_Assets->FontsAtlas.Width;
    
    int DstPx = std::ceil(Global_Assets->FontAtlasAtP.x);
    int DstPy = std::ceil(Global_Assets->FontAtlasAtP.y);
    
    if(DstPx + SrcW >= DstSize)
    {
        DstPx = 0;
        DstPy = Global_Assets->FontAtlasMaxRowY;
    }
    
    Assert(DstPy + SrcH < DstSize);
    
    f32 OneOverSize = 1.0f / DstSize;
    
    Glyph->MinUV = V2(DstPx, DstPy) * OneOverSize;
    Glyph->MaxUV = V2(DstPx + SrcW, DstPy + SrcH) * OneOverSize;
    
    for(int y = 0; y < SrcH; y++)
    {
        for(int x = 0; x < SrcW; x++)
        {
            int DstPixelY = DstPy + y;
            int DstPixelX = DstPx + x;
            
            u32* DstPixel = (u32*)Dst->Pixels + DstPixelY * DstSize + DstPixelX;
            u32* SrcPixel = (u32*)Src->Pixels + y * SrcW + x;
            
            *DstPixel = *SrcPixel;
        }
    }
    
    Global_Assets->FontAtlasAtP = V2(DstPx + SrcW, DstPy);
    Global_Assets->FontAtlasMaxRowY = std::max(Global_Assets->FontAtlasMaxRowY,
                                               DstPy + SrcH);
}

INTERNAL_FUNCTION void AddFontToAtlas(font* Font)
{
    for(int GlyphIndex = 0;
        GlyphIndex < Font->GlyphCount;
        GlyphIndex++)
    {
        glyph* Glyph = &Font->Glyphs[GlyphIndex];
        
        AddGlyphToAtlas(Glyph);
    }
}

INTERNAL_FUNCTION asset_id AddAsset(const char* Name, u32 Type, void* Ptr)
{
    asset NewAsset = {};
    
    CopyStringsSafe(NewAsset.Name, ArrayCount(NewAsset.Name), (char*)Name);
    NewAsset.Type = Type;
    NewAsset.Ptr = Ptr;
    
    // NOTE(Dima): Inserting to assets array
    asset_id NewAssetID = Global_Assets->Assets.size();
    Global_Assets->Assets.push_back(NewAsset);
    
    // NOTE(Dima): Insering to table
    auto NewPair = std::pair<std::string, asset_id>(std::string(Name), 
                                                    NewAssetID);
    
    Global_Assets->NameToAssetID->insert(NewPair);
    
    return(NewAssetID);
}

inline asset_id GetByName(char* AssetName)
{
    auto FindIt = Global_Assets->NameToAssetID->find(std::string(AssetName));
    Assert(FindIt != Global_Assets->NameToAssetID->end());
    
    asset_id Result = FindIt->second;
    
    return(Result);
}

inline void* GetAssetDataByIDInternal(asset_id ID, u32 AssetType)
{
    asset* Asset = &Global_Assets->Assets[ID];
    
    Assert(Asset->Type == AssetType);
    
    void* Result = Asset->Ptr;
    return(Result);
}

#define GetAssetDataByID(id, data_type, asset_type) (data_type*)GetAssetByIDInternal(id, asset_type);

INTERNAL_FUNCTION void InitAssetSystem(memory_arena* Arena)
{
    Global_Assets = PushStruct(Arena, asset_system);
    Global_Assets->Arena = Arena;
    
    asset_system* A = Global_Assets;
    
    // NOTE(Dima): Reserving memory for assets array
    A->Assets.reserve(1024);
    A->NameToAssetID = PushNew<std::unordered_map<std::string, asset_id>>(Arena);
    
    // NOTE(Dima): Font atlas initializing
    int FontAtlasSize = 2048;
    void* FontsAtlasMem = calloc(FontAtlasSize * FontAtlasSize * sizeof(u32), 1);
    Global_Assets->FontsAtlas = AllocateImageInternal(FontAtlasSize,
                                                      FontAtlasSize,
                                                      FontsAtlasMem);
    
    // NOTE(Dima): Loading fonts
    A->TimesNewRoman = LoadFontFile("C:/Windows/Fonts/times.ttf", 30);
    A->LifeIsGoofy = LoadFontFile("../Data/Fonts/Life is goofy.ttf", 30);
    A->Arial = LoadFontFile("c:/windows/fonts/arial.ttf", 30);
    
    AddFontToAtlas(&A->TimesNewRoman);
    AddFontToAtlas(&A->LifeIsGoofy);
    AddFontToAtlas(&A->Arial);
    
    // NOTE(Dima): Loading assets
    A->Cube = MakeUnitCube();
    A->Plane = MakePlane();
    
    A->BoxTexture = LoadImageFile("../Data/Textures/container_diffuse.png");;
    A->PlaneTexture = LoadImageFile("E:/Media/PixarTextures/png/ground/Red_gravel_pxr128.png");
    A->Palette = LoadImageFile("E:/Development/Modeling/Pallette/MyPallette.png", true);
    
    A->BearDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear.tga");
    A->BearNormal = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear Normals.tga");
    A->BearEyesDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Bear.tga");
    A->BearEyesShine = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Shine Bear.tga");
    
    A->FoxDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox.tga");
    A->FoxNormal = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox Normals.tga");
    A->FoxEyesDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Green.tga");
    A->FoxEyesShine = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Shine.tga");
    
    loading_params BearParams = DefaultLoadingParams();
    BearParams.Model_DefaultScale = 0.01f;
    BearParams.Model_FixInvalidRotation = true;
    
    loading_params FoxParams = BearParams;
    
    A->Bear = LoadModel("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/bear.FBX", BearParams);
    A->Fox = LoadModel("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/Fox.FBX", FoxParams);
    A->Supra = LoadModel("E:/Development/Modeling/Modeling challenge/ToyotaSupra/Supra.FBX");
    
    A->BearSuccess = LoadSkeletalAnimation("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Success.FBX");
    A->BearIdle = LoadSkeletalAnimation("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Idle.FBX");
    A->FoxTalk = LoadSkeletalAnimation("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/animations/Talk.FBX");
    
    // NOTE(Dima): Bear materials
    A->BearMaterial = {};
    A->BearMaterial.Diffuse = &A->BearDiffuse;
    
    A->BearEyesMaterial = {};
    A->BearEyesMaterial.Diffuse = &A->BearEyesDiffuse;
    
    A->BearEyesShineMaterial = {};
    A->BearEyesShineMaterial.Diffuse = &A->BearEyesShine;
    
    A->Bear->Materials[0] = &A->BearMaterial;
    A->Bear->Materials[1] = &A->BearEyesMaterial;
    A->Bear->Materials[2] = &A->BearEyesShineMaterial;
    A->Bear->Meshes[1]->MaterialIndexInModel = 1;
    A->Bear->Meshes[2]->MaterialIndexInModel = 2;
    A->Bear->Meshes[3]->MaterialIndexInModel = 1;
    A->Bear->Meshes[4]->MaterialIndexInModel = 2;
    
    // NOTE(Dima): Fox materials
    A->FoxMaterial = {};
    A->FoxMaterial.Diffuse = &A->FoxDiffuse;
    
    A->FoxEyesMaterial = {};
    A->FoxEyesMaterial.Diffuse = &A->FoxEyesDiffuse;
    
    A->FoxEyesShineMaterial = {};
    A->FoxEyesShineMaterial.Diffuse = &A->FoxEyesShine;
    
    A->Fox->Materials[0] = &A->FoxMaterial;
    A->Fox->Materials[1] = &A->FoxEyesMaterial;
    A->Fox->Materials[2] = &A->FoxEyesShineMaterial;
    A->Fox->Meshes[1]->MaterialIndexInModel = 1;
    A->Fox->Meshes[2]->MaterialIndexInModel = 2;
    A->Fox->Meshes[3]->MaterialIndexInModel = 1;
    A->Fox->Meshes[4]->MaterialIndexInModel = 2;
    
    // NOTE(Dima): Other materials
    A->Palette = LoadImageFile("E:/Development/Modeling/Pallette/MyPallette.png", true);
    A->PaletteMaterial = {};
    A->PaletteMaterial.Diffuse = &A->Palette;
    
    A->GroundMaterial = {};
    A->GroundMaterial.Diffuse = &A->PlaneTexture;
    
    // NOTE(Dima): Supra material
    A->Supra->Materials[0] = &A->PaletteMaterial;
    
    // NOTE(Dima): Describing assets
    AddAsset("Times_New_Roman", Asset_Font, &A->TimesNewRoman);
    AddAsset("Life_Is_Goofy", Asset_Font, &A->LifeIsGoofy);
    AddAsset("Arial", Asset_Font, &A->Arial);
    
    AddAsset("Mesh_Cube", Asset_Mesh, &A->Cube);
    AddAsset("Mesh_Plane", Asset_Mesh, &A->Plane);
    
    AddAsset("Texture_Box", Asset_Image, &A->BoxTexture);
    AddAsset("Texture_Plane", Asset_Image, &A->PlaneTexture);
    AddAsset("Texture_Palette", Asset_Image, &A->Palette);
    
    AddAsset("Bear_Diffuse", Asset_Image, &A->BearDiffuse);
    AddAsset("Bear_Normal", Asset_Image, &A->BearNormal);
    AddAsset("Bear_Eyes_Diffuse", Asset_Image, &A->BearEyesDiffuse);
    AddAsset("Bear_Eyes_Shine", Asset_Image, &A->BearEyesShine);
    
    AddAsset("Fox_Diffuse", Asset_Image, &A->FoxDiffuse);
    AddAsset("Fox_Normal", Asset_Image, &A->FoxNormal);
    AddAsset("Fox_Eyes_Diffuse", Asset_Image, &A->FoxEyesDiffuse);
    AddAsset("Fox_Eyes_Shine", Asset_Image, &A->FoxEyesShine);
    
    AddAsset("Fox", Asset_Model, &A->Fox);
    AddAsset("Bear", Asset_Model, &A->Bear);
    AddAsset("Supra", Asset_Model, &A->Supra);
    
    AddAsset("Bear_Success", Asset_Animation, &A->BearSuccess);
    AddAsset("Bear_Idle", Asset_Animation, &A->BearIdle);
    AddAsset("Fox_Talk", Asset_Animation, &A->FoxTalk);
    
    AddAsset("Material_Bear", Asset_Material, &A->BearMaterial);
    AddAsset("Material_Fox", Asset_Material, &A->FoxMaterial);
    AddAsset("Material_Palette", Asset_Material, &A->PaletteMaterial);
    AddAsset("Material_Ground", Asset_Material, &A->GroundMaterial);
    
}
