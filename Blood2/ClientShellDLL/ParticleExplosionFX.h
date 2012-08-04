// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleExplosionFX.h
//
// PURPOSE : Particle explosion - Definition
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_EXPLOSION_FX_H__
#define __PARTICLE_EXPLOSION_FX_H__

#include "BaseParticleSystemFX.h"
#include "client_physics.h"

#define MAX_EMITTERS 10

struct PESCREATESTRUCT : public SFXCREATESTRUCT
{
	PESCREATESTRUCT::PESCREATESTRUCT();

	DRotation	rSurfaceRot;
	DVector		vPos;
	DVector		vColor1;
	DVector		vColor2;
	DVector		vMinVel;
	DVector		vMaxVel;
	DVector		vMinDriftOffset;
	DVector		vMaxDriftOffset;
	DFLOAT		fLifeTime;
	DFLOAT		fFadeTime;
	DFLOAT		fOffsetTime;
	DFLOAT		fRadius;
	DFLOAT		fGravity;
	DBYTE		nSurfaceType;
	DBYTE		nNumPerPuff;
	DBYTE		nNumEmitters;
	DBYTE		nEmitterFlags;
	DBYTE		nNumSteps;
	DBOOL		bSmall;
	DBOOL		bCreateDebris;
	DBOOL		bRotateDebris;
	DBOOL		bIgnoreWind;
	DBOOL		bBounce;
	char*		pFilename;
};

inline PESCREATESTRUCT::PESCREATESTRUCT()
{
	memset(this, 0, sizeof(PESCREATESTRUCT));
	ROT_INIT(rSurfaceRot);
	VEC_SET(vMinDriftOffset, 0.0f, 5.0f, 0.0f);
	VEC_SET(vMaxDriftOffset, 0.0f, 6.0f, 0.0f);
	nNumSteps = 2;
}

class CParticleExplosionFX : public CBaseParticleSystemFX
{
	public :

		CParticleExplosionFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vMinVel);
			VEC_INIT(m_vMaxVel);
			VEC_INIT(m_vMinDriftOffset);
			VEC_INIT(m_vMaxDriftOffset);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_bSmall		= DFALSE;
			m_nNumSteps		= 2;

			m_bFirstUpdate	= DTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			m_nSurfaceType	= 0;
			m_bIgnoreWind	= DFALSE;

			memset(m_Emitters, 0, sizeof(MovingObject)*MAX_EMITTERS);
			memset(m_ActiveEmitters, 0, sizeof(DBOOL)*MAX_EMITTERS);
			memset(m_BounceCount, 0, sizeof(DBYTE)*MAX_EMITTERS);
			memset(m_hDebris, 0, sizeof(HOBJECT)*MAX_EMITTERS);

			m_nNumEmitters		= 0;
			m_nEmitterFlags		= 0;

			m_bCreateDebris		= DFALSE;
			m_bRotateDebris		= DFALSE;
			m_fPitch			= 0.0f;
			m_fYaw				= 0.0f;
			m_fPitchVel			= 0.0f;
			m_fYawVel			= 0.0f;
		}

		~CParticleExplosionFX()
		{
			for (int i=0; i < m_nNumEmitters; i++)
			{
				if (m_hDebris[i] && m_pClientDE)
				{
					m_pClientDE->DeleteObject(m_hDebris[i]);
				}
			}
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		DRotation m_rSurfaceRot;	// Rotation of surface
		DVector	m_vLastPos;			// Last Particle particle position
		DVector	m_vMinVel;			// Minimum emmitter velocity
		DVector	m_vMaxVel;			// Maximum emmitter velocity
		DVector	m_vMinDriftOffset;	// Particle min drift offset
		DVector	m_vMaxDriftOffset;	// Particle max drift offset

		DFLOAT	m_fFadeTime;	// When system should start to fade
		DFLOAT	m_fLifeTime;	// How long system stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fOffsetTime;	// Time between particles

		DBYTE	m_nNumPerPuff;	// Number of particles per Particle puff
		DBOOL	m_bSmall;		// Relative size of Particle

		DFLOAT	m_fLastTime;	// Last time we created some particles
		DBOOL	m_bFirstUpdate;	// First update
		DBOOL	m_bIgnoreWind;	// Ignore world wind?
		DBOOL	m_bCreateDebris;// Should we create debris?
		DBYTE	m_nSurfaceType;	// Impact surface
		DBYTE	m_nNumSteps;	// Number of steps between each particle puff
		DBOOL	m_bBounce;

		MovingObject	m_Emitters[MAX_EMITTERS];				// Particle emmitters
		DBYTE			m_nNumEmitters;						// Num in array
		DBYTE			m_nEmitterFlags;						// MoveObject flags
		DBOOL			m_ActiveEmitters[MAX_EMITTERS];		// Active?	
		DBYTE			m_BounceCount[MAX_EMITTERS];			// Number of bounces
		HLOCALOBJ		m_hDebris[MAX_EMITTERS];

		// Emmitter rotation stuff...
		
		DBOOL			m_bRotateDebris;
		DFLOAT			m_fPitch;		
		DFLOAT			m_fYaw;
		DFLOAT			m_fPitchVel;
		DFLOAT			m_fYawVel;

		DBOOL UpdateEmmitter(MovingObject* pObject, ClientIntersectInfo* pInfo);
		void AddParticles(MovingObject* pObject);
		HLOCALOBJ CreateDebris();
};

#endif // __PARTICLE_EXPLOSION_FX_H__