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
#include "dplobby.h"
#include "ClientRes.h"


// Defines...

#define LASTHOSTPORT_VARNAME		"LastHostPort"

#define	DLG_NEXT			1
#define DLG_CANCEL			2
#define DLG_BACK			3
#define DLG_ERROR			4
							
#define RETAIL_LEVEL		0
#define CUSTOM_LEVEL		1
							
#define NET_JOIN			0
#define NET_HOST			1
							
#define CONNECT_FULL		0
#define CONNECT_TCPIP		1
#define CONNECT_TCPIP_QUICK	2
#define CONNECT_TCPIP_HOST	3

#define HIGHPING_UPDATERATE	3
#define MODEM_UPDATERATE	6
#define ISDN_UPDATERATE		10
#define LAN_UPDATERATE		20
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

static	ILTClient*		s_pClient        = NULL;
static	NetStart*		s_pNetStart      = NULL; 
static	ServerOptions*	s_pServerOptions = NULL;
static	HWND			s_hMainWnd       = NULL;
static	HINSTANCE		s_hInst          = NULL;
static	CNinfoMgr		s_NinfoMgr;
static	CIpMgr			s_IpMgr;


// These are here so we remember the flags for the service we're using.
static	HNETSERVICE		g_ServiceHandles[MAX_NETSTART_SERVICES];
static	uint32			g_ServiceFlags[MAX_NETSTART_SERVICES];
static	HNETSERVICE		g_SelectedService=0;

static  uint32			g_HostPort=0;
static	char			s_sSessionName[128] = { "" };
static	char			s_sPlayerName[32]   = { "" };
static	char			s_sFirstLevel[128]  = { "" };
static	int				s_nGameType         = 0;
static	int				s_nEndType          = 0;
static	int				s_nEndFrags         = 0;
static	int				s_nEndTime          = 0;
static	int				s_nPlayerColor      = 0;
static	int				s_nMech             = 0;
static	int				s_nNetService       = 0;
static	int				s_nNetJoinHost      = 0;
static	int				s_nNetLatency       = DEFAULT_UPDATERATE;
static	int				s_nMaxPlayers       = MAX_MULTI_PLAYERS_DISPLAY;
static	int				s_nFpsLimit         = 30;
static	int				s_nNumJoinRetries   = 0;
static	BOOL			s_bLimitFps         = FALSE;
static	BOOL			s_bSaveLevels       = TRUE;

static	NetPlayer		s_NetPlayer;
static	NetGame			s_NetGame;

static	int				s_nErrorString = IDS_NETERR_GENERIC;
static	BOOL			s_bLobbyLaunch = FALSE;

static	int				s_nConnectType = CONNECT_FULL;

static	BOOL			s_bDontDisplayNoSessions     = FALSE;
static	BOOL			s_bDontDisplaySettingsDialog = FALSE;
static	BOOL			s_bHaveSettingInfo           = FALSE;

static	RMode			s_RMode;
static	NetSession*		s_pSessionList = NULL;


// Externs...

extern void *g_hLTDLLInstance;


// Macros...

#ifdef _DEBUG
#define	DEBUG_MSG(s)	MessageBox(NULL, s, "Shogo Debug", MB_OK);
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

BOOL NetStart_FillLevelList(HWND hList, char* sDir, int nData = 0);
BOOL NetStart_FillPlayerStruct(HWND hDlg);
BOOL NetStart_FillGameStruct(HWND hDlg);
BOOL NetStart_FillGameLevels(HWND hDlg);
BOOL NetStart_FillGameEnd(HWND hDlg);
void NetStart_FillColorList(HWND hCombo);

BOOL NetStart_FillNetSpeed(HWND hDlg);
BOOL NetStart_SetEndGameInfo(HWND hDlg);

void NetStart_InitFpsLimit(HWND hDlg);
void NetStart_FillFpsLimit(HWND hDlg);

int  NetStart_GetConnectCommands(ILTClient* pClient, NetStart* pNetStart);
BOOL NetStart_DoTcpIpDialog(HINSTANCE hInst, HWND hParentWnd);

BOOL NetStart_InitOptions(HWND hDlg);
BOOL NetStart_FillOptions(HWND hDlg);

BOOL NetStart_HostTcpIp(ILTClient* pClientDE);

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
BOOL CALLBACK NetDlg_Settings(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ForceActiveFocus(HWND hWnd);

int    GetConsoleInt(char* sKey, int nDefault);
void   GetConsoleString(char* sKey, char* sDest, char* sDefault);
void   WriteConsoleString(char* sKey, char* sValue);
void   WriteConsoleInt(char* sKey, int nValue);
LTFLOAT GetConsoleLTFLOAT(char* sKey, LTFLOAT fDefault);
void   WriteConsoleLTFLOAT(char* sKey, LTFLOAT fValue);

BOOL AddSelToList(HWND hSrcList, HWND hDestList);
BOOL RemoveSelFromList(HWND hList);
void UpdateGameLevelControls(HWND hDlg);

int  AddComboInfo(HWND hComboBox, char* sText, DWORD dwValue);
int	 DoNoSessionsMessage(HWND hParentWnd);

void FixAddress(char* sAddress);

BOOL  DoOptionHelp(HWND hParentWnd, int iMsg, int iTitle);
void  SetDlgItemLTFLOAT(HWND hDlg, int iCtrl, LTFLOAT fValue);
LTFLOAT GetDlgItemLTFLOAT(HWND hDlg, int iCtrl);


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

LTBOOL NetStart_DoWizard(ILTClient* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(LTFALSE);


	// Let the main function do the real wizard work...

	NetStart ns;

	LTBOOL bRet = NetStart_DoWizard(pClientDE, &ns);
	if (!bRet) return(LTFALSE);


	// Start the game as requested...

	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	if (ns.m_bHost)
	{
		req.m_Type = STARTGAME_HOST;

		strcpy(req.m_WorldName, ns.m_sLevel);
		memcpy(&req.m_HostInfo, &ns.m_NetHost, sizeof(NetHost));
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
	}

	req.m_pGameInfo   = NetStart_GetGameStruct();
	req.m_GameInfoLen = sizeof(NetGame_t);

	LTRESULT dr = pClientDE->StartGame(&req);

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
		g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
		hInst = (HINSTANCE)hModule;
		if (!hInst) return (LTFALSE);
		
		if(dr == LT_CANTBINDTOPORT)
			s_nErrorString = IDS_NETERR_CANTBINDTOPORT;
		else if(dr == LT_REJECTED)
			s_nErrorString = IDS_NETERR_CONNECTIONREJECTED;
		else if (dr == LT_NOTSAMEGUID)
			s_nErrorString = IDS_NETERR_NOTSAMEGUID;
		else
			s_nErrorString = IDS_NETERR_GENERIC;
		
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// All done...

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoWizard
//
//	PURPOSE:	Starts the multiplayer wizard
//
// ----------------------------------------------------------------------- //

LTBOOL NetStart_DoWizard(ILTClient* pClient, NetStart* pNetStart)
{
	// Sanity checks...

	if (!pClient) return(LTFALSE);
	if (!pNetStart) return(LTFALSE);


	// Set our static pointers...

	s_pClient        = pClient;
	s_pNetStart      = pNetStart;
	s_pServerOptions = &g_ServerOptions;

	s_bLobbyLaunch = FALSE;


	// Init the net game info manager...

	s_NinfoMgr.Term();
	s_NinfoMgr.Init();


	// Clear the NetStart structure...

	memset(s_pNetStart, 0, sizeof(NetStart));


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));


	// Fill in the default server options...

	memset(s_pServerOptions, 0, sizeof(ServerOptions));

	s_pServerOptions->m_bTractorBeam    = TRUE;
	s_pServerOptions->m_bDoubleJump     = TRUE;
	s_pServerOptions->m_bRammingDamage  = TRUE;
	s_pServerOptions->m_fWorldTimeSpeed = -1.0f;
	s_pServerOptions->m_fRunSpeed       = 1.3f;
	s_pServerOptions->m_fMissileSpeed   = 1.0;
	s_pServerOptions->m_fRespawnScale   = 1.0;
	s_pServerOptions->m_fHealScale      = 1.0;

	strcpy(s_pServerOptions->m_sWorldNightColor, "0.5 0.5 0.5");


	// Get the module handle...

	HINSTANCE hInst;
	void* hModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
	hInst = (HINSTANCE)hModule;
	if (!hInst) return(NULL);

	s_hInst = hInst;


	// Init the engine's networking...

	LTRESULT dr = pClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Read some default values from the console...

	GetConsoleString("NetPlayerName", s_sPlayerName, "Sanjuro");

#ifdef _DEMO
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Shogo Demo");
#else
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Shogo");
#endif

	s_nGameType    = GetConsoleInt("NetGameType", NGT_DEATHMATCH);
	s_nEndType     = GetConsoleInt("NetGameEnd", NGE_FRAGS);
	s_nEndFrags    = GetConsoleInt("NetEndFrags", 25);
	s_nEndTime     = GetConsoleInt("NetEndTime", 15);
	s_nPlayerColor = GetConsoleInt("NetPlayerColor", NPC_DEFAULT);
	s_nMech        = GetConsoleInt("NetMech", NMT_ORDOG);
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

	s_pServerOptions->m_bTractorBeam    = GetConsoleInt("NetTractorBeam", s_pServerOptions->m_bTractorBeam);
	s_pServerOptions->m_bDoubleJump     = GetConsoleInt("NetDoubleJump", s_pServerOptions->m_bDoubleJump);
	s_pServerOptions->m_bRammingDamage  = GetConsoleInt("NetRammingDamage", s_pServerOptions->m_bRammingDamage);
	s_pServerOptions->m_fWorldTimeSpeed = GetConsoleLTFLOAT("NetWorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
	s_pServerOptions->m_fRunSpeed       = GetConsoleLTFLOAT("NetRunSpeed", s_pServerOptions->m_fRunSpeed);
	s_pServerOptions->m_fMissileSpeed   = GetConsoleLTFLOAT("NetMissileSpeed", s_pServerOptions->m_fMissileSpeed);
	s_pServerOptions->m_fRespawnScale   = GetConsoleLTFLOAT("NetRespawnScale", s_pServerOptions->m_fRespawnScale);
	s_pServerOptions->m_fHealScale      = GetConsoleLTFLOAT("NetHealScale", s_pServerOptions->m_fHealScale);

	GetConsoleString("NetWorldNightColor", s_pServerOptions->m_sWorldNightColor, s_pServerOptions->m_sWorldNightColor);


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
			return(LTFALSE);
		}

		goto FinishedDlg;
	}


	// Do the services dialog...

ServicesDlg:
	nRet = DialogBox(hInst, "NET_SERVICES", hParentWnd, (DLGPROC)NetDlg_Services);

	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Do the player dialog...

PlayerDlg:
	nRet = DialogBox(hInst, "NET_PLAYER", hParentWnd, (DLGPROC)NetDlg_Player);

	if (nRet == DLG_BACK) goto ServicesDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
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
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}

	goto FinishedDlg;


	// Do the game dialog (host only)...

GameDlg:
	nRet = DialogBox(hInst, "NET_GAME", hParentWnd, (DLGPROC)NetDlg_Game);

	if (nRet == DLG_BACK) goto PlayerDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Do the game server options dialog (host only)...

OptionsDlg:
	nRet = DialogBox(hInst, "NET_OPTIONS", hParentWnd, (DLGPROC)NetDlg_Options);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Do the game levels dialog (host only)...

	nRet = DialogBox(hInst, "NET_GAMELEVELS", hParentWnd, (DLGPROC)NetDlg_GameLevels);

	if (nRet == DLG_BACK) goto OptionsDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
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

		WriteConsoleInt("NetTractorBeam", s_pServerOptions->m_bTractorBeam);
		WriteConsoleInt("NetDoubleJump", s_pServerOptions->m_bDoubleJump);
		WriteConsoleInt("NetRammingDamage", s_pServerOptions->m_bRammingDamage);
		WriteConsoleLTFLOAT("NetWorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
		WriteConsoleLTFLOAT("NetRunSpeed", s_pServerOptions->m_fRunSpeed);
		WriteConsoleLTFLOAT("NetMissileSpeed", s_pServerOptions->m_fMissileSpeed);
		WriteConsoleLTFLOAT("NetRespawnScale", s_pServerOptions->m_fRespawnScale);
		WriteConsoleLTFLOAT("NetHealScale", s_pServerOptions->m_fHealScale);
		WriteConsoleString("NetWorldNightColor", s_pServerOptions->m_sWorldNightColor);
	}

	WriteConsoleString("NetPlayerName", s_sPlayerName);

	WriteConsoleInt("NetPlayerColor", s_nPlayerColor);
	WriteConsoleInt("NetMech", s_nMech);
	WriteConsoleInt("NetService", s_nNetService);
	WriteConsoleInt("NetJoinHost", s_nNetJoinHost);
	WriteConsoleInt("UpdateRate", s_nNetLatency);
	WriteConsoleInt("NetFpsLimit", s_nFpsLimit);
	WriteConsoleInt("NetLimitFps", s_bLimitFps);
	WriteConsoleInt("NetSaveLevels", s_bSaveLevels);


	// All done...

	return(LTTRUE);
}

NetPlayer* NetStart_GetPlayerStruct()
{
	return(&s_NetPlayer);
}

NetGame* NetStart_GetGameStruct()
{
	return(&s_NetGame);
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
			const char* sValue = s_pClient->GetVarValueString(hVar);
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
			LTFLOAT fValue = s_pClient->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

LTFLOAT GetConsoleLTFLOAT(char* sKey, LTFLOAT fDefault)
{
	if (s_pClient)
	{
		HCONSOLEVAR hVar = s_pClient->GetConsoleVar(sKey);
		if (hVar)
		{
			LTFLOAT fValue = s_pClient->GetVarValueFloat(hVar);
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

void WriteConsoleLTFLOAT(char* sKey, LTFLOAT fValue)
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
	MessageBox(NULL, sError, "Shogo Demo", MB_OK);
#else
	MessageBox(NULL, sError, "Shogo", MB_OK);
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoLobbyLaunchWizard
//
//	PURPOSE:	Does the net start wizard for a lobby launched game
//
// ----------------------------------------------------------------------- //

LTBOOL NetStart_DoLobbyLaunchWizard(ILTClient* pClient)
{
	// Sanity checks...

	if (!pClient) return(LTFALSE);


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

	s_pServerOptions->m_bTractorBeam    = TRUE;
	s_pServerOptions->m_bDoubleJump     = TRUE;
	s_pServerOptions->m_bRammingDamage  = TRUE;
	s_pServerOptions->m_fWorldTimeSpeed = -1.0f;
	s_pServerOptions->m_fRunSpeed       = 1.3f;
	s_pServerOptions->m_fMissileSpeed   = 1.0;
	s_pServerOptions->m_fRespawnScale   = 1.0;
	s_pServerOptions->m_fHealScale      = 1.0;

	strcpy(s_pServerOptions->m_sWorldNightColor, "0.5 0.5 0.5");


	// Init the net game info manager...

	s_NinfoMgr.Term();
	s_NinfoMgr.Init();


	// Clear the NetStart structure...

	memset(s_pNetStart, 0, sizeof(NetStart));


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));


	// Get the module handle...

	HINSTANCE hInst;

	void* hModule;
	g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
	hInst = (HINSTANCE)hModule;
	if (!hInst) return (LTFALSE);

	s_hInst = hInst;


	// Init the engine's networking...

	LTRESULT dr = pClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Get the lobby launch info...

	void* pLobbyLaunchInfo = NULL;

	dr = pClient->GetLobbyLaunchInfo("dplay2", &pLobbyLaunchInfo);
	if (dr != LT_OK)
	{
		s_nErrorString = IDS_NETERR_INIT;
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}

	LPDPLCONNECTION pDplConn = (LPDPLCONNECTION)pLobbyLaunchInfo;


	// Read some default values from the console...

	GetConsoleString("NetPlayerName", s_sPlayerName, "Sanjuro");

#ifdef _DEMO
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Shogo Demo");
#else
	GetConsoleString("NetSessionName", s_sSessionName, "Multiplayer Shogo");
#endif

	s_nGameType    = GetConsoleInt("NetGameType", NGT_DEATHMATCH);
	s_nEndType     = GetConsoleInt("NetGameEnd", NGE_FRAGS);
	s_nEndFrags    = GetConsoleInt("NetEndFrags", 25);
	s_nEndTime     = GetConsoleInt("NetEndTime", 15);
	s_nPlayerColor = GetConsoleInt("NetPlayerColor", NPC_DEFAULT);
	s_nMech        = GetConsoleInt("NetMech", NMT_ORDOG);
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


	// Set our join host info...

	if (pDplConn->dwFlags & DPLCONNECTION_CREATESESSION)
	{
		s_pNetStart->m_bHost = LTTRUE;
	}
	else
	{
		s_pNetStart->m_bHost = LTFALSE;
	}


	// Do the player dialog...

PlayerDlg:
	nRet = DialogBox(hInst, "NET_PLAYER", hParentWnd, (DLGPROC)NetDlg_Player);

	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}

	if (!s_pNetStart->m_bHost) goto Launch;


	// Do the game dialog (host only)...

GameDlg:
	nRet = DialogBox(hInst, "NET_GAME", hParentWnd, (DLGPROC)NetDlg_Game);

	if (nRet == DLG_BACK) goto PlayerDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
	}


	// Do the game levels dialog (host only)...

	nRet = DialogBox(hInst, "NET_GAMELEVELS", hParentWnd, (DLGPROC)NetDlg_GameLevels);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_CANCEL) return(LTFALSE);
	if (nRet == DLG_ERROR)
	{
		NetStart_DisplayError(hInst);
		return(LTFALSE);
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
		return(LTFALSE);
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

	WriteConsoleString("NetPlayerName", s_sPlayerName);

	WriteConsoleInt("NetPlayerColor", s_nPlayerColor);
	WriteConsoleInt("NetMech", s_nMech);
	WriteConsoleInt("NetLatency", s_nNetLatency);


	// Display a loading screen...

	uint32 cxLoading, cyLoading, cxScreen, cyScreen;
	HSURFACE hLoading = pClient->CreateSurfaceFromBitmap ("interface/loading.pcx");
	pClient->GetSurfaceDims (hLoading, &cxLoading, &cyLoading);
	HSURFACE hScreen = pClient->GetScreenSurface();
	pClient->GetSurfaceDims (hScreen, &cxScreen, &cyScreen);

	pClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
	pClient->Start3D();
	pClient->StartOptimized2D();
	pClient->DrawSurfaceToSurface(hScreen, hLoading, LTNULL, ((int)cxScreen - (int)cxLoading) / 2, ((int)cyScreen - (int)cyLoading) / 2);
	pClient->EndOptimized2D();
	pClient->End3D(END3D_CANDRAWCONSOLE);
	pClient->FlipScreen (0);


	// All done...

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_MinimizeMainWnd
//
//	PURPOSE:	Minimizes the main window
//
// ----------------------------------------------------------------------- //

LTBOOL NetStart_MinimizeMainWnd(ILTClient* pClient)
{
	// Sanity checks...

	if (!pClient) return(LTFALSE);


	// Get the window handle...

	s_hMainWnd = NULL;

	LTRESULT dr = pClient->GetEngineHook("HWND", (void**)&s_hMainWnd);
	if (dr != LT_OK || !s_hMainWnd)
	{
		s_hMainWnd = GetActiveWindow();
	}


	// Save the current render mode...

	pClient->GetRenderMode(&s_RMode);


	// Shutdown the renderer, minimize it, and hide it...

	pClient->ShutdownRender(RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW);


	// All done...

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_RestoreMainWnd
//
//	PURPOSE:	Restores the main window
//
// ----------------------------------------------------------------------- //

LTBOOL NetStart_RestoreMainWnd()
{
	// Sanity checks...

	if (!s_pClient) return(LTFALSE);


	// Restore the main window as necessary...

	LTRESULT dr = s_pClient->SetRenderMode(&s_RMode);
	if (dr != LT_OK) return(LTFALSE);


	// All done...

	return(LTTRUE);
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

	LTRESULT dr = s_pClient->GetServiceList(pListHead);
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

	sprintf(str, "High-Ping/Modem (%d)", HIGHPING_UPDATERATE);
	AddComboInfo(hComboBox, str, HIGHPING_UPDATERATE);
	
	sprintf(str, "28.8k-56k Modem (%d)", MODEM_UPDATERATE);
	AddComboInfo(hComboBox, str, MODEM_UPDATERATE);
	
	sprintf(str, "ISDN-T1 Internet (%d)", ISDN_UPDATERATE);
	AddComboInfo(hComboBox, str, ISDN_UPDATERATE);
	
	sprintf(str, "LAN Game (%d)", LAN_UPDATERATE);
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
		sprintf(str, "Custom (%d)", nVal);
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

	LTRESULT dr = s_pClient->SelectService(hNetService);
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

				CheckDlgButton(hDlg, IDC_JOIN, (s_nNetJoinHost == NET_JOIN));
				CheckDlgButton(hDlg, IDC_HOST, (s_nNetJoinHost == NET_HOST));

				if (!NetStart_FillNetSpeed(hDlg))
				{
					EndDialog(hDlg, DLG_ERROR);
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
	uint32 flags;
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

		CNinfoGame* pGame = s_NinfoMgr.AddGame(pCur->m_sName, pCur, pCur->m_dwCurConnections);
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
	if (!hList) return(LTFALSE);


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
	if (!hList) return(LTFALSE);

	SET_NODRAW(hList)
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	CNinfoPlayer* pPlayer = pGame->GetFirstPlayer();

	if (!pPlayer && cPlayers > 0)
	{
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)"(player info not sent)");
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
	if (!hList) return(LTFALSE);

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
#ifdef _DEMO
			wsprintf(sName, "Multiplayer Shogo Demo with %s", s_sPlayerName);
#else
			wsprintf(sName, "Multiplayer Shogo with %s", s_sPlayerName);
#endif
		}
		else
		{
#ifdef _DEMO
			strcpy(sName, "Multiplayer Shogo Demo");
#else
			strcpy(sName, "Multiplayer Shogo");
#endif
		}
	}

	strcpy(s_sSessionName, sName);


	// Get the game type...

	int nType = NGT_DEATHMATCH;

	if (IsDlgButtonChecked(hDlg, IDC_COOPERATIVE)) nType = NGT_COOPERATIVE;
	if (IsDlgButtonChecked(hDlg, IDC_CAPTURETHEFLAG)) nType = NGT_CAPTUREFLAG;

	s_nGameType = nType;


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

	s_pNetStart->m_NetHost.m_dwMaxConnections = s_nMaxPlayers;
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
	ILTClient *pClientDE = s_pClient;
	HCONSOLEVAR hVar;
	int startVal;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			ForceActiveFocus(hDlg);

			SetDlgItemText(hDlg, IDC_NAME, s_sSessionName);
			NetStart_SetEndGameInfo(hDlg);

			if (GetSelectedServiceFlags() & NETSERVICE_TCPIP)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PORTNUMTEXT), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_PORTNUM), SW_SHOW);
				
				startVal = 0;
				if(pClientDE && (hVar = pClientDE->GetConsoleVar(LASTHOSTPORT_VARNAME)))
				{
					startVal = (int)pClientDE->GetVarValueFloat(hVar);
					startVal = LTCLAMP(startVal, 0, 65534);
				}
				
				SetDlgItemInt(hDlg, IDC_PORTNUM, startVal, FALSE);
			}
			else
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PORTNUMTEXT), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_PORTNUM), SW_HIDE);
			}

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
	SetWindowText(hWnd, "Shogo Demo Multiplayer Wizard");
#endif

	SetActiveWindow(hWnd);
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
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
	ILTClient *pClientDE;

	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the game type...

	int nType = NGT_DEATHMATCH;

	if (IsDlgButtonChecked(hDlg, IDC_COOPERATIVE)) nType = NGT_COOPERATIVE;
	if (IsDlgButtonChecked(hDlg, IDC_CAPTURETHEFLAG)) nType = NGT_CAPTUREFLAG;
	s_NetGame.m_byType = nType;

	BOOL bSuccess;
	char sTemp[128];

	if (GetSelectedServiceFlags() & NETSERVICE_TCPIP)
	{
		g_HostPort = (uint32)GetDlgItemInt(hDlg, IDC_PORTNUM, &bSuccess, FALSE);
		if(!bSuccess) g_HostPort = 0;
	}


	// Remember the last value...

	if(pClientDE = s_pClient)
	{
		sprintf(sTemp, "+%s %d", LASTHOSTPORT_VARNAME, g_HostPort);
		pClientDE->RunConsoleString(sTemp);
	}


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
			SetDlgItemText(hDlg, IDC_NAME, s_sPlayerName);

#ifdef _DEMO
			CheckDlgButton(hDlg, IDC_ORDOG, BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_ENFORCER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_AKUMA), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PREDATOR), FALSE);
#else
			CheckDlgButton(hDlg, IDC_ORDOG,    (s_nMech == NMT_ORDOG)    ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_ENFORCER, (s_nMech == NMT_ENFORCER) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_AKUMA,    (s_nMech == NMT_AKUMA)    ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_PREDATOR, (s_nMech == NMT_PREDATOR) ? BST_CHECKED : BST_UNCHECKED);
#endif

			if (s_bLobbyLaunch && !s_pNetStart->m_bHost) SetDlgItemText(hDlg, IDC_NEXT, "&Finished");
			if (s_pNetStart->m_bHaveTcpIp) SetDlgItemText(hDlg, IDC_NEXT, "&Finished");

			NetStart_FillColorList(GetDlgItem(hDlg, IDC_COLOR));
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
				if (strlen(s_pNetStart->m_sPlayer) <= 0) strcpy(s_pNetStart->m_sPlayer, "Sanjuro");
				NetStart_FillPlayerStruct(hDlg);
				NetStart_FillFpsLimit(hDlg);
				NetStart_SelectNetSpeed(GetDlgItem(hDlg, IDC_NETCONNECTION));
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


#define ADDCOLOR(n, i)	{ int nIndex = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)n); if (nIndex != CB_ERR) SendMessage(hCombo, CB_SETITEMDATA, nIndex, i); }

void NetStart_FillColorList(HWND hCombo)
{
	if (!hCombo) return;
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

	ADDCOLOR("Black", NPC_BLACK);
	ADDCOLOR("White", NPC_WHITE);
	ADDCOLOR("Red", NPC_RED);
	ADDCOLOR("Green", NPC_GREEN);
	ADDCOLOR("Blue", NPC_BLUE);
	ADDCOLOR("Cyan", NPC_CYAN);
	ADDCOLOR("Yellow", NPC_YELLOW);
	ADDCOLOR("Purple", NPC_PURPLE);

	SendMessage(hCombo, CB_SETCURSEL, NPC_DEFAULT, 0);

	int nCount = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
	if (nCount == CB_ERR) return;

	for (int i = 0; i < nCount; i++)
	{
		int nColor = SendMessage(hCombo, CB_GETITEMDATA, i, 0);
		if (nColor != CB_ERR)
		{
			if (nColor == s_nPlayerColor)
			{
				SendMessage(hCombo, CB_SETCURSEL, i, 0);
				return;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillPlayerStruct
//
//	PURPOSE:	Fills the player struct with the player info from the dlg
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillPlayerStruct(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the player name...

	GetDlgItemText(hDlg, IDC_NAME, s_NetPlayer.m_sName, MAX_PLAYER_NAME-1);
	if (strlen(s_NetPlayer.m_sName) <= 0) strcpy(s_NetPlayer.m_sName, "Sanjuro");

	strcpy(s_sPlayerName, s_NetPlayer.m_sName);


	// Get the color...

	s_NetPlayer.m_byColor = NPC_DEFAULT;

	HWND hCombo = GetDlgItem(hDlg, IDC_COLOR);
	if (!hCombo) return(FALSE);

	int nIndex = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if (nIndex != CB_ERR)
	{
		DWORD dwColor = SendMessage(hCombo, CB_GETITEMDATA, nIndex, 0);
		if (dwColor != CB_ERR)
		{
			s_NetPlayer.m_byColor = (uint8)dwColor;
		}
	}

	s_nPlayerColor = s_NetPlayer.m_byColor;


	// Get the mech type...

	s_NetPlayer.m_byMech = NMT_ORDOG;

	if (IsDlgButtonChecked(hDlg, IDC_ORDOG) == BST_CHECKED) s_NetPlayer.m_byMech = NMT_ORDOG;
	else if (IsDlgButtonChecked(hDlg, IDC_ENFORCER) == BST_CHECKED) s_NetPlayer.m_byMech = NMT_ENFORCER;
	else if (IsDlgButtonChecked(hDlg, IDC_PREDATOR) == BST_CHECKED) s_NetPlayer.m_byMech = NMT_PREDATOR;
	else if (IsDlgButtonChecked(hDlg, IDC_AKUMA) == BST_CHECKED) s_NetPlayer.m_byMech = NMT_AKUMA;

	s_nMech = s_NetPlayer.m_byMech;


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

BOOL NetStart_FillLevelList(HWND hList, char* sDir, int nData)
{
	// Sanity checks...

	if (!hList) return(FALSE);
	if (!sDir) return(FALSE);


	// Get a list of world names and put them in the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);

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
				int nIndex = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)sLevel);
				if (nIndex != LB_ERR)
				{
					SendMessage(hList, LB_SETITEMDATA, nIndex, nData);
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


	// Get the max players...

	s_nMaxPlayers = GetDlgItemInt(hDlg, IDC_MAXPLAYERS, NULL, FALSE);
	if (s_nMaxPlayers <= 0) s_nMaxPlayers = 2;
	if (s_nMaxPlayers >= MAX_MULTI_PLAYERS) s_nMaxPlayers = MAX_MULTI_PLAYERS;


	// All done...

	return(TRUE);
}

BOOL NetStart_SetEndGameInfo(HWND hDlg)
{
	if (!hDlg) return(FALSE);

	SetDlgItemInt(hDlg, IDC_MAXPLAYERS, s_nMaxPlayers, FALSE);
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
	if (!strstr(sTemp, "\\MULTI\\"))
	{
		nType = CUSTOM_LEVEL;
	}

#ifdef _DEMO
	nType = RETAIL_LEVEL;
#endif


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

int NetStart_GetConnectCommands(ILTClient* pClient, NetStart* pNetStart)
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

		s_NetPlayer.m_byColor   = s_nPlayerColor;
		s_NetPlayer.m_byMech    = s_nMech;
		s_NetPlayer.m_dwLatency = s_nNetLatency;

		HCONSOLEVAR hVar = pClient->GetConsoleVar("NetPort");
		if (hVar)
		{
			g_HostPort = (int)pClient->GetVarValueFloat(hVar);
			g_HostPort = LTCLAMP(g_HostPort, 0, 65534);
		}

		pNetStart->m_bHost          = LTFALSE;
		pNetStart->m_bHaveTcpIp     = LTTRUE;
		pNetStart->m_NetHost.m_Port = g_HostPort;

		return(CONNECT_TCPIP_HOST);
	}


	// Look for the "Connect" command-line info...

	HCONSOLEVAR hVar = pClient->GetConsoleVar("Connect");
	if (hVar)
	{
		const char* sAddress = pClient->GetVarValueString(hVar);
		if (!sAddress) return(CONNECT_FULL);
		if (strcmp(sAddress, "0") == 0) return(CONNECT_FULL);

		strcpy(pNetStart->m_sAddress, sAddress);
		if (strcmp(sAddress, "*") == 0) strcpy(pNetStart->m_sAddress, "");

		strcpy(pNetStart->m_sPlayer, s_sPlayerName);
		//FixAddress(pNetStart->m_sAddress);

		pNetStart->m_bHost      = LTFALSE;
		pNetStart->m_bHaveTcpIp = LTTRUE;

		strcpy(s_NetPlayer.m_sName, s_sPlayerName);
		s_NetPlayer.m_byColor   = s_nPlayerColor;
		s_NetPlayer.m_byMech    = s_nMech;
		s_NetPlayer.m_dwLatency = s_nNetLatency;

		return(CONNECT_TCPIP_QUICK);
	}


	// Look for the "ConnectPlr" command-line info...

	hVar = pClient->GetConsoleVar("ConnectPlr");
	if (hVar)
	{
		const char* sAddress = pClient->GetVarValueString(hVar);
		if (!sAddress) return(CONNECT_FULL);
		if (strcmp(sAddress, "0") == 0) return(CONNECT_FULL);

		strcpy(pNetStart->m_sAddress, sAddress);
		if (strcmp(sAddress, "*") == 0) strcpy(pNetStart->m_sAddress, "");
		//FixAddress(pNetStart->m_sAddress);

		pNetStart->m_bHost      = LTFALSE;
		pNetStart->m_bHaveTcpIp = LTTRUE;

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

LTBOOL NetStart_DoConsoleConnect(ILTClient* pClientDE, char* sAddress)
{
	// Sanity checks...

	if (!pClientDE) return(LTFALSE);
	if (!sAddress) return(LTFALSE);


	// Set somne static info...

	s_pClient      = pClientDE;
	s_bLobbyLaunch = FALSE;


	// Tweak the address if necessary...

	if (strcmp(sAddress, "*") == 0) strcpy(sAddress, "");


	// Clear our static player and game structures...

	memset(&s_NetPlayer, 0, sizeof(NetPlayer));
	memset(&s_NetGame, 0, sizeof(NetGame));


	// Init the engine's networking...

	LTRESULT dr = pClientDE->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		return(LTFALSE);
	}


	// Read some default values from the console...

	GetConsoleString("NetPlayerName", s_sPlayerName, "Sanjuro");

	s_nPlayerColor = GetConsoleInt("NetPlayerColor", NPC_DEFAULT);
	s_nMech        = GetConsoleInt("NetMech", NMT_ORDOG);
	s_nNetLatency  = GetConsoleInt("UpdateRate", DEFAULT_UPDATERATE);


	// Get all the info we need...

	NetStart ns;

	ns.m_bHost      = LTFALSE;
	ns.m_bHaveTcpIp = LTTRUE;
	strcpy(ns.m_sPlayer, s_sPlayerName);
	strcpy(ns.m_sAddress, sAddress);

	strcpy(s_NetPlayer.m_sName, s_sPlayerName);
	s_NetPlayer.m_byColor   = s_nPlayerColor;
	s_NetPlayer.m_byMech    = s_nMech;
	s_NetPlayer.m_dwLatency = s_nNetLatency;


	// Start the game as requested...

	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	req.m_Type = STARTGAME_CLIENTTCP;

	strcpy(req.m_TCPAddress, ns.m_sAddress);

	dr = pClientDE->StartGame(&req);

	NetStart_FreeSessionList(pClientDE);

	if (dr != LT_OK)
	{
		return(LTFALSE);
	}


	// All done...

	return(LTTRUE);
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
			else if (wParam == IDC_HELP_RESPAWNSCALE)
			{
				DoOptionHelp(hDlg, IDS_HELP_RESPAWNSCALE, IDS_TITLE_RESPAWNSCALE);
				return(TRUE);
			}
			else if (wParam == IDC_HELP_HEALSCALE)
			{
				DoOptionHelp(hDlg, IDS_HELP_HEALSCALE, IDS_TITLE_HEALSCALE);
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

	CheckDlgButton(hDlg, IDC_TRACTORBEAM,   s_pServerOptions->m_bTractorBeam   ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_DOUBLEJUMP,    s_pServerOptions->m_bDoubleJump    ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_RAMMINGDAMAGE, s_pServerOptions->m_bRammingDamage ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemLTFLOAT(hDlg, IDC_RUNSPEED,       s_pServerOptions->m_fRunSpeed);
	SetDlgItemLTFLOAT(hDlg, IDC_MISSILESPEED,   s_pServerOptions->m_fMissileSpeed);
	SetDlgItemLTFLOAT(hDlg, IDC_HEALSCALE,      s_pServerOptions->m_fHealScale);
	SetDlgItemLTFLOAT(hDlg, IDC_RESPAWNSCALE,   s_pServerOptions->m_fRespawnScale);

	SetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor);

	LTFLOAT fTime = s_pServerOptions->m_fWorldTimeSpeed;
	//fTime = fTime / HOURS_PER_SEC;
	SetDlgItemLTFLOAT(hDlg, IDC_TIMESPEED, fTime);

	return(TRUE);
}

void SetDlgItemLTFLOAT(HWND hDlg, int iCtrl, LTFLOAT fValue)
{
	char sBuf[64];
	sprintf(sBuf, "%f", fValue);
	SetDlgItemText(hDlg, iCtrl, sBuf);
}

LTFLOAT GetDlgItemLTFLOAT(HWND hDlg, int iCtrl)
{
	char sBuf[64];
	GetDlgItemText(hDlg, iCtrl, sBuf, 62);
	return((LTFLOAT)atof(sBuf));
}

BOOL NetStart_FillOptions(HWND hDlg)
{
	if (!hDlg) return(FALSE);
	if (!s_pServerOptions) return(FALSE);

	s_pServerOptions->m_bTractorBeam   = IsDlgButtonChecked(hDlg, IDC_TRACTORBEAM) == BST_CHECKED;
	s_pServerOptions->m_bDoubleJump    = IsDlgButtonChecked(hDlg, IDC_DOUBLEJUMP) == BST_CHECKED;
	s_pServerOptions->m_bRammingDamage = IsDlgButtonChecked(hDlg, IDC_RAMMINGDAMAGE) == BST_CHECKED;

	s_pServerOptions->m_fRunSpeed       = GetDlgItemLTFLOAT(hDlg, IDC_RUNSPEED);
	s_pServerOptions->m_fMissileSpeed   = GetDlgItemLTFLOAT(hDlg, IDC_MISSILESPEED);
	s_pServerOptions->m_fRespawnScale   = GetDlgItemLTFLOAT(hDlg, IDC_RESPAWNSCALE);
	s_pServerOptions->m_fHealScale      = GetDlgItemLTFLOAT(hDlg, IDC_HEALSCALE);

	GetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor, 30);

	LTFLOAT fTime = GetDlgItemLTFLOAT(hDlg, IDC_TIMESPEED);
	if (fTime != 0) 
		s_pServerOptions->m_fWorldTimeSpeed = fTime;

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_RunServerOptions
//
//	PURPOSE:	Runs all of the server options
//
// ----------------------------------------------------------------------- //

LTBOOL NetStart_RunServerOptions(ILTClient* pClientDE)
{
	return(NetStart_RunServerOptions(pClientDE, &g_ServerOptions));
}

LTBOOL NetStart_RunServerOptions(ILTClient* pClientDE, ServerOptions* pServerOptions)
{
	// Sanity checks...

	if (!pClientDE) return(LTFALSE);
	if (!pServerOptions) return(LTFALSE);


	// Run the toggle options...

	char sBuf[128];

	sprintf(sBuf, "serv TractorBeam %i", pServerOptions->m_bTractorBeam);
	pClientDE->RunConsoleString(sBuf);

	sprintf(sBuf, "serv DoubleJump %i", pServerOptions->m_bDoubleJump);
	pClientDE->RunConsoleString(sBuf);

	sprintf(sBuf, "serv RammingDamage %i", pServerOptions->m_bRammingDamage);
	pClientDE->RunConsoleString(sBuf);


	// Run the speed options...

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


	// Run the scale options...

	if (pServerOptions->m_fRespawnScale != 0)
	{
		sprintf(sBuf, "serv RespawnScale %f", pServerOptions->m_fRespawnScale);
		pClientDE->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_fHealScale != 0)
	{
		sprintf(sBuf, "serv HealScale %f", pServerOptions->m_fHealScale);
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

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FreeSessionList
//
//	PURPOSE:	Frees the session list
//
// ----------------------------------------------------------------------- //

void NetStart_FreeSessionList(ILTClient* pClientDE)
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

NetSession* NetStart_GetSessionList(ILTClient* pClientDE, char* pInfo)
{
	// Sanity checks...

	if (!pClientDE) return(NULL);
	if (!pInfo) return(NULL);


	// Free any previous session list...

	NetStart_FreeSessionList(pClientDE);


	// Get the session list...

	LTRESULT dr = s_pClient->GetSessionList(s_pSessionList, pInfo);
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

LTBOOL NetStart_DoSettingsDialog()
{
	// Sanity checks...

	if (!s_hInst) return(LTFALSE);


	// Check if we should do the dialog...

	if (s_bHaveSettingInfo && s_bDontDisplaySettingsDialog && !IsKeyDown(VK_SHIFT))
	{
		return(LTTRUE);
	}


	// Do the settings dialog...

	int nRet = DialogBox(s_hInst, "NET_SETTINGS", NULL, (DLGPROC)NetDlg_Settings);
	if (nRet == 0) return(LTFALSE);


	// Update our console flags as necessary...

	if (s_bDontDisplaySettingsDialog)
	{
		WriteConsoleInt("DontDisplaySettingsDialog", 1);
	}

	WriteConsoleInt("HaveSettingInfo", 1);


	// All done...

	return(LTTRUE);
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

#ifdef _DEMO
			CheckDlgButton(hDlg, IDC_ORDOG, BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_ENFORCER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_AKUMA), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PREDATOR), FALSE);
#else
			CheckDlgButton(hDlg, IDC_ORDOG,    (s_nMech == NMT_ORDOG)    ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_ENFORCER, (s_nMech == NMT_ENFORCER) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_AKUMA,    (s_nMech == NMT_AKUMA)    ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_PREDATOR, (s_nMech == NMT_PREDATOR) ? BST_CHECKED : BST_UNCHECKED);
#endif

			CheckDlgButton(hDlg, IDC_DONTDISPLAY, s_bDontDisplaySettingsDialog ? BST_CHECKED : BST_UNCHECKED);

			NetStart_FillColorList(GetDlgItem(hDlg, IDC_COLOR));
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
				if (strlen(s_pNetStart->m_sPlayer) <= 0) strcpy(s_pNetStart->m_sPlayer, "Sanjuro");

				BOOL bOld = s_bDontDisplaySettingsDialog;
				s_bDontDisplaySettingsDialog = IsDlgButtonChecked(hDlg, IDC_DONTDISPLAY) == BST_CHECKED;
				WriteConsoleInt("DontDisplaySettingsDialog", s_bDontDisplaySettingsDialog);
				if (!bOld && s_bDontDisplaySettingsDialog) DoOptionHelp(hDlg, IDS_DONTDISPLAY_INFO, IDS_DONTDISPLAY_TITLE);

				NetStart_FillPlayerStruct(hDlg);
				NetStart_FillFpsLimit(hDlg);
				NetStart_SelectNetSpeed(GetDlgItem(hDlg, IDC_NETCONNECTION));

				EndDialog(hDlg, 1);
				return(TRUE);
			}
		}
	}

	return(FALSE);
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

				g_DlgIP = newIP;
				g_DlgIPLen = sizeof(newIP);

				if(DialogBox(s_hInst, "NET_ADDIP", hDlg, (DLGPROC)AddIPDlgProc) == IDOK)
				{
					if(s_IpMgr.AddIp(newIP))
					{
						s_IpMgr.FillListBox(GetDlgItem(hDlg, IDC_IPLIST));
						bRestartQuery = TRUE;
					}
				}

				g_DlgIP = NULL;
				
				//if (s_IpMgr.AddIpFromEditControl(GetDlgItem(hDlg, IDC_IP), GetDlgItem(hDlg, IDC_IPLIST)))
				//{
				//	bRestartQuery = TRUE;
				//}
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

LTBOOL NetStart_StartSessionQuery(ILTClient* pClientDE, char* sInfo)
{
	// Sanity checks...

	if (!pClientDE) return(LTFALSE);


	// Terminate any previous query...

	NetStart_EndSessionQuery(pClientDE);


	// Start the new query...

	pClientDE->StartQuery(sInfo);


	// All done...

	return(LTTRUE);
}

LTBOOL NetStart_StartSessionQuery(ILTClient* pClientDE, CIpMgr* pIpMgr)
{
	// Sanity checks...

	if (!pClientDE) return(LTFALSE);
	if (!pIpMgr) return(LTFALSE);


	// Terminate any previous query...

	NetStart_EndSessionQuery(pClientDE);


	// Get the string of ips for search on...

	char sBuf[2048];

	if (!pIpMgr->GetAllIpString(sBuf, 2040))
	{
		return(LTFALSE);
	}

	if((sizeof(sBuf) - strlen(sBuf)) > 10)
	{
		// If there's room, automatically broadcast.
		strcat(sBuf, ";*");
	}


	// Start the new query...

	pClientDE->StartQuery(sBuf);


	// All done...

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_EndSessionQuery
//
//	PURPOSE:	Ends an existing session query
//
// ----------------------------------------------------------------------- //

void NetStart_EndSessionQuery(ILTClient* pClientDE)
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

void NetStart_UpdateSessionQuery(ILTClient* pClientDE)
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

NetSession* NetStart_GetSessionQueryResults(ILTClient* pClientDE)
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

		CNinfoGame* pGame = s_NinfoMgr.AddGame(pCur->m_sName, pCur, pCur->m_dwCurConnections);
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

BOOL NetStart_HostTcpIp(ILTClient* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(FALSE);


	// Get the service list...

	NetService* pCur      = NULL;
	NetService* pListHead = NULL;

	LTRESULT dr = s_pClient->GetServiceList(pListHead);
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



