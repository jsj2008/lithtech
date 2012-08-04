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

struct PESCREATESTRUCT : public BPSCREATESTRUCT
{
    PESCREATESTRUCT();

    LTRotation   rSurfaceRot;
    LTVector     vPos;
    LTVector     vColor1;
    LTVector     vColor2;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vMinDriftVel;
    LTVector     vMaxDriftVel;
    LTFLOAT      fLifeTime;
    LTFLOAT      fFadeTime;
    LTFLOAT      fOffsetTime;
    LTFLOAT      fRadius;
    LTFLOAT      fGravity;
    uint8       nSurfaceType;
    uint8       nNumPerPuff;
    uint8       nNumEmitters;
    uint8       nEmitterFlags;
    uint8       nNumSteps;
    LTBOOL       bCreateDebris;
    LTBOOL       bRotateDebris;
    LTBOOL       bIgnoreWind;
	char*		pFilename;
};

inline PESCREATESTRUCT::PESCREATESTRUCT()
{
    rSurfaceRot.Init();
	vPos.Init();
	vColor1.Init();
	vColor2.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDriftVel.Init(0.0f, 5.0f, 0.0f);
	vMaxDriftVel.Init(0.0f, 6.0f, 0.0f);
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	fOffsetTime		= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;
	nSurfaceType	= 0;
	nNumPerPuff		= 0;
	nNumEmitters	= 0;
	nEmitterFlags	= 0;
	nNumSteps		= 2;
    bCreateDebris   = LTTRUE;
    bRotateDebris   = LTTRUE;
    bIgnoreWind     = LTTRUE;
    pFilename       = LTNULL;
}

class CParticleExplosionFX : public CBaseParticleSystemFX
{
	public :

		CParticleExplosionFX() : CBaseParticleSystemFX()
		{
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vMinVel);
			VEC_INIT(m_vMaxVel);
			VEC_INIT(m_vMinDriftVel);
			VEC_INIT(m_vMaxDriftVel);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fOffsetTime	= 0.0f;
			m_nNumPerPuff	= 1;
			m_nNumSteps		= 2;

            m_bFirstUpdate  = LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

			m_nSurfaceType	= 0;
            m_bIgnoreWind   = LTFALSE;

			memset(m_Emitters, 0, sizeof(MovingObject)*MAX_EMITTERS);
            memset(m_ActiveEmitters, 0, sizeof(LTBOOL)*MAX_EMITTERS);
            memset(m_BounceCount, 0, sizeof(uint8)*MAX_EMITTERS);
			memset(m_hDebris, 0, sizeof(HOBJECT)*MAX_EMITTERS);

			m_nNumEmitters		= 0;
			m_nEmitterFlags		= 0;

            m_bCreateDebris     = LTFALSE;
            m_bRotateDebris     = LTFALSE;
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

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_PARTICLEEXPLOSION_ID; }

	private :

        LTRotation m_rSurfaceRot;    // Rotation of surface
        LTVector m_vLastPos;         // Last Particle particle position
        LTVector m_vMinVel;          // Minimum emitter velocity
        LTVector m_vMaxVel;          // Maximum emitter velocity
        LTVector m_vMinDriftVel;     // Particle min drift velocity
        LTVector m_vMaxDriftVel;     // Particle max drift velocity

        LTFLOAT  m_fFadeTime;    // When system should start to fade
        LTFLOAT  m_fLifeTime;    // How long system stays around
        LTFLOAT  m_fStartTime;   // When did we start this crazy thing
        LTFLOAT  m_fOffsetTime;  // Time between particles

        uint8   m_nNumPerPuff;  // Number of particles per Particle puff

        LTFLOAT  m_fLastTime;    // Last time we created some particles
        LTBOOL   m_bFirstUpdate; // First update
        LTBOOL   m_bIgnoreWind;  // Ignore world wind?
        LTBOOL   m_bCreateDebris;// Should we create debris?
        uint8   m_nSurfaceType; // Impact surface
        uint8   m_nNumSteps;    // Number of steps between each particle puff

		MovingObject	m_Emitters[MAX_EMITTERS];			// Particle Emitters
        uint8           m_nNumEmitters;                     // Num in array
        uint8           m_nEmitterFlags;                    // MoveObject flags
        LTBOOL           m_ActiveEmitters[MAX_EMITTERS];     // Active?
        uint8           m_BounceCount[MAX_EMITTERS];        // Number of bounces
		HLOCALOBJ		m_hDebris[MAX_EMITTERS];

		// Emitter rotation stuff...

        LTBOOL           m_bRotateDebris;
        LTFLOAT          m_fPitch;
        LTFLOAT          m_fYaw;
        LTFLOAT          m_fPitchVel;
        LTFLOAT          m_fYawVel;

        LTBOOL UpdateEmitter(MovingObject* pObject);
		void AddParticles(MovingObject* pObject);
		HLOCALOBJ CreateDebris();
};

#endif // __PARTICLE_EXPLOSION_FX_H__