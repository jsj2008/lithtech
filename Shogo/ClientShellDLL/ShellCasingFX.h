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
	SHELLCREATESTRUCT::SHELLCREATESTRUCT();

	LTRotation	rRot;
	LTVector		vStartPos;
	uint8		nWeaponId;
};

inline SHELLCREATESTRUCT::SHELLCREATESTRUCT()
{
	memset(this, 0, sizeof(SHELLCREATESTRUCT));
}

class CShellCasingFX : public CSpecialFX
{
	public :

		CShellCasingFX();

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	private:

		LTFLOAT		m_fExpireTime;
		LTVector		m_vLastPos;
		
		LTFLOAT		m_fPitchVel;
		LTFLOAT		m_fYawVel;
		LTFLOAT		m_fPitch;
		LTFLOAT		m_fYaw;

        LTBOOL       m_bInVisible;
        int         m_nVisibleUpdate;
		int			m_nBounceCount;        

		LTRotation	m_rRot;
		LTVector		m_vStartPos;
		uint8		m_nWeaponId;
		LTBOOL		m_bResting;

		LTFLOAT		m_fDieTime;
		LTVector		m_vScale;

		MovingObject m_movingObj;

		char* GetModelName();
		char* GetSkinName();
};


#endif  // __SHELLCASING_FX_H__