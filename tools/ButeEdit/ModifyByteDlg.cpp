// ModifyByteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyByteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyByteDlg dialog


CModifyByteDlg::CModifyByteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyByteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyByteDlg)
	m_nValue = 0;
	m_sName = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
}


void CModifyByteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyByteDlg)
	DDX_Text(pDX, IDC_EDIT_BYTE, m_nValue);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyByteDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyByteDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyByteDlg message handlers

BOOL CModifyByteDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyByteDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
