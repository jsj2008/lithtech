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

bool Sparam_Get(char* sDest, const char* sSource, const char* sId)
{
	// Sanity checks...

	_ASSERT(sSource);
	_ASSERT(sId);


	// Scan for the ID...

	char sRealId[256];
	wsprintf(sRealId, "[%s:", sId);

	const char* sStart = strstr(sSource, sRealId);
	if (!sStart) return(false);


	// Move to the start of the param...

	int nLen = strlen(sRealId);
	sStart = &sStart[nLen];
	if (strlen(sStart) < 2) return(false);


	// Find the end of the param...

	const char* pEnd = strstr(sStart, "]");
	if (!pEnd) return(false);
	if (pEnd <= sStart) return(false);


	// Fill the dest string with the param...

	int i = 0;

	while (&sStart[i] != pEnd)
	{
		sDest[i] = sStart[i];
		i++;
	}

	sDest[i] = '\0';


	// All done...

	return (true);
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

bool Sparam_Add(char* sSource, const char* sId, const char* sParam)
{
	// Sanity checks...

	_ASSERT(sSource);
	_ASSERT(sId);
	if (!sParam) return(false);

	
	// Add the real id...

	strcat(sSource, "[");
	strcat(sSource, sId);
	strcat(sSource, ":");


	// Add the param...

	strcat(sSource, sParam);


	// Add the closing id...

	strcat(sSource, "]");


	// All done...

	return(true);
}

bool Sparam_Add(char* sSource, const char* sId, int nParam)
{
	char sTmp[256];
	wsprintf(sTmp, "%i", nParam);

	return(Sparam_Add(sSource, sId, sTmp));
}


