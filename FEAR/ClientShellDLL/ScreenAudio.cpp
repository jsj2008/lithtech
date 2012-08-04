// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.cpp
//
// PURPOSE : Interface screen for setting audio options
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenAudio.h"

static int kGap = 200;
static int kWidth = 200;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenAudio::CScreenAudio()
{
	m_nSoundVolume = 100;
	m_nSpeechVolume = 100;
	m_nMusicVolume = 100;
	m_bEAXEnable = true;

	m_pSoundVolumeCtrl=NULL;
	m_pSpeechVolumeCtrl=NULL;
	m_pMusicVolumeCtrl=NULL;

	m_pEAXEnableCtrl=NULL;
	m_pEAX4EnableCtrl=NULL;
	m_pHWSoundsEnableCtrl=NULL;
}

CScreenAudio::~CScreenAudio()
{

}

// Build the screen
bool CScreenAudio::Build()
{
	CreateTitle("IDS_TITLE_SOUND");

	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	SoundCapabilities SoundCaps;

	g_pLTClient->SoundMgr()->GetSoundCapabilities(&SoundCaps);
	
	//background frame
	CLTGUICtrl_create cs;
	cs.rnBaseRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);

	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,cs);
	AddControl(pFrame);

	cs.rnBaseRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,1);
	hFrame.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,0));
	pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,cs,true);
	AddControl(pFrame);

	m_DefaultPos = m_ScreenRect.m_vMin;

	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_SOUNDVOL";
	scs.pnValue = &m_nSoundVolume;
	scs.nMin = 0;
	scs.nMax = 100;
	scs.nIncrement = 5;
	m_pSoundVolumeCtrl= AddSlider("IDS_SOUND_FXVOL", scs );
	m_pSoundVolumeCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) );

	scs.szHelpID = "IDS_HELP_SPEECHVOL";
	scs.pnValue = &m_nSpeechVolume;
	m_pSpeechVolumeCtrl= AddSlider("IDS_SPEECH_FXVOL", scs );
	m_pSpeechVolumeCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) );

	scs.szHelpID = "IDS_HELP_MUSICVOL";
	scs.pnValue = &m_nMusicVolume;
	m_pMusicVolumeCtrl= AddSlider("IDS_SOUND_MUSICVOL", scs );
	m_pMusicVolumeCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) );


	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	tcs.szHelpID = "IDS_HELP_SOUND_USE_HW";
	tcs.pbValue = &m_bHWSoundsEnable;
	tcs.nHeaderWidth = kGap;

	// Enable HW Sounds
	m_pHWSoundsEnableCtrl= AddToggle("IDS_SOUND_USE_HW", tcs );
	m_pHWSoundsEnableCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) && SoundCaps.bSupportsHWSounds);

	// Use EAX
	tcs.szHelpID = "IDS_HELP_SOUND_USE_EAX";
	tcs.pbValue = &m_bEAXEnable;
	m_pEAXEnableCtrl= AddToggle("IDS_SOUND_USE_EAX", tcs );
	m_pEAXEnableCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) && SoundCaps.bSupportsEAX);

	// Use EAX 4.0, if available
	tcs.szHelpID = "IDS_HELP_SOUND_USE_EAX4";
	tcs.pbValue = &m_bEAX4Enable;
	m_pEAX4EnableCtrl= AddToggle("IDS_SOUND_USE_EAX4", tcs );
	m_pEAX4EnableCtrl->Enable( !!(dwAdvancedOptions & AO_SOUND) && SoundCaps.bSupportsEAX4);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;

}

uint32 CScreenAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};

void CScreenAudio::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		uint16 nVol = 0;
		SoundCapabilities SoundCaps;
		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->GetVolume(nVol);


		pProfile->SetSound();
		g_pLTClient->SoundMgr()->GetSoundCapabilities(&SoundCaps);

		m_nSoundVolume = (int)(100.0f * pProfile->m_fSoundVolume);
		m_nSpeechVolume = (int)(100.0f * pProfile->m_fSpeechVolume);
		m_nMusicVolume = (int)(100.0f * pProfile->m_fMusicVolume);

		m_bHWSoundsEnable = pProfile->m_bUseHWSound;
		m_bEAXEnable = pProfile->m_bUseEAX;
		m_bEAX4Enable = pProfile->m_bUseEAX4;

		const bool bSoundOn = !!(GetConsoleInt("soundenable",1));
		const bool bMusicOn = !!(GetConsoleInt("MusicEnable",1));
		m_pSoundVolumeCtrl->Enable(bSoundOn);
		m_pSpeechVolumeCtrl->Enable(bSoundOn);
		m_pMusicVolumeCtrl->Enable(bSoundOn && bMusicOn);
		
		m_pHWSoundsEnableCtrl->Enable(bSoundOn && SoundCaps.bSupportsHWSounds);
		m_pEAXEnableCtrl->Enable(bSoundOn && SoundCaps.bSupportsEAX);
		m_pEAX4EnableCtrl->Enable(bSoundOn && SoundCaps.bSupportsEAX4);

		UpdateData(false);
	}
	else
	{
		SaveSoundSettings();
		pProfile->Save();
	}

	CBaseScreen::OnFocus(bFocus);
}

// Save the sound settings
void CScreenAudio::SaveSoundSettings()
{
	UpdateData(true);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	
	pProfile->m_fSoundVolume = (float)m_nSoundVolume / 100.0f;
	pProfile->m_fSpeechVolume = (float)m_nSpeechVolume / 100.0f;
	pProfile->m_fMusicVolume = (float)m_nMusicVolume / 100.0f;

	pProfile->m_bUseHWSound = m_bHWSoundsEnable;
	pProfile->m_bUseEAX = m_bEAXEnable;
	pProfile->m_bUseEAX4 = m_bEAX4Enable;

	pProfile->ApplySound();

}

// Override the left and right controls so that the volumes can be changed
bool CScreenAudio::OnLeft()
{
	bool handled = CBaseScreen::OnLeft();
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}

	return handled;
}

bool CScreenAudio::OnRight()
{
	bool handled = CBaseScreen::OnRight();
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}


bool CScreenAudio::OnEnter()
{
	bool handled = CBaseScreen::OnEnter();
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

bool CScreenAudio::OnLButtonUp(int x, int y)
{
	bool handled = CBaseScreen::OnLButtonUp(x, y);
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

bool CScreenAudio::OnRButtonUp(int x, int y)
{
	bool handled = CBaseScreen::OnRButtonUp(x, y);
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

