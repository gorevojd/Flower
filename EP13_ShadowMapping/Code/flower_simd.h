#ifndef FLOWER_SIMD_H
#define FLOWER_SIMD_H

#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#define MM4(mm, i) (mm).m128_f32[i]
#define MMI4(mm, i) (mm).m128i_u32[i]

#define MM_LERP(a, b, t) _mm_add_ps(a, _mm_mul_ps(_mm_sub_ps(b, a), t))

struct i32_4x{
    __m128i e;
};

struct f32_4x{
    __m128 e;
};

struct v2_4x{
    union{
        struct{
            
            __m128 x;
            __m128 y;
        };
        
        __m128 e[2];
    };
};

struct v3_4x{
    union{
        struct{
            __m128 x;
            __m128 y;
            __m128 z;
        };
        
        __m128 e[3];
    };
};

struct v4_4x{
    union{
        struct{
            __m128 x;
            __m128 y;
            __m128 z;
            __m128 w;
        };
        
        __m128 e[4];
    };
};

struct m44_4x{
    union{
        struct{
            __m128 m00;
            __m128 m01;
            __m128 m02;
            __m128 m03;
            
            __m128 m10;
            __m128 m11;
            __m128 m12;
            __m128 m13;
            
            __m128 m20;
            __m128 m21;
            __m128 m22;
            __m128 m23;
            
            __m128 m30;
            __m128 m31;
            __m128 m32;
            __m128 m33;
        };
        
        __m128 e[16];
        v4_4x Rows[4];
    };
};

// *****************************
// NOTE(Dima): Contstructors int
// *****************************
inline i32_4x I32_4X(int A)
{
    i32_4x Result;
    
    Result.e = _mm_set1_epi32(A);
    
    return(Result);
}

inline i32_4x I32_4X(int A, int B, int C, int D)
{
    i32_4x Result;
    
    Result.e = _mm_setr_epi32(A, B, C, D);
    
    return(Result);
}

inline i32_4x IndicesStartFrom(int StartIndex, int Step = 1)
{
    i32_4x Result;
    
    int Index0 = StartIndex + 0 * Step;
    int Index1 = StartIndex + 1 * Step;
    int Index2 = StartIndex + 2 * Step;
    int Index3 = StartIndex + 3 * Step;
    
    Result.e = _mm_setr_epi32(Index0,
                              Index1,
                              Index2,
                              Index3);
    
    return(Result);
}

inline i32_4x I32_4X(__m128i Value)
{
    i32_4x Result;
    
    Result.e = Value;
    
    return(Result);
}

inline i32_4x I32_4X_Load(int* Src, b32 Aligned)
{
    i32_4x Result;
    
    Result.e =  _mm_loadu_si128((__m128i*)Src);
    
    return(Result);
}

// ******************************
// NOTE(Dima): Constructors float
// ******************************

inline f32_4x F32_4X(f32 A)
{
    f32_4x Result;
    
    Result.e = _mm_set1_ps(A);
    
    return(Result);
}

inline f32_4x F32_4X(f32 A, f32 B, f32 C, f32 D)
{
    f32_4x Result;
    
    Result.e = _mm_setr_ps(A, B, C, D);
    
    return(Result);
}

inline f32_4x F32_4X(__m128 Value)
{
    f32_4x Result;
    
    Result.e = Value;
    
    return(Result);
}

inline f32_4x F32_4X_Load(f32* Src)
{
    f32_4x Result;
    
    Result.e = _mm_loadu_ps(Src);
    
    return(Result);
}

#define SIMD_QUICK_PARAM_FILL(after_dot) _mm_setr_ps(A.##after_dot, \
B.##after_dot, \
C.##after_dot, \
D.##after_dot)

#define SIMD_QUICK_PARAM_STORE(to_store, after_dot) {\
A.##after_dot = MM4(to_store.##after_dot, 0);\
B.##after_dot = MM4(to_store.##after_dot, 1);\
C.##after_dot = MM4(to_store.##after_dot, 2);\
D.##after_dot = MM4(to_store.##after_dot, 3);}



// ***************************
// NOTE(Dima): Constructors V2
// ***************************
inline v2_4x V2_4X(const v2& A,
                   const v2& B,
                   const v2& C,
                   const v2& D)
{
    v2_4x Result;
    
    Result.x = SIMD_QUICK_PARAM_FILL(x);
    Result.y = SIMD_QUICK_PARAM_FILL(y);
    
    return(Result);
}

inline v2_4x V2_4X(const v2& A)
{
    v2_4x Result;
    
    Result.x = _mm_set1_ps(A.x);
    Result.y = _mm_set1_ps(A.y);
}

inline v2_4x V2_4X_Load(v2* Src)
{
    __m128 a = _mm_loadu_ps((f32*)Src);
    __m128 b = _mm_loadu_ps((f32*)(Src + 1));
    
    v2_4x Result;
    
    Result.x = _mm_shuffle_ps(a, b, 0x88);
    Result.y = _mm_shuffle_ps(a, b, 0xDD);
    
    return(Result);
}

// ***************************
// NOTE(Dima): Constructors V3
// ***************************
inline v3_4x V3_4X(const v3& A,
                   const v3& B,
                   const v3& C,
                   const v3& D)
{
    v3_4x Result;
    
    Result.x = SIMD_QUICK_PARAM_FILL(x);
    Result.y = SIMD_QUICK_PARAM_FILL(y);
    Result.z = SIMD_QUICK_PARAM_FILL(z);
    
    return(Result);
}

inline v3_4x V3_4X(const v3& A)
{
    v3_4x Result;
    
    Result.x = _mm_set1_ps(A.x);
    Result.y = _mm_set1_ps(A.y);
    Result.z = _mm_set1_ps(A.z);
    
    return(Result);
}

inline v3_4x V3_4X_Load(f32* Src)
{
    __m128 a = _mm_loadu_ps(Src); // [0, 1, 2, 3]
    __m128 b = _mm_loadu_ps(Src + 4); // [4, 5, 6, 7]
    __m128 c = _mm_loadu_ps(Src + 8); // [8, 9, 10, 11]
    
    __m128 t1 = _mm_shuffle_ps(a, b, 0x49); // [1, 2, 4, 5]
    __m128 t2 = _mm_shuffle_ps(b, c, 0x9E); // [6, 7, 9, 10]
    
    v3_4x Result;
    Result.x = _mm_shuffle_ps(a, t2, 0x8C); // [0, 3, 6, 9]
    Result.y = _mm_shuffle_ps(t1, t2, 0xD8); // [1, 4, 7, 10]
    Result.z = _mm_shuffle_ps(t1, c, 0xCD); // [2, 5, 8, 11]
    
    return(Result);
}


// ***************************
// NOTE(Dima): Constructors V4
// ***************************

inline v4_4x V4_4X(const v4& A,
                   const v4& B,
                   const v4& C,
                   const v4& D)
{
    v4_4x Result;
    
    Result.x = SIMD_QUICK_PARAM_FILL(x);
    Result.y = SIMD_QUICK_PARAM_FILL(y);
    Result.z = SIMD_QUICK_PARAM_FILL(z);
    Result.w = SIMD_QUICK_PARAM_FILL(w);
    
    return(Result);
}

inline v4_4x V4_4X(const v4& A)
{
    v4_4x Result;
    
    Result.x = _mm_set1_ps(A.x);
    Result.y = _mm_set1_ps(A.y);
    Result.z = _mm_set1_ps(A.z);
    Result.w = _mm_set1_ps(A.w);
    
    return(Result);
}

inline v4_4x V4_4X(const quat& A)
{
    v4_4x Result;
    
    Result.x = _mm_set1_ps(A.x);
    Result.y = _mm_set1_ps(A.y);
    Result.z = _mm_set1_ps(A.z);
    Result.w = _mm_set1_ps(A.w);
    
    return(Result);
}

inline v4_4x V4_4X(const quat& A,
                   const quat& B,
                   const quat& C,
                   const quat& D)
{
    v4_4x Result;
    
    Result.x = SIMD_QUICK_PARAM_FILL(x);
    Result.y = SIMD_QUICK_PARAM_FILL(y);
    Result.z = SIMD_QUICK_PARAM_FILL(z);
    Result.w = SIMD_QUICK_PARAM_FILL(w);
    
    return(Result);
}

inline v4_4x V4_4X(const __m128& x,
                   const __m128& y,
                   const __m128& z,
                   const __m128& w)
{
    v4_4x Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return(Result);
}

inline v4_4x Shuffle4(const __m128& a,
                      const __m128& b,
                      const __m128& c,
                      const __m128& d)
{
    __m128 t1 = _mm_shuffle_ps(a, b, 0x88); // [0, 2, 4, 6]
    __m128 t2 = _mm_shuffle_ps(a, b, 0xDD); // [1, 3, 5, 7]
    __m128 t3 = _mm_shuffle_ps(c, d, 0x88); // [8, 10, 12, 14]
    __m128 t4 = _mm_shuffle_ps(c, d, 0xDD); // [9, 11, 13, 15]
    
    v4_4x Result;
    
    Result.x = _mm_shuffle_ps(t1, t3, 0x88); // [0, 4, 8, 12]
    Result.y = _mm_shuffle_ps(t2, t4, 0x88); // [1, 5, 9, 13]
    Result.z = _mm_shuffle_ps(t1, t3, 0xDD); // [2, 6, 10, 14]
    Result.w = _mm_shuffle_ps(t2, t4, 0xDD); // [3, 7, 11, 15]
    
    return(Result);
}

inline v4_4x Shuffle4(const v4_4x& Value)
{
    v4_4x Result = Shuffle4(Value.x,
                            Value.y,
                            Value.z,
                            Value.w);
    
    return(Result);
}

inline v4_4x V4_4X_Load(f32* Src)
{
    __m128 a = _mm_loadu_ps(Src); // [0, 1, 2, 3]
    __m128 b = _mm_loadu_ps(Src + 4); // [4, 5, 6, 7]
    __m128 c = _mm_loadu_ps(Src + 8); // [8, 9, 10, 11]
    __m128 d = _mm_loadu_ps(Src + 12); // [12, 13, 14, 15]
    
    v4_4x Result = Shuffle4(a, b, c, d);
    
    return(Result);
}

// ****************************
// NOTE(Dima): Constructors M44
// ****************************
inline m44_4x M44_4X(const m44& A,
                     const m44& B,
                     const m44& C,
                     const m44& D)
{
    m44_4x Result;
    
    Result.e[0] = SIMD_QUICK_PARAM_FILL(e[0]);
    Result.e[1] = SIMD_QUICK_PARAM_FILL(e[1]);
    Result.e[2] = SIMD_QUICK_PARAM_FILL(e[2]);
    Result.e[3] = SIMD_QUICK_PARAM_FILL(e[3]);
    
    Result.e[4] = SIMD_QUICK_PARAM_FILL(e[4]);
    Result.e[5] = SIMD_QUICK_PARAM_FILL(e[5]);
    Result.e[6] = SIMD_QUICK_PARAM_FILL(e[6]);
    Result.e[7] = SIMD_QUICK_PARAM_FILL(e[7]);
    
    Result.e[8] = SIMD_QUICK_PARAM_FILL(e[8]);
    Result.e[9] = SIMD_QUICK_PARAM_FILL(e[9]);
    Result.e[10] = SIMD_QUICK_PARAM_FILL(e[10]);
    Result.e[11] = SIMD_QUICK_PARAM_FILL(e[11]);
    
    Result.e[12] = SIMD_QUICK_PARAM_FILL(e[12]);
    Result.e[13] = SIMD_QUICK_PARAM_FILL(e[13]);
    Result.e[14] = SIMD_QUICK_PARAM_FILL(e[14]);
    Result.e[15] = SIMD_QUICK_PARAM_FILL(e[15]);
    
    return(Result);
}

inline m44_4x M44_4X(const m44& A)
{
    m44_4x Result;
    
    Result.e[0] = _mm_set1_ps(A.e[0]);
    Result.e[1] = _mm_set1_ps(A.e[1]);
    Result.e[2] = _mm_set1_ps(A.e[2]);
    Result.e[3] = _mm_set1_ps(A.e[3]);
    
    Result.e[4] = _mm_set1_ps(A.e[4]);
    Result.e[5] = _mm_set1_ps(A.e[5]);
    Result.e[6] = _mm_set1_ps(A.e[6]);
    Result.e[7] = _mm_set1_ps(A.e[7]);
    
    Result.e[8] = _mm_set1_ps(A.e[8]);
    Result.e[9] = _mm_set1_ps(A.e[9]);
    Result.e[10] = _mm_set1_ps(A.e[10]);
    Result.e[11] = _mm_set1_ps(A.e[11]);
    
    Result.e[12] = _mm_set1_ps(A.e[12]);
    Result.e[13] = _mm_set1_ps(A.e[13]);
    Result.e[14] = _mm_set1_ps(A.e[14]);
    Result.e[15] = _mm_set1_ps(A.e[15]);
    
    return(Result);
}

inline m44_4x M44_4X_Load(f32* Src)
{
    v4_4x t1 = V4_4X_Load(Src);
    v4_4x t2 = V4_4X_Load(Src + 16);
    v4_4x t3 = V4_4X_Load(Src + 32);
    v4_4x t4 = V4_4X_Load(Src + 48);
    
    v4_4x r1 = Shuffle4(t1.x, t2.x, t3.x, t4.x);
    v4_4x r2 = Shuffle4(t1.y, t2.y, t3.y, t4.y);
    v4_4x r3 = Shuffle4(t1.z, t2.z, t3.z, t4.z);
    v4_4x r4 = Shuffle4(t1.w, t2.w, t3.w, t4.w);
    
    m44_4x Result;
    
    Result.e[0] = r1.x;
    Result.e[1] = r2.x;
    Result.e[2] = r3.x;
    Result.e[3] = r4.x;
    
    Result.e[4] = r1.y;
    Result.e[5] = r2.y;
    Result.e[6] = r3.y;
    Result.e[7] = r4.y;
    
    Result.e[8] = r1.z;
    Result.e[9] = r2.z;
    Result.e[10] = r3.z;
    Result.e[11] = r4.z;
    
    Result.e[12] = r1.w;
    Result.e[13] = r2.w;
    Result.e[14] = r3.w;
    Result.e[15] = r4.w;
    
    return(Result);
}

inline m44_4x TranslationMatrix(const v3& A,
                                const v3& B,
                                const v3& C,
                                const v3& D)
{
    m44_4x Result;
    
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One = _mm_set1_ps(1.0f);
    
    Result.e[0] = One;
    Result.e[1] = Zero;
    Result.e[2] = Zero;
    Result.e[3] = Zero;
    
    Result.e[4] = Zero;
    Result.e[5] = One;
    Result.e[6] = Zero;
    Result.e[7] = Zero;
    
    Result.e[8] = Zero;
    Result.e[9] = Zero;
    Result.e[10] = One;
    Result.e[11] = Zero;
    
    Result.e[12] = SIMD_QUICK_PARAM_FILL(x);
    Result.e[13] = SIMD_QUICK_PARAM_FILL(y);
    Result.e[14] = SIMD_QUICK_PARAM_FILL(z);
    Result.e[15] = One;
    
    return(Result);
}

inline m44_4x ScalingMatrix(const v3& A,
                            const v3& B,
                            const v3& C,
                            const v3& D)
{
    m44_4x Result;
    
    __m128 Zero = _mm_set1_ps(0.0f);
    
    __m128 x = SIMD_QUICK_PARAM_FILL(x);
    __m128 y = SIMD_QUICK_PARAM_FILL(y);
    __m128 z = SIMD_QUICK_PARAM_FILL(z);
    
    Result.e[0] = x;
    Result.e[1] = Zero;
    Result.e[2] = Zero;
    Result.e[3] = Zero;
    
    Result.e[4] = Zero;
    Result.e[5] = y;
    Result.e[6] = Zero;
    Result.e[7] = Zero;
    
    Result.e[8] = Zero;
    Result.e[9] = Zero;
    Result.e[10] = z;
    Result.e[11] = Zero;
    
    Result.e[12] = Zero;
    Result.e[13] = Zero;
    Result.e[14] = Zero;
    Result.e[15] = _mm_set1_ps(1.0f);
    
    return(Result);
}

inline m44_4x RotationMatrix(const quat& A,
                             const quat& B,
                             const quat& C,
                             const quat& D)
{
    m44_4x Result;
    
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One = _mm_set1_ps(1.0f);
    __m128 Two = _mm_set1_ps(2.0f);
    
    __m128 x = SIMD_QUICK_PARAM_FILL(x);
    __m128 y = SIMD_QUICK_PARAM_FILL(y);
    __m128 z = SIMD_QUICK_PARAM_FILL(z);
    __m128 w = SIMD_QUICK_PARAM_FILL(w);
    
    __m128 xx = _mm_mul_ps(x, x);
    __m128 yy = _mm_mul_ps(y, y);
    __m128 zz = _mm_mul_ps(z, z);
    
    __m128 xy = _mm_mul_ps(x, y);
    __m128 zw = _mm_mul_ps(z, w);
    __m128 xz = _mm_mul_ps(x, z);
    __m128 yw = _mm_mul_ps(y, w);
    __m128 yz = _mm_mul_ps(y, z);
    __m128 xw = _mm_mul_ps(x, w);
    
    Result.e[0] = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(yy, zz)));
    Result.e[1] = _mm_mul_ps(Two, _mm_add_ps(xy, zw));
    Result.e[2] = _mm_mul_ps(Two, _mm_sub_ps(xz, yw));
    Result.e[3] = Zero;
    
    Result.e[4] = _mm_mul_ps(Two, _mm_sub_ps(xy, zw));
    Result.e[5] = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(xx, zz)));
    Result.e[6] = _mm_mul_ps(Two, _mm_add_ps(yz, xw));
    Result.e[7] = Zero;
    
    Result.e[8] = _mm_mul_ps(Two, _mm_add_ps(xz, yw));
    Result.e[9] = _mm_mul_ps(Two, _mm_sub_ps(yz, xw));
    Result.e[10] = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(xx, yy)));
    Result.e[11] = Zero;
    
    Result.e[12] = Zero;
    Result.e[13] = Zero;
    Result.e[14] = Zero;
    Result.e[15] = One;
    
    return(Result);
}

// ****************************
// NOTE(Dima): Store operations
// ****************************
inline void I32_4X_Store(int* Dst, const i32_4x& ToStore)
{
    _mm_storeu_si128((__m128i*)Dst, ToStore.e);
}

inline void F32_4X_Store(f32* Dst, const f32_4x& ToStore)
{
    _mm_storeu_ps(Dst, ToStore.e);
}


inline void V2_4X_Store(const v2_4x& Ref, v2& A, v2& B, v3& C, v3& D)
{
    SIMD_QUICK_PARAM_STORE(Ref, x);
    SIMD_QUICK_PARAM_STORE(Ref, y);
}

inline void V2_4X_Store(f32* Dst, const v2_4x& Ref)
{
    __m128 s1 = _mm_shuffle_ps(Ref.x, Ref.y, 0x88);
    __m128 s2 = _mm_shuffle_ps(Ref.x, Ref.y, 0xDD);
    
    __m128 t1 = _mm_shuffle_ps(s1, s2, 0x88);
    __m128 t2 = _mm_shuffle_ps(s1, s2, 0xDD);
    
    _mm_storeu_ps(Dst, t1);
    _mm_storeu_ps(Dst + 4, t2);
}

inline void V3_4X_Store(const v3_4x& Ref, v3& A, v3& B, v3& C, v3& D)
{
    SIMD_QUICK_PARAM_STORE(Ref, x);
    SIMD_QUICK_PARAM_STORE(Ref, y);
    SIMD_QUICK_PARAM_STORE(Ref, z);
}

inline void V3_4X_Store(f32* Dst, const v3_4x& Ref)
{
    __m128 a = Ref.x; // [0, 3, 6, 9]
    __m128 b = Ref.y; // [1, 3, 7, 10]
    __m128 c = Ref.z; // [2, 5, 8, 11]
    
    __m128 t1 = _mm_shuffle_ps(a, b, 0x44); // [0, 3, 1, 4]
    __m128 t2 = _mm_shuffle_ps(a, b, 0xEE); // [6, 9, 7, 10]
    __m128 t3 = _mm_shuffle_ps(a, c, 0xDD); // [3, 9, 5, 11]
    __m128 t4 = _mm_shuffle_ps(c, t3, 0x44); // [2, 5, 3, 9]
    __m128 t5 = _mm_shuffle_ps(b, c, 0x99); // [4, 7, 5, 8]
    __m128 t6 = _mm_shuffle_ps(b, c, 0xBB); //[10, 7, 11, 8] 
    __m128 t7 = _mm_shuffle_ps(t6, a, 0xBB);
    
    __m128 r1 = _mm_shuffle_ps(t1, t4, 0x88); //[0, 1, 2, 3]
    __m128 r2 = _mm_shuffle_ps(t5, t2, 0x88); //[4, 5, 6, 7]
    __m128 r3 = _mm_shuffle_ps(t7, t6, 0x88); //[8, 9, 10, 11]
    
    _mm_storeu_ps(Dst, r1);
    _mm_storeu_ps(Dst + 4, r2);
    _mm_storeu_ps(Dst + 8, r3);
}

inline void V4_4X_Store(const v4_4x& Ref, 
                        v4& A, v4& B,
                        v4& C, v4& D)
{
    SIMD_QUICK_PARAM_STORE(Ref, x);
    SIMD_QUICK_PARAM_STORE(Ref, y);
    SIMD_QUICK_PARAM_STORE(Ref, z);
    SIMD_QUICK_PARAM_STORE(Ref, w);
}


inline void V4_4X_Store(f32* Dst, const v4_4x& Ref)
{
    v4_4x ToStoreShuffled = Shuffle4(Ref.x, Ref.y, Ref.z, Ref.w);
    
    _mm_storeu_ps(Dst, ToStoreShuffled.x);
    _mm_storeu_ps(Dst + 4, ToStoreShuffled.y);
    _mm_storeu_ps(Dst + 8, ToStoreShuffled.z);
    _mm_storeu_ps(Dst + 12, ToStoreShuffled.w);
}

inline void M44_4X_Store(const m44_4x& Ref,
                         m44& A, m44& B,
                         m44& C, m44& D)
{
    SIMD_QUICK_PARAM_STORE(Ref, e[0]);
    SIMD_QUICK_PARAM_STORE(Ref, e[1]);
    SIMD_QUICK_PARAM_STORE(Ref, e[2]);
    SIMD_QUICK_PARAM_STORE(Ref, e[3]);
    SIMD_QUICK_PARAM_STORE(Ref, e[4]);
    SIMD_QUICK_PARAM_STORE(Ref, e[5]);
    SIMD_QUICK_PARAM_STORE(Ref, e[6]);
    SIMD_QUICK_PARAM_STORE(Ref, e[7]);
    SIMD_QUICK_PARAM_STORE(Ref, e[8]);
    SIMD_QUICK_PARAM_STORE(Ref, e[9]);
    SIMD_QUICK_PARAM_STORE(Ref, e[10]);
    SIMD_QUICK_PARAM_STORE(Ref, e[11]);
    SIMD_QUICK_PARAM_STORE(Ref, e[12]);
    SIMD_QUICK_PARAM_STORE(Ref, e[13]);
    SIMD_QUICK_PARAM_STORE(Ref, e[14]);
    SIMD_QUICK_PARAM_STORE(Ref, e[15]);
}

inline void M44_4X_Store(f32* Dst, const m44_4x& Ref)
{
    v4_4x t1 = Shuffle4(Ref.e[0],
                        Ref.e[1],
                        Ref.e[2],
                        Ref.e[3]);
    
    v4_4x t2 = Shuffle4(Ref.e[4],
                        Ref.e[5],
                        Ref.e[6],
                        Ref.e[7]);
    
    v4_4x t3 = Shuffle4(Ref.e[8],
                        Ref.e[9],
                        Ref.e[10],
                        Ref.e[11]);
    
    v4_4x t4 = Shuffle4(Ref.e[12],
                        Ref.e[13],
                        Ref.e[14],
                        Ref.e[15]);
    
    _mm_storeu_ps(Dst, t1.x);
    _mm_storeu_ps(Dst + 4, t2.x);
    _mm_storeu_ps(Dst + 8, t3.x);
    _mm_storeu_ps(Dst + 12, t4.x);
    
    _mm_storeu_ps(Dst + 16, t1.y);
    _mm_storeu_ps(Dst + 20, t2.y);
    _mm_storeu_ps(Dst + 24, t3.y);
    _mm_storeu_ps(Dst + 28, t4.y);
    
    _mm_storeu_ps(Dst + 32, t1.z);
    _mm_storeu_ps(Dst + 36, t2.z);
    _mm_storeu_ps(Dst + 40, t3.z);
    _mm_storeu_ps(Dst + 44, t4.z);
    
    _mm_storeu_ps(Dst + 48, t1.w);
    _mm_storeu_ps(Dst + 52, t2.w);
    _mm_storeu_ps(Dst + 56, t3.w);
    _mm_storeu_ps(Dst + 60, t4.w);
}

// ************************
// NOTE(Dima): Dot products
// ************************

inline __m128 DotInternal(const v2_4x& A, const v2_4x& B)
{
    __m128 Result = _mm_add_ps(_mm_mul_ps(A.x, B.x),
                               _mm_mul_ps(A.y, B.y));
    
    return(Result);
}

inline __m128 DotInternal(const v3_4x& A, const v3_4x& B)
{
    __m128 Result = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.x, B.x),
                                          _mm_mul_ps(A.y, B.y)),
                               _mm_mul_ps(A.z, B.z));
    
    return(Result);
}

inline __m128 DotInternal(const v4_4x& A, const v4_4x& B)
{
    __m128 Result = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.x, B.x),
                                          _mm_mul_ps(A.y, B.y)),
                               _mm_add_ps(_mm_mul_ps(A.z, B.z),
                                          _mm_mul_ps(A.w, B.w)));
    
    return(Result);
}


inline f32_4x Dot(const v2_4x& A, const v2_4x& B)
{
    f32_4x Result;
    Result.e = DotInternal(A, B);
    
    return(Result);
}

inline f32_4x Dot(const v3_4x& A, const v3_4x& B)
{
    f32_4x Result;
    Result.e = DotInternal(A, B);
    
    return(Result);
}

inline f32_4x Dot(const v4_4x& A, const v4_4x& B)
{
    f32_4x Result;
    Result.e = DotInternal(A, B);
    
    return(Result);
}

// *****************************
// NOTE(Dima): Matrix operations
// *****************************
inline v4_4x MulVectorByMatrixSIMD(const v4_4x& V, const m44_4x& B)
{
    v4_4x Result;
    
    Result.e[0] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(V.e[0], B.e[0]),
                                        _mm_mul_ps(V.e[1], B.e[4])),
                             _mm_add_ps(_mm_mul_ps(V.e[2], B.e[8]),
                                        _mm_mul_ps(V.e[3], B.e[12])));
    
    Result.e[1] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(V.e[0], B.e[1]),
                                        _mm_mul_ps(V.e[1], B.e[5])),
                             _mm_add_ps(_mm_mul_ps(V.e[2], B.e[9]),
                                        _mm_mul_ps(V.e[3], B.e[13])));
    
    Result.e[2] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(V.e[0], B.e[2]),
                                        _mm_mul_ps(V.e[1], B.e[6])),
                             _mm_add_ps(_mm_mul_ps(V.e[2], B.e[10]),
                                        _mm_mul_ps(V.e[3], B.e[14])));
    
    Result.e[3] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(V.e[0], B.e[3]),
                                        _mm_mul_ps(V.e[1], B.e[7])),
                             _mm_add_ps(_mm_mul_ps(V.e[2], B.e[11]),
                                        _mm_mul_ps(V.e[3], B.e[15])));
    
    return(Result);
}

// NOTE(Dima): This is really fast if everything written from scratch
inline m44_4x operator*(const m44_4x& A, const m44_4x& B)
{
    m44_4x Result;
    // NOTE(Dima): First row
    Result.e[0] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[0], B.e[0]),
                                        _mm_mul_ps(A.e[1], B.e[4])),
                             _mm_add_ps(_mm_mul_ps(A.e[2], B.e[8]),
                                        _mm_mul_ps(A.e[3], B.e[12])));
    
    Result.e[1] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[0], B.e[1]),
                                        _mm_mul_ps(A.e[1], B.e[5])),
                             _mm_add_ps(_mm_mul_ps(A.e[2], B.e[9]),
                                        _mm_mul_ps(A.e[3], B.e[13])));
    
    Result.e[2] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[0], B.e[2]),
                                        _mm_mul_ps(A.e[1], B.e[6])),
                             _mm_add_ps(_mm_mul_ps(A.e[2], B.e[10]),
                                        _mm_mul_ps(A.e[3], B.e[14])));
    
    Result.e[3] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[0], B.e[3]),
                                        _mm_mul_ps(A.e[1], B.e[7])),
                             _mm_add_ps(_mm_mul_ps(A.e[2], B.e[11]),
                                        _mm_mul_ps(A.e[3], B.e[15])));
    
    // NOTE(Dima): Second row
    Result.e[4] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[4], B.e[0]),
                                        _mm_mul_ps(A.e[5], B.e[4])),
                             _mm_add_ps(_mm_mul_ps(A.e[6], B.e[8]),
                                        _mm_mul_ps(A.e[7], B.e[12])));
    
    Result.e[5] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[4], B.e[1]),
                                        _mm_mul_ps(A.e[5], B.e[5])),
                             _mm_add_ps(_mm_mul_ps(A.e[6], B.e[9]),
                                        _mm_mul_ps(A.e[7], B.e[13])));
    
    Result.e[6] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[4], B.e[2]),
                                        _mm_mul_ps(A.e[5], B.e[6])),
                             _mm_add_ps(_mm_mul_ps(A.e[6], B.e[10]),
                                        _mm_mul_ps(A.e[7], B.e[14])));
    
    Result.e[7] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[4], B.e[3]),
                                        _mm_mul_ps(A.e[5], B.e[7])),
                             _mm_add_ps(_mm_mul_ps(A.e[6], B.e[11]),
                                        _mm_mul_ps(A.e[7], B.e[15])));
    
    
    // NOTE(Dima): Third row
    Result.e[8] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[8], B.e[0]),
                                        _mm_mul_ps(A.e[9], B.e[4])),
                             _mm_add_ps(_mm_mul_ps(A.e[10], B.e[8]),
                                        _mm_mul_ps(A.e[11], B.e[12])));
    
    Result.e[9] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[8], B.e[1]),
                                        _mm_mul_ps(A.e[9], B.e[5])),
                             _mm_add_ps(_mm_mul_ps(A.e[10], B.e[9]),
                                        _mm_mul_ps(A.e[11], B.e[13])));
    
    Result.e[10] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[8], B.e[2]),
                                         _mm_mul_ps(A.e[9], B.e[6])),
                              _mm_add_ps(_mm_mul_ps(A.e[10], B.e[10]),
                                         _mm_mul_ps(A.e[11], B.e[14])));
    
    Result.e[11] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[8], B.e[3]),
                                         _mm_mul_ps(A.e[9], B.e[7])),
                              _mm_add_ps(_mm_mul_ps(A.e[10], B.e[11]),
                                         _mm_mul_ps(A.e[11], B.e[15])));
    
    
    // NOTE(Dima): Fourth row
    Result.e[12] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[12], B.e[0]),
                                         _mm_mul_ps(A.e[13], B.e[4])),
                              _mm_add_ps(_mm_mul_ps(A.e[14], B.e[8]),
                                         _mm_mul_ps(A.e[15], B.e[12])));
    
    Result.e[13] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[12], B.e[1]),
                                         _mm_mul_ps(A.e[13], B.e[5])),
                              _mm_add_ps(_mm_mul_ps(A.e[14], B.e[9]),
                                         _mm_mul_ps(A.e[15], B.e[13])));
    
    Result.e[14] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[12], B.e[2]),
                                         _mm_mul_ps(A.e[13], B.e[6])),
                              _mm_add_ps(_mm_mul_ps(A.e[14], B.e[10]),
                                         _mm_mul_ps(A.e[15], B.e[14])));
    
    Result.e[15] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A.e[12], B.e[3]),
                                         _mm_mul_ps(A.e[13], B.e[7])),
                              _mm_add_ps(_mm_mul_ps(A.e[14], B.e[11]),
                                         _mm_mul_ps(A.e[15], B.e[15])));
    
    return(Result);
}

#endif //FLOWER_SIMD_H
