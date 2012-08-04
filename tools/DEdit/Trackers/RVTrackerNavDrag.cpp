//////////////////////////////////////////////////////////////////////
// RVTrackerNavDrag.cpp - Implementation for the Navigation drag tracker 

#include "bdefs.h"
#include "rvtrackernavdrag.h"
#include "eventnames.h"
#include "resource.h"

CRVTrackerNavDrag::CRVTrackerNavDrag(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= FALSE;

	m_bIdleOnly			= TRUE;

	//load in the cursors that we are going to use
	m_hHandCursor		= GetApp()->LoadCursor(IDC_CURSOR_HAND);

	m_bFast				= FALSE;
	m_pFastTracker		= NULL;

	FlushTracker();
}

CRVTrackerNavDrag::~CRVTrackerNavDrag()
{
	delete m_pFastTracker;
}

void CRVTrackerNavDrag::FlushTracker()
{
	//reset the fast tracker
	delete m_pFastTracker;
	m_pFastTracker = new CRegionViewTracker(UIE_NAV_DRAG_FAST, m_pView);
	if(m_pFastTracker)
	{
		m_pFastTracker->SetAutoCapture(FALSE);
		m_pFastTracker->SetAutoHide(FALSE);
		m_pFastTracker->SetAutoCenter(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

// Watch for the shift key to be pressed
void CRVTrackerNavDrag::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pFastTracker)
	{
		m_pFastTracker->ProcessEvent(cEvent);
		m_bFast = m_pFastTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNavDrag::OnStart()
{
	return TRUE;
}

BOOL CRVTrackerNavDrag::OnUpdate(const CUIEvent &cEvent)
{
	//the original cursor
	HCURSOR	hOldCursor = ::GetCursor();

	//need to set up the hand cursor
	::SetCursor(m_hHandCursor);


	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	//see if we are orthographic
	if(m_pView->IsParallelViewType())
	{
		//lets get the position of the last location
		LTVector vLastPos;
		m_pView->GetVertexFromPoint( m_cLastPt, vLastPos, FALSE );
			
		//now get the position of the new location
		LTVector vNewPos;
		m_pView->GetVertexFromPoint( m_cCurPt, vNewPos, FALSE );

		//slide to compensate
		m_pView->Nav().Pos() += (vLastPos - vNewPos) * (m_bFast ? 3.0f : 1.0f);
	}
	else
	{
		//slide to compensate
		int32 nDeltaX = (m_cCurPt.x - m_cLastPt.x) * (m_bFast ? 3 : 1);
		int32 nDeltaY = (m_cCurPt.y - m_cLastPt.y) * (m_bFast ? 3 : 1);

		m_pView->Nav().Pos() -= m_pView->Nav().Right() * (float)nDeltaX;
		m_pView->Nav().Pos() += m_pView->Nav().Up() * (float)nDeltaY;
	}
	

	m_pView->Invalidate();

	//restore the cursor
	::SetCursor(hOldCursor);

	//preserve the point
	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerNavDrag::OnEnd()
{
	//need to free the hand cursor

	return TRUE;
}

