// AnimCleanupDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "animcleanupdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimCleanupDlg dialog


CAnimCleanupDlg::CAnimCleanupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimCleanupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimCleanupDlg)
	m_bPreserveRot = FALSE;
	m_VarianceString = _T("");
	//}}AFX_DATA_INIT
}


void CAnimCleanupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimCleanupDlg)
	DDX_Check(pDX, IDC_PRESERVEROT, m_bPreserveRot);
	DDX_Text(pDX, IDC_MINVARIANCE, m_VarianceString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimCleanupDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimCleanupDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimCleanupDlg message handlers
