// ----------------------------------------------------------------------- //
//
// MODULE  : LaserCannonFX.h
//
// PURPOSE : LaserCannon special fx class - Definition
//
// CREATED : 1/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_CANNON_FX_H__
#define __LASER_CANNON_FX_H__

#include "SpecialFX.h"

struct LCCREATESTRUCT : public SFXCREATESTRUCT
{
	LCCREATESTRUCT::LCCREATESTRUCT();

	uint32	dwClientID;
};

inline LCCREATESTRUCT::LCCREATESTRUCT()
{
	memset(this, 0, sizeof(LCCREATESTRUCT));
}


class CLaserCannonFX : public CSpecialFX
{
	public :

		CLaserCannonFX() : CSpecialFX() 
		{
			m_dwClientID	= -1;

			m_bFirstUpdate	= LTTRUE;
			m_fLastRotTime	= 0.0f;
			m_fRotPerSec	= 1.0f;
			m_fRotAmount	= 0.0f;
	
			m_fNextLightTime = 0.0f;
			m_fLightWaitTime = 2.0f;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		void CreateLightFX(LTVector* pvPos);

		uint8		m_dwClientID;

		LTBOOL		m_bFirstUpdate;
		LTFLOAT		m_fLastRotTime;
		LTFLOAT		m_fRotPerSec;
		LTFLOAT		m_fRotAmount;

		LTFLOAT		m_fNextLightTime;
		LTFLOAT		m_fLightWaitTime;
};

#endif // __LASER_CANNON_FX_H__