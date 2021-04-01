// NOTE(Dima): Functions after that line must be executed on platform layer
INTERNAL_FUNCTION void JobWorkerThreadFunction(job_queue* Queue)
{
    for(;;)
    {
        if(ShouldSleepAfterPerformJob(Queue))
        {
            std::unique_lock<std::mutex> UniqueLock(*Queue->SygnalLock);
            Queue->Sygnal->wait(UniqueLock);
        }
    }
}

INTERNAL_FUNCTION void InitJobQueue(memory_arena* Arena, 
                                    job_queue* Queue, 
                                    int ThreadCount,
                                    int JobCount)
{
    // NOTE(Dima): Allocate jobs array
    Queue->Jobs = PushArray(Arena, job, JobCount);
    Queue->JobsCount = JobCount;
    
    // NOTE(Dima): Init indices
    Queue->AddIndex = 0;
    Queue->DoIndex = 0;
    
    Queue->Started = 0;
    Queue->Finished = 0;
    
    Queue->Lock = PushNew<std::mutex>(Arena);
    Queue->SygnalLock = PushNew<std::mutex>(Arena);
    Queue->Sygnal = PushNew<std::condition_variable>(Arena);
    
    // NOTE(Dima): Init threads
    Queue->CreationThreadCount = ThreadCount;
}

INTERNAL_FUNCTION void StartJobQueueThreads(job_system* JobSystem)
{
    // NOTE(Dima): Queues initializing
    for(int QueueIndex = 0;
        QueueIndex < JobPriority_Count;
        QueueIndex++)
    {
        job_queue* Queue = &JobSystem->Queues[QueueIndex];
        
        int ThreadCount = Queue->CreationThreadCount;
        
        for(int ThreadIndex = 0;
            ThreadIndex < ThreadCount;
            ThreadIndex++)
        {
            std::thread NewThread(JobWorkerThreadFunction, Queue);
            NewThread.detach();
        }
    }
}

INTERNAL_FUNCTION job_system* InitJobSystem(memory_arena* Arena, 
                                            int* ThreadCounts,
                                            int* JobCounts)
{
    job_system* Result = PushStruct(Arena, job_system);
    
    // NOTE(Dima): Queues initializing
    for(int QueueIndex = 0;
        QueueIndex < JobPriority_Count;
        QueueIndex++)
    {
        job_queue* Queue = &Result->Queues[QueueIndex];
        
        int ThreadCount = ThreadCounts[QueueIndex];
        int JobCount = JobCounts[QueueIndex];
        
        InitJobQueue(Arena, Queue, ThreadCount, JobCount);
    }
    
    StartJobQueueThreads(Result);
    
    return(Result);
}