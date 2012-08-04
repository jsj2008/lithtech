//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// UndoWarningDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "undowarningdlg.h"
#include "optionsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUndoWarningDlg dialog
			

CUndoWarningDlg::CUndoWarningDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUndoWarningDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUndoWarningDlg)
	m_bShowWarningDialog = true;
	//}}AFX_DATA_INIT

	m_nWorldNodeCount = 0;
}


void CUndoWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUndoWarningDlg)
	DDX_Check(pDX, IDC_UW_SHOW_WARNINGS, m_bShowWarningDialog);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUndoWarningDlg, CDialog)
	//{{AFX_MSG_MAP(CUndoWarningDlg)
	ON_BN_CLICKED(IDC_UW_YES,		OnYes)
	ON_BN_CLICKED(IDC_UW_NO,		OnNo)
	ON_BN_CLICKED(IDC_UW_ALWAYS,	OnAlways)
	ON_BN_CLICKED(IDC_UW_NEVER,		OnNever)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUndoWarningDlg message handlers

BOOL CUndoWarningDlg::OnInitDialog() 
{
	char theText[512];
	CWnd *pWnd;

	UpdateData(false);

	sprintf(theText, "Your current world's undo buffer contains %d items.  Too many items in the undo buffer may lead to high memory usage and performance issues.  Would you like to clear old items in the current undo buffer?", m_nWorldNodeCount);

	CDialog::OnInitDialog();

	if(pWnd = GetDlgItem(IDC_UW_TEXT))
		pWnd->SetWindowText(theText);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUndoWarningDlg::OnYes()
{
	SaveShowWarningDialog();
	EndDialog(IDC_UW_YES);
}

void CUndoWarningDlg::OnNo()
{
	SaveShowWarningDialog();
	EndDialog(IDC_UW_NO);
}

void CUndoWarningDlg::OnAlways()
{
	SaveShowWarningDialog();
	EndDialog(IDC_UW_ALWAYS);
}

void CUndoWarningDlg::OnNever()
{
	SaveShowWarningDialog();
	EndDialog(IDC_UW_NEVER);
}

void CUndoWarningDlg::SaveShowWarningDialog()
{
	UpdateData();
	GetApp()->GetOptions().GetMiscOptions()->SetShowUndoWarnings(m_bShowWarningDialog);
}





