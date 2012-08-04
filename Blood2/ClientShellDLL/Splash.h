/****************************************************************************
;
;	 MODULE:		Splash (.H)
;
;	PURPOSE:		Splash screen state functions
;
;	HISTORY:		10/18/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _SPLASH_H_
#define _SPLASH_H_


// Includes...

#include "cpp_clientshell_de.h"


// Prototypes...

DBOOL	 Splash_SetState(CClientDE* pClientDE, char* sScreen, DBOOL bExit = DFALSE);
DBOOL	 Splash_SetInfo(CClientDE* pClientDE, char* sScreen, DBOOL bExit = DFALSE);
void	 Splash_Term();
void	 Splash_Update();
void	 Splash_OnKeyDown(int nKey);
HSURFACE Splash_Display(CClientDE* pClientDE, char* sScreen, DBOOL bStretch = DFALSE);


// EOF...

#endif

