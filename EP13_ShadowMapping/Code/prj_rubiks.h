#ifndef FLOWER_RUBIKS_H
#define FLOWER_RUBIKS_H

#include <unordered_map>
#include <unordered_set>

enum rubiks_axis
{
    RubiksAxis_X,
    RubiksAxis_Y,
    RubiksAxis_Z,
    
    RubiksAxis_Count,
};

GLOBAL_VARIABLE v3 RubiksAxisValue[RubiksAxis_Count] =
{
    V3_Left(),
    V3_Up(),
    V3_Forward(),
};

enum rubiks_cube_directions
{
    RubiksDirection_Up,
    RubiksDirection_Down,
    RubiksDirection_Left,
    RubiksDirection_Right,
    RubiksDirection_Front, 
    RubiksDirection_Back,
    
    RubiksDirection_Count,
};

enum rubiks_cube_sides
{
    RubSide_Up,
    RubSide_Down,
    RubSide_Left,
    RubSide_Right,
    RubSide_Front, 
    RubSide_Back,
    
    RubSide_Count,
};

GLOBAL_VARIABLE int RubiksRotOuterSides4Rotation[3][4] = 
{
    // NOTE(Dima): Right-Left look
    {RubiksDirection_Up, RubiksDirection_Back, RubiksDirection_Down, RubiksDirection_Front},
    
    // NOTE(Dima): Bottom-Top look
    {RubiksDirection_Front, RubiksDirection_Right, RubiksDirection_Back, RubiksDirection_Left},
    
    // NOTE(Dima): Front-Back look
    {RubiksDirection_Up, RubiksDirection_Right, RubiksDirection_Down, RubiksDirection_Left},
};

GLOBAL_VARIABLE b32 RubiksShouldInvertOuterGetting[3][4] = 
{
    {true, false, true, true},
    {false, false, false, false},
    {false, false, true, true},
};

GLOBAL_VARIABLE b32 RubiksShouldInvertLineGetting[3][4] = 
{
    {true, false, true, true},
    {true, true, true, true},
    {true, false, false, true},
};

GLOBAL_VARIABLE b32 RubiksLineIsRow[3][4] = 
{
    {false, false, false, false},
    {true, true, true, true},
    {true, false, true, false},
};

enum rubiks_side_edge_get
{
    RubSideEdge_Top,
    RubSideEdge_Right,
    RubSideEdge_Bottom,
    RubSideEdge_Left,
};

enum rubiks_side_corner_get
{
    RubSideGetCorn_TopLeft,
    RubSideGetCorn_TopRight,
    RubSideGetCorn_BotLeft,
    RubSideGetCorn_BotRight,
};

// NOTE(Dima): Here I store all edges centers info. And how to get it from Sides arrays
// NOTE(Dima): 12 edges. Each edge consists from 2 colors
GLOBAL_VARIABLE int RubiksEdgesSides[12][4] = 
{
    {RubSide_Up, RubSide_Front, RubSideEdge_Bottom, RubSideEdge_Top},
    {RubSide_Up, RubSide_Right, RubSideEdge_Right, RubSideEdge_Top},
    {RubSide_Up, RubSide_Back, RubSideEdge_Top, RubSideEdge_Top},
    {RubSide_Up, RubSide_Left, RubSideEdge_Left, RubSideEdge_Top},
    
    {RubSide_Down, RubSide_Front, RubSideEdge_Top, RubSideEdge_Bottom},
    {RubSide_Down, RubSide_Right, RubSideEdge_Right, RubSideEdge_Bottom},
    {RubSide_Down, RubSide_Back, RubSideEdge_Bottom, RubSideEdge_Bottom},
    {RubSide_Down, RubSide_Left, RubSideEdge_Left, RubSideEdge_Bottom},
    
    {RubSide_Front, RubSide_Right, RubSideEdge_Right, RubSideEdge_Left},
    {RubSide_Right, RubSide_Back, RubSideEdge_Right, RubSideEdge_Left},
    {RubSide_Back, RubSide_Left, RubSideEdge_Right, RubSideEdge_Left},
    {RubSide_Left, RubSide_Front, RubSideEdge_Right, RubSideEdge_Left},
};

GLOBAL_VARIABLE b32 RubiksInvertEdgesGet[12][2] = 
{
    false, false,
    true, false,
    true, false,
    false, false,
    
    false, false,
    false, false,
    true, false,
    true, false,
    
    false, false,
    false, false,
    false, false,
    false, false,
};

GLOBAL_VARIABLE int RubiksCornersPrep[8][6] = 
{
    // NOTE(Dima): TopCorners
    {
        // NOTE(Dima): UpFrontRight
        RubSide_Up, RubSide_Front, RubSide_Right, 
        RubSideGetCorn_BotRight, RubSideGetCorn_TopRight, RubSideGetCorn_TopLeft,
    },
    {
        // NOTE(Dima): UpRightBack
        RubSide_Up, RubSide_Right, RubSide_Back,
        RubSideGetCorn_TopRight, RubSideGetCorn_TopRight, RubSideGetCorn_TopLeft
    },
    {
        // NOTE(Dima): UpBackLeft
        RubSide_Up, RubSide_Back, RubSide_Left,
        RubSideGetCorn_TopLeft, RubSideGetCorn_TopRight, RubSideGetCorn_TopLeft,
    },
    {
        // NOTE(Dima): UpLeftFront
        RubSide_Up, RubSide_Left, RubSide_Front,
        RubSideGetCorn_BotLeft, RubSideGetCorn_TopRight, RubSideGetCorn_TopLeft,
    },
    
    // NOTE(Dima): Bottom corners
    {
        // NOTE(Dima): DownRightFront
        RubSide_Down, RubSide_Right, RubSide_Front,
        RubSideGetCorn_TopRight, RubSideGetCorn_BotLeft, RubSideGetCorn_BotRight, 
    },
    {
        // NOTE(Dima): DownBackRight
        RubSide_Down, RubSide_Back, RubSide_Right,
        RubSideGetCorn_BotRight, RubSideGetCorn_BotLeft, RubSideGetCorn_BotRight, 
    },
    {
        // NOTE(Dima): DownLeftBack
        RubSide_Down, RubSide_Left, RubSide_Back, 
        RubSideGetCorn_BotLeft, RubSideGetCorn_BotLeft, RubSideGetCorn_BotRight, 
    },
    {
        // NOTE(Dima): DownFrontLeft
        RubSide_Down, RubSide_Front, RubSide_Left,
        RubSideGetCorn_TopLeft, RubSideGetCorn_BotLeft, RubSideGetCorn_BotRight, 
    },
};

enum rubiks_corner_rotation
{
    RubCornRot_Correct,
    RubCornRot_SecondAtFirst, // NOTE(Dima): Second color takes place of first color
    RubCornRot_ThirdAtFirst // NOTE(Dima): Third color takes place of first color
};

enum rubiks_cube_corners
{
    RubCorn_UpFrontRight,
    RubCorn_UpRightBack,
    RubCorn_UpBackLeft,
    RubCorn_UpLeftFront,
    
    RubCorn_DownRightFront,
    RubCorn_DownBackRight,
    RubCorn_DownLeftBack,
    RubCorn_DownFrontLeft,
};

enum rubiks_cube_edges
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
    
    RubEdge_Count,
};

GLOBAL_VARIABLE const char* RubiksDirectionName[] = 
{
    "Up",
    "Down",
    "Left",
    "Right",
    "Front",
    "Back",
};

GLOBAL_VARIABLE v3 RubiksDirection[] = 
{
    V3_Up(),
    V3_Down(),
    V3_Left(),
    V3_Right(),
    V3_Back(),
    V3_Forward(),
};

enum rubiks_cube_colors
{
    RubiksColor_White,
    RubiksColor_Yellow,
    RubiksColor_Orange,
    RubiksColor_Red,
    RubiksColor_Green,
    RubiksColor_Blue,
    
    RubiksColor_Count,
};

GLOBAL_VARIABLE v4 RubiksColors[] = 
{
    ColorFromHex("#FFFFFF"),
    ColorFromHex("#FFD500"),
    ColorFromHex("#FF5800"),
    ColorFromHex("#B71234"),
    ColorFromHex("#009b48"),
    ColorFromHex("#0046ad"),
};

struct rubiks_visible_cubies
{
    m44* FinalTransform;
    m44* Transform;
    m44* AppliedRotation;
    v3* InitP;
    int* MeshIndex;
    int* InitIndices;
    
    int Count;
};

struct helper_rubiks_mesh
{
    std::vector<v2> P;
    std::vector<u32> Ids;
    
    std::vector<v2> PerimeterPoints;
};

struct rubiks_precomp_loop
{
    int LoopCubiesCount;
};

/*

0000000
0111110
0122210
0123210
0122210
0111110
0000000

rubiks_precomp_face will hold 3 loops of cubies indices on face for 0, 1, 2.
It will not hold the center loop for 3.

*/

struct rubiks_precomp_face
{
    int LoopCount;
    
    int** InitLoops;
    int** RotatedClockwise;
    int** RotatedCounterClockwise;
    
    int* LoopCubiesCount;
};

struct rubiks_rotate_face
{
    // NOTE(Dima): These arrays hold current indices, which are rotated
    int* Face;
    int* TempFace;
    
    // NOTE(Dima): This array stores indices into Current array to store rotated Current indices from Face
    int* IndicesInCurrent;
    
    int Count;
    int AxisIndex;
    v3 RotateOrigin;
};

typedef m44 rubiks_rotation_function(float Angle);
typedef rubiks_rotate_face* rubiks_get_face_function(struct rubiks_cube* Cube, int First, int Last);

struct rubiks_beginned_rotation
{
    rubiks_rotation_function* RotationMatrix;
    rubiks_get_face_function* GetFace;
    rubiks_rotate_face* RotateFace;
    int FirstFaceIndex;
    int LastFaceIndex;
    
    f32 InRotationTime;
    f32 TimeForRotation;
    b32 IsClockwise;
    
    m44 AppliedRotation;
    int AxisIndex;
};

enum rubiks_command_type
{
    RubiksCommand_Rotation,
    RubiksCommand_ChangeState,
    
    RubiksCommand_ResetSpeed,
    RubiksCommand_ResetSpeedDynamic,
};

struct rubiks_command
{
    int Axis;
    
    union
    {
        struct
        {
            u16 FirstFaceIndex;
            u16 LastFaceIndex;
            u8 IsClockwise;
        };
        
        struct 
        {
            u8 FinalState; //rubiks_cube_solving_state
        };
        
        struct
        {
            u8 IsDynamicSpeedChange;
            f32 TargetSpeed;
        };
    };
    
    u8 Type;
};

struct rubiks_cube
{
    int CubiesCount;
    // NOTE(Dima): This Cubies point to cubies in rubiks_visible_cubies 
    // NOTE(Dima): (-1 means invisible cubie and we should not update it
    int* CubiesToVisible;
    // NOTE(Dima): This arrays stores information about each cubie. Size is Dim * Dim * Dim
    rubiks_visible_cubies Visible;
    // NOTE(Dima): This array stores indices to cubies, which represent current state of cube.
    // NOTE(Dima): Size of this array is Dim * Dim * Dim
    int* Current;
    
    int NumTasks;
    task_memory_pool* TaskPool;
    
    // NOTE(Dima): Side faces colors. Each side stores colors indices Left-Right Top-Bottom when looking
    u8* Sides[RubiksColor_Count];
    u8* SideTemp;
    u8* RotColors;
    u8* RotColorsTemp;
    
    int Dim;
    
    f32 SideLen;
    f32 OneCubieLen;
    f32 HalfSideLen;
    f32 CubieOffset;
    
    f32 CurrentSpeed;
    f32 TargetSpeed;
    f32 BeginLerpSpeed;
    f32 LastSpeedChangeTime;
    
    helper_rubiks_mesh HelperStickerUp;
    helper_rubiks_mesh HelperStickerDown;
    
    f32 StickerHeight;
    
    helper_mesh StickersMeshes[RubiksDirection_Count];
    
    // NOTE(Dima): 26 = 6 faces + 12 edges + 8 angles
#define RUBIKS_MESHES_COUNT 26
    mesh UniqueMeshes[RUBIKS_MESHES_COUNT];
    // NOTE(Dima): This should be equal to 26
    int UniqueMeshesCount;
    // NOTE(Dima): x, y, z (one for each)
    mesh InnerMeshes[RubiksAxis_Count];
    b32 InnerMeshIsGenerated[RubiksAxis_Count];
    
    rubiks_beginned_rotation BeginnedRotation;
    // NOTE(Dima): This for updating transforms of face cubies
    rubiks_rotate_face RotateFace;
    rubiks_precomp_face PrecompFace;
    
    // NOTE(Dima): Commands
    int CommandsCount;
    rubiks_command* Commands;
    int DoIndex;
    int AddIndex;
    
    b32 IsRotatingNow;
    u32 SolvingState;
    b32 ExecutingSolvingNow;
};

enum rubiks_cube_solving_state
{
    RubState_Disassembled,
    RubState_Solved,
    
    RubState_SolvingCenters,
    // NOTE(Dima): Center White
    RubState_SolvingCenters_SolveWhite,
    RubState_SolvingCenters_SolveWhite_ToUp,
    RubState_SolvingCenters_SolveWhite_GreenToFront,
    RubState_SolvingCenters_SolveWhite_Func,
    
    // NOTE(Dima): Center Yellow
    RubState_SolvingCenters_SolveYellow,
    RubState_SolvingCenters_SolveYellow_ToUp,
    RubState_SolvingCenters_SolveYellow_GreenToFront,
    RubState_SolvingCenters_SolveYellow_Func,
    
    // NOTE(Dima): Center Green
    RubState_SolvingCenters_SolveGreen,
    RubState_SolvingCenters_SolveGreen_ToUp,
    RubState_SolvingCenters_SolveGreen_OrangeToFront,
    RubState_SolvingCenters_SolveGreen_Func,
    
    // NOTE(Dima): Center Orange
    RubState_SolvingCenters_SolveOrange,
    RubState_SolvingCenters_SolveOrange_ToUp,
    RubState_SolvingCenters_SolveOrange_BlueToFront,
    RubState_SolvingCenters_SolveOrange_Func,
    
    // NOTE(Dima): Center Blue & Red at a time
    RubState_SolvingCenters_SolveBlue,
    RubState_SolvingCenters_SolveBlue_ToUp,
    RubState_SolvingCenters_SolveBlue_RedToFront,
    RubState_SolvingCenters_SolveBlue_Func,
    
    RubState_SolvingEdgesCenters,
    RubState_SolvingEdgesCenters_FindMoveSrc,
    RubState_SolvingEdgesCenters_Solve,
    
    RubState_SolvingParityErrors,
    RubState_SolvingParityErrors_Solve,
    
    RubState_MoveCenters_Green,
    RubState_MoveCenters_White,
    RubState_MakeWhiteCross,
    
    RubState_FlipToYellowAfterCross,
    RubState_SolveFirstLayer,
    RubState_SolveSecondLayer,
    RubState_SolveYellowCross,
    
    RubState_SolveYellowEdges_Match,
    RubState_SolveYellowEdges_PrepareAndExecute,
    RubState_SolveYellowEdges,
    
    RubState_PlaceYellowCorners,
    
    RubState_OrientYellowCorners_Prepare,
    RubState_OrientYellowCorners,
    RubState_OrientYellowCorners_MatchInTheEnd,
};

//#define RUBIKS_TIME_FOR_ROTATION 10.0f
#define RUBIKS_TIME_TO_ACCELERATE_MIN 10.0f
#define RUBIKS_TIME_TO_ACCELERATE_MAX 16.0f
#define RUBIKS_TIME_FOR_ROTATION 1.0f

#define RUBIKS_MIN_DIM_HELP 3.0f
#define RUBIKS_MAX_DIM_HELP 50.0f

#define RUBIKS_SPEED_DEFAULT 50.0f
//#define RUBIKS_SPEED_DEFAULT 35.0f
//#define RUBIKS_SPEED_DEFAULT 1.0f

#define RUBIKS_SPEED_SLOW 1.0f
#define RUBIKS_SPEED_AVG 10.0f
#define RUBIKS_SPEED_FAST 100.0f
#define RUBIKS_SPEED_SUPER_FAST 1000.0f


enum rubiks_create_cube_flags
{
    RubCreateCube_RoundStickers = 1,
    RubCreateCube_StickerHaveWalls = 2,
    
    RubCreateCube_Default = (RubCreateCube_RoundStickers | 
                             RubCreateCube_StickerHaveWalls),
};


#endif //FLOWER_RUBIKS_H
