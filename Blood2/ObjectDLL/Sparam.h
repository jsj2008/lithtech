/****************************************************************************
;
;	 MODULE:		SPARAM (.H)
;
;	PURPOSE:		String Parameter funcions
;
;	HISTORY:		10/13/96 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _SPARAM_H_
#define _SPARAM_H_

#include "basetypes_de.h"

// Includes...

//#include "Windows.h"	GK - including this causes problems for some files..


// Prototypes...

DBOOL	Sparam_Get(char* sDest, const char* sSource, const char* sId);
DBOOL	Sparam_Add(char* sSource, const char* sId, const char* sParam);
DBOOL	Sparam_Add(char* sSource, const char* sId, int nParam);
DBOOL	Sparam_Remove(char* sSource, const char* sId);


// EOF...

#endif

