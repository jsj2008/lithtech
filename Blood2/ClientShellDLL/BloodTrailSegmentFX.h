
 /****************************************************************************
 ;
 ;	MODULE:		BloodTrailSegmentFX.h
 ;
 ;	PURPOSE:	Blood trail effect header
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/20/98 8:08:13 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#ifndef __BLOOD_TRAIL_SEGMENT_FX_H__
#define __BLOOD_TRAIL_SEGMENT_FX_H__

#include "BaseParticleSystemFX.h"


struct BTSCREATESTRUCT : public SFXCREATESTRUCT
{
	BTSCREATESTRUCT::BTSCREATESTRUCT();

	DVector vColor1;
	DFLOAT  fLifeTime;
	DFLOAT  fOffsetTime;
	DFLOAT	fRadius;
	DBYTE   nNumPerPuff;
};

inline BTSCREATESTRUCT::BTSCREATESTRUCT()
{
	memset(this, 0, sizeof(BTSCREATESTRUCT));
}

class CBloodTrailSegmentFX : public CBaseParticleSystemFX
{
	public :

		CBloodTrailSegmentFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vColor1);

			m_fLifeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;

			m_bFirstUpdate	= DTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DVector	m_vLastPos;		// Last blood particle position
		DVector	m_vColor1;		// Color of darkest blood particles

		DFLOAT	m_fLifeTime;	// How long system stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		DBYTE	m_nNumPerPuff;	// Number of particles per blood puff

		DFLOAT	m_fLastTime;	// Last time we created some particles
		DBOOL	m_bFirstUpdate;	// First update
};

#endif // __BLOOD_TRAIL_SEGMENT_FX_H__