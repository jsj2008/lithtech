// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAir.h
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_AIR_H
#define __HUD_AIR_H

#include "HUDItem.h"
#include "HUDBar.h"

//******************************************************************************************
//** HUD Air display
//******************************************************************************************
class CHUDAir : public CHUDItem
{
public:
	CHUDAir();

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
    virtual void	Update();
	virtual void    ScaleChanged();
    virtual void	UpdateLayout();

private:
	void	UpdateBar();

	LTRect2n	m_rBar;
	bool		m_bDraw;

	CHUDBar		m_Bar;

};

#endif