
 // ----------------------------------------------------------------------- //
//
// MODULE  : BloodSplatSFX.h
//
// PURPOSE : Blood splat special fx class - Definition
//
// CREATED : 7/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __BLOODSPLATSFX_H__
#define __BLOODSPLATSFX_H__

#include "SpecialFX.h"
#include "dlink.h"

struct BSCREATESTRUCT : public SFXCREATESTRUCT
{
	BSCREATESTRUCT::BSCREATESTRUCT();

	DVector		m_Pos;
	DRotation	m_Rotation;
	DFLOAT		m_fScale;
	DFLOAT		m_fGrowScale;
	HSTRING		m_hstrSprite;
};

inline BSCREATESTRUCT::BSCREATESTRUCT()
{
	memset(this, 0, sizeof(BSCREATESTRUCT));
}

//extern DList g_BloodSplatList;

class CBloodSplatFX : public CSpecialFX
{
	public :

		CBloodSplatFX() : CSpecialFX()
		{
			VEC_INIT( m_vPos );
			ROT_INIT( m_Rotation );
			m_hstrSprite = DNULL;
			VEC_INIT(m_vScale);
			m_fGrowScale = 0.0f;

			if( m_BloodSplatList.m_nElements == 0 )
				dl_InitList( &m_BloodSplatList );

			dl_AddTail( &m_BloodSplatList, &m_BloodSplatLink, this );
		}

		~CBloodSplatFX()
		{
			if( m_hstrSprite && m_pClientDE )
				m_pClientDE->FreeString( m_hstrSprite );

			dl_RemoveAt( &m_BloodSplatList, &m_BloodSplatLink );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		DBOOL		  Term();
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		HOBJECT	GetHandle()		{return m_hObject;}

	private :

		DVector		m_vPos;
		DRotation	m_Rotation;
		DVector		m_vForward;
		HSTRING		m_hstrSprite;
		DFLOAT		m_fGrowScale;
		DVector		m_vScale;

		DBOOL		m_bShrink;
		DFLOAT		m_fGrowTime;

		static DList m_BloodSplatList;
		DLink		m_BloodSplatLink;
};

#endif // __BLOODSPLATSFX_H__
