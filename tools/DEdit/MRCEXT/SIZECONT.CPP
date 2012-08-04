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
// SIZECONT.CPP
// $Date:   03 Jul 1996 14:04:06  $
// $Revision:   1.1  $
// $Author:   MRC  $
// sizecont.cpp : implementation file
// Sizeable Control Bar


#include "mrcstafx.h"
#include "mrcext.h"
#include "mrcpriv.h"
#include "mrcresrc.h"
#include "resource.h"


extern MRC_AUX_DATA afxData;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMRCSizeControlBar, CControlBar)

/////////////////////////////////////////////////////////////////////////
// CSCBButton

CSCBButton::CSCBButton()
{
    bRaised = FALSE;
    bPushed = FALSE;
}

void CSCBButton::Paint(CDC* pDC)
{
    CRect rc = GetRect();

    if (bPushed)
        pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNSHADOW),
            ::GetSysColor(COLOR_BTNHIGHLIGHT));
    else
        if (bRaised)
            pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));

    COLORREF clrOldTextColor = pDC->GetTextColor();
    pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(60, 96, ppi); // 6 points at 96 ppi
    font.CreatePointFont(pointsize, _T("Marlett"));
    CFont* oldfont = pDC->SelectObject(&font);

    pDC->TextOut(ptOrg.x + 2, ptOrg.y + 2, CString(_T("r"))); // x-like
    
    pDC->SelectObject(oldfont);
    pDC->SetBkMode(nPrevBkMode);
    pDC->SetTextColor(clrOldTextColor);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMRCSizeControlBar


CObArray * CMRCSizeControlBar::m_parrAllocBars;	

void CMRCSizeControlBar::TidyUp()
{
// just need to delete the contents of the array - the destructor will remove the element
// from the array. 
	if (m_parrAllocBars != NULL)
	{
		for (int i = m_parrAllocBars->GetUpperBound(); i >= 0; i--)
		{
			ASSERT((*m_parrAllocBars)[i]->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)));
			delete ((*m_parrAllocBars)[i]);
		}
		delete m_parrAllocBars;
		m_parrAllocBars = NULL;
	}
}


CMRCSizeControlBar::CMRCSizeControlBar(int nStyle)
{
	m_Style = nStyle;						
    m_PrevSize.cx = 0xffff;         // dummy values so WindowPosChanged will respond correctly
    m_bPrevFloating = 3;            // neither TRUE not FALSE;
	m_nGripperSize=18;
	m_bShowIcons = true;

	m_FloatingPosition  = CPoint(0,0);	
	m_dwAllowDockingState = 0;
	if (nStyle & SZBARF_AUTOTIDY)
	{
		if (m_parrAllocBars == NULL)
			m_parrAllocBars = new CObArray;
		
		m_parrAllocBars->Add(this);
	}
}


CMRCSizeControlBar::~CMRCSizeControlBar()
{
	// if the bar was created with this flag, then ensure it is deleted with it also.
	if (m_Style & SZBARF_AUTOTIDY)
	{
		int i;
		for (i = m_parrAllocBars->GetUpperBound(); i >= 0; i--)
			if ((*m_parrAllocBars)[i] == this)	
			{
				m_parrAllocBars->RemoveAt(i);
				break;
			}
		ASSERT(i >= 0);			// means we didn't delete this item from the list
	}

	// This loop of debug code checks that we don;t have any other references in the array.
	// This happens if we changed the auto delete flag during the lifetime of the control bar.
#ifdef _DEBUG
	if (m_parrAllocBars != NULL)
	{
		for (int i = m_parrAllocBars->GetUpperBound(); i >= 0; i--)
			ASSERT ((*m_parrAllocBars)[i] != this);	
	}
#endif

    // delete the dock context here - in an attempt to call the correct destructor
    delete (CDragDockContext *) m_pDockContext;
    m_pDockContext = NULL;
}


BEGIN_MESSAGE_MAP(CMRCSizeControlBar, CControlBar)
        //{{AFX_MSG_MAP(CMRCSizeControlBar)
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
    ON_WM_PAINT()	
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_MRC_HIDE, OnHide)
	ON_COMMAND(ID_MRC_HIDE_CURRENT, OnHideCurrent) // Hide the current tab
	ON_COMMAND(ID_MRC_UNHIDE_ALL, OnUnhideAll) // Unhide all hidden tabs
	ON_COMMAND(ID_MRC_SHOW_ICONS, OnShowIcons) // Show/Hide the tab icons
	ON_COMMAND(ID_MRC_ALLOWDOCKING, OnToggleAllowDocking)
	ON_COMMAND(ID_MRC_MDIFLOAT,	OnFloatAsMDI)
	ON_MESSAGE(WM_ADDCONTEXTMENUITEMS, OnAddContextMenuItems)
	ON_MESSAGE(WM_USER_AFTERFLOAT_MSG, OnAfterFloatMessage)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMRCSizeControlBar message handlers
CSize CMRCSizeControlBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
#ifdef _VERBOSE_TRACE
	CString strTitle;
	GetWindowText(strTitle);
	TRACE("CalcFixedLayout: '%s' Horz(%d,%d)\n", LPCTSTR(strTitle), 
				m_HorzDockSize.cx, m_HorzDockSize.cy);
#endif	
	CControlBar::CalcFixedLayout(bStretch, bHorz);
    if (IsFloating())
		return m_FloatSize;

    if (bHorz)
		return m_HorzDockSize;
	else
		return m_VertDockSize;
}


//-----------------------------------------------------------------------------------------
void CMRCSizeControlBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
//-----------------------------------------------------------------------------------------
// need to supply this, or else we can't instantiate the class. Derived classes should
// subclass this if they need to update their gadgets using this interface
{
	BOOL bNeedPaint = FALSE;

    CPoint pt;
    ::GetCursorPos(&pt);
    BOOL bHit = (OnNcHitTest(pt) == HTCLOSE);
    BOOL bLButtonDown = (::GetKeyState(VK_LBUTTON) < 0);

    BOOL bWasPushed = m_biHide.bPushed;
    m_biHide.bPushed = bHit && bLButtonDown;

    BOOL bWasRaised = m_biHide.bRaised;
    m_biHide.bRaised = bHit && !bLButtonDown;

    bNeedPaint |= (m_biHide.bPushed ^ bWasPushed) || 
                  (m_biHide.bRaised ^ bWasRaised);

    if (bNeedPaint)
        SendMessage(WM_NCPAINT);
}


//-----------------------------------------------------------------------------------------
BOOL CMRCSizeControlBar::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
							DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
// CWnd-style create - need ability to specific window class in order to prevent flicker
// during resizing. 
//-----------------------------------------------------------------------------------------
{
    ASSERT(pParentWnd != NULL);

    // have to set the style here
    m_dwStyle = dwStyle;

    CRect Rectx;
    Rectx = rect;

    // calculate a sensible default rectangle if that's what the user wanted...
    if (memcmp(&rect, &CFrameWnd::rectDefault, sizeof(RECT)) == 0)
    {
        pParentWnd->GetClientRect(&Rectx);
        CSize def;
        def.cx = Rectx.right / 2;
        def.cy = Rectx.bottom  / 4;
        Rectx.left = Rectx.right - def.cx;
        Rectx.top  = Rectx.bottom - def.cy;
	}

   	// the rectangle specifies the default floating size.
   	m_FloatSize = Rectx.Size();
	
	// set default values for the docked sizes, based on this size.
	m_HorzDockSize.cx = m_FloatSize.cx;
	m_HorzDockSize.cy = m_FloatSize.cy / 2;	

	m_VertDockSize.cx = m_HorzDockSize.cy * 3 / 2;
	m_VertDockSize.cy = m_HorzDockSize.cx * 2 / 3;
        
    return CControlBar::Create(lpszClassName, lpszWindowName, dwStyle, Rectx, pParentWnd, nID, pContext);
}


//-----------------------------------------------------------------------------------------
BOOL CMRCSizeControlBar::Create(CWnd * pParentWnd, LPCTSTR lpszWindowName, UINT nID,
								DWORD dwStyle, const RECT & rect)
//-----------------------------------------------------------------------------------------
{
	return Create(NULL, lpszWindowName, dwStyle, rect, pParentWnd, nID);
}


void CMRCSizeControlBar::SetSizeDockStyle(DWORD dwStyle)
{
	m_Style = dwStyle;
}


//------------------------------------------------------------------------------------
void CMRCSizeControlBar::EnableDocking(DWORD dwDockStyle)
// Largely a copy of CControlBar::EnableDocking() - but uses a different class for the
// m_pDockContext, to give us different (hopefully you'll think better) dragging 
// behaviour.
//------------------------------------------------------------------------------------
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    m_dwDockStyle = dwDockStyle;
    if (m_pDockContext == NULL)
    	m_pDockContext = new CDragDockContext(this);
	
    // permanently wire the bar's owner to its current parent
	if (m_hWndOwner == NULL)
    	m_hWndOwner = ::GetParent(m_hWnd);
}


//--------------------------------------------------------------------------
afx_msg LONG CMRCSizeControlBar::OnAfterFloatMessage(UINT wParam, LONG lParam)
// message handler. Force the parent of the control bar to update it's style
// after floating, otherwise we'll wait till an WM_NCHITTEST.
//--------------------------------------------------------------------------
{
	CWnd * pFrame = GetParentFrame();
	if (pFrame != NULL && pFrame->IsKindOf(RUNTIME_CLASS(CSizableMiniDockFrameWnd)))
		((CSizableMiniDockFrameWnd *) pFrame)->GetContainedBarType();
	
	return TRUE;			// message handled.
}


//--------------------------------------------------------------------------
BOOL CMRCSizeControlBar::OnEraseBkgnd(CDC* pDC)
// paint the background of the window - probably want a style flag to turn this
// off as for many control bars it won't be required.
//--------------------------------------------------------------------------
{
	CRect rect;
    pDC->GetClipBox(&rect);
    pDC->FillSolidRect(&rect, afxData.clrBtnFace); 
    return TRUE;
}


//--------------------------------------------------------------------------
LRESULT CMRCSizeControlBar::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
// Normally a CControlBar would just pass most of these messages through to
// the parent. We want to handle them properly though - again may be this should
// be a behviour flag
//--------------------------------------------------------------------------
{
    ASSERT_VALID(this);

    // We need to ensure WM_COMMAND and other messages get through to the derived class.
    // Primarily done so we receive notifications from child windows. The default CControlBar
    // code routes messsages through to the parent. This means WM_COMMANDs, etc make their
    // way to a FrameWnd eventually. This is needed for toolbar's, dialog bars, etc, but isn't
    // very useful if we want to put controls on a CMRCSizeControlBar and process them 
	// locally
       
    // In case any of these messages are actually needed by the owner window, we check to see
    // if CWnd would handle them first. If not, then we pass them through to the owner window,
    // as CControlBar would.

	// SHP - We also need the WM_CLOSE window message	

    switch (nMsg)
    {
    case WM_COMMAND:
    	if (OnCommand(wParam, lParam))          // post normal commands....
        	return 1L; // command handled
		break;
	case WM_SYSCOMMAND:
		{
			OnControlBarClose();
			break;
		}
	case WM_CLOSE:
		if (OnControlBarClose())
			return 1L;
		break;

    }
	return CControlBar::WindowProc(nMsg, wParam, lParam);
}

BOOL CMRCSizeControlBar::OnControlBarClose()
{
	// The default implementation does nothing
	return FALSE;
}

//------------------------------------------------------------------
void CMRCSizeControlBar::OnWindowPosChanged(WINDOWPOS * lpwndpos)
// This handler is used to notify sizeable bars if their size has
// changed, or if they are docked/undocked.
//------------------------------------------------------------------
{
    CControlBar::OnWindowPosChanged(lpwndpos);

    CSize NewSize(lpwndpos->cx, lpwndpos->cy);
    
	// This is meant to return "floating" if we're not docked yet...
    BOOL bFloating = IsProbablyFloating();
    
    int Flags = (NewSize == m_PrevSize ? 0 : 1);
    Flags |= (bFloating == m_bPrevFloating ? 0 : 2);
    if (Flags != 0)
    {
		m_PrevSize = NewSize;
        m_bPrevFloating = bFloating;
        OnSizedOrDocked(NewSize.cx, NewSize.cy, bFloating, Flags);
	}
}


//------------------------------------------------------------------
void CMRCSizeControlBar::OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags)
// override this function to respond to a redraw as a result of a
// resize or docked/undocked notification
//------------------------------------------------------------------
{	
}


//------------------------------------------------------------------
BOOL CMRCSizeControlBar::IsProbablyFloating()
//------------------------------------------------------------------
{ 
	// used to check the dock bar status, but this has problems when we 
	// docking/undocking - so check the actual bar style instread
	return (m_pDockBar == NULL || (GetBarStyle() & CBRS_FLOATING));
}	


//--------------------------------------------------------------------------
LONG CMRCSizeControlBar::OnAddContextMenuItems(WPARAM wParam, LPARAM lParam)
//--------------------------------------------------------------------------
{
	HMENU hMenu = (HMENU)lParam;		// handle of menu.
	CMenu Menu;
	Menu.Attach(hMenu);

	DWORD dwDockStyle = m_dwDockStyle & CBRS_ALIGN_ANY;
	DWORD style;
	CString strMenu;

	BOOL bMDIFloating = FALSE;
	CFrameWnd *pParentFrame = GetParentFrame();
	if (IsFloating())
	{
		if (pParentFrame != NULL && pParentFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)))
		{
			bMDIFloating = TRUE;
		}
	}
	style = (bMDIFloating ? MF_STRING | MF_CHECKED : MF_STRING);
	
	// if allowed - add the float as MDI floating window
	if (m_Style & SZBARF_ALLOW_MDI_FLOAT)
	{	
		VERIFY(strMenu.LoadString(IDS_PROJ_BAR_FLOAT));
		Menu.AppendMenu(style, ID_MRC_MDIFLOAT, strMenu);
	}
	
	
	VERIFY(strMenu.LoadString(IDS_PROJ_BAR_HIDE_CURRENT));
	Menu.AppendMenu(MF_STRING, ID_MRC_HIDE_CURRENT, strMenu);

	VERIFY(strMenu.LoadString(IDS_PROJ_BAR_UNHIDE_ALL));
	Menu.AppendMenu(MF_STRING, ID_MRC_UNHIDE_ALL, strMenu);
	
	VERIFY(strMenu.LoadString(IDS_PROJ_BAR_HIDE));
	Menu.AppendMenu(MF_STRING, ID_MRC_HIDE, strMenu);

	VERIFY(strMenu.LoadString(IDS_PROJ_BAR_SHOW_ICONS));
	Menu.AppendMenu(MF_STRING, ID_MRC_SHOW_ICONS, strMenu);

	// Set checkmark appropriately
	Menu.CheckMenuItem(ID_MRC_SHOW_ICONS, m_bShowIcons ? MF_CHECKED : MF_UNCHECKED );

	if (!bMDIFloating && (dwDockStyle != 0 || m_dwAllowDockingState != 0))	// ie docking is actually allowed ...
	{
		DWORD style = (dwDockStyle != 0 ? MF_STRING | MF_CHECKED : MF_STRING);
		VERIFY(strMenu.LoadString(IDS_PROJ_BAR_ALLOW_DOCKING));
		Menu.AppendMenu(style, ID_MRC_ALLOWDOCKING, strMenu);
	}

	Menu.Detach();	// detatch MFC object
	return TRUE;
}


//------------------------------------------------------------------
void CMRCSizeControlBar::OnHide()
//------------------------------------------------------------------
{
	CFrameWnd * pFrameWnd = GetParentFrame();
	pFrameWnd->ShowControlBar(this, FALSE, FALSE);
}

/************************************************************************/
// This gets called when the "hide" button is pressed (otherwise known as "X" in the top of the window -DC)
void CMRCSizeControlBar::OnButtonHide()
{
	CFrameWnd * pFrameWnd = GetParentFrame();
	pFrameWnd->ShowControlBar(this, FALSE, FALSE);
}


//------------------------------------------------------------------
void CMRCSizeControlBar::OnToggleAllowDocking()
//------------------------------------------------------------------
{
	CFrameWnd *pParentFrame = GetParentFrame();
	if ((m_dwDockStyle & CBRS_ALIGN_ANY) != 0)
	{											   // docking currently allowed - disable it
		m_dwAllowDockingState = m_dwDockStyle & CBRS_ALIGN_ANY;	// save previous state
		if (!IsFloating())
		{	
			ASSERT(m_pDockContext != NULL);
			m_pDockContext->ToggleDocking();
		}
		EnableDocking(0);				// disable docking
	}
	else if(m_pDockSite)
	{
		EnableDocking (m_dwAllowDockingState);	// re-enable docking...
	}
}

//------------------------------------------------------------------
void CMRCSizeControlBar::OnFloatAsMDI()
//------------------------------------------------------------------
{
	ASSERT(m_Style & SZBARF_ALLOW_MDI_FLOAT);		// must have specified this

	CMRCMDIFrameWndSizeDock * pFrame = (CMRCMDIFrameWndSizeDock *) AfxGetMainWnd();
	ASSERT(pFrame != NULL);
	ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFrameWndSizeDock)));

	CFrameWnd *pParentFrame = GetParentFrame();
	BOOL bMDIFloating = (IsFloating() && pParentFrame != NULL && pParentFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)));
	
	ASSERT(m_pDockContext != NULL);
	
	if (bMDIFloating)
	{
		pFrame->UnFloatInMDIChild(this, m_pDockContext->m_ptMRUFloatPos);
	}
	else
	{
		pFrame->FloatControlBarInMDIChild(this, m_pDockContext->m_ptMRUFloatPos);
	}		
}

 
//----------------------------------------------------------------------------------
void CMRCSizeControlBar::OnContextMenu(CWnd* pWnd, CPoint point) 
// Now run off WM_CONTEXTMENU: if user wants standard handling, then let him have it
//----------------------------------------------------------------------------------
{
	if (m_Style & SZBARF_STDMOUSECLICKS)
	{
		CMenu menu;
		if (menu.CreatePopupMenu())
		{
			OnAddContextMenuItems(0, (LPARAM)menu.m_hMenu);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

		}
	} 
	else
		CControlBar::OnContextMenu(pWnd, point);
}

BOOL CMRCSizeControlBar::IsFloating()
{
    return !IsHorzDocked() && !IsVertDocked();
}

BOOL CMRCSizeControlBar::IsHorzDocked()
{
	int nDockBarID=GetParent()->GetDlgCtrlID();

    return (nDockBarID == AFX_IDW_DOCKBAR_TOP || nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

BOOL CMRCSizeControlBar::IsVertDocked()
{
	int nDockBarID=GetParent()->GetDlgCtrlID();

    return (nDockBarID == AFX_IDW_DOCKBAR_LEFT || nDockBarID == AFX_IDW_DOCKBAR_RIGHT);
}

void CMRCSizeControlBar::OnNcPaint() 
{
	// get window DC that is clipped to the non-client area
    CWindowDC dc(this);

    CRect rcClient, rcBar;
    GetClientRect(rcClient);
    ClientToScreen(rcClient);
    GetWindowRect(rcBar);
    rcClient.OffsetRect(-rcBar.TopLeft());
    rcBar.OffsetRect(-rcBar.TopLeft());

    // client area is not our bussiness :)
    dc.ExcludeClipRect(rcClient);

    // draw borders in non-client area
    CRect rcDraw = rcBar;
    DrawBorders(&dc, rcDraw);

    // erase parts not drawn
    dc.IntersectClipRect(rcDraw);

    // erase NC background	
	dc.FillRect(rcDraw, &CBrush(GetSysColor(COLOR_BTNFACE)));

    if (m_nGripperSize > 0 && !IsFloating())
	{
        NcPaintGripper(&dc, rcClient);
	}

    //ReleaseDC(&dc);
}

void CMRCSizeControlBar::NcPaintGripper(CDC* pDC, CRect rcClient)
{
    // paints a simple "two raised lines" gripper
    // override this if you want a more sophisticated gripper
    CRect gripper = rcClient;
	CRect rcbtn = m_biHide.GetRect();
    BOOL bHorz = IsHorzDocked();
    
    gripper.DeflateRect(1, 1);
    if (bHorz)
    {   // gripper at left
        gripper.left -= m_nGripperSize;
        gripper.right = gripper.left + 3;
		gripper.top = rcbtn.bottom + 3;
		gripper.bottom -= 3;
        // gripper.top = rcbtn.bottom + 3;
    }
    else
    {   // gripper at top
		gripper.left += 3;
		gripper.right = rcbtn.left - 3;
        gripper.top -= m_nGripperSize;
        gripper.bottom = gripper.top + 3;		

		gripper.OffsetRect(0,6);
        // gripper.right = gripper.left + 3;
    }

    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

    gripper.OffsetRect(bHorz ? 3 : 0, bHorz ? 0 : 3);
    
    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

	m_biHide.Paint(pDC);
}

void CMRCSizeControlBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// compute the the client area
    CRect rcClient = lpncsp->rgrc[0];        

	if (!IsFloating())
	{
		int nDockBarID=GetParent()->GetDlgCtrlID();
		switch(nDockBarID)
		{
		case AFX_IDW_DOCKBAR_TOP:
			rcClient.DeflateRect(m_nGripperSize, 0, 0, 0);
			break;
		case AFX_IDW_DOCKBAR_BOTTOM:			
			rcClient.DeflateRect(m_nGripperSize, 0, 0, 0);
			break;
		case AFX_IDW_DOCKBAR_LEFT:			
			rcClient.DeflateRect(0, m_nGripperSize, 0, 0);
			break;
		case AFX_IDW_DOCKBAR_RIGHT:			
			rcClient.DeflateRect(0, m_nGripperSize, 0, 0);
			break;
		default:
			break;
		}    
	}

	// "hide" button positioning
    CPoint ptOrgBtn;
    if (IsHorzDocked())
	{
        ptOrgBtn = CPoint(rcClient.left - m_nGripperSize - 1, rcClient.top - 1);
	}
    else
	{
        ptOrgBtn = CPoint(rcClient.right - 12, rcClient.top - m_nGripperSize +4 );
	}

    m_biHide.Move(ptOrgBtn - CRect(lpncsp->rgrc[0]).TopLeft());

    lpncsp->rgrc[0] = rcClient;	
}

UINT CMRCSizeControlBar::OnNcHitTest(CPoint point) 
{
	if (IsFloating())
	{	
		return CControlBar::OnNcHitTest(point);
	}
	else
	{
		CRect rcBar;
	    GetWindowRect(rcBar);

		CRect rc = m_biHide.GetRect();
		rc.OffsetRect(rcBar.TopLeft());
		if (rc.PtInRect(point))
		{
			return HTCLOSE;
		}
		else
		{
			return HTCLIENT;
		}
	}
}

void CMRCSizeControlBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    if (IsFloating())
    {
        CControlBar::OnNcLButtonDown(nHitTest, point);
        return;
    }    

	// only start dragging if clicked in "void" space
	if (m_pDockBar != NULL)
	{
		if ((nHitTest >= HTSIZEFIRST) && (nHitTest <= HTSIZELAST))
		{
			// start the drag
			ASSERT(m_pDockContext != NULL);
			ClientToScreen(&point);
			m_pDockContext->StartDrag(point);
		}
	}
	else
	{
		CControlBar::OnNcLButtonDown(nHitTest, point);
	}
}

void CMRCSizeControlBar::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{	
	if (m_pDockBar != NULL)
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		m_pDockContext->ToggleDocking();
	}
	else
	{
		CControlBar::OnNcLButtonDblClk(nHitTest, point);
	}	
}

void CMRCSizeControlBar::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{	
	if (nHitTest == HTCLOSE)
	{
        OnButtonHide();
	}

	CControlBar::OnNcLButtonUp(nHitTest, point);	
}

void CMRCSizeControlBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CControlBar::OnLButtonDown(nFlags, point);
}
