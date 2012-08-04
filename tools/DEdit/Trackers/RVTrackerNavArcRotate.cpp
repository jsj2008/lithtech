//////////////////////////////////////////////////////////////////////
// RVTrackerNavArcRotate.cpp - Implementation for the Navigation arc rotate tracker 

#include "bdefs.h"
#include "rvtrackernavarcrotate.h"
#include "eventnames.h"
#include "optionscontrols.h"
#include "resource.h"

CRVTrackerNavArcRotate::CRVTrackerNavArcRotate(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= FALSE;

	m_bIdleOnly			= TRUE;

	//load in the cursors that we are going to use
	m_hRotateXY			= GetApp()->LoadCursor(IDC_CURSOR_ROTATE);
	m_hRotateZ			= GetApp()->LoadCursor(IDC_CURSOR_ROTATE_Z);

	m_bRoll				= FALSE;
	m_pRollTracker		= NULL;

	FlushTracker();
}

CRVTrackerNavArcRotate::~CRVTrackerNavArcRotate()
{
	delete m_pRollTracker;
}

void CRVTrackerNavArcRotate::FlushTracker()
{
	//reset the fast tracker
	delete m_pRollTracker;
	m_pRollTracker = new CRegionViewTracker(UIE_NAV_ARC_ROTATE_ROLL, m_pView);
	if(m_pRollTracker)
	{
		m_pRollTracker->SetAutoCapture(FALSE);
		m_pRollTracker->SetAutoHide(FALSE);
		m_pRollTracker->SetAutoCenter(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

// Watch for the shift key to be pressed
void CRVTrackerNavArcRotate::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pRollTracker)
	{
		m_pRollTracker->ProcessEvent(cEvent);
		m_bRoll = m_pRollTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNavArcRotate::OnStart()
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
		m_pView->Nav().LookAt(m_vOrbitFocus);
		m_pView->Nav().UpdateLooking();
	}

	return TRUE;
}

BOOL CRVTrackerNavArcRotate::OnUpdate(const CUIEvent &cEvent)
{
	//see if we are orthographic
	if(m_pView->IsParallelViewType())
	{
		//we don't really do ortho views with this
		return FALSE;
	}

	//the original cursor
	HCURSOR	hOldCursor = ::GetCursor();
	::SetCursor(m_bRoll ? m_hRotateZ : m_hRotateXY);

	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	
	if(m_bRoll)
	{
		//we are rolling

		//build up two vectors, one to the old point, one to the new

		//first find a center point
		RECT ViewClient;
		m_pView->GetClientRect(&ViewClient);

		int32 nCenterX = (ViewClient.right - ViewClient.left) / 2;
		int32 nCenterY = (ViewClient.bottom - ViewClient.top) / 2;

		//now build up the two vectors
		LTVector vOldVec((float)(nCenterX - m_cLastPt.x), (float)(nCenterY - m_cLastPt.y), 0);
		LTVector vNewVec((float)(nCenterX - m_cCurPt.x), (float)(nCenterY - m_cCurPt.y), 0);

		//normalize
		vOldVec.Norm();
		vNewVec.Norm();

		//find the angle between them
		float fCosVal = vOldVec.Dot(vNewVec);

		float fAngle;

		//prevent the angle from being calculated incorrectly if fCosVal is not
		//in a valid domain.
		if(fCosVal >= 0.9999f)
			fAngle = 0;
		else if(fCosVal <= -0.9999f)
			fAngle = MATH_HALFPI;
		else
			fAngle = (float)acos(fCosVal);

		//see which way we are actually rotating
			
		//create a vector perpendicular to the new vector
		LTVector vPerp(-vNewVec.y, vNewVec.x, 0);

		//now we see which side the old one lies on. If it is in front, we are going
		//backwards
		if(vPerp.Dot(vOldVec) > 0.0f)
		{
			fAngle = -fAngle;
		}

		//build the matrix for the roll
		LTMatrix mRoll;
		mRoll.SetupRot(m_pView->Nav().Forward(), fAngle );

		//now roll around that angle
		mRoll.Apply( m_pView->Nav().Up() );
		m_pView->Nav().MakeRight();

		//ensure orthogonality
		m_pView->Nav().MakeForward();

		//preserve the point
		m_cLastPt = m_cCurPt;
	}
	else
	{
		//we are essentially in orbit mode
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

	m_pView->Invalidate();

	//restore the cursor
	::SetCursor(hOldCursor);

	//update the previous point
	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerNavArcRotate::OnEnd()
{
	return TRUE;
}

