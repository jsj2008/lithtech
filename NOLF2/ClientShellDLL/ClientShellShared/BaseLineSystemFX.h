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

		CBaseLineSystemFX() : CSpecialFX(),
			m_vPos(0.0f, 0.0f, 0.0f),
			m_rRot(LTQuaternionf(0.0f, 0.0f, 0.0f, 0.0f)) // Note : The extra 0 means that it needs initialization
		{
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

        LTVector   m_vPos;
        LTRotation m_rRot;
};

#endif // __BASE_LINE_SYSTEM_FX_H__