// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.h
//
// PURPOSE : Definition of class used to manage the client player
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_MGR_H__
#define __PLAYER_MGR_H__

#include "DamageFxMgr.h"
#include "ContainerCodes.h"
#include "ilttexturemgr.h"
#include "EngineTimer.h"
#include "objectdetector.h"
#include "ClientFXSequence.h"

#ifndef __SONICDATA_H__
#include "SonicData.h"
#endif//__SONICDATA_H__

extern VarTrack	g_vtActivationDistance;
extern VarTrack	g_vtTargetDistance;
extern VarTrack	g_vtPickupDistance;

enum EffectState
{
	eEffectState_Entering,
	eEffectState_In,
	eEffectState_Exiting,
	eEffectState_Out,
};

// Forward declarations to reduce header dependancy.
class HeadBobMgr;
class CCameraOffsetMgr;
class Flashlight;
class CMoveMgr;
class CLeanMgr;
class CClientWeaponMgr;
class CPlayerCamera;
class CTargetMgr;
class CPlayerViewAttachmentMgr;
class CVolumeBrushFX;
class CPlayerBodyMgr;
class CHUDOverlay;
class CTurretFX;
class CForensicObjectFX;

class CHealthEffect
{
public:
	CHealthEffect();
	virtual ~CHealthEffect();

	virtual	bool	Init(const char* szEffectName);
	void			InitTimer();

	//state functions
	void	Update();
	
	void	Start(float fDuration = 0.0f,bool bFromMsg = true); //start effect
	void	Stop(bool bFromMsg = true); //fade effect out

	void	Reset(); //stop and clear all state information

	void	Save(ILTMessage_Write *pMsg);
	void	Load(ILTMessage_Read *pMsg);

	static const uint8	kPlayVolume;

protected:
	//actually turns on/off the effect
	void	Play(bool bOn);

	HLTSOUND		m_hLoopSound;

	bool			m_bStartedByMsg;
	bool			m_bStartedByHealth;

	StopWatchTimer	m_Timer;
	EffectState		m_eState;

	float			m_fTransitionTime;
	uint32			m_nHealthThreshold;
	HRECORD			m_hSoundRec;
};

class CPlayerMgr 
{
public:
	CPlayerMgr();
	virtual ~CPlayerMgr();

	virtual	bool	Init();
	virtual void	Term();
	virtual void	Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	virtual void	Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	virtual void	OnEnterWorld();
	virtual void	PreExitWorld();
	virtual void	OnExitWorld();

	void	PreUpdate();
	void	FirstUpdate();
	virtual void Update();
	void	UpdatePlaying();
	void	UpdateNotPlaying();
	void	PostUpdate();

	void	FirstPlayingUpdate();

	virtual	bool	OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

	virtual	bool	OnCommandOn(int command);
	virtual	bool	OnCommandOff(int command);
	virtual void	OnClearAllCommands();

	bool	OnKeyDown(int key, int rep);
	bool	OnKeyUp(int key);

	//	Gets the amount of time that has elapsed for the local player's update interval
	float	GetPlayerTimerElapsedS( ) const;

	//**************************************************************************
	// Mgr access functions
	//**************************************************************************

	CClientWeaponMgr*			GetClientWeaponMgr()			const { return m_pClientWeaponMgr; }
	CMoveMgr*					GetMoveMgr()					const { return m_pMoveMgr; }
	CCameraOffsetMgr*			GetCameraOffsetMgr()			const { return m_pCameraOffsetMgr; }
	CPlayerCamera*				GetPlayerCamera()				const { return m_pPlayerCamera; }
	CLeanMgr*					GetLeanMgr()					const { return m_pLeanMgr; }
	CTargetMgr*					GetTargetMgr()					const { return m_pTargetMgr; }
	CPlayerViewAttachmentMgr*	GetPlayerViewAttachmentMgr()	const { return m_pPVAttachmentMgr; }
	HeadBobMgr*					GetHeadBobMgr()					const { return m_pHeadBobMgr; }

	ContainerCode		GetCurContainerCode() const { return m_eCurContainerCode; }
	HOBJECT				GetCurContainerObject( ) const { return m_hCurContainerObject; }
	
	bool		InSafetyNet()	const				{ return m_bInSafetyNet; }

	void		AllowPlayerMovement(bool bAllowPlayerMovement);
	void		SetMouseInput(bool bAllowInput, bool bRestoreBackupAngels);

   	bool		IsPlayerMovementAllowed()			{ return m_bAllowPlayerMovement;}
	bool		IsUnderwater()		const			{ return IsLiquid(m_eCurContainerCode); }
	bool		IsSwimmingAllowed( );
	
	bool		IsPlayerInWorld();

	// Called when the player object is first added to the client.
	void		InitLocalPlayer( CCharacterFX& characterFx );
	
	// GRENADE PROTOTYPE
	bool		GrenadeReady() const {return (m_hPreGrenadeWeapon && !m_bSwitchingWeapons); }

	bool		IsManualAim( ) const { return m_bManualAim; }

	bool		IsMouseStrafing()	const { return m_bStrafing; }

	PlayerState	GetPlayerState()	const { return m_ePlayerState; }
	bool		IsPlayerDead()		const	{ return (m_ePlayerState == ePlayerState_Dead || 
												m_ePlayerState == ePlayerState_Dying_Stage1 || 
												m_ePlayerState == ePlayerState_Dying_Stage2 ); 
											}
	bool		IsPlayerAlive()		const	{ return (m_ePlayerState == ePlayerState_Alive); }
	StopWatchTimer const& GetRespawnTime()	const { return  m_EarliestRespawnTime;}
	bool		WillRespawnOnTimeOut() const {return m_bRespawnWhenTimedOut;}
	float		GetRespawnPenalty() const { return m_fRespawnPenalty;}

    uint32      GetPlayerFlags() const { return m_dwPlayerFlags; }

	bool		WasPlayerBlocked()	const { return m_bBlocked; }
	void		ResetPlayerBlocked() { m_bBlocked = false; }

	bool		IsSpectating() const	{ return ( GetPlayerState() == ePlayerState_Spectator || m_eSpectatorMode != eSpectatorMode_None); }
	SpectatorMode GetSpectatorMode( ) const { return m_eSpectatorMode; }
	void		SetSpectatorMode( SpectatorMode eSpectatorMode );
	bool		AllowSpectatorMode( SpectatorMode eSpectatorMode ) const;
	SpectatorMode GetNextSpectatorMode( SpectatorMode eSpectatorMode ) const;
	CCharacterFX* GetNextSpectatorFollow( CCharacterFX* pCurCharFx ) const;
	void		SetSpectatorFollowCharacter( CCharacterFX* pFound );

	// Ensures the target being followed is hidden to the player.
	void		UpdateSpectatorFollowTarget( );

	// Requests a spectator mode from the server.
	void	RequestSpectatorMode( SpectatorMode eSpectatorMode );

	// Gets the current follow character for spectator mode.
	HOBJECT GetSpectatorFollowTarget( ) const { return m_hSpectatorFollowCharacter; }

	// Sets target of tracking spectator mode.
	void		SetSpectatorTrackTarget( HOBJECT hSpectatorTrackTarget ) { m_hSpectatorTrackTarget = hSpectatorTrackTarget; }
	HOBJECT		GetSpectatorTrackTarget( ) const { return m_hSpectatorTrackTarget; }

	bool		CanRespawn( ) const;

	bool		IsInvisibleMode() const		{ return m_bInvisibleMode; }
	void		SetInvisibleMode(bool bOn);

	void		StartServerAccurateRotation()	{ m_bServerAccurateRotation = true; }
	void		EndServerAccurateRotation()		{ m_bServerAccurateRotation = false; }

	//**************************************************************************
	// Camera/Player function
	//**************************************************************************
	void		UpdateCamera();

    void		ClearCurContainerCode();

	void		UpdateModelGlow();
    LTVector&   GetModelGlow() {return m_vCurModelGlow;}

	void		ClearPlayerModes(bool bWeaponOnly = false);
	void		ClearEarringEffect( );
	void		RestorePlayerModes();

	void		UpdatePlayerFlags();
	void		HandleWeaponDisable(bool bDisabled);

	virtual void DoActivate();

	void		Teleport(const LTVector & vPos);

    bool		PreRender();

	Flashlight* GetFlashLight() const { return m_pFlashLight; }

	enum eConstants
	{
		kNumDamageSectors = 12,
	};

	float GetDamageFromSector(uint8 nSector) 
	{
		if (nSector >= kNumDamageSectors) 
			return 0.0f; 
		else
			return m_fDamage[nSector];
	};


	float		GetDistanceIndicatorPercent() const { return m_fDistanceIndicatorPercent; }
	HTEXTURE	GetDistanceIndicatorIcon() const { return m_hDistanceIndicatorIcon; }


	// This should only be called from CClientWeaponMgr::ChangeWeapon()...
	// jrg 9/2/02 - this will get called twice most of the time when switching weapons:
	//			once when the weapon switch is started
	//			and again when it completes (if a switching animation was needed)
	//		bImmediateSwitch will be true on the second call (or the first if the switch is immediate)
	//		(I'm using the repeated call to track whether we are in mid switch)
	void	HandleWeaponChanged( HWEAPON hWeapon, HAMMO hAmmo, bool bImmediateSwitch );

	// Tests if the player meets the requirements for firing a weapon...
	bool	CanFireWeapon( ) const;
	bool	CanAltFireWeapon( ) const;
	bool	CanThrowGrenade( ) const;
 
	// returns if the camera duck movement has stopped or not...
	bool	IsFinishedDucking(bool bUp=true) const 
	{ 
		return ( bUp ? (m_fCamDuck == 0.0f) : (m_fCamDuck == m_fMaxDuckDistance) ); 
	}

	float	GetCamDuckAmount() const { return m_fCamDuck; }

	// Get the current recoil value (0.0 to 1.0)
	float GetRecoilValue() const;

	// Calculate the difference in X and Y axis movement (usually based off of mouse movement)...
	void CalculateAxisOffsets();
	const LTVector2& GetAxisOffsets() const { return m_v2AxisOffsets; }

// PLAYER_BODY...
	void	HideShowAttachments(HOBJECT hObj);
	void	ShowPlayer(bool bShow=true);

	// Check if currently in slowmo mode.
	bool	IsInSlowMo( ) const { return m_eSlowMoState != eEffectState_Out; }

	//end player initiated slowmo
	void	EndPlayerSlowMo( ); 

	double	GetSlowMoCurCharge( ) const { return m_fSlowMoCurCharge; }
	float	GetSlowMoMinCharge( ) const { return m_fSlowMoMinCharge; }
	float	GetSlowMoMaxCharge( ) const { return m_fSlowMoMaxCharge; }
	float	GetSlowMoRecharge( ) const { return m_fSlowMoRecharge; }

	double	GetFlashlightCharge( ) const { return m_fFlashlightCharge; }
	float	GetFlashlightMaxCharge( ) const { return m_fFlashlightMaxCharge; }
	float	GetFlashlightRecharge( ) const { return m_fFlashlightRecharge; }
	void	ResetFlashlight() { m_fFlashlightCharge = m_fFlashlightMaxCharge; }


	// Override sound filters from containers functionality..
	void	OverrideSoundFilter(const char* pOverrideFilterName, bool bOverrideOn);

	void	SetKiller(uint32 nKiller);


	// Used to track pickupitems that can be swapped/picked up with activate based on 
	// viewing.
	ObjectDetector&	GetPickupObjectDetector( ) { return m_PickupObjectDetector; }

	// Called by pickupobjectdetector to determine custom priority.
	static void FocusObjectDetectorSpatialCB( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, void* pUserData );
	static bool PickupObjectDetectorCustomTestCB( ObjectDetectorLink* pLink, float& fMetRR, void* pUserData );

	// Used to track forensicobjects for activating forensic tools.
	ObjectDetector&	GetForensicObjectDetector( ) { return m_ForensicObjectDetector; }

	// Called by forensicobjectdetector to determine custom priority.
	static bool ForensicObjectDetectorCustomTestCB( ObjectDetectorLink* pLink, float& fMetRR, void* pUserData );

	// Focus related interfaces
	float	GetFocusAccuracy() { return m_fFocusAccuracy; }

	// Sonic related interfaces
	const SonicData& GetSonicData() { return m_iSonicData; }
	void IntoneSonic( const char* sSonic );
	SonicType GetSonicType() const { return m_eLastSonicType; }

	// Forensic related interfaces
	CForensicObjectFX* GetForensicObject() const { return m_pForensicObject; }
	void SetForensicObject(CForensicObjectFX* pFX);

	// Stair related interfaces
	bool WithinStairVolume();

	// Cache the turret the player is activating and handle anyother player setup for operating a turret...
	void SetOperatingTurret( CTurretFX *pTurret );

	bool IsOperatingTurret( ) const { return (m_pTurretFX != NULL);	}
	CTurretFX* GetTurret( ) const { return m_pTurretFX; }

	// Test to determine if the player is carrying an object...
	bool	IsCarryingObject( ) const { return !!m_hCarryObject; }


	// Head bob node controller
	void	InitHeadBobNodeControl();
	void	TermHeadBobNodeControl();

	static void HeadBobCycleDirection( bool bMaxExtent, void* pUserData, bool bApproachFromCenter );
	static void HeadBobCustomCycleDirection( bool bMaxExtent, void* pUserData, bool bApproachFromCenter );
	static void HeadBobNodeControl( const NodeControlData& iData, void *pUserData );

	// Access node data...
	HOBJECT	GetPlayerNodeGoto( ) const { return m_hPlayerNodeGoto; }
	
	// Test if the player is at the goto node position...
	bool IsAtNodePosition( ) const { return (m_hPlayerNodeGoto && m_bAtNodePosition); }
	void ReachedNodePosition( );

	// Test if the player is at the goto node rotation...
	bool IsAtNodeRotation( ) const {return (m_hPlayerNodeGoto && m_bAtNodeRotation); }
	void ReachedNodeRotation( ) { m_bAtNodeRotation = true; }

	// Notify server the player has arrived at its destination...
	void HandleNodeArrival( );

	// Tell server about our player information.
	void	UpdatePlayerInfo(bool bPlaying, bool bForceSend);

	bool	InStoryMode() const {return m_bStoryMode;}
	bool	CanSkipStory() const { return m_bCanSkipStory;	}

	HOBJECT	GetSyncObject() const { return m_hSyncObject; }

protected:

	void	UpdateServerPlayerModel();

	//		Handle OnMessage Type			(message)
	void	HandleMsgPlayerStateChange		(ILTMessage_Read*);
	void	HandleMsgClientPlayerUpdate		(ILTMessage_Read*);
	void	HandleMsgPlayerDamage			(ILTMessage_Read*);
    void    HandleMsgChangeWorldProperties	(ILTMessage_Read*);
	void	HandleMsgAddPusher				(ILTMessage_Read*);
	void	HandleMsgSlowMo					(ILTMessage_Read*);
	bool	HandleMsgPlayerEvent			(ILTMessage_Read*);
	bool	HandleMsgPlayerBody				(ILTMessage_Read*);
	bool	HandleMsgSonic					(ILTMessage_Read*);
	bool	HandleMsgGoto					(ILTMessage_Read*);
	void	HandleMsgSpectatorMode			(ILTMessage_Read*);
	void	HandleMsgPlayerLeash			(ILTMessage_Read*);

	void	UpdateContainers();
	void	UpdateUnderWaterFX(bool bUpdate=true);
	void	UpdateBreathingFX(bool bUpdate=true);
	void	UpdateDynamicSectors(HLOCALOBJ* pContainerArray, uint32 nNumContainers);
	void	UpdateWeaponModel();
	void	StartWeaponRecoil();
	void	DecayWeaponRecoil();
	void	UpdateDuck();
	void	UpdateDistanceIndicator();
	void	UpdateSoundFilters(HRECORD nSoundFilterId, bool bOverride);
	CVolumeBrushFX* UpdateVolumeBrushFX(CVolumeBrushFX* pFX, ContainerCode & eCode);

	// Move and rotate the player based on the input offsets...
	void	UpdatePlayerMovement( );

	// Hide and show the player...
	void	UpdatePlayerVisibility( );
	

	void	HandleRespawn();

	virtual void InitTargetMgr();

	void	UpdateDamage();
	void	ClearDamageSectors();

	void	UpdateConcussionAudioEffect();

	// SlowMo State functions.
	void	UpdateSlowMo( );
	void	EnterSlowMo( HRECORD hSlowMoRecord, bool bTransition, double fSlowMoPeriod, bool bPlayer );
	void	ExitingSlowMo( );
	void	ExitSlowMo( bool bTermClientFx );

	// Specifies a prop record to use as an object the player is carrying...
	void	SetCarryObject( HRECORD hCarryProp );

	// Update the position of the object being carried...
	void	UpdateCarryObject( );

	void	SetStoryMode(bool bOn, bool bCanSkip = true);

	void	CreateWeaponEffect(const char* pszFX, const char* pszSocket);

	void	HandleSyncAction( HRECORD hSyncAction, HOBJECT hObject );

	// helpers for writing simulation log entry
	void	WritePlayerUpdateToSimulationLog(uint16 nChangeFlags);
	void	WriteNullUpdateToSimulationLog();
	void	WriteWeaponChangeToSimulationLog(HWEAPON hWeapon, HAMMO hAmmo);
	void	WritePlayerRespawnToSimulationLog();

protected:
	//
	// To reduce header dependencies, these classes have been forward declared,
	// the class declared as a pointer, and then formally instantiated/destroyed
	// during the construction/destruction of the class using dynamic memory calls.
	//
	//save these
	HeadBobMgr					*m_pHeadBobMgr;			// Handle head/weapon bob/cant
	CCameraOffsetMgr			*m_pCameraOffsetMgr;	// Adjust camera orientation
	Flashlight					*m_pFlashLight;			// flash light for the player
	CMoveMgr					*m_pMoveMgr;			// Always around...
	CLeanMgr					*m_pLeanMgr;			// Handles the player leaning
	CClientWeaponMgr			*m_pClientWeaponMgr;	// client weapon manager
	CTargetMgr					*m_pTargetMgr;			// handles tracking what the player is aimed at
	CPlayerViewAttachmentMgr	*m_pPVAttachmentMgr;	// handles model attachments to our player-view models (weapons, vehicles)
	CPlayerCamera				*m_pPlayerCamera;		// Handles camera functionality...

	
	// Player movement variables...

    uint32          m_dwPlayerFlags;    // What is the player doing
	PlayerState		m_ePlayerState;		// What is the state of the player

	SonicType		m_eLastSonicType;	// What is the last sonic button pressed

	// Player update stuff...

	bool		m_bLastSent3rdPerson;

	bool			m_bBlocked;			// Player's last attack was blocked.

    float			m_fFireJitterPitch;         // Weapon firing jitter pitch adjust
    float			m_fFireJitterYaw;           // Weapon firing jitter yaw adjust

	// Last gametime the weapon fired.
	StopWatchTimer	m_LastFireTimer;

	//variables to control semi-auto firing
	bool			m_bSemiAutoFire;
	double			m_fSemiAutoFireTime;
	bool			m_bSemiAutoFireDuringReload;

	//values for camera recoil calculation
	float			m_fCurrentRecoil;

	bool		m_bAllowPlayerMovement;		// External camera stuff
	bool		m_bLastAllowPlayerMovement;
	bool		m_bRestoreOrientation;
	bool		m_bStartedPlaying;

   	SpectatorMode	m_eSpectatorMode;		// Are we in spectator mode
	bool		m_bInvisibleMode;		// Are we in invisible mode

	// Glowing models...

    LTVector    m_vCurModelGlow;        // Current glowing model light color
    LTVector    m_vMaxModelGlow;        // Max glowing model light color
    LTVector    m_vMinModelGlow;        // Min glowing model light color
    float		m_fModelGlowCycleTime;  // Current type throught 1/2 cycle
	bool		m_bModelGlowCycleUp;	// Cycle color up or down?

	// Container FX helpers...

	ContainerCode	m_eCurContainerCode;	// Code of container currently in
    StopWatchTimer	m_ContainerStartTimer;  // Time we entered current container
	HRECORD			m_hSoundFilterIdCurrentContainer; // SoundFilterId for our current container
	HRECORD			m_hSoundFilterIdGlobal;	// Global (i.e., whole level) sound filter
	HRECORD			m_hSoundFilterIDOverride; // SoundFilterID for the override
	HRECORD			m_pSoundFilterIDCurrent;	// current SoundFilterID (in use)
	bool			m_bSoundFilterOverrideOn;	// true if the sound filter is being overridden
	bool			m_bInSafetyNet;			// Are we in a saftey net container?
	HLOCALOBJ		m_hCurContainerObject;	// object handle for container currently in

	// Camera ducking variables...

	bool		m_bStartedDuckingDown;		// Have we started ducking down
	bool		m_bStartedDuckingUp;		// Have we started back up
    float	 m_fCamDuck;                 // How far to move camera
    float	 m_fDuckDownV;               // Ducking down velocity
    float	 m_fDuckUpV;                 // Ducking up velocity
    float	 m_fMaxDuckDistance;         // Max distance we can duck
    StopWatchTimer m_StartDuckTimer;           // When duck up/down started

	// Camera variables...

		
	bool		m_bFirstUpdate;			// Is this the first update

	bool		m_bPlayerUpdated;		// Has the server sent us a player update?

	bool		m_bStrafing;			// Are we strafing?  This used to implement mouse strafing.
	
    StopWatchTimer	m_EarliestRespawnTime;
	float			m_fRespawnPenalty;

	uint16			m_nPlayerInfoChangeFlags;
	double			m_fPlayerInfoLastSendTime;
	double			m_fPlayerInfoLastUniqueSendTime;
	CLTMsgRef_Read	m_cLastPlayerInfoMsg;
	CLTMsgRef_Read	m_cLastPlayerInfoMoveMsg;
	CLTMsgRef_Read	m_cLastPlayerInfoAnimMsg;
	CLTMsgRef_Read	m_cLastPlayerInfoCameraMsg;
	
	// Container FX helpers...

	bool		m_bUseWorldFog;		// Tells if we should use global fog settings or
									// let the container handling do it.

    HLTSOUND    m_hContainerSound;  // Container sound...

	bool		m_bCameraDip;

	float		m_fDamage[kNumDamageSectors];

	bool		m_bServerAccurateRotation;

	float		m_fDistanceIndicatorPercent;
	TextureReference	m_hDistanceIndicatorIcon;

	// are we in the middle of a weapon switch animation
	bool		m_bSwitchingWeapons;

	// GRENADE PROTOTYPE
	HWEAPON		m_hPreGrenadeWeapon;
	bool		m_bChangingToGrenade;


	// Manual aim overrides autoaim.
	bool		m_bManualAim;

	bool		m_bRespawnRequested;
	bool		m_bRespawnWhenTimedOut;
	
	// The differnece from last frame of the axi (usually the mouse)...
	LTVector2	m_v2AxisOffsets;

	// The PlayerBody needs to be enabled...
	bool		m_bShouldEnableBody;

	// Timer how long we should stay dead.
	HLTSOUND		m_hDeathSound;  // Death sound...
	LTObjRef		m_hKiller;
	CClientFXLink	m_fxDeathLoop;
	CClientFXLink	m_fxHeadShot;

	// Timer to track recoil decay.
	StopWatchTimer	m_RecoilDecayTimer;

	StopWatchTimer	m_MeleeDamageEffectTimer;

	float			m_fMeleeDamageDistortionIntensity;
	float			m_fMeleeDamageDistortionI1;
	
	float			m_fMeleeDamageOverlayBlur;
	float			m_fMeleeDamageOverlayB1;

	EffectState		m_eSlowMoState;

	CHealthEffect	m_HeartBeat;
	CHealthEffect	m_Breath;

	// Times the transitions into and out of slowmo.
	HRECORD			m_hSlowMoRecord;
	CClientFXSequence m_fxSlowMoSequence;
	StopWatchTimer	m_SlowMoTimer;
	bool			m_bSlowMoPlayer;
	double			m_fSlowMoCurCharge;

	float			m_fSlowMoMinCharge;
	float			m_fSlowMoMaxCharge;
	float			m_fSlowMoRecharge;

	double			m_fFlashlightCharge;
	float			m_fFlashlightMaxCharge;
	float			m_fFlashlightRecharge;

	// Audio concussion effect helper..

	HLTSOUND m_hRingSound;
	StopWatchTimer	m_AudioTimer;
	int16	m_nAudioEffectState;

	// Used to monitor pickupitems for swapping/picking up with activate key.
	ObjectDetector	m_PickupObjectDetector;

	// Used to monitor forensic areas for activating forensic tools.
	ObjectDetector	m_ForensicObjectDetector;

	// Client-side Sonic data for the player
	SonicData		m_iSonicData;

	// If the player is currently operating a turret this provides access to it...
	CTurretFX		*m_pTurretFX;

	// If the player is currently in a forensic area, this provides access to the area's properties...
	CForensicObjectFX* m_pForensicObject;

	// Object that the player is carrying...
	LTObjRef			m_hCarryObject;

	// Flashlight button tracking
	float				m_fFlashlightButtonPressTime;
	float				m_fFlashlightTransitionTime;
	
	// A node the player needs to visit...
	LTObjRef			m_hPlayerNodeGoto;

	// Should the player align its rotation with the nodes' forward...
	bool			m_bFaceNodeForwardPitch;
	bool			m_bFaceNodeForwardYaw;
	bool			m_bAlignPitchWithNode;
	bool			m_bAlignYawWithNode;

	// Has the player reached the nodes location...
	bool			m_bAtNodePosition;

	// Has the player rotated to face in the direction of the nodes forward...
	bool			m_bAtNodeRotation;

	// Holds current character we're following in spectator mode.
	LTObjRef		m_hSpectatorFollowCharacter;

	// Target of spectator tracking.
	LTObjRef		m_hSpectatorTrackTarget;

	// Focus related stuff
	float			m_fFocusAccuracy;

	// Join grace period respawn timer.
	StopWatchTimer	m_tmrJoinGrace;

	// Is the player in story mode?
	bool			m_bStoryMode;

	// Can the player skip past this story element.
	bool			m_bCanSkipStory;

	// Last object that requested a syncaction (usally an AI)
	LTObjRef		m_hSyncObject;
};

extern CPlayerMgr* g_pPlayerMgr;

#endif // __PLAYER_MGR_H__
