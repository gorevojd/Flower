inline void* PushRenderCommand_(render_commands* Commands, u32 CommandType, u32 SizeOfCommandStruct)
{
    render_command_header* Head = &Commands->CommandHeaders[Commands->CommandCount++];
    
    Head->CommandType = CommandType;
    Head->OffsetToCommand = GetUsedInCurrentBlock(&Commands->CommandsBuffer);
    
    void* CommandData = AllocateFromArena(&Commands->CommandsBuffer, SizeOfCommandStruct, 8);
    
    return(CommandData);
}

#define PushRenderCommand(commands, type, struct_type) (struct_type*)PushRenderCommand_(commands, type, sizeof(struct_type))

inline void PushMesh(render_commands* Commands, mesh* Mesh, v3 P, v3 C)
{
    render_command_mesh* Entry = PushRenderCommand(Commands, RenderCommand_Mesh, render_command_mesh);
    
    Entry->Mesh = Mesh;
    Entry->P = P;
    Entry->C = C;
}

inline void PushBitmap(render_commands* Commands, image* Img, v2 P, v3 C)
{
    render_command_image* Entry = PushRenderCommand(Commands, RenderCommand_Image, render_command_image);
    
    Entry->Image = Img;
    Entry->P = P;
    Entry->C = C;
}

INTERNAL_FUNCTION render_command_header* 
GetFirstCommand(render_commands* Commands)
{
    memory_block* Block = GetCurrentBlock(&Commands->CommandsBuffer);
    
    render_command_header* Head = (render_command_header*)Block->Base;
    
    return(Head);
}

inline void* GetRenderCommand_(render_commands* Commands, int CommandIndex)
{
    render_command_header* Header = &Commands->CommandHeaders[CommandIndex];
    
    void* Result = Header->OffsetToCommand + (u8*)Commands->CommandsBuffer.Block->Base;
    
    return(Result);
}
#define GetRenderCommand(commands, index, struct_type) (struct_type*)GetRenderCommand_(commands, index)

INTERNAL_FUNCTION void BeginRender(render_commands* Commands)
{
    
}

INTERNAL_FUNCTION void EndRender(render_commands* Commands)
{
    FreeArena(&Commands->CommandsBuffer, true);
    
    Commands->CommandCount = 0;
}