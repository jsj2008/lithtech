
// The string manager gets strings out of bound modules and formats them.
// Strings are treated abstractly, so you shouldn't really access them.

#ifndef __STRINGMGR_H__
#define __STRINGMGR_H__


#ifndef __LTBASEDEFS_H__
    #include "ltbasedefs.h"
#endif

#ifndef __LTMODULE_H__
	#include "ltmodule.h"
#endif

    class CBindModuleType;

	typedef void (*StringShowFn)(char *pData, void *pUser);

	void str_Init();
	void str_Term();
	void str_ShowAllStringsAllocated(StringShowFn fn, void *pUser);
    LTBOOL str_LoadStringTable(void);


	// bufferLen is set to the number of bytes in the string excluding the null terminator.
	// You MUST free the string returned by here with str_FreeStringBuffer.
	uint8* str_FormatString(CBindModuleType *hModule, int stringCode, va_list *marker, int *bufferLen);
	void str_FreeStringBuffer(uint8 *pBuffer);

//	LTBOOL str_GetString(CBindModuleType *hModule, int stringCode,
//		uint8 *pBuffer, int bufferLen, int *pBufferLen);

	HSTRING str_CreateString(uint8 *pBuffer);
	HSTRING str_CreateStringAnsi(char *pString);
	HSTRING str_CopyString(HSTRING hString);
	void str_FreeString(HSTRING hString);
	LTBOOL str_CompareStrings(HSTRING hString1, HSTRING hString2);
	LTBOOL str_CompareStringsUpper(HSTRING hString1, HSTRING hString2);
	char* str_GetStringData(HSTRING hString);

	int str_GetNumStringCharacters(HSTRING hString);

	// Gives you a pointer to the string's bytes.
	// Optionally fills in the number of bytes this string takes, including
	// the null terminating character (ie: if you want to transfer the string around,
	// you can send pNumBytes worth of the bytes it returns and CreateString() with that).
	uint8* str_GetStringBytes(HSTRING hString, int *pNumBytes);

// --------------------------------------------------------
// PSX2-specific string lookup table stuff
// --------------------------------------------------------

	LTBOOL StringTableInit(void);
	void StringTableTerm(void);
	char * StringTableLookup(int stringCode);

// IStringMgr
class IStringMgr : public IBase
{
public:
	interface_version(IStringMgr, 0);
    declare_interface(IStringMgr);

	typedef void (*StringShowFn)(char *pData, void *pUser);

	virtual void Init() = 0;
	virtual void Term() = 0;
	virtual void ShowAllStringsAllocated(StringShowFn fn, void *pUser) = 0;

	// bufferLen is set to the number of bytes in the string excluding the null terminator.
	// You MUST free the string returned by here with FreeStringBuffer.
	virtual uint8* FormatString(CBindModuleType *hModule, int stringCode, va_list *marker, int *bufferLen) = 0;
	virtual void FreeStringBuffer(uint8 *pBuffer) = 0;

	virtual bool GetString(CBindModuleType *hModule, int stringCode, 
		uint8 *pBuffer, int bufferLen, int *pBufferLen) = 0;

	virtual HSTRING CreateString(uint8 *pBuffer) = 0;
	virtual HSTRING CreateStringAnsi(char *pString) = 0;
	virtual HSTRING CopyString(HSTRING hString) = 0;
	virtual void FreeString(HSTRING hString) = 0;
	virtual bool CompareStrings(HSTRING hString1, HSTRING hString2) = 0;
	virtual bool CompareStringsUpper(HSTRING hString1, HSTRING hString2) = 0;
	virtual char* GetStringData(HSTRING hString) = 0;

	virtual int GetNumStringCharacters(HSTRING hString) = 0;

	// Gives you a pointer to the string's bytes.
	// Optionally fills in the number of bytes this string takes, including
	// the null terminating character (ie: if you want to transfer the string around,
	// you can send pNumBytes worth of the bytes it returns and CreateString() with that).
	virtual uint8* GetStringBytes(HSTRING hString, int *pNumBytes) = 0;
};

#endif  // __STRINGMGR_H__

