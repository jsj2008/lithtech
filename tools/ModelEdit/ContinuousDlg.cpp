// ContinuousDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "continuousdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CContinuousDlg dialog


CContinuousDlg::CContinuousDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CContinuousDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContinuousDlg)
	m_nDelay = 0;
	//}}AFX_DATA_INIT
}


void CContinuousDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContinuousDlg)
	DDX_Text(pDX, IDC_DELAY, m_nDelay);
	DDV_MinMaxDWord(pDX, m_nDelay, 1, 60000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContinuousDlg, CDialog)
	//{{AFX_MSG_MAP(CContinuousDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContinuousDlg message handlers
