// ----------------------------------------------------------------------- //
//
// MODULE  : SplashFX.h
//
// PURPOSE : Splash special fx class - Definition
//
// CREATED : 6/23/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPLASH_FX_H__
#define __SPLASH_FX_H__

#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct SPLASHCREATESTRUCT : public SFXCREATESTRUCT
{
	SPLASHCREATESTRUCT::SPLASHCREATESTRUCT();

	DVector vPos;
	DVector vDir;
	DFLOAT	fRadius;
	DFLOAT	fPosRadius;
	DFLOAT	fHeight;
	DFLOAT	fDensity;
	DFLOAT	fSpread;
	DVector vColor1;
	DVector vColor2;
	DFLOAT	fSprayTime;
	DFLOAT	fDuration;
	DFLOAT	fGravity;
	HSTRING hstrTexture;
};

// ----------------------------------------------------------------------- //

inline SPLASHCREATESTRUCT::SPLASHCREATESTRUCT()
{
	memset(this, 0, sizeof(SPLASHCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

class CSplashFX : public CBaseParticleSystemFX
{
	public :

		CSplashFX() : CBaseParticleSystemFX() 
		{
			VEC_SET(m_vDir, 0.0f, 1.0f, 0.0f);
			m_fRadius			= 100.0f;
			m_fPosRadius		= 1.0f;
			m_fHeight			= 100.0f;
			m_fDensity			= 1.0f;
			m_fSpread			= 100.0f;
			m_fSprayTime		= 1.0f;
			m_fDuration			= 1.0f;

			VEC_SET(m_vColor1, 0.0f, 0.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);

			m_hstrTexture		= DNULL;
			m_fStartTime		= 0.0f;
		}
		
		~CSplashFX()
		{
			g_pClientDE->FreeString( m_hstrTexture );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL AddParticles();

		DVector m_vDir;				// Direction splash shoots

		DFLOAT	m_fPosRadius;		// Radius of the splash origin
		DFLOAT	m_fHeight;			// How high to shoot the particles
		DFLOAT	m_fDensity;			// How many particles to place within the radius per update
		DFLOAT	m_fSpread;			// How far to randomly spread the particles
		DFLOAT	m_fSprayTime;		// Time to shoot particles from the origin
		DFLOAT	m_fDuration;		// Lifetime of each particle

		HSTRING m_hstrTexture;		// Texture to sprite to use

		DFLOAT	m_fStartTime;		// When did we start
};

#endif // __SPLASH_FX_H__