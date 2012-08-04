// ----------------------------------------------------------------------- //
//
// MODULE  : UberAssert.cpp
//
// PURPOSE : More powerful assert implementation:
//           - breaks at the line of code that called assert.
//           - gives a plain english explaination, along with the expression.
//           - shows a stack trace.
//           - copies output to windows clipboard.
//
// CREATED : 6/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include <stdafx.h>
#include <crtdbg.h>
#include "UberAssert.h"
#include "stacktrace.h"

#ifdef _DEBUG


#define MAX_BUFFER 512


bool UberAssert( long nLine, char const* szFile, char const* szExp, char const* szDesc, ... )
{
	// Get the module name.
	char szModName[MAX_PATH + 1];
	GetModuleFileName ( NULL , szModName , MAX_PATH );

	const int kMaxStackSize = MAX_BUFFER - 50;
	char szStack[kMaxStackSize];
	DoStackTrace(szStack, kMaxStackSize, 1);

	char szMsg[MAX_BUFFER];
	if( szDesc && szDesc[0] )
	{
		// Format the description with the variable arguments.
		char szFormattedDesc[MAX_BUFFER];
		va_list marker;
		va_start( marker, szDesc );
		_vsnprintf( szFormattedDesc, ARRAY_LEN( szFormattedDesc ), szDesc, marker );
		va_end( marker );
		szFormattedDesc[ARRAY_LEN( szFormattedDesc ) - 1] = 0;

		_snprintf( szMsg, ARRAY_LEN( szMsg ), "%s\n%s\n\nStack Trace:\n%s\n", szExp, szFormattedDesc, szStack );
		szMsg[ARRAY_LEN( szMsg ) - 1] = 0;
	}
	else
	{
		_snprintf(szMsg, ARRAY_LEN( szMsg ), "%s\n\nStack Trace:\n%s\n", szExp, szStack);
		szMsg[ARRAY_LEN( szMsg ) - 1] = 0;
	}

	char szFileInfo[MAX_BUFFER];
	sprintf(szFileInfo, "\nDebug Assertion Failed!\nModule: %s\nFile: %s\nLine: %d\n\nExpression: ", szModName, szFile, nLine);

	OutputDebugString(szFileInfo);
	OutputDebugString(szMsg);

	if( OpenClipboard( NULL ) )
	{
		HGLOBAL hMem;
		char* pMem;

		hMem = GlobalAlloc( GHND | GMEM_DDESHARE, strlen(szMsg) + strlen(szFileInfo) + 1);

		if(hMem)
		{
			pMem = (char*)GlobalLock( hMem );
			strcpy( pMem, szFileInfo ); 
			strcat( pMem, szMsg );
			GlobalUnlock( hMem );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, hMem );
		}
		
		CloseClipboard();
	}

	_CrtSetReportHook(LTNULL);

	if( 1 == _CrtDbgReport(_CRT_ASSERT, szFile, nLine, szModName, szMsg) )
	{
		return true;
	}

	return false;
}



#endif
