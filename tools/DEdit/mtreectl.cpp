//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// MyTreeCtrl.cpp : implementation file
//

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "bdefs.h"
#include <afxtempl.h>
#include "mtreectl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyTreeCtrl

CMyTreeCtrl::CMyTreeCtrl()
{
	m_bDragging			= FALSE;
	m_pimagelist		= NULL;
	m_nTimerIDHover		= 0;
}

CMyTreeCtrl::~CMyTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CMyTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyTreeCtrl message handlers
void CMyTreeCtrl::OnDestroy()
{
	CImageList	*pimagelist;

	pimagelist = GetImageList(TVSIL_NORMAL);
	if( pimagelist )
		pimagelist->DeleteImageList();
	delete pimagelist;
	pimagelist = GetImageList(TVSIL_STATE);
	if( pimagelist )
		pimagelist->DeleteImageList();
	delete pimagelist;
}

void CMyTreeCtrl::SetNewStyle(long lStyleMask, BOOL bSetBits)
{
	long		lStyleOld;

	lStyleOld = GetWindowLong(m_hWnd, GWL_STYLE);
	lStyleOld &= ~lStyleMask;
	if (bSetBits)
		lStyleOld |= lStyleMask;

	SetWindowLong(m_hWnd, GWL_STYLE, lStyleOld);
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CMyTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	TV_DISPINFO		*ptvinfo;

	ptvinfo = (TV_DISPINFO *)pnmhdr;
	if (ptvinfo->item.pszText != NULL)
	{
		ptvinfo->item.mask = TVIF_TEXT;
		SetItem(&ptvinfo->item);
	}
	*pLResult = TRUE;
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	HTREEITEM			hitem;
	UINT				flags;

	if ( m_nTimerIDHover )
	{
		KillTimer( m_nTimerIDHover );
		m_nTimerIDHover = 0;
	}

	if (m_bDragging)
	{
		ASSERT(m_pimagelist != NULL);
		m_pimagelist->DragMove(point);

		// Start the hover timer
		m_nTimerIDHover = SetTimer(TIMER_ID_HOVER, 500, NULL);
		m_hoverPoint=point;

		if ((hitem = HitTest(point, &flags)) != NULL)
		{
			m_pimagelist->DragLeave(this);
			SelectDropTarget(hitem);
			m_hitemDrop = hitem;
			m_pimagelist->DragEnter(this, point);
		}
		else
		{
			m_hitemDrop = NULL;
		}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

BOOL CMyTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
	do
	{
		if (hitemChild == hitemSuspectedParent)
			break;
	}
	while ((hitemChild = GetParentItem(hitemChild)) != NULL);

	return (hitemChild != NULL);
}


HTREEITEM CMyTreeCtrl::TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
	TV_INSERTSTRUCT		tvstruct;
	TCHAR				sztBuffer[50];
	HTREEITEM			hNewItem, hFirstChild;

		// avoid an infinite recursion situation
	tvstruct.item.hItem = hitemDrag;
	tvstruct.item.cchTextMax = 49;
	tvstruct.item.pszText = sztBuffer;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	GetItem(&tvstruct.item);  // get information of the dragged element
	tvstruct.hParent = hitemDrop;
	tvstruct.hInsertAfter = TVI_SORT;
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	hNewItem = InsertItem(&tvstruct);

	while ((hFirstChild = GetChildItem(hitemDrag)) != NULL)
	{
		this->TransferItem(hFirstChild, hNewItem);  // recursively transfer all the items
		DeleteItem(hFirstChild);		// delete the first child and all its children
	}
	return hNewItem;
}

void CMyTreeCtrl::OnButtonUp()
{
	if (m_bDragging)
	{
		KillTimer( m_nTimerIDScroll );

		ASSERT(m_pimagelist != NULL);
		m_pimagelist->DragLeave(this);
		m_pimagelist->EndDrag();
		delete m_pimagelist;
		m_pimagelist = NULL;

		if (m_hitemDrag != m_hitemDrop && !IsChildNodeOf(m_hitemDrop, m_hitemDrag) && 
															GetParentItem(m_hitemDrag) != m_hitemDrop)
		{
			TransferItem(m_hitemDrag, m_hitemDrop);
			DeleteItem(m_hitemDrag);
		}
		else
			MessageBeep(0);

		m_bDragging = FALSE;
		ReleaseCapture();
		SelectDropTarget(NULL);
	}
}

void CMyTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CMyTreeCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp();
	CTreeCtrl::OnRButtonUp(nFlags, point);
}

void CMyTreeCtrl::OnBeginDrag(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	CPoint		ptAction(0,0);

	*pLResult = 0;

	// This code is to prevent accidental drags.
	if( (GetTickCount() - m_dwDragStart) < DRAG_DELAY )
		return;
	
	// Set up the timer
	m_nTimerIDScroll = SetTimer( TIMER_ID_SCROLL, 25, NULL);

	ASSERT( !m_bDragging );
	m_hitemDrag = (( NM_TREEVIEW * )pnmhdr )->itemNew.hItem;

	m_bDragging = TRUE;
	m_hitemDrop = NULL;

	ASSERT(m_pimagelist == NULL);
	m_pimagelist = CreateDragImage(m_hitemDrag);  // get the image list for dragging
	m_pimagelist->DragShowNolock(TRUE);
	m_pimagelist->SetDragCursorImage(0, CPoint(0, 0));
	m_pimagelist->BeginDrag(0, CPoint(0,0));
	m_pimagelist->DragMove(ptAction);
	m_pimagelist->DragEnter(this, ptAction);
	SetCapture();

}

void CMyTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_dwDragStart = GetTickCount( );
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CMyTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_dwDragStart = GetTickCount( );
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CMyTreeCtrl::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == m_nTimerIDScroll)
	{
		HandleScrollTimer();
	}
	else if (nIDEvent == m_nTimerIDHover)
	{
		HandleHoverTimer();
	}		

	CTreeCtrl::OnTimer(nIDEvent);
}

void CMyTreeCtrl::HandleScrollTimer()
{
	// Doesn't matter that we didn't initialize m_timerticks
	m_timerticks++;

	POINT pt, clientPt;
	GetCursorPos( &pt );
	RECT rect;
	GetClientRect( &rect );
	ClientToScreen( &rect );
	clientPt = pt;
	ScreenToClient( &clientPt );

	CImageList::DragMove(clientPt);

	HTREEITEM hitem = GetFirstVisibleItem();

	if( pt.y < rect.top + 10 )
	{
		// We need to scroll up
		// Scroll slowly if cursor near the treeview control
		int slowscroll = 6 - (rect.top + 10 - pt.y) / 4;
		if( 0 == ( m_timerticks % (slowscroll > 0? slowscroll : 1) ) )
		{
			CImageList::DragShowNolock(FALSE);
			SendMessage( WM_VSCROLL, SB_LINEUP);
			SelectDropTarget(hitem);
			m_hitemDrop = hitem;
			CImageList::DragShowNolock(TRUE);
		}
	}
	else if( pt.y > rect.bottom - 10 )
	{
		// We need to scroll down
		// Scroll slowly if cursor near the treeview control
		int slowscroll = 6 - (pt.y - rect.bottom + 10 ) / 4;
		if( 0 == ( m_timerticks % (slowscroll > 0? slowscroll : 1) ) )
		{
			CImageList::DragShowNolock(FALSE);
			SendMessage( WM_VSCROLL, SB_LINEDOWN);
			int nCount = GetVisibleCount();
			for ( int i=0; i<nCount-1; ++i )
				hitem = GetNextVisibleItem(hitem);
			if( hitem )
				SelectDropTarget(hitem);
			m_hitemDrop = hitem;
			CImageList::DragShowNolock(TRUE);
		}
	}
}

void CMyTreeCtrl::HandleHoverTimer()
{
	// Kill the timer
	KillTimer( m_nTimerIDHover );
	m_nTimerIDHover = 0;
	
	// Get the item that the mouse is over
	HTREEITEM	trItem = 0;
	UINT		uFlag = 0;
	
	trItem = HitTest(m_hoverPoint, &uFlag);
	if (trItem)
	{
		SelectItem(trItem);
		Expand(trItem,TVE_EXPAND);
	}
}

BOOL CMyTreeCtrl::PreTranslateMessage( MSG *pMsg )
{
	if( pMsg->message == WM_KEYDOWN )
	{
		// When an item is being edited make sure the edit control
		// receives certain important key strokes...
		if( GetEditControl( ) && ( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || 
			pMsg->wParam == VK_ESCAPE || GetKeyState( VK_CONTROL )))
		{
			::TranslateMessage( pMsg );
			::DispatchMessage( pMsg );
			// DO NOT process further...
			return TRUE;
		}
	}

	return CTreeCtrl::PreTranslateMessage( pMsg );
}

/************************************************************************/
// Recursively sorts a node and its children
void CMyTreeCtrl::SortNodeAndChildren(HTREEITEM hItem)
{
	// Sort the node
	SortChildren(hItem);

	// Determine if this item has children
	if (ItemHasChildren(hItem))
	{
		// Get the first child item
		HTREEITEM hChild=GetChildItem(hItem);

		// Sort the child
		SortNodeAndChildren(hChild);

		// Sort the rest of the children
		HTREEITEM hSibling=GetNextSiblingItem(hChild);
		while (hSibling)
		{
			SortNodeAndChildren(hSibling);
			hSibling=GetNextSiblingItem(hSibling);
		}
	}
}

/************************************************************************/
// Recursively sorts a node and its children using an application
// specific sorting function
void CMyTreeCtrl::SortNodeAndChildren(HTREEITEM hItem, PFNTVCOMPARE lpfnCompare)
{
	// Create the sorting structure
	TVSORTCB sort;
	sort.hParent=hItem;
	sort.lParam=0;
	sort.lpfnCompare=lpfnCompare;

	// Sort the node
	SortChildrenCB(&sort);

	// Determine if this item has children
	if (ItemHasChildren(hItem))
	{
		// Get the first child item
		HTREEITEM hChild=GetChildItem(hItem);

		// Sort the child
		SortNodeAndChildren(hChild, lpfnCompare);

		// Sort the rest of the children
		HTREEITEM hSibling=GetNextSiblingItem(hChild);
		while (hSibling)
		{
			SortNodeAndChildren(hSibling, lpfnCompare);
			hSibling=GetNextSiblingItem(hSibling);
		}
	}
}

/************************************************************************/
// Sets the items font
void CMyTreeCtrl::SetItemFont(HTREEITEM hItem, LOGFONT& logfont)
{
	Color_Font cf;
	if( !m_mapColorFont.Lookup( hItem, cf ) )
	{
		cf.color = (COLORREF)-1;
	}

	cf.logfont = logfont;	m_mapColorFont[hItem] = cf;
}

/************************************************************************/
// Sets the item bold state
void CMyTreeCtrl::SetItemBold(HTREEITEM hItem, BOOL bBold)
{
	SetItemState( hItem, bBold ? TVIS_BOLD: 0, TVIS_BOLD );
}

/************************************************************************/
// Returns bold state for an otem
BOOL CMyTreeCtrl::GetItemBold(HTREEITEM hItem)
{
	return GetItemState( hItem, TVIS_BOLD ) & TVIS_BOLD;
}

/************************************************************************/
// Sets the item color
void CMyTreeCtrl::SetItemColor(HTREEITEM hItem, COLORREF color)
{
	Color_Font cf;
	if( !m_mapColorFont.Lookup( hItem, cf ) )
	{
		cf.logfont.lfFaceName[0] = '\0';
	}

	cf.color = color;
	m_mapColorFont[hItem] = cf;
}

/************************************************************************/
// Returns the item color
COLORREF CMyTreeCtrl::GetItemColor(HTREEITEM hItem)
{
	// Returns (COLORREF)-1 if color was not set
	Color_Font cf;
	if( !m_mapColorFont.Lookup( hItem, cf ) )
	{
		return (COLORREF)-1;
	}
	return cf.color;
}

/************************************************************************/
// Sets the item font
BOOL CMyTreeCtrl::GetItemFont(HTREEITEM hItem, LOGFONT * plogfont)
{
	Color_Font cf;

	if( !m_mapColorFont.Lookup( hItem, cf ) )
	{
		return FALSE;
	}

	if( cf.logfont.lfFaceName[0] == '\0' )
	{
		return FALSE;
	}
	
	*plogfont = cf.logfont;

	return TRUE;
}

void CMyTreeCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC( &dc );
	
	CRect rcClip, rcClient;
	dc.GetClipBox( &rcClip );
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
	CBitmap *pOldBitmap=memDC.SelectObject( &bitmap );

	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect( &rcClip );
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// First let the control do its default drawing.
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

	HTREEITEM hItem = GetFirstVisibleItem();
	
	int n = GetVisibleCount()+1;
	while( hItem && n--)
	{
		CRect rect;

		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		
		Color_Font cf;	
		if ( !(GetItemState( hItem, selflag ) & selflag ) && m_mapColorFont.Lookup( hItem, cf ))
		{
			CFont *pFontDC;
			CFont fontDC;

			LOGFONT logfont;
			
			if( cf.logfont.lfFaceName[0] != '\0' )
			{
				logfont = cf.logfont;
			}
			else
			{
				// No font specified, so use window font
				CFont *pFont = GetFont();
				pFont->GetLogFont( &logfont );
			}
			
			if( GetItemBold( hItem ) )
			{
				logfont.lfWeight = 700;
			}

			fontDC.CreateFontIndirect( &logfont );
			pFontDC = memDC.SelectObject( &fontDC );
			
			if( cf.color != (COLORREF)-1 )
			{
				memDC.SetTextColor( cf.color );
			}
			
			CString sItem = GetItemText( hItem );
			GetItemRect( hItem, &rect, TRUE );
			memDC.SetBkColor( GetSysColor( COLOR_WINDOW ) );
			memDC.TextOut( rect.left+2, rect.top+1, sItem );			

			memDC.SelectObject( pFontDC );
		}	
		hItem = GetNextVisibleItem( hItem );
	}

	dc.BitBlt( rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC,
			   rcClip.left, rcClip.top, SRCCOPY );

	memDC.SelectObject(pOldBitmap);
	bitmap.DeleteObject();
}
