// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFX.h
//
// PURPOSE : Generic client-side Special FX wrapper class - Definition
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIAL_FX_H__
#define __SPECIAL_FX_H__

#include "basedefs_de.h"
#include "cpp_client_de.h"
#include <memory.h>  // for memset


struct SFXCREATESTRUCT
{
	SFXCREATESTRUCT::SFXCREATESTRUCT();

	HLOCALOBJ	hServerObj;
};

inline SFXCREATESTRUCT::SFXCREATESTRUCT()
{
	memset(this, 0, sizeof(SFXCREATESTRUCT));
}


class CSpecialFX
{
	public :

		CSpecialFX()
		{
			m_bWantRemove		= DFALSE;
			m_pClientDE			= DNULL;
			m_hObject			= DNULL;
			m_hServerObject		= DNULL;
			m_fUpdateDelta		= 0.001f;
			m_fNextUpdateTime	= 0.0f;
		}

		virtual ~CSpecialFX()
		{
			if (m_pClientDE && m_hObject)
			{
				m_pClientDE->DeleteObject(m_hObject);
				m_hObject = DNULL;
			}
		}
		
		virtual DBOOL Term()
		{
			return DTRUE;
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE) 
		{
			if (!pClientDE) return DFALSE;
			m_pClientDE = pClientDE;
			return DTRUE;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{ 
			if (!psfxCreateStruct) return DFALSE;

			m_hServerObject = psfxCreateStruct->hServerObj;

			return DTRUE;
		}

		// Return of DFALSE indicates special fx is done and can be removed.

		virtual DBOOL Update() = 0;

		// Call this to tell special fx to finish up so we can remove it...

		virtual void WantRemove(DBOOL bRemove=DTRUE) 
		{
			m_bWantRemove = bRemove; 
			if (m_bWantRemove) m_hServerObject = DNULL;
		}
		DBOOL IsWaitingForRemove() const { return m_bWantRemove; }
		
		HLOCALOBJ	GetObject()		const { return m_hObject; }
		HLOCALOBJ	GetServerObj()	const { return m_hServerObject; }

		DFLOAT	GetUpdateDelta()	const { return m_fUpdateDelta; }

		DFLOAT	m_fNextUpdateTime;	// When do we update next

	protected :

		CClientDE*	m_pClientDE;
		HLOCALOBJ	m_hObject;			// Special FX object
		HLOCALOBJ	m_hServerObject;	// Local handle to Server-side object
		DBOOL		m_bWantRemove;
		DFLOAT		m_fUpdateDelta;		// Time between updates
};

#endif // __SPECIAL_FX_H__