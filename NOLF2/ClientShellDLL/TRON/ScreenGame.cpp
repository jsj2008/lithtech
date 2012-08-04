// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenGame.cpp
//
// PURPOSE : Interface screen for setting gameplay options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenGame.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
#include "GameSettings.h"


namespace
{
	int kGap = 0;
	int kWidth = 0;
}

//extern VarTrack	g_vtSubtitles;
VarTrack	g_vtSubtitles;
extern VarTrack g_vtHUDLayout;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenGame::CScreenGame()
{
	m_nDifficulty = 2;
    m_nSubtitles  = 0;
    m_bGore = LTFALSE;
	m_bAlwaysRun = LTTRUE;
	m_nLayout = 0;
	m_nHeadBob = 0;
	m_nWeaponSway = 0;
	m_nPickupMsgDur = 0;
	m_bObjMessages = LTTRUE;

}

CScreenGame::~CScreenGame()
{

}

// Build the screen
LTBOOL CScreenGame::Build()
{

	CreateTitle(IDS_TITLE_GAME_OPTIONS);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_GAME,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_GAME,"SliderWidth");

	//crosshair menu
	AddTextItem(IDS_CONTROLS_CROSSHAIR, CMD_CROSSHAIR, IDS_HELP_CROSSHAIRMENU);

	CLTGUICycleCtrl* pCycle = AddCycle(IDS_DISPLAY_SUBTITLES,IDS_HELP_SUBTITLES,kGap,&m_nSubtitles);
	char szTmp[64];
    FormatString(IDS_OFF,szTmp,sizeof(szTmp));
	pCycle->AddString(szTmp);

    FormatString(IDS_ON,szTmp,sizeof(szTmp));
	pCycle->AddString(szTmp);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile && pProfile->m_bAllowGore)
	{
		CLTGUIToggle *pGore = AddToggle(IDS_DISPLAY_GORE,IDS_HELP_GORE,kGap,&m_bGore);
	}

	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	//always run
	CLTGUIToggle* pToggle = AddToggle(IDS_ADVCONTROLS_RUNLOCK, IDS_HELP_RUNLOCK, kGap, &m_bAlwaysRun );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	// Add the Difficulty option
	m_pDifficultyCtrl = AddCycle(IDS_DIFFICULTY,IDS_HELP_DIFFICULTY,kGap,&m_nDifficulty);

    FormatString(IDS_NEW_EASY,szTmp,sizeof(szTmp));
	m_pDifficultyCtrl->AddString(szTmp);
	
	FormatString(IDS_NEW_MEDIUM,szTmp,sizeof(szTmp));
	m_pDifficultyCtrl->AddString(szTmp);
	
	FormatString(IDS_NEW_HARD,szTmp,sizeof(szTmp));
	m_pDifficultyCtrl->AddString(szTmp);

	FormatString(IDS_NEW_INSANE,szTmp,sizeof(szTmp));
	m_pDifficultyCtrl->AddString(szTmp);

	pCycle = AddCycle(IDS_HUDLAYOUT,IDS_HELP_HUDLAYOUT,kGap,&m_nLayout);
	for (int hl = 0; hl < g_pLayoutMgr->GetNumHUDLayouts(); hl++)
	{
		char szTmpBuffer[128];
		FormatString(g_pLayoutMgr->GetLayoutName(hl),szTmpBuffer,sizeof(szTmpBuffer));
		pCycle->AddString(szTmpBuffer);
	}


	CLTGUISlider *pSlider=AddSlider(IDS_HEADBOB, IDS_HELP_HEADBOB, kGap, kWidth, -1, &m_nHeadBob);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_WEAPONSWAY, IDS_HELP_WEAPONSWAY, kGap, kWidth, -1, &m_nWeaponSway);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_PICKUP_MSG_DUR, IDS_HELP_PICKUP_MSG_DUR, kGap, kWidth, -1, &m_nPickupMsgDur);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pToggle = AddToggle(IDS_OBJECTIVE_MSGS, IDS_HELP_OBJECTIVE_MSGS, kGap, &m_bObjMessages );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{

	switch(dwCommand)
	{
	case CMD_CROSSHAIR:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_CROSSHAIR);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

void CScreenGame::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetGameOptions();
	
		m_nDifficulty = pProfile->m_nDifficulty;
		m_nSubtitles  = pProfile->m_nSubtitles;
		m_bGore = pProfile->m_bGore;
		m_bAlwaysRun = pProfile->m_bAlwaysRun;
		m_nLayout = pProfile->m_nLayout;
		m_nHeadBob = pProfile->m_nHeadBob;
		m_nWeaponSway = pProfile->m_nWeaponSway;
		m_nPickupMsgDur = pProfile->m_nPickupMsgDur;
		m_bObjMessages = pProfile->m_bObjMessages;

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_nDifficulty = m_nDifficulty;
		pProfile->m_nSubtitles  = m_nSubtitles;
		pProfile->m_bGore = m_bGore;
		pProfile->m_bAlwaysRun = m_bAlwaysRun;
		pProfile->m_nLayout = m_nLayout;
		pProfile->m_nHeadBob = m_nHeadBob;
		pProfile->m_nWeaponSway = m_nWeaponSway;
		pProfile->m_nPickupMsgDur = m_nPickupMsgDur;
		pProfile->m_bObjMessages = m_bObjMessages;

		pProfile->ApplyGameOptions();
		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
}


