// ----------------------------------------------------------------------- //
//
// MODULE  : WarpGateFX.h
//
// PURPOSE : Warp gate (spawner) special fx class - Definition
//
// CREATED : 8/15/98
//
// ----------------------------------------------------------------------- //

#ifndef __WARPGATE_FX_H__
#define __WARPGATE_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"
#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct WARPGATESPRITECS : public SFXCREATESTRUCT
{
	WARPGATESPRITECS::WARPGATESPRITECS();

	HLOCALOBJ	hObj;
	DFLOAT		fRampUpTime;
	DFLOAT		fRampDownTime;
	DFLOAT		fMinScale;
	DFLOAT		fMaxScale;
	DFLOAT		fAlpha;
	DDWORD		nRampUpType;
	DDWORD		nRampDownType;
	DBYTE		bAlign;
	HSTRING		szSprite;
};

// ----------------------------------------------------------------------- //

inline WARPGATESPRITECS::WARPGATESPRITECS()
{
	memset(this, 0, sizeof(WARPGATESPRITECS));
}

// ----------------------------------------------------------------------- //

struct WARPGATEPARTICLECS : public SFXCREATESTRUCT
{
	WARPGATEPARTICLECS::WARPGATEPARTICLECS();

	HLOCALOBJ	hObj;
	DFLOAT		fRampUpTime;
	DFLOAT		fRampDownTime;
	DFLOAT		fSystemRadius;
	DFLOAT		fPosRadius;
	DVector		vOffset;
	DVector		vRotations;
	DFLOAT		fMinVelocity;
	DFLOAT		fMaxVelocity;
	DDWORD		nNumParticles;
	DDWORD		nEmitType;
	DVector		vMinColor;
	DVector		vMaxColor;
	DFLOAT		fAlpha;
	DFLOAT		fMinLifetime;
	DFLOAT		fMaxLifetime;
	DFLOAT		fAddDelay;
	DFLOAT		fGravity;
	DDWORD		nRampUpType;
	DDWORD		nRampDownType;
	DBYTE		bAlign;
	HSTRING		szParticle;
};

// ----------------------------------------------------------------------- //

inline WARPGATEPARTICLECS::WARPGATEPARTICLECS()
{
	memset(this, 0, sizeof(WARPGATEPARTICLECS));
}

// ----------------------------------------------------------------------- //

class CWarpGateSpriteFX : public CSpecialFX
{
	public :

		CWarpGateSpriteFX()
		{
			m_fMinScale = m_fMaxScale = 1.0f;
			m_fAlpha = 1.0f;
			m_nRampUpType = m_nRampDownType = 0;
			m_bAlign = 0;
			m_szSprite = 0;
			m_fTriggerTime = 0.0f;
			m_bFirstUpdate = 1;
		}
		~CWarpGateSpriteFX()
		{
			g_pClientDE->FreeString( m_szSprite );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		HLOCALOBJ	m_hObj;				// Object from the server
		DFLOAT		m_fRampUpTime;		// Time to ramp up the FX
		DFLOAT		m_fRampDownTime;	// Time to ramp down the FX

		DFLOAT		m_fMinScale;		// The smallest scale of the sprite
		DFLOAT		m_fMaxScale;		// The largest scale of the sprite
		DFLOAT		m_fAlpha;			// Alpha of the sprite
		DDWORD		m_nRampUpType;		// How should we ramp up the sprite?
		DDWORD		m_nRampDownType;	// How should we ramp down the sprite?
		DBOOL		m_bAlign;			// Should we align the sprite to the forward vector

		HSTRING		m_szSprite;			// File to use for the sprite

		DFLOAT		m_fTriggerTime;		// Time that a trigger occured
		DBOOL		m_bFirstUpdate;		// First update for FX?
		DBOOL		m_bStarted;			// Has the effect started up?
		DVector		m_vScale;			// Scale of the sprite
};

// ----------------------------------------------------------------------- //

class CWarpGateParticleFX : public CBaseParticleSystemFX
{
	public :

		CWarpGateParticleFX()
		{
			m_fTriggerTime = 0.0f;
			m_fLastAddTime = 0.0f;
			m_bFirstUpdate = 1;
			m_bStarted = 0;
		}
		~CWarpGateParticleFX()
		{
			g_pClientDE->FreeString( m_szParticle );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		void	AddParticles();

		HLOCALOBJ	m_hObj;				// Object from the server
		DFLOAT		m_fRampUpTime;		// Time to ramp up the FX
		DFLOAT		m_fRampDownTime;	// Time to ramp down the FX

		DFLOAT		m_fPosRadius;		// Emission radius of the particles
		DVector		m_vOffset;			// Offset of the particles system from the object
		DVector		m_vRotations;		// Amount to rotate the particle system
		DFLOAT		m_fMinVelocity;		// Minimum velocity for each particle
		DFLOAT		m_fMaxVelocity;		// Maximum velocity for each particle
		DDWORD		m_nNumParticles;	// Number of particles to add for each update
		DDWORD		m_nEmitType;		// How to add the particles
		DFLOAT		m_fAlpha;			// Alpha for the system
		DFLOAT		m_fMinLifetime;		// Minimum lifetime for the particles
		DFLOAT		m_fMaxLifetime;		// Maximum lifetime for the particles
		DFLOAT		m_fAddDelay;		// Delay to add more particles
		DDWORD		m_nRampUpType;		// How to ramp up the system
		DDWORD		m_nRampDownType;	// How to ramp down the system
		DBOOL		m_bAlign;			// Should we align the system to the forward vector

		HSTRING		m_szParticle;		// File to use for the particle
		DFLOAT		m_fTriggerTime;		// Time that the effect was triggered
		DFLOAT		m_fLastAddTime;		// Time that last batch of particles were added
		DBOOL		m_bFirstUpdate;		// First update for FX?
		DBOOL		m_bStarted;			// Has the effect started up?
};

#endif // __WARPGATE_FX_H__