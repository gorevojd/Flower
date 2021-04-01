INTERNAL_FUNCTION void InitSSAO(postprocessing* PP)
{
    // NOTE(Dima): Init SSAO kernel
    for(int i = 0; i < ARC(PP->SSAO_Kernel); i++)
    {
        v3 RandomVector = NOZ(V3(RandomBilateral(&PP->Random),
                                 RandomBilateral(&PP->Random),
                                 RandomUnilateral(&PP->Random)));
        
        f32 Scale = RandomUnilateral(&PP->Random);
        Scale = Lerp(0.1f, 1.0f, Scale * Scale);
        
        PP->SSAO_Kernel[i] = RandomVector * Scale;
    }
    
    // NOTE(Dima): Init SSAO noise
    for(int i = 0; i < ARC(PP->SSAO_Noise); i++)
    {
        
        PP->SSAO_Noise[i] = Normalize(V3(RandomBilateral(&PP->Random), 
                                         RandomBilateral(&PP->Random), 
                                         RandomBilateral(&PP->Random)));
    }
    
    PP->SSAO_Params = PP_SSAO_DefaultParams();
}

INTERNAL_FUNCTION void InitPostprocessing(postprocessing* PP)
{
    PP->Random = SeedRandom(62313);
    
    InitSSAO(PP);
    
    PP->DOF_Params = PP_DepthOfFieldDefaultParams();
}