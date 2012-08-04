// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailSegmentFX.h
//
// PURPOSE : ParticleTrail segment special fx class - Definition
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_TRAIL_SEGMENT_FX_H__
#define __PARTICLE_TRAIL_SEGMENT_FX_H__

#include "BaseParticleSystemFX.h"


struct PTSCREATESTRUCT : public SFXCREATESTRUCT
{
	PTSCREATESTRUCT::PTSCREATESTRUCT();

	uint8	nType;
	LTVector vColor1;
	LTVector vColor2;
	LTVector vDriftOffset;
	LTBOOL   bSmall;
	LTFLOAT  fLifeTime;
	LTFLOAT	fFadeTime;
	LTFLOAT  fOffsetTime;
	LTFLOAT	fRadius;
	LTFLOAT  fGravity;
	uint8   nNumPerPuff;
};

inline PTSCREATESTRUCT::PTSCREATESTRUCT()
{
	memset(this, 0, sizeof(PTSCREATESTRUCT));
}

class CParticleTrailSegmentFX : public CBaseParticleSystemFX
{
	public :

		CParticleTrailSegmentFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vDriftOffset);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_bSmall		= LTFALSE;
			m_nType			= 0;

			m_bFirstUpdate	= LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			m_bIgnoreWind	= LTFALSE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

	private :

		LTVector	m_vLastPos;		// Last Particle particle position
		LTVector	m_vDriftOffset;	// Particle drift offset

		LTFLOAT	m_fFadeTime;	// When system should start to fade
		LTFLOAT	m_fLifeTime;	// How long system stays around
		LTFLOAT	m_fStartTime;	// When did we start this crazy thing
		LTFLOAT	m_fOffsetTime;	// Time between particles

		uint8	m_nNumPerPuff;	// Number of particles per Particle puff
		LTBOOL	m_bSmall;		// Relative size of Particle
		uint8	m_nType;		// Type of particle

		LTFLOAT	m_fLastTime;	// Last time we created some particles
		LTBOOL	m_bFirstUpdate;	// First update
		LTBOOL	m_bIgnoreWind;	// Ignore world wind?
};

#endif // __PARTICLE_TRAIL_SEGMENT_FX_H__