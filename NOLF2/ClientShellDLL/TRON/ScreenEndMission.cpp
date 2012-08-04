// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndMission.cpp
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenEndMission.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ScreenPreload.h"
#include "TronInterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"

namespace
{
	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 30.0f;
	LTRect	stringRect(20,240,620,400);
	uint8	stringSize = 24;
	LTIntPt helpPos(160,440);
	uint8	helpSize = 12;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenEndMission::CScreenEndMission()
{
	m_pString = LTNULL;
}

CScreenEndMission::~CScreenEndMission()
{

}


// Build the screen
LTBOOL CScreenEndMission::Build()
{
	CreateTitle(IDS_TITLE_ENDMISSION);


	m_pString = AddTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenEndMission::OnFocus(LTBOOL bFocus)
{

	if (bFocus)
	{
		g_fDuration = 0.0f;

		int nMission = g_pMissionMgr->GetCurrentMission();
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission);

		if (m_pString && pMission)
		{
			m_pString->SetString(LoadTempString(pMission->nNameId));
		}
	}
	else
	{
		if (m_pString)
		{
			m_pString->SetString(" ");
		}
	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenEndMission::Escape()
{

	OnFocus(LTFALSE);

	CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
	if (pPreload)
	{
		pPreload->SetWaitingToExit(true);
	}
	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);

}

LTBOOL CScreenEndMission::HandleKeyDown(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenEndMission::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenEndMission::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenEndMission::Render(HSURFACE hDestSurf)
{
	CBaseScreen::Render(hDestSurf);
	g_fDuration += g_pGameClientShell->GetFrameTime();
	if (g_fDuration >= g_fDelay)
		Escape();
	return LTTRUE;

}




