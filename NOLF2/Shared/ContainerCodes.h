// ----------------------------------------------------------------------- //
//
// MODULE  : ContainerCodes.h
//
// PURPOSE : Container code definitions
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __CONTAINER_CODES_H__
#define __CONTAINER_CODES_H__

#include "ClientServerShared.h"
#include "CommonUtilities.h"

#ifdef _CLIENTBUILD
#include "iclientshell.h"
#endif

#define DEFAULT_BUBBLE_TEXTURE		"FX\\Particles\\bubble1_1.dtx"

#define MAX_CONTAINER_VISCOSITY		5000.0f
#define MAX_OVERLAPPING_CONTAINERS	20

// [KLS 6/28/02] - Make sure you update the CC_XXX_FLAGs below if you add
// a new container code!!!!
enum ContainerCode 
{
	CC_NO_CONTAINER=0,
	CC_WATER=1,
	CC_CORROSIVE_FLUID=2,
	CC_ICE=3,
	CC_FREEZING_WATER=4,
	CC_POISON_GAS=5,
	CC_ELECTRICITY=6,
	CC_ENDLESS_FALL=7,
	CC_WIND=8,
	CC_COLDAIR=9,
	CC_BURN=10,
	CC_LADDER=11,
	CC_WEATHER=12,
	CC_VOLUME=13,
	CC_FILTER=14,
	CC_SAFTEY_NET=15,
	CC_TRANSITION_AREA=16,
	CC_JUMP_VOLUME=17,
	CC_DYNAMIC_OCCLUDER_VOLUME=18,
	CC_GRAVITY=19,
	CC_MAX_CONTAINER_CODES=20
};

#define CC_NO_CONTAINER_FLAG			(1<<0)
#define CC_WATER_FLAG					(1<<1)
#define CC_CORROSIVE_FLUID_FLAG			(1<<2)
#define CC_ICE_FLAG						(1<<3)
#define CC_FREEZING_WATER_FLAG			(1<<4)
#define CC_POISON_GAS_FLAG				(1<<5)
#define CC_ELECTRICITY_FLAG				(1<<6)
#define CC_ENDLESS_FALL_FLAG			(1<<7)
#define CC_WIND_FLAG					(1<<8)
#define CC_COLDAIR_FLAG					(1<<9)
#define CC_BURN_FLAG					(1<<10)
#define CC_LADDER_FLAG					(1<<11)
#define CC_WEATHER_FLAG					(1<<12)
#define CC_VOLUME_FLAG					(1<<13)
#define CC_FILTER_FLAG					(1<<14)
#define CC_SAFTEY_NET_FLAG				(1<<15)
#define CC_TRANSITION_AREA_FLAG			(1<<16)
#define CC_JUMP_VOLUME_FLAG				(1<<17)
#define CC_DYNAMIC_OCCLUDER_VOLUME_FLAG	(1<<18)
#define CC_GRAVITY_FLAG					(1<<19)
#define CC_ALL_FLAG						0xFFFFFFFF


// Containers the player doesn't care about...

#define CC_PLAYER_IGNORE_FLAGS (CC_VOLUME | CC_NO_CONTAINER_FLAG | CC_WEATHER_FLAG)


// Following are flags used by the Weather container...

#define WFLAG_LIGHT_RAIN	(1<<0)
#define WFLAG_NORMAL_RAIN	(1<<1)
#define WFLAG_HEAVY_RAIN	(1<<2)
#define	WFLAG_LIGHT_SNOW	(1<<3)
#define	WFLAG_NORMAL_SNOW	(1<<4)
#define	WFLAG_HEAVY_SNOW	(1<<5)

#define	WFLAG_RAIN			(WFLAG_LIGHT_RAIN | WFLAG_NORMAL_RAIN | WFLAG_HEAVY_RAIN)
#define	WFLAG_SNOW			(WFLAG_LIGHT_SNOW | WFLAG_NORMAL_SNOW | WFLAG_HEAVY_SNOW)

inline uint32 GetPointContainers(const LTVector vPos, HOBJECT* pList, uint32 nMaxListSize,
								 uint32 nCCFlags=CC_ALL_FLAG)
{	
	if (!pList || !nMaxListSize) return 0;

	HOBJECT ObjList[MAX_OVERLAPPING_CONTAINERS];
	uint32 dwNum = g_pLTBase->GetPointContainers(const_cast<LTVector*>(&vPos), ObjList, MAX_OVERLAPPING_CONTAINERS);

	// Build our list of containers we care about...

	uint32 nListSize = 0;
	for (uint32 i=0; i < dwNum; i++)
	{
		if (ObjList[i])
		{
			uint32 dwUserFlags = 0;
			g_pCommonLT->GetObjectFlags(ObjList[i], OFT_User, dwUserFlags);

			// We only care about visible containers...

			if ((dwUserFlags & USRFLG_VISIBLE))
			{
				uint16 code;
				if (g_pLTBase->GetContainerCode(ObjList[i], &code))
				{
					// Check and see if this is a container we care about...

					uint32 nFlag = (1<<code);

					if (nFlag & nCCFlags)
					{
						// Add it to the list if we have room...

						if (nListSize < nMaxListSize)
						{
							pList[nListSize] = ObjList[i];
							nListSize++;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}

	return nListSize;
}


inline ContainerCode GetContainerCode(const LTVector &vPos, uint32 nCCIngoreFlags=CC_PLAYER_IGNORE_FLAGS)
{
	ContainerCode eContainerCode = CC_NO_CONTAINER;

	// Get all the containers we care about...

	HOBJECT objList[MAX_OVERLAPPING_CONTAINERS];
	uint32 dwNum = ::GetPointContainers(vPos, objList, MAX_OVERLAPPING_CONTAINERS, (CC_ALL_FLAG & ~nCCIngoreFlags));

	if (dwNum > 0)
	{
        uint16 code;
        if (g_pLTBase->GetContainerCode(objList[0], &code))
		{
			eContainerCode = (ContainerCode)code;
		}
	}

	return eContainerCode;
}

inline bool IsLiquid(ContainerCode code)
{
    bool bRet = false;

	switch (code)
	{
		case CC_WATER:
		case CC_CORROSIVE_FLUID:
		case CC_FREEZING_WATER:
            bRet = true;
		break;

		default : break;
	}

	return bRet;
}

inline uint32 GetLiquidFlags()
{
	// This should probably be sometype of static lookup, but this method
	// makes adding new liquid flags easier (i.e., you only need to update
	// IsLiquid() above...

	uint32 nFlags = 0;
	for (int i=0; i < CC_MAX_CONTAINER_CODES; i++)
	{
		if ( IsLiquid( (ContainerCode)i ) )
		{
			nFlags |= (1<<i);
		}
	}

	return nFlags;
}

inline bool GetLiquidColorRange(ContainerCode code, LTVector* pvC1, LTVector* pvC2)
{
    if (!IsLiquid(code) || !pvC1 || !pvC2) return false;

	switch (code)
	{
		case CC_WATER:
			VEC_SET(*pvC1, 235.0f, 235.0f, 235.0f);
			VEC_SET(*pvC2, 235.0f, 235.0f, 255.0f);
		break;

		case CC_CORROSIVE_FLUID:
			VEC_SET(*pvC2, 200.0f, 200.0f, 0.0f);
			VEC_SET(*pvC2, 255.0f, 255.0f, 0.0f);
		break;

		case CC_FREEZING_WATER:
			VEC_SET(*pvC2, 200.0f, 200.0f, 255.0f);
			VEC_SET(*pvC2, 200.0f, 200.0f, 255.0f);
		break;

		default : break;
	}

    return true;
}

inline bool IsFreeMovement(ContainerCode code)
{
	return(IsLiquid(code) || (code == CC_LADDER));
}

#endif // __CONTAINER_CODES_H__