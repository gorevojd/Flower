#ifndef FLOWER_DEFINES_H
#define FLOWER_DEFINES_H

#include <stdint.h>

#define INTERNAL_BUILD

#if defined(INTERNAL_BUILD)
#define Assert(cond) if(!(cond)){ *((int*)0) = 0;}
#else
#define Assert(cond)
#endif

#define InvalidCodePath Assert(!"Invalid code path")

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

#define GlobalVariable static
#define InternalFunction static
#define LocalPersist static

#define GLOBAL_VARIABLE static
#define INTERNAL_FUNCTION static
#define LOCAL_PERSIST static

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#define Kilobytes(count) ((count) * 1000)
#define Megabytes(count) ((count) * 1000000)
#define Gigabytes(count) ((count) * 1000000000)

#define Kibibytes(count) ((count) * 1024)
#define Mibibytes(count) ((count) * 1024 * 1024)
#define Gibibytes(count) ((count) * 1024 * 1024 * 1024)

#define Align(value, align) ((value + (align - 1)) & (~(align - 1)))

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef int b32;

typedef float f32;
typedef double f64;

// NOTE(Dima): Memory index
typedef size_t mi;

#endif