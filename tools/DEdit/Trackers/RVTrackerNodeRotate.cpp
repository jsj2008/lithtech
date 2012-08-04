//////////////////////////////////////////////////////////////////////
// RVTrackerNodeRotate.h - Implementation for the node rotation tracker 

#include "bdefs.h"
#include "rvtrackernoderotate.h"
#include "eventnames.h"

CRVTrackerNodeRotate::CRVTrackerNodeRotate(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter	= FALSE;
	m_bAutoHide		= TRUE;

	m_pStepTracker	= NULL;
	m_bStep			= TRUE;

	FlushTracker();
}

CRVTrackerNodeRotate::~CRVTrackerNodeRotate()
{
	delete m_pStepTracker;
}

void CRVTrackerNodeRotate::FlushTracker()
{
	delete m_pStepTracker;
	m_pStepTracker = new CRegionViewTracker(UIE_NODE_ROTATE_STEP, m_pView);
	if(m_pStepTracker)
	{
		m_pStepTracker->SetAutoCapture(FALSE);
		m_pStepTracker->SetAutoCenter(FALSE);
		m_pStepTracker->SetAutoHide(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerNodeRotate::WatchEvent(const CUIEvent &cEvent)
{
	// Keep track of the step toggle
	if(m_pStepTracker)
	{
		m_pStepTracker->ProcessEvent(cEvent);
		m_bStep = !m_pStepTracker->GetActive();
	}

	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNodeRotate::OnStart()
{
	// Only start if we're in brush or object mode
	if ((m_pView->GetEditMode() != BRUSH_EDITMODE) && (m_pView->GetEditMode() != OBJECT_EDITMODE))
		return FALSE;

	PreActionList actionList;
	DWORD i;
	CEditRegion *pRegion = m_pView->GetRegion();

	// Don't activate if nothing's selected
	if (!pRegion->m_Selections.GetSize())
		return FALSE;

	// Setup undos...
	for(i=0; i < pRegion->m_Selections; i++)
		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Selections[i]));

	// Write the undo list
	m_pView->GetRegionDoc()->Modify(&actionList, TRUE);

	// Get the rotation center & direction
	m_vTrackRotationVector = m_pView->m_pViewDef->m_Nav.Forward( );
	m_vTrackRotationVector.Norm( );
	m_vTrackRotationCenter = pRegion->m_vMarker;

	m_iTotalRotate = 0;
	m_iTotalMove = 0;

	return TRUE;
}

BOOL CRVTrackerNodeRotate::OnUpdate(const CUIEvent &cEvent)
{
	// Only process on idle events
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update the rotation if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	CReal rRotation;
	CMatrix matRot;
	DWORD i;
	CEditRegion *pRegion = m_pView->GetRegion();

	m_iTotalMove += m_cCurPt.x - m_cLastPt.x;
	int iStepRotate = (m_bStep) ? (m_iTotalMove / 15 * 15) : m_iTotalMove;
	rRotation = ( CReal )( iStepRotate - m_iTotalRotate );
	m_iTotalRotate += (int)rRotation;
	// Make sure input is -2PI <= x <= 2PI...
	if( rRotation >= 0 )
		rRotation = (float)fmod( rRotation * MATH_PI / 180.0f, 2.0f * MATH_PI );
	else
		rRotation = -(float)fmod( -rRotation * MATH_PI / 180.0f, 2.0f * MATH_PI );
	matRot.SetupRot( m_vTrackRotationVector, rRotation );

	// Loop over all selected nodes...
	for( i = 0; i < pRegion->m_Selections.GetSize( ); i++ )
		pRegion->m_Selections[i]->Rotate( matRot, m_vTrackRotationCenter );

	m_pView->GetRegionDoc()->UpdateSelectionBox();

	// Update the other views...
	if( GetApp()->m_bFullUpdate )
		m_pView->GetDocument()->UpdateAllViews(m_pView);

	CenterCursor();

	return TRUE;
}

BOOL CRVTrackerNodeRotate::OnEnd()
{
	DWORD i;
	CLinkedList<CEditBrush*> brushList;
	CEditRegion *pRegion = m_pView->GetRegion();

	for( i = 0; i < pRegion->m_Selections.GetSize( ); i++ )
		if(pRegion->m_Selections[i]->GetType() == Node_Brush)
			brushList.AddTail(pRegion->m_Selections[i]->AsBrush());

	// Update the geometry stuff...
	pRegion->UpdatePlanes(&brushList);
	pRegion->CleanupGeometry(&brushList);

	m_pView->GetRegionDoc()->UpdateSelectionBox();

	// Update the other views...
	if( GetApp()->m_bFullUpdate )
		m_pView->GetDocument()->UpdateAllViews(m_pView);

	m_pView->GetRegionDoc()->SetupPropertiesDlg( FALSE );

	return TRUE;
}

