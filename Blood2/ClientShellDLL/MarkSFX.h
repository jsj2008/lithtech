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
#include "dlink.h"


struct MARKCREATESTRUCT : public SFXCREATESTRUCT
{
	MARKCREATESTRUCT::MARKCREATESTRUCT();

	DVector		m_Pos;
	DRotation	m_Rotation;
	DFLOAT		m_fScale;
	HSTRING		m_hstrSprite;
	DBOOL		m_bServerObj;
//	DBOOL		m_bScaleY;
//	DFLOAT		m_fScaleCount;
//	DVector		m_vColor;
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
			ROT_INIT( m_Rotation );
			m_hstrSprite = DNULL;
			VEC_INIT(m_vScale);
//			m_bScaleY = DFALSE;
//			m_fScaleCount = 0;
//			VEC_INIT(m_vColor);
		}

		~CMarkSFX()
		{
			if( m_hstrSprite && m_pClientDE )
				m_pClientDE->FreeString( m_hstrSprite );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		HOBJECT	GetHandle()		{return m_hObject;}

	private :

		DVector		m_Pos;
		DRotation	m_Rotation;
		DVector		m_vForward;
		HSTRING		m_hstrSprite;
		DVector		m_vScale;
		DBOOL		m_bServerObj;
//		DBOOL		m_bScaleY;
//		DFLOAT		m_fScaleCount;
//		DVector		m_vColor;

//		DFLOAT		m_fShrinkStart;
};

#endif // __MARKSFX_H__