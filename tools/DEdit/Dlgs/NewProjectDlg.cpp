//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// NewProjectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "newprojectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDlg dialog


CNewProjectDlg::CNewProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewProjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewProjectDlg)
	m_ProjectDir = _T("");
	//}}AFX_DATA_INIT
}


void CNewProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewProjectDlg)
	DDX_Text(pDX, IDC_PROJECTDIR_LOCATION, m_ProjectDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewProjectDlg, CDialog)
	//{{AFX_MSG_MAP(CNewProjectDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDlg message handlers

