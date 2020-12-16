#ifndef FLOWER_RENDER_PRIMITIVES_H
#define FLOWER_RENDER_PRIMITIVES_H

#if 0
struct vertex
{
    v3 P;
    v2 UV;
    v3 N;
    v3 C;
};

struct skinned_vertex
{
    v3 P;
    v2 UV;
    v3 N;
    v3 C;
    
    v4 BoneWeights;
    u32 BoneIndices; 
};
#endif

struct mesh_handles
{
    u64 ArrayObject;
    u64 BufferObject;
    u64 ElementBufferObject;
    
    b32 Initialized;
};

struct mesh
{
    void* VerticesDataBuffer;
    u32 VerticesDataBufferSize;
    
    v3* P;
    v2* UV;
    v3* N;
    v3* C;
    v4* BoneWeights;
    u32* BoneIndices;
    
    u32 OffsetToP;
    u32 OffsetToUV;
    u32 OffsetToN;
    u32 OffsetToC;
    u32 OffsetToBoneWeights;
    u32 OffsetToBoneIndices;
    
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
    b32 IsSkinned;
    
    mesh_handles ApiHandles;
};

struct image
{
    void* Pixels;
    
    int Width;
    int Height;
    
    float WidthOverHeight;
    
    b32 FilteringIsClosest;
    
    u64 ApiHandle;
};

struct material
{
    image* Diffuse;
};

struct glyph
{
    image Image;
    
    u32 Codepoint;
    
    f32 Advance;
    f32 LeftBearing;
    
    f32 XOffset;
    f32 YOffset;
    
    v2 MinUV;
    v2 MaxUV;
};

struct font
{
    glyph* Glyphs;
    int GlyphCount;
    
    float* KerningPairs;
    
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
    
    f32 LineAdvance;
    
    int Size;
};

#endif //FLOWER_RENDER_PRIMITIVES_H
