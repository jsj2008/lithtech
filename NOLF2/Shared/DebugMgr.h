
#ifndef _STACK_WALK_H_
#define _STACK_WALK_H_

#include <stdio.h>
#include <Windows.h>
#include <Winnt.h>
#include <ImageHlp.h>
#include <CrtDbg.h>

#pragma comment (lib, "ImageHlp.lib")

#define	DEBUGMGR_RESOLVESYMBOLS		0x00000001
#define	DEBUGMGR_UNMANGLESYMBOLS	0x00000002
#define	DEBUGMGR_RESOLVESOURCELINE	0x00000003

class CDebugMgr
{
	public :

		static void EnableAssertHandler();
		static void DisableAssertHandler();

		static void EnableExceptionHandler();
		static void DisableExceptionHandler();

		static void SetApp(const char* szAppName) { lstrcpy(m_szAppName, szAppName); }
		static void ShowApp();
		static void HideApp();
		static void KillApp();

		static long __stdcall ExceptionHandler(PEXCEPTION_POINTERS pExceptionPointers);

		static void RecordStack(const CONTEXT* pContext);
		static void RecordException(PEXCEPTION_POINTERS pExceptionInfo);

		static void ClearBuffer() { m_szBuffer[0] = 0; m_pchBuffer = m_szBuffer; }
		static const char* GetBuffer() { return m_szBuffer; }
		static void PrintfBuffer(const char* szMessage, ...);

		static void SetOption(DWORD dwOption) { m_dwOptions |= dwOption; }
		static void ClearOption(DWORD dwOption) { m_dwOptions &= ~dwOption; }
		static void SetOptions(DWORD dwOptions) { m_dwOptions = dwOptions; }
		static void ClearOptions() { m_dwOptions = 0x00000000; }
		static DWORD GetOptions() { return m_dwOptions; }

	protected :

		static int AssertHandler(int nReportType, char* szMessage, int* pnReturnValue);

		static BOOL FindLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset);
		static BOOL FindSymbol(const char* szMap, DWORD dwSection, DWORD dwOffset, char* szSymbol, char* szObject);
		static BOOL FindSourceLine(const char* szMap, const char* szObject, DWORD dwSection, DWORD dwOffset, char* szSource, char* szLine);

		static char* _strstr(char* szString, const char* szSubstring, const char* pchStringEnd = NULL);
		static char* _strchr(char* szString, char chCharacter, const char* pchStringEnd = NULL);
		static DWORD _str2dword(const char* szValue);

	private :

		static DWORD m_dwOptions;
		static char m_szBuffer[32000];
		static char* m_pchBuffer;
		static char m_szAppName[128];

		static LPTOP_LEVEL_EXCEPTION_FILTER m_prevExceptionHandler;
		static _CRT_REPORT_HOOK	m_prevAssertHandler;
};

#endif