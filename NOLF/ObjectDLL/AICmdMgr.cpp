// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AICmdMgr.h"

// Globals

CAICmdMgr* g_pAICmdMgr = LTNULL;

// Methods

CAICmdMgr::CAICmdMgr()
{
	g_pAICmdMgr = this;
	m_cCmds = 0;
	m_bInitialized = LTFALSE;
	m_apCmds = LTNULL;
}

CAICmdMgr::~CAICmdMgr()
{
	g_pAICmdMgr = LTNULL;
	Term();
}

void CAICmdMgr::Term()
{
	m_cCmds = 0;
	m_bInitialized = LTFALSE;
	if ( m_apCmds )
	{
		debug_deletea(m_apCmds);
		m_apCmds = LTNULL;
	}
}

void CAICmdMgr::Init()
{
	Term();

	// First, we count up the number of Cmds in the level

	HCLASS  hAICmd = g_pLTServer->GetClass("AICmd");
	HOBJECT	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAICmd))
		{
			m_cCmds++;
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAICmd))
		{
			m_cCmds++;
		}
	}

	if ( 0 == m_cCmds ) return;

	m_apCmds = debug_newa(AICmd*, m_cCmds);

	uint32 iCmd = 0;

	// Now we put the Cmds into our array

	uint32 nId = 0;
	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAICmd))
		{
			m_apCmds[nId] = (AICmd*)g_pLTServer->HandleToObject(hCurObject);
			m_apCmds[nId]->Verify();

			nId++;
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAICmd))
		{
			m_apCmds[nId] = (AICmd*)g_pLTServer->HandleToObject(hCurObject);
			m_apCmds[nId]->Verify();

			nId++;
		}
	}

	// All done

    g_pLTServer->CPrint("Added %d Cmds", m_cCmds);

	m_bInitialized = LTTRUE;
}
