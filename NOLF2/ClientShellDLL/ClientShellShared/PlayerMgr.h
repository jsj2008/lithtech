// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.h
//
// PURPOSE : Definition of class used to manage the client player
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_MGR_H__
#define __PLAYER_MGR_H__

#include "DamageFxMgr.h"
#include "ContainerCodes.h"
#include "WeaponMgr.h"
#include "ilttexinterface.h"


// Forward declarations to reduce header dependancy.
class CHeadBobMgr;
class CCameraOffsetMgr;
class CFlashLightPlayer;
class CGadgetDisabler;
class CSearcher;
class CMoveMgr;
class CVisionModeMgr;
class CAttachButeMgr;
class CLeanMgr;
class CClientWeaponMgr;
class IClientWeaponBase;
class CPlayerCamera;
class CTargetMgr;
class CPlayerViewAttachmentMgr;
class CVolumeBrushFX;

class CPlayerMgr 
{
public:
	CPlayerMgr();
	virtual ~CPlayerMgr();

	virtual LTBOOL	Init();
	virtual void	Term();
	virtual void	Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	virtual void	Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState);

	virtual void	OnEnterWorld();
	virtual void	OnExitWorld();

	void	PreUpdate();
	void	FirstUpdate();
	virtual void Update();
	void	UpdatePlaying();
	void	PostUpdate();

	void	FirstPlayingUpdate();

	virtual float	GetJumpVelocity(float fJumpVel, float fSuperJumpVel) { return fJumpVel; }

    virtual LTBOOL	OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

	virtual LTBOOL	OnCommandOn(int command);
	virtual LTBOOL	OnCommandOff(int command);

	LTBOOL	OnKeyDown(int key, int rep);
	LTBOOL	OnKeyUp(int key);


	//**************************************************************************
	// Camera/Player access function
	//**************************************************************************
	IClientWeaponBase*	GetCurrentClientWeapon() const;
	CClientWeaponMgr*	GetClientWeaponMgr() const	{ ASSERT( 0 != m_pClientWeaponMgr ); return m_pClientWeaponMgr; }

	CMoveMgr*			GetMoveMgr()				{ return m_pMoveMgr; }
	ContainerCode		GetCurContainerCode() const { return m_eCurContainerCode; }
	CVisionModeMgr*		GetVisionModeMgr()			{ return m_pVisionModeMgr; }

    CAttachButeMgr*     GetAttachButeMgr()          { return m_pAttachButeMgr; }
	CCameraOffsetMgr*	GetCameraOffsetMgr()		{ return m_pCameraOffsetMgr; }
	CPlayerCamera*		GetPlayerCamera()			{ return m_pPlayerCamera; }
	CLeanMgr*			GetLeanMgr()				{ return m_pLeanMgr; }
	CTargetMgr*			GetTargetMgr()				{ return m_pTargetMgr; }
	CSearcher*			GetSearcher()				{ return m_pSearcher; }
	CGadgetDisabler*	GetGadgetDisabler()			{ return m_pGadgetDisabler;	}
	
	bool		InSafetyNet()	const				{ return m_bInSafetyNet; }

	CPlayerViewAttachmentMgr*	GetPlayerViewAttachmentMgr() { return m_pPVAttachmentMgr; }

	HLOCALOBJ	GetCamera()	const					{ return m_hCamera; }

    LTBOOL		IsUsingExternalCamera()	const	{ return m_bUsingExternalCamera; }
	LTBOOL		IsCameraListener()		const	{ return m_bCamIsListener; }
	LTBOOL		InCameraGadgetRange(HOBJECT hObj);
	void		AllowPlayerMovement(LTBOOL bAllowPlayerMovement);
    void		SetMouseInput(LTBOOL bAllowInput, LTBOOL bRestoreBackupAngels);

    void		SetCameraFOV(float fFovX, float fFovY);
	void		ResetCamera();

    LTBOOL		IsPlayerMovementAllowed()       { return m_bAllowPlayerMovement;}
    LTBOOL      IsZoomed()          const       { return (m_nZoomView > 0 || m_bZooming); }
	LTBOOL		IsZooming()			const		{ return m_bZooming; }
    LTBOOL      IsUnderwater()      const       { return IsLiquid(m_eCurContainerCode); }
    bool		UsingSpyVision()    const       { return m_bSpyVision; }
    LTBOOL      UsingCamera()		const       { return m_bCamera; }

    LTBOOL      IsFirstPerson();
    LTBOOL      IsPlayerInWorld();

	bool		IsSearching();
	bool		IsDisabling();

	bool		IsMouseStrafing()	const { return (m_bStrafing ? true : false); }

	PlayerState	GetPlayerState()	const { return m_ePlayerState; }
    LTBOOL      IsPlayerDead()      const { return (m_ePlayerState == PS_DEAD || m_ePlayerState == PS_DYING); }
	float       GetRespawnTime()	const { return  m_fEarliestRespawnTime;}
	void		SetRespawnTime( float fEarliestRespawnTime ) { m_fEarliestRespawnTime = fEarliestRespawnTime; }
	bool		GetCancelRevive( ) const { return m_bCancelRevive; }
	void		SetCancelRevive( bool bCancelRevive );

    uint32      GetPlayerFlags()    const { return m_dwPlayerFlags; }

    void        GetCameraRotation(LTRotation &rRot);
    void        GetPlayerRotation(LTRotation &rRot);
    void        GetPlayerPitchYawRoll(LTVector & vVec) const { vVec.x = m_fPlayerPitch; vVec.y = m_fPlayerYaw; vVec.z = m_fPlayerRoll; }

	float		GetPitch()  const { return m_fPitch; }
    float		GetYaw()    const { return m_fYaw; }
    float		GetRoll()   const { return m_fRoll; }

    float		GetPlayerPitch()    const { return m_fPlayerPitch; }
    float		GetPlayerYaw()      const { return m_fPlayerYaw; }
    float		GetPlayerRoll()     const { return m_fPlayerRoll; }

	// These should only be called after first getting the value
	// from the GetXXX() functions above...

    void        SetPitch(float fPitch)     { m_fPitch  = fPitch; }
    void        SetYaw(float fYaw)         { m_fYaw    = fYaw; }
    void        SetRoll(float fRoll)       { m_fRoll   = fRoll; }

    void        SetPlayerPitch(float fPitch)   { m_fPlayerPitch = fPitch; }
    void        SetPlayerYaw(float fYaw)       { m_fPlayerYaw   = fYaw; }
    void        SetPlayerRoll(float fRoll)     { m_fPlayerRoll  = fRoll; }

    LTBOOL		IsSpectatorMode()               { return m_bSpectatorMode; }
    void        SetSpectatorMode(LTBOOL bOn=LTTRUE);
 
	LTBOOL		IsInvisibleMode()               { return m_bInvisibleMode; }
    void        SetInvisibleMode(LTBOOL bOn)	{ m_bInvisibleMode = bOn; }

	uint8		GetSoundFilter()		{ return m_nSoundFilterId; }
	uint8		GetGlobalSoundFilter()	{ return m_nGlobalSoundFilterId; }

	void		StartServerAccurateRotation()	{ m_bServerAccurateRotation = true; }
	void		EndServerAccurateRotation()		{ m_bServerAccurateRotation = false; }

	void		StartSendingCameraOffsetToServer()	{ m_bSendCameraOffsetToServer = true; }
	void		EndSendingCameraOffsetToServer()	{ m_bSendCameraOffsetToServer = false; }

	//**************************************************************************
	// Camera/Player function
	//**************************************************************************
	void		UpdateCamera();

    void		ShakeScreen(LTVector vAmount);

	void		ClearCurContainerCode();

	void		BeginSpyVision();
	void		EndSpyVision();

	void		UpdateModelGlow();
    LTVector&   GetModelGlow() {return m_vCurModelGlow;}

	void		ClearPlayerModes(LTBOOL bWeaponOnly = LTFALSE);
	void		RestorePlayerModes();

	void		UpdatePlayerFlags();
    void        HandleWeaponDisable(LTBOOL bDisabled);

	void        ChangeWeapon(uint8 nWeaponId, uint8 nAmmoId = WMGR_INVALID_ID, int dwAmmo = -1);
	void        ToggleHolster(bool bPlayDeselect);
	void		LastWeapon();

    virtual void DoActivate();

	void		Teleport(const LTVector & vPos);

    void		TurnOffAlternativeCamera(uint8 nCamType);
    void		TurnOnAlternativeCamera(uint8 nCamType, float fFovX, float fFovY);
    void		SetExternalCamera(LTBOOL bExternal=LTTRUE);

	LTBOOL		PreRender();

	LTBOOL		CanChangeFOV() const { return (!m_bUsingExternalCamera && !m_bZooming && !m_nZoomView && !IsLiquid(m_eCurContainerCode)); }

	
	// Carrying methods...

	uint8		GetCarryingObject() const { return m_nCarryingObject; }
	bool		IsCarryingHeavyObject() const { return m_nCarryingObject == CFX_CARRY_BODY || m_nCarryingObject == CFX_CARRY_DD_CORE;}
	void		SetCarryingObject( uint8 nCarry, bool bUpdateCameraDip );
	uint8		CanCarryObject();
	bool		CanDropCarriedObject() { return m_bCanDropCarriedObject; }
	void		SetCanDropCarriedObject( bool bCanDrop ) { m_bCanDropCarriedObject = bCanDrop; }
	
	CFlashLightPlayer * GetFlashLight() const { return m_pFlashLight; }

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
	void	HandleWeaponChanged(uint8 nWeaponId,uint8 nAmmoId, bool bImmediateSwitch);

	void	AttachCameraToHead( bool bAttach = true, bool bInterpolate = false );

	//returns the id of a gadget that has the specified damagetype
	uint8	GetGadgetFromDamageType(DamageType eDamageType);

	//returns whether the activate key should be treated as a fire key
	bool	FireOnActivate();

	// returns if the camera duck movement has stopped or not...
	bool	IsFinishedDucking(bool bUp=true) const 
	{ 
		return ( bUp ? (m_fCamDuck == 0.0f) : (m_fCamDuck == m_fMaxDuckDistance) ); 
	} 

	// Is the camera using an animation to update it's position...
	LTBOOL	IsCameraAttachedToHead() const { return m_bCameraAttachedToHead; }

protected:

    void	ShowPlayer(LTBOOL bShow=LTTRUE);
	void	UpdateServerPlayerModel();

	//		Handle OnMessage Type			(message)
	void	HandleMsgShakeScreen			(ILTMessage_Read*);
	void	HandleMsgPlayerStateChange		(ILTMessage_Read*);
	void	HandleMsgClientPlayerUpdate		(ILTMessage_Read*);
    void    HandleMsgWeaponChange			(ILTMessage_Read*);
	void	HandleMsgPlayerDamage			(ILTMessage_Read*);
	void	HandleMsgPlayerOrientation		(ILTMessage_Read*);
    void    HandleMsgChangeWorldProperties	(ILTMessage_Read*);
	void	HandleMsgGadgetTarget			(ILTMessage_Read*);
	void	HandleMsgSearch					(ILTMessage_Read*);
	void	HandleMsgAddPusher				(ILTMessage_Read*);
    void	HandleMsgObjectivesData			(ILTMessage_Read*);


	// Camera helper functions...

	void	UpdateCameraZoom();
	void	UpdateCameraDisplacement();
	void	UpdateCameraShake( LTVector& vDisplacement );
	void	UpdateVehicleCamera( LTVector& vDisplacement );
	void	UpdateCameraSway();
	void	InitPlayerCamera();
    LTBOOL	UpdatePlayerCamera();
	void	UpdateCameraPosition();
	void	UpdateMultiplayerCameraPosition();
	void	CalculateCameraRotation();
    LTBOOL	UpdateCameraRotation();
	void	UpdatePlayerInfo(bool bPlaying);
    LTBOOL	UpdateAlternativeCamera();
	
	void	BeginZoom();
    void    HandleZoomChange(LTBOOL bReset=LTFALSE);
	void	EndZoom();

	void	Update3rdPersonInfo();
	void	HideShowAttachments(HOBJECT hObj);

	void	GetPlayerHeadPosRot(LTVector & vPos, LTRotation & rRot);
	
	void	ChangeWeapon(ILTMessage_Read *pMsg);
	void	UpdateContainers();
    void    UpdateUnderWaterFX(LTBOOL bUpdate=LTTRUE);
    void    UpdateBreathingFX(LTBOOL bUpdate=LTTRUE);
	void	UpdateDynamicOccluders(HLOCALOBJ* pContainerArray, uint32 nNumContainers);
	void	UpdateWeaponModel();
	void	StartWeaponRecoil();
	void	DecayWeaponRecoil();
	void	UpdateDuck();
	void	UpdatePlayer();
	void	UpdateDistanceIndicator();
	void	UpdateSoundFilters(uint8 nSoundFilterId);
	CVolumeBrushFX* UpdateVolumeBrushFX(CVolumeBrushFX* pFX, ContainerCode & eCode);
	

	void	HandleRespawn();

	virtual void InitTargetMgr();

	void	UpdateDamage();
	void	ClearDamageSectors();


	//switches weapons to a gadget that has the specified damagetype
	bool	UseGadget(DamageType eDamageType);


protected:
	//
	// To reduce header dependancies, these classes have been forward declared,
	// the class declared as a pointer, and then formally instantiated/destroyed
	// during the construction/destruction of the class using dynamic memory calls.
	//
	//save these
	CHeadBobMgr            *m_pHeadBobMgr;           // Handle head/weapon bob/cant
	CCameraOffsetMgr       *m_pCameraOffsetMgr;      // Adjust camera orientation
	CFlashLightPlayer      *m_pFlashLight;           // flash light for the player
	CGadgetDisabler        *m_pGadgetDisabler;       // Handles the disabling of a gadget
	CSearcher              *m_pSearcher;             // Handles searching bodies and props
	CMoveMgr               *m_pMoveMgr;              // Always around...
	CVisionModeMgr         *m_pVisionModeMgr;        // Controls the vision mode switching and updating
	CAttachButeMgr         *m_pAttachButeMgr;
	CLeanMgr               *m_pLeanMgr;              // Handles the player leaning
	CClientWeaponMgr       *m_pClientWeaponMgr;      // client weapon manager
	CTargetMgr			   *m_pTargetMgr;			 // handles tracking what the player is aimed at
	CPlayerViewAttachmentMgr *m_pPVAttachmentMgr;	 // handles model attachments to our player-view models (weapons, vehicles)

	float          m_fYawBackup;
	float          m_fPitchBackup;

	// Player movement variables...

    uint32          m_dwPlayerFlags;    // What is the player doing
	PlayerState		m_ePlayerState;		// What is the state of the player

	// Player update stuff...

    LTBOOL          m_bLastSent3rdPerson;

	LTRotation	  m_rRotation;                // Player view rotation
    float         m_fPitch;                   // Pitch of camera
    float         m_fYaw;                     // Yaw of camera
    float         m_fRoll;                    // Roll of camera
    float         m_fFireJitterPitch;         // Weapon firing jitter pitch adjust
    float         m_fFireJitterYaw;           // Weapon firing jitter yaw adjust

    float         m_fPlayerPitch;             // Pitch of player object
    float         m_fPlayerYaw;               // Yaw of player object
    float         m_fPlayerRoll;              // Roll of player object

	//the euler angles for the orientation that the camera was in when it was attached
	//to the model. This allows for the orientation of the player to be taken from
	//the model when it ends
	float			m_fModelAttachPitch;
	float			m_fModelAttachYaw;
	float			m_fModelAttachRoll;

    LTBOOL          m_bAllowPlayerMovement;     // External camera stuff
    LTBOOL          m_bLastAllowPlayerMovement;
    LTBOOL          m_bWasUsingExternalCamera;  // so we can detect when we start using it
	LTBOOL          m_bUsingExternalCamera;
	LTBOOL			m_bCamIsListener;
    LTBOOL          m_bRestoreOrientation;
    LTBOOL			m_bCameraPosInited;     // Make sure the position is valid
	LTBOOL			m_bStartedPlaying;

    LTVector        m_vShakeAmount;         // Amount to shake screen
    LTBOOL          m_bSpectatorMode;		// Are we in spectator mode
	LTBOOL			m_bInvisibleMode;		// Are we in invisible mode

	// Glowing models...

    LTVector    m_vCurModelGlow;        // Current glowing model light color
    LTVector    m_vMaxModelGlow;        // Max glowing model light color
    LTVector    m_vMinModelGlow;        // Min glowing model light color
    float		m_fModelGlowCycleTime;  // Current type throught 1/2 cycle
    LTBOOL      m_bModelGlowCycleUp;    // Cycle color up or down?

	// Container FX helpers...

	ContainerCode	m_eCurContainerCode;	// Code of container currently in
    float			m_fContainerStartTime;  // Time we entered current container
    float			m_fFovXFXDir;           // Variable use in UpdateUnderWaterFX()
	uint8			m_nSoundFilterId;		// SoundFilterId for our current container
	uint8			m_nGlobalSoundFilterId;	// Global (i.e., whole level) sound filter
	bool			m_bInSafetyNet;			// Are we in a saftey net container?

	// Camera zoom related variables...

	int				m_nZoomView;		// Are we in zoom mode (m_nZoomView > 0)
    LTBOOL          m_bZooming;         // Are we zooming
    LTBOOL          m_bZoomingIn;       // Are we zooming in
    float			m_fSaveLODScale;    // LOD Scale value before zooming


	// Camera ducking variables...

    LTBOOL   m_bStartedDuckingDown;      // Have we started ducking down
    LTBOOL   m_bStartedDuckingUp;        // Have we started back up
    float	 m_fCamDuck;                 // How far to move camera
    float	 m_fDuckDownV;               // Ducking down velocity
    float	 m_fDuckUpV;                 // Ducking up velocity
    float	 m_fMaxDuckDistance;         // Max distance we can duck
    float	 m_fStartDuckTime;           // When duck up/down started

    LTBOOL          m_bCamera;			// does this player currently use a camera
    LTVector        m_vSVLightScale;    // light scale for spy vision

	// Camera variables...

	HLOCALOBJ       m_hCamera;			// The camera
	CPlayerCamera  *m_pPlayerCamera;	// Handle 3rd person view

	bool			m_bCameraAttachedToHead;
	
    LTBOOL          m_bFirstUpdate;     // Is this the first update

    LTBOOL          m_bPlayerUpdated;   // Has the server sent us a player update?

    LTBOOL      m_bStrafing;            // Are we strafing?  This used to implement mouse strafing.
    LTBOOL      m_bHoldingMouseLook;    // Is the user holding down the mouselook key?

    float       m_fEarliestRespawnTime;
	bool		m_bCancelRevive;

    uint16      m_nPlayerInfoChangeFlags;
    float		m_fPlayerInfoLastSendTime;

	// Container FX helpers...

    LTBOOL      m_bUseWorldFog;     // Tells if we should use global fog settings or
									// let the container handling do it.

    HLTSOUND    m_hContainerSound;  // Container sound...

	bool		m_bCameraDip;
	uint8		m_nCarryingObject;
	bool		m_bCanDropCarriedObject;

	float		m_fDamage[kNumDamageSectors];

	bool		m_bServerAccurateRotation;
	bool		m_bSendCameraOffsetToServer;

	float		m_fDistanceIndicatorPercent;
	HTEXTURE	m_hDistanceIndicatorIcon;

	bool 		m_bSpyVision;

	//used to track what wepon player was using before auto-switching to gadget
	uint8		m_nPreGadgetWeapon;

	// are we currently trying to auto=switch to a gadget
	bool		m_bChangingToGadget;

	// are we in the middle of a weapon switch animation
	bool		m_bSwitchingWeapons;

	float		m_fMultiplayerDeathCamMoveTimer;
	float		m_fMultiAttachDeathCamTimer;

	bool		m_bLerpAttachedCamera;
};

extern CPlayerMgr* g_pPlayerMgr;

#endif // __PLAYER_MGR_H__
