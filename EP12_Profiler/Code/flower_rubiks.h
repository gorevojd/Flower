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

GLOBAL_VARIABLE const char* RubiksDirectionName[] = 
{
    "Up",
    "Down",
    "Left",
    "Right",
    "Front",
    "Back"
};

GLOBAL_VARIABLE v3 RubiksDirection[] = 
{
    V3_Up(),
    V3_Down(),
    V3_Left(),
    V3_Right(),
    V3_Forward(),
    V3_Back(),
};

#if 0
GLOBAL_VARIABLE v4 RubiksColors[] = 
{
    ColorWhite(),
    ColorYellow(),
    ColorOrange(),
    ColorRed(),
    ColorBlue(),
    ColorGreen(),
};
#else
GLOBAL_VARIABLE v4 RubiksColors[] = 
{
    ColorFromHex("#FFFFFF"),
    ColorFromHex("#FFD500"),
    ColorFromHex("#FF5800"),
    ColorFromHex("#B71234"),
    ColorFromHex("#0046ad"),
    ColorFromHex("#009b48"),
};
#endif

#if 0
struct rubiks_cubie
{
    // TODO(Dima): Maybe store those as pointers because a lot of cubes will not be visible
    m44 Transform;
    m44 AppliedRotation;
    v3 InitP;
    
    int MeshIndex;
};
#endif

struct rubiks_visible_cubies
{
    m44* FinalTransform;
    m44* Transform;
    m44* AppliedRotation;
    v3* InitP;
    int* MeshIndex;
    
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
    
    int FaceIndex;
};

typedef m44 rubiks_rotation_function(float Angle);
typedef rubiks_rotate_face* rubiks_get_face_function(struct rubiks_cube* Cube, int value);

struct rubiks_beginned_rotation
{
    rubiks_rotation_function* RotationMatrix;
    rubiks_get_face_function* GetFace;
    int FaceIndex;
    f32 DirectionMultiplier;
    
    f32 InRotationTime;
    f32 TimeForRotation;
    b32 IsClockwise;
    
    m44 AppliedRotation;
    int AxisIndex;
};

struct rubiks_command
{
    int Axis;
    int FaceIndex;
    b32 IsClockwise;
};

#define RUBIKS_TIME_FOR_ROTATION 0.2f
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
    
    int Dim;
    
    f32 SideLen;
    f32 OneCubieLen;
    f32 HalfSideLen;
    f32 CubieOffset;
    
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
    
    // NOTE(Dima): These for rotating internal structure of cube
    int* ToRotateIndices;
    int* RotatedIndices;
    
    // NOTE(Dima): Commands
    int CommandsCount;
    rubiks_command* Commands;
    int DoIndex;
    int AddIndex;
    
    b32 IsRotatingNow;
};

#endif //FLOWER_RUBIKS_H
