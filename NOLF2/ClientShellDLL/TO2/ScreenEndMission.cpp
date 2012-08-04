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
#include "TO2InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"

namespace
{
	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 30.0f;

	bool s_bFlash;
	float s_fFlashTime;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenEndMission::CScreenEndMission()
{
	m_pMission = NULL;
	m_pAccuracy = NULL;
	m_pMarksmanship = NULL;
	m_pNonInjury = NULL;
	m_pTimesHit = NULL;
	m_pList = NULL;
}

CScreenEndMission::~CScreenEndMission()
{

}


// Build the screen
LTBOOL CScreenEndMission::Build()
{
	CreateTitle(IDS_TITLE_ENDMISSION);

	uint8 statSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"StatSize");
	if (!statSize) statSize = 12;

	m_pMission = AddTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_nextPos.y += 8;

	m_pList = AddList(m_nextPos,(GetPageBottom() - m_nextPos.y));

	m_pAccuracy = CreateTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pAccuracy->SetFont(NULL,statSize);
	m_pList->AddControl(m_pAccuracy);

	m_pMarksmanship = CreateTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pMarksmanship->SetFont(NULL,statSize);
	m_pList->AddControl(m_pMarksmanship);

	m_pNonInjury = CreateTextItem(IDS_M_STAT_UNDAMAGED,LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pNonInjury->SetFont(NULL,statSize);
	m_pList->AddControl(m_pNonInjury);

	m_pTimesHit = CreateTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_pTimesHit->SetFont(NULL,statSize);
	m_pList->AddControl(m_pTimesHit);


	LTIntPt pos(GetPageLeft(),GetPageBottom());
	m_pContinue = AddTextItem(LoadTempString(IDS_PRESS_ANY_KEY),NULL,NULL,pos,LTTRUE);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenEndMission::OnFocus(LTBOOL bFocus)
{

	if (bFocus)
	{
		//the quickload game confirmation dialog might be up at this point
		// close it here because loading at this point gets us into an undefined (i.e. bad) state
		g_pInterfaceMgr->CloseMessageBox(LTFALSE);

		g_fDuration = 0.0f;
		char szString[256] = "";

		int nMission = g_pMissionMgr->GetCurrentMission();
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission);

		if (pMission)
		{
			sprintf(szString,"%s ", LoadTempString(IDS_STAT_MISSION));
			strcat(szString, LoadTempString(pMission->nNameId));
			m_pMission->SetString(szString);
		}
		else
		{
			m_pMission->SetString(" ");
		}

		const MissionStats *pStats = g_pPlayerStats->GetMissionStats();

		if (pStats)
		{
			if (pStats->dwNumShotsFired)
			{
				
				float fAccuracy = 100.0f * (float)pStats->dwNumHits / (float) pStats->dwNumShotsFired;
				FormatString(IDS_M_STAT_ACCURACY,szString,sizeof(szString),pStats->dwNumHits,pStats->dwNumShotsFired,(int)fAccuracy);
			}
			else
			{
				FormatString(IDS_M_STAT_ACCURACY,szString,sizeof(szString),0,0,0);
			}
			m_pAccuracy->SetString(szString);

			m_pMarksmanship->SetString(FormatTempString(IDS_M_STAT_MARKSMANSHIP,pStats->dwHitLocations[HL_HEAD]));

			m_pNonInjury->Show(g_pPlayerStats->GetMissionDamage() == 0);

			m_pTimesHit->SetString(FormatTempString(IDS_M_STAT_TIMES_HIT,pStats->dwNumTimesHit));

			m_pList->SetStartIndex(0);
				
		}

		s_bFlash = false;
		s_fFlashTime = g_pLTClient->GetTime() + 0.333f;
		m_pContinue->Show(LTFALSE);

	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenEndMission::Escape()
{

	OnFocus(LTFALSE);

	if (g_pMissionMgr->IsGameOver())
	{
		g_pMissionMgr->ClearGameOver();

#ifdef _TO2DEMO
		g_pInterfaceMgr->ShowDemoScreens(LTFALSE);
#else
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
#endif

	}
	else
	{
		CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
		if (pPreload)
		{
			pPreload->SetWaitingToExit(true);
		}
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
	}

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

	float fTime = g_pLTClient->GetTime();
	if (fTime > s_fFlashTime)
	{
		s_bFlash = !s_bFlash;
		s_fFlashTime = fTime + 0.333f;
		if (s_bFlash)
		{
			m_pContinue->SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
		}
		else
		{
			m_pContinue->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		}

	}

	g_fDuration += g_pGameClientShell->GetFrameTime();

	if (g_fDuration > g_fMinDelay && !m_pContinue->IsVisible())
	{
		m_pContinue->Show(LTTRUE);
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\pressanykey.wav");
	}


	// [KLS 9/13/02] Only auto switch in Multiplayer...
	if (g_fDuration >= g_fDelay && IsMultiplayerGame())
	{
		Escape();
	}

	return LTTRUE;

}




