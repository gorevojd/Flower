PLATFORM_READ_FILE_AND_NULL_TERMINATE(StandardReadFileAndNullTerminate)
{
    char* Result = 0;
    
    if(FileName != 0)
    {
        FILE* fp = fopen(FileName, "rb");
        if(fp)
        {
            fseek(fp, 0, SEEK_END);
            
            int FileSize = ftell(fp);
            
            Result = (char*)malloc(FileSize + 1);
            
            fseek(fp, 0, SEEK_SET);
            size_t ObjectsRead = fread((void*)Result, sizeof(char), FileSize, fp);
            
            Result[FileSize] = 0;
            
            fclose(fp);
        }
    }
    
    return(Result);
}

PLATFORM_READ_FILE(StandardReadFile)
{
    u64 Size = 0;
    void* Result = 0;
    
    if(FileName != 0)
    {
        FILE* fp = fopen(FileName, "rb");
        if(fp)
        {
            fseek(fp, 0, SEEK_END);
            
            Size = ftell(fp);
            
            Result = malloc(Size);
            
            fseek(fp, 0, SEEK_SET);
            fread(Result, sizeof(unsigned char), Size, fp);
            
            fclose(fp);
        }
    }
    
    if(FileSize)
    {
        *FileSize = Size;
    }
    
    return(Result);
}

PLATFORM_ALLOCATE_MEMORY(StandardAllocateMemory)
{
    void* Result = calloc(Size, 1);
    
    return(Result);
}

PLATFORM_FREE_MEMORY(StandardFreeMemory)
{
    if(Memory)
    {
        free(Memory);
    }
}