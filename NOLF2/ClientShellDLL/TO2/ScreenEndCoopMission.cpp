// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndCoopMission.cpp
//
// PURPOSE : Interface screen for handling end of mission in a co-op game
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenEndCoopMission.h"
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
	uint8  statSize = 12;
	uint16 NameWidth = 150;
	uint16 AccuracyWidth = 100;
	uint16 KillsWidth = 75;
	uint16 DeathsWidth = 75;

	bool s_bFlash;
	float s_fFlashTime;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenEndCoopMission::CScreenEndCoopMission()
{
	m_pMission = NULL;
	m_pList = NULL;
}

CScreenEndCoopMission::~CScreenEndCoopMission()
{

}


// Build the screen
LTBOOL CScreenEndCoopMission::Build()
{
	CreateTitle(IDS_TITLE_ENDMISSION);

	uint8 statSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"StatSize");
	if (!statSize) statSize = 12;

	uint16 NameWidth = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"NameWidth");
	if (!NameWidth) NameWidth = 150;

	uint16 AccuracyWidth = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"AccuracyWidth");
	if (!AccuracyWidth) AccuracyWidth = 100;

	uint16 KillsWidth = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"KillsWidth");
	if (!KillsWidth) KillsWidth = 75;

	uint16 DeathsWidth = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_END_MISSION,"DeathsWidth");
	if (!DeathsWidth) DeathsWidth = 75;

	m_pMission = AddTextItem(" ",LTNULL,LTNULL,kDefaultPos,LTTRUE);
	m_nextPos.y += 8;

	m_pList = AddList(m_nextPos,(GetPageBottom() - m_nextPos.y));

	LTIntPt pos(GetPageLeft(),GetPageBottom());
	m_pContinue = AddTextItem(LoadTempString(IDS_PRESS_ANY_KEY),NULL,NULL,pos,LTTRUE);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenEndCoopMission::OnFocus(LTBOOL bFocus)
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

		m_pList->RemoveAll();

		CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();

		CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(NULL,NULL,kDefaultPos,LTTRUE);

		pCtrl->AddColumn(" ", (NameWidth-16));
		pCtrl->AddColumn(LoadTempString(IDS_COOP_STAT_ACCURACY), AccuracyWidth);
		pCtrl->AddColumn(LoadTempString(IDS_COOP_STAT_KILLS), KillsWidth);
		pCtrl->AddColumn(LoadTempString(IDS_COOP_STAT_DEATHS), DeathsWidth);

		m_pList->AddControl(pCtrl);


		CLIENT_INFO* pInfo = pCIMgr->GetFirstClient();
		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID (&nLocalID);

		char szTmp[64];
		while (pInfo)
		{
			CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(NULL,NULL,kDefaultPos,LTTRUE);
			pCtrl->SetFont(LTNULL,statSize);

			m_pList->AddControl(pCtrl);

			pCtrl->AddColumn(pInfo->sName.c_str(), NameWidth);

		
			const MissionStats *pStats = NULL;
			

			if (pInfo->nID == nLocalID)
			{
				pStats = g_pPlayerStats->GetMissionStats();
				pCtrl->SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
			}
			else
			{
				pStats = &pInfo->sStats;
			}

			if (pStats)
			{
				
				if (pStats->dwNumShotsFired)
				{
					float fAccuracy = 100.0f * (float)pStats->dwNumHits / (float) pStats->dwNumShotsFired;
					sprintf(szTmp,"%0.1f%",fAccuracy);
				}
				else
				{
					strcpy(szTmp," - ");
				}

				pCtrl->AddColumn(szTmp, AccuracyWidth);
			

				sprintf(szTmp,"%d", pStats->dwNumEnemyKills);
				pCtrl->AddColumn(szTmp, KillsWidth);

				sprintf(szTmp,"%d", pStats->dwNumTimesKilled);
				pCtrl->AddColumn(szTmp, DeathsWidth);

					
			}
			pInfo = pInfo->pNext;

		}

		m_pList->SetStartIndex(0);

		s_bFlash = false;
		s_fFlashTime = g_pLTClient->GetTime() + 0.333f;
		m_pContinue->Show(LTFALSE);

	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenEndCoopMission::Escape()
{

	OnFocus(LTFALSE);

	if (g_pMissionMgr->IsGameOver())
	{
		g_pMissionMgr->ClearGameOver();
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MULTI);
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

LTBOOL CScreenEndCoopMission::HandleKeyDown(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenEndCoopMission::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenEndCoopMission::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenEndCoopMission::Render(HSURFACE hDestSurf)
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
	if (g_fDuration >= g_fDelay)
		Escape();
	return LTTRUE;

}




