//////////////////////////////////////////////////////////////////////
// RVTrackerZoom.h - Implementation for the mouse wheel zooming tracker 

#include "bdefs.h"
#include "rvtrackerzoom.h"
#include "optionscontrols.h"
#include "eventnames.h"

CRVTrackerZoom::CRVTrackerZoom(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter	= FALSE;
	m_bAutoHide		= FALSE;

	m_bFast			= FALSE;
	m_pFastTracker	= NULL;

	m_bIdleOnly		= TRUE;

	FlushTracker();
}

CRVTrackerZoom::~CRVTrackerZoom()
{
	delete m_pFastTracker;
}

void CRVTrackerZoom::FlushTracker()
{
	delete m_pFastTracker;

	m_pFastTracker = new CRegionViewTracker(UIE_ZOOM_FAST, m_pView);
	if(m_pFastTracker)
	{
		m_pFastTracker->SetAutoCapture(FALSE);
		m_pFastTracker->SetAutoHide(FALSE);
		m_pFastTracker->SetAutoCenter(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

// Watch for the shift key to be pressed
void CRVTrackerZoom::WatchEvent(const CUIEvent &cEvent)
{
	if (m_pFastTracker)
	{
		m_pFastTracker->ProcessEvent(cEvent);
		m_bFast = m_pFastTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerZoom::OnStart()
{
	return TRUE;
}

BOOL CRVTrackerZoom::OnUpdate(const CUIEvent &cEvent)
{
	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	int32 zDelta = GetWheelDelta();

	if( zDelta == 0 )
		return FALSE;

	BOOL bZoomToCursor = GetApp()->GetOptions().GetControlsOptions()->IsZoomToCursor();

	//clear out the wheel delta
	SetWheelDelta( 0 );

	CNavigator *pNav = &m_pView->Nav();

	// Set the initial speed value
	float fSpeed = (m_bFast) ? 3.0f : 0.5f;

	//see if we are in a parallel viewport
	if( m_pView->IsParallelViewType() )
	{
		

		//see if we want to zoom to the cursor
		if(bZoomToCursor)
		{
			// Convert cursor position to world coordinates:
			CVector vCursor;
			CRect rect;
			m_pView->GetWindowRect( &rect );
			CPoint cursorPt = m_cCurPt;
			m_pView->GetVertexFromPoint( cursorPt, vCursor, FALSE );

			// Zoom:
			m_pView->ViewDef()->m_Magnify += ((CReal)zDelta * m_pView->ViewDef()->m_Magnify) / 300.0f;
			if( m_pView->ViewDef()->m_Magnify < 0.0001f )
				m_pView->ViewDef()->m_Magnify = 0.0001f;

			CVector vCursor2;
			m_pView->GetVertexFromPoint( cursorPt, vCursor2, FALSE );
			// This effectively makes the cursor position the focal point for the zoom.
			pNav->Pos() += vCursor - vCursor2;  
		}
		else
		{
			//we just want to zoom in
			m_pView->ViewDef()->m_Magnify += ((CReal)zDelta * m_pView->ViewDef()->m_Magnify) / 300.0f;
			if( m_pView->ViewDef()->m_Magnify < 0.0001f )
				m_pView->ViewDef()->m_Magnify = 0.0001f;
		}
		
	}
	//Walk-through, meaning that we take a ray through the screen
	else 
	{
		//we are in a perspective viewport.

		if(!bZoomToCursor)
		{
			pNav->Pos() += pNav->Forward() * (float)zDelta * fSpeed;
		}
		else
		{
			CEditRay ray = m_pView->ViewDef()->MakeRayFromScreenPoint( m_cCurPt );
			CVector forward = ray.m_Dir;
			forward.Norm();
			pNav->Pos() += forward * ((CReal)zDelta * fSpeed);
		}
	}

	m_pView->Invalidate();

	return TRUE;
}

BOOL CRVTrackerZoom::OnEnd()
{
	return TRUE;
}

