// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSlowMo.h
//
// PURPOSE : HUDItem to display slow mo bar
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SLOWMO_H
#define __HUD_SLOWMO_H

#include "HUDBar.h"


//******************************************************************************************
//** HUD Slow Mo display
//******************************************************************************************
class CHUDSlowMo : public CHUDItem
{
public:
	CHUDSlowMo();

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

	virtual void	OnExitWorld();


	void			ResetBar();

private:
	void			UpdateBar();

	uint32				m_nFadeThreshold;
	CHUDBar				m_Bar;
	LTRect2f			m_rfBarRect;
	float				m_fBarWidth;

	//layout info
	uint32	m_cDisabledColor;
	uint32	m_cSavedTextColor;


	//runtime info
	bool		m_bDraw;
	float		m_fDuration;

	TextureReference	m_hRechargeTexture;
	LTVector2n			m_vRechargeBasePos;
	LTVector2n			m_vRechargeSize;
	LTPoly_GT4			m_RechargePoly;
	uint32				m_cRechargeColor;

	float				m_fLastValue;
	float				m_fCurrentMax;
	float				m_fGrowth;


};

#endif