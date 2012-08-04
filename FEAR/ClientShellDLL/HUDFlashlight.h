// ----------------------------------------------------------------------- //
//
// MODULE  : HUDFlashlight.h
//
// PURPOSE : HUDItem to display flashlight bar
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_FLASHLIGHT_H
#define __HUD_FLASHLIGHT_H

#include "HUDBar.h"


//******************************************************************************************
//** HUD Flashlight display
//******************************************************************************************
class CHUDFlashlight : public CHUDItem
{
public:
	CHUDFlashlight();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();
	virtual void	UpdateFade();
	virtual void	UpdateFlicker();
	virtual void	EndFlicker();
	virtual void	UpdateFlash();

	void			ResetBar();

private:
	void			UpdateBar();

	uint32				m_nFadeThreshold;
	CHUDBar				m_Bar;
	LTRect2f			m_rfBarRect;
	float				m_fBarWidth;


	//runtime info
	bool		m_bDraw;
	float		m_fDuration;

	float				m_fLastValue;


};

#endif