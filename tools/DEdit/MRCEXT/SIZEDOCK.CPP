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
// SIZEDOCK.CPP
// $Date:   12 Jul 1996 14:10:48  $
// $Revision:   1.2  $
// $Author:   JPW  $
// sizedock.cpp : implementation file
// Sizeable Dock Frame Window

#include "mrcstafx.h"
#include "mrcpriv.h"
#include "resource.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSizableMiniDockFrameWnd, CMiniDockFrameWnd)

MRC_AUX_DATA afxData;

////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------
MRC_AUX_DATA::MRC_AUX_DATA()
// constructor ensures we have all the colors to hand
//----------------------------------------------------------------------------------
{
    DWORD dwVersion = ::GetVersion();
	//nWinVer = (LOBYTE(dwVersion) << 8) + HIBYTE(dwVersion);
	//bWin32s = (dwVersion & 0x80000000) != 0;
	bWin4 = (BYTE)dwVersion >= 4;
	
	cxBorder2 = bWin4 ? CX_BORDER*2 : CX_BORDER;
	cyBorder2 = bWin4 ? CY_BORDER*2 : CY_BORDER;

    hcurWait = ::LoadCursor(NULL, IDC_WAIT);
    hcurArrow = ::LoadCursor(NULL, IDC_ARROW);
    hcurSizeWE = NULL;
    hcurSizeNS = NULL;
		    
	UpdateSysColors();
}


MRC_AUX_DATA::~MRC_AUX_DATA()
{
}


//------------------------------------------------------------------------------
void MRC_AUX_DATA::UpdateSysColors()
// Update the cached colors we use. Eventually, need to call this when the
// main frame window receives a WM_SYSCOLORCHANGE
//------------------------------------------------------------------------------
{

    clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
    clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
    clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
    clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
    clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);
}


////////////////////////////////////////////////////////////////////////////////////
// CSplitterRect Functions


//-----------------------------------------------------------------------------------
CSplitterRect::CSplitterRect(int type, const RECT & rect)
// in-line constructor moved out of line as ASSERT causes problems for .DEF file
// (end up with a symbol that contains THIS_FILE, and hence a path name, so we can't
// by portable across directories).
//-----------------------------------------------------------------------------------
{
	ASSERT(type == SPLITTER_VERT || type == SPLITTER_HORZ);
    m_rect = rect; m_type = type;
}; 

//-----------------------------------------------------------------------------------
void CSplitterRect::Draw(CDC *pDC)
//-----------------------------------------------------------------------------------
{
    CRect rect = m_rect;
    switch (m_type)
    {
        case SPLITTER_VERT:
        	rect.left ++;
        	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), afxData.clrBtnFace);
        	rect.left ++;
        	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), afxData.clrBtnHilite);

        	rect.right --;
        	pDC->FillSolidRect(rect.right, rect.top, 1, rect.Height(), afxData.clrWindowFrame);
        	rect.right --;
        	pDC->FillSolidRect(rect.right, rect.top, 1, rect.Height(), afxData.clrBtnShadow);
        	break;


        case SPLITTER_HORZ:
            rect.top ++;
            pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, afxData.clrBtnFace);
            rect.top ++;
            pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, afxData.clrBtnHilite);

            rect.bottom --;
            pDC->FillSolidRect(rect.left, rect.bottom, rect.Width(), 1, afxData.clrWindowFrame);
            rect.bottom --;
            pDC->FillSolidRect(rect.left, rect.bottom, rect.Width(), 1, afxData.clrBtnShadow);
            break;

         default:
            ASSERT(FALSE);
            break;
 	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMRCDragRectTracker - subclassed CMRCRectTracker
class CMRCDragRectTracker : public CMRCRectTracker
{
	void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd);

};

/////////////////////////////////////////////////////////////////////////////
// CSizableMiniDockFrameWnd

BEGIN_MESSAGE_MAP(CSizableMiniDockFrameWnd, CMiniDockFrameWnd)
	//{{AFX_MSG_MAP(CSizableMiniDockFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_NCLBUTTONDBLCLK()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_NCHITTEST()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
    // Global help commands
END_MESSAGE_MAP()

//-------------------------------------------------------------------
int CSizableMiniDockFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
//-------------------------------------------------------------------
{
	if (CMiniDockFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    m_nContainedBarType = Unknown;
	return 1;
}


//------------------------------------------------------------------------------
void AdjustForBorders(CRect& rect, DWORD dwStyle)
// Helper function: used below
// adjusts the size, depending on the borders specified by the CControlBar style
//------------------------------------------------------------------------------
{
	if (dwStyle & CBRS_BORDER_LEFT)
		rect.left -= afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_TOP)
		rect.top -= afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_RIGHT)
		rect.right += afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect.bottom += afxData.cyBorder2;
}


//--------------------------------------------------------------------------
void CSizableMiniDockFrameWnd::OnSize(UINT nType, int cx, int cy)
// respond to the miniframe resizing. If we've got a sizeable control
// bar in the window, then we set it's size. Need to adjust for the
// window borders. The window will then get repositioned by a ReCalcLayout()
//--------------------------------------------------------------------------
{
    if (cx == 0 && cy == 0)
    	return;

    // We don't support CBRS_FLOAT_MULTI
    if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
        // CMiniDockFrameWnd class assumes if there is only 1 bar, then it's at position 1
        // in the array
        CMRCSizeControlBar* pBar = ((CSizeDockBar *)(&m_wndDockBar))->GetFirstControlBar();
        // ignore size request if not visible....
        if (pBar != NULL && IsSizeable(pBar) && pBar->IsVisible())
        {
			CRect rect(0, 0, cx, cy);
    		AdjustForBorders(rect, pBar->m_dwStyle);

            pBar->m_FloatSize.cx = rect.Width();
            pBar->m_FloatSize.cy = rect.Height();
        }
    }
    RecalcLayout();
}


//-------------------------------------------------------------------
BOOL CSizableMiniDockFrameWnd::PreCreateWindow(CREATESTRUCT &cs)
//-------------------------------------------------------------------
{
// modify frame style so it is fully sizeable - we will modify this again later
// if we discover that we have a standard MFC control bar inside the frame
// turn on thick frame styles. MFS_THICKFRAME is what's expected, but also need WS_THICKFRAME,
// as NT expects this to be able to do the resizing.
	cs.style |= MFS_THICKFRAME | WS_THICKFRAME; 
    cs.style &= ~( MFS_MOVEFRAME | MFS_4THICKFRAME );		
    return CMiniDockFrameWnd::PreCreateWindow(cs);
}



//-------------------------------------------------------------------
UINT CSizableMiniDockFrameWnd::OnNcHitTest(CPoint point)
// over-ridden so we can find out the type of the bar in this window
//-------------------------------------------------------------------
{
	enum ContainedBarType Type = GetContainedBarType();
	return CMiniDockFrameWnd::OnNcHitTest(point);
}


//-------------------------------------------------------------------
void CSizableMiniDockFrameWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
// if we've got a CMRCSizeControlBar inside this frame, and it's a resize,
// then we can use the default window's sizing. 
//-------------------------------------------------------------------
{
	enum ContainedBarType Type = GetContainedBarType();
	if (Type == MRCSizeBar)
	{
		if (nHitTest >= HTSIZEFIRST && nHitTest <= HTSIZELAST)
		{
			// special activation for floating toolbars
			ActivateTopParent();
			CMiniFrameWnd::OnNcLButtonDown(nHitTest, point);
			return;
		}
		else if (nHitTest == HTSYSMENU)
		{
			// do the system menu - ie give move,size,hide, etc options as would Win95.						
			//InvertSysMenu();
			CPoint pt(0,0);
			ClientToScreen(&pt);
			CRect rcSysIcon;
			GetWindowRect(&rcSysIcon);
			rcSysIcon.right = rcSysIcon.left + 12;		// NB:hard-coded value for width of system icon
			rcSysIcon.bottom = pt.y;					
			
			CMenu * pSysMenu = GetSystemMenu(FALSE);
			int nCmd = pSysMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this, & rcSysIcon);
			//InvertSysMenu();
			if (nCmd != 0)
			{
				SendMessage(WM_SYSCOMMAND, nCmd, 0);
			}
			return;
		}
	}
	CMiniDockFrameWnd::OnNcLButtonDown(nHitTest, point);
}


//-------------------------------------------------------------------
int CSizableMiniDockFrameWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
//-------------------------------------------------------------------
{
	enum ContainedBarType Type = GetContainedBarType();
	if (Type == MRCSizeBar)
	{
		if (nHitTest >= HTSIZEFIRST && nHitTest <= HTSIZELAST)
			return CMiniFrameWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	}
	return CMiniDockFrameWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}



//-------------------------------------------------------------------
enum CSizableMiniDockFrameWnd::ContainedBarType CSizableMiniDockFrameWnd::GetContainedBarType()
// returns the type of the contained bar
// Floating frames are created from CFrameWnd::CreateFloatingFrame(), and at this point,
// we don't know what type of control bar will eventually be in this frame. This routine
// determines the type of control bar, and sets the neccessary styles. Generally this routine gets
// called as soon as we do a WM_NCHITTEST on the window - ie before the user can get a chance to
// manipulate it with the mouse
//
// CMRCSizeControlBar - default style of WS_THICKFRAME is ok (we've overridden the create too)
//					    but we need to ensure "SC_SIZE" is on the menu, otherwise we can't
//						the default WM_NCLBUTTONDOWN won't generate an SC_SIZE.
//
// CControlBar, CBRS_SIZE_DYNAMIC
//					  - set MFS_4THICKFRAME (disallow diagonal sizing)
//
// CControlBar, not CBRS_SIZE_DYNAMIC
//					  - remove WS_THICKFRAME & add MFS_MOVEFRAME (we don't want sizing hit test values)
//-------------------------------------------------------------------
{
	if (m_nContainedBarType == Unknown)
	{
		m_nContainedBarType = MFCBase;
		if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0) // don't handle this
	    {
	       	CMRCSizeControlBar* pBar = ((CSizeDockBar *)(&m_wndDockBar))->GetFirstControlBar();
	        if (pBar != NULL)
			{
				if (IsSizeable(pBar))
				{		
					m_nContainedBarType = MRCSizeBar;
					GetSystemMenu(TRUE);					// reset the system menu
					
					// delete sys menu items that might be present....
					CMenu* pSysMenu = GetSystemMenu(FALSE);
					pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
					pSysMenu->DeleteMenu(SC_MINIMIZE, MF_BYCOMMAND);
					pSysMenu->DeleteMenu(SC_RESTORE, MF_BYCOMMAND);
					pSysMenu->DeleteMenu(SC_TASKLIST, MF_BYCOMMAND);
					while (pSysMenu->DeleteMenu(0, MF_BYCOMMAND));	// remove anything with ID=0
					
					//pSysMenu->AppendMenu(MF_STRING | MF_ENABLED, SC_SIZE, "&Size");

					// if window is meant to close, ensure sysmenu has "Close" on it, not "Hide"
					// MFC4.0 replaces "Close" with "Hide"
					// extract close text from ID_FILE_CLOSE after the "\n" for the tooltip prompt
					if (!(pBar->m_Style & SZBARF_DESTROY_ON_CLOSE))
					{
						CString strHide;

						if(pBar->m_Style & SZBARF_HIDE_ON_CLOSE)
						{
							if (strHide.LoadString(AFX_IDS_HIDE))
							{
								// modify menu text to be "Hide" instrad of close
								VERIFY(pSysMenu->ModifyMenu(SC_CLOSE, MF_BYCOMMAND | MF_STRING, SC_CLOSE, strHide));  
							} 
						}
						else
						{
							if (strHide.LoadString(AFX_IDS_DOCK))
							{
								// modify menu text to be "Dock" instrad of close
								VERIFY(pSysMenu->ModifyMenu(SC_CLOSE, MF_BYCOMMAND | MF_STRING, SC_CLOSE, strHide));  
							} 
						}

						/*
						CString strClose;
						if (strClose.LoadString(ID_FILE_CLOSE))
						{
							LPCTSTR pCloseText = strchr(strClose, '\n');
							if (pCloseText != NULL)
								pSysMenu->DeleteMenu(SC_CLOSE, MF_BYCOMMAND);
								pSysMenu->AppendMenu(MF_STRING|MF_ENABLED, SC_CLOSE, pCloseText + 1);
						} */
					}
				}
				else
				{
					if ((pBar->m_dwStyle & CBRS_SIZE_DYNAMIC))	 // dynamic bar - turn on MFS_4THICKFRAME
					{
						ModifyStyle(0, MFS_4THICKFRAME);	
						return m_nContainedBarType;
					}
			
				}
			}
		}
		if (m_nContainedBarType == MFCBase)
		{
			ModifyStyle(WS_THICKFRAME, MFS_MOVEFRAME);
		}
	}		
	// if bar is MFC bar (and not CBRS_SIZE_DYNAMIC, turn on MFS_MOVEFRAME)
	return m_nContainedBarType;
}


//-------------------------------------------------------------------
void CSizableMiniDockFrameWnd::OnClose()
//-------------------------------------------------------------------
{
	if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
     // CMiniDockFrameWnd class assumes if there is only 1 bar, then it's at position 1
     // in the array
    	CControlBar* pBar = ((CSizeDockBar *) &m_wndDockBar)->GetFirstControlBar();
     	if (pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar)) )
 			if (((CMRCSizeControlBar *)pBar)->m_Style & SZBARF_DESTROY_ON_CLOSE)
            {
              // close the Frame Window
              CFrameWnd::OnClose();       // close the window
              delete pBar;            // now explicitly delete the control bar
              return;
             }
    }

    // otherwise just show it.
    CMiniDockFrameWnd::OnClose();
    return;
}


//--------------------------------------------------------------------------
void MiniDockToClient(CRect & rect, BOOL bConvertToClient)
// convert MiniDock size to a client size.. or vice versa.
//--------------------------------------------------------------------------
{
	static int nCaptionY = -1;
	static int nBorderX, nBorderY;

	// if not set up, create a temporary floating frame to see how big it is.
	if (nCaptionY == -1)
	{
		CFrameWnd * pMainFrame = (CFrameWnd *) AfxGetMainWnd();
		CMiniDockFrameWnd * pTmpFloatFrame = pMainFrame->CreateFloatingFrame(0);
	
		// calculate frame dragging rectangle
		CRect rcFloat(0,0,0,0);
		
		pTmpFloatFrame->CalcWindowRect(&rcFloat);
	
		rcFloat.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	    
		nCaptionY = - rcFloat.top;
		nBorderY = rcFloat.bottom;
		nBorderX = rcFloat.right;
		
		pTmpFloatFrame->DestroyWindow();
	}		

	if (bConvertToClient)
	{
		rect.left += nBorderX;
		rect.right -= nBorderX;
		rect.top += nCaptionY;
		rect.bottom -= nBorderY;
	}
	else
	{
		rect.left -= nBorderX;
		rect.right += nBorderX;
		rect.top -= nCaptionY;
		rect.bottom += nBorderY;
	}
}


//------------------------------------------------------------------------
CControlBar * GetDockedControlBar(int nPos, const CPtrArray & arrBars)
// helper which can acts on any array of windows
//------------------------------------------------------------------------
{
	CControlBar* pResult = (CControlBar*)arrBars[nPos];
	if (HIWORD(pResult) == 0)
		return NULL;
	return pResult;
}


/////////////////////////////////////////////////////////////////////////////
// CSizeDockBar - derived from CDockBar

CSizeDockBar::CSizeDockBar() 
{
    m_pSplitCapture = NULL;
    m_hcurLast = NULL;
    m_LayoutSize.cx = 0xffff;       // some stupid values to force automatic resize
    m_LayoutSize.cy = 0xffff;
	m_CountBars = 0;
}


CSizeDockBar::~CSizeDockBar()
{
    DeleteSplitterRects();          // delete any outstanding splitter rects
}


BEGIN_MESSAGE_MAP(CSizeDockBar, CDockBar)
        //{{AFX_MSG_MAP(CSizeDockBar)
        ON_WM_PAINT()
        ON_WM_MOUSEMOVE()
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONUP()
        ON_WM_SETCURSOR()
        //}}AFX_MSG_MAP
        ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()


#ifdef _DEBUG
void DumpArrayBars(CDumpContext & dc, const CPtrArray & arrBars)
{
	for (int nPos = 0; nPos < arrBars.GetSize(); nPos++)
	{
		LPVOID pVoid = arrBars[nPos];
		dc << "   [" << nPos << "]" << pVoid;
		CControlBar * pBar = GetDockedControlBar(nPos, arrBars);
		if (pBar != NULL)
		{
			CString strTitle;
			pBar->GetWindowText(strTitle);
			dc << " " << strTitle;
			if (!pBar->IsVisible())
				dc << " hidden";
		}				
		dc << "\n";			
	}
}


void CSizeDockBar::Dump( CDumpContext& dc ) const
{
	CDockBar::Dump(dc);
	DumpArrayBars(dc, m_arrBars);

	// now go through the splitter array.
	int nDepth = dc.GetDepth();
	dc.SetDepth(1);
	m_SplitArr.Dump(dc);		
	dc.SetDepth(nDepth);
}


void CSplitterRect::Dump( CDumpContext& dc ) const
{
	dc << "pos = " << m_nPos;
	dc << (m_type == SPLITTER_HORZ ? " Horz" : " Vert");
}


#endif



/////////////////////////////////////////////////////////////////////////////
// CSizeDockBar message handlers


/////////////////////////////////////////////////////////////////////////////
// CDockBar layout

//---------------------------------------------------------------------------
CSize CSizeDockBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
// most of this code is copied from MFC CDockBar - with additional comments to help
// my understanding of it. The basic idea is that our DockBar is being asked how big
// it is. To find out, it looks at the bars inside it
//---------------------------------------------------------------------------
{
    ASSERT_VALID(this);

    CSize sizeFixed = CControlBar::CalcFixedLayout(bStretch, bHorz);

    // ID for this bar...used to set MRU docked position
	UINT uDockBarID = GetWindowLong(m_hWnd, GWL_ID);
	
	// prepare for layout
    AFX_SIZEPARENTPARAMS layout;
    layout.hDWP = m_bLayoutQuery ?
            NULL : ::BeginDeferWindowPos(m_arrBars.GetSize());

    CPoint pt(-afxData.cxBorder2, -afxData.cyBorder2);

    BOOL bDrawBarForRow = FALSE;                    // true if we should draw a bar for this row
    BOOL bDrawBarWithinRow = FALSE;                 // true if we should draw a bar for this column


    DeleteSplitterRects();          // clear the splitter rects
    int nWidth = 0;
    int nFirstSplitterInRow = 0;
    int nFirstPaneInRow = 0;
    BOOL bFirstPaneInRow = TRUE;

    // layout all the control bars
	int nLastVisSep = -1;
    for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
    {
		void * pVoid = m_arrBars[nPos];
		CControlBar* pBar = GetDockedControlBar(nPos);
        if (pVoid != NULL)
        {
            if (pBar != NULL && pBar->IsVisible())
            {
				if (bFirstPaneInRow)
                {
                    bFirstPaneInRow = FALSE;        // remember where the first pane in the row is..
                    nFirstPaneInRow = nPos;

                    if (m_dwStyle & (CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT))
                    {
                        bDrawBarForRow = IsRowSizeable(nPos);
                    }

                    if (bDrawBarForRow)
                    {
                        if (bHorz)
                        {
                            AddSplitterRect(SPLITTER_HORZ, pt.x, pt.y, 0 ,pt.y + CY_SPLIT, nPos);   // width set at end
                            pt.y += CY_SPLIT;
                        }
                        else
                        {
                            AddSplitterRect(SPLITTER_VERT, pt.x, pt.y, pt.x + CX_SPLIT, 0, nPos);   // height set at end
                            pt.x += CX_SPLIT;
                        }
                        bDrawBarForRow = FALSE;
                    }
                }

                if (bDrawBarWithinRow)  // draw a bar between 2 previous
                {
                    if (bHorz)
                    {
                        AddSplitterRect(SPLITTER_VERT, pt.x, pt.y, pt.x + CX_SPLIT, 0, nPos); // width set at end of row
                        pt.x += CX_SPLIT;
                    }
                    else
                    {
                        AddSplitterRect(SPLITTER_HORZ, pt.x, pt.y, 0, pt.y + CY_SPLIT, nPos); // width set at end of row
                        pt.y += CY_SPLIT;
                    }

                }
                bDrawBarWithinRow = TRUE;               // potentially could clear this if 2 dockbars
                                                                                // side by side are non-sizeable

                if (IsSizeable(pBar))                   // if the bar is sizeable, then we'll need a sizer after it
                {
                    bDrawBarForRow = TRUE;
                }

                // get ideal rect for bar
                CSize sizeBar = pBar->CalcFixedLayout(FALSE,
                        (pBar->m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE);
                CRect rect(pt, sizeBar);

                // get current rect for bar
                CRect rectBar;
                pBar->GetWindowRect(&rectBar);
                ScreenToClient(&rectBar);

                if (bHorz)
                {

                    // change position if size changed or top not the same or
                    // rectbar.left < rect.left
                    // if floating compress
                    pt.x = rectBar.left;
                    if (rect.Size() != rectBar.Size() ||
                            rect.top != rectBar.top ||
                            (rectBar.left != rect.left && !m_bFloating) ||
                            (rectBar.left != rect.left && m_bFloating))
                    {
                        AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
                        pt.x = rect.left;
                    }
                    pt.x += sizeBar.cx - afxData.cxBorder2;
                    nWidth = max(nWidth, sizeBar.cy);
                }
                else
                {
                    // change position if size changed or top not the same or
                    // rectbar.left < rect.left
                    // if floating compress
                    pt.y = rectBar.top;
                    if (rect.Size() != rectBar.Size() ||
                            rect.left != rectBar.left ||
                            (rectBar.top != rect.top && !m_bFloating) ||
                            (rectBar.top != rect.top && m_bFloating))
                    {
                        AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
                        pt.y = rect.top;
                    }
                    pt.y += sizeBar.cy - afxData.cyBorder2;
                    nWidth = max(nWidth, sizeBar.cx);
                }
				// repositioned the bar, so update the MRU dock position.
				CDockContext * pDockContext = pBar->m_pDockContext;
				ASSERT(pDockContext != NULL);
				if (pDockContext != NULL)
				{
					pDockContext->m_rectMRUDockPos = rect;
					pDockContext->m_uMRUDockID = uDockBarID;
				}			

				// handle any delay/show hide for the bar
				pBar->RecalcDelayShow(&layout);
		
			}
        }
        else
		{
			//if (nWidth != 0)
            if (!bFirstPaneInRow)	// FALSE if we've hit anything....
            {
                 // end of row because pBar == NULL
                if (bHorz)
                {
                    pt.y += nWidth - afxData.cyBorder2;
                    sizeFixed.cx = max(sizeFixed.cx, pt.x);
                    sizeFixed.cy = max(sizeFixed.cy, pt.y);
                    pt.x = -afxData.cxBorder2;
                    SetSplitterSizeInRange(nFirstSplitterInRow, SPLITTER_VERT, pt.y);
				}
                else
                {
                    pt.x += nWidth - afxData.cxBorder2;
                    sizeFixed.cx = max(sizeFixed.cx, pt.x);
                    sizeFixed.cy = max(sizeFixed.cy, pt.y);
                    pt.y = -afxData.cyBorder2;
                    SetSplitterSizeInRange(nFirstSplitterInRow, SPLITTER_HORZ, pt.x);
				}
				nLastVisSep = nPos;			// record separator for last vis position
			}
			nFirstSplitterInRow = max(m_SplitArr.GetSize(), 0);
			nWidth = 0;
			bDrawBarWithinRow = FALSE;
			bFirstPaneInRow = TRUE;
		}
	}

    // special case when bars are at top or left.
    // use of nFirstPaneInRow (nPos where row started) so that sizing code can cope ok

    if (nFirstPaneInRow != 0 && bDrawBarForRow && (m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT)))                         // there is at least one pane.
    {
        ASSERT(nLastVisSep != -1);
		//while (m_arrBars[nLastVisibleBar] != NULL)
			//nLastVisibleBar++;
		
		if (m_dwStyle & CBRS_ALIGN_TOP)
        {
            AddSplitterRect(SPLITTER_HORZ, pt.x, pt.y, 0 , pt.y + CY_SPLIT, nLastVisSep);
            sizeFixed.cy += CY_SPLIT;
        }
        else
        {
            AddSplitterRect(SPLITTER_VERT, pt.x, pt.y, pt.x + CX_SPLIT, 0, nLastVisSep);
            sizeFixed.cx += CX_SPLIT;
        }
    }

    if (!m_bLayoutQuery)
    {
            // move and resize all the windows at once!
        if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
            TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
    }

    // Finally go back and set the size of the bars between the rows
    if (bHorz)
        SetSplitterSizeInRange(0, SPLITTER_HORZ, sizeFixed.cx);         // set widths of inter-rows
    else
        SetSplitterSizeInRange(0, SPLITTER_VERT, sizeFixed.cy);         // set heights of inte-rcolumns

    return sizeFixed;

}




//-----------------------------------------------------------------------------
void CSizeDockBar::AddSplitterRect(int type, int x1, int y1, int x2, int y2, int nPos)
//-----------------------------------------------------------------------------
{
    CSplitterRect * pSplit = new CSplitterRect(type, CRect(x1, y1, x2, y2));
    pSplit->m_nPos = nPos;
    ASSERT( pSplit != NULL);
    m_SplitArr.Add(pSplit);
}


//-----------------------------------------------------------------------------
void CSizeDockBar::SetSplitterSizeInRange(int start, int type, int length)
// helper function: Sets the length of all CSplitterRects in the range (start->end of array)
// with the specified type. Used at the end of a row to set all the heights to
// the calculated width.
//-----------------------------------------------------------------------------
{
    ASSERT(type == SPLITTER_VERT || type == SPLITTER_HORZ);
    ASSERT(start >= 0 && start <= m_SplitArr.GetSize());

    for (int i = m_SplitArr.GetUpperBound(); i >= start; i--)
    {
        CSplitterRect * pItem = (CSplitterRect *)m_SplitArr[i];
        if (pItem->m_type == type)
        {
            if (type == SPLITTER_VERT)
                pItem->m_rect.bottom = length;
            else
                pItem->m_rect.right = length;
        }
    }
}


//-----------------------------------------------------------------------------
void CSizeDockBar::DeleteSplitterRects()
//-----------------------------------------------------------------------------
{
    for (int i = m_SplitArr.GetUpperBound(); i >= 0 ; i--)
        delete (m_SplitArr[i]);
    m_SplitArr.RemoveAll();
}


//-----------------------------------------------------------------------------
void CSizeDockBar::OnPaint()
//-----------------------------------------------------------------------------
{
    CPaintDC dc(this); // device context for painting

    for (int i = m_SplitArr.GetUpperBound(); i >= 0; i--)
        ((CSplitterRect *)(m_SplitArr[i]))->Draw(&dc);
}




//----------------------------------------------------------------------------
CSplitterRect * CSizeDockBar::HitTest(CPoint pt)
//----------------------------------------------------------------------------
{
	for (int i = m_SplitArr.GetUpperBound(); i >= 0; i--)
    {
    	CSplitterRect *pSplit = GetSplitter(i);
        if (pSplit->m_rect.PtInRect(pt))
        	return(pSplit);
    }
    return NULL;
}


//---------------------------------------------------------------------------------
BOOL CSizeDockBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
//---------------------------------------------------------------------------------
{
    if (nHitTest == HTCLIENT && pWnd == this) // && !m_bTracking)
        return TRUE;    // we will handle it in the mouse move

    return CDockBar::OnSetCursor(pWnd, nHitTest, message);
}



//-------------------------------------------------------------------------------------
int CSizeDockBar::StartPosOfRow(int nPos)
// if already positioned on NULL, go back one further
//-------------------------------------------------------------------------------------
{
    if (nPos > 0)
    {
        if (m_arrBars[nPos] == NULL)
            nPos --;
        while (nPos >= 0)
        {
            if (m_arrBars[nPos] == NULL)
                return (nPos + 1);
            nPos --;
        }
    }
    return 0;
}


//-------------------------------------------------------------------------------------
int CSizeDockBar::StartPosOfPreviousRow(int nPos)
// returns strart of previous row.
// This function includes logic to cope with nPos beyond the end of the array.
//-------------------------------------------------------------------------------------
{
    ASSERT (nPos > 0);
    if (nPos >= m_arrBars.GetUpperBound())
        return (StartPosOfRow(nPos));
    else
        return StartPosOfRow(nPos - 1);
 }



//-------------------------------------------------------------------------------------
void CSizeDockBar::GetRowSizeInfo(int nPos,  ROWSIZEINFO * pRZI, const CPtrArray & arrBars)
// Go through control bars in the row.
// returns the number of sizeable bars in the row, and the total space they currently
// take up
//-------------------------------------------------------------------------------------
{
	BOOL bHorz = IsBarHorizontal();

    // zero all the fields
    memset (pRZI, 0, sizeof (ROWSIZEINFO));

    while (nPos <= arrBars.GetUpperBound())
    {
    	CRect rect;
        void * pVoid = arrBars[nPos];
		if (pVoid == NULL)
        	break;                  // end of the row
		CControlBar * pBar = ::GetDockedControlBar(nPos, arrBars);
		if (pBar!= NULL && pBar->IsVisible())
		{	
        	if (pRZI->nTotalBars != 0)
            	pRZI->nFixedWidth += (bHorz ? CX_SPLIT : CY_SPLIT);     // add a splitter size

            pBar->GetWindowRect(&rect);
            int nWidth = max (0, (bHorz ? rect.Width() : rect.Height()) );
            int nHeight = max (0, (bHorz ? rect.Height() : rect.Width()) );

            pRZI->nTotalBars ++;
            if (nHeight > pRZI->nMaxHeight)
                pRZI->nMaxHeight = nHeight;

            if (IsSizeable(pBar))
            {
                pRZI->nFlexBars ++;
                pRZI->nFlexWidth += nWidth;
            }
            else
            {
                pRZI->nFixedWidth += nWidth;
                if (nHeight > pRZI->nMaxFixedHeight)
 	            	pRZI->nMaxFixedHeight = nHeight;
            }
        }
        nPos ++;
    }
   	pRZI->nTotalWidth = pRZI->nFixedWidth + pRZI->nFlexWidth;
	return;
}


//------------------------------------------------------------------------
BOOL CSizeDockBar::AdjustAllRowSizes(int nNewSize)
// Adjusts the sizes of all the bars on a dockbar to fit a new size
//------------------------------------------------------------------------
{
    BOOL bAdjusted = FALSE;
    int nPos = 0;
    while (nPos < m_arrBars.GetSize())
    {
        CControlBar * pBar = (CControlBar *) m_arrBars[nPos];
        if (pBar == NULL)
        {                                               // skip over NULLs
			nPos ++;
            continue;
        }
        bAdjusted |= AdjustRowSizes(nPos, nNewSize, m_arrBars);                    // adjust the sizes on a row
        while (m_arrBars[nPos] != NULL)                 // skip to end of row
                nPos++;
    }
    return bAdjusted;
}


//------------------------------------------------------------------------
BOOL CSizeDockBar::AdjustRowSizes(int nPos, int nNewSize, CPtrArray & arrBars)
// Adjusts the size of a row - returns TRUE if any changes were made
//------------------------------------------------------------------------
{
	BOOL bHorz = IsBarHorizontal();

    ROWSIZEINFO RZI;
    GetRowSizeInfo(nPos, &RZI, arrBars);
    if (RZI.nFlexBars == 0)
    	return FALSE;                   // no flexible bars - nothing to do !

	int nTotalSizeChange = (nNewSize - RZI.nTotalWidth);
    int nSizeChangeRemaining = nTotalSizeChange;
	int nSizeChange = nTotalSizeChange / RZI.nFlexBars;
	// have to apply this size change to the bars on this row. Note: This will work
    // by setting the docked size of the controls bars directly. Then ReCalcLayout will
    // do the rest.
    int nCountFlexBars = 0;
    while (TRUE)
    {
    	void * pVoid = arrBars[nPos];
		if (pVoid == NULL)
            break;          // end of the row, stop
		CMRCSizeControlBar * pBar = (CMRCSizeControlBar *)::GetDockedControlBar(nPos, arrBars);	// note:slight abuse of cast
        
        if (pBar != NULL && pBar->IsVisible() && IsSizeable(pBar))
        {
        	int nWidth = (bHorz ? pBar->m_HorzDockSize.cx : pBar->m_VertDockSize.cy);
			nCountFlexBars ++;
            if (nCountFlexBars == RZI.nFlexBars)    // last bar adjust size change
            {
            	nSizeChange = nSizeChangeRemaining;
            	// nSizeChange = nTotalSizeChange - ((RZI.nFlexBars - 1) * nSizeChange);
            }
            else
			{
				nSizeChange = (nWidth + 1) * nTotalSizeChange / (RZI.nFlexWidth + RZI.nFlexBars);
			}
			
#ifdef _VERBOSE_TRACE
			CString strTitle;
			pBar->GetWindowText(strTitle);
			TRACE("Size change: %d on %s\n", nSizeChange, strTitle);
#endif			
			int nNewWidth = max(nWidth + nSizeChange, 0);
			nSizeChangeRemaining -= (nNewWidth - nWidth);
			
			if (bHorz)
            {
				pBar->m_HorzDockSize.cx = nNewWidth;
                pBar->m_HorzDockSize.cy = RZI.nMaxHeight;
				SetWindowSize(pBar, pBar->m_HorzDockSize);
			}
            else
            {
                pBar->m_VertDockSize.cy = nNewWidth;
            	pBar->m_VertDockSize.cx = RZI.nMaxHeight;
				SetWindowSize(pBar, pBar->m_VertDockSize);
            }
         }
        nPos++;
    }

	return TRUE;
}


//------------------------------------------------------------------------
void CSizeDockBar::TileDockedBars()
// Adjusts the sizes of all the bars on a dockbar to fit a new size
//------------------------------------------------------------------------
{
    int nPos = 0;
    while (nPos < m_arrBars.GetSize())
    {
        CControlBar * pBar = (CControlBar *) m_arrBars[nPos];
        if (pBar == NULL)
        {                                               // skip over NULLs
            nPos ++;
            continue;
        }
        TileDockedBarsRow(nPos);                                        // adjust the sizes on a row
        while (m_arrBars[nPos] != NULL)                         // skip to end of row
            nPos++;
    }
    return;
}


//---------------------------------------------------------------------------
void CSizeDockBar::TileDockedBarsRow(int nPos)
// Tiles the docked bars:
//---------------------------------------------------------------------------
{
    BOOL bHorz = IsBarHorizontal();

    ROWSIZEINFO RZI;
    GetRowSizeInfo(nPos, &RZI, m_arrBars);
    if (RZI.nFlexBars == 0)
            return;                         // no flexible bars - nothing to do !

    int nNewSize = (bHorz ? m_LayoutSize.cx : m_LayoutSize.cy);
    int nTotalSize = max (0, nNewSize - RZI.nFixedWidth);
    int nNewWidth = nTotalSize / RZI.nFlexBars;

    int nCountFlexBars = 0;
    while(TRUE)
    {
		void * pVoid = m_arrBars[nPos];    
		if (pVoid == NULL)
                break;          // end of the row, stop
		CMRCSizeControlBar * pBar = (CMRCSizeControlBar *) GetDockedControlBar(nPos); // note:slight abuse of cast
        
        if (pBar != NULL && IsSizeable(pBar) && pBar->IsVisible())
        {
            nCountFlexBars ++;
            if (nCountFlexBars == RZI.nFlexBars)    // last bar adjust size change
            {
                nNewWidth = nTotalSize - ((RZI.nFlexBars - 1) * nNewWidth);
            }
            if (bHorz)
            {
                pBar->m_HorzDockSize.cx = nNewWidth;
                pBar->m_HorzDockSize.cy = RZI.nMaxHeight;
            }
            else
            {
                pBar->m_VertDockSize.cy = nNewWidth;
                pBar->m_VertDockSize.cx = RZI.nMaxHeight;
            }
        }
        nPos++;
    }
}


//-------------------------------------------------------------------------------------
int FindInArray(const CPtrArray & arrBars, int nStartIndex, void * pFind)
//-------------------------------------------------------------------------------------
{
	
	while (nStartIndex < arrBars.GetUpperBound())	
	{
		if (arrBars[nStartIndex] == pFind)
			return nStartIndex;
		if (arrBars[nStartIndex] == NULL)
			break;		
		nStartIndex++;
	}
	return -1;
}

//-------------------------------------------------------------------------------------
void * FindInArray(void * pFindId, void ** pArray)
//-------------------------------------------------------------------------------------
{
	while (*pArray != NULL)
	{
		if (*pArray == pFindId)
			return pArray;
		pArray++;
	}
	return NULL;
}	


#ifdef _DEBUG
//-------------------------------------------------------------------------------------
CString GetBarTitles(const CPtrArray & arrBars, int nPos)  
// DEBUG only helper function  
//-------------------------------------------------------------------------------------
{
	CString strMsg, strTitle;
	while (arrBars[nPos] != 0)
	{
		CControlBar * pBar = GetDockedControlBar(nPos, arrBars);
		pBar->GetWindowText(strTitle);
		strMsg += strTitle;
		strMsg += ",";
		nPos ++;
	}
	return strMsg;	
}
#endif

//-------------------------------------------------------------------------------------
BOOL CSizeDockBar::WasBarHidden(CControlBar *pBar)
// Returns TRUE is BAR is in m_arrInvisibleBars
//-------------------------------------------------------------------------------------
{
	for (int i= 0; i < m_arrHiddenBars.GetSize(); i++)
		{							
			if (m_arrHiddenBars[i] == pBar)
				return TRUE;
		}
	return FALSE;
}


//-------------------------------------------------------------------------------------
LRESULT CSizeDockBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
// WM_SIZEPARENT message is sent from CFrameWnd::RepositionBars() to tell the dockbar to
// calculate it's size.
// The only reason for intercepting this was to actually find out the size the dockbar is taking
// up in the layout, so we can opt to re-layout a row to fit the desired size.
// There might well be a better way of doing this.
//-------------------------------------------------------------------------------------
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	
	BOOL bInvalidate = FALSE;
	BOOL bHorz = IsBarHorizontal();

    CRect LayRect;
    LayRect.CopyRect(&lpLayout->rect);
    CSize LaySize = LayRect.Size();  // maximum size available
	int nLayoutWidth = bHorz ? LaySize.cx : LaySize.cy;
	BOOL bLayoutWidthChanged = (nLayoutWidth != (bHorz ? m_LayoutSize.cx : m_LayoutSize.cy));
	m_LayoutSize = LaySize;

	// Attempt to detect bars that have changed state from Hidden->Visible. For these we attempt
	// to adjust the other (previously visible) bars on the row so that the newly shown bars
	// restore their previous size.
	CPtrArray	arrVisibleBarsInRow;		// Bars visible in the row (ones we can shrink)
	int nWidthNeeded = 0;
	for (int i = 0; i < m_arrBars.GetSize(); i++)
    {
		if (m_arrBars[i] == NULL)
		{
			ROWSIZEINFO RZI;
			if (arrVisibleBarsInRow.GetSize() != 0 && nWidthNeeded != 0)
			{
				arrVisibleBarsInRow.Add(NULL);
				
				GetRowSizeInfo(0, &RZI, arrVisibleBarsInRow);
				int nNewWidth = max(0, RZI.nTotalWidth - nWidthNeeded);
				AdjustRowSizes(0, nNewWidth, arrVisibleBarsInRow);
			}
			nWidthNeeded = 0;
			arrVisibleBarsInRow.RemoveAll();
		}
		else
		{
			CControlBar* pBar = GetDockedControlBar(i);
			if (pBar != NULL)
			{
				if (pBar->IsVisible())
				{
					if (WasBarHidden(pBar))
					{
						CRect rect;
						pBar->GetWindowRect(&rect);
						nWidthNeeded += (bHorz ? rect.Width() : rect.Height());
					}
					else
					{
						arrVisibleBarsInRow.Add(pBar);		// Track visible bars in this row that we can shrink
					}
				}
			}
		}
	}
	
	// construct new array of bars that are hidden in this dockbar
	m_arrHiddenBars.RemoveAll();
	for (i = 0; i < m_arrBars.GetSize(); i++)
    {
		CControlBar* pBar = GetDockedControlBar(i);
		if (pBar != NULL && ! pBar->IsVisible())
			m_arrHiddenBars.Add(pBar);
	}

	int nCheckSum = CheckSumBars();

    // any other changes and we size the bars to fit the layout width
	if (bLayoutWidthChanged || nCheckSum != m_CountBars)
    {
        AdjustAllRowSizes(nLayoutWidth);
        m_CountBars = nCheckSum;
        InvalidateRect(NULL);   // force redraw of the dock bar - seems a bit of a sledgehammer
    }

	// set m_bLayoutQuery to TRUE if lpLayout->hDWP == NULL
	BOOL bLayoutQuery = m_bLayoutQuery;
    m_bLayoutQuery = (lpLayout->hDWP == NULL);
	LRESULT lResult = CControlBar::OnSizeParent(wParam, lParam);
    // restore m_bLayoutQuery
    m_bLayoutQuery = bLayoutQuery;

    return lResult;
}



//---------------------------------------------------------------------------
int CSizeDockBar::CheckSumBars() const
// Simple checksum for bars. Designed to spot the case when a bars moves within
// a dockrow.
//---------------------------------------------------------------------------
{
	int nCount = 0;         // total no of bars
    int nCheckSum = 0;      // XOR, power of 2 checksum
    for (int i = 0; i < m_arrBars.GetSize(); i++)
    {
    	if (m_arrBars[i] == NULL)
			nCheckSum *= 2;
		else
		{        
			CControlBar* pBar = GetDockedControlBar(i);
			ASSERT(pBar == NULL || pBar->IsKindOf(RUNTIME_CLASS(CControlBar)));
        	if (pBar != NULL && pBar->IsVisible())
            {
            	nCheckSum++;
                nCount++;
            }
		}
	}
        // LSB = actual no of dockbars (limited to 256 !)
        // bits 8-31 = checksum based on layout of rows.
    return ((nCheckSum << 8) | (nCount & 0xff));
}


//-----------------------------------------------------------------------------
void CSizeDockBar::AdjustForNewBar(CControlBar *pNewBar)		
// Adjust sizes for specified newly added bar.
//-----------------------------------------------------------------------------
{
	int nPos = FindBar(pNewBar);
	ASSERT(nPos != -1);			// bar should have been found.
	
	// Go back to start of row.
	while (m_arrBars[nPos] != NULL)
		nPos--;

	nPos++;

	// create an array for the bars on the row, that aren't this one
	CPtrArray arrOtherBarsInRow;
	while (nPos < m_arrBars.GetSize() && m_arrBars[nPos] != NULL)
	{
		CControlBar* pBar = GetDockedControlBar(nPos);
		if (pBar != pNewBar)
			arrOtherBarsInRow.Add(pBar);
		nPos++;
	}

	ROWSIZEINFO RZI;
	arrOtherBarsInRow.Add(NULL);
	GetRowSizeInfo(0, &RZI, arrOtherBarsInRow);
	CRect rcNewBar;
	pNewBar->GetWindowRect(&rcNewBar);
	int nWidthNeeded = (IsBarHorizontal() ? rcNewBar.Width() : rcNewBar.Height());
	int nNewWidth = max(0, RZI.nTotalWidth - nWidthNeeded);
	AdjustRowSizes(0, nNewWidth, arrOtherBarsInRow);
}

//-----------------------------------------------------------------------------
CSplitterRect * CSizeDockBar::SetHitCursor(CPoint pt)
// Hit test the mouse position - and set cursor accordingly
//-----------------------------------------------------------------------------
{
	// Set up the split cursors here. This guarantees the app is around
    if (afxData.hcurSizeWE == NULL)
    { 
    	afxData.hcurSizeWE = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR);
		if (afxData.hcurSizeWE == NULL)
    		afxData.hcurSizeWE = ::LoadCursor(NULL, IDC_SIZEWE);
	}			
   
    if (afxData.hcurSizeNS == NULL)
    {
    	afxData.hcurSizeNS = AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR);
		if (afxData.hcurSizeNS == NULL)
			afxData.hcurSizeNS = ::LoadCursor(NULL, IDC_SIZENS);
	}


    HCURSOR hcurNew;
    CSplitterRect * pSplit = HitTest(pt);
    if (pSplit != NULL)
   		hcurNew = (pSplit->m_type == SPLITTER_VERT ? afxData.hcurSizeWE : afxData.hcurSizeNS);
    else
        hcurNew = afxData.hcurArrow;

    ::SetCursor(hcurNew);
    return pSplit;
}


//-----------------------------------------------------------------------------
void CSizeDockBar::OnMouseMove(UINT nFlags, CPoint point)
//-----------------------------------------------------------------------------
{
    SetHitCursor(point);
    CDockBar::OnMouseMove(nFlags, point);
}



//-----------------------------------------------------------------------------
void CSizeDockBar::OnLButtonDown(UINT nFlags, CPoint point)
//-----------------------------------------------------------------------------
{
	if (m_pSplitCapture == NULL)
	{
		m_pSplitCapture = SetHitCursor(point);
		if (m_pSplitCapture != NULL)
		{
			StartTracking(point);
			m_pSplitCapture = NULL;
		}
	}
}


//-----------------------------------------------------------------------------
void CSizeDockBar::OnLButtonUp(UINT nFlags, CPoint point)
//-----------------------------------------------------------------------------
{
    //m_pSplitCapture = NULL;
    CDockBar::OnLButtonUp(nFlags, point);
}


//-----------------------------------------------------------------------------
void CSizeDockBar::StartTracking(CPoint pt)
//-----------------------------------------------------------------------------
{
    ASSERT(m_pSplitCapture != NULL);

    // Some organizational flags: helps to cut down the cases
    BOOL bHorz      = IsBarHorizontal();
    BOOL bVertSplitter = (m_pSplitCapture->m_type == SPLITTER_VERT);
    BOOL bRowDivider = ((!bVertSplitter) && bHorz) || (bVertSplitter && (!bHorz));
    int nPos = m_pSplitCapture->m_nPos;

    CMRCRectTracker MvRect;

    // attempt to clip move rect by current layout size of the dockbar
    CRect LayoutRect(CPoint(0,0), m_LayoutSize);
    MvRect.m_rect = m_pSplitCapture->m_rect;

	// Shrink the splitter rectangle to end up with a solid bar
	if (bVertSplitter)
		MvRect.m_rect.InflateRect(-2, 0);
	else
		MvRect.m_rect.InflateRect(0, -2);


    MvRect.m_rect.IntersectRect(MvRect.m_rect, LayoutRect);
    ASSERT(!(MvRect.m_rect.IsRectEmpty()));
    
    // get main window - all dragging is done relative to this window.
	// this should be the frame window.....
	CWnd * pClipWnd = GetParentFrame();

	
    if (bVertSplitter)
		MvRect.m_nStyle |= RectTracker_OnlyMoveHorz;      // allow horizontal movement
    else
    	MvRect.m_nStyle |= RectTracker_OnlyMoveVert;      // allow horizontal movement

    
	// workout a limiting rectangle; - very dependent on orientation. Eventually may need to work
    // out the fixed size of the windows beyond the current splitter, so it could get nasty.
    // for now just use the client area of the window
    ROWSIZEINFO RZI;
    CRect LimitRect;
    pClipWnd->GetClientRect(&LimitRect);
    pClipWnd->ClientToScreen(&LimitRect);
    ScreenToClient(&LimitRect);			// map to co-ords of pWnd

#ifdef _VERBOSE_TRACE
	Dump(afxDump);
#endif

    if (bRowDivider)
    {
		if (m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT))             // apply to previous row for top/bottom
        {
        	nPos = StartPosOfPreviousRow(nPos);
            ASSERT(nPos != 0);
       	}
       	GetRowSizeInfo(nPos, &RZI, m_arrBars);             // get the row information:
       	switch (m_dwStyle & CBRS_ALIGN_ANY)
       	{
        	case CBRS_ALIGN_BOTTOM:
                LimitRect.bottom = min (LimitRect.bottom, MvRect.m_rect.top + (RZI.nMaxHeight - RZI.nMaxFixedHeight));
                break;
            case CBRS_ALIGN_TOP:
            	LimitRect.top = max (LimitRect.top, MvRect.m_rect.top - (RZI.nMaxHeight - RZI.nMaxFixedHeight));
              	break;
            case CBRS_ALIGN_LEFT:
                LimitRect.left = max (LimitRect.left, MvRect.m_rect.left - (RZI.nMaxHeight - RZI.nMaxFixedHeight));
                break;
            case CBRS_ALIGN_RIGHT:
                LimitRect.right = max (LimitRect.right, MvRect.m_rect.left + (RZI.nMaxHeight - RZI.nMaxFixedHeight));
                break;
            default:
                ASSERT(FALSE);
  		}
    }
	else
    {
        // How far can we go to down/right
        int nFlexToRight, nFlexToLeft;
        int nDownRight = ShrinkRowToRight(nPos, 16000, FALSE, &nFlexToRight);
        int nUpLeft = ShrinkRowToLeft(nPos - 1, 16000, FALSE, &nFlexToLeft);

        if ((nFlexToRight + nFlexToLeft) <= 1 )  // only 1 flex bar in the array - no movement !
        {
            nDownRight = 0;
            nUpLeft = 0;
        }

        if (bHorz)
        {
			LimitRect.left = max(LimitRect.left, MvRect.m_rect.left - nUpLeft);
            LimitRect.right = min(LimitRect.right, MvRect.m_rect.left + nDownRight);
        }
        else
        {
            LimitRect.top = max(LimitRect.top , MvRect.m_rect.top - nUpLeft);
            LimitRect.bottom = min(LimitRect.bottom, MvRect.m_rect.top + nDownRight);
        }
    }

    // Now enter the CMoveRect's modal track function
    MvRect.m_LimitRect = LimitRect;	  
    if (!MvRect.TrackFromHitTest (HTCAPTION, this, pt, pClipWnd))
     	return;

        // Workout the size change cause by the drag:
	int nSizeChange;
    if (m_pSplitCapture->m_type == SPLITTER_VERT)
    	nSizeChange = MvRect.m_rect.left - MvRect.m_OrigRect.left;
	else
        nSizeChange = MvRect.m_rect.top - MvRect.m_OrigRect.top;
    if (nSizeChange == 0)
        return;

    // CSplitterRect::m_nPos is the pane position that the splitter was created at.
    // For a row divider: this is the pane that immediately starts the next row
    // For a column divider: this is the pane that is to the right of it.
    // special case will be needed for the splitter used at the end of a left/top aligned
    // dockbar.
    int nSizeMoved;
    if (bRowDivider)
    {
        if (m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT))             // apply to previous row for top/bottom
        {
             nSizeChange = -nSizeChange;             // reverse polarity of change
        }

        int nNewHeight = max (RZI.nMaxFixedHeight, RZI.nMaxHeight - nSizeChange);

        // go along the rows applying size change to each bar in turn....
        while (nPos < m_arrBars.GetSize())	// need to check size now
        {
            void * pVoid = m_arrBars[nPos];
			if (pVoid == NULL)
                break;
            CMRCSizeControlBar * pBar = (CMRCSizeControlBar *)GetDockedControlBar(nPos);
            // should check for visible ???
			if (pBar != NULL && pBar->IsVisible() && IsSizeable(pBar))
            {
                if (bHorz)
                   	pBar->m_HorzDockSize.cy = nNewHeight;
                else
                	pBar->m_VertDockSize.cx = nNewHeight;
            }
            nPos ++;
     	}

    }
    else
    {
        if (nSizeChange < 0)
        {                                                               // move to left/up
        	nSizeMoved = ShrinkRowToLeft(nPos - 1, - nSizeChange, TRUE);
            ShrinkRowToRight(nPos, - nSizeMoved, TRUE);
        }
        else
        {                                                               // move to right/down
        	nSizeMoved = ShrinkRowToRight(nPos, nSizeChange, TRUE);
            ShrinkRowToLeft(nPos - 1, - nSizeMoved, TRUE);
        }

    }
    // reposition the bars..
    InvalidateRect(NULL);
    //((CFrameWnd *)AfxGetMainWnd())->RecalcLayout();
    ASSERT(pClipWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
    ((CFrameWnd *)pClipWnd)->RecalcLayout();
    return;
}




//----------------------------------------------------------------------------
int CSizeDockBar::ShrinkRowToLeft(int nPos, int nOrigAmount, BOOL bApply, int * pnFlexBars)
// amount to shrink row to left.
// nPos = current pane: nPos -1 = pane to go for:
// return value = amount of space we actually sized
// bApply: if TRUE, apply changes to bar sizes
//----------------------------------------------------------------------------
{
    ASSERT(nPos >= 0 && nPos <= m_arrBars.GetSize());

    int nAmount = nOrigAmount;
    int nFlexBars = 0;
    while (nPos >= 0)
    {
		if (m_arrBars[nPos] == NULL)
            break;
        CMRCSizeControlBar * pBar = (CMRCSizeControlBar *)GetDockedControlBar(nPos);
        if (pBar!= NULL && IsSizeable(pBar) && pBar->IsVisible())
        {
            nFlexBars ++;
            if (IsBarHorizontal())
            {
                if (pBar->m_HorzDockSize.cx >= nAmount)
                {
                    if (bApply)
                        pBar->m_HorzDockSize.cx -= nAmount;
                    nAmount = 0;
                    break;
                }
                else
                {
                    nAmount -= pBar->m_HorzDockSize.cx;
                    if (bApply)
                        pBar->m_HorzDockSize.cx = 0;
                }
            }
            else
            {
                if (pBar->m_VertDockSize.cy >= nAmount)
                {
                    if (bApply)
                        pBar->m_VertDockSize.cy -= nAmount;
                    nAmount = 0;
                    break;
                }
                else
                {
                    if (bApply)
                        pBar->m_VertDockSize.cy = 0;
                    nAmount -= pBar->m_VertDockSize.cy;
                }
            }

        }
        nPos--;
    }

    // return no of flexible components encountered (if pointer supplied)
    if (pnFlexBars != NULL)
        *pnFlexBars = nFlexBars;

    // reached left/top of row - return what size is still left to allocate
    return (nOrigAmount - nAmount);
}


//----------------------------------------------------------------------------
int CSizeDockBar::ShrinkRowToRight(int nPos, int nOrigAmount, BOOL bApply, int *pnFlexBars)
// amount to shrink row to right.
// nPos = current pane: nPos -1 = pane to go for:
// return value = amount of space we actually sized
//----------------------------------------------------------------------------
{
    ASSERT(nPos >= 0 && nPos <= m_arrBars.GetSize());
    int nAmount = nOrigAmount;
    int nFlexBars = 0;

    CMRCSizeControlBar * pLastBar = NULL;

    while (nPos < m_arrBars.GetSize())
    {
        if (m_arrBars[nPos] == NULL)
			break;

		CMRCSizeControlBar * pBar = (CMRCSizeControlBar *)GetDockedControlBar(nPos);
        if (pBar != NULL)
		{				
			pLastBar = pBar;
			if (IsSizeable(pBar) && pBar->IsVisible())
			{
				nFlexBars ++;
				if (IsBarHorizontal())
				{
					if (pBar->m_HorzDockSize.cx >= nAmount)
					{
						if (bApply)
							pBar->m_HorzDockSize.cx -= nAmount;
						nAmount = 0;
						break;
					}
					else
					{
						nAmount -= pBar->m_HorzDockSize.cx;
						if (bApply)
							pBar->m_HorzDockSize.cx = 0;
					}
				}
				else		// Vertical
				{
					if (pBar->m_VertDockSize.cy >= nAmount)
					{
						if (bApply)
							pBar->m_VertDockSize.cy -= nAmount;
						nAmount = 0;
						break;
					}
					else
					{
						nAmount -= pBar->m_VertDockSize.cy;
						if (bApply)
							pBar->m_VertDockSize.cy = 0;
					}
				}
				
			}
        }
		nPos++;
    }
    // We've reached the end of the row. If we still have size left to find, the only way we can do it is if there
    // is a flexble area at the end of the control bars..
    if (nAmount > 0 && pLastBar != NULL)
    {
        int nSub;
        CRect rect;
        pLastBar->GetWindowRect(&rect);
        ScreenToClient(&rect);
        if (IsBarHorizontal())
            nSub = m_LayoutSize.cx - rect.right;
        else
            nSub = m_LayoutSize.cy - rect.bottom;
        nAmount -= min (max( 0, nSub), nAmount);
    }

    // return no of flexible components encountered (if pointer supplied)
    if (pnFlexBars != NULL)
        *pnFlexBars = nFlexBars;

    // return amount allocated
    return (nOrigAmount - nAmount);
}



//---------------------------------------------------------------------------
CMRCSizeControlBar * CSizeDockBar::GetFirstControlBar()
// returns the first bar in the array - NULL if none
// used by the simplistic floating size routine
//---------------------------------------------------------------------------
{
    // CMiniDockFrameWnd assumes that if there's only one bar, then it's at position 1
    // in the array
    // need to make a check for 0 sized array however
	if (m_arrBars.GetSize() > 1)
		return ((CMRCSizeControlBar *) GetDockedControlBar(1));
	else
		return NULL;
}


//---------------------------------------------------------------------------
BOOL CSizeDockBar::IsRowSizeable(int nPos)
// returns TRUE if a CControlBar in the row is sizeable;
// intended to determine if the row should contain a splitter or not
//---------------------------------------------------------------------------
{
    ASSERT(nPos >= 0 && nPos < m_arrBars.GetSize());
    while (nPos < m_arrBars.GetSize())
    {
		if (m_arrBars[nPos] == NULL)
			break;
		CControlBar *pBar = GetDockedControlBar(nPos);
        if (pBar!= NULL && IsSizeable(pBar)  && pBar->IsVisible())
			return TRUE;
        nPos++;
    }
    return FALSE;
}


//-------------------------------------------------------------------------
int CSizeDockBar::TestInsertPosition(CControlBar* pBarIns, CRect rect)
// Essentially the same as CDockBar::Insert(). Returns the position in the 
// bar array that the object will be inserted.
// nPos = 0 => before first position... But will have to check if this dockbar
// is the same as the present one...(perhaps)
//-------------------------------------------------------------------------
{
    CPoint ptMid(rect.left + rect.Width()/2, rect.top + rect.Height()/2);
	// hang-on: Don't we want to work in client co-ords ???
	ScreenToClient(&ptMid);
	
	ASSERT_VALID(this);
	ASSERT(pBarIns != NULL);

	int nPos = 0;
	int nPosInsAfter = 0;
	int nWidth = 0;
	int nTotalWidth = 0;
	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	for (nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		void * pVoid = m_arrBars[nPos];
		CControlBar* pBar = GetDockedControlBar(nPos);

		if (pVoid == NULL)
		{
			nTotalWidth += nWidth - afxData.cyBorder2;
			nWidth = 0;
			if ((bHorz ? ptMid.y : ptMid.x) < nTotalWidth)
			{
				if (nPos == 0) // ie in first section....
					return 0;  // indicate before first position....
			//	if (nPos == 0) // first section
			//		m_arrBars.InsertAt(nPosInsAfter+1, (CObject*)NULL);
			//	m_arrBars.InsertAt(nPosInsAfter+1, pBarIns);
				return nPosInsAfter+1;
			}
			nPosInsAfter = nPos;
		}
		else
			if (pBar != NULL && pBar->IsVisible())	
			{
				CRect rectBar;
				pBar->GetWindowRect(&rectBar);
				ScreenToClient(&rectBar);
				nWidth = max(nWidth,
					bHorz ? rectBar.Size().cy : rectBar.Size().cx - 1);
				//if (bHorz ? rect.left > rectBar.left : rect.top > rectBar.top)
				// don't need above test - only interested if it should go on the row or not...
				nPosInsAfter = nPos;
			}
	}

	return nPosInsAfter+1;
}


//---------------------------------------------------------------------------
int CSizeDockBar::BarsOnThisRow(CControlBar *pBarIns, CRect rect)
// returns no of bars that will be in the row (excluding the one to be inserted)
//---------------------------------------------------------------------------
{
	int nPos = TestInsertPosition(pBarIns, rect);
	
	// if inserting before the first row, or after the last row, then return 0
	// (there are no bars on this row).
	if (nPos == 0 ||nPos > m_arrBars.GetUpperBound())		// case if inserting before first bar in the array.
		return 0;		// return 0 to use the full size
	
	// go back to start of row.
	while (nPos != 0 && m_arrBars[nPos - 1] != 0)
		nPos --;

	int nCount = 0;
	while (TRUE)
	{
		void * pVoid = m_arrBars[nPos];
		CControlBar * pBar = GetDockedControlBar(nPos);
		if (pVoid == NULL)
			break;
		if (pBar != NULL && pBar != pBarIns) 
			nCount++;
		nPos++;
	}
	return nCount;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMRCRectTracker
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


CMRCRectTracker::CMRCRectTracker()
{
	m_LimitRect.SetRectEmpty();
}




//------------------------------------------------------------------------------
BOOL CMRCRectTracker::TrackFromHitTest(int nHitTest, CWnd* pWnd, CPoint point,
	CWnd* pWndClipTo, BOOL bAllowInvert)
//------------------------------------------------------------------------------
{
	m_OrigRect = m_rect;		// save original rectangle
	m_bAllowInvert = bAllowInvert;
	int nHandle;
	switch (nHitTest)
	{
	case HTLEFT:
		nHandle = hitLeft;
		break;
	case HTRIGHT:
		nHandle = hitRight;
		break;
	case HTTOP:         
		nHandle = hitTop;
		break;
	case HTTOPLEFT:    
		nHandle = hitTopLeft;
		break;
	case HTTOPRIGHT:    
		nHandle = hitTopRight;
		break;
	case HTBOTTOM:      
		nHandle = hitBottom;
		break;
	case HTBOTTOMLEFT:  
		nHandle = hitBottomLeft;
		break;
	case HTBOTTOMRIGHT: 
		nHandle = hitBottomRight;
		break;
	default:
	    nHandle = hitMiddle;		// default is move
		break;
	}

	return CRectTracker::TrackHandle(nHandle, pWnd, point, pWndClipTo);
}




//-----------------------------------------------------------------------------
void CMRCRectTracker::DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd)
//-----------------------------------------------------------------------------
{
	// first, normalize the rectangle for drawing
	CRect rect = *lpRect;
	rect.NormalizeRect();

	// convert to client coordinates
	if (pWndClipTo != NULL)
	{
		pWnd->ClientToScreen(&rect);
		pWndClipTo->ScreenToClient(&rect);
	}
	CSize size(0, 0);
	if (!m_bFinalErase)
	{
		size.cx = 2;
		size.cy = 2;
	}
	// and draw it
	if (m_bFinalErase || !m_bErase)
		pDC->DrawDragRect(rect, size, m_rectLast, m_sizeLast);
	m_rectLast = rect;
	m_sizeLast = size; 
}


//-----------------------------------------------------------------------------
void CMRCRectTracker::AdjustRect(int nHandle, LPRECT lpRect)
//-----------------------------------------------------------------------------
{
// clips to limiting rectangle...
	if (!m_LimitRect.IsRectNull())
	{
		if (nHandle == hitMiddle)  // if moving then have to ensure size is maintained...
		{
			CSize size = m_OrigRect.Size();
			lpRect->left = max (m_LimitRect.left , min (m_LimitRect.right , lpRect->left));
            lpRect->top =  max (m_LimitRect.top  , min (m_LimitRect.bottom - 10, lpRect->top ));
            lpRect->right  = lpRect->left + size.cx;
            lpRect->bottom = lpRect->top  + size.cy;
		}
		else
		{		
			CRect iRect;
			iRect.IntersectRect(m_LimitRect, lpRect);
			::CopyRect(lpRect, iRect);		
		}
	}

// enforces minimum width, etc
	CRectTracker::AdjustRect(nHandle, lpRect);

	if (m_nStyle & RectTracker_OnlyMoveHorz)
	{
		lpRect->top = m_OrigRect.top;
		lpRect->bottom = m_OrigRect.bottom;
	}
	
	if (m_nStyle & RectTracker_OnlyMoveVert)
	{
		lpRect->left = m_OrigRect.left;
		lpRect->right = m_OrigRect.right;
	}
	
}
