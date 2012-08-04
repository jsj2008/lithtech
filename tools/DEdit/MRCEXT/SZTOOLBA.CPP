// MRCEXT: Micro Focus Extension DLL for MFC 2.1+
// Copyright (C)1994-5	Micro Focus Inc, 2465 East Bayshore Rd, Palo Alto, CA 94303.
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation. In addition, you may also charge for any
//  application	using MRCEXT, and are under no obligation to supply source
//  code. You must accredit Micro Focus Inc in the "About Box", or banner
//  of your application. 
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should also have received a copy of the GNU General Public License with this
//  software, also indicating additional rights you have when using MRCEXT.  
//
//
// sztoolba.cpp : implementation file
// $Date:   23 May 1996 16:46:06  $
// $Author:   MRC  $
// 
//
// classes in this file
// CMRCSizeToolBar - a sizeable toolbar. A sizeable control bar containing a CToolBarCtrl object.
//				  which responds to tooltips/command enabling through the normal MFC architecture
//
// CToolCmdUI   - CCmdUI private class.
//
// CMRCToolBarCtrl - CToolBarCtrl derived object with addition left mouse button handling, etc.

#include "mrcstafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define TOOLCTRL_ID 99

/////////////////////////////////////////////////////////////////////////////
// CMRCSizeToolBar

IMPLEMENT_DYNAMIC(CMRCSizeToolBar, CMRCSizeControlBar);

CMRCSizeToolBar::CMRCSizeToolBar(int nStyle) : CMRCSizeControlBar(nStyle)
{
	// these are the default sizes for the toolbar bitmap
	m_ToolCtrlButtonSize.cx = 16;
	m_ToolCtrlButtonSize.cy = 15;
	m_pBitmapIds = NULL;
	m_nBitmapButtons = 0;
}


CMRCSizeToolBar::~CMRCSizeToolBar()
{
}



BEGIN_MESSAGE_MAP(CMRCSizeToolBar, CMRCSizeControlBar)
	//{{AFX_MSG_MAP(CMRCSizeToolBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_RANGE(TTN_NEEDTEXT, 0, 0xffff, OnTBNToolTip)
	ON_NOTIFY_RANGE(TBN_QUERYINSERT, 0, 0xffff, OnTBNQueryInsert)
	ON_NOTIFY_RANGE(TBN_QUERYDELETE, 0, 0xffff, OnTBNQueryDelete)
	ON_NOTIFY_RANGE(TBN_BEGINADJUST, 0, 0xffff, OnTBNBeginAdjust)
	ON_NOTIFY_RANGE(TBN_TOOLBARCHANGE, 0, 0xffff, OnTBNToolBarChange)
//	ON_NOTIFY(TBN_GETBUTTONINFO, TOOLCTRL_ID, OnTBNGetButtonInfo)
  	ON_NOTIFY_RANGE(TBN_GETBUTTONINFO, 0, 0xffff, OnTBNGetButtonInfo)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMRCSizeToolBar message handlers


#define DOCKED_HORZ_BORDER 6
#define DOCKED_VERT_BORDER 4

//-------------------------------------------------------------------
void CMRCSizeToolBar::OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags)
// respond to this event as we need to override it
//-------------------------------------------------------------------
{
	CRect rect(0,0,cx,cy);		// rectangle for client area
	CRect TabRect;

	// shrink rectangle if we're docked
	//if (!IsProbablyFloating())
		rect.InflateRect(- DOCKED_HORZ_BORDER,  - DOCKED_VERT_BORDER );

	m_ToolCtrl.MoveWindow(&rect);
	m_ToolCtrl.AutoSize();
}


//-----------------------------------------------------------------------------
BOOL CMRCSizeToolBar::Create(CWnd * pParent, DWORD dwStyle, UINT nID, LPRECT pRect)
//-----------------------------------------------------------------------------
{
	m_ToolCtrlButtonSize.cx = 16;
	m_ToolCtrlButtonSize.cy = 15;
	
	// if no rectangle supplied, then what is hopefully as sensible default.
	// ie a single row of buttons
	CRect rect;
	if (pRect != NULL)
		rect.CopyRect(pRect);
	else
	{
		pParent->GetClientRect(&rect);
		rect.left = 0;
		rect.top = 0;
		rect.bottom = m_ToolCtrlButtonSize.cy + 18;
		rect.right -= 8;
	}
		
	BOOL status = CMRCSizeControlBar::Create(pParent, NULL, nID, dwStyle, rect); 
	if (status == TRUE)
	{
	}
	return status;
}



//-----------------------------------------------------------------------------
int CMRCSizeToolBar::OnCreate(LPCREATESTRUCT lpCS) 
//-----------------------------------------------------------------------------
{
	if (CMRCSizeControlBar::OnCreate(lpCS) == -1)
		return -1;

	CRect rect;
	GetClientRect(&rect);
	
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | TBSTYLE_WRAPABLE // | TBSTYLE_TOOLTIPS
				| CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;  // | CCS_NOHILITE*  - CCS_NOHILITE doesn't exist in VC2.2 
	
	// If there a list of bitmap id's, then let the user configure the toolbar
	if (m_pBitmapIds != NULL)
		dwStyle |= CCS_ADJUSTABLE;
	
	if (!m_ToolCtrl.Create(dwStyle, rect, this, TOOLCTRL_ID))
	{
		TRACE("Failed to create CToolBarCtrl\n");
		return -1;
	}
	return 0;
}



// These are functions designed as 1-1 replacements for existing CToolBar functions. They
// are not the most efficient way to make use of the CSizeToolBarCtrl control, but mean it'll
// work with standard AppWizard created applications.

//-----------------------------------------------------------------------------
BOOL CMRCSizeToolBar::LoadBitmap(LPCTSTR lpszResourceName)
//-----------------------------------------------------------------------------
{
	if (!m_Bitmap.LoadBitmap(lpszResourceName))
		return FALSE;

	// get the bitmap info, and use this to get the no of buttons in the bitmap
	BITMAP bm;
	m_Bitmap.GetObject(sizeof BITMAP, &bm); 
	
	int nButtons = bm.bmWidth / m_ToolCtrlButtonSize.cx;
	
	if (m_ToolCtrl.AddBitmap(nButtons, &m_Bitmap) == -1)
		return FALSE;

	return TRUE;
}


//-----------------------------------------------------------------------------
void CMRCSizeToolBar::SetSizes(SIZE sizeButton, SIZE sizeImage)
//-----------------------------------------------------------------------------
{
// if succeeded, keep track of it in the size of the control
	if (m_ToolCtrl.SetButtonSize(sizeButton))
		m_ToolCtrlButtonSize = sizeButton;

	m_ToolCtrl.SetBitmapSize(sizeImage);
}


//-----------------------------------------------------------------------------
void CMRCSizeToolBar::SetBitmapIds(UINT * pIds, int nButtons)
//-----------------------------------------------------------------------------
{
	m_pBitmapIds = pIds;
	m_nBitmapButtons = nButtons;
}


//-----------------------------------------------------------------------------
int CMRCSizeToolBar::FindBitmapIndex(UINT nID)
//-----------------------------------------------------------------------------
{
	ASSERT(m_pBitmapIds != NULL);
	for (int i = 0; i < m_nBitmapButtons ; i++)
	{
		if (m_pBitmapIds[i] == (int)nID)
			return i;
	}
	return -1;
}



//-----------------------------------------------------------------------------
BOOL CMRCSizeToolBar::SetButtons(UINT * pButtons, int nButtons)
// emulate CToolBar::SetButtons()
//-----------------------------------------------------------------------------
{
	// allocate an array of TBBUTTON's
	TBBUTTON * parrButtons = new TBBUTTON[nButtons];		// allocate an array

	int nImageNo = 0;		// no of image in bitmap (coun
	int nBtn = 0; 			// no of buttons we've actually created (including separators)
							// may not equal no of buttons supplied by user, as id's may not
							// be found
	for (int i = 0; i < nButtons; i++)
	{
		UINT nID = pButtons[i];
		parrButtons[i].dwData =  NULL;
		parrButtons[i].iString = NULL;
		if (nID == ID_SEPARATOR)
		{
			parrButtons[nBtn].iBitmap = NULL;
			parrButtons[nBtn].idCommand = 0;
			parrButtons[nBtn].fsState = 0;
			parrButtons[nBtn].fsStyle = TBSTYLE_SEP;  
		} 
		else
		{
			if (m_pBitmapIds != NULL)		// if there's a list of bitmaps, then translate this
			{
				nImageNo = FindBitmapIndex(nID);
				if (nImageNo == -1)
				{
					TRACE("Couldn't find bitmap for ID=%d\n",nID);
					continue;					// skip to next iteration
					nImageNo = 0;
				}
				else
				{
					parrButtons[i].iBitmap = nImageNo;
				}
			}
			else
			{
				parrButtons[i].iBitmap = nImageNo;
				nImageNo++;
			}
			parrButtons[nBtn].idCommand = nID;
			parrButtons[nBtn].fsState = TBSTATE_ENABLED;
			parrButtons[nBtn].fsStyle = TBSTYLE_BUTTON; 
		}
		nBtn++;
	}

	BOOL status = m_ToolCtrl.AddButtons(nBtn, parrButtons);
	delete parrButtons;

	return status;
}	



//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNToolTip(UINT uID, NMHDR *pNMHDR, LRESULT * pResult)
// tool tip notification handler for tool bar
//-----------------------------------------------------------------------------
{
	TOOLTIPTEXT * pTip = (TOOLTIPTEXT *) pNMHDR;
	pTip->hinst = NULL;
	pTip->lpszText = NULL;
	UINT ButtonId = pTip->hdr.idFrom;	
	
	CString strText;
	if (strText.LoadString(ButtonId))
	{
		char * pTipText = strchr(strText, '\n');   // tool tip is after "\n" in the string
		if (pTipText != NULL)
		{	
			strcpy(pTip->szText, pTipText + 1);
			pTip->lpszText = pTip->szText;
		
			// try to ensure tool tip control and ensure it is top most in the Z-order
			CToolTipCtrl * pToolTipCtrl = m_ToolCtrl.GetToolTips();
			pToolTipCtrl->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	
		}	
		// Set the text in the main window. Doesn't appear to be an easy way to check if
		// we're not over any particular bit of the tool tip.
		AfxGetMainWnd()->SendMessage(WM_SETMESSAGESTRING, ButtonId);
		return; 
	}	
	
	TRACE("CMRCSizeToolBar:No Tooltip prompt for ID=%d\n", ButtonId);
	return;	
}


//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNGetButtonInfo(UINT nID, NMHDR *pNMHDR, LRESULT * pResult)
// Return information for bitmap indexes in the toolbar
//-----------------------------------------------------------------------------
{
	if (nID != TOOLCTRL_ID)
		return;
	TBNOTIFY * pTBN = (TBNOTIFY *)pNMHDR;
	
	int nIndex = pTBN->iItem;
	if (nIndex < m_nBitmapButtons)
	{
		*pResult = TRUE;

 	    UINT nButtonId = m_pBitmapIds[nIndex];
 	    pTBN->tbButton.iBitmap = nIndex;
 	    pTBN->tbButton.idCommand = nButtonId;
		pTBN->tbButton.fsState = TBSTATE_ENABLED;
		pTBN->tbButton.fsStyle = TBSTYLE_BUTTON;
		pTBN->tbButton.iString = 0;		
		if (pTBN->pszText != NULL)
		{		
			CString strText;
			if (strText.LoadString(nButtonId))
			{
				char * pTipText = strchr(strText, '\n');   // tool tip is after "\n" in the string
				if (pTipText != NULL)
				{	
					strncpy(pTBN->pszText, pTipText + 1, pTBN->cchText);
           			return;
				}
			}
			TRACE("CMRCSizeToolBar:No Tooltip prompt for ID=%d\n", nButtonId);
			strncpy(pTBN->pszText, "???", pTBN->cchText);
		}
	}
	else 
		*pResult = FALSE;
}



//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNBeginAdjust(UINT uID, NMHDR *pNMHDR, LRESULT * pResult)
//-----------------------------------------------------------------------------
{
}


//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNQueryInsert(UINT uID, NMHDR *pNMHDR, LRESULT * pResult)
//-----------------------------------------------------------------------------
{
	*pResult = TRUE;		// always allow buttons to be inserted
}


//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNQueryDelete(UINT uID, NMHDR *pNMHDR, LRESULT * pResult)
//-----------------------------------------------------------------------------
{
	*pResult = TRUE;		// always allow buttons to be deleted
}

//-----------------------------------------------------------------------------
afx_msg void CMRCSizeToolBar::OnTBNToolBarChange(UINT uID, NMHDR *pNMHDR, LRESULT * pResult)
//-----------------------------------------------------------------------------
{
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMRCToolBarCtrl
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CMRCToolBarCtrl::CMRCToolBarCtrl()
{
}

CMRCToolBarCtrl::~CMRCToolBarCtrl()
{
}


BEGIN_MESSAGE_MAP(CMRCToolBarCtrl, CToolBarCtrl)
	//{{AFX_MSG_MAP(CMRCToolBarCtrl)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMRCToolBarCtrl message handlers

//-----------------------------------------------------------------------------
void CMRCToolBarCtrl::GetBoundingSize(CSize & size)
// Returns the size that would bound all the buttons in the toolbar 
//-----------------------------------------------------------------------------
{
	size.cx = 0;
	size.cy = 0;
	
	for (int i = GetButtonCount(); i >= 0; i--)
	{
		CRect rect;
		if (GetItemRect(i, &rect))
		{
			size.cx = max(size.cx, rect.right);
			size.cy = max(size.cy, rect.bottom);
		}
	}
}



//-----------------------------------------------------------------------------
int CMRCToolBarCtrl::HitTestButtons(CPoint point) 
// returns index of button that lies at that point
//-----------------------------------------------------------------------------
{
	// Hit test all the buttons. It might be an idea to cache the rectangles for all the
	// items, but for now this loop doesn't seem too bad
	for (int i = GetButtonCount(); i >= 0; i--)
	{
		TBBUTTON tbButton;
		CRect rect;
		if (GetItemRect(i, &rect))
		{
			if (rect.PtInRect(point))
			{
				GetButton(i, &tbButton);
				if (tbButton.fsStyle == TBSTYLE_SEP)
					return -1;				// inside a separator		
				else
					return i;				// inside a button
			}
			if (point.y > rect.bottom)		// if point we're testing is below items in the
										// array, then stop (minor optimization)
				break;
		}
	}
	return -1;
}



//-----------------------------------------------------------------------------
void CMRCToolBarCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
//-----------------------------------------------------------------------------
{
	if (HitTestButtons(point) != -1)
	{
		CToolBarCtrl::OnLButtonDown(nFlags, point);
	}
	else
	{
		CWnd * pParent = GetOwner();
		ASSERT(pParent != NULL);
		// map to co-ordinates of parent window
		ClientToScreen(&point);
		pParent->ScreenToClient(&point);
		// let the parent handle it
		pParent->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));  
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CToolBar idle update through CToolCmdUI class. 

class CToolCmdUI : public CCmdUI        // class private to this file !
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

//-----------------------------------------------------------------------------
void CToolCmdUI::Enable(BOOL bOn)
//-----------------------------------------------------------------------------
{
	m_bEnableChanged = TRUE;
	CMRCSizeToolBar* pSizeToolBar = (CMRCSizeToolBar *)m_pOther;
	CMRCToolBarCtrl * pToolBar = &pSizeToolBar->m_ToolCtrl;
	ASSERT(pToolBar != NULL);

	ASSERT(m_nIndex < m_nIndexMax);

	// Get toolbar button state
	TBBUTTON TB;
	pToolBar->GetButton(m_nIndex, &TB);
	UINT nNewState = TB.fsState & ~TBSTATE_ENABLED;
	if (bOn)
		nNewState |= TBSTATE_ENABLED;
	if (nNewState != TB.fsState)
		pToolBar->SetState(m_nID, nNewState);
}


//-----------------------------------------------------------------------------
void CToolCmdUI::SetCheck(int nCheck)
//-----------------------------------------------------------------------------
{
	ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
	CMRCSizeToolBar* pSizeToolBar = (CMRCSizeToolBar *)m_pOther;
	CMRCToolBarCtrl * pToolBar = &pSizeToolBar->m_ToolCtrl;
	ASSERT(pToolBar != NULL);

	ASSERT(m_nIndex < m_nIndexMax);

	// Get toolbar button state
	TBBUTTON TB;
	pToolBar->GetButton(m_nIndex, &TB);
	UINT nNewState = TB.fsState & ~ (TBSTATE_CHECKED | TBSTATE_INDETERMINATE);
	
	if (nCheck == 1)
		nNewState |= TBSTATE_CHECKED;
	else
		if (nCheck == 2)
			nNewState |= TBSTATE_INDETERMINATE;
	
	if (nNewState != TB.fsState)
		pToolBar->SetState(m_nID, nNewState);

   // should we set the button style too ?
   //	pToolBar->_SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX); */
}


//-----------------------------------------------------------------------------
void CToolCmdUI::SetText(LPCTSTR)
//-----------------------------------------------------------------------------
{
	// ignore it
}


//-----------------------------------------------------------------------------
void CMRCSizeToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
//-----------------------------------------------------------------------------
{
	CToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = m_ToolCtrl.GetButtonCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		TBBUTTON TB;
		m_ToolCtrl.GetButton(state.m_nIndex, &TB);
		if (!(TB.fsStyle & TBSTYLE_SEP))
		{
			state.m_nID = TB.idCommand;
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}
	// update any dialog controls added to the toolbar (probably unlikely in this case)
	UpdateDialogControls(pTarget, bDisableIfNoHndler);

}


//-----------------------------------------------------------------------------
CToolBarCtrl * CMRCSizeToolBar::GetToolBarCtrl()
//-----------------------------------------------------------------------------
{
	return &m_ToolCtrl;
}
