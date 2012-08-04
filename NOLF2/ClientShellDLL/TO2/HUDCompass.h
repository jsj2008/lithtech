// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCompass.h
//
// PURPOSE : HUDItem to display a compass
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_COMPASS_H
#define __HUD_COMPASS_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD Compass display
//******************************************************************************************
class CHUDCompass : public CHUDItem
{
public:
	CHUDCompass();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	void		Toggle() {m_bDraw = !m_bDraw;}
	void		SetDraw( bool bDraw ) { m_bDraw = bDraw; }
	bool		GetDraw( ) { return m_bDraw; }

private:
    LTIntPt		m_BasePos;
	uint16		m_nSize;

	bool		m_bDraw;
	float		m_fLastRotation;


	LTPoly_GT4	m_Poly[2];
	HTEXTURE	m_hBack;
	HTEXTURE	m_hNeedle;


};

#endif