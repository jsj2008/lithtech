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

#include "StdAfx.h"
#include "NetStart.h"
#include "CoolServ.h"
#include "Resource.h"
#include "ServerUtils.h"
#include "Direct.h"
#include "io.h"


// Guids...

DGUID GAMEGUID = { /* 87EEDE80-0ED4-11d2-BA96-006008904776 */
	0x87eede80,	0xed4, 0x11d2, 0xba, 0x96, 0x0, 0x60, 0x8, 0x90, 0x47, 0x76
};


// Defines...

#define	DLG_NEXT		1
#define DLG_CANCEL		2
#define DLG_BACK		3
#define DLG_ERROR		4

#define RETAIL_LEVEL	0
#define CUSTOM_LEVEL	1

#define HOURS_PER_SEC	0.00027f


// Statics...

static	ServerInterface*	s_pServerMgr     = NULL;
static	ServerInfo*			s_pServerInfo    = NULL; 
static	ServerOptions*		s_pServerOptions = NULL;
static	NetGame*			s_pNetGame       = NULL;
static	HWND				s_hMainWnd       = NULL;
static	HICON				s_hIcon		     = NULL;
static	HINSTANCE			s_hInst			 = NULL;
							
static	char				s_sServerName[256]  = { "" };
static	char				s_sServiceName[256] = { "" };
static	int					s_nGameType         = NGT_DEATHMATCH;
static	int					s_nEndType          = NGE_FRAGS;
static	int					s_nEndFrags         = 25;
static	int					s_nEndTime          = 10;
static	int					s_nNetService       = 0;
static	int					s_nMaxPlayers       = MAX_MULTI_PLAYERS_DISPLAY;
static	int					s_nPort				= 0;

static	BOOL				s_bUpdateInfo		= TRUE;
static	BOOL				s_bServerReg		= TRUE;
static	BOOL				s_bUseGameSpy		= TRUE;
static	BOOL				s_bDashGoInfo       = FALSE;
static	BOOL				s_bIsTcpIp          = FALSE;

static	int					s_nErrorString = IDS_NETERR_GENERIC;

static	CStringList			s_collLevels;
static	CStringList			s_collRezFiles;


// Macros...

#define RM_GET_FLOAT(s, f, d) { char sBuf[64] = {""}; UINT32 uSize = 62; rm.Get(s, sBuf, uSize, ""); if (strlen(sBuf) > 0) f = (float)atof(sBuf); else f = d; }
#define RM_SET_FLOAT(s, f) { char sBuf[64]; sprintf(sBuf, "%f", f); rm.Set(s, sBuf); }


// Prototypes...

BOOL NetStart_HandleDefaultDialogCommands(HWND hDlg, WPARAM wParam);
void NetStart_DisplayError();
BOOL NetStart_GetWorldNightColor(HWND hParenWnd);

BOOL NetStart_FillServiceList(HWND hList);
BOOL NetStart_SelectCurrentService(HWND hList);

BOOL NetStart_FillGameInfo(HWND hDlg);
BOOL NetStart_FillLevelList(HWND hList, char* sDir, int nData = 0);
BOOL NetStart_FillGameLevels(HWND hDlg);
BOOL NetStart_FillGameEnd(HWND hDlg);

BOOL NetStart_SetEndGameInfo(HWND hDlg);

BOOL NetStart_InitOptions(HWND hDlg);
BOOL NetStart_FillOptions(HWND hDlg);

BOOL CALLBACK NetDlg_Welcome(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Services(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Game(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Options(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_GameEnd(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_GameLevels(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_Finished(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NetDlg_RezFiles(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ForceActiveFocus(HWND hWnd);
void SetIcon(HWND hWnd);

BOOL AddSelToList(HWND hSrcList, HWND hDestList);
BOOL RemoveSelFromList(HWND hList);
void UpdateGameLevelControls(HWND hDlg);

void LoadLevelStrings();
void SaveLevelStrings();

int  FillAvailableRez(HWND hList);
int  FillSelectedRez(HWND hList);
BOOL AddSelectedRez(HWND hSourceList, HWND hDestList);
BOOL RemoveSelectedRez(HWND hSourceList, HWND hDestList);
BOOL IsRezSelected(HWND hDestList, const char* sRez);
BOOL IsSourceItemAddable(HWND hSourceList, HWND hDestList);
BOOL IsDestItemRemovable(HWND hSourceList, HWND hDestList);
void UpdateRezControls(HWND hDlg);
int	 AddSelectedToList(HWND hDestList);
void SaveRezStrings();
void LoadRezStrings();
void InitGameLevels(HWND hDlg);
void UpdateSaveGameLevels(HWND hDlg);

BOOL  DoOptionHelp(HWND hParentWnd, int iMsg, int iTitle);
void  SetDlgItemFloat(HWND hDlg, int iCtrl, float fValue);
float GetDlgItemFloat(HWND hDlg, int iCtrl);


// Functions...

// ----------------------------------------------------------------------- //
// These functions get/set console variable values.
// For the Get functions, the value you pass in is used as the default value.
// ----------------------------------------------------------------------- //

BOOL SetGameVar(const char *pName, const char *pVal)
{
	char str[512];

	if(!s_pServerMgr)
		return FALSE;

	sprintf(str, "\"+%s\" \"%s\"", pName, pVal);
	s_pServerMgr->RunConsoleString(str);
	return TRUE;
}

void SetGameVar(const char *pName, float val)
{
	char valStr[256];

	sprintf(valStr, "%f", val);
	SetGameVar(pName, valStr);
}

void SetGameVar(const char *pName, int val)
{
	char valStr[256];

	sprintf(valStr, "%d", val);
	SetGameVar(pName, valStr);
}


BOOL NS_GetGameVar(char *pName, char *pVal, DWORD maxLen)
{
	HCONSOLEVAR hVar;

	if(!s_pServerMgr)
		return FALSE;
		
	if(s_pServerMgr->GetConsoleVar(pName, &hVar, pVal) == LT_OK)
	{
		return s_pServerMgr->GetVarValueString(hVar, pVal, maxLen) == LT_OK;
	}
	else
	{
		return SetGameVar(pName, pVal);
	}
}

BOOL NS_GetGameVar(char *pName, int &val)
{
	HCONSOLEVAR hVar;
	char valStr[256];

	if(!s_pServerMgr)
		return FALSE;

	sprintf(valStr, "%d", val);
	if(s_pServerMgr->GetConsoleVar(pName, &hVar, valStr) == LT_OK)
	{
		if(s_pServerMgr->GetVarValueString(hVar, valStr, sizeof(valStr)) == LT_OK)
		{
			val = atoi(valStr);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return SetGameVar(pName, valStr);
	}
}

BOOL NS_GetGameVar(char *pName, float &val)
{
	HCONSOLEVAR hVar;
	char valStr[256];

	if(!s_pServerMgr)
		return FALSE;

	sprintf(valStr, "%f", val);
	if(s_pServerMgr->GetConsoleVar(pName, &hVar, valStr) == LT_OK)
	{
		if(s_pServerMgr->GetVarValueString(hVar, valStr, sizeof(valStr)) == LT_OK)
		{
			val = (float)atof(valStr);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return SetGameVar(pName, valStr);
	}
}


// ----------------------------------------------------------------------- //
// These load/save all the relevant console vars.
// ----------------------------------------------------------------------- //

void NetStart_LoadConsoleVars()
{
	NS_GetGameVar("ServerName", s_sServerName, sizeof(s_sServerName));
	NS_GetGameVar("ServiceName", s_sServiceName, sizeof(s_sServiceName));

	NS_GetGameVar("GameType", s_nGameType);
	NS_GetGameVar("EndType", s_nEndType);
	NS_GetGameVar("EndFrags", s_nEndFrags);
	NS_GetGameVar("EndTime", s_nEndTime);
	NS_GetGameVar("NetService", s_nNetService);
	NS_GetGameVar("MaxPlayers", s_nMaxPlayers);
	NS_GetGameVar("UpdateInfo", s_bUpdateInfo);
	NS_GetGameVar("ServerReg", s_bServerReg);
	NS_GetGameVar("UseGameSpy", s_bUseGameSpy);
	
	s_bDashGoInfo = 0;
	NS_GetGameVar("DashGoInfo", s_bDashGoInfo);
	
	NS_GetGameVar("Port", s_nPort);

	NS_GetGameVar("TractorBeam", s_pServerOptions->m_bTractorBeam);
	NS_GetGameVar("DoubleJump", s_pServerOptions->m_bDoubleJump);
	NS_GetGameVar("RammingDamage", s_pServerOptions->m_bRammingDamage);

	NS_GetGameVar("WorldColorNight", s_pServerOptions->m_sWorldNightColor, sizeof(s_pServerOptions->m_sWorldNightColor));

	NS_GetGameVar("RunSpeed", s_pServerOptions->m_fRunSpeed);
	NS_GetGameVar("MissileSpeed", s_pServerOptions->m_fMissileSpeed);
	NS_GetGameVar("WorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
	NS_GetGameVar("RespawnScale", s_pServerOptions->m_fRespawnScale);
	NS_GetGameVar("HealScale", s_pServerOptions->m_fHealScale);
}

void NetStart_SaveConsoleVars()
{
	SetGameVar("ServerName", s_sServerName);
	SetGameVar("ServiceName", s_sServiceName);

	SetGameVar("GameType", s_nGameType);
	SetGameVar("EndType", s_nEndType);
	SetGameVar("EndFrags", s_nEndFrags);
	SetGameVar("EndTime", s_nEndTime);
	SetGameVar("NetService", s_nNetService);
	SetGameVar("MaxPlayers", s_nMaxPlayers);
	SetGameVar("UpdateInfo", s_bUpdateInfo);
	SetGameVar("ServerReg", s_bServerReg);
	SetGameVar("UseGameSpy", s_bUseGameSpy);
	SetGameVar("DashGoInfo", 1);
	SetGameVar("Port", s_nPort);

	SetGameVar("TractorBeam", s_pServerOptions->m_bTractorBeam);
	SetGameVar("DoubleJump", s_pServerOptions->m_bDoubleJump);
	SetGameVar("RammingDamage", s_pServerOptions->m_bRammingDamage);
	SetGameVar("WorldColorNight", s_pServerOptions->m_sWorldNightColor);

	SetGameVar("RunSpeed", s_pServerOptions->m_fRunSpeed);
	SetGameVar("MissileSpeed", s_pServerOptions->m_fMissileSpeed);
	SetGameVar("WorldTimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
	SetGameVar("RespawnScale", s_pServerOptions->m_fRespawnScale);
	SetGameVar("HealScale", s_pServerOptions->m_fHealScale);
}

DRESULT NetStart_SaveConfigFile(char *pFilename)
{
	if (!s_pServerMgr)
		return LT_ERROR;

	return s_pServerMgr->SaveConfigFile(pFilename);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoWizard
//
//	PURPOSE:	Starts the multiplayer wizard
//
// ----------------------------------------------------------------------- //

BOOL NetStart_DoWizard(HINSTANCE hInst, ServerInfo* pServerInfo, ServerOptions* pServerOptions, NetGame* pNetGame, BOOL bNoDlgs, char *pConfig)
{
	// Sanity checks...

	if (!pServerInfo) return(FALSE);
	if (!pServerOptions) return(FALSE);
	if (!pNetGame) return(FALSE);


	// Set our static pointers...

	s_pServerInfo    = pServerInfo;
	s_pServerOptions = pServerOptions;
	s_pNetGame       = pNetGame;
	s_hInst          = hInst;


	// Clear the structures...

	memset(s_pServerInfo, 0, sizeof(ServerInfo));
	memset(s_pServerOptions, 0, sizeof(ServerOptions));
	memset(s_pNetGame, 0, sizeof(NetGame));


	// Fill in the default server options...

	s_pServerOptions->m_bTractorBeam    = TRUE;
	s_pServerOptions->m_bDoubleJump     = TRUE;
	s_pServerOptions->m_bRammingDamage  = TRUE;
	s_pServerOptions->m_fWorldTimeSpeed = -1.0f;
	s_pServerOptions->m_fRunSpeed       = 1.0;
	s_pServerOptions->m_fMissileSpeed   = 1.0;
	s_pServerOptions->m_fRespawnScale   = 1.0;
	s_pServerOptions->m_fHealScale      = 1.0;

	strcpy(s_pServerOptions->m_sWorldNightColor, "0.5 0.5 0.5");


	// Load some string resources...

	LoadString(s_hInst, IDS_DEFAULTSERVERNAME, s_sServerName, 250);


	// Get the server interface...

	s_pServerMgr = GetServerInterface();
	if (!s_pServerMgr) 
	{
		return(FALSE);
	}


	// Load the server config file...

	s_pServerMgr->LoadConfigFile(pConfig);


	// Declare local variables we'll be using...

	int		nRet;
	int		cRezFiles       = 0;
	HWND	hParentWnd      = NULL;
	BOOL	bAddedResources = FALSE;


	// Load our icon...

	s_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	// Init the networking...

	BOOL bRet = s_pServerMgr->InitNetworking(NULL, 0);
	if (!bRet)
	{
		s_nErrorString = IDS_NETERR_INIT;
		goto Failure;
	}


	// Load some info...

	NetStart_LoadConsoleVars();

	LoadLevelStrings();
	LoadRezStrings();


	// Check if we should skip the dialogs...

	if (bNoDlgs)
	{
		if (NetStart_GoNoDialogs(pServerInfo, pNetGame))
		{
			return(TRUE);
		}
	}


	// Do the services dialog...

ServicesDlg:
	nRet = DialogBox(hInst, "NET_SERVICES", hParentWnd, (DLGPROC)NetDlg_Services);

	if (nRet == DLG_ERROR) goto Failure;
	if (nRet == DLG_CANCEL) goto Cancel;


	// Do the rez files dialog...

RezFilesDlg:
	cRezFiles = FillAvailableRez(NULL);
	if (cRezFiles <= 0) goto GameDlg;

	nRet = DialogBox(hInst, "NET_REZFILES", hParentWnd, (DLGPROC)NetDlg_RezFiles);

	if (nRet == DLG_BACK) goto ServicesDlg;
	if (nRet == DLG_ERROR) goto Failure;
	if (nRet == DLG_CANCEL) goto Cancel;


	// Do the game dialog...

GameDlg:
	nRet = DialogBox(hInst, "NET_GAME", hParentWnd, (DLGPROC)NetDlg_Game);

	if (nRet == DLG_BACK && cRezFiles <= 0) goto ServicesDlg;
	if (nRet == DLG_BACK && cRezFiles > 0) goto RezFilesDlg;
	if (nRet == DLG_ERROR) goto Failure;
	if (nRet == DLG_CANCEL) goto Cancel;


	// Do the options dialog...

OptionsDlg:
	nRet = DialogBox(hInst, "NET_OPTIONS", hParentWnd, (DLGPROC)NetDlg_Options);

	if (nRet == DLG_BACK) goto GameDlg;
	if (nRet == DLG_ERROR) goto Failure;
	if (nRet == DLG_CANCEL) goto Cancel;


	// Do the game levels dialog...

	if (!bAddedResources)
	{
		CWaitCursor wc;

		if (!GetTheApp()->AddResources(s_collRezFiles))
		{
			goto Failure;
		}

		bAddedResources = TRUE;
	}

	nRet = DialogBox(hInst, "NET_GAMELEVELS", hParentWnd, (DLGPROC)NetDlg_GameLevels);

	if (nRet == DLG_BACK) goto OptionsDlg;
	if (nRet == DLG_ERROR) goto Failure;
	if (nRet == DLG_CANCEL) goto Cancel;


	// Display the "-go" info if we haven't already...

	if (!s_bDashGoInfo)
	{
		DoOptionHelp(NULL, IDS_HELP_DASHGO, IDS_TITLE_DASHGO);
	}

	SaveLevelStrings();


	// All done...

	return(TRUE);


	// Cancel...

Cancel:
	return(FALSE);


	// Failure...

Failure:
	NetStart_DisplayError();
	return(FALSE);
}

void NetStart_DisplayError()
{
	AfxMessageBox(s_nErrorString);
}

int NetStart_GetPort()
{
	return(s_nPort);
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
	if (!s_pServerMgr) return(FALSE);


	// Get the service list...

	NetService *pCur, *pListHead = NULL;

	BOOL bRet = s_pServerMgr->GetServiceList(pListHead);
	if (!bRet) return(FALSE);

	if (!pListHead)
	{
		return(FALSE);
	}


	// Add each service to the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	for (pCur=pListHead; pCur; pCur=pCur->m_pNext)
	{
		NetService* pService = pCur;

		int iItem = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pService->m_sName);
		if (iItem != LB_ERR)
		{
			SendMessage(hList, LB_SETITEMDATA, iItem, (LPARAM)pService->m_handle);
		}
	}


	// Select the last used service...

	int nRet = SendMessage(hList, LB_SETCURSEL, s_nNetService, 0);
	if (nRet == LB_ERR)
	{
		SendMessage(hList, LB_SETCURSEL, 0, 0);
	}


	// Free the service list...

	s_pServerMgr->FreeServiceList(pListHead);


	// All done...

	return(TRUE);
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

	BOOL bRet = s_pServerMgr->SelectService(hNetService);
	if (!bRet) return(FALSE);

	s_nNetService = nIndex;


	// Get the string name...

	char sBuf[128];
	nRet = SendMessage(hList, LB_GETTEXT, nIndex, (LPARAM)sBuf);
	if (nRet == LB_ERR) s_pServerInfo->m_sService[0] = '\0';
	else
	{
		strcpy(s_pServerInfo->m_sService, sBuf);
		strcpy(s_sServiceName, s_pServerInfo->m_sService);
		strlwr(sBuf);

		if (strstr(sBuf, "ipx")) strcpy(s_pServerInfo->m_sService, "Ipx");
		else if (strstr(sBuf, "modem")) strcpy(s_pServerInfo->m_sService, "Modem");
		else if (strstr(sBuf, "serial")) strcpy(s_pServerInfo->m_sService, "Serial Cable");
		else if (strstr(sBuf, "tcp"))
		{
			strcpy(s_pServerInfo->m_sService, "Tcp/ip");
			s_bIsTcpIp = TRUE;
		}
	}


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
			s_bIsTcpIp = FALSE;

			SetIcon(hDlg);
			ForceActiveFocus(hDlg);

			if (!NetStart_FillServiceList(GetDlgItem(hDlg, IDC_SERVICELIST)))
			{
				s_nErrorString = IDS_NETERR_FILLSERVICE;
				EndDialog(hDlg, DLG_ERROR);
			}

			CheckDlgButton(hDlg, IDC_USEGAMESPY, s_bUseGameSpy ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_UPDATEINFO, s_bUpdateInfo ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_SERVERREG, s_bServerReg ? BST_CHECKED : BST_UNCHECKED);

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
				else
				{
					s_bUpdateInfo = IsDlgButtonChecked(hDlg, IDC_UPDATEINFO) == BST_CHECKED;
					s_bServerReg  = IsDlgButtonChecked(hDlg, IDC_SERVERREG) == BST_CHECKED;
					s_bUseGameSpy = IsDlgButtonChecked(hDlg, IDC_USEGAMESPY) == BST_CHECKED;

					s_pServerInfo->m_bUseGameSpy     = s_bUseGameSpy;
					s_pServerInfo->m_bUpdateGameInfo = s_bUpdateInfo;
					s_pServerInfo->m_bRegisterServer = s_bServerReg;

					EndDialog(hDlg, DLG_NEXT);
				}
				return(TRUE);
			}

			return(TRUE);
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_FillGameInfo
//
//	PURPOSE:	Fills the game info from the dialog
//
// ----------------------------------------------------------------------- //

BOOL NetStart_FillGameInfo(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Get the server name...

	char sName[128];
	sName[0] = '\0';

	GetDlgItemText(hDlg, IDC_NAME, sName, 120);

	if (strlen(sName) == 0)
	{
		LoadString(s_hInst, IDS_DEFAULTSERVERNAME, sName, 124);
	}

	strcpy(s_pServerInfo->m_sName, sName);
	strcpy(s_sServerName, sName);


	// Get the game type...

	int nType = NGT_DEATHMATCH;

	if (IsDlgButtonChecked(hDlg, IDC_COOPERATIVE)) nType = NGT_COOPERATIVE;
	if (IsDlgButtonChecked(hDlg, IDC_CAPTURETHEFLAG)) nType = NGT_CAPTUREFLAG;

	s_pNetGame->m_byType = nType;


	// Get the max players...

	s_nMaxPlayers = GetDlgItemInt(hDlg, IDC_MAXPLAYERS, NULL, FALSE);
	if (s_nMaxPlayers <= 0) s_nMaxPlayers = 2;
	if (s_nMaxPlayers >= MAX_MULTI_PLAYERS) s_nMaxPlayers = MAX_MULTI_PLAYERS;

	s_pServerInfo->m_dwMaxPlayers = s_nMaxPlayers;


	// Get the port number...

	s_nPort = GetDlgItemInt(hDlg, IDC_PORT, NULL, FALSE);


	// All done...

	return(TRUE);
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
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetIcon(hDlg);
			ForceActiveFocus(hDlg);
			SetDlgItemText(hDlg, IDC_NAME, s_sServerName);
			SetDlgItemInt(hDlg, IDC_PORT, s_nPort, FALSE);
			NetStart_SetEndGameInfo(hDlg);

			if (!s_bIsTcpIp)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PORT_TEXT), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_PORT), SW_HIDE);
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
				if (!NetStart_FillGameInfo(hDlg))
				{
					EndDialog(hDlg, DLG_ERROR);
				}

				if (!NetStart_FillGameEnd(hDlg))
				{
					EndDialog(hDlg, DLG_ERROR);
				}

				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}

			return(TRUE);
		}
	}

	return(FALSE);
}

void ForceActiveFocus(HWND hWnd)
{
	SetActiveWindow(hWnd);
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
}

void SetIcon(HWND hWnd)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)s_hIcon);
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
			SetIcon(hDlg);
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
			SetIcon(hDlg);
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


	// Get all the level files in the given directory...

	FileEntry* pFiles = s_pServerMgr->GetFileList(sDir);
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

	s_pServerMgr->FreeFileList(pFiles);


	// Set the current selection...

	SendMessage(hList, LB_SETCURSEL, 0, 0);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GameEnd
//
//	PURPOSE:	Does the GAMEEND dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_GameEnd(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetIcon(hDlg);
			ForceActiveFocus(hDlg);

			SetDlgItemInt(hDlg, IDC_MAXPLAYERS, s_nMaxPlayers, FALSE);
			SetDlgItemInt(hDlg, IDC_FRAGS, s_nEndFrags, FALSE);
			SetDlgItemInt(hDlg, IDC_MINUTES, s_nEndTime, FALSE);
			CheckDlgButton(hDlg, IDC_AFTERFRAGS,   (s_nEndType == NGE_FRAGS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_AFTERMINUTES, (s_nEndType == NGE_TIME)  ? BST_CHECKED : BST_UNCHECKED);

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
				NetStart_FillGameEnd(hDlg);
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
		}
	}

	return(FALSE);
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

	s_pNetGame->m_byEnd = nType;
	s_nEndType          = nType;


	// Get the game end value...

	s_nEndFrags = GetDlgItemInt(hDlg, IDC_FRAGS, NULL, FALSE);
	s_nEndTime  = GetDlgItemInt(hDlg, IDC_MINUTES, NULL, FALSE);

	s_pNetGame->m_dwEndTime  = s_nEndTime;
	s_pNetGame->m_dwEndFrags = s_nEndFrags;


	// Get the max players...

	s_nMaxPlayers = GetDlgItemInt(hDlg, IDC_MAXPLAYERS, NULL, FALSE);
	if (s_nMaxPlayers <= 0) s_nMaxPlayers = 2;
	if (s_nMaxPlayers >= MAX_MULTI_PLAYERS) s_nMaxPlayers = MAX_MULTI_PLAYERS;

	s_pServerInfo->m_dwMaxPlayers = s_nMaxPlayers;


	// All done...

	return(TRUE);
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
			SetIcon(hDlg);
			ForceActiveFocus(hDlg);

			hRetailList = GetDlgItem(hDlg, IDC_RETAILLIST);
			hCustomList = GetDlgItem(hDlg, IDC_CUSTOMLIST);
			hGameList   = GetDlgItem(hDlg, IDC_GAMELIST);

			NetStart_FillLevelList(hRetailList, "Worlds\\Multi", RETAIL_LEVEL);
			NetStart_FillLevelList(hCustomList, "\\", CUSTOM_LEVEL);

			InitGameLevels(hDlg);

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
				NetStart_FillGameLevels(hDlg);
				UpdateSaveGameLevels(hDlg);
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

void InitGameLevels(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return;


	// Get the "save" flag...

	BOOL bSave = FALSE;
	NS_GetGameVar("SaveGameLevels", bSave);
	if (!bSave) return;

	CheckDlgButton(hDlg, IDC_SAVE, bSave ? BST_CHECKED : BST_UNCHECKED);


	// If we get here, fill in the saved levels...

	HWND hList = GetDlgItem(hDlg, IDC_GAMELIST);
	if (!hList) return;

	POSITION pos = s_collLevels.GetHeadPosition();

	while (pos)
	{
		CString sGameLevel = s_collLevels.GetNext(pos);

		if (!sGameLevel.IsEmpty())
		{
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


			// Add the level to the list...

			int nType = RETAIL_LEVEL;

			strupr(sTemp);
			if (!strstr(sTemp, "\\MULTI\\"))
			{
				nType = CUSTOM_LEVEL;
			}

			int nIndex = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sLevel);
			if (nIndex != LB_ERR) SendMessage(hList, LB_SETITEMDATA, nIndex, nType);
		}
	}
}

void UpdateSaveGameLevels(HWND hDlg)
{
	BOOL bSave = IsDlgButtonChecked(hDlg, IDC_SAVE) == BST_CHECKED;
	SetGameVar("SaveGameLevels", bSave);
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

	s_collLevels.RemoveAll();

	s_pNetGame->m_byNumLevels = 0;
	for (int j = 0; j < MAX_GAME_LEVELS; j++) s_pNetGame->m_sLevels[j][0] = '\0';

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
			int nData = SendMessage(hList, LB_GETITEMDATA, i, 0);

			if (nData == CUSTOM_LEVEL)
			{
				wsprintf(sLevel, "%s", sText);
			}
			else
			{
				wsprintf(sLevel, "Worlds\\Multi\\%s", sText);
			}

			strcpy(s_pNetGame->m_sLevels[s_pNetGame->m_byNumLevels], sLevel);
			s_collLevels.AddTail(sLevel);
			s_pNetGame->m_byNumLevels++;
		}
	}


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



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SaveLevelStrings
//
//	PURPOSE:	Saves all the level strings to the registry
//
// ----------------------------------------------------------------------- //

void SaveLevelStrings()
{
	// Save the levels...

	POSITION pos   = s_collLevels.GetHeadPosition();
	int      count = 0;

	while (pos)
	{
		CString sRez = s_collLevels.GetNext(pos);
		if (!sRez.IsEmpty())
		{
			char sLabel[32];
			wsprintf(sLabel, "Level%i", count);
			SetGameVar(sLabel, sRez);
			count++;
		}
	}


	// Write out the level count...

	SetGameVar("NumLevels", count);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadLevelStrings
//
//	PURPOSE:	Loads all the level strings from the registry
//
// ----------------------------------------------------------------------- //

void LoadLevelStrings()
{
	// Load the level count...

	int count = 0;
	NS_GetGameVar("NumLevels", count);


	// Load the strings...

	s_collLevels.RemoveAll();

	for (int i = 0; i < count; i++)
	{
		char sLabel[32];
		char sLevel[128];
		wsprintf(sLabel, "Level%i", i);
		strcpy(sLevel, "");

		NS_GetGameVar(sLabel, sLevel, 120);

		if (sLevel[0] != '\0')
		{
			s_collLevels.AddTail(sLevel);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GoNoDialogs
//
//	PURPOSE:	Sets everything up using saved values instead of dialogs
//
// ----------------------------------------------------------------------- //

BOOL NetStart_GoNoDialogs(ServerInfo* pServerInfo, NetGame* pNetGame)
{
	// Sanity checks...

	if (!pServerInfo) return(FALSE);
	if (!pNetGame) return(FALSE);


	// Make sure we have a service name...

	if (strlen(s_sServiceName) <= 0) return(FALSE);


	// Make sure we have some levels...

	if (s_collLevels.GetCount() <= 0) return(FALSE);


	// Get the service list...

	NetService *pCur, *pListHead = NULL;

	BOOL bRet = s_pServerMgr->GetServiceList(pListHead);
	if (!bRet) return(FALSE);

	if (!pListHead)
	{
		return(FALSE);
	}


	// Find the service we want...

	int nIndex = -1;
	NetService *pFound = NULL;

	for (pCur=pListHead; pCur; pCur=pCur->m_pNext)
	{
		if (stricmp(pCur->m_sName, s_sServiceName) == 0)
		{
			pFound = pCur;
			break;
		}
	}

	if (!pFound)
	{
		s_pServerMgr->FreeServiceList(pListHead);
		return(FALSE);
	}

	NetService* pService = pFound;
	HNETSERVICE hNetService = pService->m_handle;

	s_pServerMgr->FreeServiceList(pListHead);


	// Select the service...

	bRet = s_pServerMgr->SelectService(hNetService);
	if (!bRet) return(FALSE);

	strcpy(s_pServerInfo->m_sService, s_sServiceName);
	strlwr(s_pServerInfo->m_sService);

	if (strstr(s_pServerInfo->m_sService, "ipx")) strcpy(s_pServerInfo->m_sService, "Ipx");
	else if (strstr(s_pServerInfo->m_sService, "tcp")) strcpy(s_pServerInfo->m_sService, "Tcp/ip");
	else if (strstr(s_pServerInfo->m_sService, "modem")) strcpy(s_pServerInfo->m_sService, "Modem");
	else if (strstr(s_pServerInfo->m_sService, "serial")) strcpy(s_pServerInfo->m_sService, "Serial Cable");


	// Fill in the update options...

	pServerInfo->m_bUpdateGameInfo = s_bUpdateInfo;
	pServerInfo->m_bRegisterServer = s_bServerReg;
	pServerInfo->m_bUseGameSpy     = s_bUseGameSpy;


	// Set the server name...

	strcpy(pServerInfo->m_sName, s_sServerName);


	// Get the max players...

	pServerInfo->m_dwMaxPlayers = s_nMaxPlayers;


	// Get the game end info...

	pNetGame->m_byType     = s_nGameType;
	pNetGame->m_byEnd      = s_nEndType;
	pNetGame->m_dwEndTime  = s_nEndTime;
	pNetGame->m_dwEndFrags = s_nEndFrags;


	// Add the resources...

	if (!GetTheApp()->AddResources(s_collRezFiles))
	{
		return(FALSE);
	}


	// Add each level from our collection...

	pNetGame->m_byNumLevels = 0;
	for (int j = 0; j < MAX_GAME_LEVELS; j++) s_pNetGame->m_sLevels[j][0] = '\0';

	int nCount = s_collLevels.GetCount();

	POSITION pos = s_collLevels.GetHeadPosition();
	if (!pos) return(FALSE);

	while (pos)
	{
		CString sLevel = s_collLevels.GetNext(pos);
		strcpy(pNetGame->m_sLevels[pNetGame->m_byNumLevels], sLevel);
		pNetGame->m_byNumLevels++;
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_RezFiles
//
//	PURPOSE:	Does the REZFILES dialog
//
// ----------------------------------------------------------------------- //

BOOL CALLBACK NetDlg_RezFiles(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetIcon(hDlg);
			ForceActiveFocus(hDlg);

			if (FillAvailableRez(GetDlgItem(hDlg, IDC_SOURCELIST)) <= 0)
			{
				EndDialog(hDlg, DLG_NEXT);
			}

			FillSelectedRez(GetDlgItem(hDlg, IDC_DESTLIST));

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
				AddSelectedToList(GetDlgItem(hDlg, IDC_DESTLIST));
				SaveRezStrings();
				EndDialog(hDlg, DLG_NEXT);
				return(TRUE);
			}
			else if (wParam == IDC_ADD)
			{
				AddSelectedRez(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
				UpdateRezControls(hDlg);
			}
			else if (wParam == IDC_REMOVE)
			{
				RemoveSelectedRez(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
				UpdateRezControls(hDlg);
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				UpdateRezControls(hDlg);
			}
			else if (HIWORD(wParam) == LBN_DBLCLK)
			{
				if (LOWORD(wParam) == IDC_SOURCELIST)
				{
					AddSelectedRez(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
					UpdateRezControls(hDlg);
				}
				else if (LOWORD(wParam) == IDC_DESTLIST)
				{
					RemoveSelectedRez(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
					UpdateRezControls(hDlg);
				}
			}
		}
	}

	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FillAvailableRez
//
//	PURPOSE:	Fills the given list box with the available rez files
//
// ----------------------------------------------------------------------- //

int FillAvailableRez(HWND hList)
{
	// Remove the current list contents...

	if (hList) SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Change to the "custom" direcotry...

	if (chdir("Custom") != 0) return(0);


	// Enumerate the available rez files and add them to the list box...

	int		count = 0;
	long	hFile;
    struct	_finddata_t fd;

	hFile = _findfirst("*.rez", &fd);
	if (hFile == -1)
	{
		chdir ("..");
		return(0);
	}

	if (hList) SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)fd.name);
	count++;

	BOOL bContinue = TRUE;

	while (bContinue)
	{
		if (_findnext(hFile, &fd) != 0)
		{
			bContinue = FALSE;
		}
		else
		{
			if (hList) SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)fd.name);
			count++;
		}
	}


	// Restore the directory...

	chdir ("..");


	// All done...

	return(count);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FillSelectedRez
//
//	PURPOSE:	Fills the given list box with the selected rez files
//
// ----------------------------------------------------------------------- //

int FillSelectedRez(HWND hList)
{
	// Sanity checks...

	if (!hList) return(0);


	// Remove the current list contents...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Enumerate the selected rez files and add them to the list box...

	POSITION pos   = s_collRezFiles.GetHeadPosition();
	int      count = 0;

	while (pos)
	{
		CString sRez = s_collRezFiles.GetNext(pos);

		char sTemp[128];
		strncpy(sTemp, sRez, 125);
		strupr(sTemp);
		if (strncmp(sTemp, "CUSTOM\\", 7) == 0)
		{
			strncpy(sTemp, sRez, 125);
			sRez = &sTemp[7];
		}

		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sRez);
		count++;
	}


	// All done...

	return(count);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddSelectedRez
//
//	PURPOSE:	Adds the selected source rez to the dest list
//
// ----------------------------------------------------------------------- //

BOOL AddSelectedRez(HWND hSourceList, HWND hDestList)
{
	// Sanity checks...

	if (!hSourceList) return(FALSE);
	if (!hDestList) return(FALSE);


	// Get the selected item from the source list...

	int nIndex = SendMessage(hSourceList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);

	char sRez[256];
	int nRet = SendMessage(hSourceList, LB_GETTEXT, nIndex, (LPARAM)(LPCSTR)sRez);
	if (nRet == LB_ERR) return(FALSE);


	// Check if this rez is already selected...

	if (IsRezSelected(hDestList, sRez))
	{
		return(TRUE);
	}


	// Add the selected string to our list...

	nRet = SendMessage(hDestList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sRez);
	if (nRet == LB_ERR) return(FALSE);


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveSelectedRez
//
//	PURPOSE:	Removes the selected dest rez to the dest list
//
// ----------------------------------------------------------------------- //

BOOL RemoveSelectedRez(HWND hSourceList, HWND hDestList)
{
	// Sanity checks...

	if (!hSourceList) return(FALSE);
	if (!hDestList) return(FALSE);


	// Get the selected rez in the dest list...

	int nIndex = SendMessage(hDestList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);


	// Remove the current item from the dest list...

	int nRet = SendMessage(hDestList, LB_DELETESTRING, nIndex, 0);
	if (!nRet == LB_ERR) return(FALSE);


	// Try to select the previous item...

	nRet = SendMessage(hDestList, LB_SETCURSEL, nIndex, 0);
	if (nRet == LB_ERR && nIndex > 0)
	{
		SendMessage(hDestList, LB_SETCURSEL, nIndex-1, 0);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsRezSelected
//
//	PURPOSE:	Determines if the given rez is in the dest list
//
// ----------------------------------------------------------------------- //

BOOL IsRezSelected(HWND hDestList, const char* sRez)
{
	// Sanity checks...

	if (!hDestList) return(FALSE);
	if (!sRez) return(FALSE);


	// Look for this string in the list...

	int nCount = SendMessage(hDestList, LB_GETCOUNT, 0, 0);
	if (nCount == LB_ERR) return(FALSE);

	for (int i = 0; i < nCount; i++)
	{
		char sText[256];

		int nRet = SendMessage(hDestList, LB_GETTEXT, i, (LPARAM)(LPSTR)sText);
		if (nRet != LB_ERR)
		{
			if (strcmp(sRez, sText) == 0) return(TRUE);
		}
	}


	// If we get here, we didn't find this rez...

	return(FALSE);
}

BOOL IsSourceItemAddable(HWND hSourceList, HWND hDestList)
{
	// Sanity checks...

	if (!hSourceList) return(FALSE);
	if (!hDestList) return(FALSE);


	// Get the selected rez in the source list...

	int nIndex = SendMessage(hSourceList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(FALSE);

	char sRez[256];
	int nRet = SendMessage(hSourceList, LB_GETTEXT, nIndex, (LPARAM)(LPCSTR)sRez);
	if (nRet == LB_ERR) return(FALSE);


	// Determine if this rez is already selected...

	return(!IsRezSelected(hDestList, sRez));
}

BOOL IsDestItemRemovable(HWND hSourceList, HWND hDestList)
{
	// Sanity checks...

	if (!hSourceList) return(FALSE);
	if (!hDestList) return(FALSE);


	// Get the selected rez in the dest list...

	int nIndex = SendMessage(hDestList, LB_GETCURSEL, 0, 0);
	return (nIndex != LB_ERR);
}

void UpdateRezControls(HWND hDlg)
{
	if (!hDlg) return;

	BOOL bAddable = IsSourceItemAddable(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
	EnableWindow(GetDlgItem(hDlg, IDC_ADD), bAddable);

	BOOL bRemable = IsDestItemRemovable(GetDlgItem(hDlg, IDC_SOURCELIST), GetDlgItem(hDlg, IDC_DESTLIST));
	EnableWindow(GetDlgItem(hDlg, IDC_REMOVE), bRemable);
}

int AddSelectedToList(HWND hDestList)
{
	// Sanity checks...
	
	if (!hDestList) return(0);


	// Remove all the items from the current string list...

	s_collRezFiles.RemoveAll();


	// Add each item in the list box to our string list...

	int nCount = SendMessage(hDestList, LB_GETCOUNT, 0, 0);
	if (nCount == LB_ERR) return(0);

	int c = 0;

	for (int i = 0; i < nCount; i++)
	{
		char sRez[256];
		int nRet = SendMessage(hDestList, LB_GETTEXT, i, (LPARAM)(LPCSTR)sRez);
		if (nRet != LB_ERR)
		{
			if (strstr(sRez, "\\"))
			{
				s_collRezFiles.AddTail(sRez);
			}
			else
			{
				char sTemp[256];
				sprintf(sTemp, "Custom\\%s", sRez);
				s_collRezFiles.AddTail(sTemp);
			}
			c++;
		}
	}


	// All done...

	return(c);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SaveRezStrings
//
//	PURPOSE:	Saves all the strings to the registry if requested
//
// ----------------------------------------------------------------------- //

void SaveRezStrings()
{
	// Save each rez file string...

	POSITION pos    = s_collRezFiles.GetHeadPosition();
	int      nCount = 0;

	while (pos)
	{
		CString sRez = s_collRezFiles.GetNext(pos);
		if (!sRez.IsEmpty())
		{
			char sLabel[32];
			sprintf(sLabel, "RezFile%i", nCount);
			SetGameVar(sLabel, sRez);
			nCount++;
		}
	}

	// Save the count of strings...

	SetGameVar("NumRezFiles", nCount);
} 


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadRezStrings
//
//	PURPOSE:	Loads all the strings from the registry if requested
//
// ----------------------------------------------------------------------- //

void LoadRezStrings()
{
	// Clear the list of rez files...

	s_collRezFiles.RemoveAll();


	// Load the count of rez strings...

	int nCount = 0;

	if (!NS_GetGameVar("NumRezFiles", nCount)) return;
	if (nCount <= 0) return;


	// Load the strings...

	for (int i = 0; i < nCount; i++)
	{
		char sLabel[32];
		sprintf(sLabel, "RezFile%i", i);

		char sRez[128] = { "" };
		NS_GetGameVar(sLabel, sRez, 125);

		if (sRez[0] != '\0')
		{
			s_collRezFiles.AddTail(sRez);
		}
	}
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
			SetIcon(hDlg);
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
			else if (wParam == IDC_COLORPICKER)
			{
				if (NetStart_GetWorldNightColor(NULL))
				{
					SetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor);
				}
				return(TRUE);
			}

			return(TRUE);
		}
	}

	return(FALSE);
}

BOOL DoOptionHelp(HWND hParentWnd, int iMsg, int iTitle)
{
	if (!s_hInst) s_hInst = GetModuleHandle(NULL);

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

	SetDlgItemFloat(hDlg, IDC_RUNSPEED,       s_pServerOptions->m_fRunSpeed);
	SetDlgItemFloat(hDlg, IDC_MISSILESPEED,   s_pServerOptions->m_fMissileSpeed);
	SetDlgItemFloat(hDlg, IDC_HEALSCALE,      s_pServerOptions->m_fHealScale);
	SetDlgItemFloat(hDlg, IDC_RESPAWNSCALE,   s_pServerOptions->m_fRespawnScale);

	SetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor);

	float fTime = s_pServerOptions->m_fWorldTimeSpeed;
	//fTime = fTime / HOURS_PER_SEC;
	SetDlgItemFloat(hDlg, IDC_TIMESPEED, fTime);

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

	s_pServerOptions->m_bTractorBeam   = IsDlgButtonChecked(hDlg, IDC_TRACTORBEAM) == BST_CHECKED;
	s_pServerOptions->m_bDoubleJump    = IsDlgButtonChecked(hDlg, IDC_DOUBLEJUMP) == BST_CHECKED;
	s_pServerOptions->m_bRammingDamage = IsDlgButtonChecked(hDlg, IDC_RAMMINGDAMAGE) == BST_CHECKED;

	s_pServerOptions->m_fRunSpeed       = GetDlgItemFloat(hDlg, IDC_RUNSPEED);
	s_pServerOptions->m_fMissileSpeed   = GetDlgItemFloat(hDlg, IDC_MISSILESPEED);
	s_pServerOptions->m_fRespawnScale   = GetDlgItemFloat(hDlg, IDC_RESPAWNSCALE);
	s_pServerOptions->m_fHealScale      = GetDlgItemFloat(hDlg, IDC_HEALSCALE);

	GetDlgItemText(hDlg, IDC_NIGHTCOLOR, s_pServerOptions->m_sWorldNightColor, 30);

	float fTime = GetDlgItemFloat(hDlg, IDC_TIMESPEED);
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

BOOL NetStart_RunServerOptions(ServerInterface* pServerMgr, ServerOptions* pServerOptions)
{
	// Sanity checks...

	if (!pServerMgr) return(FALSE);
	if (!pServerOptions) return(FALSE);


	// Run the toggle options...

	char sBuf[128];

	sprintf(sBuf, "TractorBeam %i", pServerOptions->m_bTractorBeam);
	pServerMgr->RunConsoleString(sBuf);

	sprintf(sBuf, "DoubleJump %i", pServerOptions->m_bDoubleJump);
	pServerMgr->RunConsoleString(sBuf);

	sprintf(sBuf, "RammingDamage %i", pServerOptions->m_bRammingDamage);
	pServerMgr->RunConsoleString(sBuf);


	// Run the speed options...

	if (pServerOptions->m_fRunSpeed != 0)
	{
		sprintf(sBuf, "RunSpeed %f", pServerOptions->m_fRunSpeed);
		pServerMgr->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_fMissileSpeed != 0)
	{
		sprintf(sBuf, "MissileSpeed %f", pServerOptions->m_fMissileSpeed);
		pServerMgr->RunConsoleString(sBuf);
	}


	// Run the scale options...

	if (pServerOptions->m_fRespawnScale != 0)
	{
		sprintf(sBuf, "RespawnScale %f", pServerOptions->m_fRespawnScale);
		pServerMgr->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_fHealScale != 0)
	{
		sprintf(sBuf, "HealScale %f", pServerOptions->m_fHealScale);
		pServerMgr->RunConsoleString(sBuf);
	}


	// Run the world options...

	if (pServerOptions->m_fWorldTimeSpeed != 0)
	{
		sprintf(sBuf, "WorldTimeSpeed %f", pServerOptions->m_fWorldTimeSpeed);
		pServerMgr->RunConsoleString(sBuf);
	}

	if (pServerOptions->m_sWorldNightColor[0] != '\0')
	{
		sprintf(sBuf, "WorldColorNight \"%s\"", pServerOptions->m_sWorldNightColor);
		pServerMgr->RunConsoleString(sBuf);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_DoOptionsDialog
//
//	PURPOSE:	Does the options dialog
//
// ----------------------------------------------------------------------- //

BOOL NetStart_DoOptionsDialog(HWND hParentWnd, ServerInterface* pServerMgr, ServerOptions* pServerOptions)
{
	// Sanity checks...

	if (!pServerMgr) return(FALSE);
	if (!pServerOptions) return(FALSE);


	// Do the dialog...

	if (!s_hInst) s_hInst = GetModuleHandle(NULL);

	int nRet = DialogBox(s_hInst, "SERVER_OPTIONS", hParentWnd, (DLGPROC)NetDlg_Options);
	if (nRet != DLG_NEXT) return(FALSE);


	// Run the console options...

	if (!NetStart_RunServerOptions(pServerMgr, pServerOptions))
	{
		return(FALSE);
	}

	SetGameVar("TractorBeam", s_pServerOptions->m_bTractorBeam);
	SetGameVar("DoubleJump", s_pServerOptions->m_bDoubleJump);
	SetGameVar("RammingDamage", s_pServerOptions->m_bRammingDamage);
	SetGameVar("WorldNightColor", s_pServerOptions->m_sWorldNightColor);

	SetGameVar("RunSpeed", s_pServerOptions->m_fRunSpeed);
	SetGameVar("MissileSpeed", s_pServerOptions->m_fMissileSpeed);
	SetGameVar("TimeSpeed", s_pServerOptions->m_fWorldTimeSpeed);
	SetGameVar("RespawnScale", s_pServerOptions->m_fRespawnScale);
	SetGameVar("HealScale", s_pServerOptions->m_fHealScale);
	

	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NetStart_GetWorldNightColor
//
//	PURPOSE:	Gets the world night color via a color dialog
//
// ----------------------------------------------------------------------- //

BOOL NetStart_GetWorldNightColor(HWND hParenWnd)
{
	// Do the color dialog...

	CColorDialog dlg;

	int nRet = dlg.DoModal();
	if (nRet != IDOK) return(FALSE);


	// Update the color...

	COLORREF cr = dlg.GetColor();

	BYTE byRed   = (BYTE)(cr);
	BYTE byGreen = (BYTE)(cr >> 8);
	BYTE byBlue  = (BYTE)(cr >> 16);

	float fRed   = (float)byRed / 255.0f;
	float fGreen = (float)byGreen / 255.0f;
	float fBlue  = (float)byBlue / 255.0f;

	sprintf(s_pServerOptions->m_sWorldNightColor, "%.2f %.2f %.2f", fRed, fGreen, fBlue);


	// All done...

	return(TRUE);
}






