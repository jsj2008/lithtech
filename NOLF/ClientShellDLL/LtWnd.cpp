/****************************************************************************
;
;	 MODULE:		LTWnd (.cpp)
;
;	PURPOSE:		LithTech Window class
;
;	HISTORY:		11/30/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/

#include "StdAfx.h"
#include "MouseMgr.h"
#include "LTWnd.h"


#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "SoundMgr.h"
extern CGameClientShell* g_pGameClientShell;

// Externs
extern HWND g_hMainWnd;
extern RECT *g_prcClip;

// Statics...

CLTWnd* CLTWnd::s_pWndActive = NULL;
CLTWnd* CLTWnd::s_pMainWnd = NULL;
CLTWnd* CLTWnd::s_pWndCapture = NULL;
CLTWnd* CLTWnd::s_pLastMouseWnd = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::InitFromBitmap
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd, char* szBitmap, int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized)
		return FALSE;

	// We will be deleting the surface
	m_bDeleteSurfOnTerm = TRUE;

	// Create the surface
	HSURFACE hSurf = g_pInterfaceResMgr->GetSharedSurface(szBitmap);
//  HSURFACE hSurf = g_pLTClient->CreateSurfaceFromBitmap(szBitmap);
	if(!hSurf)
	{
		TRACE("CLTWnd::InitFromBitmap - ERROR - Could not create the surface: ""%s""\n",szBitmap);
		return FALSE;
	}

	if(!Init(nControlID,szWndName,pParentWnd,hSurf,xPos,yPos,dwFlags,dwState))
	{
		TERMSHAREDSURF(hSurf);
		return FALSE;
	}

	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized)
		return FALSE;

	// Set the simple variables
	m_nControlID = nControlID;
	m_dwFlags = dwFlags;
	m_sWndName = szWndName;
	m_xCursor = 0;
	m_yCursor = 0;
	m_xCursorClick = 0;
	m_yCursorClick = 0;
	m_dwState = dwState;
	m_hColorTransparent = g_hColorTransparent;

	// xPos and yPos are offsets from the parent window
	m_xPos = xPos;
	m_yPos = yPos;

	DWORD dwParentState = LTWS_NORMAL;

	// If we have a parent, then add us as a child
	if (pParentWnd)
	{
		pParentWnd->AddChild(this);
		m_pParentWnd = pParentWnd;
		dwParentState = pParentWnd->GetState();
	}
	else
	{
		// Otherwise, we're the main window
		if(s_pMainWnd)
		{
			TRACE("CLTWnd::Init - ERROR - There already exists a main window (without a parent)\n");
			return FALSE;
		}

		m_xPos = xPos;
		m_yPos = yPos;
		m_pParentWnd = NULL;
		s_pMainWnd = this;
	}

	// Set up the surface variables
	m_hSurf = hSurf;
	if (hSurf)
	{
		DWORD x, y;
        g_pLTClient->GetSurfaceDims(hSurf, &x, &y);
		m_nSurfWidth = m_nWidth = (int)x;
		m_nSurfHeight = m_nHeight = (int)y;

		if((m_xPos == COORD_CENTER) || (m_yPos == COORD_CENTER))
		{
			// We want centerage
			if(m_pParentWnd)
			{
				DWORD xParent, yParent;
                g_pLTClient->GetSurfaceDims(m_pParentWnd->GetSurface(),&xParent,&yParent);
				m_xPos = (xParent-x)/2;
				m_yPos = (yParent-y)/2;
			}
			else
			{
				m_xPos = 0;
				m_yPos = 0;
			}
		}
	}

	// All done!
	m_bInitialized = TRUE;

	if(IsFlagSet(LTWF_DISABLED))
		EnableWindow(FALSE);

	if((m_dwState & LTWS_CLOSED))// why was this here? || (dwParentState & (LTWS_CLOSED | LTWS_MINIMIZED)))
		ShowWindow(FALSE,FALSE,FALSE);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTWnd::Term()
{
	// Sanity Check
	if(!m_bInitialized)
		return;

	// All children should die... uh... I mean all of our child windows should be removed.
	RemoveAllChildren();

	// Delete the surface if necessary
	if(m_bDeleteSurfOnTerm)
	{
		FreeAllSurfaces();
	}

	// If we've got the capture, release it
	if(s_pWndCapture == this)
		s_pWndCapture = NULL;
	if(s_pWndActive == this)
		s_pWndActive = NULL;
	if(s_pMainWnd == this)
		s_pMainWnd = NULL;

	m_nWidth = m_nSurfWidth = m_nHeight = m_nSurfHeight = 0;

	m_bInitialized = FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::RemoveAllChildren
//
//	PURPOSE:	Deletes and removes all of our child windows
//
// ----------------------------------------------------------------------- //
void CLTWnd::RemoveAllChildren()
{
	POSITION pos = m_lstChildren.GetHeadPosition();
	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetNext(pos);

		// Terming the window will take care of removing all of it's children
        if (pWnd)
        {
            debug_delete(pWnd);
            pWnd = NULL;
        }
	}

	m_lstChildren.RemoveAll();

	 // Remove us from parent window list
	RemoveFromParent();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::ShowAllChildren
//
//	PURPOSE:	Shows or hides all child windows
//
// ----------------------------------------------------------------------- //
void CLTWnd::ShowAllChildren(BOOL bShow)
{
	POSITION pos = m_lstChildren.GetHeadPosition();
	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetNext(pos);
		ASSERT(pWnd);

		// Show us
		pWnd->ShowWindow(bShow);

		// And all of our children
		pWnd->ShowAllChildren(bShow);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::SetSurface
//
//	PURPOSE:	Changes the main surface
//
// ----------------------------------------------------------------------- //
HSURFACE CLTWnd::SetSurface(HSURFACE hSurf, BOOL bDeleteSurf)
{
	if(hSurf == m_hSurf)
		return m_hSurf;

	// Get the dims of the current surface
	HSURFACE hSurfTemp = m_hSurf;
	m_hSurf = hSurf;
	DWORD x, y;
    g_pLTClient->GetSurfaceDims(hSurf, &x, &y);

/*#ifdef _DEBUG
	if(((int)x != m_nSurfWidth) || ((int)y != m_nSurfHeight))
	{
		TRACE("CLTWnd::SetSurf - WARNING - Surface dimensions changed from %d,%d to %d,%d!\n",m_nSurfWidth,m_nSurfHeight,x,y);
	}
#endif*/

	m_nSurfWidth = m_nWidth = (int)x;
	m_nSurfHeight = m_nHeight = (int)y;

	// Delete the old surface if so desired
	if (bDeleteSurf)
	{
		TERMSHAREDSURF(hSurfTemp);
		return NULL;
	}
	return hSurfTemp;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::SetTransparentColor
//
//	PURPOSE:	Sets the transparent color of the window
//
// ----------------------------------------------------------------------- //
void CLTWnd::SetTransparentColor(float r, float g, float b)
{
    m_hColorTransparent = g_pLTClient->CreateColor(r, g, b, FALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::DeleteTransparentColor
//
//	PURPOSE:	Frees the transparent color of the window
//
// ----------------------------------------------------------------------- //
void CLTWnd::DeleteTransparentColor()
{
    g_pLTClient->DeleteColor(m_hColorTransparent);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::EnableWindow
//
//	PURPOSE:	Enables/disables a window to receive input
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::EnableWindow(BOOL bEnable)
{
	BOOL bRet;
	if (!m_bEnabled)
	{
		bRet = TRUE;

		if(!bEnable)
		{
			// If we had the focus and became disabled, we need to pass the focus on
			if(s_pWndActive == this)
			{
				s_pWndActive = NULL;
				/////////////////////////////////////////////////////
				// TODO:  Change this to give the focus to the parent
				/////////////////////////////////////////////////////
				/*ASSERT(s_pMainWnd);
				if(!s_pMainWnd->m_lstChildren.IsEmpty())
				{
					CLTWnd* pWnd = (CLTWnd*)s_pMainWnd->m_lstChildren.GetTail();
					ASSERT(pWnd);
					pWnd->SetFocus();
				}*/
			}
		}

	}

	m_bEnabled = bEnable;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::ShowWindow
//
//	PURPOSE:	Shows or hides the window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
	BOOL bRet = FALSE;
	if(m_bVisible)
	{
		bRet = TRUE;

		if(!bShow)
		{
			// We're going invisible
			m_dwState |= LTWS_CLOSED;

			// Play a sound if necessary
			if(bPlaySound && m_sHideSound.GetLength())
				g_pClientSoundMgr->PlayInterfaceSound((char *)(LPCSTR)m_sHideSound);

		}
		else
		{
			m_dwState &= ~LTWS_CLOSED;
		}
	}
	else
	{
		if(bShow)
		{
			// going visible
			m_dwState &= ~LTWS_CLOSED;

			// Play a sound if necessary
			if(bPlaySound && m_sShowSound.GetLength())
				g_pClientSoundMgr->PlayInterfaceSound((char *)(LPCSTR)m_sShowSound);

		}
		else
		{
			m_dwState |= LTWS_CLOSED;
		}
	}

	m_bVisible = bShow;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetWindowPos
//
//	PURPOSE:	Sets the window position
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::SetWindowPos(CLTWnd* pInsertAfter, int xPos, int yPos, int nWidth, int nHeight, DWORD dwFlags)
{
	if(pInsertAfter)
	{
		CLTWnd* pParentWnd = m_pParentWnd;
		RemoveFromParent();
		m_pos = pParentWnd->m_lstChildren.InsertBefore(pInsertAfter->GetPos(),this);
		m_pParentWnd = pInsertAfter->GetParent();
	}
	else
	{
		// TODO: Check other insert params
	}

	m_xPos = xPos;
	m_yPos = yPos;

	if(IsFlagSet(LTWF_SIZEABLE))
	{
		// TODO: Resize the surface as necessary
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::MoveWindow
//
//	PURPOSE:	Moves the window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::MoveWindow(int xPos, int yPos, int nWidth, int nHeight, BOOL bReDraw)
{
	m_xPos = xPos;
	m_yPos = yPos;

	if(IsFlagSet(LTWF_SIZEABLE))
	{
		// Resize the surface as necessary
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::MoveWindow
//
//	PURPOSE:	Moves the window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::MoveWindow(CRect& rcPos, BOOL bReDraw)
{
	return(MoveWindow(rcPos.left,rcPos.top,rcPos.Width(),rcPos.Height(),bReDraw));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::GetWindowRect
//
//	PURPOSE:	Gets the window rectangle in screen coordinates
//
//	NOTE:		The values don't necessarily have to be on the screen
//				For example, you might have (-5,-5,645,485) for a window
//				that is larger than the 640x480 screen
//
// ----------------------------------------------------------------------- //
void CLTWnd::GetWindowRect(CRect *pRect)
{
	int x = GetWindowLeft();
	int y = GetWindowTop();

	pRect->SetRect(x, y, x + m_nWidth, y + m_nHeight);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::GetSurfaceRect
//
//	PURPOSE:	Gets the surface rectangle in screen coordinates
//
// ----------------------------------------------------------------------- //
void CLTWnd::GetSurfaceRect(CRect *pRect)
{
	int x = GetWindowLeft() + m_xSurfPos;
	int y = GetWindowTop() + m_ySurfPos;
	pRect->SetRect(x, y, x + m_nSurfWidth, y + m_nSurfHeight);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::SetFocus
//
//	PURPOSE:	Makes this window the active window
//
// ----------------------------------------------------------------------- //
CLTWnd* CLTWnd::SetFocus()
{
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pParent)
	{
		// Move us to the front of our parent's list
		pParent->m_lstChildren.RemoveAt(pWnd->GetPos());
		pParent->AddChild(pWnd);
		// Do the same for our parent
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}
	// If we're here, pWnd should be the main window
	ASSERT(pWnd == s_pMainWnd);

	// Set pWnd to the window that previously had the focus (for return value)
	pWnd = s_pWndActive;
	s_pWndActive = this;

	return(pWnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::PtInWnd
//
//	PURPOSE:	Tests a point in a window
//
//	NOTE:		The window needs to be visible, but not necessarily enabled
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::PtInWnd(int x, int y)
{
	// Translate the rectangle to screen coordinates
	if(!m_bVisible || !m_bEnabled)
		return FALSE;

	CRect rc;
	GetClipRect(&rc);

	return(PtInRect(&rc,x,y));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::Draw
//
//	PURPOSE:	Draws the window (and all its child windows to a surface)
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::Draw(HSURFACE hSurf)
{
	if (!m_bVisible || IsFlagSet(LTWF_MANUALDRAW))
		return TRUE;

	if(!DrawToSurface(hSurf))
	{
		TRACE("CLTWnd::Draw FAILED\n");
	}

	POSITION pos = m_lstChildren.GetHeadPosition();
	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetNext(pos);
		ASSERT(pWnd);
		pWnd->Draw(hSurf);
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::DrawToSurface
//
//	PURPOSE:	Does the dirty work for drawing a single window to a surface
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::DrawToSurface(HSURFACE hSurfDest)
{
	// We don't draw the main window (the only one without a parent)
	if (m_pParentWnd)
	{
		CRect	rcSurf;
		GetSurfaceRect(&rcSurf);

		CRect	rcClip;
		GetClipRect(&rcClip);

		CRect	rcSource;

		// check for intersection of the clip area and the surface
		if (rcSource.IntersectRect(rcSurf, rcClip))
		{
			// retain the absolute coordinates of the intersection relative to the main drawing window
			CPoint	ptIntersect = rcSource.TopLeft();

			// normalize coordinates relative to child surface
			rcSource -= rcSurf.TopLeft();

			// create a lithtech-compatible rect
            LTRect   drcSource;

			drcSource.left = rcSource.left;
			drcSource.top = rcSource.top;
			drcSource.right = rcSource.right;
			drcSource.bottom = rcSource.bottom;

			// draw the surface
			if (m_hSurf)
			{
				if (IsFlagSet(LTWF_TRANSPARENT))
				{
                    if(g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurfDest,m_hSurf, &drcSource, ptIntersect.x, ptIntersect.y, m_hColorTransparent) != LT_OK)
						return FALSE;
				}
				else
				{
                    if(g_pLTClient->DrawSurfaceToSurface(hSurfDest, m_hSurf, &drcSource, ptIntersect.x, ptIntersect.y) != LT_OK)
						return FALSE;
				}
			}
		}
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::GetWndFromPt
//
//	PURPOSE:	Gets the topmost visible window that the given point is in
//
// ----------------------------------------------------------------------- //
CLTWnd* CLTWnd::GetWndFromPt(int x, int y)
{
	POSITION pos = m_lstChildren.GetTailPosition();

	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetPrev(pos);
		ASSERT(pWnd);
		if(pWnd->PtInWnd(x,y))
		{
			// No need to continue searching backward
			return(pWnd->GetWndFromPt(x,y));
		}
	}

	return this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleMouseMove
//
//	PURPOSE:	Passes a mouse move message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleMouseMove(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnMouseMove(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);

	if (s_pLastMouseWnd && s_pLastMouseWnd != pWnd)
		s_pLastMouseWnd->OnMouseLeave();
	if(pWnd)
	{
		if (s_pLastMouseWnd != pWnd)
			pWnd->OnMouseEnter();
		s_pLastMouseWnd = pWnd;
		return(pWnd->OnMouseMove(xPos,yPos));
	}

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleLButtonDown
//
//	PURPOSE:	Passes a left button down message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleLButtonDown(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnLButtonDown(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);
	if(pWnd)
		return(pWnd->OnLButtonDown(xPos,yPos));

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleLButtonUp
//
//	PURPOSE:	Passes a left button up message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleLButtonUp(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnLButtonUp(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);
	if(pWnd)
		return(pWnd->OnLButtonUp(xPos,yPos));

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleLButtonDblClick
//
//	PURPOSE:	Passes a left button double click message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleLButtonDblClick(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnLButtonDblClick(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);
	if(pWnd)
		return(pWnd->OnLButtonDblClick(xPos,yPos));

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleRButtonDown
//
//	PURPOSE:	Passes a right button down message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleRButtonDown(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnRButtonDown(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);
	if(pWnd)
		return(pWnd->OnRButtonDown(xPos,yPos));

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::HandleRButtonUp
//
//	PURPOSE:	Passes a right button up message on to the appropriate window
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::HandleRButtonUp(int xPos, int yPos)
{
	if(s_pWndCapture)
	{
		return(s_pWndCapture->OnRButtonUp(xPos,yPos));
	}

	CLTWnd* pWnd = GetWndFromPt(xPos,yPos);
	if(pWnd)
		return(pWnd->OnRButtonUp(xPos,yPos));

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::OnLButtonDown
//
//	PURPOSE:	This is the actual left button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	SetFocus();
	s_pWndCapture = this;

	// Find our parent
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	//pWnd->SetFocus();
	//s_pWndCapture = pWnd;

	// If we're draggable, start the draggage
	if(pWnd->IsFlagSet(LTWF_DRAGGABLE))
	{
		pWnd->m_xCursorClick = xPos - pWnd->GetWindowLeft();
		pWnd->m_yCursorClick = yPos - pWnd->GetWindowTop();

		if(pWnd != this)
		{
			// Our parent is draggable, but we're not
			// can our parent be dragged around by us?
			if(!IsFlagSet(LTWF_PARENTDRAG))
			{
				// We can't allow draggage
				return TRUE;
			}
		}

		// Don't allow the mouse to drag us outside of our parent's window
		ASSERT(pWnd->GetParent());
		CRect rcClip;
		// Subtract 1 from the right and the bottom to go from a windows rect to a real rect
		// (0,0,640,480) -> (0,0,639,479)
		pWnd->GetParent()->GetClipRect(&rcClip);
		rcClip.right -= 1;
		rcClip.bottom -= 1;

		g_mouseMgr.SetClipRect(&rcClip,!pWnd->IsFlagSet(LTWF_VDRAG),!pWnd->IsFlagSet(LTWF_HDRAG));

		::MapWindowPoints(g_hMainWnd,NULL,(POINT*)&rcClip,2);
		ClipCursor(&rcClip);

		return TRUE;
	}

	return(this != s_pMainWnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::OnLButtonUp
//
//	PURPOSE:	This is the actual left button up handler
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::OnLButtonUp(int xPos, int yPos)
{
	// Stop any draggage
	//if(s_pWndDrag)
	g_mouseMgr.SetClipRect(NULL);
	ClipCursor(g_prcClip);
	//s_pWndDrag = NULL;
	s_pWndCapture = NULL;
	return(this != s_pMainWnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::OnRButtonDown
//
//	PURPOSE:	This is the actual right button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	SetFocus();
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	if(pParent && !pWnd->IsFlagSet(LTWF_NOCLOSE))
	{
		pWnd->ShowWindow(FALSE);
		return TRUE;
	}

	return(this != s_pMainWnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::OnMouseMove
//
//	PURPOSE:	This is the actual mouse move handler
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::OnMouseMove(int xPos, int yPos)
{
	// Update the cursor's position on us
	m_xCursor = xPos - GetWindowLeft();
	m_yCursor = yPos - GetWindowTop();

	// Handle draggage if necessary
	if(s_pWndCapture == this)
	{
		if(IsFlagSet(LTWF_DRAGGABLE))
		{
			OnDrag(xPos,yPos);
		}
		else
		if(IsFlagSet(LTWF_PARENTDRAG))
		{
			// Drag our parent
			// Find our parent
			CLTWnd* pWnd = this;
			CLTWnd* pParent = m_pParentWnd;
			while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
			{
				pWnd = pParent;
				pParent = pWnd->GetParent();
			}
			pWnd->OnDrag(xPos,yPos);
		}
	}

	return(this != s_pMainWnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::OnDrag
//
//	PURPOSE:	Window draggage
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::OnDrag(int xPos, int yPos)
{
	// Make sure that we haven't left the boundaries of our parent window
	ASSERT(m_pParentWnd);
	ASSERT(m_pParentWnd->PtInWnd(xPos,yPos));
	return(MoveWindow(xPos - m_pParentWnd->GetWindowLeft() - m_xCursorClick,yPos - m_pParentWnd->GetWindowTop() - m_yCursorClick));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::GetClipRect
//
//	PURPOSE:	Gets the clip rect relative to the
//				parent window's client area
//
// ----------------------------------------------------------------------- //
void CLTWnd::GetClipRect(CRect *pRect)
{
	CLTWnd* pParent;

	// get our window rect in screen coordinates
	GetWindowRect(pRect);
	pParent = m_pParentWnd;
	CRect rcParent;

	// Intersect this rect to all parent windows
	while (pParent != NULL)
	{
		pParent->GetWindowRect(&rcParent);
		(*pRect) &= rcParent;
		pParent = pParent->GetParent();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::FreeAllSurfaces
//
//	PURPOSE:	Frees all the surfaces
//
// ----------------------------------------------------------------------- //
void CLTWnd::FreeAllSurfaces(CLTSurfaceArray* pcollSurfs)
{
	if(m_hSurf)
	{
		TERMSHAREDSURF(m_hSurf);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::ArrangeAllChildren
//
//	PURPOSE:	Makes sure that all child windows keep their respective
//				distance from the right and bottom of the screen
//				when the resolution changes
//
// ----------------------------------------------------------------------- //
void CLTWnd::ArrangeAllChildren(DWORD dwWidth, DWORD dwHeight, DWORD dwOldWidth, DWORD dwOldHeight)
{
	CRect rc;
	POSITION pos = m_lstChildren.GetHeadPosition();
	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetNext(pos);
		if(pWnd->GetParent() && !pWnd->IsFlagSet(LTWF_FIXEDCHILD))
		{
			int nDistFromLeft = pWnd->GetWindowLeft();
			int nDistFromTop = pWnd->GetWindowTop();
			int nDistFromRight = dwOldWidth - nDistFromLeft;
			int nDistFromBottom = dwOldHeight - nDistFromTop;

			if(nDistFromLeft > nDistFromRight)
				nDistFromLeft = dwWidth-nDistFromRight;

			if(nDistFromTop > nDistFromBottom)
				nDistFromTop = dwHeight-nDistFromBottom;

			pWnd->MoveWindow(nDistFromLeft,nDistFromTop);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTWnd::Update
//
//	PURPOSE:	Updates all child windows
//
// ----------------------------------------------------------------------- //
BOOL CLTWnd::Update(float fTimeDelta)
{
	if(IsFlagSet(LTWF_NOUPDATE))
		return FALSE;

	if(IsFlagSet(LTWF_TERM))
	{
		Term();
		return FALSE;
	}

	POSITION pos = m_lstChildren.GetHeadPosition();
	while (pos)
	{
		CLTWnd* pWnd = (CLTWnd*)m_lstChildren.GetNext(pos);
		ASSERT(pWnd);
		if(!pWnd->Update(fTimeDelta))
			return FALSE;
	}
	return TRUE;
}