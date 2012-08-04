// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceTypes.h
//
// PURPOSE : Definition of surface types
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_TYPES_H__
#define __SURFACE_TYPES_H__

#include "PlayerModeTypes.h"
#include "WeaponDefs.h"
#include "stdio.h"

extern char s_FileBuffer[_MAX_PATH];

enum SurfaceType {
	ST_UNKNOWN				= 0,	// Unknown value
	ST_AIR					= 1,	// No a surface, but not the sky either
	ST_CHAINFENCE			= 9,	// Chain link fence	
	ST_STONE				= 10,	// Stone
	ST_STONE_HEAVY			= 11,	// Heavy stone
	ST_STONE_LIGHT			= 12,	// Light stone
	ST_METAL				= 20,	// Metal
	ST_METAL_HEAVY			= 21,	// Heavy metal
	ST_METAL_LIGHT			= 22,	// Light metal
	ST_METAL_HOLLOW			= 23,	// Hollow metal
	ST_METAL_HOLLOW_HEAVY	= 24,	// Hollow heavy metal
	ST_METAL_HOLLOW_LIGHT	= 25,	// Hollow light metal
	ST_WOOD					= 30,	// Wood
	ST_DENSE_WOOD			= 31,	// Dense wood
	ST_LIGHT_WOOD			= 32,	// Light wood
	ST_GLASS				= 40,	// Glass
	ST_ENERGY				= 50,	// Energy (Force fields, etc)
	ST_BUILDING				= 60,
	ST_TERRAIN				= 70,	// Dirt, ice, etc.
	ST_CLOTH				= 80,	// Cloth, carpet, furniture
	ST_PLASTIC				= 90,	// Linoleum, teflon
	ST_PLASTIC_HEAVY		= 91,	// Heavy plastic
	ST_PLASTIC_LIGHT		= 92,	// Light plastic
	ST_FLESH				= 100,
	ST_SKY					= 110,

	ST_MECHA				= 200,
	ST_LIQUID				= 201
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsMark
//
//	PURPOSE:	Does this type of surface show marks
//
// ----------------------------------------------------------------------- //

inline LTBOOL ShowsMark(SurfaceType eSurfType)
{
	LTBOOL bRet = LTTRUE;

	switch (eSurfType)
	{
		case ST_SKY:
		case ST_CHAINFENCE:
		case ST_MECHA:
		case ST_FLESH:
		case ST_LIQUID:
		case ST_AIR:
			bRet = LTFALSE;
		break;

		default : break;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSprite()
//
//	PURPOSE:	Get impact sprite with this surface
//
// ----------------------------------------------------------------------- //

inline char* GetImpactSprite(SurfaceType eSurfType, RiotWeaponId nId)
{
	char* pFile = LTNULL;

	if (GetWeaponType(nId) != VECTOR || !ShowsMark(eSurfType))
	{
		return LTNULL;
	}

	switch (nId)
	{
		case GUN_SNIPERRIFLE_ID :
		case GUN_COLT45_ID :
		case GUN_SHOTGUN_ID	:
		case GUN_ASSAULTRIFLE_ID :
		case GUN_MAC10_ID :
		case GUN_TANTO_ID :
		{
			char* pSurface = LTNULL; 

			switch (eSurfType)
			{
				case ST_STONE:
				case ST_STONE_HEAVY:
				case ST_STONE_LIGHT:
					pSurface = "stone1";
				break;

				case ST_UNKNOWN:
				case ST_METAL:
				case ST_METAL_HEAVY:
				case ST_METAL_LIGHT:
				case ST_METAL_HOLLOW:
				case ST_METAL_HOLLOW_HEAVY:
				case ST_METAL_HOLLOW_LIGHT:
					pSurface = "metal1";
				break;

				case ST_WOOD:
				case ST_DENSE_WOOD:
				case ST_LIGHT_WOOD:
					pSurface = "wood1";
				break;

				case ST_GLASS:
					pSurface = "glass1";
				break;

				case ST_ENERGY:
					pSurface = "energy1";
				break;

				case ST_BUILDING:
					pSurface = "building1";
				break;

				case ST_TERRAIN:
					pSurface = "terrain1";
				break;

				case ST_CLOTH:
					pSurface = "cloth1";
				break;

				case ST_PLASTIC:
				case ST_PLASTIC_HEAVY:
				case ST_PLASTIC_LIGHT:
					pSurface = "plastic1";
				break;

				default : break;
			}

			if (pSurface)
			{
				sprintf(s_FileBuffer,"Sprites\\Bulletholes\\%s.spr", pSurface);
				return s_FileBuffer;
			}
		}
		break;

		case GUN_LASERCANNON_ID :
		{
			sprintf(s_FileBuffer,"Sprites\\Bulletholes\\Laser1.spr");
			return s_FileBuffer;
		}
		break;

		default : break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

inline SurfaceType UserFlagToSurface(uint32 dwUserFlag)
{
	// Top byte contains surface flag

	SurfaceType eSurfType = (SurfaceType) (dwUserFlag >> 24);

	return eSurfType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

inline uint32 SurfaceToUserFlag(SurfaceType eSurfType)
{
	// Top byte should contain surface flag

	uint8	nByte = (uint8)eSurfType;
	uint32	dwUserFlag = (nByte << 24);

	return dwUserFlag;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetFootStepSound()
//
//	PURPOSE:	Get foot step sounds associated with this surface
//
// ----------------------------------------------------------------------- //

inline char* GetFootStepSound(SurfaceType eSurfType, int nPlayerMode, LTBOOL bLeftFoot)
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\Footsteps\\");


	// Based on the player type, get the right sound...

	if (nPlayerMode == PM_MODE_FOOT)
	{
		strcat(s_FileBuffer, "f");
	}
	else if (nPlayerMode == PM_MODE_KID)
	{
		strcat(s_FileBuffer, "k");
	}
	else  // Mech
	{
		strcat(s_FileBuffer, "m");
	}


	switch (eSurfType)
	{
		case ST_STONE:
		case ST_STONE_HEAVY:
		case ST_STONE_LIGHT:
		{
			strcat(s_FileBuffer, "stone1");
		}
		break;

		case ST_UNKNOWN:
		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_HEAVY:
		case ST_METAL_HOLLOW_LIGHT:
		{
			strcat(s_FileBuffer, "metal1");
		}
		break;

		case ST_WOOD:
		{
			strcat(s_FileBuffer, "wood1");
		}
		break;

		case ST_GLASS:
		case ST_DENSE_WOOD:
		case ST_LIGHT_WOOD:
		{
			strcat(s_FileBuffer, "glass1");
		}
		break;

		case ST_ENERGY:
		{
			strcat(s_FileBuffer, "energy1");
		}
		break;

		case ST_BUILDING:
		{
			strcat(s_FileBuffer, "building1");
		}
		break;

		case ST_TERRAIN:
		{
			strcat(s_FileBuffer, "terrain1");
		}
		break;

		case ST_CLOTH:
		{
			strcat(s_FileBuffer, "cloth1");
		}
		break;

		case ST_PLASTIC:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
		{
			strcat(s_FileBuffer, "plastic1");
		}
		break;

		case ST_FLESH:
		{
			strcat(s_FileBuffer, "flesh1");
		}
		break;

		case ST_LIQUID:
		{
			strcat(s_FileBuffer, "water1");
		}
		break;

		case ST_MECHA:
		case ST_SKY:
		default :
			return LTNULL;
		break;
	}


	// If we've got a sound, figure out if we should use left or right foot...

	if (s_FileBuffer[0] && s_FileBuffer[0] != ' ')
	{
		strcat(s_FileBuffer, bLeftFoot ? "a.wav" : "b.wav");
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSoundDir()
//
//	PURPOSE:	Get impact sound directory associated with this surface
//
// ----------------------------------------------------------------------- //

inline char* GetImpactSoundDir(SurfaceType eSurfType)
{
	char *pDir = LTNULL;

	switch (eSurfType)
	{
		case ST_STONE:
		case ST_STONE_HEAVY:
		case ST_STONE_LIGHT:
			pDir = "Stone";
		break;

		case ST_UNKNOWN:
		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_HEAVY:
		case ST_METAL_HOLLOW_LIGHT:
			pDir = "Metal";
		break;

		case ST_WOOD:
		case ST_DENSE_WOOD:
		case ST_LIGHT_WOOD:
			pDir = "Wood";
		break;

		case ST_GLASS:
			pDir = "Glass";
		break;

		case ST_ENERGY:
			pDir = "Energy";
		break;

		case ST_BUILDING:
			pDir = "Metal";
		break;

		case ST_TERRAIN:
			pDir = "Terrain";
		break;

		case ST_CLOTH:
			pDir = "Cloth";
		break;

		case ST_PLASTIC:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
			pDir = "Plastic";
		break;

		case ST_FLESH:
			pDir = "Flesh";
		break;

		case ST_MECHA:
		case ST_LIQUID:
		case ST_SKY:
		default :
		break;
	}

	return pDir;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSound()
//
//	PURPOSE:	Get impact sounds associated with this surface
//
// ----------------------------------------------------------------------- //

inline char* GetImpactSound(SurfaceType eSurfType, RiotWeaponId nId)
{
	char* pSound = LTNULL;

	switch (nId)
	{
		case GUN_PULSERIFLE_ID :
			pSound = "Sounds\\Weapons\\PulseRifle\\impact.wav";
		break;

		case GUN_SPIDER_ID :
			pSound = "Sounds\\Weapons\\Spider\\impact.wav";
		break;

		case GUN_BULLGUT_ID :
			pSound = "Sounds\\Weapons\\Bullgut\\impact.wav";	
		break;

		case GUN_JUGGERNAUT_ID :
			pSound = "Sounds\\Weapons\\Juggernaut\\impact.wav";
		break;

		case GUN_SHREDDER_ID :
			pSound = "Sounds\\Weapons\\Shredder\\impact.wav";
		break;

		case GUN_REDRIOT_ID :
			pSound = "Sounds\\Weapons\\RedRiot\\impact.wav";
		break;

		case GUN_ENERGYBATON_ID :
			pSound = "Sounds\\Weapons\\EnergyBaton\\impact.wav";
		break;

		case GUN_ENERGYBLADE_ID :
			pSound = "Sounds\\Weapons\\EnergyBlade\\impact.wav";
		break;

		case GUN_KATANA_ID :
			pSound = "Sounds\\Weapons\\Katana\\impact.wav";
		break;

		case GUN_MONOKNIFE_ID :
			pSound = "Sounds\\Weapons\\MonoKnife\\impact.wav";
		break;

		case GUN_TANTO_ID :
			pSound = "Sounds\\Weapons\\Tanto\\impact.wav";
		break;

		case GUN_COLT45_ID :
		case GUN_SHOTGUN_ID	:
		case GUN_ASSAULTRIFLE_ID :
		case GUN_MAC10_ID :
		case GUN_SNIPERRIFLE_ID :
		{
			char *pDir = LTNULL;

			pDir = GetImpactSoundDir( eSurfType );
			if (pDir)
			{
				sprintf(s_FileBuffer,"Sounds\\Weapons\\impacts\\%s\\impact%d.wav", pDir, GetRandom(1,5));
				return s_FileBuffer;
			}
		}
		break;

		case GUN_ENERGYGRENADE_ID :
			pSound = "Sounds\\Weapons\\EnergyGrenade\\impact.wav";
		break;

		case GUN_KATOGRENADE_ID :
			pSound = "Sounds\\Weapons\\KatoGrenade\\impact.wav";
		break;

		case GUN_TOW_ID	:
			pSound = "Sounds\\Weapons\\TOW\\impact.wav";
		break;

		case GUN_LASERCANNON_ID :
			pSound = "Sounds\\Weapons\\LaserCannon\\impact.wav";
		break;

		default : break;
	}

	return pSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetBounceSound()
//
//	PURPOSE:	Get bounce sound associated with this surface
//
// ----------------------------------------------------------------------- //

inline char* GetBounceSound(SurfaceType eSurfType, RiotWeaponId nId)
{
	char* pSound = LTNULL;

	switch (nId)
	{
		case GUN_KATOGRENADE_ID:
		{
			if (GetRandom(0,1) == 0)
			{
				return "Sounds\\Weapons\\Katogrenade\\Bounce1.wav";
			}
			else
			{
				return "Sounds\\Weapons\\Katogrenade\\Bounce2.wav";
			}
		}
		break;
		
		default : break;
	}

	return pSound;
}

#endif // __SURFACE_TYPES_H__