// GameServDlg.cpp : implementation file
//


// Includes...

#include "stdafx.h"
#include "coolserv.h"
#include "GameServDlg.h"
#include "NetStart.h"
#include "Sparam.h"
#include "Utils.h"
#include "mmsystem.h"


// Defines...

#define DEFAULT_PORT	27888
#define USE_POSTMSG		0


// Debug...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Macros...

#define DEC_TIMER(t, d)		if (d >= t) t = 0; else t -= d;


// Externs...

extern	ServerInfo		g_ServerInfo;
extern	ServerOptions	g_ServerOptions;
extern	NetGame			g_NetGame;
extern	BOOL	 		g_bEmptyExit;


// Globals...

DWORD	g_timeCur   = 0;
DWORD	g_timeLast  = 0;
DWORD	g_timeDelta = 0;
DWORD	g_timeStart = 0;

DWORD	g_dwPosted  = 0;
DWORD	g_dwHandled = 0;
DWORD	g_dwRemoved = 0;

BOOL	g_bDoEmptyExit    = FALSE;
int		g_nEmptyExitDelay = 60;
DWORD	g_timerEmptyExit  = 0;
BOOL	g_bNoExitConfirm  = FALSE;

CString	g_sFullTcpIpAddress;
WORD	g_wPort = 0;

#define SERVER_VERSION	"2.1"
#define DEFAULT_PORT	27888

CString	g_sSpyGameName;
CString	g_sSpyGameVer;
CString	g_sSpyGameKey;
CString g_sWebGameName;
CString	g_sWebGameVer;
CString g_sWebRegUrl;


// Statics...

static	DWORD	s_timerWebRegUpdate  = 400; 

static	BOOL	s_bThreadDone = FALSE;


// Classes...

class CGameServerAppHandler : public ServerAppHandler
{
public:
	DRESULT ShellMessageFn(char* pInfo);
	DRESULT ConsoleOutputFn(char* pInfo);
	DRESULT	OutOfMemory();
	DRESULT ProcessPacket(char *pData, DDWORD dataLen, DBYTE senderAddr[4], D_WORD senderPort);
};


// Globals...

CGameServDlg*			g_pDialog = NULL;
CGameServerAppHandler	g_AppHandler;


// Prototypes...

DWORD WINAPI ThreadUpdate(LPVOID lpParameter);


// Functions...

DRESULT CGameServerAppHandler::ShellMessageFn(char* pInfo)
{
	if (g_pDialog)
	{
		g_pDialog->OnShellMessage(pInfo);
	}

	return(LT_OK);
}

DRESULT CGameServerAppHandler::ConsoleOutputFn(char* pInfo)
{
	if (g_pDialog)
	{
		g_pDialog->OnConsoleOutput(pInfo);
	}

	return(LT_OK);
}

DRESULT CGameServerAppHandler::OutOfMemory()
{
	if (g_pDialog)
	{
		g_pDialog->OnOutOfMemory();
	}

	return(LT_OK);
}

DRESULT CGameServerAppHandler::ProcessPacket(char *pData, DDWORD dataLen, DBYTE senderAddr[4], D_WORD senderPort)
{
	if(g_pDialog)
	{
		g_pDialog->OnProcessNetPacket(pData, dataLen, senderAddr, senderPort);
	}

	return LT_OK;
}





// ----------------------------------------------------------------------- //
// Replacement for MFC's trace.  Doesn't ASSERT.
// ----------------------------------------------------------------------- //

void SS_Trace(const char *pMsg, ...)
{
	#ifdef _DEBUG
		char buf[1024*16];
		va_list marker;

		va_start(marker, pMsg);
		vsprintf(buf, pMsg, marker);
		va_end(marker);

		OutputDebugString(buf);
	#endif
}


/////////////////////////////////////////////////////////////////////////////
// CGameServSendHandler
/////////////////////////////////////////////////////////////////////////////

void CGameServSendHandler::SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port)
{
	ServerInterface *pMgr;

	if(m_pDlg && (pMgr = m_pDlg->GetServerMgr()))
	{
		pMgr->SendTo(pData, len, sAddr, port);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CGameServDlg dialog

CGameServDlg::CGameServDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGameServDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGameServDlg)
	//}}AFX_DATA_INIT

	Clear();

	m_pConfigFilename = NULL;
	g_pDialog = this;
	m_SendHandler.SetDlg(this);

	m_sGameSpyGameMode = "openplaying";
	m_sGameSpyGameType = "deathmatch";
}


void CGameServDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGameServDlg)
	DDX_Control(pDX, IDC_PLAYERS_LIST, m_lbPlayers);
	DDX_Control(pDX, IDC_LEVELS_LIST, m_lbLevels);
	DDX_Control(pDX, IDC_CONSOLE_WINDOW, m_edConsole);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGameServDlg, CDialog)
	//{{AFX_MSG_MAP(CGameServDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_QUIT, OnQuit)
	ON_BN_CLICKED(IDC_SERVER, OnServer)
	ON_BN_CLICKED(IDC_CONSOLE_SEND, OnConsoleSend)
	ON_BN_CLICKED(IDC_CONSOLE_CLEAR, OnConsoleClear)
	ON_BN_CLICKED(IDC_COMMANDS_NEXTLEVEL, OnCommandsNextLevel)
	ON_BN_CLICKED(IDC_PLAYERS_BOOT, OnPlayersBoot)
	ON_LBN_SELCHANGE(IDC_PLAYERS_LIST, OnSelchangePlayersList)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_COMMANDS_OPTIONS, OnCommandsOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CGameServDlg::Init()
{
	if(!m_bCSInitted)
	{
		InitializeCriticalSection(&m_CS);
		m_bCSInitted = TRUE;
	}
	
	// Display a console message...

	WriteConsoleString(IDS_CONSOLE_INITSERVER);


	// Get the server interface...

	m_pServerMgr = GetServerInterface();
	if (!m_pServerMgr) return(FALSE);


	// Set our app handler...

	m_pServerMgr->SetAppHandler(&g_AppHandler);


	// Set the tab stops for the player list box...

	int nTabs[10] = { 30, 116, 181, 84, 86, 88, 90, 92, 94, 96 };

	m_lbPlayers.SetTabStops(10, nTabs);


	// Load some strings...

	g_sSpyGameName.LoadString(IDS_SPYGAMENAME);
	g_sSpyGameKey.LoadString(IDS_SPYGAMEKEY);
	g_sWebRegUrl.LoadString(IDS_WEBREGURL);

#ifdef _ADDON
	g_sSpyGameVer.LoadString(IDS_SPYGAMEVER_AO);
	g_sWebGameName.LoadString(IDS_WEBGAMENAME_AO);
	g_sWebGameVer.LoadString(IDS_WEBGAMEVER_AO);
#else
	g_sSpyGameVer.LoadString(IDS_SPYGAMEVER);
	g_sWebGameName.LoadString(IDS_WEBGAMENAME);
	g_sWebGameVer.LoadString(IDS_WEBGAMEVER);
#endif


	// All done...

	WriteConsoleString(IDS_CONSOLE_SERVERINITED);
	return(TRUE);
}

void CGameServDlg::Term()
{
	if(m_bCSInitted)
	{
		DeleteCriticalSection(&m_CS);
		m_bCSInitted = FALSE;
	}

	Clear();
	g_pDialog = NULL;

	Sleep(2);
	SS_Trace("Updates posted  = %lu\n", g_dwPosted);
	SS_Trace("Updates handled = %lu\n", g_dwHandled);
	SS_Trace("Updates removed = %lu\n", g_dwRemoved);
	SS_Trace("Updates per sec = %i\n", (int)((float)(g_dwHandled / (float)(timeGetTime() - g_timeStart)) * 1000));
}

void CGameServDlg::Clear()
{
	m_bCSInitted = FALSE;
	memset(&m_CS, 0, sizeof(m_CS));

	m_pServerMgr = NULL;
	g_pDialog    = NULL;

	m_bRunning       = FALSE;
	m_bLevelChanging = FALSE;

	m_timeServerRunning = 0;
	m_timeServerStart   = 0;
	m_timeServerLast    = 0;

	m_timeLevelRunning = 0;
	m_timeLevelStart   = 0;
	m_timeLevelLast    = 0;

	m_cPlayers  = 0;
	m_nCurLevel = 0;

	m_hThread    = NULL;
	m_dwThreadID = NULL;

	m_sCurLevel.Empty();
}

BOOL CGameServDlg::StartServer()
{
	// Sanity checks...

	if (!m_pServerMgr) return(FALSE);


	// Set the debug level...

	m_pServerMgr->RunConsoleString("debuglevel 0");


	// Set the server options...

	NetStart_RunServerOptions(m_pServerMgr, &g_ServerOptions);


	// Set the game type string based on the game type...

	int nStringID = IDS_GAMETYPE_DEATHMATCH;

	switch (g_NetGame.m_byType)
	{
		case NGT_CAPTUREFLAG:	nStringID = IDS_GAMETYPE_CAPTUREFLAG; break;
		case NGT_COOPERATIVE:	nStringID = IDS_GAMETYPE_COOPERATIVE; break;
		case NGT_TEAMS:			nStringID = IDS_GAMETYPE_TEAM; break;

#ifdef _ADDON
		case NGT_SOCCER:		nStringID = IDS_GAMETYPE_SOCCER; break;
#endif

	}

	m_sGameSpyGameType.LoadString(nStringID);


	// Look at some console vars...

	HCONSOLEVAR hVar = NULL;

	DRESULT dr = m_pServerMgr->GetConsoleVar("WebRegUpdate", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		float fVal = 0;
		dr = m_pServerMgr->GetVarValueFloat(hVar, &fVal);
		if (dr == LT_OK && fVal > 1)
		{
			s_timerWebRegUpdate = (DWORD)fVal;
		}
	}

	hVar = NULL;
	dr   = m_pServerMgr->GetConsoleVar("EmptyExit", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		float fVal = 0;
		dr = m_pServerMgr->GetVarValueFloat(hVar, &fVal);
		if (dr == LT_OK && fVal >= 1)
		{
			g_bEmptyExit = TRUE;
		}
	}

	hVar = NULL;
	dr   = m_pServerMgr->GetConsoleVar("EmptyExitDelay", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		float fVal = 0;
		dr = m_pServerMgr->GetVarValueFloat(hVar, &fVal);
		if (dr == LT_OK && fVal >= 1)
		{
			g_nEmptyExitDelay = (int)fVal;
		}
	}

	hVar = NULL;
	dr   = m_pServerMgr->GetConsoleVar("SpyGameType", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		char sTemp[64] = { "" };
		dr = m_pServerMgr->GetVarValueString(hVar, sTemp, 64);
		if (dr == LT_OK && sTemp[0] != '\0')
		{
			m_sGameSpyGameType = sTemp;
		}
	}

	hVar = NULL;
	dr   = m_pServerMgr->GetConsoleVar("SpyGameMode", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		char sTemp[64] = { "" };
		dr = m_pServerMgr->GetVarValueString(hVar, sTemp, 64);
		if (dr == LT_OK && sTemp[0] != '\0')
		{
			m_sGameSpyGameMode = sTemp;
		}
	}


	// Host the game...

	CWaitCursor wc;

	NetHost nh;

	memset(&nh, 0, sizeof(nh));

	nh.m_dwMaxPlayers = g_ServerInfo.m_dwMaxPlayers;
	nh.m_Port         = NetStart_GetPort();

	CString sHost;

#ifdef _ADDON
	sHost.LoadString(IDS_APPNAME_AO);
#else
	sHost.LoadString(IDS_APPNAME);
#endif

	Sparam_Add(nh.m_sName, NST_GAMENAME,  g_ServerInfo.m_sName);
	Sparam_Add(nh.m_sName, NST_GAMEHOST,  sHost);
	Sparam_Add(nh.m_sName, NST_GAMELEVEL, g_NetGame.m_sLevels[0]);
	Sparam_Add(nh.m_sName, NST_GAMETYPE,  g_NetGame.m_byType);

	if (m_pServerMgr->HostGame(&nh) != DTRUE)
	{
		WriteConsoleString(IDS_CONSOLE_UNABLETOHOST);
		StopServer();
		return(FALSE);
	}


	// Start the world...

	StartGameRequest req;
	memset(&req, 0, sizeof(req));

	req.m_Type = STARTGAME_HOST;
	req.m_pGameInfo   = &g_NetGame;
	req.m_GameInfoLen = sizeof(NetGame_t);

	strcpy(req.m_WorldName, g_NetGame.m_sLevels[0]);

	if (m_pServerMgr->StartWorld(&req) != DTRUE)
	{
		WriteConsoleString(IDS_CONSOLE_UNABLETOSTARTWORLD);
		WriteConsoleString(req.m_WorldName);
		StopServer();
		return(FALSE);
	}


	// Update the service info with the tcp/ip address if available...

	g_ServerInfo.m_sAddress[0] = '\0';
	g_wPort = 0;

	m_pServerMgr->GetTcpIpAddress(g_ServerInfo.m_sAddress, 128, g_wPort);

	CString sService    = g_ServerInfo.m_sService;
	g_sFullTcpIpAddress = "";
	if (g_ServerInfo.m_sAddress[0] != '\0')
	{
		sService += " ";
		g_sFullTcpIpAddress = g_ServerInfo.m_sAddress;
		if (g_wPort > 0 && g_wPort != DEFAULT_PORT)
		{
			char sTemp[32];
			wsprintf(sTemp, ":%i", g_wPort);
			g_sFullTcpIpAddress += sTemp;
		}
		sService += "[";
		sService += g_sFullTcpIpAddress;
		sService += "]";
	}
	SetDlgItemText(IDC_SERVER_SERVICE, sService);


	// Tell the server shell to update the game info parameters...

	m_pServerMgr->SendToServerShell("GAMEINIT");


	// Tell the server that this is a stand-alone server hosted game...

	m_pServerMgr->SendToServerShell("SERVHOST");


	// Init some stuff now that we're running...

	m_bRunning        = TRUE;
	m_timeServerStart = timeGetTime();
	g_timeStart       = timeGetTime();
	m_timeLevelStart  = m_timeServerStart;


	// Init the GameSpy manager...

	if (g_ServerInfo.m_bUseGameSpy)
	{
		if (m_GameSpyMgr.Init(g_sSpyGameName, g_sSpyGameVer, g_sSpyGameKey, 0, g_wPort, (g_wPort+166), GSMF_USEGAMEPORTFORHEARTBEAT))
		{
			m_GameSpyMgr.SetSendHandler(&m_SendHandler);
		}
		else
		{
			g_ServerInfo.m_bUseGameSpy = FALSE;
		}
	}


	// Create the thread to do our updating...

	m_hThread = CreateThread(NULL, 0, ThreadUpdate, (void*)g_pDialog, 0, &m_dwThreadID);


	// All done...

	WriteConsoleString(IDS_CONSOLE_SERVERRUNNING);
	return(TRUE);
}

BOOL CGameServDlg::StopServer()
{
	// Unregister the server if necessary...

	if (m_bRunning)
	{
		UnregisterServer();
	}


	// Terminate the GameSpy manager...

	m_GameSpyMgr.Term();


	// Flag that we are done...

	m_bRunning = FALSE;


	// Wait for the thread to stop...

	if (m_hThread)
	{
		Sleep(0);
		DWORD dwRet = WaitForSingleObject(m_hThread, 1000);
		Sleep(0);
		
		if (dwRet == WAIT_OBJECT_0)
		{
			CloseHandle(m_hThread);
		}
		else
		{
			TerminateThread(m_hThread, 0);
		}

		m_hThread = NULL;
	}


	// All done...

	return(TRUE);
}

void CGameServDlg::WriteConsoleString(LPCTSTR pMsg, ...)
{
	TCHAR		str[500];
	va_list		marker;
	int			nLen;

	static int nMax = 500;

	if(m_edConsole.GetLineCount() > nMax)
	{
		// Nuke the oldest 75%.
		int iLine = (m_edConsole.GetLineCount()*75) / 100;
		int iChar = m_edConsole.LineIndex(iLine);
		
		m_edConsole.SetRedraw(FALSE);
		m_edConsole.SetSel(0, iChar);
		m_edConsole.ReplaceSel("", FALSE);
		m_edConsole.SetRedraw(TRUE);
	}

	va_start(marker, pMsg);
	vsprintf(str, pMsg, marker);
	va_end(marker);

	int len = strlen(str);
	if (len <= 0) return;
	if (len > 0 && str[len-1] < 32) str[len-1] = '\0';

	strcat(str, "\r\n");

	nLen = m_edConsole.SendMessage(EM_GETLIMITTEXT, 0, 0);
	m_edConsole.SetSel(nLen, nLen);
	m_edConsole.ReplaceSel(str);
}

void CGameServDlg::WriteConsoleString(int nStringID)
{
	CString sMsg;
	if (!sMsg.LoadString(nStringID)) return;
	WriteConsoleString(sMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CGameServDlg message handlers

BOOL CGameServDlg::OnInitDialog() 
{
	// Let the base class do it's thing...

	CDialog::OnInitDialog();


	// Set the window title if necessary...

#ifdef _ADDON
	CString sTitle;
	sTitle.LoadString(IDS_DIALOGTITLE_AO);
	SetWindowText(sTitle);
#endif


	// Fill in some of the text info...

	SetDlgItemText(IDC_SERVER_NAME, g_ServerInfo.m_sName);
	SetDlgItemText(IDC_SERVER_TIME, "0:00");

	CString sService(g_ServerInfo.m_sService);
	if (g_ServerInfo.m_sAddress[0] != '\0')
	{
		sService += " (";
		sService += g_ServerInfo.m_sAddress;
		sService += ")";
	}
	SetDlgItemText(IDC_SERVER_SERVICE, sService);

	SetDlgItemText(IDC_GAME_PLAYERS, "0");
	SetDlgItemText(IDC_GAME_CURLEVEL, g_NetGame.m_sLevels[0]);
	SetDlgItemText(IDC_GAME_TIME, "0:00");

	m_sCurLevel = g_NetGame.m_sLevels[0];

	if (g_NetGame.m_byNumLevels > 1) SetDlgItemText(IDC_GAME_NEXTLEVEL, g_NetGame.m_sLevels[1]);
	else SetDlgItemText(IDC_GAME_NEXTLEVEL, "");

	CString sBuf;
	if (g_NetGame.m_byEnd == NGE_FRAGS) sBuf.Format("%i Frags", g_NetGame.m_dwEndFrags);
	else if (g_NetGame.m_byEnd == NGE_TIME) sBuf.Format("%i Minutes", g_NetGame.m_dwEndTime);
	else if (g_NetGame.m_byEnd == NGE_FRAGSANDTIME) sBuf.Format("%i Frags or %i Minutes", g_NetGame.m_dwEndFrags, g_NetGame.m_dwEndTime);
	else sBuf = "None. Level never ends.";
	SetDlgItemText(IDC_GAME_GOAL, sBuf);

	for (int i = 0; i < g_NetGame.m_byNumLevels; i++)
	{
		m_lbLevels.AddString(g_NetGame.m_sLevels[i]);
	}


	// Set the icon...

	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);


	for(i=0; i < 30; i++)
	{
		if(SetTimer((UINT)i, 500, NULL))
			break;
	}

	// All done...

	return(TRUE);
}

BOOL CGameServDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam)
	{
		case ID_INIT:
		{
			if (!Init())
			{
				return(TRUE);
			}

			if (!StartServer())
			{
				return(TRUE);
			}

			return(TRUE);
		}

		case ID_UPDATE:
		{
			OnUpdate();
			return(TRUE);
		}
	}
	
	return(CDialog::OnCommand(wParam, lParam));
}

void CGameServDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	static BOOL bNeedInit = TRUE;

	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow && bNeedInit)
	{
		bNeedInit = FALSE;
		PostMessage(WM_COMMAND, ID_INIT);
	}
}

void CGameServDlg::UpdateServerMgr(DWORD timeDelta)
{
	// Sanity checks...

	if (!m_pServerMgr) return;
	if (!IsRunning()) return;


	// Let the server update...

	if (m_pServerMgr->Update(0) != DTRUE)
	{
		WriteServerError();
		StopServer();
	}
}

void CGameServDlg::WriteServerError()
{
	char str[256];

	if (m_pServerMgr)
	{
		m_pServerMgr->GetErrorString(str, sizeof(str)-1);
		WriteConsoleString(str);
	}
}

void CGameServDlg::OnClose() 
{
	CDialog::OnClose();
}

void CGameServDlg::OnTimer(UINT nIDEvent) 
{
	// Call the update function...

	//OnUpdate();
	UpdateTimers();
}


void CGameServDlg::OnUpdate()
{
	// Increment our counter...

	g_dwHandled++;


	// If we're currently changing levels, just return...

	if (IsLevelChanging()) return;


	// Get the current time...

	g_timeCur = timeGetTime();
	if (g_timeLast == 0) g_timeLast = g_timeCur;


	// Calculate the delta time...

	g_timeDelta = g_timeCur - g_timeLast;
	g_timeLast  = g_timeCur;


	// Update everything if we're running...

	if (IsRunning())
	{
		// Update the empty exit...

		if (g_bDoEmptyExit)
		{
			DEC_TIMER(g_timerEmptyExit, g_timeDelta);
			if (g_timerEmptyExit == 0)
			{
				g_bNoExitConfirm = TRUE;
				PostMessage(WM_CLOSE);
			}
		}


		// Update the server manager...

		UpdateServerMgr(g_timeDelta);
		if (!IsRunning()) return;


		// Update the game spy manager...

		UpdateGameSpyMgr(g_timeDelta);


		// Update the server registration...

		UpdateServerRegistration(g_timeDelta);
	}
}

void CGameServDlg::UpdateTimers()
{
	DWORD timeCur;

	if(!IsRunning())
		return;

	timeCur = timeGetTime();

	// Update the server time...

	m_timeServerRunning = timeCur - m_timeServerStart;
	if ((m_timeServerLast / 1000) != (m_timeServerRunning / 1000))
	{
		SetDlgItemText(IDC_SERVER_TIME, TimeToString(m_timeServerRunning));
		m_timeServerLast = m_timeServerRunning;
	}


	// Update the game/level time...

	m_timeLevelRunning = timeCur - m_timeLevelStart;
	if ((m_timeLevelLast / 1000) != (m_timeLevelRunning / 1000))
	{
		SetDlgItemText(IDC_GAME_TIME, TimeToString(m_timeLevelRunning));
		m_timeLevelLast = m_timeLevelRunning;
	}


	// Remove any update messages in the queue...

	RemoveMessage(WM_TIMER, 500);
}


void CGameServDlg::OnQuit() 
{
	PostMessage(WM_COMMAND, IDCANCEL);
}

void CGameServDlg::OnServer() 
{
	PostMessage(WM_COMMAND, IDCANCEL);
}

BOOL CGameServDlg::UpdateServerRegistration(DWORD timeDelta)
{
	// Sanity checks...

	if (!m_pServerMgr) return(FALSE);
	if (!g_ServerInfo.m_bRegisterServer) return(FALSE);
	if (g_ServerInfo.m_sAddress[0] == '\0') return(FALSE);


	// Check if it's really time to update...

	static DWORD timerUpdate = 7000;

	DEC_TIMER(timerUpdate, timeDelta);
	if (timerUpdate != 0) return(FALSE);
	else timerUpdate = s_timerWebRegUpdate * 1000;


	// Build the info string...

	CString sInfo;

	sInfo.Format("game=%s&name=%s&ip=%s&players=%i&version=%s", g_sWebGameName, g_ServerInfo.m_sName, g_sFullTcpIpAddress, m_pServerMgr->GetNumClients(), g_sWebGameVer);


	// Send the server info the web page...

	SendWebInfo(sInfo, FALSE);


	// All done...

	return(TRUE);
}

BOOL CGameServDlg::UpdateGameSpyMgr(DWORD timeDelta)
{
	// Sanity checks...

	if (!m_pServerMgr) return(FALSE);
	if (!g_ServerInfo.m_bUseGameSpy) return(FALSE);


	// Update the GameSpy manager...

	m_GameSpyMgr.Update();


	// All done...

	return(TRUE);
}

BOOL CGameServDlg::UnregisterServer()
{
	// Sanity checks...

	if (!g_ServerInfo.m_bRegisterServer) return(FALSE);
	if (g_ServerInfo.m_sAddress[0] == '\0') return(FALSE);


	// Build the info string...

	CString sInfo;

	sInfo.Format("game=%s&name=%s&ip=%s&players=-1&version=%s", g_sWebGameName, g_ServerInfo.m_sName, g_sFullTcpIpAddress, g_sWebGameVer);


	// Send the server info the web page...

	SendWebInfo(sInfo, FALSE);


	// All done...

	return(TRUE);
}

void CGameServDlg::OnShellMessage(char* pMsg)
{
	// Sanity checks...

	if (!pMsg) return;


	// Figure out what type of message this if...

	char sTemp[128];

	if (!Sparam_Get(sTemp, pMsg, NST_GENERICMESSAGE))
	{
		return;
	}

	switch (atoi(sTemp))
	{
		case NGM_STANDARDUPDATE:	OnStandardUpdate(pMsg); break;
		case NGM_LEVELCHANGING:		OnLevelChanging(pMsg); break;
		case NGM_LEVELCHANGED:		OnLevelChangeUpdate(pMsg); break;
		case NGM_LEVELCHANGESTOP:	OnLevelChangeStop(); break;
		case NGM_CONSOLEMSG:		OnConsoleMessageUpdate(pMsg); break;
	}
}


int ClientIDToIndex(ServerInterface *pInterface, DWORD id)
{
	int i, count;
	ClientInfo info;

	count = pInterface->GetNumClients();
	for(i=0; i < count; i++)
	{
		pInterface->GetClientInfo(i, &info);
		if(info.m_ClientID == id)
			return i;
	}
	return -1;
}


void CGameServDlg::OnStandardUpdate(char* pMsg)
{
	// Update the level info...

	char sTemp[128];

	if (Sparam_Get(sTemp, pMsg, NST_CURLEVEL))
	{
		SetDlgItemText(IDC_GAME_CURLEVEL, sTemp);
	}

	if (Sparam_Get(sTemp, pMsg, NST_NEXTLEVEL))
	{
		SetDlgItemText(IDC_GAME_NEXTLEVEL, sTemp);
	}


	// Get the current player selection info...

	CString sCurSel;
	CString sCurSelName;
	int     nCurSel = m_lbPlayers.GetCurSel();

	if (nCurSel != LB_ERR)
	{
		m_lbPlayers.GetText(nCurSel, sCurSel);

		int i = 0;

		while (sCurSel[i] >= 32)
		{
			sCurSelName += sCurSel[i];
			i++;
		}
	}


	// Update the player info...

	m_lbPlayers.ResetContent();

	int count    = 0;
	int cPlayers = 0;
	int nNewSel  = nCurSel;

	if (Sparam_Get(sTemp, pMsg, NST_PLRCOUNT))
	{
		count = atoi(sTemp);
	}

	for (int i = 1; i <= count; i++)
	{
		char sBase[32];
		char sName[64];

		int j = i-1;

		wsprintf(sBase, "%s%i", NST_PLRNAME_BASE, i);

		if (Sparam_Get(sName, pMsg, sBase))
		{
			strcpy(m_aPis[j].sName, sName);

			wsprintf(sBase, "%s%i", NST_PLRFRAG_BASE, i);

			if (Sparam_Get(sTemp, pMsg, sBase))
			{
				m_aPis[j].nFrags = atoi(sTemp);

				wsprintf(sBase, "%s%i", NST_PLRID_BASE, i);

				if (Sparam_Get(sTemp, pMsg, sBase))
				{
					m_aPis[j].dwID = atoi(sTemp);
				}
				else
				{
					m_aPis[j].dwID = 0;
				}
			}
		}

		int clientIndex = ClientIDToIndex(m_pServerMgr, m_aPis[j].dwID);
		m_aPis[j].dwPing = 0;
		if(clientIndex != -1)
		{
			ClientInfo info;

			if(m_pServerMgr->GetClientInfo(clientIndex, &info))
			{
				m_aPis[j].dwPing = (DWORD)(info.m_Ping * 1000.0f);
			}
		}

		cPlayers++;
	}


	// Sort the players...

	PLAYERINFO TempPi;

	for (i = 0; i < count - 1; i++)
	{
		for (int j = i+1; j < count; j++)
		{
			BOOL bSwap = FALSE;

			if (m_aPis[i].nFrags < m_aPis[j].nFrags) bSwap = TRUE;

			if (m_aPis[i].nFrags == m_aPis[j].nFrags)
			{
				if (strcmp(m_aPis[i].sName, m_aPis[j].sName) > 0) bSwap = TRUE;
			}

			if (bSwap)
			{
				memcpy(&TempPi,  &m_aPis[i], sizeof(PLAYERINFO));
				memcpy(&m_aPis[i], &m_aPis[j], sizeof(PLAYERINFO));
				memcpy(&m_aPis[j], &TempPi,  sizeof(PLAYERINFO));
			}
		}
	}


	// Add the players to the list box...

	nNewSel = LB_ERR;

	for (i = 0; i < count; i++)
	{
		CString sPlayer;
		sPlayer.Format("(%d)\t%s\t%i", m_aPis[i].dwPing, m_aPis[i].sName, m_aPis[i].nFrags);
		int nIndex = m_lbPlayers.AddString(sPlayer);

		if (nIndex != LB_ERR)
		{
			m_lbPlayers.SetItemData(nIndex, m_aPis[i].dwID);

			if (!sCurSelName.IsEmpty() && strcmp(m_aPis[i].sName, sCurSelName) == 0) nNewSel = nIndex;
		}
	}


	// Reselect the selected player if necessary...

	if (nNewSel != LB_ERR)
	{
		m_lbPlayers.SetCurSel(nNewSel);
	}

	CWnd* pWnd = GetDlgItem(IDC_PLAYERS_BOOT);
	if (pWnd)
	{
		pWnd->EnableWindow(nNewSel != LB_ERR);
	}


	// Check for empty exit...

	if (g_bEmptyExit)
	{
		if (m_cPlayers > 0 && cPlayers == 0)
		{
			WriteConsoleString(IDS_CONSOLE_LASTPLAYERLEFT);
			g_bDoEmptyExit   = TRUE;
			g_timerEmptyExit = g_nEmptyExitDelay * 1000;
		}
		else
		{
			if (cPlayers >= 1) g_bDoEmptyExit = FALSE;
		}
	}


	// Set the number of players...

	m_cPlayers = cPlayers;

	SetDlgItemInt(IDC_GAME_PLAYERS, cPlayers);
}

void CGameServDlg::OnLevelChanging(char* pMsg)
{
	// Flag the the level is changing...
	m_bLevelChanging = TRUE;
	EnterCriticalSection(&m_CS);
}

void CGameServDlg::OnLevelChangeUpdate(char* pMsg)
{
	OnLevelChangeStop();

	// Update the level info...

	char sTemp[128];

	if (Sparam_Get(sTemp, pMsg, NST_CURLEVEL))
	{
		SetDlgItemText(IDC_GAME_CURLEVEL, sTemp);
		m_sCurLevel = sTemp;
	}

	if (Sparam_Get(sTemp, pMsg, NST_NEXTLEVEL))
	{
		SetDlgItemText(IDC_GAME_NEXTLEVEL, sTemp);
	}


	// Reset the level timer...

	m_timeLevelStart = timeGetTime();
}

void CGameServDlg::OnLevelChangeStop()
{
	// Flag the level is done changing and is now changed...
	m_bLevelChanging = FALSE;
	LeaveCriticalSection(&m_CS);
}

void CGameServDlg::OnConsoleMessageUpdate(char* pMsg)
{
	char sTemp[128];

	if (Sparam_Get(sTemp, pMsg, NST_CONSOLEMSG))
	{
		WriteConsoleString(sTemp);
	}
}

void CGameServDlg::OnConsoleOutput(char* pMsg)
{
	WriteConsoleString(pMsg);
}

void CGameServDlg::OnConsoleSend() 
{
	if (!m_pServerMgr) return;

	char sCmd[128];
	sCmd[0] = '\0';
	if (GetDlgItemText(IDC_CONSOLE_COMMAND, sCmd, 120) == 0) return;

	WriteConsoleString(sCmd);
	SetDlgItemText(IDC_CONSOLE_COMMAND, "");

	m_pServerMgr->RunConsoleString(sCmd);

	UpdateConsoleVars();
}

void CGameServDlg::OnConsoleClear() 
{
	SetDlgItemText(IDC_CONSOLE_WINDOW, "");
}

BOOL CGameServDlg::DestroyWindow() 
{
	StopServer();
	Clear();
	return(CDialog::DestroyWindow());
}

void CGameServDlg::OnOutOfMemory()
{
	AfxMessageBox(IDS_ERROR_MEMORY);
	g_bNoExitConfirm = TRUE;
	SendMessage(WM_COMMAND, IDCANCEL);
	exit(0);
}

void CGameServDlg::OnCommandsNextLevel() 
{
	if (m_pServerMgr)
	{
		WriteConsoleString(IDS_NEXTLEVEL);
		m_pServerMgr->SendToServerShell("NEXTLEVEL");
	}
}

void CGameServDlg::OnPlayersBoot() 
{
	int nCurSel = m_lbPlayers.GetCurSel();
	if (nCurSel == LB_ERR) return;

	DWORD dwID = m_lbPlayers.GetItemData(nCurSel);

	m_pServerMgr->BootClient(dwID);
}

void CGameServDlg::OnSelchangePlayersList() 
{
	int nCurSel = m_lbPlayers.GetCurSel();

	CWnd* pWnd = GetDlgItem(IDC_PLAYERS_BOOT);
	if (pWnd)
	{
		pWnd->EnableWindow(nCurSel != LB_ERR);
	}
}

DWORD WINAPI ThreadUpdate(LPVOID lpParameter)
{
	// Sanity checks...

	if (!lpParameter) return(0);


	// Call the update until we are done...

	CGameServDlg* pDlg = (CGameServDlg*)lpParameter;
	HWND           hWnd = pDlg->GetSafeHwnd();

	while (pDlg->IsRunning())
	{
		DWORD timeNow = timeGetTime();

		if (timeNow - g_timeCur >= 8 && !pDlg->IsLevelChanging())
		{

#if USE_POSTMSG
			pDlg->RemoveMessage(WM_TIMER, 500);
			pDlg->PostMessage(WM_TIMER);
			g_dwPosted++;
#else
			EnterCriticalSection(pDlg->GetCS());
			pDlg->OnUpdate();
			LeaveCriticalSection(pDlg->GetCS());
#endif

		}

		Sleep(9);
	}


	// All done...

	s_bThreadDone = TRUE;
	SS_Trace("Thread is exiting...\n");
	return(TRUE);
}

void CGameServDlg::OnCancel() 
{
	if (g_bNoExitConfirm)
	{
		StopServer();
		CDialog::OnCancel();
		return;
	}

	CString str, title;

	str.LoadString(IDS_CHECKEXIT);

#ifdef _ADDON
	title.LoadString(IDS_APPNAME_AO);
#else
	title.LoadString(IDS_APPNAME);
#endif

	if (MessageBox(str, title, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		StopServer();
		CDialog::OnCancel();
	}
}

void CGameServDlg::OnDestroy() 
{
	StopServer();
	CDialog::OnDestroy();
}

void CGameServDlg::RemoveMessage(int nMsg, int nMax)
{
	MSG msg;

	for (int i = 0; i < nMax; i++)
	{
		if (!PeekMessage(&msg, GetSafeHwnd(), nMsg, nMsg, PM_REMOVE))
		{
			return;
		}

		g_dwRemoved++;
	}
}

PLAYERINFO* CGameServDlg::GetFirstPlayerInfo()
{
	if (m_cPlayers <= 0) return(NULL);

	m_iPlayer = -1;

	return(GetNextPlayerInfo());
}

PLAYERINFO* CGameServDlg::GetNextPlayerInfo()
{
	m_iPlayer++;
	if (m_iPlayer >= m_cPlayers) return(NULL);

	return(&m_aPis[m_iPlayer]);
}

void CGameServDlg::OnCommandsOptions() 
{
	NetStart_DoOptionsDialog(GetSafeHwnd(), m_pServerMgr, &g_ServerOptions);
}

void CGameServDlg::OnProcessNetPacket(char* sData, DWORD nLen, BYTE senderAddr[4], DWORD senderPort)
{
	char sAddr[128];

	if (g_ServerInfo.m_bUseGameSpy)
	{
		if(nLen > 0 && sData[0] == '\\')
		{
			sprintf(sAddr, "%d.%d.%d.%d", senderAddr[0], senderAddr[1], senderAddr[2], senderAddr[3]);
			m_GameSpyMgr.OnQuery(sAddr, senderPort, sData, nLen);
		}
	}
}

void CGameServDlg::UpdateConsoleVars()
{
	HCONSOLEVAR hVar = NULL;
	DRESULT     dr   = m_pServerMgr->GetConsoleVar("SpyGameType", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		char sTemp[64] = { "" };
		dr = m_pServerMgr->GetVarValueString(hVar, sTemp, 64);
		if (dr == LT_OK && sTemp[0] != '\0')
		{
			m_sGameSpyGameType = sTemp;
		}
	}

	hVar = NULL;
	dr   = m_pServerMgr->GetConsoleVar("SpyGameMode", &hVar, NULL);
	if (dr == LT_OK && hVar)
	{
		char sTemp[64] = { "" };
		dr = m_pServerMgr->GetVarValueString(hVar, sTemp, 64);
		if (dr == LT_OK && sTemp[0] != '\0')
		{
			m_sGameSpyGameMode = sTemp;
		}
	}
}
