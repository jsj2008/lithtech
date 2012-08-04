// CommandStringDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "commandstringdlg.h"
#include "model_ops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommandStringDlg dialog


CCommandStringDlg::CCommandStringDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommandStringDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommandStringDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCommandStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandStringDlg)
	DDX_Control(pDX, IDC_COMMANDSTRING, m_CommandString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommandStringDlg, CDialog)
	//{{AFX_MSG_MAP(CCommandStringDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandStringDlg message handlers

void CCommandStringDlg::OnOK() 
{
	TCHAR theString[2000];

	m_CommandString.GetWindowText(theString, 2000);
	m_pModel->m_CommandString = m_pModel->AddString(theString);
	m_pModel->ParseCommandString();
	
	CDialog::OnOK();
}

BOOL CCommandStringDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ASSERT(m_pModel);
	if(m_pModel->m_CommandString)
		m_CommandString.SetWindowText(m_pModel->m_CommandString);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
