//////////////////////////////////////////////////////////////////////
// RVTrackerFarDist.h - Implementation for the far clipping distance tracker 

#include "bdefs.h"
#include "rvtrackerfardist.h"

CRVTrackerFarDist::CRVTrackerFarDist(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = TRUE;
	m_bAutoHide = TRUE;
}

BOOL CRVTrackerFarDist::OnStart()
{
	return TRUE;
}

BOOL CRVTrackerFarDist::OnUpdate(const CUIEvent &cEvent)
{
	// Don't update the plane if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	m_pView->FarZ() -= (m_cCurPt.y - m_cLastPt.y);
	if( m_pView->FarZ() < (m_pView->NearZ() + 10.0f) )
		m_pView->FarZ() = m_pView->NearZ() + 10.0f;

	m_pView->Invalidate(FALSE);

	return TRUE;
}

BOOL CRVTrackerFarDist::OnEnd()
{
	return TRUE;
}


