#ifndef FLOWER_ASSET_H
#define FLOWER_ASSET_H

struct vertex
{
    v3 P;
    v2 UV;
    v3 N;
    v3 C;
};

struct mesh_handles
{
    u64 ArrayObject;
    u64 BufferObject;
    u64 ElementBufferObject;
};

struct mesh
{
    vertex* Vertices;
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
    mesh_handles ApiHandles;
};


inline b32 MeshInitialized(mesh* Mesh)
{
    b32 Result = Mesh->ApiHandles.ArrayObject != 0;
    
    return(Result);
}

struct image
{
    void* Pixels;
    
    int Width;
    int Height;
    
    float WidthOverHeight;
    
    u64 ApiHandle;
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

struct asset_system
{
    image FontsAtlas;
    v2 FontAtlasAtP;
    int FontAtlasMaxRowY;
};

#endif //FLOWER_ASSET_H
