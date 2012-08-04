// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Definition
//
// CREATED : 10/2/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CMOVEMGR_H__
#define __CMOVEMGR_H__


#include "iltclient.h"
#include "ContainerCodes.h"
#include "SharedMovement.h"
#include "VarTrack.h"
#include "CameraOffsetMgr.h"
#include "PlayerRigidBody.h"

class CGameClientShell;
class CCharacterFX;
class CVehicleMgr;
class Pusher;
class CPolyGridFX;

class CContainerInfo
{
  public:
	float				m_fGravity;
	float				m_fViscosity;
	LTVector			m_Current;
	ContainerCode		m_ContainerCode;
	bool				m_bHidden;
	LTObjRef			m_hObject;
	PlayerPhysicsModel	m_ePPhysicsModel;
};

class CMoveMgr
{
public:

	CMoveMgr();
	~CMoveMgr();

	bool		Init();
	void		Term( );

	void		Update();
	void		UpdateModels();

	LTVector	GetVelocity() const;
	void		SetVelocity(const LTVector &vVel);
	float		GetMaxVelMag() const;
	float		GetBaseVelMag() const;
	LTVector	GetLastVelocity() const {return m_vLastVel;}

	void		SetGravity(float fGravity) { m_fGravity = fGravity; }

	float		GetMovementPercent() const;
	float		GetVelMagnitude();

	CCharacterFX*	GetCharacterFX()			const { return m_pCharFX; }
	void			SetCharacterFX(CCharacterFX* pFX);

	void		OnPhysicsUpdate(uint16 changeFlags, ILTMessage_Read *pMsg);
	void		UpdateMouseStrafeFlags(const LTVector2 &vAxisOffsets);

	void		OnEnterWorld();
	void		OnExitWorld();

	void		Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	LTRESULT	OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos);
	LTRESULT	OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot);

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

	uint32		GetControlFlags()			const { return m_dwControlFlags; }

	bool		Jumped()	const { return m_bJumped; }

	// Did the move object jump out of water...
	bool		SwimmingJumped( ) const { return (m_bJumped && m_bSwimJumped); }

	// CMoveMgr keeps a list of spheres that repel the player object.
	// These are created from explosions.
	LTRESULT	AddPusher(const LTVector &pos, float radius, float startDelay, float duration, float strength);

	void		SetObject( HOBJECT hObject ) { m_hObject = hObject; }
	HOBJECT		GetObject() const {return m_hObject;}
	HOBJECT		GetServerObject() const;

	void		SetSpectatorMode( SpectatorMode eSpectatorMode );
	void		OnServerForcePos(ILTMessage_Read *pMsg);

	void		WritePositionInfo(ILTMessage_Write *pMsg);

	SurfaceType	GetStandingOnSurface() const { return m_eStandingOnSurface; }

	bool		CanDoFootstep();
	bool		IsHeadInLiquid()	const { return IsLiquid(m_eCurContainerCode); }
	bool		IsFreeMovement()	const { return ::IsFreeMovement(m_eCurContainerCode); }
	bool		IsBodyInLiquid()	const { return m_bBodyInLiquid; }
	bool		IsOnGround()		const { return m_bOnGround; }
	bool		IsOnLift()			const { return m_bOnLift; }
	bool		IsFalling()			const { return m_bFalling; }
	bool		IsMovingQuietly()	const; 
	bool		IsDucking()			const { return !!(m_dwControlFlags & BC_CFLG_DUCK); } 
	void		SetDucking(bool bDuck) { if (bDuck) m_dwControlFlags |= BC_CFLG_DUCK; else m_dwControlFlags &= ~BC_CFLG_DUCK; }

	bool		RunLock()			const { return m_bRunLock; }
	bool		DuckLock()			const { return m_bDuckLock; }
	void		SetRunLock(bool bRunLock)		{ m_bRunLock = bRunLock; }
	void		SetDuckLock(bool bDuckLock)	{ m_bDuckLock = bDuckLock; }

	LTVector	GetGroundNormal()	const { return m_vGroundNormal; }

	void		UpdateOnGround();

	float		GetMoveMultiplier()			const { return m_fMoveMultiplier; }
	float		GetMoveAccelMultiplier()	const { return m_fMoveAccelMultiplier; }
	LTVector	GetTotalCurrent()			const { return m_vTotalCurrent; }
	float		GetTotalViscosity()			const { return m_fTotalViscosity; }

	void		AllowMovement(bool b=true) { m_bAllowMovement = b; }
	bool		GetAllowMovement( ) { return m_bAllowMovement; }

	void			UpdateStartMotion(bool bForce, HRECORD hLandSound, CPolyGridFX* pPolyGridFXSplash );

	CVehicleMgr* GetVehicleMgr()	const { return m_pVehicleMgr; }

	void		AddDamagePenalty(float fDuration, float fMultiplier);

	uint32		GetNumContainers() const { return m_nContainers; }
	CContainerInfo*	GetContainerInfo( uint32 nContainer );
	void		UpdateContainerList();

	void		UpdateContainerViscosity(CContainerInfo *pInfo);
	void		UpdateContainerGravity(CContainerInfo *pInfo);

	// Is it safe to save right now?
	bool		CanSave() const;

	bool		IsInWorld() const { return m_hObject != NULL; }

	float		GetCrouchHeightDifference() const;
	float		GetCurrentHeightDifference() const;

	bool		CanStandUp();
	LTVector const&	GetStandingDims( ) const { return m_vStandDims; }

	bool		IsMoving( ) { return ( (GetVelocity().Mag() > 0.1f) || (g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_MOVING) ); }

	void		SetPlayerLeash( float fInnerRadius, float fOuterRadius, LTVector* pvPos=NULL );
	bool		IsPlayerLeashed();
	LTVector	GetLeashPosition()			const { return m_vLeashPosition; }
	float		GetLeashInnerRadius()		const { return m_vLeashParameters.x; }
	float		GetLeashOuterRadius()		const { return m_vLeashParameters.y; }

	void		EnableSprintBoost( bool bEnable );
	bool		IsSprintBoostEnabled();
	float		GetSprintBoostPercent();

	float		GetDistanceFallenPercent();
	float		GetDamageMovementMultiplier() const;

	// Query if the player is within either posture window...
	// This is useful for determining when certain melee attacks are valid...
	bool		InPostureDownWindow( ) const { return (m_PostureDownTime.IsStarted( ) && !m_PostureDownTime.IsTimedOut( )); }
	bool		InPostureUpWindow( ) const { return (m_PostureUpTime.IsStarted( ) && !m_PostureUpTime.IsTimedOut( )); }

	void		ClearAllRequests();

protected:
	void		InitWorldData();

	void		ShowPos(char *pBlah);
	void		UpdatePushers();

	bool		AreDimsCorrect();
	void		ResetDims(LTVector *pOffset=NULL);

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

	void		UpdateNormalControlFlags();
	void		UpdateNormalMotion();
	void		UpdateNormalFriction();
	void		UpdateAcceleration();
	void		UpdateVelocity();
	void		UpdateSprintBoost();

	void		UpdateContainerMotion();
	void		UpdateInLiquid(CContainerInfo *pInfo);
	void		HandleFallLand(float fDistFell, SurfaceType eLandSurface);

	void		MoveLocalSolidObject();
	void		UpdateVelMagnitude();
	void		SetClientObjNonsolid();

	void		TermLevel();

	
	
	// Handle movement that is encoded in animations...
	void		UpdateMovementEncoding( );

	void		JumpFromLadder();

	// Handle movement towards a goto node...
	void		UpdateGotoNodeMovement( );

	// Update the rotation movement...
	void		UpdateRotation( );

	// Determine if the passed in object can move the player object...
	bool		CanMovePlayer(HOBJECT hObj, bool bTeleport);

	LTVector2	GetCurrentMinProximity();

protected :

	uint8		m_ClientMoveCode;

	// The object representing our movement.
	// NOTE: CMoveMgr does NOT own this object!!! That means don't remove it...
	LTObjRef	m_hObject;

	LTList<Pusher*>	m_Pushers;


	LTVector	m_vWantedDims;

	// Movement state.
	uint32		m_dwControlFlags;
	uint32		m_dwLastControlFlags;

	bool		m_bBodyInLiquid;
	bool		m_bSwimmingOnSurface;
	bool		m_bCanSwimJump;

	ContainerCode	m_eBodyContainerCode;  // Body container code

	bool		m_bLoading;
	LTVector	m_vSavedVel;

	HPOLY			m_hStandingOnPoly;
	SurfaceType		m_eStandingOnSurface;
	SurfaceType		m_eBodyContainerSurface;
	LTVector		m_vGroundNormal;
	bool			m_bOnGround;
	bool			m_bOnLift;
	bool			m_bFalling;
	bool			m_bRunLock;
	bool			m_bDuckLock;

	bool		m_bUsingPlayerModel;

	bool		m_bForceToServerPos;

	float		m_fBaseMoveAccel;
	float		m_fMoveAccelMultiplier;
	float		m_fLastOnGroundY;
	bool		m_bJumped;
	bool		m_bSwimJumped;

	LTVector	m_vTotalCurrent;
	float		m_fTotalViscosity;

	bool		m_bGravityOverride;
	float		m_fTotalContainerGravity;

	// Movement speeds.
	float		m_fJumpVel;
	float		m_fJumpMultiplier;

	float		m_fSwimVel;
	float		m_fWalkVel;
	float		m_fRunVel;
	float		m_fCrawlVel;
	float		m_fMoveMultiplier;
	float		m_fForceIdleVel;


	float		m_fGravity;

	bool		m_bSwimmingJump;
	bool		m_bFirstAniUpdate;
	bool		m_bAllowMovement;
	bool		m_bWaterAffectsSpeed;

	ContainerCode	m_eLastContainerCode;
	ContainerCode	m_eCurContainerCode;
	CContainerInfo	m_Containers[MAX_TRACKED_CONTAINERS];
	uint32			m_nContainers;

	// Spectator speed multiplier.
	VarTrack		m_CV_SpectatorSpeedMul;

	CCharacterFX*	m_pCharFX;

	CVehicleMgr*	m_pVehicleMgr;

	StopWatchTimer	m_DamageTimer;
	float			m_fDamageMoveMultiplier;

	// Player dims semi-constants
	LTVector		m_vCrouchDims;
	LTVector		m_vStandDims;

	bool			m_bWasMovementEncoded;

	LTVector		m_vLastPos;
	LTVector		m_vLastVel;

	// Leash implementation
	static void		PlayerLeashFn( int argc, char** argv );

	LTVector		m_vLeashPosition;
	LTVector2		m_vLeashParameters;

	float			m_fYawSnapShot;
	StopWatchTimer	m_YawTimer;
	bool			m_bInterpolateYaw;
	LTRotation		m_rLastCamRot;
	bool			m_bSeperateCameraFromBody;

	// Sprint boost!
	float			m_fSprintBoostPercent;
	bool			m_bSprintBoostEnabled;

	// Standing on a rigid body that has moved.
	LTObjRef		m_hStandingOnRigidBody;

	// Timers used to give some leeway when changing postures...
	StopWatchTimer	m_PostureDownTime;
	StopWatchTimer	m_PostureUpTime;

	// Rigid body that pushes away physics objects at the feet of players...
	CPlayerRigidBody m_PlayerRigidBody;

	LTRigidTransform m_rtSavedEncodedTransform;
	bool			 m_bHasSavedEncodedTransform;


	//used to track whether we were in a slide kick last frame
	// (because the slide kick can get the player into an area that should force him to crouch)
	bool			m_bWasSlideKicking;
};



extern CMoveMgr* g_pMoveMgr;

#endif  // __CMOVEMGR_H__
