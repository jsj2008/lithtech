// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STRATEGY_H__
#define __AI_HUMAN_STRATEGY_H__

#include "AIMovement.h"
#include "AnimationMgr.h"
#include "AIBrain.h"

class CCharacter;
class CAIHuman;
class CWeapon;
class CAINode;
class CAIVolume;
class CAIPath;
class CAIPathWaypoint;
class CAnimationContext;

// Classes

class CAIHumanStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHumanStrategy);

	public : // Public member variables

		typedef enum AIHumanStrategyType
		{
			eStrategyNone,
			eStrategyOneShotAni,
			eStrategyFollowPath,
			eStrategyDodge,
			eStrategyCoverDuck,
			eStrategyCover1WayCorner,
			eStrategyCover2WayCorner,
			eStrategyCoverBlind,
			eStrategyGrenadeThrow,
			eStrategyShootBurst,
			eStrategyShootBurstBlind,
			eStrategyFlashlight,
		};

	public :

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}

		// Updates

		virtual void Update() {}
		virtual void UpdateAnimation() {}

		// Simple accessors

		virtual AIHumanStrategyType GetType() { return eStrategyNone; }

	protected :

		// Simple accessors

		CAIHuman* GetAI() { return m_pAIHuman; }
		CAnimationContext* GetAnimationContext();

	protected :

		CAIHuman*			m_pAIHuman;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyOneShotAni
//
// PURPOSE : AI CheckingPulse - ability to ... to play a one shot ani
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyOneShotAni : DEFINE_FACTORY_CLASS(CAIHumanStrategyOneShotAni), public CAIHumanStrategy
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyOneShotAni);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Set(const CAnimationProp& Prop);
		void Reset() { m_eState = eUnset; m_Prop.Clear(); }

		void Update();
		void UpdateAnimation();

		// State

		LTBOOL IsUnset() { return m_eState == eUnset; }
		LTBOOL IsSet() { return m_eState == eSet; }
		LTBOOL IsAnimating() { return m_eState == eAnimating; }
		LTBOOL IsDone() { return m_eState == eDone; }

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyOneShotAni; }

	protected :

		enum State
		{
			eUnset,
			eSet,
			eAnimating,
			eDone,
		};

	protected :

		State			m_eState;
		CAnimationProp	m_Prop;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyFollowPath
//
// PURPOSE : AI Follow path ability - to walk a path of AINodes
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyFollowPath : DEFINE_FACTORY_CLASS(CAIHumanStrategyFollowPath), public CAIHumanStrategy
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyFollowPath);

	public :

		enum Urgency
		{
			eUrgencyCourteous,
			eUrgencyAggressive,
		};

		enum Medium
		{
			eMediumGround,
			eMediumUnderwater,
		};

	public :

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		LTBOOL Set(const LTVector& vDestination);
		LTBOOL Set(CAINode* pNodeDestination);
		LTBOOL Set(CAIVolume* pVolumeDestination);

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
		void HandleModelString(ArgList* pArgList);

		// Movement types

		void SetMovement(CAnimationProp aniMovement) { m_AIMovement.SetMovement(aniMovement); }
		const CAnimationProp& GetMovement() const { return m_AIMovement.GetMovement(); }
		void SetMovementModifier(const CAnimationProp& ani) { m_bModifiedMovement = LTTRUE; m_aniModifiedMovement = ani; }
		void ClearMovementModifier() { m_bModifiedMovement = LTFALSE; }

		// Urgency types

		void SetUrgency(Urgency eUrgency) { m_eUrgency = eUrgency; }

		// Medium types

		void SetMedium(Medium eMedium) { m_eMedium = eMedium; m_AIMovement.SetUnderwater(eMedium == eMediumUnderwater); }

		// Simple accessors

		AIHumanStrategyType GetType() { return CAIHumanStrategy::eStrategyFollowPath; }

		LTBOOL IsUnset() { return m_eState == eStateUnset; }
		LTBOOL IsSet() { return m_eState == eStateSet; }
		LTBOOL IsDone() { return m_eState == eStateDone; }

	protected :

		LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateFaceDoor(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateOpenDoors(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateWaitForDoors(CAIPathWaypoint* pWaypoint);

		void Reset();

	protected : // Protected enumerations

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

		enum DoorState
		{
			eDoorStateNone,
			eDoorStateWaitingForAnimationToStart,
			eDoorStateWaitingForModelString,
			eDoorStateWaitingForAnimationToFinish,
			eDoorStateWaitingForDoorToOpen,
		};

	protected :

		CAIPath*				m_pPath;
		State					m_eState;
		Urgency					m_eUrgency;
		Medium					m_eMedium;
		CAIMovementHuman		m_AIMovement;
		DoorState				m_eDoorState;
		CAnimationProp			m_aniModifiedMovement;
		LTBOOL					m_bModifiedMovement;
		LTFLOAT					m_fStuckOnDoorTimer;
		LTBOOL					m_bDoorShootThroughable;

		// HACK OF HACKS

		class CAIHumanStrategyShootBurst*	m_pStrategyShoot;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyDodge
//
// PURPOSE : AI ability to dodge
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyDodge : DEFINE_FACTORY_CLASS(CAIHumanStrategyDodge), public CAIHumanStrategy
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyDodge);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Update();
		void UpdateAnimation();

		// Dodge

		void Dodge();
		LTBOOL IsDodging() const { return m_eState == eStateDodging; }

		// Outcome

		DodgeStatus GetStatus() const { return m_eDodgeStatus; }
		DodgeAction GetAction() const { return m_eDodgeAction; }

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyDodge; }

	protected :

		// Update Check

		void UpdateCheck();

		// Update Dodge

		void UpdateDodge();
		void UpdateDodgeShuffle();
		void UpdateDodgeRoll();
		void UpdateDodgeDive();
		void UpdateDodgeFlee();
		void UpdateDodgeCover();

		// Check

		void Check();

		// Dodge

		void Shuffle();
		void Roll();
		void Dive();
		void Flee();
		void Cover();

		// H A C K

		LTFLOAT GetMovementData();

	protected :

		enum State
		{
			eStateChecking,
			eStateDodging,
		};

	protected :

		LTFLOAT		m_fCheckTimeVector;
		LTFLOAT		m_fCheckTimeProjectile;

		uint32		m_dwNode;

		State		m_eState;
		DodgeStatus	m_eDodgeStatus;
		DodgeAction	m_eDodgeAction;

		Direction	m_eDirection;
		LTVector	m_vDir;

		LTFLOAT		m_fAnimTimePrev;	// Hack for movement encoding
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCover
//
// PURPOSE : AI ability to use cover
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCover : public CAIHumanStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHumanStrategyCover);

	public :

		// Ctors/Dtors/etc

		virtual void Clear();

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Updates

		virtual void Update();
		virtual void UpdateAnimation();

		// Cover

		virtual void Cover(LTFLOAT fDelay = 0.0f);
		virtual void Uncover(LTFLOAT fDelay= 0.0f);

		// Simple accessors

		void SetCoverNode(CAINode *pCoverNode) { m_pCoverNode = pCoverNode; }
		void SetCoverTime(LTFLOAT fCoverTime, LTFLOAT fCoverTimeRandom = 0.0f) { m_fCoverTime = fCoverTime + GetRandom(0.0f, fCoverTimeRandom); }
		void SetUncoverTime(LTFLOAT fUncoverTime, LTFLOAT fUncoverTimeRandom = 0.0f) { m_fUncoverTime = fUncoverTime + GetRandom(0.0f, fUncoverTimeRandom); }

		LTBOOL IsCovered() { return m_eState == eCovered; }
		LTBOOL IsUncovered() { return m_eState == eUncovered; }
		LTBOOL IsCovering() { return m_eState == eCovering; }
		LTBOOL IsUncovering() { return m_eState == eUncovering; }

		virtual LTBOOL CanBlindFire() { return LTFALSE; }

		virtual AIHumanStrategyType GetType() { return eStrategyNone; }

	protected :

		void SwitchToCover();
		void SwitchToUncover();

		enum State
		{
			eCovered,
			eWillUncover,
			eUncovering,
			eUncovered,
			eWillCover,
			eCovering,
		};

	protected :

		State	m_eState;				// Our state

		LTBOOL	m_bWantCover;			// Has Cover been requested
		LTFLOAT	m_fCoverTimer;			// How long have we been Covered
		LTFLOAT	m_fCoverTime;			// How long should we stay Covered for
		LTFLOAT	m_fCoverDelay;			// When should we execute a Cover request

		LTBOOL	m_bWantUncover;			// Has Uncover been requested
		LTFLOAT	m_fUncoverTimer;		// How long have we been Uncovered
		LTFLOAT	m_fUncoverTime;			// How long should we stay Uncovered for
		LTFLOAT	m_fUncoverDelay;		// When should we execute a Uncover request

		// These don't need saving

		CAINode*	m_pCoverNode;	// Our cover node
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCoverDuck
//
// PURPOSE : AI cover ability that uses ducking
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCoverDuck : DEFINE_FACTORY_CLASS(CAIHumanStrategyCoverDuck), public CAIHumanStrategyCover
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyCoverDuck);

	public :

		// Updates

		void Update();
		void UpdateAnimation();

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyCoverDuck; }
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCoverBlind
//
// PURPOSE : AI cover ability that simply stays behind cover and blind fires
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCoverBlind : DEFINE_FACTORY_CLASS(CAIHumanStrategyCoverBlind), public CAIHumanStrategyCover
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyCoverBlind);

	public :

		// Updates

		void Update();
		void UpdateAnimation();

		// Simple accessors

		LTBOOL CanBlindFire() { return LTTRUE; }

		AIHumanStrategyType GetType() { return eStrategyCoverBlind; }
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCover1WayCorner
//
// PURPOSE : AI cover ability that uses Cornerping in one direction
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCover1WayCorner : DEFINE_FACTORY_CLASS(CAIHumanStrategyCover1WayCorner), public CAIHumanStrategyCover
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyCover1WayCorner);

	public :

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Update();
		void UpdateAnimation();

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyCover1WayCorner; }

	protected:

		LTFLOAT GetMovementData();

	protected :

		Direction	m_eDirection;
		LTVector	m_vDir;
		LTFLOAT		m_fAnimTimePrev;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCover2WayCorner
//
// PURPOSE : AI cover ability that uses Cornerping in one direction
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCover2WayCorner : DEFINE_FACTORY_CLASS(CAIHumanStrategyCover2WayCorner), public CAIHumanStrategyCover
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyCover2WayCorner);

	public :

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Update();
		void UpdateAnimation();

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyCover2WayCorner; }

	protected:

		LTFLOAT GetMovementData();

	protected :

		Direction	m_eDirection;
		LTVector	m_vDir;
		LTFLOAT		m_fAnimTimePrev;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyGrenade
//
// PURPOSE : AI ability to use Grenade
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyGrenade : public CAIHumanStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHumanStrategyGrenade);

	public :
		
		// Factory

		static CAIHumanStrategyGrenade* Create(AIHumanStrategyType eType);

	public :

		// Throw

		virtual void Throw(LTFLOAT fTime = 0.5f) { }
		virtual LTBOOL ShouldThrow() { return LTFALSE; }
		virtual LTBOOL IsThrowing() { return LTFALSE; }

		// Handlers

		virtual void HandleModelString(ArgList* pArgList) {}

		// Simple accessors

		virtual AIHumanStrategyType GetType() { return eStrategyNone; }

	protected :
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyGrenadeThrow
//
// PURPOSE : AI Grenade ability that throws the grenade
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyGrenadeThrow : DEFINE_FACTORY_CLASS(CAIHumanStrategyGrenadeThrow), public CAIHumanStrategyGrenade
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyGrenadeThrow);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Throw

		void Throw(LTFLOAT fTime = 0.5f);
		LTBOOL ShouldThrow();
		LTBOOL IsThrowing();

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyGrenadeThrow; }

	protected :

		enum State
		{
			eStateNone,
			eStateThrowing,
			eStateThrow,
			eStateThrown,
			eStateDone,
		};

	protected :

		State		m_eState;
		LTFLOAT		m_fHangtime;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyShoot
//
// PURPOSE : AI shoot ability - to fire at something
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyShoot : public CAIHumanStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHumanStrategyShoot);

	public :

		// Ctors/Dtors/etc

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Updates

        void Update(HOBJECT hTarget = LTNULL);
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Reloading

		LTBOOL IsReloading() const { return m_eState == eStateReloading; }
		void Reload(LTBOOL bInstant = LTFALSE);
		LTBOOL NeedsReload() { return m_bNeedsReload; }
		LTBOOL OutOfAmmo() { return m_bOutOfAmmo; }

		// FOV

		void SetIgnoreFOV(LTBOOL bIgnoreFOV) { m_bIgnoreFOV = bIgnoreFOV; }

		// Hack...

		void ClearFired() { m_bFired = LTFALSE; }

		// Simple accessors

		virtual AIHumanStrategyType GetType() = 0;

	protected :

		// Updates

		virtual void UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon) = 0;
		virtual void UpdateAiming(HOBJECT hTarget) = 0;

		void UpdateNeedsReload(CWeapon* pWeapon);
		void UpdateReloading(CWeapon* pWeapon);

		// Handlers

		virtual void HandleFired();

		// Aim/Fire

		void Aim();
		void Fire();

	protected :

		enum State
		{
			eStateNone,
			eStateAiming,
			eStateFiring,
			eStateReloading,
		};

	protected :

		State	m_eState;			// Our state
		LTBOOL	m_bFired;			// Did we fire?
		LTBOOL	m_bNeedsReload;		// Do we need to reload?
		LTBOOL	m_bOutOfAmmo;		// Are we out of ammo?
		LTBOOL	m_bFirstUpdate;		// Our first update?
		LTBOOL	m_bIgnoreFOV;		// Fire regardless of FOV
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyShootBurst
//
// PURPOSE : AI shoot ability - to burst fire at something
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyShootBurst : DEFINE_FACTORY_CLASS(CAIHumanStrategyShootBurst), public CAIHumanStrategyShoot
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyShootBurst);

	public :

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyShootBurst; }

	protected :

		// Updates

		void UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon);
		void UpdateAiming(HOBJECT hTarget);

		// Handlers

		void HandleFired();

		// Burst methods

		void CalculateBurst();

		// Blind fire?

		virtual LTBOOL IsBlind() const { return LTFALSE; }

	protected :

		LTFLOAT		m_fBurstInterval;		// How long between bursts (this is the timer)
		int			m_nBurstShots;			// How many shots in each burst (this is the counter)
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyShootBurstBlind
//
// PURPOSE : AI shoot ability - to burst fire blindly
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyShootBurstBlind : DEFINE_FACTORY_CLASS(CAIHumanStrategyShootBurstBlind), public CAIHumanStrategyShootBurst
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyShootBurstBlind);

	public :

		// Simple accessors

		AIHumanStrategyType GetType() { return eStrategyShootBurstBlind; }

	protected :

		LTBOOL IsBlind() const { return LTTRUE; }
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyFlashlight
//
// PURPOSE : AI strategy for using a flashlight
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyFlashlight : DEFINE_FACTORY_CLASS(CAIHumanStrategyFlashlight), public CAIHumanStrategy
{
	DEFINE_FACTORY_METHODS(CAIHumanStrategyFlashlight);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Handlers

		void HandleModelString(ArgList* pArgList);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple accessors

		AIHumanStrategyType GetType() { return CAIHumanStrategy::eStrategyFlashlight; }

	protected :

		void FlashlightShow();
		void FlashlightHide();
		void FlashlightOn();
		void FlashlightOff();
		void FlashlightCreate();
		void FlashlightDestroy();

	protected :

		HOBJECT		m_hFlashlightModel;
};

#endif
