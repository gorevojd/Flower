#ifndef FLOWER_PLATFORM_H
#define FLOWER_PLATFORM_H

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

struct platform_api
{
    platform_allocate_block* AllocateBlock;
    platform_deallocate_block* DeallocateBlock;
    
    platform_process_input* ProcessInput;
    
    platform_renderer_render* Render;
    platform_renderer_swapbuffers* SwapBuffers;
};

extern platform_api Platform;

#endif //FLOWER_PLATFORM_H
