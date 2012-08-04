// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectFX.h
//
// PURPOSE : General object special fx class - Definition
//
// CREATED : 8/25/98
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_FX_H__
#define __OBJECT_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"
#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct OBJECTPARTICLEFXCS : public SFXCREATESTRUCT
{
	OBJECTPARTICLEFXCS::OBJECTPARTICLEFXCS();

	HLOCALOBJ	hObj;
	DVector		vOffset;
	DFLOAT		fRampUpTime;
	DFLOAT		fRampDownTime;
	DFLOAT		fDuration;
	DFLOAT		fAddDelay;
	DFLOAT		fRadius;
	DFLOAT		fGravity;
	DVector		vRotations;
	DFLOAT		fPosRadius;
	DDWORD		nEmitType;
	DDWORD		nNumParticles;
	DFLOAT		fDensity;
	DFLOAT		fAlpha;
	DVector		vMinColor;
	DVector		vMaxColor;
	DFLOAT		fMinVelocity;
	DFLOAT		fMaxVelocity;
	DFLOAT		fMinLifetime;
	DFLOAT		fMaxLifetime;
	DDWORD		nRampUpType;
	DDWORD		nRampDownType;
	DDWORD		nFXFlags;
	HSTRING		szParticle;
};

// ----------------------------------------------------------------------- //

inline OBJECTPARTICLEFXCS::OBJECTPARTICLEFXCS()
{
	memset(this, 0, sizeof(OBJECTPARTICLEFXCS));
}

// ----------------------------------------------------------------------- //

#define		OBJPSFX_REMOVESTOPPED		0x01
#define		OBJPSFX_RAMPDOWNSTOPPED		0x02
#define		OBJPSFX_MOVINGSOURCE		0x04
#define		OBJPSFX_MOVINGSTREAM		0x08
#define		OBJPSFX_ROTATINGSOURCE		0x10
#define		OBJPSFX_ALIGNROTATION		0x20
#define		OBJPSFX_SMOOTHSTOP			0x03 // OBJPSFX_REMOVESTOPPED & OBJPSFX_RAMPDOWNSTOPPED
#define		OBJPSFX_STREAMING			0x0C // OBJPSFX_MOVINGSOURCE & OBJPSFX_MOVINGSTREAM
#define		OBJPSFX_FREEROTATION		0x30 // OBJPSFX_ROTATINGSOURCE & OBJPSFX_ALIGNROTATION
#define		OBJPSFX_ALL					0xFF // Everything

#define		OBJPSFX_RAMPALPHA			0x01
#define		OBJPSFX_RAMPVELOCITY		0x02
#define		OBJPSFX_RAMPNUMPARTICLES	0x04
#define		OBJPSFX_RAMPROTATION		0x08
#define		OBJPSFX_RAMPPOSRADIUS		0x10

#define		OBJPSFX_RAMPUP				1
#define		OBJPSFX_RAMPDOWN			2

#define		OBJPSFX_EMITLOCATION		0
#define		OBJPSFX_EMITRING			1
#define		OBJPSFX_EMITSPHERE			2
#define		OBJPSFX_EMITCYLINDER		3
#define		OBJPSFX_EMITFILLEDRING		4
#define		OBJPSFX_EMITFILLEDSPHERE	5
#define		OBJPSFX_EMITFILLEDCYLINDER	6
#define		OBJPSFX_EMITCOMETTAIL		7
#define		OBJPSFX_EMITPOWERRING		8

// ----------------------------------------------------------------------- //

class CObjectParticleFX : public CBaseParticleSystemFX
{
	public :

		CObjectParticleFX()
		{
			m_bFirstUpdate = 1;
			m_bStarted = 0;
			m_fRampRatio = 1.0f;
			m_bRampState = 0;
		}
		~CObjectParticleFX()
		{
			if( m_szParticle )
				g_pClientDE->FreeString( m_szParticle );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :
		void	AddParticles();
		void	GetRandomColor(DVector &vector);

		// Reference object and position / rotation variables
		HLOCALOBJ	m_hObj;				// Reference object for the FX
		DVector		m_vOffset;			// Offset from the object rotation (vU, vR, vF)
		DVector		m_vPos;				// Current position of the system
		DVector		m_vLastPos;			// Last position of the system
		DVector		m_vLastObjPos;		// Last position of the object
		DVector		m_vOrigin;			// First position of the object
		DRotation	m_rRot;				// Current rotation
		DRotation	m_rLastRot;			// Rotation of the object when using ROTATINGSOURCE

		// FX time variables
		DFLOAT		m_fRampUpTime;		// Time to ramp up the FX
		DFLOAT		m_fRampDownTime;	// Time to ramp down the FX
		DFLOAT		m_fDuration;		// Total time to display the FX
		DFLOAT		m_fAddDelay;		// Delay to add more particles
		DFLOAT		m_fLastAddTime;		// Time that last batch of particles were added
		DFLOAT		m_fStartTime;		// Time that the FX was started

		// System detail variables
		DVector		m_vRotations;		// Amount to rotate the particle system (vU, vR, vF)
		DFLOAT		m_fPosRadius;		// Emission radius of the particles
		DDWORD		m_nEmitType;		// How to add the particles
		DDWORD		m_nNumParticles;	// How many particles to add per burst
		DFLOAT		m_fDensity;			// Ratio of particles to add per space
		DFLOAT		m_fAlpha;			// Alpha for the system
		DFLOAT		m_fRampRatio;		// Ratio for the ramp values
		DBOOL		m_bRampState;		// State of ramp

		// Particle detail variables
		DFLOAT		m_fMinVelocity;		// Minimum velocity for each particle
		DFLOAT		m_fMaxVelocity;		// Maximum velocity for each particle
		DFLOAT		m_fMinLifetime;		// Minimum lifetime for the particles
		DFLOAT		m_fMaxLifetime;		// Maximum lifetime for the particles

		// General Flags
		DDWORD		m_nRampUpType;		// How to ramp up the system
		DDWORD		m_nRampDownType;	// How to ramp down the system
		DDWORD		m_nFXFlags;			// What type of FX stuff should we do?

		DBOOL		m_bFirstUpdate;		// First update for FX?
		DBOOL		m_bStarted;			// Has the effect started up?
		DBOOL		m_bMove;			// Move the FX (has it stopped already?)

		HSTRING		m_szParticle;		// File to use for the particle

		// Multiplay fixes
		DBOOL		m_bFirstStop;
		DFLOAT		m_fStopTime;
};

// ----------------------------------------------------------------------- //

struct OBJECTFXCS : public SFXCREATESTRUCT
{
	OBJECTFXCS::OBJECTFXCS();

	HOBJECT		hObj;
	DVector		vOffset;
	DFLOAT		fScale;
	DDWORD		nScaleFlags;
	DDWORD		nFXType;
	DDWORD		nFXFlags;
};

// ----------------------------------------------------------------------- //

inline OBJECTFXCS::OBJECTFXCS()
{
	memset(this, 0, sizeof(OBJECTFXCS));
}

// ----------------------------------------------------------------------- //

class CObjectGeneralFX : public CSpecialFX
{
	public :

		CObjectGeneralFX()
		{
			m_fScale = 1.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update() { return DFALSE; }

	private :

		HLOCALOBJ	m_hObj;				// Reference object for the FX
		DVector		m_vOffset;			// Offset from the object rotation (vU, vR, vF)
		DFLOAT		m_fScale;			// Scale of the FX
		DDWORD		m_nScaleFlags;		// What parts of the FX to scale
		DDWORD		m_nFXType;			// What type of FX to create
		DDWORD		m_nFXFlags;			// General flags for the FX
};

#endif // __OBJECT_FX_H__