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
// FRAMEWND.CPP
// $Date:   03 Jul 1996 14:03:38  $
// $Revision:   1.1  $
// $Author:   MRC  $
// framewnd.cpp : implementation file
//

#include "mrcstafx.h"
#include "mrcpriv.h"

extern MRC_AUX_DATA afxData;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
//#define _DEBUG_WNDPOS			// provide debug info on the window positioning algorithm
#endif


// dwMRCDockBarMap - table mapping standard ID's to styles
// Exists in MFC30.DLL, but not exported, so have to code it here
// renamed it 'cos there seems to be a difference between MFC4.0 and 4.1
static const DWORD dwMRCDockBarMap[4][2] =
{
        { AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
        { AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
        { AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
        { AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};


/////////////////////////////////////////////////////////////////////////////
// CMRCFrameWndSizeDock

IMPLEMENT_DYNCREATE(CMRCFrameWndSizeDock, CFrameWnd)

CMRCFrameWndSizeDock::CMRCFrameWndSizeDock()
{
}

CMRCFrameWndSizeDock::~CMRCFrameWndSizeDock()
{
	CMRCSizeControlBar::TidyUp();		// tidy up any outstanding control bars...
#ifdef _DEBUG
	CObArray arrWnd;
	GetFloatingBars(arrWnd);  // debug code to see what's still around !
#endif
}


BEGIN_MESSAGE_MAP(CMRCFrameWndSizeDock, CFrameWnd)
        //{{AFX_MSG_MAP(CMRCFrameWndSizeDock)
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMRCFrameWndSizeDock message handlers

// dock bars will be created in the order specified by dwMRCDockBarMap
// this also controls which gets priority during layout
// this order can be changed by calling EnableDocking repetitively
// with the exact order of priority
//------------------------------------------------------------------------------
void CMRCFrameWndSizeDock::EnableDocking(DWORD dwDockStyle, CRuntimeClass *pFloatingClass)
// This is over-ridden primarily because we need to insert our own CDockBar class
// to handle the recalc layout, and this is the place they are created.
//------------------------------------------------------------------------------
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    m_pFloatingFrameClass = pFloatingClass; // protected member
    for (int i = 0; i < 4; i++)
    {
    	if (dwMRCDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY)          // protected
        {
        	CDockBar* pDock = (CDockBar*)GetControlBar(dwMRCDockBarMap[i][0]);
            if (pDock == NULL)
            	{
                	pDock = new CSizeDockBar;
                    if (!pDock->Create(this,
						               WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
                                              dwMRCDockBarMap[i][1], dwMRCDockBarMap[i][0]))
                    {
                    	AfxThrowResourceException();
                    }
                }
        }
    }
}


//------------------------------------------------------------------------------------
void CMRCFrameWndSizeDock::FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle)
// although this is not a virtual function in MFC4.0 and we can't guarantee the override
// 
//------------------------------------------------------------------------------------
{
	CFrameWnd::FloatControlBar(pBar, point, dwStyle);
	pBar->SendMessage(WM_USER_AFTERFLOAT_MSG);		// force update of float style
}




//-----------------------------------------------------------------------------
void CMRCFrameWndSizeDock::TileDockedBars(DWORD dwDockStyle)
// Tiles the bars docked in the specified orientation
//-----------------------------------------------------------------------------
{
	for (int i = 0; i < 4; i++)
    {
    	if (dwMRCDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY)  //protected
        {
        	CSizeDockBar* pDock = (CSizeDockBar*)GetControlBar(dwMRCDockBarMap[i][0]);
            // ASSERT(pDock == NULL || pDock->IsKindOf(RUNTIME_CLASS(CSizeDockBar)));
            if (pDock != NULL && (pDock->m_dwStyle & dwDockStyle))
            {
            	pDock->TileDockedBars();
            }
        }
   	}
}



//-----------------------------------------------------------------------------
void CMRCFrameWndSizeDock::ArrangeFloatingBars(DWORD dwOrient)
//-----------------------------------------------------------------------------
{
	CObArray arrWnd;
	GetFloatingBars(arrWnd);
	ArrangeWindows(arrWnd, dwOrient);
}


//-----------------------------------------------------------------------------
void CMRCMDIFrameWndSizeDock::ArrangeFloatingBars(DWORD dwOrient)
//-----------------------------------------------------------------------------
{
	CObArray arrWnd;
	GetFloatingBars(arrWnd);
	ASSERT (this->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));
 	ASSERT(m_hWndMDIClient != NULL);
	// Use the MDI Client window - not the normal client area
	CWnd * pMDIClientWnd = CWnd::FromHandle(m_hWndMDIClient);
	ArrangeWindowsInWindow (pMDIClientWnd, arrWnd, dwOrient);

	// clear all the MOVED flags for sizeable windows...
	for (int i = arrWnd.GetUpperBound(); i >= 0; i--)
	{
        CSizableMiniDockFrameWnd * pFloatFrame = (CSizableMiniDockFrameWnd *) arrWnd[i]; 
        
		ASSERT(pFloatFrame->IsKindOf(RUNTIME_CLASS(CMiniDockFrameWnd)));
		pFloatFrame->ModifyStyle(CBRS_MOVED_BY_USER, 0);
	}
}


//-----------------------------------------------------------------------------
void CMRCMDIFrameWndSizeDock::ArrangeWindows(CObArray & arrWnd, DWORD dwOrient)
//-----------------------------------------------------------------------------
{
	ASSERT (this->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));
 	ASSERT(m_hWndMDIClient != NULL);
	// Use the MDI Client window - not the normal client area
	CWnd * pMDIClientWnd = CWnd::FromHandle(m_hWndMDIClient);
	ArrangeWindowsInWindow (pMDIClientWnd, arrWnd, dwOrient);
}



//-----------------------------------------------------------------------------
void CMRCFrameWndSizeDock::ArrangeWindows(CObArray & arrWnd, DWORD dwOrient)
//-----------------------------------------------------------------------------
{
	ArrangeWindowsInWindow (this, arrWnd, dwOrient);
}


//-----------------------------------------------------------------
void CMRCFrameWndSizeDock::GetFloatingBars(CObArray & arrWnd)
// Appends the floating bars, visible bars to an array
//-----------------------------------------------------------------
{
	CPtrList & listControlBars = m_listControlBars;
	
	POSITION pos = listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar = (CControlBar*)listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		if (!pBar->IsDockBar() && pBar->IsFloating() && pBar->IsVisible()) 	// not a dockbar and floating....
		{
			ASSERT(pBar->m_pDockBar != NULL);
			CWnd * pFloatFrame = ((CWnd *)pBar->m_pDockBar)->GetParent(); 
			ASSERT(pBar != NULL);
			arrWnd.Add(pFloatFrame);
		}
	}
}


//------------------------------------------------------------------------
void CMRCFrameWndSizeDock::DestroyDynamicBars()
// Destroys the dynamic bars in an application
//------------------------------------------------------------------------
{
	CPtrList & listControlBars = m_listControlBars;
	
	CObArray arrBars;
	CMRCSizeControlBar* pBar;
	
	// pass through the list and build an array of bars to destroy
	POSITION pos = listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		pBar = (CMRCSizeControlBar *) listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		if (pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)) &&
				(pBar->m_Style & SZBARF_AUTOTIDY) != 0)
			arrBars.Add(pBar);
	}

	// now destroy these bars...
	for (int i = arrBars.GetUpperBound(); i >= 0; i--)
	{
		pBar = (CMRCSizeControlBar *) arrBars[i];
		pBar->DestroyWindow();
	}
}


struct BarSizeSaveInfo 
{
    CSize   FloatSize;          // floating size
	CSize	HorzDockSize;       // size when docked horizontally
    CSize   VertDockSize;       // size when docked vertically
	BOOL	bMDIFloating;		// floating in an MDI child window
};



//------------------------------------------------------------------------
void CMRCFrameWndSizeDock::SaveSizeBarState(LPCTSTR pszProfileName)
//------------------------------------------------------------------------
{
	DestroyDynamicBars();			// remove bars allocated dynamically
									// - we reload these at present
	
	CFrameWnd::SaveBarState(pszProfileName);	// save the raw states
#ifdef _VERBOSE_TRACE
	TRACE("Loading Bar Sizes\n");
#endif
	SaveBarSizes(pszProfileName, TRUE);			// save additional info
	AfxGetApp()->WriteProfileInt(pszProfileName, REG_VERSION, REG_VERSION_NO);
}



//------------------------------------------------------------------------
void CMRCFrameWndSizeDock::LoadSizeBarState(LPCTSTR pszProfileName)
//------------------------------------------------------------------------
{
	// check the registry version. If it doesn't match, delete the previous key and return
	// this prevents us trying to load registry info from a previous version. This means that
	// bar states will be restored, but dockbars, etc will not.
	if (AfxGetApp()->GetProfileInt(pszProfileName, REG_VERSION, 0) != REG_VERSION_NO)
	{
		WriteProfileString(pszProfileName, NULL, NULL);		// this deletes this key from the registry
		return;
	}
	CFrameWnd::LoadBarState(pszProfileName);
#ifdef _VERBOSE_TRACE
	TRACE("Loading Bar Sizes\n");
#endif
	SaveBarSizes(pszProfileName, FALSE);		// load the sizes back 

	// Clear the dockbars' hidden lists to prevent interference with recalc layout
	for (int i = 0; i < 4; i++)
	{
		CSizeDockBar* pDock = (CSizeDockBar*)GetControlBar(dwMRCDockBarMap[i][0]);
		if (pDock != NULL)
		{
			ASSERT(pDock->IsKindOf(RUNTIME_CLASS(CSizeDockBar)));
			pDock->m_arrHiddenBars.RemoveAll();
		}
	}

}


//------------------------------------------------------------------------
void CMRCFrameWndSizeDock::SaveBarSizes(LPCTSTR pszSection, BOOL bSave)
// Saves all the sizeable bars info
// uses the "ID" of the bar as a key. The bar will already exist on a 
// load, so this seems safe enough
//------------------------------------------------------------------------
{
	
	struct BarSizeSaveInfo BSI;
	CMRCSizeControlBar* pBar;
	char szBarId[20] = "BarSize_";

	CPtrArray arrFloatingBars;
	
	POSITION pos = m_listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		pBar = (CMRCSizeControlBar *) m_listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		if (pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)))
		{
			UINT nID = pBar->GetDlgCtrlID();
			_itoa(nID, szBarId + 8, 10);

			if (bSave)
			{
				BSI.VertDockSize 	= pBar->m_VertDockSize;
				BSI.HorzDockSize	= pBar->m_HorzDockSize;
				BSI.FloatSize 		= pBar->m_FloatSize;
				BSI.bMDIFloating = FALSE;
				// if floating in a MDI Float window.
				CFrameWnd * pBarFrame = pBar->GetDockingFrame();
				if (pBarFrame != NULL && pBarFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)))
				{
					ASSERT(pBar->IsFloating());
					BSI.bMDIFloating = TRUE;
				}
				
				MRCWriteProfileBinary (pszSection, szBarId, &BSI, sizeof BSI);
			}
			else
			{
				if (MRCGetProfileBinary (pszSection, szBarId, &BSI, sizeof BSI))
				{
					pBar->m_VertDockSize	= BSI.VertDockSize;
					pBar->m_HorzDockSize	= BSI.HorzDockSize;
					pBar->m_FloatSize 		= BSI.FloatSize;
					
					// Now have to set the actual window size. The reason for this is
					// that the Adjustment algorithm looks at actual window rect sizes, so
					// it doesn't have to worry about borders etc.
					CSize NewSize = pBar->CalcFixedLayout(FALSE, (pBar->m_dwStyle & CBRS_ORIENT_HORZ));
					pBar->SetWindowPos(0, 0, 0, NewSize.cx, NewSize.cy, 
								SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOMOVE);
					if (pBar->IsFloating())
					{
						if (BSI.bMDIFloating)	// floating in an MDI frame - do the float
						{
							// have to cast to CMRCMDIFrameWndSizeDock - as this is a CFrameWnd function
							ASSERT(this->IsKindOf(RUNTIME_CLASS(CMRCMDIFrameWndSizeDock)));
							arrFloatingBars.Add(pBar);
						}
						else
						{
							CFrameWnd * pFrame = pBar->GetParentFrame();
							if (pFrame != NULL)
								pFrame->RecalcLayout();
						}
					}
				}
#ifdef _VERBOSE_TRACE
				TRACE("Bar ID=%d, Floating(%d,%d), HorzDocked(%d,%d), VertDocked(%d.%d)\n",
						nID,
						BSI.FloatSize.cx,	BSI.FloatSize.cy,
						BSI.VertDockSize.cx, BSI.VertDockSize.cy,
						BSI.HorzDockSize.cx, BSI.HorzDockSize.cy);
#endif
			
			}

#ifdef _VERBOSE_TRACE
			CString strTitle;
			pBar->GetWindowText(strTitle);
			TRACE("%s '%s' ID=%d Float(%d,%d) Horz(%d,%d) Vert(%d,%d)\n",
					LPCTSTR(pBar->GetRuntimeClass()->m_lpszClassName),
					LPCTSTR(strTitle), nID,
					pBar->m_FloatSize.cx, pBar->m_FloatSize.cy,
					pBar->m_HorzDockSize.cx,  pBar->m_HorzDockSize.cy,
					pBar->m_VertDockSize.cx,  pBar->m_VertDockSize.cy);
#endif
		}
	}

	RecalcLayout();			// recalc the layout - so we end up with a meaningful set of bars
	if (!bSave)
	{
		for (int i = 0; i < arrFloatingBars.GetSize(); i++)
		{
			pBar = (CMRCSizeControlBar *) arrFloatingBars[i];
			ASSERT(pBar->m_pDockContext != NULL);
			((CMRCMDIFrameWndSizeDock *)this)->FloatControlBarInMDIChild(pBar,
											pBar->m_pDockContext->m_ptMRUFloatPos);
		}
	}
}





//-----------------------------------------------------------------
// private class - don't want this exported !
//-----------------------------------------------------------------
class CWndSpaceElt : public CObject
{
DECLARE_DYNAMIC(CWndSpaceElt);
public:
	WORD ht;
	WORD wd;

};

IMPLEMENT_DYNAMIC(CWndSpaceElt, CObject);



//-----------------------------------------------------------------
// helper function to find list position
void PositionInSpcList(CWnd *pWnd, CObList & SpcList, DWORD dwOrient,
								CWnd * pParentWnd, CSize & ParentSize, HDWP hDwp)	
//-----------------------------------------------------------------
{
	CRect WndRect;
	pWnd->GetWindowRect(&WndRect);		// external dimensions of the window
	CSize WndSize;
	WndSize = WndRect.Size();			// size of rectangle
   	POSITION pos;

#ifdef _DEBUG
	CString strTitle;
	pWnd->GetWindowText(strTitle);
#ifdef _DEBUG_WNDPOS
	TRACE("Inserting Window: %s, cx = %d, cy = %d\n", LPCTSTR(strTitle), WndSize.cx, WndSize.cy);
#endif
	pos = SpcList.GetHeadPosition();
	int nTotalHeightBefore = 0;
	while (pos != NULL)
	{
		CWndSpaceElt * pSpcElt = (CWndSpaceElt *)SpcList.GetNext(pos);
		ASSERT(pSpcElt != NULL);
#ifdef _DEBUG_WNDPOS
		TRACE("    ht= %d w=%d\n", pSpcElt->ht, pSpcElt->wd);
#endif
		nTotalHeightBefore += pSpcElt->ht;
	}
	ASSERT(nTotalHeightBefore == ParentSize.cy);		
#endif

	int nHt = WndSize.cy;			// height of window....
	int nWd = WndSize.cx;			// width of window (used below)
	int nHtLeft;

	int nCurY = 0;					// current Y position of scan
	int nMinX = 0xffff;				// minimum X position found so far;
	int nMinY;						// again should be ok...
	
	POSITION MinListPos = NULL;		// position in the list with this minimum X;
	
	pos = SpcList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION posCurrent = pos;
		CWndSpaceElt * pSpcElt = (CWndSpaceElt *)SpcList.GetNext(pos);
		ASSERT(pSpcElt != NULL);
		ASSERT_VALID(pSpcElt);
		
		// if we inserted in this position, what would the width be ? 
		// Set nHtLeft, ThisPosX accordingly....
		nHtLeft = nHt;
		int nThisX = 0;
		POSITION posLoop = posCurrent;
		while (posLoop != NULL) 
		{
			CWndSpaceElt * pLoopSpcElt = (CWndSpaceElt *)SpcList.GetNext(posLoop);
			nThisX = max (nThisX, pLoopSpcElt->wd);
			if (nThisX > nMinX)
				break;		// give up if we're already beyond the current minimum
			nHtLeft -= pLoopSpcElt->ht;
			if (nHtLeft <= 0)
			{
				if (nThisX < nMinX)
				{
					nMinX = nThisX;
					MinListPos = posCurrent;	// acutually the position after the current index in the list
					nMinY = nCurY;
				}
				break;
			}
		}
		
   		nCurY += pSpcElt->ht;	// update current Y position.
   	}
	
	
	if (MinListPos == NULL || nMinX > ParentSize.cx)	// window wouldn't fit anywhere in the window cleanly, 
	{
#ifdef 	_DEBUG_WNDPOS
		TRACE("No insert position found\n");
#endif
			return;				// ignore this for now
	}

	ASSERT(MinListPos != NULL && nMinX < 0xffff);
	
	// work out the new position for the window
	// Might want to delay window positioning in future
	CPoint WndPt;
	WndPt.x = ((dwOrient & CBRS_ARRANGE_LEFT) ?  nMinX : ParentSize.cx - nMinX - WndSize.cx);
	WndPt.y = ((dwOrient & CBRS_ARRANGE_TOP)  ?  nMinY : ParentSize.cy - nMinY - WndSize.cy);
	ASSERT(WndPt.y >= -1);
	
#ifdef _DEBUG_WNDPOS
	TRACE("Positioning at: (%d, %d) nMinY=%d, nMinX=%d\n", WndPt.x, WndPt.y, nMinY, nMinX);
#endif
	
	// if not child of requested window, convert co-ords to Screen
	if ((pWnd->GetStyle() & WS_POPUP) || pWnd->GetParent() != pParentWnd)
		pParentWnd->ClientToScreen(&WndPt);
	

	CRect rcWnd;
	pWnd->GetWindowRect(rcWnd);
	// attempt to optimize by only moving windows that have changed position...
	if (rcWnd.TopLeft() != WndPt || rcWnd.Size() != WndSize)
	{
		if (hDwp == NULL)
  			pWnd->SetWindowPos(NULL, WndPt.x, WndPt.y, WndSize.cx, WndSize.cy,
								SWP_NOSIZE | SWP_NOZORDER);
		else
			::DeferWindowPos(hDwp, pWnd->m_hWnd, NULL, WndPt.x,  WndPt.y, WndSize.cx, WndSize.cy,
								SWP_NOSIZE | SWP_NOZORDER);
	}
	
	// now update the SpcList.
	nHtLeft = nHt;
	ASSERT(nHt > 0);
	pos = MinListPos;
	ASSERT(MinListPos != NULL);	// can't actually happen
	CWndSpaceElt * pSpcElt;
	POSITION InsertPos = NULL;
	while (pos != NULL)
	{
		POSITION Oldpos = pos;
		pSpcElt = (CWndSpaceElt *) SpcList.GetNext(pos);

		ASSERT_VALID(pSpcElt);
		if (pSpcElt->ht > nHtLeft)
		{		
			pSpcElt->ht -= nHtLeft;			
			nHtLeft = 0;
			InsertPos = Oldpos;		// position to insert before
			break;
		}
		nHtLeft -= pSpcElt->ht;
		
		CWndSpaceElt * pOldElt = (CWndSpaceElt *)SpcList.GetAt(Oldpos);
		ASSERT(pOldElt != NULL && pOldElt->IsKindOf(RUNTIME_CLASS(CWndSpaceElt)));
		SpcList.RemoveAt(Oldpos);				// remove that element
		
		ASSERT(pSpcElt != NULL && pSpcElt->IsKindOf(RUNTIME_CLASS(CWndSpaceElt)));
		delete pOldElt;
	}
	ASSERT(nHtLeft == 0);
	
	// should now be looking at the element we need to shrink...
	// NB: If pos = NULL then we removed to the end of the list...
	pSpcElt = new CWndSpaceElt;
	pSpcElt->wd = nMinX + nWd;
	pSpcElt->ht = nHt;
	if (InsertPos == NULL)
		SpcList.AddTail(pSpcElt);
	else
		SpcList.InsertBefore(InsertPos, pSpcElt);


#ifdef _DEBUG
#ifdef _DEBUG_WNDPOS
	TRACE("After insert:\n");
#endif
	pos = SpcList.GetHeadPosition();
	int nTotalHeightAfter = 0;
	while (pos != NULL)
	{
		CWndSpaceElt * pSpcElt = (CWndSpaceElt *)SpcList.GetNext(pos);
		ASSERT(pSpcElt != NULL);
		ASSERT_VALID(pSpcElt);
		nTotalHeightAfter += pSpcElt->ht;
#ifdef _DEBUG_WNDPOS
		TRACE("    ht= %d w=%d\n", pSpcElt->ht, pSpcElt->wd);
#endif
	}
	
	ASSERT(nTotalHeightAfter == ParentSize.cy);		
	ASSERT(nTotalHeightBefore == nTotalHeightAfter);
#endif
}


int CompareWndRect(const void *elem1, const void *elem2 )
{
	CRect rect1;
	CRect rect2;
	CWnd * pWnd1 = *((CWnd **)elem1);
	CWnd * pWnd2 = *((CWnd **)elem2);
	pWnd1->GetWindowRect(&rect1);
	pWnd2->GetWindowRect(&rect2);
	// array will be sorted into increasing order, so want the larger rectangles first.
	CSize size1 = rect1.Size();
	CSize size2 = rect2.Size();
	return ((size2.cx * size2.cy) - (size1.cx * size1.cy));
}



//--------------------------------------------------------------------------------
void ArrangeWindowsInWindow (CWnd * pParentWnd, CObArray & arrWnd, DWORD dwOrient)
// Arranges the windows within the rectangle of another window.
//--------------------------------------------------------------------------------
{
	if (arrWnd.GetSize() == 0)			// no windows to size.. do nothing
		return;

	CRect ClientRect;
	pParentWnd->GetClientRect(&ClientRect);
	
	CSize ParentSize = ClientRect.Size();
	if (ParentSize.cy == 0)
		return;							// no height => not much we can do	
	
	CObList SpcList;					// list used to keep track of window spacing
	
	// add initial Arrange rectangle to the list;	
	CWndSpaceElt * pSpcElt = new CWndSpaceElt;
	pSpcElt->wd = 0;
	pSpcElt->ht = ClientRect.Height();
	SpcList.AddTail(pSpcElt); 


	// sort array of window positions by size so that we position the largest windows first.
	// this improves the results quite a bit
	CObject ** pArrData = arrWnd.GetData();
	ASSERT(pArrData != NULL);		// shouldn't be NULL as array is non-empty, but check anyway	
	qsort(pArrData, arrWnd.GetSize(), sizeof(CObject *), CompareWndRect); 	
	
	
	HDWP hDWP = BeginDeferWindowPos(arrWnd.GetSize());	   // defer window moves to save on refresh

	// iterate thru all the windows in the list looking for a position to put it
	for (int nWndNo = 0; nWndNo < arrWnd.GetSize(); nWndNo++)
	{
		CWnd * pWnd = (CWnd *)arrWnd[nWndNo];
		ASSERT(pWnd != NULL);
		ASSERT_VALID(pWnd);
		PositionInSpcList(pWnd, SpcList, dwOrient, pParentWnd, ParentSize, hDWP);
	} 

	if (hDWP != NULL)
		::EndDeferWindowPos(hDWP);		// move the windows

	// Remove elements from the SpcList;
	while (!SpcList.IsEmpty())
	{
		CWndSpaceElt * pElt = (CWndSpaceElt *) SpcList.GetTail();
		delete pElt;
		SpcList.RemoveTail();
	}
}




////////////////////////////////////////////////////////////////////////////////
// Diagnostics
#ifdef _DEBUG
void CMRCFrameWndSizeDock::AssertValid() const
{
        CFrameWnd::AssertValid();
}


void CMRCFrameWndSizeDock::Dump(CDumpContext& dc) const
{
        CFrameWnd::Dump(dc);
        // dc << "\nCMRCFrameWndSizeDock - dockbars";
        // for (int i = 0; i < 4; i++)
        // {
        // GetControlBar is not a const function, so we can;t use it here ... bugger..
        //      // CSizeDockBar* pDock = (CSizeDockBar*)GetControlBar(dwMRCDockBarMap[i][0]);      //protected
        //      if (pDock != NULL)
		//      {
        //              pDock->Dump(dc);
        //      }
        // }
}
#endif




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// CMRCMDIFrameWndSizeDock frame
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMRCMDIFrameWndSizeDock, CMDIFrameWnd)

CMRCMDIFrameWndSizeDock::CMRCMDIFrameWndSizeDock()
{
}

CMRCMDIFrameWndSizeDock::~CMRCMDIFrameWndSizeDock()
{
	CMRCSizeControlBar::TidyUp();		// tidy up any outstanding control bars...
#ifdef _DEBUG
	CObArray arrWnd;
	GetFloatingBars(arrWnd);  // debug code to see what's still around !
#endif
}




BEGIN_MESSAGE_MAP(CMRCMDIFrameWndSizeDock, CMDIFrameWnd)
        //{{AFX_MSG_MAP(CMRCMDIFrameWndSizeDock)
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//-----------------------------------------------------------------------------------------------------
void CMRCMDIFrameWndSizeDock::FloatControlBarInMDIChild(CControlBar* pBar, CPoint point, DWORD dwStyle)
// float a control bar in an MDI Child window
// pBar = bar to float
// point = position in screen co-ordinates
//-----------------------------------------------------------------------------------------------------
{
	ASSERT(pBar != NULL);

	// point is in screen co-ords - map to client
	::ScreenToClient(m_hWndMDIClient, &point);

	// clip to client MDI client rectangle - ensures it's going to be visible
	CRect rcMDIClient;
	::GetClientRect(m_hWndMDIClient, &rcMDIClient);
	point.x = min (point.x, rcMDIClient.right - 32);
	point.x = max (point.x, rcMDIClient.left);		
	point.y = min (point.y, rcMDIClient.bottom - 20);
	point.y = max (point.y, rcMDIClient.top);		
	
	// If the bar is already floating in an MDI child, then just move it
	// MFC has a similar optimization for CMiniDockFrameWnd 
	if (pBar->m_pDockSite != NULL && pBar->m_pDockBar != NULL)
	{
		CDockBar* pDockBar = pBar->m_pDockBar;
		ASSERT(pDockBar->IsKindOf(RUNTIME_CLASS(CDockBar)));
		CFrameWnd* pDockFrame = (CFrameWnd*)pDockBar->GetParent();
		ASSERT(pDockFrame != NULL);
		ASSERT(pDockFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
		if (pDockFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)))
		{
		 	// already a floating as an MDI child, so just move it.
			if (pDockBar->m_bFloating && pDockBar->GetDockedCount() == 1 &&
				(dwStyle & pDockBar->m_dwStyle & CBRS_ALIGN_ANY) != 0)
		 	{
				pDockFrame->SetWindowPos(NULL, point.x, point.y, 0, 0,
							SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				return;
			}
		}
	}

	// Create a CMRCMDIFloatWnd, and dock the bar into it.
	CMRCMDIFloatWnd * pDockFrame = (CMRCMDIFloatWnd *)(RUNTIME_CLASS(CMRCMDIFloatWnd))->CreateObject();
	ASSERT(pDockFrame != NULL);
	if (!pDockFrame->Create(this, dwStyle))
		AfxThrowResourceException();
	
	pDockFrame->SetWindowPos(NULL, point.x, point.y, 0, 0,
		SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	
	if (pDockFrame->m_hWndOwner == NULL)
		pDockFrame->m_hWndOwner = pBar->m_hWnd;

	// Gets the dockbar created by the CMRCMDIFloatWnd
	CDockBar* pDockBar = (CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
	ASSERT(pDockBar != NULL);
	ASSERT(pDockBar->IsKindOf(RUNTIME_CLASS(CDockBar)));

	ASSERT(pBar->m_pDockSite == this);
	// if this assertion occurred it is because the parent of pBar was not
	//  initially this CFrameWnd when pBar's OnCreate was called
	// (this control bar should have been created with a different
	//  parent initially)

	pDockBar->DockControlBar(pBar);
	pDockFrame->RecalcLayout();
	pDockFrame->ShowWindow(SW_SHOWNA);
	pDockFrame->UpdateWindow(); 
}	


//--------------------------------------------------------------------------------------------
void CMRCMDIFrameWndSizeDock::UnFloatInMDIChild(CControlBar* pBar, CPoint point, DWORD dwStyle)
// removes the control bar from an MDI floating window, and then floats the bar
//--------------------------------------------------------------------------------------------
{
	ASSERT(pBar != NULL);
	ASSERT(pBar->IsFloating());
	CMRCMDIFloatWnd * pFloatFrame = (CMRCMDIFloatWnd *)pBar->GetParentFrame();
	ASSERT(pFloatFrame->IsKindOf(RUNTIME_CLASS(CMRCMDIFloatWnd)));

	// point at which to float is ignored at present - use the co-ordinates of the current frame
	CRect rcMDIFloat;
	pFloatFrame->GetWindowRect(&rcMDIFloat);
	point = rcMDIFloat.TopLeft();
	
	// This is basically the code from MFC's CFrameWnd::FloatControlBar(), with the
	// test to avoid destroying/creating the floating frame window removed. 
	// Tried explicitly removing the control bar, but this doesn't work as it destroys the
	// CMDIFloatWnd, which in turn kills the child control bar. So need to create the floating
	// frame first, and then dock into this.  

	ASSERT(m_pFloatingFrameClass != NULL);

	CMiniDockFrameWnd* pDockFrame = CreateFloatingFrame(dwStyle);
	ASSERT(pDockFrame != NULL);
	pDockFrame->SetWindowPos(NULL, point.x, point.y, 0, 0,
		SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	if (pDockFrame->m_hWndOwner == NULL)
		pDockFrame->m_hWndOwner = pBar->m_hWnd;

	CDockBar* pDockBar = (CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
	ASSERT(pDockBar != NULL);
	ASSERT(pDockBar->IsKindOf(RUNTIME_CLASS(CDockBar)));

	ASSERT(pBar->m_pDockSite == this);
	// if this assertion occurred it is because the parent of pBar was not
	//  initially this CFrameWnd when pBar's OnCreate was called
	// (this control bar should have been created with a different
	//  parent initially)

	pDockBar->DockControlBar(pBar);
	pDockFrame->RecalcLayout();
	pDockFrame->ShowWindow(SW_SHOWNA);
	pDockFrame->UpdateWindow();
}


#ifdef _DEBUG
void CMRCMDIFrameWndSizeDock::AssertValid() const
{
        CMDIFrameWnd::AssertValid();
}


void CMRCMDIFrameWndSizeDock::Dump(CDumpContext& dc) const
{
        CMDIFrameWnd::Dump(dc);
        dc << "\nCMRCMDIFrameWndSizeDock - dockbars";
}
#endif



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// CMRCMDIFrameWndSizeDock frame
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMRCMDIChildWndSizeDock, CMDIChildWnd)

CMRCMDIChildWndSizeDock::CMRCMDIChildWndSizeDock()
{
}

CMRCMDIChildWndSizeDock::~CMRCMDIChildWndSizeDock()
{
//	CMRCSizeControlBar::TidyUp();		// tidy up any outstanding control bars...// ??
}


BEGIN_MESSAGE_MAP(CMRCMDIChildWndSizeDock, CMDIChildWnd)
        //{{AFX_MSG_MAP(CMRCMDIChildWndSizeDock)
                // NOTE - the ClassWizard will add and remove mapping macros here.
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()



//-------------------------------------------------------------------------
BOOL MRCWriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, 
										LPVOID pData, DWORD nBufferSize)
// Write a binary value into the registry. If the pointer to the buffer
// is NULL then the current value is deleted. This can be generally used
// for removing any value not just binary ones
//-------------------------------------------------------------------------
{
	CWinApp * pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(pApp->m_pszRegistryKey != NULL); // We must be using the registry not 
									  // INI files for binary to be supported
	LONG lRes;
	
	HKEY hSecKey = pApp->GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	if (pData == NULL)
	{
		lRes = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
	}
	else
	{
		lRes = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
								(LPBYTE)pData, nBufferSize);
	}
	RegCloseKey(hSecKey);
	return (lRes == ERROR_SUCCESS) ? TRUE : FALSE;
}


//-------------------------------------------------------------------------
BOOL MRCGetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
					LPVOID pData, DWORD nBufferSize)
// Read the registry for a binary value. 
// Various assertions fail if the size of the value does not match that
// which is asked for. We can assume that the registry data should always
// match the current software. Any conversion required between versions
// should have taken place when initializing the app 
//-------------------------------------------------------------------------
{
	CWinApp * pApp = AfxGetApp();
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(pApp->m_pszRegistryKey != NULL); // We must be using the registry not INI files 
									 //for binary to be supported

	HKEY hSecKey = pApp->GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;
	DWORD dwType;
	DWORD dwCount = nBufferSize;
	LONG lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, (unsigned long *)&dwType,
					(LPBYTE)pData, (unsigned long *)&dwCount);
	RegCloseKey(hSecKey);
	ASSERT(lRes != ERROR_MORE_DATA); // Is Data in the registry larger than the buffer?
		 							 // We should have converted registry data on start up.
	if (lRes == ERROR_SUCCESS)
	{
		ASSERT(dwType == REG_BINARY);
		ASSERT(dwCount = nBufferSize); // The data should be the expected size
		return TRUE;
	}
	return FALSE;
}



//-----------------------------------------------------------------------------------------
LPVOID MRCGetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, int * pBytesRead)
// similar to function above, but allocates a block of data to read into and returns
// a pointer to it. User must delete this area when no longer required
//-----------------------------------------------------------------------------------------
{
	CWinApp * pApp = AfxGetApp();
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(pApp->m_pszRegistryKey != NULL); // We must be using the registry not INI files 


	HKEY hSecKey = pApp->GetSectionKey(lpszSection);
	if (hSecKey == NULL)
    	return NULL;
	
	LPBYTE lpValue = NULL;
	DWORD dwType, dwCount;
	LONG lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, (unsigned long *)&dwType, NULL, (unsigned long *)&dwCount);
	if (lRes == ERROR_SUCCESS)
	{
		lpValue = new BYTE[dwCount];
		ASSERT(dwType == REG_BINARY);
		lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, (unsigned long *)&dwType, lpValue, (unsigned long *)&dwCount);
		if (lRes != ERROR_SUCCESS)
		{
			delete lpValue;			// error reading - deallocate the memory
   			lpValue = NULL;
   		}
	}
	RegCloseKey(hSecKey);

	if (pBytesRead != NULL)	   		// return length if we want this
		*pBytesRead = dwCount;

	return lpValue;
}


void CMRCMDIFrameWndSizeDock::OnSysColorChange() 
{
	afxData.UpdateSysColors();
	CMDIFrameWnd::OnSysColorChange();
}


void CMRCFrameWndSizeDock::OnSysColorChange() 
{
	afxData.UpdateSysColors();		
	CFrameWnd::OnSysColorChange();
}


void ForceLayoutAdjust(CControlBar * pBar)
// if control bar supplied, then set just resize it
{
	CDockBar * pDockBar;
	ASSERT(pBar != NULL);
	pDockBar = pBar->m_pDockBar;
	if (pDockBar!= NULL && pDockBar->IsKindOf(RUNTIME_CLASS(CSizeDockBar)))
		((CSizeDockBar *)pDockBar)->m_CountBars = 0;			
}
