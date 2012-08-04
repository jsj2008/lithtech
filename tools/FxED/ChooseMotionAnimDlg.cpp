// ChooseMotionAnimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ChooseMotionAnimDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseMotionAnimDlg dialog


CChooseMotionAnimDlg::CChooseMotionAnimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseMotionAnimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseMotionAnimDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pFav = NULL;
}


void CChooseMotionAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseMotionAnimDlg)
	DDX_Control(pDX, IDC_FAVOURITES, m_favourites);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		CListBox *pBox = (CListBox *)GetDlgItem(IDC_FAVOURITES);

		// We are loading the dialog

		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		CLinkListNode<MK_FAVOURITE *> *pNode = pApp->GetMoveFavourites()->GetHead();

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
			m_pFav = (MK_FAVOURITE *)m_favourites.GetItemData(nCurSel);
		}
	}
}


BEGIN_MESSAGE_MAP(CChooseMotionAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseMotionAnimDlg)
	ON_BN_CLICKED(IDC_DELCLRFAV, OnDelclrfav)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseMotionAnimDlg message handlers

void CChooseMotionAnimDlg::OnDelclrfav() 
{
	int nIndex = m_favourites.GetCurSel();
	if (nIndex == LB_ERR) return;

	MK_FAVOURITE *pClrFav = (MK_FAVOURITE *)m_favourites.GetItemData(nIndex);

	// Delete the object
	
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->GetMoveFavourites()->Remove(pClrFav);

	// And remove it from the list box

	m_favourites.DeleteString(nIndex);
}
