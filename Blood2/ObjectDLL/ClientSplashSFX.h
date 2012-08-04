// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSplashSFX.h
//
// PURPOSE : CClientSplashSFX - Definition
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SPLASH_SFX_H__
#define __CLIENT_SPLASH_SFX_H__

#include "ClientSFX.h"

class CClientSplashSFX : public CClientSFX
{
	public :

	void CClientSplashSFX::Setup(DVector *vPos, DVector *vDir, DFLOAT fRadius, DFLOAT fPosRadius, DFLOAT fHeight,
								 DFLOAT fDensity, DFLOAT fSpread, DVector *vColor1, DVector *vColor2, DVector *vRippleScale,
								 DBOOL bRipple, DFLOAT fSprayTime, DFLOAT fDuration, DFLOAT fGravity, char *pFile);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

};

#endif // __CLIENT_SPLASH_SFX_H__