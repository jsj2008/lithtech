// ----------------------------------------------------------------------- //
//
// MODULE  : CVehicleMgr.cpp
//
// PURPOSE : Client side vehicle movement mgr - Definition
//
// CREATED : 6/12/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_MGR_H__
#define __VEHICLE_MGR_H__

#include "iltclient.h"
#include "SharedMovement.h"
#include "CameraOffsetMgr.h"
#include "Timer.h"

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

	void		OnEnterWorld();
	void		OnExitWorld();

	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()	const { return m_dwControlFlags; }
 
	LTBOOL      IsVehiclePhysics();
	LTBOOL      IsTurning()				const { return (m_bVehicleTurning && !m_bVehicleStopped); }
	int			GetTurnDirection()		const { return m_nVehicleTurnDirection; }

	PlayerPhysicsModel GetPhysicsModel()	const { return m_ePPhysicsModel; }

	void		CalculateVehicleRotation(LTVector & vPlayerPYR, LTVector & vPYR, LTFLOAT fYawDelta);

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

  protected:

	void		UpdateMotorcycleControlFlags();
	void		UpdateMotorcycleGear();
	void		UpdateMotorcycleDials();

	void		UpdateVehicleMotion();
	void		UpdateVehicleGear();
	void		UpdateVehicleSounds();
	char*		GetIdleSnd();
	char*		GetBrakeSnd();
	char*		GetAccelSnd();
	char*		GetDecelSnd();
	char*		GetImpactSnd(LTFLOAT fCurVelocityPercent, LTBOOL bHitCharacter);
	
	LTFLOAT		GetVehicleAccelTime();
	LTFLOAT		GetVehicleMinTurnAccel();
	LTFLOAT		GetVehicleMaxDecel();
	LTFLOAT		GetVehicleDecelTime();
	LTFLOAT		GetVehicleMaxBrake();
	LTFLOAT		GetVehicleBrakeTime();

	void		UpdateSnowmobileControlFlags();
	void		UpdateSnowmobileGear();
	void		UpdateSnowmobileHandleBars();

	void        KillAllVehicleSounds(HLTSOUND hException=LTNULL);
	
	void		PlayVehicleAni(char* pAniName, LTBOOL bReset=LTTRUE, LTBOOL bLoop=LTFALSE);
	LTBOOL		IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone);

	LTBOOL		PreSetPhysicsModel(PlayerPhysicsModel eModel);

	void		SetMotorcyclePhysicsModel();
	void		SetSnowmobilePhysicsModel();
	void		SetNormalPhysicsModel();

	void		CreateVehicleModel();
	void		CreateVehicleAttachModel();
	void		UpdateVehicleModel();
	void		UpdateVehicleAttachModel();
	void		UpdateVehicleFriction(LTFLOAT fSlideToStopTime);

	void		ShowVehicleModel(LTBOOL bShow=LTTRUE);

	LTFLOAT		GetSurfaceVelMult(SurfaceType eType) const;

  protected :

	HOBJECT		m_hVehicleModel;
	HOBJECT		m_hVehicleAttachModel;
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

    LTBOOL			m_bVehicleStopped;
    LTBOOL          m_bVehicleAtMaxSpeed;
    LTBOOL			m_bVehicleTurning;          // Are we turning (vehicle mode)
	int				m_nVehicleTurnDirection;	// Direction we are turning
 
	CCameraOffsetMgr	m_VehicleModelOffsetMgr; // Adjust vehicle model orientation

	LTFLOAT			m_fVehiclePitch;
	LTFLOAT			m_fVehicleYaw;
	LTFLOAT			m_fVehicleRoll;

	SurfaceType		m_eSurface;
	SurfaceType		m_eLastSurface;

	LTFLOAT			m_fYawDiff;
	LTFLOAT			m_fYawDelta;	
	LTBOOL			m_bKeyboardTurning;
	LTFLOAT			m_fHandlebarRoll;
	LTFLOAT			m_fHeadYaw;

	enum TurnDirection { TD_LEFT, TD_CENTER, TD_RIGHT };
	TurnDirection	m_eMouseTurnDirection;
};


#endif  // __VEHICLE_MGR_H__