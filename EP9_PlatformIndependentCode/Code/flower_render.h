#ifndef FLOWER_RENDER_H
#define FLOWER_RENDER_H

#define RENDER_DEFAULT_2D_LINE_THICKNESS 4.0f
#define RENDER_DEFAULT_2D_LINE_DASH_LENGTH 20.0f
#define RENDER_DEFAULT_2D_LINE_DASH_SPACING 8.0f
#define RENDER_MAX_BONES 256

enum render_command_type
{
    RenderCommand_Image,
    RenderCommand_Mesh,
    RenderCommand_InstancedMesh,
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
    m44* SkinningMatrices;
    v3 C;
    int SkinningMatricesCount;
    
    m44 ModelToWorld;
};

struct render_command_instanced_mesh
{
    mesh* Mesh;
    material* Material;
    v3 C;
    
    m44* InstanceSkinningMatrices;
    int NumSkinningMatricesPerInstance;
    
    m44* InstanceMatrices;
    int InstanceCount;
    int MaxInstanceCount;
};

struct render_mesh_instance
{
    render_mesh_instance* NextInHash;
    
    mesh* Mesh;
    render_command_instanced_mesh* Command;
};

struct render_command_header
{
    u32 CommandType;
    void* CommandData;
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


struct window_dimensions
{
    int InitWidth;
    int InitHeight;
    
    int Width;
    int Height;
};

struct render_commands
{
    memory_arena CommandsBuffer;
    
    // TODO(Dima): Make those dynamic
#define MAX_RENDER_COMMANDS_COUNT 200000
    render_command_header CommandHeaders[MAX_RENDER_COMMANDS_COUNT];
    int CommandCount;
    
    rect_buffer Rects2D;
    rect_buffer Rects3D;
    
    m44 View;
    m44 Projection;
    m44 ViewProjection;
    m44 ScreenOrthoProjection;
    
    window_dimensions WindowDimensions;
    image* FontAtlas;
    
#define RENDER_INSTANCE_TABLE_SIZE 256
    render_mesh_instance* InstanceTable[RENDER_INSTANCE_TABLE_SIZE];
};

#endif //FLOWER_RENDER_H
