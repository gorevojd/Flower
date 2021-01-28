#include "flower_rubiks.h"

inline int GetCubieIndex(rubiks_cube* Cube, int x, int y, int z)
{
    int ResultIndex = z * Cube->Dim * Cube->Dim + y * Cube->Dim + x;
    
    return(ResultIndex);
}

inline int GetCurrentIndex(rubiks_cube* Cube, int x, int y, int z)
{
    int Index = GetCubieIndex(Cube, x, y, z);
    
    int Result = Cube->Current[Index];
    
    return(Result);
}

inline int GetVisibleCubie(rubiks_cube* Cube, int x, int y, int z)
{
    int ResultIndex = GetCubieIndex(Cube, x, y, z);
    
    int Result = Cube->CubiesToVisible[ResultIndex];
    
    return(Result);
}

INTERNAL_FUNCTION inline void PushStickerPolygon(helper_rubiks_mesh* Mesh,
                                                 v2 A, v2 B, v2 C, v2 D)
{
    
    // NOTE(Dima): Pushing positions
    int Base = Mesh->P.size();
    Mesh->P.push_back(A);
    Mesh->P.push_back(B);
    Mesh->P.push_back(C);
    Mesh->P.push_back(D);
    
    // NOTE(Dima): PUshing ids
    Mesh->Ids.push_back(Base + 0);
    Mesh->Ids.push_back(Base + 1);
    Mesh->Ids.push_back(Base + 2);
    
    Mesh->Ids.push_back(Base + 0);
    Mesh->Ids.push_back(Base + 2);
    Mesh->Ids.push_back(Base + 3);
}

INTERNAL_FUNCTION inline void RubiksPushCircleSegmentToPerimeter(std::vector<v2>& PerimeterPoints,
                                                                 v2 CornerP,
                                                                 int AngleStartIndex,
                                                                 const std::vector<v2> CirclePoints,
                                                                 int OneCornerSegments)
{
    int CirclePointsSize = CirclePoints.size();
    for(int AngleIndex = 0;
        AngleIndex <= OneCornerSegments;
        AngleIndex++)
    {
        v2 AnglePushPoint = CornerP + CirclePoints[(AngleIndex + AngleStartIndex) % CirclePointsSize];
        
        PerimeterPoints.push_back(AnglePushPoint);
    }
}

INTERNAL_FUNCTION inline void RubiksPushRoundTrapezoid(helper_rubiks_mesh* Result,
                                                       int OneCornerSegments,
                                                       int StartPoint)
{
    // NOTE(Dima): Pushing upper rounded trapezoid
    int TrapPointCount = 2 * (OneCornerSegments + 1);
    int BeginPoint = StartPoint;
    int EndPoint = StartPoint + TrapPointCount - 1;
    
    while((BeginPoint + 1) < (EndPoint - 1))
    {
        v2 A = Result->PerimeterPoints[BeginPoint];
        v2 B = Result->PerimeterPoints[BeginPoint + 1];
        v2 C = Result->PerimeterPoints[EndPoint - 1];
        v2 D = Result->PerimeterPoints[EndPoint];
        
        PushStickerPolygon(Result, A, B, C, D);
        
        BeginPoint++;
        EndPoint--;
    }
}

INTERNAL_FUNCTION helper_rubiks_mesh GetStickerMeshInternal(f32 CubieLen, 
                                                            f32 StickerPercentage,
                                                            b32 Rounded,
                                                            f32 Roundness)
{
    helper_rubiks_mesh Result = {};
    
    f32 S = StickerPercentage * CubieLen;
    f32 r = Roundness * S;
    
    f32 HalfS = S * 0.5f;
    if(Rounded)
    {
        
        f32 HalfSR = HalfS - r;
        
        // NOTE(Dima): Generating circle points
        int OneCornerSegments = 3;
        int SegmentsCount = OneCornerSegments * 4;
        
        std::vector<v2> CirclePoints;
        for(int SegmentIndex = 0; 
            SegmentIndex < SegmentsCount;
            SegmentIndex++)
        {
            f32 CurrentAngle = (f32)SegmentIndex * F_TWO_PI / (f32)SegmentsCount;
            
            v2 Direction = V2(Cos(CurrentAngle), Sin(CurrentAngle));
            
            CirclePoints.push_back(Direction * r);
        }
        
        // NOTE(Dima): Corner 1 perimeter
        RubiksPushCircleSegmentToPerimeter(Result.PerimeterPoints,
                                           V2(HalfSR, HalfSR), 
                                           0 * OneCornerSegments, 
                                           CirclePoints,
                                           OneCornerSegments);
        
        // NOTE(Dima): Corner 2 perimeter
        RubiksPushCircleSegmentToPerimeter(Result.PerimeterPoints,
                                           V2(-HalfSR, HalfSR), 
                                           1 * OneCornerSegments, 
                                           CirclePoints,
                                           OneCornerSegments);
        
        // NOTE(Dima): Corner 3 perimeter
        RubiksPushCircleSegmentToPerimeter(Result.PerimeterPoints,
                                           V2(-HalfSR, -HalfSR), 
                                           2 * OneCornerSegments, 
                                           CirclePoints,
                                           OneCornerSegments);
        
        // NOTE(Dima): Corner 4 perimeter
        RubiksPushCircleSegmentToPerimeter(Result.PerimeterPoints,
                                           V2(HalfSR, -HalfSR), 
                                           3 * OneCornerSegments, 
                                           CirclePoints,
                                           OneCornerSegments);
        
        // NOTE(Dima): Pushing main quad
#if 1    
        v2 Poi1 = V2(-HalfS, HalfSR);
        v2 Poi2 = V2(HalfS, HalfSR);
        v2 Poi3 = V2(HalfS, -HalfSR);
        v2 Poi4 = V2(-HalfS, -HalfSR);
        
        PushStickerPolygon(&Result, Poi1, Poi2, Poi3, Poi4);
#endif
        
        // NOTE(Dima): Pushing upper rounded trapezoid
        RubiksPushRoundTrapezoid(&Result, 
                                 OneCornerSegments,
                                 0);
        
        // NOTE(Dima): Pushing upper rounded trapezoid
        RubiksPushRoundTrapezoid(&Result, 
                                 OneCornerSegments,
                                 2 * (OneCornerSegments + 1));
    }
    else
    {
        v2 Poi1 = V2(-HalfS, HalfS);
        v2 Poi2 = V2(HalfS, HalfS);
        v2 Poi3 = V2(HalfS, -HalfS);
        v2 Poi4 = V2(-HalfS, -HalfS);
        
        PushStickerPolygon(&Result, Poi1, Poi2, Poi3, Poi4);
        
        Result.PerimeterPoints.push_back(Poi1);
        Result.PerimeterPoints.push_back(Poi2);
        Result.PerimeterPoints.push_back(Poi3);
        Result.PerimeterPoints.push_back(Poi4);
    }
    
    // NOTE(Dima): PUshing lower rounded trapezoid
    
    return(Result);
}

struct rubiks_is_outer_bool
{
    b32 Result[RubiksDirection_Count];
};

INTERNAL_FUNCTION inline rubiks_is_outer_bool RubiksIsOuterCubie(rubiks_cube* Cube, 
                                                                 int x, int y, int z)
{
    rubiks_is_outer_bool Result = {};
    
    if(x == 0)
    {
        Result.Result[RubiksDirection_Right] = true;
    }
    
    if(x == (Cube->Dim - 1))
    {
        Result.Result[RubiksDirection_Left] = true;
    }
    
    if(y == 0)
    {
        Result.Result[RubiksDirection_Down] = true;
    }
    
    if(y == (Cube->Dim - 1))
    {
        Result.Result[RubiksDirection_Up] = true;
    }
    
    if(z == 0)
    {
        Result.Result[RubiksDirection_Back] = true;
    }
    
    if(z == (Cube->Dim - 1))
    {
        Result.Result[RubiksDirection_Front] = true;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 RubiksIsOuter(rubiks_cube* Cube, 
                                           int x, int y, int z)
{
    rubiks_is_outer_bool Temp = RubiksIsOuterCubie(Cube, x, y, z);
    
    b32 Result = false;
    
    for(int DirectionIndex = 0;
        DirectionIndex < RubiksDirection_Count;
        DirectionIndex++)
    {
        if(Temp.Result[DirectionIndex])
        {
            Result = true;
            
            break;
        }
    }
    
    return(Result);
}

// NOTE(Dima): x - 0, y - 1, z - 2
INTERNAL_FUNCTION void GenerateInnerMesh(rubiks_cube* Cube, int Axis)
{
    Cube->InnerMeshIsGenerated[Axis] = false;
    
    if(Cube->Dim > 2)
    {
        f32 Dim = Cube->OneCubieLen * (f32)(Cube->Dim - 2);
        
        f32 HalfDim = Dim * 0.5f;
        f32 HalfCubie = Cube->OneCubieLen * 0.5f;
        
        v3 A, B, C, D;
        v3 AxisDir;
        
        switch(Axis)
        {
            case RubiksAxis_X:
            {
                A = V3(0.0f, HalfDim, -HalfDim);
                B = V3(0.0f, HalfDim, HalfDim);
                C = V3(0.0f, -HalfDim, HalfDim);
                D = V3(0.0f, -HalfDim, -HalfDim);
                
                AxisDir = V3_Left();
            }break;
            
            case RubiksAxis_Y:
            {
                A = V3(HalfDim, 0.0f, HalfDim);
                B = V3(-HalfDim, 0.0f, HalfDim);
                C = V3(-HalfDim, 0.0f, -HalfDim);
                D = V3(HalfDim, 0.0f, -HalfDim);
                
                AxisDir = V3_Up();
            }break;
            
            case RubiksAxis_Z:
            {
                A = V3(HalfDim, HalfDim, 0.0f);
                B = V3(-HalfDim, HalfDim, 0.0f);
                C = V3(-HalfDim, -HalfDim, 0.0f);
                D = V3(HalfDim, -HalfDim, 0.0f);
                
                AxisDir = V3_Forward();
            }break;
        }
        
        v3 AxisHalf = AxisDir * HalfCubie;
        
        helper_mesh InnerHelper = {};
        InnerHelper.Name = std::string("InnerHelper");
        
        PushFlatPolygon(InnerHelper, 
                        A + AxisHalf, 
                        B + AxisHalf, 
                        C + AxisHalf, 
                        D + AxisHalf,
                        AxisDir,
                        V3(0.0f));
        
        PushFlatPolygon(InnerHelper,
                        A - AxisHalf,
                        B - AxisHalf,
                        C - AxisHalf,
                        D - AxisHalf,
                        -AxisDir,
                        V3(0.0f));
        
        Cube->InnerMeshes[Axis] = MakeMesh(InnerHelper);
        Cube->InnerMeshIsGenerated[Axis] = true;
    }
}

INTERNAL_FUNCTION void RubiksGenerateInnerMeshes(rubiks_cube* Cube)
{
    for(int AxisIndex = 0;
        AxisIndex < RubiksAxis_Count;
        AxisIndex++)
    {
        GenerateInnerMesh(Cube, AxisIndex);
    }
}

INTERNAL_FUNCTION mesh GenerateCubieMesh(rubiks_cube* Cube,
                                         int x, int y, int z)
{
    helper_mesh HelperMesh = {};
    HelperMesh.Name = std::string("CubieMesh");
    
    v3 CubeColor = V3(0.035f);
    f32 SideLen = Cube->OneCubieLen;
    
    // NOTE(Dima): Forward side
    PushUnitCubeSide(HelperMesh,
                     0, 1, 2, 3, 
                     V3_Forward(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Left side
    PushUnitCubeSide(HelperMesh,
                     1, 5, 6, 2, 
                     V3_Left(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Back side
    PushUnitCubeSide(HelperMesh,
                     5, 4, 7, 6, 
                     V3_Back(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Right side
    PushUnitCubeSide(HelperMesh,
                     4, 0, 3, 7, 
                     V3_Right(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Up
    PushUnitCubeSide(HelperMesh,
                     4, 5, 1, 0, 
                     V3_Up(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Down
    PushUnitCubeSide(HelperMesh,
                     3, 2, 6, 7, 
                     V3_Down(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Generating sticker
    rubiks_is_outer_bool CubieIsOuter = RubiksIsOuterCubie(Cube, x, y, z);
    
    for(int DirectionIndex = 0;
        DirectionIndex < RubiksDirection_Count;
        DirectionIndex++)
    {
        b32 IsOuter = CubieIsOuter.Result[DirectionIndex];
        
        if(IsOuter)
        {
            // NOTE(Dima): Pushing sticker mesh to cubie
            HelperMesh = CombineHelperMeshes(&HelperMesh, &Cube->StickersMeshes[DirectionIndex]);
        }
    }
    
    // NOTE(Dima): Initializing mesh
    mesh Result = MakeMesh(HelperMesh);
    
    return(Result);
}

inline v3 RubiksPlaneTo3DPoint(rubiks_cube* Cube, v2 PlaneP, u32 Direction, b32 IsUpper)
{
    
    v3 Normal = RubiksDirection[Direction];
    f32 Offset = Cube->OneCubieLen * 0.5f;
    
    if(IsUpper)
    {
        Offset += Cube->StickerHeight;
    }
    
    v3 Result = Normal * Offset;
    
    switch(Direction)
    {
        case RubiksDirection_Down:
        case RubiksDirection_Up:
        {
            Result.x = PlaneP.x;
            Result.z = PlaneP.y;
        }break;
        
        case RubiksDirection_Left:
        case RubiksDirection_Right:
        {
            Result.y = PlaneP.x;
            Result.z = PlaneP.y;
        }break;
        
        case RubiksDirection_Back:
        case RubiksDirection_Front:
        {
            Result.x = PlaneP.x;
            Result.y = PlaneP.y;
        }break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void GenerateStickerMeshes(rubiks_cube* Cube, b32 RoundStickers)
{
    // NOTE(Dima): Generating sticker helper meshes
    Cube->HelperStickerUp = GetStickerMeshInternal(Cube->OneCubieLen,
                                                   0.88f, 
                                                   RoundStickers, 0.1f);
    Cube->HelperStickerDown = GetStickerMeshInternal(Cube->OneCubieLen,
                                                     0.91f, 
                                                     RoundStickers, 0.1f);
    
    // NOTE(Dima): Generating sticker meshes
    for(int DirectionIndex = 0;
        DirectionIndex < RubiksDirection_Count;
        DirectionIndex++)
    {
        helper_mesh& HelperMesh = Cube->StickersMeshes[DirectionIndex];
        
        HelperMesh.Name = std::string("StickerMesh");
        
        v3 StickerColor = RubiksColors[DirectionIndex].rgb;
        
        helper_rubiks_mesh* StickerUp = &Cube->HelperStickerUp;
        helper_rubiks_mesh* StickerDown = &Cube->HelperStickerDown;
        
        // NOTE(Dima): Generating sticker sides
        
        Assert(StickerUp->PerimeterPoints.size() == StickerDown->PerimeterPoints.size());
        
        int NumPerimP = StickerUp->PerimeterPoints.size();
        for(int PIndex = 0; 
            PIndex < NumPerimP; 
            PIndex++)
        {
            v3 CurTop = RubiksPlaneTo3DPoint(Cube, 
                                             StickerUp->PerimeterPoints[PIndex], 
                                             DirectionIndex, true);
            v3 NextTop = RubiksPlaneTo3DPoint(Cube, 
                                              StickerUp->PerimeterPoints[(PIndex + 1) % NumPerimP], 
                                              DirectionIndex, true);
            v3 CurBot = RubiksPlaneTo3DPoint(Cube, 
                                             StickerDown->PerimeterPoints[PIndex], 
                                             DirectionIndex, false);
            v3 NextBot = RubiksPlaneTo3DPoint(Cube, 
                                              StickerDown->PerimeterPoints[(PIndex + 1) % NumPerimP], 
                                              DirectionIndex, false);
            
            v3 Normal = NOZ(Cross(NextBot - CurTop, NextTop - CurTop));
            
            PushFlatPolygon(HelperMesh, 
                            CurTop, NextTop,
                            NextBot, CurBot,
                            Normal, StickerColor);
        }
        
        // NOTE(Dima): Generating sticker top
        std::vector<v3> Points;
        
        helper_mesh StickerUpperMesh = {};
        
        // NOTE(Dima): Converting plane to 3d points
        int NumP = StickerUp->P.size();
        for(int PIndex = 0;
            PIndex < NumP;
            PIndex++)
        {
            v3 CurP = RubiksPlaneTo3DPoint(Cube, 
                                           StickerUp->P[PIndex], 
                                           DirectionIndex, true);
            
            StickerUpperMesh.Vertices.push_back(CurP);
            StickerUpperMesh.Normals.push_back(RubiksDirection[DirectionIndex]);
            StickerUpperMesh.TexCoords.push_back(V2(0.0f, 0.0f));
            StickerUpperMesh.Colors.push_back(StickerColor);
        }
        
        // NOTE(Dima): Copying indices
        StickerUpperMesh.Indices = StickerUp->Ids;
        
        HelperMesh = CombineHelperMeshes(&HelperMesh, &StickerUpperMesh);
    }
}

INTERNAL_FUNCTION std::string OuterCombinationUniqueName(rubiks_is_outer_bool* Bool)
{
    char Buf[128];
    Buf[0] = 0;
    
    for(int DirectionIndex = 0;
        DirectionIndex < RubiksDirection_Count;
        DirectionIndex++)
    {
        if(Bool->Result[DirectionIndex])
        {
            AppendStringSafe(Buf, ArrayCount(Buf), (char*)RubiksDirectionName[DirectionIndex]);
        }
    }
    
    std::string UniqueName = std::string(Buf);
    
    return(UniqueName);
}

INTERNAL_FUNCTION void RubiksPrecomputeMeshes(rubiks_cube* Cube)
{
    // NOTE(Dima): precomputing
    std::map<std::string, int> UniqueMeshMap;
    
    Cube->UniqueMeshesCount = 0;
    
    for(int x = 0; x < Cube->Dim; x++)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int z = 0; z < Cube->Dim; z++)
            {
                int VisibleCubie = GetVisibleCubie(Cube, x, y, z);
                rubiks_is_outer_bool OuterBool = RubiksIsOuterCubie(Cube, x, y, z);
                
                std::string UniqueName = OuterCombinationUniqueName(&OuterBool);
                
                if(RubiksIsOuter(Cube, x, y, z))
                {
                    if(UniqueMeshMap.find(UniqueName) == UniqueMeshMap.end())
                    {
                        int MeshIndex = Cube->UniqueMeshesCount++;
                        
                        UniqueMeshMap.insert(std::pair<std::string, int>(UniqueName, MeshIndex));
                        
#if 0                        
                        NewMesh = GenerateCubieMesh(Cube, x, y, z);
                        Cube->UniqueMeshes[MeshIndex] = NewMesh;
#else
                        Cube->UniqueMeshes[MeshIndex] = GenerateCubieMesh(Cube, x, y, z);
#endif
                    }
                    
                    Cube->Visible.MeshIndex[VisibleCubie] = UniqueMeshMap[UniqueName];
                }
                else
                {
                    Cube->Visible.MeshIndex[VisibleCubie] = -1;
                }
            }
        }
    }
    
    //Assert(UniqueMeshMap.size() == RUBIKS_MESHES_COUNT);
    //Assert(Cube->UniqueMeshesCount == RUBIKS_MESHES_COUNT);
}

INTERNAL_FUNCTION inline v3 GetCubieP(rubiks_cube* Cube, int x, int y, int z)
{
    v3 CubieP = V3((f32)x * Cube->OneCubieLen + Cube->CubieOffset,
                   (f32)y * Cube->OneCubieLen + Cube->CubieOffset,
                   (f32)z * Cube->OneCubieLen + Cube->CubieOffset);
    
    return(CubieP);
}

INTERNAL_FUNCTION inline void AddCubieToRotatedFace(rubiks_cube* Cube, 
                                                    rubiks_rotate_face* RotFace,
                                                    int x, int y, int z)
{
    int CubieIndex = GetCubieIndex(Cube, x, y, z);
    
    RotFace->Face[RotFace->Count] = Cube->Current[CubieIndex];
    RotFace->TempFace[RotFace->Count] = RotFace->Face[RotFace->Count];
    
    RotFace->IndicesInCurrent[RotFace->Count] = CubieIndex;
    
    RotFace->Count++;
}

INTERNAL_FUNCTION inline rubiks_rotate_face* GetFaceX(rubiks_cube* Cube, int x)
{
    rubiks_rotate_face* Result = &Cube->RotateFace;
    
    Result->Count = 0;
    Result->AxisIndex = RubiksAxis_X;
    Result->FaceIndex = x;
    
    for(int z = Cube->Dim - 1; z >= 0; z--)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            AddCubieToRotatedFace(Cube, Result, x, y, z);
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline rubiks_rotate_face* GetFaceY(rubiks_cube* Cube, int y)
{
    rubiks_rotate_face* Result = &Cube->RotateFace;
    
    Result->Count = 0;
    Result->AxisIndex = RubiksAxis_Y;
    Result->FaceIndex = y;
    
    for(int z = 0; z < Cube->Dim; z++)
    {
        for(int x = 0; x < Cube->Dim; x++)
        {
            AddCubieToRotatedFace(Cube, Result, x, y, z);
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline rubiks_rotate_face* GetFaceZ(rubiks_cube* Cube, int z)
{
    rubiks_rotate_face* Result = &Cube->RotateFace;
    
    Result->Count = 0;
    Result->AxisIndex = RubiksAxis_Z;
    Result->FaceIndex = z;
    
    for(int y = 0; y < Cube->Dim; y++)
    {
        for(int x = Cube->Dim - 1; x >= 0; x--)
        {
            AddCubieToRotatedFace(Cube, Result, x, y, z);
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline int RubiksGetRotatedIndex(int Current, int Shift, 
                                                   int Count)
{
    int Result = Current + Shift;
    
    if(Result < 0)
    {
        Result += Count;
    }
    else if(Result >= Count)
    {
        Result -= Count;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void RubiksCalcPrecomputedFace(rubiks_cube* Cube,
                                                 rubiks_precomp_face* Face, 
                                                 memory_arena* Arena)
{
    
    Face->LoopCount = Cube->Dim / 2;
    
    Face->InitLoops = PushArray(Arena, int*, Face->LoopCount);
    Face->RotatedClockwise = PushArray(Arena, int*, Face->LoopCount);
    Face->RotatedCounterClockwise = PushArray(Arena, int*, Face->LoopCount);
    Face->LoopCubiesCount = PushArray(Arena, int, Face->LoopCount);
    
    for(int i = 0; i < Face->LoopCount; i++)
    {
        int CurrentDim = Cube->Dim - 2 * i;
        
        int CurLoopCubiesCount = (CurrentDim - 1) * 4;
        
        Face->LoopCubiesCount[i] = CurLoopCubiesCount;
        Face->InitLoops[i] = PushArray(Arena, int, CurLoopCubiesCount);
        Face->RotatedClockwise[i] = PushArray(Arena, int, CurLoopCubiesCount);
        Face->RotatedCounterClockwise[i] = PushArray(Arena, int, CurLoopCubiesCount);
    }
    
    // NOTE(Dima): This loop will walk LoopCount times
    int LoopIndex = 0;
    int CurrentStart = 0;
    int CurrentEnd = Cube->Dim - 1;
    while(CurrentStart < CurrentEnd)
    {
        int* ToRotateIndices = Face->InitLoops[LoopIndex];
        int* RotatedClockwise = Face->RotatedClockwise[LoopIndex];
        int* RotatedCounterClockwise = Face->RotatedCounterClockwise[LoopIndex];
        
        int ToRotateCount = 0;
        
        for(int x = CurrentStart;
            x < CurrentEnd;
            x++)
        {
            ToRotateIndices[ToRotateCount++] = CurrentStart * Cube->Dim + x;
        }
        
        for(int y = CurrentStart;
            y < CurrentEnd;
            y++)
        {
            ToRotateIndices[ToRotateCount++] = y * Cube->Dim + CurrentEnd;
        }
        
        for(int x = CurrentEnd;
            x > CurrentStart;
            x--)
        {
            ToRotateIndices[ToRotateCount++] = CurrentEnd * Cube->Dim + x;
        }
        
        for(int y = CurrentEnd;
            y > CurrentStart;
            y--)
        {
            ToRotateIndices[ToRotateCount++] = y * Cube->Dim + CurrentStart;
        }
        
        Assert(ToRotateCount == Face->LoopCubiesCount[LoopIndex]);
        
        // NOTE(Dima): Getting shift value
        int RotateShift = CurrentEnd - CurrentStart;
        
        int ShiftClockwise = RotateShift;
        int ShiftCounterClockwise = -RotateShift;
        
        // NOTE(Dima): Rotating indices
        for(int i = 0;
            i < ToRotateCount;
            i++)
        {
            int CWIndex = RubiksGetRotatedIndex(i, ShiftClockwise, ToRotateCount);
            int CCWIndex = RubiksGetRotatedIndex(i, ShiftCounterClockwise, ToRotateCount);
            
            RotatedClockwise[i] = ToRotateIndices[CWIndex];
            RotatedCounterClockwise[i] = ToRotateIndices[CCWIndex];
        }
        
        CurrentStart++;
        CurrentEnd--;
        
        LoopIndex++;
    }
}

INTERNAL_FUNCTION void RotateInternalStructure(rubiks_cube* Cube)
{
    rubiks_beginned_rotation* BeginnedRotation = &Cube->BeginnedRotation;
    
    rubiks_rotate_face* Face = &Cube->RotateFace;
    
    // NOTE(Dima): Updating internal rotation
    b32 IsOuter = (Face->FaceIndex == 0) || (Face->FaceIndex == Cube->Dim - 1);
    
    int LoopIndex = 0;
    int CurrentStart = 0;
    int CurrentEnd = Cube->Dim - 1;
    while(CurrentStart < CurrentEnd)
    {
        // NOTE(Dima): Preparing
        rubiks_precomp_face* PrecompFace = &Cube->PrecompFace;
        int* ToRotateIndices = PrecompFace->InitLoops[LoopIndex];
        int ToRotateCount = PrecompFace->LoopCubiesCount[LoopIndex];
        
        int* RotatedIndices = PrecompFace->RotatedCounterClockwise[LoopIndex];
        if(BeginnedRotation->IsClockwise)
        {
            RotatedIndices = PrecompFace->RotatedClockwise[LoopIndex];
        }
        
        // NOTE(Dima): Using rotated indices
        for(int i = 0;
            i < ToRotateCount;
            i++)
        {
            Face->Face[ToRotateIndices[i]] = Face->TempFace[RotatedIndices[i]];
        }
        
        CurrentStart++;
        CurrentEnd--;
        LoopIndex++;
        
        if(!IsOuter)
        {
            break;
        }
    }
    
    // NOTE(Dima): Copying rotated indices to Current
    for(int i = 0; i < Face->Count; i++)
    {
        Cube->Current[Face->IndicesInCurrent[i]] = Face->Face[i];
    }
}

INTERNAL_FUNCTION void AddCommandToCube(rubiks_cube* Cube,
                                        int Axis,
                                        int FaceIndex,
                                        int IsClockwise)
{
    rubiks_command* NewCommand = &Cube->Commands[Cube->AddIndex];
    
    NewCommand->Axis = Axis;
    NewCommand->FaceIndex = FaceIndex;
    NewCommand->IsClockwise = IsClockwise;
    
    Cube->AddIndex = (Cube->AddIndex + 1) % Cube->CommandsCount;
    
    Assert(Cube->AddIndex != Cube->DoIndex);
}

INTERNAL_FUNCTION void FinishCommandExecution(rubiks_cube* Cube)
{
    Cube->DoIndex = (Cube->DoIndex + 1) % Cube->CommandsCount;
}

INTERNAL_FUNCTION b32 CanExecuteCommand(rubiks_cube* Cube)
{
    b32 Result = (Cube->AddIndex != Cube->DoIndex) && !Cube->IsRotatingNow;
    
    return(Result);
}

INTERNAL_FUNCTION void FinishCubeRotation(rubiks_cube* Cube)
{
    rubiks_beginned_rotation* Beginned = &Cube->BeginnedRotation;
    
    int FaceCubiesCount = Cube->Dim * Cube->Dim;
    
    // NOTE(Dima): Updating transforms
    for(int FaceCubieIndex = 0;
        FaceCubieIndex < FaceCubiesCount;
        FaceCubieIndex++)
    {
        int CubieIndex = Cube->RotateFace.Face[FaceCubieIndex];
        int VisibleCubie = Cube->CubiesToVisible[CubieIndex];
        
        if(VisibleCubie != -1)
        {
            Cube->Visible.Transform[VisibleCubie] = Cube->Visible.Transform[VisibleCubie] * Beginned->AppliedRotation;
            Cube->Visible.AppliedRotation[VisibleCubie] = IdentityMatrix4();
        }
    }
    
    RotateInternalStructure(Cube);
    
    Beginned->AppliedRotation = IdentityMatrix4();
    Cube->IsRotatingNow = false;
    
    FinishCommandExecution(Cube);
}

// NOTE(Dima): Function returns true if rotation was beginned
INTERNAL_FUNCTION b32 BeginRotateFace(rubiks_cube* Cube, 
                                      int Axis,
                                      int FaceIndex,
                                      f32 Speed,
                                      b32 IsClockwise)
{
    rubiks_beginned_rotation* BeginnedRotation = &Cube->BeginnedRotation;
    
    b32 Result = false;
    
    if(!Cube->IsRotatingNow)
    {
        BeginnedRotation->InRotationTime = 0.0f;
        BeginnedRotation->TimeForRotation = RUBIKS_TIME_FOR_ROTATION / Speed;
        
        switch(Axis)
        {
            case RubiksAxis_X:
            {
                BeginnedRotation->RotationMatrix = RotationMatrixX;
                BeginnedRotation->GetFace = GetFaceX;
            }break;
            
            case RubiksAxis_Y:
            {
                BeginnedRotation->RotationMatrix = RotationMatrixY;
                BeginnedRotation->GetFace = GetFaceY;
                
            }break;
            
            case RubiksAxis_Z:
            {
                BeginnedRotation->RotationMatrix = RotationMatrixZ;
                BeginnedRotation->GetFace = GetFaceZ;
            }break;
        }
        BeginnedRotation->FaceIndex = FaceIndex;
        BeginnedRotation->IsClockwise = IsClockwise;
        BeginnedRotation->DirectionMultiplier = FaceIndex > (Cube->Dim / 2) ? -1.0f : 1.0f;
        
        Cube->IsRotatingNow = true;
        
        Result = true;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void UpdateBeginnedRotation(rubiks_cube* Cube)
{
    FUNCTION_TIMING();
    
    rubiks_beginned_rotation* BeginnedRotation = &Cube->BeginnedRotation;
    
    if(Cube->IsRotatingNow)
    {
        f32 t = Clamp01(BeginnedRotation->InRotationTime / BeginnedRotation->TimeForRotation);
        
        f32 Angle = F_PI * 0.5f * t;
        //f32 Angle = F_PI * 0.5f * t * BeginnedRotation->DirectionMultiplier;
        
        if(!BeginnedRotation->IsClockwise)
        {
            Angle = -Angle;
        }
        
        // NOTE(Dima): Applying cubies rotation
        m44 AppliedRotation = BeginnedRotation->RotationMatrix(Angle);
        rubiks_rotate_face* RotateFace = BeginnedRotation->GetFace(Cube, BeginnedRotation->FaceIndex);
        
        BeginnedRotation->AxisIndex = RotateFace->AxisIndex;
        BeginnedRotation->AppliedRotation = AppliedRotation;
        
        for(int CubieIndex = 0;
            CubieIndex < RotateFace->Count;
            CubieIndex++)
        {
            int VisibleCubie = Cube->CubiesToVisible[RotateFace->Face[CubieIndex]];
            
            if(VisibleCubie != -1)
            {
                Cube->Visible.AppliedRotation[VisibleCubie] = AppliedRotation;
            }
        }
        
        // NOTE(Dima): Finishing rotation (updating cube structure)
        if(BeginnedRotation->InRotationTime > BeginnedRotation->TimeForRotation)
        {
            FinishCubeRotation(Cube);
        }
        
        BeginnedRotation->InRotationTime += Global_Time->DeltaTime;
    }
}

INTERNAL_FUNCTION inline void InitToVisibleMapping(rubiks_cube* Cube)
{
    std::unordered_set<std::string> UniqueNames;
    
    // NOTE(Dima): Initializing cubies
    for(int x = 0; x < Cube->Dim; x++)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int z = 0; z < Cube->Dim; z++)
            {
                int VisibleIndex = -1;
                
                if(RubiksIsOuter(Cube, x, y, z))
                {
                    char Buf[128];
                    stbsp_sprintf(Buf, "x%d y%d z%d", x, y, z);
                    
                    std::string Name = std::string(Buf);
                    
                    VisibleIndex = UniqueNames.size();
                    UniqueNames.insert(Name);
                }
                
                int CubieIndex = GetCubieIndex(Cube, x, y, z);
                Cube->CubiesToVisible[CubieIndex] = VisibleIndex;
            }
        }
    }
    
    Assert(UniqueNames.size() == Cube->Visible.Count);
}

INTERNAL_FUNCTION inline void ResetCubies(rubiks_cube* Cube)
{
    rubiks_visible_cubies* Vis = &Cube->Visible;
    
    // NOTE(Dima): Initializing cubies
    for(int x = 0; x < Cube->Dim; x++)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int z = 0; z < Cube->Dim; z++)
            {
                int CubieIndex = GetCubieIndex(Cube, x, y, z);
                Cube->Current[CubieIndex] = CubieIndex;
                
                int VisibleCubie = GetVisibleCubie(Cube, x, y, z);
                if(VisibleCubie != -1)
                {
                    v3 CubieP = GetCubieP(Cube, x, y, z);
                    
                    Vis->Transform[VisibleCubie] = TranslationMatrix(CubieP);
                    Vis->AppliedRotation[VisibleCubie] = IdentityMatrix4();
                    Vis->InitP[VisibleCubie] = CubieP;
                }
            }
        }
    }
}

inline int GetVisibleCubiesCount(int Dim)
{
    int OuterDim = Dim;
    int InnerDim = FlowerMax(Dim - 2, 0);
    
    int Result = OuterDim * OuterDim * OuterDim - InnerDim * InnerDim * InnerDim;
    
    return(Result);
}

inline rubiks_cube CreateCube(memory_arena* Arena, 
                              int CubeDim, 
                              f32 SideLen, 
                              b32 RoundStickers = true)
{
    rubiks_cube Result = {};
    
    int OneFaceCount = CubeDim * CubeDim;
    Result.CubiesCount = OneFaceCount * CubeDim;
    Result.Dim = CubeDim;
    Result.CubiesToVisible = PushArray(Arena, int, Result.CubiesCount);
    Result.Current = PushArray(Arena, int, Result.CubiesCount);
    
    // NOTE(Dima): Init visible cubies
    int VisibleCount = GetVisibleCubiesCount(CubeDim);
    Result.Visible.Count = VisibleCount;
    Result.Visible.FinalTransform = PushArray(Arena, m44, CeilAlign(VisibleCount, 4));
    Result.Visible.Transform = PushArray(Arena, m44, CeilAlign(VisibleCount, 4));
    Result.Visible.AppliedRotation = PushArray(Arena, m44, CeilAlign(VisibleCount, 4));
    Result.Visible.InitP = PushArray(Arena, v3, CeilAlign(VisibleCount, 4));
    Result.Visible.MeshIndex = PushArray(Arena, int, CeilAlign(VisibleCount, 4));
    
    InitToVisibleMapping(&Result);
    
    Result.SideLen = SideLen;
    Result.BeginnedRotation.InRotationTime = 0.0f;
    Result.BeginnedRotation.TimeForRotation = 0.0f;
    
    Result.RotateFace.Face = PushArray(Arena, int, OneFaceCount);
    Result.RotateFace.TempFace = PushArray(Arena, int, OneFaceCount);
    Result.RotateFace.IndicesInCurrent = PushArray(Arena, int, OneFaceCount);
    Result.RotateFace.Count = 0;
    
    Result.ToRotateIndices = PushArray(Arena, int, 4 * (CubeDim - 1));
    Result.RotatedIndices = PushArray(Arena, int, 4 * (CubeDim - 1));
    
    Result.OneCubieLen = Result.SideLen / (f32)Result.Dim;
    Result.HalfSideLen = Result.SideLen * 0.5f;
    Result.CubieOffset = Result.OneCubieLen * 0.5f - Result.HalfSideLen;
    Result.StickerHeight = Result.OneCubieLen * 0.018f;
    Result.IsRotatingNow = false;
    
    // NOTE(Dima): Precomputing face indices
    RubiksCalcPrecomputedFace(&Result, &Result.PrecompFace, Arena);
    
    // NOTE(Dima): Generating sticker meshes
    GenerateStickerMeshes(&Result, RoundStickers);
    
    // NOTE(Dima): Initializing cubies
    
    ResetCubies(&Result);
    RubiksPrecomputeMeshes(&Result);
    RubiksGenerateInnerMeshes(&Result);
    
    // NOTE(Dima): Commands
    Result.DoIndex = 0;
    Result.AddIndex = 0;
    Result.CommandsCount = CubeDim * CubeDim * 4 + 100;
    Result.Commands = PushArray(Arena, rubiks_command, Result.CommandsCount);
    
    return(Result);
}

// TODO(Dima): Walk only on outer cubies
INTERNAL_FUNCTION void ShowCube(rubiks_cube* Cube, v3 P, b32 DebugMode = false)
{
    FUNCTION_TIMING();
    
    m44 OffsetMatrix = TranslationMatrix(P);
    
    rubiks_visible_cubies* Vis = &Cube->Visible;
    
    if(Cube->IsRotatingNow && !DebugMode)
    {
        rubiks_beginned_rotation* Rot = &Cube->BeginnedRotation;
        
        if(Cube->InnerMeshIsGenerated[Rot->AxisIndex])
        {
            v3 RotationOrigin = (RubiksAxisValue[Rot->AxisIndex] * 
                                 ((Cube->OneCubieLen * Rot->FaceIndex) + Cube->CubieOffset));
            
            m44 InnerTransform = TranslationMatrix(RotationOrigin) * Rot->AppliedRotation * OffsetMatrix;
            
            mesh* InnerMesh = &Cube->InnerMeshes[Rot->AxisIndex];
            
            PushMesh(InnerMesh,
                     0,
                     InnerTransform,
                     V3(1.0f));
        }
    }
    
#if 0  
    {
        BLOCK_TIMING("Cube Transforms Calculation");
        
        // NOTE(Dima): Transformations calculation
        for(int VisibleIndex = 0;
            VisibleIndex < Vis->Count;
            VisibleIndex++)
        {
            Vis->FinalTransform[VisibleIndex] = Vis->Transform[VisibleIndex] * Vis->AppliedRotation[VisibleIndex] * OffsetMatrix;
        }
    }
#else
    m44_4x OffsetMatrix4x = M44_4X(OffsetMatrix);
    
    {
        BLOCK_TIMING("Cube Transforms Calculation SIMD");
        
        // NOTE(Dima): Transformations calculation
        for(int VisibleIndex = 0;
            VisibleIndex < Vis->Count;
            VisibleIndex += 4)
        {
            m44_4x Transform = M44_4X(Vis->Transform[VisibleIndex + 0],
                                      Vis->Transform[VisibleIndex + 1],
                                      Vis->Transform[VisibleIndex + 2],
                                      Vis->Transform[VisibleIndex + 3]);
            
            m44_4x AppliedRotation = M44_4X(Vis->AppliedRotation[VisibleIndex + 0],
                                            Vis->AppliedRotation[VisibleIndex + 1],
                                            Vis->AppliedRotation[VisibleIndex + 2],
                                            Vis->AppliedRotation[VisibleIndex + 3]);
            
            m44_4x FinalTransform = Transform * AppliedRotation * OffsetMatrix4x;
            
            M44_4X_Store((f32*)&Vis->FinalTransform[VisibleIndex], FinalTransform);
        }
    }
#endif
    
    // NOTE(Dima): For DEBUG mode
    if(DebugMode)
    {
        for(int x = 0; x < Cube->Dim; x++)
        {
            for(int y = 0; y < Cube->Dim; y++)
            {
                for(int z = 0; z < Cube->Dim; z++)
                {
                    if(RubiksIsOuter(Cube, x, y, z))
                    {
                        int CubieCurrentIndex = GetCurrentIndex(Cube, x, y, z);
                        int VisibleCubie = Cube->CubiesToVisible[CubieCurrentIndex];
                        
                        char Buf[16];
                        IntegerToString(CubieCurrentIndex, Buf);
                        
                        v3 NumP = Vis->InitP[VisibleCubie];
                        
                        PrintText3D(Global_UI->Params.Font,
                                    Buf,
                                    V3_Left(), V3_Up(),
                                    P + NumP,
                                    ColorRed(),
                                    0.3f, 
                                    false);
                    }
                }
            }
        }
    }
    else
    {
        BLOCK_TIMING("Cube Push to Render");
        
        // NOTE(Dima): Showing cubies
        for(int VisibleIndex = 0;
            VisibleIndex < Vis->Count;
            VisibleIndex++)
        {
            int MeshIndex = Vis->MeshIndex[VisibleIndex];
            mesh* Mesh = &Cube->UniqueMeshes[MeshIndex];
            
            v3 Color = V3(1.0f);
            
#if 1
            PushInstanceMesh(Cube->Dim * Cube->Dim,
                             Mesh, 0,
                             Vis->FinalTransform[VisibleIndex]);
#else
            
            PushMesh(Mesh, 0, Vis->FinalTransform[VisibleIndex]);
#endif
        }
        
    }
}

INTERNAL_FUNCTION void GenerateScrubmle(rubiks_cube* Cube, int Seed = 123)
{
    random_generation Random = SeedRandom(Seed);
    
    int ScrumbleCount = RandomBetweenU32(&Random, Cube->Dim * 8, Cube->Dim * 10);
    
    for(int ScrumbleIndex = 0;
        ScrumbleIndex < ScrumbleCount;
        ScrumbleIndex++)
    {
        AddCommandToCube(Cube,
                         RandomBetweenU32(&Random, 0, RubiksAxis_Count),
                         RandomBetweenU32(&Random, 0, Cube->Dim),
                         RandomBool(&Random));
    }
}

INTERNAL_FUNCTION void UpdateCube(rubiks_cube* Cube,
                                  v3 P, 
                                  f32 Speed = 1.0f, 
                                  b32 DebugMode = false)
{
    FUNCTION_TIMING();
    
    UpdateBeginnedRotation(Cube);
    
    if(CanExecuteCommand(Cube))
    {
        rubiks_command* Command = &Cube->Commands[Cube->DoIndex];
        
        BeginRotateFace(Cube, 
                        Command->Axis,
                        Command->FaceIndex,
                        Speed,
                        Command->IsClockwise);
    }
    
    ShowCube(Cube, P, DebugMode);
}
