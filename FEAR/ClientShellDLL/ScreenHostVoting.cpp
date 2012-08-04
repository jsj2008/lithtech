// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostVoting.cpp
//
// PURPOSE : Screen to set server options related to user voting
//
// CREATED : 12/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenHostVoting.h"
#include "GameModeMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostVoting::CScreenHostVoting()
{
}

CScreenHostVoting::~CScreenHostVoting()
{
}


// Build the screen
bool CScreenHostVoting::Build()
{

	CreateTitle("ScreenHost_Voting");
	uint32 kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	uint32 kColumn1 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	uint32 kColumn2 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,2);

	//allow vote kick
	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	tcs.nHeaderWidth = kColumn0;
	tcs.pCommandHandler = this;

	tcs.szHelpID = "ScreenVote_AllowKick_Help";
	tcs.pbValue = &m_bAllowVote[eVote_Kick];

	CLTGUIToggle *pAllowVote = AddToggle("ScreenVote_AllowKick",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));


	//allow vote team kick
	tcs.szHelpID = "ScreenVote_AllowTeamKick_Help";
	tcs.pbValue = &m_bAllowVote[eVote_TeamKick];

	pAllowVote = AddToggle("ScreenVote_AllowTeamKick",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));

	//allow vote ban
	tcs.szHelpID = "ScreenVote_AllowBan_Help";
	tcs.pbValue = &m_bAllowVote[eVote_Ban];

	pAllowVote = AddToggle("ScreenVote_AllowBan",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));

	//allow vote next round
	tcs.szHelpID = "ScreenVote_AllowNextRound_Help";
	tcs.pbValue = &m_bAllowVote[eVote_NextRound];

	pAllowVote = AddToggle("ScreenVote_AllowNextRound",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));

	//allow vote next map
	tcs.szHelpID = "ScreenVote_AllowNextMap_Help";
	tcs.pbValue = &m_bAllowVote[eVote_NextMap];

	pAllowVote = AddToggle("ScreenVote_AllowNextMap",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));

	//allow vote select map
	tcs.szHelpID = "ScreenVote_AllowSelectMap_Help";
	tcs.pbValue = &m_bAllowVote[eVote_SelectMap];

	pAllowVote = AddToggle("ScreenVote_AllowSelectMap",tcs );
	pAllowVote->SetOnString(LoadString("IDS_YES"));
	pAllowVote->SetOffString(LoadString("IDS_NO"));

	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kColumn0+kColumn1,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kColumn0;
	scs.szHelpID = "ScreenVote_MinPlayers_Help";
	scs.pnValue = &m_nMinPlayersForVote;
	scs.nMin = 3;
	scs.nMax = 8;
	scs.nIncrement = 1;
	scs.bNumericDisplay = true;
	AddSlider("ScreenVote_MinPlayers", scs );

	scs.szHelpID = "ScreenVote_MinPlayersTeam_Help";
	scs.pnValue = &m_nMinPlayersForTeamVote;
	scs.nMin = 3;
	scs.nMax = 8;
	scs.nIncrement = 1;
	scs.bNumericDisplay = true;
	AddSlider("ScreenVote_MinPlayersTeam", scs );

	scs.szHelpID = "ScreenVote_VoteLifetime_Help";
	scs.pnValue = &m_nVoteLifetime;
	scs.nMin = 15;
	scs.nMax = 60;
	scs.nIncrement = 5;
	scs.bNumericDisplay = true;
	AddSlider("ScreenVote_VoteLifetime", scs );

	scs.szHelpID = "ScreenVote_BanDuration_Help";
	scs.pnValue = &m_nVoteBanDuration;
	scs.nMin = 30;
	scs.nMax = 120;
	scs.nIncrement = 10;
	scs.bNumericDisplay = true;
	AddSlider("ScreenVote_BanDuration", scs );

	scs.szHelpID = "ScreenVote_VoteDelay_Help";
	scs.pnValue = &m_nVoteDelay;
	scs.nMin = 0;
	scs.nMax = 120;
	scs.nIncrement = 15;
	scs.bNumericDisplay = true;
	AddSlider("ScreenVote_VoteDelay", scs );


	// Make sure to call the base class
	return CBaseScreen::Build();
}


// Change in focus
void CScreenHostVoting::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		for (uint8 n = 0; n < kNumVoteTypes; ++n)
		{
			m_bAllowVote[n] = GameModeMgr::Instance( ).m_ServerSettings.m_bAllowVote[n];
		}
		
		m_nMinPlayersForVote = (int)GameModeMgr::Instance( ).m_ServerSettings.m_nMinPlayersForVote;
		m_nMinPlayersForTeamVote = (int)GameModeMgr::Instance( ).m_ServerSettings.m_nMinPlayersForTeamVote;
		m_nVoteLifetime = (int)GameModeMgr::Instance( ).m_ServerSettings.m_nVoteLifetime;
		m_nVoteBanDuration = (int)GameModeMgr::Instance( ).m_ServerSettings.m_nVoteBanDuration;
		m_nVoteDelay = (int)GameModeMgr::Instance( ).m_ServerSettings.m_nVoteDelay;

		UpdateData(false);
	}
	else
	{
		UpdateData();

		for (uint8 n = 0; n < kNumVoteTypes; ++n)
		{
			GameModeMgr::Instance( ).m_ServerSettings.m_bAllowVote[n] = m_bAllowVote[n];
		}
		GameModeMgr::Instance( ).m_ServerSettings.m_nMinPlayersForVote = (uint8)m_nMinPlayersForVote;
		GameModeMgr::Instance( ).m_ServerSettings.m_nMinPlayersForTeamVote = (uint8)m_nMinPlayersForTeamVote;
		GameModeMgr::Instance( ).m_ServerSettings.m_nVoteLifetime = (uint8)m_nVoteLifetime;
		GameModeMgr::Instance( ).m_ServerSettings.m_nVoteBanDuration = (uint8)m_nVoteBanDuration;
		GameModeMgr::Instance( ).m_ServerSettings.m_nVoteDelay = (uint8)m_nVoteDelay;

		GameModeMgr::Instance().WriteToOptionsFile( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ));

	}
	CBaseScreen::OnFocus(bFocus);

}
