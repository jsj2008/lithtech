// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.h
//
// PURPOSE : Player object definition
//
// CREATED : 9/18/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CPLAYER_OBJ_H__
#define __CPLAYER_OBJ_H__

#include "Character.h"
#include "MsgIDs.h"
#include "CheatDefs.h"
#include "TemplateList.h"
#include "VarTrack.h"
#include "ClientServerShared.h"
#include "AnimatorPlayer.h"
#include "ActivationData.h"
#include "SharedMission.h"
#include "SharedScoring.h"
#include "IDList.h"
#include "NetDefs.h"
#include "PlayerInventory.h"
#include "AIEnumStimulusTypes.h"
#include "SonicData.h"
#include "LeanNodeController.h"
#include "EventCaster.h"
#include "ServerNodeTrackerContext.h"
#include "PlayerRigidBody.h"

LINKTO_MODULE( PlayerObj );

class TeleportPoint;
class PlayerLure;
struct WeaponFireInfo;
class Turret;

class PlayerTrackerInfo
{
public:
	PlayerTrackerInfo()
	{
		m_trkID = MAIN_TRACKER;
		m_hWeightSet = INVALID_MODEL_WEIGHTSET;
		m_nTime = 0;
		m_hAni = INVALID_MODEL_ANIM;

	};
	PlayerTrackerInfo(ANIMTRACKERID	trkID, HMODELWEIGHTSET hWeightSet)
	{
		m_trkID = trkID;
		m_hWeightSet = hWeightSet;
		m_nTime = 0;
		m_hAni = INVALID_MODEL_ANIM;
	};

	virtual ~PlayerTrackerInfo() {};

	ANIMTRACKERID	m_trkID;
	HMODELWEIGHTSET	m_hWeightSet;
	HMODELANIM		m_hAni;
	uint32			m_nTime;
};
typedef std::vector<PlayerTrackerInfo> TrackerList;

class PlayerBroadcastInfo
{
public:
	PlayerBroadcastInfo() : 
		nBroadcastID( 0 ),
		bForceClient( false ),
		nPriority( 0 ),
		bDamageBroadcast( false ),
		bSendToTeam( true ),
		bPlayerInitiated( false ),
		hTarget( NULL )

		{}
	virtual ~PlayerBroadcastInfo() {}

	uint32 nBroadcastID;
	bool bForceClient;
	int8 nPriority;
	bool bDamageBroadcast;
	bool bSendToTeam;
	bool bPlayerInitiated;
	HCLIENT hTarget;

private:
	// Copy ctor and assignment operator not implemented and should never be used.
	PlayerBroadcastInfo( PlayerBroadcastInfo const& other );
	PlayerBroadcastInfo& operator=( PlayerBroadcastInfo const& other );
};


class CPlayerObj : public CCharacter
{
	typedef CCharacter super;

	public :
		DEFINE_CAST( CPlayerObj );

		CPlayerObj();
		virtual ~CPlayerObj();

		// Attachments

		virtual void CreateAttachments();
		
		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		void SendStartLevelCommand();
		void SetClient(HCLIENT hClient);
		HCLIENT GetClient() const		{ return m_hClient; }

		void SetClientSaveData(ILTMessage_Read *pMsg);
		ILTMessage_Read* GetClientSaveData( ) const { return m_pClientSaveData; }

		void BuildKeepAlives(ObjectList* pList);

		// If the update had CLIENTUPDATE_ALLOWINPUT, then it returns FALSE because nothing
		// else will be in the packet.
		bool ClientUpdate(ILTMessage_Read *pMsg);

		bool ClientInit( );

		bool MultiplayerUpdate(ILTMessage_Read *pMsg);

		// Check if player is allowed to respawn.
		bool CanRespawn( );

		// set or retrieve the lock spectator mode state
		bool IsLockSpectatorMode() { return m_bLockSpectatorMode; }
		void SetLockSpectatorMode(bool bLockSpectatorMode) { m_bLockSpectatorMode = bLockSpectatorMode; }


        void Respawn();
		void ResetAfterDeath();

		virtual bool IsDead() const	{ return ( GetPlayerState( ) == ePlayerState_Dead || 
										GetPlayerState( ) == ePlayerState_Dying_Stage1 ||
										GetPlayerState( ) == ePlayerState_Dying_Stage2 ); 
									}
		virtual bool IsAlive() const { return ( GetPlayerState( ) == ePlayerState_Alive ); }
		bool IsSpectating( ) const { return GetPlayerState( ) == ePlayerState_Spectator || m_eSpectatorMode != eSpectatorMode_None; }

		void SetSpectatorMode( SpectatorMode eSpectatorMode, bool bForce );
		void SetInvisibleMode(bool bOn);
		void ToggleGodMode();

		void FullAmmoCheat();
		void FullWeaponCheat();
		void FullModsCheat();
		void FullGearCheat();
		void RepairArmorCheat();
		void HealCheat();
		void GimmeGunCheat( HWEAPON hWeapon );
		void GimmeModCheat( HMOD hMod );
		void GimmeGearCheat( HGEAR hGear );
		void GimmeAmmoCheat( HAMMO hAmmo );

		// Call to setup the player for new mission.
		bool SetupForNewMission( );

		virtual CAttachments*	TransferAttachments( bool bRemove ); // Responsibility of call to delete attachments
		virtual void			DropWeapons();
		virtual void			DropCurrentWeapon();
		virtual	void			RemoveAttachments( bool bDestroyAttachments );


		// Tell the player to follow a lure.
		bool FollowLure( PlayerLure& playerLure );
		bool StopFollowingLure( );

		virtual void PushCharacter(const LTVector &vPos, float fRadius, float fStartDelay, float fDuration, float fStrength);

		void ChangeWeapon( HWEAPON hWeapon, bool bForce, HAMMO hAmmo, bool bPlaySelect, bool bPlayDeselect );
				
		void SetControlFlags(uint32 nFlags) { m_nControlFlags = nFlags; }
		uint32 GetControlFlags() const { return m_nControlFlags; }

		void SetPlayerState(PlayerState eNewState, bool bForceReset);
		PlayerState GetPlayerState() const { return m_ePlayerState; }
		void UpdatePlayerState( );

		void HandlePreExit();
		void HandleExit( bool bNewMission );
		virtual void  HandleDamage(const DamageStruct& damage);
		void HandleActivateMessage(ILTMessage_Read *pMsg);
		void HandlePlayerPositionMessage(ILTMessage_Read *pMsg);
		void HandleWeaponFireMessage(ILTMessage_Read *pMsg);
		void HandleWeaponFinishMessage(ILTMessage_Read *pMsg);
		void HandleWeaponFinishRagdollMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundMessage(ILTMessage_Read *pMsg);
		void HandleWeaponSoundLoopMessage(ILTMessage_Read *pMsg);
		void HandleClientMsg(ILTMessage_Read *pMsg);
		void HandleTeleportMsg(ILTMessage_Read *pMsg);
		void HandleAnimTrackerMsg( ILTMessage_Read *pMsg );
		void HandleSonicMsg( ILTMessage_Read* pMsg );
		void HandleDropGrenadeMessage( ILTMessage_Read *pMsg );
		void HandlePlayerSlowMoMsg(ILTMessage_Read *pMsg);
		void HandlePlayerEventMsg(ILTMessage_Read *pMsg);

		void WeaponSwap( HOBJECT hTarget, const LTRigidTransform& tPickup ) { m_Inventory.WeaponSwap( hTarget, tPickup ); }

		wchar_t const*	GetNetUniqueName( );
		char const*		GetPatch( );

		void	SetChatting(bool bChatting);
		void	SetHasSlowMoRecharge(bool bSlowMoRecharge);

		virtual bool DoDialogueSubtitles() { return true; }

		void	ForceDuck(bool bForce) {m_bForceDuck = bForce;}

		void	DoWeaponChange( HWEAPON hWeapon, HAMMO hAmmo );
		void	DoWeaponReload( HWEAPON hWeapon, HAMMO hAmmo );
		void	DoWeaponSwap( HWEAPON hFromWeapon, HWEAPON hToWeapon );

		MissionStats* GetMissionStats() { return &m_Stats; }
		
		void TeleportClientToServerPos( bool bWithRotation );

		virtual float ComputeDamageModifier(ModelsDB::HNODE hModelNode);

		void SendIDToClients();

		virtual void	HideCharacter(bool bHide);
		
		// damage filtering
		virtual bool FilterDamage( DamageStruct *pDamageStruct );

		// Get the complete list of all PlayerObj's.
		typedef std::vector< CPlayerObj*, LTAllocator<CPlayerObj*, LT_MEM_TYPE_OBJECTSHELL> > PlayerObjList;
		static PlayerObjList const& GetPlayerObjList( ) { return m_lstPlayerObjs; }
		static uint32 GetNumberPlayersWithClients( );
		
		
		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
		
		PlayerPhysicsModel	GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		// Set model
		void	SetModel( ModelsDB::HMODEL hModel );

		// The teamid the player is on.
		uint8	GetTeamID( ) const;
		void	HandleTeamSwitchRequest();

		// Called when a new client enters the world
		void	OnClientEnterWorld(HCLIENT hClient, CPlayerObj *pPlayer);

		void	RemoveWeapons();

		void	WarnWeaponWillBreak(HWEAPON hWeapon);
		void	HandleWeaponBroke(HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon);

		// VolumeBrush information changed.
		void	OnVolumeBrushChanged( )
		{
			m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
		}

		virtual CPlayerInventory*	GetInventory()	{ return &m_Inventory; }

		virtual float	GetStealthModifier() { return m_Inventory.GetStealthModifier(); }

		virtual void	GetViewTransform( LTRigidTransform &tfView ) const { tfView = m_tfCameraView; }
		virtual void	GetTrueViewTransform( LTRigidTransform &tfView ) const { tfView = m_tfTrueCameraView; }

		//  Accessor the player timer scale.  All player timer changes should go through these
		//	accessores rather than the engine directly.
		bool SetTimerScale( uint32 nNumerator, uint32 nDenominator );

		// Enter slowmo.
		enum EEnterSlowMoFlags
		{ kTransition = (1<<0), kPlayerControlled = (1<<1), kUsePlayerTimeScale = (1<<2), kDontUpdateCharge = (1<<3), };
		bool EnterSlowMo( HRECORD hSlowMoRecord, HCLIENT hActivator, uint8 nSlowMoActivatorTeamId, uint32 dwFlags );

		// Exit slowmo.
		void ExitSlowMo( bool bDoTransition );

		//Does player prefer WeaponA to WeaponB.
		virtual bool IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const;

		// Set or clear the turret the player is using...
		void	SetOperatingTurret( const Turret &rTurret, bool bUsing );
		HOBJECT	GetTurret() const { return m_hTurret; }

		// Handle the AI grabbing the player, holding them in place and 
		// beating them.
		void	BerserkerAttack( HOBJECT hAI );

		// Handle the AI aborting the above attack.
		void	BerserkerAbort( HOBJECT hAI );

		virtual void	AddRemoteCharge(HOBJECT hRemote);

		bool		StartStoryMode(HOBJECT hStoryMode, bool bCanSkip, bool bFromRestore);
		bool		InStoryMode();
		bool		EndStoryMode();

		void		HandleBroadcastMsg(ILTMessage_Read *pMsg);
		void		HandleBroadcast(const PlayerBroadcastInfo& pbi);

		virtual bool	IsCrouched() {return m_bIsCrouched;}

		void		WriteAnimInfo(ILTMessage_Write *pMsg);

		virtual void	HandleGib();
		virtual void	HandleSever(ModelsDB::HSEVERPIECE hPiece);

		// Teleport player to postion/rot.
		void Teleport(const LTVector & vPos, const LTRotation& rRot);

		//get the last client requested velocity
		const LTVector& GetLastVelocity() const {return m_vLastVelocity;}

		// Event fires when player drops inventory.
		DECLARE_EVENT( DropInventoryEvent );

		// Event fires when player scores kill.  Sends PlayerScoredKillEventParams.
		struct PlayerScoredKillEventParams : public EventCaster::NotifyParams
		{
			PlayerScoredKillEventParams( EventCaster& eventCaster, HOBJECT hVictim, HOBJECT hKiller, 
				bool bHeadShot, HRECORD hAmmo ) : 
				EventCaster::NotifyParams( eventCaster ),
				m_hVictim( hVictim ),
				m_hKiller( hKiller )
			{
				m_bHeadShot = bHeadShot;
				m_hAmmo = hAmmo;
			}

			LTObjRef m_hVictim;
			LTObjRef m_hKiller;
			bool m_bHeadShot;
			HRECORD m_hAmmo;
		};
		static EventCaster PlayerScoredKillEvent;

		// Time when last respawned.  Used in MP, don't save.  Time in GameTimeTimer units. <0 if haven't respawned yet.
		double GetLastRespawnTime( ) const { return m_fLastRespawnTime; }

	protected :

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetPlayerAlignment();

		void    WeaponCheat(uint8 nWeaponId);

		void	StartDeath();
		void	HandleDead();
		virtual void	PrepareToSever();

		float	GetFootstepVolume();

		virtual void ProcessDamageMsg( DamageStruct &rDamage );

		virtual LTVector GetHeadOffset() const { return m_vHeadOffset; }

        bool	ProcessCommandLure(const CParsedMsg &cMsg);

		
		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// [kml] Since we have a derived class, these need to be virtual
        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		virtual bool Update();
		virtual void UpdateInterface( bool bForceUpdate ); // Send client interface info.
		virtual void	UpdateDead( bool bCanBeRemoved );


		void	PostPropRead(ObjectCreateStruct *pStruct);
		bool	InitialUpdate(int nInfo);

		virtual void UpdateCommands();		// Update our animator based on client commands
		virtual void UpdateMovement();		// Update player movement
		virtual bool UpdateDamageAnimations(); // Update our animatior based on any damage we are taking

		virtual HMODELANIM GetDeathAni(bool bFront);

		virtual HMODELANIM GetCrouchDeathAni(bool bFront);

        void UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers);

		void UpdateClientPhysics();		// Update client physics state.
		void UpdateAirLevel();			// Update player's air

		virtual void UpdateHealth();	// Update player's health (regeneration)

		void BecomeVisible();			// Sets the visibility based stimuli to on
		void BecomeInvisible();			// Sets the visibility based stimuli to off

        void ResetPlayer( ); // Reset the player
		void ResetHealth(bool bMaxOnly = false, float fHealthPercent = 1.0f, float fArmorPercent = 1.0f);
		void ResetInventory(bool bRemoveGear=true);
		void ResetModel();

		bool PlayingCinematic(bool bStopCinematic=false);

		void  HandleGameRestore();

		void UpdateSpecialFX();

		void UpdateClientViewPos();
		void UpdateConsoleVars();

		void UpdateClientFadeTime();

        void TeleFragObjects(LTVector & vPos);

		void HandleTeleport(const LTVector& vPos);
		void HandleTeleport(TeleportPoint* pTeleportPoint);

		void WriteVehicleMessage(ILTMessage_Write *pMsg);

		void ChangeSpeedsVar(float &var, float fVal)
		{
			if (var != fVal)
			{
				m_PStateChangeFlags |= PSTATE_SPEEDS;
			}
			var = fVal;
		}

		void	SetLeashLen(float fVal)		{ChangeSpeedsVar(m_fLeashLen, fVal);}
		void	SetLeashScale(float fVal)	{ChangeSpeedsVar(m_fLeashScale, fVal);}
		void	SetLeashSpring(float fVal)	{ChangeSpeedsVar(m_fLeashSpring, fVal);}
		void	SetLeashSpringRate(float fVal)	{ChangeSpeedsVar(m_fLeashSpringRate, fVal);}
		void	SetRunVel(float fVal)		{ChangeSpeedsVar(m_fRunVel, fVal);}
		void	SetWalkVel(float fVal)		{ChangeSpeedsVar(m_fWalkVel, fVal);}
		void	SetJumpVel(float fVal)		{ChangeSpeedsVar(m_fJumpVel, fVal);}
		void	SetSwimVel(float fVal)		{ChangeSpeedsVar(m_fSwimVel, fVal);}
		void	SetCrawlVel(float fVal)		{ChangeSpeedsVar(m_fCrawlVel, fVal);}
		void	SetMoveMul(float fVal)		{ChangeSpeedsVar(m_fMoveMultiplier, fVal);}
		void	SetJumpVelMul(float fVal)	{ChangeSpeedsVar(m_fJumpMultiplier, fVal);}
		void	SetLadderVel(float fVal)	{ChangeSpeedsVar(m_fLadderVel, fVal);}

		virtual bool	SetPhysicsModel( PlayerPhysicsModel eModel = PPM_NORMAL, ModelsDB::HMODEL hModel = NULL, bool bUpdateClient = true );
		virtual void	SetNormalPhysicsModel();

		void	AcquireDefaultWeapon();
		void	AcquireDefaultWeapons();
		bool	AcquireLevelDefaultWeapons( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultMods( int nMissionId, int nLevelId );
		bool	AcquireLevelDefaultAmmo( int nMissionId, int nLevelId );
		bool	AcquireCurrentLevelDefaults();
		bool	AcquireMPDefaultWeapons();
		bool	AcquireMPLoadoutWeapons();
		void	AcquireWeapon( const char *pszWeaponName );
		void	AcquireMod( const char *pszModName );
		void	AcquireAmmo( const char *pszAmmoName );
		void	AcquireGear( const char *pszGearName );
		void	ChangeToWeapon( const char *pszWeaponName );
		void	ChangeToLastWeapon();

		virtual float GetVerticalThreshold() const;

		virtual bool DoActivate(CActivationData* pData);

		// Accessors the lean value.
		void SetLean( CPPlayerLeanTypes eLean, LTVector const& vPos );
		CPPlayerLeanTypes GetLean( ) { return m_ePlayerLean; }

		//handle request to switch teams
		void SwitchToTeam(uint8 nTeam );

		// Handle a message from the client to update the camera data...
		void ClientCameraInfoUpdate( ILTMessage_Read *pMsg, uint32 dwClientFlags );

		// Handle a message from the client to update animations...
		void ClientAnimationUpdate( ILTMessage_Read *pMsg );

		// Handle a message from the client to add animation trackers...
		void ClientAnimationAddTrackers( ILTMessage_Read *pMsg );

		// Handle message from the client to remove animation trackers...
		void ClientAnimationRemoveTrackers( ILTMessage_Read *pMsg );


		// Change the weapon sound looping status
		void SetWeaponSoundLooping(uint32 nType, uint32 nWeaponID);
		// Update the weapon sound looping status for a client
		void UpdateWeaponSoundLooping(HCLIENT hClient);
		bool IsWeaponSoundLooping() const { return m_nWeaponSoundLoopType != PSI_INVALID; }

		void SaveOverlays(ILTMessage_Write *pMsg);
		void LoadOverlays(ILTMessage_Read *pMsg);

		// Sends the current timer scale to update the CharacterFX object.
		bool SendTimerScale( );

		// Drops gear items when the character dies... 
		virtual void SpawnGearItemsOnDeath( );

		TrackerList::iterator FindTracker( ANIMTRACKERID trkID );

		// After loading a saved game activate the ladder if the character has one
		virtual void ActivateLadderOnLoad( );

	protected :  // Data members

		float		m_fOldHitPts;
		float		m_fOldArmor;
		float		m_fOldAirLevel;
		float		m_fAirLevel;

		PlayerState m_ePlayerState;
		
		bool		m_b3rdPersonView;
        uint32      m_nSavedFlags;


		bool		m_bIsCrouched;

		// Animator

		CAnimatorPlayer	m_Animator;

		// Info about cheats...

		bool		m_bGodMode;
		bool		m_bAllowInput;
		bool		m_bChatting;
		bool		m_bSliding;
		bool		m_bSlowMoCharge;
	    uint16		m_nClientChangeFlags;

		bool		m_bCinematicInvulnerability;

		// Note: These aren't saved/loaded because changing levels
		// will mess up the state of these (and they are cheats anyway)
		SpectatorMode	m_eSpectatorMode;	
		bool		m_bInvisible;

		CLTMsgRef_Read m_pClientSaveData;  // Client data to save

		std::string			m_sStartLevelCommand;

		PlayerPhysicsModel	m_ePPhysicsModel;

		bool				m_bForceDuck;

		MissionStats		m_Stats;

		LTVector			m_vHeadOffset;

		CPlayerInventory	m_Inventory;

		LTObjRef			m_StoryModeObject;
		bool				m_bCanSkipStory;

		bool				m_bFadeInitiated;
		bool				m_bFadeIn;
		float				m_fFadeTimeRemaining;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

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

		// Motion status from client

        uint8       m_nMotionStatus;
        uint8       m_nWeaponStatus;
		CPPlayerLeanTypes m_ePlayerLean;

		// Client/server movement interaction variables
		LTVector	m_vLastClientPos;	// Last position provided by the client
		bool		m_bUseLeash;		// Is leashing turned on?

		// the last time that we forced the client to a certain position - this is used to disregard
		// updates that occurred prior to the client receiving the teleport message from the server
		uint32 m_nLastPositionForcedTime;

		// Console variables used to tweak player movement...

		VarTrack	m_LeashLenTrack;
		VarTrack	m_LeashSpringTrack;
		VarTrack	m_LeashScaleTrack;
		VarTrack	m_LeashSpringRateTrack;
		VarTrack	m_ShowNodesTrack;
		VarTrack	m_JumpVelMulTrack;
		VarTrack	m_SwimVelTrack;
		VarTrack	m_LadderVelTrack;
		VarTrack	m_BaseMoveAccelTrack;
		VarTrack	m_vtRunSpeedMul;

		float		m_fGravity;
		float		m_fLeashLen;
		float		m_fLeashScale;
		float		m_fLeashSpring;
		float		m_fLeashSpringRate;

		// Lure Object used for keyframing player's position.
		LTObjRef	m_hPlayerLure;

		EnumAIStimulusID m_eLeanVisibleStimID;

		LTVector	m_vLastVelocity; // the velocity last requested by the client

		static PlayerObjList	m_lstPlayerObjs;

		LTVector	m_vClientCameraOffset;
		int			m_nClientCameraOffsetTimeReceivedMS;

		bool		m_bSendStartLevelCommand;

		// Client-side control flags
		uint32		m_nControlFlags;

		CActivationData m_ActivationData;

		// The position and rotation of the camera view for the player...
		LTRigidTransform	m_tfCameraView;
		LTRigidTransform	m_tfTrueCameraView; //this will include pitch and roll, even when the player
												// is on a ladder or using a turret

		// [RP] 9/14/02 - Save off the velocity so we can zero out the players vel when they load
		//		and then reset the saved vel when they actually respawn in the world.

		LTVector	m_vSavedVelocity;
		uint32		m_dwSavedObjectFlags;

		// this will specify the weapon loadout that the player requested
		uint8		m_nLoadout;

		// Weapon sound looping status
		uint32		m_nWeaponSoundLoopType, m_nWeaponSoundLoopWeapon;

		StringSet	m_ActiveOverlays;

		//movement modifier based on selected weapon
		float		m_fWeaponMoveMultiplier;

		// The sonic data for the player
		SonicData	m_iSonicData;

		// Previous current weapon before doing a forced weapon switch (ie. Operating a turret)...
		HWEAPON		m_hPreviousWeapon;

		// Indicates that the player is operating a turret...
		LTObjRef	m_hTurret;

		// Ragdoll finishing move vars (for releasing the grabbed character after a set amount of time)
		HPHYSICSBREAKABLE	m_hGrabConstraint;
		double				m_fGrabEndTime;
		CCharacter*			m_pPendingGrab;

		// Node controller for specifically handling the lean of a player...
		CLeanNodeController	m_LeanNodeController;

		// Node controller for handling torso tracking of a player...
		CNodeTrackerContext m_NodeTrackerContext;

		// Forensic data
		uint32				m_dwLastForensicTypeMask;

		// Times how long it should take use to complete dying.
		StopWatchTimer	m_DyingTimer;

		StopWatchTimer	m_DamageMsgTimer;
		static StopWatchTimer s_RespawnMsgTimer;

		// The next time we will upate clients about our rotation.
		double	m_fNextClientRotationUpdateTime;
		LTRotation m_rLastRotation;

		// weapon requested at level start, save here until first update
		HWEAPON		m_hRequestedWeapon;

		// list of animation trackers the client has told us about
		TrackerList m_Trackers;

		static PlayerObjList	m_lstDyingPlayers;

		// Stores the mpmodel index.  Needed because player can exist without being attached to
		// gameclientdata, which also stores mpmodel index.
		uint8 m_nMPModelIndex;

		// Store the clientid since we may need to exist as a standbyplayer.
		uint8 m_nClientId;

		// Rigid body that pushes away physics objects at the feet of players...
		CPlayerRigidBody	m_PlayerRigidBody;

		// Timer that times how long player holds slowmo.
		StopWatchTimer	m_tmrSlowMoHoldCharger;

		// indicates if this player is locked into spectator mode
		bool m_bLockSpectatorMode;

		// Time when last respawned.  Used in MP, don't save.  Time in GameTimeTimer units. <0 if haven't respawned yet.
		double m_fLastRespawnTime;

		// Message Handlers...
		DECLARE_MSG_HANDLER( CPlayerObj, HandleMissionMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleObjectiveMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleTransmissionMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleFlashlightMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleCrosshairMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleHeartbeatMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleBreathMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleSignalMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleTextMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleScreenFadeMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleMissionFailedMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleRemoveBodiesMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleRemoveAllBadAIMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleTraitorMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleFaceObjectMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleResetInventoryMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAcquireWeaponMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAcquireAmmoMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAcquireModMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAcquireGearMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleChangeWeaponMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleLastWeaponMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleFullHealthMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleDismountMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleFollowLureMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleCancelLureMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleOverlayMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleScoreMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleChangeModelMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleSimulationTimerMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandlePlayerTimerMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleSlowMoMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleMixerMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleSoundFilterMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleRestartLevelMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAnimateMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleAnimateCamRotMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleWeightSetMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleExitLevelMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleSonicMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleCarryMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleGotoMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandleWeaponEffectMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandlePlayerMovementMsg );
		DECLARE_MSG_HANDLER( CPlayerObj, HandlePlayerEarRingOffMsg );
};


#endif  // __CPLAYER_OBJ_H__

// EOF
