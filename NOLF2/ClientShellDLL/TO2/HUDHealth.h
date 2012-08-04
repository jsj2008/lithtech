// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealth.h
//
// PURPOSE : HUDItem to display player health
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_HEALTH_H
#define __HUD_HEALTH_H

#include "HUDItem.h"
#include "HUDBar.h"


//******************************************************************************************
//** HUD Health display
//******************************************************************************************
class CHUDHealth : public CHUDItem
{
public:
	CHUDHealth();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    LTIntPt		m_HealthBasePos;

    LTBOOL		m_bUseHealthBar;
    LTIntPt		m_HealthBarOffset;
	LTIntPt		m_ArmorBarOffset;

    LTBOOL		m_bUseHealthText;
    LTIntPt		m_HealthTextOffset;
	LTIntPt		m_ArmorTextOffset;

    LTBOOL		m_bUseHealthIcon;
    LTIntPt		m_HealthIconOffset;
    LTIntPt		m_ArmorIconOffset;
	uint8		m_nHealthIconSize;

	int			m_nBarHeight;
	uint8		m_nTextHeight;
    LTFLOAT		m_fBarScale;

	uint32		m_HealthColor;
	uint32		m_ArmorColor;


	LTPoly_GT4	m_Poly[2];
	HTEXTURE	m_hHealth;		// health icon
	HTEXTURE	m_hArmor;		// armor icon

	CHUDBar		m_HealthBar;
	CHUDBar		m_ArmorBar;



	CUIPolyString*	m_pHealthStr;
	CUIPolyString*	m_pArmorStr;

};


#endif