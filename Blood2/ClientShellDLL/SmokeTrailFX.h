// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeTrailFX.h
//
// PURPOSE : SmokeTrail special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKE_TRAIL_FX_H__
#define __SMOKE_TRAIL_FX_H__

#include "SpecialFX.h"


struct STCREATESTRUCT : public SFXCREATESTRUCT
{
	STCREATESTRUCT::STCREATESTRUCT();

	DVector vVel;
	DBOOL   bSmall;
};

inline STCREATESTRUCT::STCREATESTRUCT()
{
	memset(this, 0, sizeof(STCREATESTRUCT));
}

class CSmokeTrailFX : public CSpecialFX
{
	public :

		CSmokeTrailFX() : CSpecialFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vVel);
			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);

			m_fStartTime	= -1.0f;
			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_fSegmentTime	= 1.0f;
			m_bSmall		= DFALSE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vLastPos;		// Last smoke particle position
		DVector	m_vVel;			// Velocity of smoking projectile
		DVector	m_vColor1;		// Color of darkest smoke particles
		DVector	m_vColor2;		// Color of lightest smoke particles

		DFLOAT	m_fSegmentTime;	// When should we create a new segment
		DFLOAT	m_fFadeTime;	// When should segment start to fade
		DFLOAT	m_fLifeTime;	// How long segment stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		int		m_nNumPerPuff;	// Number of particles per smoke puff
		DBOOL	m_bSmall;		// Relative size of smoke
};

#endif // __SMOKE_TRAIL_FX_H__