#include "bdefs.h"
#include "editregion.h"
#include "proputils.h"
#include "processing.h"
#include "editpoly.h"
#include "node_ops.h"
#include "texturedims.h"

//maximum vertices that a polygon can have...
#define MAX_VERTS		256

typedef CMoArray<CEditBrush*>	CBrushArray;

//structure to hold all the information necessary for edge generation
class CEdgeGenInfo
{
public:

	//dimensions of the texture being used
	uint32			m_nTexWidth;
	uint32			m_nTexHeight;

	//the name of the texture
	const char*		m_pszTexture;

	//the name of the texture effect
	const char*		m_pszTexEffect;

	//the node that the edges should be placed beneath
	CWorldNode*		m_pPlaceUnder;

	//the thickness of the edges
	float			m_fThickness;

	//the cosine of the crease angle
	float			m_fCosCrease;

	//should we adjust the texture coordinates on the edges to prevent wrapping when
	//bilinear filtering
	bool			m_bAdjustEdgeUVs;

	//should we shrink the source polygon? Good for if there is no blending, reduces
	//overdraw
	bool			m_bShrinkSourcePoly;

	//the amount to extrude the edges from their source positions
	float			m_fExtrude;
};

//determines if the specified polygon is small enough that when edged, it will only
//consist of edges and none of the source polygon will be visible
static bool IsPolyOnlyEdges(CEditPoly* pPoly, float fThickness)
{
	//run through all of the edges, and for each one, if all the points are less than
	//2 * thickness away, then it will consist entirely of edges
	for(uint32 nCurrPt = 0; nCurrPt < pPoly->NumVerts(); nCurrPt++)
	{
		//get the edge normal
		LTVector vToNext = pPoly->Pt(nCurrPt) - pPoly->Pt((nCurrPt + 1) % pPoly->NumVerts());
		LTVector vEdgeNormal = pPoly->Normal().Cross(vToNext);

		vEdgeNormal.Norm();

		//get the planar distance
		float fPlaneDist = vEdgeNormal.Dot(pPoly->Pt(nCurrPt));

		//ok, now see if any of the points are in front
		bool bInFront = false;

		for(uint32 nTestPt = 0; nTestPt < pPoly->NumVerts(); nTestPt++)
		{
			if((pPoly->Pt(nTestPt).Dot(vEdgeNormal) - fPlaneDist) > 2 * fThickness)
			{
				//we have one in front
				bInFront = true;
				break;
			}
		}

		//if none were in front, we need to indicate it was entirely edges
		if(!bInFront)
			return true;
	}

	//all edges had some in front, it is not an edge only poly
	return false;
}

//handles converting all the decal polygons to actual brushes
static CEditBrush* CreateBrush(	CEditPoly* pSrcPoly, CEditRegion* pRegion, 
								uint32 nNumVerts, LTVector *pVerts,
								const char* pszTexture, const LTVector& vO, 
								const LTVector& vP, const LTVector& vQ,
								CWorldNode* pParent)
{
	//only bother with valid polygons
	if((nNumVerts < 3) || (pSrcPoly == NULL))
		return NULL;

	//first off create the new brush
	CEditBrush* pNewBrush = new CEditBrush;

	if(pNewBrush == NULL)
		return NULL;

	//now add the polygon to the brush
	CEditPoly* pNewPoly = new CEditPoly(pNewBrush);

	if(pNewPoly == NULL)
	{
		//out of memory
		delete pNewBrush;
		return NULL;
	}

	//add the vertices to the brush
	pNewBrush->m_Points.SetSize(nNumVerts);
	pNewPoly->m_Indices.SetSize(nNumVerts);

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		pNewBrush->m_Points[nCurrVert] = pVerts[nCurrVert];
		pNewPoly->m_Indices[nCurrVert] = nCurrVert;
	}

	//add the polygon to the brush list
	pNewBrush->m_Polies.SetSize(1);
	pNewBrush->m_Polies[0] = pNewPoly;

	//copy over the texture information
	pNewPoly->GetTexture(0).m_pTextureName = pRegion->m_pStringHolder->AddString(pszTexture);

	//setup the OPQ's for this brush
	pNewPoly->SetTextureSpace(0, vO, vP, vQ);

	//have the brush update its info
	pNewBrush->UpdatePlanes();
	pNewBrush->UpdateBoundingInfo();

	//now we need to copy the properties from the source brush into the new brush
	pNewBrush->m_PropList.CopyValues(pSrcPoly->m_pBrush->GetPropertyList());

	//override special properties
	CBaseProp* pProp = pNewBrush->m_PropList.GetProp("ClipLight");
	if(pProp && (pProp->GetType() == PT_BOOL))
		((CBoolProp*)pProp)->m_Value = FALSE;

	//now we add this brush underneath the terrain object
	no_AttachNode(pRegion, pNewBrush, pParent);

	//success
	return pNewBrush;
}

//calculates the OPQ's for a surface given a starting orientation, texture info
static bool CalculateOPQ(uint32 nNumVerts, const LTVector* pVerts, const CEdgeGenInfo& Info, 
						 LTVector& vO, LTVector& vP, LTVector& vQ)
{
	//run through the vertices and ensure that the O and P cover them all
	float fPMag = 0.0f;
	float fQMag = 0.0f;

	//make sure that they are unit length
	vP.Normalize();
	vQ.Normalize();

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		float fDot = vP.Dot(pVerts[nCurrVert] - vO);
		if(fabs(fDot) > fPMag)
			fPMag = fDot;

		fDot = vQ.Dot(pVerts[nCurrVert] - vO);
		if(fabs(fDot) > fQMag)
			fQMag = fDot;
	}

	if(!Info.m_bAdjustEdgeUVs)
	{
		//alright, now find our final scale
		vP *= Info.m_nTexWidth / fPMag;
		vQ *= Info.m_nTexHeight / fQMag;
	}
	else
	{
		static const uint32 knGutterSize = 2;

		//first off, we need to scale the P and Q vectors so that they cover the area equal to
		//the existing area plus the gutters on both sides
		vP *= (Info.m_nTexWidth - 2 * knGutterSize) / fPMag;
		vQ *= (Info.m_nTexHeight - 2 * knGutterSize) / fQMag;		

		vO -= vP * knGutterSize / ((float)Info.m_nTexWidth * vP.MagSqr());
		vO -= vQ * knGutterSize / ((float)Info.m_nTexHeight * vQ.MagSqr());
	}			

	return true;
}

//generates the brushes for an individual polygon face of a brush
static bool GeneratePolygonBrushes(CEditRegion* pRegion, CEditPoly* pPoly, 
									const CEdgeGenInfo& Info, bool* pEdges, LTVector* pVertDisplace)
{
	//vertices used for generation of the polygons
	LTVector vOutVerts[MAX_VERTS];
	uint32	 nOutVerts = 0;

	//the vertices that compose the interior polygon points
	LTVector vInnerVerts[MAX_VERTS];
	uint32	 nInnerVerts = 0;

	LTVector vSrcO = pPoly->GetTexture(0).GetO();
	LTVector vSrcP = pPoly->GetTexture(0).GetP();
	LTVector vSrcQ = pPoly->GetTexture(0).GetQ();

	uint32 nNumVerts = pPoly->NumVerts();

	/*
	//if the polygon is only edges, mainly what we need to do is make a new brush
	//(since the old one will be destroyed), and change the texture to the edge
	//texture
	if(IsPolyOnlyEdges(pPoly, fThickness))
	{
		//just copy over the verts, and use that to create the polygon
		for(uint32 nCurrPt = 0; nCurrPt < nNumVerts; nCurrPt++)
		{
			if(nOutVerts < MAX_VERTS)
				vOutVerts[nOutVerts++] = pPoly->Pt(nCurrPt);
		}
		return CreateBrush(pPoly, pRegion, nOutVerts, vOutVerts, pszTexture, vSrcO, vSrcP, vSrcQ);
	}
	*/

	//ok, now we know that at least a portion of the original polygon is going to
	//be valid, so we need to form the outer edges, as well as the inner polygon.


	//what we need to do now is make our scaled down version of the polygon. This is
	//done by sliding each point forward, and clipping it against the other planes
	uint32 nCurrPt;
	for(nCurrPt = 0; nCurrPt < nNumVerts; nCurrPt++)
	{
		//get the normal of the edge formed by the previous and current points
		LTVector vPrev = pPoly->Pt((nCurrPt + nNumVerts - 1) % nNumVerts);
		LTVector vCurr = pPoly->Pt(nCurrPt);

		LTVector vEdgeNorm = (vCurr - vPrev).Cross(pPoly->Normal());
		vEdgeNorm.Norm();

		//figure out the thickness offset
		float fThickness = pEdges[nCurrPt] ? Info.m_fThickness : 0.0f;

		//now adjust the points accordingly
		vPrev += vEdgeNorm * fThickness;
		vCurr += vEdgeNorm * fThickness;

		//now we need to clip these against all the other edges
		for(uint32 nTestPt = 0; nTestPt < nNumVerts; nTestPt++)
		{
			//skip it if it is the same edge
			if(nTestPt == nCurrPt)
				continue;

			//get the normal of the edge formed by the previous and current points
			LTVector vClipPrev = pPoly->Pt((nTestPt + nNumVerts - 1) % nNumVerts);
			LTVector vClipCurr = pPoly->Pt(nTestPt);

			LTVector vClipNorm = (vClipCurr - vClipPrev).Cross(pPoly->Normal());
			vClipNorm.Norm();

			//figure out the thickness offset
			float fTestThickness = pEdges[nTestPt] ? Info.m_fThickness : 0.0f;

			vClipPrev += vClipNorm * fTestThickness;
			vClipCurr += vClipNorm * fTestThickness;

			float fClipDist = vClipNorm.Dot(vClipCurr);

			//see if the current point is behind the plane
			if(vClipNorm.Dot(vCurr - vClipCurr) < 0.0f)
			{
				//it is, we need to slide it back into the valid area
				float fT = (fClipDist - vClipNorm.Dot(vPrev)) / vClipNorm.Dot(vCurr - vPrev);

				//update the point
				vCurr = vPrev + fT * (vCurr - vPrev);
			}
		}

		//we now have the final position of this point, so add it to the list
		//of inside polygons
		if(nInnerVerts < MAX_VERTS)
			vInnerVerts[nInnerVerts++] = vCurr;
	}

	//the amount to offset vertices inside of the polygon
	LTVector vInnerDisplace = pPoly->Normal() * Info.m_fExtrude;

	//we now have the inner and outer rings of the polygon, so we need to use those
	//to create the edge polygons
	for(nCurrPt = 0; nCurrPt < nNumVerts; nCurrPt++)
	{
		//reset the vertices
		nOutVerts = 0;

		//get the previous and current vertices
		LTVector vPrev = pPoly->Pt((nCurrPt + nNumVerts - 1) % nNumVerts);
		LTVector vCurr = pPoly->Pt(nCurrPt);

		//don't bother if they are too close, or there shouldn't be an edge
		if(!pEdges[nCurrPt] || vPrev.NearlyEquals(vCurr, 0.01f))
			continue;

		//the direction of this edge
		LTVector vEdgeDir = vPrev - vCurr;
		vEdgeDir.Normalize();

		LTVector vPrevDisplace = pVertDisplace[(nCurrPt + nNumVerts - 1) % nNumVerts];
		LTVector vCurrDisplace = pVertDisplace[nCurrPt];

		//now setup the first two vertices to be the outer edge
		vOutVerts[nOutVerts++] = vPrev + vPrevDisplace;
		vOutVerts[nOutVerts++] = vCurr + vCurrDisplace;

		//get the inside vertices
		LTVector vInnerPrev = vInnerVerts[(nCurrPt + nNumVerts - 1) % nNumVerts] + vInnerDisplace;
		LTVector vInnerCurr = vInnerVerts[nCurrPt] + vInnerDisplace;

		//displace them by the normal amount....unless one of them falls on a section with no
		//adjacent edges, in which case we need to make the offset alight with the next vertex
		if(!pEdges[(nCurrPt + nNumVerts - 1) % nNumVerts])
			vInnerPrev += vEdgeDir * vEdgeDir.Dot(vPrevDisplace);

		if(!pEdges[(nCurrPt + 1) % nNumVerts])
			vInnerCurr += vEdgeDir * vEdgeDir.Dot(vCurrDisplace);
		
		//always add the first one...
		vOutVerts[nOutVerts++] = vInnerCurr;

		//only add the second one if they are far enough away...
		if(!vInnerPrev.NearlyEquals(vInnerCurr, 0.01f) && (nOutVerts < MAX_VERTS))
			vOutVerts[nOutVerts++] = vInnerPrev;

		//now we need to calculate the OPQ vectors for this brush, the O should match
		//up with the upper left of the polygon, the P should expand to cover the poly
		//to the right, and the Q should span the length of the edge
		LTVector vO = vOutVerts[1];
		LTVector vQ = vOutVerts[0] - vOutVerts[1];
		LTVector vP = vQ.Cross(pPoly->Normal());

		CalculateOPQ(nOutVerts, vOutVerts, Info, vO, vP, vQ);

		//now go ahead and create this brush
		CEditBrush* pBrush = CreateBrush(	pPoly, pRegion, nOutVerts, vOutVerts, 
											Info.m_pszTexture, vO, vP, vQ, Info.m_pPlaceUnder);

		if(!pBrush)
			return false;

		//override the properties that are applicable
		CBaseProp* pProp = pBrush->m_PropList.GetProp("TextureEffect");
		if(pProp->GetType() == PT_STRING)
			strcpy(((CStringProp*)pProp)->m_String, Info.m_pszTexEffect);
	}

	//reset the output vertices
	nOutVerts = 0;

	if(Info.m_bShrinkSourcePoly)
	{
		//create our new polygon
		CEditPoly *pNewPoly = new CEditPoly(pPoly->m_pBrush);

		if(!pNewPoly)
			return false;

		//copy over the texture information
		pNewPoly->GetTexture(0).m_pTextureName = pRegion->m_pStringHolder->AddString(pPoly->GetTexture(0).m_pTextureName);

		//setup the OPQ's for this brush
		pNewPoly->SetTextureSpace(0, pPoly->GetTexture(0).GetO(), pPoly->GetTexture(0).GetP(), pPoly->GetTexture(0).GetQ());

		//add it to our brush
		pPoly->m_pBrush->m_Polies.Append(pNewPoly);

		//now setup the polygons vertices

		//finally we need to create the inside polygon
		for(nCurrPt = 0; nCurrPt < nNumVerts; nCurrPt++)
		{
			//get the previous and current vertices
			LTVector vInnerPrev = vInnerVerts[(nCurrPt + nNumVerts - 1) % nNumVerts];
			LTVector vInnerCurr = vInnerVerts[nCurrPt];

			//add it only if they are far enough away
			if(!vInnerPrev.NearlyEquals(vInnerCurr, 0.01f))
			{
				pNewPoly->m_Indices.Append(pNewPoly->m_pBrush->AddVertOrGetClosest(vInnerCurr, 0.01f));
			}
		}

		//have the brush update its info
		pNewPoly->m_pBrush->UpdatePlanes();
		pNewPoly->m_pBrush->UpdateBoundingInfo();
	}

	//success
	return true;
}



//given a node, it will recurse and find all the brushes underneath
static bool BuildBrushListR(CWorldNode* pNode, CBrushArray& BrushList)
{
	//see if this node is a brush
	if(pNode->GetType() == Node_Brush)
	{
		//its a brush, add it to the list
		BrushList.Append(pNode->AsBrush());
	}

	//recurse through all the children
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		if(!BuildBrushListR(pChild, BrushList))
			return false;
	}

	return true;
}

//given a polygon, a list of possibly touching brushes, and a vertex for the first point on the edge,
//it will see if any other polygons share an edge, and if so if they violate the crease angle
static bool IsEdgeCreased(CEditPoly* pSrcPoly, CBrushArray& TouchingList, uint32 nVert, float fCosCrease)
{
	//first off, find the edge normal and polygon normal
	LTVector vNormal		= pSrcPoly->Normal();

	LTVector vPt[2];
	vPt[0]					= pSrcPoly->Pt((nVert + pSrcPoly->NumVerts() - 1) % pSrcPoly->NumVerts());
	vPt[1]					= pSrcPoly->Pt(nVert);

	//ok, now we can run through all the other polygons and determine which edges are collinear
	for(uint32 nCurrBrush = 0; nCurrBrush < TouchingList.GetSize(); nCurrBrush++)
	{
		//run through the polygons of this brush
		for(uint32 nCurrPoly = 0; nCurrPoly < TouchingList[nCurrBrush]->m_Polies.GetSize(); nCurrPoly++)
		{
			//cache the polygon
			CEditPoly* pPoly = TouchingList[nCurrBrush]->m_Polies[nCurrPoly];

			//ignore if they are the same
			if(pSrcPoly == pPoly)
				continue;

			//now see if the edges are collinear
			uint32 nNumVerts = pPoly->NumVerts();
			for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
			{
				LTVector vTestPt		= pPoly->Pt(nCurrVert);
				LTVector vPrevTestPt	= pPoly->Pt((nCurrVert + nNumVerts - 1) % nNumVerts);

				if((vTestPt.NearlyEquals(vPt[0], 0.1f) && vPrevTestPt.NearlyEquals(vPt[1], 0.1f)) ||
				   (vTestPt.NearlyEquals(vPt[1], 0.1f) && vPrevTestPt.NearlyEquals(vPt[0], 0.1f)))
				{
					if(pPoly->Normal().Dot(vNormal) >= fCosCrease)
						return false;

					return true;
				}
			}
		}
	}

	//we didn't hit anything, therefore it counts as a crease
	return true;
}

//given a vertex position, this will find the vertex normal with all edges influencing it
static LTVector FindPointNormal(const LTVector& vPt, CEditPoly* pSrcPoly, float fExtrude, CBrushArray& TouchingList)
{
	LTVector vRV(pSrcPoly->Normal() * fExtrude);

	for(uint32 nCurrBrush = 0; nCurrBrush < TouchingList.GetSize(); nCurrBrush++)
	{
		for(uint32 nCurrPoly = 0; nCurrPoly < TouchingList[nCurrBrush]->m_Polies.GetSize(); nCurrPoly++)
		{
			CEditPoly* pPoly = TouchingList[nCurrBrush]->m_Polies[nCurrPoly];

			if(pPoly == pSrcPoly)
				continue;

			for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
			{
				if(vPt.NearlyEquals(pPoly->Pt(nCurrVert), 0.1f))
				{
					vRV += pPoly->Normal() * fExtrude;
					break;
				}
			}
		}
	}

	//vRV.Normalize();

	return vRV;	
}

//handles generation of edges for a single brush
static bool GenerateBrushEdges(CEditRegion* pRegion, CEditBrush* pBrush, const CEdgeGenInfo& Info, CBrushArray& TouchingList)
{
	//the list of active edges
	bool bActiveEdges[MAX_VERTS];
	LTVector vVertDisplace[MAX_VERTS];

	//we need to take a polygon based approach for edge generation, so run through
	//all the polygons

	//note that it is important to cache this value since during edge creation polygons
	//may be added in the process, resulting in an infinite loop
	uint32 nNumPolies = pBrush->m_Polies.GetSize();

	for(uint32 nCurrPoly = 0; nCurrPoly < nNumPolies; nCurrPoly++)
	{
		//cache some information
		CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];
		uint32 nNumVerts = pPoly->NumVerts();

		//make sure that it is a valid polygon
		if(nNumVerts < 3)
			continue;

		//for each poly, we need to look at all the edges, and see which ones meet the crease angle criteria
		if(Info.m_fCosCrease < 0.999f)
		{
			//we need to consider a crease, so for each edge, see if it is matched
			for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
				bActiveEdges[nCurrVert] = IsEdgeCreased(pPoly, TouchingList, nCurrVert, Info.m_fCosCrease);
		}
		else
		{
			//everything is creased.... all edges are used
			for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
				bActiveEdges[nCurrVert] = true;
		}

		//gather vertex normals
		for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
		{
			if(bActiveEdges[nCurrVert] || bActiveEdges[(nCurrVert + 1) % nNumVerts])
			{
				vVertDisplace[nCurrVert] = FindPointNormal(pPoly->Pt(nCurrVert), pPoly, Info.m_fExtrude, TouchingList);
			}
		}

		//alright, we now have a list of which edges of the polygon will have edges generated....now let us generate
		//the edges
		if(!GeneratePolygonBrushes(pRegion, pPoly, Info, bActiveEdges, vVertDisplace))
			return false;

	}

	//remove the original faces if we are shrinking them
	if(Info.m_bShrinkSourcePoly)
	{
		for(int32 nRemovePoly = nNumPolies; nRemovePoly >= 0; nRemovePoly--)
		{
			pBrush->m_Polies.Remove(nRemovePoly);
		}
	}

	return true;
}

//ok, we now have the object that is generating the edge, and all the brushes it will
//generate on
static bool GenerateEdgeGroup(CEditRegion* pRegion, CBaseEditObj* pEdgeObj, CBrushArray& BrushList)
{
	CEdgeGenInfo Info;

	//read in the properties from the edge object
	Info.m_pszTexture			= GetStringProp(pEdgeObj, "Texture", "");
	Info.m_pszTexEffect			= GetStringProp(pEdgeObj, "TextureEffect", "");
	Info.m_fThickness			= GetRealProp(pEdgeObj, "Thickness", 0.0f);
	Info.m_fExtrude				= GetRealProp(pEdgeObj, "Extrude", 0.0f);
	Info.m_bShrinkSourcePoly	= GetBoolProp(pEdgeObj, "ShrinkBrushes", LTTRUE);

	const char* pszWorldModel	= GetStringProp(pEdgeObj, "WorldModel", "");
	float fCrease				= GetRealProp(pEdgeObj, "CreaseAngle", 0.0f);

	//let us take this oppertunity to bail out if they don't want any edges
	if(Info.m_fThickness < 0.1f)
		return true;

	//make sure that we can load up the texture to be used for the edges
	if(!GetTextureDims(Info.m_pszTexture, Info.m_nTexWidth, Info.m_nTexHeight))
	{
		//print out some error here
		return true;
	}

	//now we need to try and find the object that we are supposed to place the nodes under
	CBaseEditObj* pParentObject;
	if(pRegion->FindObjectsByName(pszWorldModel, &pParentObject, 1) == 0)
	{
		//we didn't find an object to place it under, so just place it under the current parent
		Info.m_pPlaceUnder = pEdgeObj;
	}
	else
	{
		Info.m_pPlaceUnder = pParentObject;
	}

	//setup the crease angle
	Info.m_fCosCrease = (float)cos(fCrease * MATH_PI / 180.0f);
	Info.m_bAdjustEdgeUVs		= true;

	//a list for brushes that could possibly intersect the current brush
	CBrushArray TouchingList;
	TouchingList.SetCacheSize(BrushList.GetSize());

	//alright, now that we know about the edges we are to generate and the brushes to generate on, let us generate
	for(uint32 nCurrBrush = 0; nCurrBrush < BrushList.GetSize(); nCurrBrush++)
	{
		//empty out the old touch list
		TouchingList.RemoveAll();

		//always add the source brush first so it will be checked for edge creasing first, huge performance
		//boost...
		TouchingList.Append(BrushList[nCurrBrush]);

		//we need to build up a list of brushes that could potentially share edges
		for(uint32 nTouchBrush = 0; nTouchBrush < BrushList.GetSize(); nTouchBrush++)
		{
			//don't want to add the main brush twice...
			if(nTouchBrush == nCurrBrush)
				continue;

			//note that a brush will be included in this list as well, which is good
			//since the brush needs to check itself for touching polies
			if(BrushList[nCurrBrush]->m_BoundingSphere.IntersectsSphere(BrushList[nTouchBrush]->m_BoundingSphere))
			{
				//they intersect potentially, add to the list
				TouchingList.Append(BrushList[nTouchBrush]);
			}
		}

		GenerateBrushEdges(pRegion, BrushList[nCurrBrush], Info, TouchingList);
	}

	return true;
}

//given a node, this will recurse into its children to generate edges, and if this node is an edge generator, it will
//gather all brushes beneath it for processing
static bool CreateNodeEdgesR(CWorldNode* pNode, CEditRegion* pRegion)
{
	//we need to recurse into all of the children. This ordering allows for the user
	//to place edge generators recursively to generate edging for edging (not too useful
	//but less error prone than any other configuration)
	//first we need to recurse on the children (this is because we will be messing up the heirarchy below)
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		if(!CreateNodeEdgesR(pChild, pRegion))
			return false;
	}

	//see if this current node is an object
	if(pNode->GetType() == Node_Object)
	{
		//it is an object, we need to see if this object is an edge generator
		if(stricmp(pNode->GetClassName(), "EdgeGenerator") == 0)
		{
			//we need to extract all the information from the object, build up a brush list, and
			//then generate our edges and reconstruct our node tree
			CBrushArray BrushList;
			BrushList.SetCacheSize(256);

			if(!BuildBrushListR(pNode, BrushList))
				return false;

			//ok, we now have the object that is generating the edge, and all the brushes it will
			//generate on
			GenerateEdgeGroup(pRegion, pNode->AsObject(), BrushList);

			//we can now destroy this object (this will also attach all children of this object to the parent)
			no_DestroyNode(pRegion, pNode, false);
		}
	}

	return true;
}




//given a world, it will go through finding the brushes
//marked for generation of edges, and upon finding them
//will break the brush apart into polygon brushes, and
//generate the appropriate polygons for the edging
bool CreatePolyEdges(CEditRegion* pRegion)
{
	return CreateNodeEdgesR(pRegion->GetRootNode(), pRegion);
}

