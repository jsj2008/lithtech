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

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    LTIntPt		m_BasePos;

    LTBOOL		m_bUseBar;
    LTIntPt		m_BarOffset;

    LTBOOL		m_bUseText;
    LTIntPt		m_TextOffset;

    LTBOOL		m_bUseIcon;
    LTIntPt		m_IconOffset;
	uint8		m_nIconSize;

	int			m_nBarHeight;
	uint8		m_nTextHeight;
    LTFLOAT		m_fBarScale;

	uint32		m_Color;

	LTBOOL		m_bDraw;


	LTPoly_GT4	m_Poly;
	HTEXTURE	m_hIcon;			//  icon

	CHUDBar		m_Bar;


	CUIPolyString*	m_pStr;

};

#endif