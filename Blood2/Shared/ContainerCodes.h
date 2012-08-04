// ----------------------------------------------------------------------- //
//
// MODULE  : ContainerCodes.h
//
// PURPOSE : Blood 2 container codes
//
// CREATED : 3/2/98
//
// ----------------------------------------------------------------------- //

#ifndef __CONTAINERCODES_H__
#define __CONTAINERCODES_H__


enum ContainerCode {
	CC_NOTHING=0,
	CC_WATER,			// Water
	CC_BLOOD,			// Pools o' Blood
	CC_ACID,			// Acid
	CC_WIND,			// Wind
	CC_LADDER,			// Ladders
	CC_CONVEYOR,		// Conveyor belt
// Level specific containers
	CC_CRANECONTROL,	// Control for the crane
	CC_DAMAGE,			// Causes damage or instant death.
	CC_FREEFALL,		// Freefall
	CC_MINEFIELD,		// Instant death with an explosion
	CC_MAX_CONTAINER_CODES
};


inline DBOOL IsLiquid(ContainerCode code)
{
	DBOOL bRet = DFALSE;

	switch (code)
	{
		case CC_WATER:
		case CC_BLOOD:
		case CC_ACID:
		case CC_FREEFALL:
			bRet = DTRUE;
		break;

		default : break;
	}

	return bRet;
}


inline DBOOL IsFreeMovement(ContainerCode code)
{
	return(IsLiquid(code) || (code == CC_LADDER));
}


inline DBOOL IsSpecialControl(ContainerCode code)
{
	return((code == CC_CRANECONTROL));
}


#endif // __CONTAINERCODES_H__
