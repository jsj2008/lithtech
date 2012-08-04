#include "bdefs.h"
#include "EditRegion.h"
#include "EditPoly.h"
#include "node_ops.h"
#include "geomroutines.h"
#include "processing.h"
#include "proputils.h"
#include "texturedims.h"

//some macros used for the brush ignore list
#define MAX_IGNORE_BRUSHES			64

//structure for storing a decal polygon
struct CDecalPoly
{
public:

	enum {	MAX_VERTS	= 128	};

	CDecalPoly() :
		m_nNumVerts(0),
		m_pSrcBrush(NULL)
	{
	}


	//adds a vertex to the polygon's list
	void AddVert(const LTVector& Vert)
	{
		if(m_nNumVerts < MAX_VERTS)
		{
			m_vVert[m_nNumVerts] = Vert;
			m_nNumVerts++;
		}
	}

	//removes a vertex from the list
	void RemoveVert(uint32 nIndex)
	{
		if(nIndex < m_nNumVerts)
		{
			for(uint32 nCurrVert = nIndex; nCurrVert < m_nNumVerts - 1; nCurrVert++)
			{
				m_vVert[nCurrVert] = m_vVert[nCurrVert + 1];
			}
			m_nNumVerts--;
		}
	}

	//the plane that this polygon lies on
	LTPlane		m_Plane;

	//vertices of the actual polygon
	LTVector	m_vVert[MAX_VERTS];
	uint32		m_nNumVerts;

	//the brush that this polygon was created from
	CEditBrush*	m_pSrcBrush;

	//-----------------------
	//Occlusion information

	//the ID of the source polygon that this polygon came from
	uint32		m_nSrcPolyID;

	//-----------------------
	//Merging information
	uint32		m_nPlaneID;
};

//structure to hold info about a brush that has been created but not yet added to the node
//tree
struct CDecalBrush
{
public:
	CEditBrush*		m_pBrush;
	CWorldNode*		m_pParent;
};

//structure to hold information for a frustum
struct CFrustum
{
public:
	LTPlane		m_Planes[6];
};


//utility typedef
typedef CMoArray<CBaseEditObj*> CObjectArray;
typedef CMoArray<CDecalPoly*>	CPolyArray;
typedef CMoArray<CDecalBrush>	CDecalBrushArray;

//goes through the node heirarchy removing decal objects and adding them to the list
static bool RemoveDecalObjectsR(CWorldNode* pNode, CEditRegion* pRegion, CObjectArray& DecalList)
{
	//see if this node is a valid world node
	assert(pNode);
	assert(pRegion);

	//now see if it is a decal
	if((pNode->GetType() == Node_Object) && (stricmp(pNode->GetClassName(), "Decal") == 0))
	{
		//we have a decal. Make sure it doesn't have any children
		GPOS Pos = pNode->m_Children;

		if(Pos)
		{
			//this has children. Inform the user
			DrawStatusText(eST_Error, "Found children underneath decal object %s. These will be destroyed.", pNode->GetName());
			GDeleteAndRemoveElements(pNode->m_Children);
			Pos = NULL;
		}

		//remove it from the tree, but make sure to preserve the parent link (which will be used later, to figure
		//out which node to put the brushes under)
		CWorldNode* pParent = pNode->GetParent();
		pNode->RemoveFromTree();
		pNode->SetParent(pParent);

		//also remove it from the region's internal list
		pRegion->RemoveObject(pNode->AsObject());		

		//add it to our list
		if(!ShouldIgnoreNode(pNode))
		{
			DecalList.Append(pNode->AsObject());
		}
	}
	else
	{	
		//ok, now we need to run through all the children and recurse
		GPOS Pos = pNode->m_Children;
		while(Pos)
		{
			CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
			RemoveDecalObjectsR(pChild, pRegion, DecalList);
		}
	}

	return true;
}

//goes through the world node heirarchy and tries to find the specified object, if it
//finds it, it will return a pointer to it
CBaseEditObj* FindObjectR(const char* pszName, CWorldNode* pNode)
{
	//check for the end of the line
	assert(pNode);

	//see if this node is the match
	if((pNode->GetType() == Node_Object) && (stricmp(pNode->GetName(), pszName) == 0))
	{
		//it is the chosen one...
		return pNode->AsObject();
	}

	//this is not the correct one, keep looking
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		CBaseEditObj* pRV = FindObjectR(pszName, pChild);

		if(pRV)
			return pRV;
	}

	//no luck.
	return NULL;
}



//determines if an edit polygon intersects the specified frustum (not 100% accurate, but fairly fast)
static bool DoesPolyIntersect(CEditPoly* pPoly, const CFrustum& Frustum)
{
	for(uint32 nCurrPlane = 0; nCurrPlane < 6; nCurrPlane++)
	{
		bool bFrontSide = false;

		//just see if any points are on the front side
		for(uint32 nCurrPt = 0; nCurrPt < pPoly->m_Indices.GetSize(); nCurrPt++)
		{
 			if(Frustum.m_Planes[nCurrPlane].DistTo(pPoly->Pt(nCurrPt)) > 0.0f)
			{
				bFrontSide = true;
				break;
			}
		}

		if(!bFrontSide)
		{
			return false;
		}
	}

	return true;
}

//creates a new decal polygon given an edit polygon
CDecalPoly* CreateDecalPoly(CEditPoly* pPoly)
{
	assert(pPoly);

	//create the decal poly
	CDecalPoly* pNewPoly = new CDecalPoly;

	//make sure it worked
	if(pNewPoly == NULL)
		return NULL;

	//copy over the vertices
	for(uint32 nCurrPt = 0; nCurrPt < pPoly->m_Indices.GetSize(); nCurrPt++)
	{
		pNewPoly->AddVert(pPoly->Pt(nCurrPt));
	}

	//copy over the normal
	pNewPoly->m_Plane = pPoly->m_Plane;
	
	//save the source brush
	pNewPoly->m_pSrcBrush = pPoly->m_pBrush;

	//success
	return pNewPoly;
}

//given a brush, it will determine if it intersects the specified frustum, and if so, it will add all non
//backfacing polygons to the specified list
static void AddBrushPoliesInFrustum(CEditBrush* pBrush, CPolyArray& PolyList, 
									const CFrustum& Frustum, const LTVector& vForward,
									CString* psExcludeList, uint32 nNumExclude)
{
	//make sure the brush is valid
	assert(pBrush);

	//see if this brush is excluded...
	for(uint32 nCurrExclude = 0; nCurrExclude < nNumExclude; nCurrExclude++)
	{
		const char* pszExclude = psExcludeList[nCurrExclude];
		const char* pszBrush = pBrush->GetName();

		//now do an old school style string comparison with one modification, if it
		//encounters a pound in the exclude name, it counts as a successful early termination,
		//this allows for end of string wild cards. However, if the artists wants a certain subsection
		//of the brushes to come through, they can also use the + to force inclusion
		bool bMatched = true;
		bool bForceInclusion = false;

		while(*pszExclude || *pszBrush)
		{
			if(*pszExclude == '#')
			{
				//successful match, it is excluded
				return;
			}
			if(*pszExclude == '+')
			{
				bForceInclusion = true;
				break;
			}
			if(*pszExclude != *pszBrush)
			{
				bMatched = false;
				break;
			}

			++pszExclude;
			++pszBrush;
		}
		
		//if we want to force inclusion, we don't need to test any more exclude list items
		if(bForceInclusion)
			break;

		//bail if we matched
		if(bMatched)
			return;
	}

	//now we need to see if the brush is in the frustum
	for(uint32 nCurrPlane = 0; nCurrPlane < 6; nCurrPlane++)
	{
		//test for the plane being outside of any of the spheres
		if(Frustum.m_Planes[nCurrPlane].DistTo(pBrush->m_BoundingSphere.GetPos()) < -pBrush->m_BoundingSphere.GetRadius())
		{
			//outside the frustum
			return;
		}
	}

	//ok, we are inside the frustum, add any polygons that aren't backfacing
	for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
	{
		//skip over it if it is backfacing or not valid
		CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];

		//check for invalid
		if(pPoly->m_Indices.GetSize() < 3)
			continue;

		//test against the polygon normal
		if(vForward.Dot(pPoly->Normal()) > -0.05f)
		{
			//backfacing
			continue;
		}

		//this one is valid, and not backfacing, we should see if it intersects the frustum at all
		if(DoesPolyIntersect(pPoly, Frustum))
		{
			//we need to create a new polygon and add it
			PolyList.Append(CreateDecalPoly(pPoly));
		}
	}

}


//create the list of polygons that intersect the frustum
static void BuildPolygonListR(CWorldNode* pNode, CPolyArray& PolyList, const CFrustum& Frustum, 
							  const LTVector& vForward, bool bAddWorldModels,
							  CString* psExcludeList, uint32 nNumExclude)
{
	//make sure the node is valid
	assert(pNode);

	//flag indicating if this is an object that would classify a world model
	bool bWorldModel = false;

	//see if this node is a brush
	if(pNode->GetType() == Node_Brush)
	{
		//it is a brush, so we need to add appropriate polygons to the list
		AddBrushPoliesInFrustum(pNode->AsBrush(), PolyList, Frustum, vForward, psExcludeList, nNumExclude);
	}
	else
	{
		//see if this is a potential world model object
		if(pNode->GetType() == Node_Object)
		{
			bWorldModel = true;
		}
	}

	//see if we want to test the children
	if(bAddWorldModels || !bWorldModel)
	{
		//we do
		GPOS Pos = pNode->m_Children;

		while(Pos)
		{
			CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
			BuildPolygonListR(pChild, PolyList, Frustum, vForward, bAddWorldModels, psExcludeList, nNumExclude);
		}
	}
}

//initializes a plane from 3 points
static void InitPlane(LTPlane& Plane, const LTVector& a, const LTVector& b, const LTVector& c)
{
	LTVector ab = a - b;
	LTVector bc = b - c;

	Plane.m_Normal = ab.Cross(bc);
	Plane.m_Normal.Norm();

	Plane.m_Dist = Plane.m_Normal.Dot(a);
}

//extrudes the specified polygons along the appropriate normals by the amoun specified
static void OffsetPolygons(CPolyArray& PolyList, float fOffset)
{
	//we need to find all matching vertices
	CMoArray<LTVector> VertNormal;
	CMoArray<LTVector> VertList;
	CMoArray<uint32>   VertMap;

	uint32 nCurrPoly;
	for(nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CDecalPoly* pPoly = PolyList[nCurrPoly];

		//run through each vertex and setup the map
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->m_nNumVerts; nCurrVert++)
		{
			//see if we have any matching vertices
			uint32 nMatch = (uint32)-1;

			for(uint32 nTestVert = 0; nTestVert < VertList.GetSize(); nTestVert++)
			{
				if(VertList[nTestVert].NearlyEquals(pPoly->m_vVert[nCurrVert], 0.1f))
				{
					//it matches, use this one.
					nMatch = nTestVert;

					//we need to also contribute our normal to this
					VertNormal[nTestVert] += pPoly->m_Plane.m_Normal;

					break;
				}
			}

			//see if we missed any match
			if(nMatch == (uint32)-1)
			{
				//add our own vertex
				nMatch = VertList.GetSize();

				VertNormal.Append(pPoly->m_Plane.m_Normal);
				VertList.Append(pPoly->m_vVert[nCurrVert]);
			}

			//add this to the map
			VertMap.Append(nMatch);
		}
	}

	//we now need to normalize all the normals
	for(uint32 nCurrNormal = 0; nCurrNormal < VertNormal.GetSize(); nCurrNormal++)
	{
		VertNormal[nCurrNormal].Norm();
	}

	//ok, now that we have all the vertex normals figured out, we can adjust the polygons appropriately
	uint32 nMapIndex = 0;

	for(nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CDecalPoly* pPoly = PolyList[nCurrPoly];

		//run through each vertex and adjust
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->m_nNumVerts; nCurrVert++)
		{
			//move the point along the normal by the amount specified
			pPoly->m_vVert[nCurrVert] += VertNormal[VertMap[nMapIndex]] * fOffset;

			nMapIndex++;
		}
	}

	//all done
}

//clips a polygon to a plane, storing the newly clipped polygon that lies on the front side of the plane in FrontPoly,
//and the polygon that lies on the back in BackPoly
#define CLIP_EPSILON	0.01f

static void ClipPolygon(const CDecalPoly& ToClip, const LTPlane& Plane, CDecalPoly& FrontPoly, CDecalPoly& BackPoly)
{
	//reset the polygons
	FrontPoly.m_nNumVerts = 0;
	FrontPoly.m_Plane = ToClip.m_Plane;
	FrontPoly.m_pSrcBrush = ToClip.m_pSrcBrush;

	BackPoly.m_nNumVerts = 0;
	BackPoly.m_Plane = ToClip.m_Plane;
	BackPoly.m_pSrcBrush = ToClip.m_pSrcBrush;

	//make sure the poly is valid
	if(ToClip.m_nNumVerts < 3)
		return;

	//initialize the previous vertex status
	uint32  nPrevVert	= ToClip.m_nNumVerts - 1;
	float	fPrevDist	= Plane.DistTo(ToClip.m_vVert[nPrevVert]);
	bool	bPrevIn		= fPrevDist >= -CLIP_EPSILON;

	//now clip the polygon
	for(uint32 nCurrVert = 0; nCurrVert < ToClip.m_nNumVerts; nCurrVert++)
	{
		//get the status for the current vertex
		float fCurrDist = Plane.DistTo(ToClip.m_vVert[nCurrVert]);
		bool  bCurrIn	= fCurrDist >= -CLIP_EPSILON;

		//see if we are entering or leaving the space
		if(bPrevIn != bCurrIn)
		{
			//we will need the intersection vertex
			float fT = -fPrevDist / (fCurrDist - fPrevDist);
			LTVector vIntersect = (1.0f - fT) * ToClip.m_vVert[nPrevVert] + fT * ToClip.m_vVert[nCurrVert];

			BackPoly.AddVert(vIntersect);
			FrontPoly.AddVert(vIntersect);

			//now add to the appropriate lists
			if(bCurrIn)
			{
				//we re-entered the space
				FrontPoly.AddVert(ToClip.m_vVert[nCurrVert]);
			}
			else
			{
				//we left the front side
				BackPoly.AddVert(ToClip.m_vVert[nCurrVert]);
			}
		}
		else
		{
			if(bCurrIn)
			{
				//we are inside the space
				FrontPoly.AddVert(ToClip.m_vVert[nCurrVert]);
			}
			else
			{
				//we are outside the space
				BackPoly.AddVert(ToClip.m_vVert[nCurrVert]);
			}
		}

		bPrevIn		= bCurrIn;
		fPrevDist	= fCurrDist;
		nPrevVert	= nCurrVert;
	}
}


//and now we clip them so that they are entirely contained within the frustum
static void ClipPolygonsToFrustum(CPolyArray& PolyList, const CFrustum& Frustum)
{
	if(PolyList.GetSize() == 0)
		return;

	CDecalPoly BackPoly, FrontPoly[2];

	//now we need to iterate through all the polygons backwards, since some may get clipped out
	for(int32 nCurrPoly = PolyList.GetSize() - 1; nCurrPoly >= 0; nCurrPoly--)
	{
		//first clip stage, from the polygon into our juggling buffers
		ClipPolygon(*PolyList[nCurrPoly], Frustum.m_Planes[0], FrontPoly[0], BackPoly);

		//middle clip stages, juggling the buffers back and forth
		for(uint32 nCurrPlane = 1; nCurrPlane < 5; nCurrPlane++)
		{
			ClipPolygon(FrontPoly[(nCurrPlane + 1) % 2], Frustum.m_Planes[nCurrPlane], FrontPoly[nCurrPlane % 2], BackPoly);
		}

		//final clipping stage, back into the original polygon
		ClipPolygon(FrontPoly[0], Frustum.m_Planes[nCurrPlane], *PolyList[nCurrPoly], BackPoly);

		//now see if this polygon was clipped out entirely
		if(PolyList[nCurrPoly]->m_nNumVerts < 3)
		{
			//it did, remove it from the list
			delete PolyList[nCurrPoly];
			PolyList.Remove(nCurrPoly);
		}
	}
}

//determines if the property lists are different as necessary by the actual decal polygons
//(so it ignores name, position, orientation, etc)
static bool ArePropertiesDifferent(CPropList& lhs, CPropList& rhs)
{
	//if the sizes are different, then there is a difference there
	if(lhs.GetSize() != rhs.GetSize())
		return true;

	//now we have to check each property to see if their values are the same
	//(ignoring props we don't care about)
	for(uint32 nCurrProp = 0; nCurrProp < (uint32)lhs.GetSize(); nCurrProp++)
	{
		//get the prop from the left hand object
		CBaseProp* pLeft = lhs.GetAt(nCurrProp);

		if(!pLeft)
			continue;

		//ignore if it is one we don't care about
		if(	(stricmp(pLeft->GetName(), "Name") == 0) ||
			(stricmp(pLeft->GetName(), "Pos") == 0) ||
			(stricmp(pLeft->GetName(), "Or") == 0))
		{
			//don't care about this one
			continue;
		}

		//get the right hand equivilant
		CBaseProp* pRight = rhs.GetProp(pLeft->GetName());

		if(!pRight)
			continue;

		//make sure the types match
		if(pLeft->GetType() != pRight->GetType())
			return true;

		bool bMatch = false;

		//we have to explicity convert types since the property heirarchy was written completely
		//incorrectly to handle overloading of equality comparisons
		switch(pLeft->GetType())
		{
		case LT_PT_STRING: bMatch = ((CStringProp*)pLeft)->SameAs((CStringProp*)pRight); break;
		case LT_PT_VECTOR: bMatch = ((CVectorProp*)pLeft)->SameAs((CVectorProp*)pRight); break;
		case LT_PT_COLOR: bMatch = ((CColorProp*)pLeft)->SameAs((CColorProp*)pRight); break;
		case LT_PT_BOOL: bMatch = ((CBoolProp*)pLeft)->SameAs((CBoolProp*)pRight); break;
		case LT_PT_ROTATION: bMatch = ((CRotationProp*)pLeft)->SameAs((CRotationProp*)pRight); break;

		case LT_PT_REAL: 
		case LT_PT_FLAGS: 
		case LT_PT_LONGINT: 
			bMatch = ((CRealProp*)pLeft)->SameAs((CRealProp*)pRight); 
			break;
		default:
			assert(false);
			break;
		}
		//now compare the values
		if(!bMatch)
			return true;
	}

	//they are the same
	return false;
}

//handles converting all the decal polygons to actual brushes
static bool CreateBrushes(	CPolyArray& PolyList, CWorldNode* pRootObject, CEditRegion* pRegion, 
							const char* pszTexture, const LTVector& vO, const LTVector& vP, const LTVector& vQ,
							CDecalBrushArray& BrushList, const char* pszTextureEffect)
{
	//create a list of booleans indicating which polygons have been processed already
	CMoArray<bool> UsedList;
	UsedList.SetSize(PolyList.GetSize());

	for(uint32 nCurrUsed = 0; nCurrUsed < UsedList.GetSize(); nCurrUsed++)
		UsedList[nCurrUsed] = false;

	//we need to create a single brush per property group, so run through all the polygons,
	//grab the first unprocessed one, create a brush, and add to it all polygons that have
	//the same properties
	for(uint32 nPropPoly = 0; nPropPoly < PolyList.GetSize(); nPropPoly++)
	{
		//if this has already been processed, don't worry about it
		if(UsedList[nPropPoly])
			continue;

		CDecalPoly* pPropPoly = PolyList[nPropPoly];

		//do not process this brush if it doesn't have a source
		if(pPropPoly->m_pSrcBrush == NULL)
			continue;
	
		//first off create the new brush
		CEditBrush* pNewBrush = new CEditBrush;

		if(pNewBrush == NULL)
			return false;

		//run through all polygons, and create a brush for them
		for(uint32 nCurrPoly = nPropPoly; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
		{
			//don't bother if we have already been used
			if(UsedList[nCurrPoly])
				continue;

			CDecalPoly* pPoly = PolyList[nCurrPoly];

			//ignore if the properties are different and this isn't our source brush
			if((nCurrPoly != nPropPoly) && ArePropertiesDifferent(*pPropPoly->m_pSrcBrush->GetPropertyList(), *pPoly->m_pSrcBrush->GetPropertyList()))
				continue;

			//only bother with valid polygons
			if((pPoly->m_nNumVerts < 3) || (pPoly->m_pSrcBrush == NULL))
				continue;

			//now add the polygon to the brush
			CEditPoly* pNewPoly = new CEditPoly(pNewBrush);

			if(pNewPoly == NULL)
			{
				delete pNewBrush;
				return false;
			}

			//add the vertices to the brush
			pNewPoly->m_Indices.SetSize(pPoly->m_nNumVerts);

			for(uint32 nCurrVert = 0; nCurrVert < pPoly->m_nNumVerts; nCurrVert++)
			{
				//we need to find our vertex in the list
				uint32 nVertIndex = 0;

				//see if we have an existing poing
				bool bFound = false;

				for(uint32 nBrushVert = 0; nBrushVert < pNewBrush->m_Points.GetSize(); nBrushVert++)
				{
					//see if this point is a match
					if((pPoly->m_vVert[nCurrVert] - pNewBrush->m_Points[nBrushVert]).MagSqr() < 0.1f)
					{
						//we found a match, let us go ahead and use this
						nVertIndex = nBrushVert;
						bFound = true;
						break;
					}
				}

				//if we didn't find a match, we need to add our point into the map
				if(!bFound)
				{
					nVertIndex = pNewBrush->m_Points.GetSize();
					pNewBrush->m_Points.Append(pPoly->m_vVert[nCurrVert]);
				}

				//set up our index to point to it accordingly
				pNewPoly->m_Indices[nCurrVert] = nVertIndex;
			}

			//add the polygon to the brush list
			pNewBrush->m_Polies.Append(pNewPoly);

			//copy over the texture information
			pNewPoly->GetTexture(0).m_pTextureName = pRegion->m_pStringHolder->AddString(pszTexture);

			//setup the OPQ's for this brush
			pNewPoly->SetTextureSpace(0, vO, vP, vQ);

			//we have now been used
			UsedList[nCurrPoly] = true;
		}

		//have the brush update its info
		pNewBrush->UpdatePlanes();
		pNewBrush->UpdateBoundingInfo();

		//now we need to copy the properties from the source brush into the new brush
		pNewBrush->m_PropList.CopyValues(pPropPoly->m_pSrcBrush->GetPropertyList());

		//Now apply any overrides from the decal on the underlying brush
		if(pszTextureEffect)
		{
			CBaseProp* pProp = pNewBrush->GetPropertyList()->GetProp("TextureEffect");
			if(pProp && (pProp->GetType() == PT_STRING))
			{
				//this value should be changed to the overridden one
				strncpy(((CStringProp*)pProp)->m_String, pszTextureEffect, MAX_STRINGPROP_LEN);
			}
		}

		//decals should never be solid or block light
		CBaseProp* pProp = pNewBrush->m_PropList.GetProp("ClipLight");
		if(pProp && (pProp->GetType() == PT_BOOL))
			((CBoolProp*)pProp)->m_Value = FALSE;


		//add our brush onto the end of the list
		CDecalBrush Brush;
		Brush.m_pBrush	= pNewBrush;
		Brush.m_pParent = pRootObject;

		BrushList.Append(Brush);
	}

	//success
	return true;
}

#define	POLYSIDE_FRONT		0x1
#define	POLYSIDE_BACK		0x2
#define	POLYSIDE_INTERSECT	0x3

//given a plane and a polygon, it will determine which side and return it in a
//bit mask
static uint32 GetPolySide(const LTPlane& Plane, const CDecalPoly* pPoly)
{
	assert(pPoly);

	uint32	nRV = 0;
	float	fDot;

	//test all verts
	for(uint32 nVert = 0; nVert < pPoly->m_nNumVerts; nVert++)
	{
		//find the plane distance
		fDot = Plane.DistTo(pPoly->m_vVert[nVert]);

		//classify, biasing towards being on the front side
		if(fDot < -CLIP_EPSILON * 2)
			nRV |= POLYSIDE_BACK;
		else if(fDot >= -CLIP_EPSILON * 2)
			nRV |= POLYSIDE_FRONT;
	}

	return nRV;
}

//this will clean up a polygon, removing colinear vertices, duplicate points, etc.
static void CleanupPolygonPts(CDecalPoly* pPoly)
{
	assert(pPoly);

	if(pPoly->m_nNumVerts < 3)
		return;

	//remove duplicate vertices
	uint32 nCurrVert;
	for(nCurrVert = 0; nCurrVert < pPoly->m_nNumVerts;)
	{
		if(pPoly->m_vVert[nCurrVert].NearlyEquals(pPoly->m_vVert[(nCurrVert + 1) % pPoly->m_nNumVerts], CLIP_EPSILON))
		{
			pPoly->RemoveVert(nCurrVert);
		}
		else
		{
			nCurrVert++;
		}
	}

	if(pPoly->m_nNumVerts < 3)
		return;

	//remove co-linear vertices
	for(nCurrVert = 0; nCurrVert < pPoly->m_nNumVerts;)
	{
		//see if the vertex ahead is collinear
		LTVector vToNext = pPoly->m_vVert[(nCurrVert + 1) % pPoly->m_nNumVerts] - pPoly->m_vVert[nCurrVert];
		LTVector vBasis  = pPoly->m_vVert[(nCurrVert + 2) % pPoly->m_nNumVerts] - pPoly->m_vVert[nCurrVert];

		vToNext.Norm();
		vBasis.Norm();

		if(vBasis.Dot(vToNext) >= 0.9999f)
			pPoly->RemoveVert((nCurrVert + 1) % pPoly->m_nNumVerts);
		else
			nCurrVert++;
	}
}

//given a polygon, it will determine if it is convex or not
static bool IsPolygonConvex(CDecalPoly* pPoly)
{
	assert(pPoly);

	if(pPoly->m_nNumVerts < 3)
		return false;

	uint32 nPrevPt = pPoly->m_nNumVerts - 1;
	for(uint32 nCurrPt = 0; nCurrPt < pPoly->m_nNumVerts; nPrevPt = nCurrPt, nCurrPt++)
	{
		//find the edge normal
		LTVector vToNext = pPoly->m_vVert[nCurrPt] - pPoly->m_vVert[nPrevPt];
		LTVector vNormal = vToNext.Cross(pPoly->m_Plane.m_Normal);

		//now see if any points are outside of this
		for(uint32 nTestPt = 0; nTestPt < pPoly->m_nNumVerts; nTestPt++)
		{
			LTVector vToTest = pPoly->m_vVert[nTestPt] - pPoly->m_vVert[nCurrPt];

			//so now we need to see if it is on the front side
			if(vToTest.Dot(vNormal) < -CLIP_EPSILON)
			{
				//it is on the front side, not convex
				return false;
			}
		}
	}

	//it is convex
	return true;
}

//given two polygons, it will attempt to merge them together. Will return NULL if the polygons cannot be merged
static CDecalPoly* MergePolygons(CDecalPoly* pPoly1, CDecalPoly* pPoly2)
{
	//sanity check
	assert(pPoly1);
	assert(pPoly2);

	//handle degenerate polygons appropriately
	if(pPoly1->m_nNumVerts < 3)
		return NULL;
	if(pPoly2->m_nNumVerts < 3)
		return NULL;

	bool bMatchedEdge = false;

	//indicates which vert on each poly is the one that should cause the swap
	uint32 nMatchPt1;
	uint32 nMatchPt2;

	//we first need to make sure that they share an edge
	for(uint32 nV1 = 0; nV1 < pPoly1->m_nNumVerts; nV1++)
	{
		for(uint32 nV2 = 0; nV2 < pPoly2->m_nNumVerts; nV2++)
		{
			//see if these points match
			if(pPoly1->m_vVert[nV1].NearlyEquals(pPoly2->m_vVert[nV2], CLIP_EPSILON))
			{
				//they match, see if the next point matches as well
				if(pPoly1->m_vVert[(nV1 + 1) % pPoly1->m_nNumVerts].NearlyEquals(
							pPoly2->m_vVert[(nV2 + pPoly1->m_nNumVerts - 1) % pPoly1->m_nNumVerts], CLIP_EPSILON))
				{
					//we have a shared edge!
					bMatchedEdge = true;
					nMatchPt1 = nV1;
					nMatchPt2 = (nV2 + pPoly1->m_nNumVerts - 1) % pPoly2->m_nNumVerts;
					break;
				}
			}
		}

		if(bMatchedEdge)
			break;
	}

	//make sure we matched an edge, because if we didn't, we can't merge the polygons
	if(!bMatchedEdge)
		return NULL;

	//ok, we matched an edge, now lets merge the polygons
	CDecalPoly* pNewPoly = new CDecalPoly;

	if(pNewPoly == NULL)
		return NULL;

	//first copy over all of the first polygon's vertices
	uint32 nCurrVert = (nMatchPt1 + 1) % pPoly1->m_nNumVerts;
	while(nCurrVert != nMatchPt1)
	{
		pNewPoly->AddVert(pPoly1->m_vVert[nCurrVert]);
		nCurrVert = (nCurrVert + 1)  % pPoly1->m_nNumVerts;
	}

	//now copy over all the second polygon's vertices
	nCurrVert = (nMatchPt2 + 1) % pPoly2->m_nNumVerts;
	while(nCurrVert != nMatchPt2)
	{
		pNewPoly->AddVert(pPoly2->m_vVert[nCurrVert]);
		nCurrVert = (nCurrVert + 1)  % pPoly2->m_nNumVerts;
	}

	//sanity check
	assert(pNewPoly->m_nNumVerts == (pPoly1->m_nNumVerts + pPoly2->m_nNumVerts - 2));

	//copy over all the extra information
	pNewPoly->m_Plane		= pPoly1->m_Plane;
	pNewPoly->m_pSrcBrush	= pPoly1->m_pSrcBrush;
	pNewPoly->m_nPlaneID	= pPoly1->m_nPlaneID;
	pNewPoly->m_nSrcPolyID	= pPoly1->m_nSrcPolyID;

	//see if this new polygon is valid...
	if(!IsPolygonConvex(pNewPoly))
	{
		//invalid poly
		delete pNewPoly;
		return NULL;
	}	

	//the polygon was successfully merged, we need to clean up unneeded points
	CleanupPolygonPts(pNewPoly);

	return pNewPoly;
}


//given a list of polygons it will attempt to merge as many as possible to reduce polygon counts
static void MergePolygons(CPolyArray& PolyList)
{
	//we need to create a quick plane map
	CMoArray<LTPlane>	PlaneList;
	PlaneList.SetCacheSize(PolyList.GetSize());

	//now setup the map, and clean up the polygons while we're at it
	for(uint32 nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CleanupPolygonPts(PolyList[nCurrPoly]);

		PolyList[nCurrPoly]->m_nPlaneID = (uint32)-1;

		//try and find a matching plane
		for(uint32 nCurrPlane = 0; nCurrPlane < PlaneList.GetSize(); nCurrPlane++)
		{
			if(PlaneList[nCurrPlane].m_Normal.NearlyEquals(PolyList[nCurrPoly]->m_Plane.m_Normal, 0.01f) &&
				(fabs(PlaneList[nCurrPlane].m_Dist - PolyList[nCurrPoly]->m_Plane.m_Dist) < 0.01f))
			{
				PolyList[nCurrPoly]->m_nPlaneID = nCurrPlane;
				break;
			}
		}

		//see if we didn't find one
		if(PolyList[nCurrPoly]->m_nPlaneID == (uint32)-1)
		{
			//none found, so add it
			PolyList[nCurrPoly]->m_nPlaneID = PlaneList.GetSize();
			PlaneList.Append(PolyList[nCurrPoly]->m_Plane);
		}
	}

	//now that the map is set up, we can run through and try and merge together all brushes
	bool bAnyMerged = true;
	while(bAnyMerged)
	{
		//assume none will be merged
		bAnyMerged = false;

		//now try and merge them
		for(uint32 nPoly1 = 0; nPoly1 < PolyList.GetSize(); nPoly1++)
		{
			for(uint32 nPoly2 = nPoly1 + 1; nPoly2 < PolyList.GetSize(); nPoly2++)
			{
				//don't bother if planes are different
				if(PolyList[nPoly1]->m_nPlaneID != PolyList[nPoly2]->m_nPlaneID)
					continue;

				//also don't bother if the source brushes are different (this can cause issues with the properties)
				if(PolyList[nPoly1]->m_pSrcBrush != PolyList[nPoly2]->m_pSrcBrush)
					continue;

				//try and merge the polygons
				CDecalPoly* pMerged = MergePolygons(PolyList[nPoly1], PolyList[nPoly2]);

				if(pMerged)
				{
					//successful merging, add it to our list
					delete PolyList[nPoly1];
					delete PolyList[nPoly2];

					//remove them from the lists (the -1 is to compensate for the shift caused
					//by the first removal)
					PolyList.Remove(nPoly1);
					PolyList.Remove(nPoly2 - 1);

					//shift back the first loop var
					nPoly1--;

					//append our new polygon onto the list
					PolyList.Append(pMerged);

					//flag success
					bAnyMerged = true;

					//now don't bother comparing any more polygons to this one since it is now gone
					break;
				}
			}
		}
	}
}

//Given a list of polygons and information on the projection, it will self shadow the polygons in the
//list, removing the shadowed polygons
static void OccludePolygons(CPolyArray& PolyList, const LTVector& vPos, const LTVector& vForward)
{
	//first off, update the polygon ID's, and create a duplicate list for our use
	CMoArray<CDecalPoly> Blockers;
	Blockers.SetSize(PolyList.GetSize());

	for(uint32 nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		PolyList[nCurrPoly]->m_nSrcPolyID = nCurrPoly;
		Blockers[nCurrPoly] = *PolyList[nCurrPoly];
	}

	//the list of edge planes for the polygon that is currently blocking
	uint32  nNumEdges;
	LTPlane EdgeList[CDecalPoly::MAX_VERTS + 1];
	uint32  nSide   [CDecalPoly::MAX_VERTS + 1];

	//now we need to run through every polygon in the list, and for each we need to create a frustum and
	//remove the parts of the polygons that lie behind it
	for(uint32 nCurrBlocker = 0; nCurrBlocker < Blockers.GetSize(); nCurrBlocker++)
	{
		//get the poly we will be working with
		CDecalPoly* pBlocker = &Blockers[nCurrBlocker];

		//alright, now we need to create our edge list
		nNumEdges = pBlocker->m_nNumVerts;
		for(uint32 nCurrEdge = 0; nCurrEdge < nNumEdges; nCurrEdge++)
		{
			//build up the plane we will be working with
			InitPlane(EdgeList[nCurrEdge],	pBlocker->m_vVert[nCurrEdge], 
											pBlocker->m_vVert[(nCurrEdge + 1) % pBlocker->m_nNumVerts],
											pBlocker->m_vVert[nCurrEdge] + vForward);
		}
		//add the polygon's plane in as well
		EdgeList[nNumEdges] = pBlocker->m_Plane;
		nNumEdges++;

		//now we need to run through and filter every single polygon behind the source through this edge.
		//Note that we go backwards since the active polygon can be removed
		for(int32 nClipPoly = PolyList.GetSize() - 1; nClipPoly >= 0; nClipPoly--)
		{
			CDecalPoly* pClipPoly = PolyList[nClipPoly];

			//don't bother if it is the same polygon as the one that is blocking
			if(pClipPoly->m_nSrcPolyID == pBlocker->m_nSrcPolyID)
				continue;

			//now we need to quickly see if this polygon is entirely in front of any of the planes...if it is, then
			//we can skip clipping it entirely

			//flag if we even need to test it
			bool bTestPoly		= true;

			//flag if it should be removed (is completely inside of frustum)
			bool bRemovePoly	= true;

			uint32 nCurrEdge;
			for(nCurrEdge = 0; nCurrEdge < nNumEdges; nCurrEdge++)
			{
				nSide[nCurrEdge] = GetPolySide(EdgeList[nCurrEdge], pClipPoly);
				
				//check for if we need to remove this poly (all sides are backside)
				if(nSide[nCurrEdge] & POLYSIDE_FRONT)
				{
					bRemovePoly = false;
					if(nSide[nCurrEdge] == POLYSIDE_FRONT)
					{
						bTestPoly = false;
						break;
					}
				}
			}

			//see if we don't need to bother with this poly
			if(!bTestPoly)
				continue;

			//see if this polygon needs to be removed
			if(bRemovePoly)
			{
				delete pClipPoly;
				pClipPoly = NULL;

				PolyList.Remove(nClipPoly);
				continue;
			}

			//ok, we know the polygon hits the frustum, so we need to split it on all intersecting planes
			
			//this is done by first removing it from our main list
			PolyList.Remove(nClipPoly);

			//and adding this to our list of backside polygons...
			CDecalPoly* pQueuePoly = pClipPoly;

			for(nCurrEdge = 0; nCurrEdge < nNumEdges; nCurrEdge++)
			{
				//if this is intersecting, we need to split it
				if(nSide[nCurrEdge] == POLYSIDE_INTERSECT)
				{
					//we need to clip this polygon, and add the back side of the polygon onto the end of the list
					CDecalPoly* pFront = new CDecalPoly;
					CDecalPoly* pBack  = new CDecalPoly;

					if((pFront == NULL) || (pBack == NULL))
						return;

					//clip the polygon
					ClipPolygon(*pQueuePoly, EdgeList[nCurrEdge], *pFront, *pBack);

					//keep track of the ID's
					pFront->m_nSrcPolyID = pClipPoly->m_nSrcPolyID;
					pBack->m_nSrcPolyID  = pClipPoly->m_nSrcPolyID;

					//remove the original polygon
					delete pQueuePoly;

					//some sanity checks
					if(pFront->m_nNumVerts > 2)
					{
						PolyList.Append(pFront);
					}
					else
					{
						delete pFront;
					}

					if(pBack->m_nNumVerts > 2)
					{
						pQueuePoly = pBack;
					}
					else
					{
						delete pBack;
						break;
					}
				}
			}
		}
	}

	//clean up the polygon list
	MergePolygons(PolyList);
}


//handles actual creation of decal geometry given a specified decal object. It will then
//create a bunch of brushes that will be placed under the specified decal object.
static bool CreateDecalGeometry(CBaseEditObj* pDecal, CWorldNode* pDecalParent, CEditRegion* pRegion, CDecalBrushArray& BrushList)
{
	assert(pDecal);
	assert(pDecalParent);

	//first we want to try and get the texture info. If the texture doesn't exist we can skip everything else
	const char* pszTexture = GetStringProp(pDecal, "Texture");

	if(pszTexture == NULL)
		return false;

	//we also need to make sure that there are no preceding slashes, this will mess up the
	//filename
	while((pszTexture[0] == '\\') || (pszTexture[0] == '/'))
		pszTexture++;

	//now get the texture dimensions
	uint32 nTextureWidth;
	uint32 nTextureHeight;

	if(!GetTextureDims(pszTexture, nTextureWidth, nTextureHeight))
	{
		//unable to find the texture
		DrawStatusText(eST_Error, "Unable to find the texture %s for the decal (Is working directory set?). Decal not created", pszTexture);
		return false;
	}


	//we first need to create the frustum of the decal

	//get the object's position
	LTVector vPos = pDecal->GetPos();

	//now the orientation vectors
	LTVector vRot = pDecal->GetOr();
	LTMatrix OrMat;
	gr_SetupMatrixEuler( vRot, OrMat.m );

	LTVector vUp, vRight, vForward;
	OrMat.GetBasisVectors( &vRight, &vUp, &vForward );

	//now we need to get the property information
	float fNearPlane = GetRealProp(pDecal, "NearClip", 1.0f);
	float fFarPlane  = GetRealProp(pDecal, "FarClip", 2.0f);

	//half of the frustum dimensions
	float fHorzDist = GetRealProp(pDecal, "Width", 1.0f) / 2.0f;
	float fVertDist = GetRealProp(pDecal, "Height", 1.0f) / 2.0f;

	//see if they have any vectors that specify the projection dims
	CBaseProp* pDims = pDecal->GetPropertyList()->GetProp("ProjectDims");
	if(pDims && pDims->GetType() == LT_PT_VECTOR)
	{
		//we have a dims vector, this should override the base properties
		fHorzDist = ((CVectorProp*)pDims)->m_Vector.x / 2.0f;
		fVertDist = ((CVectorProp*)pDims)->m_Vector.y / 2.0f;
		fFarPlane = ((CVectorProp*)pDims)->m_Vector.z;
	}		

	//make sure the values are valid
	fNearPlane	= LTMAX(fNearPlane, 0.0f);
	fFarPlane	= LTMAX(fNearPlane, fFarPlane);
	fHorzDist	= LTMAX(fHorzDist, 0.1f);
	fVertDist	= LTMAX(fVertDist, 0.1f);

	//we can now create actual frustum vertices
	LTVector vUL = vPos + vForward * fNearPlane - vRight * fHorzDist + vUp * fVertDist;
	LTVector vUR = vPos + vForward * fNearPlane + vRight * fHorzDist + vUp * fVertDist;
	LTVector vBL = vPos + vForward * fNearPlane - vRight * fHorzDist - vUp * fVertDist;
	LTVector vBR = vPos + vForward * fNearPlane + vRight * fHorzDist - vUp * fVertDist;

	//create the outer frustum planes
	CFrustum Frustum;
	InitPlane(Frustum.m_Planes[0], vUL + vForward, vUR, vUL);
	InitPlane(Frustum.m_Planes[1], vUR + vForward, vBR, vUR);
	InitPlane(Frustum.m_Planes[2], vBR + vForward, vBL, vBR);
	InitPlane(Frustum.m_Planes[3], vBL + vForward, vUL, vBL);

	//now init the near and far planes
	InitPlane(Frustum.m_Planes[4], vBR, vUL, vUR);
	InitPlane(Frustum.m_Planes[5], vUL, vBR, vUR);

	//adjust the distance of the far plane, since we used near plane points
	Frustum.m_Planes[5].m_Dist -= fFarPlane - fNearPlane;

	//the list of polygons that intersect the frustum
	CPolyArray PolyList;
	PolyList.SetCacheSize(256);

	//get the amount we want to offset
	bool  bAddWorldModels = GetBoolProp(pDecal, "HitWorldModels", true);

	//one other thing we need to do is to read in the list of brushes that are excluded
	//from this decal, so that artists can override some of the behavior.
	const char* pszIgnore = GetStringProp(pDecal, "IgnoreBrushes", "");

	//now we need to break that apart into a listing of brushes
	CString sIgnoreList[MAX_IGNORE_BRUSHES];

	uint32 nCurrString = 0;

	//parse the string
	for(uint32 nCurrChar = 0; pszIgnore[nCurrChar]; nCurrChar++)
	{
		char ch = pszIgnore[nCurrChar];

		if((ch == ',') || (ch == ';'))
		{
			//clean up the old string
			sIgnoreList[nCurrString].TrimRight();
			sIgnoreList[nCurrString].TrimLeft();

			nCurrString++;

			//bail if out of room...
			if(nCurrString >= MAX_IGNORE_BRUSHES)
			{
				DrawStatusText(eST_Warning, "More than %d excluded brushes on decal %s, ignoring end excludes", MAX_IGNORE_BRUSHES, pDecal->GetName());
				break;
			}
		}
		else
		{
			sIgnoreList[nCurrString] += ch;
		}
	}

	//we are done. We need to add an aditional string if we were offset in the last string
	if(!sIgnoreList[nCurrString].IsEmpty())
		nCurrString++;

	//now we get to build up the list of clipped polygons that are in the frustum
	BuildPolygonListR(pRegion->GetRootNode(), PolyList, Frustum, vForward, bAddWorldModels, sIgnoreList, nCurrString);

	//now we need to extrude the polygons along the appropriate normals
	float fOffset = GetRealProp(pDecal, "Offset", 0.1f);
	OffsetPolygons(PolyList, fOffset);

	//and now we clip them so that they are entirely contained within the frustum
	ClipPolygonsToFrustum(PolyList, Frustum);

	//alright, now we need to see if the decal is clipped. If so, we need to cull out
	//occluded pieces
	if(GetBoolProp(pDecal, "Occlude"))
	{
		//we need to clip the fragments yet again, this time to all the other fragments
		//that are behind it

		OccludePolygons(PolyList, vPos, vForward);
	}

	//we need to create the texture space of the decal
	LTVector vO, vP, vQ;

	//init it to the space at the near coordinate plane
	vO = vUL;
	vP = vUR - vUL;
	vQ = vBL - vUL;

	//offset the O accordingly
	float fUOffset = GetRealProp(pDecal, "UOffset", 0.0f);
	float fVOffset = GetRealProp(pDecal, "VOffset", 0.0f);

	vO += vP * fUOffset;
	vO += vQ * fVOffset;

	//scale it appropriately for the PQ since they represent a single pixel increment
	vP *= nTextureWidth / vP.MagSqr();
	vQ *= nTextureHeight / vQ.MagSqr();

	//alright, we have our final polygon list, so we need to generate the brushes
	CreateBrushes(	PolyList, pDecalParent, pRegion, pszTexture, 
					vO, vP, vQ, BrushList, GetStringProp(pDecal, "TextureEffect"));

	//clean up the polygon list
	DeleteAndClearArray(PolyList);

	return true;
}

//given a decal brush list this will go through all the brushes and actually place them
//into the world
static void AttachBrushesToWorld(CEditRegion* pRegion, CDecalBrushArray& BrushList)
{
	for(uint32 nCurrBrush = 0; nCurrBrush < BrushList.GetSize(); nCurrBrush++)
	{
		no_AttachNode(pRegion, BrushList[nCurrBrush].m_pBrush, BrushList[nCurrBrush].m_pParent);
	}
}

//handles instantiating all of the decals. If we are importing geometry, it will also
//handle creating the geometry for the decals
bool CreateDecals(CEditRegion* pRegion, bool bCreateGeometry)
{
	//we need to run through the node heirarchy and build up our list of decals
	CObjectArray Decals;
	if(!RemoveDecalObjectsR(pRegion->GetRootNode(), pRegion, Decals))
	{
		DeleteAndClearArray(Decals);
		return false;
	}

	//see if that is all we need to do
	if(!bCreateGeometry)
	{
		DeleteAndClearArray(Decals);
		return true;
	}

	//our list of decal brushes that we will build up
	CDecalBrushArray BrushList;
	BrushList.SetCacheSize(256);

	//now we need to actually create the brushes for the decal world models
	for(uint32 nCurrDecal = 0; nCurrDecal < Decals.GetSize(); nCurrDecal++)
	{
		//get the object that we will be creating
		CBaseEditObj* pDecal = Decals[nCurrDecal];

		//now create some geometry
		CreateDecalGeometry(pDecal, pDecal->GetParent(), pRegion, BrushList);
	}
	
	//now we need to add our brushes into the actual world
	AttachBrushesToWorld(pRegion, BrushList);

	DeleteAndClearArray(Decals);
	return true;
}
