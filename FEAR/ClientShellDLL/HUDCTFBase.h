// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCTFBase.h
//
// PURPOSE : HUDItem to display team CTF flag base status.
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDCTFBASE_H__
#define __HUDCTFBASE_H__

#include "HUDItem.h"

//******************************************************************************************
//** HUD CTF Flag base.
//******************************************************************************************
class CHUDCTFBase : public CHUDItem
{
public:
	CHUDCTFBase();

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

	void			SetTeamId( uint8 nValue ) { m_nTeamId = nValue; Update( ); }
	uint8			GetTeamId( ) const { return m_nTeamId; }

	virtual void		OnExitWorld();

protected:

	virtual char const* GetLayoutRecordName( ) const = 0;
	virtual bool IsFriendly( ) const = 0;

private:

	bool			m_bDraw;
	bool			m_bUpdated;
	uint8			m_nTeamId;
	bool			m_bHasFlag;
};

class CHUDCTFBaseFriendly : public CHUDCTFBase
{
protected:
	char const* GetLayoutRecordName( ) const { return "HUDCTFBaseFriendly"; }
	virtual bool IsFriendly( ) const { return true; }
};

class CHUDCTFBaseEnemy : public CHUDCTFBase
{
protected:
	char const* GetLayoutRecordName( ) const { return "HUDCTFBaseEnemy"; }
	virtual bool IsFriendly( ) const { return false; }
};


#endif