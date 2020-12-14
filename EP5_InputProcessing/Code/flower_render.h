#ifndef FLOWER_RENDER_H
#define FLOWER_RENDER_H

#define RENDER_DEFAULT_2D_LINE_THICKNESS 4.0f

enum render_command_type
{
    RenderCommand_Image,
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
    material* Material;
    
    m44 ModelToWorld;
    
    v3 C;
};

struct render_command_header
{
    u32 CommandType;
    u32 OffsetToCommand;
};
#pragma pack(pop)

struct rect_vertex
{
    v2 P;
    v2 UV;
};

enum rect_type
{
    Rect_Textured = 0,
    Rect_Solid = 1,
};

struct rect_buffer
{
#define MAX_RECTS_COUNT 30000
    rect_vertex Vertices[MAX_RECTS_COUNT * 4];
    u32 Indices[MAX_RECTS_COUNT * 6];
    u32 Colors[MAX_RECTS_COUNT];
    u8 Types[MAX_RECTS_COUNT];
    u16 IndicesToTransforms[MAX_RECTS_COUNT];
    
    m44 Transforms[MAX_RECTS_COUNT / 10];
    int TransformsCount;
    
    int IdentityMatrixIndex;
    int OrthoMatrixIndex;
    int ViewProjMatrixIndex;
    
    int RectCount;
};

struct render_commands
{
    memory_arena CommandsBuffer;
    
    // TODO(Dima): Make those dynamic
#define MAX_RENDER_COMMANDS_COUNT 100000
    render_command_header CommandHeaders[MAX_RENDER_COMMANDS_COUNT];
    int CommandCount;
    
    rect_buffer Rects2D;
    rect_buffer Rects3D;
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
