// ProjectTabCtrl.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "projecttabctrl.h"
#include "projectbar.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectTabCtrl

CProjectTabCtrl::CProjectTabCtrl()
{
	m_pProjectBar=NULL;
}

CProjectTabCtrl::~CProjectTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CProjectTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CProjectTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGING, OnSelchanging)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectTabCtrl message handlers

/************************************************************************/
// This sets the image for the currently selected tab
// NOTE: THIS CRASHES MFC SOMETIMES. Not sure why. The GetItem call crashes
// so calls to this function are currently disabled.
void CProjectTabCtrl::SetCurrentTabImage(int nIndex)
{
	// Get the currently selected tab
	int nCurSel=GetCurSel();
	
	// Make sure that there is a selected item
	if (nCurSel != -1 )
	{
		// Get the currently selected item
		TCITEM item;
		if (GetItem(nCurSel, &item))
		{
			// Set the items image index	
			item.iImage=nIndex;
			item.mask |= TCIF_IMAGE;

			// Set the item	
			SetItem(nCurSel, &item);
		}
	}
}

void CProjectTabCtrl::SetCurrentTabImage()
{
	if (m_pProjectBar == NULL)  return;

	// Get the currently selected tab
	int nCurSel=GetCurSel();
	
	// Make sure that there is a selected item
	if (nCurSel != -1 )
	{
		// Get the currently selected item
		TCITEM item;
		if (GetItem(nCurSel, &item))
		{
			// Set the items image index	
			item.iImage = m_pProjectBar->m_TabInfo[nCurSel]->m_ControlType;
			item.mask |= TCIF_IMAGE;

			// Set the item	
			SetItem(nCurSel, &item);
		}
	}
}

// Sets all tabs to the appropriate image
void CProjectTabCtrl::SetTabImages()
{
	if (m_pProjectBar == NULL)  return;

	for (int32 index=0; index < m_pProjectBar->m_TabInfo.GetSize(); index++)
	{
		// Get the currently selected item
		TCITEM item;
		if (GetItem(index, &item))
		{
			// Set the items image index	
			item.iImage = m_pProjectBar->m_TabInfo[index]->m_ControlType;
			item.mask |= TCIF_IMAGE;
	
			// Set the item	
			SetItem(index, &item);
		}
	}
}

void CProjectTabCtrl::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	*pResult = 0;
}

void CProjectTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	// Get the currently selected tab
	int nCurTab=GetCurSel();

	CTabCtrl::OnLButtonDblClk(nFlags, point);

	// If the currently selected tab changed, then don't
	// allow the tab to be undocked from this click.
	if (nCurTab != GetCurSel())
	{
		return;
	}
	
	// Get the current item rectangle
	CRect rcItem;
	GetItemRect(nCurTab, rcItem);

	// See if the user clicked on this tab
	if (rcItem.PtInRect(point))
	{
		// Undock the tab
		m_pProjectBar->UndockTab(nCurTab, point);
	}	
}
