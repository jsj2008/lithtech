// ModifyDWordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyDWordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyDWordDlg dialog


CModifyDWordDlg::CModifyDWordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyDWordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyDWordDlg)
	m_dwValue = 0;
	m_sName = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
}


void CModifyDWordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyDWordDlg)
	DDX_Text(pDX, IDC_EDIT_DWORD, m_dwValue);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyDWordDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyDWordDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyDWordDlg message handlers

BOOL CModifyDWordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyDWordDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
		
	CDialog::OnOK();
}
