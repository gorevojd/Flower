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

inline void PushImage(render_commands* Commands, image* Img, v2 P, f32 Height, v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    render_command_image* Entry = PushRenderCommand(Commands, RenderCommand_Image, render_command_image);
    
    Entry->Image = Img;
    Entry->P = P;
    Entry->C = C;
    Entry->Dim = V2(Height * Img->WidthOverHeight, Height);
}

// TODO(Dima): Perfomance critical function. Maybe think about how can we optimize it even more
inline void PushGlyph(glyph_buffer* GlyphBuffer,
                      glyph* Glyph, 
                      v2 P, f32 Height, 
                      b32 GlyphType,
                      int IndexToTransformMatrix,
                      v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    v2 Dim = V2(Height * Glyph->Image.WidthOverHeight, Height);
    
    v2 MinUV = Glyph->MinUV;
    v2 MaxUV = Glyph->MaxUV;
    
    glyph_vertex Vertex0 = { V2(P.x, P.y), V2(MinUV.x, MinUV.y)};
    glyph_vertex Vertex1 = { V2(P.x + Dim.x, P.y), V2(MaxUV.x, MinUV.y)};
    glyph_vertex Vertex2 = { V2(P.x + Dim.x, P.y + Dim.y), V2(MaxUV.x, MaxUV.y)};
    glyph_vertex Vertex3 = { V2(P.x, P.y + Dim.y), V2(MinUV.x, MaxUV.y)};
    
    Assert(GlyphBuffer->GlyphCount < MAX_GLYPHS_COUNT);
    
    int VertexAt = GlyphBuffer->GlyphCount * 4;
    
    glyph_vertex* Vertices = &GlyphBuffer->GlyphVertices[VertexAt];;
    Vertices[0] = Vertex0;
    Vertices[1] = Vertex1;
    Vertices[2] = Vertex2;
    Vertices[3] = Vertex3;
    
    u32* Indices = &GlyphBuffer->GlyphIndices[GlyphBuffer->GlyphCount * 6];
    Indices[0] = VertexAt + 0;
    Indices[1] = VertexAt + 1;
    Indices[2] = VertexAt + 2;
    Indices[3] = VertexAt + 0;
    Indices[4] = VertexAt + 2;
    Indices[5] = VertexAt + 3;
    
    GlyphBuffer->GlyphColors[GlyphBuffer->GlyphCount] = PackRGBA(C);
    GlyphBuffer->GlyphTypes[GlyphBuffer->GlyphCount] = GlyphType;
    GlyphBuffer->GlyphIndicesToTransforms[GlyphBuffer->GlyphCount] = IndexToTransformMatrix;
    
    GlyphBuffer->GlyphCount++;
}

inline void* GetRenderCommand_(render_commands* Commands, int CommandIndex)
{
    render_command_header* Header = &Commands->CommandHeaders[CommandIndex];
    
    void* Result = Header->OffsetToCommand + (u8*)Commands->CommandsBuffer.Block->Base;
    
    return(Result);
}
#define GetRenderCommand(commands, index, struct_type) (struct_type*)GetRenderCommand_(commands, index)

INTERNAL_FUNCTION inline int PushGlyphTransform(glyph_buffer* GlyphBuffer, m44* Transform)
{
    int Index = GlyphBuffer->GlyphTransformsCount++;
    
    GlyphBuffer->GlyphTransforms[Index] = *Transform;
    
    return(Index);
}

INTERNAL_FUNCTION void PushDefaultMatricesToGlyphBuffer(render_params* Params, glyph_buffer* GlyphBuffer)
{
    m44 Id = Identity();
    GlyphBuffer->GlyphDefaultProjectionMatrixIndex = PushGlyphTransform(GlyphBuffer, &Id);
    
    // NOTE(Dima): Pushing screen orthographic projection matrix
    GlyphBuffer->GlyphOrthoMatrixIndex = PushGlyphTransform(GlyphBuffer, Params->ScreenOrthoProjection);
    
    // NOTE(Dima): Pushing screen perspective projection matrix
    GlyphBuffer->GlyphPerspMatrixIndex = PushGlyphTransform(GlyphBuffer, Params->ViewProjection);
}

INTERNAL_FUNCTION void ResetGlyphBuffer(glyph_buffer* GlyphBuffer)
{
    GlyphBuffer->GlyphCount = 0;
    GlyphBuffer->GlyphTransformsCount = 0;
}

INTERNAL_FUNCTION void BeginRender(render_params* Params)
{
    render_commands* Commands = Params->Commands;
    
    PushDefaultMatricesToGlyphBuffer(Params, &Commands->Glyphs2D);
    PushDefaultMatricesToGlyphBuffer(Params, &Commands->Glyphs3D);
}

INTERNAL_FUNCTION void EndRender(render_params* Params)
{
    render_commands* Commands = Params->Commands;
    
    FreeArena(&Commands->CommandsBuffer, true);
    
    Commands->CommandCount = 0;
    
    ResetGlyphBuffer(&Commands->Glyphs2D);
    ResetGlyphBuffer(&Commands->Glyphs3D);
}