#ifndef FLOWER_ASSET_TYPES_SHARED_H
#define FLOWER_ASSET_TYPES_SHARED_H

struct asset_node_animation_shared
{
    
};

struct asset_model_shared
{
    int NumMeshes;
    int NumMaterials;
    int NumNodes;
    int NumBones;
};

struct asset_animation_shared
{
    int NumNodeAnims;
    u32 Behaviour;
    
    f32 DurationTicks;
    f32 TicksPerSecond;
};

#endif //FLOWER_ASSET_TYPES_SHARED_H
