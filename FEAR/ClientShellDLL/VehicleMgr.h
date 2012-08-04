// ----------------------------------------------------------------------- //
//
// MODULE  : CVehicleMgr.h
//
// PURPOSE : Client side vehicle movement mgr - Definition
//
// CREATED : 6/12/00
//
// (c) 2000-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_MGR_H__
#define __VEHICLE_MGR_H__

#include "iltclient.h"
#include "SharedMovement.h"
#include "CameraOffsetMgr.h"
#include "SurfaceDB.h"
#include "LTObjRef.h"

struct VEHICLE;

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

    bool      Init();

	void		UpdateModels();

	float		GetVehicleMovementPercent() const;

	void		OnEnterWorld();
	void		OnExitWorld();
	
	void		OnTouchNotify(CollisionInfo *pInfo, float forceMag);

    uint32      GetControlFlags()	const { return m_dwControlFlags; }
 
	bool		CanShowCrosshair()	const;
	bool      IsVehiclePhysics();
	bool		CanDismount();
	bool      IsTurning()				const { return (m_bVehicleTurning && !m_bVehicleStopped); }
	int			GetTurnDirection()		const { return m_nVehicleTurnDirection; }
	bool		AllowHeadBobCant()		const;

	PlayerPhysicsModel GetPhysicsModel()	const { return m_ePPhysicsModel; }

	void		GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot);
	void		HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat);

	void		InitWorldData();

	void		UpdateControlFlags();
	void		UpdateMotion();
	void		UpdateFriction();
	void		UpdateSound();

 	void		TermLevel();
	
	CCameraOffsetMgr* GetModelOffsetMgr() { return &m_VehicleModelOffsetMgr; }

	void		SetPhysicsModel( PlayerPhysicsModel eModel, bool bDoPreSet = true );

	void		AdjustCameraRoll(float & fRoll);

	void		SetPlayerLureId( uint32 nPlayerLureId );
	uint32		GetPlayerLureId( ) { return m_nPlayerLureId; }

	float		GetVehicleContourRoll() const { return IsVehicleModel( m_ePPhysicsModel ) ? m_fContourRoll : 0.0f; }

	// Called by vehiclemgr.
	bool		MoveLocalSolidObject( );

	// Command handling functions
	bool		OnCommandOn(int command);
	bool		OnCommandOff(int command);
	void		AlignAngleToAxis(float &fAngle);
	void		SnapLightCycleToXZGrid(LTVector &vNewPos, LTVector &vOldPos, uint32 nGridSize);
	
	void		MoveVehicleObject( LTVector &vPos );

	// Serialization
	void		Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	// Is it safe to save?
	bool		CanSave() const { return m_bHaveJumpVolumeVel == false; }

	float     GetMaxVelMag(float fMult=1.0f) const;
    float     GetMaxAccelMag(float fMult=1.0f) const;

	void		SaveVehicleInfo();

	void		UpdatePlayerLure( );

  protected:

	void		UpdateGear();
	
	const char*	GetIdleSnd();
	const char*	GetBrakeSnd();
	const char*	GetAccelSnd();
	const char*	GetDecelSnd();
	const char*	GetImpactSnd(float fCurVelocityPercent, HSURFACE hSurface);
	
	float		GetAccelTime() const;
	float		GetMinTurnAccel()const;
	float		GetMaxDecel() const;
	float		GetDecelTime() const;
	float		GetMaxBrake() const;
	float		GetBrakeTime()const;

	void		UpdateVehicleHandleBars();

	void		UpdateLureMotion();

	void        KillAllVehicleSounds(HLTSOUND hException=NULL);
	
	bool		IsCurVehicleAni(const char* pAniName, bool & bIsDone);

	bool		PreSetPhysicsModel(PlayerPhysicsModel eModel);
			
	void		SetPlayerLurePhysicsModel( );
	void		SetNormalPhysicsModel( );

	void		CreateVehicleModel();

	void		ShowVehicleModel(bool bShow=true);

	float		GetSurfaceVelMult(SurfaceType eType) const;

	bool		MoveToLure( );
	bool		MoveLightCycle();

	bool		HandleJumpVolumeTouch( HOBJECT hJumpVolume );
	bool		HandleCharacterTouch( HOBJECT hCharacter );

	void		UpdateInJumpVolume();
	void		EnableWeapon( bool bEnable, bool bToggleHolster = true );
	
	void		UpdateContainerMotion();
	
  protected :

	HOBJECT		m_hVehicleModel;
    float		m_fAccelStart;
    float     m_fDecelStart;

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

    bool			m_bVehicleStopped;
    bool          m_bVehicleAtMaxSpeed;
    bool			m_bVehicleTurning;          // Are we turning (vehicle mode)
	int				m_nVehicleTurnDirection;	// Direction we are turning
 
	CCameraOffsetMgr	m_VehicleModelOffsetMgr; // Adjust vehicle model orientation

	LTVector		m_vVehiclePYR;

	float			m_fContourRoll;

	SurfaceType		m_eSurface;
	SurfaceType		m_eLastSurface;

	float			m_fYawDiff;
	float			m_fYawDelta;	
	bool			m_bKeyboardTurning;
	float			m_fHandlebarRoll;
	float			m_fHeadYaw;

	enum TurnDirection { TD_LEFT, TD_CENTER, TD_RIGHT };
	TurnDirection	m_eMouseTurnDirection;

	uint32			m_nPlayerLureId;
	float			m_fLureBumpHeight;
	float			m_fLureBobParameter;

	bool			m_bResetLure;
	bool			m_bResetLureFromSave;
	LTRigidTransform m_ResetLureTransform;

	LTObjRef		m_hCurJumpVolume;

	bool			m_bHaveJumpVolumeVel;
	float			m_fJumpVolumeVel;

	bool			m_bSetLastAngles;
	LTVector		m_vLastVehiclePYR;
	LTVector		m_vLastPlayerAngles;
	LTVector		m_vLastCameraAngles;

	bool			m_bWasTouched;
	CollisionInfo	m_cLastTouchInfo;

	LTRigidTransform m_rtSavedEncodedTransform;
	bool			 m_bHasSavedEncodedTransform;

private:

	PREVENT_OBJECT_COPYING(CVehicleMgr);
};


#endif  // __VEHICLE_MGR_H__