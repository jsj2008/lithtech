// ----------------------------------------------------------------------- //
//
// MODULE  : GameSettings.cpp
//
// PURPOSE : Handles implementation of various game settings
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "GameSettings.h"
#include "stdio.h"
#include "GameClientShell.h"
#include "VarTrack.h"

VarTrack	g_vtMouseScaleBase;
VarTrack	g_vtMouseScaleInc;

extern CGameClientShell* g_pGameClientShell;

CGameSettings::CGameSettings()
{
    m_pClientDE = NULL;
    m_pClientShell = NULL;
}

//////////////////////////////////////////////////////////////////
//
//	INIT THE SETTINGS...
//
//////////////////////////////////////////////////////////////////

bool CGameSettings::Init (ILTClient* pClientDE, CGameClientShell* pClientShell)
{
    if (!pClientDE || !pClientShell) return false;

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	g_vtMouseScaleBase.Init(g_pLTClient, "MouseScaleBase", NULL, 0.00125f);
	g_vtMouseScaleInc.Init(g_pLTClient, "MouseScaleIncrement", NULL, 0.001125f);

	// check if gore is allowed

    uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	if (dwAdvancedOptions & AO_SOUND)
	{
		m_pClientDE->SetConsoleVariableFloat("soundenable", 1.0f);
	}
	else
	{
		m_pClientDE->SetConsoleVariableFloat("soundenable", 0.0f);
	}

	m_pClientShell->InitSound();


    return true;
}

