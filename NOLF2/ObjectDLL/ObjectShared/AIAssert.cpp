
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
#include "stdafx.h"

#ifndef __AIASSERT_H__
#include "AIAssert.h"		
#endif

// Forward declarations

// Globals

// Statics


#ifndef _FINAL

void AIAssert(long nLine, char const* szFile, char const* szExp, HOBJECT hAI, char const* szDesc, ... )
{
	UBER_ASSERT( g_pLTServer, "AIAssert: No server." );

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
		_vsnprintf( szFormattedDesc, ARRAY_LEN( szFormattedDesc ), szDesc, marker );
		va_end( marker );
		szFormattedDesc[ARRAY_LEN( szFormattedDesc ) - 1] = 0;
	}

	if( g_pLTServer )
	{
		g_pLTServer->CPrint("AI ASSERT FAILED! %f %s : %s (Expr: %s File: %s Line: %d)\n",
			g_pLTServer->GetTime(),
			hAI ? szName : "",
			szFormattedDesc,
			szExp, szFile, nLine);
	}
}

#endif
