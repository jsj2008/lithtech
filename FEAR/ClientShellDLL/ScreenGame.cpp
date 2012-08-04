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

namespace
{
	uint32 kGap = 0;
	uint32 kWidth = 0;
	uint32 kTextWidth = 0;
	int	kMinFade = 4;
	int kMaxFade = 25;
	float kfFadeDefault = 10.0f;
}

//extern VarTrack	g_vtSubtitles;
VarTrack	g_vtSubtitles;
extern VarTrack g_vtHUDLayout;


const wchar_t* HUDSliderTextCallback(int nPos, void* pUserData);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenGame::CScreenGame()
{
	m_nDifficulty = 2;
    m_bSubtitles = false;
    m_bGore = false;
	m_bAlwaysRun = true;
	m_bCrouchToggle = false;
	m_nLayout = 0;
	m_nHeadBob = 0;
	m_nMsgDur = 0;
	m_nAutoWeaponSwitch = 1;
	m_nHUDFade = int(kfFadeDefault);

	m_bMPAutoWeaponSwitch = false;
	m_bSPAutoWeaponSwitch = true;
	m_bSlowMoFX	= true;

}

CScreenGame::~CScreenGame()
{

}

// Build the screen
bool CScreenGame::Build()
{

	CreateTitle("IDS_TITLE_GAME_OPTIONS");

	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	kTextWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,2);

	//background frame
	CLTGUICtrl_create cs;
	cs.rnBaseRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,cs);
	AddControl(pFrame);

	m_DefaultPos = m_ScreenRect.m_vMin;

	//crosshair menu
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_CROSSHAIR;
	cs.szHelpID = "IDS_HELP_CROSSHAIRMENU";
	AddTextItem("IDS_CONTROLS_CROSSHAIR", cs);

	CLTGUIToggle_create tcs;
	tcs.nHeaderWidth = kGap;
	CLTGUIToggle *pToggle = NULL;

#if !defined(_DEMO) || defined(_SPDEMO)
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	tcs.szHelpID = "IDS_HELP_SUBTITLES";
	tcs.pbValue = &m_bSubtitles;

	pToggle = AddToggle("IDS_DISPLAY_SUBTITLES", tcs);
	pToggle->SetOnString(LoadString("IDS_ON"));
	pToggle->SetOffString(LoadString("IDS_OFF"));

#endif

	const wchar_t* szYes = LoadString( "IDS_YES" );
	const wchar_t* szNo = LoadString( "IDS_NO" );

	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile && !g_pVersionMgr->IsLowViolence())
	{
		tcs.szHelpID = "IDS_HELP_GORE";
		tcs.pbValue = &m_bGore;
		tcs.nHeaderWidth = kGap;
		CLTGUIToggle *pGore = AddToggle("IDS_DISPLAY_GORE",tcs);
		pGore->SetOnString(szYes);
		pGore->SetOffString(szNo);
	}


	//always run
	tcs.szHelpID = "IDS_HELP_RUNLOCK";
	tcs.pbValue = &m_bAlwaysRun;
	tcs.nHeaderWidth = kGap;
	pToggle = AddToggle("IDS_ADVCONTROLS_RUNLOCK",tcs);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	tcs.szHelpID = "ScreenOptions_CrouchToggle_Help";
	tcs.pbValue = &m_bCrouchToggle;
	tcs.nHeaderWidth = kGap;
	pToggle = AddToggle("ScreenOptions_CrouchToggle",tcs);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

#if !defined(_DEMO) || defined(_SPDEMO)
	// Add the Difficulty option
	CLTGUICycleCtrl_create ccs;
	ccs.szHelpID = "IDS_HELP_DIFFICULTY";
	ccs.pnValue = &m_nDifficulty;
	ccs.nHeaderWidth = kGap;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pDifficultyCtrl = AddCycle("IDS_DIFFICULTY",ccs);

	m_pDifficultyCtrl->AddString(LoadString("IDS_NEW_EASY"));
	m_pDifficultyCtrl->AddString(LoadString("IDS_NEW_MEDIUM"));
	m_pDifficultyCtrl->AddString(LoadString("IDS_NEW_HARD"));
	m_pDifficultyCtrl->AddString(LoadString("IDS_NEW_INSANE"));

#endif

	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_PICKUP_MSG_DUR";
	scs.pnValue = &m_nMsgDur;
	scs.nMin = 1;
	scs.nMax = 10;
	scs.nIncrement = 1;
	CLTGUISlider* pSlider = AddSlider("IDS_PICKUP_MSG_DUR", scs );

	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_HEADBOB";
	scs.pnValue = &m_nHeadBob;
	scs.nMin = 1;
	scs.nMax = 10;
	scs.nIncrement = 1;
	pSlider = AddSlider("IDS_HEADBOB", scs );


#if defined(_MPDEMO)
	tcs.szHelpID = "IDS_HELP_AUTOSWITCH_WEAPONS";
	tcs.pbValue = &m_bMPAutoWeaponSwitch;
	tcs.nHeaderWidth = kGap;
	pToggle = AddToggle("IDS_AUTOSWITCH_WEAPONS",tcs);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);
#elif defined(_SPDEMO)
	tcs.szHelpID = "IDS_HELP_AUTOSWITCH_WEAPONS";
	tcs.pbValue = &m_bSPAutoWeaponSwitch;
	tcs.nHeaderWidth = kGap;
	pToggle = AddToggle("IDS_AUTOSWITCH_WEAPONS",tcs);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);
#else
	ccs.szHelpID = "IDS_HELP_AUTOSWITCH_WEAPONS";
	ccs.pnValue = &m_nAutoWeaponSwitch;
	CLTGUICycleCtrl *pCycle = AddCycle("IDS_AUTOSWITCH_WEAPONS", ccs);
	pCycle->AddString(LoadString("IDS_NEVER"));
	pCycle->AddString(LoadString("IDS_SP_ONLY"));
	pCycle->AddString(LoadString("IDS_MP_ONLY"));
	pCycle->AddString(LoadString("IDS_ALWAYS"));
#endif

	//slowmo overlay
	tcs.szHelpID = "Screen_Options_SlowMoFX_Help";
	tcs.pbValue = &m_bSlowMoFX;
	tcs.nHeaderWidth = kGap;
	pToggle = AddToggle("Screen_Options_SlowMoFX",tcs);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_HUD_FADE_TIME";
	scs.pnValue = &m_nHUDFade;
	scs.nMin = kMinFade;
	scs.nMax = kMaxFade;
	scs.nIncrement = 3;
	scs.pnTextCallback = HUDSliderTextCallback;
	scs.nDisplayWidth = kTextWidth;
	pSlider = AddSlider("IDS_HUD_FADE_TIME", scs );

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
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

void CScreenGame::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetGameOptions();
	
		m_nDifficulty = pProfile->m_nDifficulty;
		m_bSubtitles  = pProfile->m_bSubtitles;
		m_bGore = pProfile->m_bGore;
		m_bAlwaysRun = pProfile->m_bAlwaysRun;
		m_bCrouchToggle = pProfile->m_bCrouchToggle;
		m_nHeadBob = pProfile->m_nHeadBob;
		m_nMsgDur = pProfile->m_nMsgDur;
		m_bSlowMoFX = pProfile->m_bSlowMoFX;

		m_nAutoWeaponSwitch = 0;
		m_bSPAutoWeaponSwitch = pProfile->m_bSPAutoWeaponSwitch;
		m_bMPAutoWeaponSwitch = pProfile->m_bMPAutoWeaponSwitch;
		if (m_bSPAutoWeaponSwitch)
		{
			m_nAutoWeaponSwitch += 1;
		}
		if (m_bMPAutoWeaponSwitch)
		{
			m_nAutoWeaponSwitch += 2;
		}

		if (pProfile->m_bPersistentHUD)
		{
			m_nHUDFade = kMinFade;
		}
		else 
		{
			m_nHUDFade = int(kfFadeDefault * pProfile->m_fHUDFadeSpeed);
			m_nHUDFade = LTCLAMP(m_nHUDFade,kMinFade,kMaxFade);
		}

#if !defined(_DEMO) || defined(_SPDEMO)
		m_pDifficultyCtrl->Enable( !(g_pLTClient->IsConnected() && IsMultiplayerGameClient())	);
#endif

        UpdateData(false);
	}
	else
	{
		UpdateData();

		pProfile->m_nDifficulty = m_nDifficulty;
		pProfile->m_bSubtitles  = m_bSubtitles;
		pProfile->m_bGore = !!m_bGore;
		pProfile->m_bAlwaysRun = m_bAlwaysRun;
		pProfile->m_bCrouchToggle = m_bCrouchToggle;
		pProfile->m_nHeadBob = m_nHeadBob;
		pProfile->m_nMsgDur = m_nMsgDur;
		pProfile->m_bSlowMoFX = m_bSlowMoFX;

#if defined(_MPDEMO)
		pProfile->m_bMPAutoWeaponSwitch = m_bMPAutoWeaponSwitch;
#elif defined(_SPDEMO)
		pProfile->m_bSPAutoWeaponSwitch = m_bSPAutoWeaponSwitch;
#else
		pProfile->m_bSPAutoWeaponSwitch = !!(m_nAutoWeaponSwitch & 0x01 );
		pProfile->m_bMPAutoWeaponSwitch = !!(m_nAutoWeaponSwitch & 0x02 );
#endif

		pProfile->m_bPersistentHUD = (m_nHUDFade == kMinFade);
		if (pProfile->m_bPersistentHUD)
			g_pHUDMgr->ResetAllFades();
		pProfile->m_fHUDFadeSpeed = float(m_nHUDFade) / kfFadeDefault;

		pProfile->ApplyGameOptions();
		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
}


const wchar_t* HUDSliderTextCallback(int nPos, void* pUserData)
{
	if (nPos == kMinFade)
		return LoadString("IDS_HUD_FADE_DISABLED");
	else if (nPos == int(kfFadeDefault))
	{
		return LoadString("IDS_SPEED_NORMAL");
	}

	static wchar_t wszBuffer[128];
	wchar_t wszTmp[16];
	float fHUDFadeSpeed = float(nPos) / kfFadeDefault;
	LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"x%0.1f",fHUDFadeSpeed);
	if (nPos > int(kfFadeDefault))
	{
		FormatString( "IDS_SPEED_FAST", wszBuffer, LTARRAYSIZE(wszBuffer), wszTmp );
		return wszBuffer;
	}

	FormatString( "IDS_SPEED_SLOW", wszBuffer, LTARRAYSIZE(wszBuffer), wszTmp );
	return wszBuffer;
}
