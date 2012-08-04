// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleStreamFX.h
//
// PURPOSE : Special FX class for streams of particles
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLESTREAM_FX_H__
#define __PARTICLESTREAM_FX_H__

#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct PSTREAMCREATESTRUCT : public SFXCREATESTRUCT
{
	PSTREAMCREATESTRUCT::PSTREAMCREATESTRUCT();

	DFLOAT	fRadius;
	DFLOAT	fPosRadius;
	DFLOAT	fMinVel;
	DFLOAT	fMaxVel;
	DDWORD	nNumParticles;
	DFLOAT	fSpread;
	DVector vColor1;
	DVector vColor2;
	DFLOAT	fAlpha;
	DFLOAT	fMinLife;
	DFLOAT	fMaxLife;
	DFLOAT	fRampTime;
	DFLOAT	fDelay;
	DFLOAT	fGravity;
	DBYTE	bRampFlags;
	DBOOL	bOn;
	HSTRING hstrTexture;
	HSTRING	hstrSound1;
	HSTRING	hstrSound2;
	HSTRING	hstrSound3;
	DFLOAT	fSoundRadius;
};

// ----------------------------------------------------------------------- //

inline PSTREAMCREATESTRUCT::PSTREAMCREATESTRUCT()
{
	memset(this, 0, sizeof(PSTREAMCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

#define		PSTREAM_RAMP_NUM		0x01
#define		PSTREAM_RAMP_OFFSET		0x02
#define		PSTREAM_RAMP_VEL		0x04
#define		PSTREAM_RAMP_LIFE		0x08

// ----------------------------------------------------------------------- //

class CParticleStreamFX : public CBaseParticleSystemFX
{
	public :

		CParticleStreamFX() : CBaseParticleSystemFX() 
		{
			VEC_SET(m_vDir, 0.0f, 1.0f, 0.0f);
			m_fRadius			= 100.0f;
			m_fPosRadius		= 1.0f;
			m_fMinVel			= 100.0f;
			m_fMaxVel			= 150.0f;
			m_nNumParticles		= 1;
			m_fSpread			= 100.0f;

			VEC_SET(m_vColor1, 0.0f, 0.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);

			m_fAlpha			= 1.0f;
			m_fRampTime			= 2.0f;
			m_fDelay			= 0.1f;

			m_bState			= 0;
			m_hstrSound1		= DNULL;
			m_hstrSound2		= DNULL;
			m_hstrSound3		= DNULL;
			m_hsSound			= DNULL;
			m_hstrTexture		= DNULL;
		}

		~CParticleStreamFX()
		{
			g_pClientDE->FreeString( m_hstrSound1 );
			g_pClientDE->FreeString( m_hstrSound2 );
			g_pClientDE->FreeString( m_hstrSound3 );
			g_pClientDE->FreeString( m_hstrTexture );

			if( m_hsSound )
			{
				g_pClientDE->KillSound( m_hsSound );
			}
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL AddParticles();

		DVector m_vU, m_vR;			// Up and right vectors
		DVector m_vDir;				// Direction splash shoots

		DFLOAT	m_fPosRadius;		// Radius of the splash origin
		DFLOAT	m_fMinVel;			// Minimum initial velocity of partciles
		DFLOAT	m_fMaxVel;			// Maximum initial velocity of particles
		DDWORD	m_nNumParticles;	// How many particles to add every delay update
		DFLOAT	m_fSpread;			// How far to randomly spread the particles
		DFLOAT	m_fAlpha;			// Initial alpha for particle system
		DFLOAT	m_fMinLife;			// Minimum lifetime of each particle
		DFLOAT	m_fMaxLife;			// Maximum lifetime of each particle
		DFLOAT	m_fRampTime;		// Time to ramp velocities when turning on and off
		DFLOAT	m_fDelay;			// Delay between adding new particles
		DBYTE	m_bState;			// State of the particle system
		DBYTE	m_bRampFlags;		// Flags of which elements about the system to ramp

		HSTRING	m_hstrSound1;		// Ramp up sound
		HSTRING	m_hstrSound2;		// Looping sound for state 2
		HSTRING	m_hstrSound3;		// Ramp down sound
		DFLOAT	m_fSoundRadius;		// Radius to play the sounds

		HSOUNDDE	m_hsSound;		// Current sound playing

		HSTRING m_hstrTexture;		// Texture to sprite to use

		DFLOAT	m_fTriggerTime;		// When did we got a trigger message
		DFLOAT	m_fLastAddTime;		// Last time that particles were added
};

#endif // __PARTICLESTREAM_FX_H__