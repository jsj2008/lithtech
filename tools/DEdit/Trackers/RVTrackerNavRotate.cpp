//////////////////////////////////////////////////////////////////////
// RVTrackerNavRotate.h - Implementation for the Navigation movement tracker 

#include "bdefs.h"
#include "rvtrackernavrotate.h"
#include "optionscontrols.h"
#include "eventnames.h"

CRVTrackerNavRotate::CRVTrackerNavRotate(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= TRUE;
	m_bForward			= FALSE;
	m_bBackward			= FALSE;
	m_bIdleOnly			= TRUE;

	m_pForwardTracker	= NULL;
	m_pBackwardTracker	= NULL;

	FlushTracker();
}

CRVTrackerNavRotate::~CRVTrackerNavRotate()
{
	delete m_pForwardTracker;
	delete m_pBackwardTracker;
}

void CRVTrackerNavRotate::FlushTracker()
{
	delete m_pForwardTracker;
	m_pForwardTracker = new CRegionViewTracker(UIE_NAV_ROTATE_FORWARD, m_pView);
	SetupChildTracker(m_pForwardTracker);

	delete m_pBackwardTracker;
	m_pBackwardTracker = new CRegionViewTracker(UIE_NAV_ROTATE_BACKWARD, m_pView);
	SetupChildTracker(m_pBackwardTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerNavRotate::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}


// Watch for the shift key to be pressed
void CRVTrackerNavRotate::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pForwardTracker)
	{
		m_pForwardTracker->ProcessEvent(cEvent);
		m_bForward = m_pForwardTracker->GetActive();
	}

	if(m_pBackwardTracker)
	{
		m_pBackwardTracker->ProcessEvent(cEvent);
		m_bBackward = m_pBackwardTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNavRotate::OnStart()
{
	m_dwIdleDelta = 0;

	return TRUE;
}

BOOL CRVTrackerNavRotate::OnUpdate(const CUIEvent &cEvent)
{
	// Keep track of time between idles
	m_dwIdleDelta += GetTimeDelta();

	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	CReal		xAngle, yAngle;
	CNavigator *pNav = &m_pView->Nav();

	// Magnify in parallel views
	if( m_pView->IsParallelViewType() )
	{		
		m_pView->ViewDef()->m_Magnify += ((CReal)(m_cCurPt.y - m_cLastPt.y) * m_pView->ViewDef()->m_Magnify) / 300.0f;
		if( m_pView->ViewDef()->m_Magnify < 0.0001f )
			m_pView->ViewDef()->m_Magnify = 0.0001f;
	}
	else
	{
	
		xAngle = (CReal)(m_cCurPt.y - m_cLastPt.y) * 0.005f;
		yAngle = (CReal)(m_cCurPt.x - m_cLastPt.x) * 0.005f;

		// invert mouse y if the user has this option set
		if( GetApp()->GetOptions().GetControlsOptions()->IsInvertMouseY() )
			xAngle *= -1.0f;

		pNav->ORotY( yAngle );
		pNav->ORotX( xAngle );
		
		// Restrict their rotation so the camera doesn't roll.
		pNav->Right().y = 0;
		pNav->Right().Norm();

		pNav->MakeForward();
		pNav->Forward().Norm();

		pNav->MakeUp();
		pNav->Up().Norm();

		// The direction to move
		float fDirection=0.0f;
		if (m_bForward)
		{
			fDirection=1.0f;
		}
		else if (m_bBackward)
		{
			fDirection=-1.0f;
		}
		
		// The number of units to move per second
		float fUnitsPerSecond=225.0f;

		// The number of units to move based on time
		float fUnits=((float)m_dwIdleDelta/1000.0f)*fUnitsPerSecond;

		// Move the camera	
		pNav->Pos() += pNav->Forward()*fDirection*fUnits;
	}

	// Reset the idle delta accumulator
	m_dwIdleDelta = 0;

	// Reset the cursor position
	CPoint cScreenPt = GetStartPt();
	GetView()->ClientToScreen(&cScreenPt);
	SetCursorPos(cScreenPt.x, cScreenPt.y);

	m_pView->Invalidate(FALSE);

	return TRUE;
}

BOOL CRVTrackerNavRotate::OnEnd()
{
	return TRUE;
}

