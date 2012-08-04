//////////////////////////////////////////////////////////////////////
// RVTrackerTextWrap.h - Implementation for the texture wrapping tracker 

#include "bdefs.h"
#include "rvtrackertextwrap.h"
#include "texture.h"
#include "AutoTextureOptionsDlg.h"
#include "eventnames.h"

//////////////////////////////////////////////////////////////////////////////
// Tracker implementation

CRVTrackerTextureWrap::CRVTrackerTextureWrap(LPCTSTR pName, CRegionView *pView) :
	CRVTrackerTag(pName, pView)
{
	m_pSkipTracker		= NULL;
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= TRUE;
	m_bShowingDialog	= FALSE;

	FlushTracker();
}

CRVTrackerTextureWrap::~CRVTrackerTextureWrap()
{
	delete m_pSkipTracker;
}

void CRVTrackerTextureWrap::FlushTracker()
{
	delete m_pSkipTracker;
	m_pSkipTracker = new CRegionViewTracker(UIE_TEXTURE_WRAP_SKIP, m_pView);
	SetupChildTracker(m_pSkipTracker);

	CRVTrackerTag::FlushTracker();
}

void CRVTrackerTextureWrap::WatchEvent(const CUIEvent &cEvent)
{ 
	if(m_pSkipTracker)
	{
		m_pSkipTracker->ProcessEvent(cEvent);
	}

	CRegionViewTracker::WatchEvent(cEvent); 
}

BOOL CRVTrackerTextureWrap::OnStart()
{
	// Make sure we're in a valid mode...
	if ((m_pView->GetEditMode() != GEOMETRY_EDITMODE) &&
		(m_pView->GetEditMode() != BRUSH_EDITMODE) )
		return FALSE;

	// Get the poly we're pointing at..
	m_cStartPoint = m_pView->GetCurMousePos();
	CEditRay cStartRay = m_pView->ViewDef()->MakeRayFromScreenPoint(m_cStartPoint);
	m_rBasePoly = FindBestPoly(cStartRay);
	// Make sure they clicked on a poly...
	if (!m_rBasePoly.IsValid())
		return FALSE;

	// For geometry mode..
	if (m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		// Make sure we've got the poly we're clicking on selected..
		CPolyRefArray &rTaggedPolies = m_pView->TaggedPolies();
		for (uint32 nFindBaseLoop = 0; nFindBaseLoop < rTaggedPolies.GetSize(); ++nFindBaseLoop)
		{
			if (rTaggedPolies[nFindBaseLoop] == m_rBasePoly)
				break;
		}
		if (nFindBaseLoop >= rTaggedPolies.GetSize())
			return FALSE;
	}
	// For brush mode...
	else if (m_pView->GetEditMode() == BRUSH_EDITMODE)
	{
		// Make sure we're clicking on a selected brush poly
		CEditRegion *pRegion = m_pView->GetRegionDoc()->GetRegion();
		for (uint32 nBrushSearch = 0; nBrushSearch < pRegion->GetNumSelections(); ++nBrushSearch)
		{
			if (pRegion->GetSelection(nBrushSearch)->GetType() == Node_Brush)
			{
				CEditBrush *pBrush = pRegion->GetSelection(nBrushSearch)->AsBrush();
				if (m_rBasePoly.m_pBrush == pBrush)
					break;
			}
		}
		if (nBrushSearch >= pRegion->GetNumSelections())
			return FALSE;
	}

	// Get the intersection point..
	CReal fPolyDist;
	if (!m_rBasePoly()->IntersectRay(cStartRay, fPolyDist, TRUE))
	{
		// Uh..  This really shouldn't happen..  But whatever...
		return FALSE;
	}
	LTVector pos = cStartRay.m_Dir * fPolyDist + cStartRay.m_Pos;
	CEditVert vStartPos(pos);

	// Set up the drawing brush..
	m_pView->DrawingBrush().Term();
	m_pView->DrawingBrush().m_Points.Append(vStartPos);
	m_pView->DrawingBrush().m_Points.Append(vStartPos);

	return TRUE;
}

BOOL CRVTrackerTextureWrap::OnUpdate(const CUIEvent &cEvent)
{
	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	// Get the new ending position
	CEditRay cEndRay = m_pView->ViewDef()->MakeRayFromScreenPoint(m_cCurPt);
	CEditPlane *pPlane = &(m_rBasePoly()->m_Plane);
	CReal fPlaneDist = pPlane->DistTo(cEndRay.m_Pos) / -pPlane->m_Normal.Dot(cEndRay.m_Dir);

	LTVector vPos = cEndRay.m_Dir * fPlaneDist + cEndRay.m_Pos;
	CEditVert vEndPos(vPos);

	// Update the drawing brush
	m_pView->DrawingBrush().m_Points.Last() = vEndPos;

	// Redraw the views
	POSITION pos = m_pView->GetDocument()->GetFirstViewPosition();
	while (pos)
	{
		CView *pView = m_pView->GetDocument()->GetNextView(pos);
		pView->Invalidate(FALSE);
	}

	// Update the last position...
	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerTextureWrap::OnEnd()
{
	// If we're getting OnEnd because the dialog got displayed, skip out
	if (m_bShowingDialog)
		return TRUE;

	// Show the options dialog
	m_bShowingDialog = TRUE;
	CAutoTextureOptionsDlg cDlg;
	// Load the options from the registry
	cDlg.LoadOptionsFromReg();

	//if the skip tracker is active, just avoid the dialog
	int nModalResult;
	if(m_pSkipTracker && m_pSkipTracker->GetActive())
		nModalResult = IDOK;
	else
		nModalResult = cDlg.DoModal();

	m_bShowingDialog = FALSE;

	// Get the wrapping direction
	CVector vWrapDir = m_pView->DrawingBrush().m_Points[1] - m_pView->DrawingBrush().m_Points[0];
	// Clear the drawing brush (so it doesn't hang around if they cancel)
	m_pView->DrawingBrush().Term();

	// Jump out if they cancelled
	if (nModalResult != IDOK)
		return TRUE;
	// Get the style..
	m_nWrapStyle = cDlg.m_nStyle;
	if (m_nWrapStyle >= k_WrapInvalid)
		return TRUE;
	// Save the options to the registry
	cDlg.SaveOptionsToReg();
	// Get the options
	m_bScaleToFit = cDlg.m_bScale;
	m_bAdjustOffset = cDlg.m_bOffset;
	m_bRestrictWalkDir = cDlg.m_bRestrictDir;

	// This might take a while...
	m_pView->BeginWaitCursor();

	// Select the brushes with selected faces in geometry mode so this can be undone
	if (m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		CPolyRefArray &rTaggedPolies = m_pView->TaggedPolies();
		for (uint32 nFindBaseLoop = 0; nFindBaseLoop < rTaggedPolies.GetSize(); ++nFindBaseLoop)
		{
			if (!rTaggedPolies[nFindBaseLoop].IsValid())
				continue;
			m_pView->GetRegion()->SelectNode(rTaggedPolies[nFindBaseLoop]()->m_pBrush->AsNode());
		}
		m_pView->GetRegionDoc()->NotifySelectionChange();
	}

	// They'll probably want to be able to undo this...
	m_pView->GetRegionDoc()->SetupUndoForSelections();

	// If we didn't move, try and figure out a wrap direction
	if (vWrapDir.MagSqr() <= 0.01f)
	{
		// Try going to the right
		vWrapDir = m_pView->Nav().Right();
		// Make sure we're actually still on the poly...
		CEditPlane *pPlane = &(m_rBasePoly()->m_Plane);
		vWrapDir -= pPlane->m_Normal * vWrapDir.Dot(pPlane->m_Normal);
		// If that doesn't work..
		if (vWrapDir.MagSqr() <= 0.01f)
		{
			// Try going forward
			vWrapDir = m_pView->Nav().Forward();
			vWrapDir -= pPlane->m_Normal * vWrapDir.Dot(pPlane->m_Normal);
		}
	}
	vWrapDir.Norm();

	// Build the polygon set
	CTWPolyList aPolyList;
	BuildPolyList(aPolyList);

	// Find our base poly
	uint32 nBasePolyIndex = FindPolyIndex(aPolyList, m_rBasePoly());

	// Wrap the textures, starting at the base poly index
	CTextExtents cExtents;
	WrapTexture(aPolyList[nBasePolyIndex], vWrapDir, cExtents);

	// Scale the textures to an even multiple
	if (m_bScaleToFit)
		ScaleToMultiple(aPolyList, cExtents);

	// Offset the textures to fit at the top/left
	if (m_bAdjustOffset)
		AlignToTopLeft(aPolyList, cExtents);

	// Clear the polygon set
	ClearPolyList(aPolyList);

	// Redraw the views
	POSITION pos = m_pView->GetDocument()->GetFirstViewPosition();
	while (pos)
	{
		CView *pView = m_pView->GetDocument()->GetNextView(pos);
		pView->Invalidate(FALSE);
	}

	// This might take a while...
	m_pView->EndWaitCursor();

	return TRUE;
}

// Build a polygon list
void CRVTrackerTextureWrap::BuildPolyList(CTWPolyList &aList) const
{
	// Get the raw poly list
	if (m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		CPolyRefArray &rTaggedPolies = m_pView->TaggedPolies();
		for (uint32 nFindBaseLoop = 0; nFindBaseLoop < rTaggedPolies.GetSize(); ++nFindBaseLoop)
		{
			if (!rTaggedPolies[nFindBaseLoop].IsValid())
				continue;
			CEditPoly *pTaggedPoly = rTaggedPolies[nFindBaseLoop]();
			// Make sure this poly isn't already in there
			for (uint32 nDupeLoop = 0; nDupeLoop < aList.GetSize(); ++nDupeLoop)
			{
				if (pTaggedPoly == aList[nDupeLoop]->m_pPoly)
					break;
			}
			// Didn't find it?  Good, add this one
			if (nDupeLoop >= aList.GetSize())
			{
				CTWPolyInfo *pNewInfo = new CTWPolyInfo();
				pNewInfo->m_pPoly = pTaggedPoly;
				aList.Append(pNewInfo);
			}
		}
	}
	else if (m_pView->GetEditMode() == BRUSH_EDITMODE)
	{
		CEditRegion *pRegion = m_pView->GetRegionDoc()->GetRegion();
		for (uint32 nBrushLoop = 0; nBrushLoop < pRegion->GetNumSelections(); ++nBrushLoop)
		{
			if (pRegion->GetSelection(nBrushLoop)->GetType() != Node_Brush)
				continue;

			CEditBrush *pBrush = pRegion->GetSelection(nBrushLoop)->AsBrush();
			for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
			{
				CTWPolyInfo *pNewInfo = new CTWPolyInfo();
				
				pNewInfo->m_pPoly = pBrush->m_Polies[nPolyLoop];

				aList.Append(pNewInfo);
			}
		}
	}

	// Get the edge information
	uint32 nPolyLoop;
	for (nPolyLoop = 0; nPolyLoop < aList.GetSize(); ++nPolyLoop)
	{
		CTWPolyInfo *pPolyInfo = aList[nPolyLoop];
		CEditPoly *pPoly = pPolyInfo->m_pPoly;
		for (uint32 nEdgeLoop = 0; nEdgeLoop < pPoly->NumVerts(); ++nEdgeLoop)
		{
			CTWEdgeInfo *pNewEdge = new CTWEdgeInfo(pPoly->Pt(nEdgeLoop), pPoly->NextPt(nEdgeLoop), pPoly->m_Plane.m_Normal);
			pPolyInfo->m_aEdges.Append(pNewEdge);
		}
	}

	// Clear the neighbor list
	for (nPolyLoop = 0; nPolyLoop < aList.GetSize(); ++nPolyLoop)
	{
		CTWPolyInfo *pPolyInfo = aList[nPolyLoop];
		pPolyInfo->m_aNeighbors.SetSize(pPolyInfo->m_aEdges.GetSize());
		for (uint32 nEdgeLoop = 0; nEdgeLoop < pPolyInfo->m_aNeighbors.GetSize(); ++nEdgeLoop)
			pPolyInfo->m_aNeighbors[nEdgeLoop] = NULL;
	}

	// Get the adjacency information (Note : This is the brute-force, really really slow way)
	for (nPolyLoop = 0; nPolyLoop < aList.GetSize(); ++nPolyLoop)
	{
		CTWPolyInfo *pPolyInfo = aList[nPolyLoop];
		for (uint32 nEdgeLoop = 0; nEdgeLoop < pPolyInfo->m_aEdges.GetSize(); ++nEdgeLoop)
		{
			// Skip over this edge if we already know its neighbor
			if (pPolyInfo->m_aNeighbors[nEdgeLoop] != NULL)
				continue;

			CTWEdgeInfo *pEdge = pPolyInfo->m_aEdges[nEdgeLoop];

			// Search the remaining polys looking for a neighbor on this edge
			for (uint32 nSearchPoly = nPolyLoop + 1; nSearchPoly < aList.GetSize(); ++nSearchPoly)
			{
				CTWPolyInfo *pSearchPoly = aList[nSearchPoly];
				for (uint32 nSearchEdge = 0; nSearchEdge < pSearchPoly->m_aEdges.GetSize(); ++nSearchEdge)
				{
					// Skip over edges we already know have another neighbor
					if (pSearchPoly->m_aNeighbors[nSearchEdge] != NULL)
						continue;

					CTWEdgeInfo *pSearchEdge = pSearchPoly->m_aEdges[nSearchEdge];
					// Did we find a matching edge?
					if ((pEdge->m_aPt[0].NearlyEquals(pSearchEdge->m_aPt[0], 0.01f) &&
						pEdge->m_aPt[1].NearlyEquals(pSearchEdge->m_aPt[1], 0.01f)) ||
						(pEdge->m_aPt[0].NearlyEquals(pSearchEdge->m_aPt[1], 0.01f) &&
						pEdge->m_aPt[1].NearlyEquals(pSearchEdge->m_aPt[0], 0.01f)))
					{
						// Join the polys
						pPolyInfo->m_aNeighbors[nEdgeLoop] = pSearchPoly;
						pSearchPoly->m_aNeighbors[nSearchEdge] = pPolyInfo;
						break;
					}
				}
				// Jump out of the search loop if we found one
				if (pPolyInfo->m_aNeighbors[nEdgeLoop] != NULL)
					break;
			}
		}
	}
}

// Find a poly in a list
uint32 CRVTrackerTextureWrap::FindPolyIndex(const CTWPolyList &aList, CEditPoly *pPoly) const
{
	for (uint32 nFindLoop = 0; nFindLoop < aList.GetSize(); ++nFindLoop)
	{
		if (aList[nFindLoop]->m_pPoly == pPoly)
			return nFindLoop;
	}
	ASSERT(!"Base poly not found in polygon list");
	return aList.GetSize();
}

BOOL CRVTrackerTextureWrap::GetSimilarEdgeDir(const CTWPolyInfo *pPoly, const CVector &vDir, CVector &vResult, float fMinDot) const
{
	float fBestEdgeDot = -2.0f;
	uint32 nEdgeLoop = 0;
	do
	{
		CTWEdgeInfo *pCurEdge = pPoly->m_aEdges[nEdgeLoop];
		float fThisDot = pCurEdge->m_vDir.Dot(vDir);
		float fAbsDot = (float)fabs(fThisDot);
		if ((fAbsDot > fBestEdgeDot) && (fAbsDot >= fMinDot))
		{
			if (fThisDot < 0.0f)
				vResult = -pCurEdge->m_vDir;
			else
				vResult = pCurEdge->m_vDir;
			fBestEdgeDot = fAbsDot;
		}
		++nEdgeLoop;
	} while (nEdgeLoop < pPoly->m_aEdges.GetSize());

	return fBestEdgeDot > -2.0f;
}

// Wrap the textures, starting at a poly index
void CRVTrackerTextureWrap::WrapTexture(CTWPolyInfo *pPoly, const CVector &vWrapDir, CTextExtents &cExtents) const
{
	// Mark this poly as wrapped
	pPoly->m_bTouched = TRUE;

	CTexturedPlane& Texture = pPoly->m_pPoly->GetTexture(GetCurrTexture());

	// Get the texture space
	LTVector vWrapO = Texture.GetO();
	LTVector vWrapP = Texture.GetP();
	LTVector vWrapQ = Texture.GetQ();

	// Get the texture offset projections
	float fWrapOdotP = vWrapO.Dot(vWrapP);
	float fWrapOdotQ = vWrapO.Dot(vWrapQ);

	// Update the texturing extents
	for (uint32 nExtentLoop = 0; nExtentLoop < pPoly->m_aEdges.GetSize(); ++nExtentLoop)
	{
		LTVector vEdgePt = pPoly->m_aEdges[nExtentLoop]->m_aPt[0];

		float fCurU = vWrapP.Dot(vEdgePt) - fWrapOdotP;
		float fCurV = vWrapQ.Dot(vEdgePt) - fWrapOdotQ;

		cExtents.m_fMinU = LTMIN(fCurU, cExtents.m_fMinU);
		cExtents.m_fMaxU = LTMAX(fCurU, cExtents.m_fMaxU);
		cExtents.m_fMinV = LTMIN(fCurV, cExtents.m_fMinV);
		cExtents.m_fMaxV = LTMAX(fCurV, cExtents.m_fMaxV);
	}

	CMoArray<uint32> aNeighbors;
	CMoArray<float> aDots;

	// Insert the neighbors into a list in dot-product order
	for (uint32 nNeighborLoop = 0; nNeighborLoop < pPoly->m_aNeighbors.GetSize(); ++nNeighborLoop)
	{
		CTWPolyInfo *pNeighbor = pPoly->m_aNeighbors[nNeighborLoop];

		// Skip edges that don't have a neighbor
		if (!pNeighbor)
			continue;

		// Skip neighbors that are already wrapped
		if (pNeighbor->m_bTouched)
			continue;

		// Get our dot product
		float fCurDot = vWrapDir.Dot(pPoly->m_aEdges[nNeighborLoop]->m_Plane.m_Normal);

		if ((m_bRestrictWalkDir) && (fCurDot < 0.707f))
			continue;

		// Mark this neighbor as touched (to avoid later polygons pushing it onto the stack)
		pNeighbor->m_bTouched = TRUE;

		// Insert it into the list
		for (uint32 nInsertLoop = 0; nInsertLoop < aNeighbors.GetSize(); ++nInsertLoop)
		{
			if (fCurDot > aDots[nInsertLoop])
				break;
		}
		aDots.Insert(nInsertLoop, fCurDot);
		aNeighbors.Insert(nInsertLoop, nNeighborLoop);
	}

	// Recurse through its neighbors
	for (uint32 nWrapLoop = 0; nWrapLoop < aNeighbors.GetSize(); ++nWrapLoop)
	{
		CTWPolyInfo *pNeighbor = pPoly->m_aNeighbors[aNeighbors[nWrapLoop]];
		CTWEdgeInfo *pEdge = pPoly->m_aEdges[aNeighbors[nWrapLoop]];

		//////////////////////////////////////////////////////////////////////////////
		// Wrap this neighbor

		// Create a matrix representing the basis of the polygon in relation to this edge
		LTMatrix mPolyBasis;
		mPolyBasis.SetTranslation(0.0f, 0.0f, 0.0f);
		mPolyBasis.SetBasisVectors(&pEdge->m_vDir, &pPoly->m_pPoly->m_Plane.m_Normal, &pEdge->m_Plane.m_Normal);

		// Create a new basis for the neighbor polygon
		LTMatrix mNeighborBasis;
		LTVector vNeighborForward;
		vNeighborForward = pNeighbor->m_pPoly->m_Plane.m_Normal.Cross(pEdge->m_vDir);
		// Just to be sure..
		vNeighborForward.Norm();
		mNeighborBasis.SetTranslation(0.0f, 0.0f, 0.0f);
		mNeighborBasis.SetBasisVectors(&pEdge->m_vDir, &pNeighbor->m_pPoly->m_Plane.m_Normal, &vNeighborForward);

		// Create a rotation matrix from here to there
		LTMatrix mRotation;
		mRotation = mNeighborBasis * ~mPolyBasis;

		// Rotate the various vectors
		LTVector vNewP;
		LTVector vNewQ;
		LTVector vNewDir;

		mRotation.Apply3x3(vWrapP, vNewP);
		mRotation.Apply3x3(vWrapQ, vNewQ);
		mRotation.Apply3x3(vWrapDir, vNewDir);

		// Rotate the texture basis if we're following a path
		if (m_nWrapStyle == k_WrapPath)
		{
			LTVector vNeighborEdgeDir;
			if (GetSimilarEdgeDir(pNeighbor, vNewDir, vNeighborEdgeDir, 0.707f))
			{
				LTMatrix mRotatedNeighbor;
				LTVector vNeighborRight;
				vNeighborRight = vNeighborEdgeDir.Cross(pNeighbor->m_pPoly->m_Plane.m_Normal);
				vNeighborRight.Norm();
				// Make sure we're pointing the right way...
				if (vNeighborRight.Dot(pEdge->m_vDir) < 0.0f)
					vNeighborRight = -vNeighborRight;
				mRotatedNeighbor.SetTranslation(0.0f, 0.0f, 0.0f);
				mRotatedNeighbor.SetBasisVectors(&vNeighborRight, &pNeighbor->m_pPoly->m_Plane.m_Normal, &vNeighborEdgeDir);
				// Build a basis based on an edge from the current polygon 
				LTVector vBestPolyEdge;
				GetSimilarEdgeDir(pPoly, vWrapDir, vBestPolyEdge);
				LTVector vPolyRight = vBestPolyEdge.Cross(pNeighbor->m_pPoly->m_Plane.m_Normal);
				vPolyRight.Norm();
				// Make sure we're pointing the right way...
				if (vPolyRight.Dot(pEdge->m_vDir) < 0.0f)
					vPolyRight = -vPolyRight;
				// Build the poly edge matrix
				LTMatrix mPolyEdgeBasis;
				mPolyEdgeBasis.SetTranslation(0.0f, 0.0f, 0.0f);
				mPolyEdgeBasis.SetBasisVectors(&vPolyRight, &pNeighbor->m_pPoly->m_Plane.m_Normal, &vBestPolyEdge);

				// Get a matrix from here to there
				LTMatrix mRotator;
				mRotator = mRotatedNeighbor * ~mPolyEdgeBasis;
				// Rotate the texture basis
				mRotator.Apply3x3(vNewP);
				mRotator.Apply3x3(vNewQ);
				// And use the new edge as the new direction
				vNewDir = vNeighborEdgeDir;
			}

			// Remove skew from vNewP/vNewQ
			if ((float)fabs(vNewP.Dot(vNewQ)) > 0.001f)
			{
				float fMagP = vNewP.Mag();
				float fMagQ = vNewQ.Mag();
				vNewQ *= 1.0f / fMagQ;
				vNewP -= vNewQ * vNewQ.Dot(vNewP);
				vNewP.Norm(fMagP);
				vNewQ *= fMagQ;
			}
		}

		// Get the first edge point..
		CVector vEdgePt = pEdge->m_aPt[0];

		// Calculate the texture coordinate at this point
		float fWrapU = vWrapP.Dot(vEdgePt) - fWrapOdotP;
		float fWrapV = vWrapQ.Dot(vEdgePt) - fWrapOdotQ;

		// Build the new offset
		float fNewOdotP = vNewP.Dot(vEdgePt) - fWrapU;
		float fNewOdotQ = vNewQ.Dot(vEdgePt) - fWrapV;
		LTVector vNewO;
		vNewO.Init();
		float fNewPMag = vNewP.MagSqr();
		if (fNewPMag > 0.0f)
			vNewO += vNewP * (fNewOdotP / fNewPMag);
		float fNewQMag = vNewQ.MagSqr();
		if (fNewQMag > 0.0f)
			vNewO += vNewQ * (fNewOdotQ / fNewQMag);

		pNeighbor->m_pPoly->SetTextureSpace(GetCurrTexture(), vNewO, vNewP, vNewQ);

		// Recurse into this neighbor
		WrapTexture(pNeighbor, vNewDir, cExtents);
	}
}

BOOL CRVTrackerTextureWrap::GetFirstTextureMul(CTWPolyList &aList, float &fTextureU, float &fTextureV) const
{
	for (uint32 nFindTextureLoop = 0; nFindTextureLoop < aList.GetSize(); ++nFindTextureLoop)
	{
		DFileIdent* pTexFile = aList[nFindTextureLoop]->m_pPoly->GetTexture(GetCurrTexture()).m_pTextureFile;

		if(pTexFile)
		{
			CTexture *pTexture = dib_GetDibTexture(pTexFile);
			if(pTexture && pTexture->m_pDib)
			{
				// Found one!
				fTextureU = (float)(pTexture->m_UIMipmapScale)/(float)(pTexture->m_pDib->GetWidth());
				fTextureV = (float)(pTexture->m_UIMipmapScale)/(float)(pTexture->m_pDib->GetHeight());
				break;
			}	
		}
	}
	// Did we find one?
	return ((nFindTextureLoop < aList.GetSize()) && (fTextureU > 0.0f) && (fTextureV > 0.0f));
}

void CRVTrackerTextureWrap::ScaleToMultiple(CTWPolyList &aList, CTextExtents &cExtents) const
{
	// Go find out how big the texture is...
	float fTextureU, fTextureV;
	if (!GetFirstTextureMul(aList, fTextureU, fTextureV))
		return;

	// Adjust to the next even multiple
	float fRangeU = (cExtents.m_fMaxU - cExtents.m_fMinU) * fTextureU;
	float fRangeV = (cExtents.m_fMaxV - cExtents.m_fMinV) * fTextureV;
	// Get the texture's scale
	int nRepeatCountU = (int)(fRangeU + 0.5f);
	int nRepeatCountV = (int)(fRangeV + 0.5f);
	nRepeatCountU = LTMAX(nRepeatCountU, 1);
	nRepeatCountV = LTMAX(nRepeatCountV, 1);
	// Calculate the scaling necessary
	float fScaleU;
	if (fRangeU != 0.0f)
	{
		fScaleU = ((float)nRepeatCountU) / fRangeU;
	}
	else
	{
		fScaleU = 1.0f;
	}
	float fScaleV;
	if (fRangeV != 0.0f)
	{
		fScaleV = ((float)nRepeatCountV) / fRangeV;
	}
	else
	{
		fScaleV = 1.0f;
	}

	// Scale the extents
	cExtents.m_fMinU *= fScaleU;
	cExtents.m_fMaxU *= fScaleU;
	cExtents.m_fMinV *= fScaleV;
	cExtents.m_fMaxV *= fScaleV;

	// Scale the texture spaces
	for (uint32 nPolyLoop = 0; nPolyLoop < aList.GetSize(); ++nPolyLoop)
	{
		// Skip polys that weren't touched...
		if (!aList[nPolyLoop]->m_bTouched)
			continue;
		// Get the texture space
		LTVector vPolyO, vPolyP, vPolyQ;
		aList[nPolyLoop]->m_pPoly->GetTexture(GetCurrTexture()).GetTextureSpace(vPolyO, vPolyP, vPolyQ);
		// Scale it
		vPolyP *= fScaleU;
		vPolyQ *= fScaleV;
		// Put it back
		aList[nPolyLoop]->m_pPoly->SetTextureSpace(GetCurrTexture(), vPolyO, vPolyP, vPolyQ);
	}
}

void CRVTrackerTextureWrap::AlignToTopLeft(CTWPolyList &aList, CTextExtents &cExtents) const
{
	// Go find out how big the texture is...
	float fTextureU, fTextureV;
	if (!GetFirstTextureMul(aList, fTextureU, fTextureV))
		return;

	// Figure out how much we need to adjust
	float fMinTextureU = cExtents.m_fMinU * fTextureU;
	int nIntTextureU = (int)(fMinTextureU + 0.5f);
	float fOffsetU = (fMinTextureU - (float)nIntTextureU) / fTextureU;

	float fMinTextureV = cExtents.m_fMinV * fTextureV;
	int nIntTextureV = (int)(fMinTextureV + 0.5f);
	float fOffsetV = (fMinTextureV - (float)nIntTextureV) / fTextureV;

	// Update the extents
	cExtents.m_fMinU += fOffsetU;
	cExtents.m_fMaxU += fOffsetU;
	cExtents.m_fMinV += fOffsetV;
	cExtents.m_fMaxV += fOffsetV;

	// Scale the texture spaces
	for (uint32 nPolyLoop = 0; nPolyLoop < aList.GetSize(); ++nPolyLoop)
	{
		// Skip polys that weren't touched...
		if (!aList[nPolyLoop]->m_bTouched)
			continue;
		// Get the texture space
		LTVector vPolyO, vPolyP, vPolyQ;
		aList[nPolyLoop]->m_pPoly->GetTexture(GetCurrTexture()).GetTextureSpace(vPolyO, vPolyP, vPolyQ);
		// Adjust it
		float fPolyPMag = vPolyP.MagSqr();
		if (fPolyPMag > 0.0f)
			vPolyO += vPolyP * (fOffsetU / fPolyPMag);
		float fPolyQMag = vPolyQ.MagSqr();
		if (fPolyQMag > 0.0f)
			vPolyO += vPolyQ * (fOffsetV / fPolyQMag);
		// Put it back
		aList[nPolyLoop]->m_pPoly->SetTextureSpace(GetCurrTexture(), vPolyO, vPolyP, vPolyQ);
	}
}

// Clear a polygon set
void CRVTrackerTextureWrap::ClearPolyList(CTWPolyList &aList) const
{
	for (uint32 nDeleteLoop = 0; nDeleteLoop < aList.GetSize(); ++nDeleteLoop)
	{
		delete aList[nDeleteLoop];
	}

	aList.Term();
}

