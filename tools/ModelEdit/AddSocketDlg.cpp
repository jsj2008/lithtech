// AddSocketDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeleditdlg.h"
#include "addsocketdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AddSocketDlg dialog


AddSocketDlg::AddSocketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddSocketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddSocketDlg)
	m_SocketName = _T("");
	m_NodeName = _T("");
	//}}AFX_DATA_INIT
}


void AddSocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddSocketDlg)
	DDX_Text(pDX, IDC_SOCKETNAME, m_SocketName);
	DDX_Text(pDX, IDC_NODENAME, m_NodeName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddSocketDlg, CDialog)
	//{{AFX_MSG_MAP(AddSocketDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddSocketDlg message handlers

void AddSocketDlg::OnOK() 
{
	Model *pModel;

	UpdateData(TRUE);

	// Check stuff.
	if(!m_pDlg)
		return;

	pModel = m_pDlg->GetModel();
	if(!pModel)
		return;

	if(pModel->FindSocket((LPCTSTR)m_SocketName))
	{
		m_pDlg->DoMessageBox(IDS_DUPLICATESOCKETNAME, MB_OK);
		return;
	}

	if(!pModel->FindNode((LPCTSTR)m_NodeName))
	{
		m_pDlg->DoMessageBox(IDS_INVALIDNODENAME, MB_OK);
		return;
	}
	
	CDialog::OnOK();
}

BOOL AddSocketDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
