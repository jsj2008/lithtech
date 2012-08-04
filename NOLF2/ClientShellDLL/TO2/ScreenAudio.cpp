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
	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenAudio *pThisScreen = (CScreenAudio *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_AUDIO);
		if (pThisScreen)
		{
			pThisScreen->SendCommand(CMD_CONFIRM,bReturn,(uint32)pData);
		}
	}
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
    m_bMusicQuality=LTFALSE;
    m_bMusicEnabled=LTFALSE;

    m_pSoundVolumeCtrl=LTNULL;
    m_pSpeechVolumeCtrl=LTNULL;
    m_pMusicVolumeCtrl=LTNULL;
    m_pSoundQualityCtrl=LTNULL;
    m_pMusicQualityCtrl=LTNULL;

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
	m_bMusicEnabled = (dwAdvancedOptions & AO_MUSIC);

	//background frame
	LTRect frameRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_AUDIO,"FrameRect");
	LTIntPt pos(frameRect.left,frameRect.top);
	int nHt = frameRect.bottom - frameRect.top;
	int nWd = frameRect.right - frameRect.left;

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_AUDIO,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,nWd,nHt+8,LTTRUE);
	pFrame->SetBasePos(pos);
	pFrame->SetBorder(2,m_SelectedColor);
	AddControl(pFrame);

	m_pSoundVolumeCtrl=AddSlider(IDS_SOUND_FXVOL, IDS_HELP_SOUNDVOL, kGap, kWidth, -1, &m_nSoundVolume);
	m_pSoundVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSoundVolumeCtrl->SetSliderRange(SOUND_MIN_VOL, SOUND_MAX_VOL);
	m_pSoundVolumeCtrl->SetSliderIncrement(SOUND_SLIDER_INC);

   	m_pSpeechVolumeCtrl=AddSlider(IDS_SPEECH_FXVOL, IDS_HELP_SPEECHVOL, kGap, kWidth, -1, &m_nSpeechVolume);
	m_pSpeechVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSpeechVolumeCtrl->SetSliderRange(SPEECH_MIN_VOL, SPEECH_MAX_VOL);
	m_pSpeechVolumeCtrl->SetSliderIncrement(SPEECH_SLIDER_INC);

	m_pSoundQualityCtrl=AddToggle(IDS_SOUND_QUALITY, IDS_HELP_SOUNDQUAL, kGap, &m_bSoundQuality);
	m_pSoundQualityCtrl->SetOnString(LoadTempString(IDS_SOUND_HIGH));
	m_pSoundQualityCtrl->SetOffString(LoadTempString(IDS_SOUND_LOW));
	m_pSoundQualityCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );

	m_pMusicVolumeCtrl = AddSlider(IDS_SOUND_MUSICVOL, IDS_HELP_MUSICVOL, kGap, kWidth, -1, &m_nMusicVolume);
	m_pMusicVolumeCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );
	m_pMusicVolumeCtrl->SetSliderRange(MUSIC_MIN_VOL, MUSIC_MAX_VOL);
	m_pMusicVolumeCtrl->SetSliderIncrement(MUSIC_SLIDER_INC);

	m_pMusicQualityCtrl=AddToggle(IDS_MUSIC_QUALITY, IDS_HELP_MUSIC_QUALITY, kGap, &m_bMusicQuality);
	m_pMusicQualityCtrl->SetOnString(LoadTempString(IDS_SOUND_HIGH));
	m_pMusicQualityCtrl->SetOffString(LoadTempString(IDS_SOUND_LOW));
	m_pMusicQualityCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_CONFIRM)
	{
		// See if we kept the change or not...
		bool bChanged = true;

		if (dwParam2 == IDS_CONFIRM_MUSIC)
		{
			LTBOOL bChangedMusicQuality = m_bMusicQuality;

			m_bMusicQuality = (dwParam1 > 0);
			m_pMusicQualityCtrl->UpdateData(LTFALSE);

			bChanged = (bChangedMusicQuality == m_bMusicQuality);
		}
		else if (dwParam2 == IDS_CONFIRM_SOUND)
		{
			LTBOOL bChangedSoundQuality = m_bSoundQuality;

			m_bSoundQuality = (dwParam1 > 0);
			m_pSoundQualityCtrl->UpdateData(LTFALSE);

			bChanged = (bChangedSoundQuality == m_bSoundQuality);
		}

		if (bChanged)
		{
			SaveSoundSettings();
		}

		return 1;
	}
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

		m_bMusicQuality = pProfile->m_bMusicQuality;

		if (!GetConsoleInt("MusicActive",0))
			m_nMusicVolume = MUSIC_MIN_VOL;

		m_pMusicVolumeCtrl->Enable(m_bMusicEnabled);
		m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);

		m_nPerformance = g_pPerformanceMgr->GetPerformanceCfg(false);

	    UpdateData(LTFALSE);
	}
	else
	{
		SaveSoundSettings();

		//sound setting can affect performance sttings so update them here...
		g_pPerformanceMgr->GetPerformanceOptions(&pProfile->m_sPerformance);
		pProfile->Save();
	}

	CBaseScreen::OnFocus(bFocus);
}

//check to see if they really want to (for performance reasons)
void CScreenAudio::ConfirmQualityChange(bool bMusic)
{
	UpdateData(LTTRUE);

	if (bMusic)
	{
		if (m_bMusicQuality && (m_nPerformance < 2))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = (void *)IDS_CONFIRM_MUSIC;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_MUSIC,&mb,0,LTFALSE);
			return;
		}
	}
	else
	{
		if (m_bSoundQuality && (m_nPerformance == 0))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = (void *)IDS_CONFIRM_SOUND;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_SOUND,&mb,0,LTFALSE);
			return;
		}
	}

	// We accepted change, so save the new sound settings...

	SaveSoundSettings();
}

// Save the sound settings
void CScreenAudio::SaveSoundSettings()
{
    UpdateData(LTTRUE);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nSoundVolume = m_nSoundVolume;
	pProfile->m_nMusicVolume = m_nMusicVolume;

	WriteConsoleInt("MusicActive", (m_nMusicVolume > MUSIC_MIN_VOL) && m_bMusicEnabled);
	pProfile->m_fSpeechSoundMultiplier = m_nSpeechVolume / 100.0f;
	pProfile->m_bSoundQuality = m_bSoundQuality;
	pProfile->m_bMusicQuality = m_bMusicQuality;

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
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
		
	return handled;
}

LTBOOL CScreenAudio::OnRight()
{
    LTBOOL handled = CBaseScreen::OnRight();

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}


LTBOOL CScreenAudio::OnEnter()
{
    LTBOOL handled = CBaseScreen::OnEnter();

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnLButtonUp(int x, int y)
{
    LTBOOL handled = CBaseScreen::OnLButtonUp(x, y);

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnRButtonUp(int x, int y)
{
    LTBOOL handled = CBaseScreen::OnRButtonUp(x, y);

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > MUSIC_MIN_VOL);
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	return handled;
}
