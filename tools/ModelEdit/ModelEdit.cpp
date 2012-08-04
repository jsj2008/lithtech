// ModelEdit.cpp : Defines the class behaviors for the application.
//

#include "precompile.h"
#include "modeledit.h"
#include "modeleditdlg.h"
#include "tdguard.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR szRegKeyCompany[] = _T( "LithTech Inc." );
TCHAR szRegKeyApp[] = _T( "Jupiter" );
TCHAR szRegKeyVer[] = _T( "ModelEdit" );
char g_szStartPath[_MAX_PATH];	// working directory at launch
char g_szExePath[_MAX_PATH];	// path to executable


/////////////////////////////////////////////////////////////////////////////
// CModelEditApp

BEGIN_MESSAGE_MAP(CModelEditApp, CWinApp)
	//{{AFX_MSG_MAP(CModelEditApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModelEditApp construction

CModelEditApp::CModelEditApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CModelEditApp object

CModelEditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CModelEditApp initialization

BOOL CModelEditApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.


//	CMyCommandLineParse cmdLine;	// Note: For parsing command lines (took this out for now) -JE
//	ParseCommandLine(cmdLine);

//	// If it's a known process command, go ahead and run it...
//	const char* szCommand = cmdLine.GetProcessCmd();
//	bool bSuccessfulProcess = false;
//	if (szCommand) {
//		if (stricmp(szCommand,"export_ltb_d3d")==0) {
//			bSuccessfulProcess = ExportLTB_D3D(&cmdLine); }
//		// Put other types of processing here...
//	}
//	if (bSuccessfulProcess) return FALSE;

	_getcwd(g_szStartPath,_MAX_PATH);	// Save off the starting path...

	// save off the path to the executable
	char exePath[_MAX_PATH];
	if( GetModuleFileName( NULL, exePath, _MAX_PATH ) )
	{
		*strrchr( exePath,'\\') = '\0';
		strcpy( g_szExePath, exePath );
	}
	else
		g_szExePath[0] = '\0';

	CModelEditDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
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

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
