// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeSphereFX.h
//
// PURPOSE : SmokeSphere special fx class - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKESPHEREFX_H__
#define __SMOKESPHEREFX_H__

#include "BaseParticleSystemFX.h"


struct SSCREATESTRUCT : public SFXCREATESTRUCT
{
	SSCREATESTRUCT::SSCREATESTRUCT();

	LTFLOAT	m_fRadius;
	LTFLOAT	m_fLifeTime;
	uint8	m_bDense;
};

inline SSCREATESTRUCT::SSCREATESTRUCT()
{
	memset(this, 0, sizeof(SSCREATESTRUCT));
}

class CSmokeSphereFX : public CBaseParticleSystemFX
{
	public :

		CSmokeSphereFX() : CBaseParticleSystemFX() 
		{
			m_fStartTime	= -1.0f;
			m_fLifeTime		= 0.0f;
			m_nNumParticles = 0;
			m_fSphereRadius = 0.0f;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	private :

		LTFLOAT	m_fSphereRadius;// Radius of smoke sphere
		LTFLOAT	m_fLifeTime;	// How long each particle stays around
		LTFLOAT	m_fStartTime;	// When did we start this crazy thing
		uint32	m_nNumParticles;// Number of particles in sphere
};

#endif // __SMOKESPHEREFX_H__