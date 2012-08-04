
// Client side movement manager for Blood 2.

#ifndef __MOVEMGR_H__
#define __MOVEMGR_H__


#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f


	
#include "client_de.h"
#include "ContainerCodes.h"
#include "SharedMovement.h"
#include "SharedDefs.h"
//	#include "ModelFuncs.h"
enum ModelSize { MS_NORMAL=0, MS_SMALL, MS_LARGE, NUM_MODELSIZES };

	
	class CPhysicsLT;
	class CBloodClientShell;

	
	class CContainerInfo
	{
	public:
		float			m_fGravity;
		DVector			m_Current;
		ContainerCode	m_ContainerCode;
		DBOOL			m_bHidden;
	};

	
	class CMoveMgr
	{
	public:

					CMoveMgr(CBloodClientShell *pShell);
					~CMoveMgr();
		
		DBOOL		Init(ClientDE *pClientDE);
		
		void		Update();
		void		OnPhysicsUpdate(HMESSAGEREAD hRead);

		DRESULT		OnObjectMove(HOBJECT hObj, DBOOL bTeleport, DVector *pPos);
		DRESULT		OnObjectRotate(HOBJECT hObj, DBOOL bTeleport, DRotation *pNewRot);

		void		OnObjectRemove(HOBJECT hObj);

		// CMoveMgr keeps a list of spheres that repel the player object.
		// These are created from explosions.
		DRESULT		AddPusher(DVector &pos, float radius, 
			float startDelay, float duration, float strength);

		HOBJECT		GetObject() {return m_hObject;}

		void		OnServerForcePos(HMESSAGEREAD hRead);

		void		WritePositionInfo(HMESSAGEWRITE hWrite);
		DDWORD		GetControlFlags() { return m_dwControlFlags; }

		void		ClearControlFlags() { m_dwControlFlags = 0; }

		void		UpdateAxisMovement(DBOOL bUseAxisForwardBackward, float fForwardBackward, float fForwardBackwardDeadZone,
									   DBOOL bUseAxisLeftRight,float fLeftRight, float fLeftRightDeadZone );

	protected:

		void		ShowPos(char *pBlah);
		void		UpdateControlFlags();
		void		UpdateMotion();
		void		UpdatePushers();
		void		UpdatePlayerAnimation();
		
		DBOOL		AreDimsCorrect();
		void		ResetDims(DVector *pOffset=NULL);
		
		void		UpdateFriction();
		
		void		UpdateOnLadder(CContainerInfo *pInfo);
		void		UpdateOnConveyor(CContainerInfo *pInfo);
		void		UpdateInLiquid(CContainerInfo *pInfo);
		void		UpdateOnGround(CollisionInfo *pInfo);
		
		void		MoveLocalSolidObject();
		void		UpdateVelMagnitude();
		void		SetClientObjNonsolid();
		void		MoveToClientObj();

		DBOOL		IsBodyInLiquid();
		DBOOL		IsDead();

		
		DBYTE		m_ClientMoveCode;

		// The object representing our movement.  
		HOBJECT		m_hObject;

		ClientDE			*m_pClientDE;
		CPhysicsLT			*m_pPhysicsLT;
		CBloodClientShell	*m_pClientShell;

		float		m_DimsScale[NUM_MODELSIZES];
		DLink		m_Pushers;


		DVector		m_WantedDims;

		// Movement state.
		DDWORD		m_dwControlFlags;
		DDWORD		m_dwLastControlFlags;
		DDWORD		m_nMouseStrafeFlags;
		
		DBOOL		m_bBodyInLiquid;
		DBOOL		m_bSwimmingOnSurface;
		DBOOL		m_bCanSwimJump;
		float		m_fSwimmingOnSurfaceStart;
		
		DBOOL		m_bBodyOnLadder;
		DBOOL		m_bBodyOnConveyor;

		DBOOL		m_bSpectatorMode;
		DBOOL		m_bForceJump;

		DBOOL		m_bOnGround;
		
		float		m_fBaseMoveAccel;
		float		m_fMoveAccelMultiplier;

		float		m_fLeashLen;
		float		m_FrameTime;


		// Movement speeds.
		float		m_fJumpVel;
		float		m_fJumpMultiplier;

		float		m_fMoveVel;
		float		m_fLadderVel;
		float		m_fSwimVel;
		float		m_fMoveMultiplier;
		float		m_fFrictionCoeff;
		
		DBOOL		m_bSwimmingJump;

		ContainerCode	m_eLastContainerCode;
		CContainerInfo	m_Containers[MAX_TRACKED_CONTAINERS];
		DDWORD			m_nContainers;

		// special movement axis information
		DBOOL		m_bUseAxisLeftRight;
		DBOOL		m_bUseAxisForwardBackward;
		float		m_fAxisLeftRightVel; // Right is positive
		float		m_fAxisLeftRightDeadZone; 
		float		m_fAxisForwardBackwardVel; // Forward is positive ?????????
		float		m_fAxisForwardBackwardDeadZone; 

		// Still animations.
		char			m_StillAnimNames[MAX_STILL_ANIMATIONS][MAX_STILL_ANIM_NAME_LEN];
		HMODELANIM		m_hStillAnims[MAX_STILL_ANIMATIONS];
		DDWORD			m_nStillAnims;

		// Player attributes
		DBOOL		m_bFalling;
		DBOOL		m_bForcedCrouch;
		DBOOL		m_bMovementBlocked;
		DFLOAT		m_startFall;
		DBOOL		m_bLastJumpCommand;
		DBOOL		m_bPowerupActivated;
		DBOOL		m_bLastCrouchCommand;
		DBOOL		m_bSlowMode;
		DFLOAT		m_fSlowTime;

		D_WORD		m_wLastChangeFlags;

		SurfaceType	m_eLastSurface;
		
		DVector		m_vServerVelocity;
	};


#endif  // __MOVEMGR_H__


