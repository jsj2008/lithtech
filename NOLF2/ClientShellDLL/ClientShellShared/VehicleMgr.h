// ----------------------------------------------------------------------- //
//
// MODULE  : CVehicleMgr.cpp
//
// PURPOSE : Client side vehicle movement mgr - Definition
//
// CREATED : 6/12/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_MGR_H__
#define __VEHICLE_MGR_H__

#include "iltclient.h"
#include "SharedMovement.h"
#include "CameraOffsetMgr.h"
#include "Timer.h"
#include "SurfaceDefs.h"
#include "LTObjRef.h"

struct VEHICLENODE
{
	VEHICLENODE()
	{
		hNode = INVALID_MODEL_NODE;
		matTransform.Identity();
	}

	HMODELNODE	hNode;
	LTMatrix	matTransform;
};

class CVehicleMgr
{
  public:

	CVehicleMgr();
	~CVehicleMgr();

    LTBOOL      Init();

	void		UpdateModels();

    LTFLOAT     GetMaxVelMag(LTFLOAT fMult=1.0f) const;
    LTFLOAT     GetMaxAccelMag(LTFLOAT fMult=1.0f) const;

	LTFLOAT		GetVehicleMovementPercent() const;

	void		OnEnterWorld();
	void		OnExitWorld();

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()	const { return m_dwControlFlags; }
 
	LTBOOL		CanShowCrosshair()	const { return (m_ePPhysicsModel != PPM_SNOWMOBILE); }
	LTBOOL      IsVehiclePhysics();
	LTBOOL		CanDismount();
	LTBOOL      IsTurning()				const { return (m_bVehicleTurning && !m_bVehicleStopped); }
	int			GetTurnDirection()		const { return m_nVehicleTurnDirection; }

	PlayerPhysicsModel GetPhysicsModel()	const { return m_ePPhysicsModel; }

	void		CalculateVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR, LTFLOAT fYawDelta);

	// Allows vehicles to perterb the camera placement.
	void		CalculateVehicleCameraDisplacment( LTVector& vDisplacement );

	void		GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot);
	void		HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

	void		InitWorldData();

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

 	void		TermLevel();
	
	CCameraOffsetMgr* GetModelOffsetMgr() { return &m_VehicleModelOffsetMgr; }

	void		SetPhysicsModel(PlayerPhysicsModel eModel, LTBOOL bDoPreSet=LTTRUE);

	void		AdjustCameraRoll(LTFLOAT & fRoll);

	void		SetPlayerLureId( uint32 nPlayerLureId );
	uint32		GetPlayerLureId( ) { return m_nPlayerLureId; }

	float		GetVehicleContourRoll() const { return m_ePPhysicsModel == PPM_SNOWMOBILE ? m_fContourRoll : 0.0f; }

	// Called by vehiclemgr.
	bool		MoveLocalSolidObject( );

	// Command handling functions
	LTBOOL		OnCommandOn(int command);
	LTBOOL		OnCommandOff(int command);
	void		AlignAngleToAxis(float &fAngle);
	void		SnapLightCycleToXZGrid(LTVector &vNewPos, LTVector &vOldPos, uint32 nGridSize);
	
	void		MoveVehicleObject( LTVector &vPos );

	// Serialization
	void		Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	// Is it safe to save?
	bool		CanSave() const { return m_bHaveJumpVolumeVel == LTFALSE; }

  protected:

	void		UpdateVehicleMotion();
	void		UpdateVehicleGear();
	void		UpdateVehicleSounds();
	char*		GetIdleSnd();
	char*		GetBrakeSnd();
	char*		GetAccelSnd();
	char*		GetDecelSnd();
	char*		GetImpactSnd(LTFLOAT fCurVelocityPercent, SURFACE* pSurface);
	
	LTFLOAT		GetVehicleAccelTime();
	LTFLOAT		GetVehicleMinTurnAccel();
	LTFLOAT		GetVehicleMaxDecel();
	LTFLOAT		GetVehicleDecelTime();
	LTFLOAT		GetVehicleMaxBrake();
	LTFLOAT		GetVehicleBrakeTime();

	void		UpdateSnowmobileControlFlags();
	void		UpdateSnowmobileGear();
	void		UpdateSnowmobileHandleBars();

	void		UpdateLureControlFlags();
	void		UpdateLureMotion();
	
	void		UpdateLightCycleControlFlags();
	void		UpdateLightCycleMotion();


	void        KillAllVehicleSounds(HLTSOUND hException=LTNULL);
	
	void		PlayVehicleAni(char* pAniName, LTBOOL bReset=LTTRUE, LTBOOL bLoop=LTFALSE);
	LTBOOL		IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone);

	LTBOOL		PreSetPhysicsModel(PlayerPhysicsModel eModel);

	void		SetSnowmobilePhysicsModel();
	void		SetPlayerLurePhysicsModel( );
	void		SetNormalPhysicsModel();

	void		CreateVehicleModel();
	void		UpdateVehicleModel();
	void		UpdateVehicleFriction(LTFLOAT fSlideToStopTime);

	void		ShowVehicleModel(LTBOOL bShow=LTTRUE);

	LTFLOAT		GetSurfaceVelMult(SurfaceType eType) const;

	void		CalculateBankingVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR, LTFLOAT fYawDelta);
	void		CalculateLureVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR);
	void		CalculateVehicleContourRotation( LTVector &vPlayerPYR, LTVector &vPYR );
	void		CalculateLightCycleRotation(LTVector & vPlayerPYR, LTVector & vPYR);
		
	bool		MoveToLure( );
	bool		MoveLightCycle();

	bool		HandleJumpVolumeTouch( HOBJECT hJumpVolume );
	bool		HandleCharacterTouch( HOBJECT hCharacter );

	void		UpdateInJumpVolume();
	void		EnableWeapon( bool bEnable );
	
	void		UpdateContainerMotion();
	
	void		HandleCollision();

  protected :

	HOBJECT		m_hVehicleModel;
    LTFLOAT		m_fAccelStart;
    LTFLOAT     m_fDecelStart;

	// Movement state.
    uint32      m_dwControlFlags;
    uint32      m_dwLastControlFlags;

	float		m_fVehicleBaseMoveAccel;

	PlayerPhysicsModel m_ePPhysicsModel;

    HLTSOUND        m_hVehicleStartSnd;
    HLTSOUND        m_hVehicleAccelSnd;
    HLTSOUND        m_hVehicleDecelSnd;
    HLTSOUND        m_hVehicleCruiseSnd;
	HLTSOUND		m_hVehicleImpactSnd;
	HLTSOUND		m_hIdleSnd;
	HLTSOUND		m_hBrakeSnd;

	VEHICLENODE		m_VehicleNode1;
	VEHICLENODE		m_VehicleNode2;
	VEHICLENODE		m_VehicleNode3;

	HMODELSOCKET	m_hVehicleAttachSocket;

	LTVector		m_vVehicleOffset;
	LTVector		m_vHeadOffset;

	int				m_nCurGear;
	int				m_nLastGear;
	bool			m_bTurned;
	bool			m_bHolsteredWeapon;

    LTBOOL			m_bVehicleStopped;
    LTBOOL          m_bVehicleAtMaxSpeed;
    LTBOOL			m_bVehicleTurning;          // Are we turning (vehicle mode)
	int				m_nVehicleTurnDirection;	// Direction we are turning
 
	CCameraOffsetMgr	m_VehicleModelOffsetMgr; // Adjust vehicle model orientation

	LTVector		m_vVehiclePYR;

	float			m_fContourRoll;

	SurfaceType		m_eSurface;
	SurfaceType		m_eLastSurface;

	LTFLOAT			m_fYawDiff;
	LTFLOAT			m_fYawDelta;	
	LTBOOL			m_bKeyboardTurning;
	LTFLOAT			m_fHandlebarRoll;
	LTFLOAT			m_fHeadYaw;

	enum TurnDirection { TD_LEFT, TD_CENTER, TD_RIGHT };
	TurnDirection	m_eMouseTurnDirection;

	uint32			m_nPlayerLureId;
	CTimer			m_LureTimeToNextBump;
	CTimer			m_LureTimeOverBump;
	float			m_fLureBumpHeight;
	float			m_fLureBobParameter;

	bool			m_bResetLure;
	LTObjRef		m_hCurJumpVolume;

	LTBOOL			m_bHaveJumpVolumeVel;
	float			m_fJumpVolumeVel;

	bool			m_bSetLastAngles;
	LTVector		m_vLastVehiclePYR;
	LTVector		m_vLastPlayerAngles;
	LTVector		m_vLastCameraAngles;

	bool			m_bWasTouched;
	CollisionInfo	m_cLastTouchInfo;
};


#endif  // __VEHICLE_MGR_H__