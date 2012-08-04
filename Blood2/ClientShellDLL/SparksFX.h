// ----------------------------------------------------------------------- //
//
// MODULE  : SparksFX.h
//
// PURPOSE : Sparks special fx class - Definition
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPARKS_FX_H__
#define __SPARKS_FX_H__

#include "BaseParticleSystemFX.h"


struct SCREATESTRUCT : public SFXCREATESTRUCT
{
	SCREATESTRUCT::SCREATESTRUCT();

	DRotation rRot;
	DVector vPos;
	DVector vDir;
	DVector vColor1;
	DVector vColor2;
	DBYTE	nSparks;
	DFLOAT	fDuration;
	DFLOAT	fEmissionRadius;
	HSTRING hstrTexture;
	DFLOAT	fRadius;
	DFLOAT	fGravity;
	DBOOL	bFadeColors;
};

inline SCREATESTRUCT::SCREATESTRUCT()
{
	memset(this, 0, sizeof(SCREATESTRUCT));
	rRot.m_Spin = 1.0f;
}


class CSparksFX : public CBaseParticleSystemFX
{
	public :

		CSparksFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vDir);
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 0.0f);
			m_nSparks			= 5;
			m_fDuration			= 1.0f;
			m_fEmissionRadius	= 0.3f;
			
			m_fStartTime = 0.0f;
			m_hstrTexture = DNULL;
			m_bFadeColors = DFALSE;
		}

		~CSparksFX()
		{
			if( m_hstrTexture )
				g_pClientDE->FreeString( m_hstrTexture );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL AddSparks();

		DVector m_vDir;				// Direction sparks shoot
		DVector m_vColor1;			
		DVector m_vColor2;
		DBYTE	m_nSparks;			// Number of sparks
		DFLOAT	m_fDuration;		// Life time of sparks
		DFLOAT	m_fEmissionRadius;	// How far particles shoot
		HSTRING m_hstrTexture;		// Texture to sprite to use

		DFLOAT	m_fStartTime;		// When did we start
		DBOOL	m_bFadeColors;		// Fade from color1 to color2
};

#endif // __SPARKS_FX_H__