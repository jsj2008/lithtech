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

#define MAX_EMMITTERS 10

struct PESCREATESTRUCT : public SFXCREATESTRUCT
{
	PESCREATESTRUCT::PESCREATESTRUCT();

	LTRotation	rSurfaceRot;
	LTVector		vPos;
	LTVector		vColor1;
	LTVector		vColor2;
	LTVector		vMinVel;
	LTVector		vMaxVel;
	LTVector		vMinDriftOffset;
	LTVector		vMaxDriftOffset;
	LTFLOAT		fLifeTime;
	LTFLOAT		fFadeTime;
	LTFLOAT		fOffsetTime;
	LTFLOAT		fRadius;
	LTFLOAT		fGravity;
	uint8		nSurfaceType;
	uint8		nNumPerPuff;
	uint8		nNumEmmitters;
	uint8		nEmmitterFlags;
	uint8		nNumSteps;
	LTBOOL		bSmall;
	LTBOOL		bCreateDebris;
	LTBOOL		bRotateDebris;
	LTBOOL		bIgnoreWind;
	char*		pFilename;
};

inline PESCREATESTRUCT::PESCREATESTRUCT()
{
	memset(this, 0, sizeof(PESCREATESTRUCT));
	rSurfaceRot.Init();
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
			m_bSmall		= LTFALSE;
			m_nNumSteps		= 2;

			m_bFirstUpdate	= LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			m_nSurfaceType	= 0;
			m_bIgnoreWind	= LTFALSE;

			memset(m_Emmitters, 0, sizeof(MovingObject)*MAX_EMMITTERS);
			memset(m_ActiveEmmitters, 0, sizeof(LTBOOL)*MAX_EMMITTERS);
			memset(m_BounceCount, 0, sizeof(uint8)*MAX_EMMITTERS);
			memset(m_hDebris, 0, sizeof(HOBJECT)*MAX_EMMITTERS);

			m_nNumEmmitters		= 0;
			m_nEmmitterFlags	= 0;

			m_bCreateDebris		= LTFALSE;
			m_bRotateDebris		= LTFALSE;
			m_fPitch			= 0.0f;
			m_fYaw				= 0.0f;
			m_fPitchVel			= 0.0f;
			m_fYawVel			= 0.0f;
		}

		~CParticleExplosionFX()
		{
			for (int i=0; i < m_nNumEmmitters; i++)
			{
				if (m_hDebris[i] && m_pClientDE)
				{
					m_pClientDE->RemoveObject(m_hDebris[i]);
				}
			}
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

	private :

		LTRotation m_rSurfaceRot;	// Rotation of surface
		LTVector	m_vLastPos;			// Last Particle particle position
		LTVector	m_vMinVel;			// Minimum emmitter velocity
		LTVector	m_vMaxVel;			// Maximum emmitter velocity
		LTVector	m_vMinDriftOffset;	// Particle min drift offset
		LTVector	m_vMaxDriftOffset;	// Particle max drift offset

		LTFLOAT	m_fFadeTime;	// When system should start to fade
		LTFLOAT	m_fLifeTime;	// How long system stays around
		LTFLOAT	m_fStartTime;	// When did we start this crazy thing
		LTFLOAT	m_fOffsetTime;	// Time between particles

		uint8	m_nNumPerPuff;	// Number of particles per Particle puff
		LTBOOL	m_bSmall;		// Relative size of Particle

		LTFLOAT	m_fLastTime;	// Last time we created some particles
		LTBOOL	m_bFirstUpdate;	// First update
		LTBOOL	m_bIgnoreWind;	// Ignore world wind?
		LTBOOL	m_bCreateDebris;// Should we create debris?
		uint8	m_nSurfaceType;	// Impact surface
		uint8	m_nNumSteps;	// Number of steps between each particle puff

		MovingObject	m_Emmitters[MAX_EMMITTERS];				// Particle emmitters
		uint8			m_nNumEmmitters;						// Num in array
		uint8			m_nEmmitterFlags;						// MoveObject flags
		LTBOOL			m_ActiveEmmitters[MAX_EMMITTERS];		// Active?	
		uint8			m_BounceCount[MAX_EMMITTERS];			// Number of bounces
		HLOCALOBJ		m_hDebris[MAX_EMMITTERS];

		// Emmitter rotation stuff...
		
		LTBOOL			m_bRotateDebris;
		LTFLOAT			m_fPitch;		
		LTFLOAT			m_fYaw;
		LTFLOAT			m_fPitchVel;
		LTFLOAT			m_fYawVel;

		LTBOOL UpdateEmmitter(MovingObject* pObject);
		void AddParticles(MovingObject* pObject);
		HLOCALOBJ CreateDebris();
};

#endif // __PARTICLE_EXPLOSION_FX_H__