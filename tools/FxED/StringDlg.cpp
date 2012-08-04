// StringDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "StringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog


CStringDlg::CStringDlg(CString sName, BOOL bNullStringOkay, CWnd* pParent /*=NULL*/)
	: CDialog(CStringDlg::IDD, pParent)
{
	m_sName			  = sName;
	m_bNullStringOkay = bNullStringOkay;

	//{{AFX_DATA_INIT(CStringDlg)
	m_sText = _T("");
	//}}AFX_DATA_INIT
}


void CStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringDlg)
	DDX_Text(pDX, IDC_EDIT1, m_sText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringDlg, CDialog)
	//{{AFX_MSG_MAP(CStringDlg)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringDlg message handlers

BOOL CStringDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_sName);

	if (!m_bNullStringOkay) GetDlgItem(IDOK)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStringDlg::OnChangeEdit1() 
{
	CString sTxt;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sTxt);
	
	if (!m_bNullStringOkay)
	{
		if (sTxt.GetLength())
		{
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}
	}
}
