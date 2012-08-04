// ----------------------------------------------------------------------- //
//
// MODULE  : LightningFX.h
//
// PURPOSE : Special FX class for lightning-like instant particle streams
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNING_FX_H__
#define __LIGHTNING_FX_H__

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct LIGHTNINGCREATESTRUCT : public SFXCREATESTRUCT
{
	LIGHTNINGCREATESTRUCT::LIGHTNINGCREATESTRUCT();

	DVector		vSource;
	DVector		vDest;
	DBYTE		nShape;
	DBYTE		nForm;
	DBYTE		nType;
};

// ----------------------------------------------------------------------- //

inline LIGHTNINGCREATESTRUCT::LIGHTNINGCREATESTRUCT()
{
	memset(this, 0, sizeof(LIGHTNINGCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

#define LIGHTNING_LIGHTS_SOURCE		0
#define LIGHTNING_LIGHTS_CENTER		1
#define LIGHTNING_LIGHTS_DEST		2

#define LIGHTNING_SEGMAG_LOW		500.0f
#define LIGHTNING_SEGMAG_MED		250.0f
#define LIGHTNING_SEGMAG_HIGH		100.0f

#define LIGHTNING_OFFSET_RATIO		10.0f

// ----------------------------------------------------------------------- //

class CLightningBoltFX : public CSpecialFX
{
	public :

		CLightningBoltFX() : CSpecialFX() {}

		virtual	DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		void	SetupLightning();
		DBOOL	CreateLightning();

		DVector	m_vSource;			// Offset from the gun
		DVector	m_vDest;			// Destination point of the bolt

		DFLOAT	m_fMinRadius;		// Minimum system radius
		DFLOAT	m_fMaxRadius;		// Maximum system radius
		DFLOAT	m_fMinOffset;		// Minimum offset from the vector
		DFLOAT	m_fMaxOffset;		// Maximum offset from the vector
		DFLOAT	m_fDensity;			// Ratio of particles to unit length (lower = more dense)

		DFLOAT	m_fDuration;		// Time to make the lightning hang around
		DFLOAT	m_fFadeTime;		// Time to fade off at the end of lifetime

		DVector	m_vMinColor;		// Minimum color
		DVector	m_vMaxColor;		// Maximum color
		DFLOAT	m_fAlpha;			// Alpha for all systems

		DDWORD	m_nNumLights;		// How many lights should be used for the whole bolt
		DVector	m_vLightColor;		// Color of each light created
		DFLOAT	m_fLightRadius;		// Radius of each light created

		DBYTE	m_nShape;			// Shape of the lightning bolt
		DBYTE	m_nForm;			// Form of the lightning bolt
		DBYTE	m_nType;			// Type of lightning bolt to create
		DBYTE	m_nLightPos;		// How to position the lights

		DFLOAT	m_fStartTime;

		char	*m_hstrTexture;		// Texture to sprite to use
};

#endif // __LIGHTNING_FX_H__