#ifndef FLOWER_MATH_H
#define FLOWER_MATH_H

// NOTE(Dima): Structures
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
};

struct m44
{
    float e[16];
};

// NOTE(Dima): Basic functions
inline float Sin(float Rad) {
	float res = sinf(Rad);
	return(res);
}

inline float Cos(float Rad) {
	float res = cosf(Rad);
	return(res);
}

// NOTE(Dima): Vectors functions
inline v3 V3(float x, float y, float z)
{
    v3 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return(Result);
}

inline v3 Lerp(v3 a, v3 b, float t)
{
    v3 Result;
    
    Result.x = a.x + (b.x - a.x) * t;
    Result.y = a.y + (b.y - a.y) * t;
    Result.z = a.z + (b.z - a.z) * t;
    
    return(Result);
}

// NOTE(Dima): Matrices functions
inline m44 Identity(){
    m44 Result = {};
    
    Result.e[0] = 1.0f;
    Result.e[5] = 1.0f;
    Result.e[10] = 1.0f;
    Result.e[15] = 1.0f;
    
    return(Result);
}

#endif
