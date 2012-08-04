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
#include "HUDBar.h"

//******************************************************************************************
//** HUD Ammo display
//******************************************************************************************
class CHUDAmmo : public CHUDItem
{
public:
	CHUDAmmo();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:

    LTIntPt		m_BasePos;

    LTBOOL		m_bUseBar;
    LTIntPt		m_BarOffset;
    LTIntPt		m_ClipOffset;
    LTIntPt		m_ClipUnitSize;

    LTBOOL		m_bUseText;
    LTIntPt		m_TextOffset;
	uint32		m_TextColor;
    
	LTIntPt		m_IconOffset;
	uint8		m_nIconSize;

	int			m_nBarHeight;
	uint8		m_nTextHeight;
    LTFLOAT		m_fBarScale;

	LTPoly_GT4	m_Poly[2];
	HTEXTURE	m_hFull;		// clip
	HTEXTURE	m_hEmpty;		// clip
	HTEXTURE	m_hIcon;

	CHUDBar		m_Bar;

	uint8		m_nEmpty;
	uint8		m_nFull;

	LTBOOL		m_bDraw;
	LTBOOL		m_bInfinite;

	CUIPolyString*	m_pStr;

};

#endif