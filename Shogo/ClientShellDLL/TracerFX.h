// ----------------------------------------------------------------------- //
//
// MODULE  : TracerFX.h
//
// PURPOSE : Tracer special fx class - Definition
//
// CREATED : 1/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __TRACER_FX_H__
#define __TRACER_FX_H__

#include "BaseLineSystemFX.h"
#include "WeaponDefs.h"

struct TRCREATESTRUCT : public SFXCREATESTRUCT
{
	TRCREATESTRUCT::TRCREATESTRUCT();

	LTRotation	rRot;
	LTVector		vPos;
	LTVector		vVel;
	LTVector		vStartColor;
	LTVector		vEndColor;
	LTVector		vStartPos;
	LTFLOAT		fStartAlpha;
	LTFLOAT		fEndAlpha;
	uint8		nWeaponId;
};

inline TRCREATESTRUCT::TRCREATESTRUCT()
{
	memset(this, 0, sizeof(TRCREATESTRUCT));
	rRot.Init();
}


class CTracerFX : public CBaseLineSystemFX
{
	public :

		CTracerFX() : CBaseLineSystemFX() 
		{
			m_rRot.Init();
			VEC_INIT(m_vVel);
			VEC_SET(m_vStartColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vEndColor, 1.0f, 1.0f, 1.0f);
			VEC_INIT(m_vStartPos);
			m_fStartAlpha	= 1.0f;
			m_fEndAlpha		= 1.0f;

			m_bFirstUpdate	= LTTRUE;
			m_fStartTime	= 0.0f;
			m_fDuration		= 0.0f;
			m_nWeaponId		= GUN_NONE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	protected :

		LTRotation	m_rRot;
		LTVector		m_vVel;
		LTVector		m_vStartColor;
		LTVector		m_vEndColor;
		LTVector		m_vStartPos;
		LTFLOAT		m_fStartAlpha;
		LTFLOAT		m_fEndAlpha;

		LTBOOL		m_bFirstUpdate;
		LTFLOAT		m_fStartTime;
		LTFLOAT		m_fDuration;
		uint8		m_nWeaponId;
};

#endif // __TRACER_FX_H__