// SelectConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "serverapp.h"
#include "SelectConfigDlg.h"
#include "ServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectConfigDlg dialog


CSelectConfigDlg::CSelectConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectConfigDlg)
	//}}AFX_DATA_INIT
}


void CSelectConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectConfigDlg)
	DDX_Control(pDX, IDOK, m_OKCtrl);
	DDX_Control(pDX, IDC_CONFIG, m_ConfigCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectConfigDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectConfigDlg message handlers

BOOL CSelectConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Add all the config files to the combobox.
	AddConfigFilesToControl( );

	// Enable the OK button only if there are config files found.
	m_OKCtrl.EnableWindow( !!m_ConfigCtrl.GetCount( ));

	// Get the number of configs.
	int nNumConfigs = m_ConfigCtrl.GetCount( );

	// If there are no items to select from, then exit.
	if( nNumConfigs == 0 )
	{
		PostMessage( WM_COMMAND, MAKEWPARAM( IDCANCEL, 0 ));
	}
	else
	{	
		// Select the first item if there are items.
		m_ConfigCtrl.SetCurSel( 0 );

		// If there's just one config, just select it.
		if( nNumConfigs == 1 )
			PostMessage( WM_COMMAND, MAKEWPARAM( IDOK, 0 ));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSelectConfigDlg::AddConfigFilesToControl( )
{
	CFileFind fileFind;
	CString sConfigFileSearch;

	// Create a search string to find all the config files in the config directory.
	sConfigFileSearch = GetProfileFile( "*" );
	BOOL bFound = fileFind.FindFile( sConfigFileSearch );
	while( bFound )
	{
		// Get the information about the file found and continue the search.
		bFound = fileFind.FindNextFile( );
		
		// Skip directories and dots.
		if( fileFind.IsDirectory( ) || fileFind.IsDots( ))
			continue;

		// Add just the title of the file to the control.  Don't add the path
		// or the extension.  The serverdlg will figure it out.
		m_ConfigCtrl.AddString( fileFind.GetFileTitle( ));
	}

	return TRUE;
}



void CSelectConfigDlg::OnOK() 
{
	// Save the config selected.
	if( m_ConfigCtrl.GetCurSel( ) != LB_ERR )
	{
		m_ConfigCtrl.GetWindowText( m_sConfig );
	}
	
	CDialog::OnOK();
}

