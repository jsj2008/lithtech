// ----------------------------------------------------------------------- //
//
// MODULE  : LightningSegmentFX.h
//
// PURPOSE : Special FX class for lightning-like instant particle streams
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNINGSEGMENT_FX_H__
#define __LIGHTNINGSEGMENT_FX_H__

#include "BaseParticleSystemFX.h"

// ----------------------------------------------------------------------- //

struct LSEGMENTCREATESTRUCT : public SFXCREATESTRUCT
{
	LSEGMENTCREATESTRUCT::LSEGMENTCREATESTRUCT();

	DVector		vOffset;
	DVector		vNextOffset;
//	DVector		vDir;
	DVector		vColor;
	DFLOAT		fIncrement;
	DFLOAT		fAlpha;
	DFLOAT		fRadius;
	DFLOAT		fDuration;
	DFLOAT		fFadeTime;
	DDWORD		nNumParticles;
	HSTRING		hstrTexture;
};

// ----------------------------------------------------------------------- //

inline LSEGMENTCREATESTRUCT::LSEGMENTCREATESTRUCT()
{
	memset(this, 0, sizeof(LSEGMENTCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

class CLightningSegmentFX : public CBaseParticleSystemFX
{
	public :

		CLightningSegmentFX() : CBaseParticleSystemFX() {}
		~CLightningSegmentFX()
		{
			g_pClientDE->FreeString( m_hstrTexture );
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL	CreateParticles();

		DVector m_vOffset;			// Up and right vector offsets from the gun
		DVector	m_vNextOffset;		// Next position to align the rotation
//		DVector m_vDir;				// Direction vector was shot
		DFLOAT	m_fIncrement;		// Increment down the forward axis to place particles
		DFLOAT	m_fAlpha;			// Alpha for the systems
		DFLOAT	m_fDuration;		// Time to hang around
		DFLOAT	m_fFadeTime;		// Amount of time to fade during the end of the duration
		DDWORD	m_nNumParticles;	// How many particles to stretch across the Dir
		HSTRING m_hstrTexture;		// Texture to sprite to use

		DFLOAT	m_fStartTime;		// Time that the FX started
};

#endif // __LIGHTNINGSEGMENT_FX_H__