//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// MultiLineStringDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "multilinestringdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiLineStringDlg dialog


CMultiLineStringDlg::CMultiLineStringDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiLineStringDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiLineStringDlg)
	m_String = _T("");
	m_Caption = _T("");
	//}}AFX_DATA_INIT

	m_bReadOnly = FALSE;
	m_bLimitText = TRUE;
	m_nTextLimit = MAX_STRINGPROP_LEN;
}


void CMultiLineStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiLineStringDlg)
	DDX_Text(pDX, IDC_MULTILINEEDIT, m_String);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiLineStringDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiLineStringDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiLineStringDlg message handlers
BOOL CMultiLineStringDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText( m_Caption + (m_bReadOnly ? " (Read-only)" : ""));
	UpdateData( FALSE );

	GetDlgItem(IDC_MULTILINEEDIT)->SetFocus();
	((CEdit*)GetDlgItem(IDC_MULTILINEEDIT))->SetReadOnly(m_bReadOnly);

	//limit it to the specified number of characters
	if(m_bLimitText)
	{
		((CEdit*)GetDlgItem(IDC_MULTILINEEDIT))->SetLimitText(m_nTextLimit);
	}
	
	return TRUE;  
}
