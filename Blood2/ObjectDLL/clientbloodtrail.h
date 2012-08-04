
 /****************************************************************************
 ;
 ;	MODULE:		clientbloodtrail.h
 ;
 ;	PURPOSE:	server-side blood trail effects header
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/21/98 1:51:48 PM
 ;
 ;	COMMENT:	Copyright (c) 1998, Monolith Productions Inc.
 ;
 ****************************************************************************/

#ifndef __CLIENT_BLOOD_TRAIL_H__
#define __CLIENT_BLOOD_TRAIL_H__

#include "ClientSFX.h"


class CClientBloodTrail : public CClientSFX
{
	public :
			
		void Setup(DVector vVel, DVector vColor);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		void Update();

		DBOOL		m_bAlive;
		DFLOAT		m_fScale;
};

#endif // __CLIENT_BLOOD_TRAIL_H__
