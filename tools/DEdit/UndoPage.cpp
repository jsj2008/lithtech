//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// UndoPage.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "undopage.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CUndoPage property page

IMPLEMENT_DYNCREATE(CUndoPage, CPropertyPage)

CUndoPage::CUndoPage() : CPropertyPage(CUndoPage::IDD)
{
	//{{AFX_DATA_INIT(CUndoPage)
	m_dwAutoSaveTime = 15;
	m_sBackupPath = _T("c:\\DEditBackups");
	m_nNumBackups = 1;
	m_bEnableAutoSave = TRUE;
	m_bDeleteOnClose = FALSE;
	//}}AFX_DATA_INIT
}

CUndoPage::~CUndoPage()
{
}

void CUndoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUndoPage)
	DDX_Text(pDX, IDC_SAVETIME, m_dwAutoSaveTime);
	DDV_MinMaxInt(pDX, m_dwAutoSaveTime, 1, 60);
	DDX_Text(pDX, IDC_BACKUPPATH, m_sBackupPath);
	DDX_Text(pDX, IDC_NUMBACKUPS, m_nNumBackups);
	DDV_MinMaxInt(pDX, m_nNumBackups, 1, 99);
	DDX_Check(pDX, IDC_ENABLEAUTOSAVE, m_bEnableAutoSave);
	DDX_Check(pDX, IDC_DELETEONEXIT, m_bDeleteOnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUndoPage, CPropertyPage)
	//{{AFX_MSG_MAP(CUndoPage)
	ON_BN_CLICKED(IDC_ENABLEAUTOSAVE, OnEnableAutoSave)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINNUMBACKUPS, OnDeltaPosSpinNumBackups)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSAVETIME, OnDeltaPosSpinSaveTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUndoPage message handlers


BOOL CUndoPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	//set up the spinner for the minutes between autosaves
	CSpinButtonCtrl* pMinSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPINSAVETIME);
	pMinSpin->SetBuddy((CWnd*)GetDlgItem(IDC_SAVETIME));
	pMinSpin->SetRange(1, 60);
	pMinSpin->SetBase(10);
	pMinSpin->SetPos(m_dwAutoSaveTime);

	//setup the spinner for the number of backups to create
	CSpinButtonCtrl* pNumBackSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPINNUMBACKUPS);
	pNumBackSpin->SetBuddy((CWnd*)GetDlgItem(IDC_NUMBACKUPS));
	pNumBackSpin->SetRange(1, 99);
	pNumBackSpin->SetBase(10);
	pNumBackSpin->SetPos(m_nNumBackups);

	EnableAutoSaveItems(m_bEnableAutoSave);
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUndoPage::OnEnableAutoSave() 
{
	//see if it was a check or uncheck
	BOOL bChecked = ((CButton*)GetDlgItem(IDC_ENABLEAUTOSAVE))->GetCheck();

	EnableAutoSaveItems(bChecked);	
}

void CUndoPage::EnableAutoSaveItems(BOOL bVal)
{
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINNUMBACKUPS))->EnableWindow(bVal);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINSAVETIME))->EnableWindow(bVal);
	((CEdit*)GetDlgItem(IDC_SAVETIME))->EnableWindow(bVal);
	((CEdit*)GetDlgItem(IDC_NUMBACKUPS))->EnableWindow(bVal);
	((CEdit*)GetDlgItem(IDC_BACKUPPATH))->EnableWindow(bVal);
	((CButton*)GetDlgItem(IDC_DELETEONEXIT))->EnableWindow(bVal);
	
}

void CUndoPage::OnDeltaPosSpinNumBackups(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	UpdateOnSpin(IDC_NUMBACKUPS, pNMUpDown->iDelta, 1, 99);

	*pResult = 0;
}

void CUndoPage::OnDeltaPosSpinSaveTime(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	UpdateOnSpin(IDC_SAVETIME, pNMUpDown->iDelta, 1, 60);
	
	*pResult = 0;
}

void CUndoPage::UpdateOnSpin(int nControl, int nDelta, int nMin, int nMax)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(nControl);

	//get the value in the edit
	CString sVal;
	pEdit->GetWindowText(sVal);

	//convert it to an integer
	int nVal = atoi(sVal);

	//move it along with the spin
	nVal = LTMAX(nMin, LTMIN(nMax, nVal + nDelta));

	//set the edit's text
	sVal.Format("%d", nVal);
	pEdit->SetWindowText(sVal);
}


void CUndoPage::OnOK()
{
	CDialog::OnOK();
}
