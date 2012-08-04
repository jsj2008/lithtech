 // ----------------------------------------------------------------------- //
//
// MODULE  : BulletImpactSFX.h
//
// PURPOSE : BulletImpact special fx class - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __BULLETIMPACTSFX_H__
#define __BULLETIMPACTSFX_H__

#include "SpecialFX.h"
#include "MarkSFX.h"
#include "dlink.h"


struct BULLETIMPACTCREATESTRUCT : public SFXCREATESTRUCT
{
	BULLETIMPACTCREATESTRUCT::BULLETIMPACTCREATESTRUCT();

	DRotation	m_Rotation;
	DFLOAT		m_fScale;
	HSTRING		m_hstrMarkSprite;
	HSTRING		m_hstrSmokeSprite;
};

inline BULLETIMPACTCREATESTRUCT::BULLETIMPACTCREATESTRUCT()
{
	memset(this, 0, sizeof(BULLETIMPACTCREATESTRUCT));
}


class CBulletImpactSFX : public CSpecialFX
{
	public :

		CBulletImpactSFX()
		{
			ROT_INIT( m_Rotation );
			m_fScale = 1.0f;
			m_hstrMarkSprite = DNULL;
		}

		~CBulletImpactSFX()
		{
			if( m_hstrMarkSprite && m_pClientDE )
				m_pClientDE->FreeString( m_hstrMarkSprite );
			if( m_hstrSmokeSprite && m_pClientDE )
				m_pClientDE->FreeString( m_hstrSmokeSprite );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	private :

		DRotation	m_Rotation;
		DFLOAT		m_fScale;
		HSTRING		m_hstrMarkSprite;
		HSTRING		m_hstrSmokeSprite;
};

#endif // __MARKSFX_H__