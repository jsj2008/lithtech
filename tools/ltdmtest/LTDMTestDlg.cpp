// LTDMTestDlg.cpp : implementation file
//

#include "stdafx.h"
#ifdef NOLITHTECH
#include "lith.h"
#include "lithtmpl.h"
#include "ltbasedefs.h"
#endif
#include "LTDMTest.h"
#include "LTDMTestDlg.h"
#define strict
#include <objbase.h>
#include <initguid.h>
#include <conio.h>
#include <direct.h>
#include <dmusicc.h>
#include <dmusici.h>
#include "ltdirectmusic_impl.h"
#include "ltdirectmusiccontrolfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// macros
#define MULTI_TO_WIDE( x,y )	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH );

// defines
#define LTDMMaxStr 2048
#define MAXMORESEGMENTS 2048

// Globals
BOOL g_bInitialized = FALSE;

BOOL g_bCurrentlyPlaying = FALSE;

int g_nCurIntensity = 1;

CControlFileMgrRezFile* g_pControlFile;

#define MINDEBUGOUTPUTLEVEL 0
#define MAXDEBUGOUTPUTLEVEL 6
#define DEFAULTDEBUGOUTPUTLEVEL 4

signed int g_CV_LTDMConsoleOutput = DEFAULTDEBUGOUTPUTLEVEL;

CRichEditCtrl* g_pRichEditDebugOutput = NULL;

void DebugConsoleOutput(const char* msg, BYTE nRed = 255, BYTE nBlue = 255, BYTE nGreen = 255)
{
	if (g_pRichEditDebugOutput != NULL)
	{
		// set to bottom of text
		g_pRichEditDebugOutput->SetSel(-1,-1);

		// set the color of the text to output
		CHARFORMAT cf;
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = 0;
		cf.crTextColor = (nBlue << 16) | (nGreen << 8) | nRed;
		g_pRichEditDebugOutput->SetSelectionCharFormat(cf);

		// insert text
		g_pRichEditDebugOutput->ReplaceSel(msg);

		// move current view to new next
		int nMinPos;
		int nMaxPos;
		g_pRichEditDebugOutput->GetScrollRange( SB_VERT, &nMinPos, &nMaxPos );
		g_pRichEditDebugOutput->SetScrollPos( SB_VERT, nMaxPos, TRUE );
	}
}

// convert an enact type to a string for display (sName must be large enough no checking is done!)
void EnactTypeToString(LTDMEnactTypes nType, char* sName)
{
	switch (nType)
	{
		case LTDMEnactInvalid :
		{
			strcpy(sName, "Invalid");
			break;
		}
		case LTDMEnactDefault :
		{
			strcpy(sName, "Default");
			break;
		}
		case LTDMEnactImmediately :
		{
			strcpy(sName, "Immediatly");
			break;
		}
		case LTDMEnactNextBeat :
		{
			strcpy(sName, "Beat");
			break;
		}
		case LTDMEnactNextMeasure :
		{
			strcpy(sName, "Measure");
			break;
		}
		case LTDMEnactNextGrid :
		{
			strcpy(sName, "Grid");
			break;
		}
		case LTDMEnactNextSegment :
		{
			strcpy(sName, "Segment");
			break;
		}
		case LTDMEnactNextMarker :
		{
			strcpy(sName, "Marker");

		}
		default :
		{
			strcpy(sName, "");
			break;
		}
	}
}


// convert a string to an enact type
LTDMEnactTypes StringToEnactType(const char* sName)
{
	if (sName == NULL) return LTDMEnactInvalid;
	if (stricmp(sName, "Invalid") == 0) return LTDMEnactInvalid;
	if (stricmp(sName, "Default") == 0) return LTDMEnactDefault;
	if (stricmp(sName, "Immediatly") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "NextBeat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "NextMeasure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "NextGrid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "NextSegment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "NextMarker") == 0) return LTDMEnactNextMarker;
	if (stricmp(sName, "Beat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "Measure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "Grid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "Segment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "Marker") == 0) return LTDMEnactNextMarker;
	return LTDMEnactInvalid;
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

/////////////////////////////////////////////////////////////////////////////
// CLTDMTestDlg dialog

CLTDMTestDlg::CLTDMTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLTDMTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLTDMTestDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLTDMTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLTDMTestDlg)
	DDX_Control(pDX, IDC_DEBUGOUTPUTLEVEL, m_comboDebugOutputLevel);
	DDX_Control(pDX, IDC_DEBUGOUTPUT, m_richEditDebugOutput);
	DDX_Control(pDX, IDC_Intensity, m_comboIntensity);
	DDX_Control(pDX, IDC_MotifName, m_comboMotifName);
	DDX_Control(pDX, IDC_MotifEnact, m_comboMotifEnact);
	DDX_Control(pDX, IDC_SecondaryEnact, m_comboSecondaryEnact);
	DDX_Control(pDX, IDC_SecondarySegment, m_comboSecondarySegment);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLTDMTestDlg, CDialog)
	//{{AFX_MSG_MAP(CLTDMTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnInitLevel)
	ON_BN_CLICKED(IDStopMusic, OnStopMusic)
	ON_BN_CLICKED(IDPlayMusic, OnPlayMusic)
	ON_BN_CLICKED(ID_CANCEL, OnExit)
	ON_CBN_EDITCHANGE(IDC_Intensity, OnChangeIntensity)
	ON_EN_CHANGE(IDC_Volume, OnVolume)
	ON_BN_CLICKED(IDPlaySecondary, OnPlaySecondary)
	ON_BN_CLICKED(IDStopSecondary, OnStopSecondary)
	ON_BN_CLICKED(IDPlayMotif, OnPlayMotif)
	ON_BN_CLICKED(IDStopMotif, OnStopMotif)
	ON_CBN_EDITCHANGE(IDC_DEBUGOUTPUTLEVEL, OnDebugOutputLevel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDInitLevel, OnInitLevel)
	ON_BN_CLICKED(IDTermLevel, OnTermLevel)
	ON_CBN_SELCHANGE(IDC_Intensity, OnChangeIntensity)
	ON_CBN_SELCHANGE(IDC_DEBUGOUTPUTLEVEL, OnDebugOutputLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLTDMTestDlg message handlers

BOOL CLTDMTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CWinApp* pApp = AfxGetApp();
	if (!pApp) return FALSE;

	// set up rez file initial value
	CString sRezFile = pApp->GetProfileString("Defaults", "RezFile", "");
	SetDlgItemText(IDC_REZFILE, sRezFile);

	// set up working directory initial value
	CString sWorkingDirectory = pApp->GetProfileString("Defaults", "WorkingDirectory", "");
	SetDlgItemText(IDC_WorkingDirectory, sWorkingDirectory);

	// set up control file name initial value
	CString sControlFileName = pApp->GetProfileString("Defaults", "ControlFileName", "");
	SetDlgItemText(IDC_ControlFileName, sControlFileName);

	// set up initial window enables for controls
	GetDlgItem(IDC_REZFILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_WorkingDirectory)->EnableWindow(TRUE);
	GetDlgItem(IDC_ControlFileName)->EnableWindow(TRUE);
	GetDlgItem(IDInitLevel)->EnableWindow(TRUE);
	GetDlgItem(IDTermLevel)->EnableWindow(FALSE);
	GetDlgItem(IDPlayMusic)->EnableWindow(FALSE);
	GetDlgItem(IDStopMusic)->EnableWindow(FALSE);
	GetDlgItem(IDC_Intensity)->EnableWindow(FALSE);
	GetDlgItem(IDC_Volume)->EnableWindow(FALSE);
	GetDlgItem(IDC_SecondarySegment)->EnableWindow(FALSE);
	GetDlgItem(IDC_SecondaryEnact)->EnableWindow(FALSE);
	GetDlgItem(IDC_MotifName)->EnableWindow(FALSE);
	GetDlgItem(IDC_MotifEnact)->EnableWindow(FALSE);
	GetDlgItem(IDPlaySecondary)->EnableWindow(FALSE);
	GetDlgItem(IDStopSecondary)->EnableWindow(FALSE);
	GetDlgItem(IDPlayMotif)->EnableWindow(FALSE);
	GetDlgItem(IDStopMotif)->EnableWindow(FALSE);

	// set up the debug output level combo box
	{
		// clear out the combo string list for intensity
		m_comboDebugOutputLevel.ResetContent();

		// set up the list values in the combo box for intensity
		char sStr[256] = "1";
		for (int i = MINDEBUGOUTPUTLEVEL; i <= MAXDEBUGOUTPUTLEVEL; i++)
		{
			sprintf(sStr,"%i",i);
			int nIndex = m_comboDebugOutputLevel.AddString(sStr);
		}

		// set up control file name initial value
		int nDefaultDebugOutputLevel = pApp->GetProfileInt("Defaults", "DebugOutputLevel", DEFAULTDEBUGOUTPUTLEVEL);

		// set the current initial intensity to be displayed
		SetDlgItemInt(IDC_DEBUGOUTPUTLEVEL, nDefaultDebugOutputLevel);
		char sDebugOutputLevel[34];
		itoa(nDefaultDebugOutputLevel, sDebugOutputLevel, 10);
		m_comboDebugOutputLevel.SelectString(-1, sDebugOutputLevel);
		g_CV_LTDMConsoleOutput = nDefaultDebugOutputLevel;
	}

	// set up the global pointer to the debug output control
	g_pRichEditDebugOutput = &m_richEditDebugOutput;

	// Initialize the DirectMusic Mgr
	if (m_LTDMMgr.Init() != LT_OK) AfxMessageBox("ERROR, Unable to Initialize DirectMusicMgr.");

	// we are not in a level yet
	m_bInLevel = FALSE;

	// we have initialized the directmusic mgr
	m_bInitialized = TRUE;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLTDMTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLTDMTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLTDMTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


// Initialize the level
void CLTDMTestDlg::OnInitLevel() 
{
	// get the working directory to pass to InitLevel
	CString sRezFile;
	GetDlgItemText(IDC_REZFILE, sRezFile);

	// get the working directory to pass to InitLevel
	CString sWorkingDirectory;
	GetDlgItemText(IDC_WorkingDirectory, sWorkingDirectory);

	// get the control file to load from to pass to InitLevel
	CString sControlFileName;
	GetDlgItemText(IDC_ControlFileName, sControlFileName);

	// new control file name after modifying it
	CString sNewControlFileName;

	// check if control file name contains no path
	if ((strchr(sControlFileName,'\\') == NULL) && (strchr(sControlFileName,'/') == NULL) &&
		(strchr(sControlFileName,':') == NULL))
	{
		// copy working directory to string
		sNewControlFileName = sWorkingDirectory;

		// see if we need to append a backslash to directory
		if (strlen(sWorkingDirectory) > 0)
		{
			int nLastPos = strlen(sWorkingDirectory)-1;
			if ((sWorkingDirectory[nLastPos] != '\\') && 
				(sWorkingDirectory[nLastPos] != '/') &&
				(sWorkingDirectory[nLastPos] != ':'))
			{
				// append backslash
				sNewControlFileName += "\\";
			}
		}

		// append the file name
		sNewControlFileName += sControlFileName;
	}
	// control file name has a path so just use it
	else 
	{
		// set new control file name
		sNewControlFileName = sControlFileName;
	}

	// set the rez file to load from
	m_LTDMMgr.SetRezFile(sRezFile);

	// Initialize the level
	if (m_LTDMMgr.InitLevel(sWorkingDirectory, sControlFileName) == LT_OK)
	{
		g_pControlFile = new CControlFileMgrRezFile(sRezFile);

		// get our own control file information for drop down boxes
		if (g_pControlFile->Init(sNewControlFileName))
		{
			// set control enable / disable status
			GetDlgItem(IDC_REZFILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_WorkingDirectory)->EnableWindow(FALSE);
			GetDlgItem(IDC_ControlFileName)->EnableWindow(FALSE);
			GetDlgItem(IDInitLevel)->EnableWindow(FALSE);
			GetDlgItem(IDTermLevel)->EnableWindow(TRUE);
			GetDlgItem(IDPlayMusic)->EnableWindow(TRUE);
			GetDlgItem(IDStopMusic)->EnableWindow(TRUE);
			GetDlgItem(IDC_Intensity)->EnableWindow(TRUE);
			GetDlgItem(IDC_Volume)->EnableWindow(TRUE);
			GetDlgItem(IDC_SecondarySegment)->EnableWindow(TRUE);
			GetDlgItem(IDC_SecondaryEnact)->EnableWindow(TRUE);
			GetDlgItem(IDC_MotifName)->EnableWindow(TRUE);
			GetDlgItem(IDC_MotifEnact)->EnableWindow(TRUE);
			GetDlgItem(IDPlaySecondary)->EnableWindow(TRUE);
			GetDlgItem(IDStopSecondary)->EnableWindow(TRUE);
			GetDlgItem(IDPlayMotif)->EnableWindow(TRUE);
			GetDlgItem(IDStopMotif)->EnableWindow(TRUE);

			// set up the intensity combo box
			{
				// clear out the combo string list for intensity
				m_comboIntensity.ResetContent();

				// set up the list values in the combo box for intensity
				char sStr[256] = "1";
				for (int i = 1; i <= m_LTDMMgr.GetNumIntensities(); i++)
				{
					sprintf(sStr,"%i",i);
					int nIndex = m_comboIntensity.AddString(sStr);
				}

				// set the current initial intensity to be displayed
				SetDlgItemInt(IDC_Intensity, m_LTDMMgr.GetInitialIntensity());
				char sFindStr[34];
				itoa(m_LTDMMgr.GetInitialIntensity(), sFindStr, 10);
				m_comboIntensity.SelectString(-1, sFindStr);
			}

			// set up the secondary segment name combo box
			{
				// clear out the combo string list
				m_comboSecondarySegment.ResetContent();

				// go through all secondary segment commands and add them to combo box string list
				CControlFileKey* pKey = g_pControlFile->GetKey(NULL, "SECONDARYSEGMENT");
				CControlFileWord* pWord;

				// loop through all SECONDARYSEGMENT keys
				while (pKey != NULL)
				{
					// get the first value
					pWord = pKey->GetFirstWord();

					// make sure word is not null
					if (pWord != NULL)
					{
						// add the string
						m_comboSecondarySegment.AddString(pWord->GetVal());
					}

					// get the next key
					pKey = pKey->NextWithSameName();
				}
			}

			// set up the secondary segment enact time combo box
			{
				// clear out the combo string list
				m_comboSecondaryEnact.ResetContent();

				// add enact types to list box
				m_comboSecondaryEnact.AddString("Default");
				m_comboSecondaryEnact.AddString("Immediatly");
				m_comboSecondaryEnact.AddString("Beat");
				m_comboSecondaryEnact.AddString("Measure");
				m_comboSecondaryEnact.AddString("Segment");
				m_comboSecondaryEnact.AddString("Marker");


				// set the current initial value to be displayed
				m_comboSecondaryEnact.SetCurSel(3);
			}

			// set up the motif name combo box
			{
				// clear out the combo string list
				m_comboMotifName.ResetContent();

				// go through all motif commands and add them to combo box string list
				CControlFileKey* pKey = g_pControlFile->GetKey(NULL, "MOTIF");
				CControlFileWord* pWord;

				// loop through all SECONDARYSEGMENT keys
				while (pKey != NULL)
				{
					// get the first word
					pWord = pKey->GetFirstWord();
					if (pWord != NULL)
					{
						// get the next word
						pWord = pWord->Next();
						if (pWord != NULL)
						{
							// add the string
							m_comboMotifName.AddString(pWord->GetVal());
						}
					}

					// get the next key
					pKey = pKey->NextWithSameName();
				}
			}

			// set up the motif enact time combo box
			{
				// clear out the combo string list
				m_comboMotifEnact.ResetContent();

				// add enact types to list box
				m_comboMotifEnact.AddString("Default");
				m_comboMotifEnact.AddString("Immediatly");
				m_comboMotifEnact.AddString("Beat");
				m_comboMotifEnact.AddString("Measure");
				m_comboMotifEnact.AddString("Segment");
				m_comboMotifEnact.AddString("Marker");

				// set the current initial value to be displayed
				m_comboMotifEnact.SetCurSel(3);
			}

		}

		// if we failed to open our own control file we must fail initialization
		// this shouldn't happen, but we check just in case
		else
		{
			AfxMessageBox("ERROR, Failed To Open Control File.");
			m_LTDMMgr.TermLevel();
		}
	}
	else
	{
		AfxMessageBox("ERROR, Failed To Initialize Level.");
	}

	// we are now in a level
	m_bInLevel = TRUE;
}


// Terminate the level
void CLTDMTestDlg::OnTermLevel() 
{
	if (m_bInitialized)
	{

		// terminate the level
		m_LTDMMgr.TermLevel();

		// terminate the control file
		g_pControlFile->Term();
		delete g_pControlFile;
		g_pControlFile = NULL;

		// set control enable / disable status
		GetDlgItem(IDC_REZFILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_WorkingDirectory)->EnableWindow(TRUE);
		GetDlgItem(IDC_ControlFileName)->EnableWindow(TRUE);
		GetDlgItem(IDInitLevel)->EnableWindow(TRUE);
		GetDlgItem(IDTermLevel)->EnableWindow(FALSE);
		GetDlgItem(IDPlayMusic)->EnableWindow(FALSE);
		GetDlgItem(IDStopMusic)->EnableWindow(FALSE);
		GetDlgItem(IDC_Intensity)->EnableWindow(FALSE);
		GetDlgItem(IDC_Volume)->EnableWindow(FALSE);
		GetDlgItem(IDC_SecondarySegment)->EnableWindow(FALSE);
		GetDlgItem(IDC_SecondaryEnact)->EnableWindow(FALSE);
		GetDlgItem(IDC_MotifName)->EnableWindow(FALSE);
		GetDlgItem(IDC_MotifEnact)->EnableWindow(FALSE);
		GetDlgItem(IDPlaySecondary)->EnableWindow(FALSE);
		GetDlgItem(IDStopSecondary)->EnableWindow(FALSE);
		GetDlgItem(IDPlayMotif)->EnableWindow(FALSE);
		GetDlgItem(IDStopMotif)->EnableWindow(FALSE);

		// clear out the combo string list
		m_comboIntensity.ResetContent();

		// we are no longer in a level
		m_bInLevel = FALSE;
	}
}

// Begin playing music
void CLTDMTestDlg::OnPlayMusic() 
{
	m_LTDMMgr.Play();
}

// Stop playing music
void CLTDMTestDlg::OnStopMusic() 
{
	m_LTDMMgr.Stop();
}

// exit the app
void CLTDMTestDlg::OnExit() 
{
	TermAll();

	CDialog::OnOK();
}

// change the intensity of the music
void CLTDMTestDlg::OnChangeIntensity() 
{
	// get the new intensity that user has requested
	CString sIntensity;
	GetDlgItemText(IDC_Intensity, sIntensity);
	int nNewIntensity = atoi(sIntensity);

	// make sure it is valid
	if ((nNewIntensity < 1) || (nNewIntensity > m_LTDMMgr.GetNumIntensities()))
	{
		// set intensity back to current intensity
		SetDlgItemInt(IDC_Intensity, m_LTDMMgr.GetCurIntensity());

		// exit function
		return;
	}

	// actually change the intensity in the DirectMusic Mgr
	if (m_LTDMMgr.ChangeIntensity(nNewIntensity) == LT_OK)
	{
		// if we failed set intensity back to current intensity
		char sFindStr[34];
		itoa(m_LTDMMgr.GetCurIntensity(), sFindStr, 10);
		m_comboIntensity.SelectString(-1, sFindStr);
	}
}


// change volume
void CLTDMTestDlg::OnVolume() 
{
	// get the new volume value the user specified
	int nNewVolume = GetDlgItemInt(IDC_Volume);

	// set the volume
	m_LTDMMgr.SetVolume(nNewVolume);
	
}

// play secondary segment
void CLTDMTestDlg::OnPlaySecondary() 
{
	// get the name for the secondary segment to play
	CString sName;
	GetDlgItemText(IDC_SecondarySegment, sName);
	
	// get the enact time for the secondary segment to play
	CString sEnact;
	GetDlgItemText(IDC_SecondaryEnact, sEnact);
	
	// play the secondary segment
	m_LTDMMgr.PlaySecondary(sName, StringToEnactType(sEnact));
}

// stop secondary segment 
void CLTDMTestDlg::OnStopSecondary() 
{
	// get the name for the secondary segment to play
	CString sName;
	GetDlgItemText(IDC_SecondarySegment, sName);
	
	// play the secondary segment
	m_LTDMMgr.StopSecondary(sName, LTDMEnactNextBeat);
}

void CLTDMTestDlg::OnPlayMotif() 
{
	// get the name for the secondary segment to play
	CString sName;
	GetDlgItemText(IDC_MotifName, sName);
	
	// get the enact time for the secondary segment to play
	CString sEnact;
	GetDlgItemText(IDC_MotifEnact, sEnact);
	
	// play the secondary segment
	m_LTDMMgr.PlayMotif(sName, StringToEnactType(sEnact));
}

void CLTDMTestDlg::OnStopMotif() 
{
	// get the name for the secondary segment to play
	CString sName;
	GetDlgItemText(IDC_MotifName, sName);
	
	// play the secondary segment
	m_LTDMMgr.StopMotif(sName, LTDMEnactNextBeat);
}

void CLTDMTestDlg::OnDebugOutputLevel() 
{
	// get the new intensity that user has requested
	CString sDebugOutputLevel;
	GetDlgItemText(IDC_DEBUGOUTPUTLEVEL, sDebugOutputLevel);
	int nDebugOutputLevel = atoi(sDebugOutputLevel);

	if ((nDebugOutputLevel >= MINDEBUGOUTPUTLEVEL) && (nDebugOutputLevel <= MAXDEBUGOUTPUTLEVEL))
	{
		g_CV_LTDMConsoleOutput = nDebugOutputLevel;
	}
}


void CLTDMTestDlg::OnClose() 
{
	TermAll();

	CDialog::OnClose();
}


void CLTDMTestDlg::TermAll()
{
	if (m_bInitialized)
	{
		// term level if we are in a leve
		if (m_bInLevel) OnTermLevel();

		m_LTDMMgr.Term();

		CWinApp* pApp = AfxGetApp();
		if (!pApp) return;

		CString sRezFile;
		GetDlgItemText(IDC_REZFILE, sRezFile);
		pApp->WriteProfileString("Defaults", "RezFile", sRezFile);

		CString sWorkingDirectory;
		GetDlgItemText(IDC_WorkingDirectory, sWorkingDirectory);
		pApp->WriteProfileString("Defaults", "WorkingDirectory", sWorkingDirectory);

		CString sControlFileName;
		GetDlgItemText(IDC_ControlFileName, sControlFileName);
		pApp->WriteProfileString("Defaults", "ControlFileName", sControlFileName);
	
		pApp->WriteProfileInt("Defaults", "DebugOutputLevel", g_CV_LTDMConsoleOutput);

		m_bInitialized = FALSE;
	}
}
