// ----------------------------------------------------------------------- //
//
// MODULE  : TracerFX.h
//
// PURPOSE : Bullet Tracer special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __TRACER_FX_H__
#define __TRACER_FX_H__

#include "BaseLineSystemFX.h"

// Info. needed to create a TracerFX...

struct TRACERCREATESTRUCT : public SFXCREATESTRUCT
{
	TRACERCREATESTRUCT::TRACERCREATESTRUCT();

	HLOCALOBJ	hGun;
	DVector vVel;
	DVector vStartColor;
	DVector vEndColor;
	DVector	vStartPos;
	DFLOAT	fStartAlpha;
	DFLOAT	fEndAlpha;
	DVector vToPos;
	DRotation rRot;
};

inline TRACERCREATESTRUCT::TRACERCREATESTRUCT()
{
	memset(this, 0, sizeof(TRACERCREATESTRUCT));
}


class CTracerFX : public CBaseLineSystemFX
{
	public :

		CTracerFX() : CBaseLineSystemFX() 
		{
			VEC_INIT(m_vVel);
			VEC_SET(m_vStartColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vEndColor, 1.0f, 1.0f, 1.0f);
			VEC_INIT(m_vStartPos);
			m_fStartAlpha	= 1.0f;
			m_fEndAlpha		= 1.0f;

			m_bFirstUpdate	= DTRUE;
			m_fStartTime	= 0.0f;
			m_fDuration		= 0.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector m_vVel;			// Velocity
		DVector	m_vStartPos;	// Starting position
		DVector	m_vStartColor;	// Color of beginning of line
		DVector	m_vEndColor;	// Color of end of line
		DRotation m_rRotation;
		DFLOAT	m_fStartAlpha;
		DFLOAT	m_fEndAlpha;

		DFLOAT	m_fLifeTime;	// How long each particle stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DBOOL	m_bFirstUpdate;	// First update
		DFLOAT		m_fDuration;
};

#endif // __TRACER_FX_H__