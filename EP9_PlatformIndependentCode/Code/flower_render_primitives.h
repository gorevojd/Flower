#ifndef FLOWER_RENDER_PRIMITIVES_H
#define FLOWER_RENDER_PRIMITIVES_H

struct mesh_handles
{
    u64 ArrayObject;
    u64 BufferObject;
    u64 ElementBufferObject;
    
    b32 Initialized;
};

struct mesh_offsets
{
    u32 OffsetP;
    u32 OffsetUV;
    u32 OffsetN;
    u32 OffsetC;
    u32 OffsetBoneWeights;
    u32 OffsetBoneIndices;
};

struct mesh
{
    char Name[64];
    
    void* VerticesDataBuffer;
    u32 VerticesDataBufferSize;
    
    v3* P;
    v2* UV;
    v3* N;
    u32* C;
    v4* BoneWeights;
    u32* BoneIndices;
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
    b32 IsSkinned;
    int MaterialIndexInModel;
    
    mesh_handles ApiHandles;
    mesh_offsets Offsets;
};

struct image
{
    void* Pixels;
    
    int Width;
    int Height;
    
    float WidthOverHeight;
    
    b32 FilteringIsClosest;
    
    u64 ApiHandle;
};

struct material
{
    image* Diffuse;
};

struct node_animation_offsets
{
    u32 OffsetPositionKeys;
    u32 OffsetRotationKeys;
    u32 OffsetScalingKeys;
    u32 OffsetPositionTimes;
    u32 OffsetRotationTimes;
    u32 OffsetScalingTimes;
};

struct node_animation
{
    node_animation_offsets Offsets;
    
    v3* PositionKeys;
    quat* RotationKeys;
    v3* ScalingKeys;
    
    f32* PositionTimes;
    f32* RotationTimes;
    f32* ScalingTimes;
    
    int PositionCount;
    int RotationCount;
    int ScalingCount;
    
    int NodeIndex;
};

struct model_bone
{
    m44 InvBindPose;
    
    // NOTE(Dima): Index to node which represents this bone transformation in Model
    int NodeIndex;
};

struct model_node
{
    char Name[64];
    
    int ChildIndices[32];
    int MeshIndices[8];
    
    int NumChildIndices;
    int NumMeshIndices;
};

struct model_offsets
{
    u32 OffsetMeshes;
    u32 OffsetMaterials;
    u32 OffsetNodes;
    u32 OffsetNodeToModel;
    u32 OffsetNodeToParent;
    u32 OffsetParentNodeIndex;
    u32 OffsetBones;
    u32 OffsetSkinningMatrices;
};

struct model
{
    // NOTE(Dima): Meshes
    mesh** Meshes;
    int NumMeshes;
    
    // NOTE(Dima): Materials
    material** Materials;
    int NumMaterials;
    
    // NOTE(Dima): Nodes
    model_node* Nodes;
    m44* NodeToModel;
    m44* NodeToParent;
    int* ParentNodeIndex;
    int NumNodes;
    
    // NOTE(Dima): Bones
    model_bone* Bones;
    m44* SkinningMatrices;
    int NumBones;
    
    // NOTE(Dima): Offsets
    model_offsets Offsets;
};

enum animation_behaviour
{
    AnimBehaviour_Closest,
    AnimBehaviour_Repeat,
};

struct animation
{
    char Name[64];
    
    node_animation** NodeAnimations;
    int NumNodeAnimations;
    
    f32 DurationTicks;
    f32 TicksPerSecond;
    u32 Behaviour;
};

struct glyph
{
    image Image;
    
    u32 Codepoint;
    
    f32 Advance;
    f32 LeftBearing;
    
    f32 XOffset;
    f32 YOffset;
    
    v2 MinUV;
    v2 MaxUV;
};

struct font
{
    glyph* Glyphs;
    int GlyphCount;
    
    float* KerningPairs;
    
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
    
    f32 LineAdvance;
    
    int Size;
};

#endif //FLOWER_RENDER_PRIMITIVES_H
