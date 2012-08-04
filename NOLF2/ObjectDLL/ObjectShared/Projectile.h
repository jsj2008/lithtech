// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "LTObjRef.h"
#include "ImpactType.h"

LINKTO_MODULE( Projectile );

struct WeaponFireInfo;
class CCharacter;
class CWeapon;

class CProjectile : public GameBase
{
	public :

		CProjectile();
		virtual ~CProjectile() {}

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);
        void			SetLifeTime(LTFLOAT fTime) { m_fLifeTime = fTime; }

		void			AdjustDamage(LTFLOAT fModifier);

		HOBJECT          GetFiredFrom() const   { return m_hFiredFrom; }
		const LTVector&  GetFirePos() const     { return m_vFirePos; }
		const LTVector&  GetFireDir() const     { return m_vDir; }

		void			SetTeamId( uint8 nTeamId ) { m_nTeamId = nTeamId; }
		uint8			GetTeamId( ) const { return m_nTeamId; }
		bool			IsMyTeam( HOBJECT hPlayer ) const;

		virtual LTBOOL	CreateDangerStimulus(HOBJECT hFiredFrom) { return LTFALSE; }

	protected :

		// message receiving functions
		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		// handles any time a projectile touches something
		virtual void HandleTouch(HOBJECT hObj);

		virtual void AddImpact( HOBJECT hObj,
		                        const LTVector &vFirePos, 
		                        const LTVector &vImpactPos, 
		                        const LTVector &vNormal, 
		                        SurfaceType eSurfaceType=ST_UNKNOWN,
		                        IMPACT_TYPE eImpactType = IMPACT_TYPE_IMPACT,
								bool bDeflected = false);
		virtual void AddExplosion(const LTVector &vPos, const LTVector &vNormal);

		virtual void AddSpecialFX();
		virtual void RemoveObject();
		
				void CreateSpecialFX( );

		virtual void HandleImpact(HOBJECT hObj);
		virtual void ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj);

		virtual void HandleTouchSky(HOBJECT hObj);

		virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual LTBOOL CanSetLastFireInfo() { return LTTRUE; }
		virtual void Detonate(HOBJECT hObj);

		// Check if we got deflected.
		bool	Deflect( HOBJECT hTarget, const LTVector &vFirePos, const LTVector &vImpactPos, const LTVector &vNormal );

		LTVector        m_vFlashPos;            // Where the fired from special fx should be created
		LTVector        m_vFirePos;             // Where were we fired from
		LTVector        m_vDir;                 // What direction our we moving

		// special conditions
		LTBOOL          m_bSilenced;            // This weapon silenced?

		// state of object
		LTBOOL          m_bObjectRemoved;       // Have we been removed?
		LTBOOL          m_bDetonated;           // Have we detonated yet?

		// weapon/ammo ids
		int             m_nWeaponId;            // Type of weapon
		int             m_nAmmoId;              // Type of ammo

        LTBOOL			m_bSetup;				// Has this projectile been setup properly?
        LTBOOL          m_bRemoveFromWorld;     // Should we be removed?

		// touch considerations
		LTBOOL          m_bCanHitSameProjectileKind;  // ability to hit same kind of projectlies
		LTBOOL			m_bCanTouchFiredFromObj;	// ability to touch the object that fired us

		// damage
		DamageType      m_eInstDamageType;      // Instant damage type - DT_XXX
		DamageType      m_eProgDamageType;      // Progressive damage type - DT_XXX
		LTFLOAT         m_fInstDamage;          // How much instant damage do we do
		LTFLOAT         m_fProgDamage;          // How much progressive damage do we do

		LTFLOAT         m_fRadius;              // What is our damage radius
		LTFLOAT         m_fStartTime;           // When did we start
		LTFLOAT         m_fLifeTime;            // How long do we stay around
		LTFLOAT         m_fVelocity;            // What is our velocity
		LTFLOAT         m_fMass;                // What is our mass
		LTFLOAT         m_fRange;               // Vector weapon range

		LTObjRef		m_hFiredFrom;			// Who fired us

		uint8			m_nTeamId;				// The team the projectile is associated with.

		// for future optimization?
		LTBOOL          m_bNumCallsToAddImpact;


		// These are used to move through invisible brushes...

		LTBOOL          m_bProcessInvImpact;
		LTVector        m_vInvisVel;
		LTVector        m_vInvisNewPos;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables

		CDestructible   m_damage;               // Handle damage

		LTVector        m_vDims;                // Model dimensions

		uint32          m_dwFlags;              // Model flags
		DamageFlags     m_nCantDamageFlags;    // What type of damage can't damage us...

		WEAPON const   *m_pWeaponData;
		AMMO const     *m_pAmmoData;

		// the number of times this projectile has ricocheted
		int             m_nTotalRicochets;

		// MID_TOUCHNOTIFY can come several times a frame, this is one
		// way to detect this and avoid doing calculations multiple times
		int             m_nUpdateNum;
		int             m_nLastRicochetUpdateNum;

		// Client-side hit detection support
		LTVector		m_vClientObjImpactPos;
		HOBJECT			m_hClientObjImpact;

	private :

		// update functions
		void InitialUpdate(int nInfo);
		void Update();

		// 
		void DoProjectile();
		void DoVector();

		LTBOOL TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType);

		LTBOOL HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom, LTVector & vTo);
		LTBOOL HandlePotentialHitBoxImpact(IntersectInfo & iInfo, LTVector & vFrom);

		LTBOOL UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness, LTVector vImpactPos,
		                            LTVector & vFrom, LTVector & vTo);

		LTBOOL CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType);

		LTBOOL HandleClientVectorImpact();
};


#endif //  __PROJECTILE_H__
