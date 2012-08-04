/****************************************************************************
;
;	 MODULE:		SPARAM (.CPP)
;
;	PURPOSE:		String Parameter funcions
;
;	HISTORY:		10/13/96 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "Windows.h"
#include "Sparam.h"
#include "assert.h"
#include <mbstring.h>

// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Sparam_Get
//
//	PURPOSE:	Gets the parameter with the given ID
//
// ----------------------------------------------------------------------- //

BOOL Sparam_Get(char* sDest, const char* sSource, const char* sId)
{
	// Sanity checks...

	assert(sSource);
	assert(sId);


	// Scan for the ID...

	char sRealId[256];
	wsprintf(sRealId, "[%s:", sId);

	char* sStart = (char*)_mbsstr((const unsigned char*)sSource, (const unsigned char*)sRealId);
	if (!sStart) return(FALSE);


	// Move to the start of the param...

	int nLen = _mbstrlen(sRealId);
	sStart = &sStart[nLen];
	if (_mbstrlen(sStart) < 2) return(FALSE);


	// Find the end of the param...

	char* pEnd = (char*)_mbsstr((const unsigned char*)sStart, (const unsigned char*)"]");
	if (!pEnd) return(FALSE);
	if (pEnd <= sStart) return(FALSE);


	// Fill the dest string with the param...

	int i = 0;

	while (&sStart[i] != pEnd)
	{
		sDest[i] = sStart[i];
		i++;
	}

	sDest[i] = '\0';


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Sparam_Add
//
//	PURPOSE:	Addss the given parameter with the given ID
//
//	WARNING:	The source string must contain enough extra memory for
//				the new param and id!
//
// ----------------------------------------------------------------------- //

BOOL Sparam_Add(char* sSource, const char* sId, const char* sParam)
{
	// Sanity checks...

	assert(sSource);
	assert(sId);
	if (!sParam) return(FALSE);

	
	// Add the real id...

	_mbscat((unsigned char*)sSource, (const unsigned char*)"[");
	_mbscat((unsigned char*)sSource, (const unsigned char*)sId);
	_mbscat((unsigned char*)sSource, (const unsigned char*)":");


	// Add the param...

	_mbscat((unsigned char*)sSource, (const unsigned char*)sParam);


	// Add the closing id...

	_mbscat((unsigned char*)sSource, (const unsigned char*)"]");


	// All done...

	return(TRUE);
}

BOOL Sparam_Add(char* sSource, const char* sId, int nParam)
{
	char sTmp[256];
	wsprintf(sTmp, "%i", nParam);

	return(Sparam_Add(sSource, sId, sTmp));
}


