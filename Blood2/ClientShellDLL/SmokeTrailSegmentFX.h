// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeTrailSegmentFX.h
//
// PURPOSE : SmokeTrail segment special fx class - Definition
//
// CREATED : 3/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKE_TRAIL_SEGMENT_FX_H__
#define __SMOKE_TRAIL_SEGMENT_FX_H__

#include "BaseParticleSystemFX.h"


struct STSCREATESTRUCT : public SFXCREATESTRUCT
{
	STSCREATESTRUCT::STSCREATESTRUCT();

	DVector vVel;
	DVector vColor1;
	DVector vColor2;
	DBOOL   bSmall;
	DFLOAT  fLifeTime;
	DFLOAT	fFadeTime;
	DFLOAT  fOffsetTime;
	DFLOAT	fRadius;
	DFLOAT  fGravity;
	DBYTE   nNumPerPuff;
};

inline STSCREATESTRUCT::STSCREATESTRUCT()
{
	memset(this, 0, sizeof(STSCREATESTRUCT));
}

class CSmokeTrailSegmentFX : public CBaseParticleSystemFX
{
	public :

		CSmokeTrailSegmentFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vVel);
			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_bSmall		= DFALSE;

			m_bFirstUpdate	= DTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			m_bIgnoreWind	= DFALSE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DVector	m_vLastPos;		// Last smoke particle position
		DVector	m_vVel;			// Velocity of smoking projectile
		DVector	m_vColor1;		// Color of darkest smoke particles
		DVector	m_vColor2;		// Color of lightest smoke particles

		DFLOAT	m_fFadeTime;	// When system should start to fade
		DFLOAT	m_fLifeTime;	// How long system stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		DBYTE	m_nNumPerPuff;	// Number of particles per smoke puff
		DBOOL	m_bSmall;		// Relative size of smoke

		DFLOAT	m_fLastTime;	// Last time we created some particles
		DBOOL	m_bFirstUpdate;	// First update
		DBOOL	m_bIgnoreWind;	// Ignore world wind?
};

#endif // __SMOKE_TRAIL_SEGMENT_FX_H__