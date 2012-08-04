// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRespawn.h
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_RESPAWN_H
#define __HUD_RESPAWN_H

#include "HUDItem.h"
#include "HUDMeter.h"

class CHUDRespawnBar : public CHUDMeter
{
	public: // Methods...

		CHUDRespawnBar();
	
		virtual void	Update();
		virtual void	UpdateLayout();
};


//******************************************************************************************
//** HUD Respawn display
//******************************************************************************************
class CHUDRespawn : public CHUDItem
{
public:
	CHUDRespawn();

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

    virtual void	UpdateLayout();

private:


	//layout info
	uint32		m_cDisabledColor;
	uint32		m_cPenaltyColor;

	LTVector2n	m_vTimerOffset;
	LTVector2n	m_vPenaltyOffset;


	//runtime info
	bool			m_bDraw;

	float		m_fDuration;
	
	CHUDRespawnBar		m_RespawnBar;
	CLTGUIString		m_TimerText;
	CLTGUIString		m_PenaltyText;
	CFontInfo			m_sTimerFont;
	CFontInfo			m_sPenaltyFont;
	HTEXTURESTRING		m_hTimerSourceString;
};

#endif