#ifndef __WEAPONSTRINGDEFS_H
#define __WEAPONSTRINGDEFS_H

#include "WeaponDefs.h"
#include "ClientRes.h"

inline char* GetWeaponString(RiotWeaponId nWeaponId)
{
	if (!g_pLTClient) return "Error";

	uint32 nStringID = IDS_UNUSED;

	switch (nWeaponId)
	{
		case GUN_PULSERIFLE_ID :
			nStringID = IDS_WEAPON_PULSERIFLE;
		break;

		case GUN_SPIDER_ID :
			nStringID = IDS_WEAPON_SPIDER;
		break;

		case GUN_BULLGUT_ID :
			nStringID = IDS_WEAPON_BULLGUT;
		break;

		case GUN_SNIPERRIFLE_ID :
			nStringID = IDS_WEAPON_SNIPERRIFLE;
		break;

		case GUN_JUGGERNAUT_ID :
			nStringID = IDS_WEAPON_JUGGERNAUT;
		break;

		case GUN_SHREDDER_ID :
			nStringID = IDS_WEAPON_SHREDDER;
		break;

		case GUN_REDRIOT_ID :
			nStringID = IDS_WEAPON_REDRIOT;
		break;

		case GUN_ENERGYBATON_ID :
			nStringID = IDS_WEAPON_ENERGYBATON;
		break;

		case GUN_ENERGYBLADE_ID :
			nStringID = IDS_WEAPON_ENERGYBLADE;
		break;

		case GUN_KATANA_ID :
			nStringID = IDS_WEAPON_KATANA;
		break;

		case GUN_MONOKNIFE_ID :
			nStringID = IDS_WEAPON_MONOKNIFE;
		break;


		// On-foot mode weapons...

		case GUN_COLT45_ID :
			nStringID = IDS_WEAPON_COLT45;
		break;

		case GUN_SHOTGUN_ID	:
			nStringID = IDS_WEAPON_SHOTGUN;
		break;

		case GUN_ASSAULTRIFLE_ID :
			nStringID = IDS_WEAPON_ASSAULTRIFLE;
		break;

		case GUN_ENERGYGRENADE_ID :
			nStringID = IDS_WEAPON_ENERGYGRENADE;
		break;

		case GUN_KATOGRENADE_ID :
			nStringID = IDS_WEAPON_KATOGRENADE;
		break;

		case GUN_MAC10_ID :
			nStringID = IDS_WEAPON_MAC10;
		break;

		case GUN_TOW_ID	:
			nStringID = IDS_WEAPON_TOW;
		break;

		case GUN_LASERCANNON_ID :
			nStringID = IDS_WEAPON_LASERCANNON;
		break;

		case GUN_SQUEAKYTOY_ID :
			nStringID = IDS_WEAPON_SQUEAKYTOY;
		break;

		case GUN_TANTO_ID :
			nStringID = IDS_WEAPON_TANTO;
		break;

		default : break;
	}

	HSTRING hStr = g_pLTClient->FormatString (nStringID);
	if (!hStr) return "Error";

	static char strWeapon[128];
	SAFE_STRCPY(strWeapon, g_pLTClient->GetStringData (hStr));
	g_pLTClient->FreeString (hStr);

	return strWeapon;
}

inline char* GetAmmoString(RiotWeaponId nWeaponId)
{
	if (!g_pLTClient) return "Error";

	uint32 nStringID = IDS_UNUSED;

	switch (nWeaponId)
	{
		// Mech mode weapons...	
	
		case GUN_PULSERIFLE_ID :
			nStringID = IDS_AMMO_PULSERIFLE;
		break;

		case GUN_SPIDER_ID :
			nStringID = IDS_AMMO_SPIDER;
		break;

		case GUN_BULLGUT_ID :
			nStringID = IDS_AMMO_BULLGUT;
		break;

		case GUN_SNIPERRIFLE_ID :
			nStringID = IDS_AMMO_SNIPERRIFLE;
		break;

		case GUN_JUGGERNAUT_ID :
			nStringID = IDS_AMMO_JUGGERNAUT;
		break;

		case GUN_SHREDDER_ID :
			nStringID = IDS_AMMO_SHREDDER;
		break;

		case GUN_REDRIOT_ID :
			nStringID = IDS_AMMO_REDRIOT;
		break;

		// On-foot mode weapons...

		case GUN_COLT45_ID :
			nStringID = IDS_AMMO_COLT45;
		break;

		case GUN_SHOTGUN_ID	:
			nStringID = IDS_AMMO_SHOTGUN;
		break;

		case GUN_ASSAULTRIFLE_ID :
			nStringID = IDS_AMMO_ASSAULTRIFLE;
		break;

		case GUN_ENERGYGRENADE_ID :
			nStringID = IDS_AMMO_ENERGYGRENADE;
		break;

		case GUN_KATOGRENADE_ID :
			nStringID = IDS_AMMO_KATOGRENADE;
		break;

		case GUN_MAC10_ID :
			nStringID = IDS_AMMO_MAC10;
		break;

		case GUN_TOW_ID	:
			nStringID = IDS_AMMO_TOW;
		break;

		case GUN_LASERCANNON_ID :
			nStringID = IDS_AMMO_LASERCANNON;
		break;

		default : break;
	}

	HSTRING hStr = g_pLTClient->FormatString (nStringID);
	if (!hStr) return "Error";

	static char strAmmo[128];
	SAFE_STRCPY(strAmmo, g_pLTClient->GetStringData (hStr));
	g_pLTClient->FreeString (hStr);

	return strAmmo;
}

inline RiotWeaponId GetWeaponId(char* pWeaponName)
{
	if (!pWeaponName || strlen(pWeaponName) < 5) return GUN_NONE;

	for(int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (_stricmp(pWeaponName, GetWeaponString((RiotWeaponId)i)) == 0)
		{
			return ((RiotWeaponId)i);
		}
	}

	return GUN_NONE;
}

#endif
