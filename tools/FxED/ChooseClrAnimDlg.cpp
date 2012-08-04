// ChooseClrAnimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ChooseClrAnimDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimDlg dialog


CChooseClrAnimDlg::CChooseClrAnimDlg(CLinkList<CK_FAVOURITE *> *pList, CWnd* pParent /*=NULL*/)
	: CDialog(CChooseClrAnimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseClrAnimDlg)
	//}}AFX_DATA_INIT

	m_pList = pList;
}


void CChooseClrAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseClrAnimDlg)
	DDX_Control(pDX, IDC_FAVOURITES, m_favourites);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		CListBox *pBox = (CListBox *)GetDlgItem(IDC_FAVOURITES);

		// We are loading the dialog

		CLinkListNode<CK_FAVOURITE *> *pNode = m_pList->GetHead();

		while (pNode)
		{
			int nIndex = pBox->AddString(pNode->m_Data->m_sName);
			pBox->SetItemData(nIndex, (DWORD)pNode->m_Data);

			pNode = pNode->m_pNext;
		}

		pBox->SetCurSel(0);
	}
	else
	{
		// We are unloading the dialog
	}
}


BEGIN_MESSAGE_MAP(CChooseClrAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseClrAnimDlg)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_LBN_DBLCLK(IDC_FAVOURITES, OnDblclkFavourites)
	ON_BN_CLICKED(IDC_DELCLRFAV, OnDelClrFav)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimDlg message handlers

void CChooseClrAnimDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CChooseClrAnimDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if (nIDCtl == IDC_FAVOURITES)
	{
		lpMeasureItemStruct->itemHeight = 30;
	}

	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDblClkFavourites()
//
//   PURPOSE  : Handles double clicking for the favourites list
//
//------------------------------------------------------------------

void CChooseClrAnimDlg::OnDblclkFavourites() 
{
	OnOK();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnOK()
//
//   PURPOSE  : Handles clicking OK
//
//------------------------------------------------------------------

void CChooseClrAnimDlg::OnOK() 
{
	CChooseClrAnimList *pBox = (CChooseClrAnimList *)GetDlgItem(IDC_FAVOURITES);

	if (pBox->GetCurSel() != LB_ERR)
	{
		m_pFavourite = (CK_FAVOURITE *)pBox->GetItemData(pBox->GetCurSel());
	}
	else
	{
		m_pFavourite = NULL;
	}
	
	CDialog::OnOK();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnDelClrFav()
//
//   PURPOSE  : Handles deleting a favourite colour animation
//
//------------------------------------------------------------------

void CChooseClrAnimDlg::OnDelClrFav() 
{
	CChooseClrAnimList *pBox = (CChooseClrAnimList *)GetDlgItem(IDC_FAVOURITES);

	int nIndex = pBox->GetCurSel();
	if (nIndex == LB_ERR) return;

	CK_FAVOURITE *pClrFav = (CK_FAVOURITE *)pBox->GetItemData(nIndex);

	// Delete the object
	
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->GetColourFavourites()->Remove(pClrFav);

	// And remove it from the list box

	pBox->DeleteString(nIndex);
}
