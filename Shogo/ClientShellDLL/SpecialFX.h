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

#include "ltbasedefs.h"
#include "clientheaders.h"
#include <memory.h>  // for memset
#include "iltmath.h"
#include "iltcommon.h"
#include "iltsoundmgr.h"

// Get the distance from a point to a plane.
#define DIST_TO_PLANE(vec, plane) ( VEC_DOT((plane).m_Normal, (vec)) - (plane).m_Dist )

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
			m_bWantRemove		= LTFALSE;
			m_pClientDE			= LTNULL;
			m_hObject			= LTNULL;
			m_hServerObject		= LTNULL;
			m_fUpdateDelta		= 0.001f;
			m_fNextUpdateTime	= 0.0f;
		}

		virtual ~CSpecialFX()
		{
			if (m_pClientDE && m_hObject)
			{
				m_pClientDE->RemoveObject(m_hObject);
				m_hObject = LTNULL;
			}
		}
		
		virtual LTBOOL CreateObject(ILTClient* pClientDE) 
		{
			if (!pClientDE) return LTFALSE;
			m_pClientDE = pClientDE;
			return LTTRUE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{ 
			if (!psfxCreateStruct) return LTFALSE;

			m_hServerObject = psfxCreateStruct->hServerObj;

			return LTTRUE;
		}

		// Return of LTFALSE indicates special fx is done and can be removed.

		virtual LTBOOL Update() = 0;

		// Call this to tell special fx to finish up so we can remove it...

		virtual void WantRemove(LTBOOL bRemove=LTTRUE) 
		{
			m_bWantRemove = bRemove; 
			if (m_bWantRemove) m_hServerObject = LTNULL;
		}
		LTBOOL IsWaitingForRemove() const { return m_bWantRemove; }
		
		HLOCALOBJ	GetObject()		const { return m_hObject; }
		HLOCALOBJ	GetServerObj()	const { return m_hServerObject; }

		LTFLOAT	GetUpdateDelta()	const { return m_fUpdateDelta; }

		virtual void HandleTouch(CollisionInfo *pInfo, LTFLOAT forceMag) {}

		LTFLOAT	m_fNextUpdateTime;	// When do we update next

	protected :

		ILTClient*	m_pClientDE;
		HLOCALOBJ	m_hObject;			// Special FX object
		HLOCALOBJ	m_hServerObject;	// Local handle to Server-side object
		LTBOOL		m_bWantRemove;
		LTFLOAT		m_fUpdateDelta;		// Time between updates
};

#endif // __SPECIAL_FX_H__