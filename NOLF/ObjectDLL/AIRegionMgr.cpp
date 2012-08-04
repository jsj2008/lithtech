// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIRegionMgr.h"
#include "AIPath.h"
#include "FastHeap.h"
#include "WorldProperties.h"

// Globals

CAIRegionMgr* g_pAIRegionMgr = LTNULL;

// Methods

CAIRegionMgr::CAIRegionMgr()
{
	g_pAIRegionMgr = this;
	m_cRegions = 0;
	m_bInitialized = LTFALSE;
	m_aRegions = LTNULL;
}

CAIRegionMgr::~CAIRegionMgr()
{
	g_pAIRegionMgr = LTNULL;
	Term();
}

void CAIRegionMgr::Term()
{
	m_cRegions = 0;
	m_bInitialized = LTFALSE;
	if ( m_aRegions )
	{
		debug_deletea(m_aRegions);
		m_aRegions = LTNULL;
	}
}

void CAIRegionMgr::Init()
{
	Term();

	// First, we count up the number of regions in the level

	HCLASS  hAIRegion = g_pLTServer->GetClass("AIRegion");
	HOBJECT	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			m_cRegions++;
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			m_cRegions++;
		}
	}

	if ( 0 == m_cRegions ) return;

	m_aRegions = debug_newa(CAIRegion, m_cRegions);

	int32 iRegion = 0;

	// Now we put the Regions int32o our array

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			// Setup the region

            m_aRegions[iRegion].Init(iRegion, *(AIRegion*)g_pLTServer->HandleToObject(hCurObject));
			iRegion++;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			// Setup the region

            m_aRegions[iRegion].Init(iRegion, *(AIRegion*)g_pLTServer->HandleToObject(hCurObject));
			iRegion++;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

	// All done

    g_pLTServer->CPrint("Added %d regions", m_cRegions);

	m_bInitialized = LTTRUE;
}

void CAIRegionMgr::Load(HMESSAGEREAD hRead)
{
	LOAD_BOOL(m_bInitialized);
	LOAD_INT(m_cRegions);

	if ( 0 == m_cRegions ) 
	{
		if ( m_aRegions )
		{
			debug_deletea(m_aRegions); 
			m_aRegions = LTNULL;
		}

		return;
	}

	m_aRegions = debug_newa(CAIRegion, m_cRegions);
	for ( int32 iRegion = 0 ; iRegion < m_cRegions ; iRegion++ )
	{
		m_aRegions[iRegion].Load(hRead);
	}
}

void CAIRegionMgr::Save(HMESSAGEWRITE hWrite)
{
	SAVE_BOOL(m_bInitialized);
	SAVE_INT(m_cRegions);
	for ( int32 iRegion = 0 ; iRegion < m_cRegions ; iRegion++ )
	{
		m_aRegions[iRegion].Save(hWrite);
	}
}

CAIRegion* CAIRegionMgr::GetRegionByIndex(int32 iRegion)
{ 
	if ( !IsInitialized() || (iRegion >= GetNumRegions()) || (iRegion < 0) ) return LTNULL;

	return &m_aRegions[iRegion]; 
}
