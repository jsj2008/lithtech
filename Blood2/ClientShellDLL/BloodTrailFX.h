
 /****************************************************************************
 ;
 ;	MODULE:		BloodTrailFX.h
 ;
 ;	PURPOSE:	Blood trail effects header
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/21/98 1:28:35 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#ifndef __BLOOD_TRAIL_FX_H__
#define __BLOOD_TRAIL_FX_H__

#include "SpecialFX.h"


struct BTCREATESTRUCT : public SFXCREATESTRUCT
{
	BTCREATESTRUCT::BTCREATESTRUCT();

	DVector vVel;
	DVector vColor;
};

inline BTCREATESTRUCT::BTCREATESTRUCT()
{
	memset(this, 0, sizeof(BTCREATESTRUCT));
}

class CBloodTrailFX : public CSpecialFX
{
	public :

		CBloodTrailFX() : CSpecialFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vVel);
			VEC_INIT(m_vColor);

			m_fStartTime	= -1.0f;
			m_fLifeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_fSegmentTime	= 1.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vLastPos;		// Last blood particle position
		DVector	m_vVel;			// Velocity of smoking projectile
		DVector	m_vColor;		// Color of darkest blood particles

		DFLOAT	m_fSegmentTime;	// When should we create a new segment
		DFLOAT	m_fLifeTime;	// How long segment stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		int		m_nNumPerPuff;	// Number of particles per blood puff
};

#endif // __BLOOD_TRAIL_FX_H__