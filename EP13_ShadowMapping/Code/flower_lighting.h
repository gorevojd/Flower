#ifndef FLOWER_LIGHTING_H
#define FLOWER_LIGHTING_H

struct directional_light
{
    v3 Dir;
    v3 C;
    b32 CalculateShadows;
};

struct shadow_cascade_info
{
    f32 SourceFarPlane;
    struct render_pass* RenderPass;
    
    m44 View;
    v3 P;
    
    f32 Far;
    f32 Near;
    
    f32 ViewRadiusW;
    f32 ViewRadiusH;
};

GLOBAL_VARIABLE int DefaultCascadeDistances[]
{
    15, 
    60,
    200,
    800,
};

struct pcf_poisson_samples
{
    v2* Samples;
    int Count;
    
    // NOTE(Dima): 16 to tile as 4 by 4 texture
    f32 Rotations[16];
    b32 ShouldRotateSamples;
};

struct point_light
{
    v3 P;
    v3 C;
    f32 Radius;
};

struct lighting
{
    memory_arena* Arena;
    random_generation Random;
    
    f32 AmbientPercentage;
    f32 ShadowStrength;
    directional_light DirLit;
    
    pcf_poisson_samples PCF_PoissonSamples;
    
#define MAX_CASCADES 8
    shadow_cascade_info Cascades[MAX_CASCADES];
    int CascadeCount;
    int ShadowMapRes;
    f32 CascadeSafeDistance;
    
    int VarianceShadowMapBlurRadius;
    b32 BlurVarianceShadowMaps;
};

#endif //FLOWER_LIGHTING_H
