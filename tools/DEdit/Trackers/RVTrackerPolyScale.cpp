//////////////////////////////////////////////////////////////////////
// RVTrackerPolyScale.h - Implementation for the vertex scaling tracker 

#include "bdefs.h"
#include "rvtrackerpolyscale.h"
#include "eventnames.h"

CRVTrackerPolyScale::CRVTrackerPolyScale(LPCTSTR pName, CRegionView *pView, ESelectMode eSelMode) :
	CRegionViewTracker(pName, pView),
	m_eSelectMode(eSelMode)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = TRUE;

	m_bPerp = FALSE;
	m_pPerpTracker = NULL;

	FlushTracker();
}

CRVTrackerPolyScale::~CRVTrackerPolyScale()
{
	delete m_pPerpTracker;
}

void CRVTrackerPolyScale::FlushTracker()
{
	delete m_pPerpTracker;
	m_pPerpTracker = new CRegionViewTracker(UIE_SCALE_POLY_EXTRUDE, m_pView);

	if(m_pPerpTracker)
	{
		m_pPerpTracker->SetAutoCapture(FALSE);
		m_pPerpTracker->SetAutoHide(FALSE);
		m_pPerpTracker->SetAutoCenter(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

// Watch for the toggle events to happen
void CRVTrackerPolyScale::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pPerpTracker)
	{
		m_pPerpTracker->ProcessEvent(cEvent);
		m_bPerp = m_pPerpTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerPolyScale::OnStart()
{
	CEditPoly		*pPoly;
	CEditBrush		*pBrush;

	// Only start in geometry mode
	if (m_pView->GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;
 
	if (!m_pView->IPoly().IsValid())
		return FALSE;

	pPoly = m_pView->IPoly()();
	pBrush = pPoly->m_pBrush;

	pPoly->GetCenterPoint(m_vScaleRefPt);
	m_vPerpNormal = pPoly->Normal();
	//m_pView->ExtrudePoly( m_pView->IPoly(), m_cMovingVerts, TRUE, FALSE, FALSE );
	
	m_cMovingVerts.SetSize( pPoly->NumVerts() );

	for(int32 i=0; i < pPoly->NumVerts(); i++ )
	{
		m_cMovingVerts[i].Init( pBrush, pPoly->Index(i));
	}
	
	
	// Jump out if the movement list is empty for some reason
	if (!m_cMovingVerts.GetSize())
		return FALSE;

	PreActionList actionList;
	
	BOOL bStart = TRUE;
	
	// Setup an undo.
	for(i=0; i < m_cMovingVerts; i++)
	{
		AddToActionListIfNew(&actionList, 
			new CPreAction(ACTION_MODIFYNODE, m_cMovingVerts[i].m_pBrush), TRUE);
	}

	m_pView->GetRegionDoc()->Modify(&actionList, TRUE);

	return TRUE;
}

BOOL CRVTrackerPolyScale::OnUpdate(const CUIEvent &cEvent)
{
	// Only update on idle events
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	CVector			newVert;
	DWORD			i;

	// Do the autoscroll of the window
	DoAutoScroll();

	CVector vNormal(0.0f,0.0f,0.0f);
	vNormal = m_vPerpNormal;

	if (m_bPerp)
	{
		for( i=0; i < m_cMovingVerts; i++ )
		{
			m_cMovingVerts[i]() -= vNormal * (CReal)(m_cCurPt.y - m_cLastPt.y);
		}
	}
	else 
	{
		CReal scaleFactor = ((CReal)(m_cLastPt.y - m_cCurPt.y)) / 100.0f;

		for( i=0; i < m_cMovingVerts; i++ )
		{
			CEditVert	&vert = m_cMovingVerts[i]();
			CVector vDiff = vert - m_vScaleRefPt;

			vDiff -= vNormal * vNormal.Dot(vDiff);

			vert += vDiff * scaleFactor;
		}
	}

	if( GetApp()->m_bFullUpdate )
	{
		m_pView->GetDocument()->UpdateAllViews(m_pView);
		m_pView->DrawRect();
	}

	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerPolyScale::OnEnd()
{
	DWORD i;

	m_pView->UpdatePlanesOfPolies( m_cMovingVerts );

	m_cMovingVerts.Term();

	m_pView->GetRegionDoc()->UpdateSelectionBox();
	GetPropertiesDlg()->ReadControlValues();

	return TRUE;
}

