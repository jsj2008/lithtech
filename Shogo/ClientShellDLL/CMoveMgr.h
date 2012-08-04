
// Client side movement manager for Shogo.

#ifndef __CMOVEMGR_H__
#define __CMOVEMGR_H__

	
	#include "clientheaders.h"
	#include "ContainerCodes.h"
	#include "SharedMovement.h"
	#include "ModelFuncs.h"
#include "iltmath.h"
#include "iltcommon.h"
	
	class CPhysicsLT;
	class CRiotClientShell;

	
	class CContainerInfo
	{
	public:
		LTFLOAT			m_fGravity;
		LTVector			m_Current;
		ContainerCode	m_ContainerCode;
		LTBOOL			m_bHidden;
	};

	
	class CMoveMgr
	{
	public:

					CMoveMgr(CRiotClientShell *pShell);
					~CMoveMgr();
		
		LTBOOL		Init(ILTClient *pClientDE);
		
		void		Update();
		void		PostUpdate();
		void		OnPhysicsUpdate(ILTMessage_Read* hRead);
		void		UpdateMouseStrafeFlags(LTFLOAT *pAxisOffsets);
		void		UpdateAxisMovement(LTBOOL bUseAxisForwardBackward, LTFLOAT fForwardBackward, LTFLOAT fForwardBackwardDeadZone,
									   LTBOOL bUseAxisLeftRight,LTFLOAT fLeftRight, LTFLOAT fLeftRightDeadZone );

		LTRESULT		OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos);
		LTRESULT		OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot);

		void		OnObjectRemove(HOBJECT hObj);

		// CMoveMgr keeps a list of spheres that repel the player object.
		// These are created from explosions.
		LTRESULT		AddPusher(LTVector &pos, LTFLOAT radius, 
			LTFLOAT startDelay, LTFLOAT duration, LTFLOAT strength);

		HOBJECT		GetObject() {return m_hObject;}

		void		SetSpectatorMode(LTBOOL bSet);
		void		OnTractorBeamPos(ILTMessage_Read* hRead);
		void		OnServerForcePos(ILTMessage_Read* hRead);

		void		WritePositionInfo(ILTMessage_Write* hWrite);

	protected:

		void		ShowPos(char *pBlah);
		void		UpdateControlFlags();
		void		UpdateMotion();
		void		UpdatePushers();
		void		UpdatePlayerAnimation();
		
		LTBOOL		AreDimsCorrect();
		void		ResetDims(LTVector *pOffset=NULL);
		
		void		UpdateFriction();
		
		void		UpdateOnLadder(CContainerInfo *pInfo);
		void		UpdateInLiquid(CContainerInfo *pInfo);
		void		UpdateOnGround( LTVector *pvAccel );
		
		void		MoveLocalSolidObject();
		void		UpdateVelMagnitude();
		void		SetClientObjNonsolid();
		void		MoveToClientObj();

		LTBOOL		IsMecha();
		LTBOOL		IsBodyInLiquid();
		LTBOOL		IsDead();

		LTBOOL		IsInStillAnim();

		
		uint8		m_ClientMoveCode;

		// The object representing our movement.  
		HOBJECT		m_hObject;

		ILTClient			*m_pClientDE;
		ILTPhysics			*m_pPhysicsLT;
		CRiotClientShell	*m_pClientShell;

		LTFLOAT		m_DimsScale[NUM_MODELSIZES];
		LTLink		m_Pushers;


		LTVector		m_WantedDims;

		// Tractor beam info.
		LTVector		m_TBPos;
		LTFLOAT		m_TBSpeed;
		LTBOOL		m_bTBOn;
		
		// Movement state.
		uint32		m_dwControlFlags;
		uint32		m_dwLastControlFlags;
		uint32		m_nMouseStrafeFlags;

		LTBOOL		m_bBodyInLiquid;
		LTBOOL		m_bSwimmingOnSurface;
		LTBOOL		m_bCanSwimJump;
		LTFLOAT		m_fSwimmingOnSurfaceStart;
		
		LTBOOL		m_bBodyOnLadder;
		LTBOOL		m_bSpectatorMode;
		LTBOOL		m_bForceJump;

		LTBOOL		m_bOnGround;
		
		LTFLOAT		m_fBaseMoveAccel;
		LTFLOAT		m_fMoveAccelMultiplier;

		LTFLOAT		m_fLeashLen;
		LTFLOAT		m_FrameTime;


		// Movement speeds.
		LTFLOAT		m_fJumpVel;
		LTFLOAT		m_fJumpMultiplier;

		LTFLOAT		m_fSwimVel;
		LTFLOAT		m_fWalkVel;
		LTFLOAT		m_fRunVel;
		LTFLOAT		m_fLadderVel;
		LTFLOAT		m_fMoveMultiplier;

		// special movement axis information
		LTBOOL		m_bUseAxisLeftRight;
		LTBOOL		m_bUseAxisForwardBackward;
		LTFLOAT		m_fAxisLeftRightVel; // Right is positive
		LTFLOAT		m_fAxisLeftRightDeadZone; 
		LTFLOAT		m_fAxisForwardBackwardVel; // Forward is positive ?????????
		LTFLOAT		m_fAxisForwardBackwardDeadZone; 
		
		LTBOOL		m_bSwimmingJump;

		ContainerCode	m_eLastContainerCode;
		CContainerInfo	m_Containers[MAX_TRACKED_CONTAINERS];
		uint32			m_nContainers;

		// Still animations.
		char			m_StillAnimNames[MAX_STILL_ANIMATIONS][MAX_STILL_ANIM_NAME_LEN];
		HMODELANIM		m_hStillAnims[MAX_STILL_ANIMATIONS];
		uint32			m_nStillAnims;
	};


#endif  // __CMOVEMGR_H__


