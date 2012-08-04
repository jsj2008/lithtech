// ----------------------------------------------------------------------- //
//
// MODULE  : SmokePuffFX.h
//
// PURPOSE : Smoke special fx class - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKEPUFFFX_H__
#define __SMOKEPUFFFX_H__

#include "BaseParticleSystemFX.h"


struct SMPCREATESTRUCT : public SFXCREATESTRUCT
{
	SMPCREATESTRUCT::SMPCREATESTRUCT();

	DVector vColor1;
	DVector vColor2;
	DVector vMinDriftVel;
	DVector vMaxDriftVel;
	DFLOAT	fDriftDeceleration;
	DVector vPos;
	DFLOAT  fLifeTime;
	DFLOAT  fVolumeRadius;
	DFLOAT	fRadius;
	DFLOAT	fParticleCreateDelta;
	DFLOAT	fMinParticleLife;
	DFLOAT	fMaxParticleLife;
	DFLOAT	fCreateLifetime;
	DFLOAT	fMaxAlpha;
	DBYTE	nNumParticles;
	DBOOL	bIgnoreWind;
	char*	pTexture;
};

inline SMPCREATESTRUCT::SMPCREATESTRUCT()
{
	memset(this, 0, sizeof(SMPCREATESTRUCT));
}

class CSmokePuffFX : public CBaseParticleSystemFX
{
	public :

		CSmokePuffFX() : CBaseParticleSystemFX() 
		{
			m_fStartTime	= -1.0f;
			m_fLastTime		= -1.0f;

			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);
			VEC_INIT(m_vMinDriftVel);
			VEC_INIT(m_vMaxDriftVel);
			m_fDriftDeceleration	= 0.0f;
			m_fVolumeRadius			= 0.0f;
			m_fLifeTime				= 0.0f;
			m_fParticleCreateDelta	= 0.0f;
			m_nNumParticles			= 5;
			m_fMinParticleLife		= 5.0f;
			m_fMaxParticleLife		= 10.0f;
			m_fCreateLifetime		= 0.25f;
			m_fMaxAlpha				= 1.0f;
			m_bIgnoreWind			= DFALSE;
			m_pTexture			    = DNULL;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	private :

		DFLOAT	m_fVolumeRadius;// Radius of smoke volume
		DFLOAT	m_fLifeTime;	// How long each particle stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fLastTime;	// When did we last create particles
		DFLOAT	m_fFadeTime;	// Time until particles start fading

		DVector	m_vColor1;		// Color of darkest smoke particles
		DVector	m_vColor2;		// Color of lightest smoke particles
		DVector m_vMinDriftVel;	// Min Drift velocity
		DVector m_vMaxDriftVel;	// Max Drift velocity

		DFLOAT	m_fDriftDeceleration;
		DFLOAT  m_fParticleCreateDelta;	// How often we create smoke particles
		DBYTE	m_nNumParticles;		// Number we create every delta
		DFLOAT	m_fMaxParticleLife;		// Maximum lifetime of a particle
		DFLOAT	m_fMinParticleLife;		// Minimum lifetime of a particle
		DFLOAT	m_fMaxAlpha;			// Max value to use for alpha
		DFLOAT	m_fCreateLifetime;		// Percentage of lifetime to create particles
		char*	m_pTexture;				// Texture to sprite to use

		DBOOL	m_bIgnoreWind;			// Ignore world wind
};

#endif // __SMOKEPUFF_H__