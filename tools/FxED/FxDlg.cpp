// FxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FxDlg.h"
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFxDlg dialog


CFxDlg::CFxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFxDlg::IDD, pParent)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return;
	}

	//{{AFX_DATA_INIT(CFxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pFxRef = NULL;
}


void CFxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CFxMgr *pFxMgr = pApp->GetFxMgr();

	CListBox *pBox = (CListBox *)GetDlgItem(IDC_FXLIST);

	if (!pDX->m_bSaveAndValidate)
	{
		// Load the dialog

		CLinkListNode<FX_REF> *pNode = pFxMgr->GetFx()->GetHead();

		while (pNode)
		{
			int nIndex = pBox->InsertString(-1, pNode->m_Data.m_sName);
			pBox->SetItemData(nIndex, (DWORD)&pNode->m_Data);
			
			pNode = pNode->m_pNext;
		}
	}
	else
	{
		int nCurSel = pBox->GetCurSel();
		if (nCurSel == LB_ERR)
		{
			m_pFxRef = NULL;
		}
		else
		{
			m_pFxRef = (FX_REF *)pBox->GetItemData(nCurSel);
		}
	}
}


BEGIN_MESSAGE_MAP(CFxDlg, CDialog)
	//{{AFX_MSG_MAP(CFxDlg)
	ON_LBN_DBLCLK(IDC_FXLIST, OnDblclkFxlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFxDlg message handlers

void CFxDlg::OnDblclkFxlist() 
{
	CListBox *pBox = (CListBox *)GetDlgItem(IDC_FXLIST);
	
	if (pBox->GetCurSel() != LB_ERR)
	{
		OnOK();
	}	
}
