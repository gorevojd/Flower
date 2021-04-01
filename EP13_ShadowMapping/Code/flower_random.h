#ifndef FLOWER_RANDOM_H
#define FLOWER_RANDOM_H

#define F_ONE_OVER_FFFFFFFF 0.0000000002328306437080797

struct random_generation
{
    u64 State;
    u64 Inc;
};

inline random_generation SeedRandom(i64 Seed)
{
    random_generation RandomGen = {};
    
    RandomGen.Inc = ((u64)Seed << 1u) | 1u;
    RandomGen.State = (RandomGen.Inc + (u64)Seed) * 6364136223846793005ULL + RandomGen.Inc;
    
    return(RandomGen);
}

inline u32 RandomU32(random_generation* RandomGen)
{
    u64 OldState = RandomGen->State;
    // Advance internal state
    RandomGen->State = OldState * 6364136223846793005ULL + (RandomGen->Inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    u32 XorShifted = ((OldState >> 18u) ^ OldState) >> 27u;
    u32 Rot = OldState >> 59u;
    
    return (XorShifted >> Rot) | (XorShifted << ((-Rot) & 31));
}

inline b32 RandomBool(random_generation* Random)
{
    b32 Result = RandomU32(Random) & 1;
    
    return(Result);
}

inline f32 RandomUnilateral(random_generation* Random)
{
    f32 Result = (f32)RandomU32(Random) * F_ONE_OVER_FFFFFFFF;
    
    return(Result);
}

inline f32 RandomBilateral(random_generation* Random)
{
    f32 Result = RandomUnilateral(Random) * 2.0f - 1.0f;
    
    return(Result);
}

inline u32 RandomBetweenU32(random_generation* Random, u32 MinIncl, u32 MaxExcl)
{
    f32 t = RandomUnilateral(Random);
    
    f32 Dist = (f32)(MaxExcl - 1 - MinIncl);
    u32 Result = MinIncl + (u32)(t * Dist + 0.5f);
    
    return(Result);
}

inline u32 RandomIndex(random_generation* Random, u32 OnePastLast)
{
    u32 Result = RandomBetweenU32(Random, 0, OnePastLast);
    
    return(Result);
}

inline f32 RandomBetweenFloats(random_generation* Random, f32 Min, f32 Max)
{
    f32 t = RandomUnilateral(Random);
    
    f32 Result = Min + (Max - Min) * t;
    
    return(Result);
}

inline v2 RandomInUV(random_generation* Random)
{
    v2 Result = V2(RandomUnilateral(Random),
                   RandomUnilateral(Random));
    
    return(Result);
}

inline v2 RandomInRect(random_generation* Random, rc2 Rect)
{
    v2 InUV = RandomInUV(Random);
    
    v2 Result = V2(Lerp(Rect.Min.x, Rect.Max.x, InUV.x),
                   Lerp(Rect.Min.y, Rect.Max.y, InUV.y));
    
    return(Result);
}

inline v2 RandomInUnitCircleBad(random_generation* Random)
{
    f32 Angle = RandomUnilateral(Random) * 2.0f * F_PI;
    f32 Rad = RandomUnilateral(Random);
    
    v2 Result = V2(Cos(Angle), Sin(Angle)) * Rad;
    
    return(Result);
}

inline v2 RandomInUnitCircle(random_generation* Random)
{
    f32 Angle = RandomUnilateral(Random) * 2.0f * F_PI;
    f32 Rad = Sqrt(RandomUnilateral(Random));
    
    v2 Result = V2(Cos(Angle), Sin(Angle)) * Rad;
    
    return(Result);
}

inline v2 RandomOnUnitCircle(random_generation* Random)
{
    f32 Angle = RandomUnilateral(Random) * 2.0f * F_PI;
    
    v2 Result = V2(Cos(Angle), Sin(Angle));
    
    return(Result);
}

#endif //FLOWER_RANDOM_H
