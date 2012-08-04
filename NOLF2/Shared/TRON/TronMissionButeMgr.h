/****************************************************************************
;
;	MODULE:			TronMissionButeMgr.h
;
;	PURPOSE:		Tron-specific Mission Bute Manager
;
;	HISTORY:		2/26/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _TRON_MISSION_BUTE_MGR_H_
#define _TRON_MISSION_BUTE_MGR_H_

#include "MissionButeMgr.h"

#define MAX_SYSTEM_MEMORY_SLOTS 24

struct TRONMISSION : public MISSION
{
	TRONMISSION();
	virtual LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szSystemMemory[MAX_SYSTEM_MEMORY_SLOTS+1];
};

class CTronMissionButeMgr : public CMissionButeMgr
{
	public :

		CTronMissionButeMgr();
		LTBOOL		Init(const char* szAttributeFile=MISSION_DEFAULT_FILE);

	protected:

		virtual	MISSION*	CreateMission( ) { return debug_new(TRONMISSION); }
		virtual void		DestroyMission( MISSION* pMission ) { debug_delete( pMission ); }
};

extern CTronMissionButeMgr* g_pTronMissionButeMgr;

#endif // _TRON_MISSION_BUTE_MGR_H_