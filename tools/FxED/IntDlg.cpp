// IntDlg.cpp : implementation file
//
#include "stdafx.h"
#include "spelled.h"
#include "IntDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntDlg dialog


CIntDlg::CIntDlg(int iInitial, CString sName, CWnd* pParent /*=NULL*/)
	: CDialog(CIntDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIntDlg)
	m_int = iInitial;
	//}}AFX_DATA_INIT

	m_sName = sName;
}


void CIntDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIntDlg)
	DDX_Text(pDX, IDC_INTEGER, m_int);
	DDV_MinMaxInt(pDX, m_int, -1000000, 1000000);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		SetWindowText(m_sName);
	}
}


BEGIN_MESSAGE_MAP(CIntDlg, CDialog)
	//{{AFX_MSG_MAP(CIntDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntDlg message handlers
