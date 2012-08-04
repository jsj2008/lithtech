// FolderAudio.h: interface for the CFolderAudio class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_AUDIO_H_
#define _FOLDER_AUDIO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderAudio : public CBaseFolder
{
public:
	CFolderAudio();
	virtual ~CFolderAudio();

	// Build the folder
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);

	// Override the left and right controls so that the volumes can be changed
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnEnter();
    LTBOOL   OnLButtonUp(int x, int y);

protected:
	void	LoadSoundSettings();
	void	SaveSoundSettings();

	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	void	EnableDisableControls();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
    LTBOOL   m_bSoundOn;
    LTBOOL   m_bMusicOn;
    LTBOOL   m_bOldSoundOn;
    LTBOOL   m_bOldMusicOn;
	int		m_nSoundVolume;
	int		m_nMusicVolume;
    LTBOOL   m_bSoundQuality;
    LTBOOL   m_bOldSoundQuality;

	CSliderCtrl		*m_pSoundVolumeCtrl;
	CSliderCtrl		*m_pMusicVolumeCtrl;
	CToggleCtrl	*m_pSoundQualityCtrl;

};

#endif // _FOLDER_AUDIO_H_