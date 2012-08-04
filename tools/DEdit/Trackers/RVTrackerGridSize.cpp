//////////////////////////////////////////////////////////////////////
// RVTrackerGridSize.h - Implementation for the grid size tracker 

#include "bdefs.h"
#include "rvtrackergridsize.h"

CRVTrackerGridSize::CRVTrackerGridSize(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = TRUE;
	m_bAutoHide = TRUE;
}

BOOL CRVTrackerGridSize::OnStart()
{
	return TRUE;
}

BOOL CRVTrackerGridSize::OnUpdate(const CUIEvent &cEvent)
{
	// Don't update the plane if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	m_pView->SetGridSize(m_pView->GetGridSize() + (m_cCurPt.y - m_cLastPt.y));
	
	return TRUE;
}

BOOL CRVTrackerGridSize::OnEnd()
{
	return TRUE;
}


