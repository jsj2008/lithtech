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

    LTVector    vPos;
    uint8       nClientId;
    uint8       nType;
    uint8       nWeaponId;
};

inline PLAYERSOUNDCREATESTRUCT::PLAYERSOUNDCREATESTRUCT()
{
	vPos.Init();
	nClientId	= 0;
	nType		= 0;
	nWeaponId	= 0;
}

class CPlayerSoundFX : public CSpecialFX
{
	public :

		CPlayerSoundFX() : CSpecialFX()
		{
			m_vPos.Init();
			m_nClientId	= 0;
			m_nType		= 0;
			m_nWeaponId	= 0;
		}

		~CPlayerSoundFX()
		{
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update() { return LTFALSE; }

		virtual uint32 GetSFXID() { return SFX_PLAYERSOUND_ID; }

	private :

        LTVector	m_vPos;
        uint8		m_nClientId;
        uint8		m_nType;
        uint8		m_nWeaponId;
};

#endif // __PLAYER_SOUND_FX_H__