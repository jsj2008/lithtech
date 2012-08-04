// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSmokeTrail.h
//
// PURPOSE : CClientSmokeTrail - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SMOKE_TRAIL_H__
#define __CLIENT_SMOKE_TRAIL_H__

#include "ClientSFX.h"


class CClientSmokeTrail : public CClientSFX
{
	public :

		void Setup(DVector vVel, DBOOL bSmall);
};

#endif // __CLIENT_SMOKE_TRAIL_H__
