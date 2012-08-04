// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STRATEGY_H__
#define __AI_HUMAN_STRATEGY_H__

#include "AIBrain.h"
#include "AIStrategy.h"
#include "AnimationMgr.h"
#include "AIVolume.h"

class CCharacter;
class CAIHuman;
class CWeapon;
class AIVolume;
class CAIPath;
class CAIPathWaypoint;
class CAnimationContext;
class CAIHumanStrategyShoot;

class AINode;
class AINodeCover;
class AINodeSearch;
class AINodePickup;
class AINodePanic;


// Classes

class CAIHumanStrategy : public CAIClassAbstract
{
	public : // Public member variables

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Strategy);

		CAIHumanStrategy( );

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(ILTMessage_Read *pMsg) {}
		virtual void Save(ILTMessage_Write *pMsg) {}

		// Updates

		virtual void Update() {}
		virtual LTBOOL UpdateAnimation();

		// Misc

		virtual LTBOOL DelayChangeState() { return LTFALSE; }

		// Simple accessors

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

class CAIHumanStrategyOneShotAni : public CAIHumanStrategy
{
	typedef CAIHumanStrategy super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyOneShotAni, kStrat_HumanOneShotAni);

		CAIHumanStrategyOneShotAni( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Set(EnumAnimPropGroup eGroup, EnumAnimProp eProp);
		void Reset() { m_eState = eUnset; m_Prop.Clear(); }

		void Update();
		LTBOOL UpdateAnimation();

		// State

		LTBOOL IsUnset() { return m_eState == eUnset; }
		LTBOOL IsSet() { return m_eState == eSet; }
		LTBOOL IsAnimating() { return m_eState == eAnimating; }
		LTBOOL IsDone() { return m_eState == eDone; }

		// Simple accessors

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

class CAIHumanStrategyFollowPath : public CAIHumanStrategy
{
	typedef CAIHumanStrategy super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyFollowPath, kStrat_HumanFollowPath);

		CAIHumanStrategyFollowPath( );
		~CAIHumanStrategyFollowPath( );

		enum Medium
		{
			eMediumGround,
			eMediumUnderwater,
		};

	public :

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman, CAIHumanStrategyShoot* pStrategyShoot);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Reset();

		LTBOOL Set(const LTVector& vDestination, LTBOOL bDivergePaths);
		LTBOOL Set(const LTVector& vDestination, const LTVector& vDir, LTBOOL bDivergePaths);
		LTBOOL Set(AINode* pNodeDestination, LTBOOL bDivergePaths);
		LTBOOL Set(AIVolume* pVolumeDestination, LTBOOL bDivergePaths);

		LTBOOL SetRandom(AIVolume* pVolumeSrcPrev, AIVolume* pVolumeSrc, AIVolume* pVolumeSrcNext);

		void Update();
		LTBOOL UpdateAnimation();

		// Path reservation.

		void ReservePath();
		void ClearReservedPath();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Misc

		LTBOOL DelayChangeState();

		// Movement types

		void SetMovement(EnumAnimProp eMovement);
		EnumAnimProp GetMovement();

		void SetMovementModifier(EnumAnimPropGroup eGroup, EnumAnimProp eProp) { m_bModifiedMovement = LTTRUE; m_aniModifiedMovement.Set(eGroup, eProp); }
		void ClearMovementModifier() { m_bModifiedMovement = LTFALSE; }

		// Medium types

		void SetMedium(Medium eMedium);

		// Simple accessors

		LTBOOL IsUnset() { return m_eState == eStateUnset; }
		LTBOOL IsSet() { return m_eState == eStateSet; }
		LTBOOL IsDone() { return m_eState == eStateDone; }

		const LTVector& GetDest() const { return m_vDest; }
	
		void GetInitialDir(LTVector* pvDir);
		void GetFinalDir(LTVector* pvDir);
		AIVolume* GetNextVolume(AIVolume* pVolume, AIVolume::EnumVolumeType eVolumeType);

		// Debug rendering.

		void DebugDrawPath();
		void DebugRemainingDrawPath();

	protected :

		LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateClimbTo(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateJumpTo(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateFaceJumpLand(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateFaceLadder(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateFaceDoor(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateOpenDoors(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateWaitForDoors(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateReleaseGate(CAIPathWaypoint* pWaypoint);
		LTBOOL UpdateMoveTeleport(CAIPathWaypoint* pWaypoint);

		LTBOOL DoorsBlocked( HOBJECT hDoor );

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
			eDoorStateWaitingForAnimationToFinish,
			eDoorStateWaitingForDoorToOpen,
			eDoorStateWalkingThroughDoor,
		};

	protected :

		CAIPath*				m_pPath;
		State					m_eState;
		Medium					m_eMedium;
		DoorState				m_eDoorState;
		EnumAnimProp			m_eDoorAction;
		LTFLOAT					m_fLastDoor1Yaw;
		LTFLOAT					m_fLastDoor2Yaw;
		CAnimationProp			m_aniModifiedMovement;
		LTBOOL					m_bModifiedMovement;
		uint32					m_cStuckOnDoorUpdates;
		LTBOOL					m_bDoorShootThroughable;
		LTVector				m_vDest;
		LTBOOL					m_bCheckAnimStatus;

		// Debug
		LTBOOL					m_bDrawingPath;

		class CAIHumanStrategyShoot*	m_pStrategyShoot;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyDodge
//
// PURPOSE : AI ability to dodge
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyDodge : public CAIHumanStrategy
{
	typedef CAIHumanStrategy super;

	public :

		enum State
		{
			eStateChecking,
			eStateDodging,
			eStateFleeing,
			eStateCovering,
		};

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyDodge, kStrat_HumanDodge);

		CAIHumanStrategyDodge( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Dodge

		void Dodge();
		LTBOOL IsDodging() const { return m_eState == eStateDodging; }
		State GetDodgeState() const { return m_eState; }

		// Outcome

		DodgeStatus GetStatus() const { return m_eDodgeStatus; }
		DodgeAction GetAction() const { return m_eDodgeAction; }

		// Simple accessors

		const LTVector& GetScatterPosition() const { return m_vScatterPosition; }
		void ForceDodge() { m_bForceDodge = LTTRUE; }

	protected :

		// Update Check

		void UpdateCheck();

		// Update Dodge

		void UpdateDodge();
		void UpdateDodgeDive();
		void UpdateDodgeFlee();
		void UpdateDodgeCover();

		// Check

		void Check();

	protected :

		AINode*		m_pNode;

		State		m_eState;
		DodgeStatus	m_eDodgeStatus;
		DodgeAction	m_eDodgeAction;

		LTVector	m_vScatterPosition;

		Direction	m_eDirection;
		LTVector	m_vDir;

		LTBOOL		m_bForceDodge;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyTaunt
//
// PURPOSE : AI ability to taunt
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyTaunt : public CAIHumanStrategy
{
	typedef CAIHumanStrategy super;

	public :

		enum State
		{
			eStateChecking,
			eStateTaunting,
		};

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyTaunt, kStrat_HumanTaunt);

		CAIHumanStrategyTaunt( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Taunting.

		LTBOOL IsTaunting() const { return m_eState == eStateTaunting; }
		void   ResetTaunting() { Check(); }

	protected:

		void Check();

	protected :

		LTFLOAT		m_fCheckTime;
		State		m_eState;

		// The following do not need to be saved.

		LTFLOAT		m_fMinTauntDistSqr;
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
	typedef CAIHumanStrategy super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Strategy);

		CAIHumanStrategyCover( );

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Clear();

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Updates

		virtual void Update();
		virtual LTBOOL UpdateAnimation();

		// Cover

		virtual void Cover(LTFLOAT fDelay = 0.0f);
		virtual void Uncover(LTFLOAT fDelay= 0.0f);

		// Simple accessors

		void SetCoverNode(AINode *pCoverNode) { m_pCoverNode = pCoverNode; }
		void SetCoverTime(LTFLOAT fCoverTime, LTFLOAT fCoverTimeRandom = 0.0f) { m_fCoverTime = fCoverTime + GetRandom(0.0f, fCoverTimeRandom); }
		void SetUncoverTime(LTFLOAT fUncoverTime, LTFLOAT fUncoverTimeRandom = 0.0f) { m_fUncoverTime = fUncoverTime + GetRandom(0.0f, fUncoverTimeRandom); }

		LTBOOL IsCovered() { return m_eState == eCovered; }
		LTBOOL IsUncovered() { return m_eState == eUncovered; }
		LTBOOL IsCovering() { return m_eState == eCovering; }
		LTBOOL IsUncovering() { return m_eState == eUncovering; }

		virtual LTBOOL CanBlindFire();
		virtual LTBOOL OneAnimCover() { return m_bOneAnimCover; }

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

		LTBOOL	m_bOneAnimCover;		// Attack from cover is all one animation.
		LTBOOL	m_bOneAnimFiring;		// Playing the one animation firing animation.

		LTBOOL	m_bWantCover;			// Has Cover been requested
		LTFLOAT	m_fCoverTimer;			// How long have we been Covered
		LTFLOAT	m_fCoverTime;			// How long should we stay Covered for
		LTFLOAT	m_fCoverDelay;			// When should we execute a Cover request

		LTBOOL	m_bWantUncover;			// Has Uncover been requested
		LTFLOAT	m_fUncoverTimer;		// How long have we been Uncovered
		LTFLOAT	m_fUncoverTime;			// How long should we stay Uncovered for
		LTFLOAT	m_fUncoverDelay;		// When should we execute a Uncover request

		// These don't need saving

		AINode*	m_pCoverNode;	// Our cover node
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCoverDuck
//
// PURPOSE : AI cover ability that uses ducking
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCoverDuck : public CAIHumanStrategyCover
{
	typedef CAIHumanStrategyCover super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCoverDuck, kStrat_HumanCoverDuck);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Simple accessors

		virtual LTBOOL OneAnimCover() { return LTFALSE; }
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCoverBlind
//
// PURPOSE : AI cover ability that simply stays behind cover and blind fires
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCoverBlind : public CAIHumanStrategyCover
{
	typedef CAIHumanStrategyCover super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCoverBlind, kStrat_HumanCoverBlind);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Simple accessors

		virtual LTBOOL CanBlindFire() { return LTTRUE; }
		virtual LTBOOL OneAnimCover() { return LTFALSE; }
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCover1WayCorner
//
// PURPOSE : AI cover ability that uses Cornerping in one direction
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCover1WayCorner : public CAIHumanStrategyCover
{
	typedef CAIHumanStrategyCover super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCover1WayCorner, kStrat_HumanCover1WayCorner);

		CAIHumanStrategyCover1WayCorner( );

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Simple accessors

	protected:

		LTFLOAT GetMovementData();

	protected :

		Direction	m_eDirection;
		LTVector	m_vDir;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyCover2WayCorner
//
// PURPOSE : AI cover ability that uses Cornerping in one direction
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyCover2WayCorner : public CAIHumanStrategyCover
{
	typedef CAIHumanStrategyCover super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCover2WayCorner, kStrat_HumanCover2WayCorner);

		CAIHumanStrategyCover2WayCorner( );

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Simple accessors

	protected:

		LTFLOAT GetMovementData();

	protected :

		Direction	m_eDirection;
		LTVector	m_vDir;
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
	typedef CAIHumanStrategy super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Strategy);

		// Throw

		virtual void Throw(LTFLOAT fTime = 0.5f) { }
		virtual LTBOOL ShouldThrow() { return LTFALSE; }
		virtual LTBOOL IsThrowing() { return LTFALSE; }

		// Handlers

		virtual void HandleModelString(ArgList* pArgList) {}

		// Simple accessors

	protected :
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyGrenadeThrow
//
// PURPOSE : AI Grenade ability that throws the grenade
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyGrenadeThrow : public CAIHumanStrategyGrenade
{
	typedef CAIHumanStrategyGrenade super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyGrenadeThrow, kStrat_HumanGrenadeThrow);

		CAIHumanStrategyGrenadeThrow( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Updates

		void Update();
		LTBOOL UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Throw

		void Throw(LTFLOAT fTime = 0.5f);
		LTBOOL ShouldThrow();
		LTBOOL IsThrowing();

		// Simple accessors

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
	typedef CAIHumanStrategy super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Strategy);

		CAIHumanStrategyShoot( );
		~CAIHumanStrategyShoot( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Updates

        void Update(HOBJECT hTarget = LTNULL);
		LTBOOL UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Clearing

		void	Clear();

		// Reloading

		LTBOOL	IsReloading() const { return m_eState == eStateReloading; }
		void	Reload(LTBOOL bInstant = LTFALSE);
		LTBOOL	ShouldReload();

		// FOV

		void SetIgnoreFOV(LTBOOL bIgnoreFOV) { m_bIgnoreFOV = bIgnoreFOV; }

		// Blind fire

		LTBOOL IsBlind() const { return m_bShootBlind; }
		void SetShootBlind(LTBOOL bBlind) { m_bShootBlind = bBlind; }

		// Hack...

		virtual LTBOOL IsFiring() { return m_bFired; }
		void ClearFired() { m_bFired = LTFALSE; }
		void ForceFire(HOBJECT hTargetObject);

		// Simple accessors

	protected :

		// Updates

		virtual void UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon) = 0;
		virtual void UpdateAiming(HOBJECT hTarget) = 0;

		void UpdateNeedsReload(CWeapon* pWeapon);
		void UpdateReloading(CWeapon* pWeapon);

		// Handlers

		virtual void HandleFired(const char* const pszSocketName);

		// Aim/Fire

		virtual void Aim();
		virtual void Fire();

		LTVector	GetFirePosition(CWeapon*);

		// Reload

		LTBOOL	NeedsReload() { return m_bNeedsReload; }// Clip Empty?
		LTBOOL	CanReload();							// Do we have a Reserve, or can we generate?

		LTBOOL	IsOutOfAmmo() { return m_bOutOfAmmo; }	// Ammo in Reserves?

		LTBOOL	CanGenerateAmmo( int iWeaponIndex );	// Are we able to generate ammo?

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
		LTBOOL	m_bShootBlind;		// Shoot blind?
		HMODELSOCKET m_hFiringSocket;	// Socket the shot should come from
										// (if not INVALID_MODEL_SOCKET)
		uint32	m_iAnimRandomSeed;	// Randomizing aim/fire animation pairs.
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyShootBurst
//
// PURPOSE : AI shoot ability - to burst fire at something
//
// ----------------------------------------------------------------------- //

class CAIHumanStrategyShootBurst : public CAIHumanStrategyShoot
{
	typedef CAIHumanStrategyShoot super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyShootBurst, kStrat_HumanShootBurst);

		CAIHumanStrategyShootBurst( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);
		
		// Simple accessors

	protected :

		// Updates

		virtual void UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon);
		virtual void UpdateAiming(HOBJECT hTarget);

		// Aim/Fire

		virtual void Aim();
		virtual void Fire();
		
		// Handlers

		virtual void HandleFired(const char* const pszSocketName);

		// Burst methods

		void CalculateBurst();

	protected :

		LTFLOAT		m_fBurstInterval;		// How long between bursts (this is the timer)
		int			m_nBurstShots;			// How many shots in each burst (this is the counter)
		LTBOOL		m_bUseIntervals;		// Do we pause between bursts?
		LTBOOL		m_bPreFire;				
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHumanStrategyFlashlight
//
// PURPOSE : AI strategy for using a flashlight
//
// ----------------------------------------------------------------------- //

// For constructor.  warning C4355: 'this' : used in base member initializer list
#pragma warning( push )
#pragma warning( disable : 4355 )

class CAIHumanStrategyFlashlight : public CAIHumanStrategy, public ILTObjRefReceiver
{
	typedef CAIHumanStrategy super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyFlashlight, kStrat_HumanFlashlight);

		CAIHumanStrategyFlashlight( );
		~CAIHumanStrategyFlashlight( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Simple accessors

	protected :

		void FlashlightShow();
		void FlashlightHide();
		void FlashlightOn();
		void FlashlightOff();
		void FlashlightCreate();
		void FlashlightDestroy();

	protected :

		LTObjRefNotifier	m_hFlashlightModel;
};

#pragma warning( pop )

#endif
