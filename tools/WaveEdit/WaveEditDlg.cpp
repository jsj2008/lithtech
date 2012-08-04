// WaveEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>
#include "bdefs.h"
#include "wave.h"
#include "waveedit.h"
#include "waveeditdlg.h"
#include "sysstreamsim.h"
#include "tdguard.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const char s_szRegistrySection[] = "WaveEdit";
static const char s_szRegistryKeyInitDir[] = "InitDir";


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CFileDialog
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

CAboutDlg::CAboutDlg() : CFileDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CFileDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveEditDlg dialog

CWaveEditDlg::CWaveEditDlg(CWnd* pParent /*=NULL*/)
	: CFileDialog( TRUE )
{
	//{{AFX_DATA_INIT(CWaveEditDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_sInitDir = AfxGetApp()->GetProfileString( s_szRegistrySection, s_szRegistryKeyInitDir );

	m_nPlayMode = 2;
	m_bSelectionValid = FALSE;

	m_ofn.lpstrTitle = "WaveEdit";
	m_ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLETEMPLATE | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	m_szFile[0] = 0;
	m_ofn.lpstrFile = m_szFile;
	m_ofn.nMaxFile = sizeof( m_szFile ) - 1;
	m_ofn.lpstrFilter = "Wave files (*.WAV)\0*.WAV\0All files (*.*)\0*.*\0";
	m_ofn.nFilterIndex = 1;
	m_ofn.lpTemplateName = MAKEINTRESOURCE( IDD_WAVEEDIT_DIALOG );
	m_ofn.hInstance = AfxGetInstanceHandle( );
	m_ofn.lpstrInitialDir = m_sInitDir;
}

CWaveEditDlg::~CWaveEditDlg( )
{
	// Stop any currently playing sounds
	PlaySound(NULL, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT | SND_PURGE);
	AfxGetApp()->WriteProfileString( s_szRegistrySection, s_szRegistryKeyInitDir, m_sInitDir );
}

void CWaveEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaveEditDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWaveEditDlg, CFileDialog)
	//{{AFX_MSG_MAP(CWaveEditDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_WHILEPLAYING, OnChange)
	ON_BN_CLICKED(IDC_ONLOAD, OnChange)
	ON_BN_CLICKED(IDC_ATSTART, OnChange)
	ON_BN_CLICKED(IDC_STREAM, OnChange)
	ON_BN_CLICKED(IDC_DMUSICSTREAM, OnChange)
	ON_EN_SETFOCUS(IDC_PITCH, OnChange)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveEditDlg message handlers

BOOL CWaveEditDlg::OnInitDialog()
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	CWnd *pParent;

	CFileDialog::OnInitDialog();

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
	pParent = GetParent( );
	if( pParent )
	{
		pParent->SetIcon(m_hIcon, TRUE);			// Set big icon
		pParent->SetIcon(m_hIcon, FALSE);		// Set small icon

		CommDlg_OpenSave_SetControlText( pParent->m_hWnd, IDOK, "Play" );
		CommDlg_OpenSave_SetControlText( pParent->m_hWnd, IDCANCEL, "Close" );
	}

//	EnableApply( FALSE );
	UpdateUI( );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWaveEditDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CFileDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWaveEditDlg::OnPaint() 
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
		CFileDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWaveEditDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CWaveEditDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFileDialog::OnShowWindow(bShow, nStatus);
	

}

BOOL CWaveEditDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	LPOFNOTIFY lpOfNotify;
	CString sDir, sFilePath;
	ConParse parse;
	int i;
	CFileFind fileFind;
	BOOL bPlayed, bFound;

	lpOfNotify = ( LPOFNOTIFY )lParam;

	if( lpOfNotify->hdr.code == CDN_FILEOK )
	{
		SetWindowLong( m_hWnd, DWL_MSGRESULT, 1 );
		*pResult = 1;
#if 0
		CommDlg_OpenSave_SetControlText( GetParent( )->m_hWnd, IDOK, "Play" );
#endif 
		if( ParseFiles( sDir, parse ))
		{
			bPlayed = FALSE;
			for( i = 0; i < parse.m_nArgs && !bPlayed ; i++ )
			{
				sFilePath = sDir + parse.m_Args[i];
				bFound = fileFind.FindFile( sFilePath );
				while( bFound )
				{
					bFound = fileFind.FindNextFile( );

					if( !fileFind.IsDirectory( ))
					{

						PlaySound(fileFind.GetFilePath(), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
#if 0
						ShellExecute( AfxGetMainWnd( )->GetSafeHwnd( ), "open", 
							fileFind.GetFilePath( ), NULL, "", SW_SHOW );
#endif
						bPlayed = TRUE;
						break;
					}
				}
			}
		}

		return TRUE;
	}
	else if( lpOfNotify->hdr.code == CDN_SELCHANGE )
	{
		OnSelection( );
	}
    else if( lpOfNotify->hdr.code == CDN_FOLDERCHANGE )
	{
		CommDlg_OpenSave_GetFolderPath( ::GetParent( m_hWnd ), m_sInitDir.GetBuffer( _MAX_PATH ), _MAX_PATH );
	}
	
	return CFileDialog::OnNotify(wParam, lParam, pResult);
}

void CWaveEditDlg::OnApply( )
{
	CWaveHeader waveHeader;
	char szPitch[32];
	CString sDir;
	ConParse parse;
	BOOL bRecurse;

	if( !ParseFiles( sDir, parse ))
	{
//		EnableApply( FALSE );
		return;
	}

	// Setup the waveheader.
	memset( &waveHeader, 0, sizeof( CWaveHeader ));
	if(	SendDlgItemMessage( IDC_WHILEPLAYING, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		waveHeader.m_lith.m_dwSoundBufferFlags &= ~( SOUNDBUFFERFLAG_DECOMPRESSONLOAD | SOUNDBUFFERFLAG_DECOMPRESSATSTART );
	else if( SendDlgItemMessage( IDC_ONLOAD, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		waveHeader.m_lith.m_dwSoundBufferFlags |= SOUNDBUFFERFLAG_DECOMPRESSONLOAD;
	else if( SendDlgItemMessage( IDC_ATSTART, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		waveHeader.m_lith.m_dwSoundBufferFlags |= SOUNDBUFFERFLAG_DECOMPRESSATSTART;
	else if( SendDlgItemMessage( IDC_STREAM, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		waveHeader.m_lith.m_dwSoundBufferFlags |= SOUNDBUFFERFLAG_STREAM;
	else if ( SendDlgItemMessage( IDC_DMUSICSTREAM, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
	{
		// set up parameters for DirectMusic streaming
		waveHeader.m_dmus.m_bStream = TRUE;
		// 1/2 second read ahead
		// this must be in 100 ns units (believe it or not)
		waveHeader.m_dmus.m_dmusWavHeader.rtReadAhead = 5000000;
		waveHeader.m_dmus.m_dmusWavHeader.dwFlags = DMUS_WAVEF_STREAMING;
	}
	SendDlgItemMessage( IDC_PITCH, WM_GETTEXT, sizeof( szPitch ) - 1, ( LPARAM )szPitch );
	waveHeader.m_lith.m_fPitchMod = ( float )atof( szPitch );
	if(	SendDlgItemMessage( IDC_RECURSEDIR, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		bRecurse = TRUE;
	else
		bRecurse = FALSE;

	DoDirectory( sDir, parse, waveHeader, bRecurse );

//	EnableApply( FALSE );
}

void CWaveEditDlg::DoDirectory( CString &sDir, ConParse &parse, CWaveHeader &waveHeader, BOOL bRecurse )
{
	int i;
	CFileFind fileFind;
	BOOL bFound;
	CString sFilePath;

	if( sDir[sDir.GetLength( ) - 1] != '\\' )
		sDir += "\\";

	for( i = 0; i < parse.m_nArgs; i++ )
	{
		sFilePath = sDir + parse.m_Args[i];
		bFound = fileFind.FindFile( sFilePath );
		while( bFound )
		{
			bFound = fileFind.FindNextFile( );

			if( !fileFind.IsDirectory( ))
				SetWaveInfo( fileFind.GetFilePath( ), waveHeader );
		}
	}

	if( bRecurse )
	{
		sFilePath = sDir + "*.*";
		bFound = fileFind.FindFile( sFilePath );
		while( bFound )
		{
			bFound = fileFind.FindNextFile( );

			if( fileFind.IsDirectory( ) && !fileFind.IsDots( ))
			{
				DoDirectory( fileFind.GetFilePath( ), parse, waveHeader, bRecurse );
			}
		}
	}
}

void CWaveEditDlg::OnSelection( )
{
	CWaveHeader waveHeader;
	CString sPitch;
	DStream *pStream;
	CFileFind fileFind;
	CString sDir, sFilePath, sFirstValidFile;
	ConParse parse;
	int i;
	BOOL bFound;

	if( !ParseFiles( sDir, parse ))
		return;

	m_nPlayMode = 2;
	m_sPitch = "";
	m_bSelectionValid = FALSE;

	// Make sure there's a valid selection in the group.
	for( i = 0; i < parse.m_nArgs && !m_bSelectionValid; i++ )
	{
		sFilePath = sDir + parse.m_Args[i];
		bFound = fileFind.FindFile( sFilePath );
		while( bFound )
		{
			bFound = fileFind.FindNextFile( );

			if( !fileFind.IsDirectory( ))
			{
				sFirstValidFile = fileFind.GetFilePath( );
				m_bSelectionValid = TRUE;
				break;
			}
		}
	}

	// If there's only one valid file, then display it's properties.
	if( m_bSelectionValid && parse.m_nArgs == 1 && sFirstValidFile.GetLength( ))
	{
		pStream = streamsim_OpenWinFileHandle(( char * )( const char * )sFirstValidFile, GENERIC_READ );
		if( pStream )
		{
			GetWaveInfo( *pStream, waveHeader );
			pStream->Release( );

			// Default to decompress while playing.
			m_nPlayMode = 2;

			if( waveHeader.m_lith.m_dwSoundBufferFlags & SOUNDBUFFERFLAG_DECOMPRESSONLOAD )
				m_nPlayMode = 3;
			else if( waveHeader.m_lith.m_dwSoundBufferFlags & SOUNDBUFFERFLAG_DECOMPRESSATSTART )
				m_nPlayMode = 4;
			else if( waveHeader.m_lith.m_dwSoundBufferFlags & SOUNDBUFFERFLAG_STREAM )
				m_nPlayMode = 1;
			else if( waveHeader.m_dmus.m_bStream )
				m_nPlayMode = 5;
			m_sPitch.Format( "%.3f", waveHeader.m_lith.m_fPitchMod );

		}
	}

	UpdateUI( );
}


void CWaveEditDlg::UpdateUI( )
{
	CString sPitch;

	// Use the fact that if m_nPlayMode is zero, no check will show.
	CheckRadioButton( IDC_STREAM, IDC_DMUSICSTREAM, IDC_STREAM + m_nPlayMode - 1 );

	SendDlgItemMessage( IDC_PITCH, WM_SETTEXT, 0, ( LPARAM )( LPCTSTR )m_sPitch );

}

BOOL CWaveEditDlg::SetWaveInfo( const char *pszPath, CWaveHeader &waveHeader )
{
	void *pData;
	SSBufStream *pInStream;
	DStream *pStream;
	DWORD dwSize;
	CString sError;

	if( !pszPath )
		return FALSE;

	// Read in the file.
	pStream = streamsim_OpenWinFileHandle( pszPath, GENERIC_READ );
	if( !pStream )
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError( ), 0, sError.GetBuffer( 128 ), 128, NULL ); 
		MessageBox( sError, "Error", MB_OK | MB_ICONERROR );
		return FALSE;
	}

	dwSize = pStream->GetLen( );
	pData = new BYTE[dwSize];
	if( pStream->Read( pData, dwSize ) != LT_OK )
	{
		sError.Format( "Error reading file %s", pszPath );
		MessageBox( sError, "Error", MB_OK | MB_ICONERROR );
		pStream->Release( );
		delete[] pData;
		return FALSE;
	}

	pStream->Release( );

	// Set the file data to a buf stream.
	pInStream = ( SSBufStream * )streamsim_MemStreamFromBuffer(( unsigned char * )pData, dwSize );
	if( !pInStream )
	{
		sError.Format( "Error reading file %s", pszPath );
		MessageBox( sError, "Error", MB_OK | MB_ICONERROR );
		delete[] pData;
		return FALSE;
	}

	// Open the output file.
	pStream = streamsim_OpenWinFileHandle( pszPath, GENERIC_WRITE );
	if( !pStream )
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError( ), 0, sError.GetBuffer( 128 ), 128, NULL ); 
		MessageBox( sError, "Error", MB_OK | MB_ICONERROR );
		pInStream->Release( );
		delete[] pData;
		return FALSE;
	}

	// Write it out.
	WriteWave( *pInStream, *pStream, waveHeader );

	pInStream->Release( );
	pStream->Release( );
	delete[] pData;

	return TRUE;
}

void CWaveEditDlg::OnChange() 
{
//	if( m_bSelectionValid )
//		EnableApply( TRUE );
}

void CWaveEditDlg::EnableApply( BOOL bTrue )
{
	CWnd *pWnd;

	pWnd = GetDlgItem( IDC_APPLY );
	if( pWnd )
	{
		pWnd->EnableWindow( bTrue );
	}
}


BOOL CWaveEditDlg::ParseFiles( CString &sDir, ConParse &fileNames )
{
	char szFilePath[_MAX_PATH+1];
	char szSpec[_MAX_PATH+1];
	CString sFiles, sFilePath;
	int nPos;

	// Get the selected files.
	szFilePath[0] = 0;
	CommDlg_OpenSave_GetFilePath( ::GetParent( m_hWnd ), szFilePath, _MAX_PATH );
	szSpec[0] = 0;
	CommDlg_OpenSave_GetSpec( ::GetParent( m_hWnd ), szSpec, _MAX_PATH );

	if( szFilePath[0] == 0 || szSpec[0] == 0 )
		return FALSE;

	sDir = szFilePath;
	sFiles = szSpec;
	sDir = sDir.Left( sDir.GetLength( ) - sFiles.GetLength( ));

	// Make sure single files are surrounded by double quotes, just 
	// in case there are spaces in the file.
	nPos = sFiles.Find( ' ' );
	if( nPos > 0 && sFiles[nPos - 1] != '\"' )
		sFiles.Format( "\"%s\"", szSpec );

	fileNames.Init(( char * )( const char * )sFiles );
	if( !fileNames.Parse( ))
		return FALSE;

	return TRUE;
}


void CWaveEditDlg::OnTimer(UINT nIDEvent) 
{
	// Check if we have a sound going.
#if 0
	if( m_hAudio )
	{
		// Check if sound is done.
		if( AIL_quick_status( m_hAudio ) == QSTAT_DONE )
		{
			AIL_quick_unload( m_hAudio );
			m_hAudio = NULL;
			CommDlg_OpenSave_SetControlText( GetParent( )->m_hWnd, IDOK, "Play" );
		}
		else
		{
			// Still not done, wait a little longer.
			SetTimer( 1, 100, NULL );
		}
	}
#endif
	CFileDialog::OnTimer(nIDEvent);
}
