// ModifyFloatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyFloatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyFloatDlg dialog


CModifyFloatDlg::CModifyFloatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyFloatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyFloatDlg)
	m_sName = _T("");
	m_fValue = 0.0f;
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
}


void CModifyFloatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyFloatDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_FLOAT, m_fValue);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyFloatDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyFloatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyFloatDlg message handlers

BOOL CModifyFloatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyFloatDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
