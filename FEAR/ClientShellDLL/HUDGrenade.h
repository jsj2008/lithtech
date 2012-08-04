// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGrenade.h
//
// PURPOSE : HUDItem to display player grenades
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_GRENADE_H
#define __HUD_GRENADE_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ammo display
//******************************************************************************************
class CHUDGrenade : public CHUDItem
{
public:
	CHUDGrenade();

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

	uint32			m_nCount;
	bool			m_bDraw;
	bool			m_bInfinite;

	LTPoly_GT4			m_BarPoly;
	TextureReference	m_hBarTexture;
	float				m_fBarWidth;



};

#endif