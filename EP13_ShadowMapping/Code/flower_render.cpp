#include "flower_lighting.cpp"
#include "flower_postprocess.cpp"

inline void* PushRenderCommand_(u32 CommandType, u32 SizeOfCommandStruct)
{
    render_commands* Commands = Global_RenderCommands;
    
    Assert(Commands->CommandCount < MAX_RENDER_COMMANDS_COUNT);
    render_command_header* Head = &Commands->CommandHeaders[Commands->CommandCount++];
    
    Head->CommandType = CommandType;
    
    void* CommandData = AllocateFromArena(&Commands->CommandsBuffer, SizeOfCommandStruct, 8);
    Head->CommandData = CommandData;
    
    return(CommandData);
}

#define PushRenderCommand(type, struct_type) (struct_type*)PushRenderCommand_(type, sizeof(struct_type))

inline void PushClear(v3 Color, u32 Flags = RenderClear_Color | RenderClear_Depth)
{
    Global_RenderCommands->ClearCommand.Set = true;
    
    Global_RenderCommands->ClearCommand.C = Color;
    Global_RenderCommands->ClearCommand.Flags = Flags;
}

inline void PushMesh(mesh* Mesh, 
                     material* Material, 
                     const m44& ModelToWorld = IdentityMatrix4(),
                     v3 C = V3(1.0f, 1.0f, 1.0f),
                     m44* SkinningMatrices = 0,
                     int SkinningMatricesCount = 0)
{
    render_command_mesh* Entry = PushRenderCommand(RenderCommand_Mesh, render_command_mesh);
    
    Entry->Mesh = Mesh;
    Entry->Material = Material;
    Entry->C = C;
    Entry->SkinningMatrices = SkinningMatrices;
    Entry->SkinningMatricesCount = SkinningMatricesCount;
    
    Entry->ModelToWorld = ModelToWorld;
}

inline void PushVoxelChunkMesh(voxel_mesh* Mesh,
                               v3 ChunkAt, 
                               culling_info CullingInfo = DefaultCullingInfo())
{
    render_command_voxel_mesh* Entry = PushRenderCommand(RenderCommand_VoxelChunkMesh, render_command_voxel_mesh);
    
    Entry->Mesh = Mesh;
    Entry->ChunkAt = ChunkAt;
    Entry->CullingInfo = CullingInfo;
}

INTERNAL_FUNCTION inline u32 GetMeshHash(mesh* Mesh)
{
    u64 MeshInt = (u64)Mesh;
    u32 Hash = (MeshInt * 479001599 + 993319);
    
    return(Hash);
}

INTERNAL_FUNCTION inline render_mesh_instance* FindMeshInstanceInTable(mesh* Mesh)
{
    u32 Hash = GetMeshHash(Mesh);
    
    int Index = Hash % RENDER_INSTANCE_TABLE_SIZE;
    
    render_mesh_instance* Result = 0;
    
    render_mesh_instance* At = Global_RenderCommands->InstanceTable[Index];
    while(At)
    {
        if(At->Mesh == Mesh)
        {
            Result = At;
            break;
        }
        
        At = At->NextInHash;
    }
    
    return(Result);
}

/*
NOTE
This funciton allocates table entries. In the end of the frame the memory arena (that was used
to allocate from) is freed. So there is no memory leaks!!!
*/
INTERNAL_FUNCTION render_mesh_instance* AddMeshInstanceToTable(mesh* Mesh, 
                                                               render_command_instanced_mesh* Command)
{
    u32 Hash = GetMeshHash(Mesh);
    int Index = Hash % RENDER_INSTANCE_TABLE_SIZE;
    
    render_mesh_instance* Result = 0;
    
    // NOTE(Dima): Checking if there is any element with the same mesh. If so - crash
    render_mesh_instance* At = Global_RenderCommands->InstanceTable[Index];
    while(At)
    {
        Assert(At->Mesh != Mesh);
        
        At = At->NextInHash;
    }
    
    Result = PushStruct(&Global_RenderCommands->CommandsBuffer, render_mesh_instance);
    
    Result->NextInHash = Global_RenderCommands->InstanceTable[Index];
    Global_RenderCommands->InstanceTable[Index] = Result;
    Result->Mesh = Mesh;
    Result->Command = Command;
    
    return(Result);
}

INTERNAL_FUNCTION void ResetMeshInstanceTable()
{
    for(int Index = 0;
        Index < RENDER_INSTANCE_TABLE_SIZE;
        Index++)
    {
        Global_RenderCommands->InstanceTable[Index] = 0;
    }
}

inline void PushInstanceMesh(int MaxInstanceCount,
                             mesh* Mesh,
                             material* Material,
                             const m44& ModelToWorld,
                             v3 C = V3(1.0f, 1.0f, 1.0f),
                             m44* SkinningMatrices = 0,
                             int SkinningMatricesCount = 0)
{
    render_mesh_instance* Instance = FindMeshInstanceInTable(Mesh);
    
    if(!Instance)
    {
        render_command_instanced_mesh* Entry = PushRenderCommand(RenderCommand_InstancedMesh, 
                                                                 render_command_instanced_mesh);
        
        Entry->Mesh = Mesh;
        Entry->Material = Material;
        Entry->C = C;
        
        Entry->InstanceMatrices = PushArray(&Global_RenderCommands->CommandsBuffer, 
                                            m44, MaxInstanceCount);
        Entry->NumSkinningMatricesPerInstance = SkinningMatricesCount;
        
        // NOTE(Dima): 256 is the maximum number of bones
        Entry->InstanceSkinningMatrices = 0;
        if(SkinningMatricesCount > 0)
        {
            Entry->InstanceSkinningMatrices = PushArray(&Global_RenderCommands->CommandsBuffer,
                                                        m44, MaxInstanceCount * SkinningMatricesCount);
        }
        
        Entry->MaxInstanceCount = MaxInstanceCount;
        Entry->InstanceCount = 0;
        
        Instance = AddMeshInstanceToTable(Mesh, Entry);
    }
    
    Assert(Instance);
    
    int InstanceIndex = Instance->Command->InstanceCount;
    Assert(InstanceIndex < Instance->Command->MaxInstanceCount);
    
    // NOTE(Dima): Save instance transform
    Instance->Command->InstanceMatrices[InstanceIndex] = ModelToWorld;
    
    // NOTE(Dima): Copy skinning matrices
    if(SkinningMatrices)
    {
        Assert(SkinningMatricesCount == Instance->Command->NumSkinningMatricesPerInstance);
        
        for(int SkinningMatrixIndex = 0;
            SkinningMatrixIndex < SkinningMatricesCount;
            SkinningMatrixIndex++)
        {
            int CurIndex = SkinningMatricesCount * InstanceIndex + SkinningMatrixIndex;
            
            Instance->Command->InstanceSkinningMatrices[CurIndex] = SkinningMatrices[SkinningMatrixIndex];
        }
    }
    
    // NOTE(Dima): Increasing instance count
    Instance->Command->InstanceCount++;
}

inline void PushSky(cubemap* Cubemap)
{
    Global_RenderCommands->Sky = Cubemap;
    Global_RenderCommands->SkyType = RenderSky_Skybox;
}

inline void PushSky(v3 Color)
{
    Global_RenderCommands->SkyColor = Color;
    Global_RenderCommands->SkyType = RenderSky_SolidColor;
}

inline void PushImage(image* Img, v2 P, f32 Height, v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    if(Global_RenderCommands->ImageFree.Next == &Global_RenderCommands->ImageFree)
    {
        int ToAddCount = 128;
        
        render_command_image* Pool = PushArray(Global_RenderCommands->Arena, 
                                               render_command_image, 
                                               ToAddCount);
        
        for(int i = 0; i < ToAddCount; i++)
        {
            DLIST_INSERT_BEFORE_SENTINEL(&Pool[i], 
                                         Global_RenderCommands->ImageFree, 
                                         Next, Prev);
        }
    }
    
    render_command_image* Entry = Global_RenderCommands->ImageFree.Next;;
    
    DLIST_REMOVE(Entry, Next, Prev);
    DLIST_INSERT_BEFORE_SENTINEL(Entry, 
                                 Global_RenderCommands->ImageUse,
                                 Next, Prev);
    
    Entry->Image = Img;
    Entry->P = P;
    Entry->C = PremultiplyAlpha(C);
    Entry->Dim = V2(Height * Img->WidthOverHeight, Height);
}

inline void PushCenteredImage(image* Img, v2 CenterP, f32 Height, v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    f32 Width = Height * Img->WidthOverHeight;
    
    v2 PushP = CenterP - V2(Width, Height) * 0.5f;
    
    PushImage(Img, PushP, Height, C);
}

INTERNAL_FUNCTION inline void PushRectInternal(rect_buffer* RectBuffer,
                                               rect_vertex Verts[4],
                                               u32 RectType,
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
    
    RectBuffer->Colors[RectBuffer->RectCount] = PackRGBA(PremultiplyAlpha(C));
    RectBuffer->Types[RectBuffer->RectCount] = RectType;
    
    RectBuffer->RectCount++;
}

INTERNAL_FUNCTION inline void PushTriangle2D(v2 Point0, 
                                             v2 Point1, 
                                             v2 Point2,
                                             v4 C = ColorWhite())
{
    rect_vertex Verts[4];
    Verts[0] = { Point0, V2(0.0f, 0.0f)};
    Verts[1] = { Point1, V2(0.0f, 0.0f)};
    Verts[2] = { Point2, V2(0.0f, 0.0f)};
    Verts[3] = { Point2, V2(0.0f, 0.0f)};
    
    rect_buffer* RectBuffer = &Global_RenderCommands->Rects2D;
    
    PushRectInternal(RectBuffer, Verts, Rect_Solid, C);
}

INTERNAL_FUNCTION inline void PushQuadrilateral2D(v2 Point0, 
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
    
    rect_buffer* RectBuffer = &Global_RenderCommands->Rects2D;
    
    PushRectInternal(RectBuffer, Verts, Rect_Solid, C);
}

INTERNAL_FUNCTION inline void PushCircleInternal2D(v2 P, 
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
        
        PushQuadrilateral2D( CurOuter,
                            NextOuter, 
                            NextInner,
                            CurInner,
                            C);
    }
}

INTERNAL_FUNCTION inline void PushCircle2D(v2 P, f32 Radius,
                                           v4 C = ColorWhite(),
                                           int Segments = 24)
{
    PushCircleInternal2D(P, 0.0f,
                         Radius,
                         C,
                         Segments);
}

INTERNAL_FUNCTION inline void PushCircleOutline2D(v2 P, 
                                                  f32 CircleRadius,
                                                  f32 OutlineThickness,
                                                  v4 C = ColorWhite(),
                                                  int Segments = 24)
{
    PushCircleInternal2D(P,
                         CircleRadius,
                         CircleRadius + OutlineThickness,
                         C,
                         Segments);
}

INTERNAL_FUNCTION inline void PushOutlinedCircle2D(v2 P, f32 Radius,
                                                   f32 OutlineThickness,
                                                   v4 CircleC = ColorWhite(),
                                                   v4 OutlineC = ColorGray(0.05f),
                                                   int SegmentsCount = 16)
{
    PushCircle2D(P, Radius,
                 CircleC, SegmentsCount);
    
    PushCircleOutline2D(P, Radius,
                        OutlineThickness,
                        OutlineC,
                        SegmentsCount);
}

INTERNAL_FUNCTION inline void PushRect(rc2 Rect,
                                       v4 C = ColorWhite())
{
    v2 Point1 = Rect.Min;
    v2 Point2 = V2(Rect.Max.x, Rect.Min.y);
    v2 Point3 = Rect.Max;
    v2 Point4 = V2(Rect.Min.x, Rect.Max.y);
    
    PushQuadrilateral2D(Point1,
                        Point2,
                        Point3,
                        Point4,
                        C);
}

INTERNAL_FUNCTION inline void PushFullscreenRect(v4 C = ColorWhite())
{
    rc2 Rect = RectMinDim(V2(0.0f, 0.0f), 
                          V2(Global_RenderCommands->WindowDimensions.Width,
                             Global_RenderCommands->WindowDimensions.Height));
    
    PushRect(Rect, C);
}

INTERNAL_FUNCTION inline void PushRectOutline(rc2 Rect,
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
        PushRect(Lines[RectIndex], C);
    }
}

INTERNAL_FUNCTION inline void PushOutlinedRect(rc2 Rect,
                                               f32 OutlineThickness,
                                               v4 RectColor = ColorWhite(),
                                               v4 OutlineColor = ColorBlack())
{
    PushRect(Rect, RectColor);
    
    PushRectOutline(Rect, OutlineThickness, OutlineColor);
}

INTERNAL_FUNCTION inline void PushLineInternal2D(v2 Begin,
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
        
        rect_buffer* RectBuffer = &Global_RenderCommands->Rects2D;
        
        PushRectInternal(RectBuffer, Verts, Rect_Solid, C);
    }
    
    v2 ArrowOffset = Thickness * 0.5 * ArrowThickness * PerpVector;
    
    // NOTE(Dima): Pushing end arrow triangle
    if(EndIsArrow)
    {
        v2 Corner1 = End;
        v2 Corner2 = LineEnd + ArrowOffset;
        v2 Corner3 = LineEnd - ArrowOffset;
        
        PushTriangle2D(Corner1,
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
        
        PushTriangle2D(Corner1,
                       Corner2,
                       Corner3,
                       C);
    }
}

INTERNAL_FUNCTION inline void PushLine2D(v2 Begin,
                                         v2 End,
                                         f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                         v4 C = ColorWhite())
{
    PushLineInternal2D(Begin,
                       End,
                       Thickness, C);
}

INTERNAL_FUNCTION inline void PushRoundLine2D(v2 Begin,
                                              v2 End,
                                              f32 Radius,
                                              v4 C = ColorWhite())
{
    PushLineInternal2D(Begin,
                       End,
                       Radius * 2.0f, 
                       C);
    
    PushCircle2D(Begin, 
                 Radius, C,
                 12);
    
    PushCircle2D(End, 
                 Radius, C,
                 12);
}

INTERNAL_FUNCTION inline void PushDashedLine2D(v2 Begin,
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
        PushLineInternal2D(CurLineStart, 
                           CurLineEnd,
                           Thickness,
                           C);
        
        // NOTE(Dima): Increasing current offset
        At += CurDashLen + SpaceLen;
    }
}

INTERNAL_FUNCTION inline void PushArrow2D(v2 Begin,
                                          v2 End,
                                          f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                          v4 C = ColorWhite())
{
    PushLineInternal2D(Begin,
                       End,
                       Thickness, C, 
                       true, false);
    
}

// TODO(Dima): Perfomance critical function. Maybe think about how can we optimize it even more
inline void PushGlyph(rect_buffer* RectBuffer,
                      glyph* Glyph, 
                      v2 P, f32 Height, 
                      int StyleIndex,
                      v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    glyph_style* Style = &Glyph->Styles[StyleIndex];
    v2 Dim = V2(Height * Style->WidthOverHeight, Height);
    
    v2 MinUV = Style->MinUV;
    v2 MaxUV = Style->MaxUV;
    
    rect_vertex Verts[4];
    Verts[0] = { V2(P.x, P.y), V2(MinUV.x, MinUV.y)};
    Verts[1] = { V2(P.x + Dim.x, P.y), V2(MaxUV.x, MinUV.y)};
    Verts[2] = { V2(P.x + Dim.x, P.y + Dim.y), V2(MaxUV.x, MaxUV.y)};
    Verts[3] = { V2(P.x, P.y + Dim.y), V2(MinUV.x, MaxUV.y)};
    
    PushRectInternal(RectBuffer, Verts, Rect_Textured, C);
}

INTERNAL_FUNCTION void ResetRectBuffer(rect_buffer* RectBuffer)
{
    RectBuffer->RectCount = 0;
}

INTERNAL_FUNCTION render_api_dealloc_entry* AllocateDeallocEntry()
{
    BeginTicketMutex(&Global_RenderCommands->DeallocEntriesMutex);
    
    if(Global_RenderCommands->FreeDealloc.Next == &Global_RenderCommands->FreeDealloc)
    {
        int Count = 64;
        render_api_dealloc_entry* DeallocEntries = PushArray(Global_RenderCommands->Arena,
                                                             render_api_dealloc_entry,
                                                             Count);
        
        for(int i = 0; i < Count; i++)
        {
            render_api_dealloc_entry* Entry = DeallocEntries + i;
            
            DLIST_INSERT_BEFORE_SENTINEL(Entry, Global_RenderCommands->FreeDealloc, Next, Prev);
        }
    }
    
    render_api_dealloc_entry* Result = Global_RenderCommands->FreeDealloc.Next;
    
    DLIST_REMOVE(Result, Next, Prev);
    DLIST_INSERT_BEFORE_SENTINEL(Result, Global_RenderCommands->UseDealloc, Next, Prev);
    
    EndTicketMutex(&Global_RenderCommands->DeallocEntriesMutex);
    
    return(Result);
}

INTERNAL_FUNCTION void DeallocateDeallocEntry(render_api_dealloc_entry* Entry)
{
    BeginTicketMutex(&Global_RenderCommands->DeallocEntriesMutex);
    
    DLIST_REMOVE(Entry, Next, Prev);
    DLIST_INSERT_BEFORE_SENTINEL(Entry, Global_RenderCommands->FreeDealloc, Next, Prev);
    
    EndTicketMutex(&Global_RenderCommands->DeallocEntriesMutex);
}

INTERNAL_FUNCTION inline render_pass* AddRenderPass(b32 IsShadowPass = false)
{
    Assert(Global_RenderCommands->RenderPassCount < ARC(Global_RenderCommands->RenderPasses));
    render_pass* Result = &Global_RenderCommands->RenderPasses[Global_RenderCommands->RenderPassCount++];
    
    Result->ClippingPlaneIsSet = false;
    Result->IsShadowPass = IsShadowPass;
    
    return(Result);
}

INTERNAL_FUNCTION void CalculateFrustumPlanes(render_pass* RenderPass)
{
    v4* FrustumPlanes = RenderPass->FrustumPlanes;
    const m44& ViewProj = RenderPass->ViewProjection;
    
    //NOTE(dima): Left plane
    FrustumPlanes[0].A = ViewProj.e[3] + ViewProj.e[0];
    FrustumPlanes[0].B = ViewProj.e[7] + ViewProj.e[4];
    FrustumPlanes[0].C = ViewProj.e[11] + ViewProj.e[8];
    FrustumPlanes[0].D = ViewProj.e[15] + ViewProj.e[12];
    
    //NOTE(dima): Right plane
    FrustumPlanes[1].A = ViewProj.e[3] - ViewProj.e[0];
    FrustumPlanes[1].B = ViewProj.e[7] - ViewProj.e[4];
    FrustumPlanes[1].C = ViewProj.e[11] - ViewProj.e[8];
    FrustumPlanes[1].D = ViewProj.e[15] - ViewProj.e[12];
    
    //NOTE(dima): Bottom plane
    FrustumPlanes[2].A = ViewProj.e[3] + ViewProj.e[1];
    FrustumPlanes[2].B = ViewProj.e[7] + ViewProj.e[5];
    FrustumPlanes[2].C = ViewProj.e[11] + ViewProj.e[9];
    FrustumPlanes[2].D = ViewProj.e[15] + ViewProj.e[13];
    
    //NOTE(dima): Top plane
    FrustumPlanes[3].A = ViewProj.e[3] - ViewProj.e[1];
    FrustumPlanes[3].B = ViewProj.e[7] - ViewProj.e[5];
    FrustumPlanes[3].C = ViewProj.e[11] - ViewProj.e[9];
    FrustumPlanes[3].D = ViewProj.e[15] - ViewProj.e[13];
    
    //NOTE(dima): Near plane
    FrustumPlanes[4].A = ViewProj.e[3] + ViewProj.e[2];
    FrustumPlanes[4].B = ViewProj.e[7] + ViewProj.e[6];
    FrustumPlanes[4].C = ViewProj.e[11] + ViewProj.e[10];
    FrustumPlanes[4].D = ViewProj.e[15] + ViewProj.e[14];
    
    //NOTE(dima): Far plane
    FrustumPlanes[5].A = ViewProj.e[3] - ViewProj.e[2];
    FrustumPlanes[5].B = ViewProj.e[7] - ViewProj.e[6];
    FrustumPlanes[5].C = ViewProj.e[11] - ViewProj.e[10];
    FrustumPlanes[5].D = ViewProj.e[15] - ViewProj.e[14];
    
    // NOTE(Dima): Normalizing planes
    for (int PlaneIndex = 0;
         PlaneIndex < 6;
         PlaneIndex++)
    {
        FrustumPlanes[PlaneIndex] = NormalizePlane(FrustumPlanes[PlaneIndex]);
    }
}

INTERNAL_FUNCTION void SetPerspectivePassData(render_pass* RenderPass,
                                              v3 CameraP,
                                              const m44& View,
                                              f32 Width, f32 Height,
                                              f32 Far, f32 Near,
                                              f32 FOVDegrees = 45.0f)
{
    RenderPass->CameraP = CameraP;
    RenderPass->View = View;
    RenderPass->Projection = PerspectiveProjection(Width, Height,
                                                   Far, Near,
                                                   FOVDegrees);
    RenderPass->ViewProjection = RenderPass->View * RenderPass->Projection;
    
    m44 InvView = InverseMatrix4(View);
    RenderPass->CameraLeft = InvView.Rows[0].xyz;
    RenderPass->CameraUp = InvView.Rows[1].xyz;
    RenderPass->CameraFront = InvView.Rows[2].xyz;
    
    RenderPass->Width = Width;
    RenderPass->Height = Height;
    RenderPass->Far = Far;
    RenderPass->Near = Near;
    RenderPass->FOVDegrees = FOVDegrees;
    
    CalculateFrustumPlanes(RenderPass);
}

INTERNAL_FUNCTION inline void SetClippingPlane(render_pass* RenderPass, v4 ClippingPlane)
{
    RenderPass->ClippingPlaneIsSet = true;
    RenderPass->ClippingPlane = ClippingPlane;
}

INTERNAL_FUNCTION void SetOrthographicPassData(render_pass* RenderPass,
                                               v3 CameraP,
                                               const m44& View,
                                               f32 Far, f32 Near,
                                               f32 RadiusW,
                                               f32 RadiusH)
{
    RenderPass->CameraP = CameraP;
    RenderPass->View = View;
    RenderPass->Projection = OrthographicProjection(RadiusW, RadiusH,
                                                    Far, Near);
    RenderPass->ViewProjection = RenderPass->View * RenderPass->Projection;
    
    RenderPass->Far = Far;
    RenderPass->Near = Near;
}


INTERNAL_FUNCTION void UpdateShadowCascades(render_pass* MainRenderPass)
{
    int CascadesCount;
    shadow_cascade_info* Cascades = GetDirLitCascades(&Global_RenderCommands->Lighting, 
                                                      MainRenderPass, 
                                                      &CascadesCount);
    
    for(int CascadeIndex = 0;
        CascadeIndex < CascadesCount;
        CascadeIndex++)
    {
        shadow_cascade_info* Cascade = &Global_RenderCommands->Lighting.Cascades[CascadeIndex];
        
        Cascade->RenderPass = AddRenderPass(true);
        
        SetOrthographicPassData(Cascade->RenderPass,
                                Cascade->P, Cascade->View,
                                Cascade->Far, Cascade->Near,
                                Cascade->ViewRadiusW,
                                Cascade->ViewRadiusH);
    }
    
}

INTERNAL_FUNCTION inline render_water_params DefaultWaterParams()
{
    render_water_params Result = {};
    
    Result.Height = 0.0f;
    Result.Color = ColorFromHex("#D4F1F9");
    
    return(Result);
}

INTERNAL_FUNCTION void  PushWater(render_water_params Params,
                                  render_pass* MainRenderPass)
{
    render_water* Water = &Global_RenderCommands->Water;
    Global_RenderCommands->WaterIsSet = true;
    
    v4 ReflectionPlane = V4(V3_Up(), -Params.Height);
    
    Water->Params = Params;
    Water->PlaneEquation = ReflectionPlane;
    
    // NOTE(Dima): SEtting refraction pass
    Water->RefractionPass = AddRenderPass();
    SetPerspectivePassData(Water->RefractionPass, 
                           MainRenderPass->CameraP,
                           MainRenderPass->View,
                           MainRenderPass->Width,
                           MainRenderPass->Height,
                           MainRenderPass->Far,
                           MainRenderPass->Near,
                           MainRenderPass->FOVDegrees);
    SetClippingPlane(Water->RefractionPass, ReflectionPlane);
    
    // NOTE(Dima): Setting reflection pass
    v3 OppositeCameraP = MainRenderPass->CameraP;
    v3 OppositeCameraForward = V3(MainRenderPass->View.e[2],
                                  MainRenderPass->View.e[6],
                                  MainRenderPass->View.e[10]);
    
    // NOTE(Dima): Reflecting cameras position
    v3 PlaneOrigin = ReflectionPlane.ABC * (-ReflectionPlane.D);
    v3 OffsetToCameraInit = OppositeCameraP - PlaneOrigin;
    v3 ReflectedOffset = Reflect(OffsetToCameraInit, ReflectionPlane.ABC);
    OppositeCameraP = PlaneOrigin + ReflectedOffset;
    
    // NOTE(Dima): Reflecting cameras forward
    OppositeCameraForward = Reflect(OppositeCameraForward, 
                                    ReflectionPlane.ABC);
    
    m44 OppositeView = LookAt(OppositeCameraP,
                              OppositeCameraP + OppositeCameraForward,
                              V3_Down(),
                              true);
    
    Water->ReflectionPass = AddRenderPass();
    SetPerspectivePassData(Water->ReflectionPass, 
                           OppositeCameraP,
                           OppositeView,
                           MainRenderPass->Width,
                           MainRenderPass->Height,
                           MainRenderPass->Far,
                           MainRenderPass->Near,
                           MainRenderPass->FOVDegrees);
    SetClippingPlane(Water->ReflectionPass, ReflectionPlane);
}

INTERNAL_FUNCTION void RenderPushDeallocateHandle(renderer_handle* Handle)
{
    render_api_dealloc_entry* Entry = AllocateDeallocEntry();
    
    Entry->Handle = Handle;
}

INTERNAL_FUNCTION void BeginRender(window_dimensions WindowDimensions,
                                   f32 Time)
{
    render_commands* Commands = Global_RenderCommands;
    
    Commands->RenderPassCount = 0;
    Commands->WindowDimensions = WindowDimensions;
    Commands->Time = Time;
    
    // NOTE(Dima): Init sky
    Commands->SkyColor = Commands->DefaultSkyColor;
    Commands->SkyType = Commands->DefaultSkyType;
    Commands->Sky = 0;
    
    // NOTE(Dima): Init other things
    Commands->WaterIsSet = false;
    Commands->Water = {};
    
    // NOTE(Dima): Resetting mesh instance table
    ResetMeshInstanceTable();
}

INTERNAL_FUNCTION void PreRender()
{
    
}

INTERNAL_FUNCTION void EndRender()
{
    render_commands* Commands = Global_RenderCommands;
    
    FreeArena(&Commands->CommandsBuffer, true);
    Commands->CommandCount = 0;
    
    ResetRectBuffer(&Commands->Rects2D);
    Commands->Sky = 0;
    Commands->ClearCommand.Set = false;
    
    DLIST_REMOVE_ENTIRE_LIST(&Commands->ImageUse, &Commands->ImageFree, Next, Prev);
}

INTERNAL_FUNCTION void InitRender(memory_arena* Arena, window_dimensions Dimensions)
{
    Global_RenderCommands = PushStruct(Arena, render_commands);
    
    Global_RenderCommands->Arena = Arena;
    Global_RenderCommands->WindowDimensions = Dimensions;
    
    // NOTE(Dima): Init dealloc list
    InitTicketMutex(&Global_RenderCommands->DeallocEntriesMutex);
    DLIST_REFLECT_PTRS(Global_RenderCommands->UseDealloc, Next, Prev);
    DLIST_REFLECT_PTRS(Global_RenderCommands->FreeDealloc, Next, Prev);
    
    // NOTE(Dima): Init images list
    DLIST_REFLECT_PTRS(Global_RenderCommands->ImageUse, Next, Prev);
    DLIST_REFLECT_PTRS(Global_RenderCommands->ImageFree, Next, Prev);
    
    InitLighting(&Global_RenderCommands->Lighting, Arena);
    InitPostprocessing(&Global_RenderCommands->PostProcessing);
    
    Global_RenderCommands->DefaultSkyColor = V3(0.1f, 0.7f, 0.8f);
    Global_RenderCommands->DefaultSkyType = RenderSky_SolidColor;
}