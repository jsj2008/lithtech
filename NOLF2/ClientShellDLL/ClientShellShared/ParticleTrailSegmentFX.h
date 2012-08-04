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


struct PTSCREATESTRUCT : public BPSCREATESTRUCT
{
    PTSCREATESTRUCT();

    uint8   nType;
    LTVector vColor1;
    LTVector vColor2;
    LTVector vDriftOffset;
    LTFLOAT  fLifeTime;
    LTFLOAT  fFadeTime;
    LTFLOAT  fOffsetTime;
    LTFLOAT  fRadius;
    LTFLOAT  fGravity;
    uint8   nNumPerPuff;
};

inline PTSCREATESTRUCT::PTSCREATESTRUCT()
{
	vColor1.Init();
	vColor2.Init();
	vDriftOffset.Init();
	fLifeTime	= 0.0f;
	fFadeTime	= 0.0f;
	fOffsetTime	= 0.0f;
	fRadius		= 0.0f;
	fGravity	= 0.0f;
	nNumPerPuff	= 0;
	nType		= 0;
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
			m_nType			= 0;

            m_bFirstUpdate  = LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;

            m_bIgnoreWind   = LTFALSE;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_PARTICLETRAILSEG_ID; }

	private :

        LTVector m_vLastPos;     // Last Particle particle position
        LTVector m_vDriftOffset; // Particle drift offset

        LTFLOAT  m_fFadeTime;    // When system should start to fade
        LTFLOAT  m_fLifeTime;    // How long system stays around
        LTFLOAT  m_fStartTime;   // When did we start this crazy thing
        LTFLOAT  m_fOffsetTime;  // Time between particles

        uint8   m_nNumPerPuff;  // Number of particles per Particle puff
        uint8   m_nType;        // Type of particle

        LTFLOAT  m_fLastTime;    // Last time we created some particles
        LTBOOL   m_bFirstUpdate; // First update
        LTBOOL   m_bIgnoreWind;  // Ignore world wind?
};

#endif // __PARTICLE_TRAIL_SEGMENT_FX_H__