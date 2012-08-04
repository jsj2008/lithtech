//----------------------------------------------------------
//
// MODULE  : SHELLCASINGFX.H
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 5/1/98
//
//----------------------------------------------------------

#ifndef __SHELLCASING_FX_H__
#define __SHELLCASING_FX_H__

#include "SpecialFX.h"
#include "ltlink.h"
#include "client_physics.h"


struct SHELLCREATESTRUCT : public SFXCREATESTRUCT
{
    SHELLCREATESTRUCT();

    LTRotation   rRot;
    LTVector     vStartPos;
    LTVector     vStartVel;
    uint8       nWeaponId;
    uint8       nAmmoId;
    uint32      dwFlags;
    LTBOOL       b3rdPerson;
};

inline SHELLCREATESTRUCT::SHELLCREATESTRUCT()
{
    rRot.Init();
	vStartPos.Init();
	vStartVel.Init();
	nWeaponId	= 0;
	nAmmoId		= 0;
	dwFlags		= 0;
    b3rdPerson  = LTFALSE;
}

class CShellCasingFX : public CSpecialFX
{
	public :

		CShellCasingFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_SHELLCASING_ID; }

	private:

        LTFLOAT      m_fExpireTime;
        LTVector     m_vLastPos;
        LTVector     m_vStartVel;

        LTFLOAT      m_fPitchVel;
        LTFLOAT      m_fYawVel;
        LTFLOAT      m_fPitch;
        LTFLOAT      m_fYaw;

        LTBOOL       m_bInVisible;
        int         m_nVisibleUpdate;
		int			m_nBounceCount;

        LTRotation   m_rRot;
        LTVector     m_vStartPos;
        uint8       m_nWeaponId;
        uint8       m_nAmmoId;
        LTBOOL       m_bResting;
        uint32      m_dwFlags;
        LTBOOL       m_b3rdPerson;

        LTFLOAT      m_fDieTime;
        LTVector     m_vScale;

		MovingObject m_movingObj;
};


#endif  // __SHELLCASING_FX_H__