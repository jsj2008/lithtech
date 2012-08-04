// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.h
//
// PURPOSE : Definitition of HUD Armor display
//
// CREATED : 01/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDARMOR_H__
#define __HUDARMOR_H__

#include "HUDItem.h"


//******************************************************************************************
//** HUD Armor display
//******************************************************************************************
class CHUDArmor : public CHUDItem
{
public:
	CHUDArmor();

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

private:
	void			UpdateBar();

	uint32				m_nFadeThreshold;
	LTPoly_GT4			m_BarPoly;
	TextureReference	m_hBarTexture;
	float				m_fBarWidth;

	uint8				m_nLastValue;

};


#endif  // __HUDARMOR_H__
