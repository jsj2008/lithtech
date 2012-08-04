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
// DRAGDOCK.CPP
// $Date:   07 Jun 1996 16:49:20  $
// $Revision:   1.1  $
// $Author:   MRC  $
// CDragDockContext class
//
// This replaces the MFC CDockContext class. It provides the "dragging" user interface,
// and includes some extra features that try to indicate exactly where a bar will
// dock. 
//
// The gateway MFC gives us to work with is:
// 1) CControlBar::EnableDocking() creates the dock context for the control bar - so it's
//    important to use the CMRC...Frame.. implementation of EnableDocking().
//
// 2) Other than the constructor/destructor, the only function access outside this class 
//    is to StartDrag(CPoint pt), called when we want to start dragging the control bar.
//     
// The other important tie-ins are the call to Dock/Float control bar which we need to handle
// with the same interpretation of parameters as the existing MFC implementation.

#include "mrcstafx.h"
#include "mrcext.h"
#include "mrcpriv.h"
#include "mrcresrc.h"

extern MRC_AUX_DATA afxData;

#define HORZF(dw) (dw & CBRS_ORIENT_HORZ)
#define VERTF(dw) (dw & CBRS_ORIENT_VERT)

//---------------------------------------------------------------------------
static void AdjustRectangle(CRect& rect, CPoint pt)
// adjust supplied rectangle, so that point, pt is inside it.
// Adjusted slightly from original orientation to ensure point is now
// within specificed rectangle.
//---------------------------------------------------------------------------
{
	int nXOffset = (pt.x <= rect.left) ? (pt.x - rect.left) :
                                        (pt.x >= rect.right) ? (pt.x - rect.right + 1) : 0;
    int nYOffset = (pt.y <= rect.top) ? (pt.y - rect.top) :
                                      (pt.y >= rect.bottom) ? (pt.y - rect.bottom + 1) : 0;
    rect.OffsetRect(nXOffset, nYOffset);

	ASSERT(rect.PtInRect(pt));
}

/////////////////////////////////////////////////////////////////////////////
// CDragDockContext
// static members

CDragDockContext::CDragDockContext(CControlBar* pBar) : CDockContext(pBar)
{
}


CDragDockContext::~CDragDockContext()
{
}


/////////////////////////////////////////////////////////////////////////////
// CDragDockContext operations

//---------------------------------------------------------------------------
void CDragDockContext::StartDrag(CPoint pt)
// start dragging. This is the only routine exposed externally. 
// pt = mouse position at start of drag (screen co-ords) 
//---------------------------------------------------------------------------
{
   	ASSERT_VALID(m_pBar);
   	ASSERT(m_pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)));
   
   	CMRCSizeControlBar *pSzBar = (CMRCSizeControlBar *) m_pBar;

    // get styles from bar
   	m_dwDockStyle = m_pBar->m_dwDockStyle;
   	m_dwStyle = m_pBar->m_dwStyle & CBRS_ALIGN_ANY;
   	ASSERT(m_dwStyle != 0);

	// check to see we're not hanging from a CMRCMDIFloatWnd. Disallow dragging if we are...
	if (m_pBar->IsFloating())
	{
		CFrameWnd * pFrameWnd = m_pBar->GetParentFrame();
		ASSERT(pFrameWnd != NULL);
		ASSERT(pFrameWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
		if (pFrameWnd->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)))
			return;				// do nothing if floating inside a CMRCMDIFloatWnd
	}

	// Set the window focus (SHP 6/10/1999)
	m_pBar->SetFocus();

   // dragging has started message (only if window will actually dock !)
    if ((m_dwDockStyle & CBRS_ALIGN_ANY) != 0)
    	AfxGetMainWnd()->SendMessage(WM_SETMESSAGESTRING, IDS_MRC_STARTDOCKING);

    // handle pending WM_PAINT messages
    MSG msg;
    while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
    {
    	if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
        	return;
		ASSERT(msg.message == WM_PAINT);
        DispatchMessage(&msg);
   	}

    // initialize drag state
   	m_rectLast.SetRectEmpty();
   	m_sizeLast.cx = m_sizeLast.cy = 0;
   	m_bForceFrame = m_bFlip = m_bDitherLast = FALSE;


   	// get current bar location 
   	CRect rect;
   	m_pBar->GetWindowRect(rect);
   	m_ptLast = pt;
   	m_ptStart = pt;
	BOOL bHorz = HORZF(m_dwStyle);

   	// MFC includes code for flipping orientation using the shift key - I wasn't keen
	// on this... (sorry) so I've left it out for now. Some references are still left
	// in for it, in case I decide to implement it.
   	
   	// Start by working out the possible rectangles that dragging could result in.
	// These are:
	// m_rectFrameDragHorz	: floating frame, horizontal orientation
	// m_rectFrameDragVert	: floating frame, vertical orientation (not used, 'cos
	//									flipping not allowed)
	//
	// m_rectDragHorz		: docking horizontally, another bar already on this row
	// m_rectDragVert		: docking vertically, another bar already on this row
	
	// m_rectDragHorzAlone  : docking horizontally, on a new row
	// m_rectDragVertAlone  : docking vertically, on a new row
	
	
	// calculate the size of the docking area, used below
	// FIXME: for now I'm just using the CFrameWnd client area.
	CRect rcFrameClient;
	m_pDockSite->GetClientRect(rcFrameClient);	
	m_pDockSite->ClientToScreen(rcFrameClient);
	int nDockAreaWidth = rcFrameClient.Width();
	int nDockAreaHeight = rcFrameClient.Height();
	
	CSize HorzAloneSize(nDockAreaWidth, pSzBar->m_HorzDockSize.cy);
	CSize VertAloneSize(pSzBar->m_VertDockSize.cx, nDockAreaHeight);
	
	m_rectDragHorzAlone = CRect(CPoint(rcFrameClient.left, rect.top), HorzAloneSize);
	m_rectDragVertAlone = CRect(CPoint(rect.left, rcFrameClient.top),  VertAloneSize);

	// sizes to use when docking into a row that already has some bars.
	// use the stored sizes - unless they are > the max dock area - in which case make a guess.
	if (pSzBar->m_VertDockSize.cy >= nDockAreaHeight - 16)
		VertAloneSize.cy = nDockAreaHeight / 2;
	else 
		VertAloneSize.cy = pSzBar->m_VertDockSize.cy;

	if (pSzBar->m_HorzDockSize.cx >= nDockAreaWidth - 16)
		HorzAloneSize.cx = nDockAreaWidth / 2;
	else
		HorzAloneSize.cx = pSzBar->m_HorzDockSize.cx;
	
	m_rectDragHorz = CRect(rect.TopLeft(), HorzAloneSize);
	m_rectDragVert = CRect(rect.TopLeft(), VertAloneSize);


   	// rectangle for the floating frame...
   	m_rectFrameDragHorz = CRect(rect.TopLeft(), pSzBar->m_FloatSize);
   	m_rectFrameDragVert = m_rectFrameDragHorz;
   	
   	// To work out the size we actually create a floating mini frame, and then see how big
   	// it is
	CMiniDockFrameWnd* pFloatFrame =
           m_pDockSite->CreateFloatingFrame(bHorz ? CBRS_ALIGN_TOP : CBRS_ALIGN_LEFT);
   	if (pFloatFrame == NULL)
    	AfxThrowMemoryException();
   	pFloatFrame->CalcWindowRect(&m_rectFrameDragHorz);
   	pFloatFrame->CalcWindowRect(&m_rectFrameDragVert);
   	m_rectFrameDragHorz.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
   	m_rectFrameDragVert.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
   	pFloatFrame->DestroyWindow();

   	// adjust rectangles so that point is inside
   	AdjustRectangle(m_rectDragHorz, pt);
   	AdjustRectangle(m_rectDragVert, pt);
   	AdjustRectangle(m_rectDragHorzAlone, pt);
   	AdjustRectangle(m_rectDragVertAlone, pt);
   	AdjustRectangle(m_rectFrameDragHorz, pt);
   	AdjustRectangle(m_rectFrameDragVert, pt);

   	// lock window update while dragging
   	ASSERT(m_pDC == NULL);
   	CWnd* pWnd = CWnd::GetDesktopWindow();
#ifndef _MAC
    if (pWnd->LockWindowUpdate())
    	m_pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
    else
#endif
		m_pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE);
    ASSERT(m_pDC != NULL);

    // initialize tracking state and enter tracking loop
	m_dwOverDockStyle = CanDock();
    Move(pt);   // call it here to handle special keys
    Track();
}


void CDragDockContext::Move(CPoint pt)
{
	CPoint ptOffset = pt - m_ptLast;

#ifdef _MAC
    // prevent dragging the floating window completely under the menu bar
    GDHandle hgd = _AfxFindDevice(pt.x, pt.y);
    if (hgd == NULL || hgd == GetMainDevice() ||
                        TestDeviceAttribute(hgd, hasAuxMenuBar))
    {
    	CRect rect;

        if ((HORZF(m_dwStyle) && !m_bFlip) || (VERTF(m_dwStyle) && m_bFlip))
        	rect = m_rectFrameDragHorz;
        else
        	rect = m_rectFrameDragVert;

        // determine our new position
		rect.OffsetRect(ptOffset);

        // keep us on the screen if we were getting too close to the menu bar
        int yMBarBottom = (*hgd)->gdRect.top + GetMBarHeight() + 4;
        if (rect.bottom < yMBarBottom)
        {
        	pt.y += yMBarBottom - rect.bottom;
            ptOffset.y += yMBarBottom - rect.bottom;
        }
	}
#endif

	// offset all drag rects to new position
    m_rectDragHorz.OffsetRect(ptOffset);
    m_rectFrameDragHorz.OffsetRect(ptOffset);
    m_rectDragVert.OffsetRect(ptOffset);
    m_rectFrameDragVert.OffsetRect(ptOffset);
	
	// these rectangles only move in 1 direction
	m_rectDragHorzAlone.top += ptOffset.y;
    m_rectDragHorzAlone.bottom += ptOffset.y;
	m_rectDragVertAlone.left  += ptOffset.x;
	m_rectDragVertAlone.right += ptOffset.x;

    m_rectDragDock.OffsetRect(ptOffset);    // new...
    m_ptLast = pt;

    // if control key is down don't dock
    m_dwOverDockStyle = m_bForceFrame ? 0 : CanDock();

    // update feedback
    DrawFocusRect();
}


void CDragDockContext::OnKey(int nChar, BOOL bDown)
{
	if (nChar == VK_CONTROL)
    	UpdateState(&m_bForceFrame, bDown);
}


void CDragDockContext::EndDrag()
{
    CancelDrag();
	if (m_ptStart == m_ptLast)
		return;

   m_dwOverDockStyle = m_bForceFrame ? 0 : CanDock();
	if (m_dwOverDockStyle != 0)
    {
		// dockbar we're going to dock at.
		CDockBar* pDockBar = GetDockBar();
        ASSERT(pDockBar != NULL);

        // check the original dockbar - if a valid CSizeDockBar...work out the row number.
		CDockBar * pOrigDockBar = m_pBar->m_pDockBar;
		int nOrigCheckSum = -1;
		if (pOrigDockBar != NULL && pOrigDockBar->IsKindOf(RUNTIME_CLASS(CSizeDockBar)))
			nOrigCheckSum = ((CSizeDockBar *)pOrigDockBar)->CheckSumBars();

		// Now we're going to actually dock the window.
		// Update the appropriate size in the control bar.
		if (HORZF(m_dwOverDockStyle)) 
        	((CMRCSizeControlBar *) m_pBar)->m_HorzDockSize = m_rectDragDock.Size();
		else
			((CMRCSizeControlBar *) m_pBar)->m_VertDockSize = m_rectDragDock.Size();

        m_pDockSite->DockControlBar(m_pBar, pDockBar, m_rectDragDock);
        
		// if into a sizeable dockbar (always we be !), then adjust other bars in the same row
		// to attempt to maintain size
		if (pDockBar->IsKindOf(RUNTIME_CLASS(CSizeDockBar)))
		{
			if (pOrigDockBar != pDockBar || ((CSizeDockBar*)pDockBar)->CheckSumBars() != nOrigCheckSum)
			{
				((CSizeDockBar *)pDockBar)->AdjustForNewBar(m_pBar);
			}
			// force RecalcLayout below to adjust sizes always for the bar into which we have docked  - this is needed as
			// if the bar doesn't actually changed position in the array, but has changed size (because the docking 
			// algorithm above guess the size wrong, then we need to set it back again.
			((CSizeDockBar *)pDockBar)->m_CountBars = 0;			
		}
		// This RecalcLayout is what will adjust the size.
		m_pDockSite->RecalcLayout();
    }
    else
	{
    	m_ptMRUFloatPos = m_rectFrameDragHorz.TopLeft();
		m_pDockSite->FloatControlBar(m_pBar, m_rectFrameDragHorz.TopLeft(),
                        CBRS_ALIGN_TOP | (m_dwDockStyle & CBRS_FLOAT_MULTI));
		m_pBar->SendMessage(WM_USER_AFTERFLOAT_MSG);

		// set flag to indicate user has moved the bar - done as a style flag on the window.
		CWnd * pFrameWnd = m_pBar->GetParentFrame();
		ASSERT(pFrameWnd->IsKindOf(RUNTIME_CLASS(CMiniDockFrameWnd)));
		pFrameWnd->ModifyStyle(0, CBRS_MOVED_BY_USER);				
    }
}


void CDragDockContext::CancelDrag()
{
	DrawFocusRect(TRUE);    // gets rid of focus rect
    ReleaseCapture();

    CWnd* pWnd = CWnd::GetDesktopWindow();
#ifndef _MAC
    pWnd->UnlockWindowUpdate();
#endif
    if (m_pDC != NULL)
    {
    	pWnd->ReleaseDC(m_pDC);
        m_pDC = NULL;
    }

	// Tell main window to clear it's status bar
	CWnd *pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd != NULL);
	pMainWnd->SendMessage(WM_SETMESSAGESTRING, (WPARAM)AFX_IDS_IDLEMESSAGE);
}


/////////////////////////////////////////////////////////////////////////////
// Implementation

void CDragDockContext::DrawFocusRect(BOOL bRemoveRect)
{
	ASSERT(m_pDC != NULL);

    // default to thin frame
    CSize size(CX_BORDER, CY_BORDER);

    // determine new rect and size
    CRect rect;
    CBrush* pWhiteBrush = CBrush::FromHandle((HBRUSH)::GetStockObject(WHITE_BRUSH));
    CBrush* pDitherBrush = CDC::GetHalftoneBrush();
    CBrush* pBrush = pWhiteBrush;

    if (m_dwOverDockStyle != 0)
    {
    	rect = m_rectDragDock;
    }
 /* if (HORZF(m_dwOverDockStyle))
		rect = m_rectDragHorz;
    else if (VERTF(m_dwOverDockStyle))
			rect = m_rectDragVert; */
    else
    {
#ifndef _MAC
    	// use thick frame instead
        size.cx = GetSystemMetrics(SM_CXFRAME) - CX_BORDER;
        size.cy = GetSystemMetrics(SM_CYFRAME) - CY_BORDER;
#endif
		rect = m_rectFrameDragHorz;
    	pBrush = pDitherBrush;
    }
	if (bRemoveRect)
    	size.cx = size.cy = 0;

    if ( /*afxData.bWin4 &&         */
                (HORZF(m_dwOverDockStyle) || VERTF(m_dwOverDockStyle)))
    {
    // looks better one pixel in (makes the bar look pushed down)
    	rect.InflateRect(-CX_BORDER, -CY_BORDER);
	}

    // draw it and remember last size
    m_pDC->DrawDragRect(&rect, size, &m_rectLast, m_sizeLast,
                pBrush, m_bDitherLast ? pDitherBrush : pWhiteBrush);
    m_rectLast = rect;
    m_sizeLast = size;
    m_bDitherLast = (pBrush == pDitherBrush);
}


void CDragDockContext::UpdateState(BOOL* pFlag, BOOL bNewValue)
{
    if (*pFlag != bNewValue)
    {
    	*pFlag = bNewValue;
        // m_bFlip = (HORZF(m_dwDockStyle) && VERTF(m_dwDockStyle) && m_bFlip); // shift key
        m_dwOverDockStyle = (m_bForceFrame) ? 0 : CanDock();
        DrawFocusRect();
    }
}


DWORD CDragDockContext::CanDock()
{
	BOOL bStyleHorz;
    DWORD dwDock = 0; // Dock Canidate
    DWORD dwCurr = 0; // Current Orientation

    // let's check for something in our current orientation first
    // then if the shift key is not forcing our orientation then
    // check for horizontal or vertical orientations as long
    // as we are close enough
    ASSERT(m_dwStyle != 0);

    bStyleHorz = HORZF(m_dwStyle);

    m_pTargetDockBar = NULL;

    // can simplify this, 'cos most of the rectangles are actually the same
    if (dwDock == 0 && HORZF(m_dwDockStyle) && m_rectDragHorzAlone.PtInRect(m_ptLast))
    {
        dwDock = m_pDockSite->CanDock(m_rectDragHorzAlone,
                             m_dwDockStyle & ~CBRS_ORIENT_VERT, (CDockBar **)&m_pTargetDockBar);
	}
    if (dwDock == 0 && VERTF(m_dwDockStyle) && m_rectDragVertAlone.PtInRect(m_ptLast))
    {
        dwDock = m_pDockSite->CanDock(m_rectDragVertAlone,
                            m_dwDockStyle & ~CBRS_ORIENT_HORZ, (CDockBar **)&m_pTargetDockBar);
	}

	if (dwDock != 0)	// will dock somewhere - now look for 1/2 bars on the row...
	{
    	if (HORZF(dwDock))
		{
			m_rectDragDock = m_rectDragHorzAlone;
			dwCurr = m_pDockSite->CanDock(m_rectDragHorz, 
        						m_dwDockStyle & ~CBRS_ORIENT_VERT, (CDockBar **)&m_pTargetDockBar);
			if (dwCurr != 0)
				if (m_pTargetDockBar->BarsOnThisRow (m_pBar, m_rectDragHorzAlone) != 0)
					m_rectDragDock = m_rectDragHorz;
		}		
		
    	if (VERTF(dwDock))
		{
			m_rectDragDock = m_rectDragVertAlone;
			dwCurr = m_pDockSite->CanDock(m_rectDragVert, 
        						m_dwDockStyle & ~CBRS_ORIENT_HORZ, (CDockBar **)&m_pTargetDockBar);
			if (dwCurr != 0)
				if (m_pTargetDockBar->BarsOnThisRow (m_pBar, m_rectDragVertAlone) != 0)
					m_rectDragDock = m_rectDragVert;
		}		
	}

    return dwDock;
}


//---------------------------------------------------------------------------
CDockBar* CDragDockContext::GetDockBar()
// should have m_dwOverDockStyle and m_rectDragDock set before calling this
// routine.
//---------------------------------------------------------------------------
{
	DWORD dw = 0;
    CDockBar* pBar;
    if (HORZF(m_dwOverDockStyle))
    {
    	dw = m_pDockSite->CanDock(m_rectDragDock,
                        m_dwOverDockStyle & ~CBRS_ORIENT_VERT, &pBar);
        ASSERT(dw != 0);
        ASSERT(pBar != NULL);
        return pBar;
    }
	if (VERTF(m_dwOverDockStyle))
    {
    	dw = m_pDockSite->CanDock(m_rectDragDock,
                        m_dwOverDockStyle & ~CBRS_ORIENT_HORZ, &pBar);
        ASSERT(dw != 0);
        ASSERT(pBar != NULL);
        return pBar;
    }
    return NULL;
}


//-------------------------------------------------------------------------------
BOOL CDragDockContext::Track()
//-------------------------------------------------------------------------------
{
    // don't handle if capture already set
    if (::GetCapture() != NULL)
        return FALSE;

    // set capture to the window which received this message
    m_pBar->SetCapture();
    ASSERT(m_pBar == CWnd::GetCapture());

    // get messages until capture lost or cancelled/accepted
    while(CWnd::GetCapture() == m_pBar)
    {
        MSG msg;
#ifndef _MAC
        if (!::GetMessage(&msg, NULL, 0, 0))
#else
        // don't allow yielding while tracking since we don't have LockWindowUpdate
        if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
            continue;
        if (msg.message == WM_QUIT)
#endif
        {
            AfxPostQuitMessage(msg.wParam);
            break;
        }

        switch (msg.message)
        {
            case WM_LBUTTONUP:		// drag finished 
                EndDrag();
                return TRUE;
            
            case WM_MOUSEMOVE:
                Move(msg.pt);
                break;
            
            case WM_KEYUP:
                OnKey((int)msg.wParam, FALSE);
                break;
            
            case WM_KEYDOWN:
              	OnKey((int)msg.wParam, TRUE);
                if (msg.wParam == VK_ESCAPE)
            		goto exit_cancel_drag;
                break;
			
			case WM_RBUTTONDOWN:
            	goto exit_cancel_drag;

			// just dispatch rest of the messages
            default:
                DispatchMessage(&msg);
            	break;
		}
	}
    
exit_cancel_drag:			// goto  - can't use break as we're inside a switch()    
    CancelDrag();
    return FALSE;
}


//-------------------------------------------------------------------------------
void CDragDockContext::ToggleDocking()
// Wow - another genuine virtual function in CDockContext !
// overridden to prevent windows floated in an MDI window from being double-clicked
//-------------------------------------------------------------------------------
{
   	ASSERT_VALID(m_pBar);
   	ASSERT(m_pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)));

	// check to see we're not hanging from a CMRCMDIFloatWnd. Disallow toggle docking if we are
	if (m_pBar->IsFloating())
	{
		CFrameWnd * pFrameWnd = m_pBar->GetParentFrame();
		ASSERT(pFrameWnd != NULL);
		ASSERT(pFrameWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
		if (pFrameWnd->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)))
			return;				// do nothing if floating inside a CMRCMDIFloatWnd

		// If the size is zero, then just dock the window (SHP 6/10/1999)
		// This is needed for control bars that are initially floating.
		if (m_sizeLast.cx == 0 && m_sizeLast.cy == 0)
		{
			// Always dock the window on the right
			CFrameWnd *pMainFrame=(CFrameWnd *)AfxGetMainWnd();

			// Make sure that the control bar is docked to a correct side
			if (m_pBar->m_dwDockStyle & CBRS_ALIGN_LEFT)
			{
				pMainFrame->DockControlBar(m_pBar, AFX_IDW_DOCKBAR_LEFT);
			}
			else if (m_pBar->m_dwDockStyle & CBRS_ALIGN_RIGHT)
			{
				pMainFrame->DockControlBar(m_pBar, AFX_IDW_DOCKBAR_RIGHT);
			}
			else if (m_pBar->m_dwDockStyle & CBRS_ALIGN_TOP)
			{
				pMainFrame->DockControlBar(m_pBar, AFX_IDW_DOCKBAR_TOP);
			}
			else if (m_pBar->m_dwDockStyle & CBRS_ALIGN_BOTTOM)
			{
				pMainFrame->DockControlBar(m_pBar, AFX_IDW_DOCKBAR_BOTTOM);
			}
			else if (m_pBar->m_dwDockStyle & CBRS_ALIGN_ANY)
			{
				pMainFrame->DockControlBar(m_pBar);
			}
			return;
		}
	}
	CDockContext::ToggleDocking();
}


IMPLEMENT_DYNAMIC(CSizeDockBar, CDockBar)
