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

#include "stdafx.h"
#include "Sparam.h"


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

	_ASSERT(sSource);
	_ASSERT(sId);


	// Scan for the ID...

	char sRealId[256];
	wsprintf(sRealId, "[%s:", sId);

	char* sStart = strstr(sSource, sRealId);
	if (!sStart) return(FALSE);


	// Move to the start of the param...

	int nLen = strlen(sRealId);
	sStart = &sStart[nLen];
	if (strlen(sStart) < 2) return(FALSE);


	// Find the end of the param...

	char* pEnd = strstr(sStart, "]");
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

	_ASSERT(sSource);
	_ASSERT(sId);
	if (!sParam) return(FALSE);

	
	// Add the real id...

	strcat(sSource, "[");
	strcat(sSource, sId);
	strcat(sSource, ":");


	// Add the param...

	strcat(sSource, sParam);


	// Add the closing id...

	strcat(sSource, "]");


	// All done...

	return(TRUE);
}

BOOL Sparam_Add(char* sSource, const char* sId, int nParam)
{
	char sTmp[256];
	wsprintf(sTmp, "%i", nParam);

	return(Sparam_Add(sSource, sId, sTmp));
}


