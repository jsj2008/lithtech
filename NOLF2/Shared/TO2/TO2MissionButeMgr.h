/****************************************************************************
;
;	MODULE:			TO2MissionButeMgr.h
;
;	PURPOSE:		TO2-specific Mission Bute Manager
;
;	HISTORY:		2/26/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _TO2_MISSION_BUTE_MGR_H_
#define _TO2_MISSION_BUTE_MGR_H_

#include "MissionButeMgr.h"

class CTO2MissionButeMgr : public CMissionButeMgr
{
	public :

		CTO2MissionButeMgr();
		LTBOOL		Init(const char* szAttributeFile=MISSION_DEFAULT_FILE);
};

#endif // _TO2_MISSION_BUTE_MGR_H_