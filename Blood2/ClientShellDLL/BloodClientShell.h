#ifndef __BLOODCLIENTSHELL_H__
#define __BLOODCLIENTSHELL_H__

#include <stdio.h>
#include <math.h>
#include <mbstring.h>
#include "cpp_clientshell_de.h"
#include "MainMenus.h"
#include "SharedDefs.h"
#include "MessageMgr.h"
#include "NewStatusBar.h"
#include "CommLink.h"
#include "ViewWeapon.h"
#include "SFXMgr.h"
#include "PlayerCamera.h"
#include "ContainerCodes.h"
#include "LoadSave.h"
#include "Music.h"
#include "CameraFX.h"
#include "viewcreature.h"
#include "FragInfo.h"
#include "Credits.h"
#include "VoiceMgr.h"
#include "LTGUIMgr.h"
#include "MessageBoxHandler.h"
#include "TeamMgr.h"


// game version
#ifdef _DEMO
#define version_string "DEMO v1.0a (Build 109)"
#else
#ifdef _ADDON
#define version_string "NIGHTMARES v1.0.233"
#else
#define version_string "v2.1.233"
#endif
#endif


// game states
#define GS_NONE                 666		// No game state
#define GS_PLAYING				0		// Playing the game
#define GS_MENU					1		// Running the menu
#define GS_MENUANIM				2		// Menu is sliding onto the screen
#define	GS_LOADINGLEVEL			3		// Waiting for a new level to load
#define GS_SAVING				4		// Waiting for a save game acknowledgement
#define GS_MPLOADINGLEVEL		5		// Loading a multiplayer level
#define GS_CREDITS				6		// Credits screen
#define GS_MOVIES				7		// Logo movies
#define GS_SPLASH				8		// Splash screen displayed
#define GS_WAITING				9		// Waiting for a key press
#define GS_MPCHANGINGLEVELS		10		// Changing levels in a multiplayer game

#define CLIENTWEAPONSLOTS		18	// 10 standard slots, plus inventory weapons
#define CLIENTITEMSLOTS			6
#define CLIENTSPELLSLOTS		6
#define	CLIENTBINDINGSLOTS		3

#define DETAIL_LOW				0
#define DETAIL_MEDIUM			1
#define DETAIL_HIGH				2

// Message box command ID
#define MESSAGE_BOX_ID_KILL		1

// Structure to hold data read from the player.cfg file
struct ConfigStruct 
{
		ConfigStruct()
		{
			memset(this, 0, sizeof(ConfigStruct));
			nStrength = nSpeed = nMagic = nResistance = 1;
			_mbscpy((unsigned char*)szName, (const unsigned char*)"New Player");
		}
		DBYTE	nStrength;
		DBYTE	nSpeed;
		DBYTE	nMagic;
		DBYTE	nResistance;
		DBYTE	nCharacter;
		DBYTE	nSkin;
		char	szName[MAX_PLAYERNAME_LEN];
		DBYTE	nWeaponSlots[CLIENTWEAPONSLOTS];
		DBYTE	nItemSlots[CLIENTITEMSLOTS];
		DBYTE	nSpellSlots[CLIENTSPELLSLOTS];
		DBYTE	nProximityBombs;
		DBYTE	nRemoteBombs;
		DBYTE	nSmokeBombs;
		DBYTE	nTimeBombs;
		DBYTE	nBindingSlots[CLIENTBINDINGSLOTS];
}; 

//DBOOL LoadConfig(char* pfilename, ConfigStruct *pConfig);

// Structure to hold various data items sent by the client
typedef struct
{
		DBYTE			byFlags;
		DFLOAT			fPitch;
		DFLOAT			fYaw;
		DVector			GunMuzzlePos;
		DVector			lGunMuzzlePos;
		DFLOAT			MouseAxis0;
		DFLOAT			MouseAxis1;
		ContainerCode	eContainer;
		HOBJECT			hContainerAtBottom;
} ClientData;

// Abort function prototypes.
void Abort(char* sErrorMsg, DDWORD dwCode);
void Abort(int nErrorID, DDWORD dwCode);

class CMoveMgr;

// Definition of the client shell structure.
class CBloodClientShell : public CClientShellDE
{
    public:

        CBloodClientShell();
        ~CBloodClientShell();
		void			CSPrint (char* msg, ...);
		void			CSPrint (int nStringID);
		void			CSPrint2 (char* msg);

		CVoiceMgr*		GetVoiceMgr() { return &m_VoiceMgr; }
		CTeamMgr*		GetTeamMgr() { return &m_TeamMgr; }
		CSFXMgr*		GetSFXMgr() { return &m_sfxMgr; }
		HLOCALOBJ		GetCameraObj() { return m_hCamera; }
		CSavedGameInfo*	GetSavedGameInfo() { return &m_SavedGameInfo; }
		void			ShakeScreen(DVector *vShake, DFLOAT fTime);
		void			FlashScreen(DVector *vFlashColor, DFLOAT fTime, DFLOAT fRampUp);

		void			UpdateWonkyVision(DBOOL bUpdate=DTRUE);		//for thief attaching
		DBOOL			IsWonky() { return(m_bWonkyVision); }
		DBOOL			IsWonkyVision() { return(m_bWonkyVision); }
		DBOOL			IsWonkyNoMove() { return(m_bWonkyNoMove); }
		void			StartWonkyVision(DFLOAT fWonkyTime, DBOOL bNoMove);
		void			EndWonkyVision();

		DVector			GetLightScale()						{return m_vLightScale;}

		void			RemoveCreature()		{delete m_pCreature; m_pCreature = DNULL;}

		void			StartNewGame(char *pszWorld);
		DBOOL			StartNewWorld(char *pszWorld, DBYTE nGameType, DBYTE nLoadType, DBYTE nDifficulty = DIFFICULTY_EASY);
		DRESULT			InitSound();

		HSOUNDDE		PlaySound(char* sSound, DBOOL bStream=DFALSE, DBOOL bLoop=DFALSE, DBOOL bGetHandle=DFALSE);

		void			DisplayStatusBarFlagIcon(DBOOL bDisplay);

		void			AssignFrags(DDWORD dwLocalID, DDWORD dwVictim, DDWORD dwKiller);

		DBOOL			SetCharacterInfo(int nCharacter, int nSkin);

		void			ResetPlayerInventory();

		// Advanced options from launcher
		DBOOL			AdvancedDisableMusic()			{ return m_bAdvancedDisableMusic; }
		DBOOL			AdvancedDisableSound()			{ return m_bAdvancedDisableSound; }
		DBOOL			AdvancedDisableMovies()			{ return m_bAdvancedDisableMovies; }
		DBOOL			AdvancedDisableJoystick()		{ return m_bAdvancedDisableJoystick; }
		DBOOL			AdvancedEnableOptSurf()			{ return m_bAdvancedEnableOptSurf; }
		DBOOL			AdvancedDisableLightMap()		{ return m_bAdvancedDisableLightMap; }
		DBOOL			AdvancedEnableTripBuf()			{ return m_bAdvancedEnableTripBuf; }
		DBOOL			AdvancedDisableDx6Cmds()		{ return m_bAdvancedDisableDx6Cmds; }
		DBOOL			AdvancedEnableTJuncs()			{ return m_bAdvancedEnableTJuncs; }
		DBOOL			AdvancedDisableFog()			{ return m_bAdvancedDisableFog; }
		DBOOL			AdvancedDisableLines()			{ return m_bAdvancedDisableLines; }
		DBOOL			AdvancedDisableModelFB()		{ return m_bAdvancedDisableModelFB; }
		DBOOL			AdvancedEnablePixelDoubling()	{ return m_bAdvancedEnablePixelDoubling; }
		DBOOL			AdvancedEnableMultiTexturing()	{ return m_bAdvancedEnableMultiTexturing; }

		// Movie stuff
		DBOOL			PlayMovie(CClientDE* pClientDE, char* sMovie, DBOOL bCheckExistOnly = DFALSE);
		DBOOL			StartMovies(CClientDE* pClientDE);
		DBOOL			EndMovies(CClientDE* pClientDE, DBOOL bShowTitle = DFALSE);
		DBOOL			AdvanceMovies(CClientDE* pClientDE);
		void			UpdateMoviesState();

		// Menu execute functions
		DBOOL			MenuQuit();
		DBOOL			MenuNewGame(int nDifficulty, int nCharacter = CHARACTER_CALEB, int nGameType = GAMETYPE_SINGLE);
		DBOOL			MenuNewNightmaresGame(int nDifficulty);
		DBOOL			MenuHostGame();
		DBOOL			MenuJoinGame();
		DBOOL			MenuCredits();
		DBOOL			MenuHelp();
		DBOOL			MenuLoadGame(int nSlot);
		DBOOL			MenuSaveGame(int nSlot, DBYTE bySaveType = SAVETYPE_CURRENT);
		DBOOL			MenuLoadCustomLevel(char* sLevel, int nDifficulty);
		DBOOL			MenuReturnToGame();
		DBOOL			MenuSetDetail();

		// Mouse access
		void			SetMouseSensitivity(float fSpeed);
		float			GetMouseSensitivity();
		DBOOL			IsMouseInvertYAxis()					{ return m_bMouseInvertYAxis; }
		void			SetMouseInvertYAxis(DBOOL bInvert)		{ m_bMouseInvertYAxis=bInvert; }
		DBOOL			IsMouseLook()							{ return m_bMouseLook; }
		void			SetMouseLook(DBOOL bMouseLook)			{ m_bMouseLook=bMouseLook; }
		DBOOL			IsLookSpring()							{ return m_bLookSpring; }
		void			SetLookSpring(DBOOL bLookSpring)		{ m_bLookSpring=bLookSpring; }

		// Joystick routines
		DBOOL			IsUseJoystick()							{ return m_bUseJoystick; }
		void			SetUseJoystick(DBOOL bUseJoystick);

		// Keyboard stuff
		float			GetKeyboardTurnRate()					{ return m_fKeyboardTurnRate; }
		void			SetKeyboardTurnRate(float fRate);

		// Puts a message box on the screen.  It is removed when ENTER, ESC, or SPACE is pressed
		// or on the next page flip if bAsync is FALSE.
		void			DoMessageBox(char *lpszMessage, DBOOL bAsync=DTRUE);
		
		// Kills the message box on the screen
		void			KillMessageBox();

		// Game state management
		int				GetGameState()							{ return(m_nGameState); }
		DBOOL			SetGameState(int nState);

		// Determines if we are in a multiplayer game
		int				GetGameType() { return(m_nGameType); }
		DBOOL			IsMultiplayerGame();
		DBOOL			IsMultiplayerTeamBasedGame();
		DBOOL			IsMultiplayerTeams() { return(m_nGameType == NGT_TEAMS); }
		DBOOL			IsMultiplayerCtf() { return(m_nGameType == NGT_CAPTUREFLAG); }

		// Access to member variables
		DBOOL			IsInWorld()								{ return m_bInWorld; }
		DBOOL			IsDead()								{ return m_bDead; }
		DBOOL			IsTrapped()								{ return (DBOOL)m_nTrapped; }
		void			IgnoreKeyboardMessage(DBOOL bIgnore)	{ m_bIgnoreKeyboardMessage=bIgnore; }
		DBYTE			GetGlobalDetail()						{ return m_nGlobalDetail; }
		DBOOL			NightGogglesActive( )					{ return m_bNightGogglesActive; }
		DBOOL			BinocularsActive( )						{ return m_bBinocularsActive; }
		DBOOL			IsPaused()								{ return m_bPaused; }
		DBOOL			IsExternalCamera()						{ return !!m_pExternalCamera; }
		DBOOL			IsSpectatorMode()						{ return m_bSpectatorMode; }
		DBOOL			IsZoomed()								{ return m_bZoomView; }

		DBOOL			IsNetFriendlyFire() { return(m_bNetFriendlyFire); }
		DBOOL			IsNetNegTeamFrags() { return(m_bNetNegTeamFrags); }
		DBOOL			IsNetOnlyFlagScores() { return(m_bNetOnlyFlagScores); }
		DBOOL			IsNetOnlyGoalScores() { return(m_bNetOnlyGoalScores); }


		// New physics stuff
		DBYTE			GetCDataFlags() { return cdata.byFlags; }
		void			GetCameraRotation(DRotation *pRot);
		ContainerCode	GetCurContainerCode() const { return m_eCurContainerCode; }

		void			SetVelMagnitude(DFLOAT fMag) { m_fVelMagnitude = fMag; }
		DFLOAT			GetEyeLevel() { return m_fEyeLevel; }
		DBYTE			GetCharacter() { return m_Config.nCharacter; }
		void			SetPowerBarLevel(DFLOAT fPercent);

		DDWORD			GetAmmo() { return m_dwAmmo; }
		DDWORD			GetAltAmmo() { return m_dwAltAmmo; }

    protected:
    
        DRESULT			OnEngineInitialized(RMode *pMode, DGUID *pAppGuid);
        void			OnEngineTerm();
        void			OnEnterWorld();
        void			OnExitWorld();
		void			OnEvent(DDWORD dwEventID, DDWORD dwParam);
        void			UpdateCamera();
        void			FirstUpdate();
        void			Update();
        void			PostUpdate();
		void			UpdateCredits();
		DRESULT			OnObjectMove(HOBJECT hObj, DBOOL bTeleport, DVector *pPos);
		DRESULT			OnObjectRotate(HOBJECT hObj, DBOOL bTeleport, DRotation *pNewRot);
		void			PreLoadWorld(char *pWorldName);
        void			OnCommandOn(int command);
        void			OnCommandOff(int command);
        void			OnKeyDown(int key, int rep);
        void			OnKeyUp(int key);
        void			OnMessage(DBYTE messageID, HMESSAGEREAD hMessage);
		void			SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage);
		void			OnObjectRemove(HLOCALOBJ hObj);
		void			OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);

		void			SendPlayerUpdateMessage(DBOOL bSendAll);
		void			SendPlayerInitMessage(DBYTE messageID);

		// Get the mouse device name and axis names
		void			GetMouseDeviceInfo(char *lpszDeviceName, int nDeviceBufferSize,
										   char *lpszXAxis,		 int nXAxisBufferSize,
										   char *lpszYAxis,		 int nYAxisBufferSize);

		void			UpdateSoundReverb( );

	private:

		void			InitSinglePlayer();
		void			InitMultiPlayer();
		void			UpdateBob();
		void			UpdateHeadCant();
		DBOOL			UpdatePlayerCamera();
		void			SetCameraStuff(HLOCALOBJ hPlayerObj, HLOCALOBJ hCamera);
		void			UpdateGun(DRotation *rot, DVector *pos);
		void			ChangeWeapon(DBYTE slot);
        void			PauseGame (DBOOL bPause);
		void			UpdateScreenShake();
		void			UpdateScreenFlash();
		void			UpdateContainerFX();
		void			UpdateUnderWaterFX(DBOOL bUpdate=DTRUE);
		void			HandleExitWorld(HMESSAGEREAD hMessage);
		DBOOL			StartNewGame(char *pszWorldName, DBYTE nStartType, DBYTE nGameType, DBYTE nLoadType, DBYTE nDifficulty);
		void			ShowPlayer(DBOOL bShow);
		void			DrawLoadingScreen();
		DBOOL			DoMultiplayer(DBOOL bMinimize, DBOOL bHost);

		void			HandleInvAction( DBYTE nType, DBOOL bActivate );

		void			ShowInfoBox(char *lpszMessage, DBOOL bCloseButton);
		void			KillInfoBox();
		DBOOL			HandleFiring();
		CViewWeapon*	CreateWeapon(DBYTE byType, DBOOL bLeftHand);


		// Internal game state management
		DBOOL			TransitionGameState(int nOldState, int nNewState);
		DBOOL			IsGameStateValid(int nState);

		DBOOL			OnEnterPlayingState(int nOldState);
		DBOOL			OnEnterMenuState(int nOldState);
		DBOOL			OnEnterMenuAnimState(int nOldState);
		DBOOL			OnEnterLoadingLevelState(int nOldState);
		DBOOL			OnEnterSavingState(int nOldState);
		DBOOL			OnEnterMultiLoadingLevelState(int nOldState);
		DBOOL			OnEnterCreditsState(int nOldState);
		DBOOL			OnEnterMoviesState(int nOldState);
		DBOOL			OnEnterSplashState(int nOldState);
		DBOOL			OnEnterWaitingState(int nOldState);

		DBOOL			OnExitPlayingState(int nNewState);
		DBOOL			OnExitMenuState(int nNewState);
		DBOOL			OnExitMenuAnimState(int nNewState);
		DBOOL			OnExitLoadingLevelState(int nNewState);
		DBOOL			OnExitSavingState(int nNewState);
		DBOOL			OnExitMultiLoadingLevelState(int nNewState);
		DBOOL			OnExitCreditsState(int nNewState);
		DBOOL			OnExitMoviesState(int nNewState);
		DBOOL			OnExitSplashState(int nNewState);
		DBOOL			OnExitWaitingState(int nNewState);

		// Load/Save stuff

		void			SaveClientData( HMESSAGEWRITE hMsg );
		void			LoadClientData( HMESSAGEREAD hMsg );


		// New camera stuff
		void			Set3rdPersonCamera(DBOOL bExternal);
		void			UpdateCameraPosition();
		void			CalculateCameraRotation();
		void			UpdateCameraRotation();
		void			ResetGlobalFog();
		void			GetWorldTitle(char *pWorldFile);
#ifdef BRANDED
		void			CreateBrandSurface();
		void			DrawBrandString(HSURFACE hScreenSurface);
#endif

	private:

		ClientData		cdata;
		ClientData		cdataLast; 
		DRotation		m_Rotation;
		CViewWeapon*	m_pWeapon;			// 10 slots plus some for other weapons
		CViewWeapon*	m_pLWeapon;			// left hand weapons
		DBYTE			m_abyWeaponID[CLIENTWEAPONSLOTS];		// 10 slots plus some for other weapons
		DBYTE			m_abyLWeaponID[CLIENTWEAPONSLOTS];		// left hand weapons
//		HLOCALOBJ		m_hWeapObj;			// Weapon models
//		HLOCALOBJ		m_hLWeapObj;		// Weapon models
		HLOCALOBJ		m_hWeapHandObj;		// Weapon models
		HLOCALOBJ		m_hLWeapHandObj;	// Weapon models
		CMainMenus		m_Menu;				// pretty self-explanatory isn't it?
		CMessageMgr		m_MessageMgr;
//		CLevelMgr		m_LevelMgr;			// Manage the episode/level structure
		CCheatMgr		m_CheatMgr;
		CMusic			m_Music;
		CNewStatusBar	m_NewStatusBar;
		CCommLink		m_CommLink;
		CFragInfo		m_FragInfo;
		CCredits		m_Credits;
		CVoiceMgr		m_VoiceMgr;
		CTeamMgr		m_TeamMgr;

		CViewCreature*	m_pCreature;		//attach creatre (hand, thief, bone leech)

		DBOOL			m_bFirstUpdate;
		DBOOL			m_bRollCredits;

		DDWORD			m_nGameState;
		DDWORD			m_nHealth;
		DBOOL			m_bDead;
		DBOOL			m_bDeadNoHide;
		DBOOL			m_bWonkyVision;
		DFLOAT			m_fWonkyTime;
		DBOOL			m_bWonkyNoMove;

		int				m_nCurGun;
		DFLOAT			m_SpinAmount;
		DBOOL			m_bZeroXAngle;
		DBOOL			m_bShowCrosshair;
		DBOOL			m_nCrosshair;
		DBOOL			m_nLastCrosshair;
		DBOOL			m_bDemoPlayback;
		DBOOL			m_bShiftState;

		// Advanced console options...
		DBOOL		m_bAdvancedDisableMusic;		// Disable music
		DBOOL		m_bAdvancedDisableSound;		// Disable sound
		DBOOL		m_bAdvancedDisableMovies;		// Disable movies
		DBOOL		m_bAdvancedDisableJoystick;		// Disable joystick
		DBOOL		m_bAdvancedEnableOptSurf;		// Enable optimizing surfaces
		DBOOL		m_bAdvancedDisableLightMap;		// Disable light mapping
		DBOOL		m_bAdvancedEnableTripBuf;		// Enable triple buffering
		DBOOL		m_bAdvancedDisableDx6Cmds;		// Disable DX6 commands
		DBOOL		m_bAdvancedEnableTJuncs;		// Enable T-Junction sealing (gaps between polies)
		DBOOL		m_bAdvancedDisableFog;			// Disable fog
		DBOOL		m_bAdvancedDisableLines;		// Disable line systems
		DBOOL		m_bAdvancedDisableModelFB;		// Disable model fullbrights
		DBOOL		m_bAdvancedEnablePixelDoubling;	// Enable pixel doubling
		DBOOL		m_bAdvancedEnableMultiTexturing;// Enable multi-texturing

		DBOOL		m_bMusicOriginallyEnabled;			// was music originally enabled?
		DBOOL		m_bSoundOriginallyEnabled;			// was sound originally enabled?
		DBOOL		m_bLightmappingOriginallyEnabled;	// was lightmapping originally enabled?
		DBOOL		m_bModelFBOriginallyEnabled;		// were model fullbrights originally enabled?

		DFLOAT		m_fOriginalMouseSensitivity;	// Variable to store a temp sense when zoomed
		DFLOAT		m_fOriginalKeyTurnRate;			// Variable to store a temp sense when zoomed
		DBOOL		m_bCrosshairOriginallyOn;		// Was the crosshair on before we zoomed in
		DBYTE		m_nCrosshairOriginalNum;		// Which number was the crosshair on

        // Camera Stuff
		HLOCALOBJ		m_hCamera;
		HLOCALOBJ		m_hOrbObj;			// For the orb..
		HLOCALOBJ		m_hLastOrbObj;		// To test if we should use that one again
		HLOCALOBJ		m_hSeeingEyeObj;	// For the all-seeing eye
		CPlayerCamera	m_playerCamera;		// Handle 3rd person view
		DBOOL			m_b3rdPerson;
        
    	DFLOAT			m_fLastCameraFovX;
        DFLOAT			m_fLastCameraFovY;
        
		DDWORD			m_dwScreenWidth;
		DDWORD			m_dwScreenHeight;
		DBOOL			m_bHandledStartup;

		// Special FX management...

		CSFXMgr			m_sfxMgr;
		DBYTE			m_nGlobalDetail;

		// Container FX helpers...

		HSOUNDDE		m_hContainerSound;	// Container sound...
		ContainerCode	m_eCurContainerCode;// Code of container currently in
		DFLOAT			m_fContainerStartTime; // Time we entered current container

		DFLOAT			m_fFovXFXDir;		// Variable use in UpdateUnderWaterFX()
		DFLOAT			m_fLastTime;		// Variable use in UpdateUnderWaterFX()
		DVector			m_vLightScale;
		DVector			m_vCameraAdd;

		
		// Bobbin' and Swayin'
		DFLOAT			m_fPitch;
		DFLOAT			m_fYaw;
		DFLOAT			m_fAdjPitch;
		DFLOAT			m_fAdjYaw;

		DFLOAT			m_fBobAmp;
		DFLOAT			m_fBobPhase;
		DFLOAT			m_fBobHeight;
		DFLOAT			m_fBobWidth;
		DFLOAT			m_fBobCant;
		DFLOAT			m_fShakeCant;
		DFLOAT			m_fSwayPhase;
		DFLOAT			m_fSwayHeight;
		DFLOAT			m_fSwayWidth;
	
		DFLOAT			m_fCantIncrement;
		DFLOAT			m_fCantMaxDist;
		DFLOAT			m_fCamCant;

		DBOOL			m_bShakeScreen;
		DFLOAT			m_fShakeTime;
		DFLOAT			m_fShakeStart;
		DVector			m_vShakeMagnitude;

		DBOOL			m_bFlashScreen;
		DFLOAT			m_fFlashTime;
		DFLOAT			m_fFlashStart;
		DFLOAT			m_fFlashRampUp;
		DVector			m_vFlashColor;

		//wonky vision crap
		DFLOAT			m_fOffsetX;
		DFLOAT			m_fOffsetY;
		DFLOAT			m_fOffsetRot;
		DFLOAT			m_fRotDir;
		DFLOAT			m_fMaxRot;

		// Stuff received from the player
		DFLOAT			m_fEyeLevel;
		DFLOAT			m_fVelMagnitude;
//		DFLOAT			m_fVelY
		DVector			m_vMyLastPos;

		DFLOAT			m_fViewY;
		DFLOAT			m_fViewYVel;
		DFLOAT			m_fWeaponY;
		DFLOAT			m_fWeaponYVel;

		DBOOL			m_bSpectatorMode;
		DBOOL			m_bZoomView;

		DFLOAT			m_fCameraZoom;
		DFLOAT			m_fovX;
		DFLOAT			m_fovY;
		DFLOAT			m_DefFovX;
		DFLOAT			m_fovYScale;
		DFLOAT			m_fViewKick;
		DFLOAT			m_fKickTime;

		HSURFACE		m_hCrosshair;
		DDWORD			m_cxCrosshair;
		DDWORD			m_cyCrosshair;
		HSURFACE		m_hOverlayText;
		HDEFONT			m_hFont;
		DBOOL			m_bInWorld;
		DBOOL			m_bDrawStatusBar;
		DBOOL			m_bDrawFragBar;

		// Character Attibutes
		ConfigStruct	m_Config;

//		DBOOL			m_bUseExternalCamera;
		CCameraFX*		m_pExternalCamera;
		DBOOL			m_bRenderCamera;
		DBOOL			m_bStoneView;
		DBOOL			m_bAuraView;
        
		DBOOL			m_bBurn;
        DBOOL			m_bBlind;
		DBOOL			m_bFadeIn;
		DBOOL			m_bFadeOut;
		DFLOAT			m_fFadeVal;
        DFLOAT			m_fBurnTime;
        DFLOAT			m_fBlindTime;

		HSURFACE		m_hLoadingScreen;
		DFLOAT			m_fLoadingFadeTime;
		DBOOL			m_bLoadingFadeUp;
		DBOOL			m_bPlayedWaitingSound;
		DFLOAT			m_fLoadDelayTime;

		HSTRING			m_hstrTitle;
		HSTRING			m_hstrLoadScreen;
		DDWORD			m_nLoadScreenID;

		HSTRING			m_hstrObjectivesTitle;
		HSTRING			m_hstrObjectivesText;

		int				m_nLastExitType;
		StartGameRequest m_Request;
		DBOOL			m_bFirstWorld;
		char			m_szFilename[255];

		// Sky panning attributes
		DBOOL			m_bPanSky;
		DFLOAT			m_fPanSkyOffsetX;
		DFLOAT			m_fPanSkyOffsetZ;
		DFLOAT			m_fPanSkyScaleX;
		DFLOAT			m_fPanSkyScaleZ;
		DFLOAT			m_fCurSkyXOffset;
		DFLOAT			m_fCurSkyZOffset;

		// Load/Save game interface
		CSavedGameInfo	m_SavedGameInfo;

		// Mouse
		DBOOL			m_bMouseInvertYAxis;
		DBOOL			m_bMouseLook;
		DBOOL			m_bLookSpring;

		// Joystick
		DBOOL			m_bUseJoystick;

		// Keboard
		float			m_fKeyboardTurnRate;

		// Movie
		int				m_nMovieState;

		// Message box
		CMessageBoxHandler	m_messageBoxHandler;
		CLTGUIMessageBox	m_messageBox;
		CLTGUIFont			m_messageFont;
		DBOOL				m_bInMessageBox;

		// This is used to ignore a keyboard message while doing
		// the key configuration stuff.
		DBOOL			m_bIgnoreKeyboardMessage;

		HSTRING			m_hCurrentItemName;
		HSTRING			m_hCurrentItemIcon;
		HSTRING			m_hCurrentItemIconH;
		DDWORD			m_nCurrentItemCharge;
		HSTRING			m_hPrevItemIcon;
		DDWORD			m_nPrevItemCharge;
		HSTRING			m_hNextItemIcon;
		DDWORD			m_nNextItemCharge;

		int				m_nLastLoadType;
		int				m_nGameType;

		DBOOL			m_bNetFriendlyFire;
		DBOOL			m_bNetNegTeamFrags;
		DBOOL			m_bNetOnlyFlagScores;
		DBOOL			m_bNetOnlyGoalScores;

		// Inventory item stuff

		DBOOL			m_bNightGogglesActive;
		HSOUNDDE		m_hNightGogglesSound;
		HSOUNDDE		m_hTheEyeLoopingSound;

		DBOOL			m_bBinocularsActive;

		DBOOL			m_bPaused;


		CMoveMgr*		m_pMoveMgr;
		int				m_nTrapped;

#ifdef BRANDED
		HSURFACE		m_hBrandSurface;
		DFLOAT			m_fBrandCounter;
#endif

		CMusic::EMusicLevel	m_eMusicLevel;

		DDWORD			m_dwAmmo;
		DDWORD			m_dwAltAmmo;

		HSTRING			m_hCtfCapturedString1;
		HSTRING			m_hCtfCapturedString2;

#ifdef _ADDON
		HSTRING			m_hSoccerGoalString1;
		HSTRING			m_hSoccerGoalString2;
#endif // _ADDON


		// Reverb parameters
		DBOOL			m_bUseReverb;
		float			m_fReverbLevel;
		float			m_fNextSoundReverbTime;
		DVector			m_vLastReverbPos;

};


// Inlines...

inline DBOOL CBloodClientShell::IsMultiplayerTeamBasedGame()
{
	if (m_nGameType == GAMETYPE_CTF) return(DTRUE);
	if (m_nGameType == GAMETYPE_TEAMPLAY) return(DTRUE);
	if (m_nGameType == GAMETYPE_SOCCER) return(DTRUE);

	return(DFALSE);
}


// This is always available, once you create your server shell.
extern CBloodClientShell *g_pBloodClientShell;


#endif __BLOODCLIENTSHELL_H__