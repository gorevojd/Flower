inline void* PushRenderCommand_(render_commands* Commands, u32 CommandType, u32 SizeOfCommandStruct)
{
    render_command_header* Head = &Commands->CommandHeaders[Commands->CommandCount++];
    
    Head->CommandType = CommandType;
    Head->OffsetToCommand = GetUsedInCurrentBlock(&Commands->CommandsBuffer);
    
    void* CommandData = AllocateFromArena(&Commands->CommandsBuffer, SizeOfCommandStruct, 8);
    
    return(CommandData);
}

#define PushRenderCommand(commands, type, struct_type) (struct_type*)PushRenderCommand_(commands, type, sizeof(struct_type))

inline void PushMesh(render_commands* Commands, 
                     mesh* Mesh, 
                     material* Material, 
                     const m44& ModelToWorld,
                     v3 C = V3(1.0f, 1.0f, 1.0f),
                     m44* SkinningMatrices = 0,
                     int SkinningMatricesCount = 0)
{
    render_command_mesh* Entry = PushRenderCommand(Commands, RenderCommand_Mesh, render_command_mesh);
    
    Entry->Material = Material;
    Entry->Mesh = Mesh;
    Entry->ModelToWorld = ModelToWorld;
    Entry->C = C;
    Entry->SkinningMatrices = SkinningMatrices;
    Entry->SkinningMatricesCount = SkinningMatricesCount;
}

inline void PushImage(render_commands* Commands, image* Img, v2 P, f32 Height, v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    render_command_image* Entry = PushRenderCommand(Commands, RenderCommand_Image, render_command_image);
    
    Entry->Image = Img;
    Entry->P = P;
    Entry->C = C;
    Entry->Dim = V2(Height * Img->WidthOverHeight, Height);
}

INTERNAL_FUNCTION inline int PushRectTransform(rect_buffer* RectBuffer, m44* Transform)
{
    int Index = RectBuffer->TransformsCount++;
    
    RectBuffer->Transforms[Index] = *Transform;
    
    return(Index);
}

INTERNAL_FUNCTION inline int GetCurrentRectTransformIndex(rect_buffer* RectBuffer)
{
    int Result = RectBuffer->TransformsCount - 1;
    
    /*
/// By default rect transform is pushed at BeginRender function so that result must 
/// never be less than 0. But I assert here just to make sure it's OK.
*/
    Assert(Result >= 0);
    
    return(Result);
}

INTERNAL_FUNCTION inline void PushRectInternal(rect_buffer* RectBuffer,
                                               rect_vertex Verts[4],
                                               u32 RectType,
                                               int IndexToTransformMatrix,
                                               v4 C = ColorWhite())
{
    Assert(RectBuffer->RectCount < MAX_RECTS_COUNT);
    
    int VertexAt = RectBuffer->RectCount * 4;
    
    rect_vertex* Vertices = &RectBuffer->Vertices[VertexAt];;
    Vertices[0] = Verts[0];
    Vertices[1] = Verts[1];
    Vertices[2] = Verts[2];
    Vertices[3] = Verts[3];
    
    u32* Indices = &RectBuffer->Indices[RectBuffer->RectCount * 6];
    Indices[0] = VertexAt + 0;
    Indices[1] = VertexAt + 1;
    Indices[2] = VertexAt + 2;
    Indices[3] = VertexAt + 0;
    Indices[4] = VertexAt + 2;
    Indices[5] = VertexAt + 3;
    
    RectBuffer->Colors[RectBuffer->RectCount] = PackRGBA(C);
    RectBuffer->Types[RectBuffer->RectCount] = RectType;
    RectBuffer->IndicesToTransforms[RectBuffer->RectCount] = IndexToTransformMatrix;
    
    RectBuffer->RectCount++;
}

INTERNAL_FUNCTION inline void PushTriangle2D(render_commands* Commands,
                                             v2 Point0, 
                                             v2 Point1, 
                                             v2 Point2,
                                             v4 C = ColorWhite())
{
    rect_vertex Verts[4];
    Verts[0] = { Point0, V2(0.0f, 0.0f)};
    Verts[1] = { Point1, V2(0.0f, 0.0f)};
    Verts[2] = { Point2, V2(0.0f, 0.0f)};
    Verts[3] = { Point2, V2(0.0f, 0.0f)};
    
    rect_buffer* RectBuffer = &Commands->Rects2D;
    int ModelTransformMatrixIndex = RectBuffer->IdentityMatrixIndex;;
    
    PushRectInternal(RectBuffer, Verts, Rect_Solid, ModelTransformMatrixIndex, C);
}

INTERNAL_FUNCTION inline void PushQuadrilateral2D(render_commands* Commands,
                                                  v2 Point0, 
                                                  v2 Point1, 
                                                  v2 Point2,
                                                  v2 Point3,
                                                  v4 C = ColorWhite())
{
    rect_vertex Verts[4];
    Verts[0] = { Point0, V2(0.0f, 0.0f)};
    Verts[1] = { Point1, V2(0.0f, 0.0f)};
    Verts[2] = { Point2, V2(0.0f, 0.0f)};
    Verts[3] = { Point3, V2(0.0f, 0.0f)};
    
    rect_buffer* RectBuffer = &Commands->Rects2D;
    int ModelTransformMatrixIndex = RectBuffer->IdentityMatrixIndex;;
    
    PushRectInternal(RectBuffer, Verts, Rect_Solid, ModelTransformMatrixIndex, C);
}

INTERNAL_FUNCTION inline void PushCircleInternal2D(render_commands* Commands, 
                                                   v2 P, 
                                                   f32 InnerRadius,
                                                   f32 OuterRadius,
                                                   v4 C = ColorWhite(),
                                                   int Segments = 24)
{
    f32 OneSegmentAngle = 2.0f * F_PI / (f32)Segments;
    
    for(int SegmentIndex = 0;
        SegmentIndex < Segments;
        SegmentIndex++)
    {
        f32 CurAngle = (f32)SegmentIndex * OneSegmentAngle;
        f32 NextAngle = (f32)(SegmentIndex + 1) * OneSegmentAngle;
        
        v2 NormCurrent = V2(Cos(CurAngle), Sin(CurAngle));
        v2 NormNext = V2(Cos(NextAngle), Sin(NextAngle));
        
        v2 CurOuter = P + NormCurrent * OuterRadius;
        v2 NextOuter = P + NormNext * OuterRadius;
        v2 NextInner = P + NormNext * InnerRadius;
        v2 CurInner = P + NormCurrent * InnerRadius;
        
        PushQuadrilateral2D(Commands, 
                            CurOuter,
                            NextOuter, 
                            NextInner,
                            CurInner,
                            C);
    }
}

INTERNAL_FUNCTION inline void PushCircle2D(render_commands* Commands, 
                                           v2 P, f32 Radius,
                                           v4 C = ColorWhite(),
                                           int Segments = 24)
{
    PushCircleInternal2D(Commands,
                         P, 0.0f,
                         Radius,
                         C,
                         Segments);
}

INTERNAL_FUNCTION inline void PushCircleOutline2D(render_commands* Commands, 
                                                  v2 P, 
                                                  f32 CircleRadius,
                                                  f32 OutlineThickness,
                                                  v4 C = ColorWhite(),
                                                  int Segments = 24)
{
    PushCircleInternal2D(Commands,
                         P,
                         CircleRadius,
                         CircleRadius + OutlineThickness,
                         C,
                         Segments);
}

INTERNAL_FUNCTION inline void PushOutlinedCircle2D(render_commands* Commands,
                                                   v2 P, f32 Radius,
                                                   f32 OutlineThickness,
                                                   v4 CircleC = ColorWhite(),
                                                   v4 OutlineC = ColorGray(0.05f))
{
    PushCircle2D(Commands,
                 P, Radius,
                 CircleC, 16);
    
    PushCircleOutline2D(Commands,
                        P, Radius,
                        OutlineThickness,
                        OutlineC,
                        16);
}

INTERNAL_FUNCTION inline void PushRect(render_commands* Commands,
                                       rc2 Rect,
                                       v4 C = ColorWhite())
{
    v2 Point1 = Rect.Min;
    v2 Point2 = V2(Rect.Max.x, Rect.Min.y);
    v2 Point3 = Rect.Max;
    v2 Point4 = V2(Rect.Min.x, Rect.Max.y);
    
    PushQuadrilateral2D(Commands,
                        Point1,
                        Point2,
                        Point3,
                        Point4,
                        C);
}

INTERNAL_FUNCTION inline void PushRectOutline(render_commands* Commands,
                                              rc2 Rect,
                                              f32 Thickness,
                                              v4 C = ColorBlack())
{
    v2 Dim = GetDim(Rect);
    
    v2 HorzLineDim = V2(Dim.x + 2.0f * Thickness, Thickness);
    v2 VertLineDim = V2(Thickness, Dim.y);
    
    rc2 Lines[4];
    Lines[0] = RectMinDim(Rect.Min - V2(Thickness), HorzLineDim);
    Lines[1] = RectMinDim(V2(Rect.Min.x - Thickness, Rect.Max.y), HorzLineDim);
    Lines[2] = RectMinDim(V2(Rect.Min.x - Thickness, Rect.Min.y), VertLineDim);
    Lines[3] = RectMinDim(V2(Rect.Max.x, Rect.Min.y), VertLineDim);
    
    for(int RectIndex = 0;
        RectIndex < 4;
        RectIndex++)
    {
        PushRect(Commands, Lines[RectIndex], C);
    }
}

INTERNAL_FUNCTION inline void PushOutlinedRect(render_commands* Commands,
                                               rc2 Rect,
                                               f32 OutlineThickness,
                                               v4 RectColor = ColorWhite(),
                                               v4 OutlineColor = ColorBlack())
{
    PushRect(Commands, Rect, RectColor);
    
    PushRectOutline(Commands, Rect, OutlineThickness, OutlineColor);
}

INTERNAL_FUNCTION inline void PushLineInternal2D(render_commands* Commands,
                                                 v2 Begin,
                                                 v2 End,
                                                 f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                                 v4 C = ColorWhite(),
                                                 b32 EndIsArrow = false,
                                                 b32 BeginIsArrow = false)
{
    v2 Diff = End - Begin;
    v2 PerpVector = NOZ(V2(-Diff.y, Diff.x));
    v2 Offset = Thickness * 0.5f * PerpVector;
    
    f32 Mag = Magnitude(Diff);
    
    f32 ArrowLen = 6.0f * Thickness;
    f32 ArrowThickness = RENDER_DEFAULT_2D_LINE_THICKNESS;
    
    // NOTE(Dima): Calculate distance to line Begin and End
    f32 LineStopDist = Mag;
    if(EndIsArrow)
    {
        LineStopDist = Mag - ArrowLen;
    }
    
    f32 LineBeginDist = 0;
    if(BeginIsArrow)
    {
        LineBeginDist = ArrowLen;
    }
    
    // NOTE(Dima): Calculating line Begin and End
    v2 DiffNorm = Diff / Mag;
    v2 LineBegin = Begin + DiffNorm * LineBeginDist;
    v2 LineEnd = Begin + DiffNorm * LineStopDist;
    
    if(LineStopDist - LineBeginDist > 0.0f)
    {
        rect_vertex Verts[4];
        Verts[0] = { LineEnd - Offset, V2(0.0f, 0.0f)};
        Verts[1] = { LineEnd + Offset, V2(0.0f, 0.0f)};
        Verts[2] = { LineBegin + Offset, V2(0.0f, 0.0f)};
        Verts[3] = { LineBegin - Offset, V2(0.0f, 0.0f)};
        
        rect_buffer* RectBuffer = &Commands->Rects2D;
        int ModelTransformMatrixIndex = RectBuffer->IdentityMatrixIndex;
        
        PushRectInternal(RectBuffer, Verts, Rect_Solid, ModelTransformMatrixIndex, C);
    }
    
    v2 ArrowOffset = Thickness * 0.5 * ArrowThickness * PerpVector;
    
    // NOTE(Dima): Pushing end arrow triangle
    if(EndIsArrow)
    {
        v2 Corner1 = End;
        v2 Corner2 = LineEnd + ArrowOffset;
        v2 Corner3 = LineEnd - ArrowOffset;
        
        PushTriangle2D(Commands,
                       Corner1,
                       Corner2,
                       Corner3,
                       C);
    }
    
    // NOTE(Dima): Pushing Begin arrow triangle
    if(BeginIsArrow)
    {
        v2 Corner1 = Begin;
        v2 Corner2 = LineBegin + ArrowOffset;
        v2 Corner3 = LineBegin - ArrowOffset;
        
        PushTriangle2D(Commands,
                       Corner1,
                       Corner2,
                       Corner3,
                       C);
    }
}

INTERNAL_FUNCTION inline void PushLine2D(render_commands* Commands,
                                         v2 Begin,
                                         v2 End,
                                         f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                         v4 C = ColorWhite())
{
    PushLineInternal2D(Commands, 
                       Begin,
                       End,
                       Thickness, C);
}

INTERNAL_FUNCTION inline void PushRoundLine2D(render_commands* Commands,
                                              v2 Begin,
                                              v2 End,
                                              f32 Radius,
                                              v4 C = ColorWhite())
{
    PushLineInternal2D(Commands, 
                       Begin,
                       End,
                       Radius * 2.0f, 
                       C);
    
    PushCircle2D(Commands, Begin, 
                 Radius, C,
                 12);
    
    PushCircle2D(Commands, End, 
                 Radius, C,
                 12);
}

INTERNAL_FUNCTION inline void PushDashedLine2D(render_commands* Commands,
                                               v2 Begin,
                                               v2 End,
                                               f32 DashMaxLen = RENDER_DEFAULT_2D_LINE_DASH_LENGTH,
                                               f32 SpaceLen = RENDER_DEFAULT_2D_LINE_DASH_SPACING,
                                               f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                               v4 C = ColorWhite())
{
    f32 Dist = Length(Begin - End);
    
    // NOTE(Dima): Finding direction
    v2 OffsetNorm = V2(1.0f, 0.0f);
    if(Dist > 0.000001f)
    {
        OffsetNorm = (End - Begin) / Dist;
    }
    
    f32 At = 0.0f;
    
    while(At < Dist)
    {
        f32 CurDashLen = std::min(DashMaxLen, Dist - At);
        
        // NOTE(Dima): Calculating line points
        v2 CurLineStart = Begin + OffsetNorm * At;
        v2 CurLineEnd = CurLineStart + OffsetNorm * CurDashLen;
        
        // NOTE(Dima): Pushing line
        PushLineInternal2D(Commands, 
                           CurLineStart, 
                           CurLineEnd,
                           Thickness,
                           C);
        
        // NOTE(Dima): Increasing current offset
        At += CurDashLen + SpaceLen;
    }
}

INTERNAL_FUNCTION inline void PushArrow2D(render_commands* Commands,
                                          v2 Begin,
                                          v2 End,
                                          f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                          v4 C = ColorWhite())
{
    PushLineInternal2D(Commands, 
                       Begin,
                       End,
                       Thickness, C, 
                       true, false);
    
}

// TODO(Dima): Perfomance critical function. Maybe think about how can we optimize it even more
inline void PushGlyph(rect_buffer* RectBuffer,
                      glyph* Glyph, 
                      v2 P, f32 Height, 
                      int ModelTransformMatrixIndex,
                      v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    v2 Dim = V2(Height * Glyph->Image.WidthOverHeight, Height);
    
    v2 MinUV = Glyph->MinUV;
    v2 MaxUV = Glyph->MaxUV;
    
    rect_vertex Verts[4];
    Verts[0] = { V2(P.x, P.y), V2(MinUV.x, MinUV.y)};
    Verts[1] = { V2(P.x + Dim.x, P.y), V2(MaxUV.x, MinUV.y)};
    Verts[2] = { V2(P.x + Dim.x, P.y + Dim.y), V2(MaxUV.x, MaxUV.y)};
    Verts[3] = { V2(P.x, P.y + Dim.y), V2(MinUV.x, MaxUV.y)};
    
    PushRectInternal(RectBuffer, Verts, Rect_Textured, ModelTransformMatrixIndex, C);
}

inline void* GetRenderCommand_(render_commands* Commands, int CommandIndex)
{
    render_command_header* Header = &Commands->CommandHeaders[CommandIndex];
    
    void* Result = Header->OffsetToCommand + (u8*)Commands->CommandsBuffer.Block->Base;
    
    return(Result);
}
#define GetRenderCommand(commands, index, struct_type) (struct_type*)GetRenderCommand_(commands, index)

INTERNAL_FUNCTION void PushDefaultMatricesToRectBuffer(rect_buffer* RectBuffer)
{
    m44 Id = IdentityMatrix4();
    RectBuffer->IdentityMatrixIndex = PushRectTransform(RectBuffer, &Id);
    
    // NOTE(Dima): Pushing screen orthographic projection matrix
    RectBuffer->OrthoMatrixIndex = PushRectTransform(RectBuffer, &Global_RenderCommands->ScreenOrthoProjection);
    
    // NOTE(Dima): Pushing screen perspective projection matrix
    RectBuffer->ViewProjMatrixIndex = PushRectTransform(RectBuffer, &Global_RenderCommands->ViewProjection);
}

INTERNAL_FUNCTION void ResetRectBuffer(rect_buffer* RectBuffer)
{
    RectBuffer->RectCount = 0;
    RectBuffer->TransformsCount = 0;
}

INTERNAL_FUNCTION void BeginRender()
{
    render_commands* Commands = Global_RenderCommands;
    
    PushDefaultMatricesToRectBuffer(&Commands->Rects2D);
    PushDefaultMatricesToRectBuffer(&Commands->Rects3D);
}

INTERNAL_FUNCTION void EndRender()
{
    render_commands* Commands = Global_RenderCommands;
    
    FreeArena(&Commands->CommandsBuffer, true);
    
    Commands->CommandCount = 0;
    
    ResetRectBuffer(&Commands->Rects2D);
    ResetRectBuffer(&Commands->Rects3D);
}

INTERNAL_FUNCTION void InitRender(memory_arena* Arena)
{
    Global_RenderCommands = PushStruct(Arena, render_commands);
}