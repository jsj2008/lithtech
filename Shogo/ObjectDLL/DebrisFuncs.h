// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFuncs.h
//
// PURPOSE : Misc functions for creating debris
//
// CREATED : 6/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_FUNCS_H__
#define __DEBRIS_FUNCS_H__

// Includes...

#include "basetypes_de.h"
#include <memory.h>  // for memset

struct CLIENTDEBRIS
{
	CLIENTDEBRIS::CLIENTDEBRIS();

	DRotation	rRot;
	DVector		vPos;
	DVector		vMinVel;
	DVector		vMaxVel;
	DFLOAT		fLifeTime;
	DFLOAT		fFadeTime;
	DFLOAT		fMinScale;
	DFLOAT		fMaxScale;
	DBYTE		nNumDebris;
	DBYTE		nDebrisFlags;
	DBYTE		nDebrisType;
	DBOOL		bRotate;
	DBOOL		bPlayBounceSound;
	DBOOL		bPlayExplodeSound;
};

inline CLIENTDEBRIS::CLIENTDEBRIS()
{
	memset(this, 0, sizeof(CLIENTDEBRIS));
	ROT_INIT(rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateDebris()
//
//	PURPOSE:	Create client-side debris...
//
// ----------------------------------------------------------------------- //

void CreateDebris(CLIENTDEBRIS & cd);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris(DVector & vPos, DFLOAT fDimsMag, 
					  DVector & vDir, DBYTE nDebrisType, 
					  DBYTE nMinNum=10, DBYTE nMaxNum=20);


#endif // __DEBRIS_FUNCS_H__