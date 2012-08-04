// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.h
//
// PURPOSE : HUDItem to display Jet's current Weapon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_WEAPON_H
#define __HUD_WEAPON_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Weapon display
//******************************************************************************************
class CHUDWeapon : public CHUDItem
{
public:
	CHUDWeapon();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		SetUVWH(LT_POLYFT4 * pPrim, LTRect rect);
	float		m_fScale;

	uint8		m_nWeaponID;

	LT_POLYFT4	m_BasePrim;
	LT_POLYFT4	m_WeaponPrim;
	LT_POLYFT4	m_AmmoPrim;

	HTEXTURE	m_hHUDTex;
	HTEXTURE	m_hWeaponTex;

	LTRect		m_rcBackground;
	LTRect		m_rcAmmo;
};

#endif