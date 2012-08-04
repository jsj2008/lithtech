#include "bdefs.h"
#include "detectors.h"

//-------------------------------------HELPERS----------------------------------------------
static bool GetBoolProp(CPropList* pList, const char* pPropName, bool bDefault = false)
{
	ASSERT(pList);

	CBaseProp* pProp = pList->GetProp(pPropName);

	if((pProp == NULL) || (pProp->GetType() != LT_PT_BOOL))
		return bDefault;

	return ((CBoolProp*)pProp)->m_Value ? true : false;
}

static CReal GetRealProp(CPropList* pList, const char* pPropName, CReal fDefault = 0.0f)
{
	ASSERT(pList);

	CBaseProp* pProp = pList->GetProp(pPropName);

	if(pProp == NULL)
		return fDefault;

	if((pProp->GetType() == LT_PT_REAL) || (pProp->GetType() == LT_PT_LONGINT))
		return ((CRealProp*)pProp)->m_Value;

	//not a valid type
	return fDefault;
}

static LTVector GetVectorProp(CPropList* pList, const char* pPropName, LTVector vDefault = LTVector(0, 0, 0))
{
	ASSERT(pList);

	CBaseProp* pProp = pList->GetProp(pPropName);

	if(pProp == NULL)
		return vDefault;

	if(pProp->GetType() == LT_PT_VECTOR)
		return ((CVectorProp*)pProp)->m_Vector;

	//not a valid type
	return vDefault;
}

enum ELightType
{
	eLight_Flat,
	eLight_Gouraud,
	eLight_LightMap,
	eLight_ShadowMesh
};

static ELightType GetLightingType(CEditBrush* pBrush)
{
	const ELightType keDefLightType = eLight_Gouraud;

	//find the lighting string
	CBaseProp* pProp = pBrush->m_PropList.GetProp("Lighting");

	if(!pProp)
		return keDefLightType;

	if(pProp->GetType() != LT_PT_STRING)
		return keDefLightType;

	const char* pszStr = ((CStringProp*)pProp)->m_String;

	//convert the string
	if(stricmp(pszStr, "Flat") == 0)
		return eLight_Flat;
	else if(stricmp(pszStr, "Gouraud") == 0)
		return eLight_Gouraud;
	else if(stricmp(pszStr, "Lightmap") == 0)
		return eLight_LightMap;
	else if(stricmp(pszStr, "ShadowMesh") == 0)
		return eLight_ShadowMesh;

	return keDefLightType;
}

enum EBrushType
{
	eBrush_Normal,
	eBrush_SkyPortal,
	eBrush_Occluder,
	eBrush_RBSplitter,
	eBrush_RenderOnly,
	eBrush_Blocker,
	eBrush_NonSolid,
	eBrush_ParticleBlocker
};

static EBrushType GetBrushType(CEditBrush* pBrush)
{
	const EBrushType keDefBrushType = eBrush_Normal;

	//find the lighting string
	CBaseProp* pProp = pBrush->m_PropList.GetProp("Type");

	if(!pProp)
		return keDefBrushType;

	if(pProp->GetType() != LT_PT_STRING)
		return keDefBrushType;

	const char* pszStr = ((CStringProp*)pProp)->m_String;

	//convert the string
	if(stricmp(pszStr, "Normal") == 0)
		return eBrush_Normal;
	else if(stricmp(pszStr, "SkyPortal") == 0)
		return eBrush_SkyPortal;
	else if(stricmp(pszStr, "Occluder") == 0)
		return eBrush_Occluder;
	else if(stricmp(pszStr, "RBSplitter") == 0)
		return eBrush_RBSplitter;
	else if(stricmp(pszStr, "RenderOnly") == 0)
		return eBrush_RenderOnly;
	else if(stricmp(pszStr, "Blocker") == 0)
		return eBrush_Blocker;
	else if(stricmp(pszStr, "NonSolid") == 0)
		return eBrush_NonSolid;
	else if(stricmp(pszStr, "ParticleBlocker") == 0)
		return eBrush_ParticleBlocker;

	return keDefBrushType;
}

//----------------------------------CONCAVE POLY DETECTOR----------------------------------
CConcavePolyDetector::CConcavePolyDetector()	: 
	CErrorDetector("Detects concave polygons in a scene")	
{
	m_bNotifyPoly  = true;
}

bool CConcavePolyDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	if(pPoly->IsConcave())
	{
		AddNewError(new CLevelError("Concave Poly", ERRORSEV_CRITICAL, pBrush,
					"This brush was found to have concave polygons. This means that the poly has an indentation. To resolve this issue, it should be split so that all points lie on each side of the polygon\'s forming planes.", 
					this));
		return false;
	}
	return true;
}

//----------------------------------NONPLANAR DETECTOR----------------------------------
CNonplanarDetector::CNonplanarDetector()	: 
	CErrorDetector("Detects non-planar polygons in a scene")	
{
	m_bNotifyPoly  = true;
}

bool CNonplanarDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	if(!pPoly->IsCoplanar())
	{
		AddNewError(new CLevelError("Non-Planar Poly", ERRORSEV_CRITICAL, pBrush,
					"This brush was found to have non-planar polygons. This means that the poly does not lie within a single plane. This polygon must either be subdivided into coplanar pieces, or have its vertices moved into the plane", 
					this));
		return false;
	}
	return true;
}

//----------------------------------POLY EDGE LENGTH----------------------------------
CPolyEdgeLenDetector::CPolyEdgeLenDetector()	: 
	CErrorDetector("Detects polygon edges that are less than one unit long")	
{
	m_bNotifyPoly  = true;
}

bool CPolyEdgeLenDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	uint32 nNumPts = pPoly->NumVerts();

	if(nNumPts < 3)
		return true;

	uint32 nPrevPt = nNumPts - 1;
	for(uint32 nCurrPt = 0; nCurrPt < nNumPts; nPrevPt = nCurrPt, nCurrPt++)
	{
		if((pPoly->m_pBrush->m_Points[pPoly->Index(nPrevPt)] - pPoly->m_pBrush->m_Points[pPoly->Index(nCurrPt)]).MagSqr() < 1.0f)
		{
			AddNewError(new CLevelError("Short Polygon Edge", ERRORSEV_HIGH, pBrush,
						"This brush was found to have a polygon with an edge shorter than one unit. It is not recommended to have edges this small as they can cause issues to arise at processing time. The edge should either be removed or expanded.", 
						this));
			return false;
		}
	}

	return true;
}

//----------------------------------POINTS OFF GRID DETECTOR----------------------------------
CPointsOffGridDetector::CPointsOffGridDetector()	: 
	CErrorDetector("Detects brushes that have vertices that do not lie on the grid")	
{
	m_bNotifyBrush = true;
}

void CPointsOffGridDetector::OnBrush(CEditBrush* pBrush)
{
	for(uint32 nCurrPt = 0; nCurrPt < pBrush->m_Points.GetSize(); nCurrPt++)
	{
		//see how far away this point is from a grid version
		LTVector vPt = pBrush->m_Points[nCurrPt];
		vPt.x = fabs(vPt.x);
		vPt.y = fabs(vPt.y);
		vPt.z = fabs(vPt.z);
		LTVector vDist(vPt.x - (int)(vPt.x + 0.5f), vPt.y - (int)(vPt.y + 0.5f), vPt.z - (int)(vPt.z + 0.5f));

		if(vDist.MagSqr() > 0.01)
		{
			AddNewError(new CLevelError("Points Off Grid", ERRORSEV_LOW, pBrush,
						"This brush was found to have points off of the grid. This can cause issues such as polygons dropping off during processing, nonplanar surfaces, concavity and other errors. It is recommended that all points on this brush be moved onto the grid.", 
						this));
			//don't want to add this brush over and over, so bail
			return;
		}
	}
}

//----------------------------------REDUNDANT POINTS DETECTOR----------------------------------
CRedundantPointsDetector::CRedundantPointsDetector()	: 
	CErrorDetector("Detects brushes that have redundant points")	
{
	m_bNotifyBrush = true;
}

void CRedundantPointsDetector::OnBrush(CEditBrush* pBrush)
{
	for(uint32 nCurrPt = 0; nCurrPt < pBrush->m_Points.GetSize(); nCurrPt++)
	{
		//compare this to all the other points
		for(uint32 nTestPt = 0; nTestPt < pBrush->m_Points.GetSize(); nTestPt++)
		{
			//don't test against the same one
			if(nTestPt == nCurrPt)
				continue;

			//see if it is the same point
			if((pBrush->m_Points[nTestPt] - pBrush->m_Points[nCurrPt]).MagSqr() < 0.01f)
			{
				AddNewError(new CLevelError("Rundant Points", ERRORSEV_HIGH, pBrush,
						"This brush was found to multiple points in the same position. This could be caused potentially by a very short polygon edge or by bad brush construction. It is recommended that this brush be recreated.", 
						this));
				return;
			}
		}
	}
}

//----------------------------------SMALL POLY SIZE----------------------------------
CSmallPolySizeDetector::CSmallPolySizeDetector()	: 
	CErrorDetector("Detects polygons with an area less than 4 square units")	
{
	m_bNotifyPoly  = true;
}

bool CSmallPolySizeDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	CReal fArea = pPoly->GetSurfaceArea();

	if(fArea < 4.0f)
	{
		AddNewError(new CLevelError("Small Poly Size", ERRORSEV_HIGH, pBrush,
					"This brush was found to have a polygon with surface area less than four square units. Small polygons can often be corrupted during processing time and cause issues in levels with regards to visibility. It is recommended that this polygon be removed or enlarged.", 
					this));
		return false;
	}
	return true;
}

//----------------------------------INVALID POLY COUNT DETECTOR----------------------------------
CInvalidPolyCountDetector::CInvalidPolyCountDetector()	: 
	CErrorDetector("Detects brushes that have an invalid number of polygons associated with it")	
{
	m_bNotifyBrush = true;
}

void CInvalidPolyCountDetector::OnBrush(CEditBrush* pBrush)
{
	uint32 nNumPolies = pBrush->m_Polies.GetSize();

	//the number of polygons should be at least 1
	if(nNumPolies == 0)
	{
		AddNewError(new CLevelError("Invalid Poly Count", ERRORSEV_HIGH, pBrush,
					"This brush was found to no polygons. The number of polygons should be at least 1.", 
					this));
	}
}

//----------------------------------LARGE LIGHTMAPPED POLY----------------------------------
CLargeLightmappedPoliesDetector::CLargeLightmappedPoliesDetector()	: 
	CErrorDetector("Detects polygons a large surface area that have lighmapping enabled")	
{
	m_bNotifyPoly  = true;
}

bool CLargeLightmappedPoliesDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	CReal fArea = pPoly->GetSurfaceArea();

	if((fArea > 1000000.0f) && (GetLightingType(pBrush) == eLight_LightMap))
	{
		AddNewError(new CLevelError("Large Lightmapped Poly", ERRORSEV_LOW, pBrush,
					"This brush was found to have a polygon with substantial surface area that is marked as being lightmapped. This is valid, but can sometimes be undesired since it causes more lightmap memory to be consumed. Often this occurs accidentally by leaving lightmapping on sky portals.", 
					this));
		return false;
	}
	return true;
}

//----------------------------------SMALL LM GRIDSIZE DETECTOR----------------------------------
CSmallLMGridSizeDetector::CSmallLMGridSizeDetector()	: 
	CErrorDetector("Detects brushes with an LM grid size of 4 or less")	
{
	m_bNotifyBrush = true;
}

void CSmallLMGridSizeDetector::OnBrush(CEditBrush* pBrush)
{
	//get the LM Grid size
	CReal fGridSize = GetRealProp(&pBrush->m_PropList, "LMGridSize");

	if((fGridSize >= 0.8f) && (fGridSize < 4.8f) && (GetLightingType(pBrush) == eLight_LightMap))
	{
		AddNewError(new CLevelError("Small LM Grid Size", ERRORSEV_LOW, pBrush,
					"This brush was found to have a very small LM grid size. Small LM grid sices produce very high resolution lighting but take up large amounts of lightmap texture memory, add lots of polygons and drastically increase level processing time. This may be desired, but if it is not, increase the LMGridSize field. A good value is around 16.", 
					this));
	}
}

//----------------------------------INVALID SUNLIGHT DETECTOR--------------------------------
CInvalidSunlightDetector::CInvalidSunlightDetector()	: 
	CErrorDetector("Detects duplicate sunlight objects as well as sunlight objects that do not serve a purpose")	
{
	m_bNotifyObject		= true;
	m_bNotifyBrush		= true;
}

bool CInvalidSunlightDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	//search for sky portals and sunlights
	m_nNumSunlights		= 0;
	m_nNumSkyPortals	= 0;

	if(CErrorDetector::InternalBuildErrorList(pDoc) == false)
		return false;

	//see if the sunlight serves a purpose
	if((m_nNumSunlights > 0) && (m_nNumSkyPortals == 0))
	{
		AddNewError(new CLevelError("Sunlight but no sky portals", ERRORSEV_MEDIUM, NULL,
					"There is a sunlight object in your level, but there are no sky portals. This will cause processing time of the level to increase significantly. The sunlight object should be removed from the level.", 
					this));
	}

	return true;
}

void CInvalidSunlightDetector::OnObject(CBaseEditObj* pObj)
{
	if(stricmp(pObj->GetClassName(), "StaticSunLight") == 0)
	{
		m_nNumSunlights++;
		if(m_nNumSunlights > 1)
		{
			AddNewError(new CLevelError("Redundant Sunlight", ERRORSEV_MEDIUM, pObj,
					"This object is an additional sunlight. Only one sunlight is allowed per level and additional sunlights serve no purpose. To resolve this issue, the additional sunlights should be removed from the level.", 
					this));
		}
	}
}

void CInvalidSunlightDetector::OnBrush(CEditBrush* pBrush)
{
	if(GetBrushType(pBrush) == eBrush_SkyPortal)
	{
		m_nNumSkyPortals++;
	}
}

//---------------------------LIGHTMAPPED SKYPORTAL DETECTOR------------------------------
CLightmappedSkyportalDetector::CLightmappedSkyportalDetector()	: 
	CErrorDetector("Detects brushes marked with both skyportal and lightmap")	
{
	m_bNotifyBrush = true;
}

void CLightmappedSkyportalDetector::OnBrush(CEditBrush* pBrush)
{
	if(	(GetLightingType(pBrush) == eLight_LightMap) && (GetBrushType(pBrush) == eBrush_SkyPortal))
	{
		AddNewError(new CLevelError("Lightmapped Skyportal", ERRORSEV_MEDIUM, pBrush,
					"This brush is marked as both being lightmapped and being a sky portal. Sky portals cannot be lightmapped and this will only result in increased processing time and level file size. LightMap should be set to false.", 
					this));
	}
}

//---------------------------OBJECTS OUTSIDE LEVEL DETECTOR------------------------------
CObjectsOutsideLevelDetector::CObjectsOutsideLevelDetector()	: 
	CErrorDetector("Detects objects that are outside of the level\'s bounding box. (ignores static sunlight and outside definition objects)")	
{
}


bool CObjectsOutsideLevelDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	m_vBoxMin.Init((CReal)MAX_CREAL, (CReal)MAX_CREAL, (CReal)MAX_CREAL);
	m_vBoxMax = -m_vBoxMin;

	//first build the bounding box
	m_bNotifyBrush	= true;
	m_bNotifyObject = false;
	if(CErrorDetector::InternalBuildErrorList(pDoc) == false)
		return false;

	//now do the checking for outside objects
	m_bNotifyBrush	= false;
	m_bNotifyObject = true;
	return CErrorDetector::InternalBuildErrorList(pDoc);
}

void CObjectsOutsideLevelDetector::OnBrush(CEditBrush* pBrush)
{
	//adjust the bounding box to encompass all brush points
	for(uint32 nCurrPt = 0; nCurrPt < pBrush->m_Points.GetSize(); nCurrPt++)
	{
		VEC_MIN(m_vBoxMin, m_vBoxMin, pBrush->m_Points[nCurrPt]);
		VEC_MAX(m_vBoxMax, m_vBoxMax, pBrush->m_Points[nCurrPt]);
	}	
}

void CObjectsOutsideLevelDetector::OnObject(CBaseEditObj* pObj)
{
	//don't bother checking it if it is a static sunlight or an outside def
	if(	(stricmp(pObj->GetClassName(), "StaticSunlight") == 0) ||
		(stricmp(pObj->GetClassName(), "OutsideDef") == 0))
	{
		return;
	}

	//see if this object is within the level bounding box
	LTVector vPos = pObj->GetPos();

	if(	(vPos.x < m_vBoxMin.x) || (vPos.x > m_vBoxMax.x) ||
		(vPos.y < m_vBoxMin.y) || (vPos.y > m_vBoxMax.y) ||
		(vPos.z < m_vBoxMin.z) || (vPos.z > m_vBoxMax.z))
	{
		//outside the level
		AddNewError(new CLevelError("Object Outside World", ERRORSEV_MEDIUM, pObj,
					"This object was detected as being outside of the world\'s bounding box. This object should be moved into the world unless it is used only for the purpose of specifying level properties.", 
					this));
	}
}

//----------------------------------MULTIPLE SKY DIMS DETECTOR--------------------------------
CMultipleSkyDimsDetector::CMultipleSkyDimsDetector() :
	CErrorDetector("Detects if there are multiple sky objects in the level with sky dims")
{
	m_bNotifyObject = true;
}

bool CMultipleSkyDimsDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	m_bHitSkyDims = false;
	return CErrorDetector::InternalBuildErrorList(pDoc);
}

void CMultipleSkyDimsDetector::OnObject(CBaseEditObj* pObj)
{
	if(stricmp(pObj->GetClassName(), "DemoSkyWorldModel") == 0)
	{
		//see if this has the sky dims set
		LTVector vSkyDims = GetVectorProp(&pObj->m_PropList, "SkyDims");
		if(vSkyDims.Mag() > 0.1f)
		{
			if(m_bHitSkyDims)
			{
				AddNewError(new CLevelError("Multiple Sky Dims Set", ERRORSEV_MEDIUM, pObj,
					"This sky world model has extra sky dims set. Only one sky world model in a level is allowed to have the sky dims set to non-zero values. The sky dims for this object should be set to 0, 0, 0.", 
					this));
			}
			m_bHitSkyDims = true;
		}
	}
}

//----------------------------------DUPLICATE SKY INDEX DETECTOR--------------------------------
CDuplicateSkyIndexDetector::CDuplicateSkyIndexDetector() :
	CErrorDetector("Detects if there are any sky models that share the same index")
{
	m_bNotifyObject = true;
}

bool CDuplicateSkyIndexDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	m_SkyIndices.RemoveAll();
	return CErrorDetector::InternalBuildErrorList(pDoc);
}

void CDuplicateSkyIndexDetector::OnObject(CBaseEditObj* pObj)
{
	if(stricmp(pObj->GetClassName(), "DemoSkyWorldModel") == 0)
	{
		//get the index of this sky world model
		uint32 nIndex = (uint32)GetRealProp(&pObj->m_PropList, "Index");

		//see if this is already in the list
		for(uint32 nCurrIndex = 0; nCurrIndex < m_SkyIndices.GetSize(); nCurrIndex++)
		{
			if(m_SkyIndices[nCurrIndex] == nIndex)
			{
				//we have a duplicate
				AddNewError(new CLevelError("Duplicate Sky Index", ERRORSEV_MEDIUM, pObj,
					"This sky world model has the same index as another sky world model. This can potentially cause rendering issues in the sky. The index determines the drawing order from low to high, and it is best if each world model has a unique index to ensure correct rendering.", 
					this));
				return;
			}
		}		

		//add this index
		m_SkyIndices.Append(nIndex);
	}
}

//----------------------------------DUPLICATE OBJECT NAME DETECTOR--------------------------------
CDuplicateObjectNameDetector::CDuplicateObjectNameDetector() :
	CErrorDetector("Detects if there are objects that share the same name")
{
}

bool CDuplicateObjectNameDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	//we need to run through all objects
	CEditRegion* pRegion = pDoc->GetRegion();

	for(uint32 nCurrObj = 0; nCurrObj < pRegion->m_Objects.GetSize(); nCurrObj++)
	{
		for(uint32 nTestObj = nCurrObj + 1; nTestObj < pRegion->m_Objects.GetSize(); nTestObj++)
		{
			if(stricmp(pRegion->m_Objects[nCurrObj]->GetName(), pRegion->m_Objects[nTestObj]->GetName()) == 0)
			{
				//we have a duplicate
				AddNewError(new CLevelError("Duplicate Object Name", ERRORSEV_MEDIUM, pRegion->m_Objects[nTestObj],
					"This object has the exact same name as another object in the level. This can cause issues with game code as well as the engine. The name should be modified in order to ensure that it is unique.", 
					this));
			}
		}
	}

	return true;
}

//----------------------------------UNTEXTURED POLIES DETECTOR--------------------------------
CUntexturedPoliesDetector::CUntexturedPoliesDetector() :
	CErrorDetector("Detects polygons that do not have a texture associated with them")
{
	m_bNotifyPoly = true;
}

bool CUntexturedPoliesDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	if(stricmp(pPoly->GetTexture(0).m_pTextureName, "Default") == 0)
	{
		AddNewError(new CLevelError("Untextured Poly", ERRORSEV_LOW, pBrush,
					"This brush contains a polygon that does not have a texture associated with it. This can cause several rendering issues such as lightmapping to not appear on the polygon. If a flat color for the polygon is what is desired, a white texture may be applied and the brush\'s ambient light set to the desired color and receive light set to false.", 
					this));
		return false;
	}
	return true;
}

//----------------------------------MISSING TEXTURES DETECTOR--------------------------------
CMissingTexturesDetector::CMissingTexturesDetector() :
	CErrorDetector("Detects polygons that have references to textures that do not exist on disk")
{
	m_bNotifyPoly = true;
}

bool CMissingTexturesDetector::OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)
{
	//ignore untextured polies
	for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
	{
		if(stricmp(pPoly->GetTexture(nCurrTex).m_pTextureName, "Default") == 0)
		{
			continue;
		}

		if(pPoly->GetTexture(nCurrTex).m_pTextureFile == NULL)
		{
			AddNewError(new CLevelError("Missing Texture", ERRORSEV_MEDIUM, pBrush,
						"This brush contains a polygon that refers to a texture that cannot be found. Often this is caused by moving a texture around in the directory structure, or deleting textures. This polygon should have a texture associated with it or it could result in several rendering issues.", 
						this));
			//only report it once
			return false;
		}
	}
	return true;
}

//-----------------------------INVALID NON DETAIL BRUSH DETECTOR----------------------------
CInvalidNonDetailBrushDetector::CInvalidNonDetailBrushDetector()	: 
	CErrorDetector("Detects multiple non detail brushes in a level, or missing non detail brushes")	
{
	m_bNotifyBrush		= true;
	m_pFirstBrush		= NULL;
	m_bReportedFirst	= false;
}

bool CInvalidNonDetailBrushDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	m_pFirstBrush		= NULL;
	m_bReportedFirst	= false;

	if(!CErrorDetector::InternalBuildErrorList(pDoc))
		return false;

	if(m_pFirstBrush == NULL)
	{
		AddNewError(new CLevelError("Missing Non Detail Brush", ERRORSEV_CRITICAL, NULL,
					"This level contains no brushes set to Detail False, which means that it will not be able to process. A brush that encompasses the entire level should be created and set to detail false.", 
					this));
	}

	return true;
}

void CInvalidNonDetailBrushDetector::OnBrush(CEditBrush* pBrush)
{
	if(!GetBoolProp(&pBrush->m_PropList, "Detail", true))
	{
		//we have a non-detail brush. See if it is the first
		if(m_pFirstBrush)
		{
			const char* pszErrorName = "Multiple Non Detail";
			const char* pszErrorText = "This brush is one of the non-detail brushes in the level. Each level should only have a single non-detail brush that encompasses the level. All other brushes should have Detail set to false.";

			AddNewError(new CLevelError(pszErrorName, ERRORSEV_HIGH, pBrush,
						pszErrorText, 
						this));

			if(!m_bReportedFirst)
			{
				AddNewError(new CLevelError(pszErrorName, ERRORSEV_HIGH, m_pFirstBrush,
							pszErrorText, 
							this));
				m_bReportedFirst = true;
			}
		}
		else
		{
			m_pFirstBrush = pBrush;
		}
	}
}

//--------------------------------SHADOW MESH BRUSH DETECTOR---------------------------------
CShadowMeshBrushDetector::CShadowMeshBrushDetector()	: 
	CErrorDetector("Detects brushes marked with shadow meshing")	
{
	m_bNotifyBrush = true;
}

void CShadowMeshBrushDetector::OnBrush(CEditBrush* pBrush)
{
	if(GetLightingType(pBrush) == eLight_ShadowMesh)
	{
		AddNewError(new CLevelError("Shadow Mesh Brush", ERRORSEV_MEDIUM, pBrush,
					"This brush is marked as using shadow meshing. This should not be used.", 
					this));
	}
}
