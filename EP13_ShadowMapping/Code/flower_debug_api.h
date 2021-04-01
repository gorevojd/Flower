#ifndef FLOWER_DEBUG_API_H
#define FLOWER_DEBUG_API_H

#if defined(INTERNAL_BUILD)

#define DEBUG_DEFAULT_RECORDING true
#define DEBUG_TABLE_INDEX_MASK 0x80000000
#define DEBUG_TABLE_INDEX_BITSHIFT 31
#define DEBUG_RECORD_INDEX_MASK 0x7FFFFFFF

enum debug_record_type{
    DebugRecord_BeginTiming = (1 << 1),
    DebugRecord_EndTiming = (1 << 2),
    DebugRecord_FrameBarrier = (1 << 3),
};

struct debug_record{
    char* UniqueName;
    u64 TimeStampCounter;
    u16 ThreadID;
    u8 Type;
    
    struct{
        int Int;
        float Float;
    } Value;
};

#define DEBUG_ID_TO_STRING(name) #name
#define DEBUG_UNIQUE_SYMBOL_HELPER(a, b, c, d) a "|" b "|" DEBUG_ID_TO_STRING(c) "|" DEBUG_ID_TO_STRING(d)
#define DEBUG_UNIQUE_SYMBOL(name) DEBUG_UNIQUE_SYMBOL_HELPER(name, __FUNCTION__, __LINE__, __COUNTER__)

struct debug_global_table{
    struct memory_arena* Arena;
    
    // NOTE(Dima): Debug records
    std::atomic_uint RecordAndTableIndex;
    
    debug_record* RecordTables[2];
    int TableMaxRecordCount;
};

extern debug_global_table* Global_DebugTable;

inline void DEBUGAddRecord(char* UniqueName, u8 Type)
{
    if(Global_DebugTable)
    {
        u32 ToParseIndex = Global_DebugTable->RecordAndTableIndex.fetch_add(1);
        
        u32 TableIndex = (ToParseIndex & DEBUG_TABLE_INDEX_MASK) >> DEBUG_TABLE_INDEX_BITSHIFT;
        u32 RecordIndex = ToParseIndex & DEBUG_RECORD_INDEX_MASK;
        
        debug_record* TargetArray = Global_DebugTable->RecordTables[TableIndex];
        debug_record* TargetRecord = &TargetArray[RecordIndex];
        
        TargetRecord->UniqueName = UniqueName;
        TargetRecord->Type = Type;
        TargetRecord->TimeStampCounter = __rdtsc();
        TargetRecord->ThreadID = Platform.GetThreadID();
    }
}

inline void DEBUGAddFloatToLastRecord(float Value){
    u32 ToParseIndex = Global_DebugTable->RecordAndTableIndex;
    
    u32 TableIndex = (ToParseIndex & DEBUG_TABLE_INDEX_MASK) >> DEBUG_TABLE_INDEX_BITSHIFT;
    u32 RecordIndex = ToParseIndex & DEBUG_RECORD_INDEX_MASK;
    
    debug_record* TargetRecord = &Global_DebugTable->RecordTables[TableIndex][RecordIndex - 1];
    
    TargetRecord->Value.Float = Value;
}

void DEBUGParseNameFromUnique(char* To, int ToSize, char* From);
void FillAndSortStats(struct debug_state* State, 
                      struct debug_thread_frame* Frame, 
                      b32 IncludingChildren);

struct debug_timing{
    debug_timing(char* UniqueName){
        DEBUGAddRecord(UniqueName, DebugRecord_BeginTiming);
    }
    
    ~debug_timing(){
        DEBUGAddRecord("End", DebugRecord_EndTiming);
    }
};

#define FRAME_UPDATE_NODE_NAME "FrameUpdate"
#define BEGIN_TIMING(name) DEBUGAddRecord(name, DebugRecord_BeginTiming)
#define END_TIMING() DEBUGAddRecord("End", DebugRecord_EndTiming)
#define FUNCTION_TIMING() debug_timing FuncTiming_##__LINE__(DEBUG_UNIQUE_SYMBOL(__FUNCTION__))
#define BLOCK_TIMING(name) debug_timing BlockTiming_##__LINE__(DEBUG_UNIQUE_SYMBOL(name))
#define FRAME_BARRIER(dt) {\
DEBUGAddRecord("FrameBarrier", DebugRecord_FrameBarrier);\
DEBUGAddFloatToLastRecord(dt);}

#else

// NOTE(Dima): Timing stuff
#define BEGIN_TIMING(...)
#define END_TIMING(...)
#define FUNCTION_TIMING(...)

#endif

#endif //FLOWER_DEBUG_API_H
