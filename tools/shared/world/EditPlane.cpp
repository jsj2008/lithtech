//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditPlane.h
//
//	PURPOSE	  : Defines the CEditPlane class.  This is used in 
//              CBasePolies and has functions to split them.
//
//	CREATED	  : October 21 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editplane.h"
#include "basepoly.h"


// These are filled in on GetPolySide and used by SplitPoly.
uint32		g_nEditIntersect, g_nEditBack, g_nEditFront;
PolySide	g_EditPointSides[MAX_POLYSIDE_POINTS];
CReal		g_EditPointDots[MAX_POLYSIDE_POINTS];


PolySide CEditPlane::GetPolySide( CBasePoly *pPoly )
{
	WORD			i;

	
	// Classify all the points.
	g_nEditIntersect = g_nEditBack = g_nEditFront = 0;
	for( i=0; i < pPoly->m_Indices; i++ )
	{
		g_EditPointDots[i] = m_Normal.Dot(pPoly->Pt(i)) - m_Dist;
		if( g_EditPointDots[i] > POINT_SIDE_EPSILON )
		{
			++g_nEditFront;
			g_EditPointSides[i] = FrontSide;
		}
		else if( g_EditPointDots[i] < -POINT_SIDE_EPSILON )
		{
			++g_nEditBack;
			g_EditPointSides[i] = BackSide;
		}
		else
		{
			++g_nEditIntersect;
			g_EditPointSides[i] = Intersect;
		}
	}


	// See which case it is...
	if( g_nEditIntersect == pPoly->m_Indices )
	{
		if( m_Normal.Dot(pPoly->Normal()) >= 0 )
			return FrontSide;
		else
			return BackSide;
	}
	else if( g_nEditFront == 0 )
	{
		return BackSide;
	}
	else if( g_nEditBack == 0 )
	{
		return FrontSide;
	}
	else
		return Intersect;
}


void CEditPlane::SplitPoly( CBasePoly *pToSplit, CBasePoly *sides[2], CEditVertArray &verts )
{
	WORD			iCur, iNext;
	PolySide		curSide, nextSide;
	CEditVert		newPt;
	CReal			t;

	
	nextSide = g_EditPointSides[0];
	for( iCur=0; iCur < pToSplit->NumVerts(); iCur++ )
	{
		iNext = iCur+1;
		if( iNext >= pToSplit->NumVerts() )
			iNext = 0;

		curSide = nextSide;
		nextSide = g_EditPointSides[iNext];


		if( curSide == Intersect )
		{
			sides[0]->m_Indices.Append(pToSplit->m_Indices[iCur]);
			sides[1]->m_Indices.Append(pToSplit->m_Indices[iCur]);
			continue;
		}
		
		sides[curSide]->m_Indices.Append(pToSplit->m_Indices[iCur]);
		if( nextSide == Intersect || nextSide == curSide )
			continue;
			
		// Do an intersection.
		CEditVert &A = pToSplit->Pt(iCur);
		CEditVert &B = pToSplit->Pt(iNext);
		
		t = -g_EditPointDots[iCur] / (g_EditPointDots[iNext] - g_EditPointDots[iCur]);
		ASSERT( t >= 0 && t <= 1 );

		newPt = A + ( (B - A) * t );
		verts.Append( newPt );

		sides[0]->m_Indices.Append( (WORD)verts.LastI() );
		sides[1]->m_Indices.Append( (WORD)verts.LastI() );
	}

	sides[0]->CopyAttributes( pToSplit );
	sides[1]->CopyAttributes( pToSplit );
}


void CEditPlane::SplitPoly( CBasePoly *pToSplit, CBasePoly *sides[2], CEditVertArray &verts1, CEditVertArray &verts2 )
{
	CEditVertArray	*vertSides[2] = { &verts1, &verts2 };

	WORD			iCur, iNext;
	PolySide		curSide, nextSide;
	CEditVert		newPt;
	CReal			t;

	
	nextSide = g_EditPointSides[0];
	for( iCur=0; iCur < pToSplit->NumVerts(); iCur++ )
	{
		iNext = iCur+1;
		if( iNext >= pToSplit->NumVerts() )
			iNext = 0;

		curSide = nextSide;
		nextSide = g_EditPointSides[iNext];


		if( curSide == Intersect )
		{
			vertSides[0]->Append( pToSplit->Pt(iCur) );
			vertSides[1]->Append( pToSplit->Pt(iCur) );
			
			sides[0]->m_Indices.Append( (WORD)vertSides[0]->LastI() );
			sides[1]->m_Indices.Append( (WORD)vertSides[1]->LastI() );
			continue;
		}
		
		vertSides[curSide]->Append( pToSplit->Pt(iCur) );
		sides[curSide]->m_Indices.Append( (WORD)vertSides[curSide]->LastI() );
		if( nextSide == Intersect || nextSide == curSide )
			continue;
			
		// Do an intersection.
		CEditVert &A = pToSplit->Pt(iCur);
		CEditVert &B = pToSplit->Pt(iNext);
		
		t = -g_EditPointDots[iCur] / (g_EditPointDots[iNext] - g_EditPointDots[iCur]);
		ASSERT( t >= 0 && t <= 1 );

		newPt = A + ( (B - A) * t );

		newPt.m_nA = (uint8)((float)A.m_nA + ((B.m_nA - A.m_nA) * t));
		newPt.m_nR = (uint8)((float)A.m_nR + ((B.m_nA - A.m_nR) * t));
		newPt.m_nG = (uint8)((float)A.m_nG + ((B.m_nA - A.m_nG) * t));
		newPt.m_nB = (uint8)((float)A.m_nB + ((B.m_nA - A.m_nB) * t));

		vertSides[0]->Append( newPt );
		vertSides[1]->Append( newPt );
		
		sides[0]->m_Indices.Append( (WORD)vertSides[0]->LastI() );
		sides[1]->m_Indices.Append( (WORD)vertSides[1]->LastI() );
	}

	sides[0]->CopyAttributes( pToSplit );
	sides[1]->CopyAttributes( pToSplit );
}


