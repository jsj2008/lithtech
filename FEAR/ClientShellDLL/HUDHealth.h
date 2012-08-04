// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealth.h
//
// PURPOSE : Definitition of HUD Health display
//
// CREATED : 01/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDHEALTH_H__
#define __HUDHEALTH_H__

#include "HUDItem.h"


//******************************************************************************************
//** HUD Health display
//******************************************************************************************
class CHUDHealth : public CHUDItem
{
public:
	CHUDHealth();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	Reset();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();
	virtual void	UpdateFade();
	virtual void	UpdateFlicker();
	virtual void	EndFlicker();
	virtual void	UpdateFlash();

	virtual void	OnExitWorld();


private:
	void			UpdateBar();

	uint32				m_nFadeThreshold;
	uint32				m_nFlashThreshold;

	uint32				m_cNormalTextColor;
	uint32				m_cNormalIconColor;
	uint32				m_cThresholdColor;

	LTPoly_GT4			m_BarPoly;
	TextureReference	m_hBarTexture;
	float				m_fBarWidth;

	uint8				m_nLastValue;
	uint8				m_nLastMaxValue;

	CLTGUIString		m_HelpText;
	LTVector2n			m_vHelpOffset;


};


#endif  // __HUDHEALTH_H__
