#if defined(INTERNAL_BUILD)

void DEBUGParseNameFromUnique(char* To, int ToSize,
                              char* From)
{
    if(To && From){
        char* At = From;
        
        int Counter = 0;
        while(*At){
            if((Counter >= ToSize - 1) || 
               (*At == '|'))
            {
                break;
            }
            
            To[Counter++] = *At;
            
            At++;
        }
        
        To[Counter] = 0;
    }
}

INTERNAL_FUNCTION inline
debug_profiled_tree_node* AllocateTreeNode(debug_state* State, 
                                           debug_thread_frame* Frame)
{
    DLIST_ALLOCATE_FUNCTION_BODY(debug_profiled_tree_node, State->Arena,
                                 NextAlloc, PrevAlloc, 
                                 State->TreeNodeFree,
                                 Frame->RootTreeNodeUse,
                                 1024,
                                 Result);
    
    return(Result);
}

INTERNAL_FUNCTION inline 
void CreateSentinel4Element(debug_state* State, 
                            debug_thread_frame* Frame,
                            debug_profiled_tree_node* Element)
{
    Element->ChildSentinel = AllocateTreeNode(State, Frame);
    
    DLIST_REFLECT_POINTER_PTRS(Element->ChildSentinel, Next, Prev);
    Element->ChildSentinel->Parent = Element;
    Element->ChildSentinel->UniqueName = State->SentinelElementsName;
    Element->ChildSentinel->NameID = State->SentinelElementsNameHash;
    
    Element->ChildSentinel->ChildSentinel = 0;
}

INTERNAL_FUNCTION void ClearThreadsTable(debug_thread** Table,
                                         int TableSize)
{
    for(int Index = 0;
        Index < TableSize;
        Index++)
    {
        Table[Index] = 0;
    }
}

INTERNAL_FUNCTION void ClearStatsTable(debug_timing_stat** Table, 
                                       int TableSize)
{
    for(int Index = 0;
        Index < TableSize;
        Index++)
    {
        Table[Index] = 0;
    }
}

INTERNAL_FUNCTION inline void InitThreadFrame(debug_state* State, 
                                              debug_thread_frame* Frame)
{
    Frame->RootTreeNodeUse.UniqueName = State->RootNodesName;
    Frame->RootTreeNodeUse.NameID = State->RootNodesNameHash;
    
    
    DLIST_REFLECT_PTRS(Frame->RootTreeNodeUse, NextAlloc, PrevAlloc);
    DLIST_REFLECT_PTRS(Frame->RootTreeNodeUse, Next, Prev);
    DLIST_REFLECT_PTRS(Frame->StatUse, Next, Prev);
    
    ClearStatsTable(Frame->StatTable, DEBUG_STATS_TABLE_SIZE);
    ClearStatsTable(Frame->ToSortStats, DEBUG_STATS_TO_SORT_SIZE);
    Frame->ToSortStatsCount = 0;
    
    Frame->FrameUpdateNode = 0;
    
    CreateSentinel4Element(State, Frame, &Frame->RootTreeNodeUse);
    
    Frame->CurNode = &Frame->RootTreeNodeUse;
}

INTERNAL_FUNCTION inline void ClearThreadFrame(debug_state* State,
                                               debug_thread_frame* Frame)
{
    // NOTE(Dima): Freing profiled nodes
    DLIST_REMOVE_ENTIRE_LIST(&Frame->RootTreeNodeUse, 
                             &State->TreeNodeFree, 
                             NextAlloc, PrevAlloc);
    DLIST_REFLECT_PTRS(Frame->RootTreeNodeUse, NextAlloc, PrevAlloc);
    
    // NOTE(Dima): Freing stats 
    DLIST_REMOVE_ENTIRE_LIST(&Frame->StatUse,
                             &State->StatFree,
                             Next, Prev);
    DLIST_REFLECT_PTRS(Frame->StatUse, Next, Prev);
    
    
    ClearStatsTable(Frame->StatTable, DEBUG_STATS_TABLE_SIZE);
    
    Frame->ToSortStatsCount = 0;
    Frame->FrameUpdateNode = 0;
    
    // NOTE(Dima): Recreating childs 
    CreateSentinel4Element(State, Frame, &Frame->RootTreeNodeUse);
    //DLIST_REFLECT_POINTER_PTRS(Frame->RootTreeNodeUse.ChildSentinel, Next, Prev);
    
    Frame->CurNode = &Frame->RootTreeNodeUse;
}

INTERNAL_FUNCTION inline void ClearThreadsIndexFrame(debug_state* State, int FrameIndex){
    // NOTE(Dima): Clearing corresponding frames in threads
    debug_thread* ThreadAt = State->ThreadSentinel.NextAlloc;
    while(ThreadAt != &State->ThreadSentinel){
        debug_thread_frame* Frame = &ThreadAt->Frames[FrameIndex];
        
        ClearThreadFrame(State, Frame);
        
        ThreadAt = ThreadAt->NextAlloc;
    }
    
}

INTERNAL_FUNCTION debug_profiled_tree_node*
RequestTreeNode(debug_state* State, 
                debug_thread_frame* Frame, 
                debug_profiled_tree_node* Current,
                char* UniqueName,
                b32* Allocated)
{
    u32 HashID = StringHashFNV(UniqueName);
    
    debug_profiled_tree_node* Found = 0;
    debug_profiled_tree_node* At = Current->ChildSentinel->Next;
    while(At != Current->ChildSentinel){
        if(StringHashFNV(At->UniqueName) == HashID){
            Found = At;
            
            break;
        }
        
        At = At->Next;
    }
    
    b32 IsAllocated = false;
    if(!Found){
        IsAllocated = true;
        
        Found = AllocateTreeNode(State, Frame);
        Found->NameID = HashID;
        Found->UniqueName = UniqueName;
        Found->Parent = Current;
        Found->TimingSnapshot = {};
        CreateSentinel4Element(State, Frame, Found);
        
        DLIST_INSERT_BEFORE(Found, Current->ChildSentinel, Next, Prev);
    }
    
    if(Allocated){
        *Allocated = IsAllocated;
    }
    
    return(Found);
}

INTERNAL_FUNCTION debug_thread* 
CreateOrFindThreadForID(debug_state* State, u16 ThreadID)
{
    u32 Key = ThreadID % DEBUG_THREADS_TABLE_SIZE;
    
    debug_thread* Found = 0;
    debug_thread* At = State->ThreadHashTable[Key];
    
    if(At){
        while(At){
            if(At->ThreadID == ThreadID){
                Found = At;
                break;
            }
            
            At = At->NextInHash;
        }
    }
    
    if(!Found){
        // NOTE(Dima): Allocate the thread
        Found = PushStruct(State->Arena, debug_thread);
        
        // NOTE(Dima): Init the debug thread
        DLIST_INSERT_BEFORE_SENTINEL(Found, State->ThreadSentinel, 
                                     NextAlloc, PrevAlloc);
        Found->ThreadID = ThreadID;
        Found->WatchNodeUniqueName = 0;
        
        Found->NextInHash = State->ThreadHashTable[Key];
        State->ThreadHashTable[Key] = Found;
        
        Found->Frames = PushArray(State->Arena, debug_thread_frame, 
                                  DEBUG_PROFILED_FRAMES_COUNT);
        for(int FrameIndex = 0;
            FrameIndex < DEBUG_PROFILED_FRAMES_COUNT;
            FrameIndex++)
        {
            debug_thread_frame* Frame = &Found->Frames[FrameIndex];
            
            InitThreadFrame(State, Frame);
        }
        
        // NOTE(Dima): Increasing profiled debug thread count
        State->ProfiledThreadsCount++;
    }
    
    return(Found);
}

INTERNAL_FUNCTION debug_timing_stat* AllocateTimingStat(debug_state* State,
                                                        debug_thread_frame* Frame)
{
    DLIST_ALLOCATE_FUNCTION_BODY(debug_timing_stat, State->Arena,
                                 Next, Prev, 
                                 State->StatFree,
                                 Frame->StatUse,
                                 512,
                                 Result);
    
    return(Result);
}

INTERNAL_FUNCTION debug_timing_stat* 
CreateOrFindStatForUniqueName(debug_state* State, 
                              debug_thread_frame* Frame,
                              char* UniqueName)
{
    u32 NameID = StringHashFNV(UniqueName);
    
    u32 Key = NameID % DEBUG_STATS_TABLE_SIZE;
    
    debug_timing_stat* Found = 0;
    debug_timing_stat* StatAt = Frame->StatTable[Key];
    
    if(StatAt){
        while(StatAt){
            if(StatAt->NameID == NameID){
                Found = StatAt;
                
                break;
            }
            
            StatAt = StatAt->NextInHash;
        }
    }
    
    if(!Found){
        debug_timing_stat* New = AllocateTimingStat(State, Frame);
        
        New->UniqueName = UniqueName;
        New->NameID = NameID;
        New->Stat = {};
        // NOTE(Dima): Inserting to hash table
        New->NextInHash = Frame->StatTable[Key];
        Frame->StatTable[Key] = New;
        
        Found = New;
    }
    
    return(Found);
}

INTERNAL_FUNCTION void 
FillAndSortStats(debug_state* State, 
                 debug_thread_frame* Frame, 
                 b32 IncludingChildren)
{
    // NOTE(Dima): Filling to sort table
    debug_timing_stat* Stat = Frame->StatUse.Next;
    
    Frame->ToSortStatsCount = 0;
    
    for(int StatIndex = 0;
        StatIndex < DEBUG_STATS_TO_SORT_SIZE;
        StatIndex++)
    {
        if(Stat == &Frame->StatUse){
            break;
        }
        
        ++Frame->ToSortStatsCount;
        
        Frame->ToSortStats[StatIndex] = Stat;
        
        Stat = Stat->Next;
    }
    
    // NOTE(Dima): Sorting ToSort table by selection sort
    for(int i = 0; i < Frame->ToSortStatsCount - 1; i++){
        u64 MaxValue = GetClocksFromStat(Frame->ToSortStats[i], IncludingChildren);
        int MaxIndex = i;
        
        for(int j = i + 1; j < Frame->ToSortStatsCount; j++){
            u64 CurClocks = GetClocksFromStat(Frame->ToSortStats[j], IncludingChildren);
            if(CurClocks > MaxValue){
                MaxValue = CurClocks;
                MaxIndex = j;
            }
        }
        
        if(MaxIndex != i){
            debug_timing_stat* Temp = Frame->ToSortStats[i];
            Frame->ToSortStats[i] = Frame->ToSortStats[MaxIndex];
            Frame->ToSortStats[MaxIndex] = Temp;
        }
    }
}

INTERNAL_FUNCTION inline int IncrementFrameIndex(int Value){
    int Result = (Value + 1) % DEBUG_PROFILED_FRAMES_COUNT;
    
    return(Result);
}

INTERNAL_FUNCTION inline void
IncrementFrameIndices(debug_state* State){
    if(State->ViewFrameIndex != State->CollationFrameIndex){
        State->ViewFrameIndex = IncrementFrameIndex(State->ViewFrameIndex);
    }
    
    State->NewestFrameIndex = State->CollationFrameIndex;
    State->CollationFrameIndex = IncrementFrameIndex(State->CollationFrameIndex);
    
    if(State->CollationFrameIndex == State->OldestFrameIndex){
        State->OldestShouldBeIncremented = true;
    }
    
    if(State->OldestShouldBeIncremented){
        State->OldestFrameIndex = IncrementFrameIndex(State->OldestFrameIndex);
    }
}

INTERNAL_FUNCTION inline void
ProcessRecordsIndicesInc(debug_state* State){
    b32 ShouldIncrement = true;
    if(State->RecordingChangeRequested){
        State->RecordingChangeRequested = false;
        
        if(State->IsRecording){
            State->Filter = DEBUG_DEFAULT_FILTER_VALUE;
            ShouldIncrement = false;
        }
        else{
            State->Filter = DebugRecord_FrameBarrier;
        }
    }
    
    if(State->IsRecording && ShouldIncrement){
        IncrementFrameIndices(State);
    }
}

INTERNAL_FUNCTION void
FindFrameUpdateNode(debug_thread_frame* Frame){
    debug_profiled_tree_node* FrameUpdateNode = 0;
    
    debug_profiled_tree_node* At = Frame->RootTreeNodeUse.NextAlloc;
    
    while(At != &Frame->RootTreeNodeUse){
        char NameBuf[256];
        DEBUGParseNameFromUnique(NameBuf, 256, At->UniqueName);
        
        if(StringsAreEqual(NameBuf, FRAME_UPDATE_NODE_NAME))
        {
            FrameUpdateNode = At;
            break;
        }
        
        At = At->NextAlloc;
    }
    
    Frame->FrameUpdateNode = FrameUpdateNode;
}

INTERNAL_FUNCTION void DEBUGProcessRecords(debug_state* State){
    FUNCTION_TIMING();
    
    int RecordCount = Global_DebugTable->RecordAndTableIndex & DEBUG_RECORD_INDEX_MASK;
    int TableIndex = (Global_DebugTable->RecordAndTableIndex & DEBUG_TABLE_INDEX_MASK) >> 
        DEBUG_TABLE_INDEX_BITSHIFT;
    
    debug_record* RecordArray = Global_DebugTable->RecordTables[TableIndex];
    
    u32 NewRecordAndTableIndex = 0;
    NewRecordAndTableIndex |= (!TableIndex) << DEBUG_TABLE_INDEX_BITSHIFT;
    
    Global_DebugTable->RecordAndTableIndex.store(NewRecordAndTableIndex);
    
    for(int RecordIndex = 0; 
        RecordIndex <  RecordCount;
        RecordIndex++)
    {
        debug_record* Record = &RecordArray[RecordIndex];
        
        if(Record->Type & State->Filter)
        {
            switch(Record->Type)
            {
                case DebugRecord_BeginTiming:
                {
                    debug_thread* Thread = CreateOrFindThreadForID(State, Record->ThreadID);
                    
                    debug_thread_frame* Frame = GetThreadFrameByIndex(Thread, State->CollationFrameIndex);
                    
                    b32 NodeWasAllocated = false;
                    debug_profiled_tree_node* NewNode = RequestTreeNode(State, Frame, 
                                                                        Frame->CurNode,
                                                                        Record->UniqueName,
                                                                        &NodeWasAllocated);
                    
                    NewNode->TimingSnapshot.StartClock = Record->TimeStampCounter;
                    if(NodeWasAllocated){
                        NewNode->TimingSnapshot.StartClockFirstEntry = NewNode->TimingSnapshot.StartClock;
                    }
                    
                    Frame->CurNode = NewNode;
                }break;
                
                case DebugRecord_EndTiming:
                {
                    debug_thread* Thread = CreateOrFindThreadForID(State, Record->ThreadID);
                    
                    debug_thread_frame* Frame = GetThreadFrameByIndex(Thread, State->CollationFrameIndex);
                    debug_profiled_tree_node* CurNode = Frame->CurNode;
                    debug_timing_snapshot* Snapshot = &CurNode->TimingSnapshot;
                    
                    u64 ClocksElapsedThisFrame = (Record->TimeStampCounter - Snapshot->StartClock);
                    int PendingHitCount = 1;
                    
                    Snapshot->EndClock = Record->TimeStampCounter;
                    Snapshot->ClocksElapsed += ClocksElapsedThisFrame;
                    Snapshot->HitCount += PendingHitCount;
                    
                    // NOTE(Dima): Adding time to total parent's children elapsed 
                    debug_profiled_tree_node* ParentNode = CurNode->Parent;
                    if(ParentNode->Parent != 0){
                        debug_timing_stat* ParentStat = CreateOrFindStatForUniqueName(State, Frame,
                                                                                      ParentNode->UniqueName);
                        ParentStat->Stat.ClocksElapsedInChildren += ClocksElapsedThisFrame;
                    }
                    
                    // NOTE(Dima): Initializing statistic
                    debug_timing_stat* Stat = CreateOrFindStatForUniqueName(State, Frame,
                                                                            CurNode->UniqueName);
                    
                    Stat->Stat.ClocksElapsed += ClocksElapsedThisFrame;
                    Stat->Stat.HitCount += PendingHitCount;
                    
                    Frame->CurNode = CurNode->Parent;
                }break;
                
                case DebugRecord_FrameBarrier:
                {
                    debug_thread* MainThread = State->MainThread;
                    Assert(MainThread->ThreadID == Record->ThreadID);
                    
                    debug_thread_frame* OldFrame = GetThreadFrameByIndex(MainThread, 
                                                                         State->CollationFrameIndex);
                    debug_common_frame* OldFrameCommon = GetFrameByIndex(State, 
                                                                         State->CollationFrameIndex);
                    
                    // NOTE(Dima): Finding frame update node
                    FindFrameUpdateNode(OldFrame);
                    
                    // NOTE(Dima): Set frame time
                    OldFrameCommon->FrameTime = Record->Value.Float;
                    
                    Assert(OldFrame->CurNode == &OldFrame->RootTreeNodeUse);
                    
                    // NOTE(Dima): Incrementing frame indices when we needed
                    ProcessRecordsIndicesInc(State);
                    
                    // NOTE(Dima): Clearing frame
                    ClearThreadsIndexFrame(State, State->CollationFrameIndex);
                }break;
            }
        }
    }
}
#endif

void DEBUGInitGlobalTable(memory_arena* Arena)
{
    Global_DebugTable = PushStruct(Arena, debug_global_table);
    
    // NOTE(Dima): Initialize record arrays
    int RecordArrayCount = 1000000;
    Global_DebugTable->TableMaxRecordCount = RecordArrayCount;
    Global_DebugTable->RecordAndTableIndex = 0;
    Global_DebugTable->RecordTables[0] = PushArray(Arena, debug_record, RecordArrayCount);
    Global_DebugTable->RecordTables[1] = PushArray(Arena, debug_record, RecordArrayCount);
    
    // NOTE(Dima): Init memory region
    Global_DebugTable->Arena = Arena;
}

GLOBAL_VARIABLE debug_state* Global_Debug;

INTERNAL_FUNCTION void DEBUGInit(memory_arena* Arena)
{
#if defined(INTERNAL_FUNCTION)
    Global_Debug = PushStruct(Arena, debug_state);
    
    debug_state* State = Global_Debug;
    
    State->Arena = Arena;
    
    // NOTE(Dima): Init profiler stuff
    State->CollationFrameIndex = 0;
    State->NewestFrameIndex = 0;
    State->OldestFrameIndex = 0;
    State->OldestShouldBeIncremented = false;
    State->IsRecording = DEBUG_DEFAULT_RECORDING;
    State->RecordingChangeRequested = false;
    State->Filter = DEBUG_DEFAULT_FILTER_VALUE;
    State->ToShowProfileMenuType = DebugProfileMenu_TopClockEx;
    
    CopyStrings(State->RootNodesName, "RootNode");
    CopyStrings(State->SentinelElementsName, "Sentinel");
    State->RootNodesNameHash = StringHashFNV(State->RootNodesName);
    State->SentinelElementsNameHash = StringHashFNV(State->SentinelElementsName);
    
    DLIST_REFLECT_PTRS(State->TreeNodeFree, NextAlloc, PrevAlloc);
    DLIST_REFLECT_PTRS(State->TreeNodeFree, Next, Prev);
    DLIST_REFLECT_PTRS(State->StatFree, Next, Prev);
    
    State->ProfiledThreadsCount = 0;
    DLIST_REFLECT_PTRS(State->ThreadSentinel, NextAlloc, PrevAlloc);
    ClearThreadsTable(State->ThreadHashTable, DEBUG_THREADS_TABLE_SIZE);
    State->MainThread = CreateOrFindThreadForID(State, Platform.GetThreadID());
    State->WatchThread = State->MainThread;
#endif
}


INTERNAL_FUNCTION void DEBUGUpdate()
{
#if defined(INTERNAL_BUILD)
    FUNCTION_TIMING();
    
    DEBUGProcessRecords(Global_Debug);
#endif
}
