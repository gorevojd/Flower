#include <vector>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL_FUNCTION void AddGlyphToAtlas(glyph* Glyph)
{
    image* Src = &Glyph->Image;
    image* Dst = &GlobalAssetSystem->FontsAtlas;
    
    int SrcW = Src->Width;
    int SrcH = Src->Height;
    
    int DstSize = GlobalAssetSystem->FontsAtlas.Width;
    
    int DstPx = std::ceil(GlobalAssetSystem->FontAtlasAtP.x);
    int DstPy = std::ceil(GlobalAssetSystem->FontAtlasAtP.y);
    
    if(DstPx + SrcW >= DstSize)
    {
        DstPx = 0;
        DstPy = GlobalAssetSystem->FontAtlasMaxRowY;
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
    
    GlobalAssetSystem->FontAtlasAtP = V2(DstPx + SrcW, DstPy);
    GlobalAssetSystem->FontAtlasMaxRowY = std::max(GlobalAssetSystem->FontAtlasMaxRowY,
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

INTERNAL_FUNCTION image AllocateImageInternal(u32 Width, u32 Height, void* PixData)
{
    image Result = {};
    
	Result.Width = Width;
	Result.Height = Height;
    
	Result.WidthOverHeight = (float)Width / (float)Height;
    
	Result.Pixels = PixData;
    
	return(Result);
}

INTERNAL_FUNCTION void InitAssetSystem(memory_arena* Arena)
{
    GlobalAssetSystem = PushStruct(Arena, asset_system);
    
    // NOTE(Dima): Font atlas initializing
    int FontAtlasSize = 2048;
    void* FontsAtlasMem = calloc(FontAtlasSize * FontAtlasSize * sizeof(u32), 1);
    GlobalAssetSystem->FontsAtlas = AllocateImageInternal(FontAtlasSize,
                                                          FontAtlasSize,
                                                          FontsAtlasMem);
    
    
}

INTERNAL_FUNCTION image LoadImageFile(char* FilePath, b32 FilteringIsClosest = false)
{
    int Width;
    int Height;
    int Channels;
    
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char* Image = stbi_load(FilePath,
                                     &Width,
                                     &Height,
                                     &Channels,
                                     STBI_rgb_alpha);
    
    Assert(Image);
    
    int PixCount = Width * Height;
    int ImageSize = PixCount * 4;
    
    void* OurImageMem = malloc(ImageSize);
    image Result = AllocateImageInternal(Width, Height, OurImageMem);
    Result.FilteringIsClosest = FilteringIsClosest;
    
    for(int PixelIndex = 0;
        PixelIndex < PixCount;
        PixelIndex++)
    {
        u32 Pix = *((u32*)Image + PixelIndex);
        
        v4 Color = UnpackRGBA(Pix);
        Color.r *= Color.a;
        Color.g *= Color.a;
        Color.b *= Color.a;
        
        u32 PackedColor = PackRGBA(Color);
        
        *((u32*)Result.Pixels + PixelIndex) = PackedColor;
    }
    
    stbi_image_free(Image);
    
    return(Result);
}

INTERNAL_FUNCTION font LoadFontFromBuffer(u8* Buffer)
{
    font Result = {};
    
    int FontSize = 30;
    Result.Size = 30;
    
    stbtt_fontinfo StbFont;
    stbtt_InitFont(&StbFont, Buffer, 0);
    
    f32 Scale = stbtt_ScaleForPixelHeight(&StbFont, FontSize);
    
    int StbAscent;
    int StbDescent;
    int StbLineGap;
    stbtt_GetFontVMetrics(&StbFont, &StbAscent, &StbDescent, &StbLineGap);
    
    Result.Ascent = (f32)StbAscent * Scale;
    Result.Descent = (f32)StbDescent * Scale;
    Result.LineGap = (f32)StbLineGap * Scale;
    
    Result.LineAdvance = Result.Ascent - Result.Descent + Result.LineGap;
    
    std::vector<glyph> Glyphs;
    
    for(int CharIndex = ' ';
        CharIndex <= '~';
        CharIndex++)
    {
        glyph Glyph = {};
        
        int StbAdvance;
        int StbLeftBearing;
        stbtt_GetCodepointHMetrics(&StbFont, 
                                   CharIndex, 
                                   &StbAdvance, 
                                   &StbLeftBearing);
        
        Glyph.Codepoint = CharIndex;
        Glyph.Advance = (f32)StbAdvance * Scale;
        Glyph.LeftBearing = (f32)StbLeftBearing * Scale;
        
        int StbW, StbH;
        int StbXOffset, StbYOffset;
        unsigned char* StbBitmap = stbtt_GetCodepointBitmap(&StbFont,
                                                            0, Scale,
                                                            CharIndex,
                                                            &StbW, &StbH,
                                                            &StbXOffset, 
                                                            &StbYOffset);
        
        if(StbW > 20000)
        {
            StbW = 0;
        }
        
        
        if(StbH > 20000)
        {
            StbH = 0;
        }
        
        int Border = 3;
        int GlyphWidth = StbW + 2 * Border;
        int GlyphHeight = StbH + 2 * Border;
        
        int ImageSize = GlyphWidth * GlyphHeight * 4;
        
        void* OurImageMem = calloc(ImageSize, 1);
        Glyph.Image = AllocateImageInternal(GlyphWidth, GlyphHeight, OurImageMem);
        Glyph.XOffset = StbXOffset - Border;
        Glyph.YOffset = StbYOffset - Border;
        
        for(int y = 0; y < StbH; y++)
        {
            for(int x = 0; x < StbW; x++)
            {
                int PixelIndex = y * StbW + x;
                unsigned char CurTone = StbBitmap[PixelIndex];
                
                f32 Tone01 = (f32)CurTone / 255.0f;
                
                v4 Color = V4(Tone01, Tone01, Tone01, Tone01);
                u32 PackedColor = PackRGBA(Color);
                
                int TargetPixel = (y + Border) * GlyphWidth + (x + Border);
                u32* OurImagePixels = (u32*)OurImageMem;
                OurImagePixels[TargetPixel] = PackedColor;
                
            }
        }
        
        stbtt_FreeBitmap(StbBitmap, 0);
        
        Glyphs.push_back(Glyph);
    }
    
    // NOTE(Dima): Copying glyphs
    Result.Glyphs = (glyph*)calloc(Glyphs.size(), sizeof(glyph));
    Result.GlyphCount = Glyphs.size();
    memcpy(Result.Glyphs, &Glyphs[0], Glyphs.size() * sizeof(glyph));
    
    // NOTE(Dima): Loading kerning
    Result.KerningPairs = (f32*)malloc(Result.GlyphCount * Result.GlyphCount * sizeof(f32));
    
    for(int i = 0; i < Result.GlyphCount; i++)
    {
        for(int j = 0; j < Result.GlyphCount; j++)
        {
            int Index = i * Result.GlyphCount + j;
            
            u32 A = Glyphs[i].Codepoint;
            u32 B = Glyphs[j].Codepoint;
            
            int ExtractedKern = stbtt_GetCodepointKernAdvance(&StbFont, A, B);
            
            if(ExtractedKern != 0)
            {
                int a = 1;
            }
            
            Result.KerningPairs[Index] = Scale * (f32)ExtractedKern;
            //Result.KerningPairs[Index] = 10.0f;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION font LoadFontFile(char* FilePath)
{
    u8* Buffer = 0;
    
    SDL_RWops* File = SDL_RWFromFile(FilePath, "rb");
    if(File)
    {
        i64 FileSize = SDL_RWsize(File);
        
        Buffer = (u8*)malloc(FileSize);
        
        SDL_RWread(File, Buffer, FileSize, 1);
        
        SDL_RWclose(File);
    }
    
    
    font Result = {};
    if(Buffer)
    {
        Result = LoadFontFromBuffer(Buffer);
    }
    
    free(Buffer);
    
    return(Result);
}

INTERNAL_FUNCTION mesh MakeMesh(const std::vector<v3>& Vertices,
                                const std::vector<v2>& TexCoords,
                                const std::vector<v3>& Normals,
                                const std::vector<v3>& Colors,
                                const std::vector<u32>& Indices,
                                const std::vector<v4>& BoneWeights = std::vector<v4>(),
                                const std::vector<u32>& BoneIndices = std::vector<u32>(),
                                b32 IsSkinned = false)
{
    mesh Result = {};
    
    mi DataSize = 0;
    
    // NOTE(Dima): Setting offsets
    Result.OffsetToP = DataSize;
    DataSize += sizeof(v3) * Vertices.size();
    
    Result.OffsetToUV = DataSize;
    DataSize += sizeof(v2) * TexCoords.size();
    
    Result.OffsetToN = DataSize;
    DataSize += sizeof(v3) * Normals.size();
    
    Result.OffsetToC = DataSize;
    DataSize += sizeof(v3) * Colors.size();
    
    if(IsSkinned)
    {
        Result.OffsetToBoneWeights = DataSize;
        DataSize += sizeof(v4) * BoneWeights.size();
        
        Result.OffsetToBoneIndices = DataSize;
        DataSize += sizeof(u32) * BoneIndices.size();
    }
    
    // NOTE(Dima): Allocating buffer for holding vertex data
    u8* VerticesBuffer = (u8*)malloc(DataSize);
    Result.P = (v3*)(VerticesBuffer + Result.OffsetToP);
    Result.UV = (v2*)(VerticesBuffer + Result.OffsetToUV);
    Result.N = (v3*)(VerticesBuffer + Result.OffsetToN);
    Result.C = (v3*)(VerticesBuffer + Result.OffsetToC);
    if(IsSkinned)
    {
        Result.BoneWeights = (v4*)(VerticesBuffer + Result.OffsetToBoneWeights);
        Result.BoneIndices = (u32*)(VerticesBuffer + Result.OffsetToBoneIndices);
    }
    
    // NOTE(Dima): Saving vertices data buffer
    Result.VertexCount = Vertices.size();
    Result.VerticesDataBuffer = VerticesBuffer;
    Result.VerticesDataBufferSize = DataSize;
    
    // NOTE(Dima): Allocating indices
    u32 IndexSize = sizeof(u32) * Indices.size();
    Result.Indices = (u32*)malloc(IndexSize);
    Result.IndexCount = Indices.size();
    
    // NOTE(Dima): Storing vertex data
    for(int VertexIndex = 0;
        VertexIndex < Vertices.size();
        VertexIndex++)
    {
        Result.P[VertexIndex] = Vertices[VertexIndex];
        Result.UV[VertexIndex] = TexCoords[VertexIndex];
        Result.N[VertexIndex] = Normals[VertexIndex];
        Result.C[VertexIndex] = Colors[VertexIndex];
        if(IsSkinned)
        {
            Result.BoneWeights[VertexIndex] = BoneWeights[VertexIndex];
            Result.BoneIndices[VertexIndex] = BoneIndices[VertexIndex];
        }
    }
    
    for (int Index = 0; Index < Indices.size(); Index += 1)
    {
        Result.Indices[Index] = Indices[Index];
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void PushUnitCubeSide(std::vector<v3>& Vertices,
                                               std::vector<v2>& TexCoords,
                                               std::vector<v3>& Normals,
                                               std::vector<v3>& Colors,
                                               std::vector<u32>& Indices,
                                               int Index0, 
                                               int Index1,
                                               int Index2,
                                               int Index3,
                                               v3 Normal)
{
    v3 CubeVertices[8] =
    {
        V3(-1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, -1.0f, 1.0f),
        V3(-1.0f, -1.0f, 1.0f),
        
        V3(-1.0f, 1.0f, -1.0f),
        V3(1.0f, 1.0f, -1.0f),
        V3(1.0f, -1.0f, -1.0f),
        V3(-1.0f, -1.0f, -1.0f),
    };
    
    v3 CubeColors[8] =
    {
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
    };
    
    int IndexBase = Indices.size();
    int VertexBase = Vertices.size();
    
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 1);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 3);
    
    Vertices.push_back(CubeVertices[Index0]);
    Vertices.push_back(CubeVertices[Index1]);
    Vertices.push_back(CubeVertices[Index2]);
    Vertices.push_back(CubeVertices[Index3]);
    
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    
    TexCoords.push_back(V2(0.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 0.0f));
    TexCoords.push_back(V2(0.0f, 0.0f));
    
    Colors.push_back(CubeColors[Index0]);
    Colors.push_back(CubeColors[Index1]);
    Colors.push_back(CubeColors[Index2]);
    Colors.push_back(CubeColors[Index3]);
}

INTERNAL_FUNCTION mesh MakeUnitCube()
{
    std::vector<v3> Vertices;
    std::vector<v2> TexCoords;
    std::vector<v3> Normals;
    std::vector<v3> Colors;
    std::vector<u32> Indices;
    
    // NOTE(Dima): Forward side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     0, 1, 2, 3, 
                     V3_Forward());
    
    // NOTE(Dima): Left side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     1, 5, 6, 2, 
                     V3_Left());
    
    // NOTE(Dima): Back side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     5, 4, 7, 6, 
                     V3_Back());
    
    // NOTE(Dima): Right side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     4, 0, 3, 7, 
                     V3_Right());
    
    // NOTE(Dima): Up
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     4, 5, 1, 0, 
                     V3_Up());
    
    // NOTE(Dima): Down
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     3, 2, 6, 7, 
                     V3_Down());
    
    mesh Result = MakeMesh(Vertices,
                           TexCoords,
                           Normals,
                           Colors,
                           Indices);
    
    return(Result);
}

mesh MakePlane()
{
    std::vector<v3> Vertices;
    std::vector<v2> TexCoords;
    std::vector<v3> Normals;
    std::vector<v3> Colors;
    std::vector<u32> Indices;
    
    Vertices.push_back(V3(1.0f, 0.0f, 1.0f));
    Vertices.push_back(V3(-1.0f, 0.0f, 1.0f));
    Vertices.push_back(V3(-1.0f, 0.0f, -1.0f));
    Vertices.push_back(V3(1.0f, 0.0f, -1.0f));
    
    TexCoords.push_back(V2(0.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 0.0f));
    TexCoords.push_back(V2(0.0f, 0.0f));
    
    v3 N = V3_Up();
    Normals.push_back(N);
    Normals.push_back(N);
    Normals.push_back(N);
    Normals.push_back(N);
    
    v3 Color = V3_One();
    Colors.push_back(Color);
    Colors.push_back(Color);
    Colors.push_back(Color);
    Colors.push_back(Color);
    
    Indices.push_back(0);
    Indices.push_back(1);
    Indices.push_back(2);
    Indices.push_back(0);
    Indices.push_back(2);
    Indices.push_back(3);
    
    mesh Result = MakeMesh(Vertices,
                           TexCoords,
                           Normals,
                           Colors,
                           Indices);
    
    return(Result);
}

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

inline m44 AssimpToOurMatrix(aiMatrix4x4& AMat)
{
    m44 Result = {};
    
    Result.e[0] = AMat.a1;
    Result.e[1] = AMat.a2;
    Result.e[2] = AMat.a3;
    Result.e[3] = AMat.a4;
    
    Result.e[4] = AMat.b1;
    Result.e[5] = AMat.b2;
    Result.e[6] = AMat.b3;
    Result.e[7] = AMat.b4;
    
    Result.e[8] = AMat.c1;
    Result.e[9] = AMat.c2;
    Result.e[10] = AMat.c3;
    Result.e[11] = AMat.c4;
    
    Result.e[12] = AMat.d1;
    Result.e[13] = AMat.d2;
    Result.e[14] = AMat.d3;
    Result.e[15] = AMat.d4;
    
    return(Transpose(Result));
}

struct model
{
    mesh* Meshes;
    int MeshCount;
};

struct loaded_model_node
{
    std::string Name;
    
    int NodeIndex;
    int ParentNodeIndex;
    std::vector<int> ChildNodesIndices;
    
    std::vector<int> MeshIndices;
    
    m44 ToParent;
    m44 ToModel;
    
    aiNode* AssimpNode;
};

struct loaded_model_mesh
{
    std::string Name;
    
    mesh Mesh;
    
    int MaterialIndex;
};

struct loaded_model_bone
{
    m44 InvBindPose;
    int NodeIndex;
};

struct loaded_model
{
    std::vector<loaded_model_mesh> Meshes;
    std::vector<loaded_model_node> Nodes;
    std::vector<material*> Materials;
    
    std::vector<loaded_model_bone> Bones;
    
    // NOTE(Dima): Help structures
    std::map<std::string, int> NodeNameToNodeIndex;
    std::map<std::string, int> NodeNameToBoneIndex;
};

mesh ConvertAssimpMesh(const aiScene* AssimpScene, aiMesh* AssimpMesh, loaded_model* Model)
{
    std::vector<v3> Vertices;
    std::vector<v2> TexCoords;
    std::vector<v3> Normals;
    std::vector<v3> Colors;
    std::vector<u32> Indices;
    
    Vertices.reserve(AssimpMesh->mNumVertices);
    TexCoords.reserve(AssimpMesh->mNumVertices);
    Normals.reserve(AssimpMesh->mNumVertices);
    Colors.reserve(AssimpMesh->mNumVertices);
    
    // NOTE(Dima): Choosing color set to load
    int NumColorChannels = AssimpMesh->GetNumColorChannels();
    b32 CanLoadColors = false;
    int ColorSetIndex = 0;
    for(int ColorIndex = 0;
        ColorIndex < NumColorChannels;
        ColorIndex++)
    {
        CanLoadColors = AssimpMesh->HasVertexColors(ColorIndex);
        
        if(CanLoadColors)
        {
            ColorSetIndex = ColorIndex;
            break;
        }
    }
    
    // NOTE(Dima): Choosing UV set to load
    int NumUVChannels = AssimpMesh->GetNumUVChannels();
    b32 CanLoadUVs = false;
    int UVSetIndex = 0;
    for(int UVIndex = 0;
        UVIndex < NumUVChannels;
        UVIndex++)
    {
        CanLoadUVs = AssimpMesh->HasTextureCoords(UVIndex);
        
        if(CanLoadUVs)
        {
            UVSetIndex = UVIndex;
            break;
        }
    }
    
    // NOTE(Dima): Loading vertices
    for(int VertexIndex = 0;
        VertexIndex < AssimpMesh->mNumVertices;
        VertexIndex++)
    {
        // NOTE(Dima): Vertex data
        aiVector3D AssimpVertex = AssimpMesh->mVertices[VertexIndex];
        
        v3 OurVertex = V3(AssimpVertex.x,
                          AssimpVertex.y,
                          AssimpVertex.z);
        Vertices.push_back(OurVertex);
        
        // NOTE(Dima): Normal data
        aiVector3D AssimpNormal = AssimpMesh->mNormals[VertexIndex];
        v3 OurNormal = V3(AssimpNormal.x,
                          AssimpNormal.y,
                          AssimpNormal.z);
        Normals.push_back(OurNormal);
        
        // NOTE(Dima): UVs data
        v2 OurUV = V2(0.0f, 0.0f);
        if(CanLoadUVs)
        {
            aiVector3D AssimpUV = AssimpMesh->mTextureCoords[UVSetIndex][VertexIndex];
            OurUV = V2(AssimpUV.x, AssimpUV.y);
        }
        TexCoords.push_back(OurUV);
        
        // NOTE(Dima): Colors data
        v3 OurColor = V3(1.0f, 0.0f, 1.0f);
        if(CanLoadColors)
        {
            aiColor4D AssimpColor = AssimpMesh->mColors[ColorSetIndex][VertexIndex];
            OurColor = V3(AssimpColor.r,
                          AssimpColor.g,
                          AssimpColor.b);
        }
        Colors.push_back(OurColor);
    }
    
    // NOTE(Dima): Loading indices
    for(int FaceIndex = 0;
        FaceIndex < AssimpMesh->mNumFaces;
        FaceIndex++)
    {
        aiFace* Face = &AssimpMesh->mFaces[FaceIndex];
        
        if(Face->mNumIndices == 3)
        {
            Indices.push_back(Face->mIndices[0]);
            Indices.push_back(Face->mIndices[1]);
            Indices.push_back(Face->mIndices[2]);
        }
    }
    
    // NOTE(Dima): Loading weights
    struct v_weight
    {
        f32 W;
        int ID;
    };
    
    struct v_weights
    {
        std::vector<v_weight> Weights;
    };
    
    std::vector<v_weights> Weights(AssimpMesh->mNumVertices);
    
    b32 MeshIsSkinned = AssimpMesh->HasBones() && (AssimpMesh->mNumBones > 0);
    if(MeshIsSkinned)
    {
        for(int MeshBoneIndex = 0;
            MeshBoneIndex < AssimpMesh->mNumBones;
            MeshBoneIndex++)
        {
            aiBone* AiBone = AssimpMesh->mBones[MeshBoneIndex];
            
            // NOTE(Dima): Finding bone ID
            std::string BoneName = std::string(AiBone->mName.C_Str());
            int BoneIndex = Model->NodeNameToBoneIndex[BoneName];
            
            // NOTE(Dima): Processing weights
            for(int WeightIndex = 0;
                WeightIndex < AiBone->mNumWeights;
                WeightIndex++)
            {
                aiVertexWeight* Weight = &AiBone->mWeights[WeightIndex];
                
                u32 VertexIndex = Weight->mVertexId;
                f32 VertexWeight = Weight->mWeight;
                
                v_weight NewWeight = {};
                NewWeight.W = VertexWeight;
                NewWeight.ID = BoneIndex;
                
                v_weights* TargetWeights = &Weights[VertexIndex];
                TargetWeights->Weights.push_back(NewWeight);
            }
        }
    }
    
    // NOTE(Dima): Preparing weights
    std::vector<v4> BoneWeights;
    std::vector<u32> BoneIndices;
    
    for(int WeightIndex = 0;
        WeightIndex < Weights.size();
        WeightIndex++)
    {
        v_weights* CurWeights = &Weights[WeightIndex];
        
        v4 ResultWeights = V4(0.0f, 0.0f, 0.0f, 0.0f);
        u32 ResultIndices = 0;
        
        for(int InnerIndex = 0;
            InnerIndex < CurWeights->Weights.size();
            InnerIndex++)
        {
            v_weight W = CurWeights->Weights[InnerIndex];
            
            ResultWeights.e[InnerIndex] = W.W;
            ResultIndices |= (W.ID & 255) << (InnerIndex * 8);
        }
        
        BoneWeights.push_back(ResultWeights);
        BoneIndices.push_back(ResultIndices);
    }
    
    // NOTE(Dima): Actual converting
    mesh Result = MakeMesh(Vertices, 
                           TexCoords,
                           Normals, 
                           Colors,
                           Indices,
                           BoneWeights,
                           BoneIndices,
                           MeshIsSkinned);
    
    return(Result);
}

INTERNAL_FUNCTION void AssimpProcessNode(aiScene* AssimpScene,
                                         aiNode* AssimpNode,
                                         loaded_model_node* Parent,
                                         const m44& GlobalInitTransform,
                                         loaded_model* Loaded)
{
    // NOTE(Dima): Calculating this node transforms
    m44 ThisNodeToParentTransform = GlobalInitTransform;
    m44 ThisNodeToModelTransform = GlobalInitTransform;
    if(Parent != 0)
    {
        ThisNodeToParentTransform = AssimpToOurMatrix(AssimpNode->mTransformation);
        ThisNodeToModelTransform = ThisNodeToParentTransform * Parent->ToModel;
    }
    
    int ThisNodeIndex = Loaded->Nodes.size();
    
    // NOTE(Dima): Initializing new Node
    loaded_model_node NewNode;
    NewNode.Name = std::string(AssimpNode->mName.C_Str());
    NewNode.NodeIndex = ThisNodeIndex;
    NewNode.ToParent = ThisNodeToParentTransform;
    NewNode.ToModel = ThisNodeToModelTransform; 
    NewNode.AssimpNode = AssimpNode; 
    
    // NOTE(Dima): Setting child and parent nodes indices
    if(Parent != 0)
    {
        Parent->ChildNodesIndices.push_back(ThisNodeIndex);
        NewNode.ParentNodeIndex = Parent->NodeIndex;
    }
    else
    {
        NewNode.ParentNodeIndex = -1;
    }
    
    // NOTE(Dima): Pushing node
    int NewNodeIndex = Loaded->Nodes.size();
    Loaded->NodeNameToNodeIndex.insert(std::pair<std::string, int>(NewNode.Name, NewNodeIndex));
    Loaded->Nodes.push_back(NewNode);
    
    // NOTE(Dima): Recursive lookups
    for(int NodeIndex = 0; NodeIndex < AssimpNode->mNumChildren; NodeIndex++)
    {
        aiNode* CurChild = AssimpNode->mChildren[NodeIndex];
        
        AssimpProcessNode(AssimpScene, CurChild, 
                          &Loaded->Nodes[ThisNodeIndex], 
                          GlobalInitTransform, 
                          Loaded);
    }
}

INTERNAL_FUNCTION void LoadModelBones(const aiScene* AssimpScene,
                                      loaded_model* Model,
                                      const m44& GlobalInitTransform)
{
    //std::map<std::string, loaded_model_bone> NameToBoneMap;
    
    for(int MeshIndex = 0; 
        MeshIndex < AssimpScene->mNumMeshes; 
        MeshIndex++)
    {
        aiMesh* AiMesh = AssimpScene->mMeshes[MeshIndex];
        
        if(AiMesh->HasBones() && (AiMesh->mNumBones > 0))
        {
            for(int BoneIndex = 0;
                BoneIndex < AiMesh->mNumBones;
                BoneIndex++)
            {
                aiBone* AiBone = AiMesh->mBones[BoneIndex];
                
                std::string BoneName = std::string(AiBone->mName.C_Str());
                
                loaded_model_bone NewBone = {};
                
                NewBone.InvBindPose = AssimpToOurMatrix(AiBone->mOffsetMatrix) * GlobalInitTransform;
                NewBone.NodeIndex = Model->NodeNameToNodeIndex[BoneName];
                
                int ThisBoneIndex = Model->Bones.size();
                Model->Bones.push_back(NewBone);
                
                Model->NodeNameToBoneIndex.insert(std::pair<std::string, int>(BoneName, ThisBoneIndex));
            }
        }
    }
}

INTERNAL_FUNCTION void LoadModelMeshes(const aiScene* AssimpScene,
                                       loaded_model* Model)
{
    for(int NodeIndex = 0; 
        NodeIndex < Model->Nodes.size();
        NodeIndex++)
    {
        loaded_model_node* OurNode = &Model->Nodes[NodeIndex];
        
        aiNode* AssimpNode = OurNode->AssimpNode;
        
        // NOTE(Dima): Loading this nodes meshes
        for(int MeshIndex = 0; MeshIndex < AssimpNode->mNumMeshes; MeshIndex++)
        {
            int MeshIndexInScene = AssimpNode->mMeshes[MeshIndex];
            aiMesh* AssimpMesh = AssimpScene->mMeshes[MeshIndexInScene];
            
            mesh OurMesh = ConvertAssimpMesh(AssimpScene, AssimpMesh, Model);
            
            loaded_model_mesh NewMesh = {};
            NewMesh.Mesh = OurMesh;
            NewMesh.MaterialIndex = 0;
            NewMesh.Name = std::string(AssimpMesh->mName.C_Str());
            Model->Meshes.push_back(NewMesh);
            
            OurNode->MeshIndices.push_back(Model->Meshes.size() - 1);
        }
    }
}

struct loading_params
{
    f32 Model_DefaultScale;
    b32 Model_FixInvalidRotation;
};

inline loading_params DefaultLoadingParams()
{
    loading_params Result = {};
    
    Result.Model_FixInvalidRotation = false;
    Result.Model_DefaultScale = 1.0f;
    
    return(Result);
}

// TODO(Dima): Change this to "model" return type
loaded_model LoadModelFromFile(char* FilePath, loading_params Params = DefaultLoadingParams())
{
    Assimp::Importer Importer;
    
    bool WasExisting = Importer.SetPropertyInteger("AI_CONFIG_PP_SBP_REMOVE", 
                                                   aiPrimitiveType_POINT | aiPrimitiveType_LINE);
    
    const aiScene* Scene = Importer.ReadFile(FilePath,         
                                             aiProcess_CalcTangentSpace       | 
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_LimitBoneWeights | 
                                             aiProcess_ImproveCacheLocality | 
                                             aiProcess_RemoveRedundantMaterials | 
                                             aiProcess_FixInfacingNormals | 
                                             aiProcess_FindDegenerates |
                                             aiProcess_SortByPType | 
                                             aiProcess_FindInvalidData | 
                                             aiProcess_OptimizeMeshes |
                                             //aiProcess_OptimizeGraph | 
                                             aiProcess_ValidateDataStructure);
    
    
#if 1    
    double UnitScaleFactor = 1.0;
    Scene->mMetaData->Get("UnitScaleFactor", UnitScaleFactor);
#endif
    
    // NOTE(Dima): Calculating transform
    m44 RootTran = AssimpToOurMatrix(Scene->mRootNode->mTransformation);
    m44 InvRootTran = InverseTransformMatrix(RootTran);
    m44 AdditionalScale = ScalingMatrix(Params.Model_DefaultScale);
    
    InvRootTran = AdditionalScale * InvRootTran;
    
    // NOTE(Dima): Applying additional rotation
    if(Params.Model_FixInvalidRotation)
    {
        m44 AdditionalRotation = RotationMatrixX(-90.0f * F_DEG2RAD);
        
        InvRootTran = AdditionalRotation * InvRootTran;
    }
    
    // NOTE(Dima): Calculate premultiply matrix
    m44 GlobalTransform = InvRootTran;
    //m44 GlobalTransform = IdentityMatrix4();
    
    // NOTE(Dima): Loading meshes
    loaded_model Loaded;
    
    AssimpProcessNode((aiScene*)Scene, 
                      Scene->mRootNode, 
                      0, GlobalTransform, 
                      &Loaded);
    
    LoadModelBones(Scene, &Loaded, GlobalTransform);
    LoadModelMeshes(Scene, &Loaded);
    
    return(Loaded);
}