
// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.h
//
// PURPOSE : Debug line class - Definition
//
// CREATED : 8/11/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUGLINE_FX_H__
#define __DEBUGLINE_FX_H__

#include "BaseLineSystemFX.h"

struct DEBUGLINECREATESTRUCT : public SFXCREATESTRUCT
{
	DEBUGLINECREATESTRUCT::DEBUGLINECREATESTRUCT();

	DVector vFromPos;
	DVector vToPos;
};

inline DEBUGLINECREATESTRUCT::DEBUGLINECREATESTRUCT()
{
	memset(this, 0, sizeof(DEBUGLINECREATESTRUCT));
}


class CDebugLine : public CBaseLineSystemFX
{
	public :

		CDebugLine() : CBaseLineSystemFX() 
		{
			VEC_INIT(m_vFromPos);
			VEC_INIT(m_vToPos);

			m_bFirstUpdate	= DTRUE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DVector	m_vFromPos;	// Starting position
		DVector	m_vToPos;	// Ending position

		DBOOL	m_bFirstUpdate;
		DFLOAT	m_fStartTime;
};

#endif // __DEBUGLINE_FX_H__