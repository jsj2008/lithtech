// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSplatFX.h
//
// PURPOSE : CClientSplatFX - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTSPLATFX_H__
#define __CLIENTSPLATFX_H__

#include "ClientSFX.h"


class CClientSplatFX : public CClientSFX
{
	public :
		
		void Setup( DVector *pvPos, DVector *pvDir, DFLOAT fScale, DFLOAT fGrowScale);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
};

#endif // __CLIENTSPLATFX_H__
