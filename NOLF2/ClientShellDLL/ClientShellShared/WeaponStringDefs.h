//-------------------------------------------------------------------------
//
// MODULE  : WeaponStringDefs.h
//
// PURPOSE : Utility functions to retrieve the strings associated with
//				weapons and ammo
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __WEAPON_STRING_DEFS_H__
#define __WEAPON_STRING_DEFS_H__

#include "ClientResShared.h"
#include "WeaponMgr.h"

#define	  WS_ERROR_STRING	"Error"

#include "iclientshell.h"

inline char* GetWeaponString(uint8 nWeaponId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return WS_ERROR_STRING;

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return WS_ERROR_STRING;

	return pWeapon->szShortName;
}

inline char* GetAmmoString(int nAmmoId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return WS_ERROR_STRING;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return WS_ERROR_STRING;

	return pAmmo->szShortName;
}

#endif // __WEAPON_STRING_DEFS_H__