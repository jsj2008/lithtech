// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "Destructible.h"
#include "ModelButeMgr.h"
#include "Weapon.h"

class CCharacter;

class CProjectile : public GameBase
{
	public :

		CProjectile();

		virtual void	Setup(CWeapon* pWeapon, WFireInfo & info);
        void			SetLifeTime(LTFLOAT fTime) { m_fLifeTime = fTime; }

        void			AdjustDamage(LTFLOAT fModifier) { m_fInstDamage *= fModifier; }
        LTFLOAT			GetInstDamage() { return m_fInstDamage; }

		HOBJECT			GetFiredFrom() const { return m_hFiredFrom; }
		const LTVector&	GetFirePos() const { return m_vFirePos; }
		const LTVector& GetFireDir() const { return m_vDir; }

		virtual LTBOOL	CanTestImpact() const { return LTTRUE; /*LTFALSE;*/ }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void HandleTouch(HOBJECT hObj);

        virtual void AddImpact(HOBJECT hObj, LTVector vFirePos, LTVector vImpactPos, LTVector vNormal, SurfaceType eSurfaceType=ST_UNKNOWN);
        virtual void AddExplosion(LTVector vPos, LTVector vNormal);

		virtual void AddSpecialFX();
		virtual void RemoveObject();

		virtual void HandleImpact(HOBJECT hObj);
		virtual void ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj);

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		virtual LTBOOL CanSetLastFireInfo() { return LTTRUE; }
		virtual void Detonate(HOBJECT hObj);

        LTVector         m_vFlashPos;            // Where the fired from special fx should be created
        LTVector         m_vFirePos;             // Where were we fired from
        LTVector         m_vDir;                 // What direction our we moving

        LTBOOL           m_bSilenced;            // This weapon silenced?
        LTBOOL           m_bRemoveFromWorld;     // Should we be removed?
        LTBOOL           m_bObjectRemoved;       // Have we been removed?
        LTBOOL           m_bDetonated;           // Have we detonated yet?

		int				m_nWeaponId;			// Type of weapon
		int				m_nAmmoId;				// Type of ammo
		AmmoType		m_eType;				// Type of ammo
		DamageType		m_eInstDamageType;		// Instant damage type - DT_XXX
		DamageType		m_eProgDamageType;		// Progressive damage type - DT_XXX

        LTFLOAT			m_fRadius;              // What is our damage radius
        LTFLOAT         m_fStartTime;           // When did we start
        LTFLOAT         m_fInstDamage;          // How much instant damage do we do
        LTFLOAT         m_fProgDamage;          // How much progressive damage do we do
        LTFLOAT         m_fLifeTime;            // How long do we stay around
        LTFLOAT         m_fExplosionDuration;   // How long explosion lasts
        LTFLOAT         m_fVelocity;            // What is our velocity
        LTFLOAT         m_fMass;                // What is our mass
        LTFLOAT         m_fRange;               // Vector weapon range

		HOBJECT			m_hFiredFrom;			// Who fired us

		LTBOOL			m_bNumCallsToAddImpact;


		// These are used to move through invisible brushes...

		LTBOOL			m_bProcessInvImpact;
		LTVector		m_vInvisVel;
		LTVector		m_vInvisNewPos;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables

		CDestructible	m_damage;				// Handle damage

        LTVector         m_vDims;                // Model dimensions

        uint32          m_dwFlags;              // Model flags
        uint32          m_dwCantDamageTypes;    // What type of damage can't damage us...

		WEAPON*			m_pWeaponData;
		AMMO*			m_pAmmoData;

	private :

		void InitialUpdate(int nInfo);
		void Update();

		void DoProjectile();
		void DoVector();

        LTBOOL TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType);

        LTBOOL HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom, LTVector & vTo);
        LTBOOL HandlePotentialHitBoxImpact(IntersectInfo & iInfo, LTVector & vFrom);
        LTBOOL DoLocationBasedImpact(IntersectInfo & iInfo, LTVector & vFrom,
									ModelSkeleton eModelSkeleton, ModelNode& eModelNode);

        LTBOOL UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness, LTVector vImpactPos,
                                   LTVector & vFrom, LTVector & vTo);

        LTBOOL CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType);
};


#endif //  __PROJECTILE_H__