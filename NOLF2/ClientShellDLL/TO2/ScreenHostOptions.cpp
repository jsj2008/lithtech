// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostOptions.cpp
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostOptions.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "ResShared.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostOptions::CScreenHostOptions()
{
	m_nMaxPlayers = 4;
	m_bUseSkills = LTTRUE;
	m_bFriendlyFire = LTFALSE;
	m_nDifficulty = 0;
	
	m_pMaxPlayers = NULL;
	m_pSkillToggle = NULL;

}


CScreenHostOptions::~CScreenHostOptions()
{
}

// Build the screen
LTBOOL CScreenHostOptions::Build()
{
	int kColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_OPTIONS,"ColumnWidth");
	int kSlider = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_OPTIONS,"SliderWidth");

	CreateTitle(IDS_TITLE_HOST_OPTIONS);

	m_pMaxPlayers = AddSlider(IDS_MAX_PLAYERS, IDS_MAX_PLAYERS_HELP, kColumn, kSlider, -1, &m_nMaxPlayers);
	m_pMaxPlayers->SetSliderRange(1, 4);
	m_pMaxPlayers->SetSliderIncrement(1);
	m_pMaxPlayers->SetNumericDisplay(LTTRUE);


	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	CLTGUIToggle* pToggle = AddToggle(IDS_FRIENDLY_FIRE,IDS_FRIENDLY_FIRE_HELP,kColumn,&m_bFriendlyFire);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	m_pSkillToggle = AddToggle(IDS_USE_SKILLS,IDS_HELP_USE_SKILLS,kColumn,&m_bUseSkills);
	m_pSkillToggle->SetOnString(szYes);
	m_pSkillToggle->SetOffString(szNo);

	CLTGUICycleCtrl *pCycle = AddCycle(IDS_DIFFICULTY,IDS_HELP_DIFFICULTY,kColumn,&m_nDifficulty);
	pCycle->AddString(LoadTempString(IDS_NEW_EASY));
	pCycle->AddString(LoadTempString(IDS_NEW_MEDIUM));
	pCycle->AddString(LoadTempString(IDS_NEW_HARD));
	pCycle->AddString(LoadTempString(IDS_NEW_INSANE));

	CLTGUISlider*	pSlider = AddSlider(IDS_HOST_PLAYERDIFF, IDS_HELP_HOST_PLAYERDIFF, kColumn, kSlider, -1, &m_nPlayerDiff);
	pSlider->SetSliderRange(0, 20);
	pSlider->SetSliderIncrement(2);
	

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

// Change in focus
void    CScreenHostOptions::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{

		m_nMaxPlayers = (int)pProfile->m_ServerGameOptions.GetCoop().m_nMaxPlayers;
		m_bUseSkills = pProfile->m_ServerGameOptions.GetCoop().m_bUseSkills;
		m_bFriendlyFire = pProfile->m_ServerGameOptions.GetCoop().m_bFriendlyFire;
		m_nDifficulty = pProfile->m_ServerGameOptions.GetCoop().m_nDifficulty;
		m_nPlayerDiff = (int)(100.0f * pProfile->m_ServerGameOptions.GetCoop().m_fPlayerDiffFactor);


		m_pMaxPlayers->Enable(!g_pPlayerMgr->IsPlayerInWorld());
		m_pSkillToggle->Enable(!g_pPlayerMgr->IsPlayerInWorld());

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		pProfile->m_ServerGameOptions.GetCoop().m_nMaxPlayers = (uint8)m_nMaxPlayers;
		pProfile->m_ServerGameOptions.GetCoop().m_bUseSkills = !!m_bUseSkills;
		pProfile->m_ServerGameOptions.GetCoop().m_bFriendlyFire = !!m_bFriendlyFire;
		pProfile->m_ServerGameOptions.GetCoop().m_nDifficulty = m_nDifficulty;
		pProfile->m_ServerGameOptions.GetCoop().m_fPlayerDiffFactor = (float)m_nPlayerDiff / 100.0f;

		pProfile->Save();

		if (g_pPlayerMgr->IsPlayerInWorld())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
			cMsg.Writebool( pProfile->m_ServerGameOptions.GetCoop().m_bFriendlyFire );
			cMsg.Writeuint8( pProfile->m_ServerGameOptions.GetCoop().m_nDifficulty = m_nDifficulty );
			cMsg.Writefloat( pProfile->m_ServerGameOptions.GetCoop().m_fPlayerDiffFactor );
		    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}

	}
	CBaseScreen::OnFocus(bFocus);
}

