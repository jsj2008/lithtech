
// The string manager gets strings out of bound modules and formats them.
// Strings are treated abstractly, so you shouldn't really access them.

#ifndef __STRINGMGR_H__
#define __STRINGMGR_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class CBindModuleType;

typedef void (*StringShowFn)(const char *pData, void *pUser);

void str_Init();
void str_Term();
void str_ShowAllStringsAllocated(StringShowFn fn, void *pUser);


// bufferLen is set to the number of bytes in the string excluding the null terminator.
// You MUST free the string returned by here with str_FreeStringBuffer.
uint8* str_FormatString(CBindModuleType *hModule, int stringCode, va_list *marker, int *bufferLen);
void str_FreeStringBuffer(uint8 *pBuffer);

HSTRING str_CreateString(uint8 *pBuffer);
HSTRING str_CreateStringAnsi(const char *pString);
HSTRING str_CopyString(HSTRING hString);
void str_FreeString(HSTRING hString);
bool str_CompareStrings(HSTRING hString1, HSTRING hString2);
bool str_CompareStringsUpper(HSTRING hString1, HSTRING hString2);
char* str_GetStringData(HSTRING hString);

int str_GetNumStringCharacters(HSTRING hString);

// Gives you a pointer to the string's bytes.
// Optionally fills in the number of bytes this string takes, including
// the null terminating character (ie: if you want to transfer the string around,
// you can send pNumBytes worth of the bytes it returns and CreateString() with that).
uint8* str_GetStringBytes(HSTRING hString, int *pNumBytes);

#endif  // __STRINGMGR_H__
