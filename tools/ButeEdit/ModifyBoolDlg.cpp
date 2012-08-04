// ModifyBoolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyBoolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyBoolDlg dialog


CModifyBoolDlg::CModifyBoolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyBoolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyBoolDlg)
	m_nBoolValue = 0;
	m_sName = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
}


void CModifyBoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyBoolDlg)
	DDX_Radio(pDX, IDC_RADIO_VALUE, m_nBoolValue);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyBoolDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyBoolDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyBoolDlg message handlers

/************************************************************************/
// Call this to set the initial state of the dialog control
void CModifyBoolDlg::SetValue(bool bValue)
{
	if (bValue)
	{
		m_nBoolValue=0;
	}
	else
	{
		m_nBoolValue=1;
	}
}

/************************************************************************/
// Call this to get the value of the control
bool CModifyBoolDlg::GetValue()
{
	if (m_nBoolValue == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CModifyBoolDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyBoolDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}	

	CDialog::OnOK();
}
