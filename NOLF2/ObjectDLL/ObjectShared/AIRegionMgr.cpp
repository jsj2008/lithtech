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
	m_apRegions = LTNULL;
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
	if ( m_apRegions )
	{
		debug_deletea(m_apRegions);
		m_apRegions = LTNULL;
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

	m_apRegions = debug_newa(AIRegion*, m_cRegions);

	uint32 iRegion = 0;

	// Now we put the Regions into our array

	uint32 nId = 0;
	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			m_apRegions[nId] = (AIRegion*)g_pLTServer->HandleToObject(hCurObject);

			nId++;
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIRegion))
		{
			m_apRegions[nId] = (AIRegion*)g_pLTServer->HandleToObject(hCurObject);

			nId++;
		}
	}

	// All done

    g_pLTServer->CPrint("Added %d regions", m_cRegions);

	m_bInitialized = LTTRUE;
}

void CAIRegionMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_BOOL(m_bInitialized);
	LOAD_INT(m_cRegions);

	if( m_apRegions )
	{
		debug_deletea(m_apRegions);
		m_apRegions = LTNULL;
	}
	m_apRegions = debug_newa(AIRegion*, m_cRegions);

    for ( uint32 iRegion = 0 ; iRegion < m_cRegions ; iRegion++ )
	{
		HOBJECT hObject;
		LOAD_HOBJECT(hObject);
		m_apRegions[iRegion] = (AIRegion*)g_pLTServer->HandleToObject(hObject);
	}
}

void CAIRegionMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL(m_bInitialized);
	SAVE_INT(m_cRegions);

	if ( m_apRegions )
	{
        for ( uint32 iRegion = 0 ; iRegion < m_cRegions ; iRegion++ )
		{
			SAVE_HOBJECT(m_apRegions[iRegion]->m_hObject);
		}
	}
}
