// RotationDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "rotationdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RotationDlg dialog


RotationDlg::RotationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RotationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RotationDlg)
	m_sRotX = _T("");
	m_sRotY = _T("");
	m_sRotZ = _T("");
	//}}AFX_DATA_INIT
}


void RotationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RotationDlg)
	DDX_Text(pDX, IDC_ROTX, m_sRotX);
	DDX_Text(pDX, IDC_ROTY, m_sRotY);
	DDX_Text(pDX, IDC_ROTZ, m_sRotZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RotationDlg, CDialog)
	//{{AFX_MSG_MAP(RotationDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RotationDlg message handlers

