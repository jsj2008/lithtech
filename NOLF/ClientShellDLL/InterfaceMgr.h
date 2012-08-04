// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_MGR_H__
#define __INTERFACE_MGR_H__

#include "iltclient.h"
#include "iltcursor.h"

#include "PlayerStats.h"
#include "GameSettings.h"
#include "ClientInfoMgr.h"
#include "MessageMgr.h"
#include "InterfaceResMgr.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "ServerOptionMgr.h"
#include "LTWnd.h"
#include "LTDialogueWnd.h"
#include "LTMenuWnd.h"
#include "MissionData.h"
#include "WeaponChooser.h"
#include "MissionText.h"
#include "Subtitle.h"
#include "Credits.h"
#include "PopupText.h"
#include "InterfaceTimer.h"
#include "InterfaceMeter.h"
#include "MessageBox.h"
#include "Overlays.h"
#include "MultiplayerMenu.h"
#include "SpecialFX.h"
#include "BaseScaleFX.h"
#include "Music.h"
#include "LoadingScreen.h"
#include "iltvideomgr.h"

#include "Timer.h"

#define AO_MUSIC				(1<<0)
#define AO_SOUND				(1<<1)
#define AO_MOVIES				(1<<2)
#define AO_CURSOR				(1<<3)
#define AO_FOG					(1<<4)
#define AO_LINES				(1<<5)
#define AO_JOYSTICK				(1<<7)
#define AO_TRIPLEBUFFER			(1<<9)
#define AO_TJUNCTIONS			(1<<10)

#define AO_DEFAULT_ENABLED		(AO_MUSIC | AO_SOUND | AO_MOVIES | AO_CURSOR | AO_FOG | AO_LINES | AO_TRIPLEBUFFER | AO_TJUNCTIONS | AO_JOYSTICK)

// Game states


enum GameState
{
	GS_UNDEFINED=0,
	GS_PLAYING,
	GS_LOADINGLEVEL,
	GS_SPLASHSCREEN,
	GS_DIALOGUE,
	GS_MENU,
	GS_POPUP,
	GS_FOLDER,
	GS_PAUSED,
	GS_FAILURE,
	GS_DEMOSCREEN,
	GS_MOVIE
};

enum InterfaceSound
{
	IS_NONE,
	IS_CHANGE,
	IS_SELECT,
	IS_PAGE,
	IS_UP,
	IS_DOWN,
	IS_LEFT,
	IS_RIGHT,
	IS_NO_SELECT
};

enum ISFXType
{
	IFX_NORMAL,
	IFX_WORLD,
	IFX_ATTACH
};


const HLTCOLOR kWhite		= SETRGB(255,255,255);
const HLTCOLOR kGray		= SETRGB(96,96,96);
const HLTCOLOR kBlack		= SETRGB(0,0,0);
const HLTCOLOR kTransBlack	= SETRGB_T(0,0,0);


class CInterfaceMgr;
extern CInterfaceMgr* g_pInterfaceMgr;

class CInterfaceMgr
{
	public :

		CInterfaceMgr();
		virtual ~CInterfaceMgr();

        LTBOOL  Init();
		LTBOOL	InitCursor();
		void	Term();

        void    OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
		void	OnExitWorld();

        LTBOOL	OnCommandOn(int command);
        LTBOOL	OnCommandOff(int command);
        LTBOOL	OnKeyDown(int key, int rep);
        LTBOOL	OnKeyUp(int nKey);
        LTBOOL	OnMessage(uint8 messageID, HMESSAGEREAD hMessage);
        LTBOOL	OnEvent(uint32 dwEventID, uint32 dwParam);

		void	SetLetterBox(LTBOOL b) { m_bLetterBox = b; }
		void	ClosePopup() { m_PopupText.Clear(); }

		// Screen Fade functions
        void	StartScreenFadeIn(LTFLOAT fFadeTime)
		{
			if (!m_bExitAfterFade)
			{
				m_bFadeInitialized = LTFALSE;
				m_bScreenFade = LTTRUE;
				m_fTotalFadeTime = fFadeTime;
				m_bFadeIn = LTTRUE;
			}
		}
        void    StartScreenFadeOut(LTFLOAT fFadeTime)
		{
			if (!m_bExitAfterFade)
			{
				m_bFadeInitialized = LTFALSE;
				m_bScreenFade = LTTRUE;
				m_fTotalFadeTime = fFadeTime;
				m_bFadeIn = LTFALSE;
			}
		}

		void	ForceScreenFadeIn(LTFLOAT fFadeTime)
		{
			if (!m_bExitAfterFade)
			{
				if (FadingScreen() && !FadingScreenIn())
				{
					StartScreenFadeIn(fFadeTime);
				}
			}
		}

		LTBOOL	FadingScreen() const { return m_bScreenFade; }

		LTBOOL	FadingScreenIn()	const { return (m_bScreenFade && m_bFadeIn); }
		LTBOOL	FadingScreenOut()	const { return (m_bScreenFade && !m_bFadeIn); }

		LTBOOL  FadingToExit()	const { return (m_bScreenFade && !m_bFadeIn && m_bExitAfterFade); }

		LTBOOL	ScreenFadedOut()	const { return (FadingScreenOut() && ScreenFadeDone()); }
		LTBOOL	ScreenFadedIn()		const { return !m_bScreenFade; }

		LTBOOL	ScreenFadeDone() const
		{
			if (m_hFadeSurface)
			{
				LTFLOAT fAlpha = 0.0f;
				g_pLTClient->GetSurfaceAlpha(m_hFadeSurface, fAlpha);
				return (m_bFadeIn ? (fAlpha <= 0.0f) : (fAlpha >= 1.0f));
			}
			return LTTRUE;
		}

		//mouse messages
        void OnLButtonUp(int x, int y);
		void OnLButtonDown(int x, int y);
		void OnLButtonDblClick(int x, int y);
		void OnRButtonUp(int x, int y);
		void OnRButtonDown(int x, int y);
		void OnRButtonDblClick(int x, int y);
		void OnMouseMove(int x, int y);
		void OnChar(char c);

        LTBOOL   PreUpdate();
        LTBOOL   Update();
        LTBOOL   PostUpdate();

        LTBOOL   Draw();

		LTBOOL	 DrawSFX();

        LTBOOL       ChangeState(GameState eNewState);
		void		DebugChangeState(GameState eNewState);
		GameState	GetGameState() const { return m_eGameState; }

		LTBOOL		UseInterfaceCamera() {return m_bUseInterfaceCamera;}

		eFolderID	GetCurrentFolder();
        LTBOOL       SwitchToFolder(eFolderID folderID);
        LTBOOL       ForceFolderUpdate(eFolderID folderID);

		void	MissionFailed(int nFailStringId);
		void	StartMissionText(int nMissionTextId) {m_MissionText.Start(nMissionTextId); }
		void	StartMissionText(HSTRING hMissionText) {m_MissionText.Start(hMissionText); }
		void	StartMissionText(char *pszMissionText) {m_MissionText.Start(pszMissionText); }

		void	ShowSubtitle(int nSubtitleId, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f)	{m_Subtitle.Show(nSubtitleId, vSpeakerPos, fRadius, fDuration);}
		void	ClearSubtitle()	{m_Subtitle.Clear();}

		CSubtitle* GetSubtitle() {return &m_Subtitle;}

		void	ShowDemoScreens(LTBOOL bQuit);

		void	Disconnected(uint32 nDisconnectFlag);
		void	ConnectionFailed(uint32 nConnectionError);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

        void    SetDrawInterface(LTBOOL bDraw)           { m_bDrawInterface = bDraw; }
        void    SetSwitchingRenderModes(LTBOOL bSwitch)  { m_bSwitchingModes = bSwitch; }

		void	ResetMenuRestoreCamera(int nLeft, int nTop, int nRight, int nBottom);

        void    ClearScreenAlways(LTBOOL bYes = LTTRUE)   { m_bClearScreenAlways = bYes; }
		void	AddToClearScreenCount()					{ m_nClearScreenCount = 3; }
		void	ZeroClearScreenCount()					{ m_nClearScreenCount = 0; }
		void	ClearAllScreenBuffers();

		void	SetLoadGameMenu();
		void	SendMissionDataToServer(LTBOOL bUpdatePlayer=LTFALSE);

		void	DoMissionOutfitCheat();

        LTBOOL   SetMenuMusic(LTBOOL bMusicOn);
        LTBOOL   RestoreGameMusic();

		CGameSettings*	GetSettings()		{ return &m_Settings; }
		CPlayerStats*	GetPlayerStats()	{ return &m_stats; }
		CMessageMgr*	GetMessageMgr()		{ return &m_messageMgr; }
        CClientInfoMgr* GetClientInfoMgr()  { return &m_ClientInfo; }
		CFolderMgr*		GetFolderMgr()		{ return &m_FolderMgr; }
		CMissionData*	GetMissionData()	{ return &m_MissionData; }

        uint32          GetAdvancedOptions() const { return m_dwAdvancedOptions; }

        void        EnableCrosshair(LTBOOL bOn)       { m_stats.EnableCrosshair(bOn); }
        LTBOOL      IsCrosshairEnabled()             { return m_stats.CrosshairEnabled(); }
        LTBOOL      IsCrosshairOn()                  { return m_stats.CrosshairOn(); }

        void        DrawPlayerStats(LTBOOL bOn=LTTRUE)  { m_bDrawPlayerStats = bOn; }
        void        DrawFragCount(LTBOOL bOn=LTTRUE)	{ m_bDrawFragCount = bOn; }
		LTBOOL		IsFragCountDrawn()					{ return m_bDrawFragCount; }

        LTBOOL      IsChoosingWeapon()				{ return m_WeaponChooser.IsOpen();}
        LTBOOL      IsChoosingAmmo()                { return m_AmmoChooser.IsOpen();}
		void		CloseChoosers()					{ m_WeaponChooser.Close();m_AmmoChooser.Close();}

        LTBOOL      AllowCameraMovement();

        void        UpdatePlayerStats(uint8 nThing, uint8 nType1, uint8 nType2, LTFLOAT fAmount);
        void        UpdateWeaponStats(uint8 nWeaponId, uint8 nAmmoId, uint32 dwAmmo);

		void		ScreenDimsChanged();

        LTIntPt     GetCursorPos() {return m_CursorPos;}
        LTBOOL		IsCursorUsed() {return m_bUseCursor;}
        void        UseCursor(LTBOOL bUseCursor);
        void        UseHardwareCursor(LTBOOL bUseHardwareCursor);
		void		UpdateCursorState(); // hides or shows cursor based on current game state

		void			UpdateOverlays();
		eOverlayMask	GetCurrentOverlay()			 { return m_eCurrOverlay; }

        void            BeginScope(LTBOOL bNightVision = LTFALSE);
		void			EndScope();

        void            BeginZoom(LTBOOL bIn = LTTRUE);
		void			EndZoom();

		void			BeginUnderwater();
		void			EndUnderwater();

		void			BeginSpacesuit();
		void			EndSpacesuit();

		void			SetSunglassMode(eSunglassMode mode);
		eSunglassMode	GetSunglassMode() {return m_eSunglassMode;}

        void        ShowMessageBox(HSTRING hString, eMBType eType, MBCallBackFn pFn, void *pData = LTNULL, LTBOOL bLargeFont = LTTRUE, LTBOOL bDefaultReturn = LTTRUE)
								{m_MessageBox.Show(hString, eType, pFn, pData, bLargeFont, bDefaultReturn);}

		void		ChooseTeam();
		void		ViewOptions();

		void		AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType);
		void		RemoveInterfaceSFX(CSpecialFX* pSFX);

		void		RequestInterfaceSound(InterfaceSound eIS);
		void		ClearInterfaceSound();

		// Update the interface model animations
		void	UpdateModelAnimations(LTFLOAT fCurFrameDelta);

		// Hide the loading screen.
		// (Note : This does not change out of the loading state, it only stops the loading screen's rendering)
		void	HideLoadScreen();

		void	LoadFailed()  {m_bLoadFailed = LTTRUE;}
		void	SetLoadLevelString(HSTRING hWorld);
		void	SetLoadLevelPhoto(char *pszPhoto);

		void	UpdateCursor();

		//only used if playing a multiplayer game and has chosen to start a new one
		void	StartingNewGame() {m_bStartedNew = LTTRUE;}

		void	UpdateClientList();

	private :
		CInterfaceResMgr	m_InterfaceResMgr;		// manages shared resources
		CFolderMgr			m_FolderMgr;
		CLayoutMgr			m_LayoutMgr;			// bute mgr for layout info
        CServerOptionMgr    m_ServerOptionMgr;      // bute mgr for server options
		CMissionData		m_MissionData;

		CLTWnd			m_MainWnd;				// Main window
		CLTDialogueWnd	m_DialogueWnd;			// Dialogue window

		CLTMenuWnd		m_MenuWnd;				// Menu window
        CClientInfoMgr  m_ClientInfo;           // Client info mgr

		CPlayerStats	m_stats;				// Player statistics (health, ammo, armor, etc.)
		CMessageMgr		m_messageMgr;			// Message display/sending mgr
		CGameSettings	m_Settings;
		CWeaponChooser	m_WeaponChooser;		// Next/previous weapon interface
		CAmmoChooser	m_AmmoChooser;			// Next ammo interface
		CInterfaceTimer m_InterfaceTimer;		// Main interface timer
		CInterfaceMeter m_InterfaceMeter;		// Meter used for Boss levels
		CMissionText	m_MissionText;			// Display timed text
		CSubtitle		m_Subtitle;				// Display subtitle
		CCredits		m_Credits;				// Display credits
		CPopupText		m_PopupText;			// Display in game text
		CMessageBox		m_MessageBox;			// Used for simple dialog boxes

		CMultiplayerMenu	m_MultiplayerMenu;	// In-game Multiplayer menu
		CTeamMenu			m_TeamMenu;			// In-game Team menu
		COptionMenu			m_OptionMenu;		// In-game Option menu
		GameState		m_eGameState;			// Current game state

		eOverlayMask	m_eCurrOverlay;			// Currently displayed overlay

        uint32      m_dwAdvancedOptions;        // Advanced options
        uint32      m_dwOrignallyEnabled;       // Advanced options that were originally enabled

        uint8       m_nClearScreenCount;        // How many frames to clear the screen
        LTBOOL      m_bClearScreenAlways;       // Should we always clear the screen?

        LTBOOL      m_bDrawPlayerStats;         // Draw the player stats display?
        LTBOOL      m_bDrawFragCount;           // Draw the frag count?
        LTBOOL      m_bDrawInterface;           // Draw the interface?

        HLTSOUND    m_hSplashSound;             // Handle to sound played when splash screen is up

		HSURFACE	m_hGamePausedSurface;		// "Game Paused" message

        LTRect      m_rcMenuRestoreCamera;      // Camera rect to restore after leaving menus
        LTBOOL      m_bMenuRectRestore;         // Was the camera rect full-screen before going to the menus?

		LTBOOL		m_bUseInterfaceCamera;

        LTFLOAT     m_fMenuSaveFOVx;            // Fov before entering menu
        LTFLOAT     m_fMenuSaveFOVy;            // Fov before entering menu

        LTBOOL      m_bSwitchingModes;          // Switching render modes?

        LTIntPt     m_CursorPos;

		int			m_nFailStringId;			// id of the string to display on the mission failed screen
		HSURFACE	m_hFailBack;				// background of Mission Failure screen

        HLTSOUND    m_hScubaSound;              // sound looping while scuba gear is on

		HLTSOUND	m_hSound;					// current interface snd
		HSURFACE	m_hFadeSurface;				// Used to do screen fading
		HSURFACE	m_hLetterBoxSurface;		// Used for letter box border
        LTBOOL      m_bFadeInitialized;         // Have we initialized everything
        LTBOOL      m_bScreenFade;              // Should we fade the screen
        LTFLOAT     m_fTotalFadeTime;           // How long to do the fade
        LTFLOAT     m_fCurFadeTime;             // Current fade time
        LTBOOL      m_bFadeIn;                  // Should we fade in (or out)
        LTBOOL      m_bExitAfterFade;           // Exit the level after the fade

        LTBOOL      m_bLetterBox;               // Letter box mode?
		LTBOOL		m_bWasLetterBox;			// Was letter box last frame?
		LTFLOAT		m_fLetterBoxFadeEndTime;	// When do we stop fading the letter box in/out
		LTFLOAT		m_fLetterBoxAlpha;			// The current letter box border alpha


		HLTCURSOR	m_hCursor;
        LTBOOL      m_bUseCursor;
        LTBOOL      m_bUseHardwareCursor;

        uint8       m_nOverlayCount;
		HOBJECT		m_hOverlays[NUM_OVERLAY_MASKS];
        LTFLOAT     m_fOverlayScaleMult[NUM_OVERLAY_MASKS];

		CTimer		m_NextWeaponKeyDownTimer;
		CTimer		m_PrevWeaponKeyDownTimer;
		CTimer		m_NextAmmoKeyDownTimer;
		CTimer		m_AutoSwitchTimer;


		CMoArray<CSpecialFX *> m_InterfaceSFX;

		CBaseScaleFX m_BackSprite;

        LTBOOL      m_bSavedGameMusic;          // Did we save the game music state?
		CMusicState	m_GameMusicState;			// State of the game music

		CLoadingScreen m_LoadingScreen;			// The loading screen object/thread
		LTBOOL		m_bLoadFailed;

		eSunglassMode m_eSunglassMode;

		InterfaceSound	m_eNextSound;

        LTBOOL  PreChangeState(GameState eCurState, GameState eNewState);

        LTBOOL	PrePlayingState(GameState eCurState);
		void	UpdatePlayingState();
        LTBOOL  PostPlayingState(GameState eNewState);

        LTBOOL  PreDialogueState(GameState eCurState);
		void	UpdateDialogueState();
        LTBOOL  PostDialogueState(GameState eNewState);

        LTBOOL  PreMenuState(GameState eCurState);
		void	UpdateMenuState();
        LTBOOL  PostMenuState(GameState eNewState);

        LTBOOL  PrePopupState(GameState eCurState);
		void	UpdatePopupState();
        LTBOOL  PostPopupState(GameState eNewState);

        LTBOOL  PreLoadingLevelState(GameState eCurState);
		void	UpdateLoadingLevelState();
        LTBOOL  PostLoadingLevelState(GameState eNewState);

        LTBOOL  PrePauseState(GameState eCurState);
		void	UpdatePausedState();
        LTBOOL  PostPauseState(GameState eNewState);

        LTBOOL  PreSplashScreenState(GameState eCurState);
		void	UpdateSplashScreenState();
        LTBOOL  PostSplashScreenState(GameState eNewState);

        LTBOOL  PreFolderState(GameState eCurState);
		void	UpdateFolderState();
        LTBOOL  PostFolderState(GameState eNewState);

        LTBOOL  PreFailureState(GameState eCurState);
		void	UpdateFailureState();
        LTBOOL  PostFailureState(GameState eNewState);

        LTBOOL  PreDemoScreenState(GameState eCurState);
		void	UpdateDemoScreenState();
        LTBOOL  PostDemoScreenState(GameState eNewState);

        LTBOOL  PreMovieState(GameState eCurState);
		void	UpdateMovieState();
        LTBOOL  PostMovieState(GameState eNewState);

		void	ProcessAdvancedOptions();

		void	CreateOverlay(eOverlayMask eMask);
		void	RemoveOverlay(eOverlayMask eMask);

		void	UpdateScreenFade();
		void	UpdateLetterBox();

		void	CreateInterfaceBackground();
		void	UpdateInterfaceBackground();
		void	RemoveInterfaceBackground();
		void	RemoveAllInterfaceSFX();
		void	UpdateInterfaceSFX();

		HLTSOUND UpdateInterfaceSound();

		void	UpdateChooserAutoSwitch();

		void	NextDemoScreen();
		LTBOOL	m_bQuitAfterDemoScreens;
		LTBOOL	m_bSeenDemoScreens;

		void	NextMovie();
		char*	GetCurrentMovie();
		HVIDEO	m_hMovie;
		int		m_nCurMovie;

		LTBOOL  m_bStartedNew;

		float	m_fLastUpdateRequestTime;

		// Used for tracking when a new level is loaded..
		uint32	m_nLoadWorldCount;
		uint32	m_nOldLoadWorldCount;
};


inline void CInterfaceMgr::ResetMenuRestoreCamera(int nLeft, int nTop, int nRight, int nBottom)
{
	if (m_rcMenuRestoreCamera.right != 0 && m_rcMenuRestoreCamera.bottom != 0)
	{
		m_rcMenuRestoreCamera.left = nLeft;
		m_rcMenuRestoreCamera.top = nTop;
		m_rcMenuRestoreCamera.right = nRight;
		m_rcMenuRestoreCamera.bottom = nBottom;
	}
}

// ----------------------------------------------------------------------- //
// Clears the screen a few times so the backbuffer(s) get cleared.
// ----------------------------------------------------------------------- //
inline void CInterfaceMgr::ClearAllScreenBuffers()
{
	for (int i=0; i < 4; i++)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
        g_pLTClient->FlipScreen(0);
	}
}


#endif // __INTERFACE_MGR_H__