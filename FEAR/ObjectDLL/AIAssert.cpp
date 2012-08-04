
//----------------------------------------------------------------------------
//              
//	MODULE:		AIAssert.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	25.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "Stdafx.h"

#ifndef __AIASSERT_H__
#include "AIAssert.h"		
#endif


#define DO_NOT_CHANGE_THIS	0


// Forward declarations

// Globals

// Statics


#ifndef _FINAL

void AIAssert(long nLine, char const* szFile, char const* szExp, HOBJECT hAI, char const* szDesc, ... )
{
	LTASSERT( g_pLTServer, "AIAssert: No server." );

	char szName[64];
	if(hAI && g_pLTServer)
	{
		g_pLTServer->GetObjectName(hAI, szName, sizeof(szName));
	}

	char szFormattedDesc[512] = "";
	if( szDesc && szDesc[0] )
	{
		// Format the description with the variable arguments.
		va_list marker;
		va_start( marker, szDesc );
		LTVSNPrintF( szFormattedDesc, ARRAY_LEN( szFormattedDesc ), szDesc, marker );
		va_end( marker );
		szFormattedDesc[ARRAY_LEN( szFormattedDesc ) - 1] = 0;
	}

	if( g_pLTServer )
	{
		// AIAsserts absolutely must remain at debug level 0!!
		// If you do not want to see AIAsserts in your console/error.log, 
		// set the console variable:
		//    MuteAIAsserts 1.0
		//
		// AIAsserts are intended to be output to error.logs for content creators
		// to see.  We need to ensure that all content creators are aware of
		// all AI errors.  Content creators do not run with a DebugLevel set.
		// If you disagree with this, please speak to jeffo before reverting
		// the debug level to 0 (again).  Errors that do not appear in the error.log
		// cost engineering time in the end!!
		DebugCPrint(DO_NOT_CHANGE_THIS,"AI ASSERT FAILED! %f %s : %s (Expr: %s File: %s Line: %d)\n",
			g_pLTServer->GetTime(),
			hAI ? szName : "",
			szFormattedDesc,
			szExp, szFile, nLine);
	}
}

#endif
