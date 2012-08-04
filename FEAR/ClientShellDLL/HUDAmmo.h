// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmo.h
//
// PURPOSE : HUDItem to display player ammo
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_AMMO_H
#define __HUD_AMMO_H

#include "HUDItem.h"
//#include "HUDBar.h"

//******************************************************************************************
//** HUD Ammo display
//******************************************************************************************
class CHUDAmmo : public CHUDItem
{
public:
	CHUDAmmo();

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();

    virtual void	UpdateLayout();

private:

	bool		m_bDraw;
	bool		m_bInfinite;

	HAMMO		m_hLastAmmo;


};

#endif