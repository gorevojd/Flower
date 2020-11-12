#ifndef FLOWER_DEFINES_H
#define FLOWER_DEFINES_H

#include <stdint.h>

#define GlobalVariable static
#define InternalFunction static
#define LocalPersist static

#define GLOBAL_VARIABLE static
#define INTERNAL_FUNCTION static
#define LOCAL_PERSIST static

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

#endif