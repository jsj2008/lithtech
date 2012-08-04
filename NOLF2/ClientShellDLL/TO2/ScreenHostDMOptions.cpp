// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDMOptions.cpp
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostDMOptions.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "ResShared.h"

namespace
{
	const int kMaxScoreLimit = 100;
	const int kMaxTimeLimit = 60;
	const int kMaxRunSpeed = 150;
	const int kMaxRounds = 20;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostDMOptions::CScreenHostDMOptions()
{
	m_nMaxPlayers = 8;
	m_nRunSpeed = 130;
	m_nScoreLimit = 25;
	m_nTimeLimit = 10;
	m_nRounds = 1;

	m_nFragScore = 2;
	m_nTagScore = 1;

	m_pMaxPlayers = NULL;

}


CScreenHostDMOptions::~CScreenHostDMOptions()
{
}

// Build the screen
LTBOOL CScreenHostDMOptions::Build()
{
	int kColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_DM_OPTIONS,"ColumnWidth");
	int kSlider = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_DM_OPTIONS,"SliderWidth");

	CreateTitle(IDS_TITLE_HOST_OPTIONS);

	m_pMaxPlayers = AddSlider(IDS_MAX_PLAYERS, IDS_MAX_PLAYERS_HELP, kColumn, kSlider, -1, &m_nMaxPlayers);
	m_pMaxPlayers->SetSliderRange(2, 16);
	m_pMaxPlayers->SetSliderIncrement(1);
	m_pMaxPlayers->SetNumericDisplay(LTTRUE);


	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	CLTGUISlider*	pSlider = AddSlider(IDS_RUN_SPEED, IDS_RUN_SPEED_HELP, kColumn, kSlider, -1, &m_nRunSpeed);
	pSlider->SetSliderRange(100, kMaxRunSpeed);
	pSlider->SetSliderIncrement(10);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider(IDS_FRAG_LIMIT, IDS_FRAG_LIMIT_HELP, kColumn, kSlider, -1, &m_nScoreLimit);
	pSlider->SetSliderRange(0,kMaxScoreLimit);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider(IDS_TIME_LIMIT, IDS_TIME_LIMIT_HELP, kColumn, kSlider, -1, &m_nTimeLimit);
	pSlider->SetSliderRange(0,kMaxTimeLimit);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider( IDS_ROUNDS, IDS_ROUNDS_HELP, kColumn, kSlider, -1, &m_nRounds );
	pSlider->SetSliderRange( 1, kMaxRounds );
	pSlider->SetSliderIncrement( 1 );
	pSlider->SetNumericDisplay( LTTRUE );

	pSlider = AddSlider(IDS_FRAG_SCORE, IDS_FRAG_SCORE_HELP, kColumn, kSlider, -1, &m_nFragScore);
	pSlider->SetSliderRange(0,3);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider(IDS_TAG_SCORE, IDS_TAG_SCORE_HELP, kColumn, kSlider, -1, &m_nTagScore);
	pSlider->SetSliderRange(0,3);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE);


 	// Make sure to call the base class
	return CBaseScreen::Build();
}

// Change in focus
void    CScreenHostDMOptions::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{

		m_nMaxPlayers = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nMaxPlayers;
		m_nRunSpeed = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nRunSpeed;
		m_nScoreLimit = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nScoreLimit;
		m_nTimeLimit = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nTimeLimit;
		m_nRounds = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nRounds;

		m_nFragScore = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nFragScore;
		m_nTagScore = (int)pProfile->m_ServerGameOptions.GetDeathmatch().m_nTagScore;
		
		m_pMaxPlayers->Enable(!g_pPlayerMgr->IsPlayerInWorld());

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		pProfile->m_ServerGameOptions.GetDeathmatch().m_nMaxPlayers = (uint8)m_nMaxPlayers;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nRunSpeed = (uint8)m_nRunSpeed;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nScoreLimit = (uint8)m_nScoreLimit;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nTimeLimit = (uint8)m_nTimeLimit;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nRounds = (uint8)m_nRounds;

		pProfile->m_ServerGameOptions.GetDeathmatch().m_nFragScore = (uint8)m_nFragScore;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nTagScore = (uint8)m_nTagScore;
		
		pProfile->Save();

		if (g_pPlayerMgr->IsPlayerInWorld())
		{

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
			pProfile->m_ServerGameOptions.GetDeathmatch().Write(cMsg);
		    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}

	}
	CBaseScreen::OnFocus(bFocus);
}

