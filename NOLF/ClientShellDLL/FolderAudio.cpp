// FolderAudio.cpp: implementation of the CFolderAudio class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderAudio.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"

#define MUSIC_MIN_VOL		-5000
#define MUSIC_MAX_VOL		5000
#define MUSIC_SLIDER_INC	500
#define MUSIC_DEFAULT_VOL	((MUSIC_MAX_VOL - MUSIC_MIN_VOL) / 2)

#define SOUND_MIN_VOL		0
#define SOUND_MAX_VOL		100
#define SOUND_SLIDER_INC	10
#define	SOUND_DEFAULT_VOL	75

namespace
{
	int kGap = 0;
	int kWidth = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderAudio::CFolderAudio()
{
    m_bSoundOn=LTFALSE;
    m_bMusicOn=LTFALSE;
	m_nSoundVolume=SOUND_DEFAULT_VOL;
	m_nMusicVolume=MUSIC_DEFAULT_VOL;
    m_bSoundQuality=LTFALSE;
    m_bOldSoundQuality = LTFALSE;

    m_pSoundVolumeCtrl=LTNULL;
    m_pMusicVolumeCtrl=LTNULL;
    m_pSoundQualityCtrl=LTNULL;

}

CFolderAudio::~CFolderAudio()
{

}

// Build the folder
LTBOOL CFolderAudio::Build()
{

	CreateTitle(IDS_TITLE_SOUND);

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_AUDIO,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_AUDIO,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_AUDIO,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_AUDIO,"SliderWidth");

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

//	CToggleCtrl* pToggle = AddToggle(IDS_SOUND_FX, IDS_HELP_SOUNDFX, kGap, &m_bSoundOn);

	m_pSoundVolumeCtrl=AddSlider(IDS_SOUND_FXVOL, IDS_HELP_SOUNDVOL, kGap, kWidth, &m_nSoundVolume);
	m_pSoundVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );

	m_pSoundQualityCtrl=AddToggle(IDS_SOUND_QUALITY, IDS_HELP_SOUNDQUAL, kGap, &m_bSoundQuality);
	m_pSoundQualityCtrl->SetOnString(IDS_SOUND_HIGH);
	m_pSoundQualityCtrl->SetOffString(IDS_SOUND_LOW);
	m_pSoundQualityCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );

//	pToggle = AddToggle(IDS_SOUND_MUSICENABLED, IDS_HELP_MUSIC, kGap, &m_bMusicOn);

	m_pMusicVolumeCtrl = AddSlider(IDS_SOUND_MUSICVOL, IDS_HELP_MUSICVOL, kGap, kWidth, &m_nMusicVolume);
	m_pMusicVolumeCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );

	if ( m_pSoundVolumeCtrl )
	{
		m_pSoundVolumeCtrl->SetSliderRange(SOUND_MIN_VOL, SOUND_MAX_VOL);
		m_pSoundVolumeCtrl->SetSliderIncrement(SOUND_SLIDER_INC);
	}
	if ( m_pMusicVolumeCtrl )
	{
		m_pMusicVolumeCtrl->SetSliderRange(MUSIC_MIN_VOL, MUSIC_MAX_VOL);
		m_pMusicVolumeCtrl->SetSliderIncrement(MUSIC_SLIDER_INC);
	}

	// Load the sound settings
	LoadSoundSettings();
	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	//EnableDisableControls();

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CFolderAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
void CFolderAudio::EnableDisableControls()
{
	if ( !m_bSoundOn )
	{
		if ( m_pSoundVolumeCtrl )
		{
            m_pSoundVolumeCtrl->Enable(LTFALSE);
            m_pSoundQualityCtrl->Enable(LTFALSE);
		}
	}
	else
	{
		if ( m_pSoundVolumeCtrl )
		{
            m_pSoundVolumeCtrl->Enable(LTTRUE);
            m_pSoundQualityCtrl->Enable(LTTRUE);
		}
	}

	if ( !m_bMusicOn )
	{
		if ( m_pMusicVolumeCtrl )
		{
            m_pMusicVolumeCtrl->Enable(LTFALSE);
		}
	}
	else
	{
		if ( m_pMusicVolumeCtrl )
		{
            m_pMusicVolumeCtrl->Enable(LTTRUE);
		}
	}
}

void CFolderAudio::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		LoadSoundSettings();
	}
	else
	{
		SaveSoundSettings();
	}
	CBaseFolder::OnFocus(bFocus);
}


// Load the sound settings
void CFolderAudio::LoadSoundSettings()
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
	m_nSoundVolume = (int)pSettings->GetFloatVar("soundvolume");
	m_nMusicVolume = (int)pSettings->GetFloatVar("musicvolume");

	// m_bOldSoundOn = m_bSoundOn = (m_nSoundVolume != SOUND_MIN_VOL);
	m_bOldSoundOn = m_bSoundOn = (LTBOOL)pSettings->GetFloatVar("soundenable");

	m_bOldMusicOn = m_bMusicOn = (m_nMusicVolume != MUSIC_MIN_VOL);
	m_bOldSoundQuality = m_bSoundQuality = pSettings->GetBoolVar("sound16bit");
    UpdateData(LTFALSE);

}

// Save the sound settings
void CFolderAudio::SaveSoundSettings()
{
    UpdateData(LTTRUE);
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();

	// m_bSoundOn = (m_nSoundVolume != SOUND_MIN_VOL);
	// pSettings->SetBoolVar("soundenable", m_bSoundOn);
	// Turning off the sound is more efficient however, lip-syncing
	// and subtitles won't work...so, we'll leave it on and just
	// live with it...
	m_bSoundOn = (LTBOOL)pSettings->GetFloatVar("soundenable");
	
	pSettings->SetFloatVar("soundvolume",(float)m_nSoundVolume);
	pSettings->ImplementSoundVolume();

	if (m_bOldSoundOn != m_bSoundOn)
	{
		m_bOldSoundOn = m_bSoundOn;
	}

	m_bMusicOn = (m_nMusicVolume != MUSIC_MIN_VOL);
	pSettings->SetBoolVar("MusicEnable",m_bMusicOn);
	if (m_bOldMusicOn != m_bMusicOn)
	{
		pSettings->ImplementMusicSource();
		m_bOldMusicOn = m_bMusicOn;
	}
	pSettings->SetFloatVar("musicvolume",(float)m_nMusicVolume);
	pSettings->ImplementMusicVolume();




	pSettings->SetBoolVar("sound16bit",m_bSoundQuality);

	if (m_bSoundQuality != m_bOldSoundQuality)
	{
		pSettings->ImplementSoundQuality();
		m_bOldSoundQuality = m_bSoundQuality;
	}

	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	//EnableDisableControls();
}



// Override the left and right controls so that the volumes can be changed
LTBOOL CFolderAudio::OnLeft()
{
    LTBOOL handled = CBaseFolder::OnLeft();

	if (handled)
		SaveSoundSettings();
	return handled;
}

LTBOOL CFolderAudio::OnRight()
{
    LTBOOL handled = CBaseFolder::OnRight();

	if (handled)
		SaveSoundSettings();
	return handled;
}


LTBOOL CFolderAudio::OnEnter()
{
    LTBOOL handled = CBaseFolder::OnEnter();

	if (handled)
		SaveSoundSettings();
	return handled;
}

LTBOOL CFolderAudio::OnLButtonUp(int x, int y)
{
    LTBOOL handled = CBaseFolder::OnLButtonUp(x, y);

	if (handled)
		SaveSoundSettings();
	return handled;
}
