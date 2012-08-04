// ----------------------------------------------------------------------- //
//
// MODULE  : ClientTracer.h
//
// PURPOSE : ClientTracer - Definition
//
// CREATED : 12/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTTRACER_H__
#define __CLIENTTRACER_H__

#include "ClientSFX.h"


class CClientTracer : public CClientSFX
{
	public:

		void Setup(DVector *vTo, HOBJECT hGun);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
};


#endif // __CLIENTTRACER_H__
