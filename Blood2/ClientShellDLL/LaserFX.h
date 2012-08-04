// ----------------------------------------------------------------------- //
//
// MODULE  : LaserFX.h
//
// PURPOSE : Bullet Laser special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_FX_H__
#define __LASER_FX_H__

#include "BaseLineSystemFX.h"

class CLaserFX : public CBaseLineSystemFX
{
	public :

		CLaserFX() : CBaseLineSystemFX() 
		{
			VEC_INIT(m_vColor1);
			VEC_INIT(m_vColor2);

			m_fStartTime	= -1.0f;
			m_fLifeTime		= 0.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		HLOCALOBJ m_hGun;		// Object doing the firing
		DVector	m_vColor1;		// Color of darkest smoke particles
		DVector	m_vColor2;		// Color of lightest smoke particles

		DFLOAT	m_fLifeTime;	// How long each particle stays around
		DFLOAT	m_fStartTime;	// When did we start this crazy thing
		DBOOL	m_bFirstUpdate;
};


// Info. needed to create a LaserFX...

struct LASERCREATESTRUCT : public SFXCREATESTRUCT
{
	LASERCREATESTRUCT::LASERCREATESTRUCT();

	HLOCALOBJ hGun;
};

inline LASERCREATESTRUCT::LASERCREATESTRUCT()
{
	memset(this, 0, sizeof(LASERCREATESTRUCT));
}




#endif // __LASER_FX_H__