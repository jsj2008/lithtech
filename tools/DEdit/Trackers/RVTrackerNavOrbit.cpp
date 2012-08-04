//////////////////////////////////////////////////////////////////////
// RVTrackerNavOrbit.cpp - Implementation for the Navigation orbit tracker 

#include "bdefs.h"
#include "rvtrackernavorbit.h"
#include "optionscontrols.h"
#include "eventnames.h"

CRVTrackerNavOrbit::CRVTrackerNavOrbit(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= TRUE;
	m_bMove				= FALSE;
	m_bIdleOnly			= TRUE;

	m_pMoveTracker		= NULL;

	FlushTracker();
}

CRVTrackerNavOrbit::~CRVTrackerNavOrbit()
{
	delete m_pMoveTracker;
}

void CRVTrackerNavOrbit::FlushTracker()
{
	delete m_pMoveTracker;
	m_pMoveTracker = new CRegionViewTracker(UIE_NAV_ORBIT_MOVE, m_pView);
	SetupChildTracker(m_pMoveTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerNavOrbit::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}


// Watch for the shift key to be pressed
void CRVTrackerNavOrbit::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pMoveTracker)
	{
		m_pMoveTracker->ProcessEvent(cEvent);
		m_bMove = m_pMoveTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNavOrbit::OnStart()
{
	//do nothing in parallel views
	if( m_pView->IsParallelViewType() )
	{		
		return FALSE;
	}

	//initialize the focus to the marker
	CRegionDoc* pRegionDoc = m_pView->GetRegionDoc();
	
	m_vOrbitFocus = pRegionDoc->GetRegion()->GetMarker();

	//see if we actually want to center on the selection instead
	if( GetApp()->GetOptions().GetControlsOptions()->IsOrbitAroundSel() &&
		(pRegionDoc->GetRegion()->m_Selections > 0) )
	{
		m_vOrbitFocus = pRegionDoc->m_SelectionMin + (pRegionDoc->m_SelectionMax - pRegionDoc->m_SelectionMin) * 0.5f;
	}

	//make the camera look at the new position, and also preserve the distance so it doesn't
	//jump around
	m_pView->Nav().m_LookAtDist = (m_pView->Nav().Pos() - m_vOrbitFocus).Mag();
	m_pView->Nav().LookAt(m_vOrbitFocus);
	m_pView->Nav().UpdateLooking();


	return TRUE;
}

BOOL CRVTrackerNavOrbit::OnUpdate(const CUIEvent &cEvent)
{
	//do nothing in parallel views
	if( m_pView->IsParallelViewType() )
	{		
		return FALSE;
	}

	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	if(m_bMove)
	{
		CNavigator *pNav = &m_pView->Nav();

		CReal fYDelta = (CReal)(m_cCurPt.y - m_cLastPt.y);

		//shrink the distance
		CReal fDist = pNav->m_LookAtDist;

		//change the distance
		fDist += fYDelta;

		//cap the distance
		if(fDist < 10.0f)
			fDist = 10.0f;

		pNav->m_LookAtDist = fDist;

		pNav->UpdateLooking();
	}
	else
	{
		CReal		xAngle, yAngle;
		CNavigator *pNav = &m_pView->Nav();

		CVector vLookXZ, vDelta, vDeltaXZ;
		CReal lookMag, deltaMag, fraction;

		pNav->Pos() -=	pNav->Right() * (pNav->m_LookAtDist / 100.0f) * 
						((CReal)(m_cCurPt.x - m_cLastPt.x) * 0.5f);

		vDelta = pNav->Up() * (pNav->m_LookAtDist / 100.0f) * ((CReal)(m_cCurPt.y - m_cLastPt.y) * 0.5f);

		vDeltaXZ = vDelta;
		vDeltaXZ.y = 0;

		deltaMag = vDeltaXZ.Mag( );
		if( deltaMag )
		{
			vLookXZ = pNav->m_LookAt - pNav->Pos();
			vLookXZ.y = 0;

			if( vDeltaXZ.Dot( vLookXZ ) > 0.0f )
			{
				lookMag = vLookXZ.Mag( );

				if( lookMag - deltaMag >= 2.0f )
					pNav->Pos() += vDelta;
				else
				{
					fraction = ( lookMag - 2.0f ) / deltaMag;
					pNav->Pos() += vDelta * fraction;
				}
			}
			else
				pNav->Pos() += vDelta;
		}
		//make sure we stay the same distance away from the point
		pNav->UpdateLooking();
	}

	// Reset the cursor position
	CPoint cScreenPt = GetStartPt();
	GetView()->ClientToScreen(&cScreenPt);
	SetCursorPos(cScreenPt.x, cScreenPt.y);

	m_pView->Invalidate(FALSE);

	return TRUE;
}

BOOL CRVTrackerNavOrbit::OnEnd()
{
	return TRUE;
}

