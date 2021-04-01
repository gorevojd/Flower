#ifndef FLOWER_INTRINSICS_H
#define FLOWER_INTRINSICS_H

#include <intrin.h>
#include <math.h>

inline u32 Hash32(u64 A, u64 B = 0)
{
    __m128i SeedValue = _mm_set1_epi32(12324);
    __m128i Value = _mm_set_epi64x(A, B);
    __m128i Hash = _mm_aesdec_si128(Value, SeedValue);
    Hash = _mm_aesdec_si128(Hash, SeedValue);
    
    u32 Result = _mm_extract_epi32(Hash, 0);
    
    return(Result);
}

inline u32 Hash32Slow(u64 A)
{
    u8 TempStr[16];
    
    for(int i = 0; i < 8; i++)
    {
        TempStr[i] = (A >> (i * 8)) & 255;
    }
    TempStr[8] = 0;
    
    // NOTE(Dima): FNV Hash
    u32 Result = 2166136261;
    
    u8* At = TempStr;
    while (*At) {
        
        Result *= 16777619;
        Result ^= *At;
        
        At++;
    }
    
    return(Result);
}

inline float Sqrt(f32 Value)
{
	f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
    
	return(Result);
}

inline f32 RSqrt(f32 Value)
{
	f32 Result = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(Value)));
    
	return(Result);
}

inline f32 MinFloat(f32 A, f32 B)
{
    __m128 mmA = _mm_set_ss(A);
    __m128 mmB = _mm_set_ss(B);
    
    f32 Result = _mm_cvtss_f32(_mm_min_ss(mmA, mmB));
    
    return(Result);
}

inline f32 MaxFloat(f32 A, f32 B)
{
    __m128 mmA = _mm_set_ss(A);
    __m128 mmB = _mm_set_ss(B);
    
    f32 Result = _mm_cvtss_f32(_mm_max_ss(mmA, mmB));
    
    return(Result);
}

inline f32 ClampFloat(f32 Value, f32 MinValue, f32 MaxValue)
{
    __m128 mmV = _mm_set_ss(Value);
    __m128 mmMin = _mm_set_ss(MinValue);
    __m128 mmMax = _mm_set_ss(MaxValue);
    
    f32 Result = _mm_cvtss_f32(_mm_min_ss(mmMax, _mm_max_ss(mmV, mmMin)));
    
    return(Result);
}

inline f32 Clamp01Float(f32 Value)
{
    f32 Result = _mm_cvtss_f32(_mm_max_ss(_mm_min_ss(_mm_set_ss(1.0f), _mm_set_ss(Value)), _mm_set_ss(0.0f)));
    
    return(Result);
}

inline f32 SmoothstepFloat(f32 t)
{
    __m128 mmX = _mm_set_ss(t);
    
    __m128 mmRes = _mm_mul_ss(_mm_mul_ss(mmX, mmX), _mm_sub_ss(_mm_set_ss(3.0f), _mm_mul_ss(_mm_set_ss(2.0f), mmX)));
    
    f32 Result = _mm_cvtss_f32(mmRes);
    
    return(Result);
}

inline f32 Floor(f32 Value)
{
	f32 Result = floorf(Value);
	return(Result);
}

inline f32 Ceil(f32 Value)
{
	f32 Result = ceilf(Value);
	return(Result);
}

inline f32 Sin(f32 Rad)
{
	f32 Result = sinf(Rad);
	return(Result);
}

inline f32 Cos(f32 Rad)
{
	f32 Result = cosf(Rad);
	return(Result);
}

inline f32 Tan(f32 Rad)
{
	f32 Result = tanf(Rad);
	return(Result);
}

inline f32 ASin(f32 Value)
{
	f32 Result = asinf(Value);
	return(Result);
}

inline f32 ACos(f32 Value) 
{
	f32 Result = acosf(Value);
	return(Result);
}

inline f32 ATan(f32 Value)
{
	f32 Result = atan(Value);
	return(Result);
}

inline f32 ATan2(f32 Y, f32 X) 
{
	f32 Result = atan2f(Y, X);
	return(Result);
}

inline f32 Exp(f32 Value)
{
	f32 Result = expf(Value);
	return(Result);
}

inline f32 Log(f32 Value)
{
	f32 Result = logf(Value);
	return(Result);
}

inline f32 Pow(f32 a, f32 b)
{
	f32 Result = powf(a, b);
	return(Result);
}

inline f32 Lerp(f32 a, f32 b, f32 t)
{
    __m128 mmT = _mm_set_ss(t);
    __m128 mmA = _mm_set_ss(a);
    __m128 mmB = _mm_set_ss(b);
    
    __m128 RightPart = _mm_mul_ss(_mm_sub_ss(mmB, mmA), mmT);
    
	f32 Result = _mm_cvtss_f32(_mm_add_ss(mmA, RightPart));;
    
	return(Result);
}

#endif //FLOWER_INTRINSICS_H
