#ifndef FLOWER_STRINGS_H
#define FLOWER_STRINGS_H

GLOBAL_VARIABLE u64 PowersOf10[] = 
{
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000,
    100000000000,
    1000000000000,
    10000000000000,
    100000000000000,
    1000000000000000,
    10000000000000000,
    100000000000000000,
    1000000000000000000,
    10000000000000000000,
};

inline int StringLength(char* Text) {
	int Res = 0;
    
	char* At = Text;
	while (*At) {
		Res++;
        
		At++;
	}
    
	return(Res);
}

inline b32 StringsAreEqual(char* A, char* B) {
	b32 Result = false;
    
	while (*A && *B) {
        
		if (*A != *B) {
			Result = false;
			break;
		}
        
		A++;
		B++;
	}
    
	if (*A == 0 && *B == 0) {
		Result = true;
	}
    
	return(Result);
}

inline void CopyStrings(char* Dst, char* Src) {
	if (Src && Dst) {
		while (*Src) {
			*Dst++ = *Src++;
		}
        *Dst = 0;
    }
}

inline void CopyStringsSafe(char* Dst, int DstSize, char* Src){
    if(Src && Dst){
        int SpaceInDstAvailable = DstSize - 1;
        while(*Src && SpaceInDstAvailable){
            *Dst++ = *Src++;
            SpaceInDstAvailable--;
        }
        *Dst = 0;
    }
}

inline u32 StringHashFNV(char* Name) 
{
    u32 Result = 2166136261;
    
    char* At = Name;
    while (*At) {
        
        Result *= 16777619;
        Result ^= *At;
        
        At++;
    }
    
    return(Result);
}

inline b32 StringIsDecimalInteger(char* String) {
    b32 Result = 1;
    
    int FirstCheckIndex = 0;
    if (String[0] == '-') {
        FirstCheckIndex = 1;
    }
    
    char* At = String + FirstCheckIndex;
    while (*At)
    {
        if (*At >= '0' &&
            *At <= '9')
        {
            
        }
        else {
            return(0);
        }
        
        *At++;
    }
    
    return(Result);
}

inline int StringToInteger(char* String) {
    int Result = 0;
    
    char* At = String;
    
    int Len = StringLength(String);
    
    int NumberIsNegative = 1;
    int FirstNumberIndex = 0;
    if (String[0] == '-') {
        FirstNumberIndex = 1;
        NumberIsNegative = -1;
    }
    
    int CurrentMultiplier = 1;
    for (int CharIndex = Len - 1;
         CharIndex >= FirstNumberIndex;
         CharIndex--)
    {
        Result += (String[CharIndex] - '0') * CurrentMultiplier;
        CurrentMultiplier *= 10;
    }
    
    Result *= NumberIsNegative;
    
    return(Result);
}

inline float StringToFloat(char* String) {
    float Result = 0.0f;
    
    //NOTE(dima): Detecting if negative and whole part start index
    float IsNegative = 1.0f;
    int WholeStart = 0;
    if (String[0] == '-') {
        IsNegative = -1.0f;
        WholeStart = 1;
    }
    
    char* At = String + WholeStart;
    b32 DotExist = 0;
    char* DotAt = 0;
    //NOTE(dima): Detecting whole part end
    int WholeEndIndex = WholeStart;
    while (*At) {
        if (*At == '.') {
            DotExist = 1;
            DotAt = At;
            break;
        }
        At++;
        WholeEndIndex++;
    }
    
    //NOTE(dima): Converting whole part
    float CurrentMultiplier = 1.0f;
    for (int Index = WholeEndIndex - 1;
         Index >= WholeStart;
         Index--)
    {
        Result += (float)(String[Index] - '0') * CurrentMultiplier;
        CurrentMultiplier *= 10.0f;
    }
    
    //NOTE(dima): Converting fractional part if exist
    if (DotExist) {
        int FractionalPartLen = 0;
        At = DotAt;
        ++At;
        while (*At) {
            FractionalPartLen++;
            
            At++;
        }
        
        if (FractionalPartLen) {
            char* FractionalBegin = DotAt + 1;
            char* FractionalEnd = At;
            
            char* FractionalAt = FractionalBegin;
            CurrentMultiplier = 0.1f;
            while (FractionalAt != FractionalEnd) {
                float CurrentDigit = (float)(*FractionalAt - '0');
                
                Result += CurrentDigit * CurrentMultiplier;
                CurrentMultiplier /= 10.0f;
                FractionalAt++;
            }
        }
    }
    
    Result *= IsNegative;
    
    return(Result);
}

inline void IntegerToString(int Value, char* String) 
{
    int DigitIndex = 0;
    
    do {
        String[DigitIndex++] = '0' + (Value % 10);
        
        Value /= 10;
    } while (Value);
    
    //NOTE(dima): Reversing string
    int ScanBeginIndex = 0;
    int ScanEndIndex = DigitIndex - 1;
    while (ScanBeginIndex < ScanEndIndex) {
        char Temp = String[ScanBeginIndex];
        String[ScanBeginIndex] = String[ScanEndIndex];
        String[ScanEndIndex] = Temp;
        
        ScanBeginIndex++;
        ScanEndIndex--;
    }
    
    //NOTE(dima): Null terminating the string
    String[DigitIndex] = 0;
}

inline void FloatToString(char* Dst, f32 Value, int DigitsAfterPoint = 2)
{
    b32 IsNegative = false;
    if(Value < 0)
    {
        IsNegative = true;
        Value = -Value;
    }
    
    u64 WholeValue = (u64)Value;
    u64 FractionValue = 0;
    if(DigitsAfterPoint > 0)
    {
        FractionValue = fmod(Value, 1.0f) * (f32)PowersOf10[DigitsAfterPoint - 1];
    }
    
    int CharIndex = 0;
    
    // NOTE(Dima): Detecting count of digits
    int FractionDigitsLeft = DigitsAfterPoint;
    int WholeDigitsLeft = 0;
    
    // NOTE(Dima): Detection whole digits count
    int TempWorkValue = WholeValue;
    do
    {
        WholeDigitsLeft++;
        TempWorkValue /= 10;
    }while(TempWorkValue > 0);
    
    // NOTE(Dima): Printing sign
    if(IsNegative)
    {
        Dst[CharIndex++] = '-';
    }
    
    // NOTE(Dima): Printing whole part
    if(WholeValue)
    {
        while(WholeDigitsLeft)
        {
            int DigitIndex = WholeDigitsLeft - 1;
            
            int CurDigit = (WholeValue / PowersOf10[DigitIndex]) % 10;
            
            Dst[CharIndex++] = '0' + CurDigit;
            
            WholeDigitsLeft--; 
        }
    }
    else
    {
        Dst[CharIndex++] = '0';
    }
    
    if(FractionDigitsLeft)
    {
        // NOTE(Dima): Printing point
        Dst[CharIndex++] = '.';
        
        // NOTE(Dima): Printing fraction
        while(FractionDigitsLeft)
        {
            int DigitIndex = FractionDigitsLeft - 1;
            int CurDigit = (FractionValue / PowersOf10[DigitIndex]) % 10;
            
            Dst[CharIndex++] = '0' + CurDigit;
            
            FractionDigitsLeft--;
        }
    }
    
    Dst[CharIndex++] = 0;
}

inline b32 IsLetter(char C){
    if((C >= 'a' && C <= 'z') || 
       (C >= 'A' && C <= 'Z'))
    {
        return(1);
    }
    return(0);
}

inline b32 IsUpper(char C){
    b32 Result = 0;
    if(C >= 'A' && C <= 'Z'){
        Result = 1;
    }
    
    return(Result);
}

inline b32 IsWhitespace(char C){
    b32 Result = 0;
    
    if(C == ' ' ||
       C == '\n' ||
       C == '\r')
    {
        Result = 1;
    }
    
    return(Result);
}

inline void StringToUpper(char* To, char* From){
    char* AtFrom = From;
    char* AtTo = To;
    
    if(AtFrom && AtTo){
        while(*AtFrom){
            
            if(IsLetter(*AtFrom)){
                if(IsUpper(*AtFrom)){
                    *AtTo = *AtFrom;
                }
                else{
                    *AtTo = *AtFrom - 'a' + 'A';
                }
            }
            else{
                *AtTo = *AtFrom;
            }
            
            AtFrom++;
            AtTo++;
        }
        
        *AtTo = 0;
    }
}

inline void AppendStringSafe(char* To, int ToSize, char* Src)
{
    int ToCurLen = StringLength(To);
    int SrcLen = StringLength(Src);
    
    Assert(ToCurLen + SrcLen + 1 <= ToSize);
    
    int At = ToCurLen;
    for(int CharIndex = 0; CharIndex < SrcLen; CharIndex++)
    {
        To[At++] = Src[CharIndex];
    }
    
    To[At++] = 0;
}

#endif
