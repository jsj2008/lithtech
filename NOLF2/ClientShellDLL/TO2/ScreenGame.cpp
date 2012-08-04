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
#include "VersionMgr.h"
#include "ResShared.h"

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
	m_nMsgDur = 0;
	m_bAutoWeaponSwitch = LTTRUE;
	m_bLoadScreenTips = LTTRUE;
	m_bVehicleContour = LTTRUE;
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

	//background frame
	LTRect frameRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_GAME,"FrameRect");
	LTIntPt pos(frameRect.left,frameRect.top);
	int nHt = frameRect.bottom - frameRect.top;
	int nWd = frameRect.right - frameRect.left;

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_GAME,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,nWd,nHt+8,LTTRUE);
	pFrame->SetBasePos(pos);
	pFrame->SetBorder(2,m_SelectedColor);
	AddControl(pFrame);

	//crosshair menu
	AddTextItem(IDS_CONTROLS_CROSSHAIR, CMD_CROSSHAIR, IDS_HELP_CROSSHAIRMENU);

	CLTGUICycleCtrl* pCycle = AddCycle(IDS_DISPLAY_SUBTITLES,IDS_HELP_SUBTITLES,kGap,&m_nSubtitles);
	char szTmp[64];
    FormatString(IDS_OFF,szTmp,sizeof(szTmp));
	pCycle->AddString(szTmp);

    FormatString(IDS_ON,szTmp,sizeof(szTmp));
	pCycle->AddString(szTmp);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile && !g_pVersionMgr->IsLowViolence())
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
/*
	pCycle = AddCycle(IDS_HUDLAYOUT,IDS_HELP_HUDLAYOUT,kGap,&m_nLayout);
	for (int hl = 0; hl < g_pLayoutMgr->GetNumHUDLayouts(); hl++)
	{
		char szTmpBuffer[128];
		FormatString(g_pLayoutMgr->GetLayoutName(hl),szTmpBuffer,sizeof(szTmpBuffer));
		pCycle->AddString(szTmpBuffer);
	}
*/

	CLTGUISlider *pSlider=AddSlider(IDS_HEADBOB, IDS_HELP_HEADBOB, kGap, kWidth, -1, &m_nHeadBob);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_WEAPONSWAY, IDS_HELP_WEAPONSWAY, kGap, kWidth, -1, &m_nWeaponSway);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_PICKUP_MSG_DUR, IDS_HELP_PICKUP_MSG_DUR, kGap, kWidth, -1, &m_nMsgDur);
	pSlider->SetSliderRange(1, 10);
	pSlider->SetSliderIncrement(1);

	pToggle = AddToggle(IDS_AUTOSWITCH_WEAPONS, IDS_HELP_AUTOSWITCH_WEAPONS, kGap, &m_bAutoWeaponSwitch );
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	pToggle = AddToggle(IDS_LOAD_TIPS, IDS_HELP_LOAD_TIPS, kGap, &m_bLoadScreenTips );
	pToggle->SetOnString(LoadTempString(IDS_ON));
	pToggle->SetOffString(LoadTempString(IDS_OFF));

	pToggle = AddToggle(IDS_CONTOUR, IDS_HELP_CONTOUR, kGap, &m_bVehicleContour );
	pToggle->SetOnString(LoadTempString(IDS_ON));
	pToggle->SetOffString(LoadTempString(IDS_OFF));

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
		m_nMsgDur = pProfile->m_nMsgDur;
		m_bAutoWeaponSwitch = pProfile->m_bAutoWeaponSwitch;
		m_bLoadScreenTips = pProfile->m_bLoadScreenTips;
		m_bVehicleContour = pProfile->m_bVehicleContour;

		m_pDifficultyCtrl->Enable( !(g_pLTClient->IsConnected() && IsMultiplayerGame())	);

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pProfile->m_nDifficulty = m_nDifficulty;
		pProfile->m_nSubtitles  = m_nSubtitles;
		pProfile->m_bGore = !!m_bGore;
		pProfile->m_bAlwaysRun = m_bAlwaysRun;
		pProfile->m_nLayout = m_nLayout;
		pProfile->m_nHeadBob = m_nHeadBob;
		pProfile->m_nWeaponSway = m_nWeaponSway;
		pProfile->m_nMsgDur = m_nMsgDur;
		pProfile->m_bAutoWeaponSwitch = m_bAutoWeaponSwitch;
		pProfile->m_bLoadScreenTips = m_bLoadScreenTips;
		pProfile->m_bVehicleContour = m_bVehicleContour;

		pProfile->ApplyGameOptions();
		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
}


