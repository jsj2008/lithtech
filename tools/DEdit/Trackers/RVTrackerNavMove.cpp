//////////////////////////////////////////////////////////////////////
// RVTrackerNavMove.h - Implementation for the Navigation movement tracker 

#include "bdefs.h"
#include "rvtrackernavmove.h"
#include "eventnames.h"

CRVTrackerNavMove::CRVTrackerNavMove(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= TRUE;

	m_bPerp				= FALSE;
	m_pPerpTracker		= NULL;
	
	m_bFast				= FALSE;
	m_pFastTracker		= NULL;

	m_bIdleOnly			= TRUE;

	FlushTracker();
}

CRVTrackerNavMove::~CRVTrackerNavMove()
{
	delete m_pPerpTracker;
	delete m_pFastTracker;
}

void CRVTrackerNavMove::FlushTracker()
{
	delete m_pPerpTracker;
	m_pPerpTracker = new CRegionViewTracker(UIE_NAV_MOVE_PERP, m_pView);
	SetupChildTracker(m_pPerpTracker);

	delete m_pFastTracker;
	m_pFastTracker = new CRegionViewTracker(UIE_NAV_MOVE_FAST, m_pView);
	SetupChildTracker(m_pFastTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerNavMove::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}

// Watch for the shift key to be pressed
void CRVTrackerNavMove::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pFastTracker)
	{
		m_pFastTracker->ProcessEvent(cEvent);
		m_bFast = m_pFastTracker->GetActive();
	}

	if(m_pPerpTracker)
	{
		m_pPerpTracker->ProcessEvent(cEvent);
		m_bPerp = m_pPerpTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNavMove::OnStart()
{
	return TRUE;
}

BOOL CRVTrackerNavMove::OnUpdate(const CUIEvent &cEvent)
{
	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	CVector		forward, right;
	CNavigator *pNav = &m_pView->Nav();

	// Set the initial speed value
	float fSpeed = (m_bFast) ? 3.0f : 0.5f;

	// Move the navigator.
	if( m_pView->IsParallelViewType() )
	{
		// Scale the speed based on the current magnification
		fSpeed/=(m_pView->ViewDef()->m_Magnify);

		pNav->Pos() -= pNav->Up() * ((CReal)(m_cCurPt.y - m_cLastPt.y) * fSpeed);
		pNav->Pos() += pNav->Right() * ((CReal)(m_cCurPt.x - m_cLastPt.x) * fSpeed);
	}
	else if (!m_bPerp)
	{
		right = pNav->Right();
		right.y = 0.0f;
		right.Norm();

		forward = pNav->Forward();
		forward.y = 0.0f;
		forward.Norm();

		pNav->Pos() -= forward * ((CReal)(m_cCurPt.y - m_cLastPt.y) * fSpeed);
		pNav->Pos() += right   * ((CReal)(m_cCurPt.x - m_cLastPt.x) * fSpeed);
	}
	// Walk-through perpendicular
	else
	{
		pNav->Pos().y -= ((CReal)(m_cCurPt.y - m_cLastPt.y) * fSpeed);
	}

	CenterCursor();

	m_pView->Invalidate();

	return TRUE;
}

BOOL CRVTrackerNavMove::OnEnd()
{
	return TRUE;
}

