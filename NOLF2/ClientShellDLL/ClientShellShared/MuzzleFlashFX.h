// ----------------------------------------------------------------------- //
//
// MODULE  : MuzzleFlashFX.h
//
// PURPOSE : MuzzleFlash special fx class - Definition
//
// CREATED : 12/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MUZZLE_FLASH_FX_H__
#define __MUZZLE_FLASH_FX_H__

#include "SpecialFX.h"
#include "WeaponMgr.h"
#include "MuzzleFlashParticleFX.h"
#include "DynamicLightFX.h"
#include "BaseScaleFX.h"

struct MUZZLEFLASHCREATESTRUCT : public SFXCREATESTRUCT
{
    MUZZLEFLASHCREATESTRUCT();

	WEAPON const*	pWeapon;		// Weapon data
	HOBJECT			hParent;		// Character or Player model if bPlayerView = TRUE
    LTVector		vPos;			// Initial pos
    LTRotation		rRot;			// Initial rotation
    LTBOOL			bPlayerView;	// Is this fx tied to the client-side player view
};

inline MUZZLEFLASHCREATESTRUCT::MUZZLEFLASHCREATESTRUCT()
{
    rRot.Init();
	vPos.Init();
    hParent     = LTNULL;
    pWeapon     = LTNULL;
    bPlayerView = LTFALSE;
}


class CMuzzleFlashFX : public CSpecialFX
{
	public :

		CMuzzleFlashFX() : CSpecialFX()
		{
            m_bUsingParticles   = LTFALSE;
            m_bUsingScale       = LTFALSE;
            m_bUsingLight       = LTFALSE;
            m_bHidden           = LTFALSE;
		}

		void Term();

        LTBOOL Setup(MUZZLEFLASHCREATESTRUCT & cs);

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		void Hide();
		void Show();
        void SetPos(const LTVector &vWorldPos, const LTVector &vCamRelPos);
        void SetRot(const LTRotation &rRot);

		virtual uint32 GetSFXID() { return SFX_MUZZLEFLASH_ID; }
	private :

        LTBOOL   Reset(MUZZLEFLASHCREATESTRUCT & cs);
        LTBOOL   ResetFX();
		void	 ReallyHide();


		MUZZLEFLASHCREATESTRUCT		m_cs;		// Our data

		CMuzzleFlashParticleFX		m_Particle; // Particle system
		CBaseScaleFX				m_Scale;	// Model/Sprite
		CDynamicLightFX				m_Light;	// Dynamic light

        LTBOOL                       m_bUsingParticles;
        LTBOOL                       m_bUsingScale;
        LTBOOL                       m_bUsingLight;
        LTBOOL                       m_bHidden;
};

#endif // __MUZZLE_FLASH_FX_H__