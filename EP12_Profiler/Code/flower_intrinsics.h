#ifndef FLOWER_INTRINSICS_H
#define FLOWER_INTRINSICS_H

#include <intrin.h>

inline float Sqrt(float Value)
{
	float Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
    
	return(Result);
}

inline float RSqrt(float Value)
{
	float Result = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(Value)));
    
	return(Result);
}

inline float MinFloat(float A, float B)
{
    __m128 mmA = _mm_set_ss(A);
    __m128 mmB = _mm_set_ss(B);
    
    float Result = _mm_cvtss_f32(_mm_min_ss(mmA, mmB));
    
    return(Result);
}

inline float MaxFloat(float A, float B)
{
    __m128 mmA = _mm_set_ss(A);
    __m128 mmB = _mm_set_ss(B);
    
    float Result = _mm_cvtss_f32(_mm_max_ss(mmA, mmB));
    
    return(Result);
}

inline float Floor(float Value)
{
	float Result = floorf(Value);
	return(Result);
}

inline float Ceil(float Value)
{
	float Result = ceilf(Value);
	return(Result);
}

inline float Sin(float Rad)
{
	float Result = sinf(Rad);
	return(Result);
}

inline float Cos(float Rad)
{
	float Result = cosf(Rad);
	return(Result);
}

inline float Tan(float Rad)
{
	float Result = tanf(Rad);
	return(Result);
}

inline float ASin(float Value)
{
	float Result = asinf(Value);
	return(Result);
}

inline float ACos(float Value) 
{
	float Result = acosf(Value);
	return(Result);
}

inline float ATan(float Value)
{
	float Result = atan(Value);
	return(Result);
}

inline float ATan2(float Y, float X) 
{
	float Result = atan2f(Y, X);
	return(Result);
}

inline float Exp(float Value)
{
	float Result = expf(Value);
	return(Result);
}

inline float Log(float Value)
{
	float Result = logf(Value);
	return(Result);
}

inline float Pow(float a, float b)
{
	float Result = powf(a, b);
	return(Result);
}

inline float Lerp(float a, float b, float t)
{
    __m128 mmT = _mm_set_ss(t);
    __m128 mmA = _mm_set_ss(a);
    __m128 mmB = _mm_set_ss(b);
    
    __m128 RightPart = _mm_mul_ss(_mm_sub_ss(mmB, mmA), mmT);
    
	float Result = _mm_cvtss_f32(_mm_add_ss(mmA, RightPart));;
    
	return(Result);
}

#endif //FLOWER_INTRINSICS_H
