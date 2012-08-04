// ChooseKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ChooseKeyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseKeyDlg dialog


CChooseKeyDlg::CChooseKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pFav = NULL;
}


void CChooseKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseKeyDlg)
	DDX_Control(pDX, IDC_FAVOURITES, m_favourites);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		CListBox *pBox = (CListBox *)GetDlgItem(IDC_FAVOURITES);

		// We are loading the dialog

		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		CLinkListNode<FK_FAVOURITE *> *pNode = pApp->GetKeyFavourites()->GetHead();

		while (pNode)
		{
			int nIndex = pBox->AddString(pNode->m_Data->m_sName);
			pBox->SetItemData(nIndex, (DWORD)pNode->m_Data);

			pNode = pNode->m_pNext;
		}

		m_favourites.SetCurSel(0);
	}
	else
	{
		// We are unloading the dialog

		int nCurSel = m_favourites.GetCurSel();
		if (nCurSel != LB_ERR)
		{
			m_pFav = (FK_FAVOURITE *)m_favourites.GetItemData(nCurSel);
		}
	}
}


BEGIN_MESSAGE_MAP(CChooseKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseKeyDlg)
	ON_BN_CLICKED(IDC_DELCLRFAV, OnDelFavKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseKeyDlg message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnDelFavKey()
//
//   PURPOSE  : Deletes a favourite key setup
//
//------------------------------------------------------------------

void CChooseKeyDlg::OnDelFavKey() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	int nCurSel = m_favourites.GetCurSel();
	if (nCurSel == LB_ERR) return;

	FK_FAVOURITE *pFav = (FK_FAVOURITE *)m_favourites.GetItemData(nCurSel);

	// Delete the favourite key
	
	pApp->GetKeyFavourites()->Remove(pFav);
	delete pFav;

	// Delete the string from the list box

	m_favourites.DeleteString(nCurSel);
}
