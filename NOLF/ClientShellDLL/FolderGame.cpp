// FolderGame.cpp: implementation of the CFolderGame class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderGame.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"


namespace
{
	int kGap = 0;
	int kWidth = 0;
}

extern VarTrack	g_vtSubtitles;
extern VarTrack g_vtHUDLayout;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderGame::CFolderGame()
{
	m_nDifficulty = 2;
    m_nSubtitles  = 0;
    m_bGore = LTFALSE;
	m_bFadeBodies = LTFALSE;
	m_bAlwaysRun = LTTRUE;
	m_nLayout = 0;
	m_nHeadBob = 0;
	m_nWeaponSway = 0;
	m_nPickupMsgDur = 0;
	m_bObjMessages = LTTRUE;

}

CFolderGame::~CFolderGame()
{

}

// Build the folder
LTBOOL CFolderGame::Build()
{

	CreateTitle(IDS_TITLE_GAME_OPTIONS);
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_GAME,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_GAME,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_GAME,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_GAME,"SliderWidth");

	//crosshair menu
	AddTextItem(IDS_CONTROLS_CROSSHAIR, FOLDER_CMD_CROSSHAIR, IDS_HELP_CROSSHAIRMENU);

	CCycleCtrl *pCycle = AddCycleItem(IDS_DISPLAY_SUBTITLES,IDS_HELP_SUBTITLES,kGap-25,25,&m_nSubtitles);
	pCycle->AddString(IDS_OFF);
	pCycle->AddString(IDS_ON);

	if (pSettings && pSettings->IsGoreAllowed())
	{
		CToggleCtrl *pGore = AddToggle(IDS_DISPLAY_GORE,IDS_HELP_GORE,kGap,&m_bGore);
		pGore->SetOnString(IDS_ON);
		pGore->SetOffString(IDS_OFF);
	}

	CToggleCtrl *pFadeBodies = AddToggle(IDS_FADEBODIES,IDS_HELP_FADEBODIES,kGap,&m_bFadeBodies);
	pFadeBodies->SetOnString(IDS_YES);
	pFadeBodies->SetOffString(IDS_NO);

	//always run
	CToggleCtrl* pToggle = AddToggle(IDS_ADVCONTROLS_RUNLOCK, IDS_HELP_RUNLOCK, kGap, &m_bAlwaysRun );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	// Add the Difficulty option
	m_pDifficultyCtrl = AddCycleItem(IDS_DIFFICULTY,IDS_HELP_DIFFICULTY,kGap-25,25,&m_nDifficulty);
	m_pDifficultyCtrl->AddString(IDS_NEW_EASY);
	m_pDifficultyCtrl->AddString(IDS_NEW_MEDIUM);
	m_pDifficultyCtrl->AddString(IDS_NEW_HARD);
	m_pDifficultyCtrl->AddString(IDS_NEW_INSANE);

	pCycle = AddCycleItem(IDS_HUDLAYOUT,IDS_HELP_HUDLAYOUT,kGap-25,25,&m_nLayout);
	for (int hl = 0; hl < g_pLayoutMgr->GetNumHUDLayouts(); hl++)
	{
		pCycle->AddString(g_pLayoutMgr->GetLayoutName(hl));
	}


	CSliderCtrl *pSlider=AddSlider(IDS_HEADBOB, IDS_HELP_HEADBOB, kGap, kWidth, &m_nHeadBob);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_WEAPONSWAY, IDS_HELP_WEAPONSWAY, kGap, kWidth, &m_nWeaponSway);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pSlider=AddSlider(IDS_PICKUP_MSG_DUR, IDS_HELP_PICKUP_MSG_DUR, kGap, kWidth, &m_nPickupMsgDur);
	pSlider->SetSliderRange(0, 10);
	pSlider->SetSliderIncrement(1);

	pToggle = AddToggle(IDS_OBJECTIVE_MSGS, IDS_HELP_OBJECTIVE_MSGS, kGap, &m_bObjMessages );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CFolderGame::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_CROSSHAIR:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CROSSHAIR);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

void CFolderGame::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	if (bFocus)
	{
	
		m_bGore = pSettings->Gore();
		m_nDifficulty = g_pGameClientShell->GetDifficulty();
		m_bFadeBodies = g_pGameClientShell->GetFadeBodies();
		m_nSubtitles = (int)g_vtSubtitles.GetFloat();
		m_bAlwaysRun = pSettings->RunLock();

		m_nLayout = (int)g_vtHUDLayout.GetFloat();

		m_nHeadBob = (int)(10.0f * GetConsoleFloat("HeadBob",1.0f));
		m_nWeaponSway = (int)(10.0f * GetConsoleFloat("WeaponSway",1.0f));
		m_nPickupMsgDur = (int)(2.0f * GetConsoleFloat("PickupMessageDuration",5.0f));
		m_bObjMessages = ( GetConsoleInt("ObjectiveMessages",1) > 0 );

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		pSettings->SetBoolVar("Gore",m_bGore);
		g_vtSubtitles.WriteFloat((LTFLOAT)m_nSubtitles);
		g_vtHUDLayout.WriteFloat((LTFLOAT)m_nLayout);
		WriteConsoleInt("Difficulty",m_nDifficulty);
		g_pGameClientShell->SetDifficulty((GameDifficulty)m_nDifficulty);
		WriteConsoleInt("FadeBodies",(int)m_bFadeBodies);
		g_pGameClientShell->SetFadeBodies(m_bFadeBodies);
		pSettings->SetBoolVar("RunLock",m_bAlwaysRun);

		WriteConsoleFloat("HeadBob",((LTFLOAT)m_nHeadBob / 10.0f));
		WriteConsoleFloat("WeaponSway",((LTFLOAT)m_nWeaponSway / 10.0f));
		WriteConsoleFloat("PickupMessageDuration",((LTFLOAT)m_nPickupMsgDur / 2.0f));
		WriteConsoleInt("ObjectiveMessages",m_bObjMessages);

		g_pLTClient->WriteConfigFile("autoexec.cfg");


	}
	CBaseFolder::OnFocus(bFocus);
}


