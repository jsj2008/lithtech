/****************************************************************************
;
;	MODULE:			TO2MissionButeMgr.cpp
;
;	PURPOSE:		Mission bute manager for TO2
;
;	HISTORY:		2/26/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "TO2MissionButeMgr.h"

CMissionButeMgr* g_pMissionButeMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2MissionButeMgr::CTO2MissionButeMgr
//
//	PURPOSE:	Construction
//
// ----------------------------------------------------------------------- //
CTO2MissionButeMgr::CTO2MissionButeMgr() : CMissionButeMgr()
{
    g_pMissionButeMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2MissionButeMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
LTBOOL CTO2MissionButeMgr::Init(const char* szAttributeFile)
{
    if(!szAttributeFile) return LTFALSE;

	// See if we already have this attribute file loaded.
	if( m_strAttributeFile.GetLength( ) && m_strAttributeFile.CompareNoCase( szAttributeFile ) == 0 )
		return LTTRUE;
    
	// Start fresh.
	Term( );

	SetInRezFile( (stricmp(szAttributeFile, MISSION_DEFAULT_FILE ) == 0) || (stricmp(szAttributeFile, MISSION_COOP_FILE ) == 0));
    
	if (!Parse(szAttributeFile)) return LTFALSE;


	// Read in the properties for each mission...
	int nNum = 0;
	sprintf(m_aTagName, "%s%d", MMGR_MISSION_TAG, nNum);

	while (m_buteMgr.Exist(m_aTagName))
	{
		MISSION* pMission = CreateMission( );

		if (pMission && pMission->Init(m_buteMgr, m_aTagName))
		{
			pMission->nId = nNum;
			m_MissionList.push_back(pMission);
		}
		else
		{
			DestroyMission(pMission);
            return LTFALSE;
		}

		nNum++;
		sprintf(m_aTagName, "%s%d", MMGR_MISSION_TAG, nNum);
	}

    return LTTRUE;
}