#ifndef PREPROCESSOR_JSON_H
#define PREPROCESSOR_JSON_H

enum json_token_type
{
    JsonToken_ID,
    JsonToken_DoubleQuote,
    JsonToken_String,
    
    JsonToken_OpenBlock,
    JsonToken_CloseBlock,
    
    JsonToken_OpenArray,
    JsonToken_CloseArray,
    
    JsonToken_Comma,
    JsonToken_Colon,
};

struct json_token
{
    std::string Str;
    
    u32 Type;
};

struct json_tokenizer
{
    char* FileData;
    char* At;
    
    b32 InQuotes;
};

#endif //PREPROCESSOR_JSON_H

#if defined(PREPROCESSOR_JSON_IMPLEMENTATION) && !defined(PREPROCESSOR_JSON_IMPLEMENTATION_DONE)
#define PREPROCESSOR_JSON_IMPLEMENTATION_DONE

char* ReadFileAndNullTerminate(char* FileName)
{
    char* Result = 0;
    
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
    
    return(Result);
}

json_tokenizer InitTokenizer(char* FileName)
{
    char* FileData = ReadFileAndNullTerminate(FileName);
    
    json_tokenizer Result = {};
    Result.FileData = FileData;
    Result.At = FileData;
    Result.InQuotes = false;
    
    return(Result);
};

void FreeTokenizer(json_tokenizer* Tokenz)
{
    if(Tokenz->FileData)
    {
        free(Tokenz->FileData);
    }
}

inline b32 IsAlpha(char C)
{
    b32 Result = ((C >= 'a' && C <= 'z') ||
                  (C >= 'A' && C <= 'Z'));
    
    return(Result);
}


inline b32 IsWhitespace(char C)
{
    b32 Result = (C == ' ' ||
                  C == '\t' ||
                  C == '\n' ||
                  C == '\r');
    
    return(Result);
}

void SkipWhitespaces(json_tokenizer* Tokenz)
{
    while(IsWhitespace(Tokenz->At[0]) && 
          Tokenz->At[0] != 0)
    {
        Tokenz->At += 1;
    }
}

b32 CanGetNextToken(json_tokenizer* Tokenz)
{
    if(!Tokenz->InQuotes && IsWhitespace(Tokenz->At[0]))
    {
        SkipWhitespaces(Tokenz);
    }
    
    b32 Result = Tokenz->At != 0;
    
    return(Result);
}

void GetTokenInternal(json_tokenizer* Tokenz, json_token* Token)
{
    int CountCharsInToken = 0;
    
    // NOTE(Dima): We shoud set At only after skipping
    char* At = Tokenz->At;
    if(At[0] == 0)
    {
    }
    else if(At[0] == '\"')
    {
        Token->Type = JsonToken_DoubleQuote;
        Token->Str = std::string("\"");
        
        Tokenz->InQuotes = !Tokenz->InQuotes;
        CountCharsInToken = 1;
    }
    else if(Tokenz->InQuotes)
    {
        int CharCountInString = 0;
        while(At[CharCountInString] != '\"' &&
              At[CharCountInString] != 0)
        {
            CharCountInString++;
        }
        
        Token->Type = JsonToken_String;
        Token->Str = std::string(At, CharCountInString);
        
        CountCharsInToken = CharCountInString;
    }
    else
    {
        if(At[0] == '{')
        {
            Token->Type = JsonToken_OpenBlock;
            Token->Str = std::string("{");
            CountCharsInToken = 1;
        }
        else if(At[0] == '}')
        {
            Token->Type = JsonToken_CloseBlock;
            Token->Str = std::string("}");
            CountCharsInToken = 1;
        }
        else if(At[0] == '[')
        {
            Token->Type = JsonToken_OpenArray;
            Token->Str = std::string("[");
            CountCharsInToken = 1;
        }
        else if(At[0] == ']')
        {
            Token->Type = JsonToken_CloseArray;
            Token->Str = std::string("]");
            CountCharsInToken = 1;
        }
        else if(At[0] == ':')
        {
            Token->Type = JsonToken_Colon;
            Token->Str = std::string(":");
            CountCharsInToken = 1;
        }
        else if(At[0] == ',')
        {
            Token->Type = JsonToken_Comma;
            Token->Str = std::string(",");
            CountCharsInToken = 1;
        }
        else if(IsAlpha(At[0]))
        {
            int CharCountInString = 0;
            
            while(!IsWhitespace(At[CharCountInString]) &&
                  At[CharCountInString] != ',' &&
                  At[CharCountInString] != '\"' &&
                  At[CharCountInString] != 0)
            {
                CharCountInString++;
            }
            
            Token->Type = JsonToken_ID;
            Token->Str = std::string(At, CharCountInString);
            
            CountCharsInToken = CharCountInString;
        }
        else
        {
            Assert(!"Invalid symbol");
        }
    }
    
    Tokenz->At += CountCharsInToken;
}

b32 GetToken(json_tokenizer* Tokenz, json_token* Token)
{
    b32 Result = false;
    if(CanGetNextToken(Tokenz))
    {
        Result = true;
        
        GetTokenInternal(Tokenz, Token);
    }
    
    return(Result);
}

b32 NextTokenIs(json_tokenizer* Tokenz, u32 TokenType)
{
    char* TokenizerAt = Tokenz->At;
    b32 InitInQuotes = Tokenz->InQuotes;
    
    json_token Tok;
    b32 TokenGet = GetToken(Tokenz, &Tok);
    
    b32 Result = TokenGet && (Tok.Type == TokenType);
    
    Tokenz->At = TokenizerAt;
    Tokenz->InQuotes = InitInQuotes;
    
    return(Result);
}

std::string RequireToken(json_tokenizer* Tokenz, u32 TokenType)
{
    json_token Tok;
    GetToken(Tokenz, &Tok);
    Assert(Tok.Type == TokenType);
    
    std::string Result = Tok.Str;
    
    return(Result);
}

b32 JsonReadBool(json_tokenizer* Tokenz)
{
    std::string ValueStr = RequireToken(Tokenz, JsonToken_ID);
    
    b32 IsTrue = ValueStr == "true";
    b32 IsFalse = ValueStr == "false";
    
    Assert(IsTrue || IsFalse);
    
    b32 Result = false;
    if(IsTrue || IsFalse)
    {
        if(IsTrue)
        {
            Result = true;
        }
        else
        {
            Result = false;
        }
    }
    
    return(Result);
}

std::string JsonReadString(json_tokenizer* Tokenz)
{
    RequireToken(Tokenz, JsonToken_DoubleQuote);
    std::string Result = RequireToken(Tokenz, JsonToken_String);
    RequireToken(Tokenz, JsonToken_DoubleQuote);
    
    return(Result);
}

std::string JsonReadField(json_tokenizer* Tokenz)
{
    std::string FieldName = JsonReadString(Tokenz);
    RequireToken(Tokenz, JsonToken_Colon);
    
    return(FieldName);
}

inline b32 CanParseNextField(json_tokenizer* Tokenz)
{
    b32 NextTokenIsCloseBlock = NextTokenIs(Tokenz, JsonToken_CloseBlock);
    
    return(!NextTokenIsCloseBlock);
}

inline b32 ProcessEndOfField(json_tokenizer* Tokenz)
{
    // NOTE(Dima): Need to break or not???
    b32 Result = false;;
    if(NextTokenIs(Tokenz, JsonToken_Comma))
    {
        Result = true;
        
        // NOTE(Dima): If next token is comma - read it
        RequireToken(Tokenz, JsonToken_Comma);
        
        if(NextTokenIs(Tokenz, JsonToken_CloseBlock))
        {
            Result = false;
        }
    }
    
    return(Result);
}

#endif