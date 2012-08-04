// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.h
//
// PURPOSE : ParticleSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SYSTEM_FX_H__
#define __PARTICLE_SYSTEM_FX_H__

#include "BaseParticleSystemFX.h"


struct PSCREATESTRUCT : public BPSCREATESTRUCT
{
    PSCREATESTRUCT();

    LTVector     vColor1;
    LTVector     vColor2;
    LTVector     vDims;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vPos;
    uint32      dwFlags;
    LTFLOAT      fBurstWait;
    LTFLOAT      fBurstWaitMin;
    LTFLOAT      fBurstWaitMax;
    LTFLOAT      fParticlesPerSecond;
    LTFLOAT      fParticleLifetime;
    LTFLOAT      fParticleRadius;
    LTFLOAT      fGravity;
    LTFLOAT      fRotationVelocity;
    LTFLOAT      fViewDist;
	HSTRING		hstrTextureName;
};

inline PSCREATESTRUCT::PSCREATESTRUCT()
{
	vColor1.Init();
	vColor2.Init();
	vDims.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vPos.Init();
	dwFlags				= 0;
	fBurstWait			= 0.0f;
	fParticlesPerSecond	= 0.0f;
	fParticleLifetime	= 0.0f;
	fParticleRadius		= 0.0f;
	fGravity			= 0.0f;
	fRotationVelocity	= 0.0f;
	fViewDist			= 0.0f;
	fBurstWaitMin		= 0.01f;
	fBurstWaitMax		= 1.0f;
    hstrTextureName     = LTNULL;
}

class CParticleSystemFX : public CBaseParticleSystemFX
{
	public :

		CParticleSystemFX();
		~CParticleSystemFX()
		{
			if (m_cs.hstrTextureName && m_pClientDE)
			{
				m_pClientDE->FreeString(m_cs.hstrTextureName);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_PARTICLESYSTEM_ID; }

	protected :

		// Creation data...

		PSCREATESTRUCT	m_cs;		// Holds all initialization data

        LTBOOL   m_bFirstUpdate;     // Is this the first update
        LTFLOAT  m_fNextUpdate;      // Time between updates
        LTFLOAT  m_fLastTime;        // When was the last time
        LTFLOAT  m_fMaxViewDistSqr;  // Max dist to add particles (squared)

        LTVector m_vMinOffset;
        LTVector m_vMaxOffset;

		void TweakSystem();
};

#endif // __PARTICLE_SYSTEM_FX_H__