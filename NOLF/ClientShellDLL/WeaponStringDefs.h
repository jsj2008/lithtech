#ifndef __WEAPON_STRING_DEFS_H__
#define __WEAPON_STRING_DEFS_H__

#include "ClientRes.h"
#include "WeaponMgr.h"

#define	  WS_ERROR_STRING	"Error"

extern ILTClient *g_pLTClient;

inline char* GetWeaponString(uint8 nWeaponId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return WS_ERROR_STRING;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return WS_ERROR_STRING;

    uint32 nStringID = pWeapon->nNameId;

    HSTRING hStr = g_pLTClient->FormatString (nStringID);
	if (!hStr) return WS_ERROR_STRING;

	static char strWeapon[128];
    SAFE_STRCPY(strWeapon, g_pLTClient->GetStringData(hStr));
    g_pLTClient->FreeString (hStr);

	return strWeapon;
}

inline char* GetAmmoString(int nAmmoId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return WS_ERROR_STRING;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return WS_ERROR_STRING;

    uint32 nStringID = pAmmo->nNameId;

    HSTRING hStr = g_pLTClient->FormatString(nStringID);
	if (!hStr) return WS_ERROR_STRING;

	static char strAmmo[128];
    SAFE_STRCPY(strAmmo, g_pLTClient->GetStringData(hStr));
    g_pLTClient->FreeString(hStr);

	return strAmmo;
}

#endif // __WEAPON_STRING_DEFS_H__