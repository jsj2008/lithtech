// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.h
//
// PURPOSE : Explosion special fx class - Definition
//
// CREATED : 7/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_FX_H__
#define __EXPLOSION_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"
#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct EXPLOSIONMODELCS : public SFXCREATESTRUCT
{
	EXPLOSIONMODELCS::EXPLOSIONMODELCS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vScale1;
	DVector	vScale2;
	DVector	vRotations;
	DFLOAT	fDuration;
	DFLOAT	fAlpha;
	DBYTE	bWaveForm;
	DBYTE	bFadeType;
	DBYTE	bRandomRot;
	HSTRING	szModel;
	HSTRING	szSkin;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONMODELCS::EXPLOSIONMODELCS()
{
	memset(this, 0, sizeof(EXPLOSIONMODELCS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONSPRITECS : public SFXCREATESTRUCT
{
	EXPLOSIONSPRITECS::EXPLOSIONSPRITECS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vScale1;
	DVector	vScale2;
	DFLOAT	fDuration;
	DFLOAT	fAlpha;
	DBYTE	bWaveForm;
	DBYTE	bFadeType;
	DBYTE	bAlign;
	HSTRING	szSprite;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONSPRITECS::EXPLOSIONSPRITECS()
{
	memset(this, 0, sizeof(EXPLOSIONSPRITECS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONRINGCS : public SFXCREATESTRUCT
{
	EXPLOSIONRINGCS::EXPLOSIONRINGCS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vColor;
	DFLOAT	fRadius;
	DFLOAT	fPosRadius;
	DFLOAT	fVelocity;
	DFLOAT	fGravity;
	DFLOAT	fRotation;
	DDWORD	nParticles;
	DFLOAT	fDuration;
	DFLOAT	fAlpha;
	DFLOAT	fDelay;
	DBYTE	bFadeType;
	DBYTE	bRotateType;
	DBYTE	bAlign;
	HSTRING	szParticle;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONRINGCS::EXPLOSIONRINGCS()
{
	memset(this, 0, sizeof(EXPLOSIONRINGCS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONFLAMECS : public SFXCREATESTRUCT
{
	EXPLOSIONFLAMECS::EXPLOSIONFLAMECS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vColor1;		// Core color
	DVector	vColor2;		// Median color
	DVector	vColor3;		// Edge color
	DVector	vLifeTimes;
	DVector	vLifeOffsets;
	DVector	vFXTimes;
	DDWORD	nParticles;
	DFLOAT	fRadius;
	DFLOAT	fPosRadius;
	DFLOAT	fGravity;
	DFLOAT	fVelocity;
	DFLOAT	fDelay;
	DFLOAT	fAlpha;
	DBYTE	bFadeType;
	DBYTE	bRampFlags;
	HSTRING	szParticle;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONFLAMECS::EXPLOSIONFLAMECS()
{
	memset(this, 0, sizeof(EXPLOSIONFLAMECS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONWAVECS : public SFXCREATESTRUCT
{
	EXPLOSIONWAVECS::EXPLOSIONWAVECS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vScale1;
	DVector	vScale2;
	DFLOAT	fDuration;
	DFLOAT	fAlpha;
	DFLOAT	fDelay;
	DBYTE	bWaveForm;
	DBYTE	bFadeType;
	DBYTE	bAlign;
	HSTRING	szWave;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONWAVECS::EXPLOSIONWAVECS()
{
	memset(this, 0, sizeof(EXPLOSIONWAVECS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONLIGHTCS : public SFXCREATESTRUCT
{
	EXPLOSIONLIGHTCS::EXPLOSIONLIGHTCS();

	DVector	vPos;
	DVector	vColor1;
	DVector vColor2;
	DFLOAT	fDuration;
	DFLOAT	fDelay;
	DFLOAT	fRadius1;
	DFLOAT	fRadius2;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONLIGHTCS::EXPLOSIONLIGHTCS()
{
	memset(this, 0, sizeof(EXPLOSIONLIGHTCS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONFRAGCS : public SFXCREATESTRUCT
{
	EXPLOSIONFRAGCS::EXPLOSIONFRAGCS();

	DVector	vPos;
	DVector	vNormal;
	DVector	vScale;
	DVector	vRotateMax;
	DFLOAT	fSpread;
	DFLOAT	fDuration;
	DFLOAT	fVelocity;
	DFLOAT	fBounceMod;
	DFLOAT	fGravity;
	DFLOAT	fFadeTime;
	DFLOAT	fInitAlpha;
	DBOOL	bRandDir;
	DBOOL	bSpawnExp;
	DDWORD	nSpawnType;
	DDWORD	nTrailType;
	HSTRING	szModel;
	HSTRING	szSkin;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONFRAGCS::EXPLOSIONFRAGCS()
{
	memset(this, 0, sizeof(EXPLOSIONFRAGCS));
}

// ----------------------------------------------------------------------- //

struct EXPLOSIONFXCS : public SFXCREATESTRUCT
{
	EXPLOSIONFXCS::EXPLOSIONFXCS();

	DVector	vPos;
	DVector	vNormal;
	DDWORD	nType;
};

// ----------------------------------------------------------------------- //

inline EXPLOSIONFXCS::EXPLOSIONFXCS()
{
	memset(this, 0, sizeof(EXPLOSIONFXCS));
}

// ----------------------------------------------------------------------- //

class CExplosionModelFX : public CSpecialFX
{
	public :

		CExplosionModelFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			m_fDuration	= 1.0f;
			VEC_SET(m_vScale2, 1.0f, 1.0f, 1.0f);
			m_szModel = DNULL;
			m_szSkin = DNULL;
		}

		~CExplosionModelFX()
		{
			g_pClientDE->FreeString( m_szModel );
			g_pClientDE->FreeString( m_szSkin );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vPos;				// Position of the explosion
		DVector m_vNormal;			// Normal of the surface that was hit

		DVector	m_vRotations;		// Rotation values for the model

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time to scale between
		DFLOAT	m_fInitAlpha;		// Initial alpha value

		DBOOL	m_bScale;			// Scale the object?
		DVector	m_vScale1;			// Initial scale
		DVector	m_vScale2;			// Final scale

		DBYTE	m_bWaveForm;		// Type of waveform to scale with
		DBYTE	m_bFadeType;		// Type of fade to apply
		DBYTE	m_bRandomRot;		// Place the model in a random inital rotation

		HSTRING	m_szModel;			// File to use for the model
		HSTRING	m_szSkin;			// File to use for the skin
};

// ----------------------------------------------------------------------- //

class CExplosionSpriteFX : public CSpecialFX
{
	public :

		CExplosionSpriteFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			m_fDuration	= 1.0f;
			VEC_SET(m_vScale2, 1.0f, 1.0f, 1.0f);
			m_szSprite = DNULL;
		}

		~CExplosionSpriteFX()
		{
			g_pClientDE->FreeString( m_szSprite );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vPos;				// Position of the explosion
		DVector m_vNormal;			// Normal of the surface that was hit

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time to scale between
		DFLOAT	m_fInitAlpha;		// Initial alpha value

		DBOOL	m_bScale;			// Scale the sprite?
		DVector	m_vScale1;			// Initial scale
		DVector	m_vScale2;			// Final scale

		DBYTE	m_bWaveForm;		// Type of waveform to scale with
		DBYTE	m_bFadeType;		// Type of fade to apply
		DBYTE	m_bAlign;			// Place the sprite aligned to the normal

		HSTRING	m_szSprite;			// File to use for the model
};

// ----------------------------------------------------------------------- //

class CExplosionRingFX : public CBaseParticleSystemFX
{
	public :

		CExplosionRingFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			m_fDuration	= 1.0f;
			m_fRadius = 100.0f;
			m_fVelocity = 1.0f;
			m_nParticles = 8;
			m_szParticle = DNULL;
		}

		~CExplosionRingFX()
		{
			g_pClientDE->FreeString( m_szParticle );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		void	AddParticles();

		DVector m_vNormal;			// Normal of the surface that was hit

		DVector	m_vColor;			// Color of the particles
		DFLOAT	m_fPosRadius;		// Radius offset to start particles from
		DFLOAT	m_fVelocity;		// Initial velocity for each particle
		DDWORD	m_nParticles;		// Number of particles

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time to scale between
		DFLOAT	m_fRotation;		// Amount of initial or destination rotation
		DFLOAT	m_fInitAlpha;		// Initial alpha value
		DFLOAT	m_fDelay;			// Time to delay after creation of object

		DBYTE	m_bFadeType;		// Type of fade to apply
		DBYTE	m_bRotateType;
		DBYTE	m_bAlign;			// Place the wave aligned to the normal

		HSTRING	m_szParticle;		// File to use for the particle
};

// ----------------------------------------------------------------------- //

#define		EXP_FLAME_RAMP_VEL			0x01
#define		EXP_FLAME_RAMP_LIFETIME		0x02
#define		EXP_FLAME_RAMP_ALPHA		0x04

// ----------------------------------------------------------------------- //

class CExplosionFlameFX : public CBaseParticleSystemFX
{
	public :

		CExplosionFlameFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			VEC_SET(m_vColor1, 1.0f, 1.0f, 0.0f);
			VEC_SET(m_vColor2, 1.0f, 0.75f, 0.0f);
			VEC_SET(m_vColor3, 1.0f, 0.0f, 0.0f);
			VEC_SET(m_vLifeTimes, 0.5f, 1.0f, 1.5f);
			m_fRampUp = 1.0f;
			m_fDuration = 3.0f;
			m_fRampDown = 1.0f;
			m_fRadius = 100.0f;
			m_nParticles = 20;
			m_szParticle = DNULL;
		}

		~CExplosionFlameFX()
		{
			g_pClientDE->FreeString( m_szParticle );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		void	AddParticles();

		DVector m_vNormal;			// Normal of the surface that was hit

		DVector	m_vColor1;			// Color of the core particles
		DVector	m_vColor2;			// Color of the median particles
		DVector	m_vColor3;			// Color of the edge particles

		DVector	m_vLifeTimes;		// Base lifetime for each particle zone
		DVector	m_vLifeOffsets;		// Random offsets for lifetime of each particle zone
		DFLOAT	m_fRampUp;			// RampUp Time
		DFLOAT	m_fDuration;		// Total duration
		DFLOAT	m_fRampDown;		// RampDown Time

		DDWORD	m_nParticles;		// Number of particles
		DFLOAT	m_fPosRadius;		// Radius offset to start particles from
		DFLOAT	m_fVelocity;		// Initial velocity for each particle

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fAddTime;			// When to add another group of particles
		DFLOAT	m_fDelay;			// Delay between adding more particles
		DFLOAT	m_fAlpha;			// Alpha value reference for fade types

		DBYTE	m_bFadeType;		// Type of fade to apply
		DBYTE	m_bRampFlags;		// Flags for FX element ramp-up and ramp-downs

		HSTRING	m_szParticle;		// File to use for the particle
};

// ----------------------------------------------------------------------- //

class CExplosionWaveFX : public CSpecialFX
{
	public :

		CExplosionWaveFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			m_fDuration	= 1.0f;
			VEC_SET(m_vScale2, 1.0f, 1.0f, 1.0f);
			m_szWave = DNULL;
		}

		~CExplosionWaveFX()
		{
			g_pClientDE->FreeString( m_szWave );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vPos;				// Position of the explosion
		DVector m_vNormal;			// Normal of the surface that was hit

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time to scale between
		DFLOAT	m_fInitAlpha;		// Initial alpha value
		DFLOAT	m_fDelay;			// Time to delay after creation of object

		DBOOL	m_bScale;			// Scale the wave?
		DVector	m_vScale1;			// Initial scale
		DVector	m_vScale2;			// Final scale

		DBYTE	m_bWaveForm;		// Type of waveform to scale with
		DBYTE	m_bFadeType;		// Type of fade to apply
		DBYTE	m_bAlign;			// Place the wave aligned to the normal

		HSTRING	m_szWave;			// File to use for the model
};

// ----------------------------------------------------------------------- //

class CExplosionLightFX : public CSpecialFX
{
	public :

		CExplosionLightFX()
		{
			m_fDuration	= 1.0f;
			m_fRadius1 = 50.0f;
			m_fRadius2 = 100.0f;
			VEC_SET(m_vColor1, 1.0f, 1.0f, 0.0f);
			VEC_SET(m_vColor2, 1.0f, 1.0f, 1.0f);
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vPos;				// Position of the light

		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time to scale color and exist
		DFLOAT	m_fDelay;			// Time to delay after creation of light

		DBOOL	m_bChangeColor;		// Scale the color?
		DVector	m_vColor1;			// Initial color
		DVector	m_vColor2;			// Final color

		DBOOL	m_bScale;
		DFLOAT	m_fRadius1;			// Starting radius
		DFLOAT	m_fRadius2;			// Final radius
};

// ----------------------------------------------------------------------- //

class CExplosionFragFX : public CSpecialFX
{
	public :

		CExplosionFragFX()
		{
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			VEC_SET(m_vScale, 0.1f, 0.1f, 0.1f);
			m_fVelocity = 1.0f;
			m_fGravity = 10.0f;
			m_fDuration	= 5.0f;
			m_fFadeTime = 1.0f;
			m_fInitAlpha = 1.0f;
			m_fPitch = m_fYaw = m_fRoll = 0.0f;
			m_szModel = DNULL;
			m_szSkin = DNULL;
		}

		~CExplosionFragFX()
		{
			g_pClientDE->FreeString( m_szModel );
			g_pClientDE->FreeString( m_szSkin );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vPos;				// Position of the light
		DVector m_vLastPos;			// Last position of the object
		DVector	m_vNormal;			// Normal of the surface hit
		DVector	m_vScale;			// Scale of the object
		DVector	m_vVelocity;		// Current vector velocity of the object
		DVector m_vGravity;			// Gravity vector;

		DFLOAT	m_fVelocity;		// Current velocity of the object
		DFLOAT	m_fBounceMod;		// Amount to decrease velocity on a bounce
		DFLOAT	m_fGravity;			// Gravity ratio to apply to this object
		DFLOAT	m_fSpread;			// Random spread for the object 
		DFLOAT	m_fStartTime;		// When did we start
		DFLOAT	m_fDuration;		// Time that this object will exist in the world
		DFLOAT	m_fFadeTime;		// Time to fade out during the end of the duration
		DFLOAT	m_fRadius;			// Average radius to use when detecting collision
		DFLOAT	m_fInitAlpha;		// Initial alpha value for the model

		DBOOL	m_bSpawnExp;		// Create a new explosion on impact or after duration
		DDWORD	m_nSpawnType;		// Type of explosion to spawn
		DDWORD	m_nTrailType;		// Type of trail to place on the frag

		DBOOL	m_bRandDir;			// Shoot in a random direction despite normal?
		DBOOL	m_bRotate;			// Rotate the object?
		DVector	m_vRotateMax;		// Maximum values for rotation
		DFLOAT	m_fPitch, fPitch;
		DFLOAT	m_fYaw, fYaw;
		DFLOAT	m_fRoll, fRoll;
		DVector	m_vLastNormal;

		HSTRING	m_szModel;			// Model file to use for this object
		HSTRING	m_szSkin;			// Skin for the model
};

// ----------------------------------------------------------------------- //

class CExplosionFX : public CSpecialFX
{
	public :

		CExplosionFX()
		{
			VEC_INIT(m_vPos);
			VEC_SET(m_vNormal, 0.0f, 1.0f, 0.0f);
			m_nExpID = 0;
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update()		{	return	DFALSE;	}

	private :

		DVector	m_vPos;				// Position of the light
		DVector	m_vNormal;			// Normal of the surface hit
		DDWORD	m_nExpID;			// Type of explosion to create at this location
};

#endif // __EXPLOSION_FX_H__