// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDefs.h
//
// PURPOSE : Definitions for weapon types
//
// CREATED : 9/35/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_DEFS_H__
#define __WEAPON_DEFS_H__

#include "clientheaders.h"
#include "RiotCommandIds.h"
#include "PlayerModeTypes.h"
#include "WeaponFXTypes.h"
#include "ProjectileFXTypes.h"
#include "ModelFuncs.h"

#include <stdio.h>

#define WEAPON_MIN_IDLE_TIME	5.0f
#define WEAPON_MAX_IDLE_TIME	15.0f
#define WEAPON_KEY_FIRE			"FIRE_KEY"
#define WEAPON_KEY_SOUND		"SOUND_KEY"
#define WEAPON_SOUND_RADIUS		2000.0f
#define WEAPON_SOUND_DRYFIRE	1
#define WEAPON_SOUND_KEY		2
#define WEAPON_SOUND_FIRE		3


extern char s_FileBuffer[_MAX_PATH];

// This is defined in both "RiotObjectUtilities.h" and "ClientUtilities.h"
int GetRandom(int lo, int hi);


enum WeaponState { W_IDLE, W_BEGIN_FIRING, W_FIRING, W_FIRED, W_FIRED2, 
				   W_END_FIRING, W_RELOADING, W_FIRING_NOAMMO, W_SELECT, 
				   W_DESELECT };

enum ProjectileType { VECTOR=0, PROJECTILE, CANNON, MELEE };

// Defines....

enum RiotWeaponId {

	GUN_FIRST_ID = 0,
	GUN_FIRSTMECH_ID = 0,
	GUN_PULSERIFLE_ID = 0,
	GUN_LASERCANNON_ID = 1,
	GUN_SPIDER_ID = 2,
	GUN_BULLGUT_ID = 3,
	GUN_SNIPERRIFLE_ID = 4,
	GUN_JUGGERNAUT_ID = 5,
	GUN_SHREDDER_ID = 6,
	GUN_UNUSED1_ID = 7,		// Left in to decrease impact on level designers 
	GUN_REDRIOT_ID = 8,
	GUN_ENERGYBATON_ID = 9,
	GUN_ENERGYBLADE_ID = 10,
	GUN_KATANA_ID = 11,
	GUN_MONOKNIFE_ID = 12,
	GUN_LASTMECH_ID = 12,

	GUN_FIRSTONFOOT_ID = 13,
	GUN_COLT45_ID = 13,
	GUN_SHOTGUN_ID = 14,
	GUN_ASSAULTRIFLE_ID = 15,
	GUN_ENERGYGRENADE_ID = 16,
	GUN_KATOGRENADE_ID = 17,
	GUN_MAC10_ID = 18,
	GUN_TOW_ID = 19,
	GUN_UNUSED2_ID = 20,	// Left in to decrease impact on level designers
	GUN_SQUEAKYTOY_ID = 21,
	GUN_TANTO_ID = 22,
	GUN_LASTONFOOT_ID = 22,
	GUN_MAX_NUMBER = 23,

	GUN_NONE = 50
};


inline LTBOOL FiredWeapon(WeaponState eState)
{
	if (eState == W_FIRED || eState == W_FIRED2) return LTTRUE;

	return LTFALSE;
}


inline LTBOOL IsOneHandedWeapon(RiotWeaponId nWeaponId)
{
	LTBOOL bRet = LTFALSE;

	switch (nWeaponId)
	{
		case GUN_SQUEAKYTOY_ID :
		case GUN_ENERGYBATON_ID :
		case GUN_ENERGYBLADE_ID :
		case GUN_KATANA_ID :
		case GUN_MONOKNIFE_ID :
		case GUN_TANTO_ID :
			bRet = LTTRUE;
		break;

		default : break;
	}

	return bRet;
}


inline ProjectileType GetWeaponType(RiotWeaponId nWeaponId)
{
	ProjectileType eType = VECTOR;

	switch (nWeaponId)
	{
		case GUN_SNIPERRIFLE_ID :
		case GUN_COLT45_ID :
		case GUN_SHOTGUN_ID	:
		case GUN_ASSAULTRIFLE_ID :
		case GUN_MAC10_ID :
		case GUN_LASERCANNON_ID :
		case GUN_SQUEAKYTOY_ID :
			eType = VECTOR;
		break;

		case GUN_PULSERIFLE_ID :
		case GUN_BULLGUT_ID :
		case GUN_SPIDER_ID :
		case GUN_ENERGYGRENADE_ID :
		case GUN_KATOGRENADE_ID :
		case GUN_TOW_ID	:
			eType = PROJECTILE;
		break;

		case GUN_REDRIOT_ID :
		case GUN_SHREDDER_ID :
		case GUN_JUGGERNAUT_ID :
			eType = CANNON;
		break;

		case GUN_ENERGYBATON_ID :
		case GUN_ENERGYBLADE_ID :
		case GUN_KATANA_ID :
		case GUN_MONOKNIFE_ID :
		case GUN_TANTO_ID :
			eType = MELEE;
		break;

		default : break;
	}

	return eType;
}

// Map command to weapon id.
inline int GetWeaponId(int nCommandId, uint32 dwPlayerMode)
{
	int nWeaponId = -1;

	LTBOOL bOnFoot = (dwPlayerMode == PM_MODE_FOOT) ? LTTRUE : LTFALSE;

	switch (nCommandId)
	{
		case COMMAND_ID_WEAPON_1 :
			if (bOnFoot) nWeaponId = GUN_COLT45_ID;
			else nWeaponId = GUN_PULSERIFLE_ID;
		break;

		case COMMAND_ID_WEAPON_2 :
			if (bOnFoot)  nWeaponId = GUN_SHOTGUN_ID;
			else nWeaponId = GUN_LASERCANNON_ID;
		break;

		case COMMAND_ID_WEAPON_3 :
			if (bOnFoot) nWeaponId = GUN_MAC10_ID;
			else nWeaponId = GUN_SPIDER_ID;
		break;

		case COMMAND_ID_WEAPON_4 :
			if (bOnFoot) nWeaponId = GUN_ASSAULTRIFLE_ID;
			else nWeaponId = GUN_BULLGUT_ID;
		break;

		case COMMAND_ID_WEAPON_5 :
			if (bOnFoot) nWeaponId = GUN_ENERGYGRENADE_ID;
			else nWeaponId = GUN_SNIPERRIFLE_ID;
		break;

		case COMMAND_ID_WEAPON_6 :
			if (bOnFoot) nWeaponId = GUN_KATOGRENADE_ID;
			else nWeaponId = GUN_JUGGERNAUT_ID;
		break;

		case COMMAND_ID_WEAPON_7 :
			if (bOnFoot) nWeaponId = GUN_TOW_ID;
			else nWeaponId = GUN_SHREDDER_ID;
		break;

		case COMMAND_ID_WEAPON_8 :
			if (bOnFoot) nWeaponId = GUN_SQUEAKYTOY_ID;
			else nWeaponId = GUN_REDRIOT_ID;
		break;

		case COMMAND_ID_WEAPON_9 :	
		break;
		
		case COMMAND_ID_WEAPON_10 :
		{
			switch (dwPlayerMode)
			{
				case PM_MODE_FOOT :
					nWeaponId = GUN_TANTO_ID;
				break;

				case PM_MODE_MCA_AP :
					nWeaponId = GUN_ENERGYBATON_ID;
				break;

				case PM_MODE_MCA_UE :
					nWeaponId = GUN_ENERGYBLADE_ID;
				break;

				case PM_MODE_MCA_AO :
					nWeaponId = GUN_MONOKNIFE_ID;
				break;

				case PM_MODE_MCA_SA :
					nWeaponId = GUN_KATANA_ID;
				break;

				default : break;
			}
		}
		break;

		default : 
		break;
	}

	return nWeaponId;
}

// Map weapon id to command id.
inline int GetCommandId(int nWeaponId)
{
	int nCommandId = -1;

	switch (nWeaponId)
	{
		case GUN_COLT45_ID :
		case GUN_PULSERIFLE_ID :
			nCommandId = COMMAND_ID_WEAPON_1;
			break;

		case GUN_SHOTGUN_ID :
		case GUN_LASERCANNON_ID :
			nCommandId = COMMAND_ID_WEAPON_2;
			break;

		case GUN_MAC10_ID :
		case GUN_SPIDER_ID :
			nCommandId = COMMAND_ID_WEAPON_3;
			break;

		case GUN_ASSAULTRIFLE_ID :
		case GUN_BULLGUT_ID :
			nCommandId = COMMAND_ID_WEAPON_4;
			break;

		case GUN_ENERGYGRENADE_ID :
		case GUN_SNIPERRIFLE_ID :
			nCommandId = COMMAND_ID_WEAPON_5;
			break;

		case GUN_KATOGRENADE_ID :
		case GUN_JUGGERNAUT_ID :
			nCommandId = COMMAND_ID_WEAPON_6;
			break;

		case GUN_TOW_ID :
		case GUN_SHREDDER_ID :
			nCommandId = COMMAND_ID_WEAPON_7;
			break;

		case GUN_SQUEAKYTOY_ID :
		case GUN_REDRIOT_ID :
			nCommandId = COMMAND_ID_WEAPON_8;
			break;

		case GUN_ENERGYBATON_ID :
		case GUN_ENERGYBLADE_ID :
		case GUN_MONOKNIFE_ID :
		case GUN_KATANA_ID :
		case GUN_TANTO_ID :
			nCommandId = COMMAND_ID_WEAPON_10;
			break;

		default : break;
	}

	return nCommandId;
}

// get the maximum ammo for each weapon
inline uint32 GetWeaponMaxAmmo(int nWeaponId)
{
	uint32 nRet = 0;

	switch (nWeaponId)
	{
		case GUN_PULSERIFLE_ID :
			nRet = 300;
		break;

		case GUN_SPIDER_ID :
			nRet = 50;
		break;

		case GUN_BULLGUT_ID :
			nRet = 100;
		break;

		case GUN_SNIPERRIFLE_ID :
			nRet = 400;
		break;

		case GUN_JUGGERNAUT_ID :
			nRet = 50;
		break;

		case GUN_SHREDDER_ID :
			nRet = 200;
		break;

		case GUN_REDRIOT_ID :
			nRet = 10;
		break;

		case GUN_ENERGYBATON_ID :
		case GUN_ENERGYBLADE_ID :
		case GUN_MONOKNIFE_ID :
		case GUN_KATANA_ID :
			nRet = 10000;
		break;


		// On-foot mode weapons...

		case GUN_COLT45_ID :
			nRet = 200;
		break;

		case GUN_SHOTGUN_ID	:
			nRet = 100;
		break;

		case GUN_ASSAULTRIFLE_ID :
			nRet = 400;
		break;

		case GUN_ENERGYGRENADE_ID :
			nRet = 50;
		break;

		case GUN_KATOGRENADE_ID :
			nRet = 50;
		break;

		case GUN_MAC10_ID :
			nRet = 400;
		break;

		case GUN_TOW_ID	:
			nRet = 25;
		break;

		case GUN_LASERCANNON_ID :
			nRet = 200;
		break;

		case GUN_SQUEAKYTOY_ID :
			nRet = 10000;
		break;

		case GUN_TANTO_ID :
			nRet = 10000;
		break;

		default : break;
	}

	return nRet;
}


inline uint32 GetSpawnedAmmo(RiotWeaponId nWeaponId)
{
	uint32 dwAmmo = 1000;

	switch (nWeaponId)
	{
		case GUN_PULSERIFLE_ID :
			dwAmmo = GetRandom(40,80);
		break;

		case GUN_SPIDER_ID :
			dwAmmo = GetRandom(5,10);
		break;

		case GUN_BULLGUT_ID :
			dwAmmo = GetRandom(2,10) * 4;
		break;

		case GUN_SNIPERRIFLE_ID :
			dwAmmo = GetRandom(40,80);
		break;

		case GUN_JUGGERNAUT_ID :
			dwAmmo = GetRandom(10,20);
		break;

		case GUN_SHREDDER_ID :
			dwAmmo = GetRandom(10,20);
		break;

		case GUN_REDRIOT_ID :
			dwAmmo = 1;
		break;


		// On-foot mode weapons...

		case GUN_COLT45_ID :
			dwAmmo = GetRandom(20,40);
		break;

		case GUN_SHOTGUN_ID	:
			dwAmmo = GetRandom(10,20);
		break;

		case GUN_ASSAULTRIFLE_ID :
			dwAmmo = GetRandom(40,80);
		break;

		case GUN_ENERGYGRENADE_ID :
			dwAmmo = GetRandom(5,10);
		break;

		case GUN_KATOGRENADE_ID :
			dwAmmo = GetRandom(5,10);
		break;

		case GUN_MAC10_ID :
			dwAmmo = GetRandom(40,80);
		break;

		case GUN_TOW_ID	:
			dwAmmo = GetRandom(1,5);
		break;

		case GUN_LASERCANNON_ID :
			dwAmmo = GetRandom(30,60);
		break;

		default : break;
	}

	return dwAmmo;
}


inline LTVector GetHandWeaponScale(RiotWeaponId nWeaponId, ModelSize eSize=MS_NORMAL)
{
	LTVector vRet;
	VEC_SET(vRet, 1.0f, 1.0f, 1.0f)

	switch (nWeaponId)
	{
		// On-foot mode weapons...

		case GUN_TANTO_ID :
			VEC_SET(vRet, 11.5f, 11.5f, 11.5f);
		break;

		case GUN_COLT45_ID :
			VEC_SET(vRet, 6.0f, 6.0f, 6.0f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 18.0f, 18.0f, 18.0f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 15.0f, 15.0f, 15.0f);
		break;

		case GUN_ASSAULTRIFLE_ID :
		case GUN_ENERGYGRENADE_ID :
		case GUN_KATOGRENADE_ID :
			//VEC_SET(vRet, 1.0f, 1.0f, 1.0f);
			VEC_SET(vRet, 3.0f, 3.0f, 3.0f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 28.0f, 28.0f, 28.0f);
		break;

		case GUN_SQUEAKYTOY_ID :
			VEC_SET(vRet, 15.0f, 15.0f, 15.0f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_SHREDDER_ID	:
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_LASERCANNON_ID:
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_SNIPERRIFLE_ID :
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		case GUN_REDRIOT_ID :
			VEC_SET(vRet, 25.0f, 25.0f, 25.0f);
		break;

		default : break;
	}

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	VEC_MULSCALAR(vRet, vRet, fFactor);


	return vRet;
}

inline LTVector GetHandWeaponFlashOffset(RiotWeaponId nWeaponId, ModelSize eSize=MS_NORMAL)
{
	LTVector vRet;
	VEC_INIT(vRet)

	switch (nWeaponId)
	{
		case GUN_COLT45_ID :
			VEC_SET(vRet, 0.0f, 3.0f, 9.6f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 0.0f, 4.5f, 33.3f);
		break;

		case GUN_ASSAULTRIFLE_ID :
		case GUN_ENERGYGRENADE_ID :
		case GUN_KATOGRENADE_ID :
			// VEC_SET(vRet, 0.0f, 4.0f, 29.0f);
			// VEC_SET(vRet, 0.0f, 2.0f * scale, 8.75f * scale);
			VEC_SET(vRet, 0.0f, 6.0f, 26.25f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 0.0f, 4.75f, 22.5f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 0.0f, 2.8f, 28.0f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 0.0f, 5.0f, 41.25f);
		break;

		case GUN_LASERCANNON_ID :
			VEC_SET(vRet, 0.0f, 3.75f, 43.75f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 43.75f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, 0.0f, 5.0f, 45.0f);
		break;

		case GUN_SNIPERRIFLE_ID	:
			VEC_SET(vRet, 0.0f, 2.5f, 40.0f);	
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 0.0f, 5.0f, 41.25f);
		break;

		case GUN_SHREDDER_ID	:
			VEC_SET(vRet, 0.0f, 5.0f, 41.25f);
		break;

		case GUN_REDRIOT_ID	:
			VEC_SET(vRet, 0.0f, 2.5f, 42.5f);
		break;

		default : break;
	}

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	VEC_MULSCALAR(vRet, vRet, fFactor);

	return vRet;
}

inline LTBOOL UsesAmmo(RiotWeaponId nWeaponId)
{
	switch (nWeaponId)
	{
		case GUN_ENERGYBATON_ID :
		case GUN_ENERGYBLADE_ID :
		case GUN_KATANA_ID :
		case GUN_MONOKNIFE_ID :
		case GUN_SQUEAKYTOY_ID :
		case GUN_UNUSED1_ID :
		case GUN_UNUSED2_ID :
		case GUN_TANTO_ID :
		{
			return LTFALSE;
		}
		break;	
	}

	return LTTRUE;
}

// get the maximum range for each weapon
inline LTFLOAT GetWeaponRange(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 10000.0f;

	LTFLOAT s_fWeaponRange[GUN_MAX_NUMBER] =
	{
		10000.0f,	// GUN_PULSERIFLE_ID
		6000.0f,	// GUN_LASERCANNON_ID
		10000.0f,	// GUN_SPIDER_ID
		10000.0f,	// GUN_BULLGUT_ID
		10000.0f,	// GUN_SNIPERRIFLE_ID
		8000.0f,	// GUN_JUGGERNAUT_ID
		6000.0f,	// GUN_SHREDDER_ID
			0.0f,	// NOT USED1
		10000.0f,	// GUN_REDRIOT_ID
		128.0f,		// GUN_ENERGYBATON_ID
		128.0f,		// GUN_ENERGYBLADE_ID
		128.0f,		// GUN_KATANA_ID
		128.0f,		// GUN_MONOKNIFE_ID
		3000.0f,	// GUN_COLT45_ID
		2000.0f,	// GUN_SHOTGUN_ID
		6000.0f,	// GUN_ASSAULTRIFLE_ID
		10000.0f,	// GUN_ENERGYGRENADE_ID
		5000.0f,	// GUN_KATOGRENADE_ID
		2000.0f,	// GUN_MAC10_ID
		10000.0f,	// GUN_TOW_ID
		0.0f,		// NOT USED2
		3000.0f,	// GUN_SQUEAKYTOY_ID
		128.0f		// GUN_TANTO_ID
	};

	return s_fWeaponRange[nWeaponId];
}


// Return the average damage done by the weapon...

inline LTFLOAT GetWeaponDamage(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0.0f;

	LTFLOAT s_fWeaponDamage[GUN_MAX_NUMBER] =
	{	
		150.0f,		// GUN_PULSERIFLE_ID
		300.0f,		// GUN_LASERCANNON_ID
		500.0f,		// GUN_SPIDER_ID
		125.0f,		// GUN_BULLGUT_ID
		100.0f,		// GUN_SNIPERRIFLE_ID
		300.0f,		// GUN_JUGGERNAUT_ID
		400.0f,		// GUN_SHREDDER_ID
		0.0f,		// NOT USED1
		1000.0f,	// GUN_REDRIOT_ID
		600.0f,		// GUN_ENERGYBATON_ID
		800.0f,		// GUN_ENERGYBLADE_ID
		1000.0f,	// GUN_KATANA_ID
		700.0f,		// GUN_MONOKNIFE_ID
		20.0f,		// GUN_COLT45_ID
		20.0f,		// GUN_SHOTGUN_ID
		25.0f,		// GUN_ASSAULTRIFLE_ID
		80.0f,		// GUN_ENERGYGRENADE_ID
		50.0f,		// GUN_KATOGRENADE_ID
		30.0f,		// GUN_MAC10_ID
		120.0f,		// GUN_TOW_ID
		0.0f,		// NOT USED2
		0.0f,		// GUN_SQUEAKYTOY_ID
		100.0f		// GUN_TANTO_ID
	};

	return s_fWeaponDamage[nWeaponId];
}


// get the maximum range for each weapon
inline int GetWeaponDamageRadius(int nWeaponId, ModelSize eSize=MS_NORMAL)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	int s_nWeaponDamageRadius[GUN_MAX_NUMBER] =
	{
		100,	// GUN_PULSERIFLE_ID
		0,		// GUN_LASERCANNON_ID
		300,	// GUN_SPIDER_ID
		300,	// GUN_BULLGUT_ID
		0,		// GUN_SNIPERRIFLE_ID
		200,	// GUN_JUGGERNAUT_ID
		150,	// GUN_SHREDDER_ID
		0,		// NOT USED1
		700,	// GUN_REDRIOT_ID
		0,		// GUN_ENERGYBATON_ID
		0,		// GUN_ENERGYBLADE_ID
		0,		// GUN_KATANA_ID
		0,		// GUN_MONOKNIFE_ID
		0,		// GUN_COLT45_ID
		0,		// GUN_SHOTGUN_ID
		0,		// GUN_ASSAULTRIFLE_ID
		200,	// GUN_ENERGYGRENADE_ID
		300,	// GUN_KATOGRENADE_ID
		0,		// GUN_MAC10_ID
		300,	// GUN_TOW_ID
		0,		// NOT USED2
		0,		// GUN_SQUEAKYTOY_ID
		0		// GUN_TANTO_ID
	};

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	return int((LTFLOAT)s_nWeaponDamageRadius[nWeaponId] * fFactor);
}

// get the range the weapon can be heard from
inline int GetWeaponFireSoundRadius(int nWeaponId, ModelSize eSize=MS_NORMAL)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	int s_nWeaponSoundRadius[GUN_MAX_NUMBER] =
	{
		1000,	// GUN_PULSERIFLE_ID
		1000,	// GUN_LASERCANNON_ID
		1000,	// GUN_SPIDER_ID
		1500,	// GUN_BULLGUT_ID
		1000,	// GUN_SNIPERRIFLE_ID
		1500,	// GUN_JUGGERNAUT_ID
		1500,	// GUN_SHREDDER_ID
		0,		// NOT USED1
		2500,	// GUN_REDRIOT_ID
		0,		// GUN_ENERGYBATON_ID
		0,		// GUN_ENERGYBLADE_ID
		0,		// GUN_KATANA_ID
		0,		// GUN_MONOKNIFE_ID
		1000,	// GUN_COLT45_ID
		1500,	// GUN_SHOTGUN_ID
		1500,	// GUN_ASSAULTRIFLE_ID
		1500,	// GUN_ENERGYGRENADE_ID
		1000,	// GUN_KATOGRENADE_ID
		500,	// GUN_MAC10_ID
		2000,	// GUN_TOW_ID
		0,		// NOT USED2
		500,	// GUN_SQUEAKYTOY_ID
		0		// GUN_TANTO_ID
	};

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	return int((LTFLOAT)s_nWeaponSoundRadius[nWeaponId] * fFactor);
}

// get the range the weapon can be heard from
inline int GetWeaponImpactSoundRadius(int nWeaponId, ModelSize eSize=MS_NORMAL)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	int s_nWeaponImpactSoundRadius[GUN_MAX_NUMBER] =
	{
		300,	// GUN_PULSERIFLE_ID
		300,	// GUN_LASERCANNON_ID
		1500,	// GUN_SPIDER_ID
		2000,	// GUN_BULLGUT_ID
		100,	// GUN_SNIPERRIFLE_ID
		2000,	// GUN_JUGGERNAUT_ID
		1500,	// GUN_SHREDDER_ID
		0,		// NOT USED1
		5000,	// GUN_REDRIOT_ID
		0,		// GUN_ENERGYBATON_ID
		0,		// GUN_ENERGYBLADE_ID
		0,		// GUN_KATANA_ID
		0,		// GUN_MONOKNIFE_ID
		300,	// GUN_COLT45_ID
		200,	// GUN_SHOTGUN_ID
		700,	// GUN_ASSAULTRIFLE_ID
		1500,	// GUN_ENERGYGRENADE_ID
		1500,	// GUN_KATOGRENADE_ID
		200,	// GUN_MAC10_ID
		2000,	// GUN_TOW_ID
		0,		// NOT USED2
		500,	// GUN_SQUEAKYTOY_ID
		0		// GUN_TANTO_ID
	};

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	return int((LTFLOAT)s_nWeaponImpactSoundRadius[nWeaponId] * fFactor);
}

// get the maximum range for each weapon
inline uint8 GetWeaponFX(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	uint8 s_nWeaponFX[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		WFX_SPARKS,									
		/*GUN_LASERCANNON_ID*/		WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_SMOKE,
		/*GUN_SPIDER_ID*/			WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*GUN_BULLGUT_ID*/			WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*GUN_SNIPERRIFLE_ID*/		WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_MUZZLE | WFX_TRACER | WFX_SHELL,
		/*GUN_JUGGERNAUT_ID*/		WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_SPARKS | WFX_SMOKE | WFX_SHELL,	
		/*GUN_SHREDDER_ID*/			WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_SPARKS | WFX_SMOKE | WFX_SHELL,
		/*NOT USED1*/				0,
		/*GUN_REDRIOT_ID*/			WFX_FIRESOUND | WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*GUN_ENERGYBATON_ID*/		WFX_FIRESOUND | WFX_SPARKS,
		/*GUN_ENERGYBLADE_ID*/		WFX_FIRESOUND | WFX_SPARKS,
		/*GUN_KATANA_ID*/			WFX_FIRESOUND | WFX_SPARKS,
		/*GUN_MONOKNIFE_ID*/		WFX_FIRESOUND | WFX_SPARKS,
		/*GUN_COLT45_ID*/			WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_MUZZLE | WFX_TRACER | WFX_SHELL,
		/*GUN_SHOTGUN_ID*/			WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_MUZZLE | WFX_SHELL,
		/*GUN_ASSAULTRIFLE_ID*/		WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_MUZZLE | WFX_TRACER | WFX_SHELL,
		/*GUN_ENERGYGRENADE_ID*/	WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*GUN_KATOGRENADE_ID*/		WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*GUN_MAC10_ID*/			WFX_FIRESOUND | WFX_LIGHT | WFX_MARK | WFX_MUZZLE | WFX_TRACER | WFX_SHELL,
		/*GUN_TOW_ID*/				WFX_MARK | WFX_SPARKS | WFX_SMOKE,
		/*NOT USED2*/				0,
		/*GUN_SQUEAKYTOY_ID*/		0,
		/*GUN_TANTO_ID*/			WFX_FIRESOUND | WFX_SPARKS,
	};

	return s_nWeaponFX[nWeaponId];
}


// get the maximum range for each weapon
inline uint8 GetProjectileFX(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	uint8 s_nProjectileFX[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		PFX_LIGHT,
		/*GUN_LASERCANNON_ID*/		0,				
		/*GUN_SPIDER_ID*/			PFX_FLYSOUND,
		/*GUN_BULLGUT_ID*/			PFX_FLYSOUND | PFX_SMOKETRAIL | PFX_FLARE | PFX_LIGHT,
		/*GUN_SNIPERRIFLE_ID*/		0,
		/*GUN_JUGGERNAUT_ID*/		0,	
		/*GUN_SHREDDER_ID*/			0,
		/*NOT USED1*/				0,
		/*GUN_REDRIOT_ID*/			0,
		/*GUN_ENERGYBATON_ID*/		0,
		/*GUN_ENERGYBLADE_ID*/		0,
		/*GUN_KATANA_ID*/			0,
		/*GUN_MONOKNIFE_ID*/		0,
		/*GUN_COLT45_ID*/			0,
		/*GUN_SHOTGUN_ID*/			0,
		/*GUN_ASSAULTRIFLE_ID*/		0,
		/*GUN_ENERGYGRENADE_ID*/	PFX_FLYSOUND | PFX_LIGHT,
		/*GUN_KATOGRENADE_ID*/		PFX_FLYSOUND | PFX_LIGHT,
		/*GUN_MAC10_ID*/			0,
		/*GUN_TOW_ID*/				PFX_FLYSOUND | PFX_SMOKETRAIL | PFX_FLARE | PFX_LIGHT,
		/*NOT USED2*/				0,
		/*GUN_SQUEAKYTOY_ID*/		0,
		/*GUN_TANTO_ID*/			0
	};

	return s_nProjectileFX[nWeaponId];
}




////////////////////////////////////////////////////////////////////////////////
//
// The following functions replace data members in weapon.cpp that used to be
// sent from the server to the client....
//
/////////////////////////////////////////////////////////////////////////////////

// get the name of the flash used by this weapon
inline char* GetFlashFilename(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return LTNULL;

	char* s_cFlashFilename[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		LTNULL,
		/*GUN_LASERCANNON_ID*/		LTNULL,				
		/*GUN_SPIDER_ID*/			LTNULL,
		/*GUN_BULLGUT_ID*/			LTNULL,
		/*GUN_SNIPERRIFLE_ID*/		LTNULL,
		/*GUN_JUGGERNAUT_ID*/		LTNULL,	
		/*GUN_SHREDDER_ID*/			LTNULL,
		/*NOT USED1*/				LTNULL,
		/*GUN_REDRIOT_ID*/			LTNULL,
		/*GUN_ENERGYBATON_ID*/		LTNULL,
		/*GUN_ENERGYBLADE_ID*/		LTNULL,
		/*GUN_KATANA_ID*/			LTNULL,
		/*GUN_MONOKNIFE_ID*/		LTNULL,
		/*GUN_COLT45_ID*/			"Sprites\\colt45muzzleflash.spr",
		/*GUN_SHOTGUN_ID*/			"Sprites\\shotgunmuzzleflash.spr",
		/*GUN_ASSAULTRIFLE_ID*/		"Sprites\\AssaultRifleMuzzleflash.spr",
		/*GUN_ENERGYGRENADE_ID*/	LTNULL,
		/*GUN_KATOGRENADE_ID*/		LTNULL,
		/*GUN_MAC10_ID*/			"Sprites\\Weapons\\Mac10Flash.spr",
		/*GUN_TOW_ID*/				LTNULL,
		/*NOT USED2*/				LTNULL,
		/*GUN_SQUEAKYTOY_ID*/		LTNULL,
		/*GUN_TANTO_ID*/			LTNULL
	};

	return s_cFlashFilename[nWeaponId];
}


// get the duration of the flash used by this weapon
inline LTFLOAT GetFlashDuration(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0.0f;

	LTFLOAT s_fFlashDuration[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		0.0f,
		/*GUN_LASERCANNON_ID*/		0.0f,				
		/*GUN_SPIDER_ID*/			0.0f,
		/*GUN_BULLGUT_ID*/			0.0f,
		/*GUN_SNIPERRIFLE_ID*/		0.0f,
		/*GUN_JUGGERNAUT_ID*/		0.0f,	
		/*GUN_SHREDDER_ID*/			0.0f,
		/*NOT USED1*/				0.0f,
		/*GUN_REDRIOT_ID*/			0.0f,
		/*GUN_ENERGYBATON_ID*/		0.0f,
		/*GUN_ENERGYBLADE_ID*/		0.0f,
		/*GUN_KATANA_ID*/			0.0f,
		/*GUN_MONOKNIFE_ID*/		0.0f,
		/*GUN_COLT45_ID*/			0.075f,
		/*GUN_SHOTGUN_ID*/			0.08f,
		/*GUN_ASSAULTRIFLE_ID*/		0.075f,
		/*GUN_ENERGYGRENADE_ID*/	0.0f,
		/*GUN_KATOGRENADE_ID*/		0.0f,
		/*GUN_MAC10_ID*/			0.075f,
		/*GUN_TOW_ID*/				0.0f,
		/*NOT USED2*/				0.0f,
		/*GUN_SQUEAKYTOY_ID*/		0.0f,
		/*GUN_TANTO_ID*/			0.0f
	};

	return s_fFlashDuration[nWeaponId];
}

// get the scale of the flash used by this weapon
inline LTVector GetFlashScale(RiotWeaponId nWeaponId, ModelSize eSize=MS_NORMAL)
{
	LTVector vRet;
	VEC_INIT(vRet)

	switch (nWeaponId)
	{
		case GUN_COLT45_ID :
			VEC_SET(vRet, 0.06f, 0.06f, 0.0f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 0.1125f, 0.1125f, 0.0f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 0.1f, 0.1f, 0.0f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 0.1f, 0.1f, 0.0f);
		break;

		default : break;
	}

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	VEC_MULSCALAR(vRet, vRet, fFactor);

	return vRet;
}


// Get the offset of the weapon from the player object...
inline LTVector GetWeaponOffset(RiotWeaponId nWeaponId)
{
	LTVector vRet;
	VEC_SET(vRet, 1.0f, 1.0f, 1.0f)

	switch (nWeaponId)
	{
		case GUN_TANTO_ID :
			VEC_SET(vRet, 1.3f, -2.6f, 4.0f);
		break;

		case GUN_COLT45_ID :
			VEC_SET(vRet, -0.1f, -2.3f, 6.2f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 1.0f, -2.0f, 3.5f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, .7f, -2.3f, 6.6f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 1.4f, -2.2f, 7.3f);
		break;

		case GUN_ENERGYGRENADE_ID :
			VEC_SET(vRet, 0.8f, -1.8f, 5.7f);
		break;

		case GUN_KATOGRENADE_ID :
			VEC_SET(vRet, 0.8f, -1.8f, 5.7f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 1.35f, -4.5f, 8.0f);
		break;

		case GUN_SQUEAKYTOY_ID :
			VEC_SET(vRet, -1.6, -3.5f, 2.4f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 0.5f, -1.2f, 2.5f);
		break;

		case GUN_SHREDDER_ID	:
			VEC_SET(vRet, 1.0f, -1.4f, 6.5f);
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 1.0f, -2.6f, 5.5f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, 1.0f, -2.4f, 1.0f);
		break;

		case GUN_LASERCANNON_ID:
			VEC_SET(vRet, 0.6f, -1.8f, -1.0f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 0.5f, -1.6f, 6.0f);
		break;

		case GUN_SNIPERRIFLE_ID :
			VEC_SET(vRet, 0.9f, -2.8f, -1.2f);
		break;

		case GUN_REDRIOT_ID :
			VEC_SET(vRet, 2.4f, -3.7f, 7.4f);
		break;

		case GUN_ENERGYBATON_ID :
			VEC_SET(vRet, -0.4f, -3.5f, 3.7f);
		break;

		case GUN_ENERGYBLADE_ID :
			VEC_SET(vRet, 0.8f, -1.2f, 2.9f);
		break;

		case GUN_KATANA_ID :
			VEC_SET(vRet, 0.5f, -1.9f, 0.2f);
		break;

		case GUN_MONOKNIFE_ID :
			VEC_SET(vRet, 0.1f, -2.1f, 3.1f);
		break;

		default : break;
	}

	return vRet;
}


// Get the offset of the muzzle from the player object...
inline LTVector GetWeaponMuzzleOffset(RiotWeaponId nWeaponId, WeaponState eState=W_FIRED)
{
	LTVector vRet;
	VEC_SET(vRet, 0.0f, 0.0f, 0.0f)

	switch (nWeaponId)
	{
		case GUN_TANTO_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_COLT45_ID :
		{
			if (eState == W_FIRED)
			{
				VEC_SET(vRet, -6.2f, -1.3f, 12.2f);
			}
			else if (eState == W_FIRED2)
			{
				VEC_SET(vRet, 5.8f, -1.3f, 12.2f);
			}
		}
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 0.9f, 1.0f, 14.5f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 2.1f, -0.5f, 13.3f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 0.6f, 0.0f, 10.3f);
		break;

		case GUN_ENERGYGRENADE_ID :
			VEC_SET(vRet, 4.4f, -0.6f, 15.9f);
		break;

		case GUN_KATOGRENADE_ID :
			VEC_SET(vRet, 4.4f, -0.6f, 15.9f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 1.85f, 1.36f, 11.2f);
		break;

		case GUN_SQUEAKYTOY_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 1.1f, -1.0f, 21.5f);
		break;

		case GUN_SHREDDER_ID	:
			VEC_SET(vRet, 1.6f, -2.8f, 17.1f);
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 0.8f, 0.9f, 18.5f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, -2.75f, 0.36f, 16.6f);
		break;

		case GUN_LASERCANNON_ID:
			VEC_SET(vRet, 0.4f, 0.0f, 21.4f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 0.5f, 0.7f, 14.4f);
		break;

		case GUN_SNIPERRIFLE_ID :
			VEC_SET(vRet, -0.8f, 3.0f, 21.9f);
		break;

		case GUN_REDRIOT_ID :
			VEC_SET(vRet, 0.2f, 0.25f, 14.3f);
		break;

		case GUN_ENERGYBATON_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_ENERGYBLADE_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_KATANA_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_MONOKNIFE_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		default : break;
	}

	return vRet;
}


// Get the offset of the weapon from the player object...
inline LTVector GetWeaponOffset2(RiotWeaponId nWeaponId)
{
	LTVector vRet;
	VEC_SET(vRet, 0.0f, 0.0f, 0.0f)

	switch (nWeaponId)
	{
		case GUN_TANTO_ID :
			VEC_SET(vRet, 1.3f, -4.0f, 3.0f);
		break;

		case GUN_COLT45_ID :
			VEC_SET(vRet, -0.1f, -2.3f, 2.4f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 1.0f, -1.8f, -5.7f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, .7f, -2.2f, 2.5f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 1.4f, -2.2f, -0.5f);
		break;

		case GUN_ENERGYGRENADE_ID :
			VEC_SET(vRet, 0.8f, -1.8f, 0.7f);
		break;

		case GUN_KATOGRENADE_ID :
			VEC_SET(vRet, 0.8f, -1.8f, 0.7f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 1.35f, -4.5f, 0.2f);
		break;

		case GUN_SQUEAKYTOY_ID :
			VEC_SET(vRet, -1.6, -3.5f, -3.2f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 0.5f, -1.2f, -1.9f);
		break;

		case GUN_SHREDDER_ID :
			VEC_SET(vRet, 1.0f, -1.4f, -2.5f);
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 1.0f, -2.6f, -1.6f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, 1.0f, -2.4f, -3.4f);
		break;

		case GUN_LASERCANNON_ID:
			VEC_SET(vRet, 0.6f, -1.8f, -8.8f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 0.5f, -1.6f, 1.0f);
		break;

		case GUN_SNIPERRIFLE_ID :
			VEC_SET(vRet, 0.9f, -2.8f, -13.0f);
		break;

		case GUN_REDRIOT_ID :
			VEC_SET(vRet, 2.4f, -3.7f, -5.8f);
		break;

		case GUN_ENERGYBATON_ID :
			VEC_SET(vRet, -0.4f, -3.5f, 3.7f);
		break;

		case GUN_ENERGYBLADE_ID :
			VEC_SET(vRet, 0.8f, -1.2f, 0.9f);
		break;

		case GUN_KATANA_ID :
			VEC_SET(vRet, 0.5f, -1.9f, -3.7f);
		break;

		case GUN_MONOKNIFE_ID :
			VEC_SET(vRet, 0.1f, -2.1f, 1.0f);
		break;

		default : break;
	}

	return vRet;
}


// Get the offset of the muzzle from the player object...
inline LTVector GetWeaponMuzzleOffset2(RiotWeaponId nWeaponId, WeaponState eState=W_FIRED)
{
	LTVector vRet;
	VEC_SET(vRet, 0.0f, 0.0f, 0.0f)

	switch (nWeaponId)
	{
		case GUN_TANTO_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_COLT45_ID :
		{
			if (eState == W_FIRED)
			{
				VEC_SET(vRet, -6.2f, -1.3f, 8.6f);
			}
			else if (eState == W_FIRED2)
			{
				VEC_SET(vRet, 5.8f, -1.3f, 8.6f);
			}
		}
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 2.6f, -1.0f, 15.3f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 2.1f, -0.5f, 13.3f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 1.4f, -1.4f, 11.7f);
		break;

		case GUN_ENERGYGRENADE_ID :
			VEC_SET(vRet, 4.4f, -0.6f, 15.9f);
		break;

		case GUN_KATOGRENADE_ID :
			VEC_SET(vRet, 4.4f, -0.6f, 15.9f);
		break;

		case GUN_TOW_ID	:
			VEC_SET(vRet, 1.85f, 1.36f, 11.2f);
		break;

		case GUN_SQUEAKYTOY_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_PULSERIFLE_ID :
			VEC_SET(vRet, 1.1f, -1.0f, 21.5f);
		break;

		case GUN_SHREDDER_ID :
			VEC_SET(vRet, 1.4f, -3.0f, 13.3f);
		break;

		case GUN_JUGGERNAUT_ID :
			VEC_SET(vRet, 1.8f, -2.6f, 14.4f);
		break;

		case GUN_BULLGUT_ID	:
			VEC_SET(vRet, -2.75f, 0.36f, 16.6f);
		break;

		case GUN_LASERCANNON_ID :
			VEC_SET(vRet, 1.4f, -2.2f, 18.6f);
		break;

		case GUN_SPIDER_ID :
			VEC_SET(vRet, 0.5f, 0.7f, 14.4f);
		break;

		case GUN_SNIPERRIFLE_ID :
			VEC_SET(vRet, -0.5f, 2.6f, -12.8f);
		break;

		case GUN_REDRIOT_ID :
			VEC_SET(vRet, 0.2f, 0.25f, 14.3f);
		break;

		case GUN_ENERGYBATON_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_ENERGYBLADE_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_KATANA_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		case GUN_MONOKNIFE_ID :
			VEC_SET(vRet, 0.0f, 0.0f, 0.0f);
		break;

		default : break;
	}

	return vRet;
}

// get the scale of the flash used by this weapon
inline LTVector GetFlashScale2(RiotWeaponId nWeaponId, ModelSize eSize=MS_NORMAL)
{
	LTVector vRet;
	VEC_INIT(vRet)

	switch (nWeaponId)
	{
		case GUN_COLT45_ID :
			VEC_SET(vRet, 0.04f, 0.04f, 0.0f);
		break;

		case GUN_SHOTGUN_ID	:
			VEC_SET(vRet, 0.05f, 0.05f, 0.0f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 0.10f, 0.10f, 0.0f);
		break;

		case GUN_MAC10_ID :
			VEC_SET(vRet, 0.05f, 0.05f, 0.0f);
		break;

		default : break;
	}

	LTFLOAT fFactor = (eSize == MS_NORMAL ? 1.0f : (eSize == MS_SMALL ? 0.2f : 5.0f));
	VEC_MULSCALAR(vRet, vRet, fFactor);

	return vRet;
}


// Get the offset of the muzzle from the player object...
inline LTVector GetWeaponRecoil(RiotWeaponId nWeaponId)
{
	LTVector vRet;
	VEC_SET(vRet, 0.0f, 0.0f, 0.0f)

	switch (nWeaponId)
	{
		case GUN_MAC10_ID :
			VEC_SET(vRet, 0.03f, 0.03f, 0.15f);
		break;

		case GUN_ASSAULTRIFLE_ID :
			VEC_SET(vRet, 0.03f, 0.03f, 0.15f);
		break;

		default : break;
	}

	return vRet;
}


inline uint32 GetExtraWeaponFlags(RiotWeaponId nWeaponId) 
{ 
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	uint32 m_dwExtraModelFlags[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		0,
		/*GUN_LASERCANNON_ID*/		0,				
		/*GUN_SPIDER_ID*/			FLAG_ENVIRONMENTMAPONLY,
		/*GUN_BULLGUT_ID*/			0,
		/*GUN_SNIPERRIFLE_ID*/		FLAG_ENVIRONMENTMAPONLY,
		/*GUN_JUGGERNAUT_ID*/		0,	
		/*GUN_SHREDDER_ID*/			0,
		/*NOT USED1*/				0,
		/*GUN_REDRIOT_ID*/			0,
		/*GUN_ENERGYBATON_ID*/		0,
		/*GUN_ENERGYBLADE_ID*/		0,
		/*GUN_KATANA_ID*/			FLAG_ENVIRONMENTMAPONLY,
		/*GUN_MONOKNIFE_ID*/		FLAG_ENVIRONMENTMAPONLY,
		/*GUN_COLT45_ID*/			FLAG_ENVIRONMENTMAPONLY,
		/*GUN_SHOTGUN_ID*/			FLAG_ENVIRONMENTMAPONLY,
		/*GUN_ASSAULTRIFLE_ID*/		0,
		/*GUN_ENERGYGRENADE_ID*/	0,
		/*GUN_KATOGRENADE_ID*/		0,
		/*GUN_MAC10_ID*/			FLAG_ENVIRONMENTMAPONLY,
		/*GUN_TOW_ID*/				0,
		/*NOT USED2*/				0,
		/*GUN_SQUEAKYTOY_ID*/		0,
		/*GUN_TANTO_ID*/			FLAG_ENVIRONMENTMAPONLY
	};

	return m_dwExtraModelFlags[nWeaponId]; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetWeaponFireSound()
//
//	PURPOSE:	Get fire sounds associated with this weapon
//
// ----------------------------------------------------------------------- //

inline LTBOOL GetWeaponSoundDir(RiotWeaponId nId)
{
	char* pSound = LTNULL;

	switch (nId)
	{
		case GUN_PULSERIFLE_ID :
			pSound = "PulseRifle";
		break;

		case GUN_SPIDER_ID :
			pSound = "Spider";
		break;

		case GUN_BULLGUT_ID :
			pSound = "Bullgut";
		break;

		case GUN_SNIPERRIFLE_ID :
			pSound = "SniperRifle";
		break;

		case GUN_JUGGERNAUT_ID :
			pSound = "Juggernaut";
		break;

		case GUN_SHREDDER_ID :
			pSound = "Shredder";
		break;

		case GUN_REDRIOT_ID :
			pSound = "RedRiot";
		break;

		case GUN_ENERGYBATON_ID :
			pSound = "EnergyBaton";
		break;

		case GUN_ENERGYBLADE_ID :
			pSound = "EnergyBlade";
		break;

		case GUN_KATANA_ID :
			pSound = "Katana";
		break;

		case GUN_MONOKNIFE_ID :
			pSound = "MonoKnife";
		break;

		case GUN_COLT45_ID :
			pSound	= "Colt45";
		break;
		
		case GUN_SHOTGUN_ID	:
			pSound = "Shotgun";
		break;
		
		case GUN_ASSAULTRIFLE_ID :
			pSound = "AssaultRifle";
		break;
		
		case GUN_MAC10_ID :
			pSound = "Machinegun";
		break;
	
		case GUN_TANTO_ID :
			pSound = "Tanto";
		break;

		case GUN_ENERGYGRENADE_ID :
			pSound = "EnergyGrenade";
		break;

		case GUN_KATOGRENADE_ID :
			pSound = "KatoGrenade";
		break;

		case GUN_TOW_ID	:
			pSound = "TOW";
		break;

		case GUN_LASERCANNON_ID :
			pSound = "LaserCannon";
		break;

		case GUN_SQUEAKYTOY_ID :
			pSound = "Squeakytoy";
		break;

		default : 
			return LTFALSE;
			break;
	}


	sprintf( s_FileBuffer, "Sounds\\Weapons\\%s\\", pSound );
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetWeaponFireSound()
//
//	PURPOSE:	Get fire sounds associated with this weapon
//
// ----------------------------------------------------------------------- //

inline char* GetWeaponFireSound(RiotWeaponId nId)
{
	if (!GetWeaponSoundDir(nId)) return LTNULL;

	if( nId == GUN_COLT45_ID && GetRandom(0, 1) == 1 )
	{
		strcat( s_FileBuffer, "fire2.wav" );
	}
	else
	{
		strcat( s_FileBuffer, "fire.wav" );
	}

	return s_FileBuffer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetWeaponDryFireSound()
//
//	PURPOSE:	Get dry fire sounds associated with this weapon
//
// ----------------------------------------------------------------------- //

inline char* GetWeaponDryFireSound(RiotWeaponId nId)
{
	if (!GetWeaponSoundDir(nId)) return LTNULL;

	strcat(s_FileBuffer, "empty.wav");
	return s_FileBuffer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetWeaponFlyingSound()
//
//	PURPOSE:	Get flying sound associated with this weapon
//
// ----------------------------------------------------------------------- //

inline char* GetWeaponFlyingSound(RiotWeaponId nId)
{
	char* pRet = LTNULL;

	switch (nId)
	{
		case GUN_SPIDER_ID :
			pRet = "Sounds\\Weapons\\Spider\\Timer.wav";
		break;

		case GUN_BULLGUT_ID :
			pRet = "Sounds\\Weapons\\Bullgut\\projectile.wav";
		break;

		case GUN_ENERGYGRENADE_ID :
			pRet = "Sounds\\Weapons\\EnergyGrenade\\projectile.wav";
		break;

		case GUN_KATOGRENADE_ID :
			pRet = "Sounds\\Weapons\\KatoGrenade\\projectile.wav";
		break;

		case GUN_TOW_ID	:
			pRet = "Sounds\\Weapons\\Tow\\projectile.wav";
		break;

		default:
		break;
	}

	return pRet;
}


// get the duration of the flash used by this weapon
inline int GetShotsPerClip(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return -1;

	int s_nShotsPerClip[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		30,
		/*GUN_LASERCANNON_ID*/		-1,				
		/*GUN_SPIDER_ID*/			1,
		/*GUN_BULLGUT_ID*/			4,
		/*GUN_SNIPERRIFLE_ID*/		2,
		/*GUN_JUGGERNAUT_ID*/		-1,	
		/*GUN_SHREDDER_ID*/			-1,
		/*NOT USED1*/				0,
		/*GUN_REDRIOT_ID*/			1,
		/*GUN_ENERGYBATON_ID*/		-1,
		/*GUN_ENERGYBLADE_ID*/		-1,
		/*GUN_KATANA_ID*/			-1,
		/*GUN_MONOKNIFE_ID*/		-1,
		/*GUN_COLT45_ID*/			22,
		/*GUN_SHOTGUN_ID*/			1,
		/*GUN_ASSAULTRIFLE_ID*/		50,
		/*GUN_ENERGYGRENADE_ID*/	6,
		/*GUN_KATOGRENADE_ID*/		1,
		/*GUN_MAC10_ID*/			30,
		/*GUN_TOW_ID*/				1,
		/*NOT USED2*/				0,
		/*GUN_SQUEAKYTOY_ID*/		-1,
		/*GUN_TANTO_ID*/			-1
	};

	return s_nShotsPerClip[nWeaponId];
}

// Get the number of vectors this weapon shoots...
inline int GetVectorsPerShot(int nWeaponId)
{
	if (nWeaponId == GUN_SHOTGUN_ID) return 10;

	return 1;
}

// Get the number of vectors this weapon shoots...
inline LTFLOAT GetWeaponSpread(int nWeaponId)
{
	if (nWeaponId == GUN_BULLGUT_ID) return 3.5f;

	return 0.0f;
}


// get the duration of the flash used by this weapon
inline uint8 GetWeaponPerturbe(int nWeaponId, LTFLOAT & fRPerturbe, 
							   LTFLOAT & fUPerturbe)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0;

	// Important that these values are between 0 and 127...

	uint8 s_nPerturbe[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		0,
		/*GUN_LASERCANNON_ID*/		0,				
		/*GUN_SPIDER_ID*/			0,
		/*GUN_BULLGUT_ID*/			5,
		/*GUN_SNIPERRIFLE_ID*/		0,
		/*GUN_JUGGERNAUT_ID*/		10,	
		/*GUN_SHREDDER_ID*/			15,
		/*NOT USED1*/				0,
		/*GUN_REDRIOT_ID*/			20,
		/*GUN_ENERGYBATON_ID*/		0,
		/*GUN_ENERGYBLADE_ID*/		0,
		/*GUN_KATANA_ID*/			0,
		/*GUN_MONOKNIFE_ID*/		0,
		/*GUN_COLT45_ID*/			30,
		/*GUN_SHOTGUN_ID*/			60,
		/*GUN_ASSAULTRIFLE_ID*/		5,
		/*GUN_ENERGYGRENADE_ID*/	10,
		/*GUN_KATOGRENADE_ID*/		10,
		/*GUN_MAC10_ID*/			40,
		/*GUN_TOW_ID*/				0,
		/*NOT USED2*/				0,
		/*GUN_SQUEAKYTOY_ID*/		0,
		/*GUN_TANTO_ID*/			0
	};

	int nPerturbe = s_nPerturbe[nWeaponId];

	// Get a random perturbe based on nPerturbe...USE nSeed!!!  Need to come
	// up with an algorithm that uses nSeed (and adjusts nSeed?) so that this
	// can be called with the same seed to generate the same "random" values...

	fRPerturbe = ((LTFLOAT)GetRandom(-nPerturbe, nPerturbe))/1000.0f;
	fUPerturbe = ((LTFLOAT)GetRandom(-nPerturbe, nPerturbe))/1000.0f;

	return nPerturbe;
}


inline char* GetPVModelName(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return LTNULL;

	char* s_cPVModelName[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		"Models\\PV_Weapons\\PulseRifle_pv.abc",
		/*GUN_LASERCANNON_ID*/		"Models\\PV_Weapons\\lasercannon_pv.abc",				
		/*GUN_SPIDER_ID*/			"Models\\PV_Weapons\\Spider_pv.abc",
		/*GUN_BULLGUT_ID*/			"Models\\PV_Weapons\\Bullgut_pv.abc",
		/*GUN_SNIPERRIFLE_ID*/		"Models\\PV_Weapons\\SniperRifle_pv.abc",
		/*GUN_JUGGERNAUT_ID*/		"Models\\PV_Weapons\\Juggernaut_pv.abc",	
		/*GUN_SHREDDER_ID*/			"Models\\PV_Weapons\\Shredder_pv.abc",
		/*NOT USED1*/				LTNULL,
		/*GUN_REDRIOT_ID*/			"Models\\PV_Weapons\\RedRiot_pv.abc",
		/*GUN_ENERGYBATON_ID*/		"Models\\PV_Weapons\\energy_baton_pv.abc",
		/*GUN_ENERGYBLADE_ID*/		"Models\\PV_Weapons\\energy_blade_pv.abc",
		/*GUN_KATANA_ID*/			"Models\\PV_Weapons\\katana_pv.abc",
		/*GUN_MONOKNIFE_ID*/		"Models\\PV_Weapons\\monoknife_pv.abc",
		/*GUN_COLT45_ID*/			"Models\\PV_Weapons\\Colt45_pv.abc",
		/*GUN_SHOTGUN_ID*/			"Models\\PV_Weapons\\Shotgun_pv.abc",
		/*GUN_ASSAULTRIFLE_ID*/		"Models\\PV_Weapons\\AssaultRifle_pv.abc",
		/*GUN_ENERGYGRENADE_ID*/	"Models\\PV_Weapons\\EnergyGrenade_pv.abc",
		/*GUN_KATOGRENADE_ID*/		"Models\\PV_Weapons\\KatoGrenade_pv.abc",
		/*GUN_MAC10_ID*/			"Models\\PV_Weapons\\Machinegun_pv.abc",
		/*GUN_TOW_ID*/				"Models\\PV_Weapons\\TOW_pv.abc",
		/*NOT USED2*/				LTNULL,
		/*GUN_SQUEAKYTOY_ID*/		"Models\\PV_Weapons\\squeakytoy_pv.abc",
		/*GUN_TANTO_ID*/			"Models\\PV_Weapons\\tanto_pv.abc"
	};

	return s_cPVModelName[nWeaponId];
}

inline char* GetPVModelSkin(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return LTNULL;

	char* s_cPVModelSkin[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		"Skins\\weapons\\PulseRifle_pv_a.dtx",
		/*GUN_LASERCANNON_ID*/		"Skins\\weapons\\lasercannon_pv_a.dtx",				
		/*GUN_SPIDER_ID*/			"Skins\\weapons\\Spider_pv_a.dtx",
		/*GUN_BULLGUT_ID*/			"Skins\\weapons\\Bullgut_pv_a.dtx",
		/*GUN_SNIPERRIFLE_ID*/		"Skins\\weapons\\SniperRifle_pv_a.dtx",
		/*GUN_JUGGERNAUT_ID*/		"Skins\\weapons\\Juggernaut_pv_a.dtx",	
		/*GUN_SHREDDER_ID*/			"Skins\\weapons\\Shredder_pv_a.dtx",
		/*NOT USED1*/				LTNULL,
		/*GUN_REDRIOT_ID*/			"Skins\\weapons\\RedRiot_pv_a.dtx",
		/*GUN_ENERGYBATON_ID*/		"Skins\\weapons\\energy_baton_pv_a.dtx",
		/*GUN_ENERGYBLADE_ID*/		"Skins\\weapons\\energy_blade_pv_a.dtx",
		/*GUN_KATANA_ID*/			"Skins\\weapons\\katana_pv_a.dtx",
		/*GUN_MONOKNIFE_ID*/		"Skins\\weapons\\monoknife_pv_a.dtx",
		/*GUN_COLT45_ID*/			"Skins\\weapons\\Colt45_pv_a.dtx",
		/*GUN_SHOTGUN_ID*/			"Skins\\weapons\\Shotgun_pv_a.dtx",
		/*GUN_ASSAULTRIFLE_ID*/		"Skins\\weapons\\AssaultRifle_pv_a.dtx",
		/*GUN_ENERGYGRENADE_ID*/	"Skins\\weapons\\EnergyGrenade_pv_a.dtx",
		/*GUN_KATOGRENADE_ID*/		"Skins\\weapons\\KatoGrenade_pv_a.dtx",
		/*GUN_MAC10_ID*/			"Skins\\weapons\\Machinegun_pv_a.dtx",
		/*GUN_TOW_ID*/				"Skins\\weapons\\TOW_pv_a.dtx",
		/*NOT USED2*/				LTNULL,
		/*GUN_SQUEAKYTOY_ID*/		"Skins\\weapons\\squeakytoy_pv_a.dtx",
		/*GUN_TANTO_ID*/			"Skins\\weapons\\tanto_pv_a.dtx"
	};

	return s_cPVModelSkin[nWeaponId];
}


// get the velocity of this weapon (projectiles only)...

inline LTFLOAT GetWeaponVelocity(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0.0f;

	LTFLOAT s_fVelocity[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		2500.0f,
		/*GUN_LASERCANNON_ID*/		0.0f,				
		/*GUN_SPIDER_ID*/			800.0f,
		/*GUN_BULLGUT_ID*/			1000.0f,
		/*GUN_SNIPERRIFLE_ID*/		0.0f,
		/*GUN_JUGGERNAUT_ID*/		0.0f,	
		/*GUN_SHREDDER_ID*/			0.0f,
		/*NOT USED1*/				0.0f,
		/*GUN_REDRIOT_ID*/			0.0f,
		/*GUN_ENERGYBATON_ID*/		0.0f,
		/*GUN_ENERGYBLADE_ID*/		0.0f,
		/*GUN_KATANA_ID*/			0.0f,
		/*GUN_MONOKNIFE_ID*/		0.0f,
		/*GUN_COLT45_ID*/			0.0f,
		/*GUN_SHOTGUN_ID*/			0.0f,
		/*GUN_ASSAULTRIFLE_ID*/		0.0f,
		/*GUN_ENERGYGRENADE_ID*/	2000.0f,
		/*GUN_KATOGRENADE_ID*/		750.0f,
		/*GUN_MAC10_ID*/			0.0f,
		/*GUN_TOW_ID*/				2000.0f,
		/*NOT USED2*/				0.0f,
		/*GUN_SQUEAKYTOY_ID*/		0.0f,
		/*GUN_TANTO_ID*/			0.0f
	};

	return s_fVelocity[nWeaponId];
}


// get the lifetime of this projectile (projectiles only)...

inline LTFLOAT GetWeaponLifeTime(int nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || nWeaponId >= GUN_MAX_NUMBER) return 0.0f;

	LTFLOAT s_fLifeTime[GUN_MAX_NUMBER] =
	{
		/*GUN_PULSERIFLE_ID*/		4.0f,
		/*GUN_LASERCANNON_ID*/		0.0f,				
		/*GUN_SPIDER_ID*/			3.0f,
		/*GUN_BULLGUT_ID*/			6.0f,
		/*GUN_SNIPERRIFLE_ID*/		0.0f,
		/*GUN_JUGGERNAUT_ID*/		0.0f,	
		/*GUN_SHREDDER_ID*/			0.0f,
		/*NOT USED1*/				0.0f,
		/*GUN_REDRIOT_ID*/			0.0f,
		/*GUN_ENERGYBATON_ID*/		0.0f,
		/*GUN_ENERGYBLADE_ID*/		0.0f,
		/*GUN_KATANA_ID*/			0.0f,
		/*GUN_MONOKNIFE_ID*/		0.0f,
		/*GUN_COLT45_ID*/			0.0f,
		/*GUN_SHOTGUN_ID*/			0.0f,
		/*GUN_ASSAULTRIFLE_ID*/		0.0f,
		/*GUN_ENERGYGRENADE_ID*/	5.0f,
		/*GUN_KATOGRENADE_ID*/		3.0f,
		/*GUN_MAC10_ID*/			0.0f,
		/*GUN_TOW_ID*/				3.0f,
		/*NOT USED2*/				0.0f,
		/*GUN_SQUEAKYTOY_ID*/		0.0f,
		/*GUN_TANTO_ID*/			0.0f
	};

	return s_fLifeTime[nWeaponId];
}


inline LTBOOL CanWeaponZoom(int nWeaponId)
{
	if (nWeaponId == GUN_SNIPERRIFLE_ID || nWeaponId == GUN_ASSAULTRIFLE_ID) return LTTRUE;

	return LTFALSE;
}


// Calculate the adjusted path and fire position for the particular weapon...

void CalculateWeaponPathAndFirePos(RiotWeaponId nId, LTVector & vPath, 
								   LTVector & vFirePos, LTVector & vU, LTVector & vR);


#endif // __WEAPON_DEFS_H__