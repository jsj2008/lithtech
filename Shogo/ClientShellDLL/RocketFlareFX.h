// ----------------------------------------------------------------------- //
//
// MODULE  : RocketFlareFX.h
//
// PURPOSE : RocketFlare special fx class - Definition
//
// CREATED : 11/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __ROCKET_FLARE_FX_H__
#define __ROCKET_FLARE_FX_H__

#include "BaseParticleSystemFX.h"


struct RFCREATESTRUCT : public SFXCREATESTRUCT
{
	RFCREATESTRUCT::RFCREATESTRUCT();

	LTVector vVel;
	LTBOOL   bSmall;
};

inline RFCREATESTRUCT::RFCREATESTRUCT()
{
	memset(this, 0, sizeof(RFCREATESTRUCT));
}


class CRocketFlareFX : public CBaseParticleSystemFX
{
	public :

		CRocketFlareFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vVel);
			m_bSmall = LTFALSE;
		}

		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	private :

		LTBOOL AddFlareParticles();

		LTVector	m_vVel;			// Velocity of smoking projectile
		LTBOOL	m_bSmall;		// Relative size of smoke
};

#endif // __ROCKET_FLARE_FX_H__