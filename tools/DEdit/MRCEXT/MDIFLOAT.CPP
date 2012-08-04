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
// mdifloat.cpp : implementation file
// $Revision:   1.0  $
// $Date:   23 May 1996 16:45:44  $
// $Author:   MRC  $

// This is the window used when floating a control bar in an MDI client (so it appears at
// the same level as other documents in an application). Some work is still need to handle
// saving state properly.


#include "mrcstafx.h"
#include "mrcpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMRCMDIFloatWnd

IMPLEMENT_DYNCREATE(CMRCMDIFloatWnd, CMDIFloat_Parent)

CMRCMDIFloatWnd::CMRCMDIFloatWnd() : m_wndMDIDockBar(TRUE)
{
	m_wndMDIDockBar.m_bAutoDelete = FALSE;
}

CMRCMDIFloatWnd::~CMRCMDIFloatWnd()
{
}


BEGIN_MESSAGE_MAP(CMRCMDIFloatWnd, CMDIFloat_Parent)
	//{{AFX_MSG_MAP(CMRCMDIFloatWnd)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMRCMDIFloatWnd message handlers

//--------------------------------------------------------------------------
BOOL CMRCMDIFloatWnd::Create(CWnd* pParent, DWORD dwBarStyle)
//--------------------------------------------------------------------------
{
	// set m_bInRecalcLayout to avoid flashing during creation
	// RecalcLayout will be called once something is docked
	m_bInRecalcLayout = TRUE;

	static CString strFloatClass;
	if (strFloatClass.IsEmpty())
		strFloatClass = AfxRegisterWndClass(CS_DBLCLKS);

	DWORD dwStyle = WS_CLIPCHILDREN | WS_CHILD | WS_OVERLAPPEDWINDOW | FWS_SNAPTOBARS;
	if (!CMDIFloat_Parent::Create(strFloatClass, NULL, dwStyle, rectDefault, NULL  /*(CMDIFrameWnd *)pParent)*/))
	{
		m_bInRecalcLayout = FALSE;
		return FALSE;
	}

	// Create a single dockbar for this frame with ID=AFX_IDW_DOCKBAR_FLOAT (picked up later)
	dwStyle = dwBarStyle & (CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT) ?
		CBRS_ALIGN_LEFT : CBRS_ALIGN_TOP;
	dwStyle |= dwBarStyle & CBRS_FLOAT_MULTI;

	if (!m_wndMDIDockBar.Create(pParent, WS_CHILD | WS_VISIBLE | dwStyle, AFX_IDW_DOCKBAR_FLOAT))
	{
		m_bInRecalcLayout = FALSE;
		return FALSE;
	}
	m_wndMDIDockBar.SetParent(this);

	m_bInRecalcLayout = FALSE;
	return TRUE;			 
}


//---------------------------------------------------------------------
BOOL CMRCMDIFloatWnd::PreCreateWindow(CREATESTRUCT& cs) 
//---------------------------------------------------------------------
{
	// Turn off the WS_EX_CLIENTEDGE style on the Frame.
	if (!CMDIFloat_Parent::PreCreateWindow(cs))
		return FALSE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	return TRUE;

}


//--------------------------------------------------------------------------
void CMRCMDIFloatWnd::OnSize(UINT nType, int cx, int cy) 
// respond to the miniframe resizing. If we've got a sizeable control
// bar in the window, then we set it's size. Need to adjust for the
// window borders. The window will then get repositioned by a ReCalcLayout()
//--------------------------------------------------------------------------
{
    if (cx == 0 && cy == 0)
    	return;

	CDockBar* pDock = &m_wndMDIDockBar;

    // We don't support CBRS_FLOAT_MULTI
    if ((pDock->m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
        // CMiniDockFrameWnd class assumes if there is only 1 bar, then it's at position 1
        // in the array
        CMRCSizeControlBar* pBar = ((CSizeDockBar *)pDock)->GetFirstControlBar();
        // ignore size request if not visible....
        if (pBar != NULL && IsSizeable(pBar) && pBar->IsVisible())
        {
			CRect rect(0, 0, cx, cy);
    		AdjustForBorders(rect, pBar->m_dwStyle);

            pBar->m_FloatSize.cx = rect.Width();
            pBar->m_FloatSize.cy = rect.Height();
        }
    }
	CMDIFloat_Parent::OnSize(nType, cx, cy);	// call parent to update menu's etc.
}


void CMRCMDIFloatWnd::OnClose() 
{
	CDockBar* pDock = &m_wndMDIDockBar;

	if ((pDock->m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
     // CMiniDockFrameWnd class assumes if there is only 1 bar, then it's at position 1
     // in the array
    	CControlBar* pBar = ((CSizeDockBar *) pDock)->GetFirstControlBar();
     	if (pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)) )
 			if (((CMRCSizeControlBar *)pBar)->m_Style & SZBARF_DESTROY_ON_CLOSE)
            {
              	// close the Frame Window
              	CFrameWnd::OnClose();       // close the window
              	delete pBar;            // now explicitly delete the control bar
              	return;
			}
    } 

    // otherwise just close it. .. really just want to hide it...
	CMDIFloat_Parent::OnClose();
}


void CMRCMDIFloatWnd::RecalcLayout(BOOL bNotify)
{
	if (!m_bInRecalcLayout)
	{
		CMDIFloat_Parent::RecalcLayout(bNotify);

		// syncronize window text of frame window with dockbar itself
		CString strTitle;
		m_wndMDIDockBar.GetWindowText(strTitle);
		AfxSetWindowText(m_hWnd, strTitle);
	}
}


//------------------------------------------------------------------
void CMRCMDIFloatWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
// Updates the floating position for a bar. 
//------------------------------------------------------------------
{
	CMDIFloat_Parent::OnWindowPosChanged(lpwndpos);

	if ((m_wndMDIDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
		CMRCSizeControlBar* pBar = ((CSizeDockBar *) &m_wndMDIDockBar)->GetFirstControlBar();
     	if (pBar != NULL)
 		{
			ASSERT(pBar->m_pDockContext != NULL);
			CRect rcWindow;
			GetWindowRect(rcWindow);
			pBar->m_pDockContext->m_ptMRUFloatPos = rcWindow.TopLeft();
 		}	
	}
}
