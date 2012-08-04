// Server.cpp : Defines the class behaviors for the application.
//


// Includes...

#include "stdafx.h"
#include "ServerApp.h"
#include "ServerDlg.h"
#include "Resource.h"

// Globals...

HINSTANCE		g_hInst = NULL;

// Functions...

/////////////////////////////////////////////////////////////////////////////
// CServerApp

BEGIN_MESSAGE_MAP(CServerApp, CWinApp)
	//{{AFX_MSG_MAP(CServerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerApp construction

CServerApp::CServerApp(LPCTSTR lpszAppName) : CWinApp(lpszAppName)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CServerApp object

CServerApp theApp("NOLF2Srv");

CServerApp* GetTheApp()
{
	return(&theApp);
}


/////////////////////////////////////////////////////////////////////////////
// CServerApp initialization

BOOL CServerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Get our instance handle...
	g_hInst = AfxGetInstanceHandle();

	return TRUE;
}

int CServerApp::Run( )
{
	CServerDlg dlg;
	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	return nResponse;
}

