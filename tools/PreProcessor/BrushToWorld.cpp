// ------------------------------------------------------------------ //
//	FILE	  : BrushToWorld.cpp
//	CREATED	  : February 5, 1997
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
// ------------------------------------------------------------------ //

// Includes....
#include "bdefs.h"
#include "preworld.h"
#include "brushtoworld.h"
#include "processing.h"
#include "SetupPolyAlpha.h"

//utility class that will manage a list of references from vertex to polygon and aid in calculating
//vertex normals
class CVertNormalGenerator
{
public:

	CVertNormalGenerator() :
		m_pIndices(NULL),
		m_ppPolyRefs(NULL)
	{
	}

	~CVertNormalGenerator()
	{
		Term();
	}

	void Term()
	{
		delete [] m_pIndices;
		m_pIndices = NULL;

		delete [] m_ppPolyRefs;
		m_ppPolyRefs = NULL;
	}


	LTVector CalculateVertNormal(const LTVector& vPolyNormal, float fCosCrease, uint32 nVert) const
	{
		//alright, so find our starting and ending indices
		uint32 nStart	= m_pIndices[nVert];
		uint32 nEnd		= m_pIndices[nVert + 1];

		//reset the normal
		LTVector vRV(0.0f, 0.0f, 0.0f);

		for(uint32 nCurrRef = nStart; nCurrRef < nEnd; nCurrRef++)
		{
			CEditPoly* pPoly = m_ppPolyRefs[nCurrRef];

			//see if this polygon is at a crease angle
			if(vPolyNormal.Dot(pPoly->Normal()) < fCosCrease)
				continue;

			//it should be factored into the normal
			vRV += pPoly->Normal();
		}

		vRV.Normalize();
		return vRV;
	}

	bool BuildLists(CEditBrush* pEditBrush);

private:
	
	uint32*			m_pIndices;
	CEditPoly**		m_ppPolyRefs;
};


bool CVertNormalGenerator::BuildLists(CEditBrush* pEditBrush)
{
	//this is how the algorithm works:
	// Generate the number of references to each point
	// Convert that into a list of indices into a big list of references
	// Fill in the list of references to each brush
	//
	// Then all you need to do to find touching vertices is to use the index list to
	// index into the list of references and you can find all polygons that refer to
	// that point
	//
	// One nuance is that the index list needs to have an extra element indicating the end
	// so that it can always use the next value as the end of the span for the vertex refs

	//cache some data about our brush
	uint32 nNumPoints = pEditBrush->m_Points.GetSize();
	uint32 nNumPolys  = pEditBrush->m_Polies.GetSize();

	//build up a count for each reference to each vertex
	m_pIndices = new uint32 [nNumPoints + 1];

	if(!m_pIndices)
	{
		Term();
		return false;
	}

	//clear it all out
	memset(m_pIndices, 0, sizeof(uint32) * (nNumPoints + 1));

	//and now add our references
	uint32 nTotalRefs = 0;
	uint32 nCurrPoly;

	for(nCurrPoly = 0; nCurrPoly < nNumPolys; nCurrPoly++)
	{
		CEditPoly* pPoly = pEditBrush->m_Polies[nCurrPoly];
		for(uint32 nCurrIndex = 0; nCurrIndex < pPoly->m_Indices.GetSize(); nCurrIndex++)
		{
			m_pIndices[pPoly->m_Indices[nCurrIndex]]++;
		}
		nTotalRefs += pPoly->m_Indices.GetSize();
	}

	//and now convert our reference list into an index list
	uint32 nPrevRefIndex = 0;
	for(uint32 nCurrRef = 0; nCurrRef <= nNumPoints; nCurrRef++)
	{
		uint32 nVertRefs = m_pIndices[nCurrRef];
		m_pIndices[nCurrRef] = nPrevRefIndex;
		nPrevRefIndex += nVertRefs;
	}

	//alright, now allocate our reference list
	m_ppPolyRefs = new CEditPoly* [nTotalRefs];

	//verify the allocation
	if(!m_ppPolyRefs)
	{
		Term();
		return false;
	}

	//now null it out
	memset(m_ppPolyRefs, 0, sizeof(CEditPoly*) * nTotalRefs);

	//and fill it in with references
	for(nCurrPoly = 0; nCurrPoly < nNumPolys; nCurrPoly++)
	{
		CEditPoly* pPoly = pEditBrush->m_Polies[nCurrPoly];
		for(uint32 nCurrIndex = 0; nCurrIndex < pPoly->m_Indices.GetSize(); nCurrIndex++)
		{
			//use the index list to find out where we need to put ourselves
			uint32 nPlaceAt = m_pIndices[pPoly->m_Indices[nCurrIndex]];

			//move past any other references that have already been placed
			while(m_ppPolyRefs[nPlaceAt])
				nPlaceAt++;

			//and place ourselves into the list
			m_ppPolyRefs[nPlaceAt] = pPoly;
		}
	}

	//we now have our two lists and can quickly and easily find any polygons that share the same
	//vertex

	//success
	return true;
}


//given a brush name it will generate a hashed name
uint32 GenerateBrushPolygonName(const char *pName)
{
	if (!pName)
		return 0;

	uint32 nResult = 0;
	for (; *pName; ++pName)
	{
		nResult *= 29;
		nResult += toupper(*pName) - 'A';
	}

	return nResult;
}

// Given an edit polygon and a starting point that should be initialized to zero
//and continually passed into this funciton it will generate a list of pre polys
//that can be used in the main world
static CPrePoly* EditPolySectionToPrePoly(CEditPoly* pEditPoly, uint32& nStartVert, uint32 nBrushName, CEditBrush* pBrush, 
										  const CVertNormalGenerator& VertNormGen, float fCosCreaseAngle)
{
	//maximum number of vertices to allow per fragment
	const uint32 k_nMaxNumVerts = 30;

	//see if we are done
	if(nStartVert >= pEditPoly->NumVerts())
	{
		//we have completed fragmenting this polygon, no more fragments to generate
		return NULL;
	}

	//determine the maximum number of vertices we can add only K - 2 vertices since
	//we have to replicate two previous vertices
	uint32 nMaxVertsToAdd = (nStartVert > 0) ? k_nMaxNumVerts - 2 : k_nMaxNumVerts;

	//determine what our maximum ending vertex is
	uint32 nEndVert = LTMIN(nStartVert + nMaxVertsToAdd, pEditPoly->NumVerts());
	
	//determine how many vertices that translates to (again considering that if this is not
	//the first fragment that we need to add two extra verts)
	uint32 nVertCount = nEndVert - nStartVert;
	if (nStartVert > 0)
		nVertCount += 2;

	//create our polygon
	CPrePoly *pPrePoly = CreatePoly(CPrePoly, nVertCount, false);

	//setup surface properties, this must be done here otherwise the polygon doesn't have a surface
	//and therefore doesn't have a plane
	pPrePoly->m_nName = nBrushName;
	pPrePoly->m_pSurface = (CPreSurface*)pEditPoly->m_pUser1;

	// If this is an overflow poly, we need the base point and the last point from the previous poly
	if (nStartVert > 0)
	{
		pPrePoly->AddVert( pEditPoly->Pt(0), VertNormGen.CalculateVertNormal(pEditPoly->Normal(), fCosCreaseAngle, pEditPoly->m_Indices[0]) );
		pPrePoly->AddVert( pEditPoly->Pt(nStartVert - 1), VertNormGen.CalculateVertNormal(pEditPoly->Normal(), fCosCreaseAngle, pEditPoly->m_Indices[nStartVert - 1]) );
	}

	//now copy over all the vertices to the pre poly
	for(uint32 nCurrVert = nStartVert; nCurrVert < nEndVert; nCurrVert++)
	{
		pPrePoly->AddVert( pEditPoly->Pt(nCurrVert), VertNormGen.CalculateVertNormal(pEditPoly->Normal(), fCosCreaseAngle, pEditPoly->m_Indices[nCurrVert]) );
	}

	//and setup the colors of the polygon
	CopyPolyColors(pEditPoly, pPrePoly, nStartVert, nEndVert);
	SetupPolyAlpha(pBrush, pPrePoly, nVertCount);

	// Move to the end of this fragment so that next time we will generate the following fragment
	nStartVert = nEndVert;

	return pPrePoly;
}

//given a brush, it will determine the crease angle and return the cosine of that angle
static float GetCosCreaseAngle(CEditBrush* pEditBrush)
{
	//determine the crease angle for this brush
	float fCosCreaseAngle = 0.707f;	//cos(45)

	//find the crease angle property
	CRealProp *pCreaseProp = (CRealProp *)pEditBrush->m_PropList.GetMatchingProp("CreaseAngle", PT_REAL);
	if (pCreaseProp)
	{
		//found a match, convert it to a value
		fCosCreaseAngle = LTMAX(0.0f, cosf(MATH_DEGREES_TO_RADIANS(pCreaseProp->m_Value)));
	}

	return fCosCreaseAngle;
}

static void TransferBrushPolys(CEditBrush *pEditBrush, CPreWorld *pWorld)
{
	//sanity check
	assert(pEditBrush && pWorld);

	//build up our vertex normal generate;
	CVertNormalGenerator	VertNormGen;
	if(!VertNormGen.BuildLists(pEditBrush))
	{
		DrawStatusText(eST_Error, "Unable to initialize normal generator for brush %s", pEditBrush->GetName());
		return;
	}

	//determine the crease angle for this brush
	float fCosCreaseAngle = GetCosCreaseAngle(pEditBrush);

	//determine the brush name and cache this to avoid having to calculate it for every polygon
	uint32 nBrushName = GenerateBrushPolygonName(pEditBrush->GetName());

	//the number of polygons in the brush
	uint32 nNumBrushPolys = pEditBrush->m_Polies.GetSize();

	//we need to run through every polygon in the brush and generate prepoly fragments
	for(uint32 nCurrPoly = 0; nCurrPoly < nNumBrushPolys; nCurrPoly++)
	{
		//now we need to generate the fragments
		uint32 nStartVert = 0;

		CPrePoly* pOrigPoly;
		while((pOrigPoly = EditPolySectionToPrePoly(pEditBrush->m_Polies[nCurrPoly], nStartVert, nBrushName, pEditBrush, VertNormGen, fCosCreaseAngle)) != NULL)
		{
			//we have a valid fragment so we need to add this to our list of original polygons
			pWorld->m_OriginalBrushPolies.Append(pOrigPoly);
			
			//alright, now that we have done that, we of course to be efficient (ugh) need to make
			//a clone and add that to the world polys.
			CPrePoly* pWorldPoly = pOrigPoly->Clone();

			//setup the clone polygon to point to its original polygon so it can be restored after
			//BSP generation
			pWorldPoly->m_pOriginalBrushPoly = pOrigPoly;

			//add this new polygon into our list of polygons
			pWorld->m_Polies.Append(pWorldPoly);
		}		
	}
}


CPrePoly* EditPolyToPrePoly(CEditPoly *pEditPoly)
{
	CPrePoly *pPrePoly = CreatePoly(CPrePoly, pEditPoly->NumVerts(), false);
	if(!pPrePoly)
		return NULL;

	pPrePoly->m_pSurface = (CPreSurface*)pEditPoly->m_pUser1;

	for(uint32 i=0; i < pEditPoly->NumVerts(); i++)
	{
		pPrePoly->AddVert( pEditPoly->Pt(i) );
	}

	CopyPolyColors(pEditPoly, pPrePoly, 0, pPrePoly->NumVerts());
	SetupPolyAlpha(pEditPoly->m_pBrush, pPrePoly, pPrePoly->NumVerts());

	return pPrePoly;
}


bool BrushToWorld(
	GenList<CEditBrush*> &brushes, 
	CPreWorld *pWorld)
{
	// Create the TempBrushes.
	if (!brushes.GenGetSize())
	{
		DrawStatusText(eST_Error, "Empty world model BSP found (%s). Make sure some brushes are not render only.", pWorld->m_WorldName);

		return pWorld->m_Polies.GetSize() > 0;
	}

	// Move the brushes into the world
	DrawStatusTextIfMainWorld(eST_Normal, "Transferring brush polygons to world");

	clock_t TransferStart = clock();

	uint32		nProcessCount = 0, nProcessTotal;
	nProcessTotal = brushes.GenGetSize();
	for(GenListPos iBrush = brushes.GenBegin(); brushes.GenIsValid(iBrush); )
	{
		TransferBrushPolys(brushes.GenGetNext(iBrush), pWorld);
		SetProgressBar((float)nProcessCount / nProcessTotal);
		++nProcessCount;
	}

	DrawStatusTextIfMainWorld(eST_Normal, "Converting brushes took %.2f seconds", (clock() - TransferStart) / (float)CLOCKS_PER_SEC);

	// Setup all the vertex indices
	pWorld->InitPolyVertexIndices();

	// Set up the LM surface vectors
	pWorld->SetupSurfaceLMVectors();

	return true;
}



