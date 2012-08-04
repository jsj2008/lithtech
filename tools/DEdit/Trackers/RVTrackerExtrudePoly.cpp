//////////////////////////////////////////////////////////////////////
// RVTrackerExtrudePoly.h - Implementation for the extrusion tracker 

#include "bdefs.h"
#include "rvtrackerextrudepoly.h"
#include "eventnames.h"

CRVTrackerExtrudePoly::CRVTrackerExtrudePoly(LPCTSTR pName, CRegionView *pView, bool bNewBrush) :
	CRegionViewTracker(pName, pView),
	m_bNewBrush(bNewBrush)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= TRUE;

	m_bSnap				= FALSE;
	m_pSnapTracker		= NULL;

	FlushTracker();
}

CRVTrackerExtrudePoly::~CRVTrackerExtrudePoly()
{
	delete m_pSnapTracker;
}

void CRVTrackerExtrudePoly::FlushTracker()
{
	delete m_pSnapTracker;
	m_pSnapTracker = new CRegionViewTracker(UIE_EXTRUDE_POLY_SNAP, m_pView);
	if(m_pSnapTracker)
	{
		m_pSnapTracker->SetAutoCapture(FALSE);
		m_pSnapTracker->SetAutoHide(FALSE);
		m_pSnapTracker->SetAutoCenter(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

// Watch for the toggle events to happen
void CRVTrackerExtrudePoly::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pSnapTracker)
	{
		m_pSnapTracker->ProcessEvent(cEvent);
		m_bSnap = m_pSnapTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerExtrudePoly::OnStart()
{
	// Only start in geometry mode
	if (m_pView->GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;


	// Extrude the poly.  
	if (!m_pView->IPoly().IsValid())
		return FALSE;

	//reset the moved amount
	m_fTotalMoved = 0.0f;

	m_pView->IPoly()()->GetCenterPoint(m_vScaleRefPt);
	m_vPerpNormal = m_pView->IPoly()()->Normal();
	m_pView->ExtrudePoly( m_pView->IPoly(), m_cMovingVerts, !m_bNewBrush, false , m_bNewBrush);
	
	// Jump out if the movement list is empty for some reason
	if (!m_cMovingVerts.GetSize())
		return FALSE;

	if(m_bNewBrush)
	{
		// Setup an undo.
		if (m_cMovingVerts > 0)
		m_pView->GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, m_cMovingVerts[0].m_pBrush), TRUE);
	}
	else
	{
		PreActionList actionList;
		DWORD i;
		
		BOOL bStart = TRUE;
		
		// Setup an undo.
		for(i=0; i < m_cMovingVerts; i++)
		{
			AddToActionListIfNew(&actionList, 
				new CPreAction(ACTION_MODIFYNODE, m_cMovingVerts[i].m_pBrush), true);
		}

		m_pView->GetRegionDoc()->Modify(&actionList, TRUE);
	}

	CenterCursor();

	return TRUE;
}

BOOL CRVTrackerExtrudePoly::OnUpdate(const CUIEvent &cEvent)
{
	// Only update on idle events
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	//find the delta we moved
	float fDelta = (float)(m_cCurPt.y - m_cLastPt.y);

	//keep track of our old moved amount
	float fOldMove = m_fTotalMoved;	
	
	//update our total moved amount
	m_fTotalMoved += fDelta;

	//the amount we will want to offset
	float fOffset;

	if(m_bSnap)
	{
		//get our grid size
		float fGridSize = (float)m_pView->GetGridSpacing();

		//find our snap position of the original pos
		float fOldSnapPos = fOldMove - (float)fmod(fOldMove, fGridSize);

		//get our new snap position
		float fNewSnapPos = m_fTotalMoved - (float)fmod(m_fTotalMoved, fGridSize);

		//now find the total offset
		fOffset = fNewSnapPos - fOldSnapPos;
	}
	else
	{
		fOffset = fDelta;
	}

	if(fabs(fOffset) > 0.01f)
	{
		for(uint32 i=0; i < m_cMovingVerts; i++ )
		{
			m_cMovingVerts[i]() -= m_vPerpNormal * fOffset;
			m_cMovingVerts[i].m_pBrush->UpdateBoundingInfo();
		}

		if( GetApp()->m_bFullUpdate )
		{
			m_pView->GetDocument()->UpdateAllViews(m_pView);
			m_pView->DrawRect();
		}
	}

	CenterCursor();

	//m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerExtrudePoly::OnEnd()
{
	DWORD i;

	m_pView->UpdatePlanesOfPolies( m_cMovingVerts );

	m_cMovingVerts.Term();

	m_pView->GetRegionDoc()->UpdateSelectionBox();
	GetPropertiesDlg()->ReadControlValues();

	return TRUE;
}

