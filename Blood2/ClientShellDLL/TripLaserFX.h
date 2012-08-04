// ----------------------------------------------------------------------- //
//
// MODULE  : TripLaserFX.h
//
// PURPOSE : TripLaser special fx class - Definition
//
// CREATED : 5/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __TRIPLASER_FX_H__
#define __TRIPLASER_FX_H__

#include "BaseLineSystemFX.h"

class CTripLaserFX : public CBaseLineSystemFX
{
	public :

		CTripLaserFX() : CBaseLineSystemFX() 
		{
			VEC_INIT(m_vColor);

			m_fStartTime	= -1.0f;
			m_bTriggered	= DTRUE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL	m_bTriggered;	// Object is triggered on?
		DVector	m_vColor;		// Color of line
		DFLOAT	m_fLength;		// Length of the line to draw

		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DBOOL	m_bFirstUpdate;
};


// Info. needed to create a TripLaserFX...

struct TRIPLASERCREATESTRUCT : public SFXCREATESTRUCT
{
	TRIPLASERCREATESTRUCT::TRIPLASERCREATESTRUCT();

	DVector vColor;
	DFLOAT	fLength;
	DBOOL	bTriggered;
};

inline TRIPLASERCREATESTRUCT::TRIPLASERCREATESTRUCT()
{
	memset(this, 0, sizeof(TRIPLASERCREATESTRUCT));
}




#endif // __TRIPLASER_FX_H__