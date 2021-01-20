#ifndef FLOWER_RENDER_PRIMITIVES_H
#define FLOWER_RENDER_PRIMITIVES_H

#include "flower_asset_types_shared.h"

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
    
    // NOTE(Dima): This pointer holds data for all the mesh (this includes vertices and index data)
    void* Free;
    u32 FreeSize;
    
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

struct node_animation
{
    v3* PositionKeys;
    quat* RotationKeys;
    v3* ScalingKeys;
    
    f32* PositionTimes;
    f32* RotationTimes;
    f32* ScalingTimes;
    
    int NumPos;
    int NumRot;
    int NumScl;
    
    int NodeIndex;
};

enum animation_behaviour
{
    AnimBehaviour_Closest,
    AnimBehaviour_Repeat,
};

struct animation
{
    char Name[64];
    asset_animation_shared Shared;
    
    node_animation* NodeAnims;
    
    void* Free;
};

struct glyph
{
    int ImageIndex;
    f32 WidthOverHeight;
    
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
    image* GlyphImages;
    glyph* Glyphs;
    int GlyphCount;
    
    float* KerningPairs;
    
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
    
    f32 LineAdvance;
    
    int Size;
};

struct model_node
{
    char Name[64];
    
    int ChildIndices[32];
    int MeshIndices[8];
    
    int NumChildIndices;
    int NumMeshIndices;
};

struct model
{
    mesh** Meshes;
    material** Materials;
    
    // NOTE(Dima): Nodes
    model_node* Nodes;
    m44* Node_ToModel;
    m44* Node_ToParent;
    int* Node_ParentIndex;
    
    // NOTE(Dima): Bones
    m44* Bone_InvBindPose;
    int* Bone_NodeIndex;
    m44* Bone_SkinningMatrices;
    
    asset_model_shared Shared;
    
    void* Free;
};

#endif //FLOWER_RENDER_PRIMITIVES_H
