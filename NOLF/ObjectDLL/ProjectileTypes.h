// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.h
//
// PURPOSE : ProjectileTypes class - definition
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_TYPES_H__
#define __PROJECTILE_TYPES_H__

#include "Projectile.h"
#include "Timer.h"

extern CTList<class CGrenade*> g_lstGrenades;

class CGrenade : public CProjectile
{
	public :

		CGrenade();
		~CGrenade();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void HandleImpact(HOBJECT hObj);
		virtual void UpdateGrenade();
        virtual void ResetRotationVel(LTVector* pvNewVel=LTNULL, SURFACE* pSurf=LTNULL);
		virtual void RotateToRest();
		virtual void Detonate(HOBJECT hObj);

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

        LTBOOL   m_bSpinGrenade;     // Should the grenade spin
        LTBOOL   m_bUpdating;        // Are we updating the grenade

		ContainerCode m_eContainerCode;
		SurfaceType	  m_eLastHitSurface;

        LTFLOAT  m_fPitch;
        LTFLOAT  m_fYaw;
        LTFLOAT  m_fRoll;
        LTFLOAT  m_fPitchVel;
        LTFLOAT  m_fYawVel;
        LTFLOAT  m_fRollVel;
		LTBOOL	 m_bRotatedToRest;
		uint32	 m_cBounces;

		// Don't need to save this...

        HLTSOUND    m_hBounceSnd;   // Handle to current bounce sound
};


class CLipstickProx : public CGrenade
{
	public :

		CLipstickProx() : CGrenade()
		{
			m_vSurfaceNormal.Init();
            m_bArmed     = LTFALSE;
            m_bActivated = LTFALSE;
            m_pClassData = LTNULL;
		}

		virtual void Setup(CWeapon* pWeapon, WFireInfo & info);

	protected :

		virtual void UpdateGrenade();
		virtual void HandleImpact(HOBJECT hObj);
		virtual void RotateToRest();

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTVector m_vSurfaceNormal;
        LTBOOL   m_bArmed;
        LTBOOL   m_bActivated;
		CTimer	m_DetonateTime;
		CTimer  m_ArmTime;

		PROXCLASSDATA* m_pClassData;
};


class CLipstickImpact : public CGrenade
{
	public :

		CLipstickImpact() : CGrenade() {}

	protected :

		virtual void HandleImpact(HOBJECT hObj)
		{
			CProjectile::HandleImpact(hObj);
		}
};

class CLipstickTimed : public CGrenade
{
	public :

		CLipstickTimed() : CGrenade() {}
};

class CSpear : public CProjectile
{
	public :

		CSpear();
		~CSpear();

		virtual LTBOOL CanTestImpact() const { return LTTRUE; }

	protected :

		virtual void HandleImpact(HOBJECT hObj);
};

class CCoin : public CGrenade
{
	public :

	protected :

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

		virtual LTBOOL CanSetLastFireInfo() { return LTFALSE; }
		virtual void RotateToRest();
};

#endif //  __PROJECTILE_TYPES_H__