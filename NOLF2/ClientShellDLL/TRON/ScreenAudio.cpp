// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.cpp
//
// PURPOSE : Interface screen for setting audio options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenAudio.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "ProfileMgr.h"

#include "GameClientShell.h"


namespace
{
	int kGap = 0;
	int kWidth = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenAudio::CScreenAudio()
{
	m_nSoundVolume=SOUND_DEFAULT_VOL;
	m_nMusicVolume=MUSIC_DEFAULT_VOL;
	m_nSpeechVolume = SPEECH_DEFAULT_VOL;
    m_bSoundQuality=LTFALSE;

    m_pSoundVolumeCtrl=LTNULL;
    m_pSpeechVolumeCtrl=LTNULL;
    m_pMusicVolumeCtrl=LTNULL;
    m_pSoundQualityCtrl=LTNULL;

}

CScreenAudio::~CScreenAudio()
{

}

// Build the screen
LTBOOL CScreenAudio::Build()
{

	CreateTitle(IDS_TITLE_SOUND);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_AUDIO,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_AUDIO,"SliderWidth");

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	m_pSoundVolumeCtrl=AddSlider(IDS_SOUND_FXVOL, IDS_HELP_SOUNDVOL, kGap, kWidth, -1, &m_nSoundVolume);
	m_pSoundVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSoundVolumeCtrl->SetSliderRange(SOUND_MIN_VOL, SOUND_MAX_VOL);
	m_pSoundVolumeCtrl->SetSliderIncrement(SOUND_SLIDER_INC);

	m_pSpeechVolumeCtrl=AddSlider(IDS_SPEECH_FXVOL, IDS_HELP_SPEECHVOL, kGap, kWidth, -1, &m_nSpeechVolume);
	m_pSpeechVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSpeechVolumeCtrl->SetSliderRange(SPEECH_MIN_VOL, SPEECH_MAX_VOL);
	m_pSpeechVolumeCtrl->SetSliderIncrement(SPEECH_SLIDER_INC);

	m_pSoundQualityCtrl=AddToggle(IDS_SOUND_QUALITY, IDS_HELP_SOUNDQUAL, kGap, &m_bSoundQuality);

	char szTmp[64];
	FormatString(IDS_SOUND_HIGH,szTmp,sizeof(szTmp));
	m_pSoundQualityCtrl->SetOnString(szTmp);
	FormatString(IDS_SOUND_LOW,szTmp,sizeof(szTmp));
	m_pSoundQualityCtrl->SetOffString(szTmp);
	m_pSoundQualityCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );


	m_pMusicVolumeCtrl = AddSlider(IDS_SOUND_MUSICVOL, IDS_HELP_MUSICVOL, kGap, kWidth, -1, &m_nMusicVolume);
	m_pMusicVolumeCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );
	m_pMusicVolumeCtrl->SetSliderRange(MUSIC_MIN_VOL, MUSIC_MAX_VOL);
	m_pMusicVolumeCtrl->SetSliderIncrement(MUSIC_SLIDER_INC);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};


void CScreenAudio::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetSound();

		m_nSoundVolume = pProfile->m_nSoundVolume;
		m_nMusicVolume = pProfile->m_nMusicVolume;
		m_nSpeechVolume = (int)(100.0f * pProfile->m_fSpeechSoundMultiplier);
		m_bSoundQuality = pProfile->m_bSoundQuality;

		LTBOOL bSoundOn = (LTBOOL)GetConsoleInt("soundenable",1);
        m_pSoundVolumeCtrl->Enable(bSoundOn);
		m_pSpeechVolumeCtrl->Enable(bSoundOn);
        m_pSoundQualityCtrl->Enable(bSoundOn);

		LTBOOL bMusicOn = (m_nMusicVolume != MUSIC_MIN_VOL);
        m_pMusicVolumeCtrl->Enable(bMusicOn);

	    UpdateData(LTFALSE);
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
    UpdateData(LTTRUE);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nSoundVolume = m_nSoundVolume;
	pProfile->m_nMusicVolume = m_nMusicVolume;
	pProfile->m_fSpeechSoundMultiplier = m_nSpeechVolume / 100.0f;
	pProfile->m_bSoundQuality = m_bSoundQuality;

	pProfile->ApplySound();

}



// Override the left and right controls so that the volumes can be changed
LTBOOL CScreenAudio::OnLeft()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
		
	return handled;
}

LTBOOL CScreenAudio::OnRight()
{
    LTBOOL handled = CBaseScreen::OnRight();

	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}


LTBOOL CScreenAudio::OnEnter()
{
    LTBOOL handled = CBaseScreen::OnEnter();

	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnLButtonUp(int x, int y)
{
    LTBOOL handled = CBaseScreen::OnLButtonUp(x, y);

	if (handled)
	{
		SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}
