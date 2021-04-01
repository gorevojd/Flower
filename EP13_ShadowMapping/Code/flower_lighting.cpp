INTERNAL_FUNCTION void GeneratePCF_PoissonSamples(lighting* Lighting, 
                                                  int TargetCount,
                                                  int TargetRadius,
                                                  b32 ShouldRotateSamples)
{
    pcf_poisson_samples* PCF_Samples = &Lighting->PCF_PoissonSamples;
    
    // NOTE(Dima): Generating grid of poisson samples
    v2 GridDim = V2(10, 10);
    std::vector<v2> Samples = GenerateBlueNoise(GridDim, 0.5f);
    
    v2 GridCenter = GridDim * 0.5f;
    // NOTE(Dima): Making GridCenter the actual center of samples
    for(int i = 0; i < Samples.size(); i++)
    {
        Samples[i] = Samples[i] - GridCenter;
    }
    
    Assert(Samples.size() >= TargetCount);
    
    // NOTE(Dima): Finding TargetCount closest points (Selection sort)
    f32 MaxRadius = 0.0f;
    for(int i = 0; i < TargetCount; i++)
    {
        int MinIndex = i;
        f32 MinLength = LengthSq(Samples[i]);
        
        for(int j = i + 1; j < Samples.size(); j++)
        {
            f32 Length = LengthSq(Samples[j]);
            
            if(Length < MinLength)
            {
                MinIndex = j;
                MinLength = Length;
            }
        }
        
        if(MinIndex != i)
        {
            std::swap(Samples[i], Samples[MinIndex]);
        }
        
        f32 LenCurrent = Length(Samples[i]);
        if(LenCurrent > MaxRadius)
        {
            MaxRadius = LenCurrent;
        }
    }
    
    PCF_Samples->Samples = PushArray(Lighting->Arena, v2, TargetCount);
    PCF_Samples->Count = TargetCount;
    
    for(int i = 0; i < 12; i++)
    {
        // NOTE(Dima): Renormalizing
        PCF_Samples->Samples[i] = Samples[i] / MaxRadius * TargetRadius;
    }
    
    random_generation Random = SeedRandom(12);
    for(int i = 0; i < ARC(PCF_Samples->Rotations); i++)
    {
        PCF_Samples->Rotations[i] = RandomUnilateral(&Random) * 2.0f * F_PI;
    }
    PCF_Samples->ShouldRotateSamples = ShouldRotateSamples;
}

INTERNAL_FUNCTION void InitLighting(lighting* Lighting, memory_arena* Arena)
{
    Lighting->Arena = Arena;
    
    Lighting->AmbientPercentage = 0.25f;
    Lighting->ShadowStrength = 0.65f;
    Lighting->DirLit.Dir = NOZ(V3(-0.5f, -0.5f, -10.8f));
    //Lighting->DirLit.Dir = NOZ(V3(0.0f, -1.0f, 0.0f));
    Lighting->DirLit.C = V3(1.0f, 1.0f, 1.0f);
    Lighting->DirLit.CalculateShadows = true;
    
    Lighting->CascadeCount = 4;
    Lighting->ShadowMapRes = 2048;
    Lighting->CascadeSafeDistance = 15.0f;
    Lighting->BlurVarianceShadowMaps = true;
    Lighting->VarianceShadowMapBlurRadius = 5;
    
    GeneratePCF_PoissonSamples(Lighting, 12, 2.0f, true);
}

shadow_cascade_info* GetDirLitCascades(lighting* Lighting,
                                       render_pass* MainRenderPass,
                                       int* OutCascadesCount,
                                       b32 UseDefaultCascades = true)
{
    FUNCTION_TIMING();
    
    directional_light* DirLit = &Lighting->DirLit;
    int CascadeCount = Lighting->CascadeCount;
    
    if(OutCascadesCount)
    {
        *OutCascadesCount = CascadeCount;
    }
    
    if(UseDefaultCascades)
    {
        Assert(CascadeCount == 4);
    }
    
    f32 PrevCascade = 0;
    for(int CascadeIndex = 0;
        CascadeIndex < CascadeCount;
        CascadeIndex++)
    {
        shadow_cascade_info* Casc = &Lighting->Cascades[CascadeIndex];
        
        f32 TargetNear = PrevCascade;
        f32 TargetFar = DefaultCascadeDistances[CascadeIndex];
        
        // NOTE(Dima): Target Near plane
        if(CascadeIndex == 0)
        {
            TargetNear = MainRenderPass->Near;
        }
        
        // NOTE(Dima): Target Far plane
        if(CascadeIndex == CascadeCount - 1)
        {
            TargetFar = MainRenderPass->Far;
        }
        
        if(!UseDefaultCascades)
        {
            f32 FarOverNear = MainRenderPass->Far / MainRenderPass->Near;
            
            // NOTE(Dima): Target Near plane
            if(CascadeIndex > 0)
            {
                TargetNear = Pow(FarOverNear, (f32)CascadeIndex / (f32)CascadeCount);
            }
            
            // NOTE(Dima): Target Far plane
            if(CascadeIndex < CascadeCount - 1)
            {
                TargetFar = Pow(FarOverNear, (f32)(CascadeIndex + 1) / (f32)CascadeCount);
            }
            
            PrevCascade = TargetFar;
        }
        else
        {
            // NOTE(Dima): Target Near plane
            if(CascadeIndex > 0)
            {
                TargetNear = DefaultCascadeDistances[CascadeIndex - 1];
            }
            
            // NOTE(Dima): Target Far plane
            if(CascadeIndex < CascadeCount - 1)
            {
                TargetFar = DefaultCascadeDistances[CascadeIndex];
            }
        }
        
        Casc->SourceFarPlane = TargetFar;
        
        m44 TargetProj = PerspectiveProjection(MainRenderPass->Width,
                                               MainRenderPass->Height,
                                               TargetFar,
                                               TargetNear,
                                               MainRenderPass->FOVDegrees);
        
        m44 InvTargetProj = InverseMatrix4(TargetProj);
        m44 InvTargetView = InverseMatrix4(MainRenderPass->View);
        
        m44 InvTargetViewProj = InvTargetProj * InvTargetView;
        
        v3 Points[8] = 
        {
            V3(-1.0f, -1.0f, -1.0f),
            V3(1.0f, -1.0f, -1.0f),
            V3(1.0f, -1.0f, 1.0f),
            V3(-1.0f, -1.0f, 1.0f),
            
            V3(-1.0f, 1.0f, -1.0f),
            V3(1.0f, 1.0f, -1.0f),
            V3(1.0f, 1.0f, 1.0f),
            V3(-1.0f, 1.0f, 1.0f),
        };
        
        v3 LitPosition = -DirLit->Dir * 100.0f;
        m44 TempLightViewMatrix = LookAt(LitPosition,
                                         LitPosition + DirLit->Dir, 
                                         V3_Up());
        m44 TempLightToWorld = InverseMatrix4(TempLightViewMatrix);
        
        v2 MinMaxX = V2(9999999.0f, -9999999.0f);
        v2 MinMaxY = MinMaxX;
        v2 MinMaxZ = MinMaxX;
        
        for(int i = 0; i < 8; i++)
        {
            // NOTE(Dima): Converting from NDC space to world space
            v4 TempRes = V4(Points[i], 1.0f) * InvTargetViewProj;
            // TODO(Dima): Check if this is correct
            Points[i] = TempRes.xyz / TempRes.w;
        }
        
        for(int i = 0; i < 8; i++)
        {
            // NOTE(Dima): Converting from world space to Light view space
            v4 TempRes = V4(Points[i], 1.0f) * TempLightViewMatrix;
            Points[i] = TempRes.xyz;
        }
        
        for(int i = 0; i < 8; i++)
        {
            // NOTE(Dima): Min X
            v3 P = Points[i];
            if(P.x < MinMaxX.x)
            {
                MinMaxX.x = P.x;
            }
            
            // NOTE(Dima): Max X
            if(P.x > MinMaxX.y)
            {
                MinMaxX.y = P.x;
            }
            
            // NOTE(Dima): Min Y
            if(P.y < MinMaxY.x)
            {
                MinMaxY.x = P.y;
            }
            
            // NOTE(Dima): Max Y
            if(P.y > MinMaxY.y)
            {
                MinMaxY.y = P.y;
            }
            
            // NOTE(Dima): Min Z
            if(P.z < MinMaxZ.x)
            {
                MinMaxZ.x = P.z;
            }
            
            // NOTE(Dima): Max Z
            if(P.z > MinMaxZ.y)
            {
                MinMaxZ.y = P.z;
            }
        }
        
        f32 ViewW = MinMaxX.y - MinMaxX.x;
        f32 ViewH = MinMaxY.y - MinMaxY.x;
        f32 ViewZ = MinMaxZ.y - MinMaxZ.x;
        
        Casc->ViewRadiusW = ViewW * 0.5f;
        Casc->ViewRadiusH = ViewH * 0.5f;
        
        v3 DesiredLightP = V3(MinMaxX.x + Casc->ViewRadiusW,
                              MinMaxY.x + Casc->ViewRadiusH,
                              MinMaxZ.x - Lighting->CascadeSafeDistance);
        v4 DesiredLightPWorld = V4(DesiredLightP, 1.0f) * TempLightToWorld;
        Casc->P = DesiredLightPWorld.xyz;
        
        Casc->Far = ViewZ + Lighting->CascadeSafeDistance;
        Casc->Near = 0.0f;
        
        Casc->View = LookAt(Casc->P,
                            Casc->P + DirLit->Dir, 
                            V3_Up());
    }
    
    return(Lighting->Cascades);
}
