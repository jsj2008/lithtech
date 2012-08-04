// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 9/18/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_CLIENT_SHELL_H__
#define __GAME_CLIENT_SHELL_H__

#include "iclientshell.h"
#include "ClientServerShared.h"
#include "SFXMgr.h"
#include "ClientSoundMixer.h"
#include "GlobalClientMgr.h"
#include "VarTrack.h"
#include "SharedMovement.h"
#include "NetDefs.h"
#include "ClientFXMgr.h"
#include "CheatMgr.h"
#include "LightScaleMgr.h"
#include "ScreenTintMgr.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "VersionMgr.h"
#include "ClientSoundMgr.h"
#include "MsgIds.h"
#include "MissionDB.h"
#include "UserNotificationMgr.h"
#include "rendererframestats.h"
#include "ShatterEffectMgr.h"
#include "ltfilewrite.h"
#include <map>
#include "iltgameutil.h"

#if defined(PLATFORM_WIN32)
#include <winsock.h>
#endif // PLATFORM_WIN32

class CSpecialFX;
class CCharacterFX;
class CCameraFX;
class CGameTexMgr;
class CInterfaceMgr;
class CPlayerMgr;
class CMissionMgr;
class CClientSaveLoadMgr;
class ClientConnectionMgr;
class CPerformanceTest;
class CGameModelDecalMgr;

class CGameClientShell;
extern CGameClientShell* g_pGameClientShell;


class CGameClientShell : public IClientShellStub
{
public:
	CGameClientShell();
	~CGameClientShell();

	declare_interface(CGameClientShell);


	//**************************************************************************
	//general game stuff
	//**************************************************************************
public:
	void		OnEnterWorld();
	void		OnExitWorld();

	void		StartPerformanceTest();
	void		StopPerformanceTest();
	void		AbortPerformanceTest(); //cancel out early
	bool		IsRunningPerformanceTest()	 const { return m_bRunningPerfTest; }
	CPerformanceTest* GetLastPerformanceTest() { return m_pPerformanceTest; }

    virtual void PauseGame(bool bPause, bool bPauseSound=false);

    bool		IsGamePaused()			const   { return m_bGamePaused || IsServerPaused(); }
	bool		IsServerPaused()		const   { return SimulationTimer::Instance( ).IsTimerPaused(); }

    void		MinimizeMainWindow()            { m_bMainWindowMinimized = true; }
    bool		IsMainWindowMinimized()         { return m_bMainWindowMinimized; }
    void		RestoreMainWindow()             { m_bMainWindowMinimized = false; }

	CMixer*		GetMixer()		{ return &m_Mixer; }
	void		InitSound();

	void		UpdateGoreSettings();
	
	// Update the server with specific performance settings...
	void		SendPerformanceSettingsToServer( );

    void        SetInputState(bool bAllowInput);

	void		BuildClientSaveMsg(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		UnpackClientSaveMsg(ILTMessage_Read *pMsg);

	void		PreLoadWorld(const char *pWorldName);

#if defined(PLATFORM_WIN32)
	// Mouse Messages
	static void OnChar(HWND hWnd, WPARAM c);
	static void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	static void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	static void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	static void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	static void OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);
#endif // PLATFORM_WIN32

	// Command notification
	void		OnCommandOn(int command);
	void		OnCommandOff(int command);
	void		OnClearAllCommands();

	bool		IsRendererInitted() const {return m_bRendererInit;}
	bool		IsMainWindowFocus() const {return m_bMainWindowFocus;}

	void		SetFarZ(int nFarZ);
	void		ResetDynamicWorldProperties(bool bUseWorldFog = true);

	virtual CInterfaceMgr * GetInterfaceMgr() { return &m_InterfaceMgr;}
	virtual CPlayerMgr * GetPlayerMgr() { return &m_PlayerMgr;}

	CCharacterFX* GetLocalCharacterFX();

	
	bool			IsWorldLoaded()	const		{ return m_bInWorld; }
	void			SetWorldNotLoaded()		{ m_bInWorld = false; }
	

	// Do whatever we need to do after the level loads, but before the first update.
	// This happens while the loading screen is still visible so we're not stuck at 
	// a black screen.
	virtual void	PostLevelLoadFirstUpdate();


	// Check the server's status on loading the world.
	SwitchingWorldsState GetSwitchingWorldsState( ) { return m_eSwitchingWorldsState; }

	CClientFXMgr&	GetSimulationTimeClientFXMgr() { return m_SimulationTimeClientFXMgr; }
	CClientFXMgr&	GetRealTimeClientFXMgr( ) { return m_RealTimeClientFXMgr; }
	CClientFXMgr*	GetRenderTargetClientFXMgr(CRenderTarget* pRenderTarget, bool bUseSimTime);
	void			ShutdownRenderTargetClientFXMgr(CRenderTarget* pRenderTarget);

	//called to reload the effect files and restart all effects. This does nothing during final builds
	bool			ReloadClientFXFiles();

	bool		LauncherServerApp( char const* pszOptionsFile );

	//provides access to the renderer frame stats that are installed on the renderer
	CRendererFrameStats&	GetRendererFrameStats()			{ return m_RendererFrameStats; }

	//called to reset the frame stats associated with the renderer
	void		ResetRendererFrameStats();

	//called to display all the relevant console information for the frame statistics
	void		DisplayFrameStatConsoleInfo(float fRenderTimeMS);

	//called to update and render the user notifiers. This will use the specified time
	//value for the update. This must be called within a begin/end 3d block.
	void		UpdateAndRenderUserNotifiers(float fFrameTime);

	// Updates the state of the dynamic sector list
	void		UpdateDynamicSectorStates();

	//called to handle game side intiialization of the resource manager
	bool		InitResourceMgr();

	//called to handle game side cleanup of the resource manager
	void		TermResourceMgr();

	// Tests for video card compatibility
	bool		TestVideoCardCompatibility();

	// Returns the current server real time in milliseconds.
	uint32		GetServerRealTimeMS( ) const
	{
		return (uint32)((m_fLastServerRealTime + m_swtServerRealTimeAccumulator.GetElapseTime( )) * 1000.0f);
	}

	// performs any processing necessary during world loading progress notifications
	void		OnWorldLoadingProgress();

	// start and stop PunkBuster
	void		InitializePunkBuster();
	void		TerminatePunkBuster();

	// retrieve the PunkBuster client interface
	IPunkBusterClient* GetPunkBusterClient() const { return m_pPunkBusterClient; }

protected :
    uint32      OnEngineInitialized(LTGUID *pAppGuid,RMode *pMode);
	void		OnEngineTerm();
    void        OnEvent(uint32 dwEventID, uint32 dwParam);
    LTRESULT    OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos);
    LTRESULT    OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot);
    LTRESULT    OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
	void		PreUpdate();
	void		Update();
	void		PostUpdate();
	void		UpdatePlaying();
	void		OnKeyDown(int key, int rep);
	void		OnKeyUp(int key);
	void		OnObjectRemove(HLOCALOBJ hObj);
	void		OnObjectAdd( HLOCALOBJ hObject );
	virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);
    virtual void OnMessage(ILTMessage_Read *pMsg);
	void		OnModelKey(HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList *pArgs);

	void		OnGetOcclusionFromPoly(HPOLY *pHPoly, float *occlusionDampening, float *occlusionLFRatio);
	void		OnGetSoundFilterFromPoint(LTVector *pVector, char* pszSoundFilter, int32 nMaxChars);
	void		OnGetSoundFilterParamCountFromName(char* pszSoundFilter, int32* piNumParams);
	void		OnGetSoundFilterParamFromName(char* pszSoundFilter, int32 nIndex, char* pszParamName, int32 nParamLength, float* pfParamValue);

    void        RenderCamera(bool bDrawInterface = true);
	void		DontRenderCamera();
	LTRESULT	CheckForCancel();

private:

	CInterfaceMgr			m_InterfaceMgr;		// Interface manager
	CVersionMgr				m_VersionMgr;		// Same as g_pVersionMgr
	CPlayerMgr				m_PlayerMgr;		// Player manager

	//**************************************************************************
	//SEM interface handling
	//**************************************************************************
public:
	// Calling these functions enter and exit the client shell to maintain the
	// engine interface overlap
	void		EnterClientShell();
	void		ExitClientShell();

	// Convenience class for entering & exiting the client shell based on scope
	// This is generally only intended for internal use, but may be needed for
	// use callbacks that don't originate from the client shell
	class CClientShellScopeTracker
	{
	public:
		CClientShellScopeTracker()
		{
			if (g_pGameClientShell)
				g_pGameClientShell->EnterClientShell();
		}
		~CClientShellScopeTracker()
		{
			if (g_pGameClientShell)
				g_pGameClientShell->ExitClientShell();
		}
	};

	//**************************************************************************
	// settings
	//**************************************************************************
public:
	void			SetDifficulty(GameDifficulty e);
	GameDifficulty  GetDifficulty()					{return m_eDifficulty;}
private:


	//**************************************************************************
	//Debug stuff
	//**************************************************************************
public:
	void		  ToggleDebugCheat(CheatCode eCheat);
	void	   	  DebugWriteCameraPosition();

	CLTFileWrite& GetSimulationLogFile() { return m_cSimulationLogFile; }

private:

	//**************************************************************************
	//SFX stuff
	//**************************************************************************
public:
	CSFXMgr*	GetSFXMgr() { return &m_sfxMgr; }
    void        FlashScreen(const LTVector &vFlashColor, const LTVector &vPos, float fFlashRange,
		float fTime, float fRampUp, float fRampDown, bool bForce=false);


	void				ClearScreenTint();
    CScreenTintMgr*     GetScreenTintMgr()          { return &m_ScreenTintMgr; }
	CLightScaleMgr*		GetLightScaleMgr()			{ return &m_LightScaleMgr; }

    void        SpecialEffectNotify(HLOCALOBJ hObj, ILTMessage_Read *pMsg);

	void	PunkBusterProcessCommand(char *cmd);

private :
	CMissionMgr*			m_pMissionMgr;
	CClientSaveLoadMgr*		m_pClientSaveLoadMgr;	// Same as g_pClientSaveLoadMgr

	CGameModelDecalMgr*		m_pModelDecalMgr;	// Model decal manager

	CGlobalClientMgr		m_GlobalMgr;		// Contains global mgrs

	GameDifficulty	m_eDifficulty;	   // Difficulty of this game

// NOTE:  The following data members do not need to be saved / loaded
// when saving games.  Any data members that don't need to be saved
// should be added here (to keep them together)...

    bool      m_bRestoringGame;       // Are we restoring a saved game
    bool      m_bMainWindowMinimized; // Is the main window minimized?
    bool    m_bTweakingWeapon;		  // Helper, move player-view weapon around
   
	CMixer	m_Mixer;				// Sound mixer helper variable
    bool    m_bGamePaused;          // Is the game paused?
	
	SwitchingWorldsState m_eSwitchingWorldsState; // Server's switching world state.
	
    bool    m_bMainWindowFocus;     // Focus
    bool    m_bRendererInit;        // Has the renderer been initted?


	// Interface stuff...
	CCheatMgr		m_cheatMgr;				// Same as g_pCheatMgr
	CLightScaleMgr	m_LightScaleMgr;		// Class to handle light scale changes
	CScreenTintMgr	m_ScreenTintMgr;

	bool          m_bFirstUpdate;			// Is this the first update
	bool          m_bFirstPlayingUpdate;     // Is this the first update where we're playing.

	CUserNotificationMgr m_UserNotificationMgr;		// Manager that handles notifications to the user in the form of icons
	CRendererFrameStats	 m_RendererFrameStats;		// The frame stats installed on the renderer and receives all rendering information


	// Reverb parameters...

    bool          m_bUseReverb;
	float			m_fReverbLevel;
	float			m_fNextSoundReverbTime;
    LTVector        m_vLastReverbPos;


	// Special FX management...

	CSFXMgr				m_sfxMgr;
	CClientFXMgr		m_SimulationTimeClientFXMgr;			// This handles the Client fx created with FXEdit and uses simulation time.
	CClientFXMgr		m_RealTimeClientFXMgr;	// This handles the Client fx created with FXEdit and uses real time.
	CShatterEffectMgr	m_ShatterEffectMgr;		// Handles all shattering effect like breaking glass, etc.

	// List of ClientFXMgrs to manage ClientFX on a per-RenderTarget basis.
	typedef std::pair<CRenderTarget*,bool> RenderTargetClientFXKey;
	typedef std::map<RenderTargetClientFXKey,CClientFXMgr*> RenderTargetClientFXMgrMap;
	RenderTargetClientFXMgrMap m_RenderTargetClientFXMgrs;
	void	ShutdownAllRenderTargetClientFXMgrs();

	bool			m_bAdjust1stPersonCamera;	// Adjust the 1st person camera offset

	// Private helper functions...

	void	FirstUpdate();
	void	FirstPlayingUpdate();
	void	MirrorSConVar(char *pSVarName, char *pCVarName);
	void	Adjust1stPersonCamera();
	void	UpdateWeaponPosition();
	void	ResetCharacterFXSoundData();

	void	AdjustMenuPolygrid();
	void	AdjustHeadBob();

	//called to initialize the rendering layers used by the game code
	void	InitRenderingLayers();


	//		Handle OnMessage Type			(message)
	void	HandleMsgChangingLevels			(ILTMessage_Read*);
	void    HandleMsgSFXMessage				(ILTMessage_Read*);
	void    HandleMsgSFXMessageOverride		(ILTMessage_Read*);
	void    HandleMsgApplyDecal				(ILTMessage_Read*);
	void	HandleMsgPlayerLevelTransition	(ILTMessage_Read*);
    void    HandleMsgPlayerLoadClient		(ILTMessage_Read*);
	void	HandleMsgPlayerSingleplayerInit	(ILTMessage_Read*);
	void    HandleMsgPauseGame				(ILTMessage_Read*);
	void    HandleMsgSwitchingWorldState	(ILTMessage_Read*);
	void    HandleMsgMultiplayerOptions		(ILTMessage_Read*);
	void	HandleMsgShatterWorldModel		(ILTMessage_Read*);
	void	HandleMsgDynAnimProp			(ILTMessage_Read*);
	void	HandleMsgDynamicSector			(ILTMessage_Read*);
	void	HandleMsgSimulationTimerScale	(ILTMessage_Read*);
	void    HandleMsgMixer					(ILTMessage_Read*);
	void    HandleMsgSoundMisc				(ILTMessage_Read*);
	void    HandleMsgSoundFilter			(ILTMessage_Read*);
	void	HandleMsgWeaponFireFX			(ILTMessage_Read*);
	void	HandleMsgWeaponDamagePlayer		(ILTMessage_Read*);
	void	HandleMsgSoundBroadcastDB		(ILTMessage_Read*);
	void	HandleInstantNavMarker			(ILTMessage_Read*);
	void	HandleMsgDoDamage				(ILTMessage_Read*);
	void	HandleMsgLadder					(ILTMessage_Read*);
	void	HandleMsgPunkBuster				(ILTMessage_Read*);

	void	InitSinglePlayer();


	// Camera helper functions...

	void	UpdateScreenFlash();

    bool  UpdateCheats();

	// Speed Hack security monitor
	void	UpdateSpeedMonitor();

	// PunkBuster callbacks
	static void	PunkBusterGetServerAddressCallback(char* pszServerAddressBuffer, int nServerAddressBufferLength);
	static void PunkBusterGetGameVersionCallback(char* pszGameVersionBuffer, int nGameVersionBufferLength);
	static void	PunkBusterGetSocketCallback(SOCKET *sock);
	static void	PunkBusterSendGameMessageCallback(char *data, int datalen);
	static void	PunkBusterDisplayMessageCallback(char *data, int datalen, int skipnotify);
	static void	PunkBusterGetMapNameCallback(char *data, int datalen);
	static void PunkBusterGetServerHostNameCallback(char *data, int datlen);
	static void	PunkBusterGetPlayerNameCallback(char *data, int datalen);
	static void PunkBusterGetGameNameCallback(char *data, int datalen);
	static void PunkBusterGetConsoleVariableCallback(const char* pszName, char* pszValueBuffer, int nValueBufferLength);

	// Contains all multiplayer functionality.
	ClientConnectionMgr*	m_pClientMultiplayerMgr;

	// PunkBuster interface
	IPunkBusterClient*  m_pPunkBusterClient;

	// the last time we sent a keep alive message
	uint32 m_nLastKeepAliveTime;

	// Are we in a world
    bool	m_bInWorld;

	// Set when we get an oncommandon for quicksave.
	bool	m_bQuickSave;

	// Speed Hack security monitor variables
	double				m_fInitialServerTime;
	double				m_fInitialLocalTime;
	uint8				m_nSpeedCheatCounter;

	// Performance testing variables
	bool				m_bRunningPerfTest;
	CPerformanceTest	*m_pPerformanceTest;

	// Dynamic sector handling
	// This list handles the situation where we get a dynamic sector request before
	// we've actually loaded the world.
	typedef std::vector<std::pair<uint32, bool>, LTAllocator<std::pair<uint32, bool>, LT_MEM_TYPE_CLIENTSHELL> > TDynamicSectorList;
	TDynamicSectorList	m_aDynamicSectorStateBuffer;
	
	//did we launch using resource streaming?
	bool				m_bResourceStreaming;

	bool				m_bLaunchedServerApp;

	// simulation log file
	CLTFileWrite m_cSimulationLogFile;

	// The recursion count for tracking the client shell scope
	uint32				m_nEntryCount;
	// The ILTCSBase interface that should be used outside of the current client scope
	ILTCSBase *			m_pExternalScopeBaseInterface;

	// Server synchronized timer...
	StopWatchTimer		m_swtServerRealTimeAccumulator;
	double				m_fLastServerRealTime;

};

#endif  // __GAME_CLIENT_SHELL_H__
