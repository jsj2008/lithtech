// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCTFFlag.h
//
// PURPOSE : HUDItem to display player has flag.
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDCTFFLAG_H__
#define __HUDCTFFLAG_H__

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ammo display
//******************************************************************************************
class CHUDCTFFlag : public CHUDItem
{
public:
	CHUDCTFFlag();

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

	void			SetHasFlag( bool bValue ) { m_bHasFlag = bValue; Update( ); }
	bool			GetHasFlag( ) const { return m_bHasFlag; }

	virtual void		OnExitWorld();

private:

	bool			m_bUpdated;
	bool			m_bHasFlag;
};

#endif