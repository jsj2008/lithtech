// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.h
//
// PURPOSE : Player object definition
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CPLAYER_OBJ_H__
#define __CPLAYER_OBJ_H__

#include "Character.h"
#include "MsgIds.h"
#include "CheatDefs.h"
#include "TemplateList.h"
#include "CVarTrack.h"
#include "ClientServerShared.h"
#include "Skills.h"
#include "AnimatorPlayer.h"
#include "PlayerShared.h"
#include "ActivationData.h"
#include "SharedMission.h"
#include "SharedScoring.h"
#include "IDList.h"
#include "NetDefs.h"

LINKTO_MODULE( PlayerObj );


class CPlayerAttachments;
class CWeapon;
class PlayerVehicle;
class TeleportPoint;
class PlayerLure;
class CAI;
struct WeaponFireInfo;
enum  EnumAIStimulusID;


struct ObjectiveMsgInfo
{
	ObjectiveMsgInfo()
	:	m_nType		( 0 ),
		m_nRequest	( 0 ),
		m_dwId		( 0 )
	{
	}

	uint8	m_nType;
	uint8	m_nRequest;
	uint32	m_dwId;
};


class CPlayerObj : public CCharacter
{
	public :

		CPlayerObj();
		virtual ~CPlayerObj();

		// Attachments

		virtual void CreateAttachments();
		
		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		void SendStartLevelCommand();
		void SetClient(HCLIENT hClient);
		HCLIENT GetClient() const		{ return m_hClient; }

		void SetClientSaveData(ILTMessage_Read *pMsg);
		void BuildKeepAlives(ObjectList* pList);

		// If the update had CLIENTUPDATE_ALLOWINPUT, then it returns FALSE because nothing
		// else will be in the packet.
        LTBOOL ClientUpdate(ILTMessage_Read *pMsg);

        LTBOOL MultiplayerInit(ILTMessage_Read *pMsg);
        LTBOOL MultiplayerUpdate(ILTMessage_Read *pMsg);

		// Check if player is allowed to respawn.
		bool CanRespawn( );

		// Check if player is allowed to be revived.
		bool CanRevive( );

        void Respawn(uint8 nServerLoadGameFlags=LOAD_NEW_LEVEL, bool bGetStartPt=true);
		void Revive();
        void CancelRevive( );
		void ResetAfterDeath();
		void ResetMultiplayerSkills();
        void SetSpectatorMode(LTBOOL bOn);
        void SetInvisibleMode(LTBOOL bOn);
		void ToggleGodMode();

		// Makes player invulnerable for a period of time.
		void SetRespawnInvulnerability( );

		// Called when client is loaded.
		void SetClientLoaded( bool bClientLoaded );
		bool IsClientLoaded( ) { return m_bClientLoaded; }

		void FullAmmoCheat();
		void FullWeaponCheat();
		void FullModsCheat();
		void FullGearCheat();
		void RepairArmorCheat();
		void HealCheat();
		void GimmeGunCheat( uint8 nId );
		void GimmeModCheat( uint8 nId );
		void GimmeGearCheat( uint8 nId );
		void GimmeAmmoCheat( uint8 nId );
		void SkillsCheat();

		// Call to setup the player for new mission.
		bool SetupForNewMission( );

		virtual CAttachments*	TransferAttachments( bool bRemove ); // Responsibility of call to delete attachments
		virtual void			TransferWeapons(Body* pBody, bool bRemove);
		virtual	void			RemoveAttachments( bool bDestroyAttachments );


		void RideVehicle(PlayerVehicle* pVehicle);

		// Tell the player to follow a lure.
		bool FollowLure( PlayerLure& playerLure );
		bool StopFollowingLure( );

		virtual void PushCharacter(const LTVector &vPos, LTFLOAT fRadius, LTFLOAT fStartDelay, LTFLOAT fDuration, LTFLOAT fStrength);

        LTBOOL HasDangerousWeapon();
        LTBOOL HasMeleeWeapon();

        void ChangeWeapon(uint8 nCommandId, LTBOOL bAuto=LTFALSE, int32 nAmmoId=-1);

		void SetControlFlags(uint32 nFlags) { m_nControlFlags = nFlags; }
		uint32 GetControlFlags() const { return m_nControlFlags; }

		void ChangeState(PlayerState eNewState);
		PlayerState GetState() const { return m_eState; }

		void HandlePreExit();
		void HandleExit( bool bNewMission );
		virtual void  HandleDamage(const DamageStruct& damage);
		void HandleActivateMessage(ILTMessage_Read *pMsg);
		void HandlePlayerPositionMessage(ILTMessage_Read *pMsg);
		void HandleWeaponFireMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundLoopMessage(ILTMessage_Read *pMsg);
		void HandleClientMsg(ILTMessage_Read *pMsg);
		void HandleTeleportMsg(ILTMessage_Read *pMsg);
        bool HandleObjectiveMessage(const CParsedMsg &cMsg);
        bool HandleKeyMessage(const CParsedMsg &cMsg);
        bool HandleTransmissionMessage(const CParsedMsg &cMsg);
        bool HandleOverlayMessage(const CParsedMsg &cMsg);
        bool HandleCreditsMessage(const CParsedMsg &cMsg);
        bool HandleIntelMessage(const CParsedMsg &cMsg);
        bool HandleFadeScreenMessage(const CParsedMsg &cMsg, bool bFadeIn);
        bool HandleMissionTextMessage(const CParsedMsg &cMsg);
        bool HandleMissionFailedMessage(const CParsedMsg &cMsg);
        bool HandleMusicMessage(const CParsedMsg &cMsg);
		bool HandleRemoveAllBadAIMessage( );

		void SetNetClientData( NetClientData const& NetClientData );
		char const* GetNetUniqueName( );

		void	SetChatting(LTBOOL bChatting);

		virtual LTBOOL DoDialogueSubtitles() { return LTTRUE; }

		//used only for ending co-op levels
		LTBOOL  IsReadyToExit() {return m_bReadyToExit;}
		void	ReadyToExit(LTBOOL bReady) {m_bReadyToExit = bReady;}

		void	ForceDuck(LTBOOL bForce) {m_bForceDuck = bForce;}
		LTBOOL	IsForcedToDuck() const { return m_bForceDuck; }

		int   GetRespawnCount() { return(m_nRespawnCount); }
		void  SetRespawnCount(int nRespawns) { m_nRespawnCount = nRespawns; }
		void  IncRespawnCount() { m_nRespawnCount++; }
		LTBOOL  RespawnCalled() const { return m_bRespawnCalled; }

        void    DoWeaponChange(uint8 nWeaponId,uint8 nAmmoId);
		void	DoWeaponReload(uint8 nWeaponId);

		CSkills* GetPlayerSkills() { return &m_Skills; }
		void	 HandleSkillUpdate(ILTMessage_Read *pMsg);

		MissionStats* GetMissionStats() { return &m_Stats; }
		CPlayerScore* GetPlayerScore() { return &m_Score; }

		void TeleportClientToServerPos();

        virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);

		void SendIDToClients();

		// Hiding

		LTBOOL IsHidden() const { return m_bHidden; }
		void ClearHiding();
		
		void SetVisibleToEnemyAI( CAI* pAI, bool bVis );

		// Carrying...

		void	SetCarriedObject(HOBJECT hBody, bool bTransition = false );
		HOBJECT	GetCarriedObject() {return m_hCarriedObject;}
		bool	IsCarrying() {return (GetCarriedObject() != LTNULL);}
		bool	IsCarryingHeavyObject();
		void	RemoveCarriedObject();
		bool	DropCarriedObject( bool bTransition = false );


		virtual void	HideCharacter(LTBOOL bHide);

		// Handshake status of the player
		LTBOOL HasDoneHandshake() { return m_bHasDoneHandshake; }
		void FinishHandshake() { m_bHasDoneHandshake = LTTRUE; }

		void ResetObjectRelationMgr(const char* const pszNewRelation=NULL);
		
		// damage filtering
		virtual bool FilterDamage( DamageStruct *pDamageStruct );

		// Get the complete list of all PlayerObj's.
		typedef std::vector< CPlayerObj* > PlayerObjList;
		static PlayerObjList const& GetPlayerObjList( ) { return m_lstPlayerObjs; }
		static uint32 GetNumberPlayersWithClients( );
		
		
		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
		
		PlayerPhysicsModel	GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		void RemoveVehicleModel();

		void	SetUsedStartPoint( bool bUsed ) { m_bUsedStartPoint = bUsed; }
		bool	UsedStartPoint( ) const { return m_bUsedStartPoint; }

		// Creates body after respawn.
		void	CreateRespawnBody( );

		// Setup our team model.
		bool	SetupTeamModel( bool bResetModel );

		// The teamid the player is on.
		uint8	GetTeamID( ) const;
		bool	RequestedTeamChange() {return (m_nRequestedTeam != INVALID_TEAM); }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
        virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

        void    WeaponCheat(uint8 nWeaponId);

		void	StartDeath();
        void    HandleDead(LTBOOL bRemoveObj);
		void	InitializeWeapons(CWeapons* pWeapons);
		char*	GetDamageSound(DamageType eType);
		char*	GetDeathSound();

		LTFLOAT	GetFootstepVolume();

		virtual void ProcessDamageMsg(ILTMessage_Read *pMsg);

        virtual LTVector GetHeadOffset() { return GetPlayerHeadOffset( ); }

        bool	ProcessCommandLure(const CParsedMsg &cMsg);

		
		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// [kml] Since we have a derived class, these need to be virtual
        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
        virtual LTBOOL   Update();
		virtual void UpdateInterface( bool bForceUpdate ); // Send client interface info.

		virtual void GetExtraFireMessageInfo( uint8, ILTMessage_Read*, WeaponFireInfo* ) {}



		void	PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL   InitialUpdate(int nInfo);

		void	CreatePVWeapon();

		virtual void UpdateCommands();		// Update our animator based on client commands
		virtual void UpdateMovement();		// Update player movement
		virtual void UpdateVehicleMovement(); // Update movement while on a vehicle.
		virtual bool UpdateDamageAnimations(); // Update our animatior based on any damage we are taking

        virtual HMODELANIM GetDeathAni(LTBOOL bFront);

		virtual HMODELANIM GetCrouchDeathAni(LTBOOL bFront);

        void UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers);

		void UpdateClientPhysics();		// Update client physics state.
		void UpdateAirLevel();			// Update player's air
		void UpdateClientEffects();		// Update client side effects

		virtual void UpdateHealth();	// Update player's health (regeneration)

		void UpdateHiding();			// Update player's hiding state
		void StartHiding();				// player is starting to hide
		void StartHidden();				// player is now hidden
		void EndHidden();				// player is no longer hidden, goes back to starting to hide.
		LTBOOL CanHide();				// Can the player hide right now?

		void BecomeVisible();			// Sets the visibility based stimuli to on
		void BecomeInvisible();			// Sets the visibility based stimuli to off

        void ResetPlayer( ); // Reset the player
		void ResetHealth(LTBOOL bMaxOnly = LTFALSE, float fHealthPercent = 1.0f, float fArmorPercent = 1.0f);
		void ResetInventory(LTBOOL bRemoveGear=LTTRUE);
		void ResetModel();

		void TweakCameraOffset();

        LTBOOL PlayingCinematic(LTBOOL bStopCinematic=LTFALSE);

		void  HandleGameRestore();

		void UpdateSpecialFX();

		void UpdateClientViewPos();
		void UpdateConsoleVars();

        void TeleFragObjects(LTVector & vPos);

		void HandleTeleport(const LTVector& vPos);
		void HandleTeleport(TeleportPoint* pTeleportPoint);
		void Teleport(const LTVector & vPos, const LTVector & vPitchYawRoll);

		void HandleLinkBroken(HOBJECT hObj);
		void BuildCameraList();

		void WriteVehicleMessage(ILTMessage_Write *pMsg);
		void WriteInterfaceMessage(ILTMessage_Write *pMsg);

		void ChangeSpeedsVar(float &var, float fVal)
		{
			if (var != fVal)
			{
				m_PStateChangeFlags |= PSTATE_SPEEDS;
			}
			var = fVal;
		}

		void	SetLeashLen(float fVal)		{ChangeSpeedsVar(m_fLeashLen, fVal);}
		void	SetLeashSpring(float fVal)	{ChangeSpeedsVar(m_fLeashSpring, fVal);}
		void	SetLeashSpringRate(float fVal)	{ChangeSpeedsVar(m_fLeashSpringRate, fVal);}
		void	SetRunVel(float fVal)		{ChangeSpeedsVar(m_fRunVel, fVal);}
		void	SetWalkVel(float fVal)		{ChangeSpeedsVar(m_fWalkVel, fVal);}
		void	SetJumpVel(float fVal)		{ChangeSpeedsVar(m_fJumpVel, fVal);}
		void	SetSwimVel(float fVal)		{ChangeSpeedsVar(m_fSwimVel, fVal);}
		void	SetMoveMul(float fVal)		{ChangeSpeedsVar(m_fMoveMultiplier, fVal);}
		void	SetJumpVelMul(float fVal)	{ChangeSpeedsVar(m_fJumpMultiplier, fVal);}
		void	SetLadderVel(float fVal)	{ChangeSpeedsVar(m_fLadderVel, fVal);}

		virtual void	SetPhysicsModel(PlayerPhysicsModel eModel=PPM_NORMAL, LTBOOL bUpdateClient=LTTRUE);
		virtual void	SetVehiclePhysicsModel(PlayerPhysicsModel eModel);
		virtual void	SetNormalPhysicsModel();

		void	AcquireDefaultWeapon();
		bool	AcquireLevelDefaultWeapons( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultMods( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultAmmo( int nMissionId, int nLevelId );
		bool	AcquireCurrentLevelDefaults();
		void	AcquireWeapon(char* pWeaponName);
		void	AcquireMod(char* pModName);
		void	AcquireAmmo(char* pAmmoName);
		void	AcquireGear(char* pGearName);
		void	ChangeToWeapon(const char* pWeaponName);

		virtual void HandleVolumeEnter(AIInformationVolume* pNewVolume);
		virtual float GetVerticalThreshold() const;
		virtual float GetInfoVerticalThreshold() const;

        virtual LTBOOL DoActivate(CActivationData* pData);

	
		// Pickup an object.
		bool PickupObject( HOBJECT hObject );
		bool CanDropObject( );

		// Generates a unique name for all players from a given name.
		void GenerateUniqueName( char const* pszNameBase, char* pszUniqueName, int nUniqueNameSize );

		// Setup our dead body...
		virtual void	SetupBody(Body* pBody);
		// Get rid of our dead body
		void	RemoveBody();

		// Accessors the lean value.
		void SetLean( CPPlayerLeanTypes eLean, LTVector const& vPos );
		CPPlayerLeanTypes GetLean( ) { return m_ePlayerLean; }

		//handle request to switch teams
		void SwitchToTeam(uint8 nTeam );

	protected :  // Data members

        LTFLOAT      m_fOldHitPts;
		LTFLOAT		 m_fOldEnergy;
        LTFLOAT      m_fOldArmor;
        LTFLOAT      m_fOldAirLevel;
        LTFLOAT      m_fAirLevel;
        LTVector     m_vOldModelColor;
        LTFLOAT      m_fOldModelAlpha;

		// When we respawn, this is incremented so we ignore packets from an older teleport.

        uint8       m_ClientMoveCode;

		PlayerState m_eState;
		
		// Handles case where the player is invulnerable during respawn in MP.  Don't save.
		bool		m_bRespawnInvulnerability;
		CTimer		m_tmrRespawnInvulnerabilityTime;

		// Handle protection from special damage types while recovering from similar damage.
		bool		m_bGriefProtected;
		CTimer		m_tmrGriefTime;

		// Flag indicating we're waiting for the client to
		// finish loading.  Not saved.
		bool		m_bClientLoaded;

        LTBOOL      m_b3rdPersonView;
        uint32      m_nSavedFlags;
		LTBOOL		m_bRespawnCalled;

		// Animator

		CAnimatorPlayer	m_Animator;

		// Info about cheats...

        LTBOOL		m_bGodMode;
        LTBOOL      m_bAllowInput;
        LTBOOL      m_bChatting;
	    uint16		m_nClientChangeFlags;

		bool		m_bCinematicInvulnerability;

		// Note: These aren't saved/loaded because changing levels
		// will mess up the state of these (and they are cheats anyway)
        LTBOOL		m_bSpectatorMode;	
		bool		m_bInvisible;

		CLTMsgRef_Read m_pClientSaveData;  // Client data to save

		CString		m_sStartLevelCommand;

		PlayerPhysicsModel	m_ePPhysicsModel;
		LTObjRefNotifier	m_hVehicleModel;

		LTBOOL				m_bReadyToExit;
		LTBOOL				m_bForceDuck;

		CSkills				m_Skills;

		MissionStats		m_Stats;
		CPlayerScore		m_Score;


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CPlayerAttachments*		m_pPlayerAttachments;	// Our attachments aggregate

		LTObjRef	m_CurContainers[MAX_TRACKED_CONTAINERS];
        uint32      m_nCurContainers;

        bool		m_bFirstUpdate;
		bool		m_bNewMission;

		bool		m_bForceUpdateInterface;

        uint32      m_PStateChangeFlags;   // Which things need to get sent to client.

		int*		m_pnOldAmmo;

		HCLIENT		m_hClient;


		// Data members used in load/save...

        uint32      m_dwLastLoadFlags;


		// Multiplayer data
		std::string	m_sNetNameUnique;
		int			m_nRespawnCount;

		LTBOOL		m_bHasDoneHandshake; // Is the handshaking done yet?


		// ForceUpdate camera support...

		CTList<LPBASECLASS>	 m_Cameras;
        LTBOOL               m_bCameraListBuilt;


		// Motion status from client

        uint8       m_nMotionStatus;
        uint8       m_nWeaponStatus;
		CPPlayerLeanTypes m_ePlayerLean;


		// Hiding
		LTFLOAT		m_fHiddenTimer;
		LTFLOAT		m_fHideDuration;
		LTBOOL		m_bHiding;
		LTBOOL		m_bHidden;


		// Carrying

		LTObjRefNotifier	m_hCarriedObject;
		bool				m_bCanDropBody;
		bool				m_bCarryingAI;

		// Client/server movement interaction variables
		LTVector	m_vLastClientPos;	// Last position provided by the client
		LTBOOL		m_bUseLeash;		// Is leashing turned on?


		// Console variables used to tweak player movement...

		CVarTrack	m_LeashLenTrack;
		CVarTrack	m_LeashSpringTrack;
		CVarTrack	m_LeashSpringRateTrack;
		CVarTrack	m_ShowNodesTrack;
		CVarTrack	m_MoveVelMulTrack;
		CVarTrack	m_JumpVelMulTrack;
		CVarTrack	m_SwimVelTrack;
		CVarTrack	m_LadderVelTrack;

		float		m_fLeashLen;
		float		m_fLeashSpring;
		float		m_fLeashSpringRate;

        LTBOOL      m_bShowNodes;

		char		m_szMultiplayerSkin[128];
		char		m_szMultiplayerHead[128];

		// Lure Object used for keyframing player's position.
		LTObjRef	m_hPlayerLure;
		
		EnumAIStimulusID m_eLeanVisibleStimID;

		LTVector	m_vLastPos;	// our position last update
                
		static PlayerObjList	m_lstPlayerObjs;

		LTVector	m_vClientCameraOffset;
		int			m_nClientCameraOffsetTimeReceivedMS;
		
		LTObjRef	m_hDeadBody;

		float		m_fLastVehicleFootstepTime;	

		// Is an enemy AI looking at us?
		bool		m_bVisibleToEnemyAI;

		bool		m_bSendStartLevelCommand;

		// Client-side control flags
		uint32		m_nControlFlags;
		
		LTVector	m_vLastGoodDropPos;
			
		CActivationData m_ActivationData;

		LTRotation	m_rFullPlayerRot;
		
		// [RP] 9/14/02 - Save off the velocity so we can zero out the players vel when they load
		//		and then reset the saved vel when they actually respawn in the world.

		LTVector	m_vSavedVelocity;
		uint32		m_dwSavedObjectFlags;

		// Version of save game when loaded.  Passed down to client for client save data.
		uint32		m_nSaveVersion;
		
		bool		m_bUsedStartPoint;	// Whe they player first spawns at a start point this gets set

		bool		m_bCanBeRevived;

		// this will be a valid team id if the player has requested to change teams
		uint8		m_nRequestedTeam;

};


#endif  // __CPLAYER_OBJ_H__
