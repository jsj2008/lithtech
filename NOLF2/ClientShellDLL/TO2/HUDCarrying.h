// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCarrying.h
//
// PURPOSE : HUDItem to display an icon while carrying a body
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CARRYING_H
#define __HUD_CARRYING_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD Carry Icon display
//******************************************************************************************
class CHUDCarrying : public CHUDItem
{
public:
	CHUDCarrying();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();


private:
    LTIntPt		m_BasePos;
	uint16		m_nSize;

	enum	eCarryIcons
	{
		eBodyCanDrop,
		eBodyNoDrop,
		eBodyCanCarry,
		eDD_TransmitterCanDrop,
		eDD_TransmitterCanCarry,
		eDD_BatteryCanDrop,
		eDD_BatteryCanCarry,
		eDD_CoreCanDrop,
		eDD_CoreCanCarry,
		kNumCarryIcons

	};


	LTPoly_GT4	m_Poly;
	HTEXTURE	m_hIcon[kNumCarryIcons];


};

#endif