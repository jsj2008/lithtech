//////////////////////////////////////////////////////////////////////
// RVTrackerMarkerMove.h - Implementation for the marker placement tracker 

#include "bdefs.h"
#include "rvtrackermarkermove.h"

CRVTrackerMarkerMove::CRVTrackerMarkerMove(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = FALSE;
}

BOOL CRVTrackerMarkerMove::OnStart()
{
	CEditGrid *pGrid = &m_pView->EditGrid();
	m_vStartVec = m_pView->GetRegion()->m_vMarker;
	if (!m_pView->IsPerspectiveViewType())
		// Snap to grid...
		m_vStartVec -= pGrid->Forward() * (pGrid->Forward().Dot(m_vStartVec - pGrid->Pos()));

	// Force an update...
	m_cLastPt.x = m_cCurPt.x - 1;
	OnUpdate(CUIEvent(UIEVENT_NONE));

	return TRUE;
}

BOOL CRVTrackerMarkerMove::OnUpdate(const CUIEvent &cEvent)
{
	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	// Scroll the marker position into view
	DoAutoScroll();

	CVector			newVert, moveOffset, vDir;
	CEditRay		rayMousePoint;

	// Use the current mouse position and use the delta from the last to update the 
	// position of the node in 3d views...
	if (m_pView->IsPerspectiveViewType())
	{
		rayMousePoint = m_pView->ViewDef()->MakeRayFromScreenPoint(m_cCurPt);
		newVert = rayMousePoint.m_Pos;
		vDir = rayMousePoint.m_Dir;
		vDir.Norm( );
		newVert += vDir * 100.0f;
		newVert.x = ( newVert.x < 0.0f ) ? floor( newVert.x ) : ceil( newVert.x );
		newVert.y = ( newVert.y < 0.0f ) ? floor( newVert.y ) : ceil( newVert.y );
		newVert.z = ( newVert.z < 0.0f ) ? floor( newVert.z ) : ceil( newVert.z );
	}
	else if( !m_pView->GetVertexFromPoint(m_cCurPt, newVert))
		newVert = m_vStartVec;
	
	moveOffset = newVert - m_vStartVec;

	m_pView->GetRegion( )->m_vMarker += moveOffset;

	// Update starting vertex for next pass...
	m_vStartVec = newVert;

	// If user wants all the views updated, then do it...
	if( GetApp()->m_bFullUpdate )
	{
		m_pView->GetDocument()->UpdateAllViews(m_pView);
		// Update this view as well
		m_pView->DrawRect();
	}

	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerMarkerMove::OnEnd()
{
	return TRUE;
}


