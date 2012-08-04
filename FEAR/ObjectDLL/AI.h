// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_H__
#define __AI_H__

#include "Character.h"
#include "AISounds.h"
#include "AIEnumAIWeaponTypes.h"
#include "AIDB.h"
#include "AINavigationMgr.h"
#include "AIMovement.h"
#include "AIConfig.h"
#include "AIEnumAIAwareness.h"
#include "PrefetchUtilities.h"

// Forward declarations.
class CAIGoalMgr;
class CAIState;
class CAITarget;
class CAIBrain;
class CAnimationContext;
class CAIMovement;
class CAIPathKnowledgeMgr;
class CAIStimulusRecord;
class CAINodeTrackerContext;
struct AIDB_AttributesRecord;

class CAttachmentsPlugin;

class CAIWorkingMemory;
class CAIBlackBoard;
class CAISensorMgr;
class CAIWorldState;
class CAINavigationMgr;
class CAIWeaponMgr;
class CAICommandMgr;
class CAIPlan;
class CTimedSystem;

// Defines

LINKTO_MODULE( AI );


/*
enum EnumAIAwareness
{
	kAware_Relaxed,		// AIVolume StimulusMasks enabled, narrow FOV.
	kAware_Suspicious,	// AIVolume StimulusMasks disabled, narrow FOV, searches. 
	kAware_Alert,		// AIVolume StimulusMasks disabled, wide FOV, searches. 
};*/

// Classes

class CAI : public CCharacter
{
	typedef CCharacter super;

	public : // Public methods

		DEFINE_CAST( CAI );

		// Ctors/Dtors/etc

		CAI();
		virtual ~CAI();

		void	SetNextServerUpdate( float fDelta );
		void	SetUpdateAI( bool bUpdate ) { m_bUpdateAI = bUpdate; }

		HOBJECT	GetSensingObject() { return m_hObject; }
		bool	IsSensing() { return !IsDead(); }

		float	GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		double	GetNextSenseUpdate() const { return m_fNextSenseUpdate; }
		void	SetNextSenseUpdate(double fTime) { m_fNextSenseUpdate = fTime; }

		void	UpdateSensingMembers();

		bool	GetDoneProcessingStimuli() const;
		void	SetDoneProcessingStimuli( bool bDone );
		void	ClearProcessedStimuli();
		bool	ProcessStimulus(CAIStimulusRecord* pRecord);

		int		GetIntersectSegmentCount() const;
		void	ClearIntersectSegmentCount();
		void	IncrementIntersectSegmentCount();

		bool	HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		void	HandleSenses(uint32 nCycle);
		void	HandleInitWeapon();

		virtual void	HandleMeleeBlocked();
		virtual void	HandleMeleeBlockedAttacker();

		void	HandleBerserkerKicked();
		void	HandleFinishingMove( HRECORD hSyncAction );

		void	HandleDamagedPlayer( HOBJECT hPlayer );

		virtual void HandleCharacterRemoval();

		void	AttachToObject( HOBJECT hObject );
		void	DetachFromObject( HOBJECT hObject );

		// Routes straight to AIWeaponMgr

		void	HandleArsenalChange();
		void	SetCurrentWeapon(ENUM_AIWeaponType eWeaponType);
		void	HandleWeaponBroke(HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon);

		uint32  GetCurSenseFlags() const;

		const LTVector& GetSensingPosition() const { return m_vPos; }

		CRange<int>& GetSightGridRangeX() { return m_rngSightGridX; }
		CRange<int>& GetSightGridRangeY() { return m_rngSightGridY; }

		// Object info

		const char* GetName() const;

		const LTVector& GetPosition() const { return m_vPos; }
		virtual float GetVerticalThreshold() const;
        
        LTVector		GetWeaponPosition( const CWeapon *pWeapon, bool bForceCalculation);
		LTVector		GetWeaponForward( const CWeapon* pWeapon);

        const LTVector& GetEyePosition();
        const LTVector& GetEyeForward();

		const LTVector& GetTorsoRight();
        const LTVector& GetTorsoForward();

        const LTVector& GetDims() const { return m_vDims; }
        virtual float GetRadius() { return m_fRadius; }

        const LTVector& GetUpVector() const { return m_pAIMovement->GetUp(); }
        const LTVector& GetRightVector() const { return m_pAIMovement->GetRight(); }
        const LTVector& GetForwardVector() const { return m_pAIMovement->GetForward(); }
		const LTVector& GetTargetForward() const { return m_pAIMovement->GetTargetForward(); } 

		// Visiblity. Note: these functions can fill in useful values (distance, dot product, direction) of the point/object in question,
		// in order to save you from having to recompute them yourself (since they can be expensive). HOWEVER, you cannot use the values
		// if the point/object is NOT visible (since it may have skipped out of the function before comuting some of the values). ALSO,
		// in the case of checking to see if an object is visible, there is a degenerate case of the Source Position being INSIDE the
		// object, which does return TRUE, although the values will be somewhat meaningless. They will be fudged to a distance of MATH_EPSILON,
		// a dot product of 1, and a direction of the forward vector of the AI.

		bool IsInsideFOV(float flDistanceSqr, const LTVector& vDirNorm);
        bool IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, const LTVector& vObjectPosition, float fVisibleDistanceSqr, bool bFOV, bool bDoVolumeCheck, HOBJECT* phBlockingObject = NULL, float* pfDistanceSqr = NULL);

		// Query.

		bool CanSearch();
		bool RequestCrouch(HOBJECT hRequestingAI);
		bool RequestDodge(HOBJECT hRequestingAI);

		// AIPlan methods

		void SetAIPlan( CAIPlan* pAIPlan );

		// State methods

		CAIState* GetState() { return m_pState; }
		CAIState* AI_FACTORY_NEW_State(EnumAIStateType eStateType);
		void ClearState();
		void SetState(EnumAIStateType eState, bool bUnlockAnimation = true);

		// Goal methods.

		CAIGoalMgr* GetGoalMgr() { return	m_pGoalMgr;	}

		// Brain

		CAIBrain* GetBrain() { return m_pBrain; }

		// Enemy Target stuff

		CAITarget* GetTarget();
        bool HasTarget( unsigned int dwTargetFlags );
		
		// Awareness

		void			IncrementAlarmLevel(uint32 nIncr);
		bool			IsMajorlyAlarmed();
		bool			IsImmediatelyAlarmed();
		uint32			GetAlarmLevel() const { return m_nAlarmLevel; }

		// Accuracy methods

        float	GetAccuracy();
		float	GetFullAccuracyRadiusSqr() const { return m_fFullAccuracyRadiusSqr; }
		float	GetAccuracyMissPerturb();
		float	GetMaxMovementAccuracyPerturb();
		float	GetMovementAccuracyPerturbDecay() const { return m_fMovementAccuracyPerturbDecay; }

		// Activation

		void SetCanTalk( bool bCanTalk ) { m_bCanTalk = bCanTalk; }
		void SetPreserveActivateCmds( bool bPreserve ) { m_bPreserveActiveCmds = bPreserve; }
		bool HasActivateCommand() const { return ( !m_strCmdActivateOn.empty() || !m_strCmdActivateOff.empty() ); }
		void ClearActivateOnCommand() { m_strCmdActivateOn.clear(); }
		void ClearActivateOffCommand() { m_strCmdActivateOff.clear(); }
		void UpdateUserFlagCanActivate();

		// Static filter functions for intersect segments

        static bool DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static bool ShootThroughFilterFn(HOBJECT hObj, void *pUserData);
        static bool ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint);
        static bool SeeThroughFilterFn(HOBJECT hObj, void *pUserData);
        static bool SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint);

        bool CanSeeThrough() { return m_bSeeThrough; }
        bool CanShootThrough() { return m_bShootThrough; }

		// Orientation/movement methods

		void   SetCheapMovement(bool bCheap) { m_bCheapMovement = bCheap; }
		void   SetForceGround( bool bForce ) { m_bForceGround = bForce; }

		virtual void	SetMoveToFloor( bool bValue );

		// Sound methods

		virtual void PlaySound(const char *pSound) { super::PlaySound(pSound); }
		virtual const char* GetDeathSound();
		virtual const char* GetDeathSilentSound();
		virtual const char* GetDamageSound(DamageType eType);
		virtual void PlayDamageSound(DamageType /*eType*/) {}
        virtual bool CanLipSync();

		// Attachments

		virtual void CreateAttachments();

		// Trigger methods.

		void Activate();
		void Cineract(const char* szAnim, bool bLoop);
		virtual void HideCharacter(bool bHide);

		// Configuration

		void SetDamagedPlayerCommand( const char* pszCmdDamagedPlayer, int nDamagedPlayerActivationCount );

		// Hitpoints

		void BoostHitpoints(float fFactor)
		{
			m_damage.SetHitPoints(m_damage.GetHitPoints()*fFactor);
			m_damage.SetMaxHitPoints(m_damage.GetMaxHitPoints()*fFactor);
		}

		// Damage stuff

        float ComputeDamageModifier(ModelsDB::HNODE hModelNode);
		virtual	HMODELANIM	GetAlternateDeathAnimation();

		void MakeInvulnerable( bool bMakeInvulerable );
		bool IsInvulnerable() const { return m_bInvulnerable; }

		// Death stuff

		virtual void PlayDeathSound();
		virtual HMODELANIM GetDeathAni(bool bFront);
		const char* GetCrouchDeathAni();
		const char* GetProneDeathAni();
		bool WasSilentKill();
		virtual void PrepareToSever();

		// Animation

		CAnimationContext* GetAnimationContext() { return m_pAnimationContext; }
		void SetAnimObject( HOBJECT hAnimObject ) { m_hAnimObject = hAnimObject; }
		HOBJECT GetAnimObject() const { return m_hAnimObject; }

		float GetWalkSpeed() { return m_fWalkVel; }
		float GetRunSpeed() { return m_fRunVel; }
		float GetJumpSpeed() { return m_fJumpVel; }
		float GetJumpOverSpeed() { return m_fJumpOverVel; }
		float GetFallSpeed() { return m_fFallVel; }
		float GetSwimSpeed() { return m_fSwimVel; }

		// Movement

		void	Move(const LTVector& vPos);
		bool	FindFloorHeight(const LTVector& vPos, float* pfFloorHeight);
		CAIMovement* GetAIMovement() { return m_pAIMovement; }
		void	SyncPosition() { m_bSyncPosition = true;	}

		// Doors

		void OpenDoor();
		void KickDoor();

		// Path knowledge

		CAIPathKnowledgeMgr* GetPathKnowledgeMgr() const { return m_pPathKnowledgeMgr; }

		// Dialogue object

		bool IsControlledByDialogue() const { return !!m_hDialogueObject; }
		void StopDialogue()
		{
			if( m_hDialogueObject )
			{
				g_pCmdMgr->QueueMessage( this, g_pLTServer->HandleToObject( m_hDialogueObject ), "OFF" );
				m_hDialogueObject = NULL;
			}
		}

		void LinkDialogueObject(HOBJECT hDialogueObject);
		void UnlinkDialogueObject(HOBJECT hDialogueObject);

		// Client

		void UpdateCharacterFXBodyState();

		void SetClientSolid(bool bClientSolid) 
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, bClientSolid ? USRFLG_AI_CLIENT_SOLID : 0, USRFLG_AI_CLIENT_SOLID);
		}

		virtual void StartDeath();

		void SetUnconscious(bool bUnconscious);

		// damage filtering 
		void			SetDamageMask( char const* pszMask );

		// prefetching
		static void		GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

		// Get the complete list of all CAI's.
		typedef std::vector< CAI*, LTAllocator< CAI*, LT_MEM_TYPE_OBJECTSHELL> > AIList;
		static AIList const& GetAIList( ) { return m_lstAIs; }

		// Functions for handling command scripts from the CommandMgr...
		
		// Tests to see if the object is still being scripted from a message...
		virtual bool	IsScripted( );

		// Do some cleanup once an object as finished scripting...
		virtual void	ScriptCleanup( );

		// Setup a finishing move object for the player...
		virtual void	PlayerFinishingMoveSetup( );

		// Do some cleanup once a player finishing move is no longer wanted...
		virtual void	PlayerFinishingMoveCleanup( );

		// The object was interrupted while scripting so make sure it can exit gracefully...
		virtual void	InterruptScript( );


		CAIWorkingMemory*	GetAIWorkingMemory() { return m_pAIWorkingMemory; }
		CAIBlackBoard*		GetAIBlackBoard() { return m_pAIBlackBoard; }
		CAISensorMgr*		GetAISensorMgr() { return m_pAISensorMgr; }
		CAIWeaponMgr*		GetAIWeaponMgr() { return m_pAIWeaponMgr; }
		CAINavigationMgr*	GetAINavigationMgr() { return m_pAINavigationMgr; }
		CAIWorldState*		GetAIWorldState() { return m_pAIWorldState; }
		CAIPlan*			GetAIPlan() { return m_pAIPlan; }

		virtual bool		IsCrouched();
		virtual bool		IsKnockedDown();

		// The teamid the AI is on.
		virtual uint8	GetTeamID( ) const { return m_nTeamId; }
		void SetTeamID( uint8 nTeamId ) { m_nTeamId = nTeamId; }

	protected : // Protected methods

		// Update methods

		void PreUpdate();
		void Update();
		virtual void	UpdateDead( bool bCanBeRemoved );
		void PostUpdate();

		virtual void UpdateAnimation();
		virtual void UpdateOnGround();
		virtual void UpdateTarget();
		virtual void UpdateMovement();
		virtual void UpdatePosition();
		virtual void UpdateNodes();
		void		 UpdateAwareness();

		void UpdateInfo();

		// Handler methods

		virtual void HandleTeleport(const LTVector& vPos);
		virtual void HandleTeleport(TeleportPoint* pTeleportPoint);
		virtual void HandleTouch( HOBJECT hTouched );
		virtual void HandleDamage(const DamageStruct& damage);
		virtual void HandleModelString(ArgList* pArgList, ANIMTRACKERID nTrackerId);
		virtual void HandleNavMeshPolyEnter(ENUM_NMPolyID ePoly);
		virtual void HandleNavMeshPolyExit(ENUM_NMPolyID ePoly);
		virtual void HandleAIRegionEnter(ENUM_AIRegionID eAIRegion);
		virtual void HandleAIRegionExit(ENUM_AIRegionID eAIRegion);

		// Get the User-Flag version of our surface type
		virtual uint32	GetUserFlagSurfaceType() const;
		virtual void InitAnimation();

		void		 RemoveFromAIRegions();

		// Parent methods
		virtual void	PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// Engine methods

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		bool	ReadProp(const GenericPropList *pProps);
		void	PostReadProp(ObjectCreateStruct *pStruct);

		virtual void	InitialUpdate();
        virtual void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	AddWeaponToAI( HRECORD hWeapon, const char* pszSocketName );
		virtual void RemoveWeapon( CActiveWeapon* pActiveWeapon );

		// Brain methods

		bool	SetBrain( char const* pszBrain );


		// Sets the AIs position

		void SetPosition(const LTVector& vPos, bool bFindFloorHeight);

		virtual void DropWeapons();

		// Drops gear items when the character dies... 
		virtual void SpawnGearItemsOnDeath( );


	protected : // Member Variables

		// Misc

        bool      m_bCheapMovement;           // Are we using cheap movement?
        bool      m_bFirstUpdate;             // Is this before/during our first update?
		float	  m_fNextUpdateRate;
		bool		m_bUpdateAI;				// Should the AI update?
		bool		m_bIsCinematicAI;			// Are an AI in a cinematic

		// Default Attachments

		bool			m_bUseDefaultAttachments;
		bool			m_bUseDefaultWeapons;
		ENUM_AIAmmoLoadRecordID	m_eAmmoLoad;

		// Brain

		CAIBrain*		m_pBrain;				// Our brain (attributes and whatnot)

		// Goals

		CAIGoalMgr*	m_pGoalMgr;				// Manages goals.

		// State

        CAIState*    m_pState;                   // Current state

		// Object information

        LTVector	m_vPos;                     // Current pos this frame

		LTVector	m_vTorsoRight;				// Torso's right vector
		LTVector	m_vTorsoForward;			// Torso's forward vector

        LTVector	m_vEyePos;                  // The position of our eye
        LTVector	m_vEyeForward;              // Our eye's forward vector

		LTVector	m_vWeaponPos;				// Our weapon position.
		LTVector	m_vWeaponForward;			// Our weapon forward vector.

        LTVector	m_vDims;                    // Object's dims
        float     m_fRadius;                  // Radius of circle circumscribed by our bounding box

		bool		m_bUpdateNodes;				// Do the nodes need updating?

		// Senses

		float						m_fSenseUpdateRate;		// How quickly we update our senses.
		double						m_fNextSenseUpdate;
		uint32						m_flagsCurSenses;
		CRange<int>					m_rngSightGridX;
		CRange<int>					m_rngSightGridY;

		// Target

        CAITarget*  m_pTarget;                  // Enemy Target
        bool      m_bShootThrough;            // Do we shoot through shoot-throughables at our target?
        bool      m_bSeeThrough;              // Do we see through see-throughables at any stimuli? (VERY EXPENSIVE!!!)

		// Accuracy/lag modififers

        float     m_fAccuracy;
        float     m_fAccuracyIncreaseRate;
		float		m_fFullAccuracyRadiusSqr;
		float		m_fAccuracyMissPerturb;
		float		m_fMaxMovementAccuracyPerturb;
		float		m_fMovementAccuracyPerturbDecay;

		// Awareness

		EnumAIAwareness		m_eLastAwareness;		// Last awareness.
		uint32				m_nAlarmLevel;			// How alarmed the AI is based on observations.
		EnumAIStimulusID	m_eAlarmStimID;			// ID of registered stimulus for suspicion due to alarm.

		// Commands

		std::string		m_strCmdInitial;			// Initial command (one-shot)
		std::string		m_strCmdActivateOn;			// ActivateOn command
		std::string		m_strCmdActivateOff;		// ActivateOff command

		// Activation

		bool		m_bCanTalk;					// Are we allowed to be activated? If false, the AI can't be activated regardless of other settings
        bool		m_bActivated;               // Is our activation ON or OFF
		bool		m_bPreserveActiveCmds;		// Should we not delete our active cmd's when used?
		bool		m_bUnconscious;

		// Animation stuff

		CAnimationContext*	m_pAnimationContext;	// Our animation context
		LTObjRef			m_hAnimObject;			// Object to interact with while animating

		// Node tracking.

		CAINodeTrackerContext*	m_pNodeTrackerContext;

		// Node handle cache

		HMODELNODE	m_hHeadNode;

		// Movement

		float		m_fJumpOverVel;				// Speed for jumping over things.

		LTVector	m_vMovePos;					// Position to move to
		bool		m_bMove;					// Are we moving or not this frame

		CAIMovement* m_pAIMovement;
		bool		m_bForceGround;				// Force AI to ground?
		bool		m_bPosDirty;				// Is our position dirty? (do we need to refind the ground)
		LTVector	m_vLastFindFloorPos;		// Last pos where we called FindFloor.
		double		m_fNextFindFloorTime;
		bool		m_bSyncPosition;			// Sync position

		// Path knowledge

		CAIPathKnowledgeMgr*	m_pPathKnowledgeMgr;	// Path knowledge.

		// Dialogue controller

		LTObjRef	m_hDialogueObject;				// Handle to our controlling Dialogue object

		// Handle commands dispatched when the AI damages the player.

		std::string m_strCmdDamagedPlayer;		// Command to be dispatched.
		int			m_nDamagedPlayerActivationCount;	// How many times the DamagedPlayer message can be fired (<= 0 is infinite)
		int			m_nDamagedPlayerNumCount;	// How many times has the DamagedPlayer been activated

		// Debug stuff

		std::string		m_cstrCurrentInfo;

		//	Stores the pathing position for the AI.  Certain AIs, such as
		//	floating AIs, use this offset to keep consistant with ground
		//	dwellers.
		LTVector	m_vPathingPosition;

		// The distance up and down the AI checks for volumes
		float		m_flVerticalThreshold;

		bool		m_bInvulnerable;

		// List of all AI objects.
		static AIList	m_lstAIs;


		CAIWorkingMemory*	m_pAIWorkingMemory;
		CAIBlackBoard*		m_pAIBlackBoard;
		CAISensorMgr*		m_pAISensorMgr;
		CAIWorldState*		m_pAIWorldState;
		CAINavigationMgr*	m_pAINavigationMgr;
		CAIWeaponMgr*		m_pAIWeaponMgr;
		CAICommandMgr*		m_pAICommandMgr;
		CAIPlan*			m_pAIPlan;

		CAIConfig			m_AIConfig;
		bool				m_bConfigured;

		//used as index into the list of items the AI may drop on death.
		static uint8		m_nDropIndex;

		// Team AI is on.
		uint8					m_nTeamId;

		// Message Handlers...
	
	public:

		DEFINE_MSG_HANDLER_PROXY_CLASS( CAICommandMgr, m_pAICommandMgr );
};

class CAIPlugin : public IObjectPlugin
{
	public:
		CAIPlugin();
		virtual ~CAIPlugin();

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin		m_CommandMgrPlugin;
		CAttachmentsPlugin		*m_pAttachmentsPlugin;
		CArsenalPlugin			m_ArsenalPlugin;
		CAIConfigPlugin			m_AIConfigPlugin;
};

#endif // __AI_H__
