// MenuSound.cpp: implementation of the CMenuSound class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "MenuSound.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuSound::CMenuSound()
{
	m_bSoundOn=DFALSE;
	m_bMusicOn=DFALSE;
	m_nSoundVolume=50;
	m_nMusicVolume=50;
	m_bSoundQuality=DFALSE;
	m_bOldSoundQuality = DFALSE;

	m_pSoundVolumeCtrl=DNULL;
	m_pMusicVolumeCtrl=DNULL;
	m_pSoundQualityCtrl=DNULL;
}

CMenuSound::~CMenuSound()
{

}

// Build the menu
void CMenuSound::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(5);
	SetScrollWrap(DFALSE);	

	AddOnOffOption(IDS_MENU_AUDIO_SOUND, m_pMainMenus->GetSmallFont(), 100, &m_bSoundOn);	

	m_pSoundVolumeCtrl=AddSliderOption(IDS_MENU_AUDIO_SOUND_VOLUME, m_pMainMenus->GetSmallFont(), 100, m_pMainMenus->GetSurfaceSliderBar(), m_pMainMenus->GetSurfaceSliderTab(), &m_nSoundVolume);
	m_pSoundQualityCtrl=AddOnOffOption(IDS_MENU_AUDIO_16BIT_SOUND, m_pMainMenus->GetSmallFont(), 100, &m_bSoundQuality);

	AddOnOffOption(IDS_MENU_AUDIO_MUSIC, m_pMainMenus->GetSmallFont(), 100, &m_bMusicOn);

	m_pMusicVolumeCtrl=AddSliderOption(IDS_MENU_AUDIO_MUSIC_VOLUME, m_pMainMenus->GetSmallFont(), 100, m_pMainMenus->GetSurfaceSliderBar(), m_pMainMenus->GetSurfaceSliderTab(), &m_nMusicVolume);

	if ( m_pSoundVolumeCtrl )
	{
		m_pSoundVolumeCtrl->SetSliderRange(0, 50);
		m_pSoundVolumeCtrl->SetSliderIncrement(5);
	}
	if ( m_pMusicVolumeCtrl )
	{
		m_pMusicVolumeCtrl->SetSliderRange(0, 50);
		m_pMusicVolumeCtrl->SetSliderIncrement(5);
	}

	// Load the sound settings
	LoadSoundSettings();	
	UpdateData(DFALSE);	

	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	EnableDisableControls();
}

// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
void CMenuSound::EnableDisableControls()
{
	if ( !m_bSoundOn )
	{
		if ( m_pSoundVolumeCtrl )
		{
			m_pSoundVolumeCtrl->Enable(DFALSE);
			m_pSoundQualityCtrl->Enable(DFALSE);
		}	
	}
	else
	{
		if ( m_pSoundVolumeCtrl )
		{
			m_pSoundVolumeCtrl->Enable(DTRUE);
			m_pSoundQualityCtrl->Enable(DTRUE);
		}	
	}

	if ( !m_bMusicOn )
	{
		if ( m_pMusicVolumeCtrl )
		{
			m_pMusicVolumeCtrl->Enable(DFALSE);
		}	
	}
	else
	{
		if ( m_pMusicVolumeCtrl )
		{
			m_pMusicVolumeCtrl->Enable(DTRUE);
		}	
	}
}

// Load the sound settings
void CMenuSound::LoadSoundSettings()
{
	// Load the sound/music on/off console variables
	HCONSOLEVAR hVar=m_pClientDE->GetConsoleVar("soundenable");
	if (hVar && m_pClientDE->GetVarValueFloat(hVar) == 1.0f)
	{
		m_bSoundOn=DTRUE;
	}
	else
	{
		m_bSoundOn=DFALSE;
	}

	hVar=m_pClientDE->GetConsoleVar("musicenable");
	if (hVar && m_pClientDE->GetVarValueFloat(hVar) == 1.0f)
	{
		m_bMusicOn=DTRUE;
	}
	else
	{
		m_bMusicOn=DFALSE;
	}

	hVar=m_pClientDE->GetConsoleVar("soundvolume");
	if (hVar)
	{
		m_nSoundVolume=(int)m_pClientDE->GetVarValueFloat(hVar);
	}	

	hVar=m_pClientDE->GetConsoleVar("musicvolume");
	if (hVar)
	{
		m_nMusicVolume=(int)m_pClientDE->GetVarValueFloat(hVar);
	}		

	hVar=m_pClientDE->GetConsoleVar("sound16bit");
	if (hVar)
	{
		m_bOldSoundQuality = m_bSoundQuality=(m_pClientDE->GetVarValueFloat(hVar) == 0.0f) ? DFALSE : DTRUE;
	}		
}

// Save the sound settings
void CMenuSound::SaveSoundSettings()
{
	UpdateData(DTRUE);

	if ( m_bSoundOn )
	{
		m_pClientDE->RunConsoleString("+soundenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("+soundenable 0");
	}

	if ( m_bMusicOn )
	{
		m_pClientDE->RunConsoleString("+musicenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("+musicenable 0");
	}

	char szTemp[256];
	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, "+soundvolume %f", (float)m_nSoundVolume);
	m_pClientDE->RunConsoleString(szTemp);

	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, "+musicvolume %f", (float)m_nMusicVolume);
	m_pClientDE->RunConsoleString(szTemp);

	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, "+sound16bit %f", (m_bSoundQuality) ? 1.0f : 0.0f);
	m_pClientDE->RunConsoleString(szTemp);

	m_pClientDE->SetSoundVolume(m_nSoundVolume);
	m_pClientDE->SetMusicVolume(m_nMusicVolume);

	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	EnableDisableControls();
}

// Override the left and right controls so that the volumes can be changed
void CMenuSound::OnLeft()
{
	CMenuBase::OnLeft();

	SaveSoundSettings();
}

void CMenuSound::OnRight()
{
	CMenuBase::OnRight();

	SaveSoundSettings();
}

DDWORD CMenuSound::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{	
	return 0;
}

// The focus has changed
void CMenuSound::OnFocus(DBOOL bFocus)
{
	if (!bFocus)
	{
		// Only need to reinit sounds if quality changes.
		if( m_bOldSoundQuality != m_bSoundQuality )
			g_pBloodClientShell->InitSound();
	}
}
