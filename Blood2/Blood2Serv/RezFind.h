/****************************************************************************
;
;	 MODULE:		REZFIND (.H)
;
;	PURPOSE:		Routines to find the rez files
;
;	HISTORY:		08/11/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _REZFIND_H_
#define _REZFIND_H_


// Prototypes...

BOOL	FindRezFiles(HINSTANCE hInst);
char*	GetGameRezFile();
char*	GetSoundRezFile();


// EOF

#endif
