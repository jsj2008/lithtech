//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// navigatordlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "regiondoc.h"
#include "regionview.h"
#include "navigator.h"
#include "navigatordlg.h"
#include "navigatorstoredlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNavigatorDlg dialog


CNavigatorDlg::CNavigatorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNavigatorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNavigatorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pNavigatorPosArray=NULL;
	m_pRegionView=NULL;
}


void CNavigatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNavigatorDlg)
	DDX_Control(pDX, IDC_LIST_NAVIGATOR_ITEMS, m_listNavigatorItems);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNavigatorDlg, CDialog)
	//{{AFX_MSG_MAP(CNavigatorDlg)
	ON_BN_CLICKED(ID_BUTTON_GOTO, OnButtonGoto)
	ON_BN_CLICKED(IDC_BUTTON_MOVEUP, OnButtonMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_MOVEDOWN, OnButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTON_RENAME, OnButtonRename)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_LBN_DBLCLK(IDC_LIST_NAVIGATOR_ITEMS, OnDblclkListNavigatorItems)
	ON_LBN_SELCHANGE(IDC_LIST_NAVIGATOR_ITEMS, OnSelchangeListNavigatorItems)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNavigatorDlg message handlers

BOOL CNavigatorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Make sure that we have a region view pointer
	if (m_pRegionView == NULL)
	{
		ASSERT(FALSE);		
	}

	// Build the navigator list box
	BuildNavigatorList();	
	
	// Select the first item in the list
	if (m_listNavigatorItems.GetCount() > 0)
	{
		// Select the first item
		m_listNavigatorItems.SetCurSel(0);
	}
	
	// Update the enabled status of the buttons
	UpdateEnabledStatus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Build the navigator list box
void CNavigatorDlg::BuildNavigatorList()
{
	m_listNavigatorItems.ResetContent();
	if (m_pNavigatorPosArray)
	{
		int i;
		for (i=0; i < m_pNavigatorPosArray->GetSize(); i++)
		{
			m_listNavigatorItems.AddString(m_pNavigatorPosArray->GetAt(i)->GetDescription());
		}
	}
}

// Update the enabled status of the buttons
void CNavigatorDlg::UpdateEnabledStatus()
{
	// The currently selected item
	int nCurSel=m_listNavigatorItems.GetCurSel();

	// Set the status for the Rename, Delete, and GoTo button
	if (nCurSel != LB_ERR)
	{
		GetDlgItem(ID_BUTTON_GOTO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_RENAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(ID_BUTTON_GOTO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_RENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(FALSE);
	}

	// Update the MoveUp button
	if (nCurSel != LB_ERR && nCurSel != 0)
	{
		GetDlgItem(IDC_BUTTON_MOVEUP)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_MOVEUP)->EnableWindow(FALSE);
	}

	// Update the MoveDown button
	if (nCurSel != LB_ERR && nCurSel != m_listNavigatorItems.GetCount()-1)
	{
		GetDlgItem(IDC_BUTTON_MOVEDOWN)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_MOVEDOWN)->EnableWindow(FALSE);
	}
}

// The GoTo button was pressed
void CNavigatorDlg::OnButtonGoto() 
{
	// Make sure that there is an item selected
	int nSelIndex=m_listNavigatorItems.GetCurSel();
	if (nSelIndex == LB_ERR)
	{
		return;
	}
		
	// Set the navigation location for the region view
	if (m_pRegionView)
	{
		m_pRegionView->SwitchNavigator(nSelIndex);	
	}
}

// An item in the list box was double clicked on
void CNavigatorDlg::OnDblclkListNavigatorItems() 
{
	// Make sure that there is an item selected
	int nSelIndex=m_listNavigatorItems.GetCurSel();
	if (nSelIndex == LB_ERR)
	{
		return;
	}
		
	// Set the navigation location for the region view
	if (m_pRegionView)
	{
		m_pRegionView->SwitchNavigator(nSelIndex);	
	}

	UpdateEnabledStatus();
}

void CNavigatorDlg::OnSelchangeListNavigatorItems() 
{
	UpdateEnabledStatus();	
}

// The move up button was pressed
void CNavigatorDlg::OnButtonMoveUp() 
{
	// Get the current selection index
	int nSelItem=m_listNavigatorItems.GetCurSel();
	if (nSelItem == LB_ERR || nSelItem == 0)
	{
		return;
	}

	// Move the selected item up
	if (m_pNavigatorPosArray)
	{
		// Swap the current selection with the previous one
		CNavigatorPosItem *pTemp=m_pNavigatorPosArray->GetAt(nSelItem-1);
		m_pNavigatorPosArray->SetAt(nSelItem-1, m_pNavigatorPosArray->GetAt(nSelItem));
		m_pNavigatorPosArray->SetAt(nSelItem, pTemp);
	}

	// Build the navigator list box
	BuildNavigatorList();	

	// Set the selection
	m_listNavigatorItems.SetCurSel(nSelItem-1);

	UpdateEnabledStatus();	
}

// The move down button was pressed
void CNavigatorDlg::OnButtonMoveDown() 
{	
	// Get the current selection index
	int nSelItem=m_listNavigatorItems.GetCurSel();
	if (nSelItem == LB_ERR || nSelItem == m_listNavigatorItems.GetCount()-1)
	{
		return;
	}

	// Move the selected item up
	if (m_pNavigatorPosArray)
	{
		// Swap the current selection with the next one
		CNavigatorPosItem *pTemp=m_pNavigatorPosArray->GetAt(nSelItem+1);
		m_pNavigatorPosArray->SetAt(nSelItem+1, m_pNavigatorPosArray->GetAt(nSelItem));
		m_pNavigatorPosArray->SetAt(nSelItem, pTemp);
	}

	// Build the navigator list box
	BuildNavigatorList();	

	// Set the selection
	m_listNavigatorItems.SetCurSel(nSelItem+1);

	UpdateEnabledStatus();	
}

// The rename button was pressed
void CNavigatorDlg::OnButtonRename() 
{
	// Make sure that there is an item selected
	int nSelIndex=m_listNavigatorItems.GetCurSel();
	if (nSelIndex == LB_ERR)
	{
		return;
	}
	
	// Get the selected item
	CNavigatorPosItem *pNavigatorItem=m_pNavigatorPosArray->GetAt(nSelIndex);
	if (!pNavigatorItem)
	{
		return;
	}

	// Display the dialog to get the new name
	CNavigatorStoreDlg storeDlg;
	storeDlg.m_sName=pNavigatorItem->GetDescription();

	if (storeDlg.DoModal() == IDOK)
	{		
		// Set the description
		pNavigatorItem->SetDescription(storeDlg.m_sName.GetBuffer(0));

		// Build the navigator list box
		BuildNavigatorList();	

		// Set the selection
		m_listNavigatorItems.SetCurSel(nSelIndex);
	}
}

// The delete button was pressed
void CNavigatorDlg::OnButtonDelete() 
{
	// Make sure that there is an item selected
	int nSelIndex=m_listNavigatorItems.GetCurSel();
	if (nSelIndex == LB_ERR)
	{
		return;
	}
	
	// Delete and remove the selected item
	delete m_pNavigatorPosArray->GetAt(nSelIndex);
	m_pNavigatorPosArray->Remove(nSelIndex);

	// Build the navigator list box
	BuildNavigatorList();	

	// Check to see if the listbox is empty
	if (m_listNavigatorItems.GetCount() == 0)
	{
		// Make sure that nothing is selected
		m_listNavigatorItems.SetCurSel(-1);
	}	
	else
	{
		// Set the selection to the last item that was selected or the last item in the listbox
		m_listNavigatorItems.SetCurSel(LTMIN(nSelIndex, m_listNavigatorItems.GetCount()-1));	
	}
	
	UpdateEnabledStatus();	
}

