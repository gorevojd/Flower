#ifndef FLOWER_RENDER_H
#define FLOWER_RENDER_H

enum render_command_type
{
    RenderCommand_Image,
    RenderCommand_Image3D,
    RenderCommand_Mesh,
};

#define RENDER_COMMAND_STRUCT(type) render_command_##type

#pragma pack(push, 8)
struct render_command_image
{
    image* Image;
    
    v2 P;
    v2 Dim;
    v4 C;
};

struct render_command_mesh
{
    mesh* Mesh;
    
    v3 P;
    v3 C;
};

struct render_command_header
{
    u32 CommandType;
    u32 OffsetToCommand;
};
#pragma pack(pop)

struct glyph_vertex
{
    v2 P;
    v2 UV;
};

enum glyph_type
{
    Glyph_Glyph = 0,
    Glyph_Rectangle = 1,
};

struct glyph_buffer
{
#define MAX_GLYPHS_COUNT 30000
    glyph_vertex GlyphVertices[MAX_GLYPHS_COUNT * 4];
    u32 GlyphIndices[MAX_GLYPHS_COUNT * 6];
    u32 GlyphColors[MAX_GLYPHS_COUNT];
    u8 GlyphTypes[MAX_GLYPHS_COUNT];
    u16 GlyphIndicesToTransforms[MAX_GLYPHS_COUNT];
    
    m44 GlyphTransforms[MAX_GLYPHS_COUNT / 10];
    int GlyphTransformsCount;
    
    int GlyphDefaultProjectionMatrixIndex;
    int GlyphOrthoMatrixIndex;
    int GlyphPerspMatrixIndex;
    
    int GlyphCount;
};

struct render_commands
{
    memory_arena CommandsBuffer;
    
    // TODO(Dima): Make those dynamic
#define MAX_RENDER_COMMANDS_COUNT 100000
    render_command_header CommandHeaders[MAX_RENDER_COMMANDS_COUNT];
    int CommandCount;
    
    glyph_buffer Glyphs2D;
    glyph_buffer Glyphs3D;
};

struct window_dimensions
{
    int InitWidth;
    int InitHeight;
    
    int Width;
    int Height;
};

struct render_params
{
    int WindowWidth;
    int WindowHeight;
    
    window_dimensions WndDims;
    
    m44* View;
    m44* Projection;
    m44* ViewProjection;
    m44* ScreenOrthoProjection;
    
    
    f64 Time;
    mesh* Cube;
    image* CubeTexture;
    image* FontAtlas;
    
    struct render_commands* Commands;
};


#endif //FLOWER_RENDER_H
