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


// Includes...

// Prototypes...

bool	Sparam_Get(char* sDest, const char* sSource, const char* sId);
bool	Sparam_Add(char* sSource, const char* sId, const char* sParam);
bool	Sparam_Add(char* sSource, const char* sId, int nParam);
bool	Sparam_Remove(char* sSource, const char* sId);


// EOF...

#endif

