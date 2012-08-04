// DragList.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "draglist.h"

/////////////////////////////////////////////////////////////////////////////
// CDragList

CDragList::CDragList() :
	m_bDragging(FALSE),
	m_pDragImage(NULL),
	m_fDropNotify(NULL),
	m_pDropNotifyObject(NULL)
{
}

CDragList::~CDragList()
{
	if (m_pDragImage)
		delete m_pDragImage;
}


BEGIN_MESSAGE_MAP(CDragList, CListCtrl)
	//{{AFX_MSG_MAP(CDragList)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragList message handlers

void CDragList::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	*pResult = 0;

	// Create a drag image centered around the item
	CPoint pt(8,8);
	m_pDragImage = CreateDragImage(pNMListView->iItem, &pt);
	m_pDragImage->BeginDrag(0, CPoint(8,8));
	m_pDragImage->DragEnter(GetDesktopWindow(), pNMListView->ptAction);

	// Set the drag index
	pt = pNMListView->ptAction;
	m_iDragIndex = HitTest(pt);
	m_iDropIndex = -1;

	// Go into drag mode
	m_bDragging = TRUE;

	// Capture the mouse
	SetCapture();
}

void CDragList::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		// Draw the drag image
		CPoint scrPoint(point);

		ClientToScreen(&scrPoint);

		m_pDragImage->DragMove(scrPoint);

		m_pDragImage->DragShowNolock(FALSE);

		// Scroll the list if necessary
	    int iOverItem = HitTest(point);
		int iTopItem = GetTopIndex();
		int iBottomItem = iTopItem + GetCountPerPage() - 1;
		if ((iOverItem == iTopItem) && (iTopItem != 0))
		{
			EnsureVisible(--iOverItem, FALSE);
			UpdateWindow();
		}
		else if ((iOverItem == iBottomItem) && (iBottomItem < (GetItemCount() - 1)))
		{
			EnsureVisible(++iOverItem, FALSE);
			UpdateWindow();
		}

		// Show the drop target
		ShowDropTarget(iOverItem);

		m_pDragImage->DragShowNolock(TRUE);
	}
	
	CListCtrl::OnMouseMove(nFlags, point);
}

void CDragList::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListCtrl::OnLButtonUp(nFlags, point);

	// Drop the item
	if (m_bDragging)
	{
		// Let go of the mouse
		ReleaseCapture();

		// Turn off drag mode
		m_bDragging = FALSE;
		m_pDragImage->DragLeave(GetDesktopWindow());
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;

		ClearDropTarget();

		m_iDropIndex = HitTest(point);
		if (m_iDropIndex < 0)
		{
			CRect rect;
			ClientToScreen(&point);
			GetWindowRect(&rect);
			if (point.y < rect.top)
				m_iDropIndex = GetTopIndex();
			else
				m_iDropIndex = LTMIN(GetTopIndex() + GetCountPerPage(), GetItemCount());
		}

		// Call the notification function
		if (m_pDropNotifyObject && m_fDropNotify)
			(m_pDropNotifyObject->*m_fDropNotify)(m_iDropIndex);
	}
}

void CDragList::ShowDropTarget(int iItem)
{
	// Draw an indication of the drop target
	SetHotItem(LTMAX(iItem, 0));
	UpdateWindow();
}

void CDragList::ClearDropTarget()
{
	SetHotItem(-1);
	UpdateWindow();
}