#ifndef FLOWER_POSTPROCESS_H
#define FLOWER_POSTPROCESS_H

struct pp_dilation_params
{
    int Size;
    
    f32 MinThreshold;
    f32 MaxThreshold;
};

struct pp_depth_of_field_params
{
    f32 MinDistance;
    f32 MaxDistance;
    
    f32 FocusZ;
};

struct pp_ssao_params
{
    int KernelSize;
    f32 KernelRadius;
    f32 Contribution;
    f32 RangeCheck;
    int BlurRadius;
    
    b32 Enabled;
};

inline pp_ssao_params PP_SSAO_DefaultParams()
{
    pp_ssao_params Result = {};
    
    Result.KernelSize = 64;
    Result.KernelRadius = 0.6f;
    //Result.Contribution = 1.0f;
    Result.Contribution = 1.0f;
    Result.RangeCheck = 0.25f;
    Result.BlurRadius = 1;
    
    Result.Enabled = true;
    
    return(Result);
}

inline pp_dilation_params PP_DilationDefaultParams()
{
    pp_dilation_params Result = {};
    
    Result.Size = 2;
    Result.MinThreshold = 0.1f;
    Result.MaxThreshold = 0.3f;
    
    return(Result);
}

inline pp_depth_of_field_params PP_DepthOfFieldDefaultParams()
{
    pp_depth_of_field_params Result = {};
    
    //Result.MinDistance = 120.0f;
    //Result.MaxDistance = 280.0f;
    
#if 1    
    Result.MinDistance = 300.0f;
    Result.MaxDistance = 1200.0f;
#else
    Result.MinDistance = 50;
    Result.MaxDistance = 100;
#endif
    Result.FocusZ = 0.0f;
    
    return(Result);
}

struct postprocessing
{
    random_generation Random;
    
    v3 SSAO_Kernel[128];
    v3 SSAO_Noise[16];
    pp_ssao_params SSAO_Params;
    
    pp_depth_of_field_params DOF_Params;
};

#endif //FLOWER_POSTPROCESS_H
