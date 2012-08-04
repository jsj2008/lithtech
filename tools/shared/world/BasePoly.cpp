//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : BasePoly.cpp
//
//	PURPOSE	  : Defines the CBasePoly class.
//
//	CREATED	  : October 15 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "basepoly.h"
#include "geomroutines.h"
#include "editpoly.h"
#include "editregion.h"
#include "editbrush.h"
#include "ltamgr.h"
#include "ltasaveutils.h"



#define BASEPOLY_EPSILON	0.001f


CBasePoly::CBasePoly()
{
	m_pBrush = NULL;
}


CBasePoly::CBasePoly( CEditBrush *pBrush )
{
	m_pBrush = pBrush;
}


CBasePoly::CBasePoly( CEditBrush *pBrush, CMoDWordArray &indices )
{
	m_pBrush = pBrush;
	m_Indices.CopyArray( indices );
}


CBasePoly::CBasePoly( CEditBrush *pBrush, uint32 i0, uint32 i1, uint32 i2, bool bUpdatePlane )
{
	m_pBrush = pBrush;
	m_Indices.SetSize( 3 );
	m_Indices[0] = i0;
	m_Indices[1] = i1;
	m_Indices[2] = i2;

	if( bUpdatePlane )
		UpdatePlane();
}


bool CBasePoly::LoadBasePolyTBW( CAbstractIO& InFile )
{
	//load up all the vertices
	uint32 nNumVerts;
	InFile >> nNumVerts;

	m_Indices.SetSize( nNumVerts );

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		uint32 nIndex;
		InFile >> nIndex;

		Index(nCurrVert) = nIndex;
	}

	//now read in the plane data
	InFile >> Normal().x;
	InFile >> Normal().y;
	InFile >> Normal().z;

	InFile >> Dist();

	return true;
}

void CBasePoly::SaveBasePolyTBW( CAbstractIO& OutFile )
{
	//number of vertices
	uint32 nNumVerts = m_Indices.GetSize();
	OutFile << nNumVerts;

	//all the vertex indices
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		uint32 nIndex = Index(nCurrVert);
		OutFile << nIndex;
	}

	//now write out the normal
	OutFile << Normal().x;
	OutFile << Normal().y;
	OutFile << Normal().z;

	OutFile << Dist();
}


/*
( basepoly  
	( indices ( 6 0 3 7 ) )
	( normal ( 1.000000 0.000000 0.000000 ) )
	( dist 292.000000 )
)
*/

bool CBasePoly::LoadBasePolyLTA( CLTANode* pNode )
{
	// CLTANode* pIndicesNode = pNode->getElem(0); // shallow_find_list(pNode, "indices");
	CLTANode* pIndex = pNode->GetElement(1); //PairCdrNode(pIndicesNode);
	uint32 listSize = pIndex->GetNumElements();

	assert(listSize > 0);
	m_Indices.SetSize( listSize - 1 );
	
	for( uint32 i = 1; i < listSize; i++ )
	{
		Index(i-1) = GetUint32(pIndex->GetElement(i));
		if( Index(i-1) >= m_pBrush->m_Points )
		{
			Index(i-1) = 0;
			return false;
		}
	}	
	

	CLTANode* pNormalNode = pNode->GetElement(2); //shallow_find_list(pNode, "normal");
	if( pNormalNode )
	{
		Normal().x = GetFloat(pNormalNode->GetElement(1));
		Normal().y = GetFloat(pNormalNode->GetElement(2));
		Normal().z = GetFloat(pNormalNode->GetElement(3));
	}
	
	CLTANode* pDistNode = pNode->GetElement(3); //shallow_find_list(pNode, "dist");
	if( pDistNode )
	{
		Dist() = GetFloat(PairCdrNode(pDistNode));
	}
	
	return true;
}


void CBasePoly::SaveBasePolyLTA( CLTAFile* pFile, uint32 level )
{
	uint32		i;

	//write out the list of face indices
	PrependTabs(pFile, level);
	pFile->WriteStr("( f ");
	for( i=0; i < m_Indices; i++ ){
		pFile->WriteStrF("%i ", m_Indices[i] );
	}
	pFile->WriteStr(")");
	
	//write out the surface normal
	PrependTabs(pFile, level);
	pFile->WriteStrF("( n %f %f %f )", Normal().x, Normal().y, Normal().z );
	PrependTabs(pFile, level);
	pFile->WriteStrF("( dist %f )", Dist() );
}

bool CBasePoly::IsValid()
{
	for( uint32 i=0; i < NumVerts(); i++ )
		if( m_Indices[i] >= m_pBrush->m_Points )
			return false;

	return true;
}


void CBasePoly::DecrementPoints( uint32 index )
{
	for( uint32 i=0; i < m_Indices; i++ )
		if( m_Indices[i] >= index )
			--m_Indices[i];
}


bool CBasePoly::PointInPlane( const LTVector &point )
{
	CReal dist = Normal().Dot( point ) - Dist();
	return (dist > -POINT_SIDE_EPSILON) && (dist < POINT_SIDE_EPSILON);
}



bool CBasePoly::PointInPoly( const LTVector &point )
{
	CReal		area;

	for(uint32 i=0; i < NumVerts(); i++ )
	{
		area = g_TriArea2( Normal(), Pt(i), NextPt(i), point );
		if( area > 0 )
			return false;
	}

	return true;
}


void CBasePoly::RemoveCollinearVertices()
{
	int32			i;
	CVector			v1, v2;

	for( i=0; i < (int32)m_Indices; i++ )
	{
		v1 = Pt(i) - PrevPt(i);
		v2 = NextPt(i) - Pt(i);
	
		v1.Norm();
		v2.Norm();
		
		if( v1.NearlyEquals(v2, 0.00001f) )
		{
			m_Indices.Remove(i);
			i = -1;
		}
	}
}


bool CBasePoly::IsConvex()
{
	CVector			v1, v2, v3;
	CVector			vec1, vec2;
	uint32			i;
	CDReal			dot, sign=0.0f;

	
	
	for( i=0; i < NumVerts(); i++ )
	{
		v1 = Pt(i);
		v2 = NextPt(i);
		v3 = NextPt(m_Indices.NextI(i) );
	
		vec1 = v2 - v1;
		vec2 = v3 - v2;

		vec1.Norm();
		vec2.Norm();

		dot = Normal().Dot( vec1.Cross(vec2) );
		if( (sign == 0.0f) && (dot != 0.0f) )
		{
			sign = dot;
		}
		else
		{
			if( sign < 0.0f && dot > 0.0f )
				return false;
			else if( sign > 0.0f && dot < 0.0f )
				return false;
		}
	}

	return true;
}


void CBasePoly::GetCenterPoint( CVector &centerPt )
{
	centerPt.Init( 0.0f, 0.0f, 0.0f );
	for(uint32 i=0; i < NumVerts(); i++ )
		centerPt += Pt(i);

	centerPt /= (CReal)NumVerts();
}


void CBasePoly::Flip()
{
	for(uint32 i=0; i < m_Indices/2; i++ )
	{
		uint32 temp = m_Indices[i];
		m_Indices[i] = m_Indices[m_Indices-i-1];
		m_Indices[m_Indices-i-1] = temp;
	}

	Normal() = -Normal();
	Dist() = -Dist();
}


void CBasePoly::UpdatePlane()
{
	g_GenerateNormal( this );
}


bool CBasePoly::GetTriangles( CMoArray<CBasePoly*> &polies )
{
	uint32			i, i1, i2;
	CReal			area;
	
	RestartLoop:;
	if( m_Indices > 3 )
	{	
		for( i=0; i < m_Indices; i++ )
		{
			i1 = m_Indices.NextI( i );
			i2 = m_Indices.NextI( i1 );

			// Get rid of collinear vertices.
			area = g_TriArea2( Normal(), Pt(i), Pt(i1), Pt(i2) );
			if( (area >= -0.1f) && (area <= 0.1f) )
			{
				m_Indices.Remove( i1 );
				goto RestartLoop;
			}

			if( ValidTriangle(i, i1, i2) )
			{
				polies.Append( new CBasePoly(m_pBrush, m_Indices[i], m_Indices[i1], m_Indices[i2], true) );

				m_Indices.Remove( i1 );
				goto RestartLoop;
			}
		}
	}
	
	if( m_Indices == 3 )
	{
		area = g_TriArea2( Normal(), Pt(0), Pt(1), Pt(2) );
		if( area <= -0.1f )
			polies.Append( new CBasePoly(m_pBrush, m_Indices[0], m_Indices[1], m_Indices[2], true) );
	}
	
	return true;
}


/*
	Returns True if ab and cd intersect.
*/

bool CBasePoly::Intersect( const LTVector &a, const LTVector &b, const LTVector &c, const LTVector &d )
{
	CPlane		plane;
	bool		bIntersect;

	
	plane.m_Normal = Normal().Cross( b - a );
	plane.m_Dist = plane.m_Normal.Dot( a );
	bIntersect = ((plane.m_Normal.Dot(c)-plane.m_Dist)>0.0f) != ((plane.m_Normal.Dot(d)-plane.m_Dist)>0.0f);

	if( bIntersect )
	{
		plane.m_Normal = Normal().Cross( d - c );
		plane.m_Dist = plane.m_Normal.Dot( c );
		bIntersect = ((plane.m_Normal.Dot(a)-plane.m_Dist)>0.0f) != ((plane.m_Normal.Dot(b)-plane.m_Dist)>0.0f);

		return bIntersect;
	}
	else
		return false;	
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CBasePoly::Diagonalie
//		PURPOSE:		Returns True if line ab is a proper (internal OR external)
//						diagonal of poly, or |a-b|=2 and a and b are collinear
//						with the middle vertex; only checks nonblocked condition.
// ----------------------------------------------------------------------- //

bool CBasePoly::Diagonalie( uint32 a, uint32 b )
{
	uint32		k, k1;

	for( k=0; k < m_Indices; k++ )
	{
		k1 = m_Indices.NextI( k );

		// Skip edges incident to a or b.
		if( !( (k==a) || (k1==a) || (k==b) || (k1==b) ) )
		{
			if( Intersect(Pt(a), Pt(b), Pt(k), Pt(k1)) )
				return false;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CBasePoly::LeftOn
//		PURPOSE:		Tells if C is left of or on line AB.
// ----------------------------------------------------------------------- //

bool CBasePoly::LeftOn( uint32 a, uint32 b, uint32 c )
{
	CReal area = g_TriArea2( Normal(), Pt(a), Pt(b), Pt(c) );
	return (area <= 0.0f);
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CBasePoly::Left
//		PURPOSE:		Tells if C is left of line AB.
// ----------------------------------------------------------------------- //

bool CBasePoly::Left( uint32 a, uint32 b, uint32 c )
{
	CReal area = g_TriArea2( Normal(), Pt(a), Pt(b), Pt(c) );
	return (area < 0.0f);
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CBasePoly::InsideDiagonal
//		PURPOSE:		Tells if the diagonal (a,b) lies inside the polygon
//                      in the vicinity of a.
// ----------------------------------------------------------------------- //

bool CBasePoly::InsideDiagonal( uint32 a, uint32 b )
{
    uint32	a_plus_1, a_minus_1;

    a_plus_1 = m_Indices.NextI( a );
    a_minus_1 = m_Indices.PrevI( a );

    if( LeftOn(a_minus_1, a, a_plus_1) )
    {
		return Left(a, b, a_minus_1) && Left(b, a, a_plus_1);
    }
    else
    {
        return !( LeftOn(a, b, a_plus_1) && LeftOn(b, a, a_minus_1) );
    }
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CBasePoly::ValidTriangle
//		PURPOSE:		Tells if (a,b,c) is a valid triangle on this poly.
// ----------------------------------------------------------------------- //

bool CBasePoly::ValidTriangle( uint32 a, uint32 b, uint32 c )
{
	if( InsideDiagonal(a, c) )
	{
		return Diagonalie( a, c );
	}
	else
		return false;
}




