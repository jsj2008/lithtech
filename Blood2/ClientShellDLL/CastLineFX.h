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

	DVector vStartColor;
	DVector vEndColor;
	DFLOAT	fStartAlpha;
	DFLOAT	fEndAlpha;
	HLOCALOBJ	hCastTo;
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

			m_bFirstUpdate = DTRUE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	protected :

		DVector		m_vStartColor;
		DVector		m_vEndColor;
		DFLOAT		m_fStartAlpha;
		DFLOAT		m_fEndAlpha;

		DBOOL		m_bFirstUpdate;
		HLOCALOBJ	m_hCastTo;
};

#endif // __CAST_LINE_FX_H__