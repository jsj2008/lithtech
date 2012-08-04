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


struct SMCREATESTRUCT : public SFXCREATESTRUCT
{
	SMCREATESTRUCT::SMCREATESTRUCT();

	LTVector vPos;
	LTVector vColor1;
	LTVector vColor2;
	LTVector vMinDriftVel;
	LTVector vMaxDriftVel;
	LTFLOAT  fLifeTime;
	LTFLOAT  fVolumeRadius;
	LTFLOAT	fRadius;
	LTFLOAT	fParticleCreateDelta;
	LTFLOAT	fMinParticleLife;
	LTFLOAT	fMaxParticleLife;
	uint8	nNumParticles;
	LTBOOL	bIgnoreWind;
	HSTRING hstrTexture;
};

inline SMCREATESTRUCT::SMCREATESTRUCT()
{
	memset(this, 0, sizeof(SMCREATESTRUCT));
}

class CSmokeFX : public CBaseParticleSystemFX
{
	public :

		CSmokeFX() : CBaseParticleSystemFX() 
		{
			m_fStartTime	= -1.0f;
			m_fLastTime		= -1.0f;

			VEC_INIT(m_vPos);
			VEC_INIT(m_vMinDriftVel);
			VEC_INIT(m_vMaxDriftVel);
			m_fVolumeRadius			= 0.0f;
			m_fLifeTime				= 0.0f;
			m_fParticleCreateDelta	= 0.0f;
			m_nNumParticles			= 5;
			m_fMinParticleLife		= 5.0f;
			m_fMaxParticleLife		= 10.0f;
			m_bIgnoreWind			= LTFALSE;
			m_hstrTexture		    = LTNULL;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	private :

		LTFLOAT	m_fVolumeRadius;// Radius of smoke volume
		LTFLOAT	m_fLifeTime;	// How long each particle stays around
		LTFLOAT	m_fStartTime;	// When did we start this crazy thing
		LTFLOAT	m_fLastTime;	// When did we last create particles

		LTVector m_vPos;			// Starting position
		LTVector m_vMinDriftVel;	// Min Drift velocity
		LTVector m_vMaxDriftVel;	// Max Drift velocity

		LTFLOAT  m_fParticleCreateDelta;	// How often we create smoke particles
		uint8	m_nNumParticles;		// Number we create every delta
		LTFLOAT	m_fMaxParticleLife;		// Maximum lifetime of a particle
		LTFLOAT	m_fMinParticleLife;		// Minimum lifetime of a particle
		HSTRING m_hstrTexture;			// Texture to sprite to use

		LTBOOL	m_bIgnoreWind;			// Ignore world wind
};

#endif // __SMOKE_FX_H__