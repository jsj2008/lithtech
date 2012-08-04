//------------------------------------------------------------------
//
//  FILE      : DtxView.cpp
//
//  PURPOSE   :	Defines the class behaviors for the application.
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "DtxView.h"
#include "MainFrm.h"

#include "DtxViewDoc.h"
#include "DtxViewView.h"
#include "AboutDlg.h"


#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


TCHAR szRegKeyCompany[] = _T( "LithTech Inc.\\Jupiter" );


BEGIN_MESSAGE_MAP(CDtxViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()



CDtxViewApp::CDtxViewApp()
{
}


// The one and only CDtxViewApp object
CDtxViewApp theApp;



BOOL CDtxViewApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(szRegKeyCompany);

	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME, RUNTIME_CLASS(CDtxViewDoc), RUNTIME_CLASS(CMainFrame), RUNTIME_CLASS(CDtxViewView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
	{
		return FALSE;
	}

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}



void CDtxViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


//-------------------------------------------------------------------------------------------------
// Lithtech functions
//-------------------------------------------------------------------------------------------------


void* dalloc(unsigned int size)
{
	return (char*)malloc((size_t)size);
}



void dfree(void *ptr)
{
	free(ptr);
}



// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
        return malloc(size);
}



void DefStdlithFree(void *ptr)
{
        free(ptr);
}



void dsi_PrintToConsole(char *pMsg, ...)
{
}



void dsi_OnReturnError(int err)
{
}
