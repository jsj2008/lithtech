// ----------------------------------------------------------------------- //
//
// MODULE  : AssertMgr.cpp
//
// PURPOSE : AssertMgr implementation
//
// CREATED : 05.06.1999
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include "AssertMgr.h"

// Statics

LTBOOL CAssertMgr::m_bEnabled = LTFALSE;
_CRT_REPORT_HOOK CAssertMgr::m_crhPrevious = LTNULL;

// Methods

void CAssertMgr::Enable()
{
#ifdef _DEBUG
    if ( LTTRUE == m_bEnabled ) return;

    m_bEnabled = LTTRUE;
	m_crhPrevious = _CrtSetReportHook(CAssertMgr::ReportHook);
#endif
}

void CAssertMgr::Disable()
{
#ifdef _DEBUG
    if ( LTFALSE == m_bEnabled ) return;

	_CrtSetReportHook(m_crhPrevious);

    m_bEnabled = LTFALSE;
    m_crhPrevious = LTNULL;
#endif
}

int CAssertMgr::ReportHook(int nReportType, char* szMessage, int* pnReturnValue)
{
    if ( LTFALSE == m_bEnabled )
	{
		*pnReturnValue = 0;
		return 0;
	}

	const char* szAssert = NULL;

#ifdef _CLIENTBUILD
	HCONSOLEVAR hAssertVar;
    hAssertVar = g_pLTClient->GetConsoleVar("assert");
	if ( hAssertVar )
	{
        szAssert = g_pLTClient->GetVarValueString(hAssertVar);
	}
#else
	HCONVAR hAssertVar;
    hAssertVar = g_pLTServer->GetGameConVar("assert");
	if (!hAssertVar)
	{
        g_pLTServer->SetGameConVar("assert", "console");
        hAssertVar = g_pLTServer->GetGameConVar("assert");
	}
    szAssert = g_pLTServer->GetVarValueString(hAssertVar);
#endif

	if ( szAssert && !_stricmp(szAssert, "fullscreen") )
	{
		// if assert convar = "fullscreen", switch out of renderer then do abort, retry, ignore

		HWND hWnd = FindWindow("LithTech", NULL);
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

		return TRUE;
	}
	else if ( szAssert && !_stricmp(szAssert, "window") )
	{
		// if assert convar = "window", then do as normal (usually a dialog)

		HWND hWnd = FindWindow("LithTech", NULL);
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

#ifdef _CLIENTBUILD
        g_pLTClient->CPrint(szMessage);
#else
        g_pLTServer->CPrint(szMessage);
#endif
		*pnReturnValue = 0;
		return TRUE;
	}
}