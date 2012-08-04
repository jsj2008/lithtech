// TimeOffsetDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "timeoffsetdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TimeOffsetDlg dialog


TimeOffsetDlg::TimeOffsetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TimeOffsetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(TimeOffsetDlg)
	m_TimeOffset = 0;
	//}}AFX_DATA_INIT
}


void TimeOffsetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TimeOffsetDlg)
	DDX_Text(pDX, IDC_TIME_OFFSET, m_TimeOffset);
	DDV_MinMaxUInt(pDX, m_TimeOffset, 0, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TimeOffsetDlg, CDialog)
	//{{AFX_MSG_MAP(TimeOffsetDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TimeOffsetDlg message handlers
