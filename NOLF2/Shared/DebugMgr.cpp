
#include "DebugMgr.h"

// NOTES: this code is in general mostly broken when it comes to unicode

// Statics

DWORD CDebugMgr::m_dwOptions = 0x00000000;
char CDebugMgr::m_szBuffer[32000] = "";
char CDebugMgr::m_szAppName[128] = "LithTech";
char* CDebugMgr::m_pchBuffer = CDebugMgr::m_szBuffer;
LPTOP_LEVEL_EXCEPTION_FILTER CDebugMgr::m_prevExceptionHandler = NULL;
_CRT_REPORT_HOOK CDebugMgr::m_prevAssertHandler = NULL;

// Methods

void CDebugMgr::EnableAssertHandler()
{
	if ( m_prevAssertHandler ) return;

#ifdef _DEBUG
	m_prevAssertHandler = _CrtSetReportHook(CDebugMgr::AssertHandler);
#endif
}

void CDebugMgr::DisableAssertHandler()
{
	if ( !m_prevAssertHandler ) return;

#ifdef _DEBUG
	_CrtSetReportHook(m_prevAssertHandler);

	m_prevAssertHandler = NULL;
#endif
}

void CDebugMgr::EnableExceptionHandler()
{
	if ( m_prevExceptionHandler ) return;

	m_prevExceptionHandler = SetUnhandledExceptionFilter(CDebugMgr::ExceptionHandler);
}

void CDebugMgr::DisableExceptionHandler()
{
	if ( !m_prevExceptionHandler ) return;

	SetUnhandledExceptionFilter(m_prevExceptionHandler);

	m_prevExceptionHandler = NULL;
}

int CDebugMgr::AssertHandler(int nReportType, char* szMessage, int* pnReturnValue)
{
	CONTEXT context;
	memset(&context, 0, sizeof(context));
	context.ContextFlags = CONTEXT_FULL;
	HANDLE hThread = GetCurrentThread();
	GetThreadContext(hThread, &context);

	CDebugMgr::ClearBuffer();
	CDebugMgr::PrintfBuffer("An assert occured:\n\n%s", szMessage);
	CDebugMgr::RecordStack(&context);

	HideApp();

	MessageBox(NULL, CDebugMgr::GetBuffer(), "assert", MB_OK);

	ShowApp();

	*pnReturnValue = 0;
	return 1;
}

long __stdcall CDebugMgr::ExceptionHandler(PEXCEPTION_POINTERS pExceptionPointers)
{
	static BOOL bCaughtException = FALSE;
	if ( bCaughtException )
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	CDebugMgr::ClearBuffer();
	CDebugMgr::RecordException(pExceptionPointers);
	CDebugMgr::RecordStack(pExceptionPointers->ContextRecord);

	HideApp();

	MessageBox(NULL, CDebugMgr::GetBuffer(), "exception", MB_OK);

	KillApp();

	return EXCEPTION_CONTINUE_SEARCH;
}

void CDebugMgr::RecordException(PEXCEPTION_POINTERS pExceptionInfo)
{
    PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
    // First print information about the type of fault
    m_pchBuffer += wsprintf(m_pchBuffer,  "Exception code: %08X\n",
		pExceptionRecord->ExceptionCode);
    // Now print information about where the fault occured
    TCHAR szFaultingModule[MAX_PATH];    DWORD section, offset;
    FindLogicalAddress(  pExceptionRecord->ExceptionAddress,
                        szFaultingModule,
                        sizeof( szFaultingModule ),
                        section, offset );
    m_pchBuffer += wsprintf(m_pchBuffer,"Fault address:  %08X %02X:%08X %s",
              pExceptionRecord->ExceptionAddress,
              section, offset, szFaultingModule );

	TCHAR szMap[MAX_PATH] = "";
	wsprintf(szMap, szFaultingModule);
	wsprintf(&szMap[lstrlen(szMap)-3], "MAP");
	TCHAR szSymbol[256] = "<unknown>";
	TCHAR szUnmangledSymbol[1024] = "<unknown>";
	TCHAR szObject[128] = "<unknown>";

	if ( m_dwOptions & DEBUGMGR_RESOLVESYMBOLS )
	{
		if ( FindSymbol(szMap, section, offset, szSymbol, szObject) )
		{
			if ( m_dwOptions & DEBUGMGR_UNMANGLESYMBOLS && *szSymbol == '?' )
			{
				UnDecorateSymbolName(szSymbol, szUnmangledSymbol, 1024, UNDNAME_COMPLETE);
			}
			else
			{
				wsprintf(szUnmangledSymbol, szSymbol);
			}

			m_pchBuffer += wsprintf(m_pchBuffer, " in function %s in object file %s", szUnmangledSymbol, szObject);

			if ( m_dwOptions & DEBUGMGR_RESOLVESOURCELINE )
			{
				TCHAR szSource[256];
				TCHAR szLine[256];

				if ( FindSourceLine(szMap, szObject, section, offset, szSource, szLine) )
				{
					m_pchBuffer += wsprintf(m_pchBuffer, " at Line %s of %s\n", szLine, szSource);
				}
				else
				{
					m_pchBuffer += wsprintf(m_pchBuffer, " at Line ??? of ???\n");
				}
			}
			else
			{
				m_pchBuffer += wsprintf(m_pchBuffer, "\n\n");
			}
		}
		else
		{
			m_pchBuffer += wsprintf(m_pchBuffer, "\n");
		}
	}
	else
	{
		m_pchBuffer += wsprintf(m_pchBuffer, "\n");
	}

    PCONTEXT pCtx = pExceptionInfo->ContextRecord;    // Show the registers
    m_pchBuffer += wsprintf(m_pchBuffer, "EAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nESI:%08X\nEDI:%08X\n",
             pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );
    m_pchBuffer += wsprintf(m_pchBuffer,"CS:EIP:%04X:%08X\n", pCtx->SegCs, pCtx->Eip );
    m_pchBuffer += wsprintf(m_pchBuffer,"SS:ESP:%04X:%08X  EBP:%08X\n",
              pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
    m_pchBuffer += wsprintf(m_pchBuffer,"DS:%04X  ES:%04X  FS:%04X  GS:%04X\n",
              pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
    m_pchBuffer += wsprintf(m_pchBuffer,"Flags:%08X\n", pCtx->EFlags );
}

void CDebugMgr::RecordStack(const CONTEXT* pContext)
{
    m_pchBuffer += wsprintf(m_pchBuffer,"\nCall stack:\n" );

    m_pchBuffer += wsprintf(m_pchBuffer,"Address   Frame     Logical addr  Module\n" );

    DWORD pc = pContext->Eip;
    PDWORD pFrame, pPrevFrame;
    
    pFrame = (PDWORD)pContext->Ebp;

    do
    {
        TCHAR szModule[MAX_PATH] = "";
        DWORD section = 0, offset = 0;

        FindLogicalAddress((PVOID)pc, szModule,sizeof(szModule),section,offset );

        m_pchBuffer += wsprintf(m_pchBuffer,"%08X  %08X  %04X:%08X %s",
                  pc, pFrame, section, offset, szModule );

		// See if we need to spit out symbol/source/line info

		if ( m_dwOptions & DEBUGMGR_RESOLVESYMBOLS )
		{
			// Generate the name of the map file

			TCHAR szMap[MAX_PATH] = "";
			wsprintf(szMap, szModule);
			wsprintf(&szMap[lstrlen(szMap)-3], "MAP");

			TCHAR szSymbol[256] = "<unknown>";
			TCHAR szUnmangledSymbol[1024] = "<unknown>";
			TCHAR szObject[128] = "<unknown>";

			if ( FindSymbol(szMap, section, offset, szSymbol, szObject) )
			{
				if ( m_dwOptions & DEBUGMGR_UNMANGLESYMBOLS && *szSymbol == '?' )
				{
					UnDecorateSymbolName(szSymbol, szUnmangledSymbol, 1024, UNDNAME_COMPLETE);
				}
				else
				{
					wsprintf(szUnmangledSymbol, szSymbol);
				}
			}

			m_pchBuffer += wsprintf(m_pchBuffer, " in function %s in object file %s", szUnmangledSymbol, szObject);

			if ( m_dwOptions & DEBUGMGR_RESOLVESOURCELINE )
			{
				TCHAR szSource[256];
				TCHAR szLine[256];

				if ( FindSourceLine(szMap, szObject, section, offset, szSource, szLine) )
				{
					m_pchBuffer += wsprintf(m_pchBuffer, " at Line %s of %s\n", szLine, szSource);
				}
				else
				{
					m_pchBuffer += wsprintf(m_pchBuffer, " at Line ??? of ???\n");
				}
			}
			else
			{
				m_pchBuffer += wsprintf(m_pchBuffer, "\n\n");
			}
		}
		else
		{
			m_pchBuffer += wsprintf(m_pchBuffer, "\n");
		}

		// Go on to the next stack frame

        pc = pFrame[1];

        pPrevFrame = pFrame;

        pFrame = (PDWORD)pFrame[0]; // proceed to next higher frame on stack

        if ( (DWORD)pFrame & 3 )    // Frame pointer must be aligned on a
            break;                  // DWORD boundary.  Bail if not so.

        if ( pFrame <= pPrevFrame )
            break;

        // Can two DWORDs be read from the supposed frame address?          
        if ( IsBadWritePtr(pFrame, sizeof(PVOID)*2) )
            break;

    } while ( 1 );
}

BOOL CDebugMgr::FindLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset)
{
    MEMORY_BASIC_INFORMATION mbi;

    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
        return FALSE;

    DWORD hMod = (DWORD)mbi.AllocationBase;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
        return FALSE;

    // Point to the DOS header in memory
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

    // From the DOS header, find the NT (PE) header
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

    // Iterate through the section table, looking for the one that encompasses
    // the linear address.
    for (   unsigned i = 0;
            i < pNtHdr->FileHeader.NumberOfSections;
            i++, pSection++ )
    {
        DWORD sectionStart = pSection->VirtualAddress;
        DWORD sectionEnd = sectionStart
                    + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

        // Is the address in this section???
        if ( (rva >= sectionStart) && (rva <= sectionEnd) )
        {
            // Yes, address is in the section.  Calculate section and offset,
            // and store in the "section" & "offset" params, which were
            // passed by reference.
            section = i+1;
            offset = rva - sectionStart;
            return TRUE;
        }
    }

    return FALSE;   // Should never get here!
}

BOOL CDebugMgr::FindSymbol(const char* szMap, DWORD dwSection, DWORD dwOffset, char* szSymbol, char* szObject)
{
	HANDLE hFile = CreateFile(szMap, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if ( INVALID_HANDLE_VALUE == hFile ) return FALSE;

	BOOL bReturn = FALSE;
	DWORD cBytesRead = 0;
	DWORD cBytesToRead = GetFileSize(hFile, NULL);
	char* szBuffer = new char[cBytesToRead];
	char* pchBuffer = szBuffer;

	ReadFile(hFile, szBuffer, cBytesToRead, &cBytesRead, NULL);

	if ( cBytesToRead != cBytesRead )
	{
		bReturn = FALSE;
		goto Done;
	}

	pchBuffer = _strstr(pchBuffer, "Lib:Object", szBuffer+cBytesToRead);
	pchBuffer = _strchr(pchBuffer, '\n', szBuffer+cBytesToRead)+1;
	pchBuffer = _strchr(pchBuffer, '\n', szBuffer+cBytesToRead)+1;

	while ( *pchBuffer != '\n' )
	{
		pchBuffer = _strchr(pchBuffer, ' ', szBuffer+cBytesToRead)+1;

		// Parse in the 000x:0000xxxx section:offset 

		char szSection[16];
		lstrcpyn(szSection, pchBuffer, 5);

		char szOffset[16];
		lstrcpyn(szOffset, pchBuffer+6, 8);

		// If we're in the right section and passed the offset, return the last symbol we read

		if ( _str2dword(szSection) == dwSection &&
			 _str2dword(szOffset) > dwOffset )
		{
			bReturn = TRUE;
			goto Done;
		}

		// Get the symbol

		pchBuffer = pchBuffer + 20;
		char* pchSymbolEnd = _strchr(pchBuffer, ' ', szBuffer+cBytesToRead);
		lstrcpyn(szSymbol, pchBuffer, pchSymbolEnd - pchBuffer + 1);

		// Get the object

		char* pchEnd;
		pchBuffer = _strchr(pchBuffer, '.', szBuffer+cBytesToRead)+1;
		pchEnd = pchBuffer-1;

		while ( *(pchBuffer-1) != ' ' && *(pchBuffer-1) != ':' )
		{
			pchBuffer--;
		}

		if ( *(pchBuffer-1) == ':' )
		{
			pchEnd = pchBuffer-1;

			while ( *(pchBuffer-1) != ' ' )
			{
				pchBuffer--;
			}
		}
		
		lstrcpyn(szObject, pchBuffer, pchEnd - pchBuffer + 1);

		// Next line

		pchBuffer = _strchr(pchBuffer, '\n', szBuffer+cBytesToRead)+1;
	}

Done:

	CloseHandle(hFile);
	delete szBuffer;
	
	return bReturn;
}

BOOL CDebugMgr::FindSourceLine(const char* szMap, const char* szObject, DWORD dwSection, DWORD dwOffset, char* szSource, char* szLine)
{
	szSource[0] = 0;
	szLine[0] = 0;

	HANDLE hFile = CreateFile(szMap, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if ( INVALID_HANDLE_VALUE == hFile ) return FALSE;

	BOOL bReturn = FALSE;
	DWORD cBytesRead = 0;
	DWORD cBytesToRead = GetFileSize(hFile, NULL);
	DWORD dwNearestSLOffset = 0xFFFFFFFF;
	char* szBuffer = new char[cBytesToRead];
	char* pchBuffer = szBuffer;
	
	ReadFile(hFile, szBuffer, cBytesToRead, &cBytesRead, NULL);

	if ( cBytesToRead != cBytesRead )
	{
		bReturn = FALSE;
		goto Done;
	}

	// Find start of line numbers

	pchBuffer = _strstr(pchBuffer, "Line numbers for ", szBuffer+cBytesToRead);
	
	while ( pchBuffer )
	{
		pchBuffer = _strchr(pchBuffer, '(', szBuffer+cBytesToRead)-1;

		// Find the end of the object name

		while ( *pchBuffer != '.' )
		{
			pchBuffer--;
		}

		char* pchEnd = pchBuffer;

		// Find the beginning of the object name

		while ( *(pchBuffer-1) != '\\' )
		{
			pchBuffer--;
		}

		char szSLObject[256];

		lstrcpyn(szSLObject, pchBuffer, pchEnd - pchBuffer + 1);

		// We've got our object name, does it match?

		if ( !lstrcmpi(szObject, szSLObject) )
		{
			 pchBuffer = _strchr(pchBuffer, '(', szBuffer+cBytesToRead) + 1;
			 pchEnd = _strchr(pchBuffer, ')', szBuffer+cBytesToRead);

			 char szSLSource[256];
			 lstrcpyn(szSLSource, pchBuffer, pchEnd - pchBuffer + 1);

			 // Now go to the line info

			 pchBuffer = _strchr(pchBuffer, '\n', szBuffer+cBytesToRead)+1;

			 BOOL bReadingLines = TRUE;

			 while ( bReadingLines )
			 {
				 if ( !pchBuffer )
				 {
					 goto Done;
				 }

				 while ( *pchBuffer == ' ' || *pchBuffer == '\r' || *pchBuffer == '\n' && (pchBuffer < szBuffer+cBytesToRead) )
				 {
					 if ( (pchBuffer >= szBuffer+cBytesToRead) )
					 {
						 bReadingLines = FALSE;
						 break;
					 }

					 pchBuffer++;
				 }

				 if ( *pchBuffer == 'L' )
				 {
					bReadingLines = FALSE;
				 }

				 if ( !bReadingLines ) break;

				 pchEnd = _strchr(pchBuffer, ' ');

				 char szSLLine[256];
				 lstrcpyn(szSLLine, pchBuffer, pchEnd - pchBuffer + 1);

				 pchEnd = _strchr(pchBuffer, ':');
				 pchBuffer = pchEnd-4;

				 char szSLSection[16];
				 DWORD dwSLSection;
				 lstrcpyn(szSLSection, pchBuffer, pchEnd - pchBuffer + 1);
				 dwSLSection = _str2dword(szSLSection);

				 pchBuffer = pchEnd+1;
				 pchEnd = _strchr(pchBuffer, ' ');
				 char* pchEnd2 = _strchr(pchBuffer, '\r', szBuffer+cBytesToRead);

				 if ( pchEnd > pchEnd2 )
				 {
					 pchEnd = pchEnd2;
				 }

				 char szSLOffset[16];
				 DWORD dwSLOffset;
				 lstrcpyn(szSLOffset, pchBuffer, pchEnd - pchBuffer + 1);
				 dwSLOffset = _str2dword(szSLOffset);

				 // It's got to be in the same section

				 if ( dwSection == dwSLSection )
				 {
					 // If this offset is above the desired one, and closer than the closest one we've seen so far,
					 // then it is our new potential source/line match

					 if ( dwSLOffset <= dwOffset &&
						  (dwOffset - dwSLOffset) < (dwOffset - dwNearestSLOffset) )
					 {
						 dwNearestSLOffset = dwSLOffset;
						 lstrcpy(szSource, szSLSource);
						 lstrcpy(szLine, szSLLine);
						 bReturn = TRUE;
					 }
					 else if ( dwSLOffset > dwOffset )
					 {
						 // Offset will never decrease, so just skip to next source/line section

						 bReadingLines = FALSE;
					 }
				 }
				 else if ( dwSLSection > dwSection )
				 {
					 // Section will never decrease, so just skip to next source/line section

					 bReadingLines = FALSE;
				 }

				 pchBuffer = pchEnd;
			 }
		}

		// Get next source/line entry

		if ( pchBuffer )
		{
			pchBuffer = _strstr(pchBuffer, "Line numbers for ", szBuffer+cBytesToRead);
		}
	}

Done:

	CloseHandle(hFile);
	delete szBuffer;

	return bReturn;
}

char* CDebugMgr::_strstr(char* szString, const char* szSubstring, const char* pchStringEnd)
{
	int nLenSubstring;
	int nLenString;
	int iMatch = 0;
	int cMatchedCharacters = 0;

	nLenSubstring = lstrlen(szSubstring);

	if ( pchStringEnd )
	{
		nLenString = pchStringEnd - szString;
	}
	else
	{
		nLenString = lstrlen(szString);
	}

	while ( iMatch < nLenString )
	{
		if ( szString[iMatch] == szSubstring[cMatchedCharacters] )
		{
			cMatchedCharacters++;

			if ( cMatchedCharacters == nLenSubstring )
			{
				return &szString[iMatch - cMatchedCharacters + 1];
			}
		}
		else
		{
			cMatchedCharacters = 0;
		}

		iMatch++;
	}

	return NULL;
}

char* CDebugMgr::_strchr(char* szString, char chCharacter, const char* pchStringEnd)
{
	int iMatch = 0;
	int nLenString;

	if ( pchStringEnd )
	{
		nLenString = pchStringEnd - szString;
	}
	else
	{
		nLenString = lstrlen(szString);
	}

	while ( iMatch < nLenString )
	{
		if ( szString[iMatch] == chCharacter )
		{
			return &szString[iMatch];
		}
		
		iMatch++;
	}

	return NULL;
}

DWORD CDebugMgr::_str2dword(const char* szValue)
{
	int nLen = lstrlen(szValue);
	DWORD dwValue = 0;

	for ( int i = 0 ; i < nLen ; i++ )
	{
		switch ( szValue[i] )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				dwValue += (((unsigned int)szValue[i]-'0') << (4*(nLen - i - 1)));
				break;
			case 'a':
				dwValue += (10 << (4*(nLen - i - 1)));
				break;
			case 'b':
				dwValue += (11 << (4*(nLen - i - 1)));
				break;
			case 'c':
				dwValue += (12 << (4*(nLen - i - 1)));
				break;
			case 'd':
				dwValue += (13 << (4*(nLen - i - 1)));
				break;
			case 'e':
				dwValue += (14 << (4*(nLen - i - 1)));
				break;
			case 'f':
				dwValue += (15 << (4*(nLen - i - 1)));
				break;
		}
	}

	return dwValue;
}

void CDebugMgr::PrintfBuffer(const char* szMessage, ...)
{
	va_list marker;
	char szBuffer[32000];
	va_start(marker, szMessage);
	wvsprintf(szBuffer, szMessage, marker);
	va_end(marker);

	strcat(m_pchBuffer, szBuffer);
	m_pchBuffer += lstrlen(szBuffer);
}

void CDebugMgr::ShowApp()
{
	HWND hWnd = FindWindow(m_szAppName, NULL);
	ShowWindow(hWnd, SW_MAXIMIZE);
}

void CDebugMgr::HideApp()
{
	HWND hWnd = FindWindow(m_szAppName, NULL);
	ShowWindow(hWnd, SW_MINIMIZE);
}

void CDebugMgr::KillApp()
{
	HWND hWnd = FindWindow(m_szAppName, NULL);
	DestroyWindow(hWnd);
}
