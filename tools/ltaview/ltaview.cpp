// ltaview.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ltaview.h"
#include "tdguard.h"

#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp

BEGIN_MESSAGE_MAP(CLTAViewApp, CWinApp)
	//{{AFX_MSG_MAP(CLTAViewApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp construction

CLTAViewApp::CLTAViewApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLTAViewApp object

CLTAViewApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp initialization

BOOL CLTAViewApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
        ExitProcess(0);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("LithTech Inc.\\Jupiter"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);


	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	// command line was passed assume it is a filename
	if ( this->m_lpCmdLine )
	{
		char szTemp[256] = "" ;

		// not enough chars in the name  
		if( strlen( this->m_lpCmdLine ) > 4 )
		{

			char * lpTemp = this->m_lpCmdLine;

			// parse out the ""
			unsigned int y=0;

			for ( unsigned int x=0; x < strlen(this->m_lpCmdLine); x++ )
			{
				if ( *lpTemp != 0x22 )
				{
					szTemp[y] = *lpTemp;
					y++;
				}

				lpTemp++;

			}

			szTemp[y] = 0;

			int pos= strlen ( szTemp ) - 4;

			char szFileExt[5]="";

			strncpy ( szFileExt, &szTemp[pos],  4)  ;

			szFileExt[4] = 0;

			//  tolower() on the string
			for( int i = 0 ; i < 5 ; i ++ )
			{
				szFileExt[i] = tolower( szFileExt[i] );
			}

			if ( !strncmp( szFileExt, ".lta", 4 ) || 
				  !strncmp( szFileExt, ".ltc", 4 )     )
			{
				pFrame->OpenFileCmdLine( szTemp );
			}
		}
	}


	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CLTAViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp message handlers

