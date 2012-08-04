// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_CLIENT_SHELL_H__
#define __GAME_CLIENT_SHELL_H__

#include "iclientshell.h"
#include "iltfontmanager.h"
#include "ClientServerShared.h"
#include "SFXMgr.h"
#include "Music.h"
#include "GlobalClientMgr.h"
#include "VarTrack.h"
#include "AttachButeMgr.h"
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

class CSpecialFX;
class CCameraFX;
class CGameTexMgr;
class CInterfaceMgr;
class CPlayerMgr;
class CMissionMgr;
class CClientSaveLoadMgr;
class CClientWeaponAllocator;
class ClientMultiplayerMgr;
class CClientTrackedNodeMgr;
class CPerformanceTest;

class CGameClientShell;
extern CGameClientShell* g_pGameClientShell;


class CGameClientShell : public IClientShellStub
{
public:
	CGameClientShell();
	~CGameClientShell();


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

    float		GetFrameTime()              { return m_fFrameTime; }
    bool		IsGamePaused()			const   { return m_bGamePaused || m_bServerPaused; }
    bool		IsServerPaused()		const   { return m_bServerPaused; }
    bool		IsFirstUpdate()			const   { return m_bFirstUpdate; }

    void		MinimizeMainWindow()            { m_bMainWindowMinimized = true; }
    bool		IsMainWindowMinimized()         { return m_bMainWindowMinimized; }
    void		RestoreMainWindow()             { m_bMainWindowMinimized = false; }


	CMusic*		GetMusic()		{ return &m_Music; }
	void		RestoreMusic();
	void		InitSound();

	void		UpdateGoreSettings();

	void		ClearAllScreenBuffers();

    bool		HasJoystick();
    bool		IsJoystickEnabled();
    bool		EnableJoystick();

    bool		HasGamepad();
    bool		IsGamepadEnabled();
    bool		EnableGamepad();

    void        SetInputState(bool bAllowInput);

	void		CSPrint(char* msg, ...);

	void		BuildClientSaveMsg(ILTMessage_Write *pMsg, SaveDataState eSaveDataState);
	void		UnpackClientSaveMsg(ILTMessage_Read *pMsg);

	void		PreLoadWorld(const char *pWorldName);

	// Mouse Messages
    static void OnChar(HWND hWnd, char c, int rep);
	static void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	static void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	static void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	static void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);

	bool		IsRendererInitted() {return m_bRendererInit;}

	void		SetFarZ(int nFarZ);
	void		ResetDynamicWorldProperties(LTBOOL bUseWorldFog = LTTRUE);

	virtual CInterfaceMgr*		GetInterfaceMgr() { return NULL; }
	virtual CPlayerMgr*			GetPlayerMgr() = 0;
	virtual CClientWeaponAllocator const *GetClientWeaponAllocator() const = 0;

    LTBOOL      IsWorldLoaded()	const			{ return m_bInWorld; }
    void        SetWorldNotLoaded()				{ m_bInWorld = LTFALSE; }
	
	// Are we able to use the radar functionality
	virtual bool	ShouldUseRadar( ) { return false; }

	// Do whatever we need to do after the level loads, but before the first update.
	// This happens while the loading screen is still visible so we're not stuck at 
	// a black screen.
	virtual void	PostLevelLoadFirstUpdate();


	// Called when done with loading and postloading screens.
	void		SendClientLoadedMessage( );

	// Check the server's status on loading the world.
	bool		IsServerLoaded( ) { return ( m_eSwitchingWorldsState == eSwitchingWorldsStateWaitForClient ||
									m_eSwitchingWorldsState == eSwitchingWorldsStateFinished ); }
	SwitchingWorldsState GetSwitchingWorldsState( ) { return m_eSwitchingWorldsState; }

	CClientFXMgr*	GetClientFXMgr() { return &m_ClientFXMgr; }

	bool		LauncherServerApp( char const* pszProfileFile );

protected :
    uint32      OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid);
	void		OnEngineTerm();
    void        OnEvent(uint32 dwEventID, uint32 dwParam);
    LTRESULT    OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos);
    LTRESULT    OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot);
    LTRESULT    OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
	void		PreUpdate();
	void		Update();
	void		PostUpdate();
	void		UpdatePlaying();
	void		OnCommandOn(int command);
	void		OnCommandOff(int command);
	void		OnKeyDown(int key, int rep);
	void		OnKeyUp(int key);
	void		OnObjectRemove(HLOCALOBJ hObj);
    virtual void OnMessage(ILTMessage_Read *pMsg);
	void		OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
	void		OnPlaySound(PlaySoundInfo* pPlaySoundInfo);

    void        RenderCamera(bool bDrawInterface = true);

	void		OnLockRenderer();
	void		OnUnLockRenderer();
private:

	//**************************************************************************
	// settings
	//**************************************************************************
public:
	void			SetDifficulty(GameDifficulty e);
	GameDifficulty  GetDifficulty()					{return m_eDifficulty;}
	GameType		GetGameType()					{return m_eGameType;}
	void			SetGameType(GameType eGameType);
private:


	//**************************************************************************
	//Debug stuff
	//**************************************************************************
public:
	void		ToggleDebugCheat(CheatCode eCheat);
    void        ShowPlayerPos(bool bShow=true)	{ m_bShowPlayerPos = bShow; }
    void        ShowCamPosRot(bool bShow=true)  { m_bShowCamPosRot = bShow; }
	void		DebugWriteCameraPosition();

private:

	//**************************************************************************
	//SFX stuff
	//**************************************************************************
public:
	CSFXMgr*	GetSFXMgr() { return &m_sfxMgr; }
    void        FlashScreen(const LTVector &vFlashColor, const LTVector &vPos, float fFlashRange,
		float fTime, float fRampUp, float fRampDown, bool bForce=false);

    void        HandleWeaponPickup(uint8 nWeaponId, bool bSuccess = true);
    void        HandleGearPickup(uint8 nGearId, bool bSucces = true);
    void        HandleModPickup(uint8 nModId, bool bSucces = true);
    void        HandleAmmoPickup(uint8 nAmmoId, int nAmmoCount, bool bSuccess = true, uint8 nWeaponId = WMGR_INVALID_ID);

	CDamageFXMgr*		GetDamageFXMgr()			{ return &m_DamageFXMgr; }


	void				ClearScreenTint();
    CScreenTintMgr*     GetScreenTintMgr()          { return &m_ScreenTintMgr; }
	CLightScaleMgr*		GetLightScaleMgr()			{ return &m_LightScaleMgr; }

protected :

    void        SpecialEffectNotify(HLOCALOBJ hObj, ILTMessage_Read *pMsg);

private :
	CMissionMgr*			m_pMissionMgr;
	CClientSaveLoadMgr*		m_pClientSaveLoadMgr;	// Same as g_pClientSaveLoadMgr

	CGlobalClientMgr		m_GlobalMgr;		// Contains global mgrs

	CDamageFXMgr			m_DamageFXMgr;		// Same as g_pDamageFxMgr
    bool           m_bFlashScreen;     // Are we tinting the screen for a screen flash
    float          m_fFlashTime;       // Time screen stays at tint color
    float          m_fFlashStart;      // When did the flash start
    float          m_fFlashRampUp;     // Ramp up time
    float          m_fFlashRampDown;   // Ramp down time
    LTVector       m_vFlashColor;	   // Tint color


	GameDifficulty	m_eDifficulty;	   // Difficulty of this game
	GameType		m_eGameType;


// NOTE:  The following data members do not need to be saved / loaded
// when saving games.  Any data members that don't need to be saved
// should be added here (to keep them together)...

 	float     m_fFrameTime;           // Current frame delta

    bool      m_bRestoringGame;       // Are we restoring a saved game

    bool      m_bMainWindowMinimized; // Is the main window minimized?

    bool    m_bTweakingWeapon;		  // Helper, move player-view weapon around
    bool    m_bTweakingWeaponMuzzle;  // Helper, move player-view weapon muzzle around
	bool	m_bTweakingWeaponBreachOffset; // Helper, move player-view weapon breach offset around

	CMusic	m_Music;				// Music helper variable
    bool    m_bGamePaused;          // Is the game paused?
	bool	m_bServerPaused;		// Is server paused?
	SwitchingWorldsState m_eSwitchingWorldsState; // Server's switching world state.
    bool    m_bMainWindowFocus;     // Focus
    bool    m_bRendererInit;        // Has the renderer been initted?


	// Interface stuff...
	CCheatMgr		m_cheatMgr;				// Same as g_pCheatMgr
	CLightScaleMgr	m_LightScaleMgr;		// Class to handle light scale changes
	CScreenTintMgr	m_ScreenTintMgr;


	HLOCALOBJ		m_hBoundingBox;

    bool          m_bFirstUpdate;     // Is this the first update


	// Reverb parameters...

    bool          m_bUseReverb;
	float			m_fReverbLevel;
	float			m_fNextSoundReverbTime;
    LTVector        m_vLastReverbPos;


	// Special FX management...

	CSFXMgr			m_sfxMgr;
	CClientFXMgr	m_ClientFXMgr;		// This handels the Client fx created with FxED


    bool      m_bShowPlayerPos;       // Display player's position.
    bool      m_bShowCamPosRot;       // Display camera's position/rotation.
    bool      m_bAdjustLightScale;    // Adjusting the global light scale
    bool      m_bAdjustLightAdd;      // Adjusting the camera light add
    bool      m_bAdjustFOV;           // Adjusting the FOV
    bool      m_bAdjust1stPersonCamera; // Adjust the 1st person camera offset

	CClientTrackedNodeMgr*	m_pClientTrackedNodeMgr;

	// Private helper functions...

	void	FirstUpdate();
	void	MirrorSConVar(char *pSVarName, char *pCVarName);
	void	Adjust1stPersonCamera();
	void	UpdateWeaponPosition();
	void	UpdateWeaponMuzzlePosition();
	void	UpdateWeaponBreachOffset();
	void	ResetCharacterFXSoundData();

	SOUNDFILTER* GetDynamicSoundFilter();

	void	AdjustLightScale();
	void	AdjustLightAdd();
	void	AdjustFOV();
	void	AdjustMenuPolygrid();
	void	AdjustHeadBob();
	void	UpdateDebugInfo();


	//		Handle OnMessage Type			(message)
	void	HandleMsgChangingLevels			(ILTMessage_Read*);
    void    HandleMsgSFXMessage				(ILTMessage_Read*);
	void	HandleMsgPlayerLevelTransition	(ILTMessage_Read*);
    void    HandleMsgMusic					(ILTMessage_Read*);
    void    HandleMsgPlayerLoadClient		(ILTMessage_Read*);
	void	HandleMsgPlayerSingleplayerInit	(ILTMessage_Read*);
    void    HandleMsgServerError			(ILTMessage_Read*);
    void    HandleMsgBPrint					(ILTMessage_Read*);
	void	HandleMsgDefault				(ILTMessage_Read*);
	void    HandleMsgProjectile             (uint8, ILTMessage_Read*);
	void    HandleMsgPauseGame				(ILTMessage_Read*);
	void    HandleMsgSwitchingWorldState	(ILTMessage_Read*);
	void    HandleMsgMultiplayerOptions	(ILTMessage_Read*);


	void	InitSinglePlayer();


	// Camera helper functions...

	void	UpdateScreenFlash();

	void	CreateBoundingBox();
	void	UpdateBoundingBox();

    bool  UpdateCheats();

	// Speed Hack security monitor
	void	UpdateSpeedMonitor();

	// Debugging variables...

	enum Constants { kMaxDebugStrings = 3 };
	// Debug string screen locations...

	enum DSSL { eDSBottomLeft, eDSBottomRight };
	
	void	SetDebugString(char* strMessage, DSSL eLoc=eDSBottomRight, uint8 nLine=0);
	void	ClearDebugStrings();
	void	RenderDebugStrings();

	CUIPolyString* 	m_pLeftDebugString[kMaxDebugStrings];
	CUIPolyString* 	m_pRightDebugString[kMaxDebugStrings];

	// Contains all multiplayer functionality.
	ClientMultiplayerMgr*	m_pClientMultiplayerMgr;

	// Are we in a world
    bool	m_bInWorld;

	// Set when we get an oncommandon for quicksave.
	bool	m_bQuickSave;

	// Speed Hack security monitor variables
	float				m_fInitialServerTime;
	float				m_fInitialLocalTime;
	uint8				m_nSpeedCheatCounter;

	// Performance testing variables
	bool				m_bRunningPerfTest;
	CPerformanceTest	*m_pPerformanceTest;
};

void DefaultModelHook(ModelHookData *pData, void *pUser);


#endif  // __GAME_CLIENT_SHELL_H__
