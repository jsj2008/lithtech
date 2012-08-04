 // ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.h
//
// PURPOSE : Mark special fx class - Definition
//
// CREATED : 11/6/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MARKSFX_H__
#define __MARKSFX_H__

#include "SpecialFX.h"
#include "ltlink.h"

struct MARKCREATESTRUCT : public SFXCREATESTRUCT
{
	MARKCREATESTRUCT();

	LTObjRef			m_hParent;
	LTRigidTransform	m_tTransform;
	HAMMO				hAmmo;
	uint8				nSurfaceType;
};

inline MARKCREATESTRUCT::MARKCREATESTRUCT()
{
	m_tTransform.Init();
	m_hParent		= NULL;
	hAmmo			= NULL;
	nSurfaceType	= 0;
}


class CMarkSFX : public CSpecialFX
{
	public :

		CMarkSFX()
		{
			m_tTransform.Init();
			m_hAmmo = NULL;
			m_nSurfaceType = 0;
			m_fElapsedTime = 0.0f;
		}

		virtual bool	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool	Update();

		virtual bool	CreateObject(ILTClient* pClientDE);
		virtual uint32	GetSFXID() { return SFX_MARK_ID; }

		CClientFXLink*	GetClientFXLink() { return &m_fxMark; }

	private :

		LTObjRef			m_hParent;
		LTRigidTransform	m_tTransform;
		float				m_fElapsedTime;
		HAMMO				m_hAmmo;
		uint8				m_nSurfaceType;

		CClientFXLink		m_fxMark;
};

#endif // __MARKSFX_H__