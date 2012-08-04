// ----------------------------------------------------------------------- //
//
// MODULE  : BaseLineSystemFX.h
//
// PURPOSE : BaseLineSystem special fx class - Definition
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_LINE_SYSTEM_FX_H__
#define __BASE_LINE_SYSTEM_FX_H__

#include "SpecialFX.h"

class CBaseLineSystemFX : public CSpecialFX
{
	public :

		CBaseLineSystemFX() : CSpecialFX()
		{
			VEC_INIT(m_vPos);
            m_rRot.Init();
			m_rRot.m_Quat[3] = 0.0f;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

        LTVector   m_vPos;
        LTRotation m_rRot;
};

#endif // __BASE_LINE_SYSTEM_FX_H__