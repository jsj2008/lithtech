// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Definition
//
// CREATED : 10/2/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
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

class ILTClientPhysics;
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
};

class CMoveMgr
{
  public:

	CMoveMgr();
	~CMoveMgr();

    LTBOOL      Init();

	void		Update();
	void		UpdateModels();

    LTVector    GetVelocity() const;
	void		SetVelocity(LTVector vVel);
	LTFLOAT		GetMaxVelMag() const;

	void		SetGravity(LTFLOAT fGravity) { m_fGravity = fGravity; }

    LTFLOAT     GetMovementPercent() const;
    LTFLOAT     GetVelMagnitude();

	LTBOOL		IsPlayerModel();

	CCharacterFX*	GetCharacterFX()			const { return m_pCharFX; }
	void			SetCharacterFX(CCharacterFX* pFX) { m_pCharFX = pFX; }

	void		OnPhysicsUpdate(HMESSAGEREAD hRead);
	void		UpdateMouseStrafeFlags(float *pAxisOffsets);

	void		OnEnterWorld();
	void		OnExitWorld();

	void		Save(HMESSAGEWRITE hWrite);
	void		Load(HMESSAGEREAD hRead);

    LTRESULT    OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos);
    LTRESULT    OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot);

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()			const { return m_dwControlFlags; }

    LTBOOL      Jumped()            const { return m_bJumped; }

	// CMoveMgr keeps a list of spheres that repel the player object.
	// These are created from explosions.
    LTRESULT     AddPusher(LTVector &pos, float radius,
		float startDelay, float duration, float strength);

	HOBJECT		GetObject() {return m_hObject;}

    void        SetSpectatorMode(LTBOOL bSet);
	void		OnServerForcePos(HMESSAGEREAD hRead);

	void		WritePositionInfo(HMESSAGEWRITE hWrite);

	SurfaceType	GetStandingOnSurface() const { return m_eStandingOnSurface; }

    LTBOOL      CanDoFootstep();
    LTBOOL      IsHeadInLiquid()    const { return IsLiquid(m_eCurContainerCode); }
    LTBOOL      IsFreeMovement()    const { return ::IsFreeMovement(m_eCurContainerCode); }
	LTBOOL      IsBodyInLiquid()    const { return m_bBodyInLiquid; }
    LTBOOL      IsOnGround()        const { return m_bOnGround; }
    LTBOOL      IsBodyOnLadder()    const { return m_bBodyOnLadder; }
    LTBOOL      IsOnLift()          const { return m_bOnLift; }
    LTBOOL      IsFalling()         const { return m_bFalling; }
    LTBOOL      IsZipCordOn()       const { return m_bZipCordOn; }
	LTBOOL		IsMovingQuietly()	const; 

	void		SetMouseStrafeFlags(int nFlags) { m_nMouseStrafeFlags = nFlags; }

	void		UpdateOnGround();

	LTFLOAT		GetMoveMultiplier()			const { return m_fMoveMultiplier; }
	LTFLOAT		GetMoveAccelMultiplier()	const { return m_fMoveAccelMultiplier; }
	LTVector	GetTotalCurrent()			const { return m_vTotalCurrent; }
	LTFLOAT		GetTotalViscosity()			const { return m_fTotalViscosity; }

    void        AllowMovement(LTBOOL b=LTTRUE) { m_bAllowMovement = b; }

	void		TurnOnZipCord(HOBJECT hHookObj);
	void		TurnOffZipCord();

    void        UpdateStartMotion(LTBOOL bForce=LTFALSE);

	CVehicleMgr* GetVehicleMgr()	const { return m_pVehicleMgr; }

  protected:
	  
	void		InitWorldData();

	void		ShowPos(char *pBlah);
	void		UpdatePushers();
	void		UpdatePlayerAnimation();

    LTBOOL      AreDimsCorrect();
    void        ResetDims(LTVector *pOffset=NULL);

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

	void		UpdateNormalControlFlags();
	void		UpdateNormalMotion();
	void		UpdateNormalFriction();

	void		UpdateContainerMotion();
	void		UpdateContainerViscosity(CContainerInfo *pInfo);
	void		UpdateOnLadder(CContainerInfo *pInfo);
	void		UpdateInLiquid(CContainerInfo *pInfo);
    void        HandleFallLand(LTFLOAT fDistFell);

	void		MoveLocalSolidObject();
	void		UpdateVelMagnitude();
	void		SetClientObjNonsolid();
	void		MoveToClientObj();

	void		TermLevel();

  protected :

    uint8       m_ClientMoveCode;

	// The object representing our movement.
	HOBJECT		m_hObject;

    LTLink      m_Pushers;


    LTVector    m_vWantedDims;

	// Zip cord info.
    LTVector    m_vZipCordPos;
    LTVector    m_vZipHookDims;
	float		m_fZipCordVel;
    LTBOOL      m_bZipCordOn;

	// Movement state.
    uint32      m_dwControlFlags;
    uint32      m_dwLastControlFlags;
    uint32      m_nMouseStrafeFlags;

    LTBOOL      m_bBodyInLiquid;
    LTBOOL      m_bSwimmingOnSurface;
    LTBOOL      m_bCanSwimJump;

	ContainerCode	m_eBodyContainerCode;  // Body container code

    LTBOOL      m_bBodyOnLadder;
	LTBOOL		m_bLoading;
	LTVector	m_vSavedVel;

	HPOLY		m_hStandingOnPoly;
    SurfaceType m_eStandingOnSurface;
    LTBOOL      m_bOnGround;
    LTBOOL      m_bOnLift;
    LTBOOL      m_bFalling;

	LTBOOL		m_bUsingPlayerModel;

	float		m_fBaseMoveAccel;
	float		m_fMoveAccelMultiplier;
	float		m_fLastOnGroundY;
    LTBOOL      m_bJumped;

    LTVector    m_vTotalCurrent;
	float		m_fTotalViscosity;

	// Movement speeds.
	float		m_fJumpVel;
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

    HLTSOUND    m_hZipcordSnd;

	ContainerCode	m_eLastContainerCode;
	ContainerCode	m_eCurContainerCode;
	CContainerInfo	m_Containers[MAX_TRACKED_CONTAINERS];
    uint32          m_nContainers;

	// Spectator speed multiplier.
	VarTrack		m_CV_SpectatorSpeedMul;

	CCharacterFX*	m_pCharFX;

	CVehicleMgr*	m_pVehicleMgr;
};

inline LTBOOL CMoveMgr::IsMovingQuietly() const
{ 
	LTBOOL bRet = LTFALSE;

	if ((m_dwControlFlags & BC_CFLG_DUCK) || !(m_dwControlFlags & BC_CFLG_RUN))
	{
		if (!IsFreeMovement() && !IsBodyOnLadder() && IsOnGround())
		{
			bRet = LTTRUE;
		}
	}

	return bRet;
}

#endif  // __CMOVEMGR_H__