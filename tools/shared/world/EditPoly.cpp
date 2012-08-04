//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditPoly.cpp
//
//	PURPOSE	  : Defines the CEditPoly class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editpoly.h"
#include "geomroutines.h"
#include "editbrush.h"
#include "de_world.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "uvtoopq.h"
#include "PolyLightMap.h"


#ifdef DIRECTEDITOR_BUILD
	#include "texture.h"
	#include "dedit.h"
#endif



// The main texture alignment planes.
CVector g_TexturePlanes[] =
{
	CVector(0.0f, 1.0f, 0.0f),		// Bottom
	CVector(0.0f, -1.0f, 0.0f),		// Top

	CVector(1.0f, 0.0f, 0.0f),		// East-facing wall
	CVector(-1.0f, 0.0f, 0.0f),		// West-facing wall
	CVector(0.0f, 0.0f, 1.0f),		// North-facing wall
	CVector(0.0f, 0.0f, -1.0f)		// South-facing wall
};

CVector g_RightVectors[] =
{
	CVector(1.0f, 0.0f, 0.0f),
	CVector(-1.0f, 0.0f, 0.0f),

	CVector(0.0f, 0.0f, 1.0f),	
	CVector(0.0f, 0.0f, -1.0f),
	CVector(-1.0f, 0.0f, 0.0f),
	CVector(1.0f, 0.0f, 0.0f),	
};

#define NUM_TEXTURE_PLANES	(sizeof(g_TexturePlanes) / sizeof(CVector))

inline void CEditPoly::CommonConstructor( )
{
#ifndef DIRECTEDITOR_BUILD
	m_pUser1				= NULL;
#endif

	m_pBrush				= NULL;
}


CEditPoly::CEditPoly()
{
	CommonConstructor( );
}


CEditPoly::CEditPoly( CEditPoly *pCopyFrom )
{
	CommonConstructor( );
	CopyEditPoly( pCopyFrom );
}


CEditPoly::CEditPoly( CEditBrush *pBrush )
{
	CommonConstructor( );
	m_pBrush = pBrush;
}


CEditPoly::~CEditPoly()
{
	Term();
}


void CEditPoly::Term()
{
	m_Indices.SetSize(0);
}

bool CEditPoly::LoadEditPolyTBW( CAbstractIO& InFile, CStringHolder *pStringHolder )
{
	if (!LoadBasePolyTBW(InFile) )
	{
		return false;
	}

	//load in the texture information
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
	{
		//now save out this texture
		GetTexture(nCurrTex).LoadTextureInfoTBW(InFile, pStringHolder);
	}

	return true;
}

void CEditPoly::SaveEditPolyTBW( CAbstractIO& OutFile )
{
	SaveBasePolyTBW( OutFile );
		
	//now save out each additional texture
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
	{
		//now save out this texture
		GetTexture(nCurrTex).SaveTextureInfoTBW(OutFile);
	}
}


BOOL CEditPoly::LoadEditPolyLTA( CLTANode* pNode, CStringHolder *pStringHolder )
{
	BOOL retval = FALSE;
	if ( LoadBasePolyLTA(pNode) )
	{	
		//Load in the information for the first texture
		CLTANode* pTextureInfo	= pNode->GetElement(4); // shallow_find_list(pNode, "textureinfo");
		GetTexture(0).LoadTextureInfoLTA(pTextureInfo, pStringHolder);

		//Node 5 is reserved for the flags
		//Node 6 is reserved for the shade
		//Node 7 is reserved for the physics information
		//Node 8 is reserved for the surface key

		//initialize all of our surfaces jsut in case some aren't specified
		for(uint32 nCurrTex = 1; nCurrTex < NUM_TEXTURES; nCurrTex++)
		{
			SetupBaseTextureSpace(nCurrTex);
		}

		//Node 9 is all of our additional textures, load them up
		if(pNode->GetNumElements() >= 10)
		{
			//we have additional textures
			CLTANode* pAdditional = pNode->GetElement(9);

			//sanity checks
			assert(pAdditional->IsList());
			assert(pAdditional->GetNumElements() >= 2);
			assert(pAdditional->GetElement(0)->IsAtom());
			assert(pAdditional->GetElement(1)->IsList());
			assert(strcmp(pAdditional->GetElement(0)->GetValue(), "textures") == 0);

			//ok, so now we need to load in each texture
			CLTANode* pTexList = pAdditional->GetElement(1);

			for(uint32 nCurrTex = 0; nCurrTex < pTexList->GetNumElements(); nCurrTex++)
			{
				CLTANode* pTexture = pTexList->GetElement(nCurrTex);

				//read in the index of this texture
				uint32 nTex = atoi(pTexture->GetElement(0)->GetValue());

				//only proceed if it is a valid index
				if(nTex < NUM_TEXTURES)
				{
					GetTexture(nTex).LoadTextureInfoLTA(pTexture->GetElement(1), pStringHolder);
				}
			}
		}

		retval=TRUE;
	}
	return retval;
}

void CEditPoly::SaveEditPolyLTA( CLTAFile* pFile, uint32 level )
{
	PrependTabs(pFile, level);
	pFile->WriteStr("( editpoly " );
		SaveBasePolyLTA( pFile, level+1 );
		
		//save out the first texture
		GetTexture(0).SaveTextureInfoLTA( pFile, level+1 );
		GetTexture(0).SaveFlagsLTA( pFile, level + 1);
		GetTexture(0).SaveShadeLTA( pFile, level + 1);
		GetTexture(0).SavePhysicsMaterialLTA(pFile, level + 1);

		//write out the surface key
		PrependTabs(pFile, level + 1);
		pFile->WriteStr("( surfacekey \"\" )");

		//write out auxilery textures
		PrependTabs(pFile, level + 1);
		pFile->WriteStr("( textures ( ");
		
		//now save out each additional texture
		for(uint32 nCurrTex = 1; nCurrTex < NUM_TEXTURES; nCurrTex++)
		{
			//save out this texture...
			PrependTabs(pFile, level + 2);
			pFile->WriteStrF("( %d ", nCurrTex);

			//now save out this texture
			GetTexture(nCurrTex).SaveTextureInfoLTA(pFile, level + 3);

			//close off the texture tab
			PrependTabs(pFile, level + 2);
			pFile->WriteStr(") ");
		}

		//close off the texture list
		PrependTabs(pFile, level + 1);
		pFile->WriteStr(") ) ");
	
	PrependTabs(pFile, level);
	pFile->WriteStr(") " );
}


void CEditPoly::CopyAttributes( CBasePoly *pBase, CStringHolder *pStringHolder )
{
	CEditPoly	*pOther = (CEditPoly*)pBase;

	m_Plane = pOther->m_Plane;
	
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
		GetTexture(nCurrTex).CopyTextureAttributes(&pOther->GetTexture(nCurrTex), pStringHolder);

}


void CEditPoly::DecrementPoints( uint32 index )
{
	CBasePoly::DecrementPoints(index);
}


BOOL CEditPoly::IntersectRay( CEditRay &ray, CReal &t, BOOL bBackface )
{
	ASSERT(NumVerts() > 1);

	CReal dot1, dot2, div;
	LTVector testPt;

	// Trivial reject.
	if( (Normal().Dot(ray.m_Pos) - Dist()) >= 0.0f )
	{
		// It's pointing away from (or is perpendicular to) the polygon.
		if( Normal().Dot(ray.m_Dir) >= -0.001f )
			return FALSE;
	}
	else
	{
		// Backfacing.
		if( !bBackface )
		{
			return FALSE;
		}
		else
		{
			// It's pointing away from (or is perpendicular to) the polygon.
			if( Normal().Dot(ray.m_Dir) <= 0.001f )
				return FALSE;
		}
	}

	dot1 = Normal().Dot( ray.m_Pos ) - Dist();
	dot2 = Normal().Dot( ray.m_Pos + ray.m_Dir ) - Dist();

	div = dot2 - dot1;
	if(fabs(div) < 0.00001f)
		t = 0.0f;
	else
		t = -dot1 / div;
	
	ASSERT( t >= 0 );

	testPt = ray.m_Pos + (ray.m_Dir * t);
	
	// See if the intersection point lies inside the poly.
	uint32 nFanPt  = NumVerts() - 1;
	uint32 nPrevPt = 0;
	uint32 nCurrPt = 1;

	//the count of the number of triangles it is in
	uint32 nInCount = 0;

	for(; nCurrPt < NumVerts(); nPrevPt = nCurrPt, nCurrPt++)
	{
		//see if it the point is within the triangle defined by 
		//the edges fanpt->prev, fanpt->curr
		if( (g_TriArea2( Normal(), Pt(nFanPt),  Pt(nPrevPt), testPt ) > 0) ||
			(g_TriArea2( Normal(), Pt(nPrevPt), Pt(nCurrPt), testPt ) > 0) ||
			(g_TriArea2( Normal(), Pt(nCurrPt), Pt(nFanPt),  testPt ) > 0))
			continue;

		//we are in this tri
		nInCount++;
	}

	return (nInCount % 2) ? TRUE : FALSE;
}


BOOL CEditPoly::CopyEditPoly( CEditPoly *pPoly, BOOL bCopyIndices, CStringHolder *pStringHolder )
{
	if( bCopyIndices )
		m_Indices.CopyArray( pPoly->m_Indices );

	CopyAttributes( pPoly, pStringHolder );
	m_pBrush = pPoly->m_pBrush;
	
	return TRUE;
}

void CEditPoly::Flip()
{
	uint32		i, temp;

	for( i=0; i < m_Indices/2; i++ )
	{
		temp = m_Indices[i];
		m_Indices[i] = m_Indices[m_Indices-i-1];
		m_Indices[m_Indices-i-1] = temp;
	}

	Normal() = -Normal();
	Dist() = -Dist();
}


void CEditPoly::SetupBaseTextureSpace(uint32 nTex)
{
	uint32 i, closestPlane;
	CReal dot, maxDot;


	// Find the closest texture alignment plane to this poly.
	maxDot = (CReal)-MAX_CREAL;

	closestPlane = 0;
	for( i=0; i < NUM_TEXTURE_PLANES; i++ )
	{
		dot = Normal().Dot( g_TexturePlanes[i] );
		if( dot > maxDot )
		{
			maxDot = dot;
			closestPlane = i;
		}
	}

	// SCHLEGZ 1/22/98: fixed problem with deleting previous mag
	// MD 3/25/98: Polies now use explicit scale variables.
	GetTexture(nTex).SetP(g_RightVectors[closestPlane]);
	GetTexture(nTex).SetQ(GetTexture(nTex).GetP().Cross( g_TexturePlanes[closestPlane] ));
}

void CEditPoly::SetTextureSpace(uint32 nTex, const LTVector& newO, const LTVector& newP, const LTVector& newQ)
{
	GetTexture(nTex).SetTextureSpace(GetTextureNormal(), newO, newP, newQ);
}

// sets up OPQs based on uv coordinates for each vertex
// takes an array of 6 floats, 2 per vertex for the first 3 verts plus the width and height of the texture
bool CEditPoly::SetUVTextureSpace( uint32 nTex, const float* coords, const int texWidth, const int texHeight )
{
	if ( this->m_pBrush->m_Points == NULL || this->m_pBrush->m_Points.GetSize() < 3)  
		return false; 

	//build up a vertex list
	ASSERT(this->NumVerts() >= 3);

	LTVector vPos[3];
	vPos[0] = m_pBrush->m_Points[Index(0)];
	vPos[1] = m_pBrush->m_Points[Index(1)];
	vPos[2] = m_pBrush->m_Points[Index(2)];

	//now get the OPQ vectors
	CTexturedPlane& Texture = GetTexture(nTex);

	LTVector vO, vP, vQ;
	bool bRV = ConvertUVToOPQ(vPos, coords, texWidth, texHeight, vO, vP, vQ);	

	Texture.SetO(vO);
	Texture.SetP(vP);
	Texture.SetQ(vQ);

	return bRV;
}

//determines if this polygon is concave or not
bool CEditPoly::IsConcave()
{
	uint32 nNumPts = NumVerts();

	if(nNumPts <= 3)
	{
		return false;
	}

	//get the normal for this polygon
	LTVector vNormal = Normal();

	//now go through every edge
	uint32 nPrevPt = nNumPts - 1;
	for(uint32 nCurrPt = 0; nCurrPt < nNumPts; nPrevPt = nCurrPt, nCurrPt++)
	{
		//build the edge normal
		LTVector vEdge = m_pBrush->m_Points[Index(nCurrPt)] - m_pBrush->m_Points[Index(nPrevPt)];

		//find the normal
		LTVector vEdgeNorm = vNormal.Cross(vEdge);
		vEdgeNorm.Norm();

		//now run through all the other points
		for(uint32 nTestPt = 0; nTestPt < nNumPts; nTestPt++)
		{
			//ignore the points on the edge
			if((nTestPt == nCurrPt) || (nTestPt == nPrevPt))
				continue;

			//see if it is on the correct side
			if(vEdgeNorm.Dot(m_pBrush->m_Points[Index(nTestPt)] - m_pBrush->m_Points[Index(nCurrPt)]) > 0.001f)
			{
				return true;
			}
		}
	}

	return false;

}

//determines if this polygon lies entirely within a single plane
bool CEditPoly::IsCoplanar()
{
	//run through all the points, projecting them onto the normal. The
	//value should be relatively the same for all the points
	if(NumVerts() < 4)
	{
		//if we have 3 or less points, we mathmatically have to be coplanar, so just bail to
		//prevent mathmatical errors from causing problems
		return true;
	}

	//the epsilon value that we allow points to stray from the plane
	static const float kfPlaneEpsilon = 0.01f;

	LTVector vNormal = Normal();

	CReal fDist = m_pBrush->m_Points[Index(0)].Dot(Normal());

	for(uint32 nCurrPt = 1; nCurrPt < NumVerts(); nCurrPt++)
	{
		CReal fTestDist = m_pBrush->m_Points[Index(nCurrPt)].Dot(Normal());

		fTestDist -= fDist;

		if((fTestDist < -kfPlaneEpsilon) || (fTestDist > kfPlaneEpsilon))
		{
			return false;
		}
	}

	return true;
}

//determines the surface area of the polygon
CReal CEditPoly::GetSurfaceArea()
{
	//accumulate the area occupied by each triangle
	CReal fArea = 0.0f;

	uint32 nNumPts = NumVerts();

	//safety first...
	if(nNumPts < 3)
		return 0.0f;

	//fan out from vertex 0 to all other verts
	LTVector vFanPt = m_pBrush->m_Points[Index(0)];

	for(uint32 nCurrPt = 1; nCurrPt < nNumPts - 1; nCurrPt++)
	{
		//get the two edges
		LTVector vEdge1 = m_pBrush->m_Points[Index(nCurrPt)] - vFanPt;
		LTVector vEdge2 = m_pBrush->m_Points[Index(nCurrPt + 1)] - vFanPt;

		//the cross product gives us the area of the rect, so we want to add
		//half of that
		fArea += vEdge1.Cross(vEdge2).Mag() / 2;
	}

	return fArea;
}

