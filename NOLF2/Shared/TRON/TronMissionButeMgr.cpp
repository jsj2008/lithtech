/****************************************************************************
;
;	MODULE:			TronMissionButeMgr.cpp
;
;	PURPOSE:		Mission bute manager for TRON
;
;	HISTORY:		2/26/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "TronMissionButeMgr.h"

CMissionButeMgr* g_pMissionButeMgr = NULL;
CTronMissionButeMgr* g_pTronMissionButeMgr = NULL;

#define MMGR_MISSION_SYSTEM_MEMORY "SystemMemory"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRONMISSION::TRONMISSION
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
TRONMISSION::TRONMISSION() : MISSION()
{
	// Clear our our system memory
	memset(szSystemMemory,0,(MAX_SYSTEM_MEMORY_SLOTS+1)*sizeof(char));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRONMISSION::Init
//
//	PURPOSE:	Build the mission struct
//
// ----------------------------------------------------------------------- //
LTBOOL TRONMISSION::Init(CButeMgr & buteMgr, char* aTagName)
{
	if(!MISSION::Init(buteMgr,aTagName))
		return LTFALSE;

	szSystemMemory[0] = 0;

	buteMgr.GetString(aTagName, MMGR_MISSION_SYSTEM_MEMORY, szSystemMemory, MAX_SYSTEM_MEMORY_SLOTS);
	
	// Validation
	if(!szSystemMemory[0] || (strlen(szSystemMemory) < MAX_SYSTEM_MEMORY_SLOTS))
	{
		// Missions.txt wasn't set!
		TRACE("ERROR - %s didn't specify a valid system memory configuration for a mission\n", g_pMissionButeMgr->GetAttributeFile( ));

		// Let's allow us to continue at least
		memset(szSystemMemory,'.',MAX_SYSTEM_MEMORY_SLOTS*sizeof(char));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionButeMgr::CTronMissionButeMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CTronMissionButeMgr::CTronMissionButeMgr() : CMissionButeMgr()
{
    g_pMissionButeMgr = this;
	g_pTronMissionButeMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronMissionButeMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
LTBOOL CTronMissionButeMgr::Init(const char* szAttributeFile)
{
    if(!szAttributeFile) return LTFALSE;
    
	// See if we already have this attribute file loaded.
	if( m_strAttributeFile.GetLength( ) && m_strAttributeFile.CompareNoCase( szAttributeFile ) == 0 )
		return LTTRUE;

	// Start fresh.
	Term( );
    
	if (!Parse(szAttributeFile)) return LTFALSE;

	// Read in the properties for each mission...
	int nNum = 0;
	sprintf(m_aTagName, "%s%d", MMGR_MISSION_TAG, nNum);

	while (m_buteMgr.Exist(m_aTagName))
	{
		TRONMISSION* pMission = ( TRONMISSION* )CreateMission( );

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