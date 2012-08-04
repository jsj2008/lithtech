// ----------------------------------------------------------------------- //
//
// MODULE  : AssertMgr.cpp
//
// PURPOSE : AssertMgr implementation
//
// CREATED : 05.06.1999
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include <crtdbg.h>
#include "AssertMgr.h"

// Statics

bool CAssertMgr::m_bEnabled = false;
_CRT_REPORT_HOOK CAssertMgr::m_crhPrevious = NULL;

// Methods

void CAssertMgr::Enable()
{
#ifdef _DEBUG
    if ( true == m_bEnabled ) return;

    m_bEnabled = true;
	m_crhPrevious = _CrtSetReportHook(CAssertMgr::ReportHook);
#endif
}

void CAssertMgr::Disable()
{
#ifdef _DEBUG
    if ( false == m_bEnabled ) return;

	_CrtSetReportHook(m_crhPrevious);

    m_bEnabled = false;
    m_crhPrevious = NULL;
#endif
}

int CAssertMgr::ReportHook(int /*nReportType*/, char* szMessage, int* pnReturnValue)
{
    if ( false == m_bEnabled )
	{
		*pnReturnValue = 0;
		return 0;
	}

	const char* szAssert = NULL;

	HCONSOLEVAR hAssertVar = NULL;

	CLIENT_CODE
	(
		hAssertVar = g_pLTClient->GetConsoleVariable("assert");
	)

	SERVER_CODE
	(
		hAssertVar = g_pLTServer->GetConsoleVariable("assert");
		if (!hAssertVar)
		{
			g_pLTServer->SetConsoleVariableString("assert", "console");
			hAssertVar = g_pLTServer->GetConsoleVariable("assert");
		}
	)

	if ( hAssertVar )
	{
	    szAssert = g_pLTBase->GetConsoleVariableString(hAssertVar);
	}

	if ( szAssert && !_stricmp(szAssert, "fullscreen") )
	{
#if defined( PLATFORM_WIN32 )
		// if assert convar = "fullscreen", switch out of renderer then do abort, retry, ignore

		HWND hWnd = FindWindow(RESEXT_WINDOWNAME, NULL);
		ShowWindow(hWnd, SW_MINIMIZE);

		char szBuffer[512];
		wsprintf(szBuffer, "\
An assert has occurred:\n\n%s\n\
Retry will step into the debugger. You should\n\
only select this option if you are currently\n\
running the game under a debugger.\n", szMessage);

		int nResult = MessageBox(hWnd, szBuffer, "Assert", MB_ABORTRETRYIGNORE);

		if ( nResult == IDABORT )
		{
			DestroyWindow(hWnd);

			*pnReturnValue = 0;
		}
		else if ( nResult == IDRETRY )
		{
			*pnReturnValue = 1;
		}
		else // if ( nResult == IDIGNORE )
		{
			ShowWindow(hWnd, SW_MAXIMIZE);
			*pnReturnValue = 0;
		}
#endif // PLATFORM_WIN32

		return TRUE;
	}
	else if ( szAssert && !_stricmp(szAssert, "window") )
	{
#if defined( PLATFORM_WIN32 )
		// if assert convar = "window", then do as normal (usually a dialog)

		HWND hWnd = FindWindow(RESEXT_WINDOWNAME, NULL);
		ShowWindow(hWnd, SW_MINIMIZE);

		char szBuffer[512];
		wsprintf(szBuffer, "\
An assert has occurred:\n\n%s\n\
Retry will step into the debugger. You should\n\
only select this option if you are currently\n\
running the game under a debugger.\n", szMessage);

		int nResult = MessageBox(hWnd, szBuffer, "Assert", MB_ABORTRETRYIGNORE);

		if ( nResult == IDABORT )
		{
			DestroyWindow(hWnd);

			*pnReturnValue = 0;
		}
		else if ( nResult == IDRETRY )
		{
			*pnReturnValue = 1;
		}
		else // if ( nResult == IDIGNORE )
		{
			*pnReturnValue = 0;
		}
#endif // PLATFORM_WIN32

		return TRUE;
	}
	else if ( szAssert && !_stricmp(szAssert, "null") )
	{
		// if assert convar = "null", then totally ignore it

		*pnReturnValue = 0;
		return TRUE;
	}
	else // if ( !_stricmp(szAssert, "console") )
	{
		// if assert convar = "console" or none of the above, put the assert into the console

        g_pLTBase->CPrint(szMessage);
		*pnReturnValue = 0;
		return TRUE;
	}
}