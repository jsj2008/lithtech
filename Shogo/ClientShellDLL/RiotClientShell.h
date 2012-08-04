// ----------------------------------------------------------------------- //
//
// MODULE  : RiotClientShell.h
//
// PURPOSE : Riot's Client Shell - Definition
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOTCLIENTSHELL_H__
#define __RIOTCLIENTSHELL_H__

#include "clientheaders.h"
#include "RiotMenu.h"
#include "PlayerStats.h"
#include "PlayerCamera.h"
#include "WeaponDefs.h"
#include "ClientServerShared.h"
#include "MessageMgr.h"
#include "SFXMgr.h"
#include "Music.h"
#include "ContainerCodes.h"
#include "PlayerModeTypes.h"
#include "ClientInfoMgr.h"
#include "MissionObjectives.h"
#include "PlayerInventory.h"
#include "WeaponModel.h"
#include "PolygridFX.h"
#include "Credits.h"
#include "MessageBox.h"
#include "TextHelper.h"
#include "InfoDisplay.h"
#include "LightScaleMgr.h"

// These 
#define CS_MFLG_FORWARD			(1<<0)
#define CS_MFLG_REVERSE			(1<<1)
#define CS_MFLG_RIGHT			(1<<2)
#define CS_MFLG_LEFT			(1<<3)
#define CS_MFLG_JUMP			(1<<4)
#define CS_MFLG_DOUBLEJUMP		(1<<5)
#define CS_MFLG_DUCK			(1<<6)
#define CS_MFLG_STRAFE			(1<<7)
#define CS_MFLG_STRAFE_LEFT		(1<<8)
#define CS_MFLG_STRAFE_RIGHT	(1<<9)
#define CS_MFLG_RUN				(1<<10)
#define CS_MFLG_FIRING			(1<<11)
#define CS_MFLG_LOOKUP			(1<<12)
#define CS_MFLG_LOOKDOWN		(1<<13)

// game states
#define GS_UNDEFINED			0
#define GS_PLAYING				1
#define GS_MENU					2
#define GS_MOVIES				3
#define GS_BUMPER				4
#define GS_LOADINGLEVEL			5
#define GS_CREDITS				6
#define GS_MPLOADINGLEVEL		7
#define GS_INTRO				8
#define GS_PAUSED				9
#define GS_DEMO_MULTIPLAYER		10
#define GS_DEMO_INFO			11

// world menu items (temp)
#define ID_BASEWORLD			256
#define ID_BASEWORLDMULTI		512

typedef void (*YESNOPROC)(LTBOOL, uint32);

class CSpecialFX;
class CPopupMenu;
class CMoveMgr;

class CRiotClientShell : public IClientShellStub
{
	public:

		CRiotClientShell();
		~CRiotClientShell();

		void  ProcessCheat(CheatCode nCode);
		void  ShakeScreen(LTVector vAmount);

		void  HandleRecord(int argc, char **argv);
		void  HandlePlaydemo(int argc, char **argv);

		LTBOOL LoadWorld(char* pWorldFile, char* pCurWorldSaveFile=LTNULL,
					    char* pRestoreWorldFile=LTNULL, uint8 nFlags=LOAD_NEW_GAME);
		LTBOOL LoadGame(char* pWorld, char* pObjectsFile);
		LTBOOL SaveGame(char* pObjectsFile);

		LTBOOL QuickSave();
		LTBOOL QuickLoad();

		LTBOOL IsAnime()	const { return m_bAnime; }

		LTBOOL GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen);
		LTBOOL StartGame(GameDifficulty eDifficulty);

		void  SetDifficulty(GameDifficulty e) { m_eDifficulty = e; }

		CMoveMgr* GetMoveMgr() {return m_MoveMgr;}
		ContainerCode	GetCurContainerCode() const { return m_eCurContainerCode; }	
		CPlayerStats* GetPlayerStats()		  { return &m_stats; }
		CInfoDisplay* GetInfoDisplay()		  { return &m_infoDisplay; }
		CRiotSettings* GetSettings()		  { return m_menu.GetSettings(); }
		LTBOOL		IsVehicleMode()			  { CRiotSettings* pSettings = m_menu.GetSettings(); if (pSettings) return pSettings->VehicleMode(); return LTFALSE; }
		CRiotMenu*	GetMenu()				  { return &m_menu; }
		HLOCALOBJ	GetCamera()			const { return m_hCamera; }
		LTBOOL		IsOnFoot()			const { return (m_nPlayerMode == PM_MODE_FOOT); }
		uint8		GetPlayerMode()		const { return m_nPlayerMode; }
		LTBOOL		IsUnderwater()		const { return m_bUnderwater; }
		uint32		GetPlayerFlags()	const { return m_dwPlayerFlags; }
		CWeaponModel* GetWeaponModel()		  { return &m_weaponModel; }
		LTBOOL		HaveSilencer()		const { return m_bHaveSilencer; }
		LTBOOL		IsInWorld( )		const { return m_bInWorld; }
		LTBOOL		IsFirstUpdate( )	const { return m_bFirstUpdate; }
		LTBOOL		IsDialogVisible()	const { return !!m_pIngameDialog; }
		char*		GetCurrentWorldName()	  { return m_strCurrentWorldName; }
		void		SetGameState(int nState)  { m_nGameState = nState; }
		CClientInfoMgr* GetClientInfoMgr()	  { return &m_ClientInfo; }
		CPlayerInventory* GetInventory()	  { return &m_inventory; }

		LTBOOL		SoundInited()			  { return ( m_resSoundInit == LT_OK ) ? LTTRUE : LTFALSE; }
		void		MainWindowMinimized()	  { m_bMainWindowMinimized = LTTRUE; }
		LTBOOL		IsMainWindowMinimized()	  { return m_bMainWindowMinimized; }
		void		MainWindowRestored()	  { m_bMainWindowMinimized = LTFALSE; }
		LTBOOL		IsSpectatorMode()			{return m_bSpectatorMode;}
		LTBOOL		IsChaseView()				{return m_playerCamera.IsChaseView(); }

		LTBOOL		AdvancedDisableMusic()			{ return m_bAdvancedDisableMusic; }
		LTBOOL		AdvancedDisableSound()			{ return m_bAdvancedDisableSound; }
		LTBOOL		AdvancedDisableMovies()			{ return m_bAdvancedDisableMovies; }
		LTBOOL		AdvancedEnableOptSurf()			{ return m_bAdvancedEnableOptSurf; }
		LTBOOL		AdvancedDisableLightMap()		{ return m_bAdvancedDisableLightMap; }
		LTBOOL		AdvancedEnableTripBuf()			{ return m_bAdvancedEnableTripBuf; }
		LTBOOL		AdvancedDisableDx6Cmds()		{ return m_bAdvancedDisableDx6Cmds; }
		LTBOOL		AdvancedEnableTJuncs()			{ return m_bAdvancedEnableTJuncs; }
		LTBOOL		AdvancedDisableFog()			{ return m_bAdvancedDisableFog; }
		LTBOOL		AdvancedDisableLines()			{ return m_bAdvancedDisableLines; }
		LTBOOL		AdvancedDisableModelFB()		{ return m_bAdvancedDisableModelFB; }
		LTBOOL		AdvancedEnablePixelDoubling()	{ return m_bAdvancedEnablePixelDoubling; }
		LTBOOL		AdvancedEnableMultiTexturing()	{ return m_bAdvancedEnableMultiTexturing; }
		LTBOOL		AdvancedDisableJoystick()		{ return m_bAdvancedDisableJoystick; }
		
		LTBOOL		IsPlayerMovementAllowed() {return m_bAllowPlayerMovement;}
		LTBOOL		IsZoomed()			const { return m_bZoomView; }

		void		SetVelMagnitude(LTFLOAT fVelMagnitude) {m_fVelMagnitude = fVelMagnitude;}

		void		DoUpdateLoop (int nTimes = 1)	{ for (int i = 0; i < nTimes; i++) {PreUpdate();Update();PostUpdate();} }
		void		ResetMenuRestoreCamera (int nLeft, int nTop, int nRight, int nBottom) { if (m_rcMenuRestoreCamera.right != 0 && m_rcMenuRestoreCamera.bottom != 0)
																							{
																								m_rcMenuRestoreCamera.left = nLeft;
																								m_rcMenuRestoreCamera.top = nTop;
																								m_rcMenuRestoreCamera.right = nRight;
																								m_rcMenuRestoreCamera.bottom = nBottom;
																							} }

		void		ClearScreenAlways (LTBOOL bYes = LTTRUE)	{ m_bClearScreenAlways = bYes; }
		void		AddToClearScreenCount()					{ m_nClearScreenCount = 3; }
		void		ZeroClearScreenCount()					{ m_nClearScreenCount = 0; }
		
		CMusic*		GetMusic()		{ return &m_Music; }
		void		InitSound();

		void		ToggleDebugCheat(CheatCode eCheat);
		void		ShowPlayerPos(LTBOOL bShow=LTTRUE)	{ m_bDebugInfoChanged = LTTRUE; m_bShowPlayerPos = bShow; }
		void		SetSpectatorMode(LTBOOL bOn=LTTRUE);

		void		ClearAllScreenBuffers();
		LTBOOL		SetMenuMode (LTBOOL bMenuUp = LTTRUE, LTBOOL bLoadingLevel = LTFALSE);
		LTBOOL		SetMenuMusic(LTBOOL bMusicOn);

		LTBOOL		DoMessageBox (int nStringID, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCrop = LTTRUE);
		LTBOOL		DoYesNoMessageBox (int nStringID, YESNOPROC pYesNoProc, uint32 nUserData, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCrop = LTTRUE);

		bool		IsJoystickEnabled();
		bool		EnableJoystick();

		CSFXMgr*	GetSFXMgr() { return &m_sfxMgr; }

		void		TintScreen(LTVector vTintColor, LTVector vPos, LTFLOAT fTintRange,
							   LTFLOAT fTime, LTFLOAT fRampUp, LTFLOAT fRampDown,
							   LTBOOL bForce=LTFALSE);

		LTBOOL		m_bRecordingDemo;	// Are we recording a demo

		void		UpdateModelGlow();
		LTVector&	GetModelGlow() {return m_vCurModelGlow;}

		void		PrintError(char*);

		LTBOOL		IsMultiplayerGame();
		LTBOOL		IsPlayerInWorld();

		PlayerState	GetPlayerState()	const { return m_ePlayerState; }
		LTBOOL		IsPlayerDead()		const { return (m_ePlayerState == PS_DEAD || m_ePlayerState == PS_DYING); }

		void		GetCameraRotation(LTRotation *pRot);

		void		ChangeWeapon(uint8 nWeaponId, LTBOOL bZoom, uint32 dwAmmo);
		void		UpdatePlayerStats(uint8 nThing, uint8 nType, LTFLOAT fAmount);
		
		void		DemoSerialize(ILTStream *pStream, LTBOOL bLoad);

		
		LTBOOL		m_bSwitchingModes;

	protected :

		void		CSPrint (char* msg, ...);
		void		ShowSplash();
		uint32		OnEngineInitialized(struct RMode *pMode, LTGUID *pAppGuid);
		void		OnEngineTerm();
		void		OnEvent(uint32 dwEventID, uint32 dwParam);
		LTRESULT		OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos);
		LTRESULT		OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot);
		LTRESULT		OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, LTFLOAT forceMag);
		void		PreLoadWorld(char *pWorldName);
		void		OnEnterWorld();
		void		OnExitWorld();
		void		PreUpdate();
		void		Update();
		void		PostUpdate();
		void		OnCommandOn(int command);
		void		OnCommandOff(int command);
		void		OnKeyDown(int key, int rep);
		void		OnKeyUp(int key);
		void		SpecialEffectNotify(HLOCALOBJ hObj, ILTMessage_Read* hMessage);
		void		OnObjectRemove(HLOCALOBJ hObj);
		void		OnMessage(uint8 messageID, ILTMessage_Read* hMessage);
		void		OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		
		CPopupMenu* CreateIngameDialog (ILTMessage_Read* hMessage);
		void		HandleObjectives (ILTMessage_Read* hMessage);
		void		PlayIntroMovies (ILTClient* pClientDE);
		void		PauseGame (LTBOOL bPause, LTBOOL bPauseSound=LTFALSE);
		void		SetMouseInput(LTBOOL bAllowInput);

		void		ShowPlayer(LTBOOL bShow=LTTRUE);

		void		UpdateServerPlayerModel();
		void		RenderCamera(LTBOOL bDrawInterface = LTTRUE);
		void		DrawInterface();
		void		DrawTransmission();
		void		DoRenderLoop(LTBOOL bDrawInterface = LTTRUE);

		void		SetLoadGameMenu();

		void		UpdateSoundReverb( );

	private :

		CMoveMgr		*m_MoveMgr;			// Always around...

		LTBOOL			m_bUseWorldFog;		// Tells if we should use global fog settings or
											// let the container handling do it.
		LTBOOL			m_bTintScreen;		// Are we tinting the screen
		LTFLOAT			m_fTintTime;		// Time screen stays at tint color
		LTFLOAT			m_fTintStart;		// When did the tinting start
		LTFLOAT			m_fTintRampUp;		// Ramp up time
		LTFLOAT			m_fTintRampDown;	// Ramp down time
		LTVector			m_vTintColor;		// Tint color

		LTFLOAT			m_fYawBackup;
		LTFLOAT			m_fPitchBackup;

		// Player movement variables...

		uint32			m_dwPlayerFlags;	// What is the player doing
		uint8			m_nPlayerMode;		// What mode is the player in
		LTBOOL			m_bSpectatorMode;	// Are we in spectator mode
		LTBOOL			m_bMoving;			// Is the player moving
		LTBOOL			m_bMovingSide;		// Is the player moving sideways
		LTBOOL			m_bOnGround;		// Is the player on the ground
		PlayerState		m_ePlayerState;		// What is the state of the player

		CMusic::EMusicLevel	m_eMusicLevel;		// Level of IMA

		// Player update stuff...

		LTFLOAT		m_fLastSentYaw;
		LTFLOAT		m_fLastSentCamCant;
		LTVector		m_vLastSentFlashPos;
		LTVector		m_vLastSentModelPos;
		uint8		m_nLastSentCode;
		LTBOOL		m_bLastSent3rdPerson;

		LTRotation		m_rRotation;		// Player view rotation
		LTVector			m_vCameraOffset;	// Offsets to the camera position
		LTFLOAT			m_fPitch;			// Pitch of camera
		LTFLOAT			m_fYaw;				// Yaw of camera
		LTBOOL			m_bCenterView;		// Center the view?
		LTFLOAT			m_fFireJitterPitch;	// Weapon firing jitter pitch adjust

		LTBOOL			m_bAllowPlayerMovement;		// External camera stuff
		LTBOOL			m_bLastAllowPlayerMovement;
		LTBOOL			m_bWasUsingExternalCamera;	// so we can detect when we start using it
		LTBOOL			m_bUsingExternalCamera;

		LTBOOL			m_bMovieCameraRect;
		int				m_nOldCameraLeft;
		int				m_nOldCameraTop;
		int				m_nOldCameraRight;
		int				m_nOldCameraBottom;

		// Container FX helpers...

		ContainerCode	m_eCurContainerCode;	// Code of container currently in
		LTFLOAT			m_fContainerStartTime;	// Time we entered current container
		LTBOOL			m_bUnderwater;			// Are we underwater?
		LTFLOAT			m_fFovXFXDir;			// Variable use in UpdateUnderWaterFX()
		LTFLOAT			m_fLastTime;			// Variable use in UpdateUnderWaterFX()

		// Camera bobbing variables...
		// Bobbin' and Swayin' - Blood 2 style ;)

		LTFLOAT			m_fBobHeight;
		LTFLOAT			m_fBobWidth;
		LTFLOAT			m_fBobAmp;
		LTFLOAT			m_fBobPhase;
		LTFLOAT			m_fSwayPhase;
		LTFLOAT			m_fVelMagnitude;	// Player's velocity (magnitude)
		
		// Camera canting variables...

		LTFLOAT			m_fCantIncrement;	// In radians
		LTFLOAT			m_fCantMaxDist;		// In radians
		LTFLOAT			m_fCamCant;			// Camera cant amount


		// Camera zoom related variables...

		LTFLOAT			m_fCurrentFovX;		// The current (non-zoomed) fovX
		LTBOOL			m_bZoomView;		// Are we in zoom mode
		LTBOOL			m_bOldZoomView;		// Were we zoom modelast frame
		LTBOOL			m_bZooming;			// Are we zooming in/out
		LTFLOAT			m_fSaveLODScale;	// LOD Scale value before zooming


		// Camera ducking variables...

		LTBOOL	m_bStartedDuckingDown;		// Have we started ducking down
		LTBOOL	m_bStartedDuckingUp;		// Have we started back up
		LTFLOAT	m_fCamDuck;					// How far to move camera
		LTFLOAT	m_fDuckDownV;				// Ducking down velocity
		LTFLOAT	m_fDuckUpV;					// Ducking up velocity
		LTFLOAT	m_fMaxDuckDistance;			// Max distance we can duck
		LTFLOAT	m_fStartDuckTime;			// When duck up/down started


		GameDifficulty	m_eDifficulty;		// Difficulty of this game

		CPlayerStats		m_stats;			// duh - player statistics (health, ammo, armor, etc.)
		CMissionObjectives	m_objectives;		// mission objectives class
		CPlayerInventory	m_inventory;		// inventory class


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	
		LTBOOL		m_bGameOver;
		LTBOOL		m_bAnime;
		LTBOOL		m_bStartedLevel;
		uint16		m_nPlayerInfoChangeFlags;
		LTFLOAT		m_fPlayerInfoLastSendTime;

		LTBOOL		m_bCameraPosInited;		// Make sure the position is valid	
		LTBOOL		m_bRestoringGame;		// Are we restoring a saved game

		LTRESULT		m_resSoundInit;			// Was sound initialized ok?
		LTBOOL		m_bGameMusicPaused;		// Is the game muisc paused?

		LTBOOL		m_bMainWindowMinimized;	// Is the main window minimized?

		LTBOOL		m_bStrafing;			// Are we strafing?  This used to implement mouse strafing.
		LTBOOL		m_bHoldingMouseLook;	// Is the user holding down the mouselook key?

		LTRect		m_rcMenuRestoreCamera;	// Camera rect to restore after leaving menus
		LTBOOL		m_bMenuRestoreFullScreen;	// was the camera rect full-screen before going to the menus?

		LTBOOL		m_bMusicOriginallyEnabled;	// was music originally enabled?
		LTBOOL		m_bSoundOriginallyEnabled;	// was sound originally enabled?
		LTBOOL		m_bLightmappingOriginallyEnabled;	// was lightmapping originally enabled?
		LTBOOL		m_bModelFBOriginallyEnabled;		// were model fullbrights originally enabled?

		// Advanced console options...

		LTBOOL		m_bAdvancedDisableMusic;		// Disable music
		LTBOOL		m_bAdvancedDisableSound;		// Disable sound
		LTBOOL		m_bAdvancedDisableMovies;		// Disable movies
		LTBOOL		m_bAdvancedEnableOptSurf;		// Enable optimizing surfaces
		LTBOOL		m_bAdvancedDisableLightMap;		// Disable light mapping
		LTBOOL		m_bAdvancedEnableTripBuf;		// Enable triple buffering
		LTBOOL		m_bAdvancedDisableDx6Cmds;		// Disable DX6 commands
		LTBOOL		m_bAdvancedEnableTJuncs;		// Enable T-Junction sealing (gaps between polies)
		LTBOOL		m_bAdvancedDisableFog;			// Disable fog
		LTBOOL		m_bAdvancedDisableLines;		// Disable line systems
		LTBOOL		m_bAdvancedDisableModelFB;		// Disable model fullbrights
		LTBOOL		m_bAdvancedEnablePixelDoubling;	// Enable pixel doubling
		LTBOOL		m_bAdvancedEnableMultiTexturing;// Enable single-pass multi-texturing
		LTBOOL		m_bAdvancedDisableJoystick;		// Disable joystick
		
		// Glowing models...

		LTVector		m_vCurModelGlow;		// Current glowing model light color 
		LTVector		m_vMaxModelGlow;		// Max glowing model light color
		LTVector		m_vMinModelGlow;		// Min glowing model light color
		LTFLOAT		m_fModelGlowCycleTime;	// Current type throught 1/2 cycle
		LTBOOL		m_bModelGlowCycleUp;	// Cycle color up or down?

		// Panning sky...

		LTBOOL		m_bPanSky;				// Should we pan the sky
		LTFLOAT		m_fPanSkyOffsetX;		// How much do we pan in X/frame
		LTFLOAT		m_fPanSkyOffsetZ;		// How much do we pan in Z/frame
		LTFLOAT		m_fPanSkyScaleX;		// How much do we scale the texture in X
		LTFLOAT		m_fPanSkyScaleZ;		// How much do we scale the texutre in Z
		LTFLOAT		m_fCurSkyXOffset;		// What is the current x offset
		LTFLOAT		m_fCurSkyZOffset;		// What is the current z offset

		CWeaponModel	m_weaponModel;			// Current weapon model
		WeaponState		m_eWeaponState;			// State of the weapon
		LTBOOL			m_bTweakingWeapon;		// Helper, move weapon around
		LTBOOL			m_bTweakingWeaponMuzzle;// Helper, move weapon muzzle around

		LTVector			m_vShakeAmount;			// Amount to shake screen
		uint32			m_nGameState;			// Current game state

		CMusic			m_Music;				// Music helper variable

		LTVector			m_vDefaultLightScale;	// Level default light scale

		char			m_strCurrentWorldName[256];		// current world that's running
		char			m_strMoviesDir[256];			// location of movies on CDROM drive

		LTBOOL			m_bGamePaused;			// Is the game paused?

		// Interface stuff...

		CMessageMgr			m_messageMgr;		// message display/sending mgr
		CCheatMgr			m_cheatMgr;			// cheat message mgr
		CRiotMenu			m_menu;				// pretty self-explanatory isn't it?
		CClientInfoMgr		m_ClientInfo;		// info on all clients connected to server
		CCredits			m_credits;			// class to display credits
		CInfoDisplay		m_infoDisplay;		// temporary information display class
		CLightScaleMgr		m_LightScaleMgr;	// class to handle light scale changes

		LTVector				m_vLightScaleInfrared;		// default light scale for infrared powerup
		LTVector				m_vLightScaleNightVision;	// default light scale for night vision powerup
		LTVector				m_vLightScaleObjectives;	// default light scale for mission objectives
		LTVector				m_vCurContainerLightScale;	// light scale values of current container

		CPolyGridFX*		m_pMenuPolygrid;		// Polygrid menu background
		LTFLOAT				m_fMenuSaveFOVx;		// Fov before entering menu		
		LTFLOAT				m_fMenuSaveFOVy;		// Fov before entering menu

		uint32			m_nClearScreenCount;		// how many frames to clear the screen
		LTBOOL			m_bClearScreenAlways;		// should we always clear the screen?
		LTBOOL			m_bDrawInterface;			// should we draw the interface (health, ammo, armor, etc.)
		LTBOOL			m_bOldDrawInterface;		// Saved value of above
		LTBOOL			m_bDrawHud;					// should we draw the interface hud
		LTBOOL			m_bDrawMissionLog;			// should we draw the mission log?
		LTBOOL			m_bDrawOrdinance;			// should we draw the ordinance screen?
		LTBOOL			m_bDrawFragCount;			// should we draw the frag counts?
		LTBOOL			m_bStatsSizedOff;			// have the stats been sized off the screen?
		LTBOOL			m_bWaitingForMLClosure;		// are we waiting for the mission log to close?
		LTBOOL			m_bNewObjective;			// have we received a new objective?
		HSURFACE		m_hNewObjective;			// new objective notice
		LTBOOL			m_bUpdateStats;				// do we need to update the player's stats?
		LTBOOL			m_bMissionLogKeyStillDown;	// are they holding down F1?
		LTBOOL			m_bCrosshairOn;				// Is the crosshair on

		HSURFACE		m_hGameMessage;				// a game message surface, if one exists
		LTFLOAT			m_nGameMessageRemoveTime;	// time at which to remove the message
		LTRect			m_rcGameMessage;			// game message rectangle (0,0,width,height)

		CPopupMenu*		m_pIngameDialog;			// in-game dialog spawned by a CDialogTrigger
		CMessageBox*	m_pMessageBox;
		YESNOPROC		m_pYesNoProc;
		uint32			m_nYesNoUserData;

		HSURFACE		m_hBumperText;				// bumper screen text 
		char*			m_pPressAnyKeySound;		// Sound played when "press any key" text appears

		LTBOOL			m_bAllowMouseInput;			// is mouse input ok?
		LTBOOL			m_bRestoreOrientation;
		HLOCALOBJ		m_h3rdPersonCrosshair;
		HLOCALOBJ		m_hBoundingBox;

		LTBOOL			m_bHaveNightVision;			// does this player currently have the NightVision powerup?
		LTBOOL			m_bHaveInfrared;			// does this player currently have the Infrared powerup?
		LTBOOL			m_bHaveSilencer;			// does this player currently have the Silencer powerup?
		LTBOOL			m_bHaveStealth;				// does this player currently have the Stealth powerup?
		LTFLOAT			m_fNormalWeaponAlpha;		// normal weapon alpha (for resetting after stealth expires)

		HSURFACE		m_hGamePausedSurface;		// "Game Paused" message

		HLTSOUND		m_hMenuMusic;				// handle to music playing while menu is up

		LTFLOAT			m_fTransmissionTimeLeft;		// time left to display transmission
		HSURFACE		m_hTransmissionImage;			// image to display in transmission
		HSURFACE		m_hTransmissionText;			// text surface for transmission
		HLTSOUND		m_hTransmissionSound;			// sound handle of tranmission sound
		LTBOOL			m_bAnimatingTransmissionOn;		// are we animating the transmission onto the screen?
		LTBOOL			m_bAnimatingTransmissionOff;	// are we animating the transmission off of the screen?
		LTFLOAT			m_xTransmissionImage;
		LTFLOAT			m_yTransmissionImage;
		LTFLOAT			m_cxTransmissionImage;
		LTFLOAT			m_cyTransmissionImage;
		LTFLOAT			m_xTransmissionText;
		LTFLOAT			m_yTransmissionText;
		LTFLOAT			m_cxTransmissionText;
		LTFLOAT			m_cyTransmissionText;

		HSURFACE		m_hLoadingWorld;
		HSURFACE		m_hPressAnyKey;
		HSURFACE		m_hWorldName;
		uint32			m_cxPressAnyKey;
		uint32			m_cyPressAnyKey;

		LTBOOL			m_bLoadingWorld;

		HSURFACE		m_hLoading;
		uint32			m_cxLoading;
		uint32			m_cyLoading;
	
		LTBOOL			m_bQuickSave;

		LTFLOAT			m_fEarliestRespawnTime;

		// Camera variables...

		HLOCALOBJ		m_hCamera;			// The camera
		CPlayerCamera	m_playerCamera;		// Handle 3rd person view

		LTBOOL			m_bHandledStartup;	// Have we handled start up conditions
		LTBOOL			m_bFirstUpdate;		// Is this the first update
		LTBOOL			m_bInWorld;			// Are we in a world

		LTBOOL			m_bDemoPlayback;	// Are we playing back a demo

		LTBOOL			m_bPlayerPosSet;	// Has the server sent us the player pos?


		// Container FX helpers...

		HLTSOUND		m_hContainerSound;	// Container sound...
	

		// Reverb parameters
		LTBOOL			m_bUseReverb;
		LTFLOAT			m_fReverbLevel;
		LTFLOAT			m_fNextSoundReverbTime;
		LTVector			m_vLastReverbPos;

		// Special FX management...

		CSFXMgr		m_sfxMgr;



		// Debugging variables...

		LTBOOL		m_bDebugInfoChanged;	// Has the info changed?
		HSURFACE	m_hDebugInfo;			// The degug info surface
		LTRect		m_rcDebugInfo;			// Debug info rect. (0,0,width,height)
		HLOCALOBJ	m_hLeakLines;			// Leak finder lines

		LTBOOL		m_bShowPlayerPos;		// Display player's position.
		LTBOOL		m_bAdjustLightScale;	// Adjusting the global light scale


		// Private helper functions...

		void	UpdateLoadingLevel();
		void	StartNewWorld(HSTRING hstrWorld);
		void	FirstUpdate();
		void	ChangeWeapon(ILTMessage_Read* hMessage);
		void	ShowGameMessage(char* strMessage);
		void	CreateDebugSurface(char* strMessage);
		void	UpdateContainerFX();
		void	ResetGlobalFog();
		void	UpdateUnderWaterFX(LTBOOL bUpdate=LTTRUE);
		void	UpdateBreathingFX(LTBOOL bUpdate=LTTRUE);
		void	UpdateWeaponModel();
		void	UpdatePlayerFlags();
		void	UpdateHeadBob();
		void	UpdateHeadCant();
		void	UpdateDuck();
		void	UpdatePlayer();
		void	UpdateWeaponPosition();
		void	UpdateWeaponMuzzlePosition();
		void	Update3rdPersonCrossHair(LTFLOAT fDistance);

		void	UpdateMoviesState();
		void	UpdateCreditsState();
		void	UpdateIntroState();
		void	UpdateMenuState();
		void	UpdateBumperState();
		void	UpdateLoadingLevelState();
		void	UpdatePausedState();
		void	UpdateGameOver();
		void	UpdateDemoMultiplayerState();
		void	UpdateDemoInfoState();

		void	AdjustLightScale();
		void	UpdateDebugInfo();
		void	HandlePlayerStateChange(ILTMessage_Read* hMessage);
		void	HandlePlayerDamage(ILTMessage_Read* hMessage);
		void	HandleExitLevel(ILTMessage_Read* hMessage);
		void	HandleServerError(ILTMessage_Read* hMessage);
		void	HandleTransmission(ILTMessage_Read* hMessage);

		void	DoPickupItemScreenTint(PickupItemType eType);
		void	DisplayGenericPickupMessage(PickupItemType eType);
		void	HandleItemPickedup(PickupItemType eType);
		void	HandleItemExpired(PickupItemType eType);
		void	HandleRespawn();
		void	CreateBumperScreen(char* pPCXName, uint32 dwBumperTextID);
		void	RemoveBumperScreen();
		void	HandleMPChangeLevel();

		LTBOOL	ProcessMenuCommand (int nID);
		void	SetInputState(LTBOOL bAllowInput);

		void	InitMultiPlayer();
		void	InitSinglePlayer();

		void	DoStartGame();
		LTBOOL   DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile=LTNULL,
					        char* pRestoreWorldFile=LTNULL, uint8 nFlags=LOAD_NEW_GAME,
							char *pRecordFile=LTNULL, char *pPlaydemoFile=NULL);

		void	AutoSave(ILTMessage_Read* hMessage);

		void	StartLevel();

		// Load Save functionality...

		void	BuildClientSaveMsg(ILTMessage_Write* hMessage);
		void	UnpackClientSaveMsg(ILTMessage_Read* hRead);


		// Camera helper functions...

		void	UpdateCamera();
		void	UpdateCameraZoom();
		void	UpdateCameraShake();
		void	UpdateScreenTint();
		void	ClearScreenTint();
		LTBOOL	UpdatePlayerCamera();
		void	UpdateCameraPosition();
		void	CalculateCameraRotation();
		LTBOOL	UpdateCameraRotation();
		void	UpdatePlayerInfo();
		LTBOOL	UpdateAlternativeCamera();
		void	TurnOffAlternativeCamera();
		void	SetExternalCamera(LTBOOL bExternal=LTTRUE);
		void	HandleZoomChange(uint8 nWeaponId);
		void	Update3rdPersonInfo();

		void	CreateMenuPolygrid();
		void	RemoveMenuPolygrid();
		void	UpdateMenuPolygrid();

		void	CreateBoundingBox();
		void	UpdateBoundingBox();
};


#endif  // __RIOTCLIENTSHELL_H__

