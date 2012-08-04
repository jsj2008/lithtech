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

#include "HUDMgr.h"
#include "GameSettings.h"
#include "ClientInfoMgr.h"
#include "InterfaceResMgr.h"
#include "ScreenMgr.h"
#include "MenuMgr.h"
#include "LayoutDB.h"
#include "MessageBox.h"
#include "FullScreenTint.h"
#include "SpecialFX.h"
#include "LoadingScreen.h"
#include "ProfileMgr.h"
#include "PlayerStats.h"
#include "ClientFxMgr.h"
#include "CursorMgr.h"
#include "LTObjRef.h"
#include "ClientFXInstance.h"
#include "iltresourcemgr.h"
#include "InterfaceSound.h"

extern ILTModelClient*	g_pILTModelClient;
extern ILTDrawPrim*		g_pDrawPrim;
extern ILTTextureMgr*	g_pILTTextureMgr;
extern ILTTextureString* g_pTextureString;

#define AO_SOUND				(1<<1)
#define AO_MOVIES				(1<<2)
#define AO_CURSOR				(1<<3)
#define AO_FOG					(1<<4)
#define AO_LINES				(1<<5)
#define AO_TRIPLEBUFFER			(1<<9)
#define AO_TJUNCTIONS			(1<<10)
#define AO_HARDWARESOUND		(1<<11)
#define AO_SOUNDFILTERS			(1<<12)

#define AO_DEFAULT_ENABLED		(AO_SOUND | AO_MOVIES | AO_CURSOR | AO_FOG | AO_LINES | AO_TRIPLEBUFFER | AO_TJUNCTIONS | AO_HARDWARESOUND | AO_SOUNDFILTERS)

// Game states


enum GameState
{
	GS_UNDEFINED=0,
	GS_PLAYING,
	GS_EXITINGLEVEL,
	GS_LOADINGLEVEL,
	GS_SPLASHSCREEN,
	GS_MENU,
	GS_SCREEN,
	GS_PAUSED,
	GS_DEMOSCREEN,
	GS_MOVIE,
};

enum ISFXType
{
	IFX_NORMAL,
	IFX_WORLD,
	IFX_ATTACH,
	IFX_MENU_ATTACH,
};

class CInterfaceMgr;
extern CInterfaceMgr* g_pInterfaceMgr;

typedef std::vector<LTObjRef, LTAllocator<LTObjRef, LT_MEM_TYPE_CLIENTSHELL> > ObjectArray;

class IGameSpyPatchInfo;

class CInterfaceMgr
{
	public :

		CInterfaceMgr();
		virtual ~CInterfaceMgr();

        virtual bool  Init();
		virtual void	Term();
		bool	IsInitialized( ) { return m_bInitialized; }

        virtual void    OnEnterWorld(bool bRestoringGame=false);
		virtual void	OnExitWorld();

        virtual bool	OnCommandOn(int command);
        virtual bool	OnCommandOff(int command);
		virtual void	OnClearAllCommands() { return; }
        virtual bool	OnKeyDown(int key, int rep);
        virtual bool	OnKeyUp(int nKey);

        virtual bool	OnMessage(uint8 messageID, ILTMessage_Read *pMsg);
        virtual bool	OnEvent(uint32 dwEventID, uint32 dwParam);
		virtual void	OnObjectRemove(HLOCALOBJ hObj);

		void	SetLetterBox(bool b) { m_bLetterBox = b; }

		LTRect2f GetViewportRect();
		float GetDefaultFOVAspectRatioScale();

		// Check if we're in one of the "in game" states.
		bool	IsInGame( );

		// Screen Fade functions
        void	StartScreenFadeIn(float fFadeTime)
		{
            if( FadingScreenIn( ))
				return;

			m_bFadeInitialized = false;
			m_bScreenFade = true;
			m_fTotalFadeTime = fFadeTime;
			m_bFadeIn = true;
		}

        void    StartScreenFadeOut(float fFadeTime)
		{
			if( FadingScreenOut( ))
				return;

			m_bFadeInitialized = false;
			m_bScreenFade = true;
			m_fTotalFadeTime = fFadeTime;
			m_bFadeIn = false;
		}

		void	ForceScreenFadeIn(float fFadeTime)
		{
			if (FadingScreen() && !FadingScreenIn())
			{
				StartScreenFadeIn(fFadeTime);
			}
		}

		bool	FadingScreen() const { return m_bScreenFade; }

		bool	FadingScreenIn()	const { return (m_bScreenFade && m_bFadeIn); }
		bool	FadingScreenOut()	const { return (m_bScreenFade && !m_bFadeIn); }

		bool	ScreenFadedOut()	const { return (FadingScreenOut() && ScreenFadeDone()); }
		bool	ScreenFadedIn()		const { return !m_bScreenFade; }

		bool	ScreenFadeDone() const
		{
			if (m_bScreenFade)
			{
				return (m_bFadeIn ? (m_fScreenFadeAlpha <= 0.0f) : (m_fScreenFadeAlpha >= 1.0f));
			}
			return true;
		}
		
		bool	OverrideInitialFade() const { return m_bOverrideInitialFade; }

		//mouse messages
        void OnLButtonUp(int x, int y);
		void OnLButtonDown(int x, int y);
		void OnLButtonDblClick(int x, int y);
		void OnRButtonUp(int x, int y);
		void OnRButtonDown(int x, int y);
		void OnRButtonDblClick(int x, int y);
		void OnMouseMove(int x, int y);
		void OnMouseWheel(int x, int y, int zDelta);
		void OnChar(wchar_t c);

        bool   PreUpdate();
        bool   Update();
        bool   PostUpdate();

        bool   Draw();

		bool	 DrawSFX();

        bool		ChangeState(GameState eNewState, eScreenID screenID=SCREEN_ID_UNASSIGNED);
		void		DebugChangeState(GameState eNewState);
		GameState	GetGameState() const { return m_eGameState; }
		bool		UseInterfaceCamera() {return m_bUseInterfaceCamera;}
		HLOCALOBJ	GetInterfaceCamera()	const	{ return m_hInterfaceCamera; }

		eScreenID	GetCurrentScreen();
        bool       SwitchToScreen(eScreenID screenID);
        bool       ForceScreenUpdate(eScreenID screenID);

		void	AbortScreenFade();

		void		MissionFailed(const char* szFailStringId);
		const char*	GetFailStringID() { return m_szFailStringId; }

		void	ShowDemoScreens(bool bQuit);

		void	Disconnected(uint32 nDisconnectFlag);
		void	ConnectionFailed(uint32 nConnectionError);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

        void    SetSwitchingRenderModes(bool bSwitch)  { m_bSwitchingModes = bSwitch; }

		CGameSettings*	GetSettings()		{ return &m_Settings; }
        CClientInfoMgr* GetClientInfoMgr()  { return &m_ClientInfo; }
		CMenuMgr&		GetUserMenuMgr()	{ return m_UserMenuMgr; }
		CMenuMgr&		GetSystemMenuMgr( ) { return m_SystemMenuMgr; }

		CHUDMgr*		GetHUDMgr()			{ return &m_HUD;}
		CScreenMgr*		GetScreenMgr()		{ return &m_ScreenMgr;}
		CPlayerStats*	GetPlayerStats()	{ return &m_stats; }

        uint32          GetAdvancedOptions() const { return m_dwAdvancedOptions; }

 		void        EnableCrosshair(bool bOn)       { /*m_stats.EnableCrosshair(bOn); */}
        bool      IsCrosshairEnabled()             { return true;/* m_stats.CrosshairEnabled(); */}
        bool      IsCrosshairOn()                  { return true;/* m_stats.CrosshairOn(); */}

        bool      AllowCameraMovement();
		bool		AllowCameraRotation();

		virtual void UpdatePlayerStats(ILTMessage_Read& msg);
        virtual void UpdatePlayerInterface(ILTMessage_Read& msg);

		// Updates check for patch info.
		void		UpdatePatchInfo();

		void		UpdateSoundInitStatus();

		void		ScreenDimsChanged();

        LTVector2n     GetCursorPos() {return m_CursorPos;}
		void		UseCursor(bool bUseCursor, bool bLockCursorToCenter = false) {m_CursorMgr.UseCursor(bUseCursor,bLockCursorToCenter);}
		void		UpdateCursorState(); // hides or shows cursor based on current game state


	    void		ShowMessageBox(const char* szStringID, MBCreate* pCreate, uint8 nFontSize = 0, bool bDefaultReturn = true);
	    void		ShowMessageBox(const wchar_t *pString, MBCreate* pCreate, uint8 nFontSize = 0, bool bDefaultReturn = true);
	    void		CloseMessageBox(bool bReturn);
		bool		IsMessageBoxVisible() {return m_MessageBox.IsVisible(); }
		CMessageBox* GetMessageBox() { return &m_MessageBox; }


		void		ViewOptions();

		void		AddInterfaceFX(CClientFXLink* pLink, const char *pFXName, LTVector vPos, bool bLoop, bool bSmoothShutdown);
		void		RemoveInterfaceFX();
		void		RemoveInterfaceFX(CClientFXLink	*pLink);

		void		RequestInterfaceSound(InterfaceSound eIS);
		void		ClearInterfaceSound();


		//accessor for the interfaces FX manager
		CClientFXMgr&	GetInterfaceFXMgr()		{ return m_InterfaceFXMgr; }

		// Hide the loading screen.
		// (Note : This does not change out of the loading state, it only stops the loading screen's rendering)
		void	HideLoadScreen();

		// Is the loading screen visible?
		bool	IsLoadScreenVisible() { return m_LoadingScreen.IsVisible() != false; }
		// Update loading screen mission info
		void	UpdateLoadScreenInfo() { m_LoadingScreen.UpdateMissionInfo(); }
		// Set the screen to render on the loading screen
		void	SetLoadingRenderScreen( CBaseScreen *pScreen ) { m_LoadingScreen.SetRenderScreen( pScreen ); }

		CLoadingScreen* GetLoadingScreen() {return &m_LoadingScreen;}
		

		//called to indicate an intentional disconnection from the server
		void	SetIntentionalDisconnect( bool bIntentionalDisconnect ) { m_bIntentionalDisconnect = bIntentionalDisconnect; }
		bool	GetIntentionalDisconnect( ) const { return m_bIntentionalDisconnect; }

		void	LoadFailed( eScreenID eScreenToShow = SCREEN_ID_NONE, const wchar_t* pwszMsg = NULL, bool bShowMessage = true );
		bool	GetLoadFailed() const { return m_bLoadFailed; }

		void	UpdateClientList();

		LTVector GetWorldFromScreenPos(const LTVector2n& pos, float fDepth);
		LTVector GetWorldFromScreenPos(const LTVector2& pos, float fDepth);

		//transforms world space to camera space to screen coordinates
		//return vector's x and y are screen coordinates
		//return vector's z is distance from the camera
		LTVector  GetScreenFromWorldPos(const LTVector& vPos, HOBJECT hCamera, bool& bOnScreen );

		// Handle display timer message.
		bool	HandleDisplayTimerMsg( ILTMessage_Read& msg );

		// Indicates we joined using the "+join" command line and we
		// should take special precautions if we fail.
		bool	GetCommandLineJoin( ) const { return m_bCommandLineJoin; }
		void	SetCommandLineJoin( bool bCommandLineJoin ) { m_bCommandLineJoin = bCommandLineJoin; }

		// Called when a new game is started.
		bool	StartingNewGame( );

		//update the game state when a world has finished loading
		void	UpdatePostLoad();

		bool	HandleChat( const wchar_t* pwsMsg, uint32 nClientID, uint8 nTeam );
		bool	IsChatting( ) const;

		// Call to do the optimizevideomemory at the next opportunity.
		void	DoOptimizeVideoMemory( ) { m_bOptimizeVideoMemory = true; }

		// Get the patch URL.  Can be polled to see if it's valid.
		char const* GetPatchURL( ) const { return m_sPatchUrl.c_str(); }
		bool HasReceivedPatchInfo() const { return ( m_bHasRequestedPatchInfo && !m_pGameSpyPatchInfo ); }
		void AskToPatch();
		bool HasAskedToPatch() const { return m_bHasAskedToPatch; }

		void MutePlayer(uint32 nID);
		void UnmutePlayer(uint32 nID);
		bool IsPlayerMuted(uint32 nID) const;

	protected:
		GameState		m_eGameState;			// Current game state

	private :

		HUSERASSETLIST		m_hInterfaceAssetList;	// the asset list that contains all of our interface resources

		CInterfaceResMgr	m_InterfaceResMgr;		// manages shared resources
		CUserMenuMgr		m_UserMenuMgr;		// Handles menus controlled by user.
		CSystemMenuMgr		m_SystemMenuMgr;	// Handles menus controlled by game systems.
		CProfileMgr			m_ProfileMgr;			// manages player profiles

        CClientInfoMgr  m_ClientInfo;           // Client info mgr
		CCursorMgr		m_CursorMgr;			// Cursor handler (new for Tron/TO2)

		CGameSettings	m_Settings;
		CMessageBox		m_MessageBox;			// Used for simple dialog boxes

		GameState		m_eLastGameState;		// Previous game state

        uint32      m_dwAdvancedOptions;        // Advanced options
        uint32      m_dwOrignallyEnabled;       // Advanced options that were originally enabled

        HLTSOUND    m_hSplashSound;             // Handle to sound played when splash screen is up

		TextureReference	m_hGamePausedTex;			// "Game Paused" message

		bool		m_bUseInterfaceCamera;
		HLOCALOBJ	m_hInterfaceCamera;	// The camera used in the interface

        float		m_fMenuSaveFOVx;            // Fov before entering menu
        float		m_fMenuSaveFOVy;            // Fov before entering menu

        bool		m_bSwitchingModes;          // Switching render modes?

        LTVector2n     m_CursorPos;

		const char*	m_szFailStringId;			// id of the string to display on the mission failed screen

        HLTSOUND    m_hScubaSound;              // sound looping while scuba gear is on

        bool		m_bFadeInitialized;         // Have we initialized everything
        bool		m_bScreenFade;              // Should we fade the screen
		float		m_fScreenFadeAlpha;			// Alpha value of the screen fade
        float		m_fTotalFadeTime;           // How long to do the fade
        float		m_fCurFadeTime;             // Current fade time
        bool		m_bFadeIn;                  // Should we fade in (or out)
		bool		m_bOverrideInitialFade;		// Should the initial fade-in be overriden?

        bool		m_bLetterBox;               // Letter box mode?
		bool		m_bWasLetterBox;			// Was letter box last frame?
		double		m_fLetterBoxFadeEndTime;	// When do we stop fading the letter box in/out
		float		m_fLetterBoxAlpha;			// The current letter box border alpha

		CClientFXMgr		m_InterfaceFXMgr;

		CLoadingScreen m_LoadingScreen;			// The loading screen object/thread
		bool		m_bLoadFailed;
		bool		m_bCommandLineJoin;
		eScreenID	m_eLoadFailedScreen;
		std::wstring m_sLoadFailedMsg;
		bool		m_bShowLoadFailedMessage;

		InterfaceSound	m_eNextSound;

        bool  PreChangeState(GameState eCurState, GameState eNewState, eScreenID screenID);

		bool	m_bSuppressNextFlip;

		bool	m_bIntentionalDisconnect;
		bool	m_bNotifyTeamSizeBalanced;
		bool	m_bNotifyTeamScoreBalanced;
		
protected:
        bool	PrePlayingState(GameState eCurState);
		virtual void UpdatePlayingState();
        bool	PostPlayingState(GameState eNewState);

private:
		virtual bool	HandlePlayerEventMessage(ILTMessage_Read *pMsg);
		virtual void	HandlePlayerScoredMessage(ILTMessage_Read *pMsg);
	
        bool	PreMenuState(GameState eCurState);
		void	UpdateMenuState();
        bool	PostMenuState(GameState eNewState);

        bool	PreLoadingLevelState(GameState eCurState);
		void	UpdateLoadingLevelState();
        bool	PostLoadingLevelState(GameState eNewState);

        bool	PreExitingLevelState(GameState eCurState);
		void	UpdateExitingLevelState();
        bool	PostExitingLevelState(GameState eNewState);

        bool	PrePauseState(GameState eCurState);
		void	UpdatePausedState();
        bool	PostPauseState(GameState eNewState);

        bool	PreSplashScreenState(GameState eCurState);
		void	UpdateSplashScreenState();
		void	EndSplashScreen( );
        bool	PostSplashScreenState(GameState eNewState);

        bool	PreScreenState(GameState eCurState, eScreenID screenID);
		void	UpdateScreenState();
        bool	PostScreenState(GameState eNewState, eScreenID screenID);

        bool	PreDemoScreenState(GameState eCurState);
		void	UpdateDemoScreenState();
        bool	PostDemoScreenState(GameState eNewState);

        bool	PreMovieState(GameState eCurState);
		void	UpdateMovieState();
        bool	PostMovieState(GameState eNewState);

		void	ProcessAdvancedOptions();

		void	UpdateScreenFade(bool bUpdateAlpha = true);
		void	UpdateLetterBox();

		void	CreateInterfaceBackground();
		void	UpdateInterfaceBackground();
		void	RemoveInterfaceBackground();
		void	RemoveAllInterfaceFX();
		void	UpdateInterfaceFX();

		HLTSOUND UpdateInterfaceSound();

		void	HandlePlayerTeamChange();

		void	NextWeapon();
		void	PreviousWeapon();
		void	UpdateWeaponSwitch();

		void	NextDemoScreen();
		bool	m_bQuitAfterDemoScreens;
		bool	m_bSeenDemoScreens;

		void	NextMovie(bool bEndMovies=false);
		HVIDEOTEXTURE	m_hMovie;
		int		m_nCurMovie;

		bool  m_bStartedNew;

		double	m_fLastUpdateRequestTime;

		// true after Init called.  false after Term called.
		bool	m_bInitialized;

		// Used to check if new patches are available.
		IGameSpyPatchInfo*	m_pGameSpyPatchInfo;
		std::string m_sPatchUrl;
		bool	m_bHasRequestedPatchInfo;
		bool	m_bHasAskedToPatch;

		CHUDMgr				m_HUD;					// Heads-Up Display
		CScreenMgr			m_ScreenMgr;
		CPlayerStats		m_stats;				// Player statistics (health, ammo, armor, etc.)

		StopWatchTimer		m_NextWeaponKeyDownTimer;
		StopWatchTimer		m_PrevWeaponKeyDownTimer;
		StopWatchTimer		m_AutoSwitchTimer;

		StopWatchTimer		m_SplashScreenTimer;

		//this is a flag to indicate that video memory should be reset in an optimal manner and should be set once
		// after all objects in the level have been created.
		bool				m_bOptimizeVideoMemory;


		bool				m_bHasCheckedForSoundErrors;

		PlayerIDSet			m_setMutedClients;
};

inline void CInterfaceMgr::ShowMessageBox(const char* szStringID, MBCreate* pCreate, uint8 nFontSize, bool bDefaultReturn)
{
	m_MessageBox.Show(szStringID,pCreate,nFontSize,bDefaultReturn);
}
inline void CInterfaceMgr::ShowMessageBox(const wchar_t *pString, MBCreate* pCreate, uint8 nFontSize, bool bDefaultReturn)
{
	m_MessageBox.Show(pString,pCreate,nFontSize,bDefaultReturn);
}
inline void CInterfaceMgr::CloseMessageBox(bool bReturn)
{
	m_MessageBox.Close(bReturn);
}


#endif // __INTERFACE_MGR_H__
