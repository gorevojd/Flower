#ifndef FLOWER_MATH_H
#define FLOWER_MATH_H

#define F_DEG2RAD 0.0174532925f
#define F_RAD2DEG 57.2958f

#define F_PI 3.14159265359f
#define F_TWO_PI 6.28318530718f
#define F_TAU 6.28318530718f
#define F_PI_OVER_TWO 1.57079632679f

#define F_ONE_OVER_255 0.00392156862f

#define F_ENABLE_SIMD_MATH 1

// NOTE(Dima): Structures
union v2 {
	struct {
		float x;
		float y;
	};
    
    struct 
    {
        float Min;
        float Max;
    };
    
    float e[2];
};

union v3
{
    struct
    {
        float x, y, z;
    };
    
    struct 
    {
        float r, g, b;
    };
    
    struct
    {
        float Pitch;
        float Yaw;
        float Roll;
    };
    
    float e[3];
};

union v4
{
    struct {
        union
        {
            struct
            {
                float x, y, z;
            };
            
            v3 xyz;
        };
        
        float w;
    };
    
    struct {
        union
        {
            struct
            {
                float r, g, b;
            };
            
            v3 rgb;
        };
        
        float a;
    };
    
    float e[4];
};

struct m33
{
    union
    {
        float e[9];
        float e2[3][3];
        
        v3 Rows[3];
    };
};

struct m44
{
    union
    {
        float e[16];
        
        v4 Rows[4];
        
#if F_ENABLE_SIMD_MATH
        __m128 mmRows[4];
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
            float x, y, z;
        };
    };
    
    float w;
};

struct rc2
{
    v2 Min;
    v2 Max;
};

// NOTE(Dima): Basic functions
inline float CopySign(float Val1, float Val2){
    float Result = abs(Val1) * (Val2 > 0.0f ? 1.0f : -1.0f);
    return(Result);
}

inline float Sqrt(float Value) {
	float Result = sqrtf(Value);
	return(Result);
}

inline float RSqrt(float Value) {
	float Result = 1.0f / sqrtf(Value);
	return(Result);
}

inline float Floor(float Value) {
	float Result = floorf(Value);
	return(Result);
}

inline float Ceil(float Value) {
	float Result = ceilf(Value);
	return(Result);
}

inline float Sin(float Rad) {
	float Result = sinf(Rad);
	return(Result);
}

inline float Cos(float Rad) {
	float Result = cosf(Rad);
	return(Result);
}

inline float Tan(float Rad) {
	float Result = tanf(Rad);
	return(Result);
}

inline float ASin(float Value) {
	float Result = asinf(Value);
	return(Result);
}

inline float ACos(float Value) {
	float Result = acosf(Value);
	return(Result);
}

inline float ATan(float Value) {
	float Result = atan(Value);
	return(Result);
}

inline float ATan2(float Y, float X) {
	float Result = atan2f(Y, X);
	return(Result);
}

inline float Exp(float Value) {
	float Result = expf(Value);
	return(Result);
}

inline float Log(float Value) {
	float Result = logf(Value);
	return(Result);
}

inline float Pow(float a, float b) {
	float Result = powf(a, b);
	return(Result);
}

inline float Lerp(float a, float b, float t) {
	float Result = a + (b - a) * t;
    
	return(Result);
}

inline float PingPong(float Value, float MaxValue)
{
    float ModRes = fmod(Value, MaxValue);
    
    float DivRes = Value / MaxValue;
    int DivResInt = (int)DivRes;
    
    float Result = ModRes;
    if(DivResInt & 1)
    {
        Result = MaxValue - ModRes;
    }
    
    return(Result);
}

inline float Clamp01(float Val) {
	if (Val < 0.0f) {
		Val = 0.0f;
	}
    
	if (Val > 1.0f) {
		Val = 1.0f;
	}
    
	return(Val);
}

inline float Clamp(float Val, float Min, float Max) {
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
inline v2 V2(float Value)
{
	v2 Res;
    
	Res.x = Value;
	Res.y = Value;
    
	return(Res);
}

inline v2 V2(float x, float y) 
{
	v2 Res;
    
	Res.x = x;
	Res.y = y;
    
	return(Res);
}

inline v3 V3(v2 xy, float z) 
{
	v3 Res;
    
	Res.x = xy.x;
	Res.y = xy.y;
	Res.z = z;
    
	return(Res);
}

inline v3 V3(float x, float y, float z) 
{
	v3 Res;
    
	Res.x = x;
	Res.y = y;
	Res.z = z;
    
	return(Res);
}

inline v3 V3(float Value)
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

inline v4 V4(float Value) 
{
    v4 Res;
    
    Res.x = Value;
    Res.y = Value;
    Res.z = Value;
    Res.w = Value;
    
    return(Res);
}

inline v4 V4(float x, float y, float z, float w) 
{
    v4 Res;
    
    Res.x = x;
    Res.y = y;
    Res.z = z;
    Res.w = w;
    
    return(Res);
}

inline v4 V4(v3 InitVector, float w) 
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

inline quat Quaternion(float x, float y, float z, float w)
{
    quat Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return(Result);
}

inline quat AxisAngle(v3 Axis, float Angle)
{
    quat res;
    
    float HalfAngle = Angle * 0.5f;
    
    float S = Sin(HalfAngle);
    res.x = Axis.x * S;
    res.y = Axis.y * S;
    res.z = Axis.z * S;
    res.w = Cos(HalfAngle);
    
    return(res);
}

inline quat AngleAxis(float Angle, v3 Axis)
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
inline float Dot(v2 A, v2 B) 
{
    return A.x * B.x + A.y * B.y;
}

inline float Dot(v3 A, v3 B) 
{
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline float Dot(v4 A, v4 B) 
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

inline float Dot(quat A, quat B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

/*Cross product*/
inline float Cross(v2 A, v2 B) 
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
inline v2 Mul(v2 A, float S) 
{
    A.x *= S;
    A.y *= S;
    
    return(A);
}

inline v3 Mul(v3 A, float S) 
{
    A.x *= S;
    A.y *= S;
    A.z *= S;
    
    return(A);
}

inline v4 Mul(v4 A, float S) 
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

inline quat Mul(quat A, float S)
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

inline v4 Mul(v4 V, const m44& M) 
{
    v4 Result;
    
    Result.e[0] = V.e[0] * M.e[0] + V.e[1] * M.e[4] + V.e[2] * M.e[8] + V.e[3] * M.e[12];
    Result.e[1] = V.e[0] * M.e[1] + V.e[1] * M.e[5] + V.e[2] * M.e[9] + V.e[3] * M.e[13];
    Result.e[2] = V.e[0] * M.e[2] + V.e[1] * M.e[6] + V.e[2] * M.e[10] + V.e[3] * M.e[14];
    Result.e[3] = V.e[0] * M.e[3] + V.e[1] * M.e[7] + V.e[2] * M.e[11] + V.e[3] * M.e[15];
    
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
inline v2 Div(v2 A, float S) 
{
    float OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    
    return(A);
}

inline v3 Div(v3 A, float S) 
{
    float OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    A.z *= OneOverS;
    
    return(A);
}

inline v4 Div(v4 A, float S) 
{
    float OneOverS = 1.0f / S;
    
    A.x *= OneOverS;
    A.y *= OneOverS;
    A.z *= OneOverS;
    A.w *= OneOverS;
    
    return(A);
}

inline quat Div(quat A, float S) 
{
    float OneOverS = 1.0f / S;
    
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
inline float Magnitude(v2 A) { return(Sqrt(Dot(A, A))); }
inline float Magnitude(v3 A) { return(Sqrt(Dot(A, A))); }
inline float Magnitude(v4 A) { return(Sqrt(Dot(A, A))); }
inline float Magnitude(quat A) { return(Sqrt(Dot(A, A))); }

inline float Length(v2 A) { return(Sqrt(Dot(A, A))); }
inline float Length(v3 A) { return(Sqrt(Dot(A, A))); }
inline float Length(v4 A) { return(Sqrt(Dot(A, A))); }
inline float Length(quat A) { return(Sqrt(Dot(A, A))); }

/*Squared magnitude*/
inline float SqMagnitude(v2 A) { return(Dot(A, A)); }
inline float SqMagnitude(v3 A) { return(Dot(A, A)); }
inline float SqMagnitude(v4 A) { return(Dot(A, A)); }
inline float SqMagnitude(quat A) { return(Dot(A, A)); }

inline float LengthSq(v2 A) { return(Sqrt(Dot(A, A))); }
inline float LengthSq(v3 A) { return(Sqrt(Dot(A, A))); }
inline float LengthSq(v4 A) { return(Sqrt(Dot(A, A))); }
inline float LengthSq(quat A) { return(Sqrt(Dot(A, A))); }

/*v2 operator overloading*/
inline v2 operator+(v2 A) { return(A); }
inline v2 operator-(v2 A) { v2 R = { -A.x, -A.y }; return(R); }

inline v2 operator+(v2 A, v2 b) { return Add(A, b); }
inline v2 operator-(v2 A, v2 b) { return Sub(A, b); }

inline v2 operator*(v2 A, float S) { return Mul(A, S); }
inline v2 operator*(float S, v2 A) { return Mul(A, S); }
inline v2 operator/(v2 A, float S) { return Div(A, S); }

inline v2 operator*(v2 A, v2 b) { v2 R = { A.x * b.x, A.y * b.y }; return(R); }
inline v2 operator/(v2 A, v2 b) { v2 R = { A.x / b.x, A.y / b.y }; return(R); }

inline v2 &operator+=(v2& A, v2 b) { return(A = A + b); }
inline v2 &operator-=(v2& A, v2 b) { return(A = A - b); }
inline v2 &operator*=(v2& A, float S) { return(A = A * S); }
inline v2 &operator/=(v2& A, float S) { return(A = A / S); }

/*v3 operator overloading*/
inline v3 operator+(v3 A) { return(A); }
inline v3 operator-(v3 A) { v3 R = { -A.x, -A.y, -A.z }; return(R); }

inline v3 operator+(v3 A, v3 b) { return Add(A, b); }
inline v3 operator-(v3 A, v3 b) { return Sub(A, b); }

inline v3 operator*(v3 A, float S) { return Mul(A, S); }
inline v3 operator*(float S, v3 A) { return Mul(A, S); }
inline v3 operator/(v3 A, float S) { return Div(A, S); }

inline v3 operator*(v3 A, v3 b) { v3 R = { A.x * b.x, A.y * b.y, A.z * b.z }; return(R); }
inline v3 operator/(v3 A, v3 b) { v3 R = { A.x / b.x, A.y / b.y, A.z / b.z }; return(R); }

inline v3 &operator+=(v3& A, v3 b) { return(A = A + b); }
inline v3 &operator-=(v3& A, v3 b) { return(A = A - b); }
inline v3 &operator*=(v3& A, float S) { return(A = A * S); }
inline v3 &operator/=(v3& A, float S) { return(A = A / S); }

/*v4 operator overloading*/
inline v4 operator+(v4 A) { return(A); }
inline v4 operator-(v4 A) { v4 R = { -A.x, -A.y, -A.z, -A.w }; return(R); }

inline v4 operator+(v4 A, v4 B) { return Add(A, B); }
inline v4 operator-(v4 A, v4 B) { return Sub(A, B); }

inline v4 operator*(v4 A, float S) { return Mul(A, S); }
inline v4 operator*(float S, v4 A) { return Mul(A, S); }
inline v4 operator/(v4 A, float S) { return Div(A, S); }

inline v4 operator*(v4 A, v4 B) { v4 R = { A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w }; return(R); }
inline v4 operator/(v4 A, v4 B) { v4 R = { A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w }; return(R); }

inline v4 &operator+=(v4& A, v4 B) { return(A = A + B); }
inline v4 &operator-=(v4& A, v4 B) { return(A = A - B); }
inline v4 &operator*=(v4& A, float S) { return(A = A * S); }
inline v4 &operator/=(v4& A, float S) { return(A = A / S); }

/*quat operator overloading*/
inline quat operator+(quat A) { return(A); }
inline quat operator-(quat A) { return(A); }

inline quat operator+(quat A, quat B) { return Add(A, B); }
inline quat operator-(quat A, quat B) { return Sub(A, B); }

inline quat operator*(quat A, float S) { return Mul(A, S); }
inline quat operator*(float S, quat A) { return Mul(A, S); }
inline quat operator/(quat A, float S) { return Div(A, S); }

inline quat operator*(quat A, quat B) { return(Mul(A, B)); }

inline quat &operator+=(quat& A, quat B) { return(A = A + B); }
inline quat &operator-=(quat& A, quat B) { return(A = A - B); }
inline quat &operator*=(quat& A, float S) { return(A = A * S); }
inline quat &operator/=(quat& A, float S) { return(A = A / S); }

inline v3 operator*(v3 A, m33 B){
    v3 Result = Mul(A, B);
    
    return(Result);
}

inline v4 operator*(v4 A, m44 B){
    v4 Result = Mul(A, B);
    
    return(Result);
}

inline m44 operator*(m44 A, m44 B){
    m44 Result = Mul(A, B);
    
    return(Result);
}

/*Normalization operations*/
inline v2 Normalize(v2 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline v3 Normalize(v3 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline v4 Normalize(v4 A) { return(Mul(A, RSqrt(Dot(A, A)))); }
inline quat Normalize(quat A) { return(Mul(A, RSqrt(Dot(A, A)))); }

/*Safe normalization operations*/
inline v2 NOZ(v2 A) { float SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V2(0.0f, 0.0f) : A * RSqrt(SqMag)); }
inline v3 NOZ(v3 A) { float SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V3(0.0f, 0.0f, 0.0f) : A * RSqrt(SqMag)); }
inline v4 NOZ(v4 A) { float SqMag = Dot(A, A); return((SqMag) < 0.0000001f ? V4(0.0f, 0.0f, 0.0f, 0.0f) : A * RSqrt(SqMag)); }

/*Lerp operations*/
inline v2 Lerp(v2 A, v2 B, float t) { return((1.0f - t) * A + B * t); }
inline v3 Lerp(v3 A, v3 B, float t) { return((1.0f - t) * A + B * t); }
inline v4 Lerp(v4 A, v4 B, float t) { return((1.0f - t) * A + B * t); }

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

inline quat Lerp(quat A, quat B, float t) {
    quat Result;
    
    float OneMinusT = 1.0f - t;
    
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

inline quat Slerp(quat A, quat B, float t){
    A = Normalize(A);
    B = Normalize(B);
    
    float CosTheta = Dot(A, B);
    float Theta = ACos(CosTheta);
    
    float OneOverSinTheta = 1.0f / Sin(Theta);
    float cA = OneOverSinTheta * Sin((1.0f - t) * Theta);
    float cB = OneOverSinTheta * Sin(t * Theta);
    
    quat Result;
    
    Result.x = cA * A.x + cB * B.x;
    Result.y = cA * A.y + cB * B.y;
    Result.z = cA * A.z + cB * B.z;
    Result.w = cA * A.w + cB * B.w;
    
    return(Result);
}

// NOTE(Dima): Matrices operations

inline m44 LookAt(v3 Pos, v3 TargetPos, v3 WorldUp) {
    m44 Result;
    
    v3 Fwd = TargetPos - Pos;
    Fwd = NOZ(Fwd);
    
    v3 Left = Normalize(Cross(WorldUp, Fwd));
    v3 Up = Normalize(Cross(Fwd, Left));
    
    v3 Eye = Pos;
    
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

inline m44 ScalingMatrix(float Scale)
{
    m44 Result = ScalingMatrix(V3(Scale, Scale, Scale));
    
    return(Result);
}

inline m44 RotationMatrixX(float Angle)
{
    m44 Result = IdentityMatrix4();
    
    float s = Sin(Angle);
    float c = Cos(Angle);
    
    Result.e[5] = c;
    Result.e[6] = s;
    Result.e[9] = -s;
    Result.e[10] = c;
    
    return(Result);
}

inline m44 RotationMatrixY(float Angle)
{
    m44 Result = IdentityMatrix4();
    
    float s = Sin(Angle);
    float c = Cos(Angle);
    
    Result.e[0] = c;
    Result.e[2] = -s;
    Result.e[8] = s;
    Result.e[10] = c;
    
    return(Result);
}

inline m44 RotationMatrixZ(float Angle)
{
    m44 Result = IdentityMatrix4();
    
    float s = Sin(Angle);
    float c = Cos(Angle);
    
    Result.e[0] = c;
    Result.e[1] = s;
    Result.e[4] = -s;
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

inline m44 InverseScalingMatrix(float Scaling)
{
    m44 Result = ScalingMatrix(1.0f / Scaling);
    
    return(Result);
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
    
    float ScaleX = Length(RowX);
    float ScaleY = Length(RowY);
    float ScaleZ = Length(RowZ);
    
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
                                 float Far, float Near, 
                                 float FOVDegrees = 45.0f)
{
    m44 Result = {};
    
    float AspectRatio = (float)Width / (float)Height;
    
    float S = 1.0f / Tan(FOVDegrees * 0.5f * F_DEG2RAD);
    float A = -S / AspectRatio;
    float B = S;
    float OneOverFarMinusNear = 1.0f / (Far - Near);
    Result.e[0] = A;
    Result.e[5] = B;
    
    Result.e[10] = (Far + Near) * OneOverFarMinusNear;
    Result.e[14] = -(2.0f * Far * Near) * OneOverFarMinusNear;
    Result.e[11] = 1.0f;
    
    return(Result);
}

inline m44 OrthographicProjection(int Width, int Height)
{
    float a = 2.0f / (float)Width;
    float b = -1.0f;
    float c = -2.0f / (float)Height;
    float d = 1.0f;
    
    m44 Result = {};
    
    Result.e[0] = a;
    Result.e[5] = c;
    Result.e[10] = 1.0f;
    Result.e[12] = -1.0f;
    Result.e[13] = 1.0f;
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
    
    float x2 = Q.x * Q.x;
    float y2 = Q.y * Q.y;
    float z2 = Q.z * Q.z;
    
    float xy = Q.x * Q.y;
    float zw = Q.z * Q.w;
    float xz = Q.x * Q.z;
    float yw = Q.y * Q.w;
    float yz = Q.y * Q.z;
    float xw = Q.x * Q.w;
    
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

inline quat QuatFrom2DArray(float A[3][3]){
    quat Result;
    
    float Trace = A[0][0] + A[1][1] + A[2][2]; // I removed + 1.0f; see discussion with Ethan
    if( Trace > 0 ) {// I changed M_EPSILON to 0
        float S = 0.5f / sqrtf(Trace + 1.0f);
        Result.w = 0.25f / S;
        Result.x = ( A[1][2] - A[2][1] ) * S;
        Result.y = ( A[2][0] - A[0][2] ) * S;
        Result.z = ( A[0][1] - A[1][0] ) * S;
    } else {
        if ( A[0][0] > A[1][1] && A[0][0] > A[2][2] ) {
            float S = 2.0f * sqrtf( 1.0f + A[0][0] - A[1][1] - A[2][2]);
            Result.w = (A[1][2] - A[2][1] ) / S;
            Result.x = 0.25f * S;
            Result.y = (A[1][0] + A[0][1] ) / S;
            Result.z = (A[2][0] + A[0][2] ) / S;
        } else if (A[1][1] > A[2][2]) {
            float S = 2.0f * sqrtf( 1.0f + A[1][1] - A[0][0] - A[2][2]);
            Result.w = (A[2][0] - A[0][2] ) / S;
            Result.x = (A[1][0] + A[0][1] ) / S;
            Result.y = 0.25f * S;
            Result.z = (A[2][1] + A[1][2] ) / S;
        } else {
            float S = 2.0f * sqrtf( 1.0f + A[2][2] - A[0][0] - A[1][1] );
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
    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    Result.Roll = ATan2(sinr_cosp, cosr_cosp);
    
    // pitch (y-axis rotation)
    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (abs(sinp) >= 1.0f)
        Result.Pitch = CopySign(F_PI / 2.0f, sinp); // use 90 degrees if out of range
    else
        Result.Pitch = ASin(sinp);
    
    // yaw (z-axis rotation)
    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    Result.Yaw = ATan2(siny_cosp, cosy_cosp);
    
    return(Result);
}

inline quat EulerToQuat(v3 EulerAngles)
{
    quat Result = {};
    
    // Abbreviations for the various angular functions
    float cy = cosf(EulerAngles.Yaw * 0.5f);
    float sy = sinf(EulerAngles.Yaw * 0.5f);
    float cp = cosf(EulerAngles.Pitch * 0.5f);
    float sp = sinf(EulerAngles.Pitch * 0.5f);
    float cr = cosf(EulerAngles.Roll * 0.5f);
    float sr = sinf(EulerAngles.Roll * 0.5f);
    
    Result.w = cy * cp * cr + sy * sp * sr;
    Result.x = cy * cp * sr - sy * sp * cr;
    Result.y = sy * cp * sr + cy * sp * cr;
    Result.z = sy * cp * cr - cy * sp * sr;
    
    return(Result);
}

/* Smoothstep lerp */
inline float Smoothstep(float Value)
{
    float x = Clamp01(Value);
    
    float Result = x * x * (3.0f - 2.0f * x);
    
    return(Result);
}

/* Cosine lerp. Takes float as input that will be clamped to [0, 1] inside */
inline float CosLerp01(float t){
    float result = Cos(Clamp01(t) * F_PI * 0.5f);
    
    return(result);
}

inline v2 CosLerp(v2 a, v2 b, float t){
    float factor = CosLerp01(t);
    
    v2 Result = Lerp(a, b, t);
    
    return(Result);
}

inline v3 CosLerp(v3 a, v3 b, float t){
    float factor = CosLerp01(t);
    
    v3 Result = Lerp(a, b, t);
    
    return(Result);
}

inline v4 CosLerp(v4 a, v4 b, float t){
    float factor = CosLerp01(t);
    
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
	float OneOver255 = 1.0f / 255.0f;
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
	float OneOver255 = 1.0f / 255.0f;
    
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
    
    Res.r = (float)(Color & 0xFF) * F_ONE_OVER_255;
    Res.g = (float)((Color >> 8) & 0xFF) * F_ONE_OVER_255;
    Res.b = (float)((Color >> 16) & 0xFF) * F_ONE_OVER_255;
    Res.a = (float)((Color >> 24) & 0xFF) * F_ONE_OVER_255;
    
    return(Res);
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
    
    Result.r = (float)(Color & 31) / 31.0f;
    Result.g = (float)((Color >> 5) & 63) / 63.0f;
    Result.b = (float)((Color >> 11) & 31) / 31.0f;
    
    return(Result);
}

// NOTE(Dima): Rectangle functions
inline v2 GetDim(rc2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    
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

#endif