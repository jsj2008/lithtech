// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Definition
//
// CREATED : 10/2/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CMOVEMGR_H__
#define __CMOVEMGR_H__


#include "iltclient.h"
#include "ContainerCodes.h"
#include "SharedMovement.h"
#include "SurfaceMgr.h"
#include "VarTrack.h"
#include "CameraOffsetMgr.h"
#include "timer.h"

class CGameClientShell;
class CCharacterFX;
class CVehicleMgr;

class CContainerInfo
{
  public:
	float			m_fGravity;
	float			m_fViscosity;
    LTVector        m_Current;
	ContainerCode	m_ContainerCode;
    LTBOOL          m_bHidden;
	HOBJECT			m_hObject;
	PlayerPhysicsModel m_ePPhysicsModel;
};

class CMoveMgr
{
  public:

	CMoveMgr();
	~CMoveMgr();

    LTBOOL      Init();

	void		Update();
	void		UpdateModels();

    LTVector	GetVelocity() const;
	void		SetVelocity(const LTVector &vVel);
	LTFLOAT		GetMaxVelMag() const;

	void		SetGravity(LTFLOAT fGravity) { m_fGravity = fGravity; }

    LTFLOAT     GetMovementPercent() const;
    LTFLOAT     GetVelMagnitude();

	LTBOOL		IsPlayerModel();

	CCharacterFX*	GetCharacterFX()			const { return m_pCharFX; }
	void			SetCharacterFX(CCharacterFX* pFX) { m_pCharFX = pFX; }

	void		OnPhysicsUpdate(uint16 changeFlags, ILTMessage_Read *pMsg);
	void		UpdateMouseStrafeFlags(float *pAxisOffsets);

	void		OnEnterWorld();
	void		OnExitWorld();

	void		Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

    LTRESULT    OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos);
    LTRESULT    OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot);

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()			const { return m_dwControlFlags; }

    LTBOOL      Jumped()            const { return m_bJumped; }

	// CMoveMgr keeps a list of spheres that repel the player object.
	// These are created from explosions.
    LTRESULT     AddPusher(const LTVector &pos, float radius,
		float startDelay, float duration, float strength);

	HOBJECT		GetObject() {return m_hObject;}
	HOBJECT		GetServerObject();

    void        SetSpectatorMode(LTBOOL bSet);
	void		OnServerForcePos(ILTMessage_Read *pMsg);

	void		WritePositionInfo(ILTMessage_Write *pMsg);

	SurfaceType	GetStandingOnSurface() const { return m_eStandingOnSurface; }

    LTBOOL      CanDoFootstep();
    LTBOOL      IsHeadInLiquid()    const { return IsLiquid(m_eCurContainerCode); }
    LTBOOL      IsFreeMovement()    const { return ::IsFreeMovement(m_eCurContainerCode); }
	LTBOOL      IsBodyInLiquid()    const { return m_bBodyInLiquid; }
    LTBOOL      IsOnGround()        const { return m_bOnGround; }
    LTBOOL      IsBodyOnLadder()    const { return m_bBodyOnLadder; }
    LTBOOL      IsOnLift()          const { return m_bOnLift; }
    LTBOOL      IsFalling()         const { return m_bFalling; }
	LTBOOL		IsMovingQuietly()	const; 
	LTBOOL		IsDucking()			const { return (m_dwControlFlags & BC_CFLG_DUCK); } 
	void		SetDucking(bool bDuck) { if (bDuck) m_dwControlFlags |= BC_CFLG_DUCK; else m_dwControlFlags &= ~BC_CFLG_DUCK; }

    LTBOOL      RunLock()			const { return m_bRunLock; }
    LTBOOL      DuckLock()			const { return m_bDuckLock; }
    void		SetRunLock(LTBOOL bRunLock)		{ m_bRunLock = bRunLock; }
    void		SetDuckLock(LTBOOL bDuckLock)	{ m_bDuckLock = bDuckLock; }

	LTVector	GetGroundNormal()	const { return m_vGroundNormal; }

	void		UpdateOnGround();

	LTFLOAT		GetMoveMultiplier()			const { return m_fMoveMultiplier; }
	LTFLOAT		GetMoveAccelMultiplier()	const { return m_fMoveAccelMultiplier; }
	LTVector	GetTotalCurrent()			const { return m_vTotalCurrent; }
	LTFLOAT		GetTotalViscosity()			const { return m_fTotalViscosity; }

    void        AllowMovement(LTBOOL b=LTTRUE) { m_bAllowMovement = b; }
    LTBOOL		GetAllowMovement( ) { return m_bAllowMovement; }

    void        UpdateStartMotion(LTBOOL bForce=LTFALSE);

	CVehicleMgr* GetVehicleMgr()	const { return m_pVehicleMgr; }

	void		AddDamagePenalty(LTFLOAT fDuration);

	HOBJECT		GetLadderObject()	const { return m_hLadderObject; }

	uint32		GetNumContainers() const { return m_nContainers; }
	CContainerInfo*	GetContainerInfo( uint32 nContainer );
	void		UpdateContainerList();

	void		UpdateContainerViscosity(CContainerInfo *pInfo);
	void		UpdateContainerGravity(CContainerInfo *pInfo);

	// Is it safe to save right now?
	bool		CanSave() const;

	bool		IsInWorld() const { return m_hObject != LTNULL; }

	float		GetCrouchHeightDifference() const;
	float		GetCurrentHeightDifference() const;

  protected:
	void		InitWorldData();

	void		ShowPos(char *pBlah);
	void		UpdatePushers();
	void		UpdatePlayerAnimation();

    LTBOOL      AreDimsCorrect();
    void        ResetDims(LTVector *pOffset=NULL);

	bool		CanStandUp();

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

	void		UpdateNormalControlFlags();
	void		UpdateNormalMotion();
	void		UpdateNormalFriction();

	void		UpdateContainerMotion();
	void		UpdateOnLadder(CContainerInfo *pInfo);
	void		UpdateInLiquid(CContainerInfo *pInfo);
    void        HandleFallLand(LTFLOAT fDistFell);

	void		MoveLocalSolidObject();
	void		UpdateVelMagnitude();
	void		SetClientObjNonsolid();

	void		TermLevel();

	LTFLOAT		CalculateMovementPenalty() const;
	
	
  protected :

    uint8       m_ClientMoveCode;

	// The object representing our movement.
	HOBJECT		m_hObject;

    LTLink      m_Pushers;


    LTVector	m_vWantedDims;

	// Movement state.
    uint32      m_dwControlFlags;
    uint32      m_dwLastControlFlags;

    LTBOOL      m_bBodyInLiquid;
    LTBOOL      m_bSwimmingOnSurface;
    LTBOOL      m_bCanSwimJump;

	ContainerCode	m_eBodyContainerCode;  // Body container code

    LTBOOL      m_bBodyOnLadder;
	HOBJECT		m_hLadderObject;

	LTBOOL		m_bLoading;
	LTVector	m_vSavedVel;

	HPOLY		m_hStandingOnPoly;
    SurfaceType m_eStandingOnSurface;
	LTVector	m_vGroundNormal;
    LTBOOL      m_bOnGround;
    LTBOOL      m_bOnLift;
    LTBOOL      m_bFalling;
    LTBOOL      m_bRunLock;
    LTBOOL      m_bDuckLock;

	LTBOOL		m_bUsingPlayerModel;

	LTBOOL		m_bForceToServerPos;

	float		m_fBaseMoveAccel;
	float		m_fMoveAccelMultiplier;
	float		m_fLastOnGroundY;
    LTBOOL      m_bJumped;

    LTVector    m_vTotalCurrent;
	float		m_fTotalViscosity;

	bool		m_bGravityOverride;
	float		m_fTotalContainerGravity;

	// Movement speeds.
	float		m_fJumpVel;
	float		m_fSuperJumpVel;
	float		m_fJumpMultiplier;

	float		m_fSwimVel;
	float		m_fWalkVel;
	float		m_fRunVel;
	float		m_fLadderVel;
	float		m_fMoveMultiplier;

	float		m_fGravity;

    LTBOOL      m_bSwimmingJump;
    LTBOOL      m_bFirstAniUpdate;
    LTBOOL      m_bAllowMovement;

	ContainerCode	m_eLastContainerCode;
	ContainerCode	m_eCurContainerCode;
	CContainerInfo	m_Containers[MAX_TRACKED_CONTAINERS];
    uint32          m_nContainers;

	// Spectator speed multiplier.
	VarTrack		m_CV_SpectatorSpeedMul;

	CCharacterFX*	m_pCharFX;

	CVehicleMgr*	m_pVehicleMgr;

	CTimer			m_DamageTimer;

	// Player dims semi-constants
	
	LTVector		m_vCrouchDims;
	LTVector		m_vStandDims;
};



extern CMoveMgr* g_pMoveMgr;

#endif  // __CMOVEMGR_H__
