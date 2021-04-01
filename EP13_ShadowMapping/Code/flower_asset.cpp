#include "flower_asset_load.cpp"

INTERNAL_FUNCTION void AddGlyphToAtlas(font* Font, int GlyphIndex)
{
    image* Dst = &Global_Assets->FontsAtlas;
    
    glyph* Glyph = &Font->Glyphs[GlyphIndex];
    
    int DstSize = Global_Assets->FontsAtlas.Width;
    f32 OneOverSize = 1.0f / DstSize;
    
    for(int StyleIndex = 0; 
        StyleIndex < FontStyle_Count;
        StyleIndex++)
    {
        glyph_style* Style = &Glyph->Styles[StyleIndex];
        
        // NOTE(Dima): Getting image 
        int ImageIndex = Style->ImageIndex;
        if(ImageIndex != -1)
        {
            image* Src = &Font->GlyphImages[ImageIndex];
            int SrcW = Src->Width;
            int SrcH = Src->Height;
            
            int DstPx = std::ceil(Global_Assets->FontAtlasAtP.x);
            int DstPy = std::ceil(Global_Assets->FontAtlasAtP.y);
            
            Style->MinUV = V2(DstPx, DstPy) * OneOverSize;
            Style->MaxUV = V2(DstPx + SrcW, DstPy + SrcH) * OneOverSize;
            
            if(DstPx + SrcW >= DstSize)
            {
                DstPx = 0;
                DstPy = Global_Assets->FontAtlasMaxRowY;
            }
            
            Assert(DstPy + SrcH < DstSize);
            
            // NOTE(Dima): Copy pixels
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
            Global_Assets->FontAtlasMaxRowY = std::max(Global_Assets->FontAtlasMaxRowY, DstPy + SrcH);
        }
    }
}

INTERNAL_FUNCTION void AddFontToAtlas(font* Font)
{
    for(int GlyphIndex = 0;
        GlyphIndex < Font->GlyphCount;
        GlyphIndex++)
    {
        glyph* Glyph = &Font->Glyphs[GlyphIndex];
        
        AddGlyphToAtlas(Font, GlyphIndex);
    }
}

INTERNAL_FUNCTION asset_id AddAsset(const char* GUID, u32 Type, void* Ptr)
{
    asset NewAsset = {};
    
    CopyStringsSafe(NewAsset.GUID, ArrayCount(NewAsset.GUID), (char*)GUID);
    NewAsset.Type = Type;
    NewAsset.Ptr = Ptr;
    
    // NOTE(Dima): Inserting to assets array
    asset_id NewAssetID = Global_Assets->Assets.size();
    Global_Assets->Assets.push_back(NewAsset);
    
    // NOTE(Dima): Insering to table
    auto NewPair = std::pair<std::string, asset_id>(std::string(GUID), 
                                                    NewAssetID);
    
    Global_Assets->NameToAssetID->insert(NewPair);
    
    return(NewAssetID);
}

inline asset_id GetByName(char* AssetName)
{
    asset_id Result = 0;
    auto FindIt = Global_Assets->NameToAssetID->find(std::string(AssetName));
    if(FindIt != Global_Assets->NameToAssetID->end())
    {
        Result = FindIt->second;
    }
    
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
#define GetAsset(guid, data_type, asset_type) GetAssetDataByID(GetByName(guid), asset_type) 

#if 0
struct asset_source
{
    std::string GUID;
    std::string FilePath;
    
    u32 Type;
    u32 TypeSize;
    
    b32 FromFile;
    
    loading_params LoadingParams;
    
    void* Data;
};

struct loaded_assets
{
    std::vector<asset_source> Sources;
    
    memory_arena Arena;
    
    loaded_assets()
    {
        Arena = {};
    }
    
    ~loaded_assets()
    {
        FreeArena(&Arena);
    }
    
    int AddSource(const char* GUID,
                  const char* FilePath,
                  u32 AssetType)
    {
        int Result = Sources.size();
        
        asset_source New = {};
        
        New.GUID = std::string(GUID);
        New.FilePath = std::string(FilePath);
        New.FromFile = true;
        New.Type = AssetType;
        New.LoadingParams = DefaultLoadingParams();
        New.TypeSize = Global_AssetTypeSize[AssetType];
        
        Sources.push_back(New);
        
        return(Result);
    }
    
    int AddSource(const char* GUID,
                  void* Source,
                  u32 AssetType)
    {
        int Result = Sources.size();
        
        asset_source New = {};
        
        New.GUID = std::string(GUID);
        New.FromFile = false;
        New.Type = AssetType;
        New.LoadingParams = DefaultLoadingParams();
        New.TypeSize = Global_AssetTypeSize[AssetType];
        
        Sources.push_back(New);
        
        return(Result);
    }
    
    void Load()
    {
        for(auto& Source : Sources)
        {
            Source.Data = malloc(Source.TypeSize);
            
            switch(Source.Type)
            {
                case Asset_Model:
                {
                    model* Model = (model*)Source.Data;
                    
                    *Model = LoadModel();
                }break;
                
                case Asset_Animation:
                {
                    
                }break;
                
                case Asset_Image:
                {
                    
                }break;
                
                case Asset_Mesh:
                {
                    
                }break;
            }
        }
    }
};
#endif

INTERNAL_FUNCTION void InitAssetSystem(memory_arena* Arena)
{
    Global_Assets = PushStruct(Arena, asset_system);
    Global_Assets->Arena = Arena;
    
    asset_system* A = Global_Assets;
    
    // NOTE(Dima): Font atlas initializing
    int FontAtlasSize = 2048;
    void* FontsAtlasMem = calloc(FontAtlasSize * FontAtlasSize * sizeof(u32), 1);
    Global_Assets->FontsAtlas = AllocateImageInternal(FontAtlasSize,
                                                      FontAtlasSize,
                                                      FontsAtlasMem,
                                                      ImageFormat_RGBA);
    
#if 0    
    Assert(ArrayCount(Global_AssetTypeSize) == Asset_Count);
    loaded_assets Loaded;
    
    // NOTE(Dima): Loading assets
    mesh Cube = MakeUnitCube();
    mesh Plane = MakePlane();
    
    Loaded.AddSource("Mesh_Cube", &Cube, Asset_Mesh);
    Loaded.AddSource("Mesh_Plane", &Plane, Asset_Mesh);
    
    // NOTE(Dima): Adding fonts
    Loaded.AddSource("Times_New_Roman",
                     "C:/Windows/Fonts/times.ttf",
                     Asset_Font);
    Loaded.AddSource("Life_Is_Goofy",
                     "../Data/Fonts/Life is goofy.ttf",
                     Asset_Font);
    Loaded.AddSource("Arial",
                     "c:/windows/fonts/arial.ttf",
                     Asset_Font);
    
    // NOTE(Dima): Adding images
    Loaded.AddSource("Image_Cheese", "E:/Media/Photos/Internet/Images/Cheese.png", Asset_Image);
    Loaded.AddSource("Image_Mouse", "E:/Media/Photos/Internet/Images/Mouse.png", Asset_Image);
    
    Loaded.AddSource("Texture_Box", "../Data/Textures/container_diffuse.png", Asset_Image);
    Loaded.AddSource("Texture_Plane", "E:/Media/PixarTextures/png/ground/Red_gravel_pxr128.png", Asset_Image);
    int PalleteSI = Loaded.AddSource("Texture_Palette", 
                                     "E:/Development/Modeling/Pallette/MyPallette.png", 
                                     Asset_Image);
    
    // NOTE(Dima): Adding Bear Textures
    Loaded.AddSource("Bear_Diffuse", "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear.tga",
                     Asset_Image);
    Loaded.AddSource("Bear_Normal", "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear Normals.tga",
                     Asset_Image);
    Loaded.AddSource("Bear_Eyes_Diffuse", "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Bear.tga",
                     Asset_Image);
    Loaded.AddSource("Bear_Eyes_Shine", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Shine Bear.tga", 
                     Asset_Image);
    
    // NOTE(Dima): Adding Fox Textures
    Loaded.AddSource("Fox_Diffuse", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox.tga",
                     Asset_Image);
    Loaded.AddSource("Fox_Normal", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox Normals.tga",
                     Asset_Image);
    Loaded.AddSource("Fox_Eyes_Diffuse", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Green.tga",
                     Asset_Image);
    Loaded.AddSource("Fox_Eyes_Shine", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Shine.tga", 
                     Asset_Image);
    
    // NOTE(Dima): Adding models
    Loaded.AddSource("Bear", 
                     "E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/bear.FBX",
                     Asset_Model);
    
    Loaded.AddSource("Fox",
                     "E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/Fox.FBX",
                     Asset_Model);
    
    Loaded.AddSource("Supra",
                     "E:/Development/Modeling/Modeling challenge/ToyotaSupra/Supra.FBX",
                     Asset_Model);
    
    // NOTE(Dima): Adding animations
    loaded_animations BearSuccess = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Success.FBX");
    Loaded.AddSource("Bear_Success",
                     &BearSuccess.Animations[0],
                     Asset_Animation);
    
    loaded_animations BearIdle = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Idle.FBX");
    Loaded.AddSource("Bear_Idle",
                     &BearIdle.Animations[0],
                     Asset_Animation);
    
    loaded_animations FoxTalk = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/animations/Talk.FBX");
    Loaded.AddSource("Fox_Talk",
                     &FoxTalk.Animations[0],
                     Asset_Animation);
#endif
    
    
#if 1
    loading_params VoxelAtlasParams = DefaultLoadingParams();
    VoxelAtlasParams.Image_FilteringIsClosest = true;
    A->VoxelAtlas = LoadImageFile("../Data/Textures/minc_atlas2.png", VoxelAtlasParams);
    //A->VoxelAtlas = LoadImageFile("../Data/Textures/minc_atlas1.jpg", VoxelAtlasParams);
    
#if 0
    // NOTE(Dima): Loading fonts
    A->TimesNewRoman = LoadFontFile("C:/Windows/Fonts/times.ttf");
    A->LifeIsGoofy = LoadFontFile("../Data/Fonts/Life is goofy.ttf");
    A->Arial = LoadFontFile("c:/windows/fonts/arial.ttf");
#endif
    
    loading_params BerlinSansParams = DefaultLoadingParams();
    BerlinSansParams.Font_PixelHeight = 60;
    A->BerlinSans = LoadFontFile("../Data/Fonts/BerlinSans.ttf", BerlinSansParams);
    
    loading_params LibMonoParams = DefaultLoadingParams();
    LibMonoParams.Font_PixelHeight = 24;
    A->LiberationMono = LoadFontFile("../Data/Fonts/liberation-mono.ttf", LibMonoParams);
    
#if 0    
    AddFontToAtlas(&A->TimesNewRoman);
    AddFontToAtlas(&A->LifeIsGoofy);
    AddFontToAtlas(&A->Arial);
#endif
    AddFontToAtlas(&A->BerlinSans);
    AddFontToAtlas(&A->LiberationMono);
    
    // NOTE(Dima): Loading assets
    A->Cube = MakeUnitCube();
    A->Plane = MakePlane();
    
#if 0    
    A->Sky = LoadCubemap(
                         "../Data/Textures/Cubemaps/skybox/right.jpg",
                         "../Data/Textures/Cubemaps/skybox/left.jpg",
                         "../Data/Textures/Cubemaps/skybox/front.jpg",
                         "../Data/Textures/Cubemaps/skybox/back.jpg",
                         "../Data/Textures/Cubemaps/skybox/top.jpg",
                         "../Data/Textures/Cubemaps/skybox/bottom.jpg");
#else
    A->Sky = LoadCubemap(
                         "../Data/Textures/Cubemaps/Pink/left.png",
                         "../Data/Textures/Cubemaps/Pink/right.png",
                         "../Data/Textures/Cubemaps/Pink/front.png",
                         "../Data/Textures/Cubemaps/Pink/back.png",
                         "../Data/Textures/Cubemaps/Pink/up.png",
                         "../Data/Textures/Cubemaps/Pink/down.png");
#endif
    
    A->BoxTexture = LoadImageFile("../Data/Textures/container_diffuse.png");
    A->PlaneTexture = LoadImageFile("E:/Media/PixarTextures/png/ground/Red_gravel_pxr128.png");
    loading_params PaletteParams = DefaultLoadingParams();
    PaletteParams.Image_FilteringIsClosest = true;
    A->Palette = LoadImageFile("E:/Development/Modeling/Pallette/MyPallette.png", PaletteParams);
    
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
    
    loaded_animations BearSuccess = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Success.FBX");
    loaded_animations BearIdle = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/animations/Idle.FBX");
    loaded_animations FoxTalk = LoadSkeletalAnimations("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/animations/Talk.FBX");
    
    A->BearSuccess = BearSuccess.Animations[0];
    A->BearIdle = BearIdle.Animations[0];
    A->FoxTalk = FoxTalk.Animations[0];
    
    // NOTE(Dima): Bear materials
    A->BearMaterial = {};
    A->BearMaterial.Diffuse = &A->BearDiffuse;
    
    A->BearEyesMaterial = {};
    A->BearEyesMaterial.Diffuse = &A->BearEyesDiffuse;
    
    A->BearEyesShineMaterial = {};
    A->BearEyesShineMaterial.Diffuse = &A->BearEyesShine;
    
    A->Bear.Materials[0] = &A->BearMaterial;
    A->Bear.Materials[1] = &A->BearEyesMaterial;
    A->Bear.Materials[2] = &A->BearEyesShineMaterial;
    A->Bear.Meshes[1]->MaterialIndexInModel = 1;
    A->Bear.Meshes[2]->MaterialIndexInModel = 2;
    A->Bear.Meshes[3]->MaterialIndexInModel = 1;
    A->Bear.Meshes[4]->MaterialIndexInModel = 2;
    
    // NOTE(Dima): Fox materials
    A->FoxMaterial = {};
    A->FoxMaterial.Diffuse = &A->FoxDiffuse;
    
    A->FoxEyesMaterial = {};
    A->FoxEyesMaterial.Diffuse = &A->FoxEyesDiffuse;
    
    A->FoxEyesShineMaterial = {};
    A->FoxEyesShineMaterial.Diffuse = &A->FoxEyesShine;
    
    A->Fox.Materials[0] = &A->FoxMaterial;
    A->Fox.Materials[1] = &A->FoxEyesMaterial;
    A->Fox.Materials[2] = &A->FoxEyesShineMaterial;
    A->Fox.Meshes[1]->MaterialIndexInModel = 1;
    A->Fox.Meshes[2]->MaterialIndexInModel = 2;
    A->Fox.Meshes[3]->MaterialIndexInModel = 1;
    A->Fox.Meshes[4]->MaterialIndexInModel = 2;
    
    // NOTE(Dima): Other materials
    A->PaletteMaterial = {};
    A->PaletteMaterial.Diffuse = &A->Palette;
    
    A->GroundMaterial = {};
    A->GroundMaterial.Diffuse = &A->PlaneTexture;
    
    // NOTE(Dima): Supra material
    A->Supra.Materials[0] = &A->PaletteMaterial;
#endif
    
}
