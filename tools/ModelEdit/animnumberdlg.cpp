// AnimNumberDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "animnumberdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AnimNumberDlg dialog


AnimNumberDlg::AnimNumberDlg(CWnd* pParent /*=NULL*/, int maxNum)
	: CDialog(AnimNumberDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AnimNumberDlg)
	m_MoveNumber = 0;
	m_MaxNumber = maxNum;
	//}}AFX_DATA_INIT
}


void AnimNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AnimNumberDlg)
	DDX_Text(pDX, IDC_MOVENUMBER, m_MoveNumber);
	DDV_MinMaxInt(pDX, m_MoveNumber, 0, m_MaxNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AnimNumberDlg, CDialog)
	//{{AFX_MSG_MAP(AnimNumberDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AnimNumberDlg message handlers
