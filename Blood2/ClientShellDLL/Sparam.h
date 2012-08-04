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


// Prototypes...

BOOL	Sparam_Get(char* sDest, const char* sSource, const char* sId);
BOOL	Sparam_Add(char* sSource, const char* sId, const char* sParam);
BOOL	Sparam_Add(char* sSource, const char* sId, int nParam);
BOOL	Sparam_Remove(char* sSource, const char* sId);


// EOF...

#endif

