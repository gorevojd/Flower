INTERNAL_FUNCTION inline int InvertCoord(rubiks_cube* Cube, int Coord)
{
    int Result = Cube->Dim - 1 - Coord;
    
    return(Result);
}

INTERNAL_FUNCTION inline rubiks_command* AddCommandInternal(rubiks_cube* Cube,
                                                            u32 Type)
{
    rubiks_command* NewCommand = &Cube->Commands[Cube->AddIndex];
    
    NewCommand->Type = Type;
    
    Cube->AddIndex = (Cube->AddIndex + 1) % Cube->CommandsCount;
    Assert(Cube->AddIndex != Cube->DoIndex);
    
    return(NewCommand);
}

INTERNAL_FUNCTION void AddCommandToCube(rubiks_cube* Cube,
                                        int Axis,
                                        int FirstFaceIndex,
                                        int LastFaceIndex,
                                        int IsClockwise)
{
    rubiks_command* NewCommand = AddCommandInternal(Cube, RubiksCommand_Rotation);
    
    NewCommand->Axis = Axis;
    NewCommand->FirstFaceIndex = FirstFaceIndex;
    NewCommand->LastFaceIndex = LastFaceIndex;
    NewCommand->IsClockwise = IsClockwise;
}

INTERNAL_FUNCTION void AddCommandToCube(rubiks_cube* Cube,
                                        int Axis,
                                        int FaceIndex,
                                        int IsClockwise)
{
    AddCommandToCube(Cube,
                     Axis,
                     FaceIndex,
                     FaceIndex,
                     IsClockwise);
}

INTERNAL_FUNCTION void AddFinishStateCommand(rubiks_cube* Cube,
                                             u8 FinalState)
{
    rubiks_command* NewCommand = AddCommandInternal(Cube, RubiksCommand_ChangeState);
    
    NewCommand->FinalState = FinalState;
    Cube->ExecutingSolvingNow = true;
}

INTERNAL_FUNCTION void AddChangeSpeedCommand(rubiks_cube* Cube,
                                             f32 Speed,
                                             b32 Dynamic)
{
    rubiks_command* NewCommand = AddCommandInternal(Cube, RubiksCommand_ResetSpeed);
    
    NewCommand->IsDynamicSpeedChange = Dynamic;
    NewCommand->TargetSpeed = Speed;
}

INTERNAL_FUNCTION inline b32 CanStartSolvingState(rubiks_cube* Cube, u32 SolvingState)
{
    b32 Result = !Cube->ExecutingSolvingNow && (SolvingState == Cube->SolvingState);
    
    return(Result);
}

INTERNAL_FUNCTION inline void ChangeSolvingState(rubiks_cube* Cube, u32 TargetState)
{
    Cube->SolvingState = TargetState;
    Cube->ExecutingSolvingNow = false;
}

typedef void rubiks_rotate_side_command_func(rubiks_cube* Cube, b32 CounterClockwise);

// NOTE(Dima): Standard commands like R, L, U, D, F, B
INTERNAL_FUNCTION inline void RubCom_R(rubiks_cube* Cube, 
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, 0, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_L(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, Cube->Dim - 1, !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_U(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, Cube->Dim - 1, !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_D(rubiks_cube* Cube, 
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, 0, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_F(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z, 0, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_B(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z, Cube->Dim - 1, !CounterClockwise);
}

// NOTE(Dima): M, E, S
/*

M - Center on X axis. Rotation same as from L
E - Center on Y axis. Rotation same as from D
S - Center on Z axis. Rotation same as from F

*/

INTERNAL_FUNCTION inline void RubCom_M(rubiks_cube* Cube, 
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, 
                     1, Cube->Dim - 2,
                     !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_E(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, 
                     1, Cube->Dim - 2, 
                     CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_S(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z,
                     1, Cube->Dim - 2,
                     CounterClockwise);
}

// NOTE(Dima): Manual M E S for big cubes
INTERNAL_FUNCTION inline void RubCom_M_ManualMany(rubiks_cube* Cube,
                                                  int StartIndex,
                                                  int EndIndex,
                                                  b32 CounterClockwise = false)
{
    int OnCubeStart = Cube->Dim - 1 - StartIndex;
    int OnCubeEnd = Cube->Dim - 1 - EndIndex;
    
    AddCommandToCube(Cube, RubiksAxis_X,
                     OnCubeStart,
                     OnCubeEnd,
                     !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_M_Manual(rubiks_cube* Cube,
                                              int Index,
                                              b32 CounterClockwise = false)
{
    RubCom_M_ManualMany(Cube, Index, Index, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_E_ManualMany(rubiks_cube* Cube,
                                                  int StartIndex,
                                                  int EndIndex,
                                                  b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y,
                     StartIndex,
                     EndIndex,
                     CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_E_Manual(rubiks_cube* Cube,
                                              int Index,
                                              b32 CounterClockwise = false)
{
    RubCom_E_ManualMany(Cube, Index, Index, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_S_ManualMany(rubiks_cube* Cube,
                                                  int StartIndex,
                                                  int EndIndex,
                                                  b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z,
                     StartIndex,
                     EndIndex,
                     CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_S_Manual(rubiks_cube* Cube,
                                              int Index,
                                              b32 CounterClockwise = false)
{
    RubCom_S_ManualMany(Cube, Index, Index, CounterClockwise);
}

// NOTE(Dima): Commands for doubly rotation. Rotation of a side and center
INTERNAL_FUNCTION inline void RubCom_RR(rubiks_cube* Cube, 
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, 0, Cube->Dim - 2, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_LL(rubiks_cube* Cube,
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, 1, Cube->Dim - 1, !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_UU(rubiks_cube* Cube,
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, 1, Cube->Dim - 1, !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_DD(rubiks_cube* Cube, 
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, 0, Cube->Dim - 2, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_FF(rubiks_cube* Cube,
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z, 0, Cube->Dim - 2, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_BB(rubiks_cube* Cube,
                                        b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z, 1, Cube->Dim - 1, !CounterClockwise);
}

// NOTE(Dima): Commands for rotating whole cube
INTERNAL_FUNCTION inline void RubCom_X(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_X, 0, Cube->Dim - 1, CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_Y(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Y, 0, Cube->Dim - 1, !CounterClockwise);
}

INTERNAL_FUNCTION inline void RubCom_Z(rubiks_cube* Cube,
                                       b32 CounterClockwise = false)
{
    AddCommandToCube(Cube, RubiksAxis_Z, 0, Cube->Dim - 1, CounterClockwise);
}

// NOTE(Dima): Other command processing
INTERNAL_FUNCTION void FinishCommandExecution(rubiks_cube* Cube)
{
    Cube->DoIndex = (Cube->DoIndex + 1) % Cube->CommandsCount;
}

INTERNAL_FUNCTION b32 CanExecuteCommand(rubiks_cube* Cube)
{
    b32 Result = (Cube->AddIndex != Cube->DoIndex) && !Cube->IsRotatingNow;
    
    return(Result);
}

// NOTE(Dima): Solving
INTERNAL_FUNCTION inline b32 SideIsSolved(rubiks_cube* Cube, u32 SideIndex)
{
    u8* Side = Cube->Sides[SideIndex];
    
    u32 Result = true;
    if(Cube->Dim >= 3)
    {
        u32 TargetColor = Side[0];
        
        for(int y = 0; y < Cube->Dim; y++)
        {
            for(int x = 0; x < Cube->Dim; x++)
            {
                if(Side[y * Cube->Dim + x] != TargetColor)
                {
                    Result = false;
                    break;
                }
            }
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 CubeIsSolved(rubiks_cube* Cube)
{
    b32 Result = true;
    
    for(int i = 0; i < 6; i++)
    {
        if(!SideIsSolved(Cube, i))
        {
            Result = false;
            break;
        }
    }
    
    return(Result);
}

inline u8 GetSideCenterFirstColor(rubiks_cube* Cube, u32 SideIndex)
{
    Assert(Cube->Dim >= 3);
    
    u8* Side = Cube->Sides[SideIndex];
    
    u8 Result = Side[1 * Cube->Dim + 1];
    
    return(Result);
}

inline u8 GetEdgeCenterNColor(rubiks_cube* Cube, 
                              u32 SideIndex, 
                              u32 SideEdge,
                              int N,
                              b32 Invert)
{
    u8* Side = Cube->Sides[SideIndex];
    
    u8 Result = Side[0];
    
    if(Invert)
    {
        int CenterDim = Cube->Dim - 2;
        N = CenterDim - 1 - N;
    }
    
    if(Cube->Dim >= 3)
    {
        switch(SideEdge)
        {
            case RubSideEdge_Top:
            {
                Result = Side[1 + N];
            }break;
            
            case RubSideEdge_Right:
            {
                Result = Side[Cube->Dim * (1 + N) + (Cube->Dim - 1)];
            }break;
            
            case RubSideEdge_Bottom:
            {
                Result = Side[Cube->Dim * (Cube->Dim - 1) + 1 + N];
            }break;
            
            case RubSideEdge_Left:
            {
                Result = Side[Cube->Dim * (1 + N)];
            }break;
        }
    }
    
    return(Result);
}

inline u8 GetEdgeCenterFirstColor(rubiks_cube* Cube, u32 SideIndex, u32 SideEdge, b32 Invert)
{
    int N = 0;
    
    int CenterDim = Cube->Dim - 2;
    if(Cube->Dim % 2 == 1)
    {
        N = CenterDim / 2;
    }
    
    u8 Result = GetEdgeCenterNColor(Cube, SideIndex, SideEdge, N, Invert);
    
    return(Result);
}

inline u8 GetSideCornerColor(rubiks_cube* Cube, u32 SideIndex, u32 GetMode)
{
    u8* Side = Cube->Sides[SideIndex];
    
    u8 Result = Side[0];
    
    if(Cube->Dim >= 2)
    {
        switch(GetMode)
        {
            case RubSideGetCorn_TopLeft:
            {
                Result = Side[0];
            }break;
            
            case RubSideGetCorn_TopRight:
            {
                Result = Side[Cube->Dim - 1];
            }break;
            
            case RubSideGetCorn_BotLeft:
            {
                Result = Side[Cube->Dim * (Cube->Dim - 1)];
            }break;
            
            case RubSideGetCorn_BotRight:
            {
                Result = Side[Cube->Dim * Cube->Dim - 1];
            }break;
        }
    }
    
    return(Result);
}


INTERNAL_FUNCTION void GetEdgeColorsN(rubiks_cube* Cube, int EdgeIndex,
                                      u8* Color1,
                                      u8* Color2,
                                      int N)
{
    int* EdgesSides = RubiksEdgesSides[EdgeIndex];
    b32* InvertGet = RubiksInvertEdgesGet[EdgeIndex];
    
    u8 Side1 = EdgesSides[0];
    u8 Side2 = EdgesSides[1];
    
    u8 GetMethod1 = EdgesSides[2];
    u8 GetMethod2 = EdgesSides[3];
    
    b32 Invert1 = InvertGet[0];
    b32 Invert2 = InvertGet[1];
    
    u8 EdgeColor1;
    u8 EdgeColor2;
    
    if(N == -1)
    {
        EdgeColor1 = GetEdgeCenterFirstColor(Cube, Side1, GetMethod1, Invert1);
        EdgeColor2 = GetEdgeCenterFirstColor(Cube, Side2, GetMethod2, Invert2);
    }
    else
    {
        EdgeColor1 = GetEdgeCenterNColor(Cube, Side1, GetMethod1, N, Invert1);
        EdgeColor2 = GetEdgeCenterNColor(Cube, Side2, GetMethod2, N, Invert2);
    }
    
    *Color1 = EdgeColor1;
    *Color2 = EdgeColor2;
}

INTERNAL_FUNCTION void GetEdgeColors(rubiks_cube* Cube, 
                                     int EdgeIndex,
                                     u8* Color1,
                                     u8* Color2)
{
    GetEdgeColorsN(Cube, EdgeIndex, Color1, Color2, -1);
}

INTERNAL_FUNCTION b32 FindColorsOnEdgeCenter(rubiks_cube* Cube, 
                                             int EdgeIndex,
                                             u8 Color1,
                                             u8 Color2,
                                             b32 SearchAllEdgeCenter,
                                             b32* IsInverted,
                                             int* FoundIndex)
{
    
    u16 ColorCombInit = Color1 | ((u16)Color2 << 8);
    
    b32 Inverted = false;
    b32 Found = false;
    int IndexInCenterResult = -1;
    
    if(!SearchAllEdgeCenter)
    {
        u8 EdgeColor1;
        u8 EdgeColor2;
        
        GetEdgeColors(Cube, EdgeIndex, &EdgeColor1, &EdgeColor2);
        
        u16 ColorCombination1 = EdgeColor1 | ((u16)EdgeColor2 << 8);
        u16 ColorCombination2 = EdgeColor2 | ((u16)EdgeColor1 << 8);
        
        if(ColorCombInit == ColorCombination1)
        {
            Found = true;
            Inverted = false;
        }
        else if(ColorCombInit == ColorCombination2)
        {
            Found = true;
            Inverted = true;
        }
    }
    else
    {
        for(int n = 0; n < Cube->Dim - 2; n++)
        {
            u8 EdgeColor1;
            u8 EdgeColor2;
            
            GetEdgeColorsN(Cube, EdgeIndex, &EdgeColor1, &EdgeColor2, n);
            
            u16 ColorCombination1 = EdgeColor1 | ((u16)EdgeColor2 << 8);
            u16 ColorCombination2 = EdgeColor2 | ((u16)EdgeColor1 << 8);
            
            if(ColorCombInit == ColorCombination1)
            {
                Found = true;
                Inverted = false;
                IndexInCenterResult = n;
                break;
            }
            else if(ColorCombInit == ColorCombination2)
            {
                Found = true;
                Inverted = true;
                IndexInCenterResult = n;
                break;
            }
        }
    }
    
    if(IsInverted)
    {
        *IsInverted = Inverted;
    }
    
    if(FoundIndex)
    {
        *FoundIndex = IndexInCenterResult;
    }
    
    return(Found);
}

INTERNAL_FUNCTION int FindEdgeForColorsInternal(rubiks_cube* Cube,
                                                u8 Color1,
                                                u8 Color2,
                                                b32* IsInverted,
                                                int* EdgesIndices,
                                                int EdgesIndicesCount,
                                                b32 SearchAllEdgeCenter = false,
                                                int* IndexInCenter = 0)
{
    Assert(Cube->Dim >= 3);
    Assert(IsInverted);
    
    b32 Inverted = false;
    int Result = -1;
    
    if(IndexInCenter)
    {
        *IndexInCenter = -1;
    }
    
    for(int i = 0; i < EdgesIndicesCount; i++)
    {
        int EdgeIndex = EdgesIndices[i];
        
        b32 Found = FindColorsOnEdgeCenter(Cube, 
                                           EdgeIndex,
                                           Color1, Color2,
                                           SearchAllEdgeCenter,
                                           IsInverted,
                                           IndexInCenter);
        
        if(Found)
        {
            Result = EdgeIndex;
            
            break;
        }
    }
    
    if(EdgesIndicesCount == 12)
    {
        // NOTE(Dima): The edge should be found!
        Assert(Result != -1);
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindEdgeIndexForColors(rubiks_cube* Cube,
                                             u8 Color1,
                                             u8 Color2,
                                             b32* IsInverted)
{
    int EdgesIndices[12] = 
    {
        RubEdge_UpFront,
        RubEdge_UpRight,
        RubEdge_UpBack,
        RubEdge_UpLeft,
        
        RubEdge_DownFront,
        RubEdge_DownRight,
        RubEdge_DownBack,
        RubEdge_DownLeft,
        
        RubEdge_FrontRight,
        RubEdge_RightBack,
        RubEdge_BackLeft,
        RubEdge_LeftFront,
    };
    
    int Result = FindEdgeForColorsInternal(Cube,
                                           Color1, 
                                           Color2,
                                           IsInverted,
                                           EdgesIndices,
                                           12);
    
    return(Result);
}

inline u16 RubEdgeComb(u8 Color1, u8 Color2)
{
    u16 Result = (u16)Color1 | ((u16)Color2 << 8);
    
    return(Result);
}

inline u32 RubMakeColorComb(u8 Color1, u8 Color2, u8 Color3)
{
    u32 Result = Color1 | ((u32)Color2 << 8) | ((u32)Color3 << 16);
    
    return(Result);
}



INTERNAL_FUNCTION int FindCornerIndexForColors(rubiks_cube* Cube,
                                               u8 Color1,
                                               u8 Color2,
                                               u8 Color3,
                                               int* RotationResult)
{
    Assert(Cube->Dim >= 2);
    Assert(RotationResult);
    
    u32 ColorCombInit = RubMakeColorComb(Color1, Color2, Color3);
    
    int Rotation = 0;
    int Result = -1;
    for(int i = 0; i < 8; i++)
    {
        int* CornersPrep = RubiksCornersPrep[i];
        
        u8 CornCol1 = GetSideCornerColor(Cube, CornersPrep[0], CornersPrep[3]);
        u8 CornCol2 = GetSideCornerColor(Cube, CornersPrep[1], CornersPrep[4]);
        u8 CornCol3 = GetSideCornerColor(Cube, CornersPrep[2], CornersPrep[5]);
        
        u32 CurrentCombination1 = RubMakeColorComb(CornCol1, CornCol2, CornCol3);
        u32 CurrentCombination2 = RubMakeColorComb(CornCol2, CornCol3, CornCol1);
        u32 CurrentCombination3 = RubMakeColorComb(CornCol3, CornCol1, CornCol2);
        
        b32 Found = false;
        if(ColorCombInit == CurrentCombination1)
        {
            Rotation = RubCornRot_Correct;
            Found = true;
        }
        else if(ColorCombInit == CurrentCombination2)
        {
            Rotation = RubCornRot_SecondAtFirst;
            Found = true;
        }
        else if(ColorCombInit == CurrentCombination3)
        {
            Rotation = RubCornRot_ThirdAtFirst;
            Found = true;
        }
        
        if(Found)
        {
            Result = i;
            break;
        }
    }
    
    // NOTE(Dima): The edge should be found!
    Assert(Result != -1);
    *RotationResult = Rotation;
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 SideCenterOnRightPlace(rubiks_cube* Cube, u32 SideIndex)
{
    Assert(Cube->Dim >= 3);
    
    u32 TargetColor = SideIndex;
    
    u32 Result = true;
    if(Cube->Dim >= 3)
    {
        u32 CenterColor = GetSideCenterFirstColor(Cube, SideIndex);
        
        Result = (TargetColor == CenterColor);
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 EdgeCenterIsSolved(rubiks_cube* Cube, u32 EdgeIndex,
                                                int* FirstParityErrorIndex = 0,
                                                int* UnsolvedIndex = 0)
{
    b32 Result = true;
    
    int ResultUnsolvedIndex = -1;
    int FirstParityError = -1;
    
    if(Cube->Dim >= 4)
    {
        u8 EdgeColor1;
        u8 EdgeColor2;
        
        GetEdgeColors(Cube, EdgeIndex,
                      &EdgeColor1,
                      &EdgeColor2);
        
        u16 InitComb = (u16)EdgeColor1 | ((u16)EdgeColor2 << 8);
        u16 InitCombParity = (u16)EdgeColor2 | ((u16)EdgeColor1 << 8);
        
        for(int i = 0; i < Cube->Dim - 2; i++)
        {
            u8 Col1, Col2;
            GetEdgeColorsN(Cube, EdgeIndex,
                           &Col1, &Col2, i);
            
            u16 ColorComb = (u16)Col1 | ((u16)Col2 << 8);
            
            if((ColorComb != InitComb) && 
               (ColorComb != InitCombParity))
            {
                Result = false;
                ResultUnsolvedIndex = i;
                break;
            }
            
            if((ColorComb == InitCombParity) && 
               (FirstParityError == -1))
            {
                FirstParityError = i;
            }
        }
    }
    
    if(FirstParityErrorIndex)
    {
        *FirstParityErrorIndex = FirstParityError;
    }
    
    if(UnsolvedIndex)
    {
        *UnsolvedIndex = ResultUnsolvedIndex;
    }
    
    return(Result);
}

inline b32 SideCenterIsSolved(rubiks_cube* Cube, int SideIndex)
{
    b32 Result = true;
    
    if(Cube->Dim >= 4)
    {
        u8 InitColor = GetSideCenterFirstColor(Cube, SideIndex);
        
        u8* Side = Cube->Sides[SideIndex];
        
        for(int y = 1; y < Cube->Dim - 1; y++)
        {
            for(int x = 1; x < Cube->Dim - 1; x++)
            {
                if(Side[y * Cube->Dim + x] != InitColor)
                {
                    Result = false;
                    break;
                }
            }
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstSideWithUnsolvedCenter(rubiks_cube* Cube)
{
    int Result = -1;
    
    int Sides[6] = 
    {
        RubSide_Up,
        RubSide_Down,
        RubSide_Front,
        RubSide_Right,
        RubSide_Back,
        RubSide_Left,
    };
    
    for(int SideIndex = 0;
        SideIndex < RubSide_Count;
        SideIndex++)
    {
        if(!SideCenterIsSolved(Cube, Sides[SideIndex]))
        {
            Result = Sides[SideIndex];
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int GetCountOfSidesWithUnsolvedCenters(rubiks_cube* Cube)
{
    int Result = 0;
    
    for(int i = 0; i < 6; i++)
    {
        if(!SideCenterIsSolved(Cube, i))
        {
            Result++;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstWallWithUnsolvedCenter(rubiks_cube* Cube)
{
    int Result = -1;
    
    int Sides[4] = 
    {
        RubSide_Front,
        RubSide_Right,
        RubSide_Back,
        RubSide_Left,
    };
    
    for(int SideIndex = 0;
        SideIndex < 4;
        SideIndex++)
    {
        if(!SideCenterIsSolved(Cube, Sides[SideIndex]))
        {
            Result = Sides[SideIndex];
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstEdgeWithUnsolvedCenter(rubiks_cube* Cube)
{
    int Result = -1;
    
    for(int EdgeIndex = 0;
        EdgeIndex < RubEdge_Count;
        EdgeIndex++)
    {
        if(!EdgeCenterIsSolved(Cube, EdgeIndex))
        {
            Result = EdgeIndex;
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstEdgeWithParityError(rubiks_cube* Cube, int* ParityErrorIndex)
{
    int Result = -1;
    int ParityError = -1;
    
    for(int EdgeIndex = 0;
        EdgeIndex < RubEdge_Count;
        EdgeIndex++)
    {
        int FirstParity;
        b32 CenterIsSolved = EdgeCenterIsSolved(Cube, EdgeIndex, &FirstParity);
        
        if(CenterIsSolved && (FirstParity != -1))
        {
            Result = EdgeIndex;
            ParityError = FirstParity;
            
            break;
        }
    }
    
    if(ParityErrorIndex)
    {
        *ParityErrorIndex = ParityError;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 CrossEdgeCorrectlyPlaced(rubiks_cube* Cube, 
                                                      u32 TopCenterColor, 
                                                      u32 WallColor)
{
    b32 Result = true;
    
    if(Cube->Dim >= 3)
    {
        Assert(TopCenterColor == RubiksColor_White);
        
        u8 EdgeOnTopColor = GetEdgeCenterFirstColor(Cube, RubSide_Up, RubSideEdge_Bottom, false);
        u8 EdgeOnFronColor = GetEdgeCenterFirstColor(Cube, RubSide_Front, RubSideEdge_Top, false);
        
        b32 TopColorCorrect = (EdgeOnTopColor == TopCenterColor);
        b32 FrontColorCorrect = (EdgeOnFronColor == WallColor);
        
        if(!TopColorCorrect || !FrontColorCorrect)
        {
            Result = false;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline int FindSideWithCenterColor(rubiks_cube* Cube, u8 Color)
{
    int Result = -1;
    
    for(int SideIndex = 0; 
        SideIndex < 6;
        SideIndex++)
    {
        u8 CurrentColor = GetSideCenterFirstColor(Cube, SideIndex);
        if((Cube->Dim % 2 == 1) &&
           (Cube->Dim >= 4))
        {
            u8* Side = Cube->Sides[SideIndex];
            
            int HalfDim = Cube->Dim / 2;
            
            CurrentColor = Side[Cube->Dim * HalfDim + HalfDim];
        }
        
        if(CurrentColor == Color)
        {
            Result = SideIndex;
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstUnsolvedColumnOnCenter(rubiks_cube* Cube, 
                                                      int SideIndex,
                                                      u8 TargetColor)
{
    Assert(Cube->Dim >= 4);
    
    int UnsolvedColumn = -1;
    
    u8* Side = Cube->Sides[SideIndex];
    
    for(int ColumnIndex = 0;
        ColumnIndex < Cube->Dim - 2;
        ColumnIndex++)
    {
        b32 ColumnIsSolved = true;
        
        for(int RowIndex = 0;
            RowIndex < Cube->Dim - 2;
            RowIndex++)
        {
            int y = (RowIndex + 1);
            int x = (ColumnIndex + 1);
            
            b32 IsMiddleColumn = (ColumnIndex == ((Cube->Dim - 2) / 2));
            b32 IsMiddleRow = (RowIndex == ((Cube->Dim - 2) / 2));
            
            b32 IsExactMiddleOfSide = false;
            
            if(Cube->Dim % 2 == 1)
            {
                if(IsMiddleColumn && IsMiddleRow)
                {
                    IsExactMiddleOfSide = true;
                }
            }
            
            if((Side[y * Cube->Dim + x] != TargetColor) && 
               (!IsExactMiddleOfSide))
            {
                ColumnIsSolved = false;
                break;
            }
        }
        
        if(!ColumnIsSolved)
        {
            UnsolvedColumn = ColumnIndex;
            
            break;
        }
    }
    
    return(UnsolvedColumn);
}

inline b32 IsExactMiddleOfCenter(rubiks_cube* Cube,
                                 int CoordOnCenter)
{
    b32 Result = false;
    
    if(Cube->Dim % 2 == 1)
    {
        int SideCoord = CoordOnCenter + 1;
        
        b32 IsMiddle = (SideCoord == (Cube->Dim / 2));
        
        if(IsMiddle)
        {
            Result = true;
        }
    }
    
    return(Result);
}

inline b32 IsExactMiddleOfCenter(rubiks_cube* Cube, 
                                 int CenterX,
                                 int CenterY)
{
    b32 Result = false;
    
    if(IsExactMiddleOfCenter(Cube, CenterX) && 
       IsExactMiddleOfCenter(Cube, CenterY))
    {
        Result = true;
    }
    
    return(Result);
}

INTERNAL_FUNCTION int FindFirstUnsolvedRowOnCenterColumn(rubiks_cube* Cube,
                                                         int SideIndex,
                                                         int UnsolvedCenterColumn,
                                                         u8 SolveColor)
{
    Assert(Cube->Dim >= 4);
    
    int UnsolvedRow = -1;
    
    u8* Side = Cube->Sides[SideIndex];
    
    for(int RowIndex = 0;
        RowIndex < Cube->Dim - 2;
        RowIndex++)
    {
        int x = 1 + UnsolvedCenterColumn;
        int y = 1 + RowIndex;
        
        u8 CheckColor = Side[y * Cube->Dim + x];
        
        b32 IsMiddleColumn = (UnsolvedCenterColumn == ((Cube->Dim - 2) / 2));
        b32 IsMiddleRow = (RowIndex == ((Cube->Dim - 2) / 2));
        
        b32 IsExactMiddleOfSide = false;
        
        if(Cube->Dim % 2 == 1)
        {
            if(IsMiddleColumn && IsMiddleRow)
            {
                IsExactMiddleOfSide = true;
            }
        }
        
        // NOTE(Dima): Skip checking if this is the center of cube on oddd cubes
        if((CheckColor != SolveColor) && !IsExactMiddleOfSide)
        {
            UnsolvedRow = RowIndex;
            
            break;
        }
    }
    
    return(UnsolvedRow);
}

enum rub_comb_resolve_rotation
{
    RubCombResolveRotation_None,
    RubCombResolveRotation_CCW,
    RubCombResolveRotation_Double,
    RubCombResolveRotation_CW,
};

struct rub_side_coords
{
    int X;
    int Y;
};

inline rub_side_coords GetSideCoordsAfterRotation(rubiks_cube* Cube, 
                                                  int InitX,
                                                  int InitY,
                                                  u32 ResolveType)
{
    rub_side_coords Result = {};
    
    int x = InitX;
    int y = InitY;
    
    int xVar = Cube->Dim - 1 - x;
    int yVar = Cube->Dim - 1 - y;
    
#if 0    
    int PossibleCombinations[4][2] =
    {
        {x, y},
        {yVar, x},
        {xVar, yVar},
        {y, xVar},
    };
#else
    int PossibleCombinations[4][2] =
    {
        {x, y},
        {y, xVar},
        {xVar, yVar},
        {yVar, x},
    };
#endif
    
    int* Comb = PossibleCombinations[ResolveType];
    
    Result.X = Comb[0];
    Result.Y = Comb[1];
    
    return(Result);
}

INTERNAL_FUNCTION int FindMatchingCenterPiece(rubiks_cube* Cube, 
                                              int UnsolvedX,
                                              int UnsolvedY,
                                              u8 Color,
                                              int* MatchingPieceX,
                                              int* MatchingPieceY,
                                              u32* ResolveRotationType,
                                              b32 LastTwoCenters)
{
    Assert(Cube->Dim >= 4);
    
    int FoundSide = -1;
    
    int SearchSides[6] = 
    {
        RubSide_Front,
        RubSide_Down,
        RubSide_Up,
        RubSide_Back,
        RubSide_Right,
        RubSide_Left,
    };
    
    int CenterDim = Cube->Dim - 2;
    
    int VariantCenterX = CenterDim - UnsolvedX - 1;
    int VariantCenterY = CenterDim - UnsolvedY - 1;
    
    int x = UnsolvedX + 1;
    int y = UnsolvedY + 1;
    
    int xVar = VariantCenterX + 1;
    int yVar = VariantCenterY + 1;
    
    // NOTE(Dima): This order of multiplication because we solve by columns
    int UnsolvedPieceCheckIndex = x * Cube->Dim;
    
    if(LastTwoCenters)
    {
        UnsolvedPieceCheckIndex += y;
    }
    
    int PossibleCombinations[4][2] =
    {
        {x, y},
        {yVar, x},
        {xVar, yVar},
        {y, xVar},
    };
    
    b32 Found = false;
    int FoundX = -1;
    int FoundY = -1;
    int FoundComb = -1;
    
    for(int i = 0; i < 6; i++)
    {
        int SideIndex = SearchSides[i];
        
        // NOTE(Dima): No need to search on solved sides
        if(!SideCenterIsSolved(Cube, SideIndex))
        {
            u8* Side = Cube->Sides[SideIndex];
            
            b32 SideIsUp = (SideIndex == RubSide_Up);
            b32 SideIsFront = (SideIndex == RubSide_Front);
            
            // TODO(Dima): Not to forget center pieces on odd sized cubes
            
            for(int CombIndex = 0;
                CombIndex < 4;
                CombIndex++)
            {
                int* Comb = PossibleCombinations[CombIndex];
                
                int CombX = Comb[0];
                int CombY = Comb[1];
                int CombinationPieceIndex = CombY * Cube->Dim + CombX;
                
                // NOTE(Dima): This used for checking if we try to take already solved piece on up side
                int CombinationPieceCheckIndex = CombX * Cube->Dim + CombY;
                
                u8 CombinationFetchedColor = Side[CombinationPieceIndex];
                
                if(CombinationFetchedColor == Color)
                {
                    Found = true;
                    
                    if(SideIsUp)
                    {
                        // NOTE(Dima): We can not take pieces that are already solved on up side
                        if(CombinationPieceCheckIndex < UnsolvedPieceCheckIndex)
                        {
                            Found = false;
                        }
                    }
                    
                    // NOTE(Dima): We also can not take pieces that are already solved in the column
                    if(SideIsFront && !LastTwoCenters)
                    {
                        if((x == CombX) && 
                           (CombY < y))
                        {
                            Found = false;
                        }
                    }
                    
                    if(Found)
                    {
                        FoundX = CombX;
                        FoundY = CombY;
                        FoundComb = CombIndex;
                        
                        break;
                    }
                }
            }
            
            if(Found)
            {
                FoundSide = SideIndex;
                
                break;
            }
        }
    }
    
    *MatchingPieceX = FoundX;
    *MatchingPieceY = FoundY;
    *ResolveRotationType = FoundComb;
    
    return(FoundSide);
}

INTERNAL_FUNCTION void MoveSideCentersToRightPlace(rubiks_cube* Cube)
{
    // NOTE(Dima): We'll reach this point only if cube is >= 3 dimensions
    if(CanStartSolvingState(Cube, RubState_MoveCenters_Green))
    {
        u8 SideWithGreen = FindSideWithCenterColor(Cube, RubiksColor_Green);
        switch(SideWithGreen)
        {
            // NOTE(Dima): Front
            case RubiksDirection_Front:
            {
                // NOTE(Dima): Change the state to next
                //Cube->SolvingState = RubState_MoveCenters_White;
            }break;
            
            // NOTE(Dima): Back
            case RubiksDirection_Back:
            {
                RubCom_Y(Cube, false);
                RubCom_Y(Cube, false);
            }break;
            
            // NOTE(Dima): Up
            case RubiksDirection_Up:
            {
                RubCom_X(Cube, true);
            }break;
            
            
            // NOTE(Dima): Down
            case RubiksDirection_Down:
            {
                RubCom_X(Cube, false);
            }break;
            
            
            // NOTE(Dima): Left
            case RubiksDirection_Left:
            {
                RubCom_Y(Cube, true);
            }break;
            
            
            // NOTE(Dima): Right
            case RubiksDirection_Right:
            {
                RubCom_Y(Cube, false);
            }break;
        }
        
        // NOTE(Dima): Adding command to transition to next state
        AddFinishStateCommand(Cube, RubState_MoveCenters_White);
    }
    
    if(CanStartSolvingState(Cube, RubState_MoveCenters_White))
    {
        u8 SideWithWhite = FindSideWithCenterColor(Cube, RubiksColor_White);
        switch(SideWithWhite)
        {
            // NOTE(Dima): Up
            case RubiksDirection_Up:
            {
                // NOTE(Dima): Change state to next
                //Cube->SolvingState = RubState_MakeWhiteCross;
            }break;
            
            // NOTE(Dima): Down
            case RubiksDirection_Down:
            {
                RubCom_Z(Cube, false);
                RubCom_Z(Cube, false);
            }break;
            
            
            // NOTE(Dima): Left
            case RubiksDirection_Left:
            {
                RubCom_Z(Cube, false);
            }break;
            
            // NOTE(Dima): Right
            case RubiksDirection_Right:
            {
                RubCom_Z(Cube, true);
            }break;
            
            default:
            {
                // NOTE(Dima): It's impossible to get here since Front and Back should be already solved
                InvalidCodePath;
            }break;
        }
        
        AddFinishStateCommand(Cube, RubState_MakeWhiteCross);
    }
}

INTERNAL_FUNCTION void MoveEdgeCentersToRightPlace(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_MakeWhiteCross))
    {
        int SideColors[4] = 
        {
            GetSideCenterFirstColor(Cube, RubSide_Right),
            GetSideCenterFirstColor(Cube, RubSide_Back),
            GetSideCenterFirstColor(Cube, RubSide_Left),
            GetSideCenterFirstColor(Cube, RubSide_Front),
        };
        
        int CompareResults[4] = 
        {
            RubEdge_UpRight,
            RubEdge_UpBack,
            RubEdge_UpLeft,
            RubEdge_UpFront,
        };
        
        b32 AllCorrectlyPlaced = true;
        for(int i = 0; i < 4; i++)
        {
            b32 TestInverted;
            int TestFoundEdge = FindEdgeIndexForColors(Cube,
                                                       RubiksColor_White,
                                                       SideColors[i],
                                                       &TestInverted);
            
            b32 Fits = (CompareResults[i] == TestFoundEdge);
            if(Fits && TestInverted)
            {
                Fits = false;
            }
            
            if(!Fits)
            {
                AllCorrectlyPlaced = false;
                break;
            }
        }
        
        if(AllCorrectlyPlaced)
        {
            //STOP(Dima):
            AddFinishStateCommand(Cube, RubState_FlipToYellowAfterCross);
        }
        else
        {
            u8 CurrentWallColor = GetSideCenterFirstColor(Cube, RubiksDirection_Front);
            
            b32 Inverted;
            int FoundEdge = FindEdgeIndexForColors(Cube,
                                                   RubiksColor_White, 
                                                   CurrentWallColor,
                                                   &Inverted);
            
            switch(FoundEdge)
            {
                case RubEdge_UpFront:
                {
                    if(!Inverted)
                    {
                        // NOTE(Dima): Edge is already solved so move to next one
                        
                        RubCom_Y(Cube, false);
                    }
                    else
                    {
                        RubCom_F(Cube, false);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, false);
                        RubCom_U(Cube, false);
                    }
                }break;
                
                case RubEdge_UpRight:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_R(Cube, true);
                        RubCom_F(Cube, true);
                    }
                }break;
                
                case RubEdge_UpBack:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_B(Cube, true);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube);
                    }
                }break;
                
                case RubEdge_UpLeft:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube, true);
                    }
                    else
                    {
                        RubCom_L(Cube);
                        RubCom_F(Cube);
                    }
                }break;
                
                case RubEdge_DownFront:
                {
                    if(!Inverted)
                    {
                        RubCom_F(Cube);
                        RubCom_F(Cube);
                    }
                    else
                    {
                        // NOTE(Dima): Bringing to DownRight case
                        RubCom_D(Cube);
                    }
                }break;
                
                case RubEdge_DownRight:
                {
                    if(!Inverted)
                    {
                        // NOTE(Dima): To Down-Front
                        RubCom_D(Cube, true);
                    }
                    else
                    {
                        RubCom_R(Cube);
                        RubCom_F(Cube, true);
                        RubCom_R(Cube, true);
                    }
                }break;
                
                case RubEdge_DownBack:
                {
                    if(!Inverted)
                    {
                        // NOTE(Dima): Bringing to DownFront case
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                    else
                    {
                        // NOTE(Dima): Bringing to DownRight case
                        RubCom_D(Cube, true);
                    }
                }break;
                
                case RubEdge_DownLeft:
                {
                    if(!Inverted)
                    {
                        // NOTE(Dima): Bringing to DownFront case
                        RubCom_D(Cube);
                    }
                    else
                    {
                        // NOTE(Dima): Bringing to DownRight case 
                        RubCom_D(Cube, true);
                        RubCom_D(Cube, true);
                    }
                }break;
                
                case RubEdge_RightBack:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_B(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube);
                    }
                }break;
                
                case RubEdge_BackLeft:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                        RubCom_L(Cube);
                        RubCom_U(Cube, true);
                    }
                    else
                    {
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_B(Cube, true);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                    }
                }break;
                
                case RubEdge_LeftFront:
                {
                    if(!Inverted)
                    {
                        RubCom_F(Cube);
                    }
                    else
                    {
                        RubCom_U(Cube);
                        RubCom_L(Cube, true);
                        RubCom_U(Cube, true);
                    }
                }break;
                
                case RubEdge_FrontRight:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube, true);
                        RubCom_R(Cube);
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_F(Cube, true);
                    }
                }break;
            }
            
            
            AddFinishStateCommand(Cube, RubState_MakeWhiteCross);
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_FlipToYellowAfterCross))
    {
        AddFinishStateCommand(Cube, RubState_SolveFirstLayer);
    }
}

INTERNAL_FUNCTION b32 FirstLayerIsSolved(rubiks_cube* Cube)
{
    // NOTE(Dima): Checking if first layer is OK
    int SideColors[4] = 
    {
        GetSideCenterFirstColor(Cube, RubSide_Front),
        GetSideCenterFirstColor(Cube, RubSide_Right),
        GetSideCenterFirstColor(Cube, RubSide_Back),
        GetSideCenterFirstColor(Cube, RubSide_Left),
    };
    
    int CompareResults[4] = 
    {
        RubCorn_UpFrontRight,
        RubCorn_UpRightBack,
        RubCorn_UpBackLeft,
        RubCorn_UpLeftFront,
    };
    
    b32 FirstLayerCorrect = true;
    for(int i = 0; i < 4; i++)
    {
        int CornRotation;
        int TestFoundEdge = FindCornerIndexForColors(Cube,
                                                     RubiksColor_White,
                                                     SideColors[i],
                                                     SideColors[(i + 1) % 4],
                                                     &CornRotation);
        
        b32 ThisCorrect = (CompareResults[i] == TestFoundEdge);
        if(ThisCorrect && (CornRotation != RubCornRot_Correct))
        {
            ThisCorrect = false;
        }
        
        if(!ThisCorrect)
        {
            FirstLayerCorrect = false;
            break;
        }
    }
    
    return(FirstLayerCorrect);
}

INTERNAL_FUNCTION void SolveFirstLayer(rubiks_cube* Cube)
{
    MoveSideCentersToRightPlace(Cube);
    
    MoveEdgeCentersToRightPlace(Cube);
    
    if(CanStartSolvingState(Cube, RubState_SolveFirstLayer))
    {
        OutputLog("Solve first layer");
        
        b32 FirstLayerCorrect = FirstLayerIsSolved(Cube);
        
        if(FirstLayerCorrect)
        {
            // STOP(Dima)
#if 1            
            RubCom_Z(Cube);
            RubCom_Z(Cube);
            
            AddFinishStateCommand(Cube, RubState_SolveSecondLayer);
#endif
        }
        else
        {
            u8 CurrentWallColor = GetSideCenterFirstColor(Cube, RubiksDirection_Front);
            u8 RightWallColor = GetSideCenterFirstColor(Cube, RubiksDirection_Right);
            
            int CornerRotation;
            int CornerIndex = FindCornerIndexForColors(Cube, 
                                                       RubiksColor_White,
                                                       CurrentWallColor,
                                                       RightWallColor,
                                                       &CornerRotation);
            
            switch(CornerIndex)
            {
                case RubCorn_UpFrontRight:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        // NOTE(Dima): Corner is already solved so move to next one
                        RubCom_Y(Cube, false);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_R(Cube, true);
                        RubCom_D(Cube, true);
                        RubCom_R(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_F(Cube);
                        RubCom_D(Cube);
                        RubCom_F(Cube, true);
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                }break;
                
                case RubCorn_UpRightBack:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_B(Cube, true);
                        RubCom_D(Cube, true);
                        RubCom_B(Cube);
                        RubCom_D(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_R(Cube);
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                        RubCom_R(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_D(Cube);
                        RubCom_R(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_R(Cube);
                        RubCom_D(Cube);
                        RubCom_R(Cube, true);
                        RubCom_D(Cube);
                    }
                }break;
                
                case RubCorn_UpBackLeft:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_B(Cube);
                        RubCom_D(Cube);
                        RubCom_B(Cube, true);
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_L(Cube, true);
                        RubCom_D(Cube, true);
                        RubCom_L(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_B(Cube);
                        RubCom_D(Cube);
                        RubCom_B(Cube, true);
                    }
                }break;
                
                case RubCorn_UpLeftFront:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_L(Cube);
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                        RubCom_L(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_F(Cube, true);
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                        RubCom_F(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_L(Cube);
                        RubCom_D(Cube);
                        RubCom_L(Cube, true);
                        RubCom_D(Cube, true);
                    }
                }break;
                
                case RubCorn_DownRightFront:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_D(Cube);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_D(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_D(Cube, true);
                    }
                }break;
                
                case RubCorn_DownBackRight:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_R(Cube);
                        RubCom_D(Cube, true);
                        RubCom_R(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_F(Cube);
                        RubCom_D(Cube, true);
                        RubCom_F(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                }break;
                
                case RubCorn_DownLeftBack:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_D(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_D(Cube, true);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_D(Cube);
                    }
                }break;
                
                case RubCorn_DownFrontLeft:
                {
                    if(CornerRotation == RubCornRot_Correct)
                    {
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                    else if(CornerRotation == RubCornRot_SecondAtFirst)
                    {
                        RubCom_D(Cube);
                        RubCom_D(Cube);
                    }
                    else if(CornerRotation == RubCornRot_ThirdAtFirst)
                    {
                        RubCom_R(Cube, true);
                        RubCom_D(Cube);
                        RubCom_R(Cube);
                    }
                }break;
            }
            
            AddFinishStateCommand(Cube, RubState_SolveFirstLayer);
        }
    }
}

INTERNAL_FUNCTION void SolveSecondLayer(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_SolveSecondLayer))
    {
        OutputLog("Solve second layer");
        
        int SideColors[4] = 
        {
            GetSideCenterFirstColor(Cube, RubSide_Right),
            GetSideCenterFirstColor(Cube, RubSide_Back),
            GetSideCenterFirstColor(Cube, RubSide_Left),
            GetSideCenterFirstColor(Cube, RubSide_Front),
        };
        
        int CompareResults[4] = 
        {
            RubEdge_RightBack,
            RubEdge_BackLeft,
            RubEdge_LeftFront,
            RubEdge_FrontRight,
        };
        
        b32 AllCorrectlyPlaced = true;
        for(int i = 0; i < 4; i++)
        {
            b32 TestInverted;
            int TestFoundEdge = FindEdgeIndexForColors(Cube,
                                                       SideColors[i],
                                                       SideColors[(i + 1) % 4],
                                                       &TestInverted);
            
            b32 Fits = (CompareResults[i] == TestFoundEdge);
            if(Fits && TestInverted)
            {
                Fits = false;
            }
            
            if(!Fits)
            {
                AllCorrectlyPlaced = false;
                break;
            }
        }
        
        
        if(AllCorrectlyPlaced)
        {
            // STOP(Dima):
            AddFinishStateCommand(Cube, RubState_SolveYellowCross);
        }
        else
        {
            u8 CurrentWallColor = GetSideCenterFirstColor(Cube, RubSide_Front);
            u8 NextWallColor = GetSideCenterFirstColor(Cube, RubSide_Right);
            
            b32 Inverted;
            int Test = FindEdgeIndexForColors(Cube,
                                              CurrentWallColor,
                                              NextWallColor,
                                              &Inverted);
            
            switch(Test)
            {
                case RubEdge_UpFront:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube, true);
                    }
                    else
                    {
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube, true);
                        RubCom_F(Cube, true);
                        RubCom_U(Cube);
                        RubCom_F(Cube);
                    }
                }break;
                
                case RubEdge_UpRight:
                {
                    if(!Inverted)
                    {
                        RubCom_R(Cube, true);
                        RubCom_F(Cube, true);
                        RubCom_R(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_F(Cube);
                    }
                    else
                    {
                        RubCom_U(Cube);
                    }
                }break;
                
                case RubEdge_UpBack:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                    }
                }break;
                
                case RubEdge_UpLeft:
                {
                    if(!Inverted)
                    {
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                    }
                    else
                    {
                        RubCom_U(Cube, true);
                    }
                }break;
                
                case RubEdge_FrontRight:
                {
                    if(!Inverted)
                    {
                        RubCom_Y(Cube);
                    }
                    else
                    {
                        RubCom_F(Cube, true);
                        RubCom_U(Cube, true);
                        RubCom_F(Cube);
                        RubCom_U(Cube);
                        
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube);
                        
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube);
                        
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                    }
                }break;
                
                
                case RubEdge_RightBack:
                {
                    if(!Inverted)
                    {
                        RubCom_B(Cube);
                        RubCom_U(Cube);
                        RubCom_B(Cube, true);
                        RubCom_U(Cube, true);
                        
                        RubCom_F(Cube, true);
                        RubCom_U(Cube);
                        RubCom_F(Cube);
                        
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                        
                        RubCom_B(Cube);
                        RubCom_U(Cube, true);
                        RubCom_B(Cube, true);
                    }
                    else
                    {
                        RubCom_R(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_R(Cube);
                    }
                }break;
                
                case RubEdge_BackLeft:
                {
                    if(!Inverted)
                    {
                        RubCom_B(Cube, true);
                        RubCom_U(Cube);
                        RubCom_B(Cube);
                        
                        RubCom_U(Cube);
                        RubCom_L(Cube);
                        RubCom_U(Cube, true);
                        RubCom_L(Cube, true);
                        
                        RubCom_U(Cube, true);
                    }
                    else
                    {
                        RubCom_B(Cube, true);
                        RubCom_U(Cube, true);
                        RubCom_B(Cube);
                        
                        RubCom_F(Cube, true);
                        RubCom_U(Cube);
                        RubCom_F(Cube);
                        
                        RubCom_B(Cube, true);
                        RubCom_U(Cube);
                        RubCom_B(Cube);
                        
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube, true);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_R(Cube);
                        RubCom_U(Cube, true);
                        RubCom_R(Cube, true);
                    }
                }break;
                
                
                case RubEdge_LeftFront:
                {
                    if(!Inverted)
                    {
                        RubCom_L(Cube, true);
                        RubCom_U(Cube);
                        RubCom_L(Cube);
                        RubCom_U(Cube);
                        RubCom_L(Cube, true);
                        RubCom_U(Cube);
                        RubCom_L(Cube);
                        RubCom_F(Cube);
                        RubCom_U(Cube, true);
                        RubCom_F(Cube, true);
                    }
                    else
                    {
                        RubCom_F(Cube);
                        RubCom_F(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_F(Cube);
                        RubCom_F(Cube);
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        RubCom_F(Cube);
                        RubCom_F(Cube);
                    }
                }break;
                
            }
            
            AddFinishStateCommand(Cube, RubState_SolveSecondLayer);
        }
    }
}

INTERNAL_FUNCTION void SolveYellowCross(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_SolveYellowCross))
    {
        OutputLog("Solve yellow Cross");
        
        b32 YellowCrossIsSolved = true;
        
        int SideFaceGetEdgeMethod[4] = 
        {
            RubSideEdge_Bottom,
            RubSideEdge_Right,
            RubSideEdge_Top,
            RubSideEdge_Left,
        };
        
        u8 IsYellow[4];
        
        for(int i = 0; i < 4; i++)
        {
            u8 EdgeColor = GetEdgeCenterFirstColor(Cube, RubSide_Up, SideFaceGetEdgeMethod[i], false);
            IsYellow[i] = (EdgeColor == RubiksColor_Yellow);
        }
        
        for(int i = 0; i < 4; i++)
        {
            if(!IsYellow[i])
            {
                YellowCrossIsSolved = false;
                break;
            }
        }
        
        if(YellowCrossIsSolved)
        {
            AddFinishStateCommand(Cube, RubState_SolveYellowEdges_Match);
        }
        else
        {
            if(IsYellow[0] &&
               IsYellow[2])
            {
                // NOTE(Dima): Vertical bar
                RubCom_U(Cube);
            }
            else if(IsYellow[1] &&
                    IsYellow[3])
            {
                // NOTE(Dima): Horizontal bar
                RubCom_F(Cube);
                
                RubCom_R(Cube);
                RubCom_U(Cube);
                RubCom_R(Cube, true);
                RubCom_U(Cube, true);
                
                RubCom_F(Cube, true);
            }
            else if(IsYellow[3] && IsYellow[2])
            {
                // NOTE(Dima): TopLeft angle
                RubCom_F(Cube);
                
                RubCom_U(Cube);
                RubCom_R(Cube);
                RubCom_U(Cube, true);
                RubCom_R(Cube, true);
                
                RubCom_F(Cube, true);
            }
            else if(IsYellow[1] && IsYellow[2])
            {
                // NOTE(Dima): TopRight angle
                RubCom_U(Cube, true);
            }
            else if(IsYellow[1] && IsYellow[0])
            {
                // NOTE(Dima): RightBot angle
                RubCom_U(Cube);
                RubCom_U(Cube);
            }
            else if(IsYellow[0] && IsYellow[3])
            {
                RubCom_U(Cube);
            }
            else if(!IsYellow[0] &&
                    !IsYellow[1] &&
                    !IsYellow[2] &&
                    !IsYellow[3])
            {
                // NOTE(Dima): Horizontal bar
                RubCom_F(Cube);
                
                RubCom_R(Cube);
                RubCom_U(Cube);
                RubCom_R(Cube, true);
                RubCom_U(Cube, true);
                
                RubCom_F(Cube, true);
            }
            
            AddFinishStateCommand(Cube, RubState_SolveYellowCross);
        }
    }
}

INTERNAL_FUNCTION void SolveYellowEdgesMatch(rubiks_cube* Cube, u32 MatchState, u32 FinishState)
{
    if(CanStartSolvingState(Cube, MatchState))
    {
        u8 FrontCenterColor = GetSideCenterFirstColor(Cube, RubSide_Front);
        
        int SideWalls[4] = 
        {
            RubSide_Front,
            RubSide_Right,
            RubSide_Back,
            RubSide_Left,
        };
        
        int FoundSide = -1;
        for(int WallIndex = 0; WallIndex < 4; WallIndex++)
        {
            u8 EdgeColor = GetEdgeCenterFirstColor(Cube, SideWalls[WallIndex], RubSideEdge_Top, false);
            
            if(FrontCenterColor == EdgeColor)
            {
                FoundSide = WallIndex;
                break;
            }
        }
        
        Assert(FoundSide != -1);
        
        switch(FoundSide)
        {
            case 0:
            {
                // NOTE(Dima): Nothing
            }break;
            
            case 1:
            {
                RubCom_U(Cube);
            }break;
            
            case 2:
            {
                RubCom_U(Cube);
                RubCom_U(Cube);
            }break;
            
            case 3:
            {
                RubCom_U(Cube, true);
            }break;
        }
        
        AddFinishStateCommand(Cube, FinishState);
    }
}

INTERNAL_FUNCTION void SolveYellowEdges(rubiks_cube* Cube)
{
    SolveYellowEdgesMatch(Cube, 
                          RubState_SolveYellowEdges_Match,
                          RubState_SolveYellowEdges);
    
    
    if(CanStartSolvingState(Cube, RubState_SolveYellowEdges))
    {
        OutputLog("Solve Yellow Edges");
        
        int SideWalls[4] = 
        {
            RubSide_Front,
            RubSide_Right,
            RubSide_Back,
            RubSide_Left,
        };
        
        // NOTE(Dima): Getting wall colors
        u8 WallCentersColors[4];
        for(int WallIndex = 0; WallIndex < 4; WallIndex++)
        {
            WallCentersColors[WallIndex] = GetSideCenterFirstColor(Cube, SideWalls[WallIndex]);
        }
        
        // NOTE(Dima): Walking and checking if all color are correctly matched 
        b32 AllEdgesCorrect = true;
        for(int WallIndex = 0; WallIndex < 4; WallIndex++)
        {
            u8 EdgeColor = GetEdgeCenterFirstColor(Cube, SideWalls[WallIndex], RubSideEdge_Top, false);
            
            if(WallCentersColors[WallIndex] != EdgeColor)
            {
                AllEdgesCorrect = false;
                break;
            }
        }
        
        if(AllEdgesCorrect)
        {
            AddFinishStateCommand(Cube, RubState_PlaceYellowCorners);
        }
        else
        {
            AddFinishStateCommand(Cube, RubState_SolveYellowEdges_PrepareAndExecute);
            
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_SolveYellowEdges_PrepareAndExecute))
    {
        u8 LeftUpEdgeColor = GetEdgeCenterFirstColor(Cube, RubSide_Left, RubSideEdge_Top, false);
        u8 LeftCenterColor = GetSideCenterFirstColor(Cube, RubSide_Left);
        
        if(LeftCenterColor == LeftUpEdgeColor)
        {
            RubCom_Y(Cube);
            RubCom_Y(Cube);
        }
        else
        {
            RubCom_Y(Cube, true);
        }
        
        RubCom_R(Cube);
        RubCom_U(Cube);
        RubCom_R(Cube, true);
        RubCom_U(Cube);
        RubCom_R(Cube);
        RubCom_U(Cube);
        RubCom_U(Cube);
        RubCom_R(Cube, true);
        RubCom_U(Cube);
        
        AddFinishStateCommand(Cube, RubState_SolveYellowEdges_Match);
    }
}

INTERNAL_FUNCTION void PlaceYellowCorners(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_PlaceYellowCorners))
    {
        OutputLog("Place yellow Corners");
        
        int CompareResults[4] = 
        {
            RubCorn_UpFrontRight,
            RubCorn_UpRightBack,
            RubCorn_UpBackLeft,
            RubCorn_UpLeftFront,
        };
        b32 Results[4];
        
        int SideWalls[4] = 
        {
            RubSide_Front,
            RubSide_Right,
            RubSide_Back,
            RubSide_Left,
        };
        
        // NOTE(Dima): Getting wall colors
        u8 WallCentersColors[4];
        for(int WallIndex = 0; WallIndex < 4; WallIndex++)
        {
            WallCentersColors[WallIndex] = GetSideCenterFirstColor(Cube, SideWalls[WallIndex]);
        }
        
        int FirstCorrectIndex = -1;
        int CountCorrect = 0;
        for(int i = 0;
            i < 4;
            i++)
        {
            int Rotation;
            int FindResult = FindCornerIndexForColors(Cube, 
                                                      RubiksColor_Yellow,
                                                      WallCentersColors[i],
                                                      WallCentersColors[(i + 1) % 4],
                                                      &Rotation);
            
            Results[i] = (FindResult == CompareResults[i]);
            
            if(Results[i])
            {
                if(FirstCorrectIndex == -1)
                {
                    FirstCorrectIndex = i;
                }
                
                CountCorrect++;
            }
        }
        
        if(CountCorrect == 4)
        {
            // STOP(Dima):
            AddFinishStateCommand(Cube, RubState_OrientYellowCorners_Prepare);
        }
        else
        {
            if(FirstCorrectIndex != -1)
            {
                switch(FirstCorrectIndex)
                {
                    case 0:
                    {
                        // NOTE(Dima): Nothing to do
                    }break;
                    
                    case 1:
                    {
                        RubCom_Y(Cube);
                    }break;
                    
                    case 2:
                    {
                        RubCom_Y(Cube);
                        RubCom_Y(Cube);
                    }break;
                    
                    case 3:
                    {
                        RubCom_Y(Cube, true);
                    }break;
                }
            }
            
            RubCom_U(Cube);
            RubCom_R(Cube);
            RubCom_U(Cube, true);
            RubCom_L(Cube, true);
            RubCom_U(Cube);
            RubCom_R(Cube, true);
            RubCom_U(Cube, true);
            RubCom_L(Cube);
            
            AddFinishStateCommand(Cube, RubState_PlaceYellowCorners);
        }
    }
}

INTERNAL_FUNCTION int OrientFindPlace(rubiks_cube* Cube,
                                      b32* AllCorrect)
{
    
#if 1    
    int CompareResults[4] = 
    {
        RubCorn_UpFrontRight,
        RubCorn_UpRightBack,
        RubCorn_UpBackLeft,
        RubCorn_UpLeftFront,
    };
#endif
    
    int SideWalls[4] = 
    {
        RubSide_Front,
        RubSide_Right,
        RubSide_Back,
        RubSide_Left,
    };
    
    // NOTE(Dima): Getting wall colors
    u8 WallColors[4];
    for(int WallIndex = 0; WallIndex < 4; WallIndex++)
    {
        WallColors[WallIndex] = GetEdgeCenterFirstColor(Cube, SideWalls[WallIndex], 
                                                        RubSideEdge_Top, false);
    }
    
    int FirstIncorrectIndex = -1;
    
    *AllCorrect = true;
    for(int i = 0;
        i < 4;
        i++)
    {
        int Rotation;
        int FindResult = FindCornerIndexForColors(Cube, 
                                                  RubiksColor_Yellow,
                                                  WallColors[i],
                                                  WallColors[(i + 1) % 4],
                                                  &Rotation);
        
        Assert(FindResult == CompareResults[i]);
        
        b32 IsCorrect = (Rotation == 0);
        
        if(!IsCorrect)
        {
            *AllCorrect = false;
            
            if(FirstIncorrectIndex == -1)
            {
                FirstIncorrectIndex = i;
            }
            
            break;
        }
    }
    
    return(FirstIncorrectIndex);
}

INTERNAL_FUNCTION void OrientYellowCorners(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_OrientYellowCorners_Prepare))
    {
        b32 AllCorrect;
        int FirstIncorrectIndex = OrientFindPlace(Cube, &AllCorrect);
        
        if(AllCorrect)
        {
            AddFinishStateCommand(Cube, RubState_Solved);
        }
        else
        {
            Assert(FirstIncorrectIndex != -1);
            
            switch(FirstIncorrectIndex)
            {
                case 0:
                {
                    // NOTE(Dima): Nothing to do
                }break;
                
                case 1:
                {
                    RubCom_Y(Cube);
                }break;
                
                case 2:
                {
                    RubCom_Y(Cube);
                    RubCom_Y(Cube);
                }break;
                
                case 3:
                {
                    RubCom_Y(Cube, true);
                }break;
            }
            
            RubCom_R(Cube, true);
            RubCom_D(Cube, true);
            RubCom_R(Cube);
            RubCom_D(Cube);
            
            RubCom_R(Cube, true);
            RubCom_D(Cube, true);
            RubCom_R(Cube);
            RubCom_D(Cube);
            
            AddFinishStateCommand(Cube, RubState_OrientYellowCorners);
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_OrientYellowCorners))
    {
        OutputLog("Orient Yellow Corners");
        
        b32 AllCorrect;
        int FirstIncorrectIndex = OrientFindPlace(Cube, &AllCorrect);
        
        if(AllCorrect)
        {
            AddFinishStateCommand(Cube, RubState_OrientYellowCorners_MatchInTheEnd);
        }
        else
        {
            switch(FirstIncorrectIndex)
            {
                case 0:
                {
                    // NOTE(Dima): Nothing to do
                }break;
                
                case 1:
                {
                    RubCom_U(Cube);
                }break;
                
                case 2:
                {
                    RubCom_U(Cube);
                    RubCom_U(Cube);
                }break;
                
                case 3:
                {
                    RubCom_U(Cube, true);
                }break;
            }
            
            RubCom_R(Cube, true);
            RubCom_D(Cube, true);
            RubCom_R(Cube);
            RubCom_D(Cube);
            
            RubCom_R(Cube, true);
            RubCom_D(Cube, true);
            RubCom_R(Cube);
            RubCom_D(Cube);
            
            AddFinishStateCommand(Cube, RubState_OrientYellowCorners);
        }
    }
    
    SolveYellowEdgesMatch(Cube, 
                          RubState_OrientYellowCorners_MatchInTheEnd,
                          RubState_Solved);
}


INTERNAL_FUNCTION b32 SideCenterWithColorIsSolved(rubiks_cube* Cube, u8 Color)
{
    b32 Result = true;
    
    int SideIndex = -1;
    
    for(int SideIndex = 0;
        SideIndex < 6;
        SideIndex++)
    {
        u8* Side = Cube->Sides[SideIndex];
        
        if(Side[0] == Color)
        {
            // NOTE(Dima): Found side with this color.
            SideIndex = SideIndex;
            
            break;
        }
    }
    
    if(SideIndex == -1)
    {
        Result = false;
    }
    else
    {
        Result = SideCenterIsSolved(Cube, SideIndex);
    }
    
    return(Result);
}

inline int GetUndoTypeForResolve(rubiks_cube* Cube,
                                 u32 CombToResolve)
{
    int Result = RubCombResolveRotation_None;
    
    if(CombToResolve == RubCombResolveRotation_CW)
    {
        Result = RubCombResolveRotation_CCW;
    }
    else if (CombToResolve == RubCombResolveRotation_CCW)
    {
        Result = RubCombResolveRotation_CW;
    }
    else if(CombToResolve == RubCombResolveRotation_Double)
    {
        Result = RubCombResolveRotation_Double;
    }
    else
    {
        // NOTE(Dima): Do nothing
    }
    
    return(Result);
}

INTERNAL_FUNCTION int ResolveMatchingWithCommand(rubiks_cube* Cube,
                                                 rubiks_rotate_side_command_func* RotateCommand,
                                                 u32 CombToResolve)
{
    if(CombToResolve == RubCombResolveRotation_CW)
    {
        RotateCommand(Cube, false);
    }
    else if (CombToResolve == RubCombResolveRotation_CCW)
    {
        RotateCommand(Cube, true);
    }
    else if(CombToResolve == RubCombResolveRotation_Double)
    {
        RotateCommand(Cube, false);
        RotateCommand(Cube, false);
    }
    else
    {
        // NOTE(Dima): Do nothing
    }
    
    int Result = GetUndoTypeForResolve(Cube, CombToResolve);
    
    return(Result);
}

INTERNAL_FUNCTION void UndoResolve(rubiks_cube* Cube,
                                   rubiks_rotate_side_command_func* RotateCommand,
                                   u32 PrevResolveType)
{
    ResolveMatchingWithCommand(Cube, RotateCommand, PrevResolveType);
}

INTERNAL_FUNCTION b32 RotateToProtectHorzMove(rubiks_cube* Cube,
                                              int PieceSideX,
                                              int PieceSideY,
                                              rubiks_rotate_side_command_func* RotateCommand,
                                              b32 BeginProtect)
{
    b32 RotatedCounterClockwise = false;
    
    b32 FirstHalfVertical = PieceSideY < (Cube->Dim / 2);
    b32 FirstHalfHorizontal = PieceSideX < (Cube->Dim / 2);
    
    if(FirstHalfHorizontal)
    {
        if(FirstHalfVertical)
        {
            RotatedCounterClockwise = BeginProtect;
            RotateCommand(Cube, RotatedCounterClockwise);
        }
        else
        {
            RotatedCounterClockwise = !BeginProtect;
            RotateCommand(Cube, RotatedCounterClockwise);
        }
    }
    else
    {
        if(FirstHalfVertical)
        {
            RotatedCounterClockwise = !BeginProtect;
            RotateCommand(Cube, RotatedCounterClockwise);
        }
        else
        {
            RotatedCounterClockwise = BeginProtect;
            RotateCommand(Cube, RotatedCounterClockwise);
        }
    }
    
    return(RotatedCounterClockwise);
}

INTERNAL_FUNCTION b32 RotateToProtectVertMove(rubiks_cube* Cube,
                                              int PieceSideX,
                                              int PieceSideY,
                                              rubiks_rotate_side_command_func* RotateCommand,
                                              b32 BeginProtect)
{
    b32 Result = RotateToProtectHorzMove(Cube, 
                                         PieceSideX,
                                         PieceSideY,
                                         RotateCommand,
                                         !BeginProtect);
    
    return(Result);
}

INTERNAL_FUNCTION void SolveResolvedDown(rubiks_cube* Cube,
                                         int TargetX,
                                         int TargetY)
{
    
    RubCom_F(Cube); // Important to rotate clockwise here
    RubCom_D(Cube);
    
    // Because of this computations
    // NOTE(Dima): Getting side coords after fortation for target piece on Front side
    u32 FrontRotation = RubCombResolveRotation_CW;
    rub_side_coords RotCoords = GetSideCoordsAfterRotation(Cube,
                                                           TargetX,
                                                           TargetY,
                                                           FrontRotation);
    
    RubCom_M_Manual(Cube, RotCoords.X, true);
    
    RotateToProtectVertMove(Cube, 
                            RotCoords.X,
                            RotCoords.Y,
                            RubCom_F,
                            true);
    
    RubCom_M_Manual(Cube, RotCoords.X, false);
    
    RotateToProtectVertMove(Cube, 
                            RotCoords.X,
                            RotCoords.Y,
                            RubCom_F,
                            false);
    
    RubCom_F(Cube, true); //Back to original place
}

INTERNAL_FUNCTION void SolveResolvedBack(rubiks_cube* Cube,
                                         int TargetX,
                                         int TargetY)
{
    RubCom_E_Manual(Cube, InvertCoord(Cube, TargetY));
    RubCom_E_Manual(Cube, InvertCoord(Cube, TargetY));
    
    RotateToProtectHorzMove(Cube, 
                            TargetX,
                            TargetY,
                            RubCom_F,
                            true);
    
    RubCom_E_Manual(Cube, InvertCoord(Cube, TargetY));
    RubCom_E_Manual(Cube, InvertCoord(Cube, TargetY));
    
    RotateToProtectHorzMove(Cube,
                            TargetX,
                            TargetY,
                            RubCom_F,
                            false);
}

INTERNAL_FUNCTION void SolveResolvedLeft(rubiks_cube* Cube,
                                         int TargetX,
                                         int TargetY)
{
    RubCom_E_Manual(Cube, 
                    InvertCoord(Cube, TargetY));
    
    RotateToProtectHorzMove(Cube, 
                            TargetX,
                            TargetY,
                            RubCom_F,
                            true);
    
    RubCom_E_Manual(Cube,
                    InvertCoord(Cube, TargetY),
                    true);
    
    RotateToProtectHorzMove(Cube, 
                            TargetX,
                            TargetY,
                            RubCom_F,
                            false);
}

INTERNAL_FUNCTION void SolveResolvedRight(rubiks_cube* Cube,
                                          int TargetX,
                                          int TargetY)
{
    RubCom_E_Manual(Cube, 
                    InvertCoord(Cube, TargetY), 
                    true);
    
    RotateToProtectHorzMove(Cube, 
                            TargetX,
                            TargetY,
                            RubCom_F,
                            true);
    
    RubCom_E_Manual(Cube, 
                    InvertCoord(Cube, TargetY), 
                    false);
    
    RotateToProtectHorzMove(Cube, 
                            TargetX,
                            TargetY,
                            RubCom_F,
                            false);
}

INTERNAL_FUNCTION void ApplyCommutator(rubiks_cube* Cube,
                                       int TargetX, 
                                       int TargetY)
{
    // 1
    RubCom_M_Manual(Cube, TargetX, true);
    
    // 2
    b32 RotatedCCW = RotateToProtectVertMove(Cube, 
                                             TargetX,
                                             TargetY,
                                             RubCom_U,
                                             true);
    
    u32 UpRotationType = RubCombResolveRotation_CW;
    if(RotatedCCW)
    {
        UpRotationType = RubCombResolveRotation_CCW;
    }
    
    rub_side_coords RotUp = GetSideCoordsAfterRotation(Cube,
                                                       TargetX,
                                                       TargetY,
                                                       UpRotationType);
    
    Assert(TargetX != RotUp.X);
    
    // 3
    RubCom_M_Manual(Cube, RotUp.X, true);
    
    // 4
    RubCom_U(Cube, !RotatedCCW);
    
    // Undo 1
    RubCom_M_Manual(Cube, TargetX);
    
    // Undo 2
    RubCom_U(Cube, RotatedCCW);
    
    // Undo 3
    RubCom_M_Manual(Cube, RotUp.X);
    
    // Undo final 
    RubCom_U(Cube, !RotatedCCW);
    //RubCom_F(Cube, !RotatedCCW);
    
}

INTERNAL_FUNCTION void SolveSideCenter(rubiks_cube* Cube,
                                       u32 SolveState, 
                                       u32 SolveToUpState,
                                       u32 HelperToFrontState,
                                       u32 FuncState,
                                       u32 ExitState,
                                       u8 CenterColor,
                                       u8 HelperCenterColor)
{
    // NOTE(Dima): Checking if need to solve
    if(CanStartSolvingState(Cube, SolveState))
    {
        if(SideCenterWithColorIsSolved(Cube, CenterColor))
        {
            // NOTE(Dima): This side is solved. Go ahead and solve other sides
            AddFinishStateCommand(Cube, ExitState);
        }
        else
        {
            AddFinishStateCommand(Cube, SolveToUpState);
        }
    }
    
    // NOTE(Dima): Placing unsolved side on Top
    if(CanStartSolvingState(Cube, SolveToUpState))
    {
        if(SolveState == RubState_SolvingCenters_SolveWhite)
        {
            
            int UnsolvedSide = -1;
            
            if((Cube->Dim % 2 == 1) &&
               (Cube->Dim >= 4))
            {
                // NOTE(Dima): We can move white side to up. But first - find it.
                UnsolvedSide = FindSideWithCenterColor(Cube, CenterColor);
            }
            else
            {
                // NOTE(Dima): We can move any side to up. But first - find it.
                UnsolvedSide = FindFirstSideWithUnsolvedCenter(Cube);
            }
            
            // NOTE(Dima): Need to solve sides. First - rotate unsolved side to the Up
            switch(UnsolvedSide)
            {
                case RubSide_Front:
                {
                    RubCom_X(Cube);
                }break;
                
                case RubSide_Back:
                {
                    RubCom_X(Cube, true);
                }break;
                
                case RubSide_Right:
                {
                    RubCom_Z(Cube, true);
                }break;
                
                case RubSide_Left:
                {
                    RubCom_Z(Cube);
                }break;
                
                case RubSide_Up:
                {
                    // NOTE(Dima): No need to rotate as unsolved side is already on Up
                }break;
                
                case RubSide_Down:
                {
                    RubCom_Z(Cube);
                    RubCom_Z(Cube);
                }break;
            }
        }
        else
        {
            
            switch(CenterColor)
            {
                case RubiksColor_White:
                {
                    InvalidCodePath;
                }break;
                
                case RubiksColor_Yellow:
                {
                    RubCom_Z(Cube);
                    RubCom_Z(Cube);
                }break;
                
                case RubiksColor_Green:
                {
                    RubCom_X(Cube);
                    RubCom_Y(Cube);
                }break;
                
                case RubiksColor_Orange:
                {
                    RubCom_X(Cube);
                }break;
                
                case RubiksColor_Blue:
                {
                    RubCom_X(Cube);
                }break;
            }
        }
        
        AddFinishStateCommand(Cube, HelperToFrontState);
    }
    
    // NOTE(Dima): Placing helper side on front
    if(CanStartSolvingState(Cube, HelperToFrontState))
    {
        if(SolveState == RubState_SolvingCenters_SolveWhite)
        {
            int UnsolvedWall;
            if((Cube->Dim % 2 == 1) &&
               (Cube->Dim >= 4))
            {
                UnsolvedWall = FindSideWithCenterColor(Cube, HelperCenterColor);
            }
            else
            {
                UnsolvedWall = FindFirstWallWithUnsolvedCenter(Cube);
            }
            
            switch(UnsolvedWall)
            {
                case RubSide_Front:
                {
                    // NOTE(Dima): No need to rotate
                }break;
                
                case RubSide_Back:
                {
                    RubCom_Y(Cube);
                    RubCom_Y(Cube);
                }break;
                
                case RubSide_Right:
                {
                    RubCom_Y(Cube);
                }break;
                
                case RubSide_Left:
                {
                    RubCom_Y(Cube, true);
                }break;
                
                default:
                {
                    // NOTE(Dima): Helper side should always be adjasent to what we try to solve now.
                    InvalidCodePath;
                }break;
            }
        }
        
        AddFinishStateCommand(Cube, FuncState);
    }
    
    // NOTE(Dima): Actual solving
    if(CanStartSolvingState(Cube, FuncState))
    {
        b32 Solved = SideCenterIsSolved(Cube, RubSide_Up);
        
        if(Solved)
        {
            AddFinishStateCommand(Cube, ExitState);
        }
        else
        {
            int UnsolvedCentersCount = GetCountOfSidesWithUnsolvedCenters(Cube);
            b32 LastTwoCenters = UnsolvedCentersCount == 2;
            Assert(UnsolvedCentersCount >= 2);
            
            int UnsolvedColumn = FindFirstUnsolvedColumnOnCenter(Cube, 
                                                                 RubSide_Up, 
                                                                 CenterColor);
            
            // NOTE(Dima): If we got here, it means we can't have all columns solved.
            Assert(UnsolvedColumn != -1);
            
            int SideIndexForRowSearch = RubSide_Front;
            if(LastTwoCenters)
            {
                SideIndexForRowSearch = RubSide_Up;
            }
            int UnsolvedRow = FindFirstUnsolvedRowOnCenterColumn(Cube,
                                                                 SideIndexForRowSearch,
                                                                 UnsolvedColumn,
                                                                 CenterColor);
            
            b32 IsExactMiddle = IsExactMiddleOfCenter(Cube, UnsolvedColumn, UnsolvedRow);
            b32 IsFirstRowOnCenter = (UnsolvedRow == 0);
            
            int SolvePieceSideX = UnsolvedColumn + 1;
            int SolvePieceSideY = UnsolvedRow + 1;
            
            if(UnsolvedRow != -1)
            {
                // NOTE(Dima): Colomn is still unsolved - need to solve it
                
                // NOTE(Dima): These should be in side coordinates (not in center coords)
                int MatchingPieceX;
                int MatchingPieceY;
                u32 CombinationResolve;
                
                // NOTE(Dima): Find matching piece on unsolved sides
                int MatchingPieceSide = FindMatchingCenterPiece(Cube,
                                                                UnsolvedColumn,
                                                                UnsolvedRow,
                                                                CenterColor,
                                                                &MatchingPieceX,
                                                                &MatchingPieceY,
                                                                &CombinationResolve,
                                                                LastTwoCenters);
                
                // NOTE(Dima): All returned values should be valid
                Assert(MatchingPieceSide != -1);
                Assert(MatchingPieceX != -1);
                Assert(MatchingPieceY != -1);
                
                // NOTE(Dima): We need to rotate found piece so that we can insert it to our column
                
                // TODO(Dima): Resolve
                
                rubiks_rotate_side_command_func* SideRotateCommands[6];
                SideRotateCommands[RubSide_Up] = RubCom_U;
                SideRotateCommands[RubSide_Down] = RubCom_D;
                SideRotateCommands[RubSide_Right] = RubCom_R;
                SideRotateCommands[RubSide_Left] = RubCom_L;
                SideRotateCommands[RubSide_Front] = RubCom_F;
                SideRotateCommands[RubSide_Back] = RubCom_B;
                
                // NOTE(Dima): In these cases we just resolve
                rubiks_rotate_side_command_func* RotateCommand = SideRotateCommands[MatchingPieceSide];
                
                switch(MatchingPieceSide)
                {
                    case RubSide_Back:
                    case RubSide_Left:
                    case RubSide_Right:
                    case RubSide_Down:
                    {
                        ResolveMatchingWithCommand(Cube, RotateCommand, CombinationResolve);
                        
                        if(MatchingPieceSide == RubSide_Back)
                        {
                            SolveResolvedBack(Cube, 
                                              SolvePieceSideX,
                                              SolvePieceSideY);
                        }
                        else if(MatchingPieceSide == RubSide_Left)
                        {
                            SolveResolvedLeft(Cube,
                                              SolvePieceSideX,
                                              SolvePieceSideY);
                        }
                        else if(MatchingPieceSide == RubSide_Right)
                        {
                            SolveResolvedRight(Cube,
                                               SolvePieceSideX,
                                               SolvePieceSideY);
                        }
                        else if(MatchingPieceSide == RubSide_Down)
                        {
                            SolveResolvedDown(Cube,
                                              SolvePieceSideX,
                                              SolvePieceSideY);
                        }
                        
                    }break;
                    
                    case RubSide_Front:
                    {
                        if((UnsolvedRow == 0) && (!LastTwoCenters))
                        {
                            
                            // NOTE(Dima): If no elements are solved in our bar, so just resolve
                            ResolveMatchingWithCommand(Cube, RotateCommand, CombinationResolve);
                        }
                        else if(!SideCenterIsSolved(Cube, RubSide_Down))
                        {
                            if(CombinationResolve != RubCombResolveRotation_None)
                            {
                                // NOTE(Dima): Matching resolution
                                
                                b32 NeedToPreRotate = false;
                                u32 FrontRotation = RubCombResolveRotation_None;
                                if(MatchingPieceX == SolvePieceSideX)
                                {
                                    NeedToPreRotate = true;
                                    
                                    RubCom_F(Cube, true);
                                    FrontRotation = RubCombResolveRotation_CCW;
                                }
                                
                                rub_side_coords RotFront = GetSideCoordsAfterRotation(Cube,
                                                                                      MatchingPieceX,
                                                                                      MatchingPieceY,
                                                                                      FrontRotation);
                                
                                RubCom_M_Manual(Cube, RotFront.X);
                                
                                RotateToProtectVertMove(Cube,
                                                        RotFront.X,
                                                        RotFront.Y,
                                                        RubCom_D,
                                                        true);
                                
                                RubCom_M_Manual(Cube, RotFront.X, true);
                                
                                RotateToProtectVertMove(Cube,
                                                        RotFront.X,
                                                        RotFront.Y,
                                                        RubCom_D,
                                                        false);
                                
                                if(NeedToPreRotate)
                                {
                                    RubCom_F(Cube, false);
                                    RubCom_D(Cube, false);
                                }
                                
                                ResolveMatchingWithCommand(Cube, 
                                                           SideRotateCommands[RubSide_Down],
                                                           CombinationResolve);
                            }
                            
                            // NOTE(Dima): Solve
                            SolveResolvedDown(Cube,
                                              SolvePieceSideX,
                                              SolvePieceSideY);
                        }
                        else if(!SideCenterIsSolved(Cube, RubSide_Right))
                        {
                            if(CombinationResolve != RubCombResolveRotation_None)
                            {
                                b32 NeedToPreRotate = false;
                                u32 FrontRotation = RubCombResolveRotation_None;
                                if(MatchingPieceY < SolvePieceSideY)
                                {
                                    NeedToPreRotate = true;
                                    
                                    RubCom_F(Cube, true);
                                    FrontRotation = RubCombResolveRotation_CCW;
                                }
                                
                                rub_side_coords RotFront = GetSideCoordsAfterRotation(Cube,
                                                                                      MatchingPieceX,
                                                                                      MatchingPieceY,
                                                                                      FrontRotation);
                                
                                RubCom_E_Manual(Cube, InvertCoord(Cube, RotFront.Y));
                                
                                RotateToProtectHorzMove(Cube, 
                                                        RotFront.X,
                                                        RotFront.Y,
                                                        RubCom_R,
                                                        true);
                                
                                RubCom_E_Manual(Cube, InvertCoord(Cube, RotFront.Y), true);
                                
                                RotateToProtectHorzMove(Cube,
                                                        RotFront.X,
                                                        RotFront.Y,
                                                        RubCom_R,
                                                        false);
                                
                                if(NeedToPreRotate)
                                {
                                    RubCom_F(Cube, false);
                                    RubCom_R(Cube, false);
                                }
                                
                                ResolveMatchingWithCommand(Cube, 
                                                           SideRotateCommands[RubSide_Right],
                                                           CombinationResolve);
                                
                            }
                            
                            // NOTE(Dima): Solve
                            SolveResolvedRight(Cube,
                                               SolvePieceSideX,
                                               SolvePieceSideY);
                        }
                        else
                        {
                            // NOTE(Dima): Piece is found on front. 
                            
                            // So just resolve piece on front
                            
                            // And then push to the right place on top
                            Assert(LastTwoCenters);
                            
                            ResolveMatchingWithCommand(Cube,
                                                       RotateCommand,
                                                       CombinationResolve);
                            
                            ApplyCommutator(Cube, 
                                            SolvePieceSideX,
                                            SolvePieceSideY);
                        }
                        
                    }break;
                    
                    case RubSide_Up:
                    {
                        if(LastTwoCenters)
                        {
                            // NOTE(Dima): Moving piece to down
                            b32 XIsEqual = MatchingPieceX == SolvePieceSideX;
                            b32 YIsEqual = MatchingPieceY == SolvePieceSideY;
                            
                            Assert(!(XIsEqual && YIsEqual));
                            
                            ApplyCommutator(Cube, 
                                            MatchingPieceX,
                                            MatchingPieceY);
                            
                        }
                        else
                        {
                            int UndoResolveType = ResolveMatchingWithCommand(Cube, 
                                                                             SideRotateCommands[RubSide_Up],
                                                                             CombinationResolve);
                            
                            // NOTE(Dima): Moving piece to down
                            ApplyCommutator(Cube, 
                                            SolvePieceSideX,
                                            SolvePieceSideY);
                            
                            UndoResolve(Cube, 
                                        SideRotateCommands[RubSide_Up],
                                        UndoResolveType);
                        }
                    }break;
                }
            }
            else
            {
                if(!LastTwoCenters)
                {
                    
                    // NOTE(Dima): Need to insert this row into the side that we solve now
                    if(IsExactMiddleOfCenter(Cube, UnsolvedColumn))
                    {
                        RubCom_F(Cube);
                        
                        RubCom_M_Manual(Cube, SolvePieceSideX);
                        
                        RubCom_F(Cube, true);
                        
                        RubCom_M_Manual(Cube, SolvePieceSideX, true);
                    }
                    else
                    {
                        b32 SolvedMoreThanHalf = SolvePieceSideX >= (Cube->Dim / 2);
                        
                        if(SolvedMoreThanHalf)
                        {
                            RubCom_U(Cube);
                            RubCom_U(Cube);
                        }
                        
                        RubCom_M_Manual(Cube, SolvePieceSideX, true);
                        
                        RubCom_U(Cube);
                        RubCom_U(Cube);
                        
                        RubCom_M_Manual(Cube, SolvePieceSideX, false);
                        
                        if(!SolvedMoreThanHalf)
                        {
                            RubCom_U(Cube);
                            RubCom_U(Cube);
                        }
                    }
                }
            }
            
            AddFinishStateCommand(Cube, FuncState);
        }
    }
}

INTERNAL_FUNCTION void SolveSidesCenters(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_SolvingCenters))
    {
        OutputLog("Solve Sides Centers");
        
        int UnsolvedSide = FindFirstSideWithUnsolvedCenter(Cube);
        
        if(UnsolvedSide == -1)
        {
            // NOTE(Dima): All sides are solved
            // STOP(Dima)
            AddFinishStateCommand(Cube, RubState_SolvingEdgesCenters);
        }
        else
        {
            AddChangeSpeedCommand(Cube, 0.0f, true);
            AddFinishStateCommand(Cube, RubState_SolvingCenters_SolveWhite);
        }
    }
    
    // NOTE(Dima): White
    SolveSideCenter(Cube,
                    RubState_SolvingCenters_SolveWhite,
                    RubState_SolvingCenters_SolveWhite_ToUp,
                    RubState_SolvingCenters_SolveWhite_GreenToFront,
                    RubState_SolvingCenters_SolveWhite_Func,
                    RubState_SolvingCenters_SolveYellow,
                    RubiksColor_White,
                    RubiksColor_Green);
    
    // NOTE(Dima): Yellow
    SolveSideCenter(Cube,
                    RubState_SolvingCenters_SolveYellow,
                    RubState_SolvingCenters_SolveYellow_ToUp,
                    RubState_SolvingCenters_SolveYellow_GreenToFront,
                    RubState_SolvingCenters_SolveYellow_Func,
                    RubState_SolvingCenters_SolveGreen,
                    RubiksColor_Yellow,
                    RubiksColor_Green);
    
    // NOTE(Dima): Green
    SolveSideCenter(Cube,
                    RubState_SolvingCenters_SolveGreen,
                    RubState_SolvingCenters_SolveGreen_ToUp,
                    RubState_SolvingCenters_SolveGreen_OrangeToFront,
                    RubState_SolvingCenters_SolveGreen_Func,
                    RubState_SolvingCenters_SolveOrange,
                    RubiksColor_Green,
                    RubiksColor_Orange);
    
    // NOTE(Dima): Orange
    SolveSideCenter(Cube,
                    RubState_SolvingCenters_SolveOrange,
                    RubState_SolvingCenters_SolveOrange_ToUp,
                    RubState_SolvingCenters_SolveOrange_BlueToFront,
                    RubState_SolvingCenters_SolveOrange_Func,
                    RubState_SolvingCenters_SolveBlue,
                    RubiksColor_Orange,
                    RubiksColor_Blue);
    
    // NOTE(Dima): Blue
    SolveSideCenter(Cube,
                    RubState_SolvingCenters_SolveBlue,
                    RubState_SolvingCenters_SolveBlue_ToUp,
                    RubState_SolvingCenters_SolveBlue_RedToFront,
                    RubState_SolvingCenters_SolveBlue_Func,
                    RubState_SolvingCenters,
                    RubiksColor_Blue,
                    RubiksColor_Red);
    
}

// NOTE(Dima): If ToFrontRight is false - then ToLeftFront
// Type = 2 - To Up-Front
// Type = 1 - To Front-Right
// Type = 0 - To Left-Front
INTERNAL_FUNCTION void MoveEdgeCenterTo(rubiks_cube* Cube,
                                        int MoveType,
                                        u32 EdgeFrom)
{
    b32 ToFrontRight = MoveType == 1;
    b32 ToUpFront = MoveType == 2;
    
    switch(EdgeFrom)
    {
        case RubEdge_UpFront:
        {
            if(ToFrontRight)
            {
                RubCom_F(Cube);
            }
            else if(ToUpFront)
            {
                // NOTE(Dima): Nothing to do
            }
            else
            {
                RubCom_U(Cube);
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_UpRight:
        {
            if(ToFrontRight)
            {
                RubCom_R(Cube, true);
            }
            else if(ToUpFront)
            {
                RubCom_U(Cube);
            }
            else
            {
                RubCom_U(Cube);
                RubCom_U(Cube);
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_UpBack:
        {
            if(ToFrontRight)
            {
                RubCom_U(Cube);
                RubCom_R(Cube, true);
            }
            else if(ToUpFront)
            {
                RubCom_U(Cube);
                RubCom_U(Cube);
            }
            else
            {
                RubCom_U(Cube, true);
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_UpLeft:
        {
            if(ToFrontRight)
            {
                RubCom_U(Cube, true);
                RubCom_F(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_U(Cube, true);
            }
            else
            {
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_DownFront:
        {
            if(ToFrontRight)
            {
                RubCom_F(Cube, true);
            }
            else if(ToUpFront)
            {
                RubCom_F(Cube);
                RubCom_F(Cube);
            }
            else
            {
                RubCom_D(Cube, true);
                RubCom_L(Cube, true);
            }
        }break;
        
        case RubEdge_DownRight:
        {
            if(ToFrontRight)
            {
                RubCom_R(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_D(Cube, true);
                RubCom_F(Cube);
                RubCom_F(Cube);
            }
            else
            {
                RubCom_D(Cube, true);
                RubCom_D(Cube, true);
                RubCom_L(Cube, true);
            }
        }break;
        
        case RubEdge_DownBack:
        {
            if(ToFrontRight)
            {
                RubCom_D(Cube, true);
                RubCom_R(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_D(Cube);
                RubCom_D(Cube);
                RubCom_F(Cube);
                RubCom_F(Cube);
            }
            else
            {
                RubCom_D(Cube);
                RubCom_L(Cube, true);
            }
        }break;
        
        case RubEdge_DownLeft:
        {
            if(ToFrontRight)
            {
                RubCom_D(Cube);
                RubCom_F(Cube, true);
            }
            else if(ToUpFront)
            {
                RubCom_D(Cube);
                RubCom_F(Cube);
                RubCom_F(Cube);
            }
            else
            {
                RubCom_L(Cube, true);
            }
        }break;
        
        case RubEdge_FrontRight:
        {
            if(ToFrontRight)
            {
                // NOTE(Dima): Do nothing
            }
            else if(ToUpFront)
            {
                RubCom_F(Cube, true);
            }
            else
            {
                InvalidCodePath;
            }
        }break;
        
        case RubEdge_RightBack:
        {
            if(ToFrontRight)
            {
                RubCom_R(Cube);
                RubCom_R(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_R(Cube, true);
                RubCom_U(Cube);
            }
            else
            {
                RubCom_B(Cube);
                RubCom_B(Cube);
                
                RubCom_L(Cube);
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_BackLeft:
        {
            if(ToFrontRight)
            {
                RubCom_B(Cube);
                RubCom_B(Cube);
                
                RubCom_R(Cube);
                RubCom_R(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_L(Cube);
                RubCom_U(Cube, true);
            }
            else
            {
                RubCom_L(Cube);
                RubCom_L(Cube);
            }
        }break;
        
        case RubEdge_LeftFront:
        {
            if(ToFrontRight)
            {
                RubCom_F(Cube);
                RubCom_F(Cube);
            }
            else if(ToUpFront)
            {
                RubCom_F(Cube);
            }
            else
            {
                // NOTE(Dima): Nothing to do
            }
        }break;
    }
    
}

INTERNAL_FUNCTION void FlipLeftFrontEdge(rubiks_cube* Cube)
{
    RubCom_F(Cube);
    RubCom_U(Cube);
    RubCom_F(Cube, true);
    RubCom_L(Cube);
    RubCom_F(Cube, true);
    RubCom_L(Cube, true);
    RubCom_F(Cube);
}

INTERNAL_FUNCTION void FlipRightFrontEdge(rubiks_cube* Cube)
{
    RubCom_R(Cube);
    RubCom_U(Cube);
    RubCom_R(Cube, true);
    RubCom_F(Cube);
    RubCom_R(Cube, true);
    RubCom_F(Cube, true);
    RubCom_R(Cube);
}

INTERNAL_FUNCTION void SolveEdgeCenters(rubiks_cube* Cube)
{
    
    if(CanStartSolvingState(Cube, RubState_SolvingEdgesCenters))
    {
        OutputLog("Solve Edge Centers");
        
        int UnsolvedEdge = FindFirstEdgeWithUnsolvedCenter(Cube);
        
        if(UnsolvedEdge == -1)
        {
            // NOTE(Dima): No unsolved edges are left - Go agead to solve rest of the cube
            // STOP(Dima)
            AddFinishStateCommand(Cube, RubState_SolvingParityErrors);
        }
        else
        {
            // 1. Find unsolved edge and move it to front-right
            // 2. Find edge with corresponding piece and move it to left-front
            // 3. Move piece front left-front to right-front. And repeat.
            
            // NOTE(Dima): Moving edge to Front-Right
            MoveEdgeCenterTo(Cube, 1, UnsolvedEdge);
            
            AddFinishStateCommand(Cube, RubState_SolvingEdgesCenters_FindMoveSrc);
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_SolvingEdgesCenters_FindMoveSrc))
    {
        b32 Inverted;
        
        u8 DstColor1;
        u8 DstColor2;
        GetEdgeColors(Cube,
                      RubEdge_FrontRight,
                      &DstColor1,
                      &DstColor2);
        
        // NOTE(Dima): We'll search all edges except of front-right because it is destination
        int SearchEdges[11] =
        {
            RubEdge_LeftFront,
            
            RubEdge_UpFront,
            RubEdge_UpRight,
            RubEdge_UpBack,
            RubEdge_UpLeft,
            
            RubEdge_DownFront,
            RubEdge_DownRight,
            RubEdge_DownBack,
            RubEdge_DownLeft,
            
            RubEdge_RightBack,
            RubEdge_BackLeft,
        };
        
        // NOTE(Dima): Finding edge with piece
        int EdgeWithPiece = FindEdgeForColorsInternal(Cube, 
                                                      DstColor1, 
                                                      DstColor2, 
                                                      &Inverted,
                                                      SearchEdges,
                                                      11,
                                                      true);
        
        if(EdgeWithPiece == -1)
        {
            // NOTE(Dima): It means right-front edge is complete (But probably have parity errors);
            AddFinishStateCommand(Cube, RubState_SolvingEdgesCenters);
        }
        else
        {
            // NOTE(Dima): Otherwise that would mean we found element that is already should be solved
            Assert(!EdgeCenterIsSolved(Cube, RubEdge_FrontRight));
            
            MoveEdgeCenterTo(Cube, 0, EdgeWithPiece);
            
            AddFinishStateCommand(Cube, RubState_SolvingEdgesCenters_Solve);
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_SolvingEdgesCenters_Solve))
    {
        u8 SolveEdgeColor1;
        u8 SolveEdgeColor2;
        GetEdgeColors(Cube, RubEdge_FrontRight,
                      &SolveEdgeColor1,
                      &SolveEdgeColor2);
        
        u16 SolveComb = RubEdgeComb(SolveEdgeColor1, SolveEdgeColor2);
        
        int IndexOnLeftFront;
        b32 CheckIfColorsPresent = FindColorsOnEdgeCenter(Cube, 
                                                          RubEdge_LeftFront,
                                                          SolveEdgeColor1, 
                                                          SolveEdgeColor2,
                                                          true,
                                                          0,
                                                          &IndexOnLeftFront);
        
        // NOTE(Dima): They must present here because we should moved them here from previous state
        Assert(CheckIfColorsPresent);
        
        u8 Dst1Color1;
        u8 Dst1Color2;
        GetEdgeColorsN(Cube, RubEdge_FrontRight,
                       &Dst1Color1,
                       &Dst1Color2,
                       IndexOnLeftFront);
        
        u8 Dst2Color1;
        u8 Dst2Color2;
        GetEdgeColorsN(Cube, RubEdge_FrontRight,
                       &Dst2Color1,
                       &Dst2Color2,
                       InvertCoord(Cube, IndexOnLeftFront + 1) - 1);
        
        u8 SrcColor1;
        u8 SrcColor2;
        GetEdgeColorsN(Cube, RubEdge_LeftFront,
                       &SrcColor1,
                       &SrcColor2,
                       IndexOnLeftFront);
        
        u16 SrcComb = RubEdgeComb(SrcColor1, SrcColor2);
        u16 SrcComb2 = RubEdgeComb(SrcColor2, SrcColor1);
        
        Assert((SrcComb == SolveComb) || (SrcComb2 == SolveComb));
        
        u16 DstComb1 = RubEdgeComb(Dst1Color1, Dst1Color2);
        u16 DstComb2 = RubEdgeComb(Dst1Color2, Dst1Color1);
        
        u16 Dst2Comb1 = RubEdgeComb(Dst2Color1, Dst2Color2);
        u16 Dst2Comb2 = RubEdgeComb(Dst2Color2, Dst2Color1);
        
        b32 Dst1Available = false;
        b32 Dst2Available = false;
        
        if((SolveComb != DstComb1) &&
           (SolveComb != DstComb2))
        {
            Dst1Available = true;
        }
        
        if((SolveComb != Dst2Comb1) && 
           (SolveComb != Dst2Comb1))
        {
            Dst2Available = true;
        }
        
        Assert(Dst2Available || Dst1Available);
        
        b32 CanBeMatched = SrcComb == SolveComb;
        
        int IndexOnSrc = IndexOnLeftFront + 1;
        
        // NOTE(Dima): Two possible positions.
        int IndexOnDst1 = IndexOnSrc;
        int IndexOnDst2 = InvertCoord(Cube, IndexOnSrc);
        
        if(Dst1Available)
        {
            // NOTE(Dima): Solve to Dst1
            if(CanBeMatched)
            {
                FlipLeftFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst1), true);
                FlipLeftFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst1), false);
            }
            else
            {
                FlipLeftFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst2), false);
                FlipRightFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst2), true);
                
                if(Cube->Dim % 2 == 0)
                {
                    FlipRightFrontEdge(Cube);
                }
            }
        }
        else
        {
            // NOTE(Dima): Solve to Dst2
            if(CanBeMatched)
            {
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst1), false);
                FlipRightFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst1), true);
                
                if(Cube->Dim % 2 == 0)
                {
                    FlipRightFrontEdge(Cube);
                }
            }
            else
            {
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst2), true);
                FlipLeftFrontEdge(Cube);
                RubCom_E_Manual(Cube, InvertCoord(Cube, IndexOnDst2), false);
            }
        }
        
        AddFinishStateCommand(Cube, RubState_SolvingEdgesCenters_FindMoveSrc);
    }
}

INTERNAL_FUNCTION void SolveParityCommands(rubiks_cube* Cube,
                                           int IndexL,
                                           int IndexR)
{
    Assert(IndexL < IndexR);
    
    // NOTE(Dima): Section 1
    RubCom_M_Manual(Cube, IndexR);
    RubCom_U(Cube);
    RubCom_U(Cube);
    RubCom_M_Manual(Cube, IndexL);
    RubCom_F(Cube);
    RubCom_F(Cube);
    RubCom_M_Manual(Cube, IndexL, true);
    RubCom_F(Cube);
    RubCom_F(Cube);
    
    // NOTE(Dima): Section 2
    RubCom_M_Manual(Cube, IndexR);
    RubCom_M_Manual(Cube, IndexR);
    RubCom_U(Cube);
    RubCom_U(Cube);
    RubCom_M_Manual(Cube, IndexR, true);
    RubCom_U(Cube);
    RubCom_U(Cube);
    RubCom_M_Manual(Cube, IndexR, false);
    RubCom_U(Cube);
    RubCom_U(Cube);
    
    // NOTE(Dima): Section 3
    RubCom_F(Cube);
    RubCom_F(Cube);
    RubCom_M_Manual(Cube, IndexR);
    RubCom_M_Manual(Cube, IndexR);
    RubCom_F(Cube);
    RubCom_F(Cube);
}

INTERNAL_FUNCTION void SolveParityErrors(rubiks_cube* Cube)
{
    if(CanStartSolvingState(Cube, RubState_SolvingParityErrors))
    {
        OutputLog("Solve parity errors");
        
        int FirstParityIndexOnEdge;
        int UnsolvedEdge = FindFirstEdgeWithParityError(Cube, &FirstParityIndexOnEdge);
        
        b32 AllAreSolved = (UnsolvedEdge == -1);
        
        if(AllAreSolved && (FirstParityIndexOnEdge == -1))
        {
            OutputLog("Parities are solved");
            
            // STOP(Dima)
            AddChangeSpeedCommand(Cube, RUBIKS_SPEED_AVG * 0.4f, false);
            AddFinishStateCommand(Cube, RubState_MoveCenters_Green);
        }
        else
        {
            Assert(!AllAreSolved);
            MoveEdgeCenterTo(Cube, 2, UnsolvedEdge);
            
            AddFinishStateCommand(Cube, RubState_SolvingParityErrors_Solve);
        }
    }
    
    if(CanStartSolvingState(Cube, RubState_SolvingParityErrors_Solve))
    {
        int FirstParityIndexOnEdge;
        int UnsolvedEdge = FindFirstEdgeWithParityError(Cube, &FirstParityIndexOnEdge);
        
        int IndexL = FirstParityIndexOnEdge + 1;
        int IndexR = InvertCoord(Cube, IndexL);
        
        SolveParityCommands(Cube, IndexL, IndexR);
        
        AddFinishStateCommand(Cube, RubState_SolvingParityErrors);
    }
}

INTERNAL_FUNCTION void SolveCube(rubiks_cube* Cube)
{
    if(Cube->SolvingState != RubState_Solved)
    {
        SolveSidesCenters(Cube);
        
        SolveEdgeCenters(Cube);
        
        SolveParityErrors(Cube);
        
        SolveFirstLayer(Cube);
        
        SolveSecondLayer(Cube);
        
        SolveYellowCross(Cube);
        
        SolveYellowEdges(Cube);
        
        PlaceYellowCorners(Cube);
        
        OrientYellowCorners(Cube);
    }
}