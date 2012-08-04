
// SpellEd.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "SpellEd.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "SpellEdDoc.h"
#include "SpellEdView.h"
#include "Splash.h"
#include "ButeMgr.h"
#include "SpellCtrl.h"
#include "GameInfoDlg.h"
#include "stringdlg.h"
#include "resourcelocator.h"
#include "tdguard.h"
#include <direct.h>

// Globals.....

CString						g_sRezFileName;

struct LINK_STATUS
{
					LINK_STATUS()
					{
						memset(this, 0, sizeof(LINK_STATUS));
					}
	
	BOOL  bLinked;
	DWORD dwLinkedID;
	char  sNode[32];
};


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellEdApp

BEGIN_MESSAGE_MAP(CSpellEdApp, CWinApp)
	//{{AFX_MSG_MAP(CSpellEdApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVESPELLS, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVESPELLS, OnFileSave)
	ON_COMMAND(ID_FILE_SAVESPELLS_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_LAUNCH, OnFileLaunch)
	ON_COMMAND(ID_FILE_IMPORTCOLOURFAVOURITES, OnFileImportColourFavourites)
	ON_COMMAND(ID_FILE_IMPORTKEYFAVOURITES, OnFileImportKeyFavourites)
	ON_COMMAND(ID_FILE_IMPORTSCALEFAVOURITES, OnFileImportScaleFavourties)
	ON_COMMAND(ID_FILE_IMPORTMOVEFAVOURITES, OnFileImportMoveFavourites)
	ON_COMMAND(ID_FILE_RELOADRESOURCEFILE, OnFileReloadresourcefile)
	ON_COMMAND(ID_FILE_EDITGAMEINFO, OnFileEditgameinfo)
	ON_COMMAND(ID_FILE_FINDFXCONTAININGRESOURCE, OnFileFindResource)
	ON_COMMAND(ID_FORMAT_TEXT, OnFormatText)
	ON_COMMAND(ID_FORMAT_BINARY, OnFormatBinary)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellEdApp construction

CSpellEdApp::CSpellEdApp()
{
	m_bHaveFilename    = FALSE;
	m_bFirstActivation = TRUE;

	m_nFastCastSpeed   = 1000;
	m_nMediumCastSpeed = 2000;
	m_nSlowCastSpeed   = 4000;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSpellEdApp object

CSpellEdApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSpellEdApp initialization

BOOL CSpellEdApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}


	{
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);
		CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
	}
	AfxEnableControlContainer();

	// Create the image list

	m_collImages.Create(16, 16, ILC_MASK, 5, 5);
	m_collImages.SetBkColor(CLR_NONE);

	// Add the images

	HICON hIcon;

	hIcon = LoadIcon(IDI_CLOSED);
	m_collImages.Add(hIcon);

	hIcon = LoadIcon(IDI_OPEN);
	m_collImages.Add(hIcon);

	hIcon = LoadIcon(IDI_SPELL);
	m_collImages.Add(hIcon);

	hIcon = LoadIcon(IDI_RESOURCE);
	m_collImages.Add(hIcon);

	// Load the cursors

	m_hArrow = ::LoadCursor(NULL, IDC_ARROW);
	m_hLeftCursor = LoadCursor(IDC_LEFT);
	m_hRightCursor = LoadCursor(IDC_LEFT);
	m_hLeftRightCursor = LoadCursor(IDC_LEFTRIGHT);

	// Standard initialization


	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("LithTech Inc.\\Jupiter"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Initialise fx manager

	// Register document templates

	m_pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSpellEdDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CSpellEdView));
	AddDocTemplate(m_pDocTemplate);

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
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	// Try and read the rez string

	CString sDllFile = GetProfileString("", "DllLocation", "");
	CString sRezFile = GetProfileString("", "ResourceFileLocation", "");
	// Initialise rez manager

	// [dlk] Make sure the values are correct before proceeding.
	// Fixes a one time startup crash
	bool bCorrect = true;

	if(sDllFile.IsEmpty() || sRezFile.IsEmpty())
	{
		bCorrect = false;
	}
	else
	{
		if(m_fxMgr.Init(sDllFile) && m_rezMgr.Open(sRezFile))
		{
			bCorrect = true;
		}
		else
		{
			bCorrect = false;
		}
	}

	//Keep asking for the info until it's correct...
	while(!bCorrect)
	{
		bCorrect = true;
		OnFileEditgameinfo();

		sDllFile = GetProfileString("", "DllLocation", "");
		sRezFile = GetProfileString("", "ResourceFileLocation", "");		

		if (sDllFile.IsEmpty() || sRezFile.IsEmpty())
		{
			bCorrect = false;
			continue;
		}

		// Init FX Mgr
		if(!m_fxMgr.Init(sDllFile))
		{
			AfxMessageBox( "FXD location invalid!");
			bCorrect = false;
			continue;
		}

		// Init Resource File
		if(!m_rezMgr.Open(sRezFile))
		{
			AfxMessageBox( "Resource file location invalid!");
			bCorrect = false;
			continue;
		}
	}	

	// Initialise 'bute manager

	m_bm.Init();
	if (m_bm.Parse("ServerButes.txt"))
	{
		m_nFastCastSpeed = m_bm.GetInt("CastSpeeds", "Fast", 1000);
		m_nMediumCastSpeed = m_bm.GetInt("CastSpeeds", "Medium", 2000);
		m_nSlowCastSpeed = m_bm.GetInt("CastSpeeds", "Slow", 4000);
	}

	m_wFormat = FORMAT_TEXT;

	//setup the favorites directory to point to the current working directory
	char pszPath[MAX_PATH + 1];
	GetModuleFileName(AfxGetInstanceHandle(), pszPath, MAX_PATH);

	m_sFavDir = pszPath;

	//now find the ending slash and remove it
	int nPos = m_sFavDir.ReverseFind('\\');
	if(nPos != -1)
	{
		m_sFavDir = m_sFavDir.Left(nPos);
	}

	//make sure it ends with a slash
	m_sFavDir.TrimRight("\\/");
	m_sFavDir += '\\';

	// [kml] 3/29/02
	// Load the favorites here rather than when we open a file 
	// because we only want to do this once.
	LoadFavourites();

	//now load up the last file if applicable
	CString sLastOpenedFile = GetProfileString("", "LastOpenedFile", "");
	if(sLastOpenedFile.GetLength() > 0)
	{
		//clear out the string, just incase the file has a problem opening. If
		//that is the case not clearing this would force the user to hand flush
		//their registry
		WriteProfileString("", "LastOpenedFile", "");

		OpenFile(sLastOpenedFile);
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
void CSpellEdApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CSpellEdApp commands

//------------------------------------------------------------------
//
//   FUNCTION : OnFileOpen()
//
//   PURPOSE  : Opens a .dic file
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileOpen() 
{
	char sDir[256];

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.fxf", "*.fxf", OFN_OVERWRITEPROMPT, "Fx Files (*.fxf)|*.fxf||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		OpenFile(dlg.GetPathName());
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : OpenFile()
//
//   PURPOSE  : given a name it will attempt to open up the file in the specified mode, printing
//				the appropriate messages if it fails.
//
//------------------------------------------------------------------
FILE* CSpellEdApp::OpenFile(const char* pszFilename, bool bWrite, bool bText, bool bDisplayErrors)
{
	//determine what type of mode we should use to open the file
	char pszOpenMode[3];
	pszOpenMode[0] = (bWrite) ? 'w' : 'r';
	pszOpenMode[1] = (bText) ? 't' : 'b';
	pszOpenMode[2] = '\0';

	//first off, just try and open it
	FILE* pRV = fopen(pszFilename, pszOpenMode);

	//see if it worked
	if(pRV)
	{
		//success, just pass it back to the user
		return pRV;
	}

	//ok, now try and figure out what the problem is, read in the file status
	CFileStatus FileStatus;
	BOOL bFileExisted = CFile::GetStatus(pszFilename, FileStatus);

	//string to hold the message in
	CString sMsg;
	CString sCaption = "Error Opening File";

	//break the errors into two groups: read and write errors
	if(bWrite)
	{
		//write errors

		//we are trying to write, so most likely the problem is a read only file
		if(bFileExisted && (FileStatus.m_attribute & CFile::readOnly))
		{
			sMsg.Format("The file %s could not be saved because it is read only. Please make it not read only and try saving again.", pszFilename);
		}
		else
		{
			//generic error
			sMsg.Format("Unable to open the file %s for writing.", pszFilename);
		}
	}
	else
	{
		//read errors

		//most likely the file doesn't exist
		if(!bFileExisted)
		{
			sMsg.Format("Unable to open the file %s for reading. This file doesn't seem to exist. Please check that the filename is correct.", pszFilename);
		}
		else
		{
			//generic
			sMsg.Format("Unable to open the file %s for reading.", pszFilename);
		}
	}

	//display the error
	if(bDisplayErrors)
		MessageBox(NULL, sMsg, sCaption, MB_ICONEXCLAMATION | MB_OK);

	//failure case
	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : OpenFile()
//
//   PURPOSE  : Opens the specified file and loads in any other
//				associated data
//
//------------------------------------------------------------------
BOOL CSpellEdApp::OpenFile(const char* pszFilename)
{
	m_bHaveFilename = TRUE;
	m_sDicFile = pszFilename;

	// Load favourites file

	// [kml] 3/29/02
	// Moved this into InitInstance because it was causing the favorites
	// to double each time we opened a file... That and we really only
	// want to do this once unless the user explicity imports a favs file.
	//LoadFavourites();

	// Load spells....

	if(!LoadSpells(pszFilename))
		return FALSE;

	// Load cfg file....

	CString sDcfFile = pszFilename;
	char *sExt = sDcfFile.GetBuffer(sDcfFile.GetLength());
	sExt[strlen(sExt) - 3] = 'f';
	sExt[strlen(sExt) - 2] = 'c';
	sExt[strlen(sExt) - 1] = 'f';
	sDcfFile.ReleaseBuffer();

	if(!LoadCfgFile(sExt))
		return FALSE;

	WriteProfileString("", "LastOpenedFile", pszFilename);

	//save this so we can do better saving support
	m_sSrcFile = pszFilename;

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnUpdateFileSave()
//
//   PURPOSE  : CCmdUI handler
//
//------------------------------------------------------------------

void CSpellEdApp::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_sDicFile.GetLength());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileSave()
//
//   PURPOSE  : Saves a .dic file
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileSave() 
{	
	if (m_sDicFile.GetLength() == 0) return;
	
	// Save spells....

	SaveSpells(m_sDicFile);

	// Load cfg file....

	CString sDcfFile = m_sDicFile;
	char *sExt = sDcfFile.GetBuffer(sDcfFile.GetLength());
	sExt[strlen(sExt) - 3] = 'f';
	sExt[strlen(sExt) - 2] = 'c';
	sExt[strlen(sExt) - 1] = 'f';
	sDcfFile.ReleaseBuffer();

	SaveCfgFile(sExt);
}

void WriteProp(FX_PROP *pProp, FILE *fp)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	switch( pApp->GetFormat() )
	{	
	case FORMAT_TEXT:
		{
			// Write the name out
	
			fprintf( fp, "\t\t\tPropName: %s\n", pProp->m_sName );
							
			// Write the type

			fprintf( fp, "\t\t\t\tType: %i\n", pProp->m_nType );
			
			// Write the data

			switch (pProp->m_nType)
			{
				case FX_PROP::STRING  :	fprintf( fp, "\t\t\t\tSTRING: %s\n", pProp->m_data.m_sVal );	break;		
				case FX_PROP::INTEGER :	fprintf( fp, "\t\t\t\tINTEGER: %i\n", pProp->m_data.m_nVal );	break;	
				case FX_PROP::FLOAT   :	fprintf( fp, "\t\t\t\tFLOAT: %f\n", pProp->m_data.m_fVal );		break;		
				case FX_PROP::COMBO   :	fprintf( fp, "\t\t\t\tCOMBO: %s\n", pProp->m_data.m_sVal );		break;		
				case FX_PROP::VECTOR  :	fprintf( fp, "\t\t\t\tVECTOR: %f %f %f\n", pProp->m_data.m_fVec[0], pProp->m_data.m_fVec[1], pProp->m_data.m_fVec[2] ); break; 
				case FX_PROP::VECTOR4 :	fprintf( fp, "\t\t\t\tVECTOR4: %f %f %f %f\n", pProp->m_data.m_fVec4[0], pProp->m_data.m_fVec4[1], pProp->m_data.m_fVec4[2], pProp->m_data.m_fVec4[3]); break; 
				case FX_PROP::CLRKEY  :	
					{
						float r, g, b, a;
						float ratio = 1.0f / 255.0f;
						DWORD dwCol = pProp->m_data.m_clrKey.m_dwCol;

						r	= ratio * (float)(dwCol & 0x000000FF);
						g	= ratio * (float)((dwCol & 0x0000FF00) >> 8);
						b	= ratio * (float)((dwCol & 0x00FF0000) >> 16);
						a	= ratio * (float)((dwCol & 0xFF000000) >> 24);
						
						fprintf( fp, "\t\t\t\tCLRKEY: %f %f %f %f %f\n", pProp->m_data.m_clrKey.m_tmKey, r, g, b, a );
					}
					break;

				case FX_PROP::PATH	  :	fprintf( fp, "\t\t\t\tPATH: %s\n", pProp->m_data.m_sVal ); break;
			}
		}
		break;

	case FORMAT_BIN:
		{
			// Write the length of the name
	
			BYTE nameLen = strlen(pProp->m_sName);
			fwrite(&nameLen, 1, 1, fp);

			// Write the name out
			
			fwrite(pProp->m_sName, nameLen, 1, fp);
				
			// Write the type

			fwrite(&pProp->m_nType, sizeof(FX_PROP::eDataType), 1, fp);

			// Write the data

			switch (pProp->m_nType)
			{
				
				case FX_PROP::STRING  :	fwrite(pProp->m_data.m_sVal, 128, 1, fp); break;
				case FX_PROP::INTEGER :	fwrite(&pProp->m_data.m_nVal, sizeof(int), 1, fp); break;
				case FX_PROP::FLOAT   :	fwrite(&pProp->m_data.m_fVal, sizeof(float), 1, fp); break;
				case FX_PROP::COMBO   :	fwrite(pProp->m_data.m_sVal, 128, 1, fp); break;
				case FX_PROP::VECTOR  :	fwrite(pProp->m_data.m_fVec, sizeof(float), 3, fp); break;
				case FX_PROP::VECTOR4 :	fwrite(pProp->m_data.m_fVec4, sizeof(float), 4, fp); break;
				case FX_PROP::CLRKEY  :	fwrite(&pProp->m_data.m_clrKey, sizeof(FX_PROP::FX_CLRKEY), 1, fp); break;
				case FX_PROP::PATH	  :	fwrite(pProp->m_data.m_sVal, 128, 1, fp); break;
			}
		}
		break;

	default:
		{
			AfxMessageBox( "Unknown format!", MB_OK | MB_ICONSTOP );
		}
		break;
	}
	
}

void ReadProp(FX_PROP *pProp, FILE *fp, int iFormat )
{
	switch( iFormat )
	{
	case FORMAT_TEXT:
		{
			char	szTag[16] = {0};

			// Read in the name...

			fscanf( fp, "%s %s", szTag, pProp->m_sName );

			// Read the type...

			int iType;
			fscanf( fp, "%s %i", szTag, &iType );
			pProp->m_nType = (FX_PROP::eDataType)iType;

			// Read the data...

			switch( iType )
			{
				case FX_PROP::STRING	: 
					{
						fscanf( fp, "%s", szTag );
						fseek( fp, 1, SEEK_CUR );
						fgets(pProp->m_data.m_sVal, ARRAY_LEN(pProp->m_data.m_sVal), fp );
						int iStrSize = strlen( pProp->m_data.m_sVal );
						if( (iStrSize > 0) && (pProp->m_data.m_sVal[iStrSize - 1] == '\n') )
							pProp->m_data.m_sVal[iStrSize - 1] = NULL;
					}
					break;

				case FX_PROP::INTEGER	: fscanf( fp, "%s %i", szTag, &pProp->m_data.m_nVal );		break;
				case FX_PROP::FLOAT		: fscanf( fp, "%s %f", szTag, &pProp->m_data.m_fVal );		break;
				case FX_PROP::COMBO		: 
					{
						fscanf( fp, "%s", szTag );
						fseek( fp, 1, SEEK_CUR );
						fgets(pProp->m_data.m_sVal, ARRAY_LEN(pProp->m_data.m_sVal), fp );
						int iStrSize = strlen( pProp->m_data.m_sVal );
						if( (iStrSize > 0) && (pProp->m_data.m_sVal[iStrSize - 1] == '\n') )
							pProp->m_data.m_sVal[iStrSize - 1] = NULL;	
					}
					break;

				case FX_PROP::VECTOR	: fscanf( fp, "%s %f %f %f", szTag, &pProp->m_data.m_fVec[0], &pProp->m_data.m_fVec[1], &pProp->m_data.m_fVec[2] ); break;
				case FX_PROP::VECTOR4	: fscanf( fp, "%s %f %f %f %f", szTag, &pProp->m_data.m_fVec4[0], &pProp->m_data.m_fVec4[1], &pProp->m_data.m_fVec4[2], &pProp->m_data.m_fVec4[3] ); break;
				case FX_PROP::CLRKEY	: 
					{
						float r, g, b, a;
						fscanf( fp, "%s %f %f %f %f %f", szTag, &pProp->m_data.m_clrKey.m_tmKey, &r, &g, &b, &a );

						DWORD dwRed   = (int)(r * 255.0f);
						DWORD dwGreen = (int)(g * 255.0f);
						DWORD dwBlue  = (int)(b * 255.0f);
						DWORD dwAlpha = (int)(a * 255.0f);
						
						pProp->m_data.m_clrKey.m_dwCol = dwRed | (dwGreen << 8) | (dwBlue << 16) | (dwAlpha << 24);
					}
					break;

				case FX_PROP::PATH		: 
					{
						fscanf( fp, "%s", szTag );
						fseek( fp, 1, SEEK_CUR );
						fgets(pProp->m_data.m_sVal, ARRAY_LEN(pProp->m_data.m_sVal), fp );
						int iStrSize = strlen( pProp->m_data.m_sVal );
						if( (iStrSize > 0) && (pProp->m_data.m_sVal[iStrSize - 1] == '\n') )
							pProp->m_data.m_sVal[iStrSize - 1] = NULL;		
					}
					break;
			}
		}
		break;

	case FORMAT_BIN:
		{
			// Read the length of the name

			BYTE nameLen;
			fread(&nameLen, 1, 1, fp);

			// Read in the name

			fread(pProp->m_sName, nameLen, 1, fp);
			pProp->m_sName[nameLen] = 0;

			// Read the type

			FX_PROP::eDataType type;
			fread(&type, sizeof(FX_PROP::eDataType), 1, fp);

			pProp->m_nType = type;

			// Read the data

			switch (type)
			{
				case FX_PROP::STRING  : fread(pProp->m_data.m_sVal, 128, 1, fp); break;
				case FX_PROP::INTEGER : fread(&pProp->m_data.m_nVal, sizeof(int), 1, fp); break;
				case FX_PROP::FLOAT   : fread(&pProp->m_data.m_fVal, sizeof(float), 1, fp); break;
				case FX_PROP::COMBO   : fread(pProp->m_data.m_sVal, 128, 1, fp); break;
				case FX_PROP::VECTOR  : fread(pProp->m_data.m_fVec, sizeof(float), 3, fp); break;
				case FX_PROP::VECTOR4 : fread(pProp->m_data.m_fVec4, sizeof(float), 4, fp); break;
				case FX_PROP::CLRKEY  : fread(&pProp->m_data.m_clrKey, sizeof(FX_PROP::FX_CLRKEY), 1, fp); break;
				case FX_PROP::PATH	  : fread(pProp->m_data.m_sVal, 128, 1, fp); break;
			}
		}
		break;

	default:
		{
			AfxMessageBox( "Unknown format!", MB_OK | MB_ICONSTOP );
		}
		break;
	}

	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileSaveAs()
//
//   PURPOSE  : Saves a .dic file to another filename
//
//------------------------------------------------------------------

BOOL CSpellEdApp::SaveSpellsAs()
{
	char sDir[256];

	//build up the default filename
	CString sDefaultFilename = m_sSrcFile;

	//make sure it is valid (that the file exists)
	CFileStatus DummyStatus;
	if(sDefaultFilename.IsEmpty() || !CFile::GetStatus(sDefaultFilename, DummyStatus))
		sDefaultFilename = "*.fxf";

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(FALSE, "*.fxf", sDefaultFilename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Fx Files (*.fxf)|*.fxf||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		CString sName = dlg.GetPathName();

		m_bHaveFilename = TRUE;
		m_sDicFile		= sName;

		// Load spells....

		if(!SaveSpells(sName))
		{
			//unable to save the spells!
			return FALSE;
		}

		// Load cfg file....

		CString sDcfFile = sName;
		char *sExt = sDcfFile.GetBuffer(sDcfFile.GetLength());
		sExt[strlen(sExt) - 3] = 'f';
		sExt[strlen(sExt) - 2] = 'c';
		sExt[strlen(sExt) - 1] = 'f';
		sDcfFile.ReleaseBuffer();

		if(!SaveCfgFile(sExt))
		{
			//unable to save the configuration!
			return FALSE;
		}
	}

	return TRUE;
}

void CSpellEdApp::OnFileSaveAs() 
{
	SaveSpellsAs();
}

//------------------------------------------------------------------
//
//   FUNCTION : LoadSpellmgr()
//
//   PURPOSE  : Loads up a spell manager with spells
//
//------------------------------------------------------------------

BOOL CSpellEdApp::LoadSpellmgr(const char *sFilename, CSpellMgr *pSpellMgr)
{
	FILE *fp = OpenFile(sFilename, false, false);
	if( !fp ) return FALSE;

	char szBinTest[16] = {0};
	
	fread( szBinTest, 8, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "Groups: " ) )
	{
		return LoadSpellmgr_t( sFilename, pSpellMgr );
	}
	else
	{
		return LoadSpellmgr_b( sFilename, pSpellMgr );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::LoadSpellmgr_b
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

BOOL CSpellEdApp::LoadSpellmgr_b(const char *sFilename, CSpellMgr *pSpellMgr )
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	
	FILE *fp = OpenFile(sFilename, false, false);
	if (!fp) return FALSE;

	// Read in the number of FX groups

	DWORD dwNumFxGroups;
	fread(&dwNumFxGroups, sizeof(DWORD), 1, fp);

	for (DWORD i = 0; i < dwNumFxGroups; i ++)
	{
		CSpell *pNewSpell = pSpellMgr->AddSpell();
		CPhase *pSpellPhase = pNewSpell->GetPhase(1);

		// Read in the number of FX in this group

		DWORD dwNumFx;
		fread(&dwNumFx, sizeof(DWORD), 1, fp);
		
		// Read in the name of this FX group

		char sTmp[128];
		fread(sTmp, 128, 1, fp);

		// Read in the phase length

		DWORD dwPhaseLen;
		fread(&dwPhaseLen, sizeof(DWORD), 1, fp);
		pSpellPhase->SetPhaseLength(dwPhaseLen);

		// Check to make sure we don't already have this spell
		
		char sTmp2[128];
		strcpy(sTmp2, sTmp);
		
		while (1)
		{
			BOOL bOkay = TRUE;
	
			if (m_spellMgr.FindSpellByName(sTmp2))
			{
				bOkay = FALSE;
				strcat(sTmp2, "_1");
			}
			
			if (bOkay) break;
		}

		strcpy(sTmp, sTmp2);

		pNewSpell->SetName(sTmp);
		pNewSpell->SetGuid(sTmp);

		// Read in the FX

		for (DWORD j = 0; j < dwNumFx; j ++)
		{
			CKey *pKey = new CKey;

			// Read in the reference name

			fread(sTmp, 128, 1, fp);
			pKey->m_pFxRef = pApp->GetFxMgr()->FindFX( strtok(sTmp, ";") );
			pKey->SetCustomName( strtok( NULL, ";" ));

			// Read in the ID of the key

			DWORD dwID;
			fread(&dwID, sizeof(DWORD), 1, fp);
			pKey->SetID(dwID);

			// Read in the link status of the key

			LINK_STATUS ls;
			fread(&ls, sizeof(LINK_STATUS), 1, fp);

			if (ls.bLinked)
			{
				pKey->SetLink(TRUE, ls.dwLinkedID, ls.sNode);
			}
			else
			{
				pKey->SetLink(FALSE, 0, "");
			}

			// Read in the start time

			float tmStart;
			fread(&tmStart, sizeof(float), 1, fp);
			pKey->m_tmStart = (int)(tmStart * 1000.0f);

			// Read in the end time

			float tmEnd;
			fread(&tmEnd, sizeof(float), 1, fp);
			pKey->m_tmEnd = (int)(tmEnd * 1000.0f);

			// Read in the key repeat

			fread(&pKey->m_dwKeyRepeat, sizeof(DWORD), 1, fp);

			// Read in the track ID

			fread(&pKey->m_nTrack, sizeof(DWORD), 1, fp);

			// Read in the min scale

			fread(&pKey->m_minScale, sizeof(float), 1, fp);

			// Read in the max scale

			fread(&pKey->m_maxScale, sizeof(float), 1, fp);

			// Read in the number of properties

			DWORD dwNumProps;
			fread(&dwNumProps, sizeof(DWORD), 1, fp);

			for (DWORD k = 0; k < dwNumProps; k ++)
			{
				FX_PROP fxProp;
				ReadProp(&fxProp, fp, FORMAT_BIN);

				if (!stricmp(fxProp.m_sName, "Mk"))
				{
					MOVEKEY mk;
					mk.m_tmKey = fxProp.m_data.m_fVec4[0];
					mk.m_pos.x = fxProp.m_data.m_fVec4[1];
					mk.m_pos.y = fxProp.m_data.m_fVec4[2];
					mk.m_pos.z = fxProp.m_data.m_fVec4[3];

					pKey->m_collMoveKeys.AddTail(mk);
				}
				else if (!stricmp(fxProp.m_sName, "Ck"))
				{
					// Colour key

					COLOURKEY ck;

					float ratio = 1.0f / 255.0f;

					DWORD dwCol = fxProp.m_data.m_clrKey.m_dwCol;
					
					ck.m_tmKey = fxProp.m_data.m_clrKey.m_tmKey;
					ck.m_red   = ratio * (float)(dwCol & 0x000000FF);
					ck.m_green = ratio * (float)((dwCol & 0x0000FF00) >> 8);
					ck.m_blue  = ratio * (float)((dwCol & 0x00FF0000) >> 16);
					ck.m_alpha = ratio * (float)((dwCol & 0xFF000000) >> 24);

					pKey->m_collColourKeys.AddTail(ck);
				}
				else if (!stricmp(fxProp.m_sName, "Sk"))
				{
					SCALEKEY sk;
					sk.m_tmKey = fxProp.m_data.m_fVec4[0];
					sk.m_scale = fxProp.m_data.m_fVec4[1];

					pKey->m_collScaleKeys.AddTail(sk);
				}
				else
				{
					pKey->m_collProps.AddTail(fxProp);
				}
			}

			// Add this key to the correct track

			CTrack *pTrack;

			if (pKey->GetTrack() >= (int)pSpellPhase->GetTracks()->GetSize())
			{
				// This track doesn't exist so go ahead and make it

				pTrack = pSpellPhase->AddTrack();
			}
			else
			{
				pTrack = pSpellPhase->GetTracks()->Get(pKey->GetTrack());
			}

			// Before we add it, check the list of properties from the FX dll
			// to make sure that we have all the properties

			CFastList<FX_PROP> tmpProp;
			
			pKey->GetFxRef()->m_pfnGetProps(&tmpProp);
			
			CFastListNode<FX_PROP> *pPropNode = tmpProp.GetHead();

			while (pPropNode)
			{
				CFastListNode<FX_PROP> *pKeyPropNode = pKey->GetCollProps()->GetHead();

				BOOL bFound = FALSE;

				while (pKeyPropNode)
				{
					if (!stricmp(pKeyPropNode->m_Data.m_sName, pPropNode->m_Data.m_sName))
					{
						bFound = TRUE;
					}
					
					pKeyPropNode = pKeyPropNode->m_pNext;
				}

				if (!bFound)
				{
					// This must be a new property so lets go ahead and add it

					pKey->GetCollProps()->AddTail(pPropNode->m_Data);
				}

				pPropNode = pPropNode->m_pNext;
			}
					
			pTrack->GetKeys()->AddTail(pKey);
			pTrack->ArrangeKeys(pKey);

			int nTrack = pSpellPhase->GetTracks()->GetIndex(pTrack);
			pKey->SetTrack(nTrack);
		}

		// Setup the key ID's

		pSpellPhase->SetupUniqueID();
	}

	// Close the file

	fclose(fp);

	// Success !!

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::LoadSpellmgr_t
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

BOOL CSpellEdApp::LoadSpellmgr_t(const char *sFilename, CSpellMgr *pSpellMgr )
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	
	FILE *fp = OpenFile(sFilename, false, true);
	if (!fp) return FALSE;

	char szTag[16] = {0};

	// Read in the number of FX groups

	DWORD dwNumFxGroups;
	fscanf( fp, "%s %lu", szTag, &dwNumFxGroups );

	for (DWORD i = 0; i < dwNumFxGroups; i ++)
	{
		CSpell *pNewSpell = pSpellMgr->AddSpell();
		CPhase *pSpellPhase = pNewSpell->GetPhase(1);

		// Read in the name of this FX group

		char sTmp[128] = {0};

		fscanf( fp, "%s", szTag );
		fseek( fp, 1, SEEK_CUR );
		fgets(sTmp, ARRAY_LEN(sTmp), fp );
		int iStrSize = strlen( sTmp );
		if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
			sTmp[iStrSize - 1] = NULL;
		
		// Read in the number of FX in this group

		DWORD dwNumFx;
		fscanf( fp, "%s %lu", szTag, &dwNumFx );

		// Read in the phase length

		DWORD dwPhaseLen;
		fscanf( fp, "%s %lu", szTag, &dwPhaseLen );
		pSpellPhase->SetPhaseLength(dwPhaseLen);

		// Check to make sure we don't already have this spell
		
		char sTmp2[128];
		strcpy(sTmp2, sTmp);
		
		while (1)
		{
			BOOL bOkay = TRUE;
	
			if (m_spellMgr.FindSpellByName(sTmp2))
			{
				bOkay = FALSE;
				strcat(sTmp2, "_1");
			}
			
			if (bOkay) break;
		}

		strcpy(sTmp, sTmp2);

		pNewSpell->SetName(sTmp);
		pNewSpell->SetGuid(sTmp);

		// Read in the FX

		for (DWORD j = 0; j < dwNumFx; j ++)
		{
			CKey *pKey = new CKey;

			// Read in the reference name

			fscanf( fp, "%s %s", szTag, sTmp );
			pKey->m_pFxRef = pApp->GetFxMgr()->FindFX( strtok( sTmp, ";" ) );
			pKey->SetCustomName( strtok( NULL, "\n" ));

			// Read in the ID of the key

			DWORD dwID;
			fscanf( fp, "%s %lu", szTag, &dwID );
			pKey->SetID(dwID);

			// Read in the link status of the key

			LINK_STATUS ls;
			fscanf( fp, "%s %i", szTag, &ls.bLinked );
			fscanf( fp, "%s %lu", szTag, &ls.dwLinkedID );

			fscanf( fp, "%s", szTag );
			fgets( ls.sNode, ARRAY_LEN( ls.sNode ), fp );
			int iStrSize = strlen( ls.sNode );
			if( (iStrSize > 0) && (ls.sNode[iStrSize - 1] == '\n') )	ls.sNode[iStrSize - 1] = NULL;

			if (ls.bLinked)
			{
				pKey->SetLink(TRUE, ls.dwLinkedID, ls.sNode);
			}
			else
			{
				pKey->SetLink(FALSE, 0, "");
			}

			// Read in the start time

			float tmStart;
			fscanf( fp, "%s %f", szTag, &tmStart );
			pKey->m_tmStart = (int)(tmStart * 1000.0f);

			// Read in the end time

			float tmEnd;
			fscanf( fp, "%s %f", szTag, &tmEnd );
			pKey->m_tmEnd = (int)(tmEnd * 1000.0f);

			// Read in the key repeat

			fscanf( fp, "%s %lu", szTag, &pKey->m_dwKeyRepeat );

			// Read in the track ID

			fscanf( fp, "%s %lu", szTag, &pKey->m_nTrack );

			// Read in the min scale

			fscanf( fp, "%s %f", szTag, &pKey->m_minScale );

			// Read in the max scale

			fscanf( fp, "%s %f", szTag, &pKey->m_maxScale );

			// Read in the number of properties

			DWORD dwNumProps;
			fscanf( fp, "%s %lu", szTag, &dwNumProps );

			for (DWORD k = 0; k < dwNumProps; k ++)
			{
				FX_PROP fxProp;
				ReadProp(&fxProp, fp, FORMAT_TEXT);

				if (!stricmp(fxProp.m_sName, "Mk"))
				{
					MOVEKEY mk;
					mk.m_tmKey = fxProp.m_data.m_fVec4[0];
					mk.m_pos.x = fxProp.m_data.m_fVec4[1];
					mk.m_pos.y = fxProp.m_data.m_fVec4[2];
					mk.m_pos.z = fxProp.m_data.m_fVec4[3];

					pKey->m_collMoveKeys.AddTail(mk);
				}
				else if (!stricmp(fxProp.m_sName, "Ck"))
				{
					// Colour key

					COLOURKEY ck;

					float ratio = 1.0f / 255.0f;

					DWORD dwCol = fxProp.m_data.m_clrKey.m_dwCol;
					
					ck.m_tmKey = fxProp.m_data.m_clrKey.m_tmKey;
					ck.m_red   = ratio * (float)(dwCol & 0x000000FF);
					ck.m_green = ratio * (float)((dwCol & 0x0000FF00) >> 8);
					ck.m_blue  = ratio * (float)((dwCol & 0x00FF0000) >> 16);
					ck.m_alpha = ratio * (float)((dwCol & 0xFF000000) >> 24);

					pKey->m_collColourKeys.AddTail(ck);
				}
				else if (!stricmp(fxProp.m_sName, "Sk"))
				{
					SCALEKEY sk;
					sk.m_tmKey = fxProp.m_data.m_fVec4[0];
					sk.m_scale = fxProp.m_data.m_fVec4[1];

					pKey->m_collScaleKeys.AddTail(sk);
				}
				else
				{
					//make sure that this property is still inside of the object (aka remove
					//dead properties)
					CFastList<FX_PROP> tmpProp;
					pKey->GetFxRef()->m_pfnGetProps(&tmpProp);

					CFastListNode<FX_PROP> *pKeyPropNode = tmpProp.GetHead();

					while (pKeyPropNode)
					{
						if (!stricmp(pKeyPropNode->m_Data.m_sName, fxProp.m_sName))
						{
							//we found a match, it is safe to add

							//one thing we need to check for though is if it is a combo
							//and the items have changed
							if(fxProp.m_nType == FX_PROP::COMBO)
							{
								//pull out the actual value from the file's prop...
								uint32 nPos = 0;

								while(	(nPos < 128) && 
										(fxProp.m_data.m_sVal[nPos] != ',') &&
										(fxProp.m_data.m_sVal[nPos] != '\0'))
								{
									nPos++;
								}

								//now copy over the relevant part of the source combo
								const char* pszPropVal = pKeyPropNode->m_Data.m_data.m_sVal;
								uint32 nComboPos = 0;

								while(	(nComboPos < 128) &&
										(pszPropVal[nComboPos] != ',') &&
										(pszPropVal[nComboPos] != '\0'))
								{
									nComboPos++;
								}

								//ok, now we are in a position to copy them over
								strcpy(fxProp.m_data.m_sVal + nPos, pszPropVal + nComboPos);
							}

							pKey->m_collProps.AddTail(fxProp);
							break;
						}
						
						pKeyPropNode = pKeyPropNode->m_pNext;
					}
				}
			}

			// Add this key to the correct track

			CTrack *pTrack;

			if (pKey->GetTrack() >= (int)pSpellPhase->GetTracks()->GetSize())
			{
				// This track doesn't exist so go ahead and make it

				pTrack = pSpellPhase->AddTrack();
			}
			else
			{
				pTrack = pSpellPhase->GetTracks()->Get(pKey->GetTrack());
			}

			// Before we add it, check the list of properties from the FX dll
			// to make sure that we have all the properties

			CFastList<FX_PROP> tmpProp;
			
			pKey->GetFxRef()->m_pfnGetProps(&tmpProp);
			
			CFastListNode<FX_PROP> *pPropNode = tmpProp.GetHead();

			while (pPropNode)
			{
				CFastListNode<FX_PROP> *pKeyPropNode = pKey->GetCollProps()->GetHead();

				BOOL bFound = FALSE;

				while (pKeyPropNode)
				{
					if (!stricmp(pKeyPropNode->m_Data.m_sName, pPropNode->m_Data.m_sName))
					{
						bFound = TRUE;
					}
					
					pKeyPropNode = pKeyPropNode->m_pNext;
				}

				if (!bFound)
				{
					// This must be a new property so lets go ahead and add it

					pKey->GetCollProps()->AddTail(pPropNode->m_Data);
				}

				pPropNode = pPropNode->m_pNext;
			}
					
			pTrack->GetKeys()->AddTail(pKey);
			pTrack->ArrangeKeys(pKey);

			int nTrack = pSpellPhase->GetTracks()->GetIndex(pTrack);
			pKey->SetTrack(nTrack);
		}

		// Setup the key ID's

		pSpellPhase->SetupUniqueID();
	}

	// Close the file

	fclose(fp);

	// Success !!

	return TRUE;
}


//------------------------------------------------------------------
//
//   FUNCTION : LoadSpells()
//
//   PURPOSE  : Loads all spells
//
//------------------------------------------------------------------

BOOL CSpellEdApp::LoadSpells(const char *sFilename)
{
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if (!pFrame) return FALSE;

	CSpellCtrl *pCtrl = pFrame->GetSpellDlgBar()->GetTabCtrl()->GetSpellCtrl();
	if (!pCtrl) return FALSE;

//	CSpellParser sp;

	// Destroy the tree control

	pCtrl->DeleteAllItems();

	// Destroy any open documents
	
	m_pDocTemplate->CloseAllDocuments(TRUE);
	m_spellMgr.Term();

	LoadSpellmgr(sFilename, &m_spellMgr);

	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : LoadCfgFile()
//
//   PURPOSE  : Loads configuration file
//
//------------------------------------------------------------------

BOOL ReadTree(CSpellMgr *pSpellMgr, CTreeCtrl *pCtrl, HTREEITEM hItem, FILE *fp, int iFormat)
{
	char sTmp[256];
	char sMsg[1024];
	
	// Count the siblings

	int nCount = 0;

	switch( iFormat )
	{
		case FORMAT_TEXT:
		{
			char szTag[16] = {0};

			// Read in the number of siblings
	
			fscanf( fp, "%s %i", szTag, &nCount );
			
			CSpellCtrl *pSpellCtrl = (CSpellCtrl *)pCtrl;

			for (int i = 0; i < nCount; i ++)
			{
				CSpell *pNewSpell = NULL;

				// Read in the name
				
				fscanf( fp, "%s", szTag );
				fseek( fp, 1, SEEK_CUR );
				fgets(sTmp, ARRAY_LEN(sTmp), fp );
				int iStrSize = strlen( sTmp );
				if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
					sTmp[iStrSize - 1] = NULL;
				
				// Read in the images

				int iImage, iSelectedImage;

				fscanf( fp, "%s %i", szTag, &iImage );
				fscanf( fp, "%s %i", szTag, &iSelectedImage );
				

				BOOL bAddItem = TRUE;

				if (iImage == IM_SPELL) 
				{
					pNewSpell = pSpellMgr->FindSpellByName(sTmp);
					if (!pNewSpell) bAddItem = FALSE;
				}

				if (bAddItem)
				{
					HTREEITEM hNewItem = pCtrl->InsertItem(sTmp, hItem, TVI_LAST);
					
					pCtrl->SetItemImage(hNewItem, iImage, iSelectedImage);
					pCtrl->SetItemData(hNewItem, (DWORD)pNewSpell);

					// Read in child

					BOOL bHasChildren;
					fscanf( fp, "%s %i", szTag, &bHasChildren );
					
					// Read in state

					UINT iState;
					fscanf( fp, "%s %u", szTag, &iState );
					
					if (bHasChildren)
					{
						ReadTree(pSpellMgr, pCtrl, hNewItem, fp, FORMAT_TEXT);

						if (iState & 1) pCtrl->Expand(hNewItem, TVE_EXPAND);
					}

					if (iState & 2)
					{
						pCtrl->SelectItem(hNewItem);
						pCtrl->EnsureVisible(hNewItem);
					}
				}
				else
				{
					// Read in rest of data
					
					int dummy;

					fscanf( fp, "%s %i", szTag, &dummy );
					fscanf( fp, "%s %i", szTag, &dummy );
										
					sprintf(sMsg, "Spell [%s] in .cfg file not found !!", sTmp);
					AfxMessageBox(sMsg);
				}
			}
		}
		break;

		case FORMAT_BIN:
		{
			// Read in the number of siblings
	
			fread(&nCount, sizeof(int), 1, fp);

			CSpellCtrl *pSpellCtrl = (CSpellCtrl *)pCtrl;

			for (int i = 0; i < nCount; i ++)
			{
				CSpell *pNewSpell = NULL;

				// Read in the name

				int nLen;
				fread(&nLen, sizeof(int), 1, fp);
				fread(sTmp, nLen, 1, fp);

				// Read in the images

				int iImage, iSelectedImage;

				fread(&iImage, sizeof(int), 1, fp);
				fread(&iSelectedImage, sizeof(int), 1, fp);

				BOOL bAddItem = TRUE;

				if (iImage == IM_SPELL) 
				{
					pNewSpell = pSpellMgr->FindSpellByName(sTmp);
					if (!pNewSpell) bAddItem = FALSE;
				}

				if (bAddItem)
				{
					HTREEITEM hNewItem = pCtrl->InsertItem(sTmp, hItem, TVI_LAST);
					
					pCtrl->SetItemImage(hNewItem, iImage, iSelectedImage);
					pCtrl->SetItemData(hNewItem, (DWORD)pNewSpell);

					// Read in child

					BOOL bHasChildren;
					fread(&bHasChildren, sizeof(BOOL), 1, fp);

					// Read in state

					UINT iState;
					fread(&iState, sizeof(UINT), 1, fp);

					if (bHasChildren)
					{
						ReadTree(pSpellMgr, pCtrl, hNewItem, fp, FORMAT_BIN);

						if (iState & 1) pCtrl->Expand(hNewItem, TVE_EXPAND);
					}

					if (iState & 2)
					{
						pCtrl->SelectItem(hNewItem);
						pCtrl->EnsureVisible(hNewItem);
					}
				}
				else
				{
					// Read in rest of data
					
					int dummy;

					fread(&dummy, sizeof(int), 1, fp);
					fread(&dummy, sizeof(int), 1, fp);
					
					sprintf(sMsg, "Spell [%s] in .cfg file not found !!", sTmp);
					AfxMessageBox(sMsg);
				}
			}
		}
		break;
	}
	
	
	// Success !!

	return TRUE;
}

BOOL CSpellEdApp::LoadCfgFile(const char *sFilename)
{
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if (!pFrame) return FALSE;

	CSpellCtrl *pCtrl = pFrame->GetSpellDlgBar()->GetTabCtrl()->GetSpellCtrl();
	if (!pCtrl) return FALSE;

	
	// Load the scale favourites

	FILE *fp = OpenFile(sFilename, false, false);
	if( !fp ) return FALSE;

	char szBinTest[16] = {0};
	
	fread( szBinTest, 9, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "Siblings:" ) )
	{
		fp = OpenFile(sFilename, false, true);
		if (!fp) return FALSE;

		char szTag[256] = {0};

		// Enumerate the tree and write it out

		ReadTree(&m_spellMgr, pCtrl, pCtrl->GetRootItem(), fp, FORMAT_TEXT);
		pCtrl->SetItemState(pCtrl->GetRootItem(), TVIS_BOLD, TVIS_BOLD);

		((CSpellCtrl *)pCtrl)->FullUpdate(pCtrl->GetRootItem());

		// Read in window positions etc

		CRect rcDlgBar;
		pFrame->GetSpellDlgBar()->GetClientRect(&rcDlgBar);

		int nCount;
		fscanf( fp, "%s %i", szTag, &nCount );
		
		for (int i = 0; i < nCount; i ++)
		{
			char sName[256];
			CRect rcWnd;

			// Read in name

			fscanf( fp, "%s %s", szTag, sName );
			
			// Read in window rectangle

			fscanf( fp, "%s %i", szTag, &rcWnd.left );
			fscanf( fp, "%s %i", szTag, &rcWnd.top );
			fscanf( fp, "%s %i", szTag, &rcWnd.right );
			fscanf( fp, "%s %i", szTag, &rcWnd.bottom );
			
			CSpell *pSpell = m_spellMgr.FindSpellByName(sName);
			if (pSpell)
			{
				CMainFrame *pFrameWnd = (CMainFrame *)AfxGetMainWnd();

				CSpellEdDoc *pDoc = (CSpellEdDoc *)GetDocTemplate()->CreateNewDocument();
				CChildFrame *pFrame = (CChildFrame *)GetDocTemplate()->CreateNewFrame(pDoc, NULL);
				pFrame->GetView()->Init(pSpell);
				pFrame->GetView()->GetDocument()->SetTitle(pSpell->GetName());
				pFrame->MoveWindow(0, 0, 700, 600);
				GetDocTemplate()->InitialUpdateFrame(pFrame, NULL);
			}
		}

		CSpellCtrl *pSpellCtrl = (CSpellCtrl *)pCtrl;
		pSpellCtrl->FullSortTree(pSpellCtrl->GetRootItem());

		if (pSpellCtrl->GetParent())
		{
			pSpellCtrl->GetParent()->Invalidate();
		}

		// Close the file

		fclose(fp);

	}
	else
	{
		fp = OpenFile(sFilename, false, false);
		if (!fp) return FALSE;

		// Enumerate the tree and write it out

		ReadTree(&m_spellMgr, pCtrl, pCtrl->GetRootItem(), fp, FORMAT_BIN);
		pCtrl->SetItemState(pCtrl->GetRootItem(), TVIS_BOLD, TVIS_BOLD);

		((CSpellCtrl *)pCtrl)->FullUpdate(pCtrl->GetRootItem());

		// Read in window positions etc

		CRect rcDlgBar;
		pFrame->GetSpellDlgBar()->GetClientRect(&rcDlgBar);

		int nCount;
		fread(&nCount, sizeof(int), 1, fp);

		for (int i = 0; i < nCount; i ++)
		{
			char sName[256];
			int nLen;
			CRect rcWnd;

			// Read in length of spell name

			fread(&nLen, sizeof(int), 1, fp);

			// Read in name

			fread(sName, nLen, 1, fp);

			// Read in window rectangle

			fread(&rcWnd, sizeof(CRect), 1, fp);

			CSpell *pSpell = m_spellMgr.FindSpellByName(sName);
			if (pSpell)
			{
				CMainFrame *pFrameWnd = (CMainFrame *)AfxGetMainWnd();

				CSpellEdDoc *pDoc = (CSpellEdDoc *)GetDocTemplate()->CreateNewDocument();
				CChildFrame *pFrame = (CChildFrame *)GetDocTemplate()->CreateNewFrame(pDoc, NULL);
				pFrame->GetView()->Init(pSpell);
				pFrame->GetView()->GetDocument()->SetTitle(pSpell->GetName());
				pFrame->MoveWindow(0, 0, 700, 600);
				GetDocTemplate()->InitialUpdateFrame(pFrame, NULL);
			}
		}

		CSpellCtrl *pSpellCtrl = (CSpellCtrl *)pCtrl;
		pSpellCtrl->FullSortTree(pSpellCtrl->GetRootItem());

		if (pSpellCtrl->GetParent())
		{
			pSpellCtrl->GetParent()->Invalidate();
		}

		// Close the file

		fclose(fp);

	}

	
	
	
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : SaveSpells()
//
//   PURPOSE  : Saves spells based on the format we want
//
//------------------------------------------------------------------

BOOL CSpellEdApp::SaveSpells(const char *sFilename)
{	
	switch( m_wFormat )
	{
		case FORMAT_TEXT:
		{
			return SaveSpells_t( sFilename );
		}
		break;

		case FORMAT_BIN:
		{
			return SaveSpells_b( sFilename );
		}
		break;

		default:
		{
			char sMsg[256];

			sprintf( sMsg, "Unknown format!  Unable to save to file %s", sFilename );
			AfxMessageBox( sMsg, MB_OK | MB_ICONSTOP );
		}
		break;
	}

	return FALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveSpells_b
//
//  PURPOSE:	Saves spells in binary format
//
// ----------------------------------------------------------------------- //

BOOL CSpellEdApp::SaveSpells_b(const char *sFilename )
{
	FILE *fp = OpenFile(sFilename, true, false);
	if (!fp) return FALSE;

	// Write out the number of FX groups in this file

	DWORD dwNumFxGroups = m_spellMgr.GetSpells()->GetSize();
	fwrite(&dwNumFxGroups, sizeof(DWORD), 1, fp);

	// Write out the FX
	
	CLinkListNode<CSpell *> *pNode = m_spellMgr.GetSpells()->GetHead();

	while (pNode)
	{
		CPhase *pPhase = pNode->m_Data->GetActivePhase();

//tagRP - May want to switch the number and the name... already done for Text
		// Write out the number of FX in this group

		DWORD dwNumFx = pPhase->GetNumFX();
		fwrite(&dwNumFx, sizeof(DWORD), 1, fp);

		// Write out the name of this FX group

		fwrite(pNode->m_Data->GetName(), 128, 1, fp);

		// Write out the phase length

		DWORD dwPhaseLength = pPhase->GetPhaseLength();
		fwrite(&dwPhaseLength, sizeof(DWORD), 1, fp);

		// Write out all the FX

		CLinkListNode<CTrack *> *pTrackNode = pPhase->GetTracks()->GetHead();

		while (pTrackNode)
		{
			CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

			while (pKeyNode)
			{
				CKey *pKey = pKeyNode->m_Data;

				char sTmp[128];
				memset(sTmp, 0, 128);

				// Write out the reference name

				sprintf( sTmp, "%s;%s;", pKey->m_pFxRef->m_sName, pKey->GetCustomName() );
				fwrite(sTmp, 128, 1, fp);

				// Write out the ID of the key

				DWORD dwID = pKey->GetID();
				fwrite(&dwID, sizeof(DWORD), 1, fp);

				// Write out link status

				LINK_STATUS ls;
				ls.bLinked    = pKey->IsLinked();
				ls.dwLinkedID = pKey->GetLinkedID();
				strcpy(ls.sNode, pKey->GetLinkedNodeName());
				fwrite(&ls, sizeof(LINK_STATUS), 1, fp);
				
				// Write out the start time

				float tmStart = (float)pKey->m_tmStart * 0.001f;
				fwrite(&tmStart, sizeof(float), 1, fp);

				// Write out the end time

				float tmEnd = (float)pKey->m_tmEnd * 0.001f;
				fwrite(&tmEnd, sizeof(float), 1, fp);

				// Write out the key repeat value
				
				fwrite(&pKey->m_dwKeyRepeat, sizeof(DWORD), 1, fp);

				// Write out the track ID

				fwrite(&pKey->m_nTrack, sizeof(DWORD), 1, fp);

				// Write out min scale value
				
				fwrite(&pKey->m_minScale, sizeof(float), 1, fp);

				// Write out max scale value

				fwrite(&pKey->m_maxScale, sizeof(float), 1, fp);

				// Write out the number of properties

				DWORD dwNumProps = pKey->m_collProps.GetSize() +
								   pKey->m_collColourKeys.GetSize() + 
								   pKey->m_collMoveKeys.GetSize() + 
								   pKey->m_collScaleKeys.GetSize();
				fwrite(&dwNumProps, sizeof(DWORD), 1, fp);
				
				// Write out the properties
				
				CFastListNode<FX_PROP> *pPropNode = pKey->m_collProps.GetHead();
				
				while (pPropNode)
				{
					// Write out the property
					
					WriteProp(&pPropNode->m_Data, fp);
					
					pPropNode = pPropNode->m_pNext;
				}
				
				// Write out the colour keys

				CLinkListNode<COLOURKEY> *pColourNode = pKey->m_collColourKeys.GetHead();

				while (pColourNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Ck");
					fxProp.m_nType = FX_PROP::CLRKEY;

					DWORD dwRed   = (int)(pColourNode->m_Data.m_red * 255.0f);
					DWORD dwGreen = (int)(pColourNode->m_Data.m_green * 255.0f);
					DWORD dwBlue  = (int)(pColourNode->m_Data.m_blue * 255.0f);
					DWORD dwAlpha = (int)(pColourNode->m_Data.m_alpha * 255.0f);
					
					fxProp.m_data.m_clrKey.m_tmKey = pColourNode->m_Data.m_tmKey;
					fxProp.m_data.m_clrKey.m_dwCol = dwRed | (dwGreen << 8) | (dwBlue << 16) | (dwAlpha << 24);
					
					pColourNode = pColourNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}
							
				// Write out the motion keys

				CLinkListNode<MOVEKEY> *pMotionNode = pKey->m_collMoveKeys.GetHead();
				
				while (pMotionNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Mk");
					fxProp.m_nType = FX_PROP::VECTOR4;
				
					fxProp.m_data.m_fVec4[0] = pMotionNode->m_Data.m_tmKey;
					fxProp.m_data.m_fVec4[1] = pMotionNode->m_Data.m_pos.x;
					fxProp.m_data.m_fVec4[2] = pMotionNode->m_Data.m_pos.y;
					fxProp.m_data.m_fVec4[3] = pMotionNode->m_Data.m_pos.z;
					
					pMotionNode = pMotionNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}
				
				// Write out the scale keys
				
				CLinkListNode<SCALEKEY> *pScaleNode = pKey->m_collScaleKeys.GetHead();
				
				while (pScaleNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Sk");
					fxProp.m_nType = FX_PROP::VECTOR4;
				
					fxProp.m_data.m_fVec4[0] = pScaleNode->m_Data.m_tmKey;
					fxProp.m_data.m_fVec4[1] = pScaleNode->m_Data.m_scale;
					fxProp.m_data.m_fVec4[2] = 0.0f;
					fxProp.m_data.m_fVec4[3] = 0.0f;
					
					pScaleNode = pScaleNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}

				// Proceed to the next key
				
				pKeyNode = pKeyNode->m_pNext;
			}
			
			pTrackNode = pTrackNode->m_pNext;
		}

		pNode = pNode->m_pNext;
	}

	// Close the file

	fclose(fp);

	// Success !!

	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveSpells_t
//
//  PURPOSE:	Saves spells in text format
//
// ----------------------------------------------------------------------- //

BOOL CSpellEdApp::SaveSpells_t(const char *sFilename )
{
	FILE *fp = OpenFile(sFilename, true, true);
	if (!fp) return FALSE;

	// Write out the number of FX groups in this file

	DWORD dwNumFxGroups = m_spellMgr.GetSpells()->GetSize();
	fprintf( fp, "Groups: %lu\n", dwNumFxGroups );
	
	// Write out the FX
	
	CLinkListNode<CSpell *> *pNode = m_spellMgr.GetSpells()->GetHead();

	while (pNode)
	{
		CPhase *pPhase = pNode->m_Data->GetActivePhase();

		// Write out the name of this FX group

		fprintf( fp, "\tGroupName: %s\n", pNode->m_Data->GetName() );
		
		// Write out the number of FX in this group

		DWORD dwNumFx = pPhase->GetNumFX();
		fprintf( fp, "\tFxInGroup: %lu\n", dwNumFx );
		
		// Write out the phase length

		DWORD dwPhaseLength = pPhase->GetPhaseLength();
		fprintf( fp, "\tPhase: %lu\n", dwPhaseLength );
	
		// Write out all the FX

		CLinkListNode<CTrack *> *pTrackNode = pPhase->GetTracks()->GetHead();

		while (pTrackNode)
		{
			CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

			while (pKeyNode)
			{
				CKey *pKey = pKeyNode->m_Data;

				char sTmp[128];
				memset(sTmp, 0, 128);

				// Write out the reference name with the customname

				fprintf( fp, "\t\tFxName: %s;%s\n", pKey->m_pFxRef->m_sName, pKey->GetCustomName() );
				
				// Write out the ID of the key

				DWORD dwID = pKey->GetID();
				fprintf( fp, "\t\tFxID: %lu\n", dwID );
				
				// Write out link status

				LINK_STATUS ls;
				ls.bLinked    = pKey->IsLinked();
				fprintf( fp, "\t\tLinked: %i\n", ls.bLinked );

				ls.dwLinkedID = pKey->GetLinkedID();
				fprintf( fp, "\t\tLinkID: %lu\n", ls.dwLinkedID );

				strcpy(ls.sNode, pKey->GetLinkedNodeName());
				fprintf( fp, "\t\tLinkNode: %s\n", ls.sNode );
								
				// Write out the start time

				float tmStart = (float)pKey->m_tmStart * 0.001f;
				fprintf( fp, "\t\tStartTime: %f\n", tmStart );
				
				// Write out the end time

				float tmEnd = (float)pKey->m_tmEnd * 0.001f;
				fprintf( fp, "\t\tEndTime: %f\n", tmEnd );
				
				// Write out the key repeat value
				
				fprintf( fp, "\t\tRepeat: %lu\n", pKey->m_dwKeyRepeat );
				
				// Write out the track ID

				fprintf( fp, "\t\tTrackID: %i\n", pKey->m_nTrack );
				
				// Write out min scale value
				
				fprintf( fp, "\t\tMinScale: %f\n", pKey->m_minScale );
				
				// Write out max scale value

				fprintf( fp, "\t\tMaxScale: %f\n", pKey->m_maxScale );
				
				// Write out the number of properties

				DWORD dwNumProps = pKey->m_collProps.GetSize() +
								   pKey->m_collColourKeys.GetSize() + 
								   pKey->m_collMoveKeys.GetSize() + 
								   pKey->m_collScaleKeys.GetSize();
				fprintf( fp, "\t\tProperties: %lu\n", dwNumProps );
								
				// Write out the properties
				
				CFastListNode<FX_PROP> *pPropNode = pKey->m_collProps.GetHead();
				
				while (pPropNode)
				{
					// Write out the property
					
					WriteProp(&pPropNode->m_Data, fp);
					
					pPropNode = pPropNode->m_pNext;
				}
				
				// Write out the colour keys

				CLinkListNode<COLOURKEY> *pColourNode = pKey->m_collColourKeys.GetHead();

				while (pColourNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Ck");
					fxProp.m_nType = FX_PROP::CLRKEY;

					DWORD dwRed   = (int)(pColourNode->m_Data.m_red * 255.0f);
					DWORD dwGreen = (int)(pColourNode->m_Data.m_green * 255.0f);
					DWORD dwBlue  = (int)(pColourNode->m_Data.m_blue * 255.0f);
					DWORD dwAlpha = (int)(pColourNode->m_Data.m_alpha * 255.0f);
					
					fxProp.m_data.m_clrKey.m_tmKey = pColourNode->m_Data.m_tmKey;
					fxProp.m_data.m_clrKey.m_dwCol = dwRed | (dwGreen << 8) | (dwBlue << 16) | (dwAlpha << 24);
					
					pColourNode = pColourNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}
							
				// Write out the motion keys

				CLinkListNode<MOVEKEY> *pMotionNode = pKey->m_collMoveKeys.GetHead();
				
				while (pMotionNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Mk");
					fxProp.m_nType = FX_PROP::VECTOR4;
				
					fxProp.m_data.m_fVec4[0] = pMotionNode->m_Data.m_tmKey;
					fxProp.m_data.m_fVec4[1] = pMotionNode->m_Data.m_pos.x;
					fxProp.m_data.m_fVec4[2] = pMotionNode->m_Data.m_pos.y;
					fxProp.m_data.m_fVec4[3] = pMotionNode->m_Data.m_pos.z;
					
					pMotionNode = pMotionNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}
				
				// Write out the scale keys
				
				CLinkListNode<SCALEKEY> *pScaleNode = pKey->m_collScaleKeys.GetHead();
				
				while (pScaleNode)
				{
					FX_PROP fxProp;
					strcpy(fxProp.m_sName, "Sk");
					fxProp.m_nType = FX_PROP::VECTOR4;
				
					fxProp.m_data.m_fVec4[0] = pScaleNode->m_Data.m_tmKey;
					fxProp.m_data.m_fVec4[1] = pScaleNode->m_Data.m_scale;
					fxProp.m_data.m_fVec4[2] = 0.0f;
					fxProp.m_data.m_fVec4[3] = 0.0f;
					
					pScaleNode = pScaleNode->m_pNext;

					// Write out the property

					WriteProp(&fxProp, fp);
				}

				// Proceed to the next key
				
				pKeyNode = pKeyNode->m_pNext;
			}
			
			pTrackNode = pTrackNode->m_pNext;
		}

		pNode = pNode->m_pNext;
	}

	// Close the file

	fclose(fp);

	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : SaveCfgFile()
//
//   PURPOSE  : Saves configuration file
//
//------------------------------------------------------------------

BOOL WriteTree(CTreeCtrl *pCtrl, HTREEITEM hItem, FILE *fp)
{
	// Count the siblings

	int nCount = 0;

	HTREEITEM hCountItem = hItem;
	while (hCountItem)
	{
		hCountItem = pCtrl->GetNextSiblingItem(hCountItem);
		nCount ++;
	}
	
	// Which format should we save in?

	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	switch( pApp->GetFormat() )
	{	
		case FORMAT_TEXT:
		{
			char sTmp[256];

			// Write out the number of siblings
			
			fprintf( fp, "Siblings: %i\n", nCount );
			
			while (hItem)
			{
				// Write out the name

				CString sName = pCtrl->GetItemText(hItem);
				strcpy(sTmp, sName);
				fprintf( fp, "\tName: %s\n", sTmp );
				
				// Write out the images

				int iImage, iSelectedImage;
				pCtrl->GetItemImage(hItem, iImage, iSelectedImage);

				fprintf( fp, "\t\tImage: %i\n", iImage );
				fprintf( fp, "\t\tSelectedImage: %i\n", iSelectedImage );
				
				// Write out children

				BOOL bHasChildren = pCtrl->ItemHasChildren(hItem);
				fprintf( fp, "\t\tHasCildren: %i\n", bHasChildren );
				
				// Write out state

				UINT iState;
				iState = pCtrl->GetItemState(hItem, TVIS_EXPANDED);
				iState = (iState & TVIS_EXPANDED) ? 1 : 0;
				if (pCtrl->GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED) iState |= 2;
				fprintf( fp, "\t\tState: %u\n", iState );
				
				if (bHasChildren)
				{
					WriteTree(pCtrl, pCtrl->GetChildItem(hItem), fp);
				}

				// Advance to the next element

				hItem = pCtrl->GetNextSiblingItem(hItem);
			}
		}
		break;

		case FORMAT_BIN:
		{
			char sTmp[256];

			// Write out the number of siblings
			
			fwrite(&nCount, sizeof(int), 1, fp);

			while (hItem)
			{
				// Write out the name

				CString sName = pCtrl->GetItemText(hItem);
				strcpy(sTmp, sName);
				int nLen = strlen(sTmp) + 1;
				fwrite(&nLen, sizeof(int), 1, fp);
				fwrite(sTmp, nLen, 1, fp);

				// Write out the images

				int iImage, iSelectedImage;
				pCtrl->GetItemImage(hItem, iImage, iSelectedImage);

				fwrite(&iImage, sizeof(int), 1, fp);
				fwrite(&iSelectedImage, sizeof(int), 1, fp);

				// Write out children

				BOOL bHasChildren = pCtrl->ItemHasChildren(hItem);
				fwrite(&bHasChildren, sizeof(BOOL), 1, fp);

				// Write out state

				UINT iState;
				iState = pCtrl->GetItemState(hItem, TVIS_EXPANDED);
				iState = (iState & TVIS_EXPANDED) ? 1 : 0;
				if (pCtrl->GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED) iState |= 2;
				fwrite(&iState, sizeof(UINT), 1, fp);

				if (bHasChildren)
				{
					WriteTree(pCtrl, pCtrl->GetChildItem(hItem), fp);
				}

				// Advance to the next element

				hItem = pCtrl->GetNextSiblingItem(hItem);
			}
		}
		break;

	}
	
	// Success !!

	return TRUE;
}

BOOL CSpellEdApp::SaveCfgFile(const char *sFilename)
{
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if (!pFrame) return FALSE;

	CSpellCtrl *pCtrl = pFrame->GetSpellDlgBar()->GetTabCtrl()->GetSpellCtrl();
	if (!pCtrl) return FALSE;

	
	switch( m_wFormat )
	{
		case FORMAT_TEXT:
		{
			FILE *fp = OpenFile(sFilename, true, true);
			if (!fp) return FALSE;	

			// Enumerate the tree and write it out

			WriteTree(pCtrl, pCtrl->GetRootItem(), fp);

			// Write out window positions etc...

			int nCount = 0;

			POSITION pos = m_pDocTemplate->GetFirstDocPosition();

			while (pos)
			{
				CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);
				
				if (pDoc)
				{
					nCount ++;
				}
			}

			fprintf( fp, "Count: %i\n", nCount );
			
			pos = m_pDocTemplate->GetFirstDocPosition();

			while (pos)
			{
				CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);

				if (pDoc)
				{
					POSITION viewPos = pDoc->GetFirstViewPosition();
					CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);
					CWnd *pWnd = pView->GetParent();

					CRect rcWnd;
					pWnd->GetWindowRect(&rcWnd);
					pWnd->GetParent()->ScreenToClient(&rcWnd);

					const char *sName = pView->GetSpell()->GetName();
					int nLen = strlen(sName) + 1;

					fprintf( fp, "\tFXName: %s\n", sName );
										
					fprintf( fp, "\t\tLeft: %i\n", rcWnd.left );
					fprintf( fp, "\t\tTop: %i\n", rcWnd.top );
					fprintf( fp, "\t\tRight: %i\n", rcWnd.right );
					fprintf( fp, "\t\tBottom: %i\n", rcWnd.bottom );
				}
			}

			fclose(fp);

		}
		break;

		case FORMAT_BIN:
		{
			FILE *fp = OpenFile(sFilename, true, false);
			if (!fp) return FALSE;	

			// Enumerate the tree and write it out

			WriteTree(pCtrl, pCtrl->GetRootItem(), fp);

			// Write out window positions etc...

			int nCount = 0;

			POSITION pos = m_pDocTemplate->GetFirstDocPosition();

			while (pos)
			{
				CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);
				
				if (pDoc)
				{
					nCount ++;
				}
			}

			fwrite(&nCount, sizeof(int), 1, fp);

			pos = m_pDocTemplate->GetFirstDocPosition();

			while (pos)
			{
				CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);

				if (pDoc)
				{
					POSITION viewPos = pDoc->GetFirstViewPosition();
					CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);
					CWnd *pWnd = pView->GetParent();

					CRect rcWnd;
					pWnd->GetWindowRect(&rcWnd);
					pWnd->GetParent()->ScreenToClient(&rcWnd);

					const char *sName = pView->GetSpell()->GetName();
					int nLen = strlen(sName) + 1;

					fwrite(&nLen, sizeof(int), 1, fp);
					fwrite(sName, nLen, 1, fp);
					fwrite(&rcWnd, sizeof(CRect), 1, fp);
				}
			}

			fclose(fp);

		}
		break;

		default:
		{
			AfxMessageBox( "Unknown format... failed to save config file!", MB_OK | MB_ICONSTOP );
		}
		break;
	}

	
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : GetViewBySpell()
//
//   PURPOSE  : 
//
//------------------------------------------------------------------

CSpellEdView* CSpellEdApp::GetViewBySpell(CSpell *pSpell)
{
	POSITION pos = m_pDocTemplate->GetFirstDocPosition();

	while (pos)
	{
		CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);

		if (pDoc)
		{
			POSITION viewPos = pDoc->GetFirstViewPosition();
			CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);

			if (pView->GetSpell() == pSpell) return pView;
		}
	}

	// Failure !!

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : UpdateViewNames()
//
//   PURPOSE  : Synchronises the names of the spells
//
//------------------------------------------------------------------

void CSpellEdApp::UpdateViewNames()
{
	POSITION pos = m_pDocTemplate->GetFirstDocPosition();

	while (pos)
	{
		CDocument *pDoc = m_pDocTemplate->GetNextDoc(pos);

		if (pDoc)
		{
			POSITION viewPos = pDoc->GetFirstViewPosition();
			CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);

			pDoc->SetTitle(pView->GetSpell()->GetName());
		}
	}
}

void CSpellEdApp::OnFileNew() 
{
	CSpell *pNewSpell = m_spellMgr.AddSpell();
	CMainFrame *pFrameWnd = (CMainFrame *)AfxGetMainWnd();
	CSpellCtrl *pCtrl = pFrameWnd->GetSpellDlgBar()->GetTabCtrl()->GetSpellCtrl();

	// Add this spell to the tree control

	TV_INSERTSTRUCT tvItem;

	tvItem.hParent			   = pCtrl->GetRootItem();
	tvItem.hInsertAfter		   = TVI_LAST;
	tvItem.item.mask		   = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
	tvItem.item.pszText		   = (char *)(LPCSTR)pNewSpell->GetName();
	tvItem.item.cchTextMax	   = strlen(tvItem.item.pszText);
	tvItem.item.iImage		   = 2;
	tvItem.item.iSelectedImage = 2;
	tvItem.item.lParam		   = (LPARAM)pNewSpell;
	
	HTREEITEM hNewSpell = pCtrl->InsertItem(&tvItem);
	if (hNewSpell) pCtrl->EnsureVisible(hNewSpell);

	CSpellEdDoc *pDoc = (CSpellEdDoc *)GetDocTemplate()->CreateNewDocument();
	CChildFrame *pFrame = (CChildFrame *)GetDocTemplate()->CreateNewFrame(pDoc, NULL);
	pFrame->GetView()->Init(pNewSpell);
	pFrame->GetView()->GetDocument()->SetTitle(pNewSpell->GetName());
	GetDocTemplate()->InitialUpdateFrame(pFrame, NULL);
	// See if any windows are maximized

	BOOL bMoveWindow = TRUE;

	POSITION pos = GetDocTemplate()->GetFirstDocPosition();

	while (pos)
	{
		CDocument *pDoc = GetDocTemplate()->GetNextDoc(pos);

		if (pDoc)
		{
			POSITION viewPos = pDoc->GetFirstViewPosition();
			CSpellEdView *pView = (CSpellEdView *)pDoc->GetNextView(viewPos);

			if (pView->GetParent()->IsZoomed()) bMoveWindow = FALSE;
		}
	}			
	if (bMoveWindow) pFrame->MoveWindow(0, 0, 700, 600);
	pFrameWnd->RecalcLayout();	
}

//------------------------------------------------------------------
//
//   FUNCTION : GetColourFavourite()
//
//   PURPOSE  : Returns a named colour animation
//
//------------------------------------------------------------------

CK_FAVOURITE* CSpellEdApp::GetColourFavourite(CString sName)
{
	CLinkListNode<CK_FAVOURITE *> *pNode = m_collClrFavourites.GetHead();

	while (pNode)
	{
		if (pNode->m_Data->m_sName == sName)
		{
			return pNode->m_Data;
		}
		
		pNode = pNode->m_pNext;
	}
	
	// Failure !!

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : ExitInstance()
//
//   PURPOSE  : On Exit
//
//------------------------------------------------------------------

int CSpellEdApp::ExitInstance() 
{
	// Write out favourites

	SaveFavourites();

	// Delete the favourites

	CLinkListNode<CK_FAVOURITE *> *pNode = m_collClrFavourites.GetHead();

	while (pNode)
	{
		delete pNode->m_Data;

		pNode = pNode->m_pNext;
	}

	CLinkListNode<SK_FAVOURITE *> *pSkNode = m_collSclFavourites.GetHead();

	while (pSkNode)
	{
		delete pSkNode->m_Data;

		pSkNode = pSkNode->m_pNext;
	}
	
	return CWinApp::ExitInstance();
}

//------------------------------------------------------------------
//
//   FUNCTION : LoadFavourites()
//
//   PURPOSE  : Loads the favourites file
//
//------------------------------------------------------------------

void CSpellEdApp::LoadFavourites()
{
	// Load the colour favourites

	AddColourFavourites(m_sFavDir + "clranim.cfv");
	
	// Load the scale favourites

	AddScaleFavourites(m_sFavDir + "sclanim.sfv");

	// Load the key favorites

	AddKeyFavourites(m_sFavDir + "favs2.kfv");

	// Load the motion favourites

	AddMoveFavourites(m_sFavDir + "mkanim.mfv");
}

//------------------------------------------------------------------
//
//   FUNCTION : SaveFavourites()
//
//   PURPOSE  : Saves the favourites file based on format
//
//------------------------------------------------------------------

void CSpellEdApp::SaveFavourites()
{
	switch( m_wFormat )
	{	
		case FORMAT_TEXT:
		{
			SaveColorFavs_t(m_sFavDir + "clranim.cfv");
			SaveScaleFavs_t(m_sFavDir + "sclanim.sfv");
			SaveMotionFavs_t(m_sFavDir + "mkanim.mfv");
			SaveKeyFavs_t(m_sFavDir + "favs2.kfv");

		}
		break;

		case FORMAT_BIN:
		{
			SaveColorFavs_b(m_sFavDir + "clranim.cfv");
			SaveScaleFavs_b(m_sFavDir + "sclanim.sfv");
			SaveMotionFavs_b(m_sFavDir + "mkanim.mfv");
			SaveKeyFavs_b(m_sFavDir + "favs2.kfv");
		}
		break;

		default:
		{
			AfxMessageBox( "Unknown format... failed to save favorites!", MB_OK | MB_ICONSTOP );
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveColorFavs_b
//
//  PURPOSE:	Save the color keys as binary
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveColorFavs_b(const char* sFilename)
{
	FILE *fp = OpenFile(sFilename, true, false);
	if (fp) 
	{
		DWORD dwNum = m_collClrFavourites.GetSize();
		fwrite(&dwNum, sizeof(DWORD), 1, fp);

		CLinkListNode<CK_FAVOURITE *> *pNode = m_collClrFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name

			const char *sName = pNode->m_Data->m_sName;
			int nLen = strlen(sName);

			fwrite(&nLen, sizeof(int), 1, fp);
			fwrite(sName, nLen, 1, fp);

			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fwrite(&dwNumKeys, sizeof(DWORD), 1, fp);
			
			CLinkListNode<COLOURKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				fwrite(&pKeyNode->m_Data, sizeof(COLOURKEY), 1, fp);

				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdAll::SaveColorFavs_t
//
//  PURPOSE:	Save the color keys as text
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveColorFavs_t(const char* sFilename)
{
	COLOURKEY ck;
	FILE *fp = OpenFile(sFilename, true, true);
	if (fp) 
	{
		DWORD dwNum = m_collClrFavourites.GetSize();
		fprintf( fp, "ColorFavorites: %lu\n", dwNum );
		
		CLinkListNode<CK_FAVOURITE *> *pNode = m_collClrFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name
			
			fprintf( fp, "\tName: %s\n", pNode->m_Data->m_sName );

			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fprintf( fp, "\tKeys: %lu\n", dwNumKeys ); 
			
			CLinkListNode<COLOURKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				ck = pKeyNode->m_Data;

				fprintf( fp, "\t\tCLRKEY: %f %f %f %f %f\n", ck.m_tmKey, ck.m_red, ck.m_green, ck.m_blue, ck.m_alpha );
				
				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveScaleFavs_b
//
//  PURPOSE:	Save the scale keys as binary
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveScaleFavs_b(const char* sFilename)
{
	FILE *fp = OpenFile(sFilename, true, false);
	if (fp) 
	{
		DWORD dwNum = m_collSclFavourites.GetSize();
		fwrite(&dwNum, sizeof(DWORD), 1, fp);

		CLinkListNode<SK_FAVOURITE *> *pNode = m_collSclFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name

			const char *sName = pNode->m_Data->m_sName;
			int nLen = strlen(sName);

			fwrite(&nLen, sizeof(int), 1, fp);
			fwrite(sName, nLen, 1, fp);

			// Write the min scale

			fwrite(&pNode->m_Data->m_minScale, sizeof(float), 1, fp);

			// Write the max scale

			fwrite(&pNode->m_Data->m_maxScale, sizeof(float), 1, fp);
			
			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fwrite(&dwNumKeys, sizeof(DWORD), 1, fp);
			
			CLinkListNode<SCALEKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				fwrite(&pKeyNode->m_Data, sizeof(SCALEKEY), 1, fp);

				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveScaleFavs_t
//
//  PURPOSE:	Save the scale keys as text
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveScaleFavs_t(const char* sFilename)
{
	FILE *fp = OpenFile(sFilename, true, true);
	if (fp) 
	{
		DWORD dwNum = m_collSclFavourites.GetSize();
		fprintf( fp, "ScaleFavorites: %lu\n", dwNum );

		CLinkListNode<SK_FAVOURITE *> *pNode = m_collSclFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name

			fprintf( fp, "\tName: %s\n", pNode->m_Data->m_sName );
			
			// Write the min scale

			fprintf( fp, "\tMinScale: %f\n", pNode->m_Data->m_minScale );
			
			// Write the max scale

			fprintf( fp, "\tMaxScale: %f\n", pNode->m_Data->m_maxScale );
						
			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fprintf( fp, "\tKeys: %lu\n", dwNumKeys );
						
			CLinkListNode<SCALEKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				fprintf( fp, "\t\tSCLKEY: %f %f\n", pKeyNode->m_Data.m_tmKey, pKeyNode->m_Data.m_scale );
				
				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveMotionFavs_b
//
//  PURPOSE:	Saves the motion keys as binary
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveMotionFavs_b(const char* sFilename)
{
	FILE *fp = OpenFile(sFilename, true, false);
	if (fp) 
	{
		DWORD dwNum = m_collMvFavourites.GetSize();
		fwrite(&dwNum, sizeof(DWORD), 1, fp);

		CLinkListNode<MK_FAVOURITE *> *pNode = m_collMvFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name

			const char *sName = pNode->m_Data->m_sName;
			int nLen = strlen(sName);

			fwrite(&nLen, sizeof(int), 1, fp);
			fwrite(sName, nLen, 1, fp);
			
			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fwrite(&dwNumKeys, sizeof(DWORD), 1, fp);
			
			CLinkListNode<MOVEKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				fwrite(&pKeyNode->m_Data, sizeof(MOVEKEY), 1, fp);

				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveMotionFavs_t
//
//  PURPOSE:	Saves the motion keys as text
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveMotionFavs_t(const char* sFilename)
{
	FILE *fp = OpenFile(sFilename, true, true);
	if (fp) 
	{
		DWORD dwNum = m_collMvFavourites.GetSize();
		fprintf( fp, "MotionFavorites: %lu\n", dwNum );

		CLinkListNode<MK_FAVOURITE *> *pNode = m_collMvFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name
			
			fprintf( fp, "\tName: %s\n", pNode->m_Data->m_sName );
			
			// Write the number of keys

			DWORD dwNumKeys = pNode->m_Data->m_collKeys.GetSize();
			fprintf( fp, "\tKeys: %lu\n", dwNumKeys );
			
			CLinkListNode<MOVEKEY> *pKeyNode = pNode->m_Data->m_collKeys.GetHead();

			while (pKeyNode)
			{
				MOVEKEY mk = pKeyNode->m_Data;
				
				fprintf( fp, "\t\tMOVKEY: %f %f %f %f\n", mk.m_tmKey, mk.m_pos.x, mk.m_pos.y, mk.m_pos.z );
				fprintf( fp, "\t\t\t%i %f %f %f\n", mk.m_bSelected, mk.m_anchorPos.x, mk.m_anchorPos.y, mk.m_anchorPos.z );

				pKeyNode = pKeyNode->m_pNext;
			}
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveKeyFavs_b
//
//  PURPOSE:	Save the FX key favorites as binary
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveKeyFavs_b(const char* sFilename)
{
	// Write out the favourite keys

	FILE *fp = OpenFile(sFilename, true, false);
	if (fp) 
	{
		DWORD dwNum = m_collKeyFavourites.GetSize();
		fwrite(&dwNum, sizeof(DWORD), 1, fp);

		CLinkListNode<FK_FAVOURITE *> *pNode = m_collKeyFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			// Write the name

			const char *sName = pNode->m_Data->m_sName;
			int nLen = strlen(sName);

			fwrite(&nLen, sizeof(int), 1, fp);
			fwrite(sName, nLen, 1, fp);

			// Write out the ref structure

			fwrite(&pNode->m_Data->m_ref, sizeof(FX_REF), 1, fp);

			// Write the min scale

			fwrite(&pNode->m_Data->m_minScale, sizeof(float), 1, fp);

			// Write the max scale

			fwrite(&pNode->m_Data->m_maxScale, sizeof(float), 1, fp);

			// Write out the length

			fwrite(&pNode->m_Data->m_tmLength, sizeof(DWORD), 1, fp);
			
			// Write out the props

			DWORD dwSize = pNode->m_Data->m_collProps.GetSize();
			fwrite(&dwSize, sizeof(DWORD), 1, fp);

			CFastListNode<FX_PROP> *pPropNode = pNode->m_Data->m_collProps.GetHead();

			while (pPropNode)
			{
				fwrite(&pPropNode->m_Data, sizeof(FX_PROP), 1, fp);
				
				pPropNode = pPropNode->m_pNext;
			}

			// Write out the colour keys

			pNode->m_Data->m_collColourKeys.WriteList(fp);

			// Write out the scale keys

			pNode->m_Data->m_collScaleKeys.WriteList(fp);

			// Write out the motion keys

			pNode->m_Data->m_collMoveKeys.WriteList(fp);
				
			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::SaveKeyFavs_t
//
//  PURPOSE:	Save the FX key favorites as text
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::SaveKeyFavs_t(const char* sFilename)
{
	// Write out the favourite keys

	FILE *fp = OpenFile(sFilename, true, true);
	if (fp) 
	{
		DWORD dwNum = m_collKeyFavourites.GetSize();
		fprintf( fp, "KeyFavorites: %lu\n", dwNum );
		
		CLinkListNode<FK_FAVOURITE *> *pNode = m_collKeyFavourites.GetHead();

		for (DWORD i = 0; i < dwNum; i ++)
		{
			FK_FAVOURITE	*pFavKey = pNode->m_Data;

			// Write the name

			fprintf( fp, "\tKeyName: %s\n", pFavKey->m_sName );
			
			// Write out the ref structure

			fprintf( fp, "\t\tFXName: %s\n",  pFavKey->m_ref.m_sName );
			fprintf( fp, "\t\tFXType:	%lu\n",  pFavKey->m_ref.m_dwType ); 
			
			// Write the min scale

			fprintf( fp, "\t\tMinScale: %f\n",  pFavKey->m_minScale );
			
			// Write the max scale

			fprintf( fp, "\t\tMaxScale: %f\n",  pFavKey->m_maxScale );
			
			// Write out the length

			fprintf( fp, "\t\tLength: %lu\n",  pFavKey->m_tmLength );
						
			// Write out the props...

			DWORD dwNumProps =  pFavKey->m_collProps.GetSize() +
								pFavKey->m_collColourKeys.GetSize() +
								pFavKey->m_collScaleKeys.GetSize() +
								pFavKey->m_collMoveKeys.GetSize();
			fprintf( fp, "\t\tProperties: %lu\n", dwNumProps );
			
			// Write out the properties
				
			CFastListNode<FX_PROP> *pPropNode = pFavKey->m_collProps.GetHead();
			
			while (pPropNode)
			{
				// Write out the property
				
				WriteProp(&pPropNode->m_Data, fp);
				
				pPropNode = pPropNode->m_pNext;
			}

			// Write out the colour keys

			CLinkListNode<COLOURKEY> *pColourNode = pFavKey->m_collColourKeys.GetHead();

			while (pColourNode)
			{
				FX_PROP fxProp;
				strcpy(fxProp.m_sName, "Ck");
				fxProp.m_nType = FX_PROP::CLRKEY;

				DWORD dwRed   = (int)(pColourNode->m_Data.m_red * 255.0f);
				DWORD dwGreen = (int)(pColourNode->m_Data.m_green * 255.0f);
				DWORD dwBlue  = (int)(pColourNode->m_Data.m_blue * 255.0f);
				DWORD dwAlpha = (int)(pColourNode->m_Data.m_alpha * 255.0f);
				
				fxProp.m_data.m_clrKey.m_tmKey = pColourNode->m_Data.m_tmKey;
				fxProp.m_data.m_clrKey.m_dwCol = dwRed | (dwGreen << 8) | (dwBlue << 16) | (dwAlpha << 24);
				
				pColourNode = pColourNode->m_pNext;

				// Write out the property

				WriteProp(&fxProp, fp);
			}
						
			// Write out the motion keys

			CLinkListNode<MOVEKEY> *pMotionNode = pFavKey->m_collMoveKeys.GetHead();
			
			while (pMotionNode)
			{
				FX_PROP fxProp;
				strcpy(fxProp.m_sName, "Mk");
				fxProp.m_nType = FX_PROP::VECTOR4;
			
				fxProp.m_data.m_fVec4[0] = pMotionNode->m_Data.m_tmKey;
				fxProp.m_data.m_fVec4[1] = pMotionNode->m_Data.m_pos.x;
				fxProp.m_data.m_fVec4[2] = pMotionNode->m_Data.m_pos.y;
				fxProp.m_data.m_fVec4[3] = pMotionNode->m_Data.m_pos.z;
				
				pMotionNode = pMotionNode->m_pNext;

				// Write out the property

				WriteProp(&fxProp, fp);
			}
			
			// Write out the scale keys
			
			CLinkListNode<SCALEKEY> *pScaleNode = pFavKey->m_collScaleKeys.GetHead();
			
			while (pScaleNode)
			{
				FX_PROP fxProp;
				strcpy(fxProp.m_sName, "Sk");
				fxProp.m_nType = FX_PROP::VECTOR4;
			
				fxProp.m_data.m_fVec4[0] = pScaleNode->m_Data.m_tmKey;
				fxProp.m_data.m_fVec4[1] = pScaleNode->m_Data.m_scale;
				fxProp.m_data.m_fVec4[2] = 0.0f;
				fxProp.m_data.m_fVec4[3] = 0.0f;
				
				pScaleNode = pScaleNode->m_pNext;

				// Write out the property

				WriteProp(&fxProp, fp);
			}
			
			// Procede to the next Key Favorite...

			pNode = pNode->m_pNext;
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileLaunch()
//
//   PURPOSE  : Launches the engine
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileLaunch() 
{
	char sApp[256];
	char sCmdLine[256];
	
	CString sAppName = GetProfileString("", "App", "");//g_bm.GetString("App", "FileName");
	CString sRezName = GetProfileString("", "ResourceFileLocation", "");//g_bm.GetString("App", "RezFile");
	CString sCmdLineName = GetProfileString("", "CommandLine", "");//g_bm.GetString("App", "ExtraParameters");

	if ((sAppName.IsEmpty()) || (sRezName.IsEmpty()) || (sCmdLineName.IsEmpty()))
	{
		return;
	}

	sprintf(sApp, "%s ", sAppName);
	sprintf(sCmdLine, "-rez \"%s\" %s", sRezName, sCmdLineName);

	// Set the working directory

	char sWrkDir[256];
	strcpy(sWrkDir, sAppName.GetBuffer(sAppName.GetLength()));
	char *psTmp = sWrkDir + strlen(sWrkDir) - 1;	

	while (*psTmp != '\\')
	{
		if (psTmp == sWrkDir) break;

		psTmp --;
	}

	*psTmp = 0;
	SetCurrentDirectory(sWrkDir);

	// Save our current file

	OnFileSave();

	BOOL				bRet;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO			startInfo;

	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);

	// Execute
	
	bRet = CreateProcess(sApp, sCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo);
}

#include "Splash.h"
BOOL CSpellEdApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinApp::PreTranslateMessage(pMsg);
}

//------------------------------------------------------------------
//
//   FUNCTION : AddColourFavourites()
//
//   PURPOSE  : Adds a colour favourites file
//
//------------------------------------------------------------------

void CSpellEdApp::AddColourFavourites(const char *sFilename)
{
	FILE *fp = OpenFile(sFilename, false, false, false);
	if( !fp ) return;

	char szBinTest[16] = {0};
	
	fread( szBinTest, 15, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "ColorFavorites:" ) )
	{
		// We are reading as text format...

		fp = OpenFile(sFilename, false, true);
		if (!fp) return;

		char szTag[16] = {0};

		DWORD dwNum;
		fscanf( fp, "%s %lu", szTag, &dwNum );
		
		for (DWORD i = 0; i < dwNum; i ++)
		{
			CK_FAVOURITE *pFavourite = new CK_FAVOURITE;

			char sTmp[256] = {0};
			
			// Read in the name...

			fscanf( fp, "%s", szTag );
			fseek( fp, 1, SEEK_CUR );
			fgets(sTmp, ARRAY_LEN(sTmp), fp );
			int iStrSize = strlen( sTmp );
			if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
				sTmp[iStrSize - 1] = NULL;
			pFavourite->m_sName = sTmp;

			// Read in the number of keys

			DWORD dwNumKeys;
			fscanf( fp, "%s %lu", szTag, &dwNumKeys );
			
			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				COLOURKEY key;
				fscanf( fp, "%s %f %f %f %f %f", szTag, &key.m_tmKey, &key.m_red, &key.m_green, &key.m_blue, &key.m_alpha );
				
				pFavourite->m_collKeys.AddTail(key);
			}

			m_collClrFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
	else
	{
		// We are reading as binary...
		
		fp = OpenFile(sFilename, false, false);
		if (!fp) return;

		DWORD dwNum;
		fread(&dwNum, sizeof(DWORD), 1, fp);

		for (DWORD i = 0; i < dwNum; i ++)
		{
			CK_FAVOURITE *pFavourite = new CK_FAVOURITE;

			char sTmp[256];
			memset(sTmp, 0, 256);
			
			// Read in the name

			int nLen;
			fread(&nLen, sizeof(int), 1, fp);
			fread(sTmp, nLen, 1, fp);
			
			pFavourite->m_sName = sTmp;

			// Read in the number of keys

			DWORD dwNumKeys;
			fread(&dwNumKeys, sizeof(DWORD), 1, fp);

			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				COLOURKEY key;
				fread(&key, sizeof(COLOURKEY), 1, fp);

				pFavourite->m_collKeys.AddTail(key);
			}

			m_collClrFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : AddScaleFavourites()
//
//   PURPOSE  : Adds a scale favourites file
//
//------------------------------------------------------------------

void CSpellEdApp::AddScaleFavourites(const char *sFilename)
{
	// Load the scale favourites

	FILE *fp = OpenFile(sFilename, false, false, false);
	if( !fp ) return;

	char szBinTest[16] = {0};
	
	fread( szBinTest, 15, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "ScaleFavorites:" ) )
	{
		// We are reading as text format...
		fp = OpenFile(sFilename, false, true);
		if (!fp) return;

		char szTag[16] = {0};

		DWORD dwNum;
		fscanf( fp, "%s %lu", szTag, &dwNum );

		for (DWORD i = 0; i < dwNum; i ++)
		{
			SK_FAVOURITE *pFavourite = new SK_FAVOURITE;

			char sTmp[256];
			memset(sTmp, 0, 256);
			
			// Read in the name...

			fscanf( fp, "%s", szTag );
			fseek( fp, 1, SEEK_CUR );
			fgets(sTmp, ARRAY_LEN(sTmp), fp );
			int iStrSize = strlen( sTmp );
			if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
				sTmp[iStrSize - 1] = NULL;
			pFavourite->m_sName = sTmp;

			// Read in the min scale

			fscanf( fp, "%s %f", szTag, &pFavourite->m_minScale );

			// Read in the max scale

			fscanf( fp, "%s %f", szTag, &pFavourite->m_maxScale );

			// Read in the number of keys

			DWORD dwNumKeys;
			fscanf( fp, "%s %lu", szTag, &dwNumKeys );
			
			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				SCALEKEY key;
				fscanf( fp, "%s %f %f", szTag, &key.m_tmKey, &key.m_scale );
				
				pFavourite->m_collKeys.AddTail(key);
			}

			m_collSclFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
	else
	{
		// We are reading as binary...

		fp = OpenFile(sFilename, false, false);
		if (!fp) return;

		DWORD dwNum;
		fread(&dwNum, sizeof(DWORD), 1, fp);

		for (DWORD i = 0; i < dwNum; i ++)
		{
			SK_FAVOURITE *pFavourite = new SK_FAVOURITE;

			char sTmp[256];
			memset(sTmp, 0, 256);
			
			// Read in the name

			int nLen;
			fread(&nLen, sizeof(int), 1, fp);
			fread(sTmp, nLen, 1, fp);
			
			pFavourite->m_sName = sTmp;

			// Read in the min scale

			fread(&pFavourite->m_minScale, sizeof(float), 1, fp);

			// Read in the max scale

			fread(&pFavourite->m_maxScale, sizeof(float), 1, fp);

			// Read in the number of keys

			DWORD dwNumKeys;
			fread(&dwNumKeys, sizeof(DWORD), 1, fp);

			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				SCALEKEY key;
				fread(&key, sizeof(SCALEKEY), 1, fp);

				pFavourite->m_collKeys.AddTail(key);
			}

			m_collSclFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : AddMoveFavourites()
//
//   PURPOSE  : Adds a motion favourites file
//
//------------------------------------------------------------------

void CSpellEdApp::AddMoveFavourites(const char *sFilename)
{
	FILE *fp = OpenFile(sFilename, false, false, false);
	if( !fp ) return;

	char szBinTest[18] = {0};
	
	fread( szBinTest, 15, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "MotionFavorites" ) )
	{
		FILE *fp = OpenFile(sFilename, false, true);
		if (!fp) return;

		char szTag[18] = {0}; 

		DWORD dwNum;
		fscanf( fp, "%s %lu", szTag, &dwNum ); 
		
		for (DWORD i = 0; i < dwNum; i ++)
		{
			MK_FAVOURITE *pFavourite = new MK_FAVOURITE;

			char sTmp[256] = {0};
						
			// Read in the name

			fscanf( fp, "%s", szTag );
			fseek( fp, 1, SEEK_CUR );
			fgets(sTmp, ARRAY_LEN(sTmp), fp );
			int iStrSize = strlen( sTmp );
			if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
				sTmp[iStrSize - 1] = NULL;
			pFavourite->m_sName = sTmp;

			// Read in the number of keys

			DWORD dwNumKeys;
			fscanf( fp, "%s %lu", szTag, &dwNumKeys );
			
			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				MOVEKEY key;
				
				fscanf( fp, "%s %f %f %f %f", szTag, &key.m_tmKey, &key.m_pos.x, &key.m_pos.y, &key.m_pos.z );
				fscanf( fp, "%i %f %f %f %f", &key.m_bSelected, &key.m_anchorPos.x, &key.m_anchorPos.y, &key.m_anchorPos.z ); 
				
				pFavourite->m_collKeys.AddTail(key);
			}

			m_collMvFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
	else
	{
		FILE *fp = OpenFile(sFilename, false, false);
		if (!fp) return;

		DWORD dwNum;
		fread(&dwNum, sizeof(DWORD), 1, fp);

		for (DWORD i = 0; i < dwNum; i ++)
		{
			MK_FAVOURITE *pFavourite = new MK_FAVOURITE;

			char sTmp[256];
			memset(sTmp, 0, 256);
			
			// Read in the name

			int nLen;
			fread(&nLen, sizeof(int), 1, fp);
			fread(sTmp, nLen, 1, fp);
			
			pFavourite->m_sName = sTmp;

			// Read in the number of keys

			DWORD dwNumKeys;
			fread(&dwNumKeys, sizeof(DWORD), 1, fp);

			for (DWORD j = 0; j < dwNumKeys; j ++)
			{
				MOVEKEY key;
				fread(&key, sizeof(MOVEKEY), 1, fp);

				pFavourite->m_collKeys.AddTail(key);
			}

			m_collMvFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : AddKeyFavourites()
//
//   PURPOSE  : Adds a key favourites file
//
//------------------------------------------------------------------

void CSpellEdApp::AddKeyFavourites(const char *sFilename)
{
	FILE *fp = OpenFile(sFilename, false, false, false);
	if( !fp ) return;

	char szBinTest[18] = {0};
	
	fread( szBinTest, 13, 1, fp );
	
	// We got what we need now close it... 

	fclose( fp );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "KeyFavorites:" ) )
	{
		FILE *fp = OpenFile(sFilename, false, true);
		if( !fp ) return;
		
		char szTag[18] = {0};

		DWORD dwNum;
		fscanf( fp, "%s %lu", szTag, &dwNum );

		for (DWORD i = 0; i < dwNum; i ++)
		{
			FK_FAVOURITE *pFavourite = new FK_FAVOURITE;

			char sTmp[128] = {0};
			
			// Read in the name...

			fscanf( fp, "%s", szTag );
			fseek( fp, 1, SEEK_CUR );
			fgets(sTmp, ARRAY_LEN(sTmp), fp );
			int iStrSize = strlen( sTmp );
			if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
				sTmp[iStrSize - 1] = NULL;
			pFavourite->m_sName = sTmp;

			// Read in the ref structure
			
			fscanf( fp, "%s", szTag );
			fseek( fp, 1, SEEK_CUR );
			fgets(pFavourite->m_ref.m_sName, ARRAY_LEN(pFavourite->m_ref.m_sName), fp );
			iStrSize = strlen( pFavourite->m_ref.m_sName );
			if( (iStrSize > 0) && (pFavourite->m_ref.m_sName[iStrSize - 1] == '\n') )
				pFavourite->m_ref.m_sName[iStrSize - 1] = NULL;
			 			
			fscanf( fp, "%s %lu", szTag, &pFavourite->m_ref.m_dwType );

			// Read in the min scale

			fscanf( fp, "%s %f", szTag, &pFavourite->m_minScale );

			// Read in the max scale

			fscanf( fp, "%s %f", szTag, &pFavourite->m_maxScale );

			// Read in the length

			fscanf( fp, "%s %lu", szTag, &pFavourite->m_tmLength );

			// Read in the number of properties

			DWORD dwNumProps;
			fscanf( fp, "%s %lu", szTag, &dwNumProps );

			for (DWORD k = 0; k < dwNumProps; k ++)
			{
				FX_PROP fxProp;
				ReadProp(&fxProp, fp, FORMAT_TEXT);

				if (!stricmp(fxProp.m_sName, "Mk"))
				{
					MOVEKEY mk;
					mk.m_tmKey = fxProp.m_data.m_fVec4[0];
					mk.m_pos.x = fxProp.m_data.m_fVec4[1];
					mk.m_pos.y = fxProp.m_data.m_fVec4[2];
					mk.m_pos.z = fxProp.m_data.m_fVec4[3];

					pFavourite->m_collMoveKeys.AddTail(mk);
				}
				else if (!stricmp(fxProp.m_sName, "Ck"))
				{
					// Colour key

					COLOURKEY ck;

					float ratio = 1.0f / 255.0f;

					DWORD dwCol = fxProp.m_data.m_clrKey.m_dwCol;
					
					ck.m_tmKey = fxProp.m_data.m_clrKey.m_tmKey;
					ck.m_red   = ratio * (float)(dwCol & 0x000000FF);
					ck.m_green = ratio * (float)((dwCol & 0x0000FF00) >> 8);
					ck.m_blue  = ratio * (float)((dwCol & 0x00FF0000) >> 16);
					ck.m_alpha = ratio * (float)((dwCol & 0xFF000000) >> 24);

					pFavourite->m_collColourKeys.AddTail(ck);
				}
				else if (!stricmp(fxProp.m_sName, "Sk"))
				{
					SCALEKEY sk;
					sk.m_tmKey = fxProp.m_data.m_fVec4[0];
					sk.m_scale = fxProp.m_data.m_fVec4[1];

					pFavourite->m_collScaleKeys.AddTail(sk);
				}
				else
				{
					pFavourite->m_collProps.AddTail(fxProp);
				}
			}
			
			m_collKeyFavourites.AddTail(pFavourite);

		}

		fclose(fp);
	}
	else
	{
		FILE *fp = OpenFile(sFilename, false, false);
		if( !fp ) return;

		DWORD dwNum;
		fread(&dwNum, sizeof(DWORD), 1, fp);

		for (DWORD i = 0; i < dwNum; i ++)
		{
			FK_FAVOURITE *pFavourite = new FK_FAVOURITE;

			char sTmp[256];
			memset(sTmp, 0, 256);
			
			// Read in the name

			int nLen;
			fread(&nLen, sizeof(int), 1, fp);
			fread(sTmp, nLen, 1, fp);
			
			pFavourite->m_sName = sTmp;

			// Read in the ref structure

			fread(&pFavourite->m_ref, sizeof(FX_REF), 1, fp);

			// Read in the min scale

			fread(&pFavourite->m_minScale, sizeof(float), 1, fp);

			// Read in the max scale

			fread(&pFavourite->m_maxScale, sizeof(float), 1, fp);

			// Read in the length

			fread(&pFavourite->m_tmLength, sizeof(float), 1, fp);
			
			// Read in the props

			DWORD dwSize;
			fread(&dwSize, sizeof(DWORD), 1, fp);

			for (DWORD j = 0; j < dwSize; j ++)
			{
				FX_PROP fxProp;
				fread(&fxProp, sizeof(FX_PROP), 1, fp);

				pFavourite->m_collProps.AddTail(fxProp);
			}

			// Read in the colour keys

			pFavourite->m_collColourKeys.ReadList(fp);

			// Read in the scale keys

			pFavourite->m_collScaleKeys.ReadList(fp);

			// Read in the motion keys

			pFavourite->m_collMoveKeys.ReadList(fp);

			m_collKeyFavourites.AddTail(pFavourite);
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileImportColourFavourites()
//
//   PURPOSE  : Imports colour favourites from somewhere
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileImportColourFavourites() 
{
	char sDir[256];

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.cfv", "*.cfv", OFN_OVERWRITEPROMPT, "Colour Favourites Files (*.cfv)|*.cfv||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		AddColourFavourites(dlg.GetPathName().GetBuffer(strlen(dlg.GetPathName())));
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileImportScaleFavourites()
//
//   PURPOSE  : Imports scale favourites
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileImportScaleFavourties() 
{
	char sDir[256];

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.sfv", "*.sfv", OFN_OVERWRITEPROMPT, "Scale Favourites Files (*.sfv)|*.sfv||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		AddScaleFavourites(dlg.GetPathName().GetBuffer(strlen(dlg.GetPathName())));
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileImportKeyFavourites()
//
//   PURPOSE  : Imports key favourites
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileImportKeyFavourites() 
{
	char sDir[256];

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.kfv", "*.kfv", OFN_OVERWRITEPROMPT, "Key Favourites Files (*.kfv|*.kfv||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		AddKeyFavourites(dlg.GetPathName().GetBuffer(strlen(dlg.GetPathName())));
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : OnFileImportMoveFavourites()
//
//   PURPOSE  : Imports motion favourites
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileImportMoveFavourites() 
{
	char sDir[256];

	GetCurrentDirectory(256, sDir);
	CFileDialog dlg(TRUE, "*.mfv", "*.mfv", OFN_OVERWRITEPROMPT, "Motion Favourites Files (*.mfv|*.mfv||");
	dlg.m_ofn.lpstrInitialDir = sDir;

	if (dlg.DoModal() == IDOK)
	{
		AddMoveFavourites(dlg.GetPathName().GetBuffer(strlen(dlg.GetPathName())));
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ReloadResourceFile()
//
//   PURPOSE  : Reloads the resource file
//
//------------------------------------------------------------------

void CSpellEdApp::ReloadResourceFile()
{
//	int ret = AfxMessageBox("Reload resource file ?", MB_YESNO | MB_ICONEXCLAMATION);
//	if (ret == IDNO) return;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	m_rezMgr.Close();

	CString sRezFile = GetProfileString("", "ResourceFileLocation", "");

	pFrame->GetStatusBar()->SetPaneText(0, "Reloading resource file");
	if (!m_rezMgr.Open(sRezFile))
	{		
		CString sTxt;
		sTxt.Format("Couldn't open [%s]", g_sRezFileName);
		AfxMessageBox(sTxt);
	}
	pFrame->GetStatusBar()->SetPaneText(0, "Done reloading resource file");
}

//------------------------------------------------------------------
//
//   FUNCTION : GetCastAnimTime()
//
//   PURPOSE  : Returns the length of a cast animation
//
//------------------------------------------------------------------

int CSpellEdApp::GetCastAnimTime(int nCastType)
{
	int msTime = 0;

	switch (nCastType)
	{
		case SAL_FAST :
		{
			msTime = m_nFastCastSpeed;
		}
		break;

		case SAL_MEDIUM :
		{
			msTime = m_nMediumCastSpeed;
		}
		break;

		case SAL_SLOW :
		{
			msTime = m_nSlowCastSpeed;
		}
		break;
	}

	return msTime;
}

void CSpellEdApp::OnFileReloadresourcefile() 
{
	ReloadResourceFile();
}

void CSpellEdApp::OnFileEditgameinfo() 
{
	CString sRez;
	CString sApp;
	CString sCmdLine;
	CString sDll;

	sRez	 = GetProfileString("", "ResourceFileLocation", "");
	sApp	 = GetProfileString("", "App", "");
	sCmdLine = GetProfileString("", "CommandLine", "");
	sDll	 = GetProfileString("", "DllLocation", "");

	CGameInfoDlg dlg(sDll, sRez, sApp, sCmdLine);

	if (dlg.DoModal())
	{
		sRez	 = dlg.m_sRez;
		sApp	 = dlg.m_sApp;
		sCmdLine = dlg.m_sCmdLine;
		sDll	 = dlg.m_sDll;

		WriteProfileString("", "ResourceFileLocation", sRez);
		WriteProfileString("", "App", sApp);
		WriteProfileString("", "CommandLine", sCmdLine);
		WriteProfileString("", "DllLocation", sDll);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileFindResource()
//
//   PURPOSE  : Locate a named resource
//
//------------------------------------------------------------------

void CSpellEdApp::OnFileFindResource()
{
	CStringDlg dlg("Enter resource path and name");

	if (dlg.DoModal() == IDOK)
	{
		CLinkList<SPELLNAME> collRez;

		m_spellMgr.FindResource(dlg.m_sText, &collRez);

		if (collRez.GetSize())
		{
			CResourceLocator rlDlg(dlg.m_sText, &collRez);

			rlDlg.DoModal();
		}
		else
		{
			AfxMessageBox("Unable to locate that resource anywhere.");
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::OnFormatText
//
//  PURPOSE:	Sets the save mode to Text format
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::OnFormatText()
{
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	
	pMenu->CheckMenuItem( ID_FORMAT_TEXT, MF_CHECKED );
	pMenu->CheckMenuItem( ID_FORMAT_BINARY, MF_UNCHECKED );

	m_wFormat = FORMAT_TEXT;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpellEdApp::OnFormatBinary
//
//  PURPOSE:	Sets the save mode to binary format
//
// ----------------------------------------------------------------------- //

void CSpellEdApp::OnFormatBinary()
{
	CMenu *pMenu = AfxGetMainWnd()->GetMenu();
	
	pMenu->CheckMenuItem( ID_FORMAT_TEXT, MF_UNCHECKED );
	pMenu->CheckMenuItem( ID_FORMAT_BINARY, MF_CHECKED );

	m_wFormat = FORMAT_BIN;
}