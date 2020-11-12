#ifndef FLOWER_STRINGS_H
#define FLOWER_STRINGS_H

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

#endif
