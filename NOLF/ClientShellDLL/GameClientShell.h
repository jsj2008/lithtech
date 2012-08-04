// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 9/18/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_CLIENT_SHELL_H__
#define __GAME_CLIENT_SHELL_H__

#include "iclientshell.h"
#include "PlayerCamera.h"
#include "ClientServerShared.h"
#include "SFXMgr.h"
#include "Music.h"
#include "ContainerCodes.h"
#include "WeaponModel.h"
#include "LightScaleMgr.h"
#include "GlobalClientMgr.h"
#include "CMoveMgr.h"
#include "DamageFXMgr.h"
#include "ScreenTintMgr.h"
#include "ObjEditMgr.h"
#include "InterfaceMgr.h"
#include "AttachButeMgr.h"
#include "FlashLight.h"
#include "SharedMovement.h"
#include "PlayerSummary.h"
#include "IntelItemMgr.h"
#include "CameraOffsetMgr.h"
#include "HeadBobMgr.h"
#include "NetDefs.h"

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)
#define RAD2DEG(x)		(((x)*180.0f)/MATH_PI)

class CSpecialFX;
class CCameraFX;
class CGameTexMgr;


class CGameClientShell : public IClientShell
{
	public:

		CGameClientShell();
		~CGameClientShell();

		int	  GetScreenWidth();
		int	  GetScreenHeight();

        HLTSOUND PlaySoundLocal(char *szSound, BOOL bLoop = FALSE, BOOL bStream = FALSE, BOOL bGetHandle = FALSE);

        void  ProcessCheat(CheatCode nCode);
        void  ShakeScreen(LTVector vAmount);
        void  PauseGame(LTBOOL bPause, LTBOOL bPauseSound=LTFALSE);

		void  HandleRecord(int argc, char **argv);
		void  HandlePlaydemo(int argc, char **argv);
		void  HandleCheat(int argc, char **argv);
		void  HandleExitLevel(int argc, char **argv);

		void  ExitLevel();

        LTBOOL LoadGame(char* pWorld, char* pObjectsFile);
        LTBOOL SaveGame(char* pObjectsFile);

        LTBOOL QuickSave();
        LTBOOL QuickLoad();

        LTBOOL GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen);
        LTBOOL StartGame(GameDifficulty eDifficulty);

        LTBOOL LoadWorld(char* pWorldFile, char* pCurWorldSaveFile=LTNULL,
                        char* pRestoreWorldFile=LTNULL, uint8 nFlags=LOAD_NEW_GAME);
        LTBOOL StartMission(int nMissionId);
		void   InitMultiPlayer();

		int	GetMPMissionName() const {return m_nMPNameId;}
		int	GetMPMissionBriefing() const {return m_nMPBriefingId;}

        LTBOOL IsCustomLevel()   const {return m_bIsCustomLevel;}
		int GetCurrentMission() const {return m_nCurrentMission;}
		int GetCurrentLevel()	const {return m_nCurrentLevel;}

		LTBOOL DoJoinGame(char* sIpAddress);

		//returns LTTRUE if the passed in address matches the current server address
		LTBOOL CheckServerAddress(char *pszTestAddress, int nPort);

		void			SetDifficulty(GameDifficulty e);
		GameDifficulty  GetDifficulty()					{return m_eDifficulty;}
		void			SetFadeBodies(LTBOOL bFade);
		LTBOOL			GetFadeBodies()					{return m_bFadeBodies;}
		GameType		GetGameType()					{return m_eGameType;}
		void			SetGameType(GameType eGameType)	{m_eGameType = eGameType;}
		LevelEnd		GetLevelEnd()					{return m_eLevelEnd;}
		int				GetLevelEndString()				{return m_nEndString;}

        void  DoActivate(LTBOOL bEditMode);

		void  SetFarZ(int nFarZ);

        LTFLOAT             GetFrameTime()              { return m_fFrameTime; }
        CPlayerSummaryMgr*  GetPlayerSummary()          { return &m_PlayerSummary; }
        CIntelItemMgr*		GetIntelItemMgr()			{ return &m_IntelItemMgr; }
		CWeaponModel*		GetWeaponModel()			{ return &m_weaponModel; }
		CMoveMgr*			GetMoveMgr()				{ return &m_MoveMgr; }
		CDamageFXMgr*		GetDamageFXMgr()			{ return &m_DamageFXMgr; }
		ContainerCode		GetCurContainerCode() const { return m_eCurContainerCode; }
		CPlayerStats*		GetPlayerStats()			{ return m_InterfaceMgr.GetPlayerStats(); }

        CScreenTintMgr*     GetScreenTintMgr()          { return &m_ScreenTintMgr; }
		CLightScaleMgr*		GetLightScaleMgr()			{ return &m_LightScaleMgr; }

        CAttachButeMgr*     GetAttachButeMgr()          { return &m_AttachButeMgr; }
		CCameraOffsetMgr*	GetCameraOffsetMgr()		{ return &m_CameraOffsetMgr; }

		HLOCALOBJ		GetCamera()				const	{ return m_hCamera; }
		HLOCALOBJ		GetInterfaceCamera()	const	{ return m_hInterfaceCamera; }
        LTBOOL			IsUsingExternalCamera()	const	{ return m_bUsingExternalCamera; }
        LTBOOL          CanSaveGame()			const   { return (m_bInWorld && !m_bUsingExternalCamera); }
        LTBOOL          IsInWorld()				const   { return m_bInWorld; }
        LTBOOL          IsGamePaused()			const   { return m_bGamePaused; }
        LTBOOL          IsFirstUpdate()			const   { return m_bFirstUpdate; }
		char*			GetCurrentWorldName()			{ return m_strCurrentWorldName; }
        LTBOOL          SoundInited()			const   { return ( m_resSoundInit == LT_OK ) ? LTTRUE : LTFALSE; }
		LTBOOL			IsCameraListener()		const	{ return m_bCamIsListener; }

        void		MinimizeMainWindow()            { m_bMainWindowMinimized = LTTRUE; }
        LTBOOL		IsMainWindowMinimized()         { return m_bMainWindowMinimized; }
        void		RestoreMainWindow()             { m_bMainWindowMinimized = LTFALSE; }
        LTBOOL		IsSpectatorMode()               { return m_bSpectatorMode; }

		LTBOOL		InCameraGadgetRange(HOBJECT hObj);

		void		AllowPlayerMovement(LTBOOL bAllowPlayerMovement);

        LTBOOL      IsPlayerMovementAllowed()       { return m_bAllowPlayerMovement;}
        LTBOOL      IsZoomed()          const       { return (m_nZoomView > 0 || m_bZooming); }
        LTBOOL      IsUnderwater()      const       { return IsLiquid(m_eCurContainerCode); }
        LTBOOL      UsingNightVision()  const       { return m_bNightVision; }

        LTBOOL      IsFirstPerson()                 { return m_PlayerCamera.IsFirstPerson(); }

		CPlayerCamera* GetPlayerCamera() { return &m_PlayerCamera; }

		CMusic*		GetMusic()		{ return &m_Music; }
		void		InitSound();

		void		ToggleDebugCheat(CheatCode eCheat);
        void        ShowPlayerPos(LTBOOL bShow=LTTRUE)	{ m_bShowPlayerPos = bShow; }
        void        ShowCamPosRot(LTBOOL bShow=LTTRUE)  { m_bShowCamPosRot = bShow; }
        void        SetSpectatorMode(LTBOOL bOn=LTTRUE);
        void        SetPlayerNotInWorld()				{ m_bInWorld = LTFALSE; }

		void		ClearAllScreenBuffers();
		void		ClearCurContainerCode();

        LTBOOL      IsJoystickEnabled();
        LTBOOL      EnableJoystick();

		CSFXMgr*	GetSFXMgr() { return &m_sfxMgr; }

        void        FlashScreen(LTVector vFlashColor, LTVector vPos, LTFLOAT fFlashRange,
                               LTFLOAT fTime, LTFLOAT fRampUp, LTFLOAT fRampDown,
                               LTBOOL bForce=LTFALSE);

		void		BeginMineMode();
		void		EndMineMode();

		void		BeginInfrared();
		void		EndInfrared();

		void		UpdateModelGlow();
        LTVector&   GetModelGlow() {return m_vCurModelGlow;}

        LTBOOL      IsMultiplayerGame();
		LTBOOL		IsHosting();
        LTBOOL      IsPlayerInWorld();

		PlayerState	GetPlayerState()	const { return m_ePlayerState; }
        LTBOOL      IsPlayerDead()      const { return (m_ePlayerState == PS_DEAD || m_ePlayerState == PS_DYING); }

        uint32      GetPlayerFlags()    const { return m_dwPlayerFlags; }

        void        GetCameraRotation(LTRotation *pRot);
        void        GetPlayerRotation(LTRotation *pRot);
        void        GetPlayerPitchYawRoll(LTVector & vVec) const { vVec.x = m_fPlayerPitch; vVec.y = m_fPlayerYaw; vVec.z = m_fPlayerRoll; }

        void        HandleWeaponPickup(uint8 nWeaponId);
        void        HandleGearPickup(uint8 nGearId);
        void        HandleModPickup(uint8 nModId);
        void        HandleAmmoPickup(uint8 nAmmoId, int nAmmoCount);

		void		HandleMissionFailed();

        void        HandleWeaponDisable(LTBOOL bDisabled);

        void        ChangeWeapon(uint8 nWeaponId, uint8 nAmmoId, uint32 dwAmmo);
        void        DemoSerialize(ILTStream *pStream, LTBOOL bLoad);
		void		CSPrint(char* msg, ...);

        void        SetInputState(LTBOOL bAllowInput);

        LTBOOL      PreChangeGameState(GameState eNewState);
        LTBOOL      PostChangeGameState(GameState eOldState);

		void		UpdatePlayerFlags();

		// Mouse Messages
        static void OnChar(HWND hWnd, char c, int rep);
		static void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		static void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		static void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);

        LTFLOAT     GetPitch()  const { return m_fPitch; }
        LTFLOAT     GetYaw()    const { return m_fYaw; }
        LTFLOAT     GetRoll()   const { return m_fRoll; }

        LTFLOAT     GetPlayerPitch()    const { return m_fPlayerPitch; }
        LTFLOAT     GetPlayerYaw()      const { return m_fPlayerYaw; }
        LTFLOAT     GetPlayerRoll()     const { return m_fPlayerRoll; }

		// These should only be called after first getting the value
		// from the GetXXX() functions above...

        void        SetPitch(LTFLOAT fPitch)     { m_fPitch  = fPitch; }
        void        SetYaw(LTFLOAT fYaw)         { m_fYaw    = fYaw; }
        void        SetRoll(LTFLOAT fRoll)       { m_fRoll   = fRoll; }

        void        SetPlayerPitch(LTFLOAT fPitch)   { m_fPlayerPitch = fPitch; }
        void        SetPlayerYaw(LTFLOAT fYaw)       { m_fPlayerYaw   = fYaw; }
        void        SetPlayerRoll(LTFLOAT fRoll)     { m_fPlayerRoll  = fRoll; }

		// Called when the engine wants to tell the game a disconnection code (a.k.a. hack)
		virtual void SetDisconnectCode(uint32 nCode, const char *pMsg, uint32 nSubCode = 0);
		// Internal game-side support for the disconnection code
		void		ClearDisconnectCode();
		uint32		GetDisconnectCode();
		uint32		GetDisconnectSubCode();
		const char *GetDisconnectMsg();

		const char *GetServerAddress() const {return m_szServerAddress;}
		const char *GetServerName() const {return m_szServerName;}
		const LTFLOAT *GetServerOptions() const {return m_fServerOptions;}

	protected :

        uint32      OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid);
		void		OnEngineTerm();
        void        OnEvent(uint32 dwEventID, uint32 dwParam);
        LTRESULT    OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos);
        LTRESULT    OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot);
        LTRESULT    OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void		PreLoadWorld(char *pWorldName);
		void		OnEnterWorld();
		void		OnExitWorld();
		void		PreUpdate();
		void		Update();
		void		PostUpdate();
		void		UpdatePlaying();
		void		OnCommandOn(int command);
		void		OnCommandOff(int command);
		void		OnKeyDown(int key, int rep);
		void		OnKeyUp(int key);
        void        SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage);
		void		OnObjectRemove(HLOCALOBJ hObj);
        void        OnMessage(uint8 messageID, HMESSAGEREAD hMessage);
		void		OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		void		OnPlaySound(PlaySoundInfo* pPlaySoundInfo);
        void        SetMouseInput(LTBOOL bAllowInput);
        void        ShowPlayer(LTBOOL bShow=LTTRUE);

		void		UpdateServerPlayerModel();
        void        RenderCamera(LTBOOL bDrawInterface = LTTRUE);

		// Process the networking handshake message
		void		ProcessHandshake(HMESSAGEREAD hMessage);

		void		DoTaunt(uint32 nClientID,uint8 nTaunt);

	private :

		CHeadBobMgr				m_HeadBobMgr;		// Handle head/weapon bob/cant
		CCameraOffsetMgr		m_CameraOffsetMgr;	// Adjust camera orientation
		CPlayerSummaryMgr		m_PlayerSummary;	// Player stats data
		CIntelItemMgr			m_IntelItemMgr;		// intelligence item data
		CInterfaceMgr			m_InterfaceMgr;		// Interface manager
		CGlobalClientMgr		m_GlobalMgr;		// Contains global mgrs
		CMoveMgr				m_MoveMgr;			// Always around...
		CDamageFXMgr			m_DamageFXMgr;		// handle player damage
		CScreenTintMgr			m_ScreenTintMgr;	// handle screen tinting

        LTBOOL           m_bUseWorldFog;     // Tells if we should use global fog settings or
											// let the container handling do it.
        LTBOOL           m_bFlashScreen;     // Are we tinting the screen for a screen flash
        LTFLOAT          m_fFlashTime;       // Time screen stays at tint color
        LTFLOAT          m_fFlashStart;      // When did the flash start
        LTFLOAT          m_fFlashRampUp;     // Ramp up time
        LTFLOAT          m_fFlashRampDown;   // Ramp down time
        LTVector         m_vFlashColor;      // Tint color

        LTFLOAT          m_fYawBackup;
        LTFLOAT          m_fPitchBackup;

		// Player movement variables...

        uint32          m_dwPlayerFlags;    // What is the player doing
        LTBOOL          m_bSpectatorMode;   // Are we in spectator mode
		PlayerState		m_ePlayerState;		// What is the state of the player

		// Player update stuff...

        LTBOOL          m_bLastSent3rdPerson;

		LTRotation      m_rRotation;                // Player view rotation
        LTFLOAT         m_fPitch;                   // Pitch of camera
        LTFLOAT         m_fYaw;                     // Yaw of camera
        LTFLOAT         m_fRoll;                    // Roll of camera
        LTFLOAT         m_fFireJitterPitch;         // Weapon firing jitter pitch adjust
        LTFLOAT         m_fFireJitterYaw;           // Weapon firing jitter yaw adjust

        LTFLOAT         m_fPlayerPitch;             // Pitch of player object
        LTFLOAT         m_fPlayerYaw;               // Yaw of player object
        LTFLOAT         m_fPlayerRoll;              // Roll of player object

        LTBOOL          m_bAllowPlayerMovement;     // External camera stuff
        LTBOOL          m_bLastAllowPlayerMovement;
        LTBOOL          m_bWasUsingExternalCamera;  // so we can detect when we start using it
		LTBOOL          m_bUsingExternalCamera;
		LTBOOL			m_bCamIsListener;


		// Container FX helpers...

		ContainerCode	m_eCurContainerCode;	// Code of container currently in
        LTFLOAT         m_fContainerStartTime;  // Time we entered current container
        LTFLOAT         m_fFovXFXDir;           // Variable use in UpdateUnderWaterFX()
		uint8			m_nSoundFilterId;		// SoundFilterId for our current container
		uint8			m_nGlobalSoundFilterId;	// Global (i.e., whole level) sound filter

		// Camera zoom related variables...

		int				m_nZoomView;		// Are we in zoom mode (m_nZoomView > 0)
        LTBOOL          m_bZooming;         // Are we zooming
        LTBOOL          m_bZoomingIn;       // Are we zooming in
        LTFLOAT         m_fSaveLODScale;    // LOD Scale value before zooming


		// Camera ducking variables...

        LTBOOL   m_bStartedDuckingDown;      // Have we started ducking down
        LTBOOL   m_bStartedDuckingUp;        // Have we started back up
        LTFLOAT  m_fCamDuck;                 // How far to move camera
        LTFLOAT  m_fDuckDownV;               // Ducking down velocity
        LTFLOAT  m_fDuckUpV;                 // Ducking up velocity
        LTFLOAT  m_fMaxDuckDistance;         // Max distance we can duck
        LTFLOAT  m_fStartDuckTime;           // When duck up/down started


		GameDifficulty	m_eDifficulty;		// Difficulty of this game
		LTBOOL			m_bFadeBodies;
		GameType		m_eGameType;
		LevelEnd		m_eLevelEnd;
		int				m_nEndString;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		//these refer to the server we are currently connected to
		char		m_szServerAddress[32];
		int			m_nServerPort;
		char		m_szServerName[MAX_SESSION_NAME];
		LTFLOAT		m_fServerOptions[MAX_GAME_OPTIONS];

        LTBOOL      m_bIsCustomLevel;           // Is the current level a custom level
 		LTFLOAT     m_fFrameTime;               // Current frame delta

        LTRESULT    m_resSoundInit;             // Was sound initialized ok?

        LTBOOL      m_bStartedLevel;
        uint16      m_nPlayerInfoChangeFlags;
        LTFLOAT     m_fPlayerInfoLastSendTime;

        LTBOOL      m_bCameraPosInited;     // Make sure the position is valid
        LTBOOL      m_bRestoringGame;       // Are we restoring a saved game

        LTBOOL      m_bMainWindowMinimized; // Is the main window minimized?

        LTBOOL      m_bStrafing;            // Are we strafing?  This used to implement mouse strafing.
        LTBOOL      m_bHoldingMouseLook;    // Is the user holding down the mouselook key?

		// Glowing models...

        LTVector    m_vCurModelGlow;        // Current glowing model light color
        LTVector    m_vMaxModelGlow;        // Max glowing model light color
        LTVector    m_vMinModelGlow;        // Min glowing model light color
        LTFLOAT     m_fModelGlowCycleTime;  // Current type throught 1/2 cycle
        LTBOOL      m_bModelGlowCycleUp;    // Cycle color up or down?

		// Panning sky...

        LTBOOL      m_bPanSky;              // Should we pan the sky
        LTFLOAT     m_fPanSkyOffsetX;       // How much do we pan in X/frame
        LTFLOAT     m_fPanSkyOffsetZ;       // How much do we pan in Z/frame
        LTFLOAT     m_fPanSkyScaleX;        // How much do we scale the texture in X
        LTFLOAT     m_fPanSkyScaleZ;        // How much do we scale the texutre in Z
        LTFLOAT     m_fCurSkyXOffset;       // What is the current x offset
        LTFLOAT     m_fCurSkyZOffset;       // What is the current z offset

		CFlashLightPlayer	m_FlashLight;	// flash light for the player
		CWeaponModel	m_weaponModel;			// Current weapon model
        LTBOOL          m_bTweakingWeapon;      // Helper, move weapon around
        LTBOOL          m_bTweakingWeaponMuzzle;// Helper, move weapon muzzle around

		CMusic			m_Music;					// Music helper variable
        LTVector        m_vShakeAmount;         // Amount to shake screen
        LTVector        m_vDefaultLightScale;       // Level default light scale
		char			m_strCurrentWorldName[256];	// Current world that's running
        LTBOOL          m_bGamePaused;              // Is the game paused?
        LTBOOL          m_bMainWindowFocus;         // Focus


		// Interface stuff...
		CCheatMgr		m_cheatMgr;				// Same as g_pCheatMgr
		CLightScaleMgr	m_LightScaleMgr;		// Class to handle light scale changes

		CAttachButeMgr	m_AttachButeMgr;

        LTVector        m_vCurContainerLightScale;  // light scale values of current container

        LTBOOL          m_bRestoreOrientation;
		HLOCALOBJ		m_h3rdPersonCrosshair;
		HLOCALOBJ		m_hBoundingBox;

        LTBOOL          m_bNightVision;         // does this player currently use NightVision
        LTVector        m_vNVScreenTint;        // screen tint for night vision
        LTVector        m_vIRLightScale;        // default light scale for infrared powerup

        LTBOOL          m_bQuickSave;

        LTFLOAT         m_fEarliestRespawnTime;
        uint8           m_nCurrentMission;
        uint8           m_nCurrentLevel;

		int				m_nMPNameId;
		int				m_nMPBriefingId;


		// Camera variables...

		HLOCALOBJ		m_hCamera;			// The camera
		HLOCALOBJ		m_hInterfaceCamera;	// The camera used in the interface
		CPlayerCamera	m_PlayerCamera;		// Handle 3rd person view

		LTBOOL			m_bCameraAttachedToHead;

        LTBOOL          m_bFirstUpdate;     // Is this the first update
        LTBOOL          m_bInWorld;         // Are we in a world

        LTBOOL          m_bPlayerPosSet;    // Has the server sent us the player pos?

		// Container FX helpers...

        HLTSOUND        m_hContainerSound;  // Container sound...


		// Reverb parameters...

        LTBOOL          m_bUseReverb;
		float			m_fReverbLevel;
		float			m_fNextSoundReverbTime;
        LTVector        m_vLastReverbPos;


		// Special FX management...

		CSFXMgr		m_sfxMgr;

		// Connection handling
		LTBOOL			m_bForceDisconnect;	// Set this flag to disconnect on the next update

		// Debugging variables...

		HSURFACE	m_hDebugInfo;			// The degug info surface
        LTRect      m_rcDebugInfo;          // Debug info rect. (0,0,width,height)

        LTBOOL      m_bShowPlayerPos;       // Display player's position.
        LTBOOL      m_bShowCamPosRot;       // Display camera's position/rotation.
        LTBOOL      m_bAdjustLightScale;    // Adjusting the global light scale
        LTBOOL      m_bAdjustLightAdd;      // Adjusting the camera light add
        LTBOOL      m_bAdjustFOV;           // Adjusting the FOV
        LTBOOL      m_bAdjustWeaponBreach;  // Adjust the hand-held weapon breach
        LTBOOL      m_bAdjust1stPersonCamera; // Adjust the 1st person camera offset

		CObjEditMgr	m_editMgr;				// Mgr for editing objects

		// Disconnection code/msg storage
		uint32		m_nDisconnectCode;
		uint32		m_nDisconnectSubCode;
		char *		m_pDisconnectMsg;

		// Private helper functions...

		void	FirstUpdate();
		void	ChangeWeapon(HMESSAGEREAD hMessage);
		void	CreateDebugSurface(char* strMessage);
		void	UpdateContainerFX();
		void	MirrorSConVar(char *pSVarName, char *pCVarName);
		void	ResetGlobalFog();
        void    UpdateUnderWaterFX(LTBOOL bUpdate=LTTRUE);
        void    UpdateBreathingFX(LTBOOL bUpdate=LTTRUE);
		void	UpdateWeaponModel();
		void	StartWeaponRecoil();
		void	DecayWeaponRecoil();
		void	UpdateHeadBob();
		void	UpdateHeadCant();
		void	UpdateDuck();
		void	UpdatePlayer();
		void	AdjustWeaponBreach();
		void	Adjust1stPersonCamera();
		void	UpdateWeaponPosition();
		void	UpdateWeaponMuzzlePosition();
        void    Update3rdPersonCrossHair(LTFLOAT fDistance);
		void	UpdateSoundReverb();

		SOUNDFILTER* GetDynamicSoundFilter();

		void	AdjustLightScale();
		void	AdjustLightAdd();
		void	AdjustFOV();
		void	AdjustMenuPolygrid();
		void	AdjustHeadBob();
		void	UpdateDebugInfo();
		void	HandlePlayerStateChange(HMESSAGEREAD hMessage);
		void	HandlePlayerDamage(HMESSAGEREAD hMessage);
		void	HandleExitLevel(HMESSAGEREAD hMessage);
		void	HandleServerError(HMESSAGEREAD hMessage);
		void	HandleMultiplayerGameData(HMESSAGEREAD hMessage);
		void	HandleServerOptions(HMESSAGEREAD hMessage);

		void	HandleRespawn();

		void	InitSinglePlayer();

        LTBOOL  LoadCurrentLevel();
		void	DoStartGame();
        LTBOOL  DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile=LTNULL,
                            char* pRestoreWorldFile=LTNULL, uint8 nFlags=LOAD_NEW_GAME,
                            char *pRecordFile=LTNULL, char *pPlaydemoFile=NULL);

		void	AutoSave(HMESSAGEREAD hMessage);

		void	StartLevel();

		// Load Save functionality...

		void	BuildClientSaveMsg(HMESSAGEWRITE hMessage);
		void	UnpackClientSaveMsg(HMESSAGEREAD hRead);


		// Camera helper functions...

		void	UpdateCamera();
		void	UpdateCameraZoom();
		void	UpdateCameraShake();
		void	UpdateCameraSway();
		void	UpdateScreenFlash();
		void	ClearScreenTint();
		void	InitPlayerCamera();
        LTBOOL  UpdatePlayerCamera();
		void	UpdateCameraPosition();
		void	CalculateCameraRotation();
        LTBOOL  UpdateCameraRotation();
		void	UpdatePlayerInfo();
        LTBOOL  UpdateAlternativeCamera();
        void    TurnOffAlternativeCamera(uint8 nCamType);
        void    TurnOnAlternativeCamera(uint8 nCamType);
        void    SetExternalCamera(LTBOOL bExternal=LTTRUE);

		void	BeginZoom();
        void    HandleZoomChange(uint8 nWeaponId, LTBOOL bReset=LTFALSE);
		void	EndZoom();

		void	Update3rdPersonInfo();
        void    SetCameraFOV(LTFLOAT fFovX, LTFLOAT fFovY);
		void	HideShowAttachments(HOBJECT hObj);

		void	GetPlayerHeadPosRot(LTVector & vPos, LTRotation & rRot);
		void	AttachCameraToHead(LTBOOL bAttach=LTTRUE);

		void	CreateBoundingBox();
		void	UpdateBoundingBox();

        LTBOOL  UpdateCheats();
};

inline int CGameClientShell::GetScreenWidth()
{
    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	return (int)dwWidth;
}

inline int CGameClientShell::GetScreenHeight()
{
    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	return (int)dwHeight;
}


#endif  // __GAME_CLIENT_SHELL_H__