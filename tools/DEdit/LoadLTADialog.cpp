// LoadLTADialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "tdguard.h"
#include "LoadLTADialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadLTADialog dialog


CLoadLTADialog::CLoadLTADialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadLTADialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadLTADialog)
	m_sFileName = _T("");
	//}}AFX_DATA_INIT
}


void CLoadLTADialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadLTADialog)
	DDX_Text(pDX, IDC_LTALOADFILENAME, m_sFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadLTADialog, CDialog)
	//{{AFX_MSG_MAP(CLoadLTADialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadLTADialog message handlers

BOOL CLoadLTADialog::OnInitDialog() 
{

	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CAnimateCtrl* pAnim = ((CAnimateCtrl*)GetDlgItem(IDC_LOADANIMCTRL));

	if(pAnim)
	{
		pAnim->Open(IDR_LOADANIM);
		pAnim->Play(0, -1, -1);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
