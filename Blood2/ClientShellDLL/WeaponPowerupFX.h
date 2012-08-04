// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPowerupFX.h
//
// PURPOSE : Weapon Powerup special fx class - Definition
//
// CREATED : 7/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONPOWERUP_FX_H__
#define __WEAPONPOWERUP_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct WEAPPOWERCREATESTRUCT : public SFXCREATESTRUCT
{
	WEAPPOWERCREATESTRUCT::WEAPPOWERCREATESTRUCT();

	HLOCALOBJ	hGun;
	DVector		vPosOffset;
	DVector		vScale;
	DFLOAT		fLifeTime;
	DFLOAT		fInitAlpha;
	DBYTE		bFade;
	HSTRING		pSpriteFile;
};

inline WEAPPOWERCREATESTRUCT::WEAPPOWERCREATESTRUCT()
{
	memset(this, 0, sizeof(WEAPPOWERCREATESTRUCT));
}

// ----------------------------------------------------------------------- //

class CWeaponPowerupFX : public CSpecialFX
{
	public :

		CWeaponPowerupFX() : CSpecialFX() 
		{
			m_hGun			= DNULL;
			m_fLifeTime		= 0.0f;
			m_fInitAlpha	= 1.0f;
			m_fStartTime	= -1.0f;
		}
		~CWeaponPowerupFX()
		{
			g_pClientDE->FreeString( m_pSpriteFile );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	private :

		HLOCALOBJ	m_hGun;			// Object doing the firing
		DVector		m_vPosOffset;	// Offset of the powerup
		DVector		m_vScale;		// Scale of the sprite
		DFLOAT		m_fLifeTime;	// How long to display the sprite
		DFLOAT		m_fInitAlpha;	// Initial alpha value for the sprite
		DFLOAT		m_fStartTime;	// When did we start
		HSTRING		m_pSpriteFile;	// File to use for the sprite
		DBYTE		m_bFade;		// Fade type (not used currently)
};

#endif // __WEAPONPOWERUP_FX_H__