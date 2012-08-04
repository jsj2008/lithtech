// AnimMergeDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "animmergedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AnimMergeDlg dialog


AnimMergeDlg::AnimMergeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AnimMergeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AnimMergeDlg)
	m_NewAnimName = _T("");
	m_bSingleFrameDifference = FALSE;
	//}}AFX_DATA_INIT
}


void AnimMergeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AnimMergeDlg)
	DDX_Text(pDX, IDC_NEWANIMNAME, m_NewAnimName);
	DDX_Check(pDX, IDC_SINGLEFRAMEDIFFERENCE, m_bSingleFrameDifference);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AnimMergeDlg, CDialog)
	//{{AFX_MSG_MAP(AnimMergeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AnimMergeDlg message handlers
