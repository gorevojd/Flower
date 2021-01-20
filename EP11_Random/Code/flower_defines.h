#ifndef FLOWER_DEFINES_H
#define FLOWER_DEFINES_H

#include <stdint.h>

#define INTERNAL_BUILD
#include <assert.h>
#if defined(INTERNAL_BUILD)
#define Assert(cond) assert(cond)
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

#define FlowerMin(a, b) ((a) < (b) ? (a) : (b))
#define FlowerMax(a, b) ((a) > (b) ? (a) : (b))

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

#define DLIST_REFLECT_PTRS(value, next, prev) {\
(value).##next = &(value); \
(value).##prev = &(value);}

#define DLIST_REFLECT_POINTER_PTRS(ptr, next, prev) {\
(ptr)->##next = (ptr); \
(ptr)->##prev = (ptr);}

#define DLIST_FREE_IS_EMPTY(free_value, next) ((free_value).##next == &(free_value))

#define DLIST_INSERT_AFTER_SENTINEL(entry_ptr, sent_value, next, prev) \
{\
(entry_ptr)->##next = (sent_value).##next##; \
(entry_ptr)->##prev = &(sent_value); \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_INSERT_BEFORE_SENTINEL(entry_ptr, sent_value, next, prev) \
{\
(entry_ptr)->##next = &(sent_value);\
(entry_ptr)->##prev = (sent_value).##prev##; \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_INSERT_AFTER(elem, after, next, prev) \
(elem)->##next = (after)->##next; \
(elem)->##prev = after; \
\
(elem)->##next##->##prev = elem; \
(elem)->##prev##->##next = elem;

#define DLIST_INSERT_BEFORE(elem, before, next, prev) \
(elem)->##next = after; \
(elem)->##prev = (after)->##prev; \
\
(elem)->##next##->##prev = elem; \
(elem)->##prev##->##next = elem;

#define DLIST_REMOVE(elem, next, prev) \
{(elem)->##prev##->##next = (elem)->##next; \
(elem)->##next##->##prev = (elem)->##prev;

#define DLIST_REMOVE_ENTRY(entry_ptr, next, prev) \
{\
entry_ptr->##next##->##prev = entry_ptr->##prev##;\
entry_ptr->##prev##->##next = entry_ptr->##next##;}

#define DLIST_REFLECT(elem, next, prev) \
(elem)->##next = elem; \
(elem)->##prev = elem;

#define DLIST_EMPTY(elem, next) ((elem)->##next == elem)

#define DLIST_FREE_IS_EMPTY(free_value, next) ((free_value).##next == &(free_value))

// NOTE(Dima): DLIST allocate function body
#define DLIST_ALLOCATE_FUNCTION_BODY(type, mem_region_ptr, next, prev, free_sent_value, use_sent_value, grow_count, result_ptr_name) \
if(DLIST_FREE_IS_EMPTY(free_sent_value, next)){\
type* Pool = PushArray(mem_region_ptr, type, (grow_count));\
for(int Index = 0; Index < (grow_count); Index++){\
type* Prim = &Pool[Index];\
DLIST_INSERT_BEFORE_SENTINEL(Prim, free_sent_value, next, prev);\
}\
}\
type* result_ptr_name = (free_sent_value).##next##; \
DLIST_REMOVE_ENTRY(Result, next, prev); \
DLIST_INSERT_BEFORE_SENTINEL(Result, use_sent_value, next, prev);

// NOTE(Dima): DLIST deallocate function body
#define DLIST_DEALLOCATE_FUNCTION_BODY(entry_ptr, next, prev, free_sent_value) \
DLIST_REMOVE_ENTRY(entry_ptr, next, prev);\
DLIST_INSERT_BEFORE_SENTINEL(entry_ptr, free_sent_value, next, prev);
#define DLIST_REFLECT_PTRS(value, next, prev) {\
(value).##next = &(value); \
(value).##prev = &(value);}

#define DLIST_REFLECT_POINTER_PTRS(ptr, next, prev) {\
(ptr)->##next = (ptr); \
(ptr)->##prev = (ptr);}

#define DLIST_FREE_IS_EMPTY(free_value, next) ((free_value).##next == &(free_value))

#define DLIST_INSERT_AFTER_SENTINEL(entry_ptr, sent_value, next, prev) \
{\
(entry_ptr)->##next = (sent_value).##next##; \
(entry_ptr)->##prev = &(sent_value); \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_INSERT_BEFORE_SENTINEL(entry_ptr, sent_value, next, prev) \
{\
(entry_ptr)->##next = &(sent_value);\
(entry_ptr)->##prev = (sent_value).##prev##; \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_INSERT_AFTER(entry_ptr, after_ptr, next, prev) \
{\
(entry_ptr)->##next = (after_ptr)->##next##; \
(entry_ptr)->##prev = (after_ptr); \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_INSERT_BEFORE(entry_ptr, before_ptr, next, prev) \
{\
(entry_ptr)->##next = (before_ptr); \
(entry_ptr)->##prev = (before_ptr)->##prev##; \
(entry_ptr)->##prev##->##next = (entry_ptr); \
(entry_ptr)->##next##->##prev = (entry_ptr);}

#define DLIST_REMOVE_ENTRY(entry_ptr, next, prev) \
{\
entry_ptr->##next##->##prev = entry_ptr->##prev##;\
entry_ptr->##prev##->##next = entry_ptr->##next##;}

#define DLIST_REMOVE_ENTIRE_LIST(from_ptr, to_ptr, next, prev) \
{\
if((from_ptr)->##next != (from_ptr)){\
(from_ptr)->##next##->##prev = (to_ptr);\
(from_ptr)->##prev##->##next = (to_ptr)->##next;\
(to_ptr)->##next##->##prev = (from_ptr)->##prev;\
(to_ptr)->##next = (from_ptr)->##next;\
DLIST_REFLECT_POINTER_PTRS(from_ptr, next, prev);}}

// NOTE(Dima): DLIST allocate function body
#define DLIST_ALLOCATE_FUNCTION_BODY(type, mem_region_ptr, next, prev, free_sent_value, use_sent_value, grow_count, result_ptr_name) \
if(DLIST_FREE_IS_EMPTY(free_sent_value, next)){\
type* Pool = PushArray(mem_region_ptr, type, (grow_count));\
for(int Index = 0; Index < (grow_count); Index++){\
type* Prim = &Pool[Index];\
DLIST_INSERT_BEFORE_SENTINEL(Prim, free_sent_value, next, prev);\
}\
}\
type* result_ptr_name = (free_sent_value).##next##; \
DLIST_REMOVE_ENTRY(Result, next, prev); \
DLIST_INSERT_BEFORE_SENTINEL(Result, use_sent_value, next, prev);

// NOTE(Dima): DLIST deallocate function body
#define DLIST_DEALLOCATE_FUNCTION_BODY(entry_ptr, next, prev, free_sent_value) \
DLIST_REMOVE_ENTRY(entry_ptr, next, prev);\
DLIST_INSERT_BEFORE_SENTINEL(entry_ptr, free_sent_value, next, prev);


#endif