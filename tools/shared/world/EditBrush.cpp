//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditBrush.cpp
//
//	PURPOSE	  : Implements the CEditBrush class.
//
//	CREATED	  : November 12 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editbrush.h"
#include "editpoly.h"
#include "node_ops.h"
#include "ltamgr.h"
#include "ltasaveutils.h"

#ifdef DIRECTEDITOR_BUILD
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif
#endif

CEditBrush::CEditBrush()
{
	m_Type			= Node_Brush;

	m_pRegion		= NULL;

	m_Polies.SetCacheSize( 15 );
	m_Points.SetCacheSize( 15 );

	r = g = b = 255;

	SetFlag(BRUSHFLAG_VISIBLE, true);
}


CEditBrush::~CEditBrush()
{
	Term();
}


void CEditBrush::Term()
{
	DeleteAndClearArray( m_Polies );
	m_Points.Term();

	CWorldNode::Term();
}


void CEditBrush::DoCopy(CWorldNode *pOther)
{
	CopyEditBrush(pOther->AsBrush());
	CWorldNode::DoCopy(pOther);
}

void CEditBrush::DoCopy(CWorldNode *pOther, CStringHolder *pStringHolder)
{
	CopyEditBrush(pOther->AsBrush(), pStringHolder);
	CWorldNode::DoCopy(pOther);
}


/*
( polyhedron 0
			( color ( 255 255 255 ) )
			( pointlist (
				( point  ( -300.000000 300.000000 -300.000000 ) )
				( point  ( -300.000000 300.000000 300.000000 ) )
				( point  ( 300.000000 300.000000 300.000000 ) )
				( point  ( 300.000000 300.000000 -300.000000 ) )
			) )
			( polylist (
				( editpoly 
					( basepoly  
						( indices ( 0 1 2 3 ) )
						( normal ( 0.000000 1.000000 0.000000 ) )
						( dist 300.000000 )
					)
					( textureinfo 
						( O ( 0.000000 0.000000 0.000000 ) )
						( P ( 1.000000 0.000000 0.000000 ) )
						( Q ( 0.000000 0.000000 -1.000000 ) )
						( sticktopoly 1 )
						( name "Textures\St0004.dtx" )
					)
					( flags ( ) ) 
					( shade ( 0 0 0 ) ) 
				) 
			) )
		)
*/


BOOL CEditBrush::LoadLTA( CLTANode* pNode, CStringHolder *pStringHolder )
{
	Term();
	// ( color ( r g b ) )
	CLTANode * pData = pNode; //pNode->getElem(2);

	CLTANode* pColorNode = pData->GetElement(0);// PairCdrNode( shallow_find_list(pNode, "color") );
	r = GetUint8(pColorNode->GetElement(1));
	g = GetUint8(pColorNode->GetElement(2));
	b = GetUint8(pColorNode->GetElement(3));

	CLTANode* pPoints = pData->GetElement(1); //PairCdrNode(shallow_find_list(pNode, "pointlist" ));
	CLTANode* pCurPoint = NULL;

	uint32 listSize = pPoints->GetNumElements();

	assert(listSize > 0);
	m_Points.SetSize( listSize - 1 );

	for( uint32 i=1; i < listSize; i++ )
	{
		pCurPoint = pPoints->GetElement(i);
		uint32 nNumChildren = pCurPoint->GetNumElements();

		m_Points[i-1].x = GetFloat(pCurPoint->GetElement(0));
		m_Points[i-1].y = GetFloat(pCurPoint->GetElement(1));
		m_Points[i-1].z = GetFloat(pCurPoint->GetElement(2));

		//see if they also have colors to be loaded in
		if(nNumChildren >= 7)
		{
			//they have the vertex colors, read them in
			m_Points[i-1].m_nR = GetUint8(pCurPoint->GetElement(3));
			m_Points[i-1].m_nG = GetUint8(pCurPoint->GetElement(4));
			m_Points[i-1].m_nB = GetUint8(pCurPoint->GetElement(5));
			m_Points[i-1].m_nA = GetUint8(pCurPoint->GetElement(6));
		}
		else
		{
			//default these values to something standard...
			m_Points[i-1].m_nR = 255;
			m_Points[i-1].m_nG = 255;
			m_Points[i-1].m_nB = 255;
			m_Points[i-1].m_nA = 255;
		}
	}

	CLTANode* pPolyList = PairCdrNode(pData->GetElement(2)); // PairCdrNode(shallow_find_list(pNode, "polylist" ));
	listSize = pPolyList->GetNumElements();
	m_Polies.SetSize( listSize );
	for( i=0; i < listSize; i++ )
	{
		m_Polies[i] = new CEditPoly( this );
		if( !m_Polies[i]->LoadEditPolyLTA(pPolyList->GetElement(i), pStringHolder ) )
			return FALSE;
	}

	UpdatePlanes();
	UpdateBoundingInfo();

	return TRUE;
}


void CEditBrush::SaveLTA( CLTAFile* pFile, uint32 level )
{
	//sanity check
	ASSERT(pFile);

	uint32	   i;
	CEditPoly *pEditPoly = NULL;

	//print out the entire node list
	PrependTabs(pFile, level); 
	pFile->WriteStrF("( polyhedron (", m_RegionBrushIndex );
		PrependTabs(pFile, level+1); 
		pFile->WriteStrF("( color %d %d %d )", r, g, b );
		PrependTabs(pFile, level+1); 
		pFile->WriteStr("( pointlist ");
			for( i=0; i < m_Points; i++ ) {
				PrependTabs(pFile, level+2);
				pFile->WriteStrF("( %f %f %f %d %d %d %d )", m_Points[i].x,  m_Points[i].y, m_Points[i].z, m_Points[i].m_nR, m_Points[i].m_nG, m_Points[i].m_nB, m_Points[i].m_nA );
			}
		PrependTabs(pFile, level+1);
		pFile->WriteStr(")");

		PrependTabs(pFile, level+1);
		pFile->WriteStr("( polylist (");
			for( i=0; i < m_Polies; i++ )
			{
				pEditPoly = m_Polies[i];
				pEditPoly->SaveEditPolyLTA( pFile, level+2 );
			}
		PrependTabs(pFile, level+1);
		pFile->WriteStr(") )");
	PrependTabs(pFile, level);
	pFile->WriteStr(") )");
}

void CEditBrush::SaveTBW( CAbstractIO& OutFile )
{
	//write out the brush color
	OutFile << r << g << b;

	//write out the list of points used by this brush
	uint32 nNumPoints = m_Points.GetSize();
	OutFile << nNumPoints;

	for(uint32 nCurrPoint = 0; nCurrPoint < m_Points.GetSize(); nCurrPoint++)
	{
		//position
		OutFile << m_Points[nCurrPoint].x << m_Points[nCurrPoint].y << m_Points[nCurrPoint].z;
		//color
		OutFile << m_Points[nCurrPoint].m_nR << m_Points[nCurrPoint].m_nG << m_Points[nCurrPoint].m_nB << m_Points[nCurrPoint].m_nA;
	}

	//write out the list of polygons used by this brush
	uint32 nNumPolies = m_Polies.GetSize();
	OutFile << nNumPolies;

	//now write out the polygons
	for(uint32 nCurrPoly = 0; nCurrPoly < m_Polies.GetSize(); nCurrPoly++)
	{
		m_Polies[nCurrPoly]->SaveEditPolyTBW(OutFile);
	}
}

bool CEditBrush::LoadTBW( CAbstractIO& InFile, CStringHolder *pStringHolder )
{
	//clean up the brush
	Term();

	//load in our color
	InFile >> r >> g >> b;

	//how many points we have
	uint32 nNumPoints;
	InFile >> nNumPoints;

	//resize our point list
	m_Points.SetSize(nNumPoints);

	//read them in
	for( uint32 nCurrPoint = 0; nCurrPoint < nNumPoints; nCurrPoint++ )
	{
		CEditVert* pCurrPoint = &m_Points[nCurrPoint];

		//position
		InFile >> pCurrPoint->x >> pCurrPoint->y >> pCurrPoint->z;

		//color
		InFile >> pCurrPoint->m_nR >> pCurrPoint->m_nG >> pCurrPoint->m_nB >> pCurrPoint->m_nA;
	}

	//now read in the polygons
	uint32 nNumPolygons;
	InFile >> nNumPolygons;

	m_Polies.SetSize( nNumPolygons );

	for( uint32 nCurrPoly = 0; nCurrPoly < nNumPolygons; nCurrPoly++ )
	{
		m_Polies[nCurrPoly] = new CEditPoly( this );
		if( !m_Polies[nCurrPoly]->LoadEditPolyTBW(InFile, pStringHolder ) )
			return false;
	}

	UpdatePlanes();
	UpdateBoundingInfo();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Determines whether or not the brush has an open side
// Note : Brushes containing T-Junctions will be reported as open
BOOL CEditBrush::IsOpen()
{
	// If it's got less than 4 polies, it's definately open
	if (m_Polies.GetSize() < 4)
		return TRUE;

	// Keep track of the normal directions..
	uint32 nNormalFlags = 0;
	const uint32 NORMAL_FLAG_XPOS = 1 << 0;
	const uint32 NORMAL_FLAG_XNEG = 1 << 1;
	const uint32 NORMAL_FLAG_YPOS = 1 << 2;
	const uint32 NORMAL_FLAG_YNEG = 1 << 3;
	const uint32 NORMAL_FLAG_ZPOS = 1 << 4;
	const uint32 NORMAL_FLAG_ZNEG = 1 << 5;
	const uint32 NORMAL_FLAG_ALL = (1 << 6) - 1;

	uint32 nPolyLoop;

	UpdatePlanes();

	for (nPolyLoop = 0; nPolyLoop < m_Polies; ++nPolyLoop)
	{
		CEditPlane *pPlane = &m_Polies[nPolyLoop]->m_Plane;

		// Update the normal directions flags
		if (pPlane->m_Normal.x > 0)
			nNormalFlags |= NORMAL_FLAG_XPOS;
		if (pPlane->m_Normal.x < 0)
			nNormalFlags |= NORMAL_FLAG_XNEG;
		if (pPlane->m_Normal.y > 0)
			nNormalFlags |= NORMAL_FLAG_YPOS;
		if (pPlane->m_Normal.y < 0)
			nNormalFlags |= NORMAL_FLAG_YNEG;
		if (pPlane->m_Normal.z > 0)
			nNormalFlags |= NORMAL_FLAG_ZPOS;
		if (pPlane->m_Normal.z < 0)
			nNormalFlags |= NORMAL_FLAG_ZNEG;
	}

	// If we didn't find all of the cardinal directions in the brush, it's got an open face
	// (This is useful for doing a quick-check and detects a sub-divided dual-sided polygon)
	if (nNormalFlags != NORMAL_FLAG_ALL)
		return TRUE;

	// We need to be sure there's no duplicate vertices for this to work...
	RemoveDuplicatePoints(0.01f);

	// Run through the polygons looking for un-shared edges
	for (nPolyLoop = 0; nPolyLoop < m_Polies.GetSize(); ++nPolyLoop)
	{
		CEditPoly *pPoly = m_Polies[nPolyLoop];
		for (uint32 nEdgeLoop = 0; nEdgeLoop < pPoly->m_Indices.GetSize(); ++nEdgeLoop)
		{
			uint32 nPolyEdge[2];

			nPolyEdge[0] = pPoly->m_Indices[nEdgeLoop];
			nPolyEdge[1] = pPoly->m_Indices.Next(nEdgeLoop);

			bool bFoundEdge = false;

			for (uint32 nOtherPolyLoop = 0; (!bFoundEdge) && (nOtherPolyLoop < m_Polies.GetSize()); ++nOtherPolyLoop)
			{
				if (nOtherPolyLoop == nPolyLoop)
					continue;

				CEditPoly *pOtherPoly = m_Polies[nOtherPolyLoop];

				for (uint32 nOtherEdgeLoop = 0; (!bFoundEdge) && (nOtherEdgeLoop < pOtherPoly->m_Indices.GetSize()); ++nOtherEdgeLoop)
				{
					uint32 nOtherEdge[2];

					nOtherEdge[0] = pOtherPoly->m_Indices[nOtherEdgeLoop];
					nOtherEdge[1] = pOtherPoly->m_Indices.Next(nOtherEdgeLoop);

					// Compare the poly edge and the other edge, in both directions
					bFoundEdge = ((nPolyEdge[0] == nOtherEdge[0]) && (nPolyEdge[1] == nOtherEdge[1])) ||
						((nPolyEdge[0] == nOtherEdge[1]) && (nPolyEdge[1] == nOtherEdge[0]));
				}
			}

			// Found an un-shared edge..  This brush seems open..
			if (!bFoundEdge)
				return TRUE;
		}
	}

	return FALSE;
}


void CEditBrush::CopyEditBrush( CEditBrush *pOther, CStringHolder *pStringHolder )
{
	uint32		i;

	
	m_Polies.SetSize( pOther->m_Polies.GetSize() );
	for( i=0; i < pOther->m_Polies; i++ )
	{
		m_Polies[i] = new CEditPoly;
		m_Polies[i]->CopyEditPoly( pOther->m_Polies[i], TRUE, pStringHolder );
		m_Polies[i]->m_pBrush = this;
	}

	m_Points.CopyArray( pOther->m_Points );
	m_BoundingSphere = pOther->m_BoundingSphere;
	
	r = pOther->r;
	g = pOther->g;
	b = pOther->b;
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::GetUpperLeftCornerPos
//      PURPOSE:        Returns the upper left corner of the brush as it
//						would appear in DEdit.
// ----------------------------------------------------------------------- //
CVector CEditBrush::GetUpperLeftCornerPos()
{
	CVector vUpperLeft;
	vUpperLeft.x=0.0f;
	vUpperLeft.y=0.0f;
	vUpperLeft.z=0.0f;

	if (m_Points <= 0)
	{
		return vUpperLeft;
	}

	// Set the intial upper left point
	vUpperLeft=m_Points[0];

	// Find the smallest bounding edge that would appear in the upper left corner of the DEdit windows
	uint32 i;
	for( i=1; i < m_Points; i++ )
	{
		if (m_Points[i].x < vUpperLeft.x)
		{
			vUpperLeft.x=m_Points[i].x;
		}
		if (m_Points[i].y > vUpperLeft.y)
		{
			vUpperLeft.y=m_Points[i].y;
		}
		if (m_Points[i].z > vUpperLeft.z)
		{
			vUpperLeft.z=m_Points[i].z;
		}		
	}

	return vUpperLeft;
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::MoveBrush
//      PURPOSE:        Moves a brush to a position.
// ----------------------------------------------------------------------- //
void CEditBrush::MoveBrush(const LTVector& vPos)
{
	OffsetBrush(vPos-GetUpperLeftCornerPos());
}

void CEditBrush::OffsetBrush(const LTVector& vDelta)
{
	// Offset the brush	
	uint32 i;
	for (i=0; i < m_Points; i++)
	{
		m_Points[i]+=vDelta;
	}

	// Offset the texture coords
	for(i=0; i < m_Polies; i++)
	{
		CEditPoly *pPoly = m_Polies[i];

		for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		{
			CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);
			pPoly->SetTextureSpace(nCurrTex, Texture.GetO() + vDelta, Texture.GetP(), Texture.GetQ());
		}
	}

	//also move the bounding sphere...
	UpdateBoundingInfo();

	//update our planes
	UpdatePlanes();
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::Rotate
//      PURPOSE:        Rotate the brush about a point
// ----------------------------------------------------------------------- //

void CEditBrush::Rotate(LTMatrix &mMatrix, LTVector &vCenter)
{
	uint32 i;
	// Loop over all brush points...
	for( i=0; i < m_Points.GetSize( ); i++ )
	{
		CEditVert *pPt = &m_Points[i];

		// Rotate point around rotation center...
		*pPt -= vCenter;
		mMatrix.Apply( *pPt );
		*pPt += vCenter;
	}

	UpdatePlanes();
	UpdateBoundingInfo();

	// Update texture coordinates for 'stuck' polygons.
	for(i=0; i < m_Polies; i++)
	{
		CEditPoly *pPoly = m_Polies[i];

		for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		{
			CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);

			// Rotate the old texture vectors and texture origin.
			LTVector newP, newQ, newO;
			
			//find the transformed origin
			newO = Texture.GetO() - vCenter;
			mMatrix.Apply(newO);
			newO = newO + vCenter;

			//transform the P and Q (remembering that there could potentially be a translation
			//in the above matrix)
			mMatrix.Apply3x3(Texture.GetP(), newP);
			mMatrix.Apply3x3(Texture.GetQ(), newQ);

			pPoly->SetTextureSpace(nCurrTex, newO, newP, newQ);
		}
	}
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::FindPolyWithEdge
//      PURPOSE:        Finds a polygon that has the given edge.
// ----------------------------------------------------------------------- //

uint32 CEditBrush::FindPolyWithEdge( uint32 iStart, uint32 v1, uint32 v2 )
{
	uint32			p, i;
	CEditPoly		*pPoly;


	for( p=iStart; p < m_Polies; p++ )
	{
		pPoly = m_Polies[p];
		for( i=0; i < pPoly->m_Indices; i++ )
		{
			if( (pPoly->m_Indices[i] == v2) && (pPoly->m_Indices.Next(i) == v1) )
				return p;
		}
	}

	return BAD_INDEX;
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::FindPolyWithPoints
//      PURPOSE:        Finds a polygon that has the given edge.
// ----------------------------------------------------------------------- //

uint32 CEditBrush::FindPolyWithPoints( uint32 index1, uint32 index2 )
{
	uint32		i;
	CEditPoly	*pPoly;

	for( i=0; i < m_Polies; i++ )
	{
		pPoly = m_Polies[i];

		if( (pPoly->m_Indices.FindElement(index1) != BAD_INDEX) && 
			(pPoly->m_Indices.FindElement(index2) != BAD_INDEX) )
			return i;
	}

	return BAD_INDEX;
}


uint32 CEditBrush::FindClosestVert(const LTVector &vec )
{
	uint32		i, iMin = BAD_INDEX;
	CReal		dist, minDist=(CReal)MAX_CREAL;

	for( i=0; i < m_Points; i++ )
	{
		dist = (m_Points[i] - vec).Mag();
		if( dist < minDist )
		{
			iMin = i;
			minDist = dist;
		}
	}

	return iMin;
}


BOOL CEditBrush::FindPoliesWithEdge( uint32 v1, uint32 v2, CEditPoly*&pPoly1, CEditPoly*&pPoly2, uint32 &nContaining )
{
	uint32		i, index;

	nContaining = 0;
	
	pPoly1 = pPoly2 = NULL;
	for( i=0; i < m_Polies; i++ )
	{
		CMoDWordArray	&indices = m_Polies[i]->m_Indices;
		
		if( (index = indices.FindElement(v1)) != BAD_INDEX )
		{
			if( indices.Next(index) == v2 )
			{
				++nContaining;
				pPoly1 = m_Polies[i];
			}
			else if( indices.Prev(index) == v2 )
			{
				++nContaining;
				pPoly2 = m_Polies[i];
			}
		}
	}

	return (pPoly1 && pPoly2) && (nContaining == 2);
}


void CEditBrush::RemovePolyVerts( CMoDWordArray &indices, CMoArray<CEditPoly*> &exclude, CMoArray<CEditPoly*> &changedPolies )
{
	uint32			i, k, index;
	CEditPoly		*pPoly;

	
	for( i=0; i < m_Polies; i++ )
	{
		pPoly = m_Polies[i];

		if( exclude.FindElement(pPoly) == BAD_INDEX )
		{
			for( k=0; k < indices; k++ )
			{
				index = pPoly->m_Indices.FindElement( indices[k] );
				if( (index != BAD_INDEX) && (pPoly->m_Indices >= 4) )
				{
					if( (changedPolies > 0) && (changedPolies.Last() != pPoly) )
						changedPolies.Append( pPoly );

					pPoly->m_Indices.Remove( index );
				}
			}
		}
	}
}


void CEditBrush::RemoveVertex( uint32 iVert )
{
	for( uint32 i=0; i < m_Polies; i++ )
		m_Polies[i]->DecrementPoints( iVert );

	m_Points.Remove( iVert );
}


void CEditBrush::RemoveUnusedVerts()
{
	CMoByteArray	vertsUsed;
	uint32			i, k, nRemoved=0;

	vertsUsed.SetSize( m_Points.GetSize() );
	memset( vertsUsed.GetArray(), 0, vertsUsed.GetSize() );

	for( i=0; i < m_Polies; i++ )
		for( k=0; k < m_Polies[i]->m_Indices; k++ )
			vertsUsed[ m_Polies[i]->m_Indices[k] ] = TRUE;

	for( i=0; i < vertsUsed; i++ )
	{
		if( !vertsUsed[i] )
		{
			RemoveVertex( i - nRemoved );
			nRemoved++;
		}
	}
}


void CEditBrush::RemoveExtraEdges( CReal normalThresh, CReal distThresh )
{
	uint32			i, p;
	uint32			iPoly;
	CEditPoly		*pPoly, *pOther;


	for( p=0; p < m_Polies.GetSize(); p++ )
	{
		pPoly = m_Polies[p];
		
		for( i=0; i < pPoly->m_Indices.GetSize(); i++ )
		{
			iPoly = FindPolyWithEdge( p+1, pPoly->m_Indices[i], pPoly->m_Indices.Next(i) );
			if( iPoly != BAD_INDEX )
			{
				pOther = m_Polies[iPoly];
				
				pPoly->m_Plane.GetPolySide( pOther );
				if( (pPoly->Normal().Dot(pOther->Normal()) > 0.0f) && (g_nEditIntersect == pOther->m_Indices.GetSize()) )
				{
					JoinPolygons( pPoly, pOther, pPoly->m_Indices[i], pPoly->m_Indices.Next(i) );
					i = p = (uint32)-1;
					break;
				}
			}
		}
	}
}


void CEditBrush::ChangeVertexReferences( uint32 from, uint32 to )
{
	for( uint32 i=0; i < m_Polies; i++ )
	{
		for( uint32 k=0; k < m_Polies[i]->m_Indices; k++ )
			if( m_Polies[i]->m_Indices[k] == from )
				m_Polies[i]->m_Indices[k] = to;
	}
}

// Flips all of the polygons
void CEditBrush::FlipNormals()
{
	for( uint32 i=0; i < m_Polies; i++ )
		m_Polies[i]->Flip();
}

void CEditBrush::UpdatePlanes()
{
	for( uint32 i=0; i < m_Polies; i++ )
		m_Polies[i]->UpdatePlane();
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::JoinPolygons
//      PURPOSE:        Joins the two polygons, removing both the originals.
// ----------------------------------------------------------------------- //

CEditPoly* CEditBrush::JoinPolygons( CEditPoly *pPoly1, CEditPoly *pPoly2, uint32 v1, uint32 v2 )
{
	CEditPoly		*pNew;
	uint32			i, temp, i1, i2;
	uint32			iPoly1, iPoly2;
	uint32			nPtsAdded;
	CMoDWordArray	&indices1 = pPoly1->m_Indices;
	CMoDWordArray	&indices2 = pPoly2->m_Indices;


	iPoly1 = m_Polies.FindElement( pPoly1 );
	iPoly2 = m_Polies.FindElement( pPoly2 );

	i1 = indices2.FindElement( v1 );
	i2 = indices2.FindElement( v2 );
	
	if( i2 == indices2.NextI(i1) )
	{
		temp = v1;
		v1 = v2;
		v2 = temp;
	}

	
	// Join the two.
	pNew = new CEditPoly;
	pNew->CopyEditPoly( pPoly1 );

	pNew->m_Indices.Term();
	for( i = indices1.NextI(indices1.FindElement(v2)); indices1[i] != v1; i = indices1.NextI(i) )
		pNew->m_Indices.Append( indices1[i] );

	nPtsAdded = 0;
	for( i = indices2.FindElement(v1); nPtsAdded < indices2; i = indices2.NextI(i) )
	{
		pNew->m_Indices.Append( indices2[i] );
		++nPtsAdded;
	}


	pNew->RemoveCollinearVertices();

	// Delete the old polies and replace them with the new one.
	m_Polies[iPoly1] = pNew;
	m_Polies.Remove(iPoly2);

	delete pPoly1;
	delete pPoly2;

	ASSERT( pNew->m_Indices > 3 );
	return pNew;
}


//calculates a bounding box for this brush
CBoundingBox CEditBrush::CalcBoundingBox()
{
	ASSERT( m_Points > 0 );

	CBoundingBox rv;

	if(m_Points)
	{
		rv.Init( m_Points[0] );	
		for( uint32 i=1; i < m_Points; i++ )
			rv.Extend( m_Points[i] );
	}

	return rv;
}


void CEditBrush::UpdateBSphere()
{
	if(m_Points.GetSize() == 0)
		return;

	//find the center of the sphere
	LTVector vCenter = CalcBoundingBox().Center();

	//now find the radius
	float fMaxRadSqr = 0.0f;
	float fRadSqr;

	for( uint32 nCurrPt = 0; nCurrPt < m_Points; nCurrPt++ )
	{
		fRadSqr = (m_Points[nCurrPt] - vCenter).MagSqr();

		if(fRadSqr > fMaxRadSqr)
		{
			fMaxRadSqr = fRadSqr;
		}
	}

	m_BoundingSphere.Init(vCenter, (float)sqrt(fMaxRadSqr));
}

uint32 CEditBrush::AddVertOrGetClosest(const LTVector& vPos, CReal fThreshold)
{
	//square the distance to avoid sqrts
	fThreshold *= fThreshold;

	//first, run through and see if any vertices already match
	for(uint32 nCurrVert = 0; nCurrVert < m_Points.GetSize(); nCurrVert++)
	{
		//find the distance
		if((m_Points[nCurrVert] - vPos).MagSqr() <= fThreshold)
		{
			//we have a match
			return nCurrVert;
		}
	}

	//no match. Add this vertex
	m_Points.Append(vPos);

	return m_Points.LastI();
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::ChopUpBrush and SplitPolyList
//      PURPOSE:        Used for boolean stuff.  These two actually create new
//                      brushes for the different sides.
// ----------------------------------------------------------------------- //

CEditPoly* MakeCap( CEditPoly *pCapBase, CEditBrush *pChopper, CEditBrush *pCapBrush )
{
	CEditPoly	*pCap;
	uint32		i;
	
	CReal		size = 20000.0f;
	CVector		points[4];
	CVector		u, v, basePt;
	PolySide	side;

	CBasePoly	*newPolies[2];


	// Build the cap (make a huge poly out of the base cap).
	pCap = new CEditPoly( pCapBrush );
	pCap->CopyAttributes( pCapBase );
	pCap->m_pBrush = pCapBrush;
	pCapBase->UpdatePlane();

	u = pCapBase->Pt(1) - pCapBase->Pt(0);
	v = u.Cross( pCapBase->Normal() );

	u.Norm();
	v.Norm();

	// Set up the texture space for the cap
	for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
	{
		pCap->SetTextureSpace(nCurrTex, CVector(0.0f,0.0f,0.0f), u,v);
	}

	u *= size;
	v *= size;

	pCap->m_Indices.SetSize(4);

	basePt=pCapBase->Pt(0);

	for( i=0; i < 4; i++ )
		pCap->m_Indices[i] = pCapBrush->m_Points + i;

	points[0] = ( basePt + (-u + v) );
	points[1] = ( basePt + ( u + v) );
	points[2] = ( basePt + ( u - v) );
	points[3] = ( basePt + (-u - v) );

	for( i=0; i < 4; i++ )
		pCapBrush->m_Points.Append( points[i] );

	// Make sure the polygon goes the right way.
	pCap->UpdatePlane();
	if( pCap->Normal().Dot(pCapBase->Normal()) < 0 )
		pCap->Flip();

	
	// Chop the cap up on the chopper brush.
	for( i=0; i < pChopper->m_Polies; i++ )
	{
		side = pChopper->m_Polies[i]->m_Plane.GetPolySide( pCap );

		if( g_nEditIntersect == pCap->NumVerts() )
		{
		}
		else
		{
//			ASSERT( side != FrontSide );

			if( side == Intersect )
			{
				newPolies[0] = new CEditPoly( pCapBrush );
				newPolies[1] = new CEditPoly( pCapBrush );

				pChopper->m_Polies[i]->m_Plane.SplitPoly( pCap, newPolies, pCapBrush->m_Points );

				delete newPolies[FrontSide];
				delete pCap;
				pCap = (CEditPoly*)newPolies[BackSide];
				ASSERT( pCap->IsValid() );
			}
		}
	}

	return pCap;
}


// Splits the given brush on pRoot's plane.
void SimpleSplitBrush( CEditRegion *pRegion, CEditPoly *pRoot, CEditBrush *pToSplit, CEditBrush *newBrushes[2] )
{
	CBasePoly *newPolies[2];
	CEditPoly *pPoly, *pNewPoly, *pCap;

	PolySide side;
	CMoArray<CEditPoly*> sides[2];
	uint32 i, k;
	

	newBrushes[0] = no_CreateNewBrush(pRegion);
	newBrushes[1] = no_CreateNewBrush(pRegion);

	newBrushes[0]->m_PropList.CopyValues(&pToSplit->m_PropList);
	newBrushes[1]->m_PropList.CopyValues(&pToSplit->m_PropList);

#ifdef DIRECTEDITOR_BUILD
	newBrushes[0]->RefreshAllProperties( NULL );
	newBrushes[1]->RefreshAllProperties( NULL );

#endif

	// Classify the sides that each poly is on.
	for( i=0; i < pToSplit->m_Polies; i++ )
	{
		pPoly = pToSplit->m_Polies[i];
		
		side = pRoot->m_Plane.GetPolySide( pPoly );
		if( side == Intersect )
		{
			newPolies[0] = new CEditPoly( newBrushes[0] );
			newPolies[1] = new CEditPoly( newBrushes[1] );

			pRoot->m_Plane.SplitPoly( pPoly, newPolies, newBrushes[0]->m_Points, newBrushes[1]->m_Points );

			ASSERT( newPolies[FrontSide]->IsValid() );
			ASSERT( newPolies[BackSide]->IsValid() );

			newBrushes[0]->m_Polies.Append( (CEditPoly*)newPolies[0] );
			newBrushes[1]->m_Polies.Append( (CEditPoly*)newPolies[1] );
		}
		else
		{
			pNewPoly = new CEditPoly(pPoly);
			pNewPoly->m_pBrush = newBrushes[side];
			pNewPoly->m_Indices.Term();

			for( k=0; k < pPoly->NumVerts(); k++ )
			{
				newBrushes[side]->m_Points.Append( pPoly->Pt(k) );
				pNewPoly->m_Indices.Append( newBrushes[side]->m_Points.LastI() );
			}

			newBrushes[side]->m_Polies.Append( pNewPoly );

			ASSERT( pNewPoly->IsValid() );
		}
	}


	if (!pToSplit->IsOpen())
	{
		// If it was split, add a 'cap' polygon from pRoot.
		if( (newBrushes[0]->m_Polies > 0) && (newBrushes[1]->m_Polies > 0) )
		{
			pCap = MakeCap( pRoot, pToSplit, newBrushes[BackSide] );
			newBrushes[BackSide]->m_Polies.Append( pCap );

			pCap = MakeCap( pRoot, pToSplit, newBrushes[FrontSide] );
			ASSERT( pCap->IsValid() );

			pCap->Flip();
			ASSERT( pCap->IsValid() );

			newBrushes[FrontSide]->m_Polies.Append( pCap );
			newBrushes[FrontSide]->RemoveUnusedVerts();

			// And join the new polygons to the old polygons
			newBrushes[0]->RemoveDuplicatePoints(0.01f);
			newBrushes[1]->RemoveDuplicatePoints(0.01f);
		}
	}
	pRegion->UpdateBrushGeometry(newBrushes[0]);
	pRegion->UpdateBrushGeometry(newBrushes[1]);
}

void CEditBrush::RemoveDuplicatePoints( CReal distThresh )
{
	// Jump out on an empty list
	if (!m_Points.GetSize())
		return;

	// Square the distance threshold to avoid sqrts
	distThresh *= distThresh;

	uint32 iCurrPoint;
	for (iCurrPoint = 0; iCurrPoint < m_Points.GetSize() - 1; iCurrPoint++)
	{
		uint32 iOtherPoint;
		for (iOtherPoint = iCurrPoint + 1; iOtherPoint < m_Points.GetSize(); iOtherPoint++)
		{
			CVector cDistVect(m_Points[iOtherPoint] - m_Points[iCurrPoint]);
			// Don't join a vertex if it's further away from the main vertex
			if (cDistVect.Dot(cDistVect) > distThresh)
				continue;

			// Scan through the polygons looking for this vertex
			uint32 iPolyLoop;
			for (iPolyLoop = 0; iPolyLoop < m_Polies.GetSize(); iPolyLoop++)
			{
				CEditPoly *pPoly = m_Polies[iPolyLoop];
				uint32 iIndexLoop;
				for (iIndexLoop = 0; iIndexLoop < pPoly->m_Indices.GetSize(); iIndexLoop++)
				{
					// Replace duplicate polies with the correct index
					if (pPoly->m_Indices[iIndexLoop] == iOtherPoint)
						pPoly->m_Indices[iIndexLoop] = iCurrPoint;
				}
				pPoly->DecrementPoints(iOtherPoint);
			}

			// Remove the point from the list
			m_Points.Remove(iOtherPoint);
			iOtherPoint--;
		}
	}
}

#ifdef DIRECTEDITOR_BUILD
// Notification that a property of this object has changed
void CEditBrush::OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers )
{
	CWorldNode::OnPropertyChanged(pProperty, bNotifyGame, pModifiers);
}

#endif // DIRECTEDITOR_BUILD

