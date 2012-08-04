// ----------------------------------------------------------------------- //
//
// MODULE  : HUDObjectives.h
//
// PURPOSE : HUDItem to indicate that you've received a new objective
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_OBJECTIVES_H
#define __HUD_OBJECTIVES_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD Carry Icon display
//******************************************************************************************
class CHUDObjectives : public CHUDItem
{
public:
	CHUDObjectives();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	void		Show(bool bShow);


private:
	bool		m_bShow;

    LTIntPt		m_BasePos;
	LTIntPt		m_BaseSize;

	float		m_fBlinkSpeed;
	float		m_fBlinkTime;
	float		m_fBlinkDuration;
	float		m_fAlpha;
	bool		m_bBlink;
	float		m_fFadeDir;

	LTPoly_GT4	m_Poly;
	HTEXTURE	m_hIcon;


};

#endif