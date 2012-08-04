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


struct PSCREATESTRUCT : public SFXCREATESTRUCT
{
	PSCREATESTRUCT::PSCREATESTRUCT();

	LTVector		vColor1;
	LTVector		vColor2;
	uint32		dwFlags;
	LTFLOAT		fBurstWait;
	LTFLOAT		fParticlesPerSecond;
	LTFLOAT		fEmissionRadius;
	LTFLOAT		fMinimumVelocity;
	LTFLOAT		fMaximumVelocity;
	LTFLOAT		fVelocityOffset;
	LTFLOAT		fParticleLifetime;
	LTFLOAT		fParticleRadius;
	LTFLOAT		fGravity;
	LTFLOAT		fRotationVelocity;
	HSTRING		hstrTextureName;
};

inline PSCREATESTRUCT::PSCREATESTRUCT()
{
	memset(this, 0, sizeof(PSCREATESTRUCT));
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

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		// Creation data...

		PSCREATESTRUCT	m_cs;		// Holds all initialization data

		LTBOOL	m_bFirstUpdate;		// Is this the first update
		LTFLOAT	m_fNextUpdate;		// Time between updates
		LTFLOAT	m_fLastTime;		// When was the last time
		LTVector	m_vMinOffset;		// Minumum offset
		LTVector m_vMaxOffset;		// Maximum offset
		LTVector m_vMinVel;			// Minimum velocity
		LTVector m_vMaxVel;			// Maximum velocity

		uint32	m_dwLastFrameUserFlags;
};

#endif // __PARTICLE_SYSTEM_FX_H__