
#include "bdefs.h"
#include "stringmgr.h"
//#include "winbind.h"
#include "syslibraryloader.h"
#include "dsys.h"

#include <string>

typedef struct
{
	HLTMODULE	m_hInstance;
	int			m_Type;

	// Holds name of dll file so that it can be
	// deleted when freed.
	std::string m_sTempFileName;

} WinBind;


struct StringWrapper
{
	GLink m_GLink;
	int m_RefCount;
	uint16 m_StringLen; // Number of CHARACTERS in the string.
	uint16 m_DataLen;  // How long m_Bytes is (including null terminating character..)
	uint8 m_Bytes[1];
};



static int g_StringMgrInitCount=0;
static GLink g_StringHead; // All the strings..

// Used for "" strings so we don't allocate lots of extra memory.
static StringWrapper g_ZeroLengthStringWrapper =
{
	LTNULL, LTNULL, LTNULL,
	0, 0, 0, 0
};

extern int32 g_bDebugStrings;


inline StringWrapper* AllocateStringWrapper(int nStringBytes)
{
	StringWrapper* p;
	LT_MEM_TRACK_ALLOC(p = (StringWrapper*)LTMemAlloc(sizeof(StringWrapper) + (nStringBytes-1)),LT_MEM_TYPE_STRING);
	return p;
}

inline void FreeStringWrapper(StringWrapper *pString)
{
	LTMemFree(pString);
}


void str_Init()
{
	if(g_StringMgrInitCount == 0)
	{
		gn_TieOff(&g_StringHead);
	}
	
	++g_StringMgrInitCount;
}

void str_Term()
{
	GLink *pCur, *pNext;

	--g_StringMgrInitCount;
	if(g_StringMgrInitCount == 0)
	{
		// Free all the allocated strings.
		pCur = g_StringHead.m_pNext;
		while(pCur != &g_StringHead)
		{
			pNext = pCur->m_pNext;
			FreeStringWrapper((StringWrapper*)pCur->m_pData);
			pCur = pNext;
		}

		gn_TieOff(&g_StringHead);
	}
}


void str_ShowAllStringsAllocated(StringShowFn fn, void *pUser)
{
	GLink *pCur;
	StringWrapper *pString;
	
	pCur = g_StringHead.m_pNext;
	while(pCur != &g_StringHead)
	{
		pString = (StringWrapper*)pCur->m_pData;
		fn((char*)pString->m_Bytes, pUser);

		pCur = pCur->m_pNext;
	}
}


uint8* str_FormatString(CBindModuleType *hModule, int stringCode, va_list *marker, int *bufferLen)
{

#ifndef __LINUX

	WinBind *pBind = (WinBind*)hModule;
	*bufferLen = 0;

	// Load the string..
	char tempBuffer[5000];
	uint32 nBytes = LoadString((HINSTANCE)pBind->m_hInstance, stringCode, (char*)tempBuffer, sizeof(tempBuffer));
	if(nBytes == 0)
	{
		if(g_bDebugStrings)
			dsi_ConsolePrint("Couldn't get string %d", stringCode);
		
		return LTFALSE;
	}

	// Format it.
	uint8 *pBuffer = NULL;

	nBytes = FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
						tempBuffer,
						0,
						0,
						(char*)&pBuffer,
						1,
						marker
						);

	if(nBytes == 0)
	{
		//make sure that the buffer is cleared
		if(pBuffer)
			LocalFree(pBuffer);

		if(g_bDebugStrings)
			dsi_ConsolePrint("FormatMessage error on string %d", stringCode);

		return LTNULL;
	}
	else
	{
		// There should be a better way to do this.. FormatMessage doesn't define
		// its return value very well at all so we just allocate 2 bytes for
		// each character to make sure.
		nBytes = (nBytes + 1) * 2;
		*bufferLen = (int)nBytes;
		return pBuffer;
	}
#else
	if(g_bDebugStrings)
		dsi_ConsolePrint("Linux doesn't support FormatMessage - string %d", stringCode);
	return LTNULL;
#endif
}


void str_FreeStringBuffer(uint8 *pBuffer)
{

#ifndef __LINUX
	if(pBuffer)
		LocalFree(pBuffer);
#else
	if(g_bDebugStrings)
		dsi_ConsolePrint( "Linux doesn't support FreeStringBuffer" );
#endif

}

inline void str_CalcSize(uint8 *pString, int *pNumBytes, int *pNumChars)
{
	char *pCur;

	*pNumChars = 0;
	pCur = (char*)pString;
	while(*pCur != 0)
	{
		++(*pNumChars);
#ifndef __LINUX
		pCur = _tcsinc(pCur);
#else
		pCur++;
#endif
	}

	// Number of bytes we've traversed + 1 for null terminating character.
	*pNumBytes = (pCur - (char*)pString) + 1;
}


HSTRING str_CreateString(uint8 *pBuffer)
{
	StringWrapper *pString;
	int nBytes, nChars;

	if( !pBuffer )
		return ( HSTRING )LTNULL;

	if( pBuffer[0] == 0)
	{
		return (HSTRING)&g_ZeroLengthStringWrapper;
	}

	str_CalcSize(pBuffer, &nBytes, &nChars);
	pString = AllocateStringWrapper(nBytes);
	
	pString->m_RefCount = 1;
	pString->m_StringLen = (uint16)nChars;
	pString->m_DataLen = (uint16)nBytes;
	memcpy(pString->m_Bytes, pBuffer, nBytes);

	pString->m_GLink.m_pData = pString;
	gn_Insert(&g_StringHead, &pString->m_GLink);
	return (HSTRING)pString;
}


HSTRING str_CreateStringAnsi(const char *pStringData)
{
	StringWrapper *pString;
	int nBytes, nChars;

	if( !pStringData )
		return ( HSTRING )LTNULL;

	if(pStringData[0] == 0)
	{
		return (HSTRING)&g_ZeroLengthStringWrapper;
	}

	nChars = strlen(pStringData);
	nBytes = nChars+1;
	
	pString = AllocateStringWrapper(nBytes);

	pString->m_RefCount = 1;
	pString->m_StringLen = (uint16)nChars;
	pString->m_DataLen = (uint16)nBytes;
	strcpy((char*)pString->m_Bytes, pStringData);

	pString->m_GLink.m_pData = pString;
	gn_Insert(&g_StringHead, &pString->m_GLink);
	return (HSTRING)pString;
}


HSTRING str_CopyString(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

//	ASSERT(pString);
	if( !pString )
		return ( HSTRING )LTNULL;

	++pString->m_RefCount;
	return hString;
}


void str_FreeString(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

	// Don't free this one!
	if(pString == &g_ZeroLengthStringWrapper)
		return;

//	ASSERT(pString);
	if( !pString )
		return;
	
	--pString->m_RefCount;
	if(pString->m_RefCount == 0)
	{
		gn_Remove(&pString->m_GLink);
		FreeStringWrapper(pString);
	}
}


bool str_CompareStrings(HSTRING hString1, HSTRING hString2)
{
	StringWrapper *pString1 = (StringWrapper*)hString1;
	StringWrapper *pString2 = (StringWrapper*)hString2;

	if( !pString1 || !pString2 )
		return LTFALSE;
#ifndef __LINUX
	return _tcscmp((TCHAR*)pString1->m_Bytes, (TCHAR*)pString2->m_Bytes) == 0;
#else
	return strcmp((char*)pString1->m_Bytes, (char*)pString2->m_Bytes) == 0;
#endif
}


bool str_CompareStringsUpper(HSTRING hString1, HSTRING hString2)
{
	StringWrapper *pString1 = (StringWrapper*)hString1;
	StringWrapper *pString2 = (StringWrapper*)hString2;

	if( !pString1 || !pString2 )
		return LTFALSE;
#ifndef __LINUX
	return _tcsicmp((TCHAR*)pString1->m_Bytes, (TCHAR*)pString2->m_Bytes) == 0;
#else
	return strcasecmp((char*)pString1->m_Bytes, (char*)pString2->m_Bytes) == 0;
#endif
}


char* str_GetStringData(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;
	
	if( !pString )
		return LTNULL;

	return (char*)pString->m_Bytes;
}


int str_GetNumStringCharacters(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

	if( !pString )
		return 0;

	return pString->m_StringLen;
}


uint8* str_GetStringBytes(HSTRING hString, int *pNumBytes)
{
	StringWrapper *pString = (StringWrapper*)hString;

	if( !pString )
	{
		if( pNumBytes )
			*pNumBytes = 0;
		return LTNULL;
	}

	if(pNumBytes)
		*pNumBytes = pString->m_DataLen;
	
	return (uint8*)pString->m_Bytes;
}




