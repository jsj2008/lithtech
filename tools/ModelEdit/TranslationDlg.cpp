// TranslationDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "translationdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TranslationDlg dialog


TranslationDlg::TranslationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TranslationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(TranslationDlg)
	m_xTrans = _T("");
	m_yTrans = _T("");
	m_zTrans = _T("");
	//}}AFX_DATA_INIT
}


void TranslationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TranslationDlg)
	DDX_Text(pDX, IDC_XTRANS, m_xTrans);
	DDX_Text(pDX, IDC_YTRANS, m_yTrans);
	DDX_Text(pDX, IDC_ZTRANS, m_zTrans);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TranslationDlg, CDialog)
	//{{AFX_MSG_MAP(TranslationDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TranslationDlg message handlers

