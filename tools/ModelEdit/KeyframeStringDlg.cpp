// KeyframeStringDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "keyframestringdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyframeStringDlg dialog


CKeyframeStringDlg::CKeyframeStringDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyframeStringDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyframeStringDlg)
	m_KeyframeString = _T("");
	//}}AFX_DATA_INIT
}


void CKeyframeStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyframeStringDlg)
	DDX_Text(pDX, IDC_THESTRING, m_KeyframeString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyframeStringDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyframeStringDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyframeStringDlg message handlers
