// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapons.h
//
// PURPOSE : HUDItem to display player damage icons
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_WEAPONS_H
#define __HUD_WEAPONS_H

#include "HUDItem.h"


class CHUDWeapons : public CHUDItem
{
public:
	CHUDWeapons();
	~CHUDWeapons();

    LTBOOL      Init();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    void        UpdateStyle();


private:
	LTBOOL		m_bDraw;

	LT_POLYGT4		m_Poly[10];
	HTEXTURE		m_hIcon[10];			//  icon
	CUIPolyString*	m_pText;

	float		m_fAlpha;
	LTBOOL		m_bLargeIcons;
	LTBOOL		m_bNumbers;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint8		m_nLargeFontSize;		// The font size before scaling (for large icons)
	uint32		m_nTextColor;

	uint8		m_nIconSize;			// The scaled icon size to use.
	uint8		m_nBaseIconSize;		// The icon size before scaling
	uint8		m_nLargeIconSize;		// The icon size before scaling (for large icons)
	
};

#endif