#ifndef FLOWER_ASSET_TYPES_H
#define FLOWER_ASSET_TYPES_H

struct asset_model_offsets
{
    u32 OffsetMeshIDs;
    u32 OffsetMaterialIDs;
    u32 OffsetNodes;
    u32 OffsetNode_ToModel;
    u32 OffsetNode_ToParent;
    u32 OffsetNode_ParentIndex;
    u32 OffsetBone_InvBindPose;
    u32 OffsetBone_NodeIndex;
    u32 OffsetBone_SkinningMatrices;
};

struct asset_node_animation_offsets
{
    u32 OffsetPositionKeys;
    u32 OffsetRotationKeys;
    u32 OffsetScalingKeys;
    u32 OffsetPositionTimes;
    u32 OffsetRotationTimes;
    u32 OffsetScalingTimes;
};

struct asset_model
{
    // NOTE(Dima): Meshes
    u32* MeshIDs;
    u32* MaterialIDs;
    
    asset_model_shared Shared;
    model_offsets Offsets;
};

struct asset_animation
{
    char Name[64];
    
    asset_animation_shared Shared;
};

#endif //FLOWER_ASSET_TYPES_H
