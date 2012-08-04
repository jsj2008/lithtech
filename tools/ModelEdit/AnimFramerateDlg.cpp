// AnimFramerateDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "animframeratedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimFramerateDlg dialog


CAnimFramerateDlg::CAnimFramerateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimFramerateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimFramerateDlg)
	m_bAllAnimations = FALSE;
	m_FramerateString = _T("");
	//}}AFX_DATA_INIT
}


void CAnimFramerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimFramerateDlg)
	DDX_Check(pDX, IDC_ALLANIMATIONS, m_bAllAnimations);
	DDX_Text(pDX, IDC_FRAMERATESTRING, m_FramerateString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimFramerateDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimFramerateDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimFramerateDlg message handlers
