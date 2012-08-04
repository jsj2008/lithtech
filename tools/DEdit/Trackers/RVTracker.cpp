//////////////////////////////////////////////////////////////////////
// RVTracker.cpp - Implementation of the RegionView tracker class

#include "bdefs.h"
#include "..\dedit.h"
#include "rvtracker.h"
#include "globalhotkeydb.h"

CRegionViewTracker::CRegionViewTracker(LPCTSTR pName, CRegionView *pView) : 
	CUITracker(pName), 
	m_pView(pView), 
	m_bAutoCenter(TRUE),
	m_bAutoHide(TRUE),
	m_bAutoCapture(TRUE),
	m_bPhantomEnd(TRUE),
	m_cStartPt(0,0),
	m_cCurPt(0,0),
	m_cLastPt(0,0),
	m_cRect(0,0,0,0),
	m_cLastRect(0,0,0,0),
	m_dwLastUpdateTime(0),
	m_dwTimeDelta(0),
	m_nWheelDelta(0)
{
	FlushTracker();
}

CRegionViewTracker::~CRegionViewTracker() 
{
	// Manually clear the override manager to avoid double-frees
	CUITrackerList *pTrackerList = ((CUITrackerList *)&(m_cOverrideMgr.GetTrackerList()));
	// Note : Using RemoveAll will call the deconstructor, which we don't want..
	while (pTrackerList->GetSize())
		pTrackerList->Remove(pTrackerList->GetSize() - 1);
}

BOOL CRegionViewTracker::StartTracker()
{
	// Set the starting values
	SetStartPt(GetCurPt()); 
	SetRect(CRect( 0,0,0,0 ));
	SetLastRect(CRect( 0,0,0,0 ));
	SetLastUpdateTime(GetTickCount());
	SetTimeDelta(0);
	
	// Call the descendant class
	SetActive(OnStart());

	if (GetActive())
	{
		if (GetAutoCapture() && GetView() && GetEndEventList().GetSize())
			GetView()->SetCapture();
		if (GetAutoCenter())
			CenterCursor();
		if (GetAutoHide())
			HideCursor();
		SetLastPt(GetStartPt());
	}

	return GetActive();
}

BOOL CRegionViewTracker::EndTracker()
{
	// Call the descendant class
	SetActive(!OnEnd());

	if (!GetActive())
	{
		if (GetAutoCapture() && GetView())
			ReleaseCapture();
		if (GetAutoCenter())
			CenterCursor();
		if (GetAutoHide())
			UnHideCursor();
	}

	return GetActive();
}

BOOL CRegionViewTracker::Update(const CUIEvent &cEvent)
{
	// Update the tracking variables 
	CRect cNewRect(GetStartPt().x, GetStartPt().y, GetCurPt().x, GetCurPt().y);
	cNewRect.NormalizeRect();
	SetRect(cNewRect);

	// Set the tracker time deltas
	DWORD dwCurrentTime = GetTickCount();
	SetTimeDelta(dwCurrentTime - GetLastUpdateTime());
	SetLastUpdateTime(dwCurrentTime);

	// Call the descendant class
	SetActive(OnUpdate(cEvent));

	if( GetActive() && GetAutoCenter())
		CenterCursor();

	SetLastRect(GetRect());

	return GetActive();
}

void CRegionViewTracker::CenterCursor()
{
	if (!GetView() || (GetCurPt() == GetStartPt()))
		return;

	CRect cClientRect;
	GetView()->GetWindowRect(cClientRect);
	CPoint cScreenPt(cClientRect.Width() / 2, cClientRect.Height() / 2);
	SetLastPt(cScreenPt);
	SetStartPt(cScreenPt);
	GetView()->ClientToScreen(&cScreenPt);
	SetCursorPos(cScreenPt.x, cScreenPt.y);
}

void CRegionViewTracker::HideCursor()
{
	if (GetView())
		GetView()->SetCapture();
	
	// Ensure that the cursor is hidden
	int iCursorState;
	do
	{
		iCursorState = ShowCursor( FALSE );
	} while (iCursorState > 0);

	// Make sure it's not TOO hidden...
	while (iCursorState < -1)
		iCursorState = ShowCursor(TRUE);
}

void CRegionViewTracker::UnHideCursor()
{
	ReleaseCapture();

	// Ensure that the cursor is visible
	int iCursorState;
	do
	{
		iCursorState = ShowCursor( TRUE );
	} while (iCursorState < 0);

	// Make sure it's not TOO visible...
	while (iCursorState > 0)
		iCursorState = ShowCursor(FALSE);
}

// Add an event to a list
void CRegionViewTracker::AddEvent(CUIEventList &cList, const CUIEvent &cEvent, BOOL bActive)
{
	CUIEvent *cClone = cEvent.Clone();
	cList.Add(cClone);
	if (bActive != GetActive())
		m_cWaitingEventList.Add(cClone);
}

// Remove an event from a list
void CRegionViewTracker::RemoveEvent(CUIEventList &cList, const CUIEvent &cEvent, BOOL bActive)
{
	DWORD uIndex;
	if (!IsEvent(cList, cEvent, &uIndex))
		return;
	if (&cList != &m_cWaitingEventList)
		delete cList[uIndex];
	cList.Remove(uIndex);
	if (bActive != GetActive())
		RemoveEvent(m_cWaitingEventList, cEvent, GetActive());
}

// Find an event in a list
BOOL CRegionViewTracker::IsEvent(const CUIEventList &cList, const CUIEvent &cEvent, DWORD *pIndex) const
{
	for (DWORD dwLoop = 0; dwLoop < cList; dwLoop++)
	{
		if (*(cList[dwLoop]) == cEvent)
		{
			if (pIndex)
				*pIndex = dwLoop;
			return TRUE;
		}
	}
	return FALSE;
}

// Find an inverse event in a list
int CRegionViewTracker::IsInverseEvent(const CUIEventList &cList, const CUIEvent &cEvent, DWORD *pIndex) const
{
	for (DWORD dwLoop = 0; dwLoop < cList; dwLoop++)
	{
		if (cList[dwLoop]->IsInverse(cEvent))
		{
			if (pIndex)
				*pIndex = dwLoop;
			return TRUE;
		}
	}
	return FALSE;
}

void CRegionViewTracker::ClearEventList(CUIEventList &cList)
{
	for (DWORD dwLoop = 0; dwLoop < cList.GetSize(); dwLoop++)
		delete cList[dwLoop];
	cList.RemoveAll();
}

// Process an event
BOOL CRegionViewTracker::ProcessEvent(const CUIEvent &cEvent)
{
	// Update the UI event status
	WatchEvent(cEvent);

	BOOL bForceEnd = FALSE;

	// Update the tracker if it's active
	if (GetActive())
	{
		// Allow an override tracker to take over
		if (GetOverrideMgr()->ProcessEvent(cEvent))
			return TRUE;

		bForceEnd = !Update(cEvent);
	}

	// Find out whether or not we're trying to activate
	if ((!GetActive()) && (!bForceEnd))
	{
		//see if we have any events waiting
		if(GetWaitingEventList().GetSize())
		{
			return GetActive();
		}

		// Only activate if we've got a start event list
		if (GetStartEventList().GetSize())
		{
			// Start the tracker
			StartTracker();
			SetWaitingEventList(GetEndEventList());
		}
	}
	else
	{
		// Only de-activate if we've got an end event list or OnUpdate wanted us to
		if ((GetEndEventList().GetSize() > GetWaitingEventList().GetSize()) || bForceEnd)
		{
			// End the tracker
			EndTracker();
			// Toggle the waiting list
			SetWaitingEventList(GetStartEventList());

			//since this element just caused us to become inactive, that could mean that there
			//are still a bunch of keys depressed if mutliple were needed to activate. If
			//this is truly an inverse keypress, we should just use this as the waiting list
			DWORD nIndex;
			if(IsInverseEvent(GetStartEventList(), cEvent, &nIndex))
			{
				m_cWaitingEventList.RemoveAll();
				m_cWaitingEventList.Add(GetStartEventList()[nIndex]);
			}
		}
	}


	return GetActive();
}

// Keep track of the waiting list
void CRegionViewTracker::WatchEvent(const CUIEvent &cEvent)
{
	const CUIEventList	&StartEventList	= GetStartEventList();
	const CUIEventList	&EndEventList	= GetEndEventList();

	// If using phantom end mode, and a watch event comes through w/o a waiting list, require a re-start
	if (GetPhantomEnd() && (!GetActive() && !GetWaitingEventList().GetSize()))
	{
		m_cWaitingEventList.CopyArray(EndEventList);
		m_cWaitingEventList.AppendArray(StartEventList);
	}

	// Figure out which list we're waiting on
	const CUIEventList &cList = (GetActive()) ? EndEventList : StartEventList;

	DWORD uIndex;

	// Handle an event we're waiting for
	if (IsEvent(GetWaitingEventList(), cEvent, &uIndex))
	{
		m_cWaitingEventList.Remove(uIndex);
	}

	// Handle the inverse of an event we were waiting for
	if (IsInverseEvent(cList, cEvent, &uIndex))
	{
		// Make sure we don't add an event twice
		if (!IsEvent(GetWaitingEventList(), *(cList[uIndex])))
			m_cWaitingEventList.Add(cList[uIndex]);
	}

	switch (cEvent.GetType())
	{
		// Keep track of the mouse location
		case UIEVENT_MOUSEWHEEL:
			SetWheelDelta(((const CUIMouseEvent *)&cEvent)->GetWheelDelta());
		case UIEVENT_MOUSEUP :
		case UIEVENT_MOUSEDOWN :
		case UIEVENT_MOUSEMOVE :
			SetCurPt(((const CUIMouseEvent *)&cEvent)->GetPos());
			break;
	}

}

void CRegionViewTracker::Cancel()
{
	// Only cancel if we're active
	if (!GetActive())
		return;

	// Make sure we let go of the cursor
	UnHideCursor();

	// Cancel the override manager
	GetOverrideMgr()->Cancel();

	// Call the cancel notification
	OnCancel();

	// Go inactive
	SetActive(FALSE);

	// Go back to waiting for a start event
	SetWaitingEventList(GetStartEventList());
}

void CRegionViewTracker::DoAutoScroll()
{
	if (!GetView())
		return;

	// Only auto-scroll parallel views
	if (!GetView()->IsParallelViewType())
		return;

	// Make sure the window is captured
	GetView()->SetCapture();

	// Get the client rectangle
	CRect rect;
	GetView()->GetClientRect( &rect );

	// Shortcut pointer
	CNavigator *pNav = &GetView()->Nav();
	
	// Half window width and height
	int nQuarterWidth=rect.Width()/4;
	int nQuarterHeight=rect.Height()/4;

	CPoint cNewPt = GetCurPt();
	BOOL bChangePos = FALSE;

	if (cNewPt.x < 0 )
	{		
		// Scroll to the left
		pNav->Pos() -= pNav->Right() * ((float)nQuarterWidth/GetView()->ViewDef()->m_Magnify);
				
		cNewPt.x = nQuarterWidth;
		bChangePos = TRUE;
	}
	if ( cNewPt.y < 0 )
	{
		// Scroll up
		pNav->Pos() += pNav->Up() * ((float)nQuarterHeight/GetView()->ViewDef()->m_Magnify);
		
		cNewPt.y = nQuarterHeight;
		bChangePos = TRUE;
	}
	if ( cNewPt.x > rect.right )
	{
		// Scroll to the right
		pNav->Pos() += pNav->Right() * ((float)nQuarterWidth/GetView()->ViewDef()->m_Magnify);
		
		cNewPt.x = rect.right - nQuarterWidth;
		bChangePos = TRUE;
	}
	if ( cNewPt.y > rect.bottom )
	{
		// Scroll down
		pNav->Pos() -= pNav->Up() * ((float)nQuarterHeight/GetView()->ViewDef()->m_Magnify);
		
		cNewPt.y = rect.bottom - nQuarterHeight;
		bChangePos = TRUE;
	}

	if (bChangePos)
	{
		// Get the screen window and cursor pos
		CRect rectWindow;
		GetView()->GetWindowRect(&rectWindow);
	
		SetCursorPos(cNewPt.x + rectWindow.left, cNewPt.y + rectWindow.top);

		// Update the "last" & "start" points with the change
		CPoint cOffset(cNewPt.x - GetCurPt().x, cNewPt.y - GetCurPt().y);
		SetLastPt(CPoint(GetLastPt().x + cOffset.x, GetLastPt().y + cOffset.y));
		SetStartPt(CPoint(GetStartPt().x + cOffset.x, GetStartPt().y + cOffset.y));
		// Update the selection rectangle
		CRect cNewRect(GetStartPt().x, GetStartPt().y, GetCurPt().x, GetCurPt().y);
		cNewRect.NormalizeRect();
		SetRect(cNewRect);

		// Update the current point
		SetCurPt(cNewPt);
	}
}

//used by the below functions to return if it cannot find the appropriate list
//so it returns an empty one
static CUIEventList g_EmptyList;

const CUIEventList & CRegionViewTracker::GetStartEventList() const
{
	const CHotKey* pHotKey = CGlobalHotKeyDB::m_DB.GetHotKey(GetName());
	if(pHotKey)
	{
		return pHotKey->GetStartEventList();
	}
	return g_EmptyList;
}

const CUIEventList & CRegionViewTracker::GetEndEventList() const
{
	const CHotKey* pHotKey = CGlobalHotKeyDB::m_DB.GetHotKey(GetName());
	if(pHotKey)
	{
		return pHotKey->GetEndEventList();
	}
	return g_EmptyList;
}

void CRegionViewTracker::FlushTracker()
{
	m_cWaitingEventList.RemoveAll();
	m_cWaitingEventList.CopyArray(GetStartEventList());

	CUITracker::FlushTracker();
}

