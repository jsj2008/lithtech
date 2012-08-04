//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : ViewFunctions.cpp
//
//	PURPOSE	  : Implements all the non-message handler functions
//              for CRegionView.
//
//	CREATED	  : October 18 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "optionsbase.h"		//for the registry utils
#include "regiondoc.h"
#include "regionview.h"

#include "dedit.h"
#include "projectbar.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "stringdlg.h"
#include "texture.h"
#include "node_ops.h"
#include "edit_actions.h"
#include "regmgr.h"
#include "sysstreamsim.h"
#include "de_world.h"
#include "pregeometry.h"
#include "geomroutines.h"
#include "optionsviewports.h"
#include "converge.h"
#include "objectselfilterdlg.h"

#include <set>
#include <vector>


// These define which way brush selection handles want to scale.
CVector	g_SelectionHandleVectors[] =
{
	CVector(1.0f, 0.0f, 0.0f),		// X handle
	CVector(-1.0f, 0.0f, 0.0f),		// X handle

	CVector(0.0f, 1.0f, 0.0f),		// Y handle
	CVector(0.0f, -1.0f, 0.0f),		// Y handle

	CVector(0.0f, 0.0f, 1.0f),		// Z handle
	CVector(0.0f, 0.0f, -1.0f),		// Z handle

	// XY handles
	CVector(1.0f,   1.0f, 0.0f),
	CVector(-1.0f,  1.0f, 0.0f),
	CVector(1.0f,  -1.0f, 0.0f),
	CVector(-1.0f, -1.0f, 0.0f),

	// XZ handles
	CVector(1.0f,   0.0f, 1.0f),
	CVector(-1.0f,  0.0f, 1.0f),
	CVector(1.0f,   0.0f, -1.0f),
	CVector(-1.0f,  0.0f, -1.0f),

	// YZ handles
	CVector(0.0f,  1.0f,   1.0f),
	CVector(0.0f,  -1.0f,  1.0f),
	CVector(0.0f,  1.0f,  -1.0f),
	CVector(0.0f,  -1.0f, -1.0f)
};

void CRegionView::LoadProjFile( CAbstractIO &file )
{
	CRect		rect;
	WORD		tempWord;


	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	
	file >> tempWord;	SetGridSpacing(tempWord);

	file >> tempWord;	SetViewMode( (int)tempWord );

	file >> FarZ();

	file >> Nav().Pos().x >> Nav().Pos().y >> Nav().Pos().z;
	file >> Nav().Up().x >> Nav().Up().y >> Nav().Up().z;
	file >> Nav().Right().x >> Nav().Right().y >> Nav().Right().z;
	file >> Nav().Forward().x >> Nav().Forward().y >> Nav().Forward().z;

	file >> Nav().m_LookAt.x >> Nav().m_LookAt.y >> Nav().m_LookAt.z;
	file >> Nav().m_LookAtDist;

	file >> ViewDef()->m_Magnify;
	file >> EditGrid().m_DrawSize;
}


void CRegionView::SaveProjFile( CAbstractIO &file )
{
	CRect		rect;
	WORD		tempWord;
	
	//output an empty header to be backwards compatible
	tempWord = 0;
	file << tempWord;		
	file << tempWord;
	file << tempWord;
	file << tempWord;
	file << tempWord;
	file << tempWord;	
	file << tempWord;

	tempWord = (WORD)m_dwGridSpacing;	file << tempWord;
	
	tempWord = (WORD)GetViewMode();	file << tempWord;

	file << FarZ();

	file << Nav().Pos().x << Nav().Pos().y << Nav().Pos().z;
	file << Nav().Up().x << Nav().Up().y << Nav().Up().z;
	file << Nav().Right().x << Nav().Right().y << Nav().Right().z;
	file << Nav().Forward().x << Nav().Forward().y << Nav().Forward().z;
	
	file << Nav().m_LookAt.x << Nav().m_LookAt.y << Nav().m_LookAt.z;
	file << Nav().m_LookAtDist;

	file << ViewDef()->m_Magnify;
	file << EditGrid().m_DrawSize;
}


CPoint CRegionView::GetCurMousePos()
{
	CPoint		ret;

	GetCursorPos( &ret );
	ScreenToClient( &ret );

	return ret;
}

uint32	CRegionView::GetShadeMode() const
{
	return GetApp()->GetOptions().GetViewportOptions()->GetShadeMode(m_nViewID);
}

void CRegionView::SetShadeMode( int mode )
{
	GetApp()->GetOptions().GetViewportOptions()->SetShadeMode(m_nViewID, mode);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsSelectBackfaces() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsSelectBackfaces(m_nViewID);
}

void CRegionView::SetSelectBackfaces( BOOL bBackface )
{
	GetApp()->GetOptions().GetViewportOptions()->SetSelectBackfaces(m_nViewID, bBackface);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsShowNormals() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsShowNormals(m_nViewID);
}

void CRegionView::SetShowNormals( BOOL bShow )
{
	GetApp()->GetOptions().GetViewportOptions()->SetShowNormals(m_nViewID, bShow);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsShowWireframe() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsShowWireframe(m_nViewID);
}

void CRegionView::SetShowWireframe( BOOL bShow )
{
	GetApp()->GetOptions().GetViewportOptions()->SetShowWireframe(m_nViewID, bShow);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsShowGrid() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsShowGrid(m_nViewID);
}

void CRegionView::SetShowGrid(BOOL bShow)
{
	GetApp()->GetOptions().GetViewportOptions()->SetShowGrid(m_nViewID, bShow);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsShowObjects() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsShowObjects(m_nViewID);
}

void CRegionView::SetShowObjects(BOOL bShow)
{
	GetApp()->GetOptions().GetViewportOptions()->SetShowObjects(m_nViewID, bShow);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

BOOL CRegionView::IsShowMarker() const
{
	return GetApp()->GetOptions().GetViewportOptions()->IsShowMarker(m_nViewID);
}

void CRegionView::SetShowMarker(BOOL bShow)
{
	GetApp()->GetOptions().GetViewportOptions()->SetShowMarker(m_nViewID, bShow);
	GetMainFrame()->UpdateAllRegions(REGIONVIEWUPDATE_VIEW(m_nViewID));
}

int CRegionView::GetEditMode()
{
	return GetMainFrame()->GetWorldEditMode();
}


/////////////////////////////////////////////////////////////////////////////
// Tagging functions

BOOL CRegionView::GetSelectedPoly( CVertRefArray &list )
{
	uint32		i;

	list.SetSize(0);
	if( IPoly().IsValid() )
	{
		for( i=0; i < IPoly()()->NumVerts(); i++ )
			list.Append( CVertRef(IPoly()()->m_pBrush, IPoly()()->Index(i)) );
	}

	return list > 0;
}


BOOL CRegionView::GetSelectedVerts( CVertRefArray &list )
{
	list.GenCopyList(TaggedVerts());
	return list.GenGetSize() > 0;
}


BOOL CRegionView::GetImmediateVert( CVertRefArray &list )
{
	uint32			i;
	
	list.SetSize(0);
	
	if( IVert().IsValid() )
	{
		if( IsParallelViewType() )
		{
			// Find any other points parallel to this one.
			for( i=0; i < IVert().m_pBrush->m_Points; i++ )
			{
				if( i != IVert().m_iVert )
				{
					CEditVert	&testVert = IVert().m_pBrush->m_Points[i];
					
					if( (testVert.m_Projected.x == IVert()().m_Projected.x) && 
						(testVert.m_Projected.y == IVert()().m_Projected.y) )
						list.Append( CVertRef(IVert().m_pBrush, i) );
				}
			}
		}

		list.Append( IVert() );
	}

	return list > 0;
}


CBrushRef CRegionView::GetMouseOverBrush( CReal *pDist, CReal rMinDist )
{
	CPoint		point = GetCurMousePos();
	CEditRay	ray;
	CPolyRef	ref;
	CBrushRef	brushRef;


	ray = ViewDef()->MakeRayFromScreenPoint( point );
	ref = CastRayAtPolies( ray, pDist, rMinDist );
	if( ref.IsValid() )
		brushRef.Init( ref.m_pBrush );
	else
		brushRef.Init( NULL );

	return brushRef;
}





BOOL CRegionView::GetSelectedBrushes( CVertRefArray &list )
{
	uint32		i, k;
	CEditBrush	*pBrush;
	CWorldNode	*pNode;

	for( i=0; i < GetRegion()->m_Selections; i++ )
	{
		if((pNode = GetRegion()->m_Selections[i])->GetType() == Node_Brush)
		{
			pBrush = pNode->AsBrush();

			for( k=0; k < pBrush->m_Points; k++ )
				list.Append( CVertRef(pBrush, k) );
		}
	}
	
	return list > 0;
}


BOOL CRegionView::MakeTaggedEdgeList( CEdgeRefArray &list )
{
	list.SetSize(0);
	
	if( IEdge().IsValid() )
		list.Append( IEdge() );

	return list > 0;
}





typedef CLinkedList<CVertRef> CVertRefList;

void CRegionView::TagPointsInRect( CRect &rect )
{
	uint32 i, k;
	CPoint point;
	CRect norm = rect;
	CEditBrush *pBrush;
	LPOS pos;

	
	norm.NormalizeRect();

	// Only tag in selected brushes if we're in geometry mode and a brush is selected
	if ((GetEditMode() == GEOMETRY_EDITMODE) && (GetRegion()->m_Selections.GetSize() != 0))
	{
		for( i=0; i < GetRegion()->m_Selections; i++ )
		{
			CWorldNode *pNode = GetRegion()->m_Selections[i];

			//don't worry about selecting a brush if it is frozen, hidden, or not a brush 
			//at all
			if( pNode->IsFlagSet(NODEFLAG_FROZEN) ||
				pNode->IsFlagSet(NODEFLAG_HIDDEN) ||
				(pNode->GetType() != Node_Brush))
				continue;

			// Get the brush pointer...
			pBrush = pNode->AsBrush();

			if( !pBrush->IsVisible() )
				continue;

			for( k=0; k < pBrush->m_Points; k++ )
			{
				CEditVert	&vert = pBrush->m_Points[k];
				
				if( vert.IsInFrustum() )
				{
					point.x = (int)vert.m_Projected.x;
					point.y = (int)vert.m_Projected.y;

					if( norm.PtInRect(point) )
						ToggleVertexTag( CVertRef(pBrush, k) );
				}
			}
		}			
	}
	else
	{
		for(pos=m_pRegion->m_Brushes; pos; )
		{
			pBrush = m_pRegion->m_Brushes.GetNext(pos);

			if(	pBrush->IsFlagSet(NODEFLAG_FROZEN) ||
				pBrush->IsFlagSet(NODEFLAG_HIDDEN) ||
				!pBrush->IsVisible())
				continue;

			for( k=0; k < pBrush->m_Points; k++ )
			{
				CEditVert	&vert = pBrush->m_Points[k];
				
				if( vert.IsInFrustum() )
				{
					point.x = (int)vert.m_Projected.x;
					point.y = (int)vert.m_Projected.y;

					if( norm.PtInRect(point) )
						ToggleVertexTag( CVertRef(pBrush, k) );
				}
			}
		}			
	}
}


void CRegionView::RecurseAndGetObjectsInRect(CRect &rect, CMoArray< CWorldNode * > &objects, CWorldNode *pRoot, BOOL bFilter)
{
	if (!pRoot)
		return;

	if((!pRoot->IsFlagSet(NODEFLAG_HIDDEN)) && (!pRoot->IsFlagSet(NODEFLAG_FROZEN)))
	{
		if (pRoot->GetType() == Node_Object)
		{
			if(!bFilter || GetObjectSelFilterDlg()->CanSelectObject(pRoot))
			{
				CPoint testPt;
				if( TransformAndProject(pRoot->GetPos(), testPt) )
				{
					if( rect.PtInRect(testPt) )
						objects.Append(pRoot);
				}
			}
		}
		else if (pRoot->GetType() == Node_PrefabRef)
		{
			if(!bFilter || GetObjectSelFilterDlg()->CanSelectObject(pRoot))
			{
				LTMatrix mPrefabTrans;
				gr_SetupMatrixEuler(pRoot->GetOr(), mPrefabTrans.m);

				CPoint testPt;
				LTVector vPos = pRoot->GetPos() + mPrefabTrans * ((CPrefabRef*)pRoot)->GetPrefabCenter();
				if( TransformAndProject(vPos, testPt) )
				{
					if( rect.PtInRect(testPt) )
						objects.Append(pRoot);
				}
			}
		}
	}

	GPOS iCurChild = pRoot->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		CWorldNode *pChild = pRoot->m_Children.GetNext(iCurChild);
		RecurseAndGetObjectsInRect(rect, objects, pChild, bFilter);
	}
}

void CRegionView::GetObjectsInRect( CRect &rect, CMoArray< CWorldNode * > &objects, BOOL bFilter )
{
	// Normalize the rectangle
	CRect norm = rect;
	norm.NormalizeRect();

	// Send it into the recursive function to get the list
	RecurseAndGetObjectsInRect(norm, objects, m_pRegion->GetRootNode(), bFilter);
}


void CRegionView::ToggleVertexTag( CVertRef ref )
{
	GenListPos pos;

	if(TaggedVerts().GenFindElement(ref, pos))
		TaggedVerts().GenRemoveAt(pos);
	else
		TaggedVerts().GenAppend(ref);
}


BOOL CRegionView::GetClosestPoint( CPoint &point, CVector &vClosest, BOOL bSelectedOnly, CReal *pDist )
{
	CWorldNode*		obj;
	CReal			dist=(CReal)MAX_CREAL, minDist = (CReal)MAX_CREAL;
	CVector			vClosestPoint;
	CVertRef		closest;
	BOOL			bResult = FALSE;

	closest = GetClosestVert( point, bSelectedOnly, &dist );
	if( closest.IsValid( ))
	{
		minDist = dist;
		vClosest = closest( );
		bResult = TRUE;
	}

	obj = GetClosestObject( point, bSelectedOnly, &dist );
	if( obj && dist < minDist )
	{
		minDist = dist;
		vClosest = obj->GetPos();
		bResult = TRUE;
	}

	if( pDist )
		*pDist = minDist;

	return bResult;
}


CVertRef CRegionView::GetClosestVert( CPoint &point, BOOL bSelectedOnly, CReal *pDist, bool bIgnoreFrozen )
{
	CWorldNode		*pNode;
	CEditBrush		*pBrush;
	CReal			dist=(CReal)MAX_CREAL, minDist = (CReal)MAX_CREAL;
	uint32			i;
	CVertRef		vert, minVert;
	LPOS pos;
	CPoint		testPt;


	// Do all brushes...
	if( !bSelectedOnly )
	{
		for(pos=m_pRegion->m_Brushes; pos; )
		{
			pBrush = m_pRegion->m_Brushes.GetNext(pos);

			if(	pBrush->IsFlagSet(NODEFLAG_HIDDEN) ||
				(pBrush->IsFlagSet(NODEFLAG_FROZEN) && bIgnoreFrozen))
			{
				continue;
			}
			
			vert = GetClosestVert(pBrush, point, &dist);
			if( dist < minDist )
			{
				minVert = vert;
				minDist = dist;
			}
		}
	}
	// Do only selected brushes...
	else
	{
		for( i=0; i < GetRegion()->m_Selections; i++ )
		{
			pNode = GetRegion()->m_Selections[i];

			if(	pNode->IsFlagSet(NODEFLAG_HIDDEN) ||
				(pNode->IsFlagSet(NODEFLAG_FROZEN) && bIgnoreFrozen))
			{
				continue;
			}

			// Handle movement for Brush nodes...
			if(pNode->GetType() == Node_Brush)
			{
				// Get the brush pointer...
				pBrush = pNode->AsBrush();

				vert = GetClosestVert(pBrush, point, &dist);
				if( dist < minDist )
				{
					minVert = vert;
					minDist = dist;
				}
			}
		}
	}

	if( pDist )
		*pDist = minDist;

	return minVert;
}


CVertRef CRegionView::GetClosestVert( CEditBrush *pBrush, CPoint point, CReal *pDist )
{
	uint32		i, k;
	uint32		minIndex = BAD_INDEX;
	CEditPoly	*pPoly;
	CPoint		testPt;

	CReal		viewerDist, minViewerDist = (CReal)MAX_CREAL;
	int32		dist, minDist = MAX_SDWORD;

	for( i=0; i < pBrush->m_Polies; i++ )
	{
		pPoly = pBrush->m_Polies[i];

		for( k=0; k < pPoly->NumVerts(); k++ )
		{
			if( TransformAndProjectInFrustum(pPoly->Pt(k), testPt) )
			{
				dist = SQR(testPt.x - point.x) + SQR(testPt.y - point.y);
				if( dist == minDist )
				{
					viewerDist = (pPoly->Pt(k) - Nav().Pos()).Mag();
					if( viewerDist < minViewerDist )
					{
						minViewerDist = viewerDist;
						minIndex = pPoly->m_Indices[k];
						minDist = dist;
					}
				}
				else if( dist < minDist )
				{
					minViewerDist = (pPoly->Pt(k) - Nav().Pos()).Mag();
					minIndex = pPoly->m_Indices[k];
					minDist = dist;
				}
			}
		}
	}

	if( pDist )
		*pDist = (CReal)minDist;

	if( minIndex == BAD_INDEX )
		return CVertRef( NULL, minIndex );
	else
		return CVertRef( pBrush, minIndex );
}


CVertRef CRegionView::GetClosestVert( CVertRefArray &vertArray, CPoint point, CReal *pDist, bool bIgnoreFrozen )
{
	uint32		i;
	uint32		minIndex = BAD_INDEX;
	CPoint		testPt;

	CReal		viewerDist, minViewerDist = (CReal)MAX_CREAL;
	int32		dist, minDist=MAX_SDWORD;

	for( i=0; i < vertArray.GetSize( ); i++ )
	{
		if(	vertArray[i].m_pBrush->IsFlagSet(NODEFLAG_HIDDEN) ||
			(vertArray[i].m_pBrush->IsFlagSet(NODEFLAG_FROZEN) && bIgnoreFrozen))
		{
			continue;
		}

		if( TransformAndProjectInFrustum(vertArray[i](), testPt) )
		{
			dist = SQR(testPt.x - point.x) + SQR(testPt.y - point.y);
			if( dist == minDist )
			{
				viewerDist = (vertArray[i]() - Nav().Pos()).Mag();
				if( viewerDist < minViewerDist )
				{
					minViewerDist = viewerDist;
					minIndex = i;
					minDist = dist;
				}
			}
			else if( dist < minDist )
			{
				minViewerDist = (vertArray[i]() - Nav().Pos()).Mag();
				minIndex = i;
				minDist = dist;
			}
		}
	}

	if( pDist )
		*pDist = (CReal)minDist;

	if( minIndex == BAD_INDEX )
		return CVertRef( NULL, minIndex );
	else
		return vertArray[minIndex];
}


CEdgeRef CRegionView::GetClosestEdge( CPoint &point, BOOL bSelectedOnly )
{
	uint32			i;
	CEditBrush		*pBrush;
	CEdgeRef		testRef, bestRef;
	CReal			dist, minDist = (CReal)MAX_CREAL;
	LPOS pos;

	// Find the closest in all brushes
	if (!bSelectedOnly)
	{
		for(pos=m_pRegion->m_Brushes; pos; )
		{
			pBrush = m_pRegion->m_Brushes.GetNext(pos);
			
			if(pBrush->IsFlagSet(NODEFLAG_HIDDEN) || pBrush->IsFlagSet(NODEFLAG_FROZEN))
			{
				continue;
			}

			testRef = GetClosestEdge( pBrush, point, &dist );
			if( testRef.IsValid() && (dist < minDist) )
			{
				bestRef = testRef;
				minDist = dist;
			}
		}
	}
	// Find the closest in only the selected brushes
	else
	{
		for( i=0; i < GetRegion()->m_Selections; i++ )
		{
			CWorldNode *pNode = GetRegion()->m_Selections[i];

			if(pNode->IsFlagSet(NODEFLAG_HIDDEN) || pNode->IsFlagSet(NODEFLAG_FROZEN))
			{
				continue;
			}

			// Handle movement for Brush nodes...
			if(pNode->GetType() == Node_Brush)
			{
				// Get the brush pointer...
				pBrush = pNode->AsBrush();
				testRef = GetClosestEdge( pBrush, point, &dist );
				if( testRef.IsValid() && (dist < minDist) )
				{
					bestRef = testRef;
					minDist = dist;
				}
			}
		}			
	}

	return bestRef;
}


CEdgeRef CRegionView::GetClosestEdge( CEditBrush *pBrush, CPoint point, CReal *pDist )
{
	uint32			i, k;
	CEditPoly		*pPoly;
	TLVertex line[2];
	CVector			points[2];
	CVector			lineVec, normal, pointVec;

	CReal			dot, maxLineDot, bestLineDot = (CReal)MAX_CREAL;
	CEdgeRef		bestEdge;

	pointVec.Init( (CReal)point.x, (CReal)point.y, 0.0f );

	for( i=0; i < pBrush->m_Polies; i++ )
	{
		pPoly = pBrush->m_Polies[i];

		for( k=0; k < pPoly->NumVerts(); k++ )
		{
			maxLineDot = 0.0f;
			
			TransformPt( pPoly->Pt(k), line[0].m_Vec );
			TransformPt( pPoly->NextPt(k), line[1].m_Vec );

			if( ClipLineToFrustum(line) )
			{
				ViewDef()->ProjectPt( line[0].m_Vec, points[0] );
				ViewDef()->ProjectPt( line[1].m_Vec, points[1] );

				lineVec = points[1] - points[0];
				lineVec.z = 0.0f;
				lineVec.Norm();
				normal.Init( -lineVec.y, lineVec.x, 0.0f );

				// Test the distance from the line 'caps'.
				dot = lineVec.Dot( pointVec - points[1] );
				if( dot > 0 )
				{
					maxLineDot = dot;
				}
				else
				{
					dot = (-lineVec).Dot( pointVec - points[0] );
					if( dot > 0 )
						maxLineDot = dot;
				}
				
				// Test the distance from the line itself.
				dot = normal.Dot( pointVec - points[0] );
				maxLineDot = MAX( maxLineDot, ABS(dot) );
			
				if( maxLineDot < bestLineDot )
				{
					bestLineDot = maxLineDot;
					bestEdge.Init( pBrush, pPoly->m_Indices[k], pPoly->m_Indices.Next(k) );
				}
			}
		}
	}	

	if( pDist )
		*pDist = bestLineDot;

	return bestEdge;
}


CPolyRef CRegionView::CastRayAtPolies( CEditRay &ray, CReal *pDist, CReal rMinDist, BOOL bSelectedOnly )
{
	uint32			i;
	CReal			dist, minDist = (CReal)MAX_CREAL;
	CPolyRef		ref, bestRef;
	CEditBrush		*pBrush;
	LPOS pos;

	// All brushes..
	if (!bSelectedOnly)
	{
		for(pos=m_pRegion->m_Brushes; pos; )
		{
			pBrush = m_pRegion->m_Brushes.GetNext(pos);

			if(pBrush->IsFlagSet(NODEFLAG_HIDDEN) || pBrush->IsFlagSet(NODEFLAG_FROZEN))
			{
				continue;
			}

			ref = CastRayAtPolies( pBrush, ray, &dist );
			if( ref.IsValid() && (dist < minDist) && ( dist > rMinDist ))
			{
				bestRef = ref;
				minDist = dist;
			}
		}
	}
	// Selected only
	else
	{
		for( i=0; i < GetRegion()->m_Selections; i++ )
		{
			CWorldNode *pNode = GetRegion()->m_Selections[i];

			if(pNode->IsFlagSet(NODEFLAG_HIDDEN) || pNode->IsFlagSet(NODEFLAG_FROZEN))
			{
				continue;
			}

			if(pNode->GetType() == Node_Brush)
			{
				// Get the brush pointer...
				pBrush = pNode->AsBrush();

				ref = CastRayAtPolies( pBrush, ray, &dist );
				if( ref.IsValid() && (dist < minDist) && ( dist > rMinDist ))
				{
					bestRef = ref;
					minDist = dist;
				}
			}
		}
	}

	if(pDist)
	{
		*pDist = minDist;
	}

	return bestRef;
}
	
	
CPolyRef CRegionView::CastRayAtPolies( CEditBrush *pBrush, CEditRay &ray, CReal *pDist )
{	
	CReal			t;
	CReal			minDist = (CReal)MAX_CREAL;
	uint32			i;

	CEditBrush		*pMinBrush = NULL;
	uint32			minIndex = BAD_INDEX;
	

	for( i=0; i < pBrush->m_Polies; i++ )
	{
		if( pBrush->m_Polies[i]->IntersectRay(ray, t, IsSelectBackfaces()) )
		{
			if( t < minDist )
			{
				pMinBrush = pBrush;
				minDist = t;
				minIndex = i;
			}
		}
	}

	if( pDist )
		*pDist = minDist;

	return CPolyRef( pMinBrush, minIndex );
}


CWorldNode *CRegionView::RecurseAndGetClosestObject(const CPoint &point, BOOL bSelectedOnly, CWorldNode *pRoot, SDWORD &nDistMin, float &fViewMin, BOOL bFilter)
{
	CWorldNode *pResult = LTNULL;

	BOOL bMatchType		= ((pRoot->GetType() == Node_Object) || (pRoot->GetType() == Node_PrefabRef));
	BOOL bMatchState	= (!pRoot->IsFlagSet(NODEFLAG_HIDDEN)) && (!pRoot->IsFlagSet(NODEFLAG_FROZEN));
	BOOL bMatchSelCrit	= (!bSelectedOnly || pRoot->IsFlagSet(NODEFLAG_SELECTED));

	// Is this one the closest?
	if (bMatchType && bMatchSelCrit && bMatchState)
	{
		//only do the filtering if the above criteria match since it is more expensive
		if(!bFilter || GetObjectSelFilterDlg()->CanSelectObject(pRoot))
		{
			LTVector pt = pRoot->GetPos();

			//we need to override the point for prefabs since they are offset
			if(pRoot->GetType() == Node_PrefabRef)
			{
				LTMatrix mPrefabTrans;
				gr_SetupMatrixEuler(pRoot->GetOr(), mPrefabTrans.m);
				pt = pRoot->GetPos() + mPrefabTrans * ((CPrefabRef*)pRoot)->GetPrefabCenter();
			}

			CPoint testPt;

			if( TransformAndProjectInFrustum(pt, testPt) )
			{
				int32 dist = SQR(testPt.x - point.x) + SQR(testPt.y - point.y);
				if( dist == nDistMin )
				{
					float viewerDist = (pt - Nav().Pos()).Mag();
					if( viewerDist < fViewMin )
					{
						pResult = pRoot;
						fViewMin = viewerDist;
						nDistMin = dist;
					}
				}
				else if( dist < nDistMin )
				{
					pResult = pRoot;
					fViewMin = (pt - Nav().Pos()).Mag();
					nDistMin = dist;
				}
			}
		}
	}

	// Try the children
	GPOS iCurChild = pRoot->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		CWorldNode *pChild = pRoot->m_Children.GetNext(iCurChild);
		CWorldNode *pChildResult = RecurseAndGetClosestObject(point, bSelectedOnly, pChild, nDistMin, fViewMin, bFilter);
		if (pChildResult)
			pResult = pChildResult;
	}

	return pResult;
}

CWorldNode *CRegionView::GetClosestObject( const CPoint &point, BOOL bSelectedOnly, CReal *pDist, BOOL bFilter )
{
	float fMinView = (float)MAX_CREAL;
	int32 nMinDist = MAX_SDWORD;
	CWorldNode *pResult = RecurseAndGetClosestObject(point, bSelectedOnly, m_pRegion->GetRootNode(), nMinDist, fMinView, bFilter);
	if (pResult && pDist)
	{
		*pDist = (CReal)nMinDist;
	}
	return pResult;
}




					
/////////////////////////////////////////////////////////////////////////////
// Custom CRegionView functions


BOOL CRegionView::GetCurrentTextureName( char* &pTextureName, BOOL bUseSprites )
{
	char szTextureName[MAX_PATH];

	GetProjectBar()->GetCurrentTextureName( szTextureName, bUseSprites );
	if( strlen( szTextureName ))
	{
		pTextureName = GetRegionDoc()->m_RegionStringHolder.AddString( szTextureName );
		return TRUE;
	}
	else
		return FALSE;
}

void CRegionView::GiveMemoryWarning()
{
	CString str;

	str.LoadString( IDS_LOWONMEMORY );
	MessageBox( str, AfxGetAppName(), MB_OK );
}


void CRegionView::UpdatePlanesOfPolies( CVertRefArray &verts )
{
	uint32			i, k;
	CEditBrush		*pBrush;
	CEditPoly		*pPoly;

	
	for( i=0; i < verts; i++ )
	{
		ASSERT( verts[i].IsValid() );
		pBrush = verts[i].m_pBrush;

		for( k=0; k < pBrush->m_Polies; k++ )
		{
			pPoly = pBrush->m_Polies[k];
			if( pPoly->m_Indices.FindElement(verts[i].m_iVert) )
				pPoly->UpdatePlane();
		}		

	}

	GetRegion()->UpdateBrushGeometry(pBrush);
}


BOOL CRegionView::GetVertexFromPoint( CPoint point, CVector &vertex, BOOL bSnapToGrid )
{
	return EditGrid().IntersectRay( GetGridSpacing(), ViewDef()->MakeRayFromScreenPoint(point), vertex, NULL, bSnapToGrid );
}


BOOL CRegionView::UpdateImmediateSelection()
{
	CPoint			point = GetCurMousePos();
	CVertRef		vert;
	CEdgeRef		edge;
	CPolyRef		poly;
	CBrushRef		brush;
	CObjectRef		obj;
	CEditRay		ray;
	BOOL			bRet = FALSE;
	BOOL			bSelectionsOnly = GetRegion()->m_Selections.GetSize() > 0; // do we want to restrict the cast to selection?


	// We don't want to restrict the cast to the selection if it includes 
	// only objects (no geometry), so check the selection list

	if (bSelectionsOnly) 
	{
		bSelectionsOnly = false; // we will assume that the selected list is all objects until proven otherwise

		for (int i=0; (!bSelectionsOnly) && (i < GetRegion()->m_Selections.GetSize()) ; i++) {

			// if this is true, the selection list includes at least one brush
			if (GetRegion()->m_Selections[i]->GetType() != Node_Object)  bSelectionsOnly = TRUE;
		}
	}


	if( GetEditMode() == GEOMETRY_EDITMODE )
	{
		ray = ViewDef()->MakeRayFromScreenPoint( point );

		vert = GetClosestVert( point, bSelectionsOnly );
		edge = GetClosestEdge( point, bSelectionsOnly );
		poly = CastRayAtPolies( ray, NULL, 0.0f, bSelectionsOnly );
			
		brush.Init(poly.m_pBrush);
		
		if( (vert != IVert()) || (edge != IEdge()) || (poly != IPoly()) || (brush != IBrush()) )
			bRet = TRUE;
	}

	IVert() = vert;
	IEdge() = edge;
	IPoly() = poly;
	IBrush() = brush;
	
	return bRet;
}


void CRegionView::UpdateCursor()
{
	HCURSOR		theCursor;

	if( IsParallelViewType() && (GetEditMode() == BRUSH_EDITMODE || GetEditMode() == OBJECT_EDITMODE) && GetRegion()->GetNumSelections() > 0)
	{
		int32	handle = GetCurrentMouseOverHandle();
		LTVector		scaleNormal, scaleOrigin;

		if( handle != m_CurMouseOverHandle )
		{
			GetHandleInfo( handle, &theCursor, scaleNormal, scaleOrigin );
			m_CurMouseOverHandle = handle;

			::SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)theCursor );
			//	SetCursor( theCursor );
		}
	}
	else
	{
		// Check to see if the cursor wasn't set back
		if (m_CurMouseOverHandle != -1)
		{
			::SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)m_CursorArrow);
		}
		m_CurMouseOverHandle = -1;
	}
}


BOOL CRegionView::GetHandleInfo( int theHandle, HCURSOR *pCursor, CVector &scaleNormal, CVector &scaleOrigin )
{
	CReal		dot;


	if( theHandle == -1 )
	{
		if( pCursor )
			*pCursor = m_CursorArrow;
	}
	else
	{
		ASSERT( theHandle < 18 );

		// See if you shouldn't be resizing it, but moving the selections.
		dot = g_SelectionHandleVectors[theHandle].Dot( Nav().Forward() );

		if( fabs(dot) > 0.9f )
		{
			m_bSizeHandleSelected = FALSE;
			
			if( pCursor )
				*pCursor = m_CursorSizeAll;
		}
		else
		{
			m_bSizeHandleSelected = TRUE;
			
			if (pCursor)
			{
				if (theHandle < 6)
				{
					// Use the movement handle if it's competing for focus
					dot = g_SelectionHandleVectors[theHandle].Dot( Nav().Up() );
					
					if( pCursor )
						*pCursor = (fabs(dot) > 0.9f) ? m_CursorNS : m_CursorWE;
				}
				else
				{
					switch (theHandle)
					{				
						// XY handles
						case 6:		*pCursor=m_CursorNESW; break;
						case 7:		*pCursor=m_CursorNWSE; break;
						case 8:		*pCursor=m_CursorNWSE; break;
						case 9:		*pCursor=m_CursorNESW; break;

						// XZ handles
						case 10:	*pCursor=m_CursorNESW; break;
						case 11:	*pCursor=m_CursorNWSE; break;
						case 12:	*pCursor=m_CursorNWSE; break;
						case 13:	*pCursor=m_CursorNESW; break;

						// YZ handles
						case 14:	*pCursor=m_CursorNWSE; break;
						case 15:	*pCursor=m_CursorNESW; break;
						case 16:	*pCursor=m_CursorNESW; break;
						case 17:	*pCursor=m_CursorNWSE; break;
					}
				}
			}

			scaleNormal = g_SelectionHandleVectors[theHandle];			
			scaleOrigin.x = m_BoxMiddle.x - (scaleNormal.x * m_BoxHalf.x);
			scaleOrigin.y = m_BoxMiddle.y - (scaleNormal.y * m_BoxHalf.y);
			scaleOrigin.z = m_BoxMiddle.z - (scaleNormal.z * m_BoxHalf.z);
		}
	}

	return m_bSizeHandleSelected;
}


int32 CRegionView::GetCurrentMouseOverHandle()
{
	CVector test;
	CPoint point = GetCurMousePos();
	CVector vecPoint( (CReal)point.x, (CReal)point.y, 0.0f );
	int i, minIndex;
	CReal testDist, minDist = (CReal)MAX_CREAL;

	// Search the first 6 handles (this makes it easy to grab the move handle)
	minIndex = -1;	
	for( i=0; i < 6; i++ )
	{
		if( m_HandlesInFrustum[i] == 1 )
		{
			test.Init( (CReal)m_HandlePos[i].x, (CReal)m_HandlePos[i].y, 0.0f );
			
			if( (vecPoint - test).Mag() < 8.0f )
			{
				if (fabs(g_SelectionHandleVectors[i].Dot( Nav().Forward())) > 0.9f)
				{
					return i;
				}

				testDist = (m_BoxHandles[i] - Nav().Pos()).Mag();
				if( testDist < minDist )
				{
					minDist = testDist;
					minIndex = i;									
				}
			}
		}
	}
	if (minIndex != -1)
	{
		return minIndex;
	}

	// Search the rest of the handles
	for( i=6; i < NUM_BOX_HANDLES; i++ )
	{
		if( m_HandlesInFrustum[i] )
		{
			test.Init( (CReal)m_HandlePos[i].x, (CReal)m_HandlePos[i].y, 0.0f );
			
			if( (vecPoint - test).Mag() < 8.0f )
			{
				testDist = (m_BoxHandles[i] - Nav().Pos()).Mag();
				if( testDist < minDist )
				{
					minDist = testDist;
					minIndex = i;

					// SHP 4/13/1999
					return i;
				}
			}
		}
	}
	return minIndex;
}

BOOL CRegionView::IsKeyDown( int key )
{
	return ( GetAsyncKeyState(key) & 0x8000 );
}


void CRegionView::SplitEdges()
{
	CEdgeRefArray		list;
	CEditVert			newPt;
	CEdgeRef			*pEdge;
	CEditPoly			*pPoly;
	CEditBrush			*pBrush;
	uint32				i, k, e;
	uint32				index;
	PreActionList actionList;

	
	if( MakeTaggedEdgeList(list) )
	{
		// Setup an undo.
		for(e=0; e < list; e++)
		{
			AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, list[e].m_pBrush), TRUE);
		}

		GetRegionDoc()->Modify(&actionList, TRUE);


		for( e=0; e < list; e++ )
		{
			pEdge = &list[e];
			pBrush = pEdge->m_pBrush;

			CEditVert	&vert1 = pEdge->Vert1();
			CEditVert	&vert2 = pEdge->Vert2();

			newPt = vert1 + ((vert2 - vert1) * 0.5f);
			pBrush->m_Points.Append( newPt );
			index = pBrush->m_Points.LastI();
		
			// Find every polygon with this edge.
			for( i=0; i < pBrush->m_Polies; i++ )
			{
				pPoly = pBrush->m_Polies[i];
				for( k=0; k < pPoly->NumVerts(); k++ )
				{
					if( ((pPoly->m_Indices[k] == pEdge->m_Vert1) && (pPoly->m_Indices.Next(k) == pEdge->m_Vert2)) ||
						 ((pPoly->m_Indices[k] == pEdge->m_Vert2) && (pPoly->m_Indices.Next(k) == pEdge->m_Vert1)) )
					{
						pPoly->m_Indices.Insert( k+1, index );
						break;
					}
				}
			}
		}
	}
}


//------------------------------------------------------------------------------------
//
// CRegionView::DeleteSelectedNodes()
//
// Purpose:  Delete all selected nodes, which includes brushes and objects.
//
//------------------------------------------------------------------------------------
void CRegionView::DeleteSelectedNodes()
{
	CWorldNode		*pNode;
	uint32			i, index;
	LPOS lPos;
	
	// Tag document as changed...
	GetRegionDoc()->SetupUndoForSelections();

	// Loop over all selections and delete the brushes or objects...
	for( i=0; i < GetRegion()->m_Selections; i++ )
	{
		// Get the node...
		pNode = GetRegion()->m_Selections[i];
		ASSERT( pNode );

		// Remove this node from node view tree...
		GetNodeView( )->RemoveFromTree( pNode );
		GetNodeView( )->DeleteNode( pNode );

		if( pNode->GetParent( ))
			pNode->RemoveFromTree( );

		// Delete the brush...
		if(pNode->GetType() == Node_Brush)
		{
			GetRegion()->RemoveBrush(pNode->AsBrush());
		}
		// Delete the object...
		else if( pNode->m_Type == Node_Object )
		{
			GetPropertiesDlg()->TermIf( &pNode->m_PropList );
			GetRegion()->RemoveObject(pNode->AsObject());
		}

		// Remove the object from the list of path nodes
		GetRegion()->RemoveNodeFromPath(pNode);
	}

	for( i=0; i < GetRegion()->m_Selections; i++ )
	{
		// Get the node...
		pNode = GetRegion()->m_Selections[i];
		pNode->ClearFlag(NODEFLAG_SELECTED);

		delete pNode;
	}
		
	// Clear the selections...
	GetRegion()->m_Selections.Term();
	GetRegionDoc()->NotifySelectionChange();

	// Force all views to redraw...
	GetRegionDoc()->UpdateAllViews( this );
}


void CRegionView::DeleteTaggedVertices()
{
	CVertRefArray		verts;
	uint32				i;
	int32				k, j;
	CEditPoly			*pPoly;
	CEditBrush			*pBrush;
	PreActionList actionList;


	if( TaggedVerts().GenGetSize() > 0 )
	{
		verts.GenCopyList(TaggedVerts());

		for(i=0; i < verts; i++)
		{
			AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, verts[i].m_pBrush), TRUE);
		}
	}
	else if( IVert().IsValid() )
	{
		verts.Append( IVert() );

		actionList.Append(new CPreAction(ACTION_MODIFYNODE, IVert().m_pBrush));
	}
	else
		return;

	GetRegionDoc()->Modify(&actionList, TRUE);


	for( i=0; i < verts; i++ )
	{
		pBrush = verts[i].m_pBrush;

		for( k=0; k < (int32)pBrush->m_Polies; k++ )
		{
			pPoly = pBrush->m_Polies[k];

			for( j=0; j < (int32)pPoly->NumVerts(); j++ )
			{
				if( pPoly->Index(j) == verts[i].m_iVert )
				{
					if( pPoly->NumVerts() <= 3 )
					{
						delete pPoly;
						pBrush->m_Polies.Remove( k );
						--k;
						break;
					}
					else
					{
						pPoly->m_Indices.Remove(j);
						--j;
					}
				}
			}
		}

		pBrush->RemoveVertex( verts[i].m_iVert );
		for( k=(int32)i+1; k < (int32)verts; k++ )
			if( (verts[k].m_pBrush == verts[i].m_pBrush) && (verts[k].m_iVert >= verts[i].m_iVert) )
				verts[k].m_iVert--;
	}

	UpdatePlanesOfPolies( verts );

	for(LPOS iterateMPos=m_pRegion->m_Brushes; iterateMPos; )
	{
		CEditBrush *pIterateBrush = m_pRegion->m_Brushes.GetNext(iterateMPos);
		pIterateBrush->RemoveUnusedVerts();
	}

	m_pRegion->RemoveUnusedBrushes();

	UpdateImmediateSelection();
	ClearSelections();
	GetDocument()->UpdateAllViews( this );
}


void CRegionView::DeleteTaggedEdges( BOOL bRemoveOriginalPolyPoints )
{
	CEditPoly				*pPoly1, *pPoly2, *pNew;
	uint32					nContaining, i;

	CMoArray<CEditPoly*>	excludeList;
	CMoDWordArray			pointList;
	CMoArray<CEditPoly*>	changedList;


	if( IEdge().IsValid() )
	{
		GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, IEdge().m_pBrush), TRUE);

		if( IEdge().m_pBrush->FindPoliesWithEdge(IEdge().m_Vert1, IEdge().m_Vert2, pPoly1, pPoly2, nContaining) )
		{
			pNew = IEdge().m_pBrush->JoinPolygons( pPoly1, pPoly2, IEdge().m_Vert1, IEdge().m_Vert2 );
			pNew->UpdatePlane();

			// Remove the points from any connected polies.
			pointList.Append( IEdge().m_Vert1 );
			pointList.Append( IEdge().m_Vert2 );
			
			if( bRemoveOriginalPolyPoints )
				IEdge().m_pBrush->RemovePolyVerts( pointList, excludeList, changedList );

			IEdge().m_pBrush->RemoveUnusedVerts();
			GetRegion()->UpdateBrushGeometry(IEdge().m_pBrush);

			UpdateImmediateSelection();
			ClearSelections();
			GetDocument()->UpdateAllViews( this );
		}
	}
}


void CRegionView::DeleteTaggedPolygons()
{
	CPolyRefArray &aTaggedPolys = GetRegionDoc()->m_TaggedPolies;
	// Don't delete the "immediate" poly unless it's the only one in the array
	uint32 nStartPoly = (aTaggedPolys.GetSize() > 1) ? 1 : 0;
	uint32 nCurPoly, nPolyCount = 0;

	// Get the set of brushes we're going to be operating on
	typedef std::set<CEditBrush*> TBrushSet;
	TBrushSet aBrushSet;
	for (nCurPoly = nStartPoly; nCurPoly < aTaggedPolys.GetSize(); ++nCurPoly)
	{
		CPolyRef &cCurPoly = aTaggedPolys[nCurPoly];
		if( cCurPoly.IsValid() )
		{
			aBrushSet.insert(cCurPoly.m_pBrush);
			++nPolyCount;
		}
	}

	// You don't seem to have anything valid selected....
	if (aBrushSet.empty())
		return;

	// Set up an undo
	PreActionList actionList;
	TBrushSet::iterator iCurBrush = aBrushSet.begin();
	for (; iCurBrush != aBrushSet.end(); ++iCurBrush)
	{
		actionList.Append(new CPreAction(ACTION_MODIFYNODE, *iCurBrush));
	}
	GetRegionDoc()->Modify(&actionList, TRUE);

	// Remove the polys from the brushes
	// Note : This can't just delete them because the renderer might want to
	// draw during this function.  (For some unknown reason....)
	typedef std::vector<CEditPoly *> TPolyList;
	TPolyList aGarbage;
	aGarbage.reserve(nPolyCount);
	for (nCurPoly = nStartPoly; nCurPoly < aTaggedPolys.GetSize(); ++nCurPoly)
	{
		CPolyRef &cCurPoly = aTaggedPolys[nCurPoly];
		if( cCurPoly.IsValid() )
		{
			cCurPoly.m_pBrush->m_Polies[cCurPoly.m_iPoly] = 0;
			aGarbage.push_back(cCurPoly());
			cCurPoly.m_pBrush = 0;
		}
	}

	// Clean up the brushes
	for (iCurBrush = aBrushSet.begin(); iCurBrush != aBrushSet.end(); ++iCurBrush)
	{
		CEditBrush *pCurBrush = *iCurBrush;
		for (uint32 nCleanupPolyArray = 0; nCleanupPolyArray < pCurBrush->m_Polies.GetSize(); )
		{
			if (pCurBrush->m_Polies[nCleanupPolyArray])
				++nCleanupPolyArray;
			else
				pCurBrush->m_Polies.Remove(nCleanupPolyArray);
		}
		pCurBrush->RemoveUnusedVerts();
	}

	// Clean up the region & views
	m_pRegion->RemoveUnusedBrushes();
	UpdateImmediateSelection();
	ClearSelections();
	GetRegionDoc()->UpdateAllViews(this);
	DrawRect();

	// Actually delete the polys
	while (!aGarbage.empty())
	{
		delete aGarbage.back();
		aGarbage.pop_back();
	}
}

void CRegionView::DoFlipOperation()
{
	PreActionList actionList;
	CEditRegion *pRegion;

	pRegion = GetRegion();


	if( (pRegion->m_Selections == 0) || (pRegion->m_Selections[0]->GetType() != Node_Brush) )
	{
		AppMessageBox( "Can't flip without a selected brush", MB_OK );
		return;
	}

	uint uBrushLoop;

	// Add the brushes to the undo list
	for (uBrushLoop = 0; uBrushLoop < pRegion->m_Selections; uBrushLoop++)
	{
		// Add it to the undo list
		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Selections[uBrushLoop]));
	}

	GetRegionDoc()->Modify(&actionList, TRUE);

	// Go through the selected brushes
	for (uBrushLoop = 0; uBrushLoop < pRegion->m_Selections; ++uBrushLoop)
	{
		if (pRegion->m_Selections[uBrushLoop]->GetType() == Node_Brush) // verify selection is a brush
		{
			CEditBrush *pBrush = pRegion->m_Selections[uBrushLoop]->AsBrush();
	
			// Flip the polygons 
			for (uint32 uPolyLoop = 0; uPolyLoop < pBrush->m_Polies; ++uPolyLoop)
			{
				pBrush->m_Polies[uPolyLoop]->Flip();
			}

			// Remember to update the brush
			pBrush->UpdatePlanes();
		}
	}

	// Mmmm..  View update...
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// Offsets (moves them in the world) the selected nodes by a vector
void CRegionView::OffsetSelectedNodes(CVector vOffset)
{
	// Setup the undo
	GetRegionDoc()->SetupUndoForSelections();

	// Get the region
	CEditRegion *pRegion=GetRegion();
	pRegion->OffsetSelectedNodes(vOffset);

	// Resize the box encompassing all the selections...
	GetRegionDoc()->UpdateSelectionBox();

	// Redraw the views
	GetRegionDoc()->UpdateAllViews( this );
}

/************************************************************************/
// Offsets the texture map for the selected polygon in the specified
// direction (use 1 for positive, -1 for negative, or zero for no
// movement).  The grid size is used to offset the texture.  If SHIFT
// is pressed, then the texture is moved one unit at a time.
void CRegionView::OffsetTextureForPoly(int nPDirection, int nQDirection, BOOL bGrid)
{
	if (!IPoly().IsValid())
		return;

	// Get the polygon
	CEditPoly *pPoly = IPoly()();		
	if(!pPoly)
	{
		return;
	}

	//make the undo for this node
	GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, pPoly->m_pBrush), TRUE);

	// Figure out the amount to offset the texture by
	float fOffsetSize=0.0f;
	if (bGrid)
	{
		fOffsetSize=1.0f;
	}
	else
	{
		fOffsetSize=(float)m_dwGridSpacing;
	}
	
	// Determine the new O vector
	CTexturedPlane& Texture = pPoly->GetTexture(GetCurrTexture());

	CVector O=Texture.GetO()+(Texture.GetP()*fOffsetSize*(float)nPDirection)+(Texture.GetQ()*fOffsetSize*(float)nQDirection);	

	// Set the texture space on the polygon
	pPoly->SetTextureSpace(GetCurrTexture(), O, Texture.GetP(), Texture.GetQ());	

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();				
}

/************************************************************************/
// Scale the texture map for the selected polygon in the specified
// direction (use 1 to expand, -1 to shrink, or zero for neither)
// The grid size is used to scale the texture.
void CRegionView::ScaleTextureForPoly(int nPDirection, int nQDirection)
{
	if (!IPoly().IsValid())
		return;

	// Get the polygon
	CEditPoly *pPoly = IPoly()();		
	if(!pPoly)
	{
		return;
	}

	// Check to see if the polygon has a texture
	CTexturedPlane& Texture = pPoly->GetTexture(GetCurrTexture());
	if(!Texture.m_pTextureFile)
	{
		return;
	}

	// Get the texture
	CTexture *pTexture=dib_GetDibTexture(Texture.m_pTextureFile);
	if(!pTexture || !pTexture->m_pDib)
	{
		return;
	}

	//make the undo for this node
	GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, pPoly->m_pBrush), TRUE);

	// Get the width and the height of the texture		
	float fWidth = (float)pTexture->m_pDib->GetWidth();
	float fHeight = (float)pTexture->m_pDib->GetHeight();

	// Figure out the scaling values
	float fWidthScale=1.0f;
	float fHeightScale=1.0f;
	
	if (nPDirection > 0)
	{
		// Expand		
		fWidthScale=fWidth/(fWidth-(float)m_dwGridSpacing);
	}
	else if (nPDirection < 0)
	{
		// Shrink
		fWidthScale=(fWidth-(float)m_dwGridSpacing)/fWidth;
	}

	if (nQDirection > 0)
	{
		// Expand		
		fHeightScale=fHeight/(fHeight-(float)m_dwGridSpacing);
	}
	else if (nQDirection < 0)
	{
		// Shrink
		fHeightScale=(fHeight-(float)m_dwGridSpacing)/fHeight;
	}	

	// Set the texture space on the polygon
	pPoly->SetTextureSpace(GetCurrTexture(), Texture.GetO(), Texture.GetP()*fWidthScale, Texture.GetQ()*fHeightScale);	

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();				
}

/************************************************************************/
// Mirrors the selected brushes along one of the axis specified in vAxis.
// vAxis must either be (1, 0, 0) (0,1,0) or (0,0,1)
void CRegionView::MirrorSelectedNodes(CVector vAxis)
{
	DMatrix mOReflection, mPQReflection;
	DVector O, P, Q;
	uint32 iPoly;
	CEditPoly *pPoly;

	// Make sure that the axis is correct
	if (vAxis != CVector(1,0,0) && vAxis != CVector(0,1,0) && vAxis != CVector(0,0,1))
	{
		ASSERT(FALSE);
		return;
	}

	// Calculate the bounding box for the selected brushes
	CBoundingBox boundingBox;	

	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Setup the undo
	GetRegionDoc()->SetupUndoForSelections();

	// Indicates that the following iterator is seeing the first selected brush
	BOOL bFirstSelection=TRUE;

	// Create the bounding box
	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Check to see if this is a brush
		CWorldNode *pNode=pRegion->GetSelection(i);
		if (pNode->GetType() == Node_Brush)
		{
			// Get the node as a brush
			CEditBrush *pBrush=pNode->AsBrush();

			// Set the initial bounding box points
			if (bFirstSelection)
			{
				boundingBox.Init(pBrush->m_Points[0]);
				bFirstSelection=FALSE;
			}

			// Update the bounding box with the brush points
			int j;
			for (j=0; j < pBrush->m_Points.GetSize(); j++)
			{
				boundingBox.Extend(pBrush->m_Points[j]);
			}

			m_pView->GetRegion()->UpdateBrushGeometry(pBrush);
		}
		else if (pNode->GetType() == Node_Object)
		{
			CBaseEditObj *pObject=pNode->AsObject();

			// Set the initial bounding box points
			if (bFirstSelection)
			{
				boundingBox.Init(pObject->GetPos());
				bFirstSelection=FALSE;
			}
			else
			{
				// Update the bounding box
				boundingBox.Extend(pObject->GetPos());
			}
		}
	}

	// Get the center point of the bounding box
	CVector vCenter(boundingBox.m_Min.x+((boundingBox.m_Max.x-boundingBox.m_Min.x)/2),
					boundingBox.m_Min.y+((boundingBox.m_Max.y-boundingBox.m_Min.y)/2),
					boundingBox.m_Min.z+((boundingBox.m_Max.z-boundingBox.m_Min.z)/2));


	//the axis that we need to mirror the texture coordinates over
	//Note: This is not the same as the axis passed in. For example, if they
	//pass in the Y axis, then we need to mirror using the X Axis since
	//it takes a plane normal to use to reflect -JohnO
	LTVector vMirrorTexCoords;

	// Mirror the points for the selected nodes
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Check to see if this is a brush
		CWorldNode *pNode=pRegion->GetSelection(i);
		if (pNode->GetType() == Node_Brush)
		{
			// Get the node as a brush
			CEditBrush *pBrush=pNode->AsBrush();

			// Mirror each point
			int j;
			for (j=0; j < pBrush->m_Points.GetSize(); j++)
			{
				// Get this point
				CVector *pVec=&pBrush->m_Points[j];

				if (vAxis.x == 1.0)
				{
					//determine the axis we need to mirror OPQ's over
					vMirrorTexCoords.Init(0, 1, 0);
					pVec->y += (vCenter.y-pVec->y)*2;
				}
				else if (vAxis.y == 1.0)
				{
					//determine the axis we need to mirror OPQ's over
					vMirrorTexCoords.Init(1, 0, 0);
					pVec->x += (vCenter.x-pVec->x)*2;
				}
				else if (vAxis.z == 1.0)
				{
					//determine the axis we need to mirror OPQ's over
					vMirrorTexCoords.Init(0, 0, 1);
					pVec->z += (vCenter.z-pVec->z)*2;
				}
			}

			// For texture coordinates..
			mOReflection.SetupReflectionMatrix(vMirrorTexCoords, vCenter);
			mPQReflection.SetupReflectionMatrix(vMirrorTexCoords, DVector(0.0f, 0.0f, 0.0f));


			pBrush->FlipNormals();
			
			// Update the polies with a stuck texture.
			for(iPoly=0; iPoly < pBrush->m_Polies; iPoly++)
			{
				pPoly = pBrush->m_Polies[iPoly];

				for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
				{
					CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);

					Texture.GetTextureSpace(O, P, Q);
					
					O = mOReflection * O;
					P = mPQReflection * P;
					Q = mPQReflection * Q;

					pPoly->SetTextureSpace(nCurrTex, O, P, Q);
				}
			}
		}
		else if (pNode->GetType() == Node_Object)
		{
			CBaseEditObj *pObject=pNode->AsObject();

			// Get this point
			LTVector vec=pObject->GetPos();

			if (vAxis.x == 1.0)
			{
				vec.y += (vCenter.y-vec.y)*2;
			}
			else if (vAxis.y == 1.0)
			{
				vec.x += (vCenter.x-vec.x)*2;
			}
			else if (vAxis.z == 1.0)
			{
				vec.z += (vCenter.z-vec.z)*2;
			}
			pObject->SetPos(vec);
		}
	}

	// Recalculate plane equations for all planes...
	GetRegion()->UpdatePlanes();
	GetRegion()->CleanupGeometry( );

	// Update the selection box
	GetRegionDoc()->UpdateSelectionBox();	

	// Update properties...
	GetRegionDoc()->SetupPropertiesDlg( FALSE );
}

bool CRegionView::ExtrudePoly( CPolyRef poly, CVertRefArray &newVerts, bool bRemoveOriginal, bool bFlipAll, bool bNewBrush)
{
	uint32			i;
	CEditPoly		*pExtruder;
	CEditPoly		*pCap, *pBase, *pNewPoly;
	CEditBrush		*pBrush;

	if(bNewBrush) 
	{
		// Setup a new brush
		pBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
		ASSERT( pBrush );
		if( !pBrush )
			return false;
	}
	else
	{
		pBrush = poly.m_pBrush;
	}

	pExtruder = poly();
	newVerts.SetSize(0);
	
	// Make the cap polygon .. a copy of iPoly, reversed.
	pCap = new CEditPoly( pBrush );
	ASSERT( pCap );
	if( !pCap )
	{
		if(bNewBrush) 
			delete pBrush;
		return false;
	}

	pCap->m_Indices.SetSize( pExtruder->NumVerts() );
	newVerts.SetSize( pExtruder->NumVerts() );

	if(bNewBrush) // Making a new brush
	{
		// Make the base polygon .. the opposite of the cap
		pBase = new CEditPoly( pBrush );
		ASSERT( pBase );
		if( !pBase )
		{
			delete pBrush;
			return false;
		}

		pBase->m_Indices.SetSize( pExtruder->NumVerts() );

		// Make base poly
		for( i=0; i < pExtruder->NumVerts(); i++ )
		{
			CEditVert	vert =pExtruder->m_pBrush->m_Points[pExtruder->Index(i)];
			pBrush->m_Points.Append( vert );

			newVerts[i].Init( pBrush, i);
			pBase->m_Indices[i] = i;
		}
	}
		
	// Make cap poly	
	for( i=0; i < pExtruder->NumVerts(); i++ )	
	{
		CEditVert	vert = pExtruder->m_pBrush->m_Points[pExtruder->Index(i)];
		pBrush->m_Points.Append( vert );

		newVerts[i].Init( pBrush, pBrush->m_Points.LastI() );
		pCap->m_Indices[i] = pBrush->m_Points.LastI() ;
	}
	
	
	// For each edge, make a 4 point polygon connecting the edges of the 2 polies.
	for( i=0; i < pExtruder->m_Indices; i++ )
	{
		pNewPoly = new CEditPoly( pBrush );
		pNewPoly->m_Indices.SetSize(4);

		if(bNewBrush)
		{
			pNewPoly->m_Indices[0] = pBase->Index(i);
			pNewPoly->m_Indices[1] = pBase->NextIndex(i);
		}
		else
		{
			pNewPoly->m_Indices[0] = pExtruder->Index(i);
			pNewPoly->m_Indices[1] = pExtruder->NextIndex(i);
		}
		
		pNewPoly->m_Indices[2] = pCap->NextIndex(i);
		pNewPoly->m_Indices[3] = pCap->Index(i);

		// Setup texture stuff for poly
		for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		{
			CTexturedPlane& ExtrudeTex = pExtruder->GetTexture(nCurrTex);
			CTexturedPlane& Texture    = pNewPoly->GetTexture(nCurrTex);

			pNewPoly->SetTextureSpace(nCurrTex, ExtrudeTex.GetO(), ExtrudeTex.GetP(), ExtrudeTex.GetQ());
			Texture.m_pTextureName = ExtrudeTex.m_pTextureName;
			Texture.m_pTextureFile = ExtrudeTex.m_pTextureFile;
		}

		pNewPoly->UpdatePlane();

		if( bFlipAll )
			pNewPoly->Flip();

		pBrush->m_Polies.Append( pNewPoly );
	}

	if( bFlipAll )
	{
		pCap->Flip();
	}

	if(bNewBrush)
	{
		// Flip base
		if(!bFlipAll)  pBase->Flip();

		// Setup texture stuff for the base.
		for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		{
			CTexturedPlane& ExtrudeTex = pExtruder->GetTexture(nCurrTex);
			CTexturedPlane& Texture    = pBase->GetTexture(nCurrTex);
			
			pBase->SetTextureSpace(nCurrTex, ExtrudeTex.GetO(), ExtrudeTex.GetP(), ExtrudeTex.GetQ());
			Texture.m_pTextureName = ExtrudeTex.m_pTextureName;
			Texture.m_pTextureFile = ExtrudeTex.m_pTextureFile;
		}

		pBase->UpdatePlane();
		pBrush->m_Polies.Append( pBase );
	}

	// Setup texture stuff for the cap.
	for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
	{
		CTexturedPlane& ExtrudeTex = pExtruder->GetTexture(nCurrTex);
		CTexturedPlane& Texture    = pCap->GetTexture(nCurrTex);
		
		pCap->SetTextureSpace(nCurrTex, ExtrudeTex.GetO(), ExtrudeTex.GetP(), ExtrudeTex.GetQ());
		Texture.m_pTextureName = ExtrudeTex.m_pTextureName;
		Texture.m_pTextureFile = ExtrudeTex.m_pTextureFile;
	}

	pCap->UpdatePlane();
	pBrush->m_Polies.Append( pCap );
	
	if( bRemoveOriginal && !bNewBrush)
	{
		ClearSelections( FALSE, FALSE, TRUE );
		delete pExtruder;
		pBrush->m_Polies.Remove( poly.m_iPoly );
	}

	//update bounding information
	pBrush->UpdateBoundingInfo();

	return true;
}

void CRegionView::ClearSelections( BOOL bClearVerts, BOOL bClearEdges, BOOL bClearPolies, BOOL bClearBrushes )
{
	if( bClearVerts )
	{
		TaggedVerts().GenRemoveAll();
		IVert().Term();
	}

	if( bClearEdges )
		IEdge().Term();

	if( bClearPolies )
	{
		TaggedPolies().Term();
		TaggedPolies().SetSize(1);
	}

	if( bClearBrushes )
		IBrush().Term();
}

void CRegionView::FinishDrawingPoly()
{
	CEditBrush		*pNewBrush;
	CStringDlg		dlg;
	CPolyRef		polyRef;
	CReal			thickness;
	uint32			i;
	CVertRefArray	newVerts;
	CString str;
	BOOL			bOk, bValidNumber;


	// Create the new brush and setup an undo.
	pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);


	// Ask them for a brush thickness if they're in brush edit mode.
	if( GetEditMode() == BRUSH_EDITMODE )
	{
		//load the default in from the registry
		uint32 nDefaultBrushSize = ::GetApp()->GetOptions().GetDWordValue("DefaultBrushSize", 64);

		dlg.m_bAllowNumbers = TRUE;
		bOk = TRUE;
		bValidNumber = FALSE;
		dlg.m_EnteredText.Format( "%d", nDefaultBrushSize );
		while( bOk && !bValidNumber )
		{
			if( bOk = ( dlg.DoModal(IDS_NEWBRUSHCAPTION, IDS_ENTERBRUSHTHICKNESS) == IDOK ))
			{
				thickness = (CReal)atoi( dlg.m_EnteredText );
				if( thickness > 0.0f )
					bValidNumber = TRUE;
				else
					AppMessageBox( IDS_ERROR_BRUSH_THICKNESS, MB_OK );
			}
		}

		if( bOk )
		{
			//save this value for future use
			::GetApp()->GetOptions().SetDWordValue("DefaultBrushSize", (DWORD)thickness);

			SetupDrawingBrush( pNewBrush );
			polyRef.Init( pNewBrush, 0 );
			
			if( ExtrudePoly( polyRef, newVerts, FALSE, TRUE /*flip all*/) )
			{
				for( i=0; i < newVerts; i++ )
					newVerts[i]() -= polyRef()->Normal() * thickness;

				for( i=0; i < pNewBrush->m_Polies; i++ )
				{
					pNewBrush->m_Polies[i]->UpdatePlane();

					for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
					{
						pNewBrush->m_Polies[i]->SetupBaseTextureSpace(nCurrTex);
						pNewBrush->m_Polies[i]->GetTexture(nCurrTex).UpdateTextureID();
					}
				}
			}
			else
			{
				ASSERT( FALSE );
			}				
		}
		else
		{
			no_DestroyNode(GetRegion(), pNewBrush, FALSE);
			DrawingBrush().Term();
			m_EditState = EDIT_NOSTATE;

			// Redraw everything.
			GetRegionDoc()->RedrawAllViews();
			return;
		}
	}
	else
	{
		// Make the new brush.
		SetupDrawingBrush( pNewBrush );
	}

	//update the bounding information for this polygon
	pNewBrush->UpdateBoundingInfo();

	// Close everything.
	DrawingBrush().Term();
	m_EditState = EDIT_NOSTATE;
	UpdateImmediateSelection();


	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
		GetRegion()->ClearSelections();
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	// Redraw everything.
	GetRegionDoc()->RedrawAllViews();
}

#pragma optimize ("", on)

void CRegionView::SetupDrawingBrush( CEditBrush* pNewBrush )
{
	CEditBrush		*pDrawingBrush;
	CEditPoly		*pNew;
	uint32			i;
	char			*pTexName;
	char			*pPhysicsName;

	pDrawingBrush = &DrawingBrush();

	// Finish the poly.

	pDrawingBrush->m_Points.Pop();
	pDrawingBrush->m_Points.Pop();


	// Add on a new brush.
	pNewBrush->CopyEditBrush( pDrawingBrush );

	// Add on a new poly.
	pNew = new CEditPoly( pNewBrush );
	pNewBrush->m_Polies.Append( pNew );


	// Setup the new poly.
	pNew->m_Indices.SetSize( pNewBrush->m_Points.GetSize() );
	for( i=0; i < pNew->m_Indices; i++ )
		pNew->m_Indices[i] = i;
	
	// Make sure it points towards the viewer.

	pNew->UpdatePlane();
	if( (pNew->Normal().Dot(Nav().Pos()) - pNew->Dist()) < 0 )
		pNew->Flip();

	for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		pNew->SetupBaseTextureSpace(nCurrTex);
	
	// if a texture is selected, assign it
	if( GetCurrentTextureName(pTexName) )
		pNew->GetTexture(GetCurrTexture()).m_pTextureName = pTexName;

	pNew->GetTexture(GetCurrTexture()).UpdateTextureID();
}

#pragma optimize ("", on)

void CRegionView::SplitPolyWithEdge( CPolyRef poly, CEditVertArray &verts )
{
	CEditBrush	*pBrush = poly.m_pBrush;
	CEditPoly	*pPoly = poly();

	// Find the points in the brush
	uint32 nBrushIndex[2];
	nBrushIndex[0] = pBrush->FindClosestVert(verts[0]);
	nBrushIndex[1] = pBrush->FindClosestVert(verts.Last());
	// Deal gracefully with vertices that resolve to the same or invalid indices
	if ((nBrushIndex[0] == nBrushIndex[1]) ||
		(nBrushIndex[0] > pBrush->m_Points.GetSize()) ||
		(nBrushIndex[1] > pBrush->m_Points.GetSize()))
		return;

	// Find the indices in the polygon
	uint32 nPolyIndex[2];
	nPolyIndex[0] = pPoly->m_Indices.FindElement(nBrushIndex[0]);
	nPolyIndex[1] = pPoly->m_Indices.FindElement(nBrushIndex[1]);
	// Deal gracefully with bad indices
	if ((nPolyIndex[0] == nPolyIndex[1]) ||
		(nPolyIndex[0] >= pPoly->m_Indices) || 
		(nPolyIndex[1] >= pPoly->m_Indices))
		return;

	// Remember how many vertices we originally had...
	uint32 nTotalVertices = pPoly->m_Indices.GetSize();
	
	// Make a new poly
	CEditPoly *pNewPoly = new CEditPoly(pBrush);
	pNewPoly->CopyEditPoly(pPoly);

	// Add the new poly to the brush
	pBrush->m_Polies.Append(pNewPoly);

	// Sort them...
	if (nPolyIndex[0] > nPolyIndex[1])
	{
		uint32 nTemp = nPolyIndex[1];
		nPolyIndex[1] = nPolyIndex[0];
		nPolyIndex[0] = nTemp;
		CEditPoly *pTemp = pNewPoly;
		pNewPoly = pPoly;
		pPoly = pTemp;
	}

	// Delete the extra vertices from the original poly
	uint32 nRemoveLoop;
	for (nRemoveLoop = 0; nRemoveLoop < nPolyIndex[0]; ++nRemoveLoop)
		pPoly->m_Indices.Remove(0);
	for (nRemoveLoop = nPolyIndex[1] + 1; nRemoveLoop < nTotalVertices; ++nRemoveLoop)
		pPoly->m_Indices.Remove(pPoly->m_Indices.GetSize() - 1);

	// Delete the extra vertices from the new poly
	for (nRemoveLoop = nPolyIndex[0] + 1; nRemoveLoop < nPolyIndex[1]; ++nRemoveLoop)
		pNewPoly->m_Indices.Remove(nPolyIndex[0] + 1);

	// Clean up both polys
	pNewPoly->UpdatePlane();
	pPoly->UpdatePlane();
}

void CRegionView::OnImportTerrainMap( )
{
	CString importExt, fileMask, pathName, sFileName;
	CWorldNode *pNode;

	if( (GetRegion( )->m_Selections != 1) || 
		((pNode = GetRegion()->m_Selections[0])->GetType() != Node_Brush))
	{
		AppMessageBox( IDS_ERR_SINGLE_BRUSH, MB_OK );
		return;
	}

	importExt.LoadString( IDS_PCX_EXTENSION );
	fileMask.LoadString( IDS_PCX_FILEMASK );

	sFileName = GetProject()->m_BaseProjectDir + "\\Textures\\*" + importExt;
	CHelperFileDlg	dlg( TRUE, importExt, (LPCTSTR)sFileName, OFN_FILEMUSTEXIST, fileMask, this );
	if( dlg.DoModal() != IDOK )
		return;

	pathName = dlg.GetPathName( );
	
	DoImportTerrainMap(pNode->AsBrush(), pathName);
}

void SnapVector(CVector *pVec)
{
	pVec->x = (float)floor(pVec->x + 0.5f);
	pVec->y = (float)floor(pVec->y + 0.5f);
	pVec->z = (float)floor(pVec->z + 0.5f);
}

//------------------------------------------------------------------------------
//
//  CRegionView::DoImportTerrainMap
//
//	Creates a brushes represented by pixels in a PCX file, scaled by a bounding
//  brush.
//
//  pBrush is used as the bounding brush.  The extents of the brush enclose all
//  the new brushes.  The new brushes are scaled to completely fill the enclosing
//  brush.
//
//  The PCX file must be at least 2 pixels wide and 2 pixels high.  New brushes
//	are created in groups of 4.  Each one has a flat bottom and flat sides, but
//	the top can be sloped.  The base and top of each brush is a triangle, and the
//	4 brushes in a group fit together to make a square.  The height of the outside
//	corners of this square will be the height of the bounding brush times the 
//	corresponding pixel value divided by 255.  This means the pixel value is the
//	height scale.  The height of the center of the four brushes is the average
//	of the 4 corners.  If all 4 brushes have zero height, then none of the 4 are
//	created.  Each group of 4 pixels defines the heights of the 4 corners.
//
//  The upper left group of 4 pixels of the PCX file correspond to the corner
//	of the bounding brush with the maximum z and minimum x.  The lower right
//	group of 4 pixels of the PCX file correspond to the corner of the bounding
//	brush with the minimum z and maximum x.
//
//------------------------------------------------------------------------------
void CRegionView::DoImportTerrainMap( CEditBrush *pBrush, CString &pathName )
{
	CReal x_min, x_max, y_min, y_max, z_min, z_max;
	CReal x, z, x_delta, z_delta, thickness, xHigh, zHigh, x_half, z_half;

	CEditBrush *pNewBrush;
	CEditPoly *pNewPoly;
	CEditVert vEditVerts[3];
	CPolyRef polyRef;
	int u, v, iBrush;
	CMoFileIO outFile;
	LoadedBitmap pcxTexture;
	CWorldNode *pParent;
	CVertRefArray newVerts;
	int bits, planes;
	DStream *pInFile;
	BOOL bRet;


	BeginWaitCursor( );

	pInFile = streamsim_Open(pathName, "rb");
	if(!pInFile)
	{
		AppMessageBox( IDS_ERR_OPENFILE, MB_OK );
		EndWaitCursor( );
		return;
	}

	bRet = pcx_Create2(pInFile, &pcxTexture );
	pInFile->Release();

	if(!bRet)
	{
		AppMessageBox( IDS_ERR_TERRAINMAP_INVALID, MB_OK );
		EndWaitCursor( );
		return;
	}

	if(pcxTexture.m_Width < 2 || pcxTexture.m_Height < 2 || 
		pcxTexture.m_Format.m_BPP != BPP_8P)
	{
		AppMessageBox( IDS_ERR_TERRAINMAP_INVALIDDIMS, MB_OK );
		EndWaitCursor( );
		return;
	}

	// Add the null node to be parent...
	pParent = GetRegion()->AddNullNode(GetRegion()->GetRootNode());
	ASSERT( pParent );

	// Get x, y, z extents of brush...
	CBoundingBox BBox = pBrush->CalcBoundingBox();

	x_min = BBox.m_Min.x;
	y_min = BBox.m_Min.y;
	z_min = BBox.m_Min.z;
	x_max = BBox.m_Max.x;
	y_max = BBox.m_Max.y;
	z_max = BBox.m_Max.z;

	// Calculate the dimensions a 4 pixel group will map to...
	x_delta = ( x_max - x_min ) / ( pcxTexture.m_Width - 1 );
	z_delta = ( z_max - z_min ) / ( pcxTexture.m_Height - 1 );
	thickness = ( y_max - y_min );

	// Turn off the drawing of the node view
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		GetNodeView()->SetRedraw(FALSE);
	}

	// Loop over the z & v variabls...
	for( z = z_min, v = pcxTexture.m_Height - 2; v >= 0; z += z_delta, v-- )
	{
		// Get the next z coordinate...
		zHigh = z + z_delta;
		
		// Get the middle of the 4 pixel area...
		z_half = z + z_delta * 0.5f;

		// Loop over the x & u variables...
		for( x = x_min, u = 0; u < (int)(pcxTexture.m_Width - 1); x += x_delta, u++ )
		{
			// Get the next x coordinate...
			xHigh = x + x_delta;

			// Get the middle of the 4 pixel area...
			x_half = x + x_delta * 0.5f;

			// Find the heights for the 4 corners and middle...
			CReal y[5];
			y[0] = ( pcxTexture.Pixel( u, v ) / 255.0f ) * thickness;
			y[1] = ( pcxTexture.Pixel( u+1, v ) / 255.0f ) * thickness;
			y[2] = ( pcxTexture.Pixel( u+1, v+1 ) / 255.0f ) * thickness;
			y[3] = ( pcxTexture.Pixel( u, v+1 ) / 255.0f ) * thickness;
			y[4] = ( y[0] + y[1] + y[2] + y[3] ) / 4.0f;

			// Skip this 4 pixel area if there is no height...
			if( y[4] < 0.0001f )
				continue;

			// Find the x/y/z coordinates for the 4 corners ( middle already known )...
			// This defines a table of values so that each brush can be created in a loop...
			CReal xVert[4][2], zVert[4][2], yVert[4][2];
			xVert[0][0] = x;		xVert[0][1] = x;
			yVert[0][0] = y[0];		yVert[0][1] = y[3];
			zVert[0][0] = zHigh;	zVert[0][1] = z;
			xVert[1][0] = xHigh;	xVert[1][1] = x;
			yVert[1][0] = y[1];		yVert[1][1] = y[0];
			zVert[1][0] = zHigh;	zVert[1][1] = zHigh;
			xVert[2][0] = xHigh;	xVert[2][1] = xHigh;
			yVert[2][0] = y[2];		yVert[2][1] = y[1];
			zVert[2][0] = z;		zVert[2][1] = zHigh;
			xVert[3][0] = x;		xVert[3][1] = xHigh;
			yVert[3][0] = y[3];		yVert[3][1] = y[2];
			zVert[3][0] = z;		zVert[3][1] = z;

			// Loop over the 4 brushes to be created out of the 4 pixel area...
			for( iBrush = 0; iBrush < 4; iBrush++ )
			{
				// Setup a new brush and a new starting poly...
				pNewBrush = new CEditBrush;
				pNewPoly = new CEditPoly( pNewBrush );
				pNewBrush->m_Polies.Append( pNewPoly );

				vEditVerts[2].Init( xVert[iBrush][0], y_min, zVert[iBrush][0] );
				vEditVerts[1].Init( x_half, y_min, z_half );
				vEditVerts[0].Init( xVert[iBrush][1], y_min, zVert[iBrush][1] );

				SnapVector(&vEditVerts[0]);
				SnapVector(&vEditVerts[1]);
				SnapVector(&vEditVerts[2]);

				// Make bottom of brush
				pNewPoly->m_Indices.Append( 0 );
				pNewBrush->m_Points.Append( vEditVerts[0] );
				pNewPoly->m_Indices.Append( 1 );
				pNewBrush->m_Points.Append( vEditVerts[1] );
				pNewPoly->m_Indices.Append( 2 );
				pNewBrush->m_Points.Append( vEditVerts[2] );

				// Set normal...
				pNewPoly->Normal().Init( 0.0f, -1.0f, 0.0f );
				pNewPoly->Dist() = pNewPoly->Normal().Dot( pNewPoly->Pt(0) );

				// Extrude...
				polyRef.Init( pNewBrush, 0 );
				if( ExtrudePoly( polyRef, newVerts, FALSE, TRUE /*flip all*/) )
				{
					CEditVert *pVert;
					pVert = &newVerts[0]( );
					newVerts[2]() -= polyRef()->Normal() * yVert[iBrush][0];
					newVerts[1]() -= polyRef()->Normal() * y[4];
					newVerts[0]() -= polyRef()->Normal() * yVert[iBrush][1];

					for( uint32 j=0; j < pNewBrush->m_Polies; j++ )
					{
						pNewBrush->m_Polies[j]->UpdatePlane();

						for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
							pNewBrush->m_Polies[j]->SetupBaseTextureSpace(nCurrTex);					}
				}

				// Setup the node stuff.
				SetupBrushProperties(pNewBrush, GetRegion());
				GetRegion()->AttachNode(pNewBrush->AsNode(), pParent);
				GetNodeView( )->AddNode(pNewBrush->AsNode());

				// Add on the new brush.
				GetRegion()->AddBrush(pNewBrush);
			}
		}
	}	
	
	// Start drawing the node view again
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		// Turn redraw back on
		GetNodeView()->SetRedraw(TRUE);

		// Redraw the window
		GetNodeView()->Invalidate();	
	}

	GetRegion( )->UpdatePlanes();
	GetRegion( )->CleanupGeometry();
	GetRegionDoc()->RedrawAllViews();

	EndWaitCursor( );
}


// Rotates the polygon's texture coordinates around the specified origin and axis.
void RotatePolyTCoords(CEditPoly *pPoly, uint32 nTexture,
	const LTVector& vOrigin, const LTVector& vAxis, 
	float fRot, DVector &newO, DVector &newP, DVector &newQ)
{
	LTMatrix mRot;

	Mat_SetupRot(&mRot, &vAxis, fRot);
	
	newO = (mRot * (pPoly->GetTexture(nTexture).GetO() - vOrigin)) + vOrigin;
	newP = mRot * pPoly->GetTexture(nTexture).GetP();
	newQ = mRot * pPoly->GetTexture(nTexture).GetQ();
}


// The Converge callback
class TCoordNormalConvertData
{
public:
	CEditPoly	*m_pPoly;
	DVector		m_vOrigin;
	DVector		m_vAxis;
};

float AlignTextureConvergeCB(float testVal, void *pUser)
{
	TCoordNormalConvertData *pData;
	DVector vNormal, newO, newP, newQ;

	pData = (TCoordNormalConvertData*)pUser;
	RotatePolyTCoords(pData->m_pPoly, GetCurrTexture(), pData->m_vOrigin, pData->m_vAxis,
		testVal, newO, newP, newQ);

	vNormal = newO.Cross(newP);
	vNormal.Norm();
	return vNormal.Dot(pData->m_pPoly->Normal());
}


BOOL FindJoinEdge(CEditPoly *pPoly1, CEditPoly *pPoly2, DVector edge[2])
{
	uint32 i, j;

	for(i=0; i < pPoly1->NumVerts(); i++)
	{
		for(j=0; j < pPoly2->NumVerts(); j++)
		{
			if((pPoly1->Pt(i).NearlyEquals(pPoly2->Pt(j)) && pPoly1->NextPt(i).NearlyEquals(pPoly2->NextPt(j))) ||
				(pPoly1->Pt(i).NearlyEquals(pPoly2->NextPt(j)) && pPoly1->NextPt(i).NearlyEquals(pPoly2->Pt(j))))
			{
				edge[0] = pPoly1->Pt(i);
				edge[1] = pPoly1->NextPt(i);
				return TRUE;
			}
		}
	}

	return FALSE;
}


void CRegionView::OnAlignPolyTextures()
{
	CEditPoly *polies[2], *pMaster, *pAlign;
	TCoordNormalConvertData theData;
	float fBestRot;
	DVector newO, newP, newQ;
	DVector vEdge[2];
	uint32 i;
	CMoArray<CEditPoly*> selectedPolies;
	PreActionList actionList;

	
	GetSelectedPolies(selectedPolies);
	
	if(selectedPolies.GetSize() < 2)
		return;

	// Setup the undo.
	for(i=0; i < selectedPolies; i++)
	{
		AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, selectedPolies[i]->m_pBrush), TRUE);
	}
	GetRegionDoc()->Modify(&actionList, TRUE);

	for(i=0; i < selectedPolies.GetSize() - 1; i++)
	{
		pMaster = selectedPolies[i];
		pAlign = selectedPolies[i+1];

		// Find the edge they join on.
		if(!FindJoinEdge(pMaster, pAlign, vEdge))
			break;

		// Copy the texture space.
		CTexturedPlane& AlignTex = pAlign->GetTexture(GetCurrTexture());
		CTexturedPlane& MasterTex = pMaster->GetTexture(GetCurrTexture());
		pAlign->SetTextureSpace(GetCurrTexture(), MasterTex.GetO(), MasterTex.GetP(), MasterTex.GetQ());

		// Rotate it around the edge vector until it is closest to the edge normal.
		theData.m_pPoly = pAlign;
		theData.m_vOrigin = vEdge[0];
		theData.m_vAxis = vEdge[1] - vEdge[0];
		theData.m_vAxis.Norm();

		fBestRot = Converge(0.0f, MATH_CIRCLE, AlignTextureConvergeCB, &theData,
			20, 20);

		// Apply the final one.
		RotatePolyTCoords(pAlign, GetCurrTexture(), theData.m_vOrigin, theData.m_vAxis, 
			fBestRot, newO, newP, newQ);
		
		pAlign->SetTextureSpace(GetCurrTexture(), newO, newP, newQ);
	}

	GetRegionDoc()->RedrawAllViews();
}


