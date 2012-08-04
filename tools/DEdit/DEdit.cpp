//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// DEdit.cpp : Defines the class behaviors for the application.
//

#include "bdefs.h"
#include "butemgr.h"
#include "dedit.h"

#include "optionsdisplay.h"
#include "mainfrm.h"
#include "projectdirdlg.h"

#include "edithelpers.h"

#include "regiondoc.h"
#include "regionview.h"
#include "regionframe.h"
#include "splash.h"

#include "editortransfer.h"
#include "regmgr.h"
#include "resourcemgr.h"

#include "projectbar.h"
#include <afxadv.h>
#include "stringdlg.h"
#include "globalhotkeydb.h"
#include "dedit_concommand.h"
#include "tdguard.h"

#include "newprojectdlg.h"
#include "keyconfigdlg.h"
#include "deditoptions.h"
#include "version_resource.h"

#include "optionsmodels.h"
#include "eventnames.h"
#include "optionsmisc.h"
#include "ltamgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HOTKEY_REG_PATH  "HKEY_CURRENT_USER\\Software\\LithTech Inc.\\Jupiter\\DEdit\\Settings\\Hotkeys"

TCHAR szRegKeyCompany[] = _T( "LithTech Inc.\\Jupiter" );
TCHAR szRegKeyApp[] = _T( "DEdit" );
TCHAR szRegKeyVer[] = _T( "Settings" );

/////////////////////////////////////////////////////////////////////////////
// CDEditApp

BEGIN_MESSAGE_MAP(CDEditApp, CWinApp)
	//{{AFX_MSG_MAP(CDEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEWPROJECT, OnFileNewProject)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_EDIT_KEY_CONFIGURATION, OnEditKeyConfiguration)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_CLOSEPROJECT, OnFileCloseProject)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEPROJECT, OnUpdateFileCloseProject)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_NEWWORLD, OnFileNewWorld)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEWWORLD, OnUpdateFileNewWorld)
	ON_COMMAND(ID_FILE_RELOADWORLD, OnFileReloadWorld)
	ON_UPDATE_COMMAND_UI(ID_FILE_RELOADWORLD, OnUpdateFileReloadWorld)
	ON_COMMAND(ID_FILE_SAVEWORLDAS, OnFileSaveWorldAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEWORLDAS, OnUpdateFileSaveWorldAs)
	ON_COMMAND(ID_FILE_CLOSEWORLD, OnFileCloseWorld)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEWORLD, OnUpdateFileCloseWorld)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_HELP_INTRODUCTION, OnHelpIntroduction)
	ON_COMMAND(ID_HELP_CREATION_GUIDE, OnHelpCreationGuide)
	ON_COMMAND(ID_HELP_WWWDE, OnDEHomePage)
	ON_COMMAND(ID_HELP_ONLINEDOCUMENTATION, OnOnlineDoc)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDEditApp construction

CDEditApp::CDEditApp()
{
	m_pWorldTemplate		= NULL;
	m_TextureDisplaySize	= 64;
	m_bFullUpdate			= TRUE;
	m_pClassHelpButeMgr		= NULL;
	m_pClassHelpButeAgg		= NULL;

	// Init the global managers.
	dedit_InitConsoleCommands();
}


CDEditApp::~CDEditApp()
{
	// Close the palette manager.
	dedit_TermConsoleCommands();

	// Terminate the class help a
	if (m_pClassHelpButeMgr)
	{
		m_pClassHelpButeMgr->Term();
		delete m_pClassHelpButeMgr;
		m_pClassHelpButeMgr=NULL;
	}

	//clean up any old aggreagates
	if(GetClassHelpButeAgg())
	{
		GetClassHelpButeAgg()->Term();
		delete GetClassHelpButeAgg();
	}

	//make sure we don't leak any memory through the global hotkey database
	CGlobalHotKeyDB::ClearAggregateList();

}

BOOL g_bGlobalUndoDisable=FALSE;

static char* FindArg( char *pName )
{
	int		i;
	char	withDash[256];

	sprintf( withDash, "%s%s", "-", pName );

	for( i=0; i < __argc; i++ )
	{
		if( CHelpers::UpperStrcmp(__argv[i], pName) || CHelpers::UpperStrcmp(__argv[i], withDash) )
		{
			if( i < (__argc-1) )
				return __argv[i+1];
			else
				return "";
		}
	}

	return NULL;
}

static void ProcessSpecialArgs()
{
	if(FindArg("noundo"))
		g_bGlobalUndoDisable = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDEditApp object

CDEditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDEditApp initialization

BOOL CDEditApp::InitInstance()
{

	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	//initialize the global hotkey database, and then overwrite the configuration with
	//that found in the registry (this allows new keys to be added to the database
	//and not have to be contained in the registry)
	CGlobalHotKeyDB::Init();
	CGlobalHotKeyDB::m_DB.LoadFromRegistry(m_genRegMgr, HOTKEY_REG_PATH, false);
	
	// Set the working directory and the EXE directory
	GetCurrentDirectory( sizeof(m_WorkingDir), m_WorkingDir );

	char szModuleFilename[MAX_PATH];	
	GetModuleFileName(m_hInstance, (char *)szModuleFilename, MAX_PATH);

	char szDrive[_MAX_DIR];
	char szPath[MAX_PATH];

	_splitpath ( (const char *)szModuleFilename, (char *)szDrive, (char *)szPath, NULL, NULL );	
	sprintf(m_ExeDirectory, "%s%s", szDrive, szPath);	

	RegisterEditorTransferFormat();
	SetRegistryKey( szRegKeyCompany );


	// CG: The following block was added by the Splash Screen component.
	{
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	AfxInitRichEdit();

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	m_pWorldTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,		
		RUNTIME_CLASS(CRegionDoc),
		RUNTIME_CLASS(CRegionFrame), // custom MDI child frame
		RUNTIME_CLASS(CRegionView));
	AddDocTemplate(m_pWorldTemplate);

   // enable file manager drag/drop and DDE Execute open
   EnableShellOpen();
   RegisterShellFileTypes();
   
   LoadRegistryStuff();

   //setup our model manager
   GetModelMgr().Create(GetOptions().GetModelsOptions()->IsRunLowPriority(), GetOptions().GetModelsOptions()->GetMaxMemoryUse());

   //\nWorld\nWorld\nWorlds (*.ed)\n.ed\n
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	//extract the class help file if needed
	if(GetApp()->GetOptions().GetMiscOptions()->IsAutoExtractClassHelp())
	{
		//see if the file exists
		CFileStatus DummyStatus;

		if(!CFile::GetStatus("ClassHlp.but", DummyStatus))
		{
			CProjectBar::CreateFileFromResource(MAKEINTRESOURCE(IDR_CLASSHLP_BUT), "BUT", "ClassHlp.but");
		}
	}

	// Load the bute file for the class help	
	m_pClassHelpButeMgr=new CButeMgr;
	if (!m_pClassHelpButeMgr->Parse("ClassHlp.but"))
	{
		// Delete the ButeMgr file
		delete m_pClassHelpButeMgr;
		m_pClassHelpButeMgr=NULL;
	}
	else
	{
		// Don't display any ButeMgr errors
		m_pClassHelpButeMgr->SetDisplayFunc(NULL);
	}

	// Load the intro if this is the first time running DEdit
	if(m_DEditOptions.IsFirstRun())  
	{
		OnHelpIntroduction();
		m_DEditOptions.SetFirstRun(false);
	}

	// The main window has been initialized, so show and update it.
	pMainFrame->UpdateWindow();

	//open our last project
//	if(( *m_pRecentFileList )[0].GetLength( ) > 0 )
//		GetProjectBar()->Open( ( LPCTSTR )( *m_pRecentFileList )[0] );
	
	ProcessSpecialArgs();


	// Sanity check on Window Size

	RECT screen, window, final;

	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &screen, NULL);

	m_pMainWnd->GetWindowRect(&window);

	// Clip to window size

	screen.top < window.top ? final.top = window.top : final.top = screen.top;
	screen.left < window.left ? final.left = window.left : final.left = screen.left;
	screen.right > window.right ? final.right = window.right : final.right = screen.right;
	screen.bottom > window.bottom ? final.bottom = window.bottom : final.bottom = screen.bottom;

	// Check if window is off screen

	if (window.top > screen.bottom)  final.top = screen.top;
	if (window.left > screen.right)  final.left = screen.left;
	if (window.right < screen.left)  final.right = screen.right;
	if (window.bottom < screen.top)  final.bottom = screen.bottom;

	m_pMainWnd->MoveWindow(&final, true);

	//now load up the last saved project
	if(GetApp()->GetOptions().GetMiscOptions()->IsAutoLoadProj())
	{
		CString sLastProject = GetApp()->GetOptions().GetMiscOptions()->GetStringValue("LastProject", "");
		if(!sLastProject.IsEmpty())
		{
			//load up this project
			GetProjectBar()->Open(sLastProject);
		}
	}

	return TRUE;
}


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
	virtual BOOL OnInitDialog();
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
void CDEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CDEditApp commands

void CDEditApp::OnFileNewProject() 
{
	CString str, projFilename, fileExtension;
	FILE *fp;
	char dirName[500], dirNumString[10], title[200];
	int dirNum;
	DWORD dwAttribs;


	CNewProjectDlg dlg(GetMainWnd());

	// Fill in the defaults.
	for(dirNum=0; dirNum < 32000; dirNum++)
	{
		CHelpers::ExtractNames(m_pszExeName, dirName, NULL, NULL, NULL);
		sprintf(dirNumString, "%d", dirNum);
		
		dlg.m_ProjectDir = dirName;
		dlg.m_ProjectDir = dfm_BuildName(dlg.m_ProjectDir, "Project");
		dlg.m_ProjectDir += dirNumString;

		// If it doesn't exist, terminate the loop.. we've found a good name.
		dwAttribs = GetFileAttributes(dlg.m_ProjectDir);
		if(dwAttribs == 0xFFFFFFFF)
			break;
	}

	// Set the project dir to the full path.
	GetFullPathName(dlg.m_ProjectDir, 500, dirName, NULL);
	dlg.m_ProjectDir = dirName;

	CHelpers::ExtractNames(m_pszExeName, dirName, NULL, NULL, NULL);
	
	// Let them edit the stuff.
	if(dlg.DoModal() == IDOK)
	{
		// Save the current project...
		if(GetProjectBar()->IsProjectOpen())
			GetProjectBar( )->Save( );

		// Create project directory...
		if( !CreateDirectory(dlg.m_ProjectDir, NULL) )
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				str.FormatMessage(IDS_UNABLETOCREATEDIR, dlg.m_ProjectDir);
				AfxGetMainWnd()->MessageBox(str, AfxGetAppName(), MB_OK);
				return;
			}
		}

		fileExtension.LoadString(IDS_DEP_EXTENSION);
		CHelpers::ExtractNames(dlg.m_ProjectDir, NULL, title, NULL, NULL);
		projFilename = dfm_BuildName(dlg.m_ProjectDir, title + fileExtension);

		// Create project file...
		fp = fopen(projFilename, "wb");
		if(fp)
		{
			fclose(fp);
		}
		else
		{
			str.FormatMessage(IDS_ERRORCREATING_PROJECT, projFilename);
			AppMessageBox( str, MB_OK );
			return;
		}

		// Open the new project...
		if( GetProjectBar()->Open( projFilename ))
			AddToRecentFileList( projFilename );
		else
			return;
	}
	else
	{
		return;
	}

	if( !CResourceMgr::CreateDir( "Worlds", RESTYPE_WORLD ) ||
		!CResourceMgr::CreateDir( "Sprites", RESTYPE_SPRITE ) ||
		!CResourceMgr::CreateDir( "Textures", RESTYPE_TEXTURE ) ||
		!CResourceMgr::CreateDir( "Sounds", RESTYPE_SOUND ) ||
		!CResourceMgr::CreateDir( "Models", RESTYPE_MODEL ) ||
		!CResourceMgr::CreateDir( "Prefabs", RESTYPE_PREFAB ) ||
		!CResourceMgr::CreateDir( "Physics", RESTYPE_PHYSICS ))
	{
			str.FormatMessage( IDS_ERRORCREATING_PROJECT, title );
			AppMessageBox( str, MB_OK );
			return;
	}

	//prompt for new world
	if(AfxMessageBox(IDS_CREATENEWORLD,MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CStringDlg			dlg;
		CString				worldpath,fileName, extension, fullName;
		CMoFileIO			file;
		BOOL				bCreate = TRUE;
		int				status;

		dlg.m_bAllowFile = TRUE;
		dlg.m_MaxStringLen = 70;
		dlg.m_bBeeping = TRUE;

		worldpath = "Worlds";

		if( dlg.DoModal(IDS_NEWWORLD, IDS_ENTERWORLDNAME) == IDOK )
		{
			extension.LoadString( IDS_WORLDEXTENSION );
			fileName = dfm_BuildName(worldpath, dlg.m_EnteredText + extension);
			fullName = dfm_GetFullFilename( GetFileMgr( ), fileName );

			// If the file already exist, ask if they want to overwrite...
			if( dfm_OpenFileRelative( GetFileMgr(), fileName, file ))
			{
				file.Close( );

				str.FormatMessage( IDS_WORLD_ALREADY_EXISTS, dlg.m_EnteredText );
				status = AfxMessageBox( str, MB_YESNOCANCEL | MB_ICONQUESTION );
			
				if( status == IDNO )
				{
					bCreate = FALSE;
				}
				else if( status == IDCANCEL )
				{
					return;
				}
			}

			if( bCreate )
			{
				fp = fopen( fullName, "wb" );
				if( !fp )
				{
					str.FormatMessage( IDS_ERRORCREATING_WORLD, dlg.m_EnteredText );
					AppMessageBox( str, MB_OK );
					return;
				}
				fclose( fp );
			}

			GetProjectBar( )->OpenRegionDoc( fullName );
		
			GetProjectBar( )->UpdateAll();
		}
	}
}

//----------------------------------------------------------------------------------------
//
//  CDEditApp::OnFileOpen() 
//
//  Purpose:	Opens project file.  This is done through a common open file dialog.  The
//				path is sent to the project bar object, which loads the layout file.  If
//				everything is successful, the file is added to the recent file list.
//
//----------------------------------------------------------------------------------------
void CDEditApp::OnFileOpen() 
{
	CString			ext, filter;

	ext.LoadString( IDS_DEP_EXTENSION );
	filter.LoadString( IDS_FILEOPEN_FILTER );

	CFileDialog		dlg( TRUE, ext, NULL, 
							OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
							filter, AfxGetMainWnd( ));

	if( dlg.DoModal() == IDOK )
	{
		if( strlen( GetProjectBar( )->GetProjFilename( )))
			GetProjectBar( )->Save( );
		if( GetProjectBar()->Open( dlg.GetPathName( )))
			AddToRecentFileList( dlg.GetPathName( ));
	}
}


//----------------------------------------------------------------------------------------
//
//  CDEditApp::OnOpenRecentFile() 
//
//  Purpose:	Overrides default CWinApp::OnOpenRecentFile().  The override was needed
//				because the project file isn't a CDocument and doesn't follow the 
//				Doc/View framework.
//
//----------------------------------------------------------------------------------------
BOOL CDEditApp::OnOpenRecentFile(UINT nID)
{
	TCHAR szTemp[_MAX_PATH];

	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE2("MRU: open file (%d) '%s'.\n", (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

	lstrcpy(szTemp, ( LPCTSTR )( *m_pRecentFileList )[ nIndex ]);
	if( strlen( GetProjectBar( )->GetProjFilename( )))
		GetProjectBar( )->Save( );
	if( GetProjectBar()->Open( szTemp ))
		AddToRecentFileList( szTemp );
	else
		m_pRecentFileList->Remove(nIndex);

	return TRUE;
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the warning and build text
	CEdit *pEdit;
	CString str;

	pEdit = (CEdit *)GetDlgItem(IDC_ABOUTWARN);
	if( pEdit )
	{
		str.LoadString( IDS_ABOUTWARN );
		pEdit->SetWindowText( str );
	}

	pEdit = (CEdit *)GetDlgItem(IDC_BUILDSTRING);
	if (pEdit)
	{
		LTVersionInfo cVersion;
		char *pStr = str.GetBufferSetLength(256);
		if (GetLTExeVersion(GetApp()->m_hInstance, cVersion) == LT_OK)
			cVersion.GetString(pStr, 256);
		else
			str.LoadString( IDS_BUILDSTRING );
		pEdit->SetWindowText( str );
	}

	return TRUE;
}


BOOL CDEditApp::OnIdle( LONG lCount )
{
	CDocument			*pDoc;
	CRegionView			*pView;
	POSITION			docPos, viewPos;

	//SCHLEGZ: for Auto-Save
	static DWORD lastSaveTime = timeGetTime();
	
	switch( lCount )
	{
		case 0:
		{
			// Call all the views.
			for( docPos = m_pWorldTemplate->GetFirstDocPosition(); docPos != NULL; )
			{

				pDoc = m_pWorldTemplate->GetNextDoc( docPos );
			
				//let the lightmaps be calculated
				CEditRegion* pRegion = ((CRegionDoc*)pDoc)->GetRegion();

				for( viewPos = pDoc->GetFirstViewPosition(); viewPos != NULL; )
				{
					pView = (CRegionView*)pDoc->GetNextView( viewPos );
					
					if( pView->IsKindOf( RUNTIME_CLASS(CRegionView) ) )
					{
						pView->OnIdle( lCount );
					}
				}

				//added extra features to autosaving, such as the ability to change
				//the path and other fun stuff -JohnO

				//see if we need to test for autosaving
				if(GetApp()->GetOptions().IsAutoSave())
				{
					// Get the autosave time
					DWORD dwAutoSaveTime = GetApp()->GetOptions().GetAutoSaveTime();
					if((timeGetTime() - lastSaveTime) > (dwAutoSaveTime * 60000))
					{
						CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

						//update the status bar so that it will reflect the state
						CString csTemp = "Auto-saving: " + ((CRegionDoc*)pDoc)->m_FileName;
						pFrame->UpdateStatusText(0,( char * )LPCTSTR( csTemp ));

						//update the backup filename in case the options changed
						((CRegionDoc*)pDoc)->UpdateBackupName(((CRegionDoc*)pDoc)->m_FileName);

						//save the actual backup file
						((CRegionDoc*)pDoc)->SaveLTA(FALSE);

						//save the current time for the next time around
						lastSaveTime = timeGetTime();

						//return the status to ready
						pFrame->UpdateStatusText(0,"Ready");
					}
				}
			}
			break;
		}
	}

	return CWinApp::OnIdle( lCount );
}


BOOL CDEditApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinApp::PreTranslateMessage(pMsg);
}

void CDEditApp::LoadRegistryStuff()
{	
	// Load the options
	m_DEditOptions.Init(&m_genRegMgr, "HKEY_CURRENT_USER\\Software\\LithTech Inc.\\Jupiter\\DEdit");
	m_DEditOptions.Load();

	/*
	CString sProject=m_DEditOptions.GetStringValue("Project", "");
	if (sProject.GetLength() > 0)
	{
		CMoFileIO file;
		if( file.Open(sProject, "rb"))
		{
			file.Close();
			GetProjectBar()->Open( sProject );
		}
	}
	*/
}


void CDEditApp::SaveRegistryStuff()
{
	// Save the current project	
	// m_DEditOptions.SetStringValue("Project", GetProjectBar()->m_ProjectFileName);	
	
	// Save the options
	m_DEditOptions.Save();
}

BOOL CDEditApp::SaveAllModified() 
{
	//GetProjectBar()->Save();	
	SaveRegistryStuff();

	return CWinApp::SaveAllModified();
}

void CDEditApp::OnHelpIntroduction()
{
	char szPath[MAX_PATH];
	bool bShowErrors = !m_DEditOptions.IsFirstRun();

	sprintf(szPath, "%s..\\..\\..\\docs\\DEditWelcome\\welcome.htm", GetApp()->m_ExeDirectory);
	int shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath,NULL,NULL,SW_SHOWNORMAL);

	// Check installed path in registry if we don't find the file in the normal location
	if ((shellVal <= 32) && ((shellVal == ERROR_FILE_NOT_FOUND) || (shellVal == ERROR_PATH_NOT_FOUND)))
	{
		char szPath2[MAX_PATH];
		HKEY hKey;
		unsigned long dataSize = sizeof(char) * MAX_PATH, keyType = REG_SZ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\LithTech Inc.\\Jupiter\\", NULL, KEY_ALL_ACCESS, &hKey) == NO_ERROR)
		{
			if (RegQueryValueEx(hKey, "InstallDir", NULL, &keyType, (unsigned char *)szPath, &dataSize) == NO_ERROR)
			{
				if(szPath)
				{
					if (szPath[0] == '\"') // Remove quotes from path
					{
						int stringLen = strlen(szPath)-2;			ASSERT(stringLen > 0);
						strncpy(szPath2, &szPath[1], stringLen);
						szPath2[stringLen] = '\0';
						strcpy(szPath, szPath2);
					}
					sprintf(szPath2, "%s\\docs\\DEditWelcome\\welcome.htm", szPath);
					shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath2,NULL,NULL,SW_SHOW);
				}
			}
			else
			{
				shellVal = 33; // in order to skip switch below
				if(bShowErrors)  AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
			}

			RegCloseKey(hKey);
		}
		else
		{
			shellVal = 33; // in order to skip switch below
			if(bShowErrors)  AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
		}
	}


	if (shellVal <= 32)
	{
		switch (shellVal)
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				if(bShowErrors)  AppMessageBox(AFX_IDP_NO_INTRODUCTION, MB_OK);
				break;

			case SE_ERR_ASSOCINCOMPLETE:
			case SE_ERR_NOASSOC:
				if(bShowErrors)  AppMessageBox(AFX_IDP_NO_HTML_READER, MB_OK);
				break;

			default:
				CString str;
				str.FormatMessage(AFX_IDP_GENERIC_INTRODUCTION_FALURE, shellVal);
				if(bShowErrors)  AppMessageBox(str, MB_OK);
		}
	}

	return;
}

void CDEditApp::OnHelpCreationGuide()
{
	char szPath[MAX_PATH];
	sprintf(szPath, "%s..\\..\\..\\docs\\GameContentCreationGuide.pdf", GetApp()->m_ExeDirectory);
	int shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath,NULL,NULL,SW_SHOWNORMAL);

	// Check installed path in registry if we don't find the file in the normal location
	if ((shellVal <= 32) && ((shellVal == ERROR_FILE_NOT_FOUND) || (shellVal == ERROR_PATH_NOT_FOUND)))
	{
		char szPath2[MAX_PATH];
		HKEY hKey;
		unsigned long dataSize = sizeof(char) * MAX_PATH, keyType = REG_SZ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\LithTech Inc.\\Jupiter\\", NULL, KEY_ALL_ACCESS, &hKey) == NO_ERROR)
		{
			if (RegQueryValueEx(hKey, "InstallDir", NULL, &keyType, (unsigned char *)szPath, &dataSize) == NO_ERROR)
			{
				if (szPath[0] == '\"') // Remove quotes from path
				{
					int stringLen = strlen(szPath)-2;				ASSERT(stringLen > 0);
					strncpy(szPath2, &szPath[1], stringLen);
					szPath2[stringLen] = '\0';
					strcpy(szPath, szPath2);
				}
				sprintf(szPath2, "%s\\docs\\GameContentCreationGuide.pdf", szPath);
				shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath2,NULL,NULL,SW_SHOWNORMAL);
			}
			else
			{
				shellVal = 33; // in order to skip switch below
				AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
			}

			RegCloseKey(hKey);
		}
		else
		{
			shellVal = 33; // in order to skip switch below
			AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
		}
	}

	if (shellVal <= 32)
	{
		switch (shellVal)
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				AppMessageBox(AFX_IDP_NO_CREATION_GUIDE, MB_OK);
				break;

			case SE_ERR_ASSOCINCOMPLETE:
			case SE_ERR_NOASSOC:
				AppMessageBox(AFX_IDP_NO_PDF_READER, MB_OK);
				break;

			default:
				CString str;
				str.FormatMessage(AFX_IDP_GENERIC_CREATION_GUIDE_FALURE, shellVal);
				AppMessageBox(str, MB_OK);
		}
	}

	return;
}

void CDEditApp::OnDEHomePage()
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open","http://www.lithtech.com",NULL,NULL,SW_SHOW);

	return;
}

void CDEditApp::OnOnlineDoc()
{
	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\lithtech.hlp", GetApp()->m_ExeDirectory);

	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath,NULL,NULL,SW_SHOW);

	return;
}

void CDEditApp::OnFileReloadWorld()
{
	//get the currently active document
	CRegionDoc* pDoc = GetActiveRegionDoc();

	if(!pDoc)
		return;

	//alright, now see if it has changed
	if(pDoc->IsModified())
	{
		if(MessageBox(AfxGetMainWnd()->GetSafeHwnd(), "Reloading this world will cause changes to be lost. Are you sure you want to do this?", "Reload World", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
		{
			//they canceled
			return;
		}
	}

	//pull up a wait cursor instantly so the user knows what is going on
	CWaitCursor	WaitCursor;

	//alright, close out this document and reopen it
	CString sWorldName = pDoc->m_FileName;

	//if they went this far, they wanted to discard changes anyway
	pDoc->SetModifiedFlag(FALSE);

	//send a message to ourself to close the document
	SendMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_COMMAND, MAKEWPARAM(ID_FILE_CLOSE, 0), 0);

	//now we need to reopen it
	GetProjectBar()->OpenRegionDoc(sWorldName);
}

void CDEditApp::OnUpdateFileReloadWorld(CCmdUI* pCmdUI)
{
	//see if they have a currently active document
	pCmdUI->Enable(GetActiveRegionDoc() != NULL);
}

void CDEditApp::OnFileNewWorld()
{
	CStringDlg			dlg;
	FILE				*fp;
	CMoFileIO			file;
	CString				fileName, extension, fullName, str;
	BOOL				bCreate;
	int					status;
	DDirIdent *pIdent;
	CString msg;


	if( GetProjectBar( )->VerifyProjectIsOpen() )
	{
		//make sure we have already selected a world dir
		if(strcmp(GetProjectBar( )->m_Project.m_BaseProjectDir,GetWorldsDlg()->GetCurDirPath( )) == 0)
		{
			AfxMessageBox(IDS_BADWORLDDIR,MB_OK | MB_ICONINFORMATION);
			return;
		}

		dlg.m_bAllowFile = TRUE;
		dlg.m_MaxStringLen = 70;
		dlg.m_bBeeping = TRUE;

		if( dlg.DoModal(IDS_NEWWORLD, IDS_ENTERWORLDNAME) == IDOK )
		{
			if(GetApp()->GetOptions().GetMiscOptions()->IsDefaultCompressed())
			{
				extension.LoadString( IDS_COMPRESSEDWORLDEXTENSION );
			}
			else
			{
				extension.LoadString( IDS_WORLDEXTENSION );
			}
			
			// Make sure they have a worlds directory selected.
			if(GetWorldsDlg()->IsDirectorySelected())
			{
				pIdent = GetWorldsDlg()->GetSelectedDirectory();
				if(!pIdent)
					return;
			}
			else
			{
				pIdent = GetWorldsDlg()->GetFirstDirectory();
				if(pIdent)
				{
					msg.FormatMessage(IDS_NOWORLDDIRSELECTED, pIdent->m_Filename);
					if(AppMessageBox(msg, MB_OKCANCEL) == IDOK)
					{
					}
					else
					{
						return;
					}
				}
				else
				{
					AppMessageBox(IDS_MUSTHAVEONEWORLDSDIRECTORY, MB_OK);
					return;
				}
			}

			fileName = dfm_BuildName(pIdent->m_Filename, dlg.m_EnteredText + extension);
			fullName = dfm_GetFullFilename(GetFileMgr(), fileName);

			// If the file already exist, ask if they want to overwrite...
			if( dfm_OpenFileRelative( GetFileMgr(), fileName, file ))
			{
				file.Close( );

				str.FormatMessage( IDS_WORLD_ALREADY_EXISTS, dlg.m_EnteredText );
				status = AfxMessageBox( str, MB_YESNOCANCEL | MB_ICONQUESTION );
			
				if( status == IDNO )
				{
					bCreate = FALSE;
				}
				else if( status == IDCANCEL )
				{
					return;
				}
			}

			/*
				( nodehierarchy 
					( worldnode 
						( type null )
						( label "Container" ) 
						( nodeid 1 ) 
						( flags ( worldroot expanded ) ) 
						( properties
							( propid  0 ) 
						)
			*/

			if( bCreate )
			{
				CLTAWriter OutFile;

				if( OutFile.Open(fullName, CLTAUtil::IsFileCompressed(fullName)) )
 				{
 					OutFile.WriteF(false, "\n( world								");
 					OutFile.WriteF(false, "\n		( header						");
 					OutFile.WriteF(false, "\n			( versioncode 1249 )		");
 					OutFile.WriteF(false, "\n		)								");
					OutFile.WriteF(false, "\n		( nodehierarchy					");
					OutFile.WriteF(false, "\n			( worldnode					");
 					OutFile.WriteF(false, "\n				( type null )			");	
 					OutFile.WriteF(false, "\n				( label \"%s\" )		", dlg.m_EnteredText);
 					OutFile.WriteF(false, "\n				( nodeid 1 )			");
 					OutFile.WriteF(false, "\n				( flags (worldroot expanded) ) ");
 					OutFile.WriteF(false, "\n				( properties			");
 					OutFile.WriteF(false, "\n					( propid  0 )		");
 					OutFile.WriteF(false, "\n				)						");
 					OutFile.WriteF(false, "\n				( childlist ( ) )		");
 					OutFile.WriteF(false, "\n			)							");
					OutFile.WriteF(false, "\n		)								");
					OutFile.WriteF(false, "\n		( globalproplist (				");
					OutFile.WriteF(false, "\n			( proplist (				");
					OutFile.WriteF(false, "\n		) )								");
 					OutFile.WriteF(false, "\n)										");
 
 					OutFile.Close();
 				}
 				else
 				{
 					str.FormatMessage( IDS_ERRORCREATING_WORLD, dlg.m_EnteredText );
 					AppMessageBox( str, MB_OK );
 					return;
 				}
			}

			GetProjectBar( )->OpenRegionDoc(fullName);
			GetWorldsDlg()->UpdateDirectories( );
			GetWorldsDlg()->PopulateList( );
		}
	}
}

void CDEditApp::OnUpdateFileNewWorld( CCmdUI *pCmdUI )
{
	if( GetProjectBar( )->IsProjectOpen() )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CDEditApp::OnFileSave()
{
	GetProjectBar()->Save();

	return;
}

void CDEditApp::OnUpdateFileSave( CCmdUI *pCmdUI )
{
	if( GetProjectBar( )->IsProjectOpen() )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

}

void CDEditApp::OnFileCloseProject()
{
	GetProjectBar()->Close();

	return;
}

void CDEditApp::OnUpdateFileCloseProject( CCmdUI *pCmdUI )
{
	if( GetProjectBar( )->IsProjectOpen() )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

}

void CDEditApp::OnFileCloseWorld()
{
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	CView *pView = (CView *) pChild->GetActiveView();
	CRegionDoc *pDoc = (CRegionDoc*)pView->GetDocument();

	if(pDoc->SaveModifiedLTA()) // User didn't click cancel on the save request box
	{
		m_pWorldTemplate->RemoveDocument( pDoc );
		pDoc->OnCloseDocument();
	}

	return;
}

void CDEditApp::OnUpdateFileCloseWorld( CCmdUI *pCmdUI )
{
	if( ((CMainFrame*)m_pMainWnd)->GetNumRegionDocs() > 0 )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CDEditApp::OnFileSaveWorldAs()
{
	CString			ext, filter, sFileName;

	ext.LoadString( IDS_WORLDEXTENSION );
	filter.LoadString( IDS_FILESAVEAS_FILTER );
	sFileName = GetProject()->m_BaseProjectDir + "\\Worlds\\*.lta;*.ltc";

	CFileDialog		dlg(FALSE, ext, (LPCTSTR)sFileName, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, filter, AfxGetMainWnd( ));

	if( dlg.DoModal() == IDOK )
	{
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		CView *pView = (CView *) pChild->GetActiveView();
		CRegionDoc *pDoc = (CRegionDoc*)pView->GetDocument();

		char	pathName[_MAX_PATH], fileName[_MAX_PATH];
		char	worldName[_MAX_PATH], extension[_MAX_PATH], backupName[_MAX_PATH];

		CHelpers::ExtractPathAndFileName( dlg.GetPathName(), pathName, fileName );
		CHelpers::ExtractFileNameAndExtension( fileName, worldName, extension );

		pDoc->m_FileName = dlg.GetPathName();	
		pDoc->m_WorldName = worldName;
		pDoc->SetTitle(false);
		pDoc->SetPathName(dlg.GetPathName(),FALSE);

		//update the name of the backup file
		pDoc->UpdateBackupName(dlg.GetPathName());

		if( stricmp(extension,"obj") == 0 ) 
		{
			pDoc->SaveOBJ(TRUE);
		}
		else if( (stricmp(extension,"lta") == 0) || (stricmp(extension,"ltc") == 0) || (stricmp(extension,"tbw") == 0)) 
		{
			pDoc->SaveLTA(TRUE);
		}
		GetProjectBar( )->UpdateAll();
	}

	return;
}

void CDEditApp::OnUpdateFileSaveWorldAs( CCmdUI *pCmdUI )
{
	if( ((CMainFrame*)m_pMainWnd)->GetNumRegionDocs() > 0 )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


void CDEditApp::OnEditKeyConfiguration()
{
	CKeyConfigDlg KeyConfigDlg(CGlobalHotKeyDB::m_DB);

	if(KeyConfigDlg.DoModal() == IDOK)
	{
		SetHotKeyDB(KeyConfigDlg.m_Config);
	}
}


//sets up the hot key database to reflect a specified database
void CDEditApp::SetHotKeyDB(const CHotKeyDB& NewDB)
{
	CGlobalHotKeyDB::m_DB = NewDB;

	//one thing we need to do is make sure that the draw poly tracker is handled correctly.
	//this means clearing out its end event list and making sure that it is escape instead
	const CHotKey* pDrawPoly = CGlobalHotKeyDB::m_DB.GetHotKey(UIE_DRAW_POLY);
	if(pDrawPoly)
	{
		CHotKey NewDrawPoly(*pDrawPoly);
		NewDrawPoly.ClearEndEventList();
		NewDrawPoly.m_EndEventList.Add(new CUIKeyEvent(UIEVENT_KEYDOWN, VK_ESCAPE));
		CGlobalHotKeyDB::m_DB.SetHotKey(NewDrawPoly);
	}

	CGlobalHotKeyDB::m_DB.SaveToRegistry(m_genRegMgr, HOTKEY_REG_PATH);

	//have all the views reset their trackers now that they have a new configuration to use
	POSITION position;
	for( position = m_pWorldTemplate->GetFirstDocPosition(); position != NULL; )
	{
		CDocument* pDoc = m_pWorldTemplate->GetNextDoc( position );
		if( pDoc != NULL )
		{
			POSITION viewPos;
			for( viewPos = pDoc->GetFirstViewPosition(); viewPos != NULL; )
			{
				CRegionView* pView = (CRegionView*)pDoc->GetNextView( viewPos );
				if( pView != NULL )
				{
					pView->OnHotKeysChanged();
				}
			}
		}
	}
}

//attempts to get a string from the bute help files. It will first try the aggregate
//if one exists, if that fails it will try the base, and if that fails the function will
//fail. If it succeeds, the string will be returned in sMatch
BOOL CDEditApp::GetHelpString(const char* pszTag, const char* pszAttrib, CString& sMatch)
{
	//first try out the aggregate
	if(GetClassHelpButeAgg() && GetClassHelpButeAgg()->Exist(pszTag, pszAttrib))
	{
		//found it in the aggregate
		sMatch = GetClassHelpButeAgg()->GetString(pszTag, pszAttrib);
		return TRUE;
	}

	//now try out the base
	if(GetClassHelpButeMgr() && GetClassHelpButeMgr()->Exist(pszTag, pszAttrib))
	{
		//found it in the aggregate
		sMatch = GetClassHelpButeMgr()->GetString(pszTag, pszAttrib);
		return TRUE;
	}

	//failed to find it
	return FALSE;
}

//this sets the class help bute aggregate. This will be in charge of freeing memory
//associated with it. Assumes it is allocated with new
void CDEditApp::SetClassHelpButeAgg(CButeMgr* pMgr)
{
	//clean up any old aggreagates
	if(GetClassHelpButeAgg())
	{
		GetClassHelpButeAgg()->Term();
		delete GetClassHelpButeAgg();
	}

	//set the new one
	m_pClassHelpButeAgg = pMgr;
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
