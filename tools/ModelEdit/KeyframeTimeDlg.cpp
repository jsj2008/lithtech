// KeyframeTimeDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "keyframetimedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyframeTimeDlg dialog


CKeyframeTimeDlg::CKeyframeTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyframeTimeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyframeTimeDlg)
	m_nNewTime = 0;
	m_nCurrentTime = 0;
	m_sCaption = "";
	//}}AFX_DATA_INIT
}


void CKeyframeTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyframeTimeDlg)
	DDX_Text(pDX, IDC_NEW_TIME, m_nNewTime);
	DDX_Text(pDX, IDC_CURRENT_TIME, m_nCurrentTime);
	//}}AFX_DATA_MAP
}

BOOL CKeyframeTimeDlg::OnInitDialog()
{
	CDialog::OnInitDialog( );
	SetWindowText( m_sCaption );
	return FALSE;
}

BEGIN_MESSAGE_MAP(CKeyframeTimeDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyframeTimeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyframeTimeDlg message handlers
