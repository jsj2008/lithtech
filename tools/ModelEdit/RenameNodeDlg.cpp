// RenameNodeDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "renamenodedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RenameNodeDlg dialog


RenameNodeDlg::RenameNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RenameNodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RenameNodeDlg)
	m_NewName = _T("");
	m_OldName = _T("");
	//}}AFX_DATA_INIT
}


void RenameNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RenameNodeDlg)
	DDX_Text(pDX, IDC_NEWNAME, m_NewName);
	DDX_Text(pDX, IDC_OLDNAME, m_OldName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RenameNodeDlg, CDialog)
	//{{AFX_MSG_MAP(RenameNodeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RenameNodeDlg message handlers
