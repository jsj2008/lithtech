#include "bdefs.h"
#include "EditObjects.h"
#include "EditRegion.h"
#include "EditPoly.h"
#include "PolyLightMap.h"
#include "LMLight.h"
#include "DEditLightMapMgr.h"


//utility function to determine if the specified light object is an omni light or not
static inline bool IsOmni(CBaseEditObj* pObj)
{
	return strcmp(pObj->GetClassName(), "Light") ? false : true;
}



CDEditLightMapMgr::CDEditLightMapMgr(CEditRegion* pRegion) :
	m_pRegion(pRegion),
	m_bEnabled(false),
	m_bLightMap(false),
	m_bLambertian(false),
	m_bSupressDirtying(false),
	m_bShadows(false)
{
	//we MUST have a valid region
	ASSERT(m_pRegion);

	//set up the shadow calculator
	m_Generator.m_SampleGen.m_ShadowCalc.SetRegion(pRegion);
}

CDEditLightMapMgr::~CDEditLightMapMgr()
{
}

//called when a light is dirtied, this will dirty all appropriate brushes
void CDEditLightMapMgr::UpdateLight(CBaseEditObj* pLight)
{
	if(!IsLightingEnabled() || m_bSupressDirtying)
		return;

	//find the light that refers to this object
	CLMLight* pLMLight = m_Generator.m_SampleGen.GetLight(pLight);

	if(pLMLight)
	{
		//we have a match, so first, invalidate the light
		DirtyLight(pLMLight);

		//now we need to update the light properties
		m_Generator.m_SampleGen.ConvertLight(pLight, pLMLight, IsOmni(pLight));

		//now update any new brushes it may have dirtied
		DirtyLight(pLMLight);
	}		
}

//called to dirty all brushes (usually in response to loading a level
//or changing settings)
void CDEditLightMapMgr::DirtyAll()
{
	if(!IsLightingEnabled() || m_bSupressDirtying)
		return;

	//run through all the brushes
	for(LPOS pos = m_pRegion->m_Brushes; pos; )
	{
		//get the brush
		CEditBrush* pBrush = m_pRegion->m_Brushes.GetNext(pos);

		DirtyBrushSimple(pBrush);
	}
}

//given an actual light structure, it will dirty the appropriate brushes
void CDEditLightMapMgr::DirtyLight(const CLMLight* pLight)
{
	if(!IsLightingEnabled() || m_bSupressDirtying)
		return;

	//run through all the brushes
	for(LPOS pos = m_pRegion->m_Brushes; pos; )
	{
		CEditBrush* pBrush = m_pRegion->m_Brushes.GetNext(pos);
	
		//dirty the appropriate part of this brush (if any)
		DirtyBrushFromLight(pLight, pBrush);
	}
}

//called to add a light to the list
void CDEditLightMapMgr::AddLight(CBaseEditObj* pLight)
{
	//first off, create the light object for this light
	CLMLight* pNewLight = new CLMLight;

	//make sure the allocation worked
	if(pNewLight == NULL)
		return;

	//convert this light object to the light structure
	m_Generator.m_SampleGen.ConvertLight(pLight, pNewLight, IsOmni(pLight));

	//now add this onto our list
	m_Generator.m_SampleGen.AddLight(pNewLight);

	//now dirty all of the brushes that touch this new light
	DirtyLight(pNewLight);
}

//called to remove a light from the list
void CDEditLightMapMgr::RemoveLight(CBaseEditObj* pLight)
{
	//first off find the light
	CLMLight* pLMLight = m_Generator.m_SampleGen.GetLight(pLight);

	if(pLMLight)
	{
		//found the light.

		//first dirty all the brushes it was lighting
		DirtyLight(pLMLight);

		//now remove it from the list
		m_Generator.m_SampleGen.DeleteLight(pLMLight);
	}
}

//called to dirty a brush
void CDEditLightMapMgr::DirtyBrush(CEditBrush* pBrush)
{
	if(!IsLightingEnabled() || m_bSupressDirtying)
		return;

	ASSERT(pBrush);

	DirtyBrushSimple(pBrush);

	//now if we are doing shadows, we need to find all the lights that hit this brush,
	//and invalidate each of them...

	if(m_bShadows)
	{
		const CLMLight* pCurr = m_Generator.m_SampleGen.GetHeadLight();

		while(pCurr)
		{
			//see if it intersects this brush
			if(pBrush->m_BoundingSphere.IntersectsSphere(pCurr->m_vPos, pCurr->m_fRadius))
			{
				DirtyLight(pCurr);
			}

			pCurr = pCurr->m_pNext;
		}
	}
}

void CDEditLightMapMgr::DirtyBrushSimple(CEditBrush* pBrush)
{
	//first off just dirty the brush as well as the lightmaps if applicable
	if(!pBrush->IsInDirtyList())
	{
		m_Generator.DirtyVertexHolder(pBrush);
		pBrush->AddedToDirtyList();
	}

	//now dirty the faces
	if(pBrush->IsLightmapped() && IsLightMappingEnabled())
	{
		for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
		{
			CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];

			//skip over already known to be dirty polies
			if(pPoly->m_pLightMap)
			{
				if(!pPoly->m_pLightMap->IsInDirtyList())
				{
					//we know it isn't in the list, so we can just add it
					m_Generator.DirtyLightMapHolder(pPoly, false);

					pPoly->m_pLightMap->AddedToDirtyList();
				}
			}
			else
			{
				//we need to make sure that it isn't in the list, but add it if it isn't
				m_Generator.DirtyLightMapHolder(pPoly, true);
			}
		}
	}
}

//called to remove a brush entirely from the lightmap calculator (this should
//be called before a brush is deleted)
void CDEditLightMapMgr::RemoveBrush(CEditBrush* pBrush)
{
	ASSERT(pBrush);

	//first off remove the brush from the vertex light holder
	m_Generator.RemoveVertexHolder(pBrush);

	//now we need to remove all faces (regardless, incase the user switched lightmapping)
	for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
	{
		CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];

		if((pPoly->m_pLightMap == NULL) || (pPoly->m_pLightMap->IsInDirtyList()))
		{
			//this polygon could be in the dirty list, remove it...
			m_Generator.RemoveLightMapHolder(pBrush->m_Polies[nCurrPoly]);
		}
	}

	//also make sure that no lights are pointing to this brush for coherency reasons
	CLMLight* pCurr = (CLMLight*)m_Generator.m_SampleGen.GetHeadLight();

	while(pCurr)
	{
		//see if it intersects this brush
		if(pCurr->m_pLastHitBrush == pBrush)
		{
			pCurr->m_pLastHitBrush = NULL;
		}

		pCurr = pCurr->m_pNext;
	}


}

//given a light, it will go through the specified light and mark the appropriate
//polygons as lightmapped, and possibly the brush 
void CDEditLightMapMgr::DirtyBrushFromLight(const CLMLight* pLight, CEditBrush* pBrush)
{
	//sanity check
	ASSERT(pLight);
	ASSERT(pBrush);

	//first off do a comparison of bounding spheres
	if(!pBrush->m_BoundingSphere.IntersectsSphere(pLight->m_vPos, pLight->m_fRadius))
	{
		//too far away
		return;
	}

	//now we can do a frustum cull if the light is a spotlight (but make sure that the
	//half angle is less than 90, this prevents a lot of algorithm screw ups)
	if(!pLight->m_bIsOmni && (pLight->m_fCosHalfAngle > 0.01f))
	{
		//get the vector to the sphere
		LTVector vToSphere = pBrush->m_BoundingSphere.GetPos() - pLight->m_vPos;
		
		float fProjLen = vToSphere.Dot(pLight->m_vDir);

		//see if it entirely in the back
		if(fProjLen <= -pBrush->m_BoundingSphere.GetRadius())
		{
			//the brush is behind the light
			return;
		}

		//find the length of the vector that is perpindicular squared
		float fDistPerpSqr = (float)sqrt(vToSphere.MagSqr() - fProjLen * fProjLen);

		float fDistToCone = (fDistPerpSqr - fProjLen * pLight->m_fTanHalfAngle) * 
							pLight->m_fCosHalfAngle;

		//see if we are outside of the cone
		if(fDistToCone >= pBrush->m_BoundingSphere.GetRadius())
		{
			//we are outside the cone
			return;
		}		
	}

	//bail don't bother if it is already in the list
	if(!pBrush->IsInDirtyList())
	{

		//now need to do a vertex check. If no vertices are dirty, then we don't need to
		//register this for vertex processing
		for(uint32 nCurrVert = 0; nCurrVert < pBrush->m_Points.GetSize(); nCurrVert++)
		{
			if((pBrush->m_Points[nCurrVert] - pLight->m_vPos).MagSqr() < pLight->m_fRadiusSqr)
			{
				//a vertex is in the light.....update this
				m_Generator.DirtyVertexHolder(pBrush);
				pBrush->AddedToDirtyList();

				//no need to check more
				break;
			}
		}
	}

	//we can bail now if this brush doesn't support lightmapping
	if(!pBrush->IsLightmapped() || !IsLightMappingEnabled())
	{
		return;
	}

	CEditPoly*	pPoly;
	float		fDist;

	//now we get to update all the polygons
	for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
	{
		//cache the poly
		pPoly = pBrush->m_Polies[nCurrPoly];

		//if it is already dirty, bail
		if(pPoly->m_pLightMap)
		{
			if(pPoly->m_pLightMap->IsInDirtyList())
			{
				continue;
			}
		}

		//find the distance
		fDist = pPoly->Normal().Dot(pLight->m_vPos) - pPoly->Dist();

		//first off do a distance check
		if(fDist > pLight->m_fRadius)
			continue;

		//do back face culling if appropriate
		if(m_bLambertian || m_bShadows)
		{
			if(fDist < 0.0f)
				continue;
		}
		else
		{
			//bail if it is back too far
			if(fDist < -pLight->m_fRadius)
				continue;
		}

		//now see if the light intersects the polygon at all (this is a somewhat crude
		//approximation)
		bool bInPoly = true;

		uint32 nPrevVert = pPoly->NumVerts() - 1;
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nPrevVert = nCurrVert, nCurrVert++)
		{
			//we need to find the edge normal and determine if the sphere is in front
			//of that plane. If it is, it doesn't intersect the poly
			LTVector vEdgeN = (pPoly->Pt(nCurrVert) - pPoly->Pt(nPrevVert)).Cross(pPoly->Normal());
			vEdgeN.Norm();

			if(vEdgeN.Dot(pLight->m_vPos - pPoly->Pt(nCurrVert)) < -pLight->m_fRadius)
			{
				bInPoly = false;
				break;
			}
		}

		if(bInPoly)
		{
			//we need to dirty this polygon

			//if we have a lightmap, it flags for sure if it is in the list or not
			if(pPoly->m_pLightMap)
			{
				m_Generator.DirtyLightMapHolder(pPoly, false);
			}
			else
			{
				//we don't have a lightmap, so we can't know for sure if it is clean, 
				//meaning we have to search
				m_Generator.DirtyLightMapHolder(pPoly, true);
			}

			
			if(pPoly->m_pLightMap)
			{
				pPoly->m_pLightMap->AddedToDirtyList();
			}
		}
	}
}

//called to clear all the holder lists of all items, flushing out the list of
//objects to be processed
void CDEditLightMapMgr::RemoveAll()
{
	//first clear out all of the region's lighting data (note that this also clears the
	//polygon in dirty list flag since that is held in the lightmaps that are free'd)
	m_pRegion->FreeLightingData();

	//run through the vertex list
	for(uint32 nCurrHolder = 0; nCurrHolder < m_Generator.m_VertexList.GetNumHolders(); nCurrHolder++)
	{
		//get the polygon
		CEditBrush* pBrush = (CEditBrush*)m_Generator.m_VertexList.GetHolder(nCurrHolder);

		//clear the flag
		pBrush->ClearFlag(BRUSHFLAG_INDIRTYLIST);
	}

	//now we clear out the lists
	m_Generator.m_VertexList.Free();
	m_Generator.m_LightMapList.Free();
}

//sees if lighting is enabled
bool CDEditLightMapMgr::IsLightingEnabled() const
{
	return m_bEnabled;
}

//sets whether or not lighting is enabled
void CDEditLightMapMgr::EnableLighting(bool bVal)
{
	m_bEnabled = bVal;
}

//sees if lightmapping is enabled]
bool CDEditLightMapMgr::IsLightMappingEnabled() const
{
	return m_bLightMap;
}

//specifies if lightmapping should be enabled or not
void CDEditLightMapMgr::EnableLightMapping(bool bVal)
{
	m_bLightMap = bVal;
}

//determines if lambertian lighting is being used
bool CDEditLightMapMgr::IsLambertianLighting() const
{
	return m_bLambertian;
}

//specifies whether or not to use lambertian lighting. note that this should
//be used as opposed to the sample generation. This will call through to that
//but this also needs to know about it for culling reasons
void CDEditLightMapMgr::EnableLambertianLighting(bool bVal)
{
	m_bLambertian = bVal;

	//call through to the sample generator
	m_Generator.m_SampleGen.SetLambertian(bVal);
}

//determines if shadows are enabled
bool CDEditLightMapMgr::IsShadowsEnabled() const
{
	return m_bShadows;
}

//specifies if shadows are enabled
void CDEditLightMapMgr::EnableShadows(bool bVal)
{
	m_bShadows = bVal;

	//call through to sample generator
	m_Generator.m_SampleGen.SetShadows(bVal);
}

//enables supression of dirtying of objects. This should be done when everything is
//going to be dirtied and then need to be dirtied again. (i.e. while loading a level)
void CDEditLightMapMgr::SupressDirtying(bool bVal)
{
	m_bSupressDirtying = bVal;
}


