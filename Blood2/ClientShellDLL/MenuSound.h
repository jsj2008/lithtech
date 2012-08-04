// MenuSound.h: interface for the CMenuSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUSOUND_H__2AEB32F1_615A_11D2_BDA6_0060971BDC6D__INCLUDED_)
#define AFX_MENUSOUND_H__2AEB32F1_615A_11D2_BDA6_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuSound : public CMenuBase  
{
public:
	CMenuSound();
	virtual ~CMenuSound();

	// Build the menu
	void	Build();			

	// Override the left and right controls so that the volumes can be changed
	void	OnLeft();
	void	OnRight();
	void	OnFocus(DBOOL bFocus);

protected:
	void	LoadSoundSettings();
	void	SaveSoundSettings();

	// Enable/Disable the volume controls based on m_bSoundOn and m_bMusicOn
	void	EnableDisableControls();

protected:
	DBOOL	m_bSoundOn;
	DBOOL	m_bMusicOn;
	int		m_nSoundVolume;
	int		m_nMusicVolume;
	DBOOL	m_bSoundQuality;
	DBOOL	m_bOldSoundQuality;

	CLTGUISliderCtrl	*m_pSoundVolumeCtrl;
	CLTGUISliderCtrl	*m_pMusicVolumeCtrl;
	CLTGUIOnOffCtrl		*m_pSoundQualityCtrl;

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUSOUND_H__2AEB32F1_615A_11D2_BDA6_0060971BDC6D__INCLUDED_)
