// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenVote.cpp
//
// PURPOSE : Interface screen for player voting
//
// CREATED : 12/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenVote.h"
#include "ScreenCommands.h"
#include "SharedVoting.h"
#include "GameModeMgr.h"
#include "ClientVoteMgr.h"
#include "MissionMgr.h"

namespace
{
	uint32 nListFontSize = 14;
	uint16 nListWidth = 100;

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenVote::CScreenVote()
{
};

CScreenVote::~CScreenVote()
{
	m_pVoteKick = NULL;
	m_pVoteTeamKick = NULL;
	m_pVoteBan = NULL;
	m_pPlayers = NULL;
	m_pVoteNextRound = NULL;
	m_pVoteNextMap = NULL;
	m_pVoteMap = NULL;
	m_pMaps = NULL;
}

// Build the screen
bool CScreenVote::Build()
{
	CreateTitle("Menu_CallVote");


	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.pCommandHandler = this;

	cs.nCommandID = CMD_CUSTOM + eVote_Kick;
	cs.szHelpID = "ScreenVote_Kick_Help";
	m_pVoteKick = AddTextItem("ScreenVote_Kick", cs );

	cs.nCommandID = CMD_CUSTOM + eVote_TeamKick;
	cs.szHelpID = "ScreenVote_TeamKick_Help";
	m_pVoteTeamKick = AddTextItem("ScreenVote_TeamKick", cs );

	cs.nCommandID = CMD_CUSTOM + eVote_Ban;
	cs.szHelpID = "ScreenVote_Ban_Help";
	m_pVoteBan = AddTextItem("ScreenVote_Ban", cs );

	cs.nCommandID = CMD_CUSTOM + eVote_NextRound;
	cs.szHelpID = "ScreenVote_NextRound_Help";
	m_pVoteNextRound = AddTextItem("ScreenVote_NextRound", cs );

	cs.nCommandID = CMD_CUSTOM + eVote_NextMap;
	cs.szHelpID = "ScreenVote_NextMap_Help";
	m_pVoteNextMap = AddTextItem("ScreenVote_NextMap", cs );

	cs.nCommandID = CMD_CUSTOM + eVote_SelectMap;
	cs.szHelpID = "ScreenVote_SelectMap_Help";
	m_pVoteMap = AddTextItem("ScreenVote_SelectMap", cs );

	nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	nListWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	LTRect2n rList;
	rList = g_pLayoutDB->GetListRect(m_hLayout,0);


	CLTGUIListCtrl_create lcs;
	lcs.rnBaseRect.m_vMin = rList.GetTopLeft();
	lcs.rnBaseRect.m_vMax = rList.GetBottomRight();
	lcs.bArrows = false;
	m_pPlayers = AddList(lcs);
	if (m_pPlayers)
	{
		m_pPlayers->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
		m_pPlayers->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	}

	m_pMaps = AddList(lcs);
	if (m_pMaps)
	{
		m_pMaps->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
		m_pMaps->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	}



	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenVote::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM )
	{
		m_eVoteType = (VoteType)(dwCommand-CMD_CUSTOM);
		m_pMaps->Show(false);
		m_pPlayers->Show(false);
		switch(m_eVoteType)
		{
		case eVote_SelectMap:
			m_pMaps->Show(true);
			break;
		case eVote_Kick:
		case eVote_TeamKick:
		case eVote_Ban:
			UpdatePlayerList();
			m_pPlayers->Show(true);
			break;
		case eVote_NextMap:
		case eVote_NextRound:
			ClientVoteMgr::Instance().CallVoteNext(m_eVoteType);
			Escape();
			break;
		};


		return 1;
	}
	if ( dwCommand == CMD_CONFIRM)
	{
		m_pPlayers->Show(false);
		m_pMaps->Show(false);
		switch(m_eVoteType)
		{
		case eVote_SelectMap:
			ClientVoteMgr::Instance().CallVoteMap( dwParam1);
			Escape();
			break;
		case eVote_Kick:
		case eVote_TeamKick:
		case eVote_Ban:
			ClientVoteMgr::Instance().CallVoteKick(m_eVoteType,dwParam1);
			Escape();
			break;
		};

	}

	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

}

// Change in focus
void CScreenVote::OnFocus(bool bFocus)
{
	m_pVoteKick->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_Kick]);
	m_pVoteTeamKick->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_TeamKick] && GameModeMgr::Instance().m_grbUseTeams);
	m_pVoteBan->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_Ban]);
	m_pPlayers->Show(false);

	m_pVoteNextRound->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_NextRound]);
	m_pVoteNextMap->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_NextMap]);
	m_pVoteMap->Enable(GameModeMgr::Instance().m_ServerSettings.m_bAllowVote[eVote_SelectMap]);

	m_pMaps->Show(false);

	CBaseScreen::OnFocus(bFocus);
}

void CScreenVote::Escape()
{
	if (m_pPlayers->IsVisible() || m_pMaps->IsVisible())
	{
		m_pPlayers->Show(false);
		m_pMaps->Show(false);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else
	{
		CBaseScreen::Escape();
	}
}


void CScreenVote::UpdatePlayerList()
{
	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO* pCI = pCIMgr->GetFirstClient();
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);

	CLTGUICtrl_create cs;
	CLTGUITextCtrl* pItem = NULL;
	wchar_t szTemp[128] = L"";

	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nListWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));

	m_pPlayers->RemoveAll();

	switch(m_eVoteType)
	{
	case eVote_Kick:
		pItem = CreateTextItem("ScreenVote_Kick",cs,true,g_pLayoutDB->GetListFont(m_hLayout,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));
		m_pPlayers->AddControl(pItem);
		break;
	case eVote_TeamKick:
		pItem = CreateTextItem("ScreenVote_TeamKick",cs,true,g_pLayoutDB->GetListFont(m_hLayout,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));
		m_pPlayers->AddControl(pItem);
		break;
	case eVote_Ban:
		pItem = CreateTextItem("ScreenVote_Ban",cs,true,g_pLayoutDB->GetListFont(m_hLayout,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));
		m_pPlayers->AddControl(pItem);
		break;
	};

	cs.rnBaseRect.m_vMax = LTVector2n(nListWidth,nListFontSize);


	cs.pCommandHandler = this;
	cs.nCommandID = CMD_CONFIRM;

	bool bTeamFilter = GameModeMgr::Instance().m_grbUseTeams && (m_eVoteType == eVote_TeamKick);

	while (pCI)
	{
		if (!bTeamFilter || (pCI->nTeamID == pCIMgr->GetLocalTeam()) )
		{
			LTStrCpy(szTemp,pCI->sName.c_str(),LTARRAYSIZE(szTemp));

			
			cs.nParam1 = pCI->nID;
			pItem = CreateTextItem(szTemp,cs,false,g_pLayoutDB->GetListFont(m_hLayout,0),nListFontSize);

			pItem->Enable(!pCI->bIsAdmin);

			m_pPlayers->AddControl(pItem);


		}

		pCI = pCI->pNext;
	}


}


void CScreenVote::UpdateMapList( )
{
	CLTGUICtrl_create cs;
	CLTGUITextCtrl* pItem = NULL;

	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nListWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));

	m_pMaps->RemoveAll();

	pItem = CreateTextItem("ScreenVote_SelectMap",cs,true,g_pLayoutDB->GetListFont(m_hLayout,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));
	m_pMaps->AddControl(pItem);

	cs.rnBaseRect.m_vMax = LTVector2n(nListWidth,nListFontSize);

	cs.pCommandHandler = this;
	cs.nCommandID = CMD_CONFIRM;

	for (uint32 nIndex = 0; nIndex < g_pMissionMgr->GetMapList().size(); ++nIndex)
	{
		cs.nParam1 = nIndex;
		pItem = CreateTextItem( g_pMissionMgr->GetMapList()[nIndex].c_str(),cs,false,g_pLayoutDB->GetListFont(m_hLayout,0),nListFontSize);
		m_pMaps->AddControl(pItem);
	}


}