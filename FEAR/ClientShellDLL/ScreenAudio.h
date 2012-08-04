// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.h
//
// PURPOSE : Interface screen for setting audio options
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENAUDIO_H__
#define __SCREENAUDIO_H__

#include "BaseScreen.h"

class CScreenAudio : public CBaseScreen
{
public:
	CScreenAudio();
	virtual ~CScreenAudio();

	// Build the screen
	virtual bool	Build();
	virtual void	OnFocus(bool bFocus);

	// Override the left and right controls so that the volumes can be changed
	virtual bool	OnLeft();
	virtual bool	OnRight();
	virtual bool	OnEnter();
	virtual bool	OnLButtonUp(int x, int y);
	virtual bool	OnRButtonUp(int x, int y);

protected:
	void	SaveSoundSettings();

	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
	int		m_nSoundVolume;
	int		m_nSpeechVolume;
	int		m_nMusicVolume;

	bool	m_bEAXEnable;
	bool	m_bEAX4Enable;
	bool	m_bHWSoundsEnable;

	CLTGUISlider	*m_pSoundVolumeCtrl;
	CLTGUISlider	*m_pSpeechVolumeCtrl;
	CLTGUISlider	*m_pMusicVolumeCtrl;

	CLTGUIToggle	*m_pEAXEnableCtrl;
	CLTGUIToggle	*m_pEAX4EnableCtrl;
	CLTGUIToggle	*m_pHWSoundsEnableCtrl;
};


#endif  // __SCREENAUDIO_H__
