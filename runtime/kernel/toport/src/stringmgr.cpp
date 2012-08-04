// ----------------------------------------------------------------
// string manager.
//  lithtech (c) 1999
// ----------------------------------------------------------------

#include "bdefs.h"
#include "clientmgr.h"
#include "stringmgr.h"
#include "stringhelper.h"
#include "dsys.h"

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

// ----------------------------------------------------------------------- //
// CStringMgr: IStringMgr implementation.
// ----------------------------------------------------------------------- //
class CStringMgr : public IStringMgr
{
    declare_interface(CStringMgr);
    
	void Init()	{ str_Init(); }
	void Term() { str_Term(); }
	void ShowAllStringsAllocated(StringShowFn fn, void *pUser)
	{ str_ShowAllStringsAllocated(fn, pUser); }


	// bufferLen is set to the number of bytes in the string excluding the null terminator.
	// You MUST free the string returned by here with FreeStringBuffer.
	uint8* FormatString(CBindModuleType *hModule, int stringCode, va_list *marker, int *bufferLen)
	{ return str_FormatString(hModule, stringCode, marker, bufferLen); }

	void FreeStringBuffer(uint8 *pBuffer)
	{ str_FreeStringBuffer(pBuffer); }

	bool GetString(CBindModuleType *hModule, int stringCode, 
		uint8 *pBuffer, int bufferLen, int *pBufferLen)
	{ return false; }
/*	{ return str_GetString(hModule, stringCode, pBuffer, bufferLen, pBufferLen); }*/

	HSTRING CreateString(uint8 *pBuffer)		{ return str_CreateString(pBuffer); }
	HSTRING CreateStringAnsi(char *pString)		{ return str_CreateStringAnsi(pString); }
	HSTRING CopyString(HSTRING hString)			{ return str_CopyString(hString); }
	void FreeString(HSTRING hString)			{ str_FreeString(hString); }
	bool CompareStrings(HSTRING hString1, HSTRING hString2)			{ return str_CompareStrings(hString1, hString2); }
	bool CompareStringsUpper(HSTRING hString1, HSTRING hString2)	{ return str_CompareStringsUpper(hString1, hString2); }
	char* GetStringData(HSTRING hString)		{ return str_GetStringData(hString); }

	int GetNumStringCharacters(HSTRING hString) { return str_GetNumStringCharacters(hString); }

	// Gives you a pointer to the string's bytes.
	// Optionally fills in the number of bytes this string takes, including
	// the null terminating character (ie: if you want to transfer the string around,
	// you can send pNumBytes worth of the bytes it returns and CreateString() with that).
	uint8* GetStringBytes(HSTRING hString, int *pNumBytes)			{ return str_GetStringBytes(hString, pNumBytes); }

};

// ----------------------------------------------------------------
//  
// ----------------------------------------------------------------
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
	NULL, NULL, NULL,
	0, 0, 0, 0
};

extern int32 g_bDebugStrings;

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
		// Kill the SLUT
		StringTableTerm();

		// Free all the allocated strings.
		pCur = g_StringHead.m_pNext;
		while(pCur != &g_StringHead)
		{
			pNext = pCur->m_pNext;
			free(pCur->m_pData);
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
	*bufferLen = 0;

	// Load the string..
    char *rawString = StringTableLookup(stringCode);

    if(!rawString) {
        if(g_bDebugStrings) {
            dsi_ConsolePrint("Couldn't get string %d", stringCode);
        }
        return(LTNULL);
	}

    // sprintf string into a temporary buffer - this is not foolproof, if the final string > tempBuffer,
    // bad things will happen!
    char tempBuffer[4096];
    vsprintf(tempBuffer, (const char *)rawString, *marker);
    ASSERT(strlen(tempBuffer) < sizeof(tempBuffer));

    // Allocate a string the size of the final sprintf'ed string and copy it from tempBuffer
    char *newString = (char *)malloc(strlen(tempBuffer) + 1);
    strcpy(newString, tempBuffer);

    *bufferLen = strlen(newString) + 1;

    return((uint8*)newString);
}


void str_FreeStringBuffer(uint8 *pBuffer)
{
    if(pBuffer) {
        free(pBuffer);
    }
}


/*  This function is not used anywhere... is it obsolete?

LTBOOL str_GetString(CBindModuleType *hModule, int stringCode,
	uint8 *pBuffer, int bufferLen, int *pBufferLen)
{
	WinBind *pBind = (WinBind*)hModule;
	uint32 nBytes;

	nBytes = LoadString(pBind->m_hInstance, stringCode, (TCHAR*)pBuffer, bufferLen);

	if(nBytes == 0)
	{
		if(g_bDebugStrings)
			dsi_ConsolePrint("Couldn't get string %d", stringCode);

		return FALSE;
	}
	else
	{
		// There should be a better way to do this.. FormatMessage doesn't define
		// its return value very well at all so we just allocate 2 bytes for
		// each character to make sure.
		nBytes += nBytes*2 + 1;
		*pBufferLen = (int)nBytes;
		return TRUE;
	}
}
*/


static inline StringWrapper* AllocateStringWrapper(int nStringBytes)
{
	StringWrapper*  p;
	LT_MEM_TRACK_ALLOC(p = (StringWrapper*)LTMemAlloc(sizeof(StringWrapper) + (nStringBytes-1)),LT_MEM_TYPE_MISC);
	return p;
}

static inline void FreeStringWrapper(StringWrapper *pString)
{
	LTMemFree(pString);
}


static inline void str_CalcSize(uint8 *pString, int *pNumBytes, int *pNumChars)
{
    *pNumChars = strlen((char *)pString);
    *pNumBytes = *pNumChars + 1;
}


HSTRING str_CreateString(uint8 *pBuffer)
{
	StringWrapper *pString;
	int nBytes, nChars;

	if( !pBuffer )
		return ( HSTRING )NULL;

	if( pBuffer[0] == 0)
	{
		return (HSTRING)&g_ZeroLengthStringWrapper;
	}

	str_CalcSize(pBuffer, &nBytes, &nChars);
	pString = AllocateStringWrapper(nBytes);

	pString->m_RefCount = 1;
	pString->m_StringLen = nChars;
	pString->m_DataLen = nBytes;
	memcpy(pString->m_Bytes, pBuffer, nBytes);

	pString->m_GLink.m_pData = pString;
	gn_Insert(&g_StringHead, &pString->m_GLink);
	return (HSTRING)pString;
}


HSTRING str_CreateStringAnsi(char *pStringData)
{
	StringWrapper *pString;
	int nBytes, nChars;

	if( !pStringData )
		return ( HSTRING )NULL;

	if(pStringData[0] == 0)
	{
		return (HSTRING)&g_ZeroLengthStringWrapper;
	}

	nChars = strlen(pStringData);
	nBytes = nChars+1;

	pString = AllocateStringWrapper(nBytes);

	pString->m_RefCount = 1;
	pString->m_StringLen = nChars;
	pString->m_DataLen = nBytes;
	strcpy((char*)pString->m_Bytes, pStringData);

	pString->m_GLink.m_pData = pString;
	gn_Insert(&g_StringHead, &pString->m_GLink);
	return (HSTRING)pString;
}


HSTRING str_CopyString(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

	if( !pString )
		return ( HSTRING )NULL;

	++pString->m_RefCount;
	return hString;
}


void str_FreeString(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

	// Don't free this one!
	if(pString == &g_ZeroLengthStringWrapper)
		return;

	if( !pString )
		return;

	--pString->m_RefCount;
	if(pString->m_RefCount == 0)
	{
		gn_Remove(&pString->m_GLink);
		FreeStringWrapper(pString);
	}
}


LTBOOL str_CompareStrings(HSTRING hString1, HSTRING hString2)
{
	StringWrapper *pString1 = (StringWrapper*)hString1;
	StringWrapper *pString2 = (StringWrapper*)hString2;

	if( !pString1 || !pString2 )
		return FALSE;

    return(strcmp((const char *)pString1->m_Bytes, (const char *)pString2->m_Bytes));
}


LTBOOL str_CompareStringsUpper(HSTRING hString1, HSTRING hString2)
{
	StringWrapper *pString1 = (StringWrapper*)hString1;
	StringWrapper *pString2 = (StringWrapper*)hString2;

	if( !pString1 || !pString2 )
		return FALSE;

    return(stricmp((const char *)pString1->m_Bytes, (const char *)pString2->m_Bytes));
}


char* str_GetStringData(HSTRING hString)
{
	StringWrapper *pString = (StringWrapper*)hString;

	if( !pString )
		return NULL;

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
		return NULL;
	}

	if(pNumBytes)
		*pNumBytes = pString->m_DataLen;

	return (uint8*)pString->m_Bytes;
}



// --------------------------------------------------------
// PSX2-specific string lookup table (SLUT) stuff
// --------------------------------------------------------

// Andy Megowan's magical String Lookup Table

const char MAGIC[4] = {'L','T','S','R'};
const int32 RESOURCE_VERSION = 0x00010000;
const int16 ALIGNMENT_BYTECOUNT = 16;

struct LTStringHeader
{
	uint8 m_magic[4];
	uint32 m_version;
	uint32 m_stringcount;
	uint32 m_filler;
};

struct LTStringEntry
{
	uint16 m_wID;
	uint16 m_wLength;
	uint32 m_Offset;
};

uint8 * g_pStrings = LTNULL;
CMapWordToPtr g_StringMap;

LTBOOL str_LoadStringTable(void)
{
#ifdef __CODEWARRIOR

	void * pStringFile = NULL;
	int32 iStringSize = 0;
			
	// Open the file and read the whole darn thing into a chunk of memory
	{
	    ILTStream *pStream;
	    FileRef ref;
	    int32 iSize;
	    
	    ref.m_FileType = FILE_ANYFILE;
	    ref.m_pFilename = "strings/lith.str";

		pStream = client_file_mgr->OpenFile(&ref);
		
		if (pStream == NULL)
			return LTFALSE;

	    iSize = pStream->GetLen();
		iStringSize = iSize;

		LT_MEM_TRACK_ALLOC(pStringFile = new uint8[iSize],LT_MEM_TYPE_MISC);

		if (!pStringFile)
		{
	        dsi_ConsolePrint("Not enough memory to load strings");
	        pStream->Release();
			return(LTFALSE);
		}

		// Slurp the whole file into memory.
	    pStream->Read(pStringFile, iSize);
	    pStream->Release();
	}
	// Check for validity
	LTStringHeader * pHeader = (LTStringHeader *)pStringFile;

	if (strncmp((const char *)pHeader->m_magic, MAGIC, 4))
	{
		// File header of unknown type.
        dsi_ConsolePrint("File header of unknown type");
		return(LTFALSE);
	}

	if (pHeader->m_version != RESOURCE_VERSION)
	{
		dsi_ConsolePrint("File version is unknown.");
		return(LTFALSE);
	}

	// Allocate enough memory for an array of just the strings
	int32 iHeaderSize = 16 + (pHeader->m_stringcount * sizeof(LTStringEntry));
	iStringSize -= iHeaderSize;
	LT_MEM_TRACK_ALLOC(g_pStrings = new uint8[iStringSize],LT_MEM_TYPE_MISC);
	if (!g_pStrings)
	{
        dsi_ConsolePrint("Not enough memory to load strings");
		return(LTFALSE);
	}
	memcpy(g_pStrings, (void *)((uint8 *)pStringFile + iHeaderSize), iStringSize);

	// tiny hack to get the address of the array of string entries
	LTStringHeader * pTemp = pHeader;	pTemp++;
	LTStringEntry * pStringEntry = (LTStringEntry *)pTemp;

	g_StringMap.RemoveAll();
	for (uint32 i = 0; i < pHeader->m_stringcount; i++)
	{
//		g_StringMap[pStringEntry->m_wID] = (void *)((uint8 *)pStringFile + pStringEntry->m_Offset);
		g_StringMap[pStringEntry->m_wID] = (void *)((uint8 *)g_pStrings + pStringEntry->m_Offset - iHeaderSize);
		pStringEntry++;
	}
	delete (pStringFile);
	return LTTRUE;
#endif // CODEWARRIOR
	return LTFALSE;
}

void StringTableTerm(void)
{
	g_StringMap.RemoveAll();
	if (g_pStrings)
	{
		delete (g_pStrings);
		g_pStrings = LTNULL;
	}
}

/*
	Written 11/28/2K by Andy Megowan

	Performs a hashtable search of the SLUT, or String Look-Up Table (I just like calling it a SLUT)
	returns a pointer to a zero-delimited uint8 string.
*/
char * StringTableLookup(int stringCode)
{
	char * pStr = (char *)g_StringMap[stringCode];

	if (pStr)
		return pStr;

	static char rawString[32];
	sprintf(rawString, "String #%d", stringCode);
	return rawString;
}
