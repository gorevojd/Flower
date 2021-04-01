#ifndef FLOWER_JOBS_H
#define FLOWER_JOBS_H

#include <thread>
#include <mutex>
#include <condition_variable>

#define JOB_CALLBACK(name) void name(void* Data)
typedef JOB_CALLBACK(job_callback);

enum job_priority
{
    JobPriority_Low,
    JobPriority_High,
    
    JobPriority_Count,
};

struct job
{
    job_callback* Callback;
    void* Data;
};

struct job_queue
{
#define DEFAULT_JOBS_COUNT 8192
    job* Jobs;
    int JobsCount;
    
    std::atomic_uint AddIndex;
    std::atomic_uint DoIndex;
    
    std::atomic_uint Started;
    std::atomic_uint Finished;
    
    std::mutex* Lock;
    std::mutex* SygnalLock;
    std::condition_variable* Sygnal;
    
    int CreationThreadCount;
};

struct job_system
{
    job_queue Queues[JobPriority_Count];
};

struct task_memory
{
    task_memory* Next;
    task_memory* Prev;
    
    void* Memory;
    size_t MemorySize;
    
    // NOTE(Dima): Storing pointer to pool from which we were allocated
    struct task_memory_pool* ParentPool;
    
    memory_arena Arena;
};

enum task_memory_pool_type
{
    TaskMemoryPool_Static,
    TaskMemoryPool_Dynamic,
};

struct task_memory_pool
{
    memory_arena* Arena;
    
    u32 Type;
    
    task_memory Use;
    task_memory Free;
    
    int UseCount;
    int FreeCount;
    
    ticket_mutex Lock;
};

// NOTE(Dima): Returns true if no jobs to perform
inline b32 ShouldSleepAfterPerformJob(job_queue* Queue)
{
    b32 Result = false;
    
    std::uint32_t DoIndex = Queue->DoIndex;
    
    if(Queue->AddIndex != DoIndex)
    {
        std::uint32_t NewDoIndex = (DoIndex + 1) % Queue->JobsCount;
        
        if(Queue->DoIndex.compare_exchange_weak(DoIndex, NewDoIndex))
        {
            // NOTE(Dima): Calling job's callback
            job* Job = &Queue->Jobs[DoIndex];
            Job->Callback(Job->Data);
            
            Queue->Finished.fetch_add(1);
        }
    }
    else
    {
        Result = true;
    }
    
    return(Result);
}



#endif //FLOWER_JOBS_H
