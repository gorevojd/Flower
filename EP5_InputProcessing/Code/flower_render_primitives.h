#ifndef FLOWER_RENDER_PRIMITIVES_H
#define FLOWER_RENDER_PRIMITIVES_H

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
    
    b32 Initialized;
};

struct mesh
{
    vertex* Vertices;
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
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
