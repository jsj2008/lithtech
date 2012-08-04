// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.h
//
// PURPOSE : Player object definition
//
// CREATED : 9/18/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CPLAYER_OBJ_H__
#define __CPLAYER_OBJ_H__

#include "Character.h"
#include "GameStartPoint.h"
#include "MsgIds.h"
#include "CheatDefs.h"
#include "TemplateList.h"
#include "CVarTrack.h"
#include "ClientServerShared.h"
#include "PlayerSummary.h"
#include "AnimatorPlayer.h"
#include "PlayerShared.h"

#define DEFAULT_PLAYERNAME		"Player"
#define NET_NAME_LENGTH			MAX_PLAYER_NAME

class CPlayerAttachments;
class CWeapon;
class PlayerVehicle;
class CMissionData;
class TeleportPoint;

struct FlashlightInfo
{
    LTBOOL           bOn;
    LTVector         vPos;

	FlashlightInfo()
	{
		Clear();
	}

	void Clear()
	{
        bOn = LTFALSE;
	}

	void Save(HMESSAGEWRITE hWrite)
	{
		SAVE_BOOL(bOn);
		SAVE_VECTOR(vPos);
	}

	void Load(HMESSAGEREAD hRead)
	{
		LOAD_BOOL(bOn);
		LOAD_VECTOR(vPos);
	}
};

class CPlayerObj : public CCharacter
{
	public :

		CPlayerObj();
		virtual ~CPlayerObj();

		// Attachments

		virtual void CreateAttachments();

		void StartLevel();
		void SetClient(HCLIENT hClient) {m_hClient = hClient;}
		HCLIENT GetClient() const		{ return m_hClient; }

		void SetClientSaveData(HMESSAGEREAD hClientData) { m_hClientSaveData = hClientData; }
		void BuildKeepAlives(ObjectList* pList);

		// If the update had CLIENTUPDATE_ALLOWINPUT, then it returns FALSE because nothing
		// else will be in the packet.
        LTBOOL ClientUpdate(HMESSAGEREAD hMessage);

        LTBOOL MultiplayerInit(HMESSAGEREAD hMessage);

        void Respawn(uint8 nServerLoadGameFlags=LOAD_NEW_LEVEL);
        void SetSpectatorMode(LTBOOL bOn);
		void ToggleRunLock();
		void ToggleGodMode();

		void FullAmmoCheat();
		void FullWeaponCheat();
		void FullModsCheat();
		void FullGearCheat();
		void RepairArmorCheat();
		void HealCheat();

		void Setup(CMissionData* pMissionData);

		virtual CAttachments* TransferAttachments(); // Responsibility of call to delete attachments

		void RideVehicle(PlayerVehicle* pVehicle);

        LTBOOL HasDangerousWeapon();

        void ChangeWeapon(uint8 nCommandId, LTBOOL bAuto=LTFALSE, int32 nAmmoId=-1);
        void SetRunLock(LTBOOL bRunLock)     { m_bRunLock = bRunLock; }
		void ChangeState(PlayerState eNewState);
		PlayerState GetState() const { return m_eState; }

		void HandleExit() { if (!m_bGodMode) ToggleGodMode(); }
		void HandleDamage(const DamageStruct& damage);
		void HandleActivateMessage(HMESSAGEREAD hRead);
		void HandlePlayerPositionMessage(HMESSAGEREAD hRead);
		void HandleWeaponFireMessage(HMESSAGEREAD hRead);
		void HandleWeaponSoundMessage(HMESSAGEREAD hRead);
		void HandleClientMsg(HMESSAGEREAD hRead);
        LTBOOL HandleObjectiveMessage(char* pMsg);
        LTBOOL HandleTransmissionMessage(char* pMsg);
        LTBOOL HandleCreditsMessage(char* pMsg);
        LTBOOL HandlePopupMessage(char* pMsg);
        LTBOOL HandleFadeScreenMessage(char* pMsg, LTBOOL bFadeIn);
        LTBOOL HandleMissionTextMessage(char* pMsg);
        LTBOOL HandleMissionFailedMessage(char* pMsg);
        LTBOOL HandleMusicMessage(const char* szMsg);

		void	SetNetName(const char* sNetName); 
		char*	GetNetName() { return(m_sNetName); }
        uint32  GetTeamID() { return(m_dwTeamID); }
		void	UpdateTeamID();

		void	SetChatting(LTBOOL bChatting);

		//used only for ending co-op levels
		LTBOOL  IsReadyToExit() {return m_bReadyToExit;}
		void	ReadyToExit(LTBOOL bReady) {m_bReadyToExit = bReady;}

		void	ForceDuck(LTBOOL bForce) {m_bForceDuck = bForce;}

		// called after the player has been created, but before it is initted for multiplayer
		void	PreMultiplayerInit();

		int   GetFragCount() { return(m_nFragCount); }
		void  SetFragCount(int nFrags);
		void  IncFragCount();
		void  DecFragCount();

		void  AddToScore(int nAdd);

		int   GetRespawnCount() { return(m_nRespawnCount); }
		void  SetRespawnCount(int nRespawns) { m_nRespawnCount = nRespawns; }
		void  IncRespawnCount() { m_nRespawnCount++; }

        LTBOOL IsFlashlightOn() const { return m_FlashlightInfo.bOn; }
        const LTVector& GetFlashlightPos() const { return m_FlashlightInfo.vPos; }

        void    DoWeaponChange(uint8 nWeaponId);

		void SetRestoreAmmoId(LTFLOAT fRestoreAmmoId) { m_fRestoreAmmoId = fRestoreAmmoId; }

		CPlayerSummaryMgr* GetPlayerSummaryMgr() { return &m_PlayerSummary; }

		void TeleportClientToServerPos();

        LTFLOAT ComputeDamageModifier(ModelNode eModelNode);

		const char* GetHeadSkinFilename() const;

		void SendIDToClients();

		// Handshake status of the player
		LTBOOL HasDoneHandshake() { return m_bHasDoneHandshake; }
		void FinishHandshake() { m_bHasDoneHandshake = LTTRUE; }
	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        void    WeaponCheat(uint8 nWeaponId);

		void	Reset();
		void	StartDeath();
        void    HandleDead(LTBOOL bRemoveObj);
		void	InitializeWeapons(CWeapons* pWeapons);
		char*	GetDamageSound(DamageType eType);
		char*	GetDeathSound();

		LTFLOAT	GetFootstepVolume();

		virtual void ProcessDamageMsg(HMESSAGEREAD hRead);

        virtual LTVector GetHeadOffset() { return GetPlayerHeadOffset(g_pModelButeMgr, m_eModelStyle); }
        virtual LTBOOL ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);

        virtual LTBOOL Activate(LTVector vPos, LTVector vDir, LTBOOL bEditMode=LTFALSE);

		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);


	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL   InitialUpdate(int nInfo);
        LTBOOL   Update();

		void	CreatePVWeapon();

		virtual void UpdateCommands();		// Update our animator based on client commands
		virtual void UpdateMovement();		// Update player movement

        void UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers);

		void UpdateClientPhysics();		// Update client physics state.
		void UpdateAirLevel();			// Update player's air
		void UpdateClientEffects();		// Update client side effects

        void UpdateInterface(LTBOOL bForceUpdate=LTFALSE); // Send client interface info.
		void ResetPlayer(LTBOOL bSpectatorChange=LTFALSE); // Reset the player
		void ResetHealth();
		void ResetInventory(LTBOOL bRemoveGear=LTTRUE);
		void ResetModel();

		void TweakCameraOffset();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		void DoAutoSave();

        LTBOOL ProcessTriggerMsg(HMESSAGEREAD hRead);

        LTBOOL PlayingCinematic(LTBOOL bStopCinematic=LTFALSE);

		void  HandleGameRestore();

		void SetForceUpdateList(ForceUpdate* pFU);
		void UpdateSpecialFX();

		void UpdateClientViewPos();
		void UpdateConsoleVars();

        void TeleFragObjects(LTVector & vPos);

		void HandleTeleport(TeleportPoint* pTeleportPoint);

		void HandleLinkBroken(HOBJECT hObj);
		void BuildCameraList();

		void WriteVehicleMessage(HMESSAGEWRITE hWrite);

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
		void	SetRollVel(float fVal)		{ChangeSpeedsVar(m_fRollVel, fVal);}
		void	SetJumpVel(float fVal)		{ChangeSpeedsVar(m_fJumpVel, fVal);}
		void	SetSwimVel(float fVal)		{ChangeSpeedsVar(m_fSwimVel, fVal);}
		void	SetMoveMul(float fVal)		{ChangeSpeedsVar(m_fMoveMultiplier, fVal);}
		void	SetJumpVelMul(float fVal)	{ChangeSpeedsVar(m_fJumpMultiplier, fVal);}
		void	SetLadderVel(float fVal)	{ChangeSpeedsVar(m_fLadderVel, fVal);}
		void	SetZipCordVel(float fVal)	{ChangeSpeedsVar(m_fZipCordVel, fVal);}

		void	SetPhysicsModel(PlayerPhysicsModel eModel=PPM_NORMAL, LTBOOL bUpdateClient=LTTRUE);
		void	SetVehiclePhysicsModel(PlayerPhysicsModel eModel);
		void	SetNormalPhysicsModel();

		void	RemoveVehicleModel();

		void	AcquireDefaultWeapon();
		void	AcquireWeapon(char* pWeaponName);
		void	AcquireGear(char* pGearName);
		void	ChangeToWeapon(char* pWeaponName);


	private :  // Data members

        LTFLOAT      m_fOldHitPts;
        LTFLOAT      m_fOldArmor;
        LTFLOAT      m_fOldAirLevel;
        LTFLOAT      m_fAirLevel;
		LTFLOAT		 m_fDamageTime;
        LTVector     m_vOldModelColor;
        LTFLOAT      m_fOldModelAlpha;

		// When we respawn, this is incremented so we ignore packets from an older teleport.

        uint8       m_ClientMoveCode;

		PlayerState m_eState;

        LTBOOL      m_b3rdPersonView;
        uint32      m_nSavedFlags;
		GameType	m_eGameType;
		LTBOOL		m_bRespawnCalled;

		// Animator

		CAnimatorPlayer	m_Animator;

		// Info about cheats...

        LTBOOL       m_bRunLock;
        LTBOOL       m_bTweakingMovement;
        LTBOOL       m_bGodMode;

        LTBOOL       m_bAllowInput;

        LTBOOL       m_bChatting;

	    uint16      m_nClientChangeFlags;

		HMESSAGEREAD	m_hClientSaveData;  // Client data to save

		HSTRING			m_hstrStartLevelTriggerTarget;
		HSTRING			m_hstrStartLevelTriggerMessage;

		PlayerPhysicsModel	m_ePPhysicsModel;
		HOBJECT				m_hVehicleModel;

		LTBOOL				m_bReadyToExit;
		LTBOOL				m_bForceDuck;

		CPlayerSummaryMgr	m_PlayerSummary;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...


		CPlayerAttachments*		m_pPlayerAttachments;	// Our attachments aggregate

		HOBJECT		m_CurContainers[MAX_TRACKED_CONTAINERS];
        uint32      m_nCurContainers;

        LTBOOL      m_bFirstUpdate;
		LTBOOL		m_bNewLevel;

        uint32      m_PStateChangeFlags;        // Which things need to get sent to client.

		int*			m_pnOldAmmo;

		HCLIENT			m_hClient;

		// Data members used in load/save...

        uint32      m_dwLastLoadFlags;


		// Multiplayer data

		char		m_sNetName[NET_NAME_LENGTH];
		int			m_nFragCount;
		int			m_nRespawnCount;
        uint32      m_dwTeamID;

		LTBOOL		m_bHasDoneHandshake; // Is the handshaking done yet?

		// ForceUpdate camera support...

		CTList<LPBASECLASS>	m_Cameras;
        LTBOOL               m_bCameraListBuilt;


		// Motion status from client

        uint8       m_nMotionStatus;
        uint8       m_nWeaponStatus;

		// BL 09/30/00 m_fRestorAmmoId is a HACK to fix ammo save/load 
		LTFLOAT		m_fRestoreAmmoId;


		// Flashlight info

		FlashlightInfo		m_FlashlightInfo;

		// Client/server movement interaction variables
		LTVector	m_vLastClientPos;	// Last position provided by the client
		LTBOOL		m_bUseLeash;	    // Is leashing turned on?

		// Added for Update 1.002.  This is used to store the player's ziphook
		// state...

		uint8		m_nZipState;

		// Console variables used to tweak player movement...

		CVarTrack	m_LeashLenTrack;
		CVarTrack	m_LeashSpringTrack;
		CVarTrack	m_LeashSpringRateTrack;
		CVarTrack	m_ShowNodesTrack;
		CVarTrack	m_MoveVelMulTrack;
		CVarTrack	m_SwimVelTrack;
		CVarTrack	m_LadderVelTrack;
		CVarTrack	m_ZipCordVelTrack;

		float		m_fLeashLen;
		float		m_fLeashSpring;
		float		m_fLeashSpringRate;
		float		m_fZipCordVel;

        LTBOOL       m_bShowNodes;
        LTBOOL       m_bLevelStarted;
        LTBOOL       m_bWaitingForAutoSave;

		char		m_szMultiplayerSkin[128];
		char		m_szMultiplayerHead[128];
};


#endif  // __CPLAYER_OBJ_H__