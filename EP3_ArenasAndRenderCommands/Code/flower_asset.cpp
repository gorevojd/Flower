#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

INTERNAL_FUNCTION image AllocateImageInternal(u32 Width, u32 Height, void* PixData)
{
    image Result = {};
    
	Result.Width = Width;
	Result.Height = Height;
    
	Result.WidthOverHeight = (float)Width / (float)Height;
    
	Result.Pixels = PixData;
    
	return(Result);
}

INTERNAL_FUNCTION image LoadImageFile(char* FilePath)
{
    int Width;
    int Height;
    int Channels;
    
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

INTERNAL_FUNCTION mesh MakeMesh(const std::vector<v3>& Vertices,
                                const std::vector<v2>& TexCoords,
                                const std::vector<v3>& Normals,
                                const std::vector<v3>& Colors,
                                const std::vector<u32>& Indices)
{
    mesh Result = {};
    
    u32 VerticesSize = sizeof(vertex) * Vertices.size();
    u32 IndexSize = sizeof(u32) * Indices.size();
    
    Result.Vertices = (vertex*)malloc(VerticesSize);
    Result.VertexCount = Vertices.size();
    
    Result.Indices = (u32*)malloc(IndexSize);
    Result.IndexCount = Indices.size();
    
    for(int VertexIndex = 0;
        VertexIndex < Vertices.size();
        VertexIndex++)
    {
        vertex* Target = &Result.Vertices[VertexIndex];
        
        Target->P = Vertices[VertexIndex];
        Target->UV = TexCoords[VertexIndex];
        Target->N = Normals[VertexIndex];
        Target->C = Colors[VertexIndex];
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
