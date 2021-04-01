// TODO(Dima): Stop queues receiving jobs when game code should be reloaded.

INTERNAL_FUNCTION void KickJob(job_callback* Callback, void* Data, u32 Priority)
{
    job_queue* Queue = Global_Jobs->Queues + Priority;
    
    Queue->Lock->lock();
    
    // NOTE(Dima): Checking if we can to insert
    u32 NewAddIndex = (Queue->AddIndex + 1) % Queue->JobsCount;
    Assert(NewAddIndex != Queue->DoIndex);
    
    // NOTE(Dima): Init Job
    job* Job = &Queue->Jobs[Queue->AddIndex];
    Job->Callback = Callback;
    Job->Data = Data;
    
    // NOTE(Dima): Setting new add index
    Queue->AddIndex = NewAddIndex;
    
    Queue->Started++;
    Queue->Sygnal->notify_all();
    
    Queue->Lock->unlock();
}

INTERNAL_FUNCTION void WaitForCompletion(u32 Priority)
{
    job_queue* Queue = Global_Jobs->Queues + Priority;
    
    while(Queue->Started != Queue->Finished)
    {
        ShouldSleepAfterPerformJob(Queue);
    }
    
    Queue->Started = 0;
    Queue->Finished = 0;
}

INTERNAL_FUNCTION void AddTasksToPoolFreeList(task_memory_pool* Pool,
                                              int TasksCount,
                                              mi TaskSize)
{
    memory_arena* Arena = Pool->Arena;
    
    // NOTE(Dima): Init tasks
    task_memory* Tasks = PushArray(Arena, task_memory, TasksCount);
    
    for(int TaskIndex = 0;
        TaskIndex < TasksCount;
        TaskIndex++)
    {
        task_memory* Task = &Tasks[TaskIndex];
        
        Task->Memory = PushSize(Arena, TaskSize);
        Task->MemorySize = TaskSize;
        Task->Arena = CreateArenaInsideMemory(Task->Memory, TaskSize);
        
        DLIST_INSERT_BEFORE_SENTINEL(Task, Pool->Free, Next, Prev);
    }
    Pool->FreeCount += TasksCount;
}

INTERNAL_FUNCTION task_memory* FindFitTask(task_memory_pool* Pool, 
                                           mi TaskSize)
{
    task_memory* FitTask = 0;
    
    // NOTE(Dima): Finding task with amount of memory
    task_memory* At = Pool->Free.Next;
    while(At != &Pool->Free)
    {
        if(At->MemorySize >= TaskSize)
        {
            FitTask = At;
            break;
        }
        
        At = At->Next;
    }
    
    return(FitTask);
}

// NOTE(Dima): Tasks related stuff
INTERNAL_FUNCTION task_memory_pool* CreateTaskMemoryPoolStatic(memory_arena* Arena, 
                                                               int TasksCount,
                                                               mi TaskSize)
{
    task_memory_pool* Result = PushStruct(Arena, task_memory_pool);
    
    Result->Arena = Arena;
    Result->Type = TaskMemoryPool_Static;
    InitTicketMutex(&Result->Lock);
    
    // NOTE(Dima): Init sentinels
    DLIST_REFLECT_PTRS(Result->Use, Next, Prev);
    DLIST_REFLECT_PTRS(Result->Free, Next, Prev);
    
    Result->UseCount = 0;
    Result->FreeCount = 0;
    AddTasksToPoolFreeList(Result, TasksCount, TaskSize);
    
    return(Result);
}

INTERNAL_FUNCTION task_memory_pool* CreateTaskMemoryPoolDynamic(memory_arena* Arena)
{
    task_memory_pool* Result = PushStruct(Arena, task_memory_pool);
    
    Result->Arena = Arena;
    Result->Type = TaskMemoryPool_Dynamic;
    InitTicketMutex(&Result->Lock);
    
    // NOTE(Dima): Init sentinels
    DLIST_REFLECT_PTRS(Result->Use, Next, Prev);
    DLIST_REFLECT_PTRS(Result->Free, Next, Prev);
    
    Result->UseCount = 0;
    Result->FreeCount = 0;
    
    return(Result);
}

INTERNAL_FUNCTION task_memory* GetTaskMemoryForUse(task_memory_pool* Pool, mi Size)
{
    task_memory* Result = 0;
    
    BeginTicketMutex(&Pool->Lock);
    
    if(Pool->Type == TaskMemoryPool_Static)
    {
        // NOTE(Dima): Pool free list should not be empty
        Assert(Pool->Free.Next != &Pool->Free);
        
        Result = Pool->Free.Next;
    }
    else if(Pool->Type == TaskMemoryPool_Dynamic)
    {
        
        task_memory* FitTask = FindFitTask(Pool, Size);
        
        if(!FitTask)
        {
            // NOTE(Dima): If task not found
            
            // NOTE(Dima): Extending size just a little bit.
            mi CreationTaskSize = (mi)((f32)Size * 1.2);
            AddTasksToPoolFreeList(Pool, 10, CreationTaskSize);
            
            FitTask = FindFitTask(Pool, Size);
            Assert(FitTask);
        }
        
        Result = FitTask;
    }
    else
    {
        InvalidCodePath;
    }
    
    if(Result)
    {
        // NOTE(Dima): Task should have enough memory
        Assert(Size <= Result->MemorySize);
        
        // NOTE(Dima): Remove from free list
        DLIST_REMOVE(Result, Next, Prev);
        
        // NOTE(Dima): If everything is ok - push to Use list
        DLIST_INSERT_BEFORE_SENTINEL(Result, Pool->Use, Next, Prev);
        
        Result->ParentPool = Pool;
        
        Pool->FreeCount--;
        Pool->UseCount++;
    }
    
    EndTicketMutex(&Pool->Lock);
    
    return(Result);
}

INTERNAL_FUNCTION inline void FreeTaskMemory(task_memory* Task)
{
    task_memory_pool* Pool = Task->ParentPool;
    
    BeginTicketMutex(&Pool->Lock);
    
    DLIST_REMOVE(Task, Next, Prev);
    DLIST_INSERT_BEFORE_SENTINEL(Task, Pool->Free, Next, Prev);
    
    Pool->FreeCount++;
    Pool->UseCount--;
    FreeArena(&Task->Arena);
    
    EndTicketMutex(&Pool->Lock);
}