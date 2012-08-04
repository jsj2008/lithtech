// ----------------------------------------------------------------------- //
//
// MODULE  : SHELLCASINGFX.H
//
// PURPOSE : defines class for ejected shells
//
// CREATED : 5/1/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHELLCASING_FX_H__
#define __SHELLCASING_FX_H__

#include "SpecialFX.h"
#include "ltlink.h"
#include "client_physics.h"


struct SHELLCREATESTRUCT : public SFXCREATESTRUCT
{
	SHELLCREATESTRUCT();

	LTRotation	rRot;
	LTVector	vStartPos;
	LTVector	vStartVel;
	HWEAPON		hWeapon;
	HAMMO		hAmmo;
	uint32		dwFlags;
	bool		b3rdPerson;
};

inline SHELLCREATESTRUCT::SHELLCREATESTRUCT()
{
	rRot.Init();
	vStartPos.Init();
	vStartVel.Init();
	hWeapon		= NULL;
	hAmmo		= NULL;
	dwFlags		= 0;
	b3rdPerson  = false;
}

class CShellCasingFX : public CSpecialFX
{
	public :

		CShellCasingFX();

		virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool Update();
		virtual bool CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_SHELLCASING_ID; }

	private:

		bool BounceMovingObject(MovingObject *pObject, LTVector &vNewPos, 
								IntersectInfo* pInfo, bool & bBounceOnGround);


		LTVector	m_vStartVel;

		float		m_fPitchVel;
		float		m_fYawVel;

		LTRotation	m_rRot;
		LTVector	m_vStartPos;
		HWEAPON		m_hWeapon;
		HAMMO		m_hAmmo;
		uint32		m_dwFlags;
		uint16		m_nBounceCount;
		bool		m_bResting;
		bool		m_b3rdPerson;

		float		m_fModelDims;
		float		m_fDieTime;
		float		m_fElapsedTime;
		float		m_fInitialScale;
		float		m_fFinalScale;

		MovingObject m_movingObj;
};


#endif  // __SHELLCASING_FX_H__