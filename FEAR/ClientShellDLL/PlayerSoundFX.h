// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerSoundFX.h
//
// PURPOSE : Player Sound fx - Definition
//
// CREATED : 10/28/98 (was WeaponSoundFX)
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_SOUND_FX_H__
#define __PLAYER_SOUND_FX_H__

#include "SpecialFX.h"

struct PLAYERSOUNDCREATESTRUCT : public SFXCREATESTRUCT
{
	PLAYERSOUNDCREATESTRUCT();

	LTVector	vPos;
	uint8		nClientId;
	uint8		nType;
	HWEAPON		hWeapon;
};

inline PLAYERSOUNDCREATESTRUCT::PLAYERSOUNDCREATESTRUCT()
{
	vPos.Init();
	nClientId	= 0;
	nType		= 0;
	hWeapon		= NULL;
}

class CPlayerSoundFX : public CSpecialFX
{
	public :

		CPlayerSoundFX() : CSpecialFX()
		{
			m_vPos.Init();
			m_nClientId	= 0;
			m_nType		= 0;
			m_hWeapon	= NULL;
		}

		~CPlayerSoundFX()
		{
		}

		virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool CreateObject(ILTClient* pClientDE);
		virtual bool Update() { return false; }

		virtual uint32 GetSFXID() { return SFX_PLAYERSOUND_ID; }

	private :

		LTVector	m_vPos;
		uint8		m_nClientId;
		uint8		m_nType;
		HWEAPON		m_hWeapon;
};

#endif // __PLAYER_SOUND_FX_H__