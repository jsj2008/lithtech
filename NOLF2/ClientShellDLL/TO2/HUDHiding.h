// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHiding.h
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_HIDING_H
#define __HUD_HIDING_H

#include "HUDItem.h"
#include "HUDHidingBar.h"

//******************************************************************************************
//** HUD Hiding display
//******************************************************************************************
class CHUDHiding : public CHUDItem
{
public:
	CHUDHiding();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:

	enum eHidingPrims
	{
		kHidePrim,
		kHiddenPrim,
		kCantHidePrim,
		
		kNumPrims
	};

    LTIntPt		m_BasePos;
    LTIntPt		m_BaseSize;
	LTFLOAT		m_fIconSize;

	LTFLOAT		m_bDraw;
	
	LTPoly_GT4	m_Poly[kNumPrims];
	HTEXTURE	m_hIcon[kNumPrims];	//  icon

	float		m_fHideDuration;
	float		m_fHideTimer;
	float		m_fHideBarWaitTime;	
	
	CHUDHidingBar		m_HideBar;
};

#endif