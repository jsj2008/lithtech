 // ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.h
//
// PURPOSE : Mark special fx class - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __MARKSFX_H__
#define __MARKSFX_H__

#include "SpecialFX.h"
#include "ltlink.h"

struct MARKCREATESTRUCT : public SFXCREATESTRUCT
{
	MARKCREATESTRUCT::MARKCREATESTRUCT();

	LTRotation	m_Rotation;
	LTVector		m_vPos;
	LTFLOAT		m_fScale;
	HSTRING		m_hstrSprite;
};

inline MARKCREATESTRUCT::MARKCREATESTRUCT()
{
	memset(this, 0, sizeof(MARKCREATESTRUCT));
}


class CMarkSFX : public CSpecialFX
{
	public :

		CMarkSFX()
		{
			m_Rotation.Init();
			VEC_INIT( m_vPos );
			m_fScale = 1.0f;
			m_hstrSprite = LTNULL;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update() { return LTTRUE; }
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	private :

		LTRotation	m_Rotation;
		LTVector		m_vPos;
		LTFLOAT		m_fScale;
		HSTRING		m_hstrSprite;
};

#endif // __MARKSFX_H__