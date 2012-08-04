// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSparksSFX.h
//
// PURPOSE : CClientSparksSFX - Definition
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SPARKS_SFX_H__
#define __CLIENT_SPARKS_SFX_H__

#include "ClientSFX.h"


class CClientSparksSFX : public CClientSFX
{
	public :
		
		void Setup(DVector *pvDir, DVector *pvColor1, DVector *pvColor2,
				   char* pFile, DBYTE nSparks, DFLOAT fDuration, DFLOAT fEmissionRadius,
				   DFLOAT fParticleRadius, DFLOAT fGravity = -500.0f);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	
};

#endif // __CLIENT_SPARKS_SFX_H__
