//////////////////////////////////////////////////////////////////////
// RVTrackerCurveEdit.h - Implementation for the curve editing tracker 

#include "bdefs.h"
#include "rvtrackercurveedit.h"

CRVTrackerCurveEdit::CRVTrackerCurveEdit(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = TRUE;

	m_bIdleOnly = TRUE;
}

BOOL CRVTrackerCurveEdit::OnStart()
{
	// Only start if we're in object mode
	if (m_pView->GetEditMode() != OBJECT_EDITMODE)
		return FALSE;


	DWORD i, iClosestObject, iProp, iCtrlProp;
	CEditRegion *pRegion;
	CVectorProp *pProp;
	DVector vClickPt, vTestPt, curCtrlPos;
	float minDistSqr, testDistSqr;
	CBaseEditObj *pObj;
	CPoint testPt;
	DWORD iTangent, propFlags;


	if(!m_pView->IsParallelViewType())
		return FALSE;

	pRegion = m_pView->GetRegion();

	vClickPt.Init((float)m_cStartPt.x, (float)m_cStartPt.y, 0.0f);

	// Find the closest object in the frustum with a bezier tangent.
	m_iCurObject = 0xFFFFFFFF;
	minDistSqr = 500000.0f;
	for(i=0; i < pRegion->m_Objects; i++)
	{
		pObj = pRegion->m_Objects[i];

		if(!(pObj->IsFlagSet(NODEFLAG_SELECTED)))
			continue;

		for(iTangent=0; iTangent < 2; iTangent++)
		{
			propFlags = (iTangent == 0) ? PF_BEZIERPREVTANGENT : PF_BEZIERNEXTTANGENT;
			pProp = (CVectorProp*)pObj->m_PropList.GetPropByFlagsAndType(propFlags, LT_PT_VECTOR, (unsigned int*)&iCtrlProp);
			if(!pProp)
				continue;

			curCtrlPos = pObj->GetPos() + pProp->m_Vector;
			if(!m_pView->TransformAndProjectInFrustum(curCtrlPos, testPt))
				continue;

			vTestPt.Init((float)testPt.x, (float)testPt.y, 0.0f);
			testDistSqr = (vTestPt - vClickPt).MagSqr();
			if(testDistSqr < minDistSqr)
			{
				m_iCurObject = i;
				m_iCurProp = iCtrlProp;
				minDistSqr = testDistSqr;
			}
		}
	}

	if(m_iCurObject == 0xFFFFFFFF)
		return FALSE;

	m_pView->GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Objects[m_iCurObject]), TRUE);

	return TRUE;
}

BOOL CRVTrackerCurveEdit::OnUpdate(const CUIEvent &cEvent)
{
	CEditRegion *pRegion;
	CVectorProp *pProp;
	CBaseEditObj *pObj;
	CEditGrid *pGrid;
	DVector newPos, curCtrlPos;
	DWORD i;

	// Only move during idle
	if (m_bIdleOnly && (cEvent.GetType() != UIEVENT_NONE))
		return TRUE;

	// Jump out if the position hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	pRegion = m_pView->GetRegion();
	if(m_iCurObject >= pRegion->m_Objects.GetSize())
		return FALSE;
	
	pObj = pRegion->m_Objects[m_iCurObject];
	if(m_iCurProp >= pObj->m_PropList.m_Props.GetSize())
		return FALSE;

	pProp = (CVectorProp*)pObj->m_PropList.m_Props[m_iCurProp];
	if(pProp->m_Type != LT_PT_VECTOR)
		return FALSE;

	if(!m_pView->EditGrid().IntersectRay(4, m_pView->ViewDef()->MakeRayFromScreenPoint(m_cCurPt), newPos))
		return FALSE;

	curCtrlPos = pObj->GetPos() + pProp->m_Vector;

	// Use the forward vector of the grid to determine what to set.
	pGrid = &m_pView->EditGrid();
	for(i=0; i < 3; i++)
	{
		if(fabs(pGrid->m_Forward[i]) < 0.0001f)
			curCtrlPos[i] = newPos[i];
	}

	pProp->m_Vector = curCtrlPos - pObj->GetPos();
	
	m_pView->GetDocument()->UpdateAllViews(m_pView);

	m_pView->Invalidate(TRUE);

	return TRUE;
}

BOOL CRVTrackerCurveEdit::OnEnd()
{
	return TRUE;
}

