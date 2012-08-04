// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_MGR_H__
#define __INTERFACE_MGR_H__

#include "iltclient.h"
#include "iltfontmanager.h"

#include "HUDMgr.h"
#include "GameSettings.h"
#include "ClientInfoMgr.h"
#include "InterfaceResMgr.h"
#include "ScreenMgr.h"
#include "MenuMgr.h"
#include "LayoutMgr.h"
#include "MessageBox.h"
#include "WeaponChooser.h"
#include "Credits.h"
#include "InterfaceTimer.h"
#include "FullScreenTint.h"
#include "Overlays.h"
#include "SpecialFX.h"
#include "BaseScaleFX.h"
#include "Music.h"
#include "LoadingScreen.h"
#include "iltvideomgr.h"
#include "ClientResShared.h"
#include "PerformanceMgr.h"
#include "ProfileMgr.h"
#include "PopupText.h"
#include "PlayerStats.h"
#include "ClientFxMgr.h"
#include "CursorMgr.h"
#include "LTObjRef.h"


extern ILTModelClient*	g_pILTModelClient;
extern ILTDrawPrim*		g_pDrawPrim;
extern ILTFontManager*	g_pFontManager;
extern ILTTexInterface*	g_pTexInterface;

#define AO_MUSIC				(1<<0)
#define AO_SOUND				(1<<1)
#define AO_MOVIES				(1<<2)
#define AO_CURSOR				(1<<3)
#define AO_FOG					(1<<4)
#define AO_LINES				(1<<5)
#define AO_JOYSTICK				(1<<7)
#define AO_TRIPLEBUFFER			(1<<9)
#define AO_TJUNCTIONS			(1<<10)
#define AO_HARDWARESOUND		(1<<11)
#define AO_SOUNDFILTERS			(1<<12)

#define AO_DEFAULT_ENABLED		(AO_MUSIC | AO_SOUND | AO_MOVIES | AO_CURSOR | AO_FOG | AO_LINES | AO_TRIPLEBUFFER | AO_TJUNCTIONS | AO_JOYSTICK | AO_HARDWARESOUND | AO_SOUNDFILTERS)

// Game states


enum GameState
{
	GS_UNDEFINED=0,
	GS_PLAYING,
	GS_EXITINGLEVEL,
	GS_LOADINGLEVEL,
	GS_SPLASHSCREEN,
	GS_MENU,
	GS_POPUP,
	GS_SCREEN,
	GS_PAUSED,
	GS_DEMOSCREEN,
	GS_MOVIE,
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
	IFX_ATTACH,
	IFX_MENU_ATTACH,
};

const HLTCOLOR kWhite		= SETRGB(255,255,255);
const HLTCOLOR kGray		= SETRGB(96,96,96);
const HLTCOLOR kBlack		= SETRGB(0,0,0);
const HLTCOLOR kTransBlack	= SETRGB_T(0,0,0);

const uint32	argbWhite		= 0xFFFFFFFF;
const uint32	argbGray		= 0xFF606060;
const uint32	argbBlack		= 0xFF000000;
const uint32	argbTransBlack	= 0x00000000;

class CInterfaceMgr;
extern CInterfaceMgr* g_pInterfaceMgr;

typedef std::vector<LTObjRef> ObjectArray;

class CInterfaceMgr
{
	public :

		CInterfaceMgr();
		virtual ~CInterfaceMgr();

        virtual LTBOOL  Init();
		virtual void	Term();
		bool	IsInitialized( ) { return m_bInitialized; }

        virtual void    OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
		virtual void	OnExitWorld();

        virtual LTBOOL	OnCommandOn(int command);
        virtual LTBOOL	OnCommandOff(int command);
        virtual LTBOOL	OnKeyDown(int key, int rep);
        virtual LTBOOL	OnKeyUp(int nKey);

        virtual LTBOOL OnMessage(uint8 messageID, ILTMessage_Read *pMsg);
        virtual LTBOOL	OnEvent(uint32 dwEventID, uint32 dwParam);
		virtual void	OnObjectRemove(HLOCALOBJ hObj);

		void	SetLetterBox(LTBOOL b) { m_bLetterBox = b; }

		void	ShowPopup(uint32 nTextId, uint8 nPopupId, bool bHideHUD=false) 
		{
			m_bHideHUDInPopup = bHideHUD;
			m_PopupText.Show(nTextId,nPopupId);
		}
		void	ShowPopup(char *pText, uint8 nPopupId, bool bHideHUD=false) 
		{ 
			m_bHideHUDInPopup = bHideHUD;
			m_PopupText.Show(pText, nPopupId);
		}
		void	ClosePopup() { m_PopupText.Close(); }

		// Check if we're in one of the "in game" states.
		bool	IsInGame( );

		// Screen Fade functions
        void	StartScreenFadeIn(LTFLOAT fFadeTime)
		{
			m_bFadeInitialized = LTFALSE;
			m_bScreenFade = LTTRUE;
			m_fTotalFadeTime = fFadeTime;
			m_bFadeIn = LTTRUE;
		}
        void    StartScreenFadeOut(LTFLOAT fFadeTime)
		{
			m_bFadeInitialized = LTFALSE;
			m_bScreenFade = LTTRUE;
			m_fTotalFadeTime = fFadeTime;
			m_bFadeIn = LTFALSE;
		}

		void	ForceScreenFadeIn(LTFLOAT fFadeTime)
		{
			if (FadingScreen() && !FadingScreenIn())
			{
				StartScreenFadeIn(fFadeTime);
			}
		}

		LTBOOL	FadingScreen() const { return m_bScreenFade; }

		LTBOOL	FadingScreenIn()	const { return (m_bScreenFade && m_bFadeIn); }
		LTBOOL	FadingScreenOut()	const { return (m_bScreenFade && !m_bFadeIn); }

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
/*
			int iAlpha = m_FadePoly.rgba.a;
			return (m_bFadeIn ? (iAlpha <= 0) : (iAlpha >= 255));			
*/
		}

		//mouse messages
        void OnLButtonUp(int x, int y);
		void OnLButtonDown(int x, int y);
		void OnLButtonDblClick(int x, int y);
		void OnRButtonUp(int x, int y);
		void OnRButtonDown(int x, int y);
		void OnRButtonDblClick(int x, int y);
		void OnMouseMove(int x, int y);
		void OnChar(unsigned char c);

        LTBOOL   PreUpdate();
        LTBOOL   Update();
        LTBOOL   PostUpdate();

        LTBOOL   Draw();

		LTBOOL	 DrawSFX();

        LTBOOL       ChangeState(GameState eNewState);
		void		DebugChangeState(GameState eNewState);
		GameState	GetGameState() const { return m_eGameState; }
		bool		HasEnteredScreenState() const { return m_bEnteredScreenState; }

		LTBOOL		UseInterfaceCamera() {return m_bUseInterfaceCamera;}
		HLOCALOBJ	GetInterfaceCamera()	const	{ return m_hInterfaceCamera; }

		eScreenID	GetCurrentScreen();
        LTBOOL       SwitchToScreen(eScreenID screenID);
        LTBOOL       SwitchToMenu(eMenuID menuID);
        LTBOOL       ForceScreenUpdate(eScreenID screenID);

		void	AbortScreenFade();

		void	MissionFailed(int nFailStringId);
		int		GetFailStringID() {return m_nFailStringId;}

		void	ShowDemoScreens(LTBOOL bQuit);

		void	Disconnected(uint32 nDisconnectFlag);
		void	ConnectionFailed(uint32 nConnectionError);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

        void    SetSwitchingRenderModes(LTBOOL bSwitch)  { m_bSwitchingModes = bSwitch; }

		void	ResetMenuRestoreCamera(int nLeft, int nTop, int nRight, int nBottom);

        void    ClearScreenAlways(LTBOOL bYes = LTTRUE)   { m_bClearScreenAlways = bYes; }
		void	AddToClearScreenCount()					{ m_nClearScreenCount = 3; }
		void	ZeroClearScreenCount()					{ m_nClearScreenCount = 0; }
		void	ClearAllScreenBuffers();

        LTBOOL   SetupMusic();
        LTBOOL   RestoreGameMusic();

		CGameSettings*	GetSettings()		{ return &m_Settings; }
        CClientInfoMgr* GetClientInfoMgr()  { return &m_ClientInfo; }
		CMenuMgr*		GetMenuMgr()		{ return &m_MenuMgr; }

		CFullScreenTint* GetFullScreenTint() { return &m_FullScreenTint; }

		virtual CHUDMgr* GetHUDMgr() = 0;
		virtual CScreenMgr* GetScreenMgr( ) = 0;
		virtual CPlayerStats* GetPlayerStats( ) = 0;

        uint32          GetAdvancedOptions() const { return m_dwAdvancedOptions; }

 		void        EnableCrosshair(LTBOOL bOn)       { /*m_stats.EnableCrosshair(bOn); */}
        LTBOOL      IsCrosshairEnabled()             { return LTTRUE;/* m_stats.CrosshairEnabled(); */}
        LTBOOL      IsCrosshairOn()                  { return LTTRUE;/* m_stats.CrosshairOn(); */}


		void		SetHUDRenderLevel(eHUDRenderLevel eLevel) {GetHUDMgr()->SetRenderLevel(eLevel);}

        LTBOOL      IsChoosingWeapon()				{ return m_WeaponChooser.IsOpen();}
        LTBOOL      IsChoosingAmmo()                { return m_AmmoChooser.IsOpen();}
		void		CloseChoosers()					{ m_WeaponChooser.Close();m_AmmoChooser.Close();}

		virtual CWeaponChooser*	GetWeaponChooser()	{ return &m_WeaponChooser;}
		virtual CAmmoChooser*	GetAmmoChooser()	{ return &m_AmmoChooser;}

        LTBOOL      AllowCameraMovement();
		LTBOOL		AllowCameraRotation();

        virtual void UpdatePlayerStats(uint8 nThing, uint8 nType1, uint8 nType2, LTFLOAT fAmount);

		void		ScreenDimsChanged();

        LTIntPt     GetCursorPos() {return m_CursorPos;}
		void		UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter = LTFALSE) {m_CursorMgr.UseCursor(bUseCursor,bLockCursorToCenter);}
		void		UpdateCursorState(); // hides or shows cursor based on current game state

		void		UpdateOverlays();
		bool		IsOverlayActive(eOverlayMask eMask) {return (m_hOverlays[eMask] != NULL);}
		void		CreateOverlay(eOverlayMask eMask);
		void		RemoveOverlay(eOverlayMask eMask);

        void		BeginScope(LTBOOL bVisionMode = LTFALSE, LTBOOL bCamera = LTFALSE);
		void		EndScope();

        void        BeginZoom(LTBOOL bIn = LTTRUE);
		void		EndZoom();

	    void		ShowMessageBox(int nStringID, MBCreate* pCreate, uint8 nFontSize = 0, LTBOOL bDefaultReturn = LTTRUE);
	    void		ShowMessageBox(const char *pString, MBCreate* pCreate, uint8 nFontSize = 0, LTBOOL bDefaultReturn = LTTRUE);
	    void		CloseMessageBox(LTBOOL bReturn);


		void		ViewOptions();

		void		AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType);
		void		RemoveInterfaceSFX(CSpecialFX* pSFX);

		void		AddInterfaceFX(CLIENTFX_LINK* pLink, char *pFXName, LTVector vPos, LTBOOL bLoop);
		void		RemoveInterfaceFX();
		void		RemoveInterfaceFX(CLIENTFX_LINK	*pLink);

		void		AddInterfaceLight(HOBJECT hLight);
		void		RemoveInterfaceLights();

		void		RequestInterfaceSound(InterfaceSound eIS);
		void		ClearInterfaceSound();


		//accessor for the interfaces FX manager
		CClientFXMgr&	GetInterfaceFXMgr()		{ return m_InterfaceFXMgr; }

		// Update the interface model animations
		void	UpdateModelAnimations(LTFLOAT fCurFrameDelta);

		// Hide the loading screen.
		// (Note : This does not change out of the loading state, it only stops the loading screen's rendering)
		void	HideLoadScreen();

		// Is the loading screen visible?
		bool	IsLoadScreenVisible() { return m_LoadingScreen.IsVisible() != LTFALSE; }
		// Pause the loading screen animation
		void	PauseLoadScreen() { m_LoadingScreen.Pause(); }
		// Resume the loading screen animation
		void	ResumeLoadScreen() { m_LoadingScreen.Resume(); }
		// Update loading screen mission info
		void	UpdateLoadScreenInfo() { m_LoadingScreen.UpdateMissionInfo(); }
		// Set the screen to render on the loading screen
		void	SetLoadingRenderScreen( CBaseScreen *pScreen ) { m_LoadingScreen.SetRenderScreen( pScreen ); }
		

		//called to indicate an intentional disconnection from the server
		void	SetIntentionalDisconnect( bool bIntentionalDisconnect ) { m_bIntentionalDisconnect = bIntentionalDisconnect; }
		bool	GetIntentionalDisconnect( ) const { return m_bIntentionalDisconnect; }

		void	LoadFailed( eScreenID eScreenToShow = SCREEN_ID_NONE, uint32 nMsgId = -1 );

		void	UpdateClientList();

		void	SetMouseFX(char* pFXName);
		void	SetSelectFX(char* pFXName);
		void	ShowMouseFX();
		void	ShowSelectFX(const LTIntPt &pos);

		LTVector GetWorldFromScreenPos(LTIntPt pos, LTFLOAT fDepth);

		//transforms world space to camera space to screen coordinates
		//return vector's x and y are screen coordinates
		//return vector's z is distance from the camera
		LTVector  GetScreenFromWorldPos(LTVector vPos, HOBJECT hCamera);

		// Handle display timer message.
		bool	HandleDisplayTimerMsg( ILTMessage_Read& msg );

		// Indicates we joined using the "+join" command line and we
		// should take special precautions if we fail.
		bool	GetCommandLineJoin( ) const { return m_bCommandLineJoin; }
		void	SetCommandLineJoin( bool bCommandLineJoin ) { m_bCommandLineJoin = bCommandLineJoin; }

		void	SkipPreLoad( bool bSkip ) { m_bSkipPreLoadScreen = bSkip; }
		bool	ShouldSkipPreLoad() const { return m_bSkipPreLoadScreen; }
		void	SetPostLoadScreenID( eScreenID eID = SCREEN_ID_POSTLOAD ) { m_ePostLoadScreenID = eID; }

		// Called when a new game is started.
		bool	StartingNewGame( );

	protected:
		CWeaponChooser	m_WeaponChooser;		// Next/previous weapon interface
		CAmmoChooser	m_AmmoChooser;			// Next ammo interface
		GameState		m_eGameState;			// Current game state

	private :
		CInterfaceResMgr	m_InterfaceResMgr;		// manages shared resources
		CMenuMgr			m_MenuMgr;
//		CLayoutMgr			m_LayoutMgr;			// bute mgr for layout info
		CPerformanceMgr		m_PerformanceMgr;		// manages performance settings
		CProfileMgr			m_ProfileMgr;			// manages player profiles

        CClientInfoMgr  m_ClientInfo;           // Client info mgr

		CScreenSpriteMgr m_ScreenSpriteMgr;		// Screen sprite mgr (new for Tron/TO2)

		CCursorMgr		m_CursorMgr;			// Cursor handler (new for Tron/TO2)

		CGameSettings	m_Settings;
		CInterfaceTimer m_InterfaceTimer;		// Main interface timer
		CInterfaceTimer m_RedInterfaceTimer;	// Blue team interface timer
		CInterfaceTimer m_BlueInterfaceTimer;	// Red team interface timer
		CCredits		m_Credits;				// Display credits
		CPopupText		m_PopupText;			// Display in game text
		CMessageBox		m_MessageBox;			// Used for simple dialog boxes

		CFullScreenTint m_FullScreenTint;		// All full screen tinting (used for pause)

		GameState		m_eLastGameState;		// Previous game state


        uint32      m_dwAdvancedOptions;        // Advanced options
        uint32      m_dwOrignallyEnabled;       // Advanced options that were originally enabled

        uint8       m_nClearScreenCount;        // How many frames to clear the screen
        LTBOOL      m_bClearScreenAlways;       // Should we always clear the screen?

        HLTSOUND    m_hSplashSound;             // Handle to sound played when splash screen is up

		HSURFACE	m_hGamePausedSurface;		// "Game Paused" message

        LTRect      m_rcMenuRestoreCamera;      // Camera rect to restore after leaving menus
        LTBOOL      m_bMenuRectRestore;         // Was the camera rect full-screen before going to the menus?

		LTBOOL		m_bUseInterfaceCamera;
		HLOCALOBJ	m_hInterfaceCamera;	// The camera used in the interface

        LTFLOAT     m_fMenuSaveFOVx;            // Fov before entering menu
        LTFLOAT     m_fMenuSaveFOVy;            // Fov before entering menu

        LTBOOL      m_bSwitchingModes;          // Switching render modes?

        LTIntPt     m_CursorPos;

		int			m_nFailStringId;			// id of the string to display on the mission failed screen

        HLTSOUND    m_hScubaSound;              // sound looping while scuba gear is on

		HLTSOUND	m_hSound;					// current interface snd

		HSURFACE	m_hFadeSurface;				// Used to do screen fading
		HSURFACE	m_hLetterBoxSurface;		// Used for letter box border


/*
		uint16		m_nBorderSize;				// Thickness of Letterbox border
		LT_POLYF4	m_LetterBox[2];				// Letterbox top/bottom border
		LT_POLYF4	m_FadePoly;					// Used for screen fades on PSX2
*/

        LTBOOL      m_bFadeInitialized;         // Have we initialized everything
        LTBOOL      m_bScreenFade;              // Should we fade the screen
        LTFLOAT     m_fTotalFadeTime;           // How long to do the fade
        LTFLOAT     m_fCurFadeTime;             // Current fade time
        LTBOOL      m_bFadeIn;                  // Should we fade in (or out)

        LTBOOL      m_bLetterBox;               // Letter box mode?
		LTBOOL		m_bWasLetterBox;			// Was letter box last frame?
		LTFLOAT		m_fLetterBoxFadeEndTime;	// When do we stop fading the letter box in/out
		LTFLOAT		m_fLetterBoxAlpha;			// The current letter box border alpha

        uint8       m_nOverlayCount;
		HOBJECT		m_hOverlays[NUM_OVERLAY_MASKS];
        LTFLOAT     m_fOverlayScaleMult[NUM_OVERLAY_MASKS];


		SFXArray	m_InterfaceSFX;
		ObjectArray	m_InterfaceLights;

		CLIENTFX_LINK		m_MouseFX;
		CLIENTFX_LINK		m_SelectFX;
		char				m_szMouseFXName[128];
		char				m_szSelectFXName[128];
		CClientFXMgr		m_InterfaceFXMgr;


		CBaseScaleFX m_BackSprite;

        LTBOOL      m_bSavedGameMusic;          // Did we save the game music state?
		CMusicState	m_GameMusicState;			// State of the game music

		CLoadingScreen m_LoadingScreen;			// The loading screen object/thread
		LTBOOL		m_bLoadFailed;
		bool		m_bCommandLineJoin;
		eScreenID	m_eLoadFailedScreen;
		uint32		m_nLoadFailedMsgId;

		InterfaceSound	m_eNextSound;

        LTBOOL  PreChangeState(GameState eCurState, GameState eNewState);

		bool	m_bSuppressNextFlip;

		bool	m_bEnteredScreenState;
		bool	m_bIntentionalDisconnect;

		// Pre/Post load screen...

		bool		m_bSkipPreLoadScreen;
		eScreenID	m_ePostLoadScreenID;
		
protected:
        LTBOOL	PrePlayingState(GameState eCurState);
		virtual void UpdatePlayingState();
        LTBOOL  PostPlayingState(GameState eNewState);

private:
        LTBOOL  PreMenuState(GameState eCurState);
		void	UpdateMenuState();
        LTBOOL  PostMenuState(GameState eNewState);

        LTBOOL  PrePopupState(GameState eCurState);
		void	UpdatePopupState();
        LTBOOL  PostPopupState(GameState eNewState);

        LTBOOL  PreLoadingLevelState(GameState eCurState);
		void	UpdateLoadingLevelState();
        LTBOOL  PostLoadingLevelState(GameState eNewState);

        LTBOOL  PreExitingLevelState(GameState eCurState);
		void	UpdateExitingLevelState();
        LTBOOL  PostExitingLevelState(GameState eNewState);

        LTBOOL  PrePauseState(GameState eCurState);
		void	UpdatePausedState();
        LTBOOL  PostPauseState(GameState eNewState);

        LTBOOL  PreSplashScreenState(GameState eCurState);
		void	UpdateSplashScreenState();
		void	EndSplashScreen( );
        LTBOOL  PostSplashScreenState(GameState eNewState);

        LTBOOL  PreScreenState(GameState eCurState);
		void	UpdateScreenState();
        LTBOOL  PostScreenState(GameState eNewState);

        LTBOOL  PreDemoScreenState(GameState eCurState);
		void	UpdateDemoScreenState();
        LTBOOL  PostDemoScreenState(GameState eNewState);

        LTBOOL  PreMovieState(GameState eCurState);
		void	UpdateMovieState();
        LTBOOL  PostMovieState(GameState eNewState);

		void	ProcessAdvancedOptions();

		void	UpdateScreenFade(bool bUpdateAlpha = true);
		void	UpdateLetterBox();

		void	CreateInterfaceBackground();
		void	UpdateInterfaceBackground();
		void	RemoveInterfaceBackground();
		void	RemoveAllInterfaceSFX();
		void	UpdateInterfaceSFX();

		HLTSOUND UpdateInterfaceSound();

		void	HandlePlayerTeamChange();

		void	NextWeapon(int nCommand);

		void	NextDemoScreen();
		LTBOOL	m_bQuitAfterDemoScreens;
		LTBOOL	m_bSeenDemoScreens;

		void	NextMovie(bool bEndMovies=false);
		char*	GetCurrentMovie();
		HVIDEO	m_hMovie;
		int		m_nCurMovie;

		LTBOOL  m_bStartedNew;

		float	m_fLastUpdateRequestTime;

		// true after Init called.  false after Term called.
		bool	m_bInitialized;

		bool			m_bHideHUDInPopup;
		eHUDRenderLevel m_ePrePopupHUDRenderLevel;
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
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, 0 );
        g_pLTClient->FlipScreen(0);
	}
}


inline void CInterfaceMgr::ShowMessageBox(int nStringID, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn)
{
	m_MessageBox.Show(nStringID,pCreate,nFontSize,bDefaultReturn);
}
inline void CInterfaceMgr::ShowMessageBox(const char *pString, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn)
{
	m_MessageBox.Show(pString,pCreate,nFontSize,bDefaultReturn);
}
inline void CInterfaceMgr::CloseMessageBox(LTBOOL bReturn)
{
	m_MessageBox.Close(bReturn);
}


#endif // __INTERFACE_MGR_H__
