/****************************************************************************
;
;	 MODULE:		MYGAMESPYMGR (.H)
;
;	PURPOSE:		Derived Game Spy Manager for this game server
;
;	HISTORY:		09/21/98  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _MYGAMESPYMGR_H_
#define _MYGAMESPYMGR_H_


// Includes...

#include "GameSpyMgr.h"


// Classes...

class CMyGameSpyMgr : public CGameSpyMgr
{
	// Member functions...

public:
	virtual	BOOL			OnInfoQuery();
	virtual	BOOL			OnRulesQuery();
	virtual	BOOL			OnPlayersQuery();
};


// EOF...

#endif

