#include <vector>
#include <map>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define DefineOffset(name, type, count) \
{\
mi Size##name = sizeof(type) * (count); \
(TargetStruct)->Offsets.Offset##name = DataSize; \
DataSize += Size##name; \
}

#define AssignBasedOnOffset(name, type, count) \
if((count) > 0){\
(TargetStruct)->##name= (type*)((u8*)Data + (TargetStruct)->Offsets.Offset##name);\
}\
else{\
(TargetStruct)->##name = 0;}

INTERNAL_FUNCTION image AllocateImageInternal(u32 Width, u32 Height, void* PixData)
{
    image Result = {};
    
	Result.Width = Width;
	Result.Height = Height;
    
	Result.WidthOverHeight = (float)Width / (float)Height;
    
	Result.Pixels = PixData;
    
	return(Result);
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

INTERNAL_FUNCTION font LoadFontFromBuffer(u8* Buffer, int FontSize)
{
    font Result = {};
    
    Result.Size = FontSize;
    
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

INTERNAL_FUNCTION font LoadFontFile(char* FilePath, int FontSize)
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
        Result = LoadFontFromBuffer(Buffer, FontSize);
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
    mesh* TargetStruct = &Result;
    
    DefineOffset(P, v3, Vertices.size());
    DefineOffset(UV, v2, TexCoords.size());
    DefineOffset(N, v3, Normals.size());
    DefineOffset(C, u32, Colors.size());
    
    if(IsSkinned)
    {
        DefineOffset(BoneWeights, v4, BoneWeights.size());
        DefineOffset(BoneIndices, u32, BoneIndices.size());
    }
    
    // NOTE(Dima): Allocating buffer for holding vertex data
    u8* Data = (u8*)malloc(DataSize);
    
    AssignBasedOnOffset(P, v3, Vertices.size());
    AssignBasedOnOffset(UV, v2, TexCoords.size());
    AssignBasedOnOffset(N, v3, Normals.size());
    AssignBasedOnOffset(C, u32, Colors.size());
    
    if(IsSkinned)
    {
        AssignBasedOnOffset(BoneWeights, v4, BoneWeights.size());
        AssignBasedOnOffset(BoneIndices, u32, BoneIndices.size());
    }
    
    // NOTE(Dima): Saving vertices data buffer
    Result.VertexCount = Vertices.size();
    Result.VerticesDataBuffer = Data;
    Result.VerticesDataBufferSize = DataSize;
    Result.IsSkinned = IsSkinned;
    
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
        Result.C[VertexIndex] = PackRGB(Colors[VertexIndex]);
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

struct loaded_node_animation
{
    std::vector<v3> PositionKeys;
    std::vector<quat> RotationKeys;
    std::vector<v3> ScalingKeys;
    
    std::vector<f32> PositionTimes;
    std::vector<f32> RotationTimes;
    std::vector<f32> ScalingTimes;
    
    int NodeIndex;
    u32 DataSize;
};

struct loaded_animation
{
    std::vector<loaded_node_animation> NodeAnims;
    
    f32 DurationTicks;
    f32 TicksPerSecond;
    u32 Behaviour;
    
    std::string Name;
};

struct loaded_model
{
    int NumMeshes;
    mesh* Meshes;
    
    std::vector<loaded_model_node> Nodes;
    std::vector<material*> Materials;
    
    model_bone Bones[256];
    int NumBones;
    
    // NOTE(Dima): Help structures
    std::map<std::string, int> NodeNameToNodeIndex;
    std::map<std::string, int> NodeNameToBoneIndex;
    
    std::vector<loaded_animation> Animations;
    
    void* ModelFree;
    void* MeshesFree;
    void* MaterialsFree;
};

mesh ConvertAssimpMesh(const aiScene* AssimpScene, 
                       aiMesh* AssimpMesh, 
                       loaded_model* Model)
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
        v3 OurColor = V3(1.0f, 1.0f, 1.0f);
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
    
    std::vector<v4> BoneWeights;
    std::vector<u32> BoneIndices;
    
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
        
        // NOTE(Dima): Preparing weights
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
    
    std::string MeshName = std::string(AssimpMesh->mName.C_Str());
    
    CopyStringsSafe(Result.Name, ArrayCount(Result.Name), (char*)MeshName.c_str());
    Result.MaterialIndexInModel = 0;
    
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
    
    // NOTE(Dima): Initializing new Node
    int ThisNodeIndex = Loaded->Nodes.size();;
    
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
                                      loaded_model* Model)
{
    Model->NumBones = 0;
    
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
                model_bone* NewBone = &Model->Bones[Model->NumBones++];
                
                std::string BoneName = std::string(AiBone->mName.C_Str());
                
                NewBone->InvBindPose = AssimpToOurMatrix(AiBone->mOffsetMatrix);
                NewBone->NodeIndex = Model->NodeNameToNodeIndex[BoneName];
                
                Model->NodeNameToBoneIndex.insert(std::pair<std::string, int>(BoneName, Model->NumBones - 1));
            }
        }
    }
}

INTERNAL_FUNCTION void LoadModelMeshes(const aiScene* AssimpScene,
                                       loaded_model* Model)
{
    Model->NumMeshes = AssimpScene->mNumMeshes;
    Model->Meshes = (mesh*)malloc(sizeof(mesh) * Model->NumMeshes);
    
    // NOTE(Dima): Loading meshes
    for(int MeshIndex = 0;
        MeshIndex < Model->NumMeshes;
        MeshIndex++)
    {
        aiMesh* AssimpMesh = AssimpScene->mMeshes[MeshIndex];
        
        Model->Meshes[MeshIndex] = ConvertAssimpMesh(AssimpScene, AssimpMesh, Model);
    }
    
    // NOTE(Dima): Loading mesh indices
    for(int NodeIndex = 0; 
        NodeIndex < Model->Nodes.size();
        NodeIndex++)
    {
        loaded_model_node* OurNode = &Model->Nodes[NodeIndex];
        
        // NOTE(Dima): Loading this nodes meshes
        aiNode* AssimpNode = OurNode->AssimpNode;
        
        for(int MeshIndex = 0; MeshIndex < AssimpNode->mNumMeshes; MeshIndex++)
        {
            int MeshIndexInScene = AssimpNode->mMeshes[MeshIndex];
            
            OurNode->MeshIndices.push_back(MeshIndexInScene);
        }
    }
    
    Model->MeshesFree = Model->Meshes;
}

INTERNAL_FUNCTION animation* ConvertToActualAnimation(loaded_animation* Load)
{
    int NumNodeAnims = Load->NodeAnims.size();
    u32 TempSize = sizeof(animation) + 
        sizeof(node_animation*) * NumNodeAnims + 
        sizeof(node_animation) * NumNodeAnims;
    
    animation* Dst = (animation*)malloc(TempSize);
    
    node_animation** NodeAnimPtrs = (node_animation**)((u8*)Dst + sizeof(animation));
    node_animation* NodeAnims = (node_animation*)((u8*)NodeAnimPtrs + sizeof(node_animation*) * Load->NodeAnims.size());
    
    Dst->DurationTicks = Load->DurationTicks;
    Dst->TicksPerSecond = Load->TicksPerSecond;
    Dst->Behaviour = Load->Behaviour;
    Dst->NodeAnimations = NodeAnimPtrs;
    Dst->NumNodeAnimations = NumNodeAnims;
    
    CopyStringsSafe(Dst->Name, ArrayCount(Dst->Name), (char*)Load->Name.c_str());
    
    for(int NodeAnimIndex = 0;
        NodeAnimIndex < NumNodeAnims;
        NodeAnimIndex++)
    {
        loaded_node_animation* Src = &Load->NodeAnims[NodeAnimIndex];
        
        node_animation* NodeAnim = &NodeAnims[NodeAnimIndex];
        NodeAnimPtrs[NodeAnimIndex] = NodeAnim;
        
        u32 DataSize = 0;
        node_animation* TargetStruct = NodeAnim;
        
        DefineOffset(PositionKeys, v3, Src->PositionKeys.size());
        DefineOffset(RotationKeys, quat, Src->RotationKeys.size());
        DefineOffset(ScalingKeys, quat, Src->ScalingKeys.size());
        DefineOffset(PositionTimes, f32, Src->PositionTimes.size());
        DefineOffset(RotationTimes, f32, Src->RotationTimes.size());
        DefineOffset(ScalingTimes, f32, Src->ScalingTimes.size());
        
        void* Data = malloc(DataSize);
        
        AssignBasedOnOffset(PositionKeys, v3, Src->PositionKeys.size());
        AssignBasedOnOffset(RotationKeys, quat, Src->RotationKeys.size());
        AssignBasedOnOffset(ScalingKeys, v3, Src->ScalingKeys.size());
        AssignBasedOnOffset(PositionTimes, f32, Src->PositionTimes.size());
        AssignBasedOnOffset(RotationTimes, f32, Src->RotationTimes.size());
        AssignBasedOnOffset(ScalingTimes, f32, Src->ScalingTimes.size());
        
        NodeAnim->PositionCount = Src->PositionKeys.size();
        NodeAnim->RotationCount = Src->RotationKeys.size();
        NodeAnim->ScalingCount = Src->ScalingKeys.size();
        
        // NOTE(Dima): Copy position values and times
        for(int PosIndex = 0; 
            PosIndex < NodeAnim->PositionCount;
            PosIndex++)
        {
            NodeAnim->PositionKeys[PosIndex] = Src->PositionKeys[PosIndex];
            NodeAnim->PositionTimes[PosIndex] = Src->PositionTimes[PosIndex];
        }
        
        // NOTE(Dima): Copy rotation values and times
        for(int RotIndex = 0;
            RotIndex < NodeAnim->RotationCount;
            RotIndex++)
        {
            NodeAnim->RotationKeys[RotIndex] = Src->RotationKeys[RotIndex];
            NodeAnim->RotationTimes[RotIndex] = Src->RotationTimes[RotIndex];
        }
        
        // NOTE(Dima): Copy scaling values and times
        for(int ScaIndex = 0;
            ScaIndex < NodeAnim->ScalingCount;
            ScaIndex++)
        {
            NodeAnim->ScalingKeys[ScaIndex] = Src->ScalingKeys[ScaIndex];
            NodeAnim->ScalingTimes[ScaIndex] = Src->ScalingTimes[ScaIndex];
        }
        
        NodeAnim->NodeIndex = Src->NodeIndex;
    }
    
    return(Dst);
}

INTERNAL_FUNCTION void LoadModelAnimations(const aiScene* AssimpScene,
                                           loaded_model* Model)
{
    int NumAnims = AssimpScene->mNumAnimations;
    if(AssimpScene->HasAnimations() && (NumAnims > 0))
    {
        // NOTE(Dima): Loading animations
        Model->Animations.reserve(AssimpScene->mNumAnimations);
        
        for(int AnimIndex = 0;
            AnimIndex < AssimpScene->mNumAnimations;
            AnimIndex++)
        {
            aiAnimation* AiAnim = AssimpScene->mAnimations[AnimIndex];
            
            loaded_animation NewAnim;
            
            // NOTE(Dima): Setting animation info
            NewAnim.Name = std::string(AiAnim->mName.C_Str());
            NewAnim.DurationTicks = AiAnim->mDuration;
            NewAnim.TicksPerSecond = AiAnim->mTicksPerSecond;
            
            // NOTE(Dima): Loading node animations
            for(int ChannelIndex = 0;
                ChannelIndex < AiAnim->mNumChannels;
                ChannelIndex++)
            {
                aiNodeAnim* NodeAnim = AiAnim->mChannels[ChannelIndex];
                
                loaded_node_animation NewNodeAnim = {};
                
                NewNodeAnim.NodeIndex = Model->NodeNameToNodeIndex[std::string(NodeAnim->mNodeName.C_Str())];
                
                // NOTE(Dima): Load position keys
                for(int PositionKeyIndex = 0;
                    PositionKeyIndex < NodeAnim->mNumPositionKeys;
                    PositionKeyIndex++)
                {
                    aiVectorKey PosKey = NodeAnim->mPositionKeys[PositionKeyIndex];
                    
                    NewNodeAnim.PositionTimes.push_back(PosKey.mTime);
                    NewNodeAnim.PositionKeys.push_back(V3(PosKey.mValue.x,
                                                          PosKey.mValue.y,
                                                          PosKey.mValue.z));
                }
                
                // NOTE(Dima): Load rotation keys
                for(int RotationKeyIndex = 0;
                    RotationKeyIndex < NodeAnim->mNumRotationKeys;
                    RotationKeyIndex++)
                {
                    aiQuatKey RotKey = NodeAnim->mRotationKeys[RotationKeyIndex];
                    
                    NewNodeAnim.RotationTimes.push_back(RotKey.mTime);
                    NewNodeAnim.RotationKeys.push_back(Quaternion(RotKey.mValue.x,
                                                                  RotKey.mValue.y,
                                                                  RotKey.mValue.z,
                                                                  RotKey.mValue.w));
                }
                
                // NOTE(Dima): Load scaling keys
                for(int ScalingKeyIndex = 0;
                    ScalingKeyIndex < NodeAnim->mNumScalingKeys;
                    ScalingKeyIndex++)
                {
                    aiVectorKey ScaKey = NodeAnim->mScalingKeys[ScalingKeyIndex];
                    
                    NewNodeAnim.ScalingTimes.push_back(ScaKey.mTime);
                    NewNodeAnim.ScalingKeys.push_back(V3(ScaKey.mValue.x,
                                                         ScaKey.mValue.y,
                                                         ScaKey.mValue.z));
                }
                
                // NOTE(Dima): Pushing animation to animations array
                NewAnim.NodeAnims.push_back(NewNodeAnim);
            }
            
            Model->Animations.push_back(NewAnim);
        }
    }
}

INTERNAL_FUNCTION model* ConvertToActualModel(loaded_model* Load)
{
    model* Model = (model*)malloc(sizeof(model));
    
    Model->NumNodes = Load->Nodes.size();
    Model->NumBones = Load->NumBones;
    Model->NumMeshes = Load->NumMeshes;
    Model->NumMaterials = Load->Materials.size();
    
    int AllocMaterialsCount = std::max(50, (int)Load->Materials.size());
    
    {
        u32 DataSize = 0;
        model* TargetStruct = Model;
        
        DefineOffset(Meshes, mesh*, Model->NumMeshes);
        DefineOffset(Materials, material*, AllocMaterialsCount);
        DefineOffset(Nodes, model_node, Model->NumNodes);
        DefineOffset(NodeToModel, m44, Model->NumNodes);
        DefineOffset(NodeToParent, m44, Model->NumNodes);
        DefineOffset(ParentNodeIndex, int, Model->NumNodes);
        DefineOffset(Bones, model_bone, Model->NumBones);
        DefineOffset(SkinningMatrices, m44, Model->NumBones);
        
        void* Data = malloc(DataSize);
        
        AssignBasedOnOffset(Meshes, mesh*, Model->NumMeshes);
        AssignBasedOnOffset(Materials, material*, AllocMaterialsCount);
        AssignBasedOnOffset(Nodes, model_node, Model->NumNodes);
        AssignBasedOnOffset(NodeToModel, m44, Model->NumNodes);
        AssignBasedOnOffset(NodeToParent, m44, Model->NumNodes);
        AssignBasedOnOffset(ParentNodeIndex, int, Model->NumNodes);
        AssignBasedOnOffset(Bones, model_bone, Model->NumBones);
        AssignBasedOnOffset(SkinningMatrices, m44, Model->NumBones);
        
        Load->ModelFree = Data;
    }
    
    // NOTE(Dima): Setting meshes
    for(int MeshIndex = 0;
        MeshIndex < Load->NumMeshes;
        MeshIndex++)
    {
        Model->Meshes[MeshIndex] = &Load->Meshes[MeshIndex];
    }
    
    // NOTE(Dima): Setting materials
    for(int MaterialIndex = 0;
        MaterialIndex < Model->NumMaterials;
        MaterialIndex++)
    {
        Model->Materials[MaterialIndex] = Load->Materials[MaterialIndex];
    }
    
    // NOTE(Dima): Setting bones
    for(int BoneIndex = 0;
        BoneIndex < Model->NumBones;
        BoneIndex++)
    {
        Model->Bones[BoneIndex] = Load->Bones[BoneIndex];;
    }
    
    // NOTE(Dima): Setting nodes
    for(int NodeIndex = 0;
        NodeIndex < Model->NumNodes;
        NodeIndex++)
    {
        model_node* Node = &Model->Nodes[NodeIndex];
        loaded_model_node* Src = &Load->Nodes[NodeIndex];
        
        CopyStringsSafe(Node->Name, ArrayCount(Node->Name), (char*)Src->Name.c_str());
        Node->NumChildIndices = Src->ChildNodesIndices.size();
        Node->NumMeshIndices = Src->MeshIndices.size();
        
        Assert(Node->NumChildIndices <= ArrayCount(Node->ChildIndices));
        Assert(Node->NumMeshIndices <= ArrayCount(Node->MeshIndices));
        
        Node->NumChildIndices = Min(Node->NumChildIndices, ArrayCount(Node->ChildIndices));
        Node->NumMeshIndices = Min(Node->NumMeshIndices, ArrayCount(Node->MeshIndices));
        
        // NOTE(Dima): Copy child indices
        for(int ChildIndex = 0;
            ChildIndex < Node->NumChildIndices;
            ChildIndex++)
        {
            Node->ChildIndices[ChildIndex] = Src->ChildNodesIndices[ChildIndex];
        }
        
        // NOTE(Dima): Copy mesh indices
        for(int MeshIndex = 0;
            MeshIndex < Node->NumMeshIndices;
            MeshIndex++)
        {
            Node->MeshIndices[MeshIndex] = Src->MeshIndices[MeshIndex];
        }
        
        // NOTE(Dima): Copy other arrays
        Model->NodeToModel[NodeIndex] = Src->ToModel;
        Model->NodeToParent[NodeIndex] = Src->ToParent;
        Model->ParentNodeIndex[NodeIndex] = Src->ParentNodeIndex;
    }
    
    return(Model);
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

INTERNAL_FUNCTION loaded_model LoadModelFileInternal(char* FilePath,
                                                     loading_params Params = DefaultLoadingParams(),
                                                     b32 LoadAnimations = true,
                                                     b32 LoadOnlyAnimations = false)
{
    Assimp::Importer Importer;
    Importer.SetPropertyInteger("AI_CONFIG_PP_SBP_REMOVE", 
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
    
    LoadModelBones(Scene, &Loaded);
    
    if(LoadAnimations)
    {
        LoadModelAnimations(Scene, &Loaded);
    }
    
    if(!LoadOnlyAnimations)
    {
        LoadModelMeshes(Scene, &Loaded);
    }
    
    return(Loaded);
}

// NOTE(Dima): Pass LoadAnimations = true to get all animations that were in this file
model* LoadModel(char* FilePath,
                 loading_params Params = DefaultLoadingParams(),
                 b32 LoadAnimations = false)
{
    b32 LoadOnlyAnimations = false;
    
    loaded_model Loaded = LoadModelFileInternal(FilePath, Params, LoadAnimations, LoadOnlyAnimations);
    
    model* Result = ConvertToActualModel(&Loaded);
    
    return(Result);
}

// NOTE(Dima): Use this function to load only 1 animation from file
struct loaded_animations
{
    animation** Animations;
    int Count;
};

loaded_animations LoadSkeletalAnimations(char* FilePath, 
                                         loading_params Params = DefaultLoadingParams())
{
    loaded_animations Result = {};
    
    b32 LoadAnimations = true;
    b32 LoadOnlyAnimations = true;
    
    loaded_model Loaded = LoadModelFileInternal(FilePath, Params, 
                                                LoadAnimations, 
                                                LoadOnlyAnimations);
    
    Result.Count = Loaded.Animations.size();
    if(Result.Count)
    {
        Result.Animations = (animation**)malloc(sizeof(animation*) * Result.Count);
        
        for(int AnimIndex = 0;
            AnimIndex < Result.Count;
            AnimIndex++)
        {
            Result.Animations[AnimIndex] = ConvertToActualAnimation(&Loaded.Animations[AnimIndex]);
        }
    }
    
    return(Result);
}

animation* LoadSkeletalAnimation(char* FilePath, 
                                 loading_params Params = DefaultLoadingParams())
{
    
    b32 LoadAnimations = true;
    b32 LoadOnlyAnimations = true;
    
    loaded_model Loaded = LoadModelFileInternal(FilePath, Params, 
                                                LoadAnimations, 
                                                LoadOnlyAnimations);
    
    animation* Result = 0;
    if(Loaded.Animations.size())
    {
        Result = ConvertToActualAnimation(&Loaded.Animations[0]);
    }
    
    return(Result);
}

void FreeLoadedAnimations(loaded_animations* Animations)
{
    if(Animations->Count)
    {
        for(int AnimationIndex = 0;
            AnimationIndex < Animations->Count;
            AnimationIndex++)
        {
            free(Animations->Animations[AnimationIndex]);
        }
        
        free(Animations->Animations);
        Animations->Animations = 0;
    }
}