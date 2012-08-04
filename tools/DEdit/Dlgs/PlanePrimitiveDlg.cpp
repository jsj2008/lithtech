// PlanePrimitiveDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "PlanePrimitiveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanePrimitiveDlg dialog


CPlanePrimitiveDlg::CPlanePrimitiveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlanePrimitiveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlanePrimitiveDlg)
	m_fWidth = 50.0f;
	m_fHeight = 50.0f;
	m_nOrientation = 0;
	//}}AFX_DATA_INIT
}


void CPlanePrimitiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlanePrimitiveDlg)
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_fWidth);
	DDV_MinMaxFloat(pDX, m_fWidth, 0.f, 65000.f);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_fHeight);
	DDV_MinMaxFloat(pDX, m_fHeight, 0.f, 65000.f);
	DDX_Radio(pDX, IDC_RADIO_POSX, m_nOrientation);
	DDX_Radio(pDX, IDC_RADIO_TRIANGLE, m_nType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlanePrimitiveDlg, CDialog)
	//{{AFX_MSG_MAP(CPlanePrimitiveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanePrimitiveDlg message handlers
