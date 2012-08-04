// ----------------------------------------------------------------------- //
//
// MODULE  : BulletImpactSFX.h
//
// PURPOSE : BulletImpactSFX - Definition
//
// CREATED : 1/12/97
//
// ----------------------------------------------------------------------- //

#ifndef __BULLETIMPACTSFX_H__
#define __BULLETIMPACTSFX_H__

#include "ClientSFX.h"



class CBulletImpactSFX : public CClientSFX
{
	public :
		
		void Setup( DVector *pvPos, DVector *pvDir, DFLOAT fScale, HSTRING hstrSprite );

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	
};


#endif // __BULLETIMPACT_H__
