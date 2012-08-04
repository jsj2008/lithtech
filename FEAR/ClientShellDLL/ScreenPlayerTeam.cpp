// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerTeam.cpp
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayerTeam.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientConnectionMgr.h"
#include "TeamMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int CMD_TEAM1 = (CMD_CUSTOM+1);
	const int CMD_TEAM2 = (CMD_CUSTOM+2);
	#define INVALID_ANI			((HMODELANIM)-1)

	uint32 nTeamColors[2] = {argbBlack,argbBlack};
	uint32 nListFontSize = 14;
	uint16 nListWidth = 100;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayerTeam::CScreenPlayerTeam()
{
	memset(m_pTeams,0,sizeof(m_pTeams));
	memset(m_pCounts,0,sizeof(m_pCounts));
	memset(m_pScores,0,sizeof(m_pScores));
	memset(m_pPlayers,0,sizeof(m_pPlayers));
}

CScreenPlayerTeam::~CScreenPlayerTeam()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayerTeam::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayerTeam::Term()
{
	CBaseScreen::Term();
}

// Build the screen
bool CScreenPlayerTeam::Build()
{
	m_UpdateTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_AutoSelectTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	CreateTitle("IDS_TITLE_PLAYER_TEAM");

	nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	nListWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);

	LTRect2n rTeam[2];

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		rTeam[nTeam] = g_pLayoutDB->GetListRect(m_hLayout,nTeam);

		nTeamColors[nTeam] = m_SelectedColor; //g_pLayoutDB->GetTeamColor(nTeam);

	};

	LTVector2n pos(rTeam[0].Left()+4,rTeam[0].Top()+2);

	CLTGUICtrl_create cs;

	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_TEAM1;
	cs.szHelpID = "IDS_HELP_CHOOSE_TEAM_1";
	m_pTeams[0] = AddTextItem( L"<team 0>", cs);
	pos.y += 22;
	
	uint16 nHeight = (rTeam[0].Bottom()-pos.y) - 8;

	CLTGUIListCtrl_create lcs;
	lcs.rnBaseRect.m_vMin = pos;
	lcs.rnBaseRect.m_vMax = pos + LTVector2n(nListWidth,nHeight);
	lcs.bArrows = false;
	m_pPlayers[0] = AddList(lcs);
	if (m_pPlayers[0])
	{
		m_pPlayers[0]->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
		m_pPlayers[0]->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	}

	pos = LTVector2n(lcs.rnBaseRect.m_vMax.x+8,lcs.rnBaseRect.m_vMin.y+4);
	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pCounts[0] = AddTextItem( L"<team 0>", cs, true);
	pos.y += g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pScores[0] = AddTextItem( L"<team 0>", cs, true);


	pos= LTVector2n(rTeam[1].Left()+4,rTeam[1].Top()+2);
	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_TEAM2;
	cs.szHelpID = "IDS_HELP_CHOOSE_TEAM_2";
	m_pTeams[1] = AddTextItem( L"<team 1>", cs);
	pos.y += 22;
	
	nHeight = (rTeam[1].Bottom() - pos.y) - 8;
	lcs.rnBaseRect.m_vMin = pos;
	lcs.rnBaseRect.m_vMax = pos + LTVector2n(nListWidth,nHeight);
	lcs.bArrows = false;
	m_pPlayers[1] = AddList(lcs);
	if (m_pPlayers[1])
	{
		m_pPlayers[1]->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,1));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,1));
		m_pPlayers[1]->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,1));
	}

	pos = LTVector2n(lcs.rnBaseRect.m_vMax.x+8,lcs.rnBaseRect.m_vMin.y+4);
	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[1].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pCounts[1] = AddTextItem( L"<team 1>", cs, true);
	pos.y += g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pScores[1] = AddTextItem( L"<team 1>", cs, true);

	pos = m_ScreenRect.GetBottomLeft();
	pos.y -= g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos+LTVector2n(rTeam[0].GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_LAUNCH;
	cs.szHelpID = "IDS_HELP_AUTO_SELECT";
	m_pAuto = AddTextItem("IDS_AUTO_SELECT",cs);

 	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;
	UseBack(false);
	return true;

}

uint32 CScreenPlayerTeam::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_TEAM1:
		m_nTeam = 0;
		Escape();
		break;
	case CMD_TEAM2:
		m_nTeam = 1;
		Escape();
		break;
	case CMD_LAUNCH:
		m_nTeam = INVALID_TEAM;
		Escape();
		break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenPlayerTeam::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_nTeam = INVALID_TEAM;

		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( pLocalCI )
			SelectTeam(pLocalCI->nTeamID);

		m_pAuto->Show(!g_pClientConnectionMgr->HasSelectedTeam());

		for (uint8 nTeam = 0; nTeam < 2; nTeam++)
		{
			CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeam);
			if (!pTeam) continue;

			m_pTeams[nTeam]->SetString(pTeam->GetName());
		}

		UpdateTeam();


		m_AutoSelectTimer.Start(30.0f);

		UpdateData(false);
	}
	else
	{
		UpdateData();

	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenPlayerTeam::Escape()
{
	if (!g_pClientConnectionMgr->HasSelectedTeam())
	{
		g_pClientConnectionMgr->SelectTeam(m_nTeam,true);
		g_pInterfaceMgr->UpdatePostLoad();
	}
	else
	{
		g_pClientConnectionMgr->SelectTeam(m_nTeam,true);
		g_pClientConnectionMgr->SendMultiPlayerInfo();

		CBaseScreen::Escape();
	}
}



bool CScreenPlayerTeam::UpdateInterfaceSFX()
{
	if (m_AutoSelectTimer.IsTimedOut())
	{
		Escape();
		return true;
	}

	if (m_UpdateTimer.IsTimedOut())
		UpdateTeam();

	return CBaseScreen::UpdateInterfaceSFX();
}



void CScreenPlayerTeam::SelectTeam(uint8 nTeam)
{
	if( nTeam != INVALID_TEAM )
		SetSelection(GetIndex(m_pTeams[nTeam]));
	else
		SetSelection(kNoSelection);
}

void CScreenPlayerTeam::UpdateTeam()
{
	wchar_t szTemp[128];
	uint8 nPlayers[2] = {0,0};

	m_UpdateTimer.Start(3.0f);

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO* pCI = pCIMgr->GetFirstClient();
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);

	CLTGUICtrl_create cs;
	CLTGUITextCtrl* pItem;

	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nListWidth,nListFontSize);

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		m_pPlayers[nTeam]->RemoveAll();
	}
	
	while (pCI)
	{
		if (pCI->nTeamID != INVALID_TEAM)
		{
			LTStrCpy(szTemp,pCI->sName.c_str(),LTARRAYSIZE(szTemp));

			uint32 nColor = nTeamColors[pCI->nTeamID];




			pItem = CreateTextItem(szTemp,cs,true,g_pLayoutDB->GetListFont(m_hLayout,pCI->nTeamID),nListFontSize);
			pItem->SetColors(nColor,nColor,nColor);
//			pItem->SetFixedWidth( SetFixedWidth(nListWidth,true);
			m_pPlayers[pCI->nTeamID]->AddControl(pItem);
			nPlayers[pCI->nTeamID]++;
		}
		
		pCI = pCI->pNext;
	}

	for (nTeam = 0; nTeam < 2; nTeam++)
	{
		CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeam);
		if (!pTeam) continue;

		LTSNPrintF(szTemp, LTARRAYSIZE(szTemp), L"%s %d", LoadString("IDS_SCORE_PLAYERS"), nPlayers[nTeam]);
		m_pCounts[nTeam]->SetString(szTemp);
		LTSNPrintF(szTemp, LTARRAYSIZE(szTemp), L"%s %d", LoadString("IDS_SCORE_SCORE"), pTeam->GetScore());
		m_pScores[nTeam]->SetString(szTemp);
	};



}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayerTeam::CreateInterfaceSFX
//
//	PURPOSE:	Create the SFX to render on this screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayerTeam::CreateInterfaceSFX()
{
	CBaseScreen::CreateInterfaceSFX();

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
//		CreateTeamFX(nTeam);
	}
}

/*
void CScreenPlayerTeam::CreateTeamFX(uint8 nTeam)
{
}
*/


void CScreenPlayerTeam::RemoveInterfaceSFX()
{
/*
	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_TeamSFX[nTeam]);
		
		m_TeamSFX[nTeam].Reset();
		m_TeamSFX[nTeam].Term();

		ClearTeamAttachFX(nTeam);
	}
*/
	CBaseScreen::RemoveInterfaceSFX();
}


bool CScreenPlayerTeam::OnMouseMove(int x, int y)
{
	if (m_pPlayers[0]->IsOnMe(x,y))
	{
		if (GetSelectedControl() != m_pTeams[0])
		{
			SelectTeam(0);
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
			return true;
		}
	} 
	else if (m_pPlayers[1]->IsOnMe(x,y))
	{
		if (GetSelectedControl() != m_pTeams[1])
		{
			SelectTeam(1);
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
			return true;
		}
	}
	else
	{
		bool bRet =  CBaseScreen::OnMouseMove(x,y);
		
		// KLS 02/01/05 - CBaseScreen::OnMouseMove may select the Team 1 / Team 2
		// or Auto Select text so we need to check the return value...
		if (!bRet)
		{
			SelectTeam(INVALID_TEAM);
		} 
		return bRet;
	}

	return false;

}

bool CScreenPlayerTeam::OnUp()
{
	bool bHandled = CBaseScreen::OnUp();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pTeams[0])
			SelectTeam(0);
		else  if (GetSelectedControl() == m_pTeams[1])
			SelectTeam(1);
		else
			SelectTeam(INVALID_TEAM);
	}
	return bHandled;
}

bool CScreenPlayerTeam::OnDown()
{
	bool bHandled = CBaseScreen::OnDown();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pTeams[0])
			SelectTeam(0);
		else  if (GetSelectedControl() == m_pTeams[1])
			SelectTeam(1);
		else
			SelectTeam(INVALID_TEAM);
	}
	return bHandled;
}


/******************************************************************/
bool CScreenPlayerTeam::OnLButtonDown(int x, int y)
{
	if (m_pPlayers[0]->IsOnMe(x,y))
	{
		SelectTeam(0);
		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.

		m_nLMouseDownItemSel=GetIndex(m_pTeams[0]);

		return m_pTeams[0]->OnLButtonDown(x,y);
	} 
	else if (m_pPlayers[1]->IsOnMe(x,y))
	{
		SelectTeam(1);
		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.

		m_nLMouseDownItemSel=GetIndex(m_pTeams[1]);

		return m_pTeams[1]->OnLButtonDown(x,y);
	}

	SelectTeam(INVALID_TEAM);
	return CBaseScreen::OnLButtonDown( x, y);
}

/******************************************************************/
bool CScreenPlayerTeam::OnLButtonUp(int x, int y)
{
	if (m_pPlayers[0]->IsOnMe(x,y) && m_nLMouseDownItemSel == GetIndex(m_pTeams[0]))
	{
		bool bHandled = m_pTeams[0]->OnLButtonUp(x,y);
		if (bHandled)
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		return bHandled;
	} 
	else if (m_pPlayers[1]->IsOnMe(x,y) && m_nLMouseDownItemSel == GetIndex(m_pTeams[1]))
	{
		bool bHandled = m_pTeams[1]->OnLButtonUp(x,y);
		if (bHandled)
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		return bHandled;
	}
	return CBaseScreen::OnLButtonUp( x, y);
}

