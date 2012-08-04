// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "SurfaceDB.h"
#include "Destructible.h"
#include "ltobjref.h"
#include "EventCaster.h"
#include "WeaponPath.h"

LINKTO_MODULE( Projectile );

struct WeaponFireInfo;
class CCharacter;
class CWeapon;
class GameClientData;

class CProjectile : public GameBase
{
	public :

		DEFINE_CAST( CProjectile );

		CProjectile();
		virtual ~CProjectile() {}

		virtual bool		Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);
		virtual bool		Setup( HWEAPON hWeapon, HAMMO hAmmo, WeaponFireInfo const &wfi );
		virtual bool		Setup( HAMMO hAmmo, LTRigidTransform const &trans );

		void				SetLifeTime(float fTime) { m_fLifeTime = fTime; }

		void				AdjustDamage(float fModifier);

		HOBJECT				GetFiredFrom()	const	{ return m_Shared.m_hFiredFrom; }
		HOBJECT				GetFiringWeapon()	const	{ return m_hFiringWeapon; }
		const LTVector&		GetFirePos()	const	{ return m_vFirePos; }
		const LTVector&		GetFireDir()	const	{ return m_vDir; }

		void			SetTeamId( uint8 nTeamId )	{ m_Shared.m_nTeamId = nTeamId; }
		uint8			GetTeamId( )		const	{ return m_Shared.m_nTeamId; }
		bool			IsMyTeam( HOBJECT hPlayer )	const;

		// damage filtering
		virtual bool        FilterDamage( DamageStruct *pDamageStruct );

		void ClearImpactPoints( ) { m_lstImpactPoints.clear( );	}
		
		// Notify clients of a vector weapon being fired...
		void SendVectorWeaponFireMsg( );

		// External control for killing projectiles...
		void Kill() { RemoveObject(); }

	protected :

		// message receiving functions
		uint32			EngineMessageFn(uint32 messageID, void *pData, float lData);
		uint32			ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		// handles any time a projectile touches something
		virtual void	HandleTouch(HOBJECT hObj);

		virtual void	AddImpact(	HOBJECT hObj,
									const LTVector &vFirePos, 
									const LTVector &vImpactPos, 
									const LTVector &vNormal, 
									SurfaceType eSurfaceType=ST_UNKNOWN,
									HMODELNODE hNodeHit = INVALID_MODEL_NODE );
		
		virtual void	AddExplosion(const LTVector &vPos, const LTVector &vNormal);

		virtual void	AddSpecialFX();
		virtual void	RemoveObject();
		virtual void    OverrideClientFX(const char* szEffect,const char* szSameTeamEffect,const char* szOtherTeamEffect);
		virtual void    SetRecoverable(bool bRecoverable);


		virtual void	HandleImpact(HOBJECT hObj);
		virtual void	ImpactDamageObject( HOBJECT hDamager, 
											HOBJECT hObj, 
											const LTVector &vFirePos,
											const LTVector &vImpactPos,
											const LTVector &vDirection );

		virtual void	HandleTouchSky(HOBJECT hObj);

		virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		virtual void	SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags );
		virtual void	LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags );

		virtual void	Detonate(HOBJECT hObj);

		virtual void	HandleModelString( ArgList *pArgList ) {};

		virtual EPhysicsGroup GetPhysicsGroup( ) const { return ePhysicsGroup_NonSolid; }

		virtual void	HandlePlayerChange();

		LTVector		m_vFlashPos;			// Where the fired from special fx should be created
		LTVector		m_vFirePos;				// Where were we fired from
		LTVector		m_vDir;					// What direction our we moving

		// special conditions
		bool			m_bSilenced;			// This weapon silenced?

		// state of object
		bool			m_bObjectRemoved;		// Have we been removed?
		bool			m_bDetonated;			// Have we detonated yet?
		bool			m_bRecoverable;

		// weapon/ammo ids
		bool			m_bSendWeaponRecord;	// Should the weapon record be sent to clients?
		bool			m_bSendAmmoRecord;		// Should the ammo record be sent to clients?
		bool			m_bLeftHandWeapon;		// which weapon fired

		bool			m_bGuaranteedHit;		// This is set if the impact has already been verified as hitting.

		bool			m_bSetup;				// Has this projectile been setup properly?
		bool			m_bRemoveFromWorld;		// Should we be removed?

		// touch considerations
		bool			m_bCanHitSameProjectileKind;	// ability to hit same kind of projectlies
		bool			m_bCanTouchFiredFromObj;		// ability to touch the object that fired us
		bool			m_bDamagedByOwner;

		// damage
		DamageType		m_eInstDamageType;		// Instant damage type - DT_XXX
		DamageType		m_eProgDamageType;		// Progressive damage type - DT_XXX
		float			m_fInstDamage;			// How much instant damage do we do
		float			m_fProgDamage;			// How much progressive damage do we do
		float			m_fInstPenetration;		// What percentage instant damage gets through target's armor

		float			m_fRadius;				// What is our damage radius
		double			m_fStartTime;			// When did we start
		float			m_fLifeTime;			// How long do we stay around
		float			m_fVelocity;			// What is our velocity
		float			m_fMass;				// What is our mass
		float			m_fRange;				// Vector weapon range

		// Declare our delegate to receive removeclient events.
		static void OnRemoveClient( CProjectile* pProjectile, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams );
		Delegate< CProjectile, GameClientData, CProjectile::OnRemoveClient > m_delegateRemoveClient;

		// Declare our delegate to receive playerswitched events.
		static void OnPlayerSwitched( CProjectile* pProjectile, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams );
		Delegate< CProjectile, GameClientData, CProjectile::OnPlayerSwitched > m_delegatePlayerSwitched;

		LTObjRef		m_hFiringWeapon;		// The weapon object that fired us.

		// for future optimization?
		bool			m_bNumCallsToAddImpact;


		// These are used to move through invisible brushes...

		bool			m_bProcessInvImpact;
		LTVector		m_vInvisVel;
		LTVector		m_vInvisNewPos;

		PROJECTILECREATESTRUCT	m_Shared;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables

		CDestructible	m_damage;				// Handle damage

		LTVector		m_vDims;				// Model dimensions

		uint32			m_dwFlags;				// Model flags
		DamageFlags		m_nCantDamageFlags;		// What type of damage can't damage us...

		// the number of times this projectile has ricocheted
		int				m_nTotalRicochets;

		// MID_TOUCHNOTIFY can come several times a frame, this is one
		// way to detect this and avoid doing calculations multiple times
		int				m_nUpdateNum;
		int				m_nLastRicochetUpdateNum;

		HMODELNODE		m_hNodeHit;

		typedef std::vector<LTVector, LTAllocator<LTVector, LT_MEM_TYPE_OBJECTSHELL> > TVectorList;
		TVectorList		m_lstImpactPoints;
		
	private :

		// update functions
		void	InitialUpdate(int nInfo);
		void	Update();

		void	DoProjectile();
		void	DoVector( WeaponFireInfo const & info );

		// Updates the heatseeking calculations.
		void UpdateHeatSeeking( );

		bool	TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType);

		void	SetFiredFrom( HOBJECT hFiredFrom );

		// WeaponPath callbacks for impacts and character hits...
		static bool WeaponPath_OnImpactCB( CWeaponPath::COnImpactCBData &rImpactData, void *pUserData );
		static bool WeaponPath_OnCharNodeHitCB( HOBJECT hCharacter, ModelsDB::HNODE hModelNode, float &rfDamageModifier );
		static bool WeaponPath_IntersectSegment( CWeaponPath::CIntersectSegmentData &rISData );

		static void	RegisterImpactAIStimuli( HAMMO hAmmo, 
											 HOBJECT hFiredFrom, 
											 HOBJECT hObjectHit,
											 const LTVector& vImpactPos, 
											 const LTVector& vImpactDir,
											 SurfaceType eSurfaceType );
};


#endif //  __PROJECTILE_H__
