// ----------------------------------------------------------------------- //
//
// MODULE  : BulletTrailFX.h
//
// PURPOSE : SmokeTrail segment special fx class - Definition
//
// CREATED : 3/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __BULLET_TRAIL_FX_H__
#define __BULLET_TRAIL_FX_H__

#include "BaseParticleSystemFX.h"


struct BTCREATESTRUCT : public BPSCREATESTRUCT
{
    BTCREATESTRUCT();

    LTVector vStartPos;
    LTVector vDir;
    LTVector vColor1;
    LTVector vColor2;
    LTFLOAT  fLifeTime;
    LTFLOAT  fFadeTime;
    LTFLOAT  fRadius;
    LTFLOAT  fGravity;
    LTFLOAT  fNumParticles;
};

inline BTCREATESTRUCT::BTCREATESTRUCT()
{
	vStartPos.Init();
	vDir.Init();
	vColor1.Init();
	vColor2.Init();
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;
	fNumParticles	= 0.0f;
}

class CBulletTrailFX : public CBaseParticleSystemFX
{
	public :

		CBulletTrailFX() : CBaseParticleSystemFX()
		{
			VEC_INIT(m_vStartPos);
			VEC_INIT(m_vLastPos);
			VEC_INIT(m_vDir);

			m_fLifeTime		= 0.0f;
			m_fFadeTime		= 0.0f;
			m_fNumParticles	= 100.0f;

            m_bFirstUpdate  = LTTRUE;
			m_fDistance		= 0.0f;
			m_fDistTraveled	= 0.0f;
			m_fTrailVel		= 0.0f;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_BULLETTRAIL_ID; }

	private :

        LTVector m_vLastPos;     // Last bubble particle position
        LTVector m_vStartPos;    // Starting position of trail
        LTVector m_vDir;         // Direction of trail

        LTFLOAT  m_fFadeTime;    // When system should start to fade
        LTFLOAT  m_fLifeTime;    // How long system stays around
        LTFLOAT  m_fNumParticles;// Total number of particles in system

        LTBOOL   m_bFirstUpdate; // First update
        LTFLOAT  m_fDistance;    // Length of trail
        LTFLOAT  m_fDistTraveled;// How far have we gone?
        LTFLOAT  m_fTrailVel;    // Speed of trail

        LTFLOAT  m_fStartTime;   // When did we start
        LTFLOAT  m_fLastTime;    // When was the last update
};

#endif // __BULLET_TRAIL_FX_H__