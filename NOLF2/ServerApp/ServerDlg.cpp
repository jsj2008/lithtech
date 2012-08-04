// ServerDlg.cpp : implementation file
//


#include "stdafx.h"
#include "Server_Interface.h"
#include "ServerApp.h"
#include "ServerDlg.h"
#include <stdio.h>
#include <afxtempl.h>
#include <process.h>
#include "mmsystem.h"
#include "splash.h"
#include "SelectConfigDlg.h"
#include "NetDefs.h"
#include "versionmgr.h"
#include "ButeMgr.h"
#include "AutoMessage.h"
#include "CommonUtilities.h"
#include "ScmdConsole.h"
#include "ScmdConsoleDriver_ServerApp.h"
#include "ResShared.h"
#include "RegMgr32.h"

#define MIN_SERVER_UPDATE 33 // At least 33 ms between server updates = cap at 30fps

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char const REGPRODUCTNAME[] = "No One Lives Forever 2";
char const REGPRODUCTVER[] = "1.0";

//#define DIR_CUSTOM					"Custom"
#define DIR_MODS					"Custom\\Mods"
#define DIR_RESOURCES				"Custom\\Resources"
#define FIELD_SELECTEDMOD			"SelectedMod"

CServerDlg*			g_pDialog = NULL;


// Get our engine interface pointers.
SETUP_GPLTSERVER();

// Column identifiers for player list control.
enum PlayerColumns
{
	ePlayerColumnsName,
	ePlayerColumnsPing,
	ePlayerColumnsKills,
	ePlayerColumnsTags,
	ePlayerColumnsScore,
	ePlayerColumnsTime,
};

enum
{
	// Maximum number of rez files to use when launching game.
	eMaxRezFiles = 32
};


LTRESULT CGameServerAppHandler::ShellMessageFn( ILTMessage_Read& msg )
{
	if (g_pDialog)
	{
		g_pDialog->OnShellMessage( msg );
	}

	return(LT_OK);
}

LTRESULT CGameServerAppHandler::ConsoleOutputFn( const char *pMsg )
{
	if (g_pDialog)
	{
		g_pDialog->OnShellConsoleOutput( pMsg );
	}

	return(LT_OK);
}

LTRESULT CGameServerAppHandler::OutOfMemory()
{
	if (g_pDialog)
	{
		g_pDialog->OnShellFatalError( IDS_ERROR_MEMORY );
	}

	return(LT_OK);
}


/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerDlg)
	m_sServerName = _T("");
	m_sGameType = _T("");
	m_sServerTime = _T("");
	m_sTimeInLevel = _T("");
	m_sNumPlayers = _T("");
	m_sTotalPlayers = _T( "0/0" );
	m_nPeakPlayers = 0;
	m_nAveragePing = 0;
	//}}AFX_DATA_INIT

	m_bCSInitted = FALSE;
	memset(&m_CS, 0, sizeof(m_CS));

	m_nTotalPlayers = 0;
	m_nUniquePlayers = 0;
	m_nCurLevel_Shell = -1;

	m_hThread = NULL;
	m_bConfirmExit = TRUE;
	m_pServerMgr = NULL;

	m_bFirstShow = TRUE;
	g_pDialog = this;

	m_nMaxPlayers = 0;
	m_nGamePlayers = 0;

	m_bWriteErrorLog = false;
	m_bAlwaysFlushLog = false;
	m_pErrorLogFile = NULL;

	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CServerDlg::~CServerDlg() 
{
	if (m_pImageList)
		delete m_pImageList;

	StopServer(); 
	CloseHandle(m_hStopEvent);

	TermErrorLog( );
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
	DDX_Control(pDX, IDC_COMMANDS_SELLEVEL, m_SelectLevel);
	DDX_Control(pDX, IDC_PLAYER_BOOT, m_PlayerBoot);
	DDX_Control(pDX, IDC_PLAYERS, m_Players);
	DDX_Control(pDX, IDC_LEVELS, m_Levels);
	DDX_Control(pDX, IDC_CONSOLE_WINDOW, m_edConsole);
	DDX_Text(pDX, IDC_SERVER_NAME, m_sServerName);
	DDX_Text(pDX, IDC_GAME_TYPE, m_sGameType);
	DDX_Text(pDX, IDC_SERVER_TIME, m_sServerTime);
	DDX_Text(pDX, IDC_TIMEINLEVEL, m_sTimeInLevel);
	DDX_Text(pDX, IDC_NUM_PLAYERS, m_sNumPlayers);
	DDX_Text(pDX, IDC_PEAKPLAYERS, m_nPeakPlayers);
	DDX_Text(pDX, IDC_TOTALPLAYERS, m_sTotalPlayers);
	DDX_Text(pDX, IDC_AVERAGEPING, m_nAveragePing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	//{{AFX_MSG_MAP(CServerDlg)
	ON_BN_CLICKED(IDC_CONSOLE_SEND, OnConsoleSend)
	ON_BN_CLICKED(IDC_CONSOLE_CLEAR, OnConsoleClear)
	ON_BN_CLICKED(IDC_COMMANDS_NEXTLEVEL, OnCommandsNextLevel)
	ON_BN_CLICKED(IDC_COMMANDS_SELLEVEL, OnCommandsSelectLevel)
	ON_BN_CLICKED(IDC_PLAYER_BOOT, OnPlayersBoot)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDC_LEVELS, OnDblclkLevels)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_STOPSERVER, OnStopserver)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LEVELS, OnItemchangedLevels)
	ON_WM_CLOSE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PLAYERS, OnItemchangedPlayers)
	ON_COMMAND( IDC_STARTUP, OnStartup )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool CServerDlg::LoadServer()
{
	CWaitCursor wc;
	CString sWorlds, sWorldsKey, sWorld;
	SI_CREATESTATUS status;

	// Start fresh.
	StopServer( );

	WriteConsoleString(IDS_CONSOLE_INITSERVER);

	// Initialzie our critical section for the server thread.
	if( !m_bCSInitted )
	{
		InitializeCriticalSection(&m_CS);
		m_bCSInitted = TRUE;
	}

	// Create the server.
	status = CreateServer( SI_VERSION, GAMEGUID, &m_pServerMgr );
	if( status != 0 || !m_pServerMgr )
	{
		OnShellFatalError( IDS_NETERR_GENERIC );
		return(FALSE);
	}

	// Merge our interface database with the database in the DLL we just loaded.
	HMODULE hModule = GetModuleHandle( "server.dll" );
	TSetMasterFn pSetMasterFn = (TSetMasterFn)GetProcAddress( hModule, "SetMasterDatabase" );
	// Check if the function existed.
	if (pSetMasterFn != NULL) 
	{
		// Merge our database with theirs
		pSetMasterFn(GetMasterDatabase());
	}

	// Set our app handler.
	if( m_pServerMgr->SetAppHandler( &m_AppHandler ) != LT_OK )
	{
		OnShellFatalError( IDS_NETERR_GENERIC );
		return FALSE;
	}

	// Add in the resources.
	if( !AddResources( ))
	{
		OnShellFatalError( IDS_ERROR_LOADREZ );
		return FALSE;
	}

	static ScmdConsoleDriver_ServerApp scmdConsoleDriver_ServerApp;
	if( !ScmdConsole::Instance( ).Init( scmdConsoleDriver_ServerApp ))
		return false;

	// Clear some shell variables.
	m_lstShellMsgs_Shell.RemoveAll( );
	m_nCurLevel_Shell = -1;
	m_sCurLevel_Shell = "";
	m_bStopServer_Shell = FALSE;
	m_nCurLevel = -1;
	m_bPlayerUpdate_Shell = FALSE;
	m_bChangingLevels_Shell = FALSE;
	m_bChangedLevel_Shell = FALSE;

	WriteConsoleString( IDS_CONSOLE_SERVERINITED );

	return(TRUE);
}

bool CServerDlg::RunServer()
{
	CWaitCursor wc;

	if( !m_pServerMgr )
		return FALSE;

	// Set our connection service.
	if( !SetService( ))
	{
		OnShellFatalError( IDS_NETERR_SETNETSERVICE );
		return FALSE;
	}

	// Host the game.
	DWORD nHostError = DoHost( );

	if(nHostError)
	{
		OnShellFatalError( nHostError );
		return FALSE;
	}

	// Update the full ip address
	char sAddress[128];
	unsigned short nPort;
	m_pServerMgr->GetTcpIpAddress( sAddress, 128, nPort );

	m_sFullTcpIpAddress = sAddress;
	if( nPort > 0 && nPort != DEFAULT_PORT )
	{
		char sTemp[32];
		wsprintf( sTemp, ":%i", nPort );
		m_sFullTcpIpAddress += sTemp;
	}

	// Tell server about the game info.
	ServerGameOptions* pServerGameOptions = &m_ServerGameOptions;
	m_pServerMgr->SetGameInfo( &pServerGameOptions, sizeof( pServerGameOptions ));


	// Add in the servershell
	if( !m_pServerMgr->LoadBinaries( ))
	{
		OnShellFatalError( IDS_ERROR_BINARIES );
		return FALSE;
	}

	// Make sure the stop event is reset
	ResetEvent(m_hStopEvent);

	// Create the thread to do our updating.
	m_hThread = ( HANDLE )_beginthread( ThreadUpdate, 0, this );
	if(( int )m_hThread == -1 )
	{
		StopServer( );
		return FALSE;
	}

	// Give us a timer update every 100 ms.
	SetTimer( 0, 100, NULL );

	// Init some stuff now that we're running...
	m_serverStartTime = CTime::GetCurrentTime( );

	// Tell server shell to be initialized.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERSHELL_INIT );

	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	if( m_pServerMgr->SendToServerShell( *msgRefRead ) != LT_OK )
	{
		g_pDialog->OnShellFatalError( IDS_ERROR_LOADWORLD );
		return FALSE;
	}

	// All done...
	WriteConsoleString(IDS_CONSOLE_SERVERRUNNING);

	return true;
}

bool CServerDlg::StopServer( )
{
	CPlayerInfo *pPlayerInfo;
	WORD nKey;
	POSITION pos;

	// Kill the thread.
	if( m_hThread )
	{
		SetEvent(m_hStopEvent);

		DWORD dwRet = WaitForSingleObject( m_hThread, 5000 );
		if( dwRet == WAIT_TIMEOUT)
		{
			// Kill thread forcefully.
			TerminateThread( m_hThread, 0 );
			CloseHandle( m_hThread );
		}

		// Reset the stop event since it's successfully stopped
		ResetEvent(m_hStopEvent);

		m_hThread = NULL;

		WriteConsoleString( IDS_CONSOLE_SERVERSTOPPED );
	}

	// Delete the server interface.
	if( m_pServerMgr )
	{
		m_pServerMgr->SetAppHandler( NULL );
		DeleteServer();
		m_pServerMgr = NULL;
	}

	// Clear out variables.
	m_lstShellMsgs_Shell.RemoveAll( );
	m_nCurLevel_Shell = -1;
	m_sCurLevel_Shell = "";
	m_bStopServer_Shell = FALSE;

	// Clear out the player info list.
	pos = m_mapPlayerInfo_Shell.GetStartPosition( );
	while( pos )
	{
		m_mapPlayerInfo_Shell.GetNextAssoc( pos, nKey, ( void *& )pPlayerInfo );
		delete pPlayerInfo;
	}
	m_mapPlayerInfo_Shell.RemoveAll( );

	// Get rid of critical section.
	if( m_bCSInitted )
	{
		DeleteCriticalSection(&m_CS);
		m_bCSInitted = FALSE;
	}

	return TRUE;
}


bool CServerDlg::SetService( )
{
	NetService *pCur, *pListHead;
	HNETSERVICE hNetService;

	// Make sure we have server interface.
	if( !m_pServerMgr )
		return FALSE;

	pCur      = NULL;
	pListHead = NULL;
	hNetService = NULL;

	// Initialize the networking.
	if( !m_pServerMgr->InitNetworking( NULL, 0 ))
		return FALSE;

	// Get the comm services the server is supporting.
	if( !m_pServerMgr->GetServiceList( pListHead ) || !pListHead )
		return FALSE;

	// Find the TCPIP service.
	pCur = pListHead;
	while( pCur )
	{
		if( pCur->m_dwFlags & NETSERVICE_TCPIP )
		{
			hNetService = pCur->m_handle;
			break;
		}

		pCur = pCur->m_pNext;
	}

	// Free the service list.
	m_pServerMgr->FreeServiceList( pListHead );

	// Check if tcp not found.
	if( !hNetService )
		return FALSE;

	// Select it.
	if( !m_pServerMgr->SelectService( hNetService ))
		return FALSE;

	return TRUE;
}

DWORD CServerDlg::DoHost( )
{
	CWaitCursor wc;
	
	// Make sure we have server interface.
	if( !m_pServerMgr )
		return IDS_CONSOLE_UNABLETOHOST;

	// Process command line for the config file to use.
	ParseCommandLine( true, &m_ServerGameOptions );

	// Check if no profile was specified.
	if( m_ServerGameOptions.m_sProfileName.empty( ))
	{
		// Ask the user to select from a config.
		CSelectConfigDlg dlg;
		if( dlg.DoModal( ) != IDOK )
		{
			return IDS_NETERR_NOCONFIGS;
		}

		// Get the config the user selected.
		m_ServerGameOptions.m_sProfileName = dlg.GetSelectedConfig( );

		// If the config is still empty, then we cannot proceed.
		if( m_ServerGameOptions.m_sProfileName.empty( ))
		{
			return IDS_NETERR_NOCONFIGS;
		}
	}

	// Get the path to the profile file for parsing.
	std::string sProfilePath = GetProfileFile( m_ServerGameOptions.m_sProfileName.c_str( ));

	// Parse the bute file.  If we didn't find the 
	CButeMgr userProfile;
	if( !userProfile.Parse( sProfilePath.c_str( )))
		return IDS_NETERR_CORRUPTCONFIG;

	if( !m_ServerGameOptions.LoadFromBute( userProfile ))
		return IDS_NETERR_CORRUPTCONFIG;

	// Force this to be a dedicated server.
	m_ServerGameOptions.m_bDedicated = true;

	// Process command line for any overriding game info.
	ParseCommandLine( false, &m_ServerGameOptions );

	// Get the server name so we can display it.
	m_sServerName = m_ServerGameOptions.GetSessionName();

	// Fill in the name of the game type and max players...
	
	m_sGameType.LoadString( m_ServerGameOptions.GetGameTypeStringID() );
	m_nMaxPlayers = m_ServerGameOptions.GetMaxPlayers();
	m_sNumPlayers.Format("%d/%d", 0, m_nMaxPlayers);

	// Set the selected mod so players know what we are playing...

	// Open the registry.
	CRegMgr32 regMgr;
	regMgr.Init();
	if( !regMgr.OpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE", "Monolith Productions", (char*)REGPRODUCTNAME,
		(char *)REGPRODUCTVER ))
	{
		return false;
	}

	char szSelectedMod[256] = {0};
	DWORD bufSize = sizeof(szSelectedMod);
	
	if( regMgr.GetField( FIELD_SELECTEDMOD, szSelectedMod, bufSize ))
	{
		if( szSelectedMod[0] )
		{
			m_ServerGameOptions.m_sModName = szSelectedMod;
		}

	}

	if( !szSelectedMod[0] )
	{
		CString sModName;
		sModName.LoadString( IDS_RETAIL );
		m_ServerGameOptions.m_sModName = sModName;
	}

	
	// Setup the host info.
	NetHost netHost;
	netHost.m_Port = m_ServerGameOptions.m_nPort;
	netHost.m_dwMaxConnections = m_nMaxPlayers;
	strncpy( netHost.m_sName, m_sServerName, sizeof( netHost.m_sName ));
	netHost.m_sName[ sizeof( netHost.m_sName ) - 1 ] = '\0';
	netHost.m_bHasPassword = m_ServerGameOptions.m_bUsePassword;
	netHost.m_nGameType = (uint8)m_ServerGameOptions.m_eGameType;

	// Start hosting.
	if( !m_pServerMgr->HostGame( &netHost ))
		return IDS_CONSOLE_UNABLETOHOST;

	return 0;
}


void CServerDlg::WriteConsoleString(LPCTSTR pMsg, ...)
{
	TCHAR		str[500];
	va_list		marker;
	int			nLen;

	static int nMax = 250;

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
	_vsnprintf( str, sizeof( str ) - 1, pMsg, marker );
	va_end(marker);

	int len = strlen(str);
	if (len > 0)
	{
		if (len > 0 && str[len-1] < 32) str[len-1] = '\0';

		strcat(str, "\r\n");

		nLen = m_edConsole.SendMessage(EM_GETLIMITTEXT, 0, 0);
		m_edConsole.SetSel(nLen, nLen);
		m_edConsole.ReplaceSel(str);

		// Write to log file too.
		WriteToErrorLog( str );
	}
}

void CServerDlg::WriteConsoleString(int nStringID)
{
	CString sMsg;

	if( !sMsg.LoadString( nStringID ))
		return;
	
	WriteConsoleString( sMsg );
}

/////////////////////////////////////////////////////////////////////////////
// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog() 
{
	CRect rect;
	CTimeSpan timeSpan( 0 );
	CString sColumnHeader;

	// Set the title of our main window.
	CString sTitle;
	sTitle.LoadString( IDS_APPREGNAME );
	SetWindowText( sTitle );

	CSplashWnd::EnableSplashScreen( TRUE );
	CSplashWnd::ShowSplashScreen( this ); 

	CDialog::OnInitDialog();

	// Set the server up time.
	m_sServerTime = FormatTime( timeSpan );

	// Init the game progress.
	m_nMaxPlayers = 0;
	m_nGamePlayers = 0;

	m_sTimeInLevel = FormatTime( timeSpan );
	m_levelStartTime = CTime::GetCurrentTime( );
	m_serverStartTime = CTime::GetCurrentTime( );

	// Create the image list
	m_pImageList = new CImageList;
	m_pImageList->Create( 16, 16, TRUE, 9, 1 );
	m_pImageList->Add( AfxGetApp( )->LoadIcon( IDI_SELECTED ));

	// Setup player list.
	m_Players.SetImageList( m_pImageList, LVSIL_STATE );

	sColumnHeader.LoadString( IDS_COLUMNHEADER_NAME );
	m_Players.InsertColumn( ePlayerColumnsName, sColumnHeader );
	sColumnHeader.LoadString( IDS_COLUMNHEADER_PING );
	m_Players.InsertColumn( ePlayerColumnsPing, sColumnHeader );
	sColumnHeader.LoadString( IDS_COLUMNHEADER_KILLS );
	m_Players.InsertColumn( ePlayerColumnsKills, sColumnHeader );
	sColumnHeader.LoadString( IDS_COLUMNHEADER_TAGS );
	m_Players.InsertColumn( ePlayerColumnsTags, sColumnHeader );
	sColumnHeader.LoadString( IDS_COLUMNHEADER_SCORE );
	m_Players.InsertColumn( ePlayerColumnsScore, sColumnHeader );
	sColumnHeader.LoadString( IDS_COLUMNHEADER_TIMEONSERVER );
	m_Players.InsertColumn( ePlayerColumnsTime, sColumnHeader );

	m_Players.GetWindowRect( &rect );
	int nWidth = rect.Width() - 22;

	m_Players.SetColumnWidth( ePlayerColumnsName,	(int)(nWidth * 0.35));
	m_Players.SetColumnWidth( ePlayerColumnsPing,	(int)(nWidth * 0.11));
	m_Players.SetColumnWidth( ePlayerColumnsKills,	(int)(nWidth * 0.11));
	m_Players.SetColumnWidth( ePlayerColumnsTags,	(int)(nWidth * 0.11));
	m_Players.SetColumnWidth( ePlayerColumnsScore,	(int)(nWidth * 0.13));
	m_Players.SetColumnWidth( ePlayerColumnsTime,	(int)(nWidth * 0.18));

	// Disable the boot button.
	m_PlayerBoot.EnableWindow( FALSE );

	// Disable the select level button.
	m_SelectLevel.EnableWindow( FALSE );

	// Setup the world listctrl.
	m_Levels.SetImageList( m_pImageList, LVSIL_STATE );

	sColumnHeader.LoadString( IDS_COLUMNHEADER_WORLD );
	m_Levels.InsertColumn( 0, sColumnHeader );
	m_Levels.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );

	// Set the icon.
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);

	// Parse command line for any startup info.
	ParseCommandLine( false, NULL );

	InitErrorLog( );

	// All done...
	return(TRUE);
}

// Called by free running thread.
void CServerDlg::ServerUpdate()
{
	// Make sure the server is still running.
	if( !m_pServerMgr || m_bStopServer_Shell )
		return;

	// Update everything if we're running...
	if (IsRunning())
	{
		// Update the server manager...
		if ( !m_pServerMgr->Update(0))
		{
			char szError[256];

			// Something went wrong.  Put the error on the console.
			m_pServerMgr->GetErrorString( szError, sizeof( szError ) - 1 );
			m_lstShellMsgs_Shell.AddTail( CString( szError ));
			m_bStopServer_Shell = TRUE;
		}
	}
}

void CServerDlg::OnTimer(UINT nIDEvent) 
{
	UpdateUI( );
	
	CDialog::OnTimer(nIDEvent);
}

void CServerDlg::UpdateUI()
{
	int nPlayer;
	int nCurLevel;
	CString sCurLevel = "";
	CStringList lstShellMsgs;
	POSITION pos;
	bool bPlayerUpdate;
	bool bLevelChanged = FALSE;
	CString sVal;
	CTimeSpan serverRunTime, levelRunTime;
	CTimeSpan playerOnServerTime;
	int nCurSel, nPlayerId;
	WORD nKey;
	CArray< CPlayerInfo, CPlayerInfo & > playerInfo;
	CPlayerInfo *pShellPlayerInfo;

	// Make sure server is running.
	if( !IsRunning())
		return;

	// See if shell wants to stop itself.
	if( m_bStopServer_Shell )
	{
		StopServer( );
	}

	// Check if we're in the middle of changing levels.
	if( m_bChangingLevels_Shell )
		return;

	// Block the shell thread while we make some copies of its data.
	EnterCriticalSection( &m_CS );
		
		bPlayerUpdate = m_bPlayerUpdate_Shell;
		m_bPlayerUpdate_Shell = FALSE;
		nCurLevel = m_nCurLevel_Shell;
		sCurLevel = m_sCurLevel_Shell;
		lstShellMsgs.AddTail( &m_lstShellMsgs_Shell );
		m_lstShellMsgs_Shell.RemoveAll( );
		bLevelChanged = m_bChangedLevel_Shell;
		m_bChangedLevel_Shell = FALSE;

	// Unblock the shell thread.
	LeaveCriticalSection( &m_CS );

	// Update the server time.
	serverRunTime = CTime::GetCurrentTime( ) - m_serverStartTime;
	if( serverRunTime != m_serverRunTime )
	{
		// Set new time.
		m_serverRunTime = serverRunTime;

		// Choose format for time.
		m_sServerTime = FormatTime( m_serverRunTime );
	}

	// Check if we just changed levels.
	if( bLevelChanged )
	{
		m_levelStartTime = CTime::GetCurrentTime( );
		m_sTimeInLevel = FormatTime( CTimeSpan( 0 ));
	}

	// If we're not in a level currently, then there's nothing to update.
	if( nCurLevel < 0 )
		return;

	// Update the level time.
	levelRunTime = CTime::GetCurrentTime( ) - m_levelStartTime;
	if( levelRunTime != m_levelRunTime )
	{
		m_levelRunTime = levelRunTime;

		// Choose format for time.
		m_sTimeInLevel = FormatTime( m_levelRunTime );
	}

	// Get currently selected player id.
	nCurSel = m_Players.GetNextItem( -1, LVNI_SELECTED );
	if( nCurSel != -1 )
		nPlayerId = m_Players.GetItemData( nCurSel );
	else
		nPlayerId = -1;

	// Block the server shell thread while we copy the playerinfo data.
	EnterCriticalSection( &m_CS );

		// Loop over each player in shell's list.
		pos = m_mapPlayerInfo_Shell.GetStartPosition( );
		while( pos )
		{
			// Get the next player info from the shell list.
			m_mapPlayerInfo_Shell.GetNextAssoc( pos, nKey, ( void *& )pShellPlayerInfo );

			if( !pShellPlayerInfo )
			{
				// This shouldn't happen.
				ASSERT( FALSE );
				continue;
			}

			// Make a local copy.
			playerInfo.Add( *pShellPlayerInfo );
		}

	// Unblock the server shell thread.
	LeaveCriticalSection( &m_CS );

	// Make sure there are the right number of items in the list control.
	while( m_Players.GetItemCount( ) < playerInfo.GetSize( ))
	{
		if( m_Players.InsertItem( m_Players.GetItemCount( ), "" ) == -1 )
			break;
	}
	while( m_Players.GetItemCount( ) > playerInfo.GetSize( ))
	{
		if( m_Players.DeleteItem( m_Players.GetItemCount( ) - 1 ) == -1 )
			break;
	}

	// Clear out the average ping value so we can add to it as we
	// loop over the players.
	m_nAveragePing = 0;

	// Loop over each player in the playerinfo array.  Set the listcontrol 
	// items in a index to index mapping.  This may cause playerinfo data
	// to hop from one index to another, but it makes the update very easy.
	// Since most of the time, nothing is changing, this won't cause any
	// flashing on the control.
	for( nPlayer = 0; nPlayer < m_Players.GetItemCount( ); nPlayer++ )
	{
		// Update any shell information for this slot.
		if( bPlayerUpdate )
		{
			// Set the user data for the list control.
			m_Players.SetItemData( nPlayer, playerInfo[nPlayer].m_nClientId );

			// If this is the player we were pointing at before, update our
			// selection.
			if( playerInfo[nPlayer].m_nClientId == nPlayerId )
			{
				m_Players.SetSelectionMark( nPlayer );
			}

			// Update kills.
			sVal.Format( "%d", playerInfo[nPlayer].m_nKills );
			if( m_Players.GetItemText( nPlayer, ePlayerColumnsKills ).Compare( sVal ))
			{
				m_Players.SetItemText( nPlayer, ePlayerColumnsKills, sVal );
			}

			// Update tags.
			sVal.Format( "%d", playerInfo[nPlayer].m_nTags );
			if( m_Players.GetItemText( nPlayer, ePlayerColumnsTags ).Compare( sVal ))
			{
				m_Players.SetItemText( nPlayer, ePlayerColumnsTags, sVal );
			}

			// Update score.
			sVal.Format( "%d", playerInfo[nPlayer].m_nScore );
			if( m_Players.GetItemText( nPlayer, ePlayerColumnsScore ).Compare( sVal ))
			{
				m_Players.SetItemText( nPlayer, ePlayerColumnsScore, sVal );
			}

			// Update the player name.
			sVal = playerInfo[nPlayer].m_szPlayerHandle;
			if( m_Players.GetItemText( nPlayer, ePlayerColumnsName ).Compare( sVal ))
			{
				m_Players.SetItemText( nPlayer, ePlayerColumnsName, sVal );
			}
		}

		// Always update the ping regardless of shell update.
		float fPing = 0.0f;
		DWORD nPing;
		m_pServerMgr->GetClientPing( m_Players.GetItemData( nPlayer ), fPing );
		nPing = ( DWORD )( fPing + 0.5f );
		m_nAveragePing += nPing;
		sVal.Format( "%d", nPing );
		if( m_Players.GetItemText( nPlayer, ePlayerColumnsPing ).Compare( sVal ))
		{
			m_Players.SetItemText( nPlayer, ePlayerColumnsPing, sVal );
		}

		// Update the player time.
		playerOnServerTime = CTime::GetCurrentTime( ) - playerInfo[nPlayer].m_timeOnServer;
		sVal = FormatTime( playerOnServerTime );
		if( m_Players.GetItemText( nPlayer, ePlayerColumnsTime ).Compare( sVal ))
		{
			m_Players.SetItemText( nPlayer, ePlayerColumnsTime, sVal );
		}
	}

	// Set the number of players.
	if( m_Players.GetItemCount( ) != ( int )m_nGamePlayers )
	{
		m_nGamePlayers = m_Players.GetItemCount( );

		m_sNumPlayers.Format("%d/%d", m_nGamePlayers, m_nMaxPlayers);

		// Check if this is the most players we've had at one time.
		if( m_nGamePlayers > m_nPeakPlayers )
		{
			m_nPeakPlayers = m_nGamePlayers;
		}
	}

	// Finish calculation of average ping.
	if( m_nGamePlayers > 0 )
	{
		m_nAveragePing = ( DWORD )(( float )m_nAveragePing / m_nGamePlayers + 0.5f );
	}

	// Change selected level.
	if( nCurLevel != m_nCurLevel )
	{
		// Deselect previous level.
		if( 0 <= m_nCurLevel && m_nCurLevel < m_Levels.GetItemCount( ))
		{
			m_Levels.SetItemState( m_nCurLevel, 0, LVIS_STATEIMAGEMASK );
			m_Levels.RedrawItems( m_nCurLevel, m_nCurLevel );
			m_nCurLevel = -1;
		}

		// Update the current level.
		if( 0 <= nCurLevel && nCurLevel < m_Levels.GetItemCount( ))
		{
			m_Levels.SetItemState( nCurLevel, INDEXTOSTATEIMAGEMASK( 1 ), LVIS_STATEIMAGEMASK );
			m_Levels.RedrawItems( nCurLevel, nCurLevel );

			m_nCurLevel = nCurLevel;
		}
	}

	// See if the level name changed.
	if( bLevelChanged && !sCurLevel.IsEmpty( ))
	{
		if( sCurLevel.CompareNoCase( m_Levels.GetItemText( nCurLevel, nCurLevel )))
		{
			m_Levels.SetItemText( nCurLevel, 0, sCurLevel );
		}
	}

	// Update any console strings from the shell.
	pos = lstShellMsgs.GetHeadPosition( );
	while( pos )
	{
		WriteConsoleString( lstShellMsgs.GetNext( pos ));
	}

	// Send data to dialog.
	UpdateData( FALSE );
}


void CServerDlg::OnShellMessage( ILTMessage_Read& msg )
{
	uint8 nMessageId = msg.Readuint8( );

	// See if the scmdconsole handles it.
	if( ScmdConsole::Instance( ).OnMessage( nMessageId, msg ))
		return;

	switch( nMessageId )
	{
		case SERVERAPP_INIT:
			OnShellInit( msg );
			break;
		case SERVERAPP_ADDCLIENT:
			OnShellAddClient( msg );
			break;
		case SERVERAPP_REMOVECLIENT:
			OnShellRemoveClient( msg );
			break;
		case SERVERAPP_SHELLUPDATE:
			OnShellUpdate( msg );
			break;
		case SERVERAPP_PRELOADWORLD:
			OnShellPreLoadWorld( msg );
			break;
		case SERVERAPP_POSTLOADWORLD:
			OnShellPostLoadWorld( msg );
			break;
		case SERVERAPP_CONSOLEMESSAGE:
		{
			char szMsg[1024];
			msg.ReadString( szMsg, ARRAY_LEN( szMsg ));
			m_lstShellMsgs_Shell.AddTail( szMsg );
			break;
		}
		case SERVERAPP_MISSIONFAILED:
		{
			// Just relay a message back to the server shell...

			CAutoMessage cMsg;
			cMsg.Writeuint8( SERVERSHELL_MISSIONFAILED );

			CLTMsgRef_Read cMsgRef = cMsg.Read();
			
			EnterCriticalSection( &m_CS );
				m_pServerMgr->SendToServerShell( *cMsgRef );
			LeaveCriticalSection( &m_CS );				
			
		}
		break;

		default:
			break;
	}
}

void CServerDlg::OnShellInit( ILTMessage_Read& msg )
{
	char szMission[MAX_PATH*2] = "";
	m_Levels.DeleteAllItems( );

	uint8 nNumMissions = msg.Readuint8( );

	for( int i = 0; i < nNumMissions; i++ )
	{
		msg.ReadString( szMission, ARRAY_LEN( szMission ));

		// Add the mission to the list.
		m_Levels.InsertItem( m_Levels.GetItemCount( ), szMission );
	}

	// Make sure there's at least 1 level specified.
	if( !m_Levels.GetItemCount( ))
	{
		OnShellFatalError( IDS_NETERR_NOMAPS );
	}
}

void CServerDlg::OnShellAddClient( ILTMessage_Read& msg )
{
	CPlayerInfo *pPlayerInfo;
	int nClientId = 0;
	WORD nKey;
	CString sVal;

	// Get the client id for the player.
	nClientId = msg.Readuint16( );

	// Make a key.
	nKey = ( WORD )nClientId;

	// Remove any old players that might be in the player list.
	if( m_mapPlayerInfo_Shell.Lookup( nKey, ( void *& )pPlayerInfo ))
	{
		// Shouldn't happen.
		ASSERT( FALSE );

		m_mapPlayerInfo_Shell.RemoveKey( nKey );
		delete pPlayerInfo;
		pPlayerInfo = NULL;
	}

	// Create a new player.
	pPlayerInfo = new CPlayerInfo;
	if( !pPlayerInfo )
		return;

	// Read in the player info.
	pPlayerInfo->m_nClientId = nClientId;

	// Add the client to our map of unique clients.
	uint16 nPort;
	ClientIP clientIP;
	if( m_pServerMgr->GetClientAddr( nClientId, clientIP.m_nPart, &nPort ))
	{
		m_setUniquePlayers.insert( clientIP );
		m_nUniquePlayers = m_setUniquePlayers.size( );
	}

	// Load the temporary player name.
	sVal.LoadString( IDS_NEWPLAYER );
	strcpy( pPlayerInfo->m_szPlayerHandle, sVal );

	// Set our current time.
	pPlayerInfo->m_timeOnServer = CTime::GetCurrentTime( );

	// Put the player in the list.
	m_mapPlayerInfo_Shell.SetAt( nKey, pPlayerInfo );

	// Signal to updateui that the player info changed.
	m_bPlayerUpdate_Shell = TRUE;

	// Add this player to the total number of visitors.
	m_nTotalPlayers++;

	m_sTotalPlayers.Format( "%d/%d", m_nTotalPlayers, m_nUniquePlayers );

	TRACE( "Adding player (%d).\n", nClientId );
}

void CServerDlg::OnShellRemoveClient( ILTMessage_Read& msg )
{
	int nClientId = 0;
	CPlayerInfo *pPlayerInfo;
	WORD nKey;

	// Get the client id for the player.
	nClientId = msg.Readuint16( );

	// Make a key.
	nKey = ( WORD )nClientId;

	// Remove the player from the list.
	if( m_mapPlayerInfo_Shell.Lookup( nKey, ( void *& )pPlayerInfo ))
	{
		TRACE( "Removing player (%d).\n", nClientId );

		m_mapPlayerInfo_Shell.RemoveKey( nKey );
		delete pPlayerInfo;
		pPlayerInfo = NULL;
	}

	// Signal to updateui that the player info changed.
	m_bPlayerUpdate_Shell = TRUE;
}


// Handler for update message sent by game servershell.
void CServerDlg::OnShellUpdate( ILTMessage_Read& msg )
{
	uint16 nClientId;
	CPlayerInfo *pPlayerInfo;
	WORD nKey;

	// Signal to updateui that the player info changed.
	m_bPlayerUpdate_Shell = TRUE;

	// Keep reading player updates until invalid clientid given.
	while( 1 )
	{
		nClientId = msg.Readuint16( );
		if( nClientId == ( uint16 )-1 )
			break;

		// Make a key.
		nKey = ( WORD )nClientId;

		// Lookup clientid in known clients.
		if( !m_mapPlayerInfo_Shell.Lookup( nKey, ( void *& )pPlayerInfo ))
		{
			// We don't have this client.  Something's skrewy.
			ASSERT( FALSE );
			break;
		}

		// Update the playerinfo.
		msg.ReadString( pPlayerInfo->m_szPlayerHandle, sizeof( pPlayerInfo->m_szPlayerHandle ));
		pPlayerInfo->m_szPlayerHandle[sizeof( pPlayerInfo->m_szPlayerHandle ) - 1] = 0;
		pPlayerInfo->m_nKills = msg.Readint16( );
		pPlayerInfo->m_nTags = msg.Readint16( );
		pPlayerInfo->m_nScore = msg.Readint16( );
#pragma message("FIXFIX:  Make sure Frags Tags and Score is correct!")
	}
}

void CServerDlg::OnShellPreLoadWorld( ILTMessage_Read& msg )
{
	// We're about to change levels.
	m_bChangingLevels_Shell = TRUE;
	m_nCurLevel_Shell = -1;
}

void CServerDlg::OnShellPostLoadWorld( ILTMessage_Read& msg )
{
	// Level change is complete.
	m_bChangingLevels_Shell = FALSE;
	m_bChangedLevel_Shell = TRUE;
	m_nCurLevel_Shell = msg.Readuint16( );
	char szCurLevel[MAX_PATH*2] = "";
	msg.ReadString( szCurLevel, ARRAY_LEN( szCurLevel ));
	m_sCurLevel_Shell = szCurLevel;
}

void CServerDlg::OnShellConsoleOutput( const char* pMsg )
{
	// Record shell messages for UI to grab.
	m_lstShellMsgs_Shell.AddTail( CString( pMsg ));
}

void CServerDlg::OnConsoleSend() 
{
	if( !m_pServerMgr )
		return;

	// Get the string from control.
	char sCmd[128];
	sCmd[0] = '\0';
	if( GetDlgItemText( IDC_CONSOLE_COMMAND, sCmd, 120 ) == 0 )
		return;

	// Check if this is a scmd command.
	if( ScmdConsole::Instance( ).SendCommand( sCmd ))
	{
		// Clear out the string from the control.
		SetDlgItemText(IDC_CONSOLE_COMMAND, "");
		return;
	}

	CString sServer;
	sServer.Format(IDS_SERVER);

	char sMsg[256];
	sprintf(sMsg, "<< %s >>  %s", sServer, sCmd);

	// Update the UI with string.
	WriteConsoleString(sMsg);

	// Clear out the string from the control.
	SetDlgItemText(IDC_CONSOLE_COMMAND, "");

	// Tell the server shell about the console command.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERSHELL_MESSAGE );
	cMsg.WriteString( sCmd );

	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	EnterCriticalSection( &m_CS );
		m_pServerMgr->SendToServerShell( *msgRefRead );
	LeaveCriticalSection( &m_CS );
}

void CServerDlg::OnConsoleClear() 
{
	// Clear the console.
	SetDlgItemText(IDC_CONSOLE_WINDOW, "");
}

BOOL CServerDlg::DestroyWindow() 
{
	// We're shutting down.
	StopServer( );
	return(CDialog::DestroyWindow());
}

void CServerDlg::OnShellFatalError( int nStringId )
{
	AfxMessageBox( nStringId );
	m_bConfirmExit = FALSE;
	SendMessage(WM_COMMAND, IDCANCEL);

	// Call this instead of exit(0), otherwise our destructor won't get called
	::PostQuitMessage(0);
}

void CServerDlg::OnCommandsNextLevel() 
{
	if( !m_pServerMgr )
		return;

	// Send next level message to server.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERSHELL_NEXTWORLD );

	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	EnterCriticalSection( &m_CS );
		m_pServerMgr->SendToServerShell( *msgRefRead );
	LeaveCriticalSection( &m_CS );
}


void CServerDlg::OnCommandsSelectLevel() 
{
	int nWorldIndex;
	POSITION pos;

	if( !IsRunning( ))
		return;

	// Get the selected item.
	pos = m_Levels.GetFirstSelectedItemPosition( );
	if( !pos )
		return;

	// Go to next item.
	nWorldIndex = m_Levels.GetNextSelectedItem( pos );
	SelectLevel( nWorldIndex );
}


bool CServerDlg::SelectLevel( int nLevelIndex ) 
{
	// Check if we're running.
	if( !IsRunning( ))
		return FALSE;

	// Create a message.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERSHELL_SETWORLD );
	cMsg.Writeuint32( nLevelIndex );

	// Send message to the server.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	EnterCriticalSection( &m_CS );
		m_pServerMgr->SendToServerShell( *msgRefRead );
	LeaveCriticalSection( &m_CS );

	return TRUE;
}


void CServerDlg::OnPlayersBoot() 
{
	if( !m_pServerMgr )
		return;

	// Get the player selected.
	int nCurSel = m_Players.GetNextItem( -1, LVNI_SELECTED );
	if( nCurSel == -1 )
		return;

	// Get the player's id.
	DWORD dwID = m_Players.GetItemData( nCurSel );

	// Boot that player.
	EnterCriticalSection( &m_CS );
		m_pServerMgr->BootClient(dwID);
	LeaveCriticalSection( &m_CS );
}

void CServerDlg::ThreadUpdate( void *pParam )
{
	CServerDlg *pServerDlg;

	pServerDlg = ( CServerDlg * )pParam;

	uint32 nLastTime = timeGetTime();

	HANDLE hStopEvent = pServerDlg->m_hStopEvent;
	CRITICAL_SECTION *pCS = pServerDlg->GetCS();

	// Call the update until we are done...
	while(WaitForSingleObject(hStopEvent, 0) == WAIT_TIMEOUT)
	{
		EnterCriticalSection(pCS);
			pServerDlg->ServerUpdate();
		LeaveCriticalSection(pCS);

		// Restrict the server's framerate
		uint32 nCurTime = timeGetTime();
		uint32 nDelay = 0;
		if (((nLastTime + MIN_SERVER_UPDATE) > nCurTime) && (nCurTime > nLastTime))
			nDelay = (nLastTime + MIN_SERVER_UPDATE) - nCurTime;
		// If we're running too fast, slow it down..
		if (nDelay)
		{
			// Allow our sleeping to be interrupted by a terminate request
			WaitForSingleObject(hStopEvent, nDelay);
		}

		// Get the time for the next loop
		nLastTime = timeGetTime();
		// Adjust the time by however much we overslept
		if (((nLastTime - nCurTime) > nDelay) && (nDelay > 0))
		{
			nLastTime -= (nLastTime - (nCurTime + nDelay));
		}
	}

	// All done...
	TRACE( "Game update thread is exiting.\n" );
}

void CServerDlg::OnCancel() 
{
	// Ask the user to confirm stopping server.
	if( m_bConfirmExit )
	{
		if( !ConfirmStop( ))
			return;
	}

	// User really wants to stop the server.
	StopServer( );

	CDialog::OnCancel();
}


static void ReadRezFromRegCommandLine( CStringList& lstRez, CRegMgr32& regMgr, 
									  char const* pszCommandLineNum, char const* pszCommandLineKeyStub )
{
	// Get the number of updates installed.
	DWORD nNum = 0;
	if( !regMgr.GetField(( char* )pszCommandLineNum, &nNum ))
		return;

	char szCommandLineKey[256] = "";
	ConParse parse;
	char szCommandLine[256] = "";

	// Loop through each one, adding the newer, higher numbered ones, to the end
	// so they override the older ones.
	for( DWORD i = 0; i < nNum; i++ )
	{
		// Add any command line stuff added by an Update
		sprintf( szCommandLineKey, "%s%d", pszCommandLineKeyStub, i );
		DWORD nBufSize = sizeof( szCommandLine );
		if( regMgr.GetField( szCommandLineKey, szCommandLine, nBufSize ) && 
			szCommandLine[0] )
		{
			parse.Init( szCommandLine );
			if( g_pCommonLT->Parse( &parse ) == LT_OK )
			{
				// Ignore if empty.
				if( !parse.m_nArgs || !parse.m_Args[0] )
					continue;

				// Add each rez file entry to the listof rez's.
				for( int i = 0; i < parse.m_nArgs - 1; i++ )
				{
					if( stricmp( parse.m_Args[i], "-rez" ) == 0 )
					{
						lstRez.AddTail( parse.m_Args[i+1] );
						i++;
					}
				}
			}
		}
	}
}

bool CServerDlg::AddResources( )
{
	CWaitCursor wc;
	char *apszRezFiles[eMaxRezFiles];
	CStringList lstRez;
	CString sRezKey;
	CString sRez;
	int nRezIndex;
	POSITION pos;
	CButeMgr buteRez;

	// Make sure we have server interface.
	if( !m_pServerMgr ) 
		return FALSE;
	
	// In order to ensure propper use of user created .rez files that are content
	// only, such as a map pack, specify the .rez files in the resources directory 
	// first.  Then our retail game .rez files and finialy the .rez files for the 
	// selected mod, if one was specified.

	
	// Add any user created .rez files found in the resources directory...

	char szFiles[MAX_PATH] = {0};
	sprintf( szFiles, "%s\\*.rez", DIR_RESOURCES);

	CFileFind	cFileFinder;
	CString		sRezFile;

	BOOL bFound = cFileFinder.FindFile( szFiles );
	while( bFound )
	{
		bFound = cFileFinder.FindNextFile();

		// Ignore all directories...

		if( cFileFinder.IsDirectory() || cFileFinder.IsDots() )
			continue;
		
		// Add the .rez file to the command line...
		
		sRezFile.Empty();

		sRezFile = DIR_RESOURCES;
		sRezFile += "\\";
		sRezFile += cFileFinder.GetFileName();

		lstRez.AddTail( sRezFile );
	}
	
	cFileFinder.Close();


	// Add the stock rez files.

	lstRez.AddTail( "game.rez" );
	lstRez.AddTail( "game" );
	lstRez.AddTail( "game2.rez" );
	lstRez.AddTail( "gamedll.rez" );
	lstRez.AddTail( "Sound.rez" );
	lstRez.AddTail( "gamel.rez" );
	lstRez.AddTail( "gamep.rez" );
	lstRez.AddTail( "gamep2.rez" );

	// Open the registry.
	CRegMgr32 regMgr;
	regMgr.Init();
	if( !regMgr.OpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE", "Monolith Productions", (char*)REGPRODUCTNAME,
		(char *)REGPRODUCTVER ))
	{
		return false;
	}

	// Add the rez files from registry.  Add the update after the content so it overrides.
	ReadRezFromRegCommandLine( lstRez, regMgr, "ContentNum", "ContentCommandLine" );
	ReadRezFromRegCommandLine( lstRez, regMgr, "UpdateNum", "UpdateCommandLine" );

	lstRez.AddTail( "custom" );

	// Check if there's a startup file specifying custom rez files.
	if( buteRez.Parse( "customrez.txt" ))
	{
		// Find all the rezfiles.  Add them in order.  The first
		// missed rezkey means there are no more to add.
		for( nRezIndex = 0; ; nRezIndex++ )
		{
			sRezKey.Format( "Rez%d", nRezIndex );
			sRez = buteRez.GetString( "Rez", sRezKey );

			// Check if no more rez files.
			if( !buteRez.Success( ))
			{
				break;
			}

			// Add it to the list.
			lstRez.AddTail( sRez );
		}
	}

	// Now add all the .rez files associated with the selected mod, if we have one...

	char szSelectedMod[256] = {0};
	DWORD bufSize = sizeof(szSelectedMod);
	
	if( regMgr.GetField( FIELD_SELECTEDMOD, szSelectedMod, bufSize ))
	{
		if( szSelectedMod[0] )
		{
			// Search for all .rez files within the selected mod directory...

			sprintf( szFiles, "%s\\%s\\*.rez", DIR_MODS, szSelectedMod );
			
			BOOL bFound = cFileFinder.FindFile( szFiles );
			while( bFound )
			{
				bFound = cFileFinder.FindNextFile();

				// Ignore all directories...

				if( cFileFinder.IsDirectory() || cFileFinder.IsDots() )
					continue;
				
				// Add the .rez file to the command line...
				
				sRezFile.Empty();

				sRezFile = DIR_MODS;
				sRezFile += "\\";
				sRezFile += szSelectedMod;
				sRezFile += "\\";
				sRezFile += cFileFinder.GetFileName();

				lstRez.AddTail( sRezFile );
			}

			// End the search...
			
			cFileFinder.Close();
		}
	}

	// Now add them to the array to pass to the server.
	pos = lstRez.GetHeadPosition( );
	nRezIndex = 0;
	while( pos && nRezIndex < eMaxRezFiles )
	{
		apszRezFiles[nRezIndex] = ( char* )( char const* )lstRez.GetNext( pos );
		nRezIndex++;
	}

	// Add the resources.
	return m_pServerMgr->AddResources(( char const** )apszRezFiles, nRezIndex );
}

bool CServerDlg::ConfirmStop( )
{
	CString str, title;

	// Make sure server is really running.
	if( !IsLoaded( ))
		return TRUE;

	// Ask the user to confirm the server stop.
	str.LoadString(IDS_CHECKEXIT);
	title.LoadString(IDS_APPNAME);
	return ( MessageBox(str, title, MB_YESNO | MB_ICONQUESTION) == IDYES );
}


void CServerDlg::OnDblclkLevels(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos;
	int nWorldIndex;

	pos = m_Levels.GetFirstSelectedItemPosition( );
	if( !pos )
		return;

	nWorldIndex = m_Levels.GetNextSelectedItem( pos );
	SelectLevel( nWorldIndex );

	*pResult = 0;
}

void CServerDlg::ParseCommandLine( bool bCheckForProfile, ServerGameOptions* pServerGameOptions )
{
	int nArg;

	// Search each command and find setting names with matching setting values.
	for( nArg = 1; nArg < __argc; nArg++ )
	{
		// Make sure there's another argument after this one.  You can't just have
		// the setting name without the setting value.
		if( nArg + 1 >= __argc )
			break;

		// Check if they are looking for game info.
		if( !bCheckForProfile )
		{
			if( pServerGameOptions )
			{
				// Check if it's a port setting.
				if( stricmp( __argv[nArg], "-port" ) == 0 )
				{
					// Set the port.
					pServerGameOptions->m_nPort = atoi( __argv[nArg + 1] );

					// Skip the value.
					nArg++;
				}
				// Check if it's the server name.
				else if( stricmp( __argv[nArg], "-name" ) == 0 )
				{
					// Set the server name.
					pServerGameOptions->SetSessionName(__argv[nArg + 1]);

					// Skip the value.
					nArg++;
				}
			}
			else
			{
				// Check if they want to write out an error log.
				if( stricmp( __argv[nArg], "+errorlog" ) == 0 )
				{
					m_bWriteErrorLog = !!atoi( __argv[nArg + 1] );

					// Skip the value.
					nArg++;
				}
				// Check if they want to set the error log filename.
				else if( stricmp( __argv[nArg], "+errorlogfile" ) == 0 )
				{
					// Set the errorlog name.
					m_sErrorLogFileName = __argv[nArg + 1];

					// Skip the value.
					nArg++;
				}
				// Check if they want to always flush the log after every write.
				else if( stricmp( __argv[nArg], "+alwaysflushlog" ) == 0 )
				{
					m_bAlwaysFlushLog = !!atoi( __argv[nArg + 1] );

					// Skip the value.
					nArg++;
				}
			}
		}
		else
		{
			// Check if it's profile file.
			if( stricmp( __argv[nArg], "-profile" ) == 0 )
			{
				// Set the config value.
				pServerGameOptions->m_sProfileName = __argv[nArg + 1];

				// Skip the value.
				nArg++;
			}
		}
	}
}


void CServerDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	// On the first show, startup the server.
	if( bShow && m_bFirstShow )
	{
		// Only count the first show.
		m_bFirstShow = FALSE;
		
		// Now that we've been shown, we can do our startup.
		PostMessage( WM_COMMAND, WPARAM( IDC_STARTUP ));
	}	
}

void CServerDlg::OnStartup( )
{
	// Load up the server.
	if( !LoadServer( ))
	{
		OnShellFatalError( IDS_ERROR_CANTSTARTSERVER );
		return;
	}

	// Run the server.
	if( !RunServer( ))
	{
		OnShellFatalError( IDS_ERROR_CANTSTARTSERVER );
		return;
	}

	// Update our controls.
	UpdateData( FALSE );
}

void CServerDlg::OnStopserver() 
{
	OnCancel( );		
}


CString CServerDlg::FormatTime( CTimeSpan &timeSpan )
{
	CString sTime;

	if( timeSpan.GetDays( ) > 0 )
		sTime = timeSpan.Format( "%Dd:%Hh:%Mm:%Ss" );
	else if( timeSpan.GetHours( ) > 0 )
		sTime = timeSpan.Format( "%Hh:%Mm:%Ss" );
	else
		sTime = timeSpan.Format( "%Mm:%Ss" );

	return sTime;
}

BOOL CServerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if( CSplashWnd::PreTranslateAppMessage( pMsg ))
		return TRUE;
	  
	return CDialog::PreTranslateMessage(pMsg);
}


void CServerDlg::OnItemchangedLevels(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Enable the select level button only when a level is selected.
	POSITION pos = m_Levels.GetFirstSelectedItemPosition( );
	m_SelectLevel.EnableWindow( pos != NULL );

	*pResult = 0;
}

void CServerDlg::OnItemchangedPlayers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Enable the boot player button only when a player is selected.
	POSITION pos = m_Players.GetFirstSelectedItemPosition( );
	m_PlayerBoot.EnableWindow( pos != NULL );
	
	*pResult = 0;
}


bool CServerDlg::InitErrorLog( )
{
	// Make sure they want an errorlog.
	if( !m_bWriteErrorLog )
		return false;

	if( m_sErrorLogFileName.empty( ))
	{
		m_sErrorLogFileName = "nolf2srv.log";
	}

	// Make sure we start fresh.
	TermErrorLog( );

	m_pErrorLogFile = fopen( m_sErrorLogFileName.c_str( ), "wtc" );
	if( !m_pErrorLogFile )
		return false;

	return true;
}

void CServerDlg::TermErrorLog( )
{
	if( !m_pErrorLogFile )
		return;

	fflush( m_pErrorLogFile );
	fclose( m_pErrorLogFile );
	m_pErrorLogFile = LTNULL;
}

bool CServerDlg::WriteToErrorLog( char const* pszMsg )
{
	if( !pszMsg || !m_pErrorLogFile )
		return false;

	fprintf( m_pErrorLogFile, pszMsg );
	if( pszMsg[ strlen( pszMsg ) - 1 ] != '\n' )
		fprintf( m_pErrorLogFile, "\n" );
	
	if( m_bAlwaysFlushLog )
		fflush( m_pErrorLogFile );

	return true;
}

