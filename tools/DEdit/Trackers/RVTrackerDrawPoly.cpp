//////////////////////////////////////////////////////////////////////
// RVTrackerDrawPoly.h - Implementation for the polygon creation tracker 

#include "bdefs.h"
#include "rvtrackerdrawpoly.h"
#include "eventnames.h"

CRVTrackerDrawPoly::CRVTrackerDrawPoly(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter			= FALSE;
	m_bAutoHide				= FALSE;
	m_bFinishDrawingPoly	= FALSE;
	m_bVertSnap				= FALSE;
	m_bAllowCancel			= TRUE;
	m_pUndoTracker			= NULL;
	m_pNewVertexTracker		= NULL;	
	m_pSplitTracker			= NULL;
	m_pRotateTracker		= NULL;
	m_pInsertEdgeTracker	= NULL;
	m_pVertSnapTracker		= NULL;
	m_pCloseTracker			= NULL;

	FlushTracker();
}

CRVTrackerDrawPoly::~CRVTrackerDrawPoly()
{
	delete m_pUndoTracker;
	delete m_pNewVertexTracker;
	delete m_pSplitTracker;
	delete m_pRotateTracker;
	delete m_pInsertEdgeTracker;
	delete m_pVertSnapTracker;
	delete m_pCloseTracker;
}

void CRVTrackerDrawPoly::FlushTracker()
{
	//out with the old and in with the new
	delete m_pUndoTracker;
	m_pUndoTracker = new CRegionViewTracker(UIE_DRAW_POLY_UNDO, m_pView);
	SetupChildTracker(m_pUndoTracker);

	delete m_pNewVertexTracker;
	m_pNewVertexTracker = new CRegionViewTracker(UIE_DRAW_POLY_NEW_VERTEX, m_pView);
	SetupChildTracker(m_pNewVertexTracker);

	delete m_pSplitTracker;
	m_pSplitTracker = new CRegionViewTracker(UIE_DRAW_POLY_SPLIT, m_pView);
	SetupChildTracker(m_pSplitTracker);

	delete m_pRotateTracker;
	m_pRotateTracker = new CRegionViewTracker(UIE_DRAW_POLY_ROTATE, m_pView);
	SetupChildTracker(m_pRotateTracker);

	delete m_pInsertEdgeTracker;
	m_pInsertEdgeTracker = new CRegionViewTracker(UIE_DRAW_POLY_INSERT_EDGE, m_pView);
	SetupChildTracker(m_pInsertEdgeTracker);

	delete m_pVertSnapTracker;
	m_pVertSnapTracker = new CRegionViewTracker(UIE_DRAW_POLY_VERT_SNAP, m_pView);
	SetupChildTracker(m_pVertSnapTracker);

	delete m_pCloseTracker;
	m_pCloseTracker = new CRegionViewTracker(UIE_DRAW_POLY_CLOSE, m_pView);
	SetupChildTracker(m_pCloseTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerDrawPoly::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}

BOOL CRVTrackerDrawPoly::OnStart()
{
	// Make sure they're in the right edit mode for this..
	if ((m_pView->GetEditMode() != BRUSH_EDITMODE) && (m_pView->GetEditMode() != GEOMETRY_EDITMODE))
		return FALSE;

	//make sure that they weren't waiting for a polygon to finish, this fixes
	//a weird issue where a new tracker would get started after one just finished,
	//causing another brush drawing to begin right after the old one finished
	if(m_bFinishDrawingPoly)
	{
		return FALSE;
	}

	CEditVert		vert;

	if( m_pView->GetVertexFromPoint(m_cCurPt, vert) )
	{
		if( !m_pView->IsPerspectiveViewType( ))
		{
			CVector vOffset;
			vOffset = m_pView->EditGrid( ).Forward( );
			vOffset = m_pView->EditGrid( ).Forward( ) * vOffset.Dot( m_pView->GetRegion( )->m_vMarker );
			vert += vOffset;
		}
		m_pView->m_EditState = EDIT_DRAWINGPOLY;
	}
	else
		// Not sure how this would happen, but if we can't create the first vertex, cancel the tracker
		return FALSE;

	// Setup the new brush.
	m_pView->DrawingBrush().m_Points.Append( vert );
	m_pView->DrawingBrush().m_Points.Append( vert );

	// Update the view
	m_pView->GetDocument()->UpdateAllViews(m_pView);
	m_pView->DrawRect();

	return TRUE;
}

BOOL CRVTrackerDrawPoly::OnUpdate(const CUIEvent &cEvent)
{

	//see if they want to finish drawing the polygon
	if(m_bFinishDrawingPoly)
	{
		// Make sure we don't cancel when the focus goes away
		m_bAllowCancel = FALSE;
		// Create the brush
		m_pView->FinishDrawingPoly();
		m_bAllowCancel = TRUE;

		//we are no longer drawing a poly, so we can start another one
		m_bFinishDrawingPoly = FALSE;

		return FALSE;
	}

	BOOL bCancel = FALSE;

	// Handle any events that happen
	if (m_pUndoTracker)
	{
		BOOL bOldActive = m_pUndoTracker->GetActive();
		m_pUndoTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pUndoTracker->GetActive())
			bCancel = OnUndo();
	}
	if (m_pNewVertexTracker)
	{
		BOOL bOldActive = m_pNewVertexTracker->GetActive();
		m_pNewVertexTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pNewVertexTracker->GetActive())
			bCancel = OnNewVertex();
	}
	if (m_pSplitTracker)
	{
		BOOL bOldActive = m_pSplitTracker->GetActive();
		m_pSplitTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pSplitTracker->GetActive())
			bCancel = OnSplit();
	}
	if (m_pRotateTracker)
	{
		BOOL bOldActive = m_pRotateTracker->GetActive();
		m_pRotateTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pRotateTracker->GetActive())
			bCancel = OnRotate();
	}
	if (m_pInsertEdgeTracker)
	{
		BOOL bOldActive = m_pInsertEdgeTracker->GetActive();
		m_pInsertEdgeTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pInsertEdgeTracker->GetActive())
			bCancel = OnNewEdge();
	}
	if(m_pVertSnapTracker)
	{
		m_pVertSnapTracker->ProcessEvent(cEvent);
		m_bVertSnap = m_pVertSnapTracker->GetActive();
	}
	if(m_pCloseTracker)
	{
		BOOL bOldActive = m_pCloseTracker->GetActive();
		m_pCloseTracker->ProcessEvent(cEvent);
		if((bOldActive == FALSE) && m_pCloseTracker->GetActive())
			bCancel = ClosePoly();
	}

		

	if (bCancel)
		// End the tracker 
		return FALSE;

	// Only update everything else on idle events
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	// Do the autoscroll of the window
	DoAutoScroll();

	switch( m_pView->m_EditState )
	{
		case EDIT_DRAWINGPOLY:
		{
			UpdateDrawPolyVertex(m_cCurPt);
			break;
		}
	}


	// Update the views...
	m_pView->GetDocument()->UpdateAllViews(m_pView);
	m_pView->DrawRect();

	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerDrawPoly::OnEnd()
{
	// Clear out the current polygon
	CEditBrush *pBrush = &m_pView->DrawingBrush();
	while (pBrush->m_Points >= 3)
	{
		pBrush->m_Points.Pop();
		UpdateDrawPolyVertex(m_cCurPt);
	}

	// Reset the edit state
	m_pView->m_EditState = EDIT_NOSTATE;
	// Terminate the poly.
	pBrush->m_Points.Term();							

	m_pView->GetRegionDoc()->UpdateAllViews(m_pView);
	m_pView->DrawRect();

	return TRUE;
}

BOOL CRVTrackerDrawPoly::OnUndo()
{
	BOOL bCancel = FALSE;

	CEditBrush *pBrush = &m_pView->DrawingBrush();

	// Remove the last point that has been added
	if( pBrush->m_Points >= 3 )
	{
		pBrush->m_Points.Pop();
		UpdateDrawPolyVertex(m_cCurPt);
	}
	else
	{
		// Terminate the poly.
		m_pView->m_EditState = EDIT_NOSTATE;
		pBrush->m_Points.Term();
		bCancel = TRUE;
	}
	
	m_pView->GetRegionDoc()->UpdateAllViews(m_pView);
	m_pView->DrawRect();

	return bCancel;
}

BOOL CRVTrackerDrawPoly::OnNewVertex()
{
	CEditBrush *pBrush = &m_pView->DrawingBrush();

	switch (m_pView->m_EditState)
	{
		case EDIT_DRAWINGPOLY:
		{
			//first off, try and get the vertex from the mouse position
			LTVector vVertPos = CalcCurrMouseVert(m_cCurPt);

			//only bother with this point if it is unique
			pBrush->m_Points.Last() = vVertPos;
			pBrush->m_Points.Append( CEditVert(vVertPos) );
			
			//make sure we didn't just add a redundant vertex
			uint32 nNumPts = pBrush->m_Points.GetSize();
			if((nNumPts >= 3) && pBrush->m_Points[nNumPts - 3].NearlyEquals(vVertPos, 0.5f))
			{
				//it is redundant, pop off this new vertex
				pBrush->m_Points.Pop();
			}
			else
			{
				//now see if we have hit the starting vertex
				if( pBrush->m_Points[0].NearlyEquals(pBrush->m_Points.Last(), 1.0f) )
				{
					//we have hit the beginning, but lets make sure that it is a valid
					//polygon
					if(pBrush->m_Points.GetSize() < 5)
					{
						//note that the 5 is because the first must be duplicated, so
						//a triangle would need 4 verts, etc, then there is the extra one
						//for the current position

						//we don't have enough, don't let them close that point
						pBrush->m_Points.Pop();
					}
					else
					{
						//they have closed a valid brush
						// Let go of the input focus
						ReleaseCapture();
						m_bFinishDrawingPoly = TRUE;
					}
				}
			}
				
			break;
		}

	}

	m_pView->GetDocument()->UpdateAllViews(m_pView);
	m_pView->DrawRect();

	return FALSE;
}

BOOL CRVTrackerDrawPoly::OnSplit()
{
	if (m_pView->GetEditMode() != BRUSH_EDITMODE)
		return FALSE;

	m_pView->OnBrushSplitBrush();

	return TRUE;
}

void CRVTrackerDrawPoly::Cancel()
{
	// Call the base cancel if it's allowable
	if (m_bAllowCancel)
		CRegionViewTracker::Cancel();
}

BOOL CRVTrackerDrawPoly::OnRotate()
{
	// Get the brush
	CEditBrush *pBrush = &m_pView->DrawingBrush();

	// Make sure we've got at least three points specified
	if (pBrush->m_Points.GetSize() < 3)
		return FALSE;

	// The origin's point #1
	CVector vOrigin = pBrush->m_Points[0];
	// The original rotation direction's point #2
	CVector vBase = pBrush->m_Points[1];
	// The new rotation's point #3
	CVector vNew = pBrush->m_Points[2];

	// Get the base direction
	CVector vBaseDir = vBase - vOrigin;
	float fBaseMag = vBaseDir.Mag();
	// Don't allow duplicate points
	if (fBaseMag < 0.01f)
		return TRUE;
	vBaseDir *= 1.0f / fBaseMag;

	// Get the rotation direction
	CVector vNewDir = vNew - vOrigin;
	float fNewMag = vNewDir.Mag();
	// Don't allow duplicate points
	if (fNewMag < 0.01f)
		return TRUE;
	vNewDir *= 1.0f / fNewMag;

	// Get the rotation axis
	CVector vRotAxis = vNewDir.Cross(vBaseDir);

	// Get the sin of the angle from the cross product
	float fRotAngle = vRotAxis.Mag();

	// Don't bother if the angle's 0...
	if (fRotAngle == 0.0f)
		return TRUE;

	// Normalize the axis
	vRotAxis *= 1.0f / fRotAngle;

	// Get the actual angle
	fRotAngle = (float)asin(fRotAngle);

	// Handle obtuse angles..
	if (vBaseDir.Dot(vNewDir) < 0.0f)
		fRotAngle = MATH_PI - fRotAngle;

	LTMatrix mRotation;
	// Get the rotation matrix
	mRotation.SetupRot(vRotAxis, fRotAngle);

	// Set up an undo..
	CEditRegion *pRegion = m_pView->GetRegion();
	PreActionList actionList;
	for (uint32 nUndoLoop = 0; nUndoLoop < pRegion->m_Selections; ++nUndoLoop)
		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Selections[nUndoLoop]));
	m_pView->GetRegionDoc()->Modify(&actionList, TRUE);

	// If we're in geometry mode..
	if (m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		// Get the selected vertices
		CVertRefArray vertList;
		m_pView->GetSelectedVerts(vertList);
		// Rotate 'em
		for (uint32 nVertLoop = 0; nVertLoop < vertList.GetSize(); ++nVertLoop)
		{
			CVertRef vert = vertList[nVertLoop];
			if (!vert.IsValid())
				continue;
			vert() -= vOrigin;
			mRotation.Apply(vert());
			vert() += vOrigin;
		}
	}
	else
	{
		// Otherwise, rotate all the selected nodes
		for (uint32 nNodeLoop = 0; nNodeLoop < pRegion->m_Selections.GetSize( ); ++nNodeLoop)
		{
			pRegion->m_Selections[nNodeLoop]->Rotate(mRotation, vOrigin);
		}

		// Update the selection box since stuff rotated...
		m_pView->GetRegionDoc()->UpdateSelectionBox();
	}

	return TRUE;
}

BOOL CRVTrackerDrawPoly::FindSplittingBrushInfo(CEditBrush *pBrush, float &fTestError, CPolyRef &rTestRef, CEditVert &vVert1, CEditVert &vVert2)
{
	fTestError = -1.0f;

	// Get a ray for each of the points that is definately going to point toward the brush
	CBoundingBox BBox = pBrush->CalcBoundingBox();

	float fBigRadius = BBox.Dims().Mag();
	CVector vBrushCenter = (BBox.m_Min + BBox.m_Max) * 0.5f;

	CEditRay ray1;
	ray1.m_Dir = m_pView->Nav().Forward();
	ray1.m_Pos = vVert1 - ray1.m_Dir * (ray1.m_Dir.Dot(vVert1 - vBrushCenter) + fBigRadius);

	CEditRay ray2;
	ray2.m_Dir = m_pView->Nav().Forward();
	ray2.m_Pos = vVert2 - ray2.m_Dir * (ray2.m_Dir.Dot(vVert2 - vBrushCenter) + fBigRadius);

	// Go through the polygons...
	for (uint32 nCurPoly = 0; nCurPoly < pBrush->m_Polies.GetSize(); ++nCurPoly)
	{
		CEditPoly *pPoly = pBrush->m_Polies[nCurPoly];
		// Skip polygons with only 3 vertices
		if (pPoly->NumVerts() < 4)
			continue;
		for (uint32 nFirstVert = 0; nFirstVert < pPoly->NumVerts(); ++nFirstVert)
		{
			float fFirstError = ray1.DistTo(pPoly->Pt(nFirstVert));
			// Skip "first" vertices that are worse than the best anyway..
			if ((fTestError >= 0.0f) && (fFirstError > fTestError))
				continue;

			// Look for a better pair...
			uint32 nSecondVert = (nFirstVert + 2) % pPoly->NumVerts();
			do
			{
				// Test this pair
				float fThisError = fFirstError + ray2.DistTo(pPoly->Pt(nSecondVert));

				// Did we find a new best?
				if ((fTestError < 0.0f) || (fThisError < fTestError))
				{
					fTestError = fThisError;
					vVert1 = pPoly->Pt(nFirstVert);
					vVert2 = pPoly->Pt(nSecondVert);
					rTestRef.Init(pBrush, nCurPoly);
				}

				// Move to the next vertex
				nSecondVert = (nSecondVert + 1) % pPoly->NumVerts();
			// Loop until we're looking at a neighbor of the vertex again
			} while (((nSecondVert + 1) % pPoly->NumVerts()) != nFirstVert);
		}
	}

	// Did we find a good one?
	return fTestError >= 0.0f;
}

BOOL CRVTrackerDrawPoly::FindSplittingInfo(CPolyRef &rRef, CEditVert &vVert1, CEditVert &vVert2)
{
	float fBestError = -1.0f;
	CEditVert vBestVert1, vBestVert2;
	CPolyRef rBestRef;

	// Look globally
	if (!m_pView->GetRegion()->m_Selections.GetSize())
	{
		for(LPOS pos = m_pView->GetRegion()->m_Brushes; pos; )
		{
			CEditBrush *pBrush = m_pView->GetRegion()->m_Brushes.GetNext(pos);

			if (pBrush->m_Flags & NODEFLAG_HIDDEN)
				continue;

			// Find out what our error metric would be on this brush..
			float fTestError;
			CPolyRef rTestRef;
			CEditVert vTestVert1(vVert1), vTestVert2(vVert2);
			if (!FindSplittingBrushInfo(pBrush, fTestError, rTestRef, vTestVert1, vTestVert2))
				continue;

			// Keep the best result
			if ((fBestError < 0.0f) || (fTestError < fBestError))
			{
				fBestError = fTestError;
				rBestRef = rTestRef;
				vBestVert1 = vTestVert1;
				vBestVert2 = vTestVert2;
			}
		}
	}
	// Selected only
	else
	{
		for(uint32 i=0; i < m_pView->GetRegion()->m_Selections.GetSize(); i++ )
		{
			CWorldNode *pNode = m_pView->GetRegion()->m_Selections[i];

			if(pNode->m_Flags & NODEFLAG_HIDDEN)
				continue;

			if(pNode->GetType() != Node_Brush)
				continue;

			// Get the brush pointer...
			CEditBrush *pBrush = pNode->AsBrush();

			// Find out what our error metric would be on this brush..
			float fTestError;
			CPolyRef rTestRef;
			CEditVert vTestVert1(vVert1), vTestVert2(vVert2);
			if (!FindSplittingBrushInfo(pBrush, fTestError, rTestRef, vTestVert1, vTestVert2))
				continue;

			// Keep the best result
			if ((fBestError < 0.0f) || (fTestError < fBestError))
			{
				fBestError = fTestError;
				rBestRef = rTestRef;
				vBestVert1 = vTestVert1;
				vBestVert2 = vTestVert2;
			}
		}
	}
	if (fBestError >= 0.0f)
	{
		rRef = rBestRef;
		vVert1 = vBestVert1;
		vVert2 = vBestVert2;
	}
	return fBestError >= 0.0f;
}

BOOL CRVTrackerDrawPoly::OnNewEdge()
{
	// Geometry/brush mode only, please
	if ((m_pView->GetEditMode() != BRUSH_EDITMODE) && 
		(m_pView->GetEditMode() != GEOMETRY_EDITMODE))
		return FALSE;

	// Get the brush
	CEditBrush *pBrush = &m_pView->DrawingBrush();

	// Make sure we've got at least two points specified
	if (pBrush->m_Points.GetSize() < 2)
		return FALSE;

	// Get the splitting polygon & its points
	CPolyRef rPoly;
	if (!FindSplittingInfo(rPoly, pBrush->m_Points[0], pBrush->m_Points[1]))
		return FALSE;

	// Set up an undo
	PreActionList actionList;
	actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, rPoly.m_pBrush->AsNode()));
	m_pView->GetRegionDoc()->Modify(&actionList, TRUE);

	// Get rid of extra points (the splitting doesn't like them in the list..)
	while (pBrush->m_Points.GetSize() > 2)
		pBrush->m_Points.Remove(pBrush->m_Points.GetSize() - 1);

	// Clear the selected objects that are sub-brush (To make sure we don't crash)
	m_pView->ClearSelections(TRUE, TRUE, TRUE, FALSE);

	// Split the polygon
	m_pView->SplitPolyWithEdge(rPoly, pBrush->m_Points);

	// Clean up the brush
	CLinkedList<CEditBrush*> brushList;
	brushList.AddTail(rPoly.m_pBrush);
	m_pView->GetRegion()->CleanupGeometry(&brushList);

	return TRUE;
}

void CRVTrackerDrawPoly::UpdateDrawPolyVertex(CPoint &point )
{
	//just update the last position
	m_pView->DrawingBrush().m_Points.Last() = CalcCurrMouseVert(point);
}

//given a point on the screen, it will calculate the point on the edit grid that should
//be used for the brush
LTVector CRVTrackerDrawPoly::CalcCurrMouseVert(CPoint& point)
{
	LTVector vFinalVec;

	// Move the current poly's last vertex.
	// Invalidate the largest rect from where it was, its current position, and the previous vertex.
	if( m_bVertSnap )
	{
		CVertRef Vert = m_pView->GetClosestVert( point, false, NULL, false );
		
		if( Vert.IsValid() )
		{
			//we now have a vertex, but we need to make sure that it intersects the edit grid,
			//so we want to shoot a ray straight towards the grid from the vertex and find
			//out where it hits. That will then be our point.
			
			//find out the distance of this point away from the grid
			CReal fDist = m_pView->EditGrid().Normal().Dot(Vert()) - m_pView->EditGrid().Dist();

			//now move it onto the grid
			vFinalVec = Vert() - m_pView->EditGrid().Normal() * fDist;
		}
	}
	else
	{
		LTVector vIntersection;

		if( m_pView->GetVertexFromPoint(point, vIntersection) && (m_pView->DrawingBrush().m_Points > 0) )
		{
			if( !m_pView->IsPerspectiveViewType( ))
			{
				LTVector vOffset;
				vOffset = m_pView->EditGrid( ).Forward( );
				vOffset = m_pView->EditGrid( ).Forward( ) * vOffset.Dot( m_pView->GetRegion()->m_vMarker );
				vIntersection += vOffset;
			}
			vFinalVec = vIntersection;
		}
	}

	return vFinalVec;
}

//called to close the polygon. If there are enough vertices, this will
//connect the first and last points together and finishes the polygon
BOOL CRVTrackerDrawPoly::ClosePoly()
{
	CEditBrush *pBrush = &m_pView->DrawingBrush();

	//first off, make sure we have enough vertices
	if(pBrush->m_Points.GetSize() < 4)
	{
		//note that the 4 is because the first must be duplicated, so
		//a triangle would need 4 verts, etc, then there is the extra one
		//for the current position

		//we can bail (false keeps the tracker going)
		return FALSE;
	}

	//pop off the last point which is only being tracked
	pBrush->m_Points.Last() = pBrush->m_Points[0];

	//we have enough vertices, so let us add the first one again, and finish
	pBrush->m_Points.Append( pBrush->m_Points[0] );

	//they have closed a valid brush
	// Let go of the input focus
	ReleaseCapture();
	m_bFinishDrawingPoly = TRUE;

	return FALSE;
}
