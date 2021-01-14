#ifndef FLOWER_MEM_H
#define FLOWER_MEM_H

struct memory_block
{
    void* Base;
    
    u32 Size;
    u32 Used;
    
    memory_block* Prev;
    memory_block* Next;
};

struct memory_arena
{
    memory_block* Block;
    
    memory_block StaticBlockSource;
    
    b32 IsStatic;
};

inline mi GetAdvanceToNextAlignedMem(memory_block* Block, u32 Align)
{
    mi InitOffset = (mi)((u8*)Block->Base + Block->Used);
    mi AlignMinusOne = Align - 1;
    mi NextAlignedAddress = (mi)((u8*)Block->Base + Block->Used + AlignMinusOne) & (~(AlignMinusOne));
    
    mi AdvancedToAligned = NextAlignedAddress - InitOffset;
    
    return(AdvancedToAligned);
}

inline memory_block* GetCurrentBlock(memory_arena* Arena)
{
    memory_block* Block = Arena->Block;
    
    return(Block);
}

inline mi GetUsedInCurrentBlock(memory_arena* Arena)
{
    memory_block* Block = GetCurrentBlock(Arena);
    
    mi Result = 0;
    if(Block)
    {
        Result = Block->Used;
    }
    
    return(Result);
}

inline void* AllocateFromArena(memory_arena* Arena, u32 Size, u32 Align = 16)
{
    // NOTE(Dima): Checking what block we should use
    memory_block* Block = GetCurrentBlock(Arena);
    
    // NOTE(Dima): Checking if we can allocate or not
    b32 CanAllocate;
    if(!Block)
    {
        CanAllocate = false;
    }
    else
    {
        CanAllocate = (Block->Used + Size) <= Block->Size;
    }
    
    if(Arena->IsStatic && !CanAllocate)
    {
        Assert(!"Can't allocate memory!");
    }
    
    b32 ShouldAllocateBlock = !CanAllocate && !Arena->IsStatic;
    
    // NOTE(Dima): If we can't then allocate new block (if non-static arena only)
    if(ShouldAllocateBlock)
    {
        memory_block* NewBlock = 0;
        
        b32 ShouldUseNextBlock = Block && Block->Next;
        if(ShouldUseNextBlock)
        {
            NewBlock = Block->Next;
            
            Assert(NewBlock->Prev == Block);
        }
        else
        {
#define DEFAULT_MEMORY_BLOCK_SIZE Megabytes(1)
            mi NewBlockSize = DEFAULT_MEMORY_BLOCK_SIZE;
            if(Size > DEFAULT_MEMORY_BLOCK_SIZE)
            {
                NewBlockSize = Size;
            }
            
            NewBlock = Platform.AllocateBlock(NewBlockSize);
            NewBlock->Next = 0;
            NewBlock->Prev = Block;
            if(NewBlock->Prev)
            {
                NewBlock->Prev->Next = NewBlock;
            }
            
        }
        
        
        // NOTE(Dima): Setting new arena block
        Block = NewBlock;
        Arena->Block = NewBlock;
    }
    
    
    // NOTE(Dima): Actual allocation
    mi Advance = GetAdvanceToNextAlignedMem(Block, Align);
    void* Result = (u8*)Block->Base + Block->Used + Advance;
    
    Block->Used += Size + Advance;
    
    return(Result);
}

inline void FreeArena(memory_arena* Arena, b32 JustResetData = false)
{
    memory_block* At = Arena->Block;
    
    while(At)
    {
        memory_block* PrevBlock = At->Prev;
        
        if(!JustResetData)
        {
            At->Next = At->Prev = 0;
            
            Platform.DeallocateBlock(At);
        }
        else
        {
            At->Used = 0;
            
            Arena->Block = At;
        }
        
        At = PrevBlock;
    }
    
    if(!JustResetData)
    {
        Arena->Block = 0;
    }
    
}

inline memory_arena CreateArenaInsideMemory(void* Memory, mi Size)
{
    memory_arena Result;
    
    memory_block* NewBlock = &Result.StaticBlockSource;
    
    NewBlock->Base = Memory;
    NewBlock->Size = Size;
    NewBlock->Used = 0;
    NewBlock->Prev = 0;
    
    Result.Block = NewBlock;
    Result.IsStatic = true;
    
    return(Result);
}

inline memory_arena SplitArena(memory_arena* Arena)
{
    memory_arena Result = {};
    
    if(!Arena->IsStatic && Arena->Block)
    {
        void* Base = (u8*)Arena->Block->Base + Arena->Block->Used;
        mi Size = Arena->Block->Size - Arena->Block->Used;
        
        Result = CreateArenaInsideMemory(Base, Size);
    }
    
    return(Result);
}

#define PushSize(arena, size) AllocateFromArena(arena, size)
#define PushStruct(arena, type, ...) (type*)AllocateFromArena(arena, sizeof(type), __VA_ARGS__)
#define PushArray(arena, type, count, ...) (type*)AllocateFromArena(arena, sizeof(type) * count, __VA_ARGS__)

template<typename t> inline  t* PushNew(memory_arena* Arena)
{
    void* MemoryToPlaceIn = AllocateFromArena(Arena, sizeof(t));
    
    t* Result = new (MemoryToPlaceIn) t;
    
    return(Result);
}

#endif //FLOWER_MEM_H
