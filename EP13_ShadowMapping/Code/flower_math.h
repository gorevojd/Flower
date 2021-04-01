#ifndef FLOWER_MATH_H
#define FLOWER_MATH_H

#define F_DEG2RAD 0.0174532925f
#define F_RAD2DEG 57.2958f

#define F_PI 3.14159265359f
#define F_TWO_PI 6.28318530718f
#define F_TAU 6.28318530718f
#define F_PI_OVER_TWO 1.57079632679f

#define F_ONE_OVER_255 0.00392156862f
#define F_SQRT_TWO 1.41421356237f

// NOTE(Dima): Structures
union v2 {
	struct {
		f32 x;
		f32 y;
	};
    
    struct 
    {
        f32 Min;
        f32 Max;
    };
    
    f32 e[2];
};

union v3
{
    struct
    {
        union
        {
            v2 xy;
            
            struct 
            {
                
                f32 x, y;
            };
        };
        
        f32 z;
    };
    
    struct 
    {
        f32 r, g, b;
    };
    
    struct
    {
        f32 A, B, C;
    };
    
    struct
    {
        f32 Pitch;
        f32 Yaw;
        f32 Roll;
    };
    
    f32 e[3];
};

union v4
{
    struct {
        union
        {
            struct
            {
                f32 x, y, z;
            };
            
            v3 xyz;
        };
        
        f32 w;
    };
    
    struct {
        union
        {
            struct
            {
                f32 r, g, b;
            };
            
            v3 rgb;
        };
        
        f32 a;
    };
    
    struct 
    {
        union
        {
            struct 
            {
                f32 A, B, C;
            };
            
            v3 ABC;
        };
        
        f32 D;
    };
    
    f32 e[4];
};

struct m33
{
    union
    {
        f32 e[9];
        f32 e2[3][3];
        
        v3 Rows[3];
    };
};

struct m44
{
    union
    {
        f32 e[16];
        
        v4 Rows[4];
        
#if 0        
#if F_ENABLE_SIMD_MATH
        __m128 mmRows[4];
#endif
#endif
    };
};

struct quat
{
    union
    {
        v3 xyz;
        
        struct
        {
            f32 x, y, z;
        };
    };
    
    f32 w;
};

struct rc2
{
    v2 Min;
    v2 Max;
};

#include "flower_intrinsics.h"
#include "flower_simd.h"

inline f32 PingPong(f32 Value, f32 MaxValue)
{
    f32 ModRes = fmod(Value, MaxValue);
    
    f32 DivRes = Value / MaxValue;
    int DivResInt = (int)DivRes;
    
    f32 Result = ModRes;
    if(DivResInt & 1)
    {
        Result = MaxValue - ModRes;
    }
    
    return(Result);
}

inline f32 PingPong01(f32 Value)
{
    f32 Result = PingPong(Value, 1.0f);
    
    return(Result);
}

inline f32 Clamp01(f32 Val) {
    if (Val < 0.0f) {
        Val = 0.0f;
    }
    
    if (Val > 1.0f) {
        Val = 1.0f;
    }
    
    return(Val);
}

inline f32 Clamp(f32 Val, f32 Min, f32 Max) {
    if (Val < Min) {
        Val = Min;
    }
    
    if (Val > Max) {
        Val = Max;
    }
    
    return(Val);
}

inline int Clamp(int Val, int Min, int Max) {
    if (Val < Min) {
        Val = Min;
    }
    
    if (Val > Max) {
        Val = Max;
    }
    
    return(Val);
}


// NOTE(Dima): Constructors
inline v2 V2(f32 Value)
{
    v2 Res;
    
    Res.x = Value;
    Res.y = Value;
    
    return(Res);
}

inline v2 V2(f32 x, f32 y) 
{
    v2 Res;
    
    Res.x = x;
    Res.y = y;
    
    return(Res);
}

inline v3 V3(v2 xy, f32 z) 
{
    v3 Res;
    
    Res.x = xy.x;
    Res.y = xy.y;
    Res.z = z;
    
    return(Res);
}

inline v3 V3(f32 x, f32 y, f32 z) 
{
    v3 Res;
    
    Res.x = x;
    Res.y = y;
    Res.z = z;
    
    return(Res);
}

inline v3 V3(f32 Value)
{
    v3 Res;
    
    Res.x = Value;
    Res.y = Value;
    Res.z = Value;
    
    return(Res);
}

inline v3 V3_One()
{
    v3 Res;
    
    Res.x = 1.0f;
    Res.y = 1.0f;
    Res.z = 1.0f;
    
    return(Res);
}

inline v3 V3_Zero()
{
    v3 Res;
    
    Res.x = 0.0f;
    Res.y = 0.0f;
    Res.z = 0.0f;
    
    return(Res);
}

inline v3 V3_Up()
{
    v3 Result = {0.0f, 1.0f, 0.0f};
    
    return(Result);
}

inline v3 V3_Down()
{
    v3 Result = {0.0f, -1.0f, 0.0f};
    
    return(Result);
}


inline v3 V3_Left()
{
    v3 Result = {1.0f, 0.0f, 0.0f};
    
    return(Result);
}


inline v3 V3_Right()
{
    v3 Result = {-1.0f, 0.0f, 0.0f};
    
    return(Result);
}

inline v3 V3_Forward()
{
    v3 Result = {0.0f, 0.0f, 1.0f};
    
    return(Result);
}

inline v3 V3_Back()
{
    v3 Result = {0.0f, 0.0f, -1.0f};
    
    return(Result);
}

inline v4 V4(f32 Value) 
{
    v4 Res;
    
    Res.x = Value;
    Res.y = Value;
    Res.z = Value;
    Res.w = Value;
    
    return(Res);
}

inline v4 V4(f32 x, f32 y, f32 z, f32 w) 
{
    v4 Res;
    
    Res.x = x;
    Res.y = y;
    Res.z = z;
    Res.w = w;
    
    return(Res);
}

inline v4 V4(v3 InitVector, f32 w) 
{
    v4 Res;
    Res.x = InitVector.x;
    Res.y = InitVector.y;
    Res.z = InitVector.z;
    Res.w = w;
    return(Res);
}

inline quat IdentityQuaternion()
{
    quat res;
    
    res.x = 0.0f;
    res.y = 0.0f;
    res.z = 0.0f;
    res.w = 1.0f;
    
    return(res);
}

inline quat Quaternion(f32 x, f32 y, f32 z, f32 w)
{
    quat Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return(Result);
}

inline quat AxisAngle(v3 Axis, f32 Angle)
{
    quat res;
    
    f32 HalfAngle = Angle * 0.5f;
    
    f32 S = Sin(HalfAngle);
    res.x = Axis.x * S;
    res.y = Axis.y * S;
    res.z = Axis.z * S;
    res.w = Cos(HalfAngle);
    
    return(res);
}

inline quat AngleAxis(f32 Angle, v3 Axis)
{
    quat Result = AxisAngle(Axis, Angle);
    
    return(Result);
}

inline m33 Matrix3FromRows(v3 R1, v3 R2, v3 R3){
    m33 Result = {};
    
    Result.Rows[0] = R1;
    Result.Rows[1] = R2;
    Result.Rows[2] = R3;
    
    return(Result);
}

inline m44 Matrix4FromRows(v4 R1, v4 R2, v4 R3, v4 R4){
    m44 Result = {};
    
    Result.Rows[0] = R1;
    Result.Rows[1] = R2;
    Result.Rows[2] = R3;
    Result.Rows[3] = R4;
    
    return(Result);
}

inline m44 Matrix4FromRows(v3 R1, v3 R2, v3 R3)
{
    m44 Result = Matrix4FromRows(V4(R1, 0.0f),
                                 V4(R2, 0.0f),
                                 V4(R3, 0.0f),
                                 V4(0.0f, 0.0f, 0.0f, 1.0f));
    
    return(Result);
}

inline m33 IdentityMatrix3()
{
    m33 Result = {};
    
    Result.e[0] = 1.0f;
    Result.e[4] = 1.0f;
    Result.e[8] = 1.0f;
    
    return(Result);
}

inline m44 IdentityMatrix4()
{
    m44 Result = {};
    
    Result.e[0] = 1.0f;
    Result.e[5] = 1.0f;
    Result.e[10] = 1.0f;
    Result.e[15] = 1.0f;
    
    return(Result);
}

/*Dot operations*/
inline f32 Dot(v2 A, v2 B) 
{
    return A.x * B.x + A.y * B.y;
}

inline f32 Dot(v3 A, v3 B) 
{
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline f32 Dot(v4 A, v4 B) 
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

inline f32 Dot(quat A, quat B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

/*Cross product*/
inline f32 Cross(v2 A, v2 B) 
{ 
    return A.x * B.y - B.x * A.y; 
}

inline v3 Cross(v3 A, v3 B) 
{
    v3 R;
    R.x = A.y * B.z - B.y * A.z;
    R.y = A.z * B.x - B.z * A.x;
    R.z = A.x * B.y - B.x * A.y;
    return(R);
}


/*Add operations*/
inline v2 Add(v2 A, v2 B) 
{
    A.x += B.x;
    A.y += B.y;
    
    return(A);
}

inline v3 Add(v3 A, v3 B) 
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    
    return(A);
}

inline v4 Add(v4 A, v4 B) 
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;
    
    return(A);
}

inline quat Add(quat A, quat B)
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;
    
    return(A);
}

/*Subtract operations*/
inline v2 Sub(v2 A, v2 B) 
{
    A.x -= B.x;
    A.y -= B.y;
    
    return(A);
}

inline v3 Sub(v3 A, v3 B) 
{
    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    
    return(A);
}

inline v4 Sub(v4 A, v4 B) 
{
    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    A.w -= B.w;
    
    return(A);
}

inline quat Sub(quat A, quat B) 
{
    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    A.w -= B.w;
    
    return(A);
}

/*Multiply operations*/
inline v2 Mul(v2 A, f32 S) 
{
    A.x *= S;
    A.y *= S;
    
    return(A);
}

inline v3 Mul(v3 A, f32 S) 
{
    A.x *= S;
    A.y *= S;
    A.z *= S;
    
    return(A);
}

inline v4 Mul(v4 A, f32 S) 
{
    A.x *= S;
    A.y *= S;
    A.z *= S;
    A.w *= S;
    
    return(A);
}

inline quat Mul(quat A, quat B)
{
    quat R;
    
    R.w = A.w * B.w - A.x * B.x - A.y * B.y - A.z * B.z;
    R.x = A.w * B.x + A.x * B.w + A.y * B.z - A.z * B.y;
    R.y = A.w * B.y + A.y * B.w + A.z * B.x - A.x * B.z;
    R.z = A.w * B.z + A.z * B.w + A.x * B.y - A.y * B.x;
    
    return(R);
}

inline quat Mul(quat A, f32 S)
{
    A.x *= S;
    A.y *= S;
    A.z *= S;
    A.w *= S;
    
    return(A);
}


#if F_ENABLE_SIMD_MATH
inline __m128 MulVecMatSSE(__m128 V, const m44& M){
    
    __m128 vX = _mm_shuffle_ps(V, V, 0x00);
    __m128 vY = _mm_shuffle_ps(V, V, 0x55);
    __m128 vZ = _mm_shuffle_ps(V, V, 0xAA);
    __m128 vW = _mm_shuffle_ps(V, V, 0xFF);
    
    __m128 Result = _mm_mul_ps(vX, M.mmRows[0]);
    Result = _mm_add_ps(Result, _mm_mul_ps(vY, M.mmRows[1]));
    Result = _mm_add_ps(Result, _mm_mul_ps(vZ, M.mmRows[2]));
    Result = _mm_add_ps(Result, _mm_mul_ps(vW, M.mmRows[3]));
    
    return(Result);
}
#endif

inline void MulRefsToRef(m44& R, const m44& A, const m44& B){
    
#if F_ENABLE_SIMD_MATH
    R.mmRows[0] = MulVecMatSSE(A.mmRows[0], B);
    R.mmRows[1] = MulVecMatSSE(A.mmRows[1], B);
    R.mmRows[2] = MulVecMatSSE(A.mmRows[2], B);
    R.mmRows[3] = MulVecMatSSE(A.mmRows[3], B);
#else
    R.e[0] = A.e[0] * B.e[0] + A.e[1] * B.e[4] + A.e[2] * B.e[8] + A.e[3] * B.e[12];
    R.e[1] = A.e[0] * B.e[1] + A.e[1] * B.e[5] + A.e[2] * B.e[9] + A.e[3] * B.e[13];
    R.e[2] = A.e[0] * B.e[2] + A.e[1] * B.e[6] + A.e[2] * B.e[10] + A.e[3] * B.e[14];
    R.e[3] = A.e[0] * B.e[3] + A.e[1] * B.e[7] + A.e[2] * B.e[11] + A.e[3] * B.e[15];
    
    R.e[4] = A.e[4] * B.e[0] + A.e[5] * B.e[4] + A.e[6] * B.e[8] + A.e[7] * B.e[12];
    R.e[5] = A.e[4] * B.e[1] + A.e[5] * B.e[5] + A.e[6] * B.e[9] + A.e[7] * B.e[13];
    R.e[6] = A.e[4] * B.e[2] + A.e[5] * B.e[6] + A.e[6] * B.e[10] + A.e[7] * B.e[14];
    R.e[7] = A.e[4] * B.e[3] + A.e[5] * B.e[7] + A.e[6] * B.e[11] + A.e[7] * B.e[15];
    
    R.e[8] = A.e[8] * B.e[0] + A.e[9] * B.e[4] + A.e[10] * B.e[8] + A.e[11] * B.e[12];
    R.e[9] = A.e[8] * B.e[1] + A.e[9] * B.e[5] + A.e[10] * B.e[9] + A.e[11] * B.e[13];
    R.e[10] = A.e[8] * B.e[2] + A.e[9] * B.e[6] + A.e[10] * B.e[10] + A.e[11] * B.e[14];
    R.e[11] = A.e[8] * B.e[3] + A.e[9] * B.e[7] + A.e[10] * B.e[11] + A.e[11] * B.e[15];
    
    R.e[12] = A.e[12] * B.e[0] + A.e[13] * B.e[4] + A.e[14] * B.e[8] + A.e[15] * B.e[12];
    R.e[13] = A.e[12] * B.e[1] + A.e[13] * B.e[5] + A.e[14] * B.e[9] + A.e[15] * B.e[13];
    R.e[14] = A.e[12] * B.e[2] + A.e[13] * B.e[6] + A.e[14] * B.e[10] + A.e[15] * B.e[14];
    R.e[15] = A.e[12] * B.e[3] + A.e[13] * B.e[7] + A.e[14] * B.e[11] + A.e[15] * B.e[15];
#endif
    
}

inline m44 Mul(const m44& A, const m44& B)
{
    m44 Res;
    
    MulRefsToRef(Res, A, B);
    
    return(Res);
}

inline v3 MulPoint(v3 V, const m44& M)
{
    v3 Result;
    
    Result.e[0] = V.e[0] * M.e[0] + V.e[1] * M.e[4] + V.e[2] * M.e[8] + M.e[12];
    Result.e[1] = V.e[0] * M.e[1] + V.e[1] * M.e[5] + V.e[2] * M.e[9] + M.e[13];
    Result.e[2] = V.e[0] * M.e[2] + V.e[1] * M.e[6] + V.e[2] * M.e[10] + M.e[14];
    
    return(Result);
}

inline v3 MulDirection(v3 V, const m44& M)
{
    v3 Result;
    
    Result.e[0] = V.e[0] * M.e[0] + V.e[1] * M.e[4] + V.e[2] * M.e[8];
    Result.e[1] = V.e[0] * M.e[1] + V.e[1] * M.e[5] + V.e[2] * M.e[9];
    Result.e[2] = V.e[0] * M.e[2] + V.e[1] * M.e[6] + V.e[2] * M.e[10];
    
    return(Result);
}

inline v4 operator*(v4 V, const m44& M)
{
    v4 Result;
    
    Result.e[0] = V.e[0] * M.e[0] + V.e[1] * M.e[4] + V.e[2] * M.e[8] + V.e[3] * M.e[12];
    Result.e[1] = V.e[0] * M.e[1] + V.e[1] * M.e[5] + V.e[2] * M.e[9] + V.e[3] * M.e[13];
    Result.e[2] = V.e[0] * M.e[2] + V.e[1] * M.e[6] + V.e[2] * M.e[10] + V.e[3] * M.e[14];
    Result.e[3] = V.e[0] * M.e[3] + V.e[1] * M.e[7] + V.e[2] * M.e[11] + V.e[3] * M.e[15];
    
    return(Result);
}

inline v4 Mul(v4 V, const m44& M) 
{
    v4 Result = V * M;
    
    return(Result);
}

inline v3 Mul(v3 A, const m33& B)
{
    v3 Res;
    
    Res.x = A.x * B.e[0] + A.y * B.e[3] + A.z * B.e[6];
    Res.y = A.x * B.e[1] + A.y * B.e[4] + A.z * B.e[7];
    Res.z = A.x * B.e[2] + A.y * B.e[5] + A.z * B.e[8];
    
    return(Res);
}

/*Divide operations*/
inline v2 Div(v2 A, f32 S) 
{
    f32 OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    
    return(A);
}

inline v3 Div(v3 A, f32 S) 
{
    f32 OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    A.z *= OneOverS;
    
    return(A);
}

inline v4 Div(v4 A, f32 S) 
{
    f32 OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    A.z *= OneOverS;
    A.w *= OneOverS;
    
    return(A);
}

inline quat Div(quat A, f32 S) 
{
    f32 OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    A.z *= OneOverS;
    A.w *= OneOverS;
    
    return(A);
}

/*Hadamard product*/
inline v2 Hadamard(v2 A, v2 B) { return (V2(A.x * B.x, A.y * B.y)); }
inline v3 Hadamard(v3 A, v3 B) { return (V3(A.x * B.x, A.y * B.y, A.z * B.z)); }
inline v4 Hadamard(v4 A, v4 B) { return (V4(A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w)); }

/*Magnitude of the vector*/
inline f32 Magnitude(v2 A) { return(Sqrt(Dot(A, A))); }
inline f32 Magnitude(v3 A) { return(Sqrt(Dot(A, A))); }
inline f32 Magnitude(v4 A) { return(Sqrt(Dot(A, A))); }
inline f32 Magnitude(quat A) { return(Sqrt(Dot(A, A))); }

inline f32 Length(v2 A) { return(Sqrt(Dot(A, A))); }
inline f32 Length(v3 A) { return(Sqrt(Dot(A, A))); }
inline f32 Length(v4 A) { return(Sqrt(Dot(A, A))); }
inline f32 Length(quat A) { return(Sqrt(Dot(A, A))); }

/*Squared magnitude*/
inline f32 SqMagnitude(v2 A) { return(Dot(A, A)); }
inline f32 SqMagnitude(v3 A) { return(Dot(A, A)); }
inline f32 SqMagnitude(v4 A) { return(Dot(A, A)); }
inline f32 SqMagnitude(quat A) { return(Dot(A, A)); }

inline f32 LengthSq(v2 A) { return(Sqrt(Dot(A, A))); }
inline f32 LengthSq(v3 A) { return(Sqrt(Dot(A, A))); }
inline f32 LengthSq(v4 A) { return(Sqrt(Dot(A, A))); }
inline f32 LengthSq(quat A) { return(Sqrt(Dot(A, A))); }

/*v2 operator overloading*/
inline v2 operator+(v2 A) { return(A); }
inline v2 operator-(v2 A) { v2 R = { -A.x, -A.y }; return(R); }

inline v2 operator+(v2 A, v2 b) { return Add(A, b); }
inline v2 operator-(v2 A, v2 b) { return Sub(A, b); }

inline v2 operator*(v2 A, f32 S) { return Mul(A, S); }
inline v2 operator*(f32 S, v2 A) { return Mul(A, S); }
inline v2 operator/(v2 A, f32 S) { return Div(A, S); }

inline v2 operator*(v2 A, v2 b) { v2 R = { A.x * b.x, A.y * b.y }; return(R); }
inline v2 operator/(v2 A, v2 b) { v2 R = { A.x / b.x, A.y / b.y }; return(R); }

inline v2 &operator+=(v2& A, v2 b) { return(A = A + b); }
inline v2 &operator-=(v2& A, v2 b) { return(A = A - b); }
inline v2 &operator*=(v2& A, f32 S) { return(A = A * S); }
inline v2 &operator/=(v2& A, f32 S) { return(A = A / S); }

/*v3 operator overloading*/
inline v3 operator+(v3 A) { return(A); }
inline v3 operator-(v3 A) { v3 R = { -A.x, -A.y, -A.z }; return(R); }

inline v3 operator+(v3 A, v3 b) { return Add(A, b); }
inline v3 operator-(v3 A, v3 b) { return Sub(A, b); }

inline v3 operator*(v3 A, f32 S) { return Mul(A, S); }
inline v3 operator*(f32 S, v3 A) { return Mul(A, S); }
inline v3 operator/(v3 A, f32 S) { return Div(A, S); }

inline v3 operator*(v3 A, v3 b) { v3 R = { A.x * b.x, A.y * b.y, A.z * b.z }; return(R); }
inline v3 operator/(v3 A, v3 b) { v3 R = { A.x / b.x, A.y / b.y, A.z / b.z }; return(R); }

inline v3 &operator+=(v3& A, v3 b) { return(A = A + b); }
inline v3 &operator-=(v3& A, v3 b) { return(A = A - b); }
inline v3 &operator*=(v3& A, f32 S) { return(A = A * S); }
inline v3 &operator/=(v3& A, f32 S) { return(A = A / S); }

/*v4 operator overloading*/
inline v4 operator+(v4 A) { return(A); }
inline v4 operator-(v4 A) { v4 R = { -A.x, -A.y, -A.z, -A.w }; return(R); }

inline v4 operator+(v4 A, v4 B) { return Add(A, B); }
inline v4 operator-(v4 A, v4 B) { return Sub(A, B); }

inline v4 operator*(v4 A, f32 S) { return Mul(A, S); }
inline v4 operator*(f32 S, v4 A) { return Mul(A, S); }
inline v4 operator/(v4 A, f32 S) { return Div(A, S); }

inline v4 operator*(v4 A, v4 B) { v4 R = { A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w }; return(R); }
inline v4 operator/(v4 A, v4 B) { v4 R = { A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w }; return(R); }

inline v4 &operator+=(v4& A, v4 B) { return(A = A + B); }
inline v4 &operator-=(v4& A, v4 B) { return(A = A - B); }
inline v4 &operator*=(v4& A, f32 S) { return(A = A * S); }
inline v4 &operator/=(v4& A, f32 S) { return(A = A / S); }

/*quat operator overloading*/
inline quat operator+(quat A) { return(A); }
inline quat operator-(quat A) { return(A); }

inline quat operator+(quat A, quat B) { return Add(A, B); }
inline quat operator-(quat A, quat B) { return Sub(A, B); }

inline quat operator*(quat A, f32 S) { return Mul(A, S); }
inline quat operator*(f32 S, quat A) { return Mul(A, S); }
inline quat operator/(quat A, f32 S) { return Div(A, S); }

inline quat operator*(quat A, quat B) { return(Mul(A, B)); }

inline quat &operator+=(quat& A, quat B) { return(A = A + B); }
inline quat &operator-=(quat& A, quat B) { return(A = A - B); }
inline quat &operator*=(quat& A, f32 S) { return(A = A * S); }
inline quat &operator/=(quat& A, f32 S) { return(A = A / S); }

inline v3 operator*(v3 A, const m33& B)
{
    v3 Result = Mul(A, B);
    
    return(Result);
}

inline m44 operator*(const m44& A, const m44& B)
{
    m44 Result = Mul(A, B);
    
    return(Result);
}

/*Normalization operations*/
inline v2 Normalize(v2 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline v3 Normalize(v3 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline v4 Normalize(v4 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline quat Normalize(quat A) { return(Mul(A, RSqrt(Dot(A, A)))); }

/*Safe normalization operations*/
inline v2 NOZ(v2 A) { f32 SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V2(0.0f, 0.0f) : A * RSqrt(SqMag)); }
inline v3 NOZ(v3 A) { f32 SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V3(0.0f, 0.0f, 0.0f) : A * RSqrt(SqMag)); }
inline v4 NOZ(v4 A) { f32 SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V4(0.0f, 0.0f, 0.0f, 0.0f) : A * RSqrt(SqMag)); }

/*Lerp operations*/
inline v2 Lerp(v2 A, v2 B, f32 t) { return((1.0f - t) * A + B * t); }
inline v3 Lerp(v3 A, v3 B, f32 t) { return((1.0f - t) * A + B * t); }
inline v4 Lerp(v4 A, v4 B, f32 t) { return((1.0f - t) * A + B * t); }

/*Quaternion operations*/
inline quat Conjugate(quat Q){
    quat Result = {};
    
    Result.xyz = -Q.xyz;
    Result.w = Q.w;
    
    return(Result);
}

inline quat Inverse(quat Q){
    quat Result = Conjugate(Q) / Dot(Q, Q);
    
    return(Result);
}

// NOTE(Dima): Finds the 'diff' such tat diff * A = B
inline quat RotationDifference(quat A, quat B){
    quat Result = B * Inverse(A);
    
    return(Result);
}

inline quat Lerp(quat A, quat B, f32 t) {
    quat Result;
    
    f32 OneMinusT = 1.0f - t;
    
    if(Dot(A, B) < 0.0f)
    {
        Result.x = A.x * OneMinusT - B.x * t;
        Result.y = A.y * OneMinusT - B.y * t;
        Result.z = A.z * OneMinusT - B.z * t;
        Result.w = A.w * OneMinusT - B.w * t;
    }
    else
    {
        Result.x = A.x * OneMinusT + B.x * t;
        Result.y = A.y * OneMinusT + B.y * t;
        Result.z = A.z * OneMinusT + B.z * t;
        Result.w = A.w * OneMinusT + B.w * t;
    }
    
    Result = Normalize(Result);
    
    return(Result);
}

inline quat Slerp(quat A, quat B, f32 t){
    A = Normalize(A);
    B = Normalize(B);
    
    f32 CosTheta = Dot(A, B);
    f32 Theta = ACos(CosTheta);
    
    f32 OneOverSinTheta = 1.0f / Sin(Theta);
    f32 cA = OneOverSinTheta * Sin((1.0f - t) * Theta);
    f32 cB = OneOverSinTheta * Sin(t * Theta);
    
    quat Result;
    
    Result.x = cA * A.x + cB * B.x;
    Result.y = cA * A.y + cB * B.y;
    Result.z = cA * A.z + cB * B.z;
    Result.w = cA * A.w + cB * B.w;
    
    return(Result);
}

// NOTE(Dima): Matrices operations
inline m44 LookAt(v3 Pos, v3 TargetPos, v3 WorldUp, 
                  b32 InvertX = false)
{
    m44 Result;
    
    v3 Fwd = TargetPos - Pos;
    Fwd = NOZ(Fwd);
    
    v3 Eye = Pos;
    v3 Left;
    v3 Up;
    
    if(std::abs(Dot(Fwd, WorldUp)) < 0.9999999f)
    {
        if(InvertX)
        {
            Left = Normalize(Cross(Fwd, WorldUp));
            Up = Normalize(Cross(Left, Fwd));
        }
        else
        {
            Left = Normalize(Cross(WorldUp, Fwd));
            Up = Normalize(Cross(Fwd, Left));
        }
    }
    else
    {
        Left = V3_Left();
        Up = Normalize(Cross(Fwd, Left));
    }
    
    Result.e[0] = Left.x;
    Result.e[1] = Up.x;
    Result.e[2] = Fwd.x;
    Result.e[3] = 0.0f;
    
    Result.e[4] = Left.y;
    Result.e[5] = Up.y;
    Result.e[6] = Fwd.y;
    Result.e[7] = 0.0f;
    
    Result.e[8] = Left.z;
    Result.e[9] = Up.z;
    Result.e[10] = Fwd.z;
    Result.e[11] = 0.0f;
    
    Result.e[12] = -Dot(Left, Eye);
    Result.e[13] = -Dot(Up, Eye);
    Result.e[14] = -Dot(Fwd, Eye);
    Result.e[15] = 1.0f;
    
    return(Result);
}


inline m44 TranslationMatrix(v3 Translation){
    m44 Result = IdentityMatrix4();
    
    Result.e[12] = Translation.x;
    Result.e[13] = Translation.y;
    Result.e[14] = Translation.z;
    
    return(Result);
}

inline m44 ScalingMatrix(v3 Scale) 
{
    m44 Result = IdentityMatrix4();
    
    Result.e[0] = Scale.x;
    Result.e[5] = Scale.y;
    Result.e[10] = Scale.z;
    
    return(Result);
}

inline m44 ScalingMatrix(f32 Scale)
{
    m44 Result = ScalingMatrix(V3(Scale, Scale, Scale));
    
    return(Result);
}

inline m44 RotationMatrixX(f32 Angle)
{
    m44 Result = IdentityMatrix4();
    
    f32 s = Sin(Angle);
    f32 c = Cos(Angle);
    
    Result.e[5] = c;
    Result.e[6] = -s;
    Result.e[9] = s;
    Result.e[10] = c;
    
    return(Result);
}

inline m44 RotationMatrixY(f32 Angle)
{
    m44 Result = IdentityMatrix4();
    
    f32 s = Sin(Angle);
    f32 c = Cos(Angle);
    
    Result.e[0] = c;
    Result.e[2] = s;
    Result.e[8] = -s;
    Result.e[10] = c;
    
    return(Result);
}

inline m44 RotationMatrixZ(f32 Angle)
{
    m44 Result = IdentityMatrix4();
    
    f32 s = Sin(Angle);
    f32 c = Cos(Angle);
    
    Result.e[0] = c;
    Result.e[1] = -s;
    Result.e[4] = s;
    Result.e[5] = c;
    
    return(Result);
}

inline m44 InverseTranslationMatrix(v3 Translation)
{
    m44 Result = IdentityMatrix4();
    
    Result.e[12] = -Translation.x;
    Result.e[13] = -Translation.y;
    Result.e[14] = -Translation.z;
    
    return(Result);
}

inline m44 InverseScalingMatrix(v3 Scaling)
{
    m44 Result = ScalingMatrix(V3(1.0f / Scaling.x,
                                  1.0f / Scaling.y,
                                  1.0f / Scaling.z));
    
    return(Result);
}

inline m44 InverseScalingMatrix(f32 Scaling)
{
    m44 Result = ScalingMatrix(1.0f / Scaling);
    
    return(Result);
}

inline f32 Determinant3(f32 a1, f32 a2, f32 a3,
                        f32 b1, f32 b2, f32 b3,
                        f32 c1, f32 c2, f32 c3)
{
    f32 Result = 
        a1 * b2 * c3 + 
        a2 * b3 * c1 + 
        a3 * b1 * c2 -
        a3 * b2 * c1 -
        a2 * b1 * c3 - 
        c2 * b3 * a1;
    
    return(Result);
}

inline m44 InverseMatrix4(const m44& M)
{
    m44 R = IdentityMatrix4();
    
    f32 Dets[16];
    
    // NOTE(Dima): Row 1
    Dets[0] = Determinant3(M.e[5], M.e[6], M.e[7],
                           M.e[9], M.e[10], M.e[11],
                           M.e[13], M.e[14], M.e[15]);
    Dets[1] = Determinant3(M.e[4], M.e[6], M.e[7],
                           M.e[8], M.e[10], M.e[11],
                           M.e[12], M.e[14], M.e[15]);
    Dets[2] = Determinant3(M.e[4], M.e[5], M.e[7],
                           M.e[8], M.e[9], M.e[11],
                           M.e[12], M.e[13], M.e[15]);
    Dets[3] = Determinant3(M.e[4], M.e[5], M.e[6],
                           M.e[8], M.e[9], M.e[10],
                           M.e[12], M.e[13], M.e[14]);
    
    // NOTE(Dima): Row 2
    Dets[4] = Determinant3(M.e[1], M.e[2], M.e[3],
                           M.e[9], M.e[10], M.e[11],
                           M.e[13], M.e[14], M.e[15]);
    Dets[5] = Determinant3(M.e[0], M.e[2], M.e[3],
                           M.e[8], M.e[10], M.e[11],
                           M.e[12], M.e[14], M.e[15]);
    Dets[6] = Determinant3(M.e[0], M.e[1], M.e[3],
                           M.e[8], M.e[9], M.e[11],
                           M.e[12], M.e[13], M.e[15]);
    Dets[7] = Determinant3(M.e[0], M.e[1], M.e[2],
                           M.e[8], M.e[9], M.e[10],
                           M.e[12], M.e[13], M.e[14]);
    
    // NOTE(Dima): Row 3
    Dets[8] = Determinant3(M.e[1], M.e[2], M.e[3],
                           M.e[5], M.e[6], M.e[7],
                           M.e[13], M.e[14], M.e[15]);
    Dets[9] = Determinant3(M.e[0], M.e[2], M.e[3],
                           M.e[4], M.e[6], M.e[7],
                           M.e[12], M.e[14], M.e[15]);
    Dets[10] = Determinant3(M.e[0], M.e[1], M.e[3],
                            M.e[4], M.e[5], M.e[7],
                            M.e[12], M.e[13], M.e[15]);
    Dets[11] = Determinant3(M.e[0], M.e[1], M.e[2],
                            M.e[4], M.e[5], M.e[6],
                            M.e[12], M.e[13], M.e[14]);
    
    // NOTE(Dima): Row 3
    Dets[12] = Determinant3(M.e[1], M.e[2], M.e[3],
                            M.e[5], M.e[6], M.e[7],
                            M.e[9], M.e[10], M.e[11]);
    Dets[13] = Determinant3(M.e[0], M.e[2], M.e[3],
                            M.e[4], M.e[6], M.e[7],
                            M.e[8], M.e[10], M.e[11]);
    Dets[14] = Determinant3(M.e[0], M.e[1], M.e[3],
                            M.e[4], M.e[5], M.e[7],
                            M.e[8], M.e[9], M.e[11]);
    Dets[15] = Determinant3(M.e[0], M.e[1], M.e[2],
                            M.e[4], M.e[5], M.e[6],
                            M.e[8], M.e[9], M.e[10]);
    
    f32 Det = 
        Dets[0] * M.e[0] - Dets[1] * M.e[1] +
        Dets[2] * M.e[2] - Dets[3] * M.e[3];
    
    if(std::abs(Det) > 0.000000001f)
    {
        f32 InvDet = 1.0f / Det;
        
        for(int Index = 0; Index < 16; Index++)
        {
            int i = Index % 4;
            int j = Index / 4;
            
            int SrcIndex = i * 4 + j;
            if((i + j) & 1)
            {
                Dets[SrcIndex] *= -1.0f;
            }
            
            R.e[Index] = InvDet * Dets[SrcIndex];
        }
    }
    
    return(R);
}

inline m33 Transpose(const m33& M)
{
    m33 Result = {};
    
    for (int RowIndex = 0; RowIndex < 3; RowIndex++) {
        for (int ColumtIndex = 0; ColumtIndex < 3; ColumtIndex++) {
            Result.e[ColumtIndex * 3 + RowIndex] = M.e[RowIndex * 3 + ColumtIndex];
        }
    }
    
    return(Result);
}

inline m44 Transpose(const m44& M)
{
    m44 Result;
    for (int RowIndex = 0; RowIndex < 4; RowIndex++) {
        for (int ColumtIndex = 0; ColumtIndex < 4; ColumtIndex++) {
            Result.e[ColumtIndex * 4 + RowIndex] = M.e[RowIndex * 4 + ColumtIndex];
        }
    }
    
    return(Result);
}

inline m44 InverseTransformMatrix(const m44& Transformation)
{
    // NOTE(Dima): Extracting translation
    v4 Translation = Transformation.Rows[3];
    
    m44 Temp = Transformation;
    
    v3& RowX = Temp.Rows[0].xyz;
    v3& RowY = Temp.Rows[1].xyz;
    v3& RowZ = Temp.Rows[2].xyz;
    
    f32 ScaleX = Length(RowX);
    f32 ScaleY = Length(RowY);
    f32 ScaleZ = Length(RowZ);
    
    // NOTE(Dima): Rows normalization to get rotation only components
    RowX = RowX / ScaleX;
    RowY = RowY / ScaleY;
    RowZ = RowZ / ScaleZ;
    
    // NOTE(Dima): Getting inverse matrices
    m44 InvTranslation = InverseTranslationMatrix(Translation.xyz);
    m44 InvScaling = InverseScalingMatrix(V3(ScaleX, ScaleY, ScaleZ));
    m44 InvRotation = Transpose(Matrix4FromRows(RowX, RowY, RowZ));
    
    m44 Result = InvTranslation * InvRotation * InvScaling;
    
    return(Result);
}

inline m44 PerspectiveProjection(int Width, int Height, 
                                 f32 Far, f32 Near, 
                                 f32 FOVDegrees = 45.0f)
{
    m44 Result = {};
    
    f32 AspectRatio = (f32)Width / (f32)Height;
    f32 OneOverFarMinusNear = 1.0f / (Far - Near);
    
    f32 S = 1.0f / Tan(FOVDegrees * 0.5f * F_DEG2RAD);
    f32 A = -S / AspectRatio;
    f32 B = S;
    
    Result.e[0] = A;
    Result.e[5] = B;
    
    Result.e[10] = (Far + Near) * OneOverFarMinusNear;
    Result.e[14] = -(2.0f * Far * Near) * OneOverFarMinusNear;
    Result.e[11] = 1.0f;
    
    return(Result);
}


inline m44 OrthographicProjection(int Width, int Height)
{
    f32 a = 2.0f / (f32)Width;
    f32 b = -1.0f;
    f32 c = -2.0f / (f32)Height;
    f32 d = 1.0f;
    
    m44 Result = {};
    
    Result.e[0] = a;
    Result.e[5] = c;
    Result.e[10] = 1.0f;
    Result.e[12] = -1.0f;
    Result.e[13] = 1.0f;
    Result.e[15] = 1.0f;
    
    return(Result);
}

inline m44 OrthographicProjection(f32 RadiusW,
                                  f32 RadiusH,
                                  f32 Far, f32 Near)
{
    m44 Result = {};
    
    Result.e[0] = 1.0f / RadiusW;
    Result.e[5] = 1.0f / RadiusH;
    Result.e[10] = 2.0f / (Far - Near);
    Result.e[14] = -(Far + Near) / (Far - Near);
    Result.e[15] = 1.0f;
    
    return(Result);
}

/*Conversions*/
inline m44 Matrix3ToMatrix4(const m33& Matrix)
{
    m44 Result = {};
    
    Result.Rows[0] = V4(Matrix.Rows[0], 0.0f);
    Result.Rows[1] = V4(Matrix.Rows[1], 0.0f);
    Result.Rows[2] = V4(Matrix.Rows[2], 0.0f);
    Result.Rows[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return(Result);
}

inline m33 Matrix4ToMatrix3(const m44& Matrix)
{
    m33 Result = Matrix3FromRows(Matrix.Rows[0].xyz,
                                 Matrix.Rows[1].xyz,
                                 Matrix.Rows[2].xyz);
    
    return(Result);
}

inline m33 QuaternionToMatrix3(quat Q){
    m33 Result = {};
    
    f32 x2 = Q.x * Q.x;
    f32 y2 = Q.y * Q.y;
    f32 z2 = Q.z * Q.z;
    
    f32 xy = Q.x * Q.y;
    f32 zw = Q.z * Q.w;
    f32 xz = Q.x * Q.z;
    f32 yw = Q.y * Q.w;
    f32 yz = Q.y * Q.z;
    f32 xw = Q.x * Q.w;
    
    Result.e[0] = 1.0f - 2.0f * (y2 + z2);
    Result.e[1] = 2.0f * (xy + zw);
    Result.e[2] = 2.0f * (xz - yw);
    
    Result.e[3] = 2.0f * (xy - zw);
    Result.e[4] = 1.0f - 2.0f * (x2 + z2);
    Result.e[5] = 2.0f * (yz + xw);
    
    Result.e[6] = 2.0f * (xz + yw);
    Result.e[7] = 2.0f * (yz - xw);
    Result.e[8] = 1.0f - 2.0f * (x2 + y2);
    
    return(Result);
}

inline m44 QuaternionToMatrix4(quat Q)
{
    m33 Temp = QuaternionToMatrix3(Q);
    
    m44 Result  = Matrix3ToMatrix4(Temp);
    
    return(Result);
}

inline m44 RotationMatrix(quat Q)
{
    m44 Result = QuaternionToMatrix4(Q);
    
    return(Result);
}

inline v3 GetQuatLeft(quat Q)
{
    m33 Temp = QuaternionToMatrix3(Q);
    
    v3 Result = Temp.Rows[0];
    
    return(Result);
}

inline v3 GetQuatUp(quat Q)
{
    m33 Temp = QuaternionToMatrix3(Q);
    
    v3 Result = Temp.Rows[0];
    
    return(Result);
}

inline v3 GetQuatFront(quat Q)
{
    m33 Temp = QuaternionToMatrix3(Q);
    
    v3 Result = Temp.Rows[0];
    
    return(Result);
}

inline quat QuatFrom2DArray(f32 A[3][3]){
    quat Result;
    
    f32 Trace = A[0][0] + A[1][1] + A[2][2]; // I removed + 1.0f; see discussion with Ethan
    if( Trace > 0 ) {// I changed M_EPSILON to 0
        f32 S = 0.5f / sqrtf(Trace + 1.0f);
        Result.w = 0.25f / S;
        Result.x = ( A[1][2] - A[2][1] ) * S;
        Result.y = ( A[2][0] - A[0][2] ) * S;
        Result.z = ( A[0][1] - A[1][0] ) * S;
    } else {
        if ( A[0][0] > A[1][1] && A[0][0] > A[2][2] ) {
            f32 S = 2.0f * sqrtf( 1.0f + A[0][0] - A[1][1] - A[2][2]);
            Result.w = (A[1][2] - A[2][1] ) / S;
            Result.x = 0.25f * S;
            Result.y = (A[1][0] + A[0][1] ) / S;
            Result.z = (A[2][0] + A[0][2] ) / S;
        } else if (A[1][1] > A[2][2]) {
            f32 S = 2.0f * sqrtf( 1.0f + A[1][1] - A[0][0] - A[2][2]);
            Result.w = (A[2][0] - A[0][2] ) / S;
            Result.x = (A[1][0] + A[0][1] ) / S;
            Result.y = 0.25f * S;
            Result.z = (A[2][1] + A[1][2] ) / S;
        } else {
            f32 S = 2.0f * sqrtf( 1.0f + A[2][2] - A[0][0] - A[1][1] );
            Result.w = (A[0][1] - A[1][0] ) / S;
            Result.x = (A[2][0] + A[0][2] ) / S;
            Result.y = (A[2][1] + A[1][2] ) / S;
            Result.z = 0.25f * S;
        }
    }
    
    return(Result);
}

inline quat QuatFromMatrix3(m33 Mat){
    quat Result = QuatFrom2DArray(Mat.e2);
    
    return(Result);
}

inline quat LookRotation(v3 Front, v3 Up)
{
    v3 Fwd = NOZ(Front);
    v3 Lft = NOZ(Cross(Up, Fwd));
    Up = Cross(Fwd, Lft);
    
    m33 Mat = Matrix3FromRows(Lft, Up, Fwd);
    
    quat Result = QuatFromMatrix3(Mat);
    
    return(Result);
}

inline v3 QuatToEuler(quat q)
{
    v3 Result = {};
    
    // roll (x-axis rotation)
    f32 sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    f32 cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    Result.Roll = ATan2(sinr_cosp, cosr_cosp);
    
    // pitch (y-axis rotation)
    f32 sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (abs(sinp) >= 1.0f){
        
        f32 sinpSign = -1.0f;
        if(sinp > 0.0f)
        {
            sinpSign = 1.0f;
        }
        Result.Pitch = F_PI * 0.5f * sinpSign; // use 90 degrees if out of range
    }
    else
    {
        Result.Pitch = ASin(sinp);
    }
    
    // yaw (z-axis rotation)
    f32 siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    f32 cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    Result.Yaw = ATan2(siny_cosp, cosy_cosp);
    
    return(Result);
}

inline quat EulerToQuat(v3 EulerAngles)
{
    quat Result = {};
    
    // Abbreviations for the various angular functions
    f32 cy = cosf(EulerAngles.Yaw * 0.5f);
    f32 sy = sinf(EulerAngles.Yaw * 0.5f);
    f32 cp = cosf(EulerAngles.Pitch * 0.5f);
    f32 sp = sinf(EulerAngles.Pitch * 0.5f);
    f32 cr = cosf(EulerAngles.Roll * 0.5f);
    f32 sr = sinf(EulerAngles.Roll * 0.5f);
    
    Result.w = cy * cp * cr + sy * sp * sr;
    Result.x = cy * cp * sr - sy * sp * cr;
    Result.y = sy * cp * sr + cy * sp * cr;
    Result.z = sy * cp * cr - cy * sp * sr;
    
    return(Result);
}

/* Smoothstep lerp */
inline f32 Smoothstep(f32 Value)
{
    f32 x = Clamp01(Value);
    
    f32 Result = x * x * (3.0f - 2.0f * x);
    
    return(Result);
}

/* Cosine lerp. Takes f32 as input that will be clamped to [0, 1] inside */
inline f32 CosLerp01(f32 t){
    f32 result = Cos(Clamp01(t) * F_PI * 0.5f);
    
    return(result);
}

inline v2 CosLerp(v2 a, v2 b, f32 t){
    f32 factor = CosLerp01(t);
    
    v2 Result = Lerp(a, b, t);
    
    return(Result);
}

inline v3 CosLerp(v3 a, v3 b, f32 t){
    f32 factor = CosLerp01(t);
    
    v3 Result = Lerp(a, b, t);
    
    return(Result);
}

inline v4 CosLerp(v4 a, v4 b, f32 t){
    f32 factor = CosLerp01(t);
    
    v4 Result = Lerp(a, b, t);
    
    return(Result);
}


// NOTE(Dima): Color math
inline v4 ColorClear()
{
    v4 Result = V4(0.0f, 0.0f, 0.0f, 0.0f);
    
    return(Result);
}

inline v4 ColorBlack()
{
    v4 Result = V4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorWhite()
{
    v4 Result = V4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorGray(f32 Grayscale)
{
    v4 Result = V4(Grayscale, Grayscale, Grayscale, 1.0f);
    
    return(Result);
}

inline v4 ColorGray()
{
    v4 Result = V4(0.5f, 0.5f, 0.5f, 1.0f);
    
    return(Result);
}

inline v4 ColorRed()
{
    v4 Result = V4(1.0f, 0.0f, 0.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorGreen()
{
    v4 Result = V4(0.0f, 1.0f, 0.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorBlue()
{
    v4 Result = V4(0.0f, 0.0f, 1.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorYellow()
{
    v4 Result = V4(1.0f, 1.0f, 0.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorMagenta()
{
    v4 Result = V4(1.0f, 0.0f, 1.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorOrange()
{
    v4 Result = V4(1.0f, 0.4f, 0.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorCyan()
{
    v4 Result = V4(0.0f, 1.0f, 1.0f, 1.0f);
    
    return(Result);
}

inline v4 ColorFrom255(int R, int G, int B) {
    f32 OneOver255 = 1.0f / 255.0f;
    v4 Res = V4(R, G, B, 1.0f);
    Res.r *= OneOver255;
    Res.g *= OneOver255;
    Res.b *= OneOver255;
    
    Res.r = Clamp01(Res.r);
    Res.g = Clamp01(Res.g);
    Res.b = Clamp01(Res.b);
    
    return(Res);
}

inline int IntFromHexCharForColors(char C) {
    int Res = 0;
    
    if (C >= 'a' && C <= 'f') {
        C += 'A' - 'a';
    }
    
    if (C >= '0' && C <= '9') {
        Res = C - '0';
    }
    
    if (C >= 'A' && C <= 'F') {
        Res = C + 10 - 'A';
    }
    
    return(Res);
}

inline v4 ColorFromHex(char* str) {
    f32 OneOver255 = 1.0f / 255.0f;
    
    v4 Res;
    
    Assert(str[0] == '#');
    
    int R, G, B;
    R = IntFromHexCharForColors(str[1]) * 16 + IntFromHexCharForColors(str[2]);
    G = IntFromHexCharForColors(str[3]) * 16 + IntFromHexCharForColors(str[4]);
    B = IntFromHexCharForColors(str[5]) * 16 + IntFromHexCharForColors(str[6]);
    
    Res = V4(R, G, B, 1.0f);
    
    Res.r *= OneOver255;
    Res.g *= OneOver255;
    Res.b *= OneOver255;
    
    return(Res);
}

inline v4 PremultiplyAlpha(v4 Color)
{
    Color.rgb *= Color.a;
    
    return(Color);
}

inline v4 InvertColor(v4 Color)
{
    v4 Result;
    
    Result.r = 1.0f - Color.r;
    Result.g = 1.0f - Color.g;
    Result.b = 1.0f - Color.b;
    Result.a = Color.a;
    
    return(Result);
}

inline uint32_t PackRGBA(v4 Color)
{
    uint32_t Res = 
        (uint32_t)((Color.r * 255.0f + 0.5f)) |
        ((uint32_t)((Color.g * 255.0f) + 0.5f) << 8) |
        ((uint32_t)((Color.b * 255.0f) + 0.5f) << 16) |
        ((uint32_t)((Color.a * 255.0f) + 0.5f) << 24);
    
    return(Res);
}

inline v4 UnpackRGBA(uint32_t Color)
{
    v4 Res;
    
    Res.r = (f32)(Color & 0xFF) * F_ONE_OVER_255;
    Res.g = (f32)((Color >> 8) & 0xFF) * F_ONE_OVER_255;
    Res.b = (f32)((Color >> 16) & 0xFF) * F_ONE_OVER_255;
    Res.a = (f32)((Color >> 24) & 0xFF) * F_ONE_OVER_255;
    
    return(Res);
}

inline uint8_t PackGrayscale(f32 Grayscale)
{
    uint8_t Result = (uint8_t)(Grayscale * 255.0f + 0.5f);
    
    return(Result);
}

inline v4 UnpackGrayscale(uint8_t Value)
{
    v4 Result;
    
    Result.r = 1.0f;
    Result.g = 1.0f;
    Result.b = 1.0f;
    Result.a = (f32)(Value & 0xFF) * F_ONE_OVER_255;
    
    return(Result);
}

inline v4 UnpackGrayscalePremultiplied(uint8_t Value)
{
    f32 ResultValue = (f32)(Value & 0xFF) * F_ONE_OVER_255;
    
    v4 Result;
    Result.r = ResultValue;
    Result.g = ResultValue;
    Result.b = ResultValue;
    Result.a = ResultValue;
    
    return(Result);
}

inline uint32_t PackRGB(v3 Color)
{
    uint32_t Result = PackRGBA(V4(Color, 1.0f));
    
    return(Result);
}

inline v3 UnpackRGB(uint32_t Color)
{
    v4 Unpacked = UnpackRGBA(Color);
    
    return(Unpacked.rgb);
}

inline uint16_t PackRGB_565(v3 Color)
{
    uint16_t Result = ((uint16_t)(Color.r * 31.0f + 0.5f) | 
                       ((uint16_t)(Color.g * 63.0f + 0.5f) << 5) | 
                       ((uint16_t)(Color.b * 31.0f + 0.5f) << 11));
    
    return(Result);
}

inline v3 UnpackRGB_565(uint16_t Color)
{
    v3 Result;
    
    Result.r = (f32)(Color & 31) / 31.0f;
    Result.g = (f32)((Color >> 5) & 63) / 63.0f;
    Result.b = (f32)((Color >> 11) & 31) / 31.0f;
    
    return(Result);
}

// NOTE(Dima): Rectangle functions
inline v2 GetDim(rc2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    
    return(Result);
}

inline f32 GetArea(rc2 Rect)
{
    v2 Dim = GetDim(Rect);
    
    f32 Result = Dim.x * Dim.y;
    
    return(Result);
}

inline f32 GetWidth(rc2 Rect)
{
    f32 Result = Rect.Max.x - Rect.Min.x;
    
    return(Result);
}

inline f32 GetHeight(rc2 Rect)
{
    f32 Result = Rect.Max.y - Rect.Min.y;
    
    return(Result);
}

inline v2 GetCenter(rc2 Rect)
{
    v2 Result = Rect.Min + GetDim(Rect) * 0.5f;
    
    return(Result);
}

inline rc2 RectMinMax(v2 Min, v2 Max)
{
    rc2 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rc2 RectMinDim(v2 Min, v2 Dim)
{
    rc2 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rc2 RectCenterDim(v2 Center, v2 Dim)
{
    v2 HalfDim = Dim * 0.5f;
    
    rc2 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rc2 GrowRect(rc2 Rect, f32 Scale)
{
    v2 HalfDim = GetDim(Rect) * 0.5f;
    v2 Center = Rect.Min + HalfDim;
    
    Rect.Min = Center - HalfDim * Scale;
    Rect.Max = Center - HalfDim * Scale;
    
    return(Rect);
}

inline rc2 GrowRectByPixels(rc2 Rect, int Pixels)
{
    v2 GrowValue = V2(Pixels);
    
    Rect.Min -= GrowValue;
    Rect.Max += GrowValue;
    
    return(Rect);
}

inline rc2 UnionRect(rc2 A, rc2 B)
{
    rc2 Result;
    
    Result.Min = V2(MinFloat(A.Min.x, B.Min.x), 
                    MinFloat(A.Min.y, B.Min.y));
    Result.Max = V2(MaxFloat(A.Max.x, B.Max.x),
                    MaxFloat(A.Max.y, B.Max.y));
    
    return(Result);
}

inline b32 PointInRect(v2 Point, rc2 Rect)
{
    v2 Center = GetCenter(Rect);
    v2 HalfDim = GetDim(Rect) * 0.5f;
    v2 Diff = Point - Center;
    
    b32 Result = false;
    if(std::abs(Diff.x) <= HalfDim.x && 
       std::abs(Diff.y) <= HalfDim.y)
    {
        Result = true;
    }
    
    return(Result);
}

inline v2 ClampInRect(v2 Point, rc2 Rect)
{
    v2 Result = V2(Clamp(Point.x, Rect.Min.x, Rect.Max.x),
                   Clamp(Point.y, Rect.Min.y, Rect.Max.y));
    
    return(Result);
}

// NOTE(Dima): Lines
inline v2 Perp(v2 Direction)
{
    v2 Result = V2(-Direction.y, Direction.x);
    
    return(Result);
}

inline v3 LineEquationFrom2Points(v2 P1, v2 P2) 
{
    v3 Result;
    
    Result.A = P2.y - P1.y;
    Result.B = P1.x - P2.x;
    Result.C = P1.y * P2.x - P1.x * P2.y;
    
    //NOTE(dima): Normalizing line equation
    f32 PlaneNormalSq = Result.A * Result.A + Result.B * Result.B;
    
    Result *= RSqrt(PlaneNormalSq);
    
    return(Result);
}

/*Plane math*/
inline v4 NormalizePlane(v4 Plane)
{
    float NormalLen = Magnitude(Plane.ABC);
    
    v4 Result;
    Result.A = Plane.A / NormalLen;
    Result.B = Plane.B / NormalLen;
    Result.C = Plane.C / NormalLen;
    Result.D = Plane.D / NormalLen;
    
    return(Result);
}

inline float PlanePointTest(v4 Plane, v3 Point) 
{
    float Res = Dot(Plane.ABC, Point) + Plane.D;
    
    return(Res);
}

inline v3 Reflect(v3 Value, v3 Normal)
{
    v3 Result = Value - 2.0f * Dot(Normal, Value) * Normal;
    
    return(Result);
}

#endif