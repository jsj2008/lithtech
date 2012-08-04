// ScaleAnimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ScaleAnimDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScaleAnimDlg dialog


CScaleAnimDlg::CScaleAnimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleAnimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScaleAnimDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScaleAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleAnimDlg)
	DDX_Control(pDX, IDC_FAVOURITES, m_favourites);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// Load the dialog

		CLinkListNode<SK_FAVOURITE *> *pNode = pApp->GetScaleFavourites()->GetHead();

		while (pNode)
		{
			int nIndex = m_favourites.AddString(pNode->m_Data->m_sName);
			m_favourites.SetItemData(nIndex, (DWORD)pNode->m_Data);
			
			pNode = pNode->m_pNext;
		}

		m_favourites.SetCurSel(0);
	}
	else
	{
		m_pFavourite = NULL;
		
		int nCurSel = m_favourites.GetCurSel();
		if (nCurSel != LB_ERR) m_pFavourite = (SK_FAVOURITE *)m_favourites.GetItemData(nCurSel);
	}
}


BEGIN_MESSAGE_MAP(CScaleAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CScaleAnimDlg)
	ON_LBN_DBLCLK(IDC_FAVOURITES, OnDblclkFavourites)
	ON_BN_CLICKED(IDC_DELCLRFAV, OnDelclrfav)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleAnimDlg message handlers

void CScaleAnimDlg::OnDblclkFavourites() 
{
	OnOK();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDelclrFav()
//
//   PURPOSE  : Deletes the scale favourite
//
//------------------------------------------------------------------

void CScaleAnimDlg::OnDelclrfav() 
{
	int nIndex = m_favourites.GetCurSel();
	if (nIndex == LB_ERR) return;

	SK_FAVOURITE *pSclFav = (SK_FAVOURITE *)m_favourites.GetItemData(nIndex);

	// Delete the object
	
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->GetScaleFavourites()->Remove(pSclFav);

	// And remove it from the list box

	m_favourites.DeleteString(nIndex);

}
