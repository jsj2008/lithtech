//----------------------------------------------------------
//
// MODULE  : ShellCasingFX.h
//
// PURPOSE : Ejected shell special effect
//
// CREATED : 9/10/98
//
//----------------------------------------------------------

#ifndef __SHELLCASINGFX_H__
#define __SHELLCASINGFX_H__


#include "SpecialFX.h"
#include "client_physics.h"


struct SHELLCREATESTRUCT : public SFXCREATESTRUCT
{
	SHELLCREATESTRUCT::SHELLCREATESTRUCT();

	DRotation	rRot;
	DVector		vStartPos;
	DBOOL		bLeftHanded;
	DBYTE		nAmmoType;
};

inline SHELLCREATESTRUCT::SHELLCREATESTRUCT()
{
	memset(this, 0, sizeof(SHELLCREATESTRUCT));
}

class CShellCasingFX : public CSpecialFX
{
	public :

		CShellCasingFX();

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	private:

		DBOOL		GetFileNames(char **pModelName, char **pSkinName);

		DBOOL		m_bLeftHanded;
		DFLOAT		m_fExpireTime;
		DVector		m_vLastPos;
		
		DFLOAT		m_fPitchVel;
		DFLOAT		m_fYawVel;
		DFLOAT		m_fPitch;
		DFLOAT		m_fYaw;

        DBOOL       m_bInVisible;
        int         m_nVisibleUpdate;
		int			m_nBounceCount;        

		DRotation	m_rRot;
		DVector		m_vStartPos;
		DBYTE		m_nAmmoType;
		DBOOL		m_bResting;

		DFLOAT		m_fDieTime;
		DVector		m_vScale;

		MovingObject m_movingObj;
};


#endif  // __SHELLCASING_FX_H__