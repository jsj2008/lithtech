// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.cpp
//
// PURPOSE : Interface screen for setting audio options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_AUDIO_H_
#define _SCREEN_AUDIO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenAudio : public CBaseScreen
{
public:
	CScreenAudio();
	virtual ~CScreenAudio();

	// Build the screen
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);

	// Override the left and right controls so that the volumes can be changed
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnEnter();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

protected:
	void	ConfirmQualityChange(bool bMusic);
	void	SaveSoundSettings();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
	int		m_nSoundVolume;
	int		m_nMusicVolume;
	int		m_nSpeechVolume;
    LTBOOL  m_bSoundQuality;
    LTBOOL  m_bMusicQuality;
    LTBOOL  m_bMusicEnabled;

	uint8	m_nPerformance;


	CLTGUISlider	*m_pSoundVolumeCtrl;
	CLTGUISlider	*m_pMusicVolumeCtrl;
	CLTGUISlider	*m_pSpeechVolumeCtrl;
	CLTGUIToggle	*m_pSoundQualityCtrl;
	CLTGUIToggle	*m_pMusicQualityCtrl;

};

#endif // _SCREEN_AUDIO_H_