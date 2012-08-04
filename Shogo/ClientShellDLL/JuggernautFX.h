// ----------------------------------------------------------------------- //
//
// MODULE  : JuggernautFX.h
//
// PURPOSE : Juggernaut special fx class - Definition
//
// CREATED : 4/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __JUGGERNAUT_FX_H__
#define __JUGGERNAUT_FX_H__

#include "SpecialFX.h"

struct JNCREATESTRUCT : public SFXCREATESTRUCT
{
	JNCREATESTRUCT::JNCREATESTRUCT();

	LTVector		vFirePos;
	LTVector		vEndPos;
	LTFLOAT		fFadeTime;
};

inline JNCREATESTRUCT::JNCREATESTRUCT()
{
	memset(this, 0, sizeof(JNCREATESTRUCT));
}


class CJuggernautFX : public CSpecialFX
{
	public :

		CJuggernautFX() : CSpecialFX() 
		{
			VEC_INIT(m_vFirePos);
			VEC_INIT(m_vEndPos);
			m_fFadeTime = 1.0f;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		LTVector		m_vFirePos;
		LTVector		m_vEndPos;
		LTFLOAT		m_fFadeTime;
		LTFLOAT		m_fStartTime;
};

#endif // __JUGGERNAUT_FX_H__