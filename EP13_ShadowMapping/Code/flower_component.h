#ifndef FLOWER_COMPONENT_H
#define FLOWER_COMPONENT_H

enum component_type
{
    Component_component_model,
    Component_component_animator,
    Component_component_particle_system,
    
    Component_Count,
};

struct component_model
{
    model* Model;
    m44* NodeToModel;
    m44* SkinningMatrices;
    b32 ToModelIsComputed;
};

struct component_animator
{
    animation* PlayingAnimation;
};

struct component_particle_system
{
    
};

struct component
{
    union
    {
        component_model Model;
        component_animator Animator;
        component_particle_system ParticleSystem;
    } Component;
    
    void* Free;
    u32 FreeSize;
};

#endif //FLOWER_COMPONENT_H
