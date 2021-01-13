#ifndef FLOWER_PLATFORM_H
#define FLOWER_PLATFORM_H

// NOTE(Dima): Memory block allocation
#define PLATFORM_ALLOCATE_BLOCK(name) struct memory_block* name(u32 Size)
typedef PLATFORM_ALLOCATE_BLOCK(platform_allocate_block);

#define PLATFORM_DEALLOCATE_BLOCK(name) void name(memory_block* Block)
typedef PLATFORM_DEALLOCATE_BLOCK(platform_deallocate_block);

// NOTE(Dima): Input functions
#define PLATFORM_PROCESS_INPUT(name) void name()
typedef PLATFORM_PROCESS_INPUT(platform_process_input);

// NOTE(Dima): Renderer functions
#define PLATFORM_RENDERER_RENDER(name) void name()
typedef PLATFORM_RENDERER_RENDER(platform_renderer_render);

#define PLATFORM_RENDERER_SWAPBUFFERS(name) void name()
typedef PLATFORM_RENDERER_SWAPBUFFERS(platform_renderer_swapbuffers);

// NOTE(Dima): Read file functions
#define PLATFORM_READ_FILE_AND_NULL_TERMINATE(name) char* name(char* FileName)
typedef PLATFORM_READ_FILE_AND_NULL_TERMINATE(platform_read_file_and_null_terminate);

#define PLATFORM_READ_FILE(name) void* name(char* FileName, u64* FileSize)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_ALLOCATE_MEMORY(name) void* name(size_t Size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_FREE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_MEMORY(platform_free_memory);

// NOTE(Dima): Thread functions
#define PLATFORM_GET_THREAD_ID(name) u16 name()
typedef PLATFORM_GET_THREAD_ID(platform_get_thread_id);

struct platform_api
{
    platform_allocate_block* AllocateBlock;
    platform_deallocate_block* DeallocateBlock;
    
    platform_process_input* ProcessInput;
    
    platform_renderer_render* Render;
    platform_renderer_swapbuffers* SwapBuffers;
    
    platform_read_file_and_null_terminate* ReadFileAndNullTerminate;
    platform_read_file* ReadFile;
    platform_allocate_memory* AllocateMemory;
    platform_free_memory* FreeMemory;
    
    platform_get_thread_id* GetThreadID;
};

extern platform_api Platform;

#include "flower_debug_api.h"

#endif //FLOWER_PLATFORM_H
