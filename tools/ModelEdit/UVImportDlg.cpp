// UVImportDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "uvimportdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// UVImportDlg dialog


UVImportDlg::UVImportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(UVImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(UVImportDlg)
	m_AnimationName = _T("");
	//}}AFX_DATA_INIT
}


void UVImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(UVImportDlg)
	DDX_Text(pDX, IDC_ANIMATIONNAME, m_AnimationName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(UVImportDlg, CDialog)
	//{{AFX_MSG_MAP(UVImportDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// UVImportDlg message handlers
