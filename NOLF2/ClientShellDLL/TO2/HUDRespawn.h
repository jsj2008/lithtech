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

#include "HUDMeter.h"

class CHUDRespawnBar : public CHUDMeter
{
	public: // Methods...

		CHUDRespawnBar();
	
		void	Update();
		void	UpdateLayout();
};


//******************************************************************************************
//** HUD Respawn display
//******************************************************************************************
class CHUDRespawn : public CHUDItem
{
public:
	CHUDRespawn();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:


	LTIntPt m_StrPos;
	uint8	m_nStrSz;
	uint32	m_StrColor;
	uint32	m_StrDisColor;


	LTFLOAT		m_bDraw;

	CUIFormattedPolyString* m_pString;
	

	float		m_fDuration;
	bool		m_bReady;
	bool		m_bCancelRevive;
	
	CHUDRespawnBar		m_RespawnBar;
};

#endif