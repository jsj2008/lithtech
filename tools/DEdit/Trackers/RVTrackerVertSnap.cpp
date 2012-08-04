//////////////////////////////////////////////////////////////////////
// RVTrackerVertMove.h - Implementation for the vertex movement tracker 

#include "bdefs.h"
#include "rvtrackervertsnap.h"


CRVTrackerVertSnap::CRVTrackerVertSnap(LPCTSTR pName, CRegionView *pView, ESelectMode eSelMode, ESnapTarget eSnapTarget) :
	CRVTrackerVertMove(pName, pView, eSelMode),
	m_eSnapTarget(eSnapTarget)
{
}

void CRVTrackerVertSnap::FillBrushSet()
{
	// Clear the brush set
	m_BrushSet.Init();

	// Assume the worst, memory allocation-wise
	m_BrushSet.SetCacheSize(m_pView->GetRegion()->m_Selections.GetSize());

	for(uint32 nSelectLoop = 0; nSelectLoop < m_pView->GetRegion()->m_Selections.GetSize(); ++nSelectLoop)
	{
		CWorldNode *pNode = m_pView->GetRegion()->m_Selections[nSelectLoop];

		if(pNode->m_Flags & NODEFLAG_HIDDEN)
			continue;

		// We found a selected brush, so use it
		if(pNode->GetType() == Node_Brush)
		{
			m_BrushSet.Append(pNode->AsBrush());
		}
	}

	// If we didn't find anything selected and visible, copy the brush list
	if (m_pView->GetRegion()->m_Selections.GetSize() == 0)
	{
		m_BrushSet.SetCacheSize(m_pView->GetRegion()->m_Brushes.GetSize());
		for (LPOS pos = m_pView->GetRegion()->m_Brushes; pos; )
		{
			m_BrushSet.Append(m_pView->GetRegion()->m_Brushes.GetNext(pos));
		}
	}
}

BOOL CRVTrackerVertSnap::OnStart()
{
	// Get the set of selected brushes
	FillBrushSet();

	// Call our parent's OnStart
	BOOL bResult = CRVTrackerVertMove::OnStart();

	// Clear out the snap distance because our tracker doesn't use it
	m_vSnapDist.Init();

	return bResult;
}

BOOL CRVTrackerVertSnap::GetVertexFromPoint(const CPoint &cPt, CVector &vResult) 
{
	// Calculate the point on the viewing plane
	CEditRay viewRay = m_pView->ViewDef()->MakeRayFromScreenPoint(cPt);

	BOOL bFoundVert = FALSE;

	// If we're not snapping, the ray already mostly describes what we want..
	if (!m_bSnap)
	{
		vResult = viewRay.m_Pos;

		// Remove the relative forward offset from m_vStartVec
		vResult -= viewRay.m_Dir * viewRay.m_Dir.Dot(vResult - m_vStartVec);

		bFoundVert = TRUE;
	}
	else
	{
		// Start out with a distance equal to one half the edit grid size
		float fMinDist = (float)m_pView->GetGridSpacing() * 0.5f;

		switch (m_eSnapTarget)
		{
			case est_Edge :
				bFoundVert = SnapPointEdge(viewRay, vResult, fMinDist);
				break;
			case est_Vert : 
				bFoundVert = SnapPointVert(viewRay, vResult, fMinDist);
				break;
			default :
				bFoundVert = FALSE;
				break;
		}

		// Didn't find it?  Back off to what the view would give us..
		if (!bFoundVert)
			bFoundVert = m_pView->GetVertexFromPoint(cPt, vResult);
		else
		{
			// Remove the relative forward offset from m_vStartVec
			// Note : I'd rather this didn't have to happen so you wouldn't have to 
			//		  adjust verts in 2 views, but the vertex movement code's relative 
			//		  nature doesn't seem to like that adjustment...
			vResult -= viewRay.m_Dir * viewRay.m_Dir.Dot(vResult - m_vStartVec);
		}
	}

	return TRUE;
}

BOOL CRVTrackerVertSnap::FindClosestVertInBrush(CEditBrush *pBrush, const CEditRay &cRay, CVector &vResult, float &fDist)
{
	BOOL bFoundAVert = FALSE;

	for (uint32 nVertLoop = 0; nVertLoop < pBrush->m_Points.GetSize(); ++nVertLoop)
	{
		// If this point's in our active list, ignore it
		if (m_cMovingVerts.FindElement(CVertRef(pBrush, nVertLoop)) != BAD_INDEX)
			continue;

		// Find the point perpendicular to the ray's origin
		CVector vTestPt = pBrush->m_Points[nVertLoop];
		vTestPt -= cRay.m_Dir * cRay.m_Dir.Dot(vTestPt - cRay.m_Pos);
		// Find out how far away that is
		float fTestDist = (vTestPt - cRay.m_Pos).Mag();
		// Is it a better point?
		if (fTestDist < fDist)
		{
			// Use it
			vResult = pBrush->m_Points[nVertLoop];
			fDist = fTestDist;
			bFoundAVert = TRUE;
		}
	}

	return bFoundAVert;
}

// Calculate the distance from a ray to an edge
// returns FALSE if the ray projects outside of the ends of the edge
static BOOL CalcEdgeDist(const CEditRay &cRay, const CVector &vEdge1, const CVector &vEdge2, CVector &vClosePt, float &fDist)
{
	// Find the edge points projected onto the direction's plane
	CVector vTestPt[2];
	vTestPt[0] = vEdge1 - cRay.m_Dir * cRay.m_Dir.Dot(vEdge1 - cRay.m_Pos);
	vTestPt[1] = vEdge2 - cRay.m_Dir * cRay.m_Dir.Dot(vEdge2 - cRay.m_Pos);
	// Get the projected edge direction
	CVector vTestEdge = vTestPt[1] - vTestPt[0];
	// And how long is that edge?
	float fTestMag = vTestEdge.Mag();
	// If the edge is perpendicular to the plane, return vEdge1
	if (fTestMag < 0.01f)
	{
		vClosePt = vEdge1;
		fDist = (vTestPt[0] - cRay.m_Pos).Mag();
		return TRUE;
	}
	// Normalize the edge
	vTestEdge *= 1.0f / fTestMag;
	// Ok, how far along the edge is the closest point?
	float fEdgeT = vTestEdge.Dot(cRay.m_Pos - vTestPt[0]);
	if ((fEdgeT < 0.0f) || (fEdgeT > fTestMag))
		return FALSE;
	// And how far away is that?
	fDist = ((vTestPt[0] + (vTestEdge * fEdgeT)) - cRay.m_Pos).Mag();
	// Ok, get the closest point from the original edge..
	vClosePt = vEdge1 + (vEdge2 - vEdge1) * (fEdgeT / fTestMag);
	return TRUE;
}

BOOL CRVTrackerVertSnap::FindClosestEdgePtInBrush(CEditBrush *pBrush, const CEditRay &cRay, CVector &vResult, float &fDist)
{
	BOOL bFoundAVert = FALSE;

	for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
	{
		CEditPoly *pPoly = pBrush->m_Polies[nPolyLoop];

		for (uint32 nEdgeLoop = 0; nEdgeLoop < pPoly->NumVerts(); ++nEdgeLoop)
		{
			// Don't adjust edges that have points in our active list
			if ((m_cMovingVerts.FindElement(CVertRef(pBrush, pPoly->Index(nEdgeLoop))) != BAD_INDEX) ||
				(m_cMovingVerts.FindElement(CVertRef(pBrush, pPoly->NextIndex(nEdgeLoop))) != BAD_INDEX))
				continue;
			
			CVector vTestPt;
			float fTestDist;
			if (!CalcEdgeDist(cRay, pPoly->Pt(nEdgeLoop), pPoly->NextPt(nEdgeLoop), vTestPt, fTestDist))
				continue;
			if (fTestDist < fDist)
			{
				vResult = vTestPt;
				fDist = fTestDist;
				bFoundAVert = TRUE;
			}
		}
	}

	return bFoundAVert;
}

BOOL CRVTrackerVertSnap::SnapPointVert(const CEditRay &cRay, CVector &vResult, float fMinDist)
{
	BOOL bFoundVert = FALSE;

	// Find out if anything's closer
	for (uint32 nBrushLoop = 0; nBrushLoop < m_BrushSet.GetSize(); ++nBrushLoop)
	{
		bFoundVert |= FindClosestVertInBrush(m_BrushSet[nBrushLoop], cRay, vResult, fMinDist);
	}

	return bFoundVert;
}

BOOL CRVTrackerVertSnap::SnapPointEdge(const CEditRay &cRay, CVector &vResult, float fMinDist)
{
	BOOL bFoundVert = FALSE;

	// Find out if anything's closer
	for (uint32 nBrushLoop = 0; nBrushLoop < m_BrushSet.GetSize(); ++nBrushLoop)
	{
		bFoundVert |= FindClosestEdgePtInBrush(m_BrushSet[nBrushLoop], cRay, vResult, fMinDist);
	}

	return bFoundVert;
}

