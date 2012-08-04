// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.h
//
// PURPOSE : HUDItem to display Jet's Armor
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ARMOR_H
#define __HUD_ARMOR_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
class CHUDArmor : public CHUDItem
{
public:
	CHUDArmor();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	float		m_fScale;

	bool		m_bHavePiece[8];

	LT_POLYFT4	m_Base;
	LT_POLYFT4	m_ArmorPiece[8];

	HTEXTURE	m_hBaseTex;
	HTEXTURE	m_hArmorPieceTex[8];
};

#endif