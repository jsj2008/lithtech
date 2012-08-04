// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDoomsday.h
//
// PURPOSE : HUDItem to display status of doomsday pieces
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DOOM_H
#define __HUD_DOOM_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD Carry Icon display
//******************************************************************************************
class CHUDDoomsday : public CHUDItem
{
public:
	CHUDDoomsday();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();


private:
    LTIntPt		m_BasePos;
	uint16		m_nSize;
	
	enum	eDDIcons
	{
		eDD_Transmitter,
		eDD_Core,
		eDD_Battery,
		kNumDDIcons

	};


	LTPoly_GT4	m_Poly[kNumDDIcons];
	HTEXTURE	m_hIcon[kNumDDIcons][3];
	bool		m_bBlink[kNumDDIcons];
	uint8		m_nTeam[kNumDDIcons];


};

#endif