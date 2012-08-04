// ----------------------------------------------------------------------- //
//
// MODULE  : CastLineFX.h
//
// PURPOSE : CastLine special fx class - Definition
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __CAST_LINE_FX_H__
#define __CAST_LINE_FX_H__

#include "BaseLineSystemFX.h"

struct CLCREATESTRUCT : public SFXCREATESTRUCT
{
	CLCREATESTRUCT::CLCREATESTRUCT();

	LTVector vStartColor;
	LTVector vEndColor;
	LTFLOAT	fStartAlpha;
	LTFLOAT	fEndAlpha;
};

inline CLCREATESTRUCT::CLCREATESTRUCT()
{
	memset(this, 0, sizeof(CLCREATESTRUCT));
}


class CCastLineFX : public CBaseLineSystemFX
{
	public :

		CCastLineFX() : CBaseLineSystemFX() 
		{
			VEC_SET(m_vStartColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vEndColor, 1.0f, 1.0f, 1.0f);
			m_fStartAlpha	= 1.0f;
			m_fEndAlpha		= 1.0f;

			m_bFirstUpdate = LTTRUE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	protected :

		LTVector		m_vStartColor;
		LTVector		m_vEndColor;
		LTFLOAT		m_fStartAlpha;
		LTFLOAT		m_fEndAlpha;

		LTBOOL		m_bFirstUpdate;
};

#endif // __CAST_LINE_FX_H__