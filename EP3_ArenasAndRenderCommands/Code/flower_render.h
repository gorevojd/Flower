#ifndef FLOWER_RENDER_H
#define FLOWER_RENDER_H

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
    v3 C;
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

struct render_commands
{
    memory_arena CommandsBuffer;
    
#define MAX_RENDER_COMMANDS_COUNT 100000
    render_command_header CommandHeaders[MAX_RENDER_COMMANDS_COUNT];
    int CommandCount;
};

#endif //FLOWER_RENDER_H
