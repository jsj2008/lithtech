
/****************************************************************************
;
;	 MODULE:		NetStart (.CPP)
;
;	PURPOSE:		Network game start/join/host dialog code
;
;	HISTORY:		06/28/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "Windows.h"
#include <stdio.h>
#include "NetStart.h"
#include "NetInfo.h"
#include "IpMgr.h"
#include "Sparam.h"
#include "ConfigMgr.h"
#include "dplobby.h"
#include "..\Shared\ClientRes.h"
#include <mbstring.h>


// Defines...

#define LASTHOSTPORT_VARNAME		"LastHostPort"

#define	DLG_NEXT			1
#define DLG_CANCEL			2
#define DLG_BACK			3
#define DLG_ERROR			4
							
#define RETAIL_LEVEL		0
#define CUSTOM_LEVEL		1
#define ADDON_LEVEL			2
							
#define NET_JOIN			0
#define NET_HOST			1
							
#define CONNECT_FULL		0
#define CONNECT_TCPIP		1
#define CONNECT_TCPIP_QUICK	2
#define CONNECT_TCPIP_HOST	3

#define HIGHPING_UPDATERATE	3
#define MODEM_UPDATERATE	5
#define ISDN_UPDATERATE		9
#define LAN_UPDATERATE		18
#define DEFAULT_UPDATERATE	MODEM_UPDATERATE

#define MAX_NETSTART_SERVICES	16

#define HOURS_PER_SEC		0.00027f

#define TIMER_UPDATEQUERY	2
#define TIMER_UPDATEDIALOG	3	


// Macros...

#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)
#define WasKeyDown(key) (GetAsyncKeyState(key) & 0x00000001)
#define WisKeyDown(key) (GetAsyncKeyState(key) & 0x80000001)


// Globals...

ServerOptions		g_ServerOptions;


// Statics...

// Used for the tcp/ip address dialog.
static char *g_DlgIP;
static DWORD g_DlgIPLen;

static	CClientDE*		s_pClient        = NULL;
static	NetStart*		s_pNetStart      = NULL; 
static	ServerOptions*	s_pServerOptions = NULL;
static	HWND			s_hMainWnd       = NULL;
static	HINSTANCE		s_hInst          = NULL;
static	CNinfoMgr		s_NinfoMgr;
static	CConfigMgr		s_ConfigMgr;
static	CIpMgr			s_IpMgr;

static	HNETSERVICE		g_ServiceHandles[MAX_NETSTART_SERVICES];
static	DDWORD			g_ServiceFlags[MAX_NETSTART_SERVICES];
static	HNETSERVICE		g_SelectedService=0;

static  DDWORD			g_HostPort=0;
static	char			s_sSessionName[128]     = { "" };
static	char			s_sPlayerName[32]       = { "" };
static	char			s_sFirstLevel[128]      = { "" };
static	char			s_sPlayerInfoNotSent[80]= { "(player info not sent)" };
static	char			s_sNetConfigFile[128]   = { "Caleb.b2c" };
static	char			s_sHighPing[50]         = { "High-Ping/Modem" };
static	char			s_sModem[50]            = { "28.8k-56k Modem" };
static	char			s_sIsdn[50]             = { "ISDN-T1 Internet" };
static	char			s_sLan[50]              = { "LAN Game" };
static	char			s_sCustom[50]           = { "Custom" };
static	char			s_sDefaultSessionName[128] = { "" };

static	int				s_nGameType				= 0;
static	int				s_nEndType				= 0;
static	int				s_nEndFrags				= 0;
static	int				s_nEndTime				= 0;
static	int				s_nPlayerColor			= 0;
static	int				s_nMech					= 0;
static	int				s_nNetService			= 0;
static	int				s_nNetJoinHost			= 0;
static	int				s_nNetLatency			= DEFAULT_UPDATERATE;
static	int				s_nMaxPlayers			= MAX_MULTI_PLAYERS_DISPLAY;
static	int				s_nFpsLimit				= 30;
static	int				s_nErrorString			= IDS_NETERR_GENERIC;
static	int				s_nConnectType			= CONNECT_FULL;
static	int				s_nNetAmmoLevel			= LEVEL_NORMAL;
static	int				s_nNetAmmoRespawn		= 15;
static	int				s_nNetArmorLevel		= LEVEL_NORMAL;
static	int				s_nNetArmorRespawn		= 45;
static	int				s_nNetHealthLevel		= LEVEL_NORMAL;
static	int				s_nNetHealthRespawn		= 30;
static	int				s_nNetPowerupsLevel		= LEVEL_NORMAL;
static	int				s_nNetPowerupsRespawn	= 60;
static	int				s_nNetHealingRate       = 0;
static	int				s_nNetTeam              = 0;
static	int				s_nNumJoinRetries       = 0;
static	int				s_nInternalJoinHost     = NET_UNKNOWN;
static	int				s_nNetFlagValue			= 5;
static	int				s_nNetGoalValue         = 5;
static	int				s_nNetSocBallSkin       = 0;

static	BOOL			s_bLimitFps                  = FALSE;
static	BOOL			s_bSaveLevels                = TRUE;
static	BOOL			s_bLobbyLaunch               = FALSE;
static	BOOL			s_bDontDisplayNoSessions     = FALSE;
static	BOOL			s_bDontDisplaySettingsDialog = FALSE;
static	BOOL			s_bHaveSettingInfo           = FALSE;
static	BOOL			s_bNetFallDamage             = FALSE;
static	BOOL			s_bNetFriendlyFire			 = TRUE;
static	BOOL			s_bNetNegTeamFrags			 = TRUE;
static	BOOL			s_bNetOnlyFlagScores         = FALSE;
static	BOOL			s_bNetUseTeamSize			 = FALSE;
static	BOOL			s_bNetOnlyGoalScores         = FALSE;

static	NetPlayer		s_NetPlayer;
static	NetGame			s_NetGame;
static	NetClientData	s_NetClientData;
static	RMode			s_RMode;
static	NetSession*		s_pSessionList = NULL;


// Externs...

extern HINSTANCE g_hLTDLLInstance;
extern DBOOL	 g_bIsHost;


// Macros...

#ifdef _DEBUG
#define	DEBUG_MSG(s)	MessageBox(NULL, s, "Blood2 Debug", MB_OK);
#define DEBUG_NUM(s, n)	{ char sBuf[128]; wsprintf(sBuf, "%s = %i", s, n); DEBUG_MSG(sBuf); }
#else
#define DEBUG_MSG(s)	/* */
#define DEBUG_NUM(s, n)	/* */
#endif

#define SET_NODRAW(hWnd)	SendMessage(hWnd, WM_SETREDRAW, 0, 0);
#define SET_REDRAW(hWnd)	SendMessage(hWnd, WM_SETREDRAW, 1, 0); InvalidateRect(hWnd, NULL, FALSE); UpdateWindow(hWnd);


// Prototypes...

BOOL NetStart_HandleDefaultDialogCommands(HWND hDlg, WPARAM wParam);
void NetStart_DisplayError(HINSTANCE hInst);

BOOL NetStart_FillServiceList(HWND hList);
BOOL NetStart_SelectCurrentService(HWND hList);

BOOL NetStart_SelectJoinHost(HWND hDlg);
BOOL NetStart_SelectNetSpeed(HWND hComboBox);

int  NetStart_FillSessionList(HWND hDlg);
int  NetStart_FillSessionListTcpIp(HWND hDlg);
BOOL NetStart_JoinCurrentSession(HWND hList);
BOOL NetStart_HostSession1(HWND hDlg);
BOOL NetStart_HostSession2(HWND hDlg);
BOOL NetStart_DisplaySelectedSessionInfo(HWND hDlg);
BOOL NetStart_ClearSelectedSessionInfo(HWND hDlg);

BOOL NetStart_FillLevelList(HWND hList, char* sDir, int nData, BOOL bClearList = TRUE);
BOOL NetStart_FillPlayerStruct(HWND hDlg, BOOL bUseComboConfig = FALSE);
BOOL NetStart_FillGameStruct(HWND hDlg);
BOOL NetStart_FillGameLevels(HWND hDlg);
BOOL NetStart_FillGameEnd(HWND hDlg);
BOOL NetStart_FillGameItems(HWND hDlg);
BOOL NetStart_FillGameDamage(HWND hDlg);
BOOL NetStart_FillGameType(HWND hDlg);

BOOL NetStart_FillNetSpeed(HWND hDlg);
BOOL NetStart_SetEndGameInfo(HWND hDlg);

void NetStart_InitFpsLimit(HWND hDlg);
void NetStart_FillFpsLimit(HWND hDlg);

int  NetStart_GetConnectCommands(CClientDE* pClient, NetStart* pNetStart);
BOOL NetStart_DoTcpIpDialog(HINSTANCE hInst, HWND hParentWnd);

BOOL NetStart_InitOptions(HWND hDlg);
BOOL NetStart_FillOptions(HWND hDlg);

BOOL NetStart_InitTeamOptions(HWND hDlg);
BOOL NetStart_FillTeamOptions(HWND hDlg);

BOOL NetStart_InitCtfOptions(HWND hDlg);
BOOL NetStart_FillCtfOptions(HWND hDlg);

BOOL NetStart_InitSoccerOptions(HWND hDlg);
BOOL NetStart_FillSoccerOptions(HWND hDlg);

BOOL NetStart_InitPlayer(HWND hDlg);
BOOL NetStart_InitGameItems(HWND hDlg);
BOOL NetStart_InitGameDamage(HWND hDlg);
BOOL NetStart_InitConfigCombo(HWND hDlg);
BOOL NetStart_InitGameType(HWND hDlg);

BOOL NetStart_HostTcpIp(CClientDE* pClientDE);

BOOL NetStart_IsOkToDisplayLevel(char* sLevel);

BOOL CALLBACK NetDlg_Welcome(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Services(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_JoinHost(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Sessions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_SessionsTcpIp(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Player(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Game(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_GameLevels(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Finished(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_NoSessions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Options(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_TeamOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_CtfOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_SoccerOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Settings(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_GameItems(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_GameDamage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ForceActiveFocus(HWND hWnd);

int    GetConsoleInt(char* sKey, int nDefault);
void   GetConsoleString(char* sKey, char* sDest, char* sDefault);
void   WriteConsoleString(char* sKey, char* sValue);
void   WriteConsoleInt(char* sKey, int nValue);
DFLOAT GetConsoleFloat(char* sKey, DFLOAT fDefault);
void   WriteConsoleFloat(char* sKey, DFLOAT fValue);

BOOL AddSelToList(HWND hSrcList, HWND hDestList);
BOOL RemoveSelFromList(HWND hList);
void UpdateGameLevelControls(HWND hDlg);

int  AddComboInfo(HWND hComboBox, char* sText, DWORD dwValue);
int	 DoNoSessionsMessage(HWND hParentWnd);

void FixAddress(char* sAddress);

BOOL  DoOptionHelp(HWND hParentWnd, int iMsg, int iTitle);
void  SetDlgItemFloat(HWND hDlg, int iCtrl, float fValue);
float GetDlgItemFloat(HWND hDlg, int iCtrl);

void SetItemLevel(HWND hDlg, UINT idControl, int nLevel);
int  GetItemLevel(HWND hDlg, UINT idControl);
void SetItemRespawn(HWND hDlg, UINT idControl, int nRespawn);
int  GetItemRespawn(HWND hDlg, UINT idControl);
void SetHealingRate(HWND hDlg, UINT idControl, int nLevel);
int  GetHealingRate(HWND hDlg, UINT idControl);

void PostProcessName(char* sName);


// Functions...

// ----------------------------------------------------------------------- //
// Return the flags for the currently selected service.
// ----------------------------------------------------------------------- //
DWORD GetSelectedServiceFlags()
{
	DWORD i;

	for(i=0; i < MAX_NETSTART_SERVICES; i++)
	{
		if(g_ServiceHandles[i] == g_SelectedService)
		{
			return g_ServiceFlags[i];
		}
	}
	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoWizard
//
//	PURPOSE:	Starts the multiplayer wizard
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_DoWizard(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Let the main function do the real wizard work...

	NetStart ns;

	DBOOL bRet = NetStart_DoWizard(pClientDE, &ns, NET_UNKNOWN);
	if (!bRet) return(DFALSE);


	// Start the game as requested...

	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	if (ns.m_bHost)
	{
		req.m_Type = STARTGAME_HOST;

		strcpy(req.m_WorldName, ns.m_sLevel);
		memcpy(&req.m_HostInfo, &ns.m_NetHost, sizeof(NetHost));

		g_bIsHost = DTRUE;
	}
	else
	{
		req.m_Type        = STARTGAME_CLIENT;
		req.m_pNetSession = ns.m_pNetSession;

		if (ns.m_bHaveTcpIp)
		{
			req.m_Type = STARTGAME_CLIENTTCP;
			strcpy(req.m_TCPAddress, ns.m_sAddress);
		}

		g_bIsHost = DFALSE;
	}

	req.m_pGameInfo     = NetStart_GetGameStruct();
	req.m_GameInfoLen   = sizeof(NetGame_t);
	req.m_pClientData   = NetStart_GetClientDataStruct();
	req.m_ClientDataLen = sizeof(NetClientData_t);

	DRESULT dr = pClientDE->StartGame(&req);

	if (dr != LT_OK && req.m_Type == STARTGAME_CLIENTTCP)
	{
		while (dr != LT_OK && s_nNumJoinRetries > 0)
		{
			s_nNumJoinRetries--;
			Sleep(1000);
			dr = pClientDE->StartGame(&req);
		}
	}

	NetStart_FreeSessionList(pClientDE);

	if (dr == LT_OK)
	{
		NetStart_RunServerOptions(pClientDE, s_pServerOptions);
	}
	else
	{
		HINSTANCE hInst;
		void* hModule;
		g_pClientDE->GetEngineHook("cres_hinstance",&hModule);
		hInst = (HINSTANCE)hModule;
		if (!hInst) return (DFALSE);
		
		if(dr == LT_CANTBINDTOPORT)
			s_nErrorString = IDS_NETERR_CANTBINDTOPORT;
		else if(dr == LT_REJECTED)
			s_nErrorString = IDS_NETERR_CONNECTIONREJECTED;
		else if (dr == LT_NOTSAMEGUID)
			s_nErrorString = IDS_NETERR_NOTSAMEGUID;
		else
			s_nErrorString = IDS_NETERR_GENERIC;
		
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoWizard
//
//	PURPOSE:	Starts the multiplayer wizard
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_DoWizard(CClientDE* pClient, NetStart* pNetStart, int nJoinHost)
{
	// Sanity checks...

	if (!pClient) return(DFALSE);
	if (!pNetStart) return(DFALSE);


	// Set our static pointers...

	s_pClient        = pClient;
	s_pNetStart      = pNetStart;
	s_pServerOptions = &g_ServerOptions;

	s_bLobbyLaunch      = FALSE;
	s_nInternalJoinHost = nJoinHost;


	// Init the net game info manager...

	s_NinfoMgr.Term();
	s_NinfoMgr.Init(s_pClient);


	// Clear the NetStart structure...

	memset(s_pNetStart, 0, sizeof(NetStart));


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));
	memset(&s_NetClientData, 0, sizeof(NetClientData));


	// Fill in the default server options...

	memset(s_pServerOptions, 0, sizeof(ServerOptions));

	s_pServerOptions->m_fWorldTimeSpeed = -1.0f;
	s_pServerOptions->m_fRunSpeed       = 1.3f;
	s_pServerOptions->m_fMissileSpeed   = 1.0;

	strcpy(s_pServerOptions->m_sWorldNightColor, "0.5 0.5 0.5");


	// Get the module handle...

	HINSTANCE hInst;
	void* hModule;
	g_pClientDE->GetEngineHook("cres_hinstance",&hModule);
	hInst = (HINSTANCE)hModule;
	if (!hInst) return(NULL);

	s_hInst = hInst;


	// Init the engine's networking...

	DRESULT dr = pClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Set some default values...

	LoadString(hInst, IDS_NETWIZ_DEFAULTSESSIONNAME, s_sDefaultSessionName, 120);
	LoadString(hInst, IDS_NETWIZ_DEFAULTSESSIONNAME, s_sSessionName, 120);
	LoadString(hInst, IDS_NETWIZ_DEFAULTPLAYERNAME, s_sPlayerName, 30);
	LoadString(hInst, IDS_NETWIZ_PLAYERINFONOTSENT, s_sPlayerInfoNotSent, 70);

	LoadString(hInst, IDS_NETWIZ_MODEM, s_sModem, 50);
	LoadString(hInst, IDS_NETWIZ_ISDN, s_sIsdn, 50);
	LoadString(hInst, IDS_NETWIZ_LAN, s_sLan, 50);
	LoadString(hInst, IDS_NETWIZ_CUSTOM, s_sCustom, 50);

#ifdef _DEMO
	LoadString(hInst, IDS_NETWIZ_DEFAULTDEMOSESSIONNAME, s_sSessionName, 120);
	LoadString(hInst, IDS_NETWIZ_DEFAULTDEMOSESSIONNAME, s_sDefaultSessionName, 120);
#endif


	// Read some default values from the console...

	GetConsoleString("NetConfigFile", s_sNetConfigFile, s_sNetConfigFile);
	GetConsoleString("NetSessionName", s_sSessionName, s_sSessionName);
	GetConsoleString("Name", s_sPlayerName, "");

	if (s_sPlayerName[0] == '\0')
	{
		LoadString(hInst, IDS_NETWIZ_DEFAULTPLAYERNAME, s_sPlayerName, 30);
		GetConsoleString("NetPlayerName", s_sPlayerName, s_sPlayerName);
	}

	PostProcessName(s_sPlayerName);


	GetConsoleString("NetSessionName", s_sSessionName, s_sSessionName);

	s_nGameType    = GetConsoleInt("NetGameType", NGT_DEATHMATCH);
	s_nEndType     = GetConsoleInt("NetGameEnd", NGE_FRAGS);
	s_nEndFrags    = GetConsoleInt("NetEndFrags", 25);
	s_nEndTime     = GetConsoleInt("NetEndTime", 15);
	s_nNetService  = GetConsoleInt("NetService", 0);
	s_nNetJoinHost = GetConsoleInt("NetJoinHost", 0);
	s_nNetLatency  = GetConsoleInt("UpdateRate", DEFAULT_UPDATERATE);
	s_nMaxPlayers  = GetConsoleInt("NetMaxPlayers", MAX_MULTI_PLAYERS_DISPLAY);
	s_nFpsLimit    = GetConsoleInt("NetFpsLimit", s_nFpsLimit);
	s_bLimitFps    = GetConsoleInt("NetLimitFps", s_bLimitFps);
	s_bSaveLevels  = GetConsoleInt("NetSaveLevels", s_bSaveLevels);

	s_nNumJoinRetries            = GetConsoleInt("NetJoinRetry", s_nNumJoinRetries);
	s_bDontDisplayNoSessions     = GetConsoleInt("DontDisplayNoSessions", 0);
	s_bDontDisplaySettingsDialog = GetConsoleInt("DontDisplaySettingsDialog", 0);
	s_bHaveSettingInfo           = GetConsoleInt("HaveSettingInfo", 0);

	s_nNetAmmoLevel		  = GetConsoleInt("NetAmmoLevel", s_nNetAmmoLevel);
	s_nNetAmmoRespawn	  = GetConsoleInt("NetAmmoRespawn", s_nNetAmmoRespawn);
	s_nNetArmorLevel	  = GetConsoleInt("NetArmorLevel", s_nNetArmorLevel);
	s_nNetArmorRespawn	  = GetConsoleInt("NetArmorRespawn", s_nNetArmorRespawn);
	s_nNetHealthLevel	  = GetConsoleInt("NetHealthLevel", s_nNetHealthLevel);
	s_nNetHealthRespawn	  = GetConsoleInt("NetHealthRespawn", s_nNetHealthRespawn);
	s_nNetPowerupsLevel	  = GetConsoleInt("NetPowerupsLevel", s_nNetPowerupsLevel);
	s_nNetPowerupsRespawn = GetConsoleInt("NetPowerupsRespawn", s_nNetPowerupsRespawn);
	s_nNetHealingRate     = GetConsoleInt("NetHealingRate", s_nNetHealingRate);
	s_nNetTeam            = GetConsoleInt("NetTeam", s_nNetTeam);
	s_bNetFallDamage      = GetConsoleInt("NetFallDamage", s_bNetFallDamage);
	s_bNetFriendlyFire    = GetConsoleInt("NetFriendlyFire", s_bNetFriendlyFire);
	s_bNetNegTeamFrags    = GetConsoleInt("NetNegTeamFrags", s_bNetNegTeamFrags);
	s_bNetOnlyFlagScores  = GetConsoleInt("NetOnlyFlagScores", s_bNetOnlyFlagScores);
	s_bNetOnlyGoalScores  = GetConsoleInt("NetOnlyGoalScores", s_bNetOnlyGoalScores);
	s_bNetUseTeamSize     = GetConsoleInt("NetUseTeamSize", s_bNetUseTeamSize);
	s_nNetFlagValue		  = GetConsoleInt("NetFlagValue", s_nNetFlagValue);
	s_nNetGoalValue		  = GetConsoleInt("NetGoalValue", s_nNetGoalValue);
	s_nNetSocBallSkin	  = GetConsoleInt("NetSocBallSkin", s_nNetSocBallSkin);

	s_pServerOptions->m_fWorldTimeSpeed = GetConsoleFloat("NetWorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
	s_pServerOptions->m_fRunSpeed       = GetConsoleFloat("NetRunSpeed", s_pServerOptions->m_fRunSpeed);
	s_pServerOptions->m_fMissileSpeed   = GetConsoleFloat("NetMissileSpeed", s_pServerOptions->m_fMissileSpeed);

	GetConsoleString("NetWorldNightColor", s_pServerOptions->m_sWorldNightColor, s_pServerOptions->m_sWorldNightColor);

#ifndef _ADDON
	if (s_nGameType == NGT_SOCCER) s_nGameType = NGT_DEATHMATCH;
#endif


	// Declare local variables we'll be using...

	int		nRet;
	HWND	hParentWnd = NULL;


	// Check if we were given connect info on the command line...

	s_nConnectType = NetStart_GetConnectCommands(pClient, pNetStart);

	if (s_nConnectType == CONNECT_TCPIP_HOST)
	{
		if (!NetStart_HostTcpIp(pClient)) return(FALSE);
		s_pNetStart->m_bHaveTcpIp = FALSE;
		goto PlayerDlg;
	}
	else if (s_nConnectType == CONNECT_TCPIP || s_nConnectType == CONNECT_TCPIP_QUICK)
	{
		if (!NetStart_DoSettingsDialog())
		{
			return(DFALSE);
		}

		goto FinishedDlg;
	}


	// Do the services dialog...

ServicesDlg:
	nRet = DialogBox(hInst, "NET_SERVICES", hParentWnd, (DLGPROC)NetDlg_Services);

	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the player dialog...

PlayerDlg:
	nRet = DialogBox(hInst, "NET_PLAYER", hParentWnd, (DLGPROC)NetDlg_Player);

	if (nRet == DLG_BACK) goto ServicesDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}

	if (s_pNetStart->m_bHost) goto GameDlg;
	if (s_pNetStart->m_bHaveTcpIp) goto FinishedDlg;


	// Do the sessions dialog...

	if (GetSelectedServiceFlags() & NETSERVICE_TCPIP)
	{
		nRet = DialogBox(hInst, "NET_SESSIONS_TCPIP", hParentWnd, (DLGPROC)NetDlg_SessionsTcpIp);
	}
	else
	{
		nRet = DialogBox(hInst, "NET_SESSIONS", hParentWnd, (DLGPROC)NetDlg_Sessions);
	}

	s_IpMgr.WriteIps();

	if (nRet == DLG_BACK) goto PlayerDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}

	goto FinishedDlg;


	// Do the game dialog (host only)...

GameDlg:

#ifdef _ADDON
	nRet = DialogBox(hInst, "NET_GAME_AO", hParentWnd, (DLGPROC)NetDlg_Game);
#else
	nRet = DialogBox(hInst, "NET_GAME", hParentWnd, (DLGPROC)NetDlg_Game);
#endif

	if (nRet == DLG_BACK) goto PlayerDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// If necessary, do the team options dialog...

TeamOptionsDlg:
	if (s_nGameType != NGT_TEAMS) goto CtfOptionsDlg;

	nRet = DialogBox(hInst, "NET_OPTIONS_TEAMS", hParentWnd, (DLGPROC)NetDlg_TeamOptions);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// If necessary, do the ctf options dialog...

CtfOptionsDlg:
	if (s_nGameType != NGT_CAPTUREFLAG) goto SoccerOptionsDlg;

	nRet = DialogBox(hInst, "NET_OPTIONS_CTF", hParentWnd, (DLGPROC)NetDlg_CtfOptions);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// If necessary, do the soccer options dialog...

SoccerOptionsDlg:
	if (s_nGameType != NGT_SOCCER) goto OptionsDlg;

	nRet = DialogBox(hInst, "NET_OPTIONS_SOCCER", hParentWnd, (DLGPROC)NetDlg_SoccerOptions);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the game server options dialog (host only)...

OptionsDlg:
	nRet = DialogBox(hInst, "NET_OPTIONS", hParentWnd, (DLGPROC)NetDlg_Options);

	if (nRet == DLG_BACK)
	{
		if (s_nGameType == NGT_SOCCER) goto SoccerOptionsDlg;
		if (s_nGameType == NGT_CAPTUREFLAG) goto CtfOptionsDlg;
		if (s_nGameType == NGT_TEAMS) goto TeamOptionsDlg;
		goto GameDlg;
	}

	if (nRet == DLG_CANCEL) return(DFALSE);

	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the game items dialog (host only)...

GameItemsDlg:
	nRet = DialogBox(hInst, "NET_GAMEITEMS", hParentWnd, (DLGPROC)NetDlg_GameItems);

	if (nRet == DLG_BACK) goto OptionsDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the game damage dialog (host only)...

GameDamageDlg:
	nRet = DialogBox(hInst, "NET_GAMEDAMAGE", hParentWnd, (DLGPROC)NetDlg_GameDamage);

	if (nRet == DLG_BACK) goto GameItemsDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the game levels dialog (host only)...

	nRet = DialogBox(hInst, "NET_GAMELEVELS", hParentWnd, (DLGPROC)NetDlg_GameLevels);

	if (nRet == DLG_BACK) goto GameDamageDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the finished dialog...

FinishedDlg:


	// Write out some console values...

	if (s_pNetStart->m_bHost)
	{
		WriteConsoleString("NetSessionName", s_sSessionName);

		WriteConsoleInt("NetGameType", s_nGameType);
		WriteConsoleInt("NetGameEnd", s_nEndType);
		WriteConsoleInt("NetEndFrags", s_nEndFrags);
		WriteConsoleInt("NetEndTime", s_nEndTime);
		WriteConsoleInt("NetMaxPlayers", s_nMaxPlayers);

		WriteConsoleInt("NetAmmoLevel", s_nNetAmmoLevel);
		WriteConsoleInt("NetAmmoRespawn", s_nNetAmmoRespawn);
		WriteConsoleInt("NetArmorLevel", s_nNetArmorLevel);
		WriteConsoleInt("NetArmorRespawn", s_nNetArmorRespawn);
		WriteConsoleInt("NetHealthLevel", s_nNetHealthLevel);
		WriteConsoleInt("NetHealthRespawn", s_nNetHealthRespawn);
		WriteConsoleInt("NetPowerupsLevel", s_nNetPowerupsLevel);
		WriteConsoleInt("NetPowerupsRespawn", s_nNetPowerupsRespawn);
		WriteConsoleInt("NetHealingRate", s_nNetHealingRate);
		WriteConsoleInt("NetFallDamage", s_bNetFallDamage);
		WriteConsoleInt("NetFriendlyFire", s_bNetFriendlyFire);
		WriteConsoleInt("NetNegTeamFrags", s_bNetNegTeamFrags);
		WriteConsoleInt("NetOnlyFlagScores", s_bNetOnlyFlagScores);
		WriteConsoleInt("NetOnlyFlagScores", s_bNetOnlyGoalScores);
		WriteConsoleInt("NetUseTeamSize", s_bNetUseTeamSize);
		WriteConsoleInt("NetFlagValue", s_nNetFlagValue);
		WriteConsoleInt("NetGoalValue", s_nNetGoalValue);
		WriteConsoleInt("NetSocBallSkin", s_nNetSocBallSkin);

		WriteConsoleFloat("NetWorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
		WriteConsoleFloat("NetRunSpeed", s_pServerOptions->m_fRunSpeed);
		WriteConsoleFloat("NetMissileSpeed", s_pServerOptions->m_fMissileSpeed);
		WriteConsoleString("NetWorldNightColor", s_pServerOptions->m_sWorldNightColor);
	}

	PostProcessName(s_sPlayerName);

	WriteConsoleString("NetPlayerName", s_sPlayerName);
	WriteConsoleString("NetConfigFile", s_sNetConfigFile);

	WriteConsoleInt("NetService", s_nNetService);
	WriteConsoleInt("NetJoinHost", s_nNetJoinHost);
	WriteConsoleInt("UpdateRate", s_nNetLatency);
	WriteConsoleInt("NetFpsLimit", s_nFpsLimit);
	WriteConsoleInt("NetLimitFps", s_bLimitFps);
	WriteConsoleInt("NetSaveLevels", s_bSaveLevels);
	WriteConsoleInt("NetTeam", s_nNetTeam);


	// All done...

	return(DTRUE);
}

NetPlayer* NetStart_GetPlayerStruct()
{
	return(&s_NetPlayer);
}

NetGame* NetStart_GetGameStruct()
{
	return(&s_NetGame);
}

NetClientData* NetStart_GetClientDataStruct()
{
	return(&s_NetClientData);
}

void NetStart_ClearGameStruct()
{
	memset(&s_NetGame, 0, sizeof(NetGame));
}

void GetConsoleString(char* sKey, char* sDest, char* sDefault)
{
	if (s_pClient)
	{
		HCONSOLEVAR hVar = s_pClient->GetConsoleVar(sKey);
		if (hVar)
		{
			char* sValue = s_pClient->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

int GetConsoleInt(char* sKey, int nDefault)
{
	if (s_pClient)
	{
		HCONSOLEVAR hVar = s_pClient->GetConsoleVar(sKey);
		if (hVar)
		{
			float fValue = s_pClient->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

DFLOAT GetConsoleFloat(char* sKey, DFLOAT fDefault)
{
	if (s_pClient)
	{
		HCONSOLEVAR hVar = s_pClient->GetConsoleVar(sKey);
		if (hVar)
		{
			float fValue = s_pClient->GetVarValueFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char* sKey, char* sValue)
{
	if (s_pClient)
	{
		char sTemp[256];
		wsprintf(sTemp, "+%s \"%s\"", sKey, sValue);
		s_pClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleInt(char* sKey, int nValue)
{
	if (s_pClient)
	{
		char sTemp[256];
		wsprintf(sTemp, "+%s %i", sKey, nValue);
		s_pClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleFloat(char* sKey, DFLOAT fValue)
{
	if (s_pClient)
	{
		char sTemp[256];
		sprintf(sTemp, "+%s %f", sKey, fValue);
		s_pClient->RunConsoleString(sTemp);
	}
}

void NetStart_DisplayError(HINSTANCE hInst)
{
	char sError[256];

	if (LoadString(hInst, s_nErrorString, sError, 254) == 0)
	{
		strcpy(sError, "ERROR - Unable to setup the network game.");
	}

#ifdef _DEMO
	MessageBox(NULL, sError, "Blood2 Demo", MB_OK);
#else
	MessageBox(NULL, sError, "Blood2", MB_OK);
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoLobbyLaunchWizard
//
//	PURPOSE:	Does the net start wizard for a lobby launched game
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_DoLobbyLaunchWizard(CClientDE* pClient)
{
	// Sanity checks...

	if (!pClient) return(DFALSE);


	// Declare local variables we'll be using...

	int			nRet;
	HWND		hParentWnd = NULL;
	NetStart	ns;


	// Set our static pointers...

	s_pClient        = pClient;
	s_pNetStart      = &ns;
	s_pServerOptions = &g_ServerOptions;

	s_bLobbyLaunch = TRUE;


	// Fill in the default server options...

	memset(s_pServerOptions, 0, sizeof(ServerOptions));

	s_pServerOptions->m_fWorldTimeSpeed = -1.0f;
	s_pServerOptions->m_fRunSpeed       = 1.3f;
	s_pServerOptions->m_fMissileSpeed   = 1.0;

	strcpy(s_pServerOptions->m_sWorldNightColor, "0.5 0.5 0.5");


	// Init the net game info manager...

	s_NinfoMgr.Term();
	s_NinfoMgr.Init(s_pClient);


	// Clear the NetStart structure...

	memset(s_pNetStart, 0, sizeof(NetStart));


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));


	// Get the module handle...

	HINSTANCE hInst;

	void* hModule;
	g_pClientDE->GetEngineHook("cres_hinstance",&hModule);
	hInst = (HINSTANCE)hModule;
	if (!hInst) return (DFALSE);

	s_hInst = hInst;


	// Init the engine's networking...

	DRESULT dr = pClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Get the lobby launch info...

	void* pLobbyLaunchInfo = NULL;

	dr = pClient->GetLobbyLaunchInfo("dplay2", &pLobbyLaunchInfo);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}

	LPDPLCONNECTION pDplConn = (LPDPLCONNECTION)pLobbyLaunchInfo;


	// Read some default values from the console...

	GetConsoleString("NetPlayerName", s_sPlayerName, "Caleb");
	PostProcessName(s_sPlayerName);

#ifdef _DEMO
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Blood2 Demo");
#else
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Blood2");
#endif

	s_nGameType    = GetConsoleInt("NetGameType", NGT_DEATHMATCH);
	s_nEndType     = GetConsoleInt("NetGameEnd", NGE_FRAGS);
	s_nEndFrags    = GetConsoleInt("NetEndFrags", 25);
	s_nEndTime     = GetConsoleInt("NetEndTime", 15);
	s_nNetService  = GetConsoleInt("NetService", 0);
	s_nNetJoinHost = GetConsoleInt("NetJoinHost", 0);
	s_nNetLatency  = GetConsoleInt("UpdateRate", DEFAULT_UPDATERATE);
	s_nMaxPlayers  = GetConsoleInt("NetMaxPlayers", MAX_MULTI_PLAYERS_DISPLAY);
	s_nFpsLimit    = GetConsoleInt("NetFpsLimit", s_nFpsLimit);
	s_bLimitFps    = GetConsoleInt("NetLimitFps", s_bLimitFps);


	// Update info from the lobby info...

	if (strlen(pDplConn->lpSessionDesc->lpszSessionNameA) > 0)
	{
		strcpy(s_sSessionName, pDplConn->lpSessionDesc->lpszSessionNameA);
	}

	if (strlen(pDplConn->lpPlayerName->lpszShortNameA) > 0)
	{
		strcpy(s_sPlayerName, pDplConn->lpPlayerName->lpszShortNameA);
	}

	PostProcessName(s_sPlayerName);


	// Set our join host info...

	if (pDplConn->dwFlags & DPLCONNECTION_CREATESESSION)
	{
		s_pNetStart->m_bHost = DTRUE;
	}
	else
	{
		s_pNetStart->m_bHost = DFALSE;
	}


	// Do the player dialog...

PlayerDlg:
	nRet = DialogBox(hInst, "NET_PLAYER", hParentWnd, (DLGPROC)NetDlg_Player);

	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}

	if (!s_pNetStart->m_bHost) goto Launch;


	// Do the game dialog (host only)...

GameDlg:

#ifdef _ADDON
	nRet = DialogBox(hInst, "NET_GAME_AO", hParentWnd, (DLGPROC)NetDlg_Game);
#else
	nRet = DialogBox(hInst, "NET_GAME", hParentWnd, (DLGPROC)NetDlg_Game);
#endif

	if (nRet == DLG_BACK) goto PlayerDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Do the game levels dialog (host only)...

	nRet = DialogBox(hInst, "NET_GAMELEVELS", hParentWnd, (DLGPROC)NetDlg_GameLevels);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(DFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Launch the game...

Launch:
	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	req.m_flags = SG_LOBBY;	// flag that we were lobby launched

	if (s_pNetStart->m_bHost)
	{
		req.m_Type = STARTGAME_HOST;

		strcpy(req.m_WorldName, ns.m_sLevel);
		memcpy(&req.m_HostInfo, &ns.m_NetHost, sizeof(NetHost));
	}
	else
	{
		req.m_Type        = STARTGAME_CLIENT;
		req.m_pNetSession = ns.m_pNetSession;
	}

	req.m_pGameInfo   = NetStart_GetGameStruct();
	req.m_GameInfoLen = sizeof(NetGame_t);

	dr = pClient->StartGame(&req);

	NetStart_FreeSessionList(pClient);

	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(DFALSE);
	}


	// Write out some console values...

	if (s_pNetStart->m_bHost)
	{
		WriteConsoleString("NetSessionName", s_sSessionName);

		WriteConsoleInt("NetGameType", s_nGameType);
		WriteConsoleInt("NetGameEnd", s_nEndType);
		WriteConsoleInt("NetEndFrags", s_nEndFrags);
		WriteConsoleInt("NetEndTime", s_nEndTime);
		WriteConsoleInt("NetMaxPlayers", s_nMaxPlayers);
	}

	PostProcessName(s_sPlayerName);
	WriteConsoleString("NetPlayerName", s_sPlayerName);

	WriteConsoleInt("NetPlayerColor", s_nPlayerColor);
	WriteConsoleInt("NetMech", s_nMech);
	WriteConsoleInt("NetLatency", s_nNetLatency);


	// Display a loading screen...

	DDWORD cxLoading, cyLoading, cxScreen, cyScreen;
	HSURFACE hLoading = pClient->CreateSurfaceFromBitmap ("interface/loading.pcx");
	pClient->GetSurfaceDims (hLoading, &cxLoading, &cyLoading);
	HSURFACE hScreen = pClient->GetScreenSurface();
	pClient->GetSurfaceDims (hScreen, &cxScreen, &cyScreen);

	pClient->ClearScreen(DNULL, CLEARSCREEN_SCREEN);
	pClient->Start3D();
	pClient->StartOptimized2D();
	pClient->DrawSurfaceToSurface(hScreen, hLoading, DNULL, ((int)cxScreen - (int)cxLoading) / 2, ((int)cyScreen - (int)cyLoading) / 2);
	pClient->EndOptimized2D();
	pClient->End3D();
	pClient->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_MinimizeMainWnd
//
//	PURPOSE:	Minimizes the main window
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_MinimizeMainWnd(CClientDE* pClient)
{
	// Sanity checks...

	if (!pClient) return(DFALSE);


	// Get the window handle...

	s_hMainWnd = NULL;

	DRESULT dr = pClient->GetEngineHook("HWND", &s_hMainWnd);
	if (dr != LT_OK || !s_hMainWnd)
	{
		s_hMainWnd = GetActiveWindow();
	}


	// Save the current render mode...

	pClient->GetRenderMode(&s_RMode);


	// Shutdown the renderer, minimize it, and hide it...

	pClient->ShutdownRender(RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_RestoreMainWnd
//
//	PURPOSE:	Restores the main window
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_RestoreMainWnd()
{
	// Sanity checks...

	if (!s_pClient) return(DFALSE);


	// Restore the main window as necessary...

	DRESULT dr = s_pClient->SetRenderMode(&s_RMode);
	if (dr != LT_OK) return(DFALSE);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_HandleDefaultDialogCommands
//
//	PURPOSE:	Handles default dialog commands (next, back, cancel, etc)
//
// ----------------------------------------------------------------------- //

BOOL NetStart_HandleDefaultDialogCommands(HWND hDlg, WPARAM wParam)
{
	// Handle any default commands...

	switch (wParam)
	{
		case IDC_BACK:
		{
			EndDialog(hDlg, DLG_BACK);
			return(TRUE);
		}

		case IDCANCEL:
		{
			EndDialog(hDlg, DLG_CANCEL);
			return(TRUE);
		}
	}


	// If we get here, it's not a default command...

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillServiceList
//
//	PURPOSE:	Fills a list box with the available services
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillServiceList(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Get the service list...

	NetService *pCur, *pListHead = NULL;

	DRESULT dr = s_pClient->GetServiceList(pListHead);
	if (dr != LT_OK) return(FALSE);

	if (!pListHead)
	{
		return(FALSE);
	}


	// Add each service to the list box...
	DWORD iCurInfo=0;

	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	for (pCur=pListHead; pCur; pCur=pCur->m_pNext)
	{
		int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pCur->m_sName);
		if (iItem != LB_ERR)
		{
			SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pCur->m_handle);
			if(iCurInfo < MAX_NETSTART_SERVICES)
			{
				g_ServiceHandles[iCurInfo] = pCur->m_handle;
				g_ServiceFlags[iCurInfo] = pCur->m_dwFlags;
				++iCurInfo;
			}
		}
	}


	// Select the last used service...

	int nRet = SendMessage(hList, LB_SETCURSEL, s_nNetService, 0);
	if (nRet == LB_ERR)
	{
		SendMessage(hList, LB_SETCURSEL, 0, 0);
	}


	// Free the service list...

	s_pClient->FreeServiceList(pListHead);


	// All done...

	return(TRUE);
}

BOOL NetStart_FillNetSpeed(HWND hDlg)
{
	char str[128];

	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Fill the combo box with our options...

	HWND hComboBox = GetDlgItem(hDlg, IDC_NETCONNECTION);
	if (!hComboBox) return(FALSE);

	SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);

	sprintf(str, "%s (%d)", s_sHighPing, HIGHPING_UPDATERATE);
	AddComboInfo(hComboBox, str, HIGHPING_UPDATERATE);
	
	sprintf(str, "%s (%d)", s_sModem, MODEM_UPDATERATE);
	AddComboInfo(hComboBox, str, MODEM_UPDATERATE);
	
	sprintf(str, "%s (%d)", s_sIsdn, ISDN_UPDATERATE);
	AddComboInfo(hComboBox, str, ISDN_UPDATERATE);
	
	sprintf(str, "%s (%d)", s_sLan, LAN_UPDATERATE);
	AddComboInfo(hComboBox, str, LAN_UPDATERATE);

	SendMessage(hComboBox, CB_SETCURSEL, 0, 0);


	// Set the current selection...

	s_nNetLatency  = GetConsoleInt("UpdateRate", DEFAULT_UPDATERATE);
	int nVal = s_nNetLatency;
	if (nVal == 0) nVal = 200;

	int count = SendMessage(hComboBox, CB_GETCOUNT, 0, 0);
	if (count == CB_ERR) return(TRUE);

	BOOL bFound = FALSE;

	for (int i = 0; i < count; i++)
	{
		int nItemData = SendMessage(hComboBox, CB_GETITEMDATA, i, 0);
		if (nItemData != CB_ERR)
		{
			if (nItemData == nVal)
			{
				SendMessage(hComboBox, CB_SETCURSEL, i, 0);
				i = count;
				bFound = TRUE;
			}
		}
	}


	// Set a custom value if necessary...

	if (!bFound)
	{
		sprintf(str, "%s (%d)", s_sCustom, nVal);
		int nIndex = AddComboInfo(hComboBox, str, nVal);
		if (nIndex != CB_ERR)
		{
			SendMessage(hComboBox, CB_SETCURSEL, nIndex, 0);
		}
	}


	// All done...

	return(TRUE);
}

int AddComboInfo(HWND hComboBox, char* sText, DWORD dwValue)
{
	int nIndex = SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)sText);
	if (nIndex == CB_ERR) return(CB_ERR);
	SendMessage(hComboBox, CB_SETITEMDATA, nIndex, dwValue);
	return(nIndex);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_SelectCurrentService
//
//	PURPOSE:	Selects the currently selected service in the list box
//
// ----------------------------------------------------------------------- //

BOOL NetStart_SelectCurrentService(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Get the currently select list-box item...

	int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);


	// Get the item data for this index...

	int nRet = SendMessage(hList, LB_GETITEMDATA, nIndex, 0);
	if (nRet == LB_ERR) return(FALSE);

	HNETSERVICE hNetService = (HNETSERVICE)nRet;
	if (!hNetService) return(FALSE);


	// Select the service...

	DRESULT dr = s_pClient->SelectService(hNetService);
	if (dr != LT_OK) return(FALSE);

	s_nNetService = nIndex;
	g_SelectedService = hNetService;


	// All done...

	return(TRUE);
}

BOOL NetStart_SelectJoinHost(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the join host selection...

	if (IsDlgButtonChecked(hDlg, IDC_JOIN))
	{
		s_pNetStart->m_bHost = FALSE;
		s_nNetJoinHost       = NET_JOIN;
	}
	else if (IsDlgButtonChecked(hDlg, IDC_HOST))
	{
		s_pNetStart->m_bHost = TRUE;
		s_nNetJoinHost       = NET_HOST;
	}


	// All done...

	return(TRUE);
}

BOOL NetStart_SelectNetSpeed(HWND hComboBox)
{
	// Sanity checks...

	if (!hComboBox) return(FALSE);


	// Get the latency...

	s_NetPlayer.m_dwLatency = MODEM_UPDATERATE;

	int nIndex = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
	if (nIndex == CB_ERR) return(FALSE);

	int nVal = SendMessage(hComboBox, CB_GETITEMDATA, nIndex, 0);
	if (nVal == CB_ERR) return(FALSE);

	s_nNetLatency           = nVal;
	s_NetPlayer.m_dwLatency = nVal;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Services
//
//	PURPOSE:	Does the SERVICES dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Services(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			PostMessage(hDlg, WM_COMMAND, ID_FILLSTUFF, 0);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				if (!NetStart_SelectCurrentService(GetDlgItem(hDlg, IDC_SERVICELIST)))
				{
					s_nErrorString = IDS_NETERR_SELECTSERVICE;
					EndDialog(hDlg, DLG_ERROR);
				}

				if (!NetStart_SelectJoinHost(hDlg))
				{
					EndDialog(hDlg, DLG_ERROR);
				}

				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
			else if (wParam == ID_FILLSTUFF)
			{
				if (!NetStart_FillServiceList(GetDlgItem(hDlg, IDC_SERVICELIST)))
				{
					s_nErrorString = IDS_NETERR_SELECTSERVICE;
					EndDialog(hDlg, DLG_ERROR);
				}

				if (s_nInternalJoinHost == NET_HOST)
				{
					ShowWindow(GetDlgItem(hDlg, IDC_HOSTNOTE), SW_NORMAL);
				}

				if (s_nInternalJoinHost == NET_UNKNOWN)
				{
					ShowWindow(GetDlgItem(hDlg, IDC_HOSTNOTE), SW_NORMAL);
					ShowWindow(GetDlgItem(hDlg, IDC_JOINHOST_TITLE), SW_NORMAL);
					ShowWindow(GetDlgItem(hDlg, IDC_JOIN), SW_NORMAL);
					ShowWindow(GetDlgItem(hDlg, IDC_HOST), SW_NORMAL);

					CheckDlgButton(hDlg, IDC_JOIN, (s_nNetJoinHost == NET_JOIN));
					CheckDlgButton(hDlg, IDC_HOST, (s_nNetJoinHost == NET_HOST));
				}
				else
				{
					CheckDlgButton(hDlg, IDC_JOIN, (s_nInternalJoinHost == NET_JOIN));
					CheckDlgButton(hDlg, IDC_HOST, (s_nInternalJoinHost == NET_HOST));
				}

				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
// TCP/IP address dialog proc.
// ----------------------------------------------------------------------- //

BOOL CALLBACK IPDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			return 1;
		}
		
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				char sIp[IPM_MAX_ADDRESS + 2];
				GetDlgItemText(hDlg, IDC_IPADDRESS, sIp, IPM_MAX_ADDRESS);
				s_IpMgr.AddIp(sIp);
				EndDialog(hDlg, IDOK);
			}
		}

		break;
	}

	return 0;
}

BOOL CALLBACK AddIPDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			return 1;
		}
		
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				GetDlgItemText(hDlg, IDC_IPADDRESS, g_DlgIP, g_DlgIPLen);
				EndDialog(hDlg, IDOK);
			}
			else if(LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, IDCANCEL);
			}
		}

		break;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillSessionList
//
//	PURPOSE:	Fills a list box with the available sessions
//
// ----------------------------------------------------------------------- //

int NetStart_FillSessionList(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(0);


	// Get our list box...

	HWND hList = GetDlgItem(hDlg, IDC_SESSIONLIST);
	if (!hList) return(0);


	// Get the currently selected session (if any) so we can restore the selection...

	char sSel[128];

	int iSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (iSel != LB_ERR)
	{
		SendMessage(hList, LB_GETTEXT, iSel, (LPARAM)sSel);
	}
	else
	{
		sSel[0] = '\0';
	}


	char *pInfo = "";
	DDWORD flags;
	DWORD i;
	char tempIPText[256];

	// If it's TCP/IP, ask the user for an address.
	flags = GetSelectedServiceFlags();
	if(flags & NETSERVICE_TCPIP)
	{
		g_DlgIP = tempIPText;
		g_DlgIPLen = sizeof(tempIPText);

		// Ask them for an address.
		DialogBox(s_hInst, "NET_TCPIPADDRESS", hDlg, (DLGPROC)IPDlgProc);
		pInfo = g_DlgIP;
	}


	// Get the session list...

	NetSession* pCur      = NULL;
	NetSession* pListHead = NULL;

	pListHead = NetStart_GetSessionList(s_pClient, pInfo);
	if (!pListHead)	return(0);


	// Remove the current net games from the manager...

	s_NinfoMgr.RemoveGames();


	// Add each Session to the list box...

	int c = 0;

	SET_NODRAW(hList);
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	for (pCur=pListHead; pCur; pCur=pCur->m_pNext)
	{
		// Add a new net game via the manager...

		CNinfoGame* pGame = s_NinfoMgr.AddGame(pCur->m_sName, pCur, pCur->m_dwCurPlayers);
		if (pGame)
		{
			int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pGame->GetName());
			if (iItem != LB_ERR)
			{
				c++;

				SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pGame->GetSessionPointer());
			}
		}
	}


	// Select the appropriate session in the list...

	iSel = 0;

	for (i = 0; i < (DWORD)c; i++)
	{
		char sText[128];
		if (SendMessage(hList, LB_GETTEXT, i, (LPARAM)sText) != LB_ERR)
		{
			if (strcmp(sText, sSel) == 0)
			{
				iSel = i;
				i    = c;
			}
		}
	}

	SendMessage(hList, LB_SETCURSEL, iSel, 0);
	SET_REDRAW(hList);


	// Update the selected session info...

	if (!NetStart_DisplaySelectedSessionInfo(hDlg))
	{
		NetStart_ClearSelectedSessionInfo(hDlg);
	}


	// All done...

	return(c);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DisplaySelectedSessionInfo
//
//	PURPOSE:	Display information about the selected session
//
// ----------------------------------------------------------------------- //

BOOL NetStart_DisplaySelectedSessionInfo(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get our list box...

	HWND hList = GetDlgItem(hDlg, IDC_SESSIONLIST);
	if (!hList) return(DFALSE);


	// Get the currently select list-box item...

	int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);


	// Get the item data for this index...

	int nRet = SendMessage(hList, LB_GETITEMDATA, nIndex, 0);
	if (nRet == LB_ERR) return(FALSE);

	NetSession* pNetSession = (NetSession*)nRet;
	if (!pNetSession) return(FALSE);


	// Get the net game info object form the session handle...

	CNinfoGame* pGame = s_NinfoMgr.GetGame(pNetSession);
	if (!pGame) return(FALSE);


	// Set all the info...

	SetDlgItemText(hDlg, IDC_HOSTNAME, pGame->GetHost());
	SetDlgItemText(hDlg, IDC_GAMETYPE, pGame->GetTypeString());
	SetDlgItemText(hDlg, IDC_LEVEL, pGame->GetLevel());

	int cPlayers = pGame->GetNumPlayers();
	if (cPlayers == 0) cPlayers = pGame->GetNumDPlayPlayers();

	SetDlgItemInt(hDlg, IDC_NUMPLAYERS, cPlayers, TRUE);


	// Fill the player list...

	hList = GetDlgItem(hDlg, IDC_PLAYERLIST);
	if (!hList) return(DFALSE);

	SET_NODRAW(hList)
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	CNinfoPlayer* pPlayer = pGame->GetFirstPlayer();

	if (!pPlayer && cPlayers > 0)
	{
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)s_sPlayerInfoNotSent);
	}

	while (pPlayer)
	{
		char sPlayer[256];
		wsprintf(sPlayer, "%s\t%i", pPlayer->GetName(), pPlayer->GetFrags());
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sPlayer);
		pPlayer = pGame->GetNextPlayer();
	}

	SET_REDRAW(hList);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_ClearSelectedSessionInfo
//
//	PURPOSE:	Clears the information about the selected session
//
// ----------------------------------------------------------------------- //

BOOL NetStart_ClearSelectedSessionInfo(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Clear all the info...

	SetDlgItemText(hDlg, IDC_HOSTNAME, "");
	SetDlgItemText(hDlg, IDC_GAMETYPE, "");
	SetDlgItemText(hDlg, IDC_LEVEL, "");
	SetDlgItemText(hDlg, IDC_NUMPLAYERS, "");


	// Clear the player list...

	HWND hList = GetDlgItem(hDlg, IDC_PLAYERLIST);
	if (!hList) return(DFALSE);

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_JoinCurrentSession
//
//	PURPOSE:	Joins the currently selected Session in the list box
//
// ----------------------------------------------------------------------- //

BOOL NetStart_JoinCurrentSession(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Get the currently select list-box item...

	int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);


	// Get the item data for this index...

	int nRet = SendMessage(hList, LB_GETITEMDATA, nIndex, 0);
	if (nRet == LB_ERR) return(FALSE);

	NetSession* pNetSession = (NetSession*)nRet;
	if (!pNetSession) return(FALSE);


	// Select the Session by storing the handle in the NetStart structure...

	s_pNetStart->m_pNetSession = pNetSession;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_JoinHost
//
//	PURPOSE:	Does the JOINHOST dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_JoinHost(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			CheckDlgButton(hDlg, IDC_JOIN, (s_nNetJoinHost == NET_JOIN));
			CheckDlgButton(hDlg, IDC_HOST, (s_nNetJoinHost == NET_HOST));

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				if (IsDlgButtonChecked(hDlg, IDC_JOIN))
				{
					s_pNetStart->m_bHost = FALSE;
					s_nNetJoinHost       = NET_JOIN;
				}
				else if (IsDlgButtonChecked(hDlg, IDC_HOST))
				{
					s_pNetStart->m_bHost = TRUE;
					s_nNetJoinHost       = NET_HOST;
				}

				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_HostSession
//
//	PURPOSE:	Hosts a new session
//
// ----------------------------------------------------------------------- //

BOOL NetStart_HostSession1(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the session name...

	char sName[128];
	sName[0] = '\0';

	GetDlgItemText(hDlg, IDC_NAME, sName, 120);

	if (strlen(sName) == 0)
	{
		if (strlen(s_sPlayerName) > 0)
		{
			wsprintf(sName, "%s with %s", s_sDefaultSessionName, s_sPlayerName);
		}
		else
		{
			strcpy(sName, s_sDefaultSessionName);
		}
	}

	strcpy(s_sSessionName, sName);


	// All done with part 1...

	return(TRUE);
}

BOOL NetStart_HostSession2(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Create the big string with all the game parameters...

	char sString[1024];
	sString[0] = '\0';

	if (!CNinfoMgr::CreateSessionString(sString, s_sSessionName, s_pNetStart->m_sPlayer, s_sFirstLevel, s_nGameType))
	{
		strcpy(sString, s_sSessionName);
	}


	// Fill out a NetHost structure in the NetStart structure...

	memset(&s_pNetStart->m_NetHost, 0, sizeof(NetHost));

	s_pNetStart->m_NetHost.m_dwMaxPlayers = s_nMaxPlayers;
	strcpy(s_pNetStart->m_NetHost.m_sName, sString);
	s_pNetStart->m_NetHost.m_Port = g_HostPort;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Sessions
//
//	PURPOSE:	Does the SESSIONS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Sessions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HCURSOR hCurCursor;
	static BOOL	   bFirstFill = TRUE;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			bFirstFill = TRUE;
			hCurCursor = (HCURSOR)GetClassLong(hDlg, GCL_HCURSOR);

			ForceActiveFocus(hDlg);

			NetStart_ClearSelectedSessionInfo(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_NEXT), FALSE);
			int nTabs[10] = {80, 82, 84, 86, 88, 90, 92, 94, 96, 98 };
			SendMessage(GetDlgItem(hDlg, IDC_PLAYERLIST), LB_SETTABSTOPS, 10, (LPARAM)&nTabs);

			SetTimer(hDlg, 1, 500, NULL);

			return(TRUE);
		}

		case WM_TIMER:
		{
			KillTimer(hDlg, 1);

			hCurCursor = LoadCursor(NULL, IDC_WAIT);
			HCURSOR hOldCursor = SetCursor(hCurCursor);

			int c = NetStart_FillSessionList(hDlg);
			if (c == 0) NetStart_ClearSelectedSessionInfo(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_NEXT), c > 0);

			hCurCursor = hOldCursor;
			SetCursor(hCurCursor);

			if (c == 0 && bFirstFill && !s_bDontDisplayNoSessions)
			{
				bFirstFill = FALSE;
				int nRet = DoNoSessionsMessage(hDlg);
				if (nRet == ID_NO) PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
				SetTimer(hDlg, 1, 3000, NULL);
			}
			else
			{
				SetTimer(hDlg, 1, 10000, NULL);
			}

			return(TRUE);
		}

		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		{
			SetCursor(hCurCursor);
			return(FALSE);
		}

		case WM_SETCURSOR:
		{
			SetCursor(hCurCursor);
			return(TRUE);
		}
			
		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				KillTimer(hDlg, 1);
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				if (!NetStart_JoinCurrentSession(GetDlgItem(hDlg, IDC_SESSIONLIST)))
				{
					s_nErrorString = IDS_NETERR_JOINSESSION;
					EndDialog(hDlg, DLG_ERROR);
				}
				else
				{
					EndDialog(hDlg, DLG_NEXT);
				}
				KillTimer(hDlg, 1);
				return(TRUE);
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				if (!NetStart_DisplaySelectedSessionInfo(hDlg))
				{
					NetStart_ClearSelectedSessionInfo(hDlg);
				}
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Game
//
//	PURPOSE:	Does the GAME dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Game(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ClientDE *pClientDE = s_pClient;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			SetDlgItemText(hDlg, IDC_NAME, s_sSessionName);
			NetStart_SetEndGameInfo(hDlg);
			NetStart_InitGameType(hDlg);

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				if (!NetStart_HostSession1(hDlg))
				{
					s_nErrorString = IDS_NETERR_HOSTSESSION;
					EndDialog(hDlg, DLG_ERROR);
				}
				else
				{
					NetStart_FillGameStruct(hDlg);
					NetStart_FillGameEnd(hDlg);
					NetStart_FillGameType(hDlg);
					EndDialog(hDlg, DLG_NEXT);
				}
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

void ForceActiveFocus(HWND hWnd)
{

#ifdef _DEMO
	SetWindowText(hWnd, "Blood2 Demo Multiplayer Wizard");
#endif

	SetActiveWindow(hWnd);
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
}

BOOL NetStart_InitGameType(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	CheckDlgButton(hDlg, IDC_BB, s_nGameType == NGT_DEATHMATCH ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CTF, s_nGameType == NGT_CAPTUREFLAG ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_TEAMS, s_nGameType == NGT_TEAMS ? BST_CHECKED : BST_UNCHECKED);

#ifdef _ADDON
	CheckDlgButton(hDlg, IDC_SOCCER, s_nGameType == NGT_SOCCER ? BST_CHECKED : BST_UNCHECKED);
#endif

	return(TRUE);
}

BOOL NetStart_FillGameType(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	if (IsDlgButtonChecked(hDlg, IDC_BB) == BST_CHECKED) s_nGameType = NGT_DEATHMATCH;
	if (IsDlgButtonChecked(hDlg, IDC_CTF) == BST_CHECKED) s_nGameType = NGT_CAPTUREFLAG;
	if (IsDlgButtonChecked(hDlg, IDC_TEAMS) == BST_CHECKED) s_nGameType = NGT_TEAMS;

#ifdef _ADDON
	if (IsDlgButtonChecked(hDlg, IDC_SOCCER) == BST_CHECKED) s_nGameType = NGT_SOCCER;
#endif

	s_NetGame.m_byType = s_nGameType;

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillGameStruct
//
//	PURPOSE:	Fills the NetGame structure with the main game info
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillGameStruct(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Player
//
//	PURPOSE:	Does the PLAYER dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Player(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			if (s_bLobbyLaunch && !s_pNetStart->m_bHost) SetDlgItemText(hDlg, IDC_NEXT, "&Finished");
			if (s_pNetStart->m_bHaveTcpIp) SetDlgItemText(hDlg, IDC_NEXT, "&Finished");

			NetStart_InitPlayer(hDlg);
			NetStart_FillNetSpeed(hDlg);
			NetStart_InitFpsLimit(hDlg);

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				GetDlgItemText(hDlg, IDC_NAME, s_pNetStart->m_sPlayer, 60);
				if (strlen(s_pNetStart->m_sPlayer) <= 0) strcpy(s_pNetStart->m_sPlayer, "Caleb");
				PostProcessName(s_pNetStart->m_sPlayer);

				NetStart_FillPlayerStruct(hDlg);
				NetStart_FillFpsLimit(hDlg);
				NetStart_SelectNetSpeed(GetDlgItem(hDlg, IDC_NETCONNECTION));
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				CConfig* pConfig = s_ConfigMgr.GetListBoxSelection(GetDlgItem(hDlg, IDC_CONFIGS));
				if (pConfig) pConfig->FillDialog(hDlg);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_InitPlayer
//
//	PURPOSE:	Inits the player dialog
//
// ----------------------------------------------------------------------- //

BOOL NetStart_InitPlayer(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Init the config manager...

	if (!s_ConfigMgr.Init("Players"))
	{
		return(FALSE);
	}


	// Set the player name...

	PostProcessName(s_sPlayerName);
	SetDlgItemText(hDlg, IDC_NAME, s_sPlayerName);


	// Fill the list of .b2c files...

	HWND hList = GetDlgItem(hDlg, IDC_CONFIGS);
	if (!hList) return(FALSE);

	s_ConfigMgr.FillListBox(hList, s_sNetConfigFile, "Caleb.b2c");

	CConfig* pConfig = s_ConfigMgr.GetListBoxSelection(GetDlgItem(hDlg, IDC_CONFIGS));
	if (pConfig) pConfig->FillDialog(hDlg);


	// Set the team...

	CheckDlgButton(hDlg, IDC_TEAM1, s_nNetTeam == TEAM_1 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_TEAM2, s_nNetTeam == TEAM_2 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_AUTOTEAM, s_nNetTeam == TEAM_AUTO ? BST_CHECKED : BST_UNCHECKED);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillPlayerStruct
//
//	PURPOSE:	Fills the player struct with the player info from the dlg
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillPlayerStruct(HWND hDlg, BOOL bUseComboConfig)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the player name...

	GetDlgItemText(hDlg, IDC_NAME, s_pNetStart->m_sPlayer, 60);
	if (_mbstrlen(s_pNetStart->m_sPlayer) <= 0) _mbscpy((unsigned char*)s_pNetStart->m_sPlayer, (const unsigned char*)"Caleb");

	GetDlgItemText(hDlg, IDC_NAME, s_NetPlayer.m_sName, MAX_PLAYER_NAME-1);
	if (_mbstrlen(s_NetPlayer.m_sName) <= 0) _mbscpy((unsigned char*)s_NetPlayer.m_sName, (const unsigned char*)"Caleb");

	_mbscpy((unsigned char*)s_sPlayerName, (const unsigned char*)s_NetPlayer.m_sName);

	PostProcessName(s_NetPlayer.m_sName);
	PostProcessName(s_pNetStart->m_sPlayer);
	PostProcessName(s_sPlayerName);


	// Get the config file...

	_mbscpy((unsigned char*)s_NetPlayer.m_sConfig, (const unsigned char*)"Caleb.b2c");
	_mbscpy((unsigned char*)s_sNetConfigFile, (const unsigned char*)"Caleb.b2c");

	CConfig* pConfig = NULL;

	if (bUseComboConfig)
	{
		pConfig = s_ConfigMgr.GetComboBoxSelection(GetDlgItem(hDlg, IDC_CONFIGS));
	}
	else
	{
		pConfig = s_ConfigMgr.GetListBoxSelection(GetDlgItem(hDlg, IDC_CONFIGS));
	}

	if (pConfig)
	{
		_mbscpy((unsigned char*)s_NetPlayer.m_sConfig, (const unsigned char*)pConfig->GetFileName());
		_mbscpy((unsigned char*)s_sNetConfigFile, (const unsigned char*)pConfig->GetFileName());
	}


	// Get the player team...

	int nTeam = TEAM_AUTO;

	if (IsDlgButtonChecked(hDlg, IDC_TEAM1) == BST_CHECKED) nTeam = TEAM_1;
	if (IsDlgButtonChecked(hDlg, IDC_TEAM2) == BST_CHECKED) nTeam = TEAM_2;

	s_NetPlayer.m_dwTeam     = nTeam;
	s_NetClientData.m_dwTeam = nTeam;
	s_nNetTeam               = nTeam;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Welcome
//
//	PURPOSE:	Does the WELCOME dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Welcome(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			SetTimer(hDlg, 1, 500, NULL);
			return(TRUE);
		}

		case WM_TIMER:
		{
			KillTimer(hDlg, 1);
			ForceActiveFocus(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Finished
//
//	PURPOSE:	Does the FINISHED dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Finished(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				KillTimer(hDlg, 1);
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillLevelList
//
//	PURPOSE:	Fills a list box with the list of levels
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillLevelList(HWND hList, char* sDir, int nData, BOOL bClearList)
{
	// Sanity checks...

	if (!hList) return(FALSE);
	if (!sDir) return(FALSE);


	// Get a list of world names and put them in the list box...

	if (bClearList)
	{
		SendMessage(hList, LB_RESETCONTENT, 0, 0);
	}

	FileEntry* pFiles = s_pClient->GetFileList(sDir);
	if (!pFiles) return(FALSE);

	FileEntry* ptr = pFiles;

	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			if (strnicmp(&ptr->m_pBaseFilename[strlen(ptr->m_pBaseFilename) - 4], ".dat", 4) == 0)
			{
				char sLevel[128];
				strcpy(sLevel, ptr->m_pBaseFilename);
				int len = strlen(sLevel);
				if (len > 4) sLevel[len - 4] = '\0';

				if (NetStart_IsOkToDisplayLevel(sLevel))
				{
					int nIndex = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)sLevel);
					if (nIndex != LB_ERR)
					{
						SendMessage(hList, LB_SETITEMDATA, nIndex, nData);
					}
				}
			}
		}

		ptr = ptr->m_pNext;
	}

	s_pClient->FreeFileList(pFiles);

	SendMessage(hList, LB_SETCURSEL, 0, 0);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillGameEnd
//
//	PURPOSE:	Fills the NetGame structure with the game end info
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillGameEnd(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the game end type...

	int nType = NGE_NEVER;

	if (IsDlgButtonChecked(hDlg, IDC_AFTERFRAGS) == BST_CHECKED) nType = NGE_FRAGS;

	if (IsDlgButtonChecked(hDlg, IDC_AFTERMINUTES) == BST_CHECKED)
	{
		if (nType == NGE_FRAGS) nType = NGE_FRAGSANDTIME;
		else nType = NGE_TIME;
	}

	s_NetGame.m_byEnd = nType;
	s_nEndType        = nType;


	// Get the game end value...

	s_nEndFrags = GetDlgItemInt(hDlg, IDC_FRAGS, NULL, FALSE);
	s_nEndTime  = GetDlgItemInt(hDlg, IDC_MINUTES, NULL, FALSE);

	s_NetGame.m_dwEndTime  = s_nEndTime;
	s_NetGame.m_dwEndFrags = s_nEndFrags;


	// All done...

	return(TRUE);
}

BOOL NetStart_SetEndGameInfo(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	SetDlgItemInt(hDlg, IDC_FRAGS, s_nEndFrags, FALSE);
	SetDlgItemInt(hDlg, IDC_MINUTES, s_nEndTime, FALSE);
	CheckDlgButton(hDlg, IDC_AFTERFRAGS,   ((s_nEndType == NGE_FRAGS) || (s_nEndType == NGE_FRAGSANDTIME)) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_AFTERMINUTES, ((s_nEndType == NGE_TIME) || (s_nEndType == NGE_FRAGSANDTIME))  ? BST_CHECKED : BST_UNCHECKED);

	return(TRUE);
}

void NetStart_AddLevelToList(HWND hList, char* sGameLevel)
{
	// Sanity checks...

	if (!hList) return;
	if (!sGameLevel) return;
	if (sGameLevel[0] == '\0') return;


	// Prepare the level name by stripping out the prefixes...

	char sLevel[128];
	char sTemp[128];
	strcpy(sLevel, sGameLevel);
	strcpy(sTemp, sGameLevel);

	int nLen = strlen(sTemp);

	if (nLen > 2)
	{
		int i = nLen - 1;

		while (i > 0 && sTemp[i] != '\\')
		{
			i--;
		}

		if (i < nLen - 1)
		{
			if (sTemp[i] == '\\') i++;
			strcpy(sLevel, &sTemp[i]);
		}
	}


	// Figure out if this is a custom or retail level...

	int nType = RETAIL_LEVEL;

	strupr(sTemp);
	if (!strstr(sTemp, "\\MULTI\\") && !strstr(sTemp, "\\MULTI_AO\\"))
	{
		nType = CUSTOM_LEVEL;
	}

#ifdef _ADDON
	if (strstr(sTemp, "\\MULTI_AO\\"))
	{
		nType = ADDON_LEVEL;
	}
#endif

#ifdef _DEMO
	nType = RETAIL_LEVEL;
#endif


	// Make sure it's ok to use this level for the game type...

	if (!NetStart_IsOkToDisplayLevel(sLevel))
	{
		return;
	}


	// Add the level to the list...

	int nIndex = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sLevel);
	if (nIndex != LB_ERR) SendMessage(hList, LB_SETITEMDATA, nIndex, nType);
}

void NetStart_LoadLevelList(HWND hList)
{
	// Sanity checks...

	if (!hList) return;


	// Get the level count...

	int cLevels = GetConsoleInt("NetNumLevels", 0);
	if (cLevels <= 0) return;


	// Get each level...

	for (int i = 0; i < cLevels; i++)
	{
		char sLevel[256] = { "" };
		char sLabel[32];

		wsprintf(sLabel, "NetLevel%i", i);
		GetConsoleString(sLabel, sLevel, "");

		NetStart_AddLevelToList(hList, sLevel);
	}
}

void NetStart_SaveLevelList(NetGame* pNetGame)
{
	// Sanity checks...

	if (!pNetGame) return;


	// Write out the level count...

	int cLevels = s_NetGame.m_byNumLevels;

	if (!s_bSaveLevels) cLevels = 0;

	WriteConsoleInt("NetNumLevels", cLevels);


	// Write out each level...

	for (int i = 0; i < cLevels; i++)
	{
		char sLabel[32];
		wsprintf(sLabel, "NetLevel%i", i);
		WriteConsoleString(sLabel, s_NetGame.m_sLevels[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GameLevels
//
//	PURPOSE:	Does the GAMELEVELS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_GameLevels(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hRetailList = NULL;
	static HWND hCustomList = NULL;
	static HWND hGameList   = NULL;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			hRetailList = GetDlgItem(hDlg, IDC_RETAILLIST);
			hCustomList = GetDlgItem(hDlg, IDC_CUSTOMLIST);
			hGameList   = GetDlgItem(hDlg, IDC_GAMELIST);

			CheckDlgButton(hDlg, IDC_SAVELEVELS, s_bSaveLevels ? BST_CHECKED : BST_UNCHECKED);

			NetStart_FillLevelList(hRetailList, "Worlds\\Multi", RETAIL_LEVEL);

#ifdef _ADDON
			NetStart_FillLevelList(hRetailList, "Worlds_ao\\Multi_ao", ADDON_LEVEL, FALSE);
#endif

#ifdef _DEMO
			EnableWindow(hCustomList, FALSE);
#else
			NetStart_FillLevelList(hCustomList, "\\", CUSTOM_LEVEL);
#endif
			NetStart_LoadLevelList(hGameList);

			UpdateGameLevelControls(hDlg);

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				s_bSaveLevels = IsDlgButtonChecked(hDlg, IDC_SAVELEVELS) == BST_CHECKED;
				NetStart_FillGameLevels(hDlg);
				NetStart_HostSession2(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
			else if (wParam == IDC_ADDRETAIL)
			{
				AddSelToList(hRetailList, hGameList);
				UpdateGameLevelControls(hDlg);
				return(TRUE);
			}
			else if (wParam == IDC_ADDCUSTOM)
			{
				AddSelToList(hCustomList, hGameList);
				UpdateGameLevelControls(hDlg);
				return(TRUE);
			}
			else if (wParam == IDC_REMOVE)
			{
				RemoveSelFromList(hGameList);
				UpdateGameLevelControls(hDlg);
				return(TRUE);
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				UpdateGameLevelControls(hDlg);
				return(TRUE);
			}
			else if (HIWORD(wParam) == LBN_DBLCLK)
			{
				switch(LOWORD(wParam))
				{
					case IDC_RETAILLIST: PostMessage(hDlg, WM_COMMAND, IDC_ADDRETAIL, 0); break;
					case IDC_CUSTOMLIST: PostMessage(hDlg, WM_COMMAND, IDC_ADDCUSTOM, 0); break;
					case IDC_GAMELIST:   PostMessage(hDlg, WM_COMMAND, IDC_REMOVE, 0); break;
				}
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL AddSelToList(HWND hSrcList, HWND hDestList)
{
	// Sanity checks...

	if (!hSrcList) return(FALSE);
	if (!hDestList) return(FALSE);


	// Get the select item...

	char sText[256];
	int  nData;

	int nIndex = SendMessage(hSrcList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);

	int nRet = SendMessage(hSrcList, LB_GETTEXT, nIndex, (LPARAM)(LPSTR)sText);
	if (nRet == LB_ERR) return(FALSE);
	if (strlen(sText) <= 0) return(FALSE);

	nData = SendMessage(hSrcList, LB_GETITEMDATA, nIndex, 0);
	if (nData == LB_ERR) return(FALSE);


	// Add the select stuff to the destination list...

	nIndex = SendMessage(hDestList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sText);
	if (nIndex == LB_ERR) return(FALSE);

	SendMessage(hDestList, LB_SETITEMDATA, nIndex, nData);


	// All done...

	return(TRUE);
}

BOOL RemoveSelFromList(HWND hList)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Remove the currently selected item...

	int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);

	int nRet = SendMessage(hList, LB_DELETESTRING, nIndex, 0);
	if (nRet == LB_ERR) return(FALSE);


	// Keep something selected if possible...

	nRet = SendMessage(hList, LB_SETCURSEL, nIndex, 0);
	if (nRet == LB_ERR && nIndex > 0)
	{
		SendMessage(hList, LB_SETCURSEL, nIndex-1, 0);
	}


	// All done...

	return(TRUE);
}

void UpdateGameLevelControls(HWND hDlg)
{
	// Get the list boxes...

	HWND hRetailList = GetDlgItem(hDlg, IDC_RETAILLIST);
	HWND hCustomList = GetDlgItem(hDlg, IDC_CUSTOMLIST);
	HWND hGameList   = GetDlgItem(hDlg, IDC_GAMELIST);


	// Update the buttons...

	int nRet = SendMessage(hRetailList, LB_GETCURSEL, 0, 0);
	EnableWindow(GetDlgItem(hDlg, IDC_ADDRETAIL), nRet != LB_ERR);

	nRet = SendMessage(hCustomList, LB_GETCURSEL, 0, 0);
	EnableWindow(GetDlgItem(hDlg, IDC_ADDCUSTOM), nRet != LB_ERR);

	nRet = SendMessage(hGameList, LB_GETCURSEL, 0, 0);
	EnableWindow(GetDlgItem(hDlg, IDC_REMOVE), nRet != LB_ERR);

	nRet = SendMessage(hGameList, LB_GETCOUNT, 0, 0);
	EnableWindow(GetDlgItem(hDlg, IDC_NEXT), nRet > 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillGameLevels
//
//	PURPOSE:	Fills the level names in the NetGame structure
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillGameLevels(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Add each level from the game list...

	s_sFirstLevel[0] = '\0';
	s_NetGame.m_byNumLevels = 0;
	for (int j = 0; j < MAX_GAME_LEVELS; j++) s_NetGame.m_sLevels[j][0] = '\0';

	HWND hList = GetDlgItem(hDlg, IDC_GAMELIST);
	if (!hList) return(FALSE);

	int nCount = SendMessage(hList, LB_GETCOUNT, 0, 0);
	if (nCount == LB_ERR) return(FALSE);
	if (nCount == 0) return(FALSE);

	if (nCount > MAX_GAME_LEVELS) nCount = MAX_GAME_LEVELS;

	for (int i = 0; i < nCount; i++)
	{
		char sLevel[128];
		char sText[128];

		int nRet = SendMessage(hList, LB_GETTEXT, i, (LPARAM)(LPSTR)sText);
		if (nRet != LB_ERR)
		{
			if (s_sFirstLevel[0] == '\0') strcpy(s_sFirstLevel, sText);

			int nData = SendMessage(hList, LB_GETITEMDATA, i, 0);

#ifdef _DEMO
			nData = RETAIL_LEVEL;
#endif

			if (nData == CUSTOM_LEVEL)
			{
				wsprintf(sLevel, "%s", sText);
			}
#ifdef _ADDON
			else if (nData == ADDON_LEVEL)
			{
				wsprintf(sLevel, "Worlds_ao\\Multi_ao\\%s", sText);
			}
#endif
			else
			{
				wsprintf(sLevel, "Worlds\\Multi\\%s", sText);
			}

			strcpy(s_NetGame.m_sLevels[s_NetGame.m_byNumLevels], sLevel);

			s_NetGame.m_byNumLevels++;
		}
	}


	// Set the net start level by using the first level in our list...

	strcpy(s_pNetStart->m_sLevel, s_NetGame.m_sLevels[0]);


	// Save the levels...

	NetStart_SaveLevelList(&s_NetGame);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GetConnectCommands
//
//	PURPOSE:	Gets any connection commands from the command-line stuff
//
// ----------------------------------------------------------------------- //

int NetStart_GetConnectCommands(CClientDE* pClient, NetStart* pNetStart)
{
	// Sanity checks...

	if (!pClient) return(CONNECT_FULL);
	if (!pNetStart) return(CONNECT_FULL);


	// Look for the "NetHost" command-line info...

	if (GetConsoleInt("NetHost", 0) > 0)
	{
		strcpy(pNetStart->m_sAddress, "");
		strcpy(pNetStart->m_sPlayer, s_sPlayerName);
		strcpy(s_NetPlayer.m_sName, s_sPlayerName);

		s_NetPlayer.m_dwLatency = s_nNetLatency;

		HCONSOLEVAR hVar = pClient->GetConsoleVar("NetPort");
		if (hVar)
		{
			g_HostPort = (int)pClient->GetVarValueFloat(hVar);
			g_HostPort = DCLAMP(g_HostPort, 0, 65534);
		}

		pNetStart->m_bHost          = DFALSE;
		pNetStart->m_bHaveTcpIp     = DTRUE;
		pNetStart->m_NetHost.m_Port = g_HostPort;

		return(CONNECT_TCPIP_HOST);
	}


	// Look for the "Connect" command-line info...

	HCONSOLEVAR hVar = pClient->GetConsoleVar("Connect");
	if (hVar)
	{
		char* sAddress = pClient->GetVarValueString(hVar);
		if (!sAddress) return(CONNECT_FULL);
		if (strcmp(sAddress, "0") == 0) return(CONNECT_FULL);

		strcpy(pNetStart->m_sAddress, sAddress);
		if (strcmp(sAddress, "*") == 0) strcpy(pNetStart->m_sAddress, "");

		strcpy(pNetStart->m_sPlayer, s_sPlayerName);

		pNetStart->m_bHost      = DFALSE;
		pNetStart->m_bHaveTcpIp = DTRUE;

		strcpy(s_NetPlayer.m_sName, s_sPlayerName);
		s_NetPlayer.m_dwLatency = s_nNetLatency;

		return(CONNECT_TCPIP_QUICK);
	}


	// Look for the "ConnectPlr" command-line info...

	hVar = pClient->GetConsoleVar("ConnectPlr");
	if (hVar)
	{
		char* sAddress = pClient->GetVarValueString(hVar);
		if (!sAddress) return(CONNECT_FULL);
		if (strcmp(sAddress, "0") == 0) return(CONNECT_FULL);

		strcpy(pNetStart->m_sAddress, sAddress);
		if (strcmp(sAddress, "*") == 0) strcpy(pNetStart->m_sAddress, "");

		pNetStart->m_bHost      = DFALSE;
		pNetStart->m_bHaveTcpIp = DTRUE;

		return(CONNECT_TCPIP);
	}


	// If we get here, just do a full connect...

	return(CONNECT_FULL);
}

void FixAddress(char* sAddress)
{
	if (!sAddress) return;

	while (*sAddress != '\0')
	{
		if (*sAddress == ':')
		{
			*sAddress = '\0';
			return;
		}

		sAddress++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_NoSessions
//
//	PURPOSE:	Does the NOSESSIONS dialog
//
// ----------------------------------------------------------------------- //

int DoNoSessionsMessage(HWND hParentWnd)
{
	int nRet = DialogBox(s_hInst, "NET_NOSESSIONS", hParentWnd, (DLGPROC)NetDlg_NoSessions);
	return(nRet);
}

BOOL CALLBACK NetDlg_NoSessions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case ID_YES:
				{
					s_bDontDisplayNoSessions = (IsDlgButtonChecked(hDlg, IDC_DONTDISPLAY) == BST_CHECKED);
					if (s_bDontDisplayNoSessions) WriteConsoleInt("DontDisplayNoSessions", s_bDontDisplayNoSessions);
					
					EndDialog(hDlg, ID_YES);
					return(TRUE);
				}

				case ID_NO:
				{
					s_bDontDisplayNoSessions = (IsDlgButtonChecked(hDlg, IDC_DONTDISPLAY) == BST_CHECKED);
					if (s_bDontDisplayNoSessions) WriteConsoleInt("DontDisplayNoSessions", s_bDontDisplayNoSessions);

					EndDialog(hDlg, ID_NO);
					return(TRUE);
				}
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoConsoleConnect
//
//	PURPOSE:	Connects to the given tcpip address but won't display
//				anything.
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_DoConsoleConnect(CClientDE* pClientDE, char* sAddress)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sAddress) return(DFALSE);


	// Set somne static info...

	s_pClient      = pClientDE;
	s_bLobbyLaunch = FALSE;


	// Tweak the address if necessary...

	if (strcmp(sAddress, "*") == 0) strcpy(sAddress, "");


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));


	// Init the engine's networking...

	DRESULT dr = pClientDE->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		return(DFALSE);
	}


	// Read some default values from the console...

	HSTRING hString = pClientDE->FormatString(IDS_NETWIZ_DEFAULTPLAYERNAME);

	if (hString)
	{
		_mbsncpy((unsigned char*)s_sPlayerName, (const unsigned char*)pClientDE->GetStringData(hString), 32);
		pClientDE->FreeString(hString);
	}
	else
	{
		strcpy(s_sPlayerName, "Caleb");
	}

	GetConsoleString("NetPlayerName", s_sPlayerName, s_sPlayerName);
	PostProcessName(s_sPlayerName);

	s_nNetLatency  = GetConsoleInt("UpdateRate", DEFAULT_UPDATERATE);


	// Get all the info we need...

	NetStart ns;

	ns.m_bHost      = DFALSE;
	ns.m_bHaveTcpIp = DTRUE;
	strcpy(ns.m_sPlayer, s_sPlayerName);
	strcpy(ns.m_sAddress, sAddress);

	strcpy(s_NetPlayer.m_sName, s_sPlayerName);
	s_NetPlayer.m_dwLatency = s_nNetLatency;

	memset(&s_NetGame, 0, sizeof(NetGame));
	memset(&s_NetClientData, 0, sizeof(NetClientData));


	// Start the game as requested...

	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	g_bIsHost = DFALSE;

	req.m_Type = STARTGAME_CLIENTTCP;

	strcpy(req.m_TCPAddress, ns.m_sAddress);

	req.m_pGameInfo     = NetStart_GetGameStruct();
	req.m_GameInfoLen   = sizeof(NetGame_t);
	req.m_pClientData   = NetStart_GetClientDataStruct();
	req.m_ClientDataLen = sizeof(NetClientData_t);

	dr = pClientDE->StartGame(&req);

	NetStart_FreeSessionList(pClientDE);

	if (dr != LT_OK)
	{
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Options
//
//	PURPOSE:	Does the OPTIONS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Options(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			NetStart_InitOptions(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillOptions(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
			else if (wParam == IDC_HELP_RUNSPEED)
			{
				DoOptionHelp(hDlg, IDS_HELP_RUNSPEED, IDS_TITLE_RUNSPEED);
				return(TRUE);
			}
			else if (wParam == IDC_HELP_MISSILESPEED)
			{
				DoOptionHelp(hDlg, IDS_HELP_MISSILESPEED, IDS_TITLE_MISSILESPEED);
				return(TRUE);
			}
			else if (wParam == IDC_HELP_TIMESPEED)
			{
				DoOptionHelp(hDlg, IDS_HELP_TIMESPEED, IDS_TITLE_TIMESPEED);
				return(TRUE);
			}
			else if (wParam == IDC_HELP_NIGHTCOLOR)
			{
				DoOptionHelp(hDlg, IDS_HELP_NIGHTCOLOR, IDS_TITLE_NIGHTCOLOR);
				return(TRUE);
			}

			return(TRUE);
		}
	}

	return(FALSE);
}

BOOL DoOptionHelp(HWND hParentWnd, int iMsg, int iTitle)
{
	if (!s_hInst) return(FALSE);

	char sMsg[256];
	if (LoadString(s_hInst, iMsg, sMsg, 250) == 0) return(FALSE);

	char sTitle[128];
	if (LoadString(s_hInst, iTitle, sTitle, 125) == 0) return(FALSE);

	MessageBox(hParentWnd, sMsg, sTitle, MB_OK | MB_ICONINFORMATION);

	return(TRUE);
}

BOOL NetStart_InitOptions(HWND hDlg)
{
	if (!hDlg) return(FALSE);
	if (!s_pServerOptions) return(FALSE);

	if (GetSelectedServiceFlags() & NETSERVICE_TCPIP)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_PORTNUMTEXT), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_PORTNUM), SW_SHOW);
		
		HCONSOLEVAR hVar = NULL;
		int startVal     = 0;

		if (s_pClient && (hVar = s_pClient->GetConsoleVar(LASTHOSTPORT_VARNAME)))
		{
			startVal = (int)s_pClient->GetVarValueFloat(hVar);
			startVal = DCLAMP(startVal, 0, 65534);
		}
		
		SetDlgItemInt(hDlg, IDC_PORTNUM, startVal, FALSE);
	}
	else
	{
		ShowWindow(GetDlgItem(hDlg, IDC_PORTNUMTEXT), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PORTNUM), SW_HIDE);
	}

	SetDlgItemFloat(hDlg, IDC_RUNSPEED,       s_pServerOptions->m_fRunSpeed);
	SetDlgItemFloat(hDlg, IDC_MISSILESPEED,   s_pServerOptions->m_fMissileSpeed);

	SetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor);

	float fTime = s_pServerOptions->m_fWorldTimeSpeed;
	SetDlgItemFloat(hDlg, IDC_TIMESPEED, fTime);

	SetDlgItemInt(hDlg, IDC_MAXPLAYERS, s_nMaxPlayers, FALSE);

	return(TRUE);
}

void SetDlgItemFloat(HWND hDlg, int iCtrl, float fValue)
{
	char sBuf[64];
	sprintf(sBuf, "%f", fValue);
	SetDlgItemText(hDlg, iCtrl, sBuf);
}

float GetDlgItemFloat(HWND hDlg, int iCtrl)
{
	char sBuf[64];
	GetDlgItemText(hDlg, iCtrl, sBuf, 62);
	return((float)atof(sBuf));
}

BOOL NetStart_FillOptions(HWND hDlg)
{
	if (!hDlg) return(FALSE);
	if (!s_pServerOptions) return(FALSE);

	s_pServerOptions->m_fRunSpeed       = GetDlgItemFloat(hDlg, IDC_RUNSPEED);
	s_pServerOptions->m_fMissileSpeed   = GetDlgItemFloat(hDlg, IDC_MISSILESPEED);

	GetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor, 30);

	BOOL bSuccess;
	if (GetSelectedServiceFlags() & NETSERVICE_TCPIP)
	{
		g_HostPort = (DDWORD)GetDlgItemInt(hDlg, IDC_PORTNUM, &bSuccess, FALSE);
		if(!bSuccess) g_HostPort = 0;
	}

	char sTemp[64];
	sprintf(sTemp, "+%s %d", LASTHOSTPORT_VARNAME, g_HostPort);
	s_pClient->RunConsoleString(sTemp);

	float fTime = GetDlgItemFloat(hDlg, IDC_TIMESPEED);
	if (fTime != 0) 
		s_pServerOptions->m_fWorldTimeSpeed = fTime;

	s_nMaxPlayers = GetDlgItemInt(hDlg, IDC_MAXPLAYERS, NULL, FALSE);
	if (s_nMaxPlayers <= 0) s_nMaxPlayers = 2;
	if (s_nMaxPlayers >= MAX_MULTI_PLAYERS) s_nMaxPlayers = MAX_MULTI_PLAYERS;

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_RunServerOptions
//
//	PURPOSE:	Runs all of the server options
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_RunServerOptions(CClientDE* pClientDE)
{
	return(NetStart_RunServerOptions(pClientDE, &g_ServerOptions));
}

DBOOL NetStart_RunServerOptions(CClientDE* pClientDE, ServerOptions* pServerOptions)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!pServerOptions) return(DFALSE);

	return(DTRUE);	// no more server options for now


	// Run the speed options...

	char sBuf[128];

	if (pServerOptions->m_fRunSpeed != 0)
	{
		sprintf(sBuf, "serv RunSpeed %f", pServerOptions->m_fRunSpeed);
		pClientDE->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_fMissileSpeed != 0)
	{
		sprintf(sBuf, "serv MissileSpeed %f", pServerOptions->m_fMissileSpeed);
		pClientDE->RunConsoleString(sBuf);
	}


	// Run the world options...

	if (pServerOptions->m_fWorldTimeSpeed != 0)
	{
		sprintf(sBuf, "serv WorldTimeSpeed %f", pServerOptions->m_fWorldTimeSpeed);
		pClientDE->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_sWorldNightColor[0] != '\0')
	{
		sprintf(sBuf, "serv WorldNightColor %s", pServerOptions->m_sWorldNightColor);
		pClientDE->RunConsoleString(sBuf);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FreeSessionList
//
//	PURPOSE:	Frees the session list
//
// ----------------------------------------------------------------------- //

void NetStart_FreeSessionList(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return;
	if (!s_pSessionList) return;


	// Free the Session list...

	s_pClient->FreeSessionList(s_pSessionList);

	s_pSessionList = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GetSessionList
//
//	PURPOSE:	Gets the session list
//
// ----------------------------------------------------------------------- //

NetSession* NetStart_GetSessionList(CClientDE* pClientDE, char* pInfo)
{
	// Sanity checks...

	if (!pClientDE) return(NULL);
	if (!pInfo) return(NULL);


	// Free any previous session list...

	NetStart_FreeSessionList(pClientDE);


	// Get the session list...

	DRESULT dr = s_pClient->GetSessionList(s_pSessionList, pInfo);
	if (dr != LT_OK) return(NULL);


	// All done...

	return(s_pSessionList);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoSettingsDialog
//
//	PURPOSE:	Does the settings dialog for a auto-launched game
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_DoSettingsDialog()
{
	// Sanity checks...

	if (!s_hInst) return(DFALSE);


	// Check if we should do the dialog...

	if (s_bHaveSettingInfo && s_bDontDisplaySettingsDialog && !IsKeyDown(VK_SHIFT))
	{
		return(DTRUE);
	}


	// Do the settings dialog...

	int nRet = DialogBox(s_hInst, "NET_SETTINGS", NULL, (DLGPROC)NetDlg_Settings);
	if (nRet == 0) return(DFALSE);


	// Update our console flags as necessary...

	if (s_bDontDisplaySettingsDialog)
	{
		WriteConsoleInt("DontDisplaySettingsDialog", 1);
	}

	WriteConsoleInt("HaveSettingInfo", 1);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_Settings
//
//	PURPOSE:	Does the SETTINGS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_Settings(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			SetDlgItemText(hDlg, IDC_NAME, s_sPlayerName);

			CheckDlgButton(hDlg, IDC_DONTDISPLAY, s_bDontDisplaySettingsDialog ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hDlg, IDC_TEAM1, s_nNetTeam == TEAM_1 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_TEAM2, s_nNetTeam == TEAM_2 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_AUTOTEAM, s_nNetTeam == TEAM_AUTO ? BST_CHECKED : BST_UNCHECKED);

			NetStart_InitConfigCombo(hDlg);
			NetStart_FillNetSpeed(hDlg);
			NetStart_InitFpsLimit(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (wParam == IDCANCEL)
			{
				EndDialog(hDlg, 0);
				return(TRUE);
			}
			else if (wParam == IDOK)
			{
				GetDlgItemText(hDlg, IDC_NAME, s_pNetStart->m_sPlayer, 60);
				if (strlen(s_pNetStart->m_sPlayer) <= 0) strcpy(s_pNetStart->m_sPlayer, "Caleb");
				PostProcessName(s_pNetStart->m_sPlayer);

				BOOL bOld = s_bDontDisplaySettingsDialog;
				s_bDontDisplaySettingsDialog = IsDlgButtonChecked(hDlg, IDC_DONTDISPLAY) == BST_CHECKED;
				WriteConsoleInt("DontDisplaySettingsDialog", s_bDontDisplaySettingsDialog);
				if (!bOld && s_bDontDisplaySettingsDialog) DoOptionHelp(hDlg, IDS_DONTDISPLAY_INFO, IDS_DONTDISPLAY_TITLE);

				NetStart_FillPlayerStruct(hDlg, TRUE);
				NetStart_FillFpsLimit(hDlg);
				NetStart_SelectNetSpeed(GetDlgItem(hDlg, IDC_NETCONNECTION));

				EndDialog(hDlg, 1);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL NetStart_InitConfigCombo(HWND hDlg)
{
	// Init the config manager...

	if (!s_ConfigMgr.Init("Players"))
	{
		return(FALSE);
	}


	// Fill the list of .b2c files...

	HWND hCombo = GetDlgItem(hDlg, IDC_CONFIGS);
	if (!hCombo) return(FALSE);

	s_ConfigMgr.FillComboBox(hCombo, s_sNetConfigFile, "Caleb.b2c");


	// All done...

	return(TRUE);
}

void NetStart_InitFpsLimit(HWND hDlg)
{
	if (!hDlg) return;

	CheckDlgButton(hDlg, IDC_LIMIT, s_bLimitFps ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(hDlg, IDC_FPS, s_nFpsLimit, FALSE);
}

void NetStart_FillFpsLimit(HWND hDlg)
{
	if (!hDlg) return;

	s_bLimitFps = IsDlgButtonChecked(hDlg, IDC_LIMIT) == BST_CHECKED;
	s_nFpsLimit = GetDlgItemInt(hDlg, IDC_FPS, NULL, FALSE);

	if (!s_bLimitFps) WriteConsoleInt("MaxFps", 0);
	else WriteConsoleInt("MaxFps", s_nFpsLimit);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_SessionsTcpIp
//
//	PURPOSE:	Does the SESSIONS_TCPIP dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_SessionsTcpIp(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bFirstUpdate  = TRUE;
	static BOOL bRestartQuery = TRUE;
	static BOOL bInTimer      = FALSE;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			bFirstUpdate  = TRUE;
			bRestartQuery = TRUE;
			bInTimer      = FALSE;

			ForceActiveFocus(hDlg);

			NetStart_ClearSelectedSessionInfo(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_NEXT), FALSE);
			int nTabs[10] = {76, 82, 84, 86, 88, 90, 92, 94, 96, 98 };
			SendMessage(GetDlgItem(hDlg, IDC_PLAYERLIST), LB_SETTABSTOPS, 10, (LPARAM)&nTabs);

			s_IpMgr.Init(s_pClient);
			s_IpMgr.ReadIps();
			s_IpMgr.FillListBox(GetDlgItem(hDlg, IDC_IPLIST));

			SetTimer(hDlg, TIMER_UPDATEDIALOG, 100, NULL);

			return(TRUE);
		}

		case WM_TIMER:
		{
			if (bInTimer) return(TRUE);
			bInTimer = TRUE;

			if (wParam == TIMER_UPDATEQUERY)
			{
				if (bRestartQuery)
				{
					NetStart_StartSessionQuery(s_pClient, &s_IpMgr);
					bRestartQuery = FALSE;
				}
				NetStart_UpdateSessionQuery(s_pClient);
				bInTimer = FALSE;
				return(TRUE);
			}

			KillTimer(hDlg, TIMER_UPDATEDIALOG);

			if (bFirstUpdate)
			{
				// MD: set it to always broadcast (by popular demand).
				//if (s_IpMgr.GetNumIps() <= 0) NetStart_DoTcpIpDialog(s_hInst, hDlg);
				
				if (s_IpMgr.GetNumIps() > 0) s_IpMgr.FillListBox(GetDlgItem(hDlg, IDC_IPLIST));
				SetTimer(hDlg, TIMER_UPDATEQUERY, 50, NULL);
			}

			if (bRestartQuery)
			{
				NetStart_StartSessionQuery(s_pClient, &s_IpMgr);
			}

			NetStart_UpdateSessionQuery(s_pClient);

			int c = NetStart_FillSessionListTcpIp(hDlg);
			if (c == 0) NetStart_ClearSelectedSessionInfo(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_NEXT), c > 0);

			if (bFirstUpdate) SetTimer(hDlg, TIMER_UPDATEDIALOG, 500, NULL);
			else SetTimer(hDlg, TIMER_UPDATEDIALOG, 3000, NULL);

			bFirstUpdate = FALSE;
			bRestartQuery = FALSE;
			bInTimer = FALSE;
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				KillTimer(hDlg, TIMER_UPDATEQUERY);
				KillTimer(hDlg, TIMER_UPDATEDIALOG);
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				if (!NetStart_JoinCurrentSession(GetDlgItem(hDlg, IDC_SESSIONLIST)))
				{
					s_nErrorString = IDS_NETERR_JOINSESSION;
					EndDialog(hDlg, DLG_ERROR);
				}
				else
				{
					EndDialog(hDlg, DLG_NEXT);
				}
				KillTimer(hDlg, TIMER_UPDATEQUERY);
				KillTimer(hDlg, TIMER_UPDATEDIALOG);
				return(TRUE);
			}
			else if (wParam == IDC_ADD)
			{
				char newIP[256];

				g_DlgIP    = newIP;
				g_DlgIPLen = sizeof(newIP);

				if (DialogBox(s_hInst, "NET_ADDIP", hDlg, (DLGPROC)AddIPDlgProc) == IDOK)
				{
					if (s_IpMgr.AddIp(newIP))
					{
						s_IpMgr.FillListBox(GetDlgItem(hDlg, IDC_IPLIST));
						bRestartQuery = TRUE;
					}
				}

				g_DlgIP = NULL;
				
				return(TRUE);
			}
			else if (wParam == IDC_REMOVE)
			{
				if (s_IpMgr.RemoveSelectedIpFromListBox(GetDlgItem(hDlg, IDC_IPLIST)))
				{
					bRestartQuery = TRUE;
				}
				return(TRUE);
			}
			else if (wParam == IDC_REMOVEALL)
			{
				s_IpMgr.RemoveAll();
				s_IpMgr.FillListBox(GetDlgItem(hDlg, IDC_IPLIST));
				bRestartQuery = TRUE;
				return(TRUE);
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				if (LOWORD(wParam) == IDC_SESSIONLIST)
				{
					if (!NetStart_DisplaySelectedSessionInfo(hDlg))
					{
						NetStart_ClearSelectedSessionInfo(hDlg);
					}
				}
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_StartSessionQuery
//
//	PURPOSE:	Starts a new session query
//
// ----------------------------------------------------------------------- //

DBOOL NetStart_StartSessionQuery(CClientDE* pClientDE, char* sInfo)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Terminate any previous query...

	NetStart_EndSessionQuery(pClientDE);


	// Start the new query...

	pClientDE->StartQuery(sInfo);


	// All done...

	return(DTRUE);
}

DBOOL NetStart_StartSessionQuery(CClientDE* pClientDE, CIpMgr* pIpMgr)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!pIpMgr) return(DFALSE);


	// Terminate any previous query...

	NetStart_EndSessionQuery(pClientDE);


	// Get the string of ips for search on...

	char sBuf[2048];

	if (!pIpMgr->GetAllIpString(sBuf, 2040))
	{
		return(DFALSE);
	}

	if((sizeof(sBuf) - strlen(sBuf)) > 10)
	{
		// If there's room, automatically broadcast.
		strcat(sBuf, ";*");
	}


	// Start the new query...

	pClientDE->StartQuery(sBuf);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_EndSessionQuery
//
//	PURPOSE:	Ends an existing session query
//
// ----------------------------------------------------------------------- //

void NetStart_EndSessionQuery(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return;


	// End the session query...

	pClientDE->EndQuery();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_UpdateSessionQuery
//
//	PURPOSE:	Updates the existing session query
//
// ----------------------------------------------------------------------- //

void NetStart_UpdateSessionQuery(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return;


	// End the session query...

	pClientDE->UpdateQuery();
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GetSessionQueryResults
//
//	PURPOSE:	Gets the current results from the existing session query
//
// ----------------------------------------------------------------------- //

NetSession* NetStart_GetSessionQueryResults(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(NULL);


	// Free the existing session list...

	NetStart_FreeSessionList(pClientDE);


	// End the session query...

	pClientDE->GetQueryResults(s_pSessionList);


	// All done...

	return(s_pSessionList);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillSessionListTcpIp
//
//	PURPOSE:	Fills a list box with the available tcpip sessions
//
// ----------------------------------------------------------------------- //

int NetStart_FillSessionListTcpIp(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(0);


	// Get our list box...

	HWND hList = GetDlgItem(hDlg, IDC_SESSIONLIST);
	if (!hList) return(0);


	// Get the currently selected session (if any) so we can restore the selection...

	DWORD dwSelCRC = 0;

	int iSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (iSel != LB_ERR)
	{
		int nRet = SendMessage(hList, LB_GETITEMDATA, iSel, 0);
		if (nRet != LB_ERR)
		{
			NetSession* pNetSession = (NetSession*)nRet;
			CNinfoGame* pGame = s_NinfoMgr.GetGame(pNetSession);
			if (pGame) dwSelCRC = pGame->GetCRC();
		}
	}


	// Get the session list...

	NetSession* pCur      = NULL;
	NetSession* pListHead = NULL;

	pListHead = NetStart_GetSessionQueryResults(s_pClient);


	// Remove the current net games from the manager...

	s_NinfoMgr.RemoveGames();


	// Check for no games...

	if (!pListHead)
	{
		SendMessage(hList, LB_RESETCONTENT, 0, 0);
		return(0);
	}


	// Add each Session to the list box...

	int c = 0;

	SET_NODRAW(hList);
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	for (pCur=pListHead; pCur; pCur=pCur->m_pNext)
	{
		// Add a new net game via the manager...

		CNinfoGame* pGame = s_NinfoMgr.AddGame(pCur->m_sName, pCur, pCur->m_dwCurPlayers);
		if (pGame)
		{
			int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pGame->GetNameWithPing());
			if (iItem != LB_ERR)
			{
				c++;

				SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pGame->GetSessionPointer());
			}
		}
	}


	// Select the appropriate session in the list...

	iSel = 0;

	for (DWORD i = 0; i < (DWORD)c; i++)
	{
		int nRet = SendMessage(hList, LB_GETITEMDATA, i, 0);
		if (nRet != LB_ERR)
		{
			NetSession* pNetSession = (NetSession*)nRet;
			CNinfoGame* pGame = s_NinfoMgr.GetGame(pNetSession);
			if (pGame)
			{
				if (pGame->GetCRC() == dwSelCRC)
				{
					iSel = i;
					i    = c;
				}
			}
		}
	}

	SendMessage(hList, LB_SETCURSEL, iSel, 0);
	SET_REDRAW(hList);


	// Update the selected session info...

	if (!NetStart_DisplaySelectedSessionInfo(hDlg))
	{
		NetStart_ClearSelectedSessionInfo(hDlg);
	}


	// All done...

	return(c);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoTcpIpDialog
//
//	PURPOSE:	Does the dialog that asks the user for a tcpip address
//
// ----------------------------------------------------------------------- //

BOOL NetStart_DoTcpIpDialog(HINSTANCE hInst, HWND hParentWnd)
{
	// Sanity checks...

	if (!hInst) return(FALSE);


	// Do the dialog...

	DialogBox(s_hInst, "NET_TCPIPADDRESS", hParentWnd, (DLGPROC)IPDlgProc);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_HostTcpIp
//
//	PURPOSE:	Hosts a game on tcp/ip
//
// ----------------------------------------------------------------------- //

BOOL NetStart_HostTcpIp(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(FALSE);


	// Get the service list...

	NetService* pCur      = NULL;
	NetService* pListHead = NULL;

	DRESULT dr = s_pClient->GetServiceList(pListHead);
	if (dr != LT_OK) return(FALSE);
	if (!pListHead)	return(FALSE);


	// Look for the tcpip service...

	HNETSERVICE hNetService = NULL;
	DWORD		iCurInfo    = 0;

	for (pCur = pListHead; pCur; )
	{
		char sTmp[128];
		strncpy(sTmp, pCur->m_sName, 120);
		strupr(sTmp);

		if (strstr(sTmp, "TCP"))
		{
			hNetService = pCur->m_handle;
		}

		//g_ServiceHandles[iCurInfo] = pCur->m_handle;
		//g_ServiceFlags[iCurInfo]   = pCur->m_dwFlags;
		iCurInfo++;

		pCur = pCur->m_pNext;
	}


	// Free the service list...

	s_pClient->FreeServiceList(pListHead);


	// Select the service...

	if (!hNetService) return(FALSE);

	dr = s_pClient->SelectService(hNetService);
	if (dr != LT_OK) return(FALSE);

	g_SelectedService = hNetService;


	// Flag that we are the host...

	s_pNetStart->m_bHost = TRUE;
	s_nNetJoinHost       = NET_HOST;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GameItems
//
//	PURPOSE:	Does the GAMEITEMS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_GameItems(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			NetStart_InitGameItems(hDlg);

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillGameItems(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL NetStart_InitGameItems(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	SetItemLevel(hDlg, IDC_AMMO_LEVEL, s_nNetAmmoLevel);
	SetItemLevel(hDlg, IDC_ARMOR_LEVEL, s_nNetArmorLevel);
	SetItemLevel(hDlg, IDC_HEALTH_LEVEL, s_nNetHealthLevel);
	SetItemLevel(hDlg, IDC_POWERUPS_LEVEL, s_nNetPowerupsLevel);

	SetItemRespawn(hDlg, IDC_AMMO_RESPAWN, s_nNetAmmoRespawn);
	SetItemRespawn(hDlg, IDC_ARMOR_RESPAWN, s_nNetArmorRespawn);
	SetItemRespawn(hDlg, IDC_HEALTH_RESPAWN, s_nNetHealthRespawn);
	SetItemRespawn(hDlg, IDC_POWERUPS_RESPAWN, s_nNetPowerupsRespawn);

	return(TRUE);
}

BOOL NetStart_FillGameItems(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	s_nNetAmmoLevel     = GetItemLevel(hDlg, IDC_AMMO_LEVEL);
	s_nNetArmorLevel    = GetItemLevel(hDlg, IDC_ARMOR_LEVEL);
	s_nNetHealthLevel   = GetItemLevel(hDlg, IDC_HEALTH_LEVEL);
	s_nNetPowerupsLevel = GetItemLevel(hDlg, IDC_POWERUPS_LEVEL);

	s_nNetAmmoRespawn     = GetItemRespawn(hDlg, IDC_AMMO_RESPAWN);
	s_nNetArmorRespawn    = GetItemRespawn(hDlg, IDC_ARMOR_RESPAWN);
	s_nNetHealthRespawn   = GetItemRespawn(hDlg, IDC_HEALTH_RESPAWN);
	s_nNetPowerupsRespawn = GetItemRespawn(hDlg, IDC_POWERUPS_RESPAWN);

	if (s_nNetAmmoLevel >= 0)   s_NetGame.m_nAmmoLevel   = s_nNetAmmoLevel;
	if (s_nNetAmmoRespawn >= 0) s_NetGame.m_nAmmoRespawn = s_nNetAmmoRespawn;

	if (s_nNetArmorLevel >= 0)   s_NetGame.m_nArmorLevel   = s_nNetArmorLevel;
	if (s_nNetArmorRespawn >= 0) s_NetGame.m_nArmorRespawn = s_nNetArmorRespawn;

	if (s_nNetHealthLevel >= 0)   s_NetGame.m_nHealthLevel   = s_nNetHealthLevel;
	if (s_nNetHealthRespawn >= 0) s_NetGame.m_nHealthRespawn = s_nNetHealthRespawn;

	if (s_nNetPowerupsLevel >= 0)   s_NetGame.m_nPowerupsLevel   = s_nNetPowerupsLevel;
	if (s_nNetPowerupsRespawn >= 0) s_NetGame.m_nPowerupsRespawn = s_nNetPowerupsRespawn;

	return(TRUE);
}

void AddComboString(HWND hCombo, int nStringID)
{
	if (!hCombo) return;
	if (!s_hInst) return;

	char sTemp[128];
	if (LoadString(s_hInst, nStringID, sTemp, 125) <= 0) return;

	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)sTemp);
}

void SetItemLevel(HWND hDlg, UINT idControl, int nLevel)
{
	if (!hDlg) return;
	HWND hCombo = GetDlgItem(hDlg, idControl);
	if (!hCombo) return;

	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

	AddComboString(hCombo, IDS_NETWIZ_ITEMS_NONE);		// "None"
	AddComboString(hCombo, IDS_NETWIZ_ITEMS_HALF);		// "Half"
	AddComboString(hCombo, IDS_NETWIZ_ITEMS_NORMAL);	// "Normal"
	AddComboString(hCombo, IDS_NETWIZ_ITEMS_DOUBLE);	// "Double"
	AddComboString(hCombo, IDS_NETWIZ_ITEMS_INSANE);	// "Insane"

	SendMessage(hCombo, CB_SETCURSEL, nLevel, 0);
}

int GetItemLevel(HWND hDlg, UINT idControl)
{
	if (!hDlg) return(-1);
	HWND hCombo = GetDlgItem(hDlg, idControl);
	if (!hCombo) return(-1);

	return(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
}

void SetItemRespawn(HWND hDlg, UINT idControl, int nRespawn)
{
	if (!hDlg) return;

	SetDlgItemInt(hDlg, idControl, nRespawn, FALSE);
}

int GetItemRespawn(HWND hDlg, UINT idControl)
{
	if (!hDlg) return(-1);

	return(GetDlgItemInt(hDlg, idControl, NULL, FALSE));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GameItems
//
//	PURPOSE:	Does the GAMEITEMS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_GameDamage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			NetStart_InitGameDamage(hDlg);

			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillGameDamage(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL NetStart_InitGameDamage(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	SetHealingRate(hDlg, IDC_HEALRATE, s_nNetHealingRate);
	CheckDlgButton(hDlg, IDC_FALLDAMAGE, s_bNetFallDamage ? BST_CHECKED : BST_UNCHECKED);

	return(TRUE);
}

BOOL NetStart_FillGameDamage(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	s_nNetHealingRate = GetHealingRate(hDlg, IDC_HEALRATE);
	s_bNetFallDamage  = IsDlgButtonChecked(hDlg, IDC_FALLDAMAGE) == BST_CHECKED;

	s_NetGame.m_nHealingRate = s_nNetHealingRate;
	s_NetGame.m_bFallDamage  = s_bNetFallDamage;

	return(TRUE);
}

void SetHealingRate(HWND hDlg, UINT idControl, int nLevel)
{
	if (!hDlg) return;
	HWND hCombo = GetDlgItem(hDlg, idControl);
	if (!hCombo) return;

	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

	AddComboString(hCombo, IDS_NETWIZ_HEAL_NOHEALING);	// "No Healing"
	AddComboString(hCombo, IDS_NETWIZ_HEAL_REALLYSLOW);	// "Really Slow"
	AddComboString(hCombo, IDS_NETWIZ_HEAL_SLOW);		// "Slow"
	AddComboString(hCombo, IDS_NETWIZ_HEAL_NORMAL);		// "Normal"
	AddComboString(hCombo, IDS_NETWIZ_HEAL_FAST);		// "Fast"
	AddComboString(hCombo, IDS_NETWIZ_HEAL_REALLYFAST);	// "Really Fast"

	SendMessage(hCombo, CB_SETCURSEL, nLevel, 0);
}

int GetHealingRate(HWND hDlg, UINT idControl)
{
	if (!hDlg) return(-1);
	HWND hCombo = GetDlgItem(hDlg, idControl);
	if (!hCombo) return(-1);

	return(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_IsOkToDisplayLevel
//
//	PURPOSE:	Determines if it's ok to display the given level in the
//				level list.
//
// ----------------------------------------------------------------------- //

BOOL NetStart_IsOkToDisplayLevel(char* sLevel)
{
	// Sanity checks...

	if (!sLevel) return(FALSE);


	// Determine the prefix to look for based on the game type...

	char sPrefix[32] = { "BB" };

	switch (s_nGameType)
	{
		case NGT_DEATHMATCH:	strcpy(sPrefix, "BB"); break;
		case NGT_CAPTUREFLAG:	strcpy(sPrefix, "CTF"); break;
		case NGT_TEAMS:			strcpy(sPrefix, "BB"); break;
		case NGT_SOCCER:		strcpy(sPrefix, "SOC"); break;
	}


	// Check for the prefix we need...

	char sTemp[128];

	strncpy(sTemp, sLevel, 126);
	strupr(sTemp);

	if (strncmp(sTemp, sPrefix, strlen(sPrefix)) != 0)
	{
		return(FALSE);
	}


	// If we get here, it's ok to display this level...

	return(TRUE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_TeamOptions
//
//	PURPOSE:	Does the NET_OPTIONS_TEAMS dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_TeamOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			NetStart_InitTeamOptions(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillTeamOptions(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


BOOL NetStart_InitTeamOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Init the dialog controls...

	CheckDlgButton(hDlg, IDC_FRIENDLYFIRE, s_bNetFriendlyFire ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_NEGFRAGS, s_bNetNegTeamFrags ? BST_CHECKED : BST_UNCHECKED);


	// All done...

	return(TRUE);
}

BOOL NetStart_FillTeamOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Read the dialog controls...

	s_bNetFriendlyFire = IsDlgButtonChecked(hDlg, IDC_FRIENDLYFIRE) == BST_CHECKED;
	s_bNetNegTeamFrags = IsDlgButtonChecked(hDlg, IDC_NEGFRAGS) == BST_CHECKED;


	// Update the game strucutre...

	s_NetGame.m_bFriendlyFire = s_bNetFriendlyFire;
	s_NetGame.m_bNegTeamFrags = s_bNetNegTeamFrags;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_CtfOptions
//
//	PURPOSE:	Does the NET_OPTIONS_CTF dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_CtfOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			NetStart_InitCtfOptions(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillCtfOptions(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL NetStart_InitCtfOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Init the dialog controls...

	CheckDlgButton(hDlg, IDC_FRIENDLYFIRE, s_bNetFriendlyFire ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_NEGFRAGS, s_bNetNegTeamFrags ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_ONLYFLAGSCORES, s_bNetOnlyFlagScores ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_USETEAMSIZE, s_bNetUseTeamSize ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hDlg, IDC_FLAGVALUE, s_nNetFlagValue, FALSE);


	// All done...

	return(TRUE);
}

BOOL NetStart_FillCtfOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Read the dialog controls...

	s_bNetFriendlyFire = IsDlgButtonChecked(hDlg, IDC_FRIENDLYFIRE) == BST_CHECKED;
	s_bNetNegTeamFrags = IsDlgButtonChecked(hDlg, IDC_NEGFRAGS) == BST_CHECKED;

	s_bNetOnlyFlagScores = IsDlgButtonChecked(hDlg, IDC_ONLYFLAGSCORES) == BST_CHECKED;
	s_bNetUseTeamSize    = IsDlgButtonChecked(hDlg, IDC_USETEAMSIZE) == BST_CHECKED;

	s_nNetFlagValue = GetDlgItemInt(hDlg, IDC_FLAGVALUE, NULL, FALSE);


	// Update the game strucutre...

	s_NetGame.m_bFriendlyFire   = s_bNetFriendlyFire;
	s_NetGame.m_bNegTeamFrags   = s_bNetNegTeamFrags;
	s_NetGame.m_bOnlyFlagScores = s_bNetOnlyFlagScores;
	s_NetGame.m_bUseTeamSize    = s_bNetUseTeamSize;
	s_NetGame.m_nFlagValue      = s_nNetFlagValue;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_SoccerOptions
//
//	PURPOSE:	Does the NET_OPTIONS_SOCCER dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_SoccerOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);
			NetStart_InitSoccerOptions(hDlg);
			return(TRUE);
		}

		case WM_COMMAND:
		{
			if (NetStart_HandleDefaultDialogCommands(hDlg, wParam))
			{
				return(TRUE);
			}
			else if (wParam == IDC_NEXT)
			{
				NetStart_FillSoccerOptions(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

BOOL NetStart_InitSoccerOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Init the dialog controls...

	CheckDlgButton(hDlg, IDC_FRIENDLYFIRE, s_bNetFriendlyFire ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_NEGFRAGS, s_bNetNegTeamFrags ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_ONLYGOALSCORES, s_bNetOnlyGoalScores ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_USETEAMSIZE, s_bNetUseTeamSize ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_SKIN_SOCCER, (s_nNetSocBallSkin == SOCBALL_SKIN_SOCCER) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_SKIN_ZOMBIE, (s_nNetSocBallSkin == SOCBALL_SKIN_ZOMBIE) ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hDlg, IDC_GOALVALUE, s_nNetGoalValue, FALSE);


	// All done...

	return(TRUE);
}

BOOL NetStart_FillSoccerOptions(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Read the dialog controls...

	s_bNetFriendlyFire = IsDlgButtonChecked(hDlg, IDC_FRIENDLYFIRE) == BST_CHECKED;
	s_bNetNegTeamFrags = IsDlgButtonChecked(hDlg, IDC_NEGFRAGS) == BST_CHECKED;

	s_bNetOnlyGoalScores = IsDlgButtonChecked(hDlg, IDC_ONLYGOALSCORES) == BST_CHECKED;
	s_bNetUseTeamSize    = IsDlgButtonChecked(hDlg, IDC_USETEAMSIZE) == BST_CHECKED;

	if (IsDlgButtonChecked(hDlg, IDC_SKIN_SOCCER) == BST_CHECKED) s_nNetSocBallSkin = SOCBALL_SKIN_SOCCER;
	if (IsDlgButtonChecked(hDlg, IDC_SKIN_ZOMBIE) == BST_CHECKED) s_nNetSocBallSkin = SOCBALL_SKIN_ZOMBIE;

	s_nNetGoalValue = GetDlgItemInt(hDlg, IDC_GOALVALUE, NULL, FALSE);


	// Update the game strucutre...

	s_NetGame.m_bFriendlyFire   = s_bNetFriendlyFire;
	s_NetGame.m_bNegTeamFrags   = s_bNetNegTeamFrags;
	s_NetGame.m_bOnlyGoalScores = s_bNetOnlyGoalScores;
	s_NetGame.m_bUseTeamSize    = s_bNetUseTeamSize;
	s_NetGame.m_nGoalValue      = s_nNetGoalValue;
	s_NetGame.m_nSocBallSkin    = s_nNetSocBallSkin;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostProcessName
//
//	PURPOSE:	Gets rid of special characters
//
// ----------------------------------------------------------------------- //

void PostProcessName(char* sName)
{
	// Sanity checks...

	if (!sName) return;
	if (sName[0] == '\0') return;


	// Check for special characters...

	int i = 0;

	while (sName[i] != '\0')
	{
		if (sName[i] == '[') sName[i] = '(';
		if (sName[i] == ']') sName[i] = ')';
		if (sName[i] == '%') sName[i] = '#';

		i++;
	}
}

