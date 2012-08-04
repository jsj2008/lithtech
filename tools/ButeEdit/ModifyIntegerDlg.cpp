// ModifyIntegerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyIntegerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyIntegerDlg dialog


CModifyIntegerDlg::CModifyIntegerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyIntegerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyIntegerDlg)
	m_nValue = 0;
	m_sName = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
}


void CModifyIntegerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyIntegerDlg)
	DDX_Text(pDX, IDC_EDIT_INTEGER, m_nValue);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyIntegerDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyIntegerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyIntegerDlg message handlers

BOOL CModifyIntegerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyIntegerDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
