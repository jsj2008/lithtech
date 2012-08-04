// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealthEnergy.h
//
// PURPOSE : HUDItem to display countdown timer
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_HEALTHENERGY_H
#define __HUD_HEALTHENERGY_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD countdown timer display
//******************************************************************************************
class CHUDHealthEnergy : public CHUDItem
{
public:
	CHUDHealthEnergy();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	float		m_fScale;

	// Local copies only maintained to see if something has changed
	uint32		m_iHealth;
	uint32		m_iHealthMax;
	uint32		m_iEnergy;
	uint32		m_iEnergyMax;

	// FIXME add rects (XYWH) for healthBG, energyBG, healthbar, energybar
	uint32		m_iHealthWidth;		// width of the bar at full health
	uint32		m_iEnergyWidth;		// width of the bar at full energy

	bool		m_bHealthBlink;
	float		m_fHealthBlinkTime;
	float		m_fLayoutHealthBlinkTime;

	HTEXTURE	m_hHUDTex;
	HTEXTURE	m_hHUDHubTex;

	LT_POLYFT4	m_HUDHub;
	LT_POLYFT4	m_HealthBG;
	LT_POLYFT4	m_EnergyBG;
	LT_POLYFT4	m_HealthBar;
	LT_POLYFT4	m_EnergyBar;
	LT_POLYFT4	m_HealthBlinkBar;

	LT_POLYF4	m_EnergyTransfer;

	int			m_iFontNum;

	CUIPolyString * m_pHealthStr;
	CUIPolyString * m_pEnergyStr;
};

#endif