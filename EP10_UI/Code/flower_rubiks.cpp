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

inline rubiks_cubie* GetCubie(rubiks_cube* Cube, int x, int y, int z)
{
    int ResultIndex = GetCubieIndex(Cube, x, y, z);
    
    rubiks_cubie* Result = Cube->Cubies + ResultIndex;
    
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
                                                            f32 Roundness)
{
    helper_rubiks_mesh Result = {};
    
    f32 S = StickerPercentage * CubieLen;
    f32 r = Roundness * S;
    
    f32 HalfS = S * 0.5f;
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
                rubiks_cubie* Cubie = GetCubie(Cube, x, y, z);
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
                    
                    Cubie->MeshIndex = UniqueMeshMap[UniqueName];
                }
                else
                {
                    Cubie->MeshIndex = -1;
                }
            }
        }
    }
    
    //Assert(UniqueMeshMap.size() == RUBIKS_MESHES_COUNT);
    //Assert(Cube->UniqueMeshesCount == RUBIKS_MESHES_COUNT);
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

INTERNAL_FUNCTION inline v3 GetCubieP(rubiks_cube* Cube, int x, int y, int z)
{
    v3 CubieP = V3((f32)x * Cube->OneCubieLen + Cube->CubieOffset,
                   (f32)y * Cube->OneCubieLen + Cube->CubieOffset,
                   (f32)z * Cube->OneCubieLen + Cube->CubieOffset);
    
    return(CubieP);
}

INTERNAL_FUNCTION inline void ResetCubies(rubiks_cube* Cube)
{
    // NOTE(Dima): Initializing cubies
    for(int x = 0; x < Cube->Dim; x++)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int z = 0; z < Cube->Dim; z++)
            {
                v3 CubieP = GetCubieP(Cube, x, y, z);
                
                int CubieIndex = GetCubieIndex(Cube, x, y, z);
                Cube->Current[CubieIndex] = CubieIndex;
                
                rubiks_cubie* Cubie = GetCubie(Cube, x, y, z);
                
                Cubie->Transform = TranslationMatrix(CubieP);
                Cubie->AppliedRotation = IdentityMatrix4();
                Cubie->InitP = CubieP;
            }
        }
    }
}

inline rubiks_cube CreateCube(memory_arena* Arena, int CubeDim, f32 SideLen, v3 P = V3(0.0f))
{
    rubiks_cube Result = {};
    
    int OneFaceCount = CubeDim * CubeDim;
    Result.CubiesCount = OneFaceCount * CubeDim;
    Result.Dim = CubeDim;
    Result.Cubies = PushArray(Arena, rubiks_cubie, Result.CubiesCount);
    Result.Current = PushArray(Arena, int, Result.CubiesCount);
    
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
    
    // NOTE(Dima): Generating sticker helper meshes
    Result.HelperStickerUp = GetStickerMeshInternal(Result.OneCubieLen,
                                                    0.88f, 0.1f);
    Result.HelperStickerDown = GetStickerMeshInternal(Result.OneCubieLen,
                                                      0.91f, 0.1f);
    
    // NOTE(Dima): Generating sticker meshes
    for(int DirectionIndex = 0;
        DirectionIndex < RubiksDirection_Count;
        DirectionIndex++)
    {
        helper_mesh& HelperMesh = Result.StickersMeshes[DirectionIndex];
        
        HelperMesh.Name = std::string("StickerMesh");
        
        v3 StickerColor = RubiksColors[DirectionIndex].rgb;
        
        helper_rubiks_mesh* StickerUp = &Result.HelperStickerUp;
        helper_rubiks_mesh* StickerDown = &Result.HelperStickerDown;
        
        // NOTE(Dima): Generating sticker sides
        int NumPerimP = StickerUp->PerimeterPoints.size();
        for(int PIndex = 0; 
            PIndex < NumPerimP; 
            PIndex++)
        {
            v3 CurTop = RubiksPlaneTo3DPoint(&Result, 
                                             StickerUp->PerimeterPoints[PIndex], 
                                             DirectionIndex, true);
            v3 NextTop = RubiksPlaneTo3DPoint(&Result, 
                                              StickerUp->PerimeterPoints[(PIndex + 1) % NumPerimP], 
                                              DirectionIndex, true);
            v3 CurBot = RubiksPlaneTo3DPoint(&Result, 
                                             StickerDown->PerimeterPoints[PIndex], 
                                             DirectionIndex, false);
            v3 NextBot = RubiksPlaneTo3DPoint(&Result, 
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
            v3 CurP = RubiksPlaneTo3DPoint(&Result, 
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
    
    
    // NOTE(Dima): Initializing cubies
    ResetCubies(&Result);
    RubiksPrecomputeMeshes(&Result);
    RubiksGenerateInnerMeshes(&Result);
    
    return(Result);
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

// TODO(Dima): Rewrite these functions to store those indices in a loop by loop form
INTERNAL_FUNCTION inline rubiks_rotate_face* GetFaceX(rubiks_cube* Cube, int x)
{
    rubiks_rotate_face* Result = &Cube->RotateFace;
    
    Result->Count = 0;
    Result->AxisIndex = RubiksAxis_X;
    
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
    
    for(int y = 0; y < Cube->Dim; y++)
    {
        for(int x = Cube->Dim - 1; x >= 0; x--)
        {
            AddCubieToRotatedFace(Cube, Result, x, y, z);
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION void RotateInternalStructure(rubiks_cube* Cube)
{
    rubiks_beginned_rotation* BeginnedRotation = &Cube->BeginnedRotation;
    
    rubiks_rotate_face* Face = &Cube->RotateFace;
    
    // NOTE(Dima): Updating internal rotation
    int* ToRotateIndices = Cube->ToRotateIndices;
    int* RotatedIndices = Cube->RotatedIndices;
    
    int CurrentStart = 0;
    int CurrentEnd = Cube->Dim - 1;
    while(CurrentStart < CurrentEnd)
    {
        int ToRotateCount = 0;
        
        // TODO(Dima): These can be precomputed for different sizes of cube
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
        
        // NOTE(Dima): Rotating indices
        int RotateShift = CurrentEnd - CurrentStart;
        if(!BeginnedRotation->IsClockwise)
        {
            RotateShift = -RotateShift;
        }
        for(int i = 0;
            i < ToRotateCount;
            i++)
        {
            int ToGetIndex = i + RotateShift;
            if(ToGetIndex < 0)
            {
                ToGetIndex += ToRotateCount;
            }
            else if(ToGetIndex >= ToRotateCount)
            {
                ToGetIndex -= ToRotateCount;
            }
            
            RotatedIndices[i] = ToRotateIndices[ToGetIndex];
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
    }
    
    // NOTE(Dima): Copying rotated indices to Current
    for(int i = 0; i < Face->Count; i++)
    {
        Cube->Current[Face->IndicesInCurrent[i]] = Face->Face[i];
    }
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
        rubiks_cubie* Cubie = &Cube->Cubies[CubieIndex];
        
        Cubie->Transform = Cubie->Transform * Beginned->AppliedRotation;
        Cubie->AppliedRotation = IdentityMatrix4();
    }
    
    RotateInternalStructure(Cube);
    
    Beginned->AppliedRotation = IdentityMatrix4();
    Cube->IsRotatingNow = false;
}

INTERNAL_FUNCTION void UpdateBeginnedRotation(rubiks_cube* Cube)
{
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
            rubiks_cubie* Cubie = &Cube->Cubies[RotateFace->Face[CubieIndex]];
            
            Cubie->AppliedRotation = AppliedRotation;
        }
        
        // NOTE(Dima): Finishing rotation (updating cube structure)
        if(BeginnedRotation->InRotationTime > BeginnedRotation->TimeForRotation)
        {
            FinishCubeRotation(Cube);
        }
        
        BeginnedRotation->InRotationTime += Global_Time->DeltaTime;
    }
}


// NOTE(Dima): Function returns true if rotation was beginned
INTERNAL_FUNCTION b32 BeginRotateFace(rubiks_cube* Cube, 
                                      rubiks_rotation_function* RotationMatrix,
                                      rubiks_get_face_function* RubiksGetFace,
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
        BeginnedRotation->RotationMatrix = RotationMatrix;
        BeginnedRotation->GetFace = RubiksGetFace;
        BeginnedRotation->FaceIndex = FaceIndex;
        BeginnedRotation->IsClockwise = IsClockwise;
        BeginnedRotation->DirectionMultiplier = FaceIndex > (Cube->Dim / 2) ? -1.0f : 1.0f;
        
        Cube->IsRotatingNow = true;
        
        Result = true;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void ShowCube(rubiks_cube* Cube, v3 P, b32 DebugMode = false)
{
    m44 OffsetMatrix = TranslationMatrix(P);
    
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
    
    for(int x = 0; x < Cube->Dim; x++)
    {
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int z = 0; z < Cube->Dim; z++)
            {
                rubiks_cubie* Cubie = GetCubie(Cube, x, y, z);
                
                int CubieCurrentIndex = GetCurrentIndex(Cube, x, y, z);
                
                if(DebugMode)
                {
                    if(RubiksIsOuter(Cube, x, y, z))
                    {
                        char Buf[16];
                        IntegerToString(CubieCurrentIndex, Buf);
                        
                        v3 NumP = Cubie->InitP;
                        
                        PrintText3D(Global_UI->Params.Font,
                                    Buf,
                                    V3_Left(), V3_Up(),
                                    P + NumP,
                                    ColorRed(),
                                    0.3f, 
                                    false);
                    }
                }
                else
                {
                    if(RubiksIsOuter(Cube, x, y, z) && Cubie->MeshIndex != -1)
                    {
                        mesh* Mesh = &Cube->UniqueMeshes[Cubie->MeshIndex];
                        
                        v3 Color = V3(1.0f);
                        m44 Transform = Cubie->Transform * Cubie->AppliedRotation * OffsetMatrix;
                        
#if 1
                        PushInstanceMesh(Cube->Dim * Cube->Dim,
                                         Mesh, 0,
                                         Transform);
#else
                        
                        PushMesh(Global_RenderCommands, 
                                 Mesh, 0,
                                 Transform);
#endif
                    }
                }
            }
        }
    }
}
