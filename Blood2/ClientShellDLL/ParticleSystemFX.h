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

	DVector		vColor1;
	DVector		vColor2;
	DDWORD		dwFlags;
	DFLOAT		fBurstWait;
	DFLOAT		fParticlesPerSecond;
	DFLOAT		fEmissionRadius;
	DFLOAT		fMinimumVelocity;
	DFLOAT		fMaximumVelocity;
	DFLOAT		fVelocityOffset;
	DFLOAT		fParticleLifetime;
	DFLOAT		fParticleRadius;
	DFLOAT		fGravity;
	DFLOAT		fRotationVelocity;
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

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	protected :

		// Creation data...

		PSCREATESTRUCT	m_cs;		// Holds all initialization data

		DBOOL	m_bFirstUpdate;		// Is this the first update
		DFLOAT	m_fNextUpdate;		// Time between updates
		DFLOAT	m_fLastTime;		// When was the last time
		DVector	m_vMinOffset;		// Minumum offset
		DVector m_vMaxOffset;		// Maximum offset
		DVector m_vMinVel;			// Minimum velocity
		DVector m_vMaxVel;			// Maximum velocity

		DDWORD	m_dwLastFrameUserFlags;
};

#endif // __PARTICLE_SYSTEM_FX_H__