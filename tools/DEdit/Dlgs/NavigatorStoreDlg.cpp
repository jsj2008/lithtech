//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// NavigatorStoreDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "navigatorstoredlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNavigatorStoreDlg dialog


CNavigatorStoreDlg::CNavigatorStoreDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNavigatorStoreDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNavigatorStoreDlg)
	m_sName = _T("");
	//}}AFX_DATA_INIT
}


void CNavigatorStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNavigatorStoreDlg)
	DDX_Text(pDX, IDC_EDIT_NAVIGATOR_NAME, m_sName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNavigatorStoreDlg, CDialog)
	//{{AFX_MSG_MAP(CNavigatorStoreDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNavigatorStoreDlg message handlers
