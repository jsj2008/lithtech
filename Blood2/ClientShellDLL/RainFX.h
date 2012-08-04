// ----------------------------------------------------------------------- //
//
// MODULE  : RainFX.h
//
// PURPOSE : Rain special fx class - Definition
//
// CREATED : 11/16/97
//
// ----------------------------------------------------------------------- //

#ifndef __RAINFX_H__
#define __RAINFX_H__

#include "BaseParticleSystemFX.h"

// Info. needed to create a RainFX...

struct RAINCREATESTRUCT : public SFXCREATESTRUCT
{
	RAINCREATESTRUCT::RAINCREATESTRUCT();

	DDWORD	dwFlags;
	DFLOAT	fDensity;
	DFLOAT	fLifetime;
	DBOOL	bGravity;
	DFLOAT	fParticleScale;
	DFLOAT	fSpread;
	DFLOAT  fTimeLimit;
	DFLOAT  fPulse;
	DVector vDims;
	DVector vDirection;
	DVector vColor1;
	DVector vColor2;
};

inline RAINCREATESTRUCT::RAINCREATESTRUCT()
{
	memset(this, 0, sizeof(RAINCREATESTRUCT));
}


class CRainFX : public CBaseParticleSystemFX
{
	public :

		CRainFX();

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	protected :

		// Creation data...

		RAINCREATESTRUCT  m_cs;		// Holds all initialization data

		DBOOL	m_bFirstUpdate;		// Is this the first update
		int		m_nParticlesAdded;	// How many particles have been added
		DFLOAT	m_fNextUpdate;		// Time between updates
		DFLOAT	m_fTimeLen;			// How long system has been active
		DFLOAT	m_fLastTime;		// When was the last time
		DFLOAT	m_fLifetime;
		DFLOAT	m_fAreaDensity;		// Area of the rain
		DBOOL	m_bGravity;			// Rain is affected by gravity
		DVector m_vDims;
		DVector	m_vDirection;		// Direction rain is falling
		DVector	m_vMinOffset;		// Minumum offset
		DVector m_vMaxOffset;		// Maximum offset
		DVector m_vMinVel;			// Minimum velocity
		DVector m_vMaxVel;			// Maximum velocity
		DVector	m_vColor[6];		// Color array
		DFLOAT	m_fSpread;			// Spread
		DVector m_vColor1;			// Color 1
		DVector m_vColor2;			// Color 2
		DFLOAT	m_fTimeLimit;
		DFLOAT  m_fPulse;
};

#endif // __RAINFX_H__