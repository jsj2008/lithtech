// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeFX.h
//
// PURPOSE : Smoke special fx class - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKE_FX_H__
#define __SMOKE_FX_H__

#include "BaseParticleSystemFX.h"
#include "SmokePuffFX.h"


struct SMCREATESTRUCT : public SFXCREATESTRUCT
{
	SMCREATESTRUCT::SMCREATESTRUCT();

	DVector vColor1;
	DVector vColor2;
	DVector vMinDriftVel;
	DVector vMaxDriftVel;
	DFLOAT  fLifeTime;
	DFLOAT  fVolumeRadius;
	DFLOAT	fRadius;
	DFLOAT	fParticleCreateDelta;
	DFLOAT	fMinParticleLife;
	DFLOAT	fMaxParticleLife;
	DBYTE	nNumParticles;
	DBOOL	bIgnoreWind;
	DFLOAT	fSegmentTime;
	HSTRING hstrTexture;
};

inline SMCREATESTRUCT::SMCREATESTRUCT()
{
	memset(this, 0, sizeof(SMCREATESTRUCT));
}

class CSmokeFX : public CSpecialFX
{
	public :

		CSmokeFX() : CSpecialFX() 
		{
			m_fStartTime	= -1.0f;
			m_fLastTime		= -1.0f;

			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);
			VEC_INIT(m_vMinDriftVel);
			VEC_INIT(m_vMaxDriftVel);
			VEC_INIT(m_vPos);
			m_fVolumeRadius			= 0.0f;
			m_fLifeTime				= 0.0f;
			m_fParticleCreateDelta	= 0.0f;
			m_nNumParticles			= 5;
			m_fMinParticleLife		= 5.0f;
			m_fMaxParticleLife		= 10.0f;
			m_bIgnoreWind			= DFALSE;
			m_hstrTexture		    = DNULL;
		}

		~CSmokeFX()
		{
			g_pClientDE->FreeString( m_hstrTexture );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	private :

		DFLOAT	m_fVolumeRadius;// Radius of smoke volume
		DFLOAT	m_fLifeTime;	// How long each particle stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DFLOAT	m_fLastTime;	// When did we last create particles

		DVector	m_vColor1;		// Color of darkest smoke particles
		DVector	m_vColor2;		// Color of lightest smoke particles
		DVector m_vMinDriftVel;	// Min Drift velocity
		DVector m_vMaxDriftVel;	// Max Drift velocity
		DVector m_vPos;

		char*	m_pTextureName;
		DFLOAT	m_fRadius;
		DFLOAT	m_fGravity;

		DFLOAT  m_fParticleCreateDelta;	// How often we create smoke particles
		DBYTE	m_nNumParticles;		// Number we create every delta
		DFLOAT	m_fMaxParticleLife;		// Maximum lifetime of a particle
		DFLOAT	m_fMinParticleLife;		// Minimum lifetime of a particle
		HSTRING m_hstrTexture;			// Texture to sprite to use
		DFLOAT	m_fSegmentTime;			// Time until the next segment

		DBOOL	m_bIgnoreWind;			// Ignore world wind
};

#endif // __SMOKE_FX_H__