// CoolServ.cpp : Defines the class behaviors for the application.
//


// Includes...

#include "stdafx.h"
#include "CoolServ.h"
#include "GameServDlg.h"
#include "NetStart.h"
#include "ServerUtils.h"
#include "RezFind.h"
#include "Resource.h"


// Macros...

#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)


// Debug...

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Globals...

ServerInfo		g_ServerInfo;
ServerOptions	g_ServerOptions;
NetGame			g_NetGame;
HINSTANCE		g_hInst = NULL;
BOOL			g_bEmptyExit = FALSE;


// Functions...

/////////////////////////////////////////////////////////////////////////////
// CCoolServApp

BEGIN_MESSAGE_MAP(CCoolServApp, CWinApp)
	//{{AFX_MSG_MAP(CCoolServApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolServApp construction

CCoolServApp::CCoolServApp(LPCTSTR lpszAppName) : CWinApp(lpszAppName)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCoolServApp object

CCoolServApp theApp("GameServ");

CCoolServApp* GetTheApp()
{
	return(&theApp);
}

/////////////////////////////////////////////////////////////////////////////
// CCoolServApp initialization

BOOL CCoolServApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	// Get our instance handle...

	g_hInst = AfxGetInstanceHandle();

	char *pDLLName = "server.dll";
	int i;
	for(i=0; i < (__argc-1); i++)
	{
		if(stricmp(__argv[i], "-DLL") == 0)
		{
			pDLLName = __argv[i+1];
			break;
		}
	}

	// Load the server dll...

	if (!LoadServerDLL(pDLLName))
	{
		AfxMessageBox(IDS_ERROR_LOADSERVER);
		return(FALSE);
	}


	// Get our server interface...

	ServerInterface* pServerMgr = GetServerInterface();
	if (!pServerMgr)
	{
		AfxMessageBox(IDS_ERROR_ACCESSSERVER);
		return(FALSE);
	}


	// Check command-line paramters...

	BOOL bNoDlgs = FALSE;

	char sCmdLine[256];
	strcpy(sCmdLine, theApp.m_lpCmdLine);
	strupr(sCmdLine);

	if (strlen(sCmdLine) > 0 && strstr(sCmdLine, "-GO"))
	{
		bNoDlgs = TRUE;
	}

	if (strlen(sCmdLine) > 0 && strstr(sCmdLine, "-EMPTYEXIT"))
	{
		g_bEmptyExit = TRUE;
	}

	char configFile[256];
	strcpy(configFile, "server.cfg");
	LoadString(g_hInst, IDS_CONFIGFILE, configFile, 250);

	for (i = 0; i < (__argc-1); i++)
	{
		if (stricmp(__argv[i], "-CONFIG") == 0)
		{
			strncpy(configFile, __argv[i+1], sizeof(configFile));
		}
	}


	// Check for modifier keys...

	Sleep(50);

	if (IsKeyDown(VK_SHIFT))
	{
		bNoDlgs ^= 1;
	}


	// Find our resources...

	FindRezFiles(g_hInst);

	char* sGameRez = GetGameRezFile();

	if (!sGameRez || sGameRez[0] == '\0')
	{
		AfxMessageBox(IDS_ERROR_REZFILE);
		return(FALSE);
	}


	// Do the wizard...

	BOOL bRet = NetStart_DoWizard(g_hInst, &g_ServerInfo, &g_ServerOptions, &g_NetGame, bNoDlgs, configFile);
	if (!bRet)
	{
		return(FALSE);
	}


	// Do the main dialog...

	CGameServDlg dlg;
	m_pMainWnd = &dlg;

	dlg.SetConfigFilename(configFile);
	int nResponse = dlg.DoModal();
	dlg.Clear();

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}


	// Save out all the console variables.
	NetStart_SaveConsoleVars();
	NetStart_SaveConfigFile(configFile);

	// Free the server library...

	FreeServerDLL();


	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	return FALSE;
}

BOOL CCoolServApp::OnIdle(LONG lCount) 
{
	return CWinApp::OnIdle(lCount);
}

#define ADDREZ(sBase, sPost) \
{ \
	char sRezFile[128]; \
	wsprintf(sRezFile, "%s%s.rez", sBase, sPost); \
	strcpy(sTemps[i], sRezFile); \
	sGame[i] = sTemps[i]; \
	i++; \
}

BOOL CCoolServApp::AddResources(CStringList& collRezFiles)
{
	// Specify the main resources...

	ServerInterface* pServerMgr = GetServerInterface();
	if (!pServerMgr) return(FALSE);

	char  sTemps[128][128];
	char* sGame[128];
	int   i = 0;

	char* sGameRez  = GetGameRezFile();
	char* sSoundRez = GetSoundRezFile();

	sGame[i++] = sGameRez;
	if (sSoundRez && sSoundRez[0] != '\0') sGame[i++] = sSoundRez;

	sGame[i++] = "custom";


	// Specify the patch, test, and language resources...

	char sGameRezBase[64] = { "" };
	LoadString(g_hInst, IDS_REZBASE, sGameRezBase, 60);

	ADDREZ(sGameRezBase, "P");
	ADDREZ(sGameRezBase, "P2");
	ADDREZ(sGameRezBase, "P3");
	ADDREZ(sGameRezBase, "P4");
	ADDREZ(sGameRezBase, "P5");
	ADDREZ(sGameRezBase, "P6");
	ADDREZ(sGameRezBase, "P7");
	ADDREZ(sGameRezBase, "P8");
	ADDREZ(sGameRezBase, "P9");
	ADDREZ(sGameRezBase, "T");
	ADDREZ(sGameRezBase, "L");


	// Specify the custom resources...

	POSITION pos = collRezFiles.GetHeadPosition();

	while (pos)
	{
		CString sRezFile = collRezFiles.GetNext(pos);

		strcpy(sTemps[i], sRezFile);
		sGame[i] = sTemps[i];
		i++;
	}


	// Add the resources...

	char** sResources = sGame;
	DBOOL  db;

	{
		CWaitCursor wc;

		db = pServerMgr->AddResources(sResources, i);
	}

	if (db != DTRUE)
	{
		AfxMessageBox(IDS_ERROR_LOADREZ);
		return(FALSE);
	}


	// All done...

	return(TRUE);
}

