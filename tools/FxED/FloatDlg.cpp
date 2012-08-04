// FloatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FloatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloatDlg dialog


CFloatDlg::CFloatDlg(float fInitial, CString sName, CWnd* pParent /*=NULL*/)
	: CDialog(CFloatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFloatDlg)
	m_float = fInitial;
	//}}AFX_DATA_INIT

	m_sName = sName;
}


void CFloatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFloatDlg)
	DDX_Text(pDX, IDC_EDIT1, m_float);
	DDV_MinMaxFloat(pDX, m_float, -100000.0f, 100000.0f);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		SetWindowText(m_sName);
	}
}


BEGIN_MESSAGE_MAP(CFloatDlg, CDialog)
	//{{AFX_MSG_MAP(CFloatDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloatDlg message handlers
