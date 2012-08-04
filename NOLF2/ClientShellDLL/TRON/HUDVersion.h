// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHiding.h
//
// PURPOSE : HUDItem to display Jet's version number
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_VERSION_H
#define __HUD_VERSION_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD program version display
//******************************************************************************************
class CHUDVersion : public CHUDItem
{
public:
	CHUDVersion();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    LTIntPt		m_BasePos;
	LTVector	m_vColor;
	CUIPolyString * m_pString;
	uint16		m_iVersion;
	float		m_fUpdateTime;
	float		m_fTimeRemaining;
};

#endif