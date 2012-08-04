#ifndef __DETECTORS_H__
#define __DETECTORS_H__

#ifndef __ERRORDETECTOR_H__
#	include "errordetector.h"
#endif

//----------------------------------CONCAVE POLY DETECTOR----------------------------------
class CConcavePolyDetector : public CErrorDetector
{
public:

	CConcavePolyDetector();
	const char* GetName() const				{return "Concave Poly";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------NONPLANAR POLY DETECTOR----------------------------------
class CNonplanarDetector : public CErrorDetector
{
public:

	CNonplanarDetector();
	const char* GetName() const				{return "Nonplanar Poly";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------POLY EDGE LENGTH----------------------------------
class CPolyEdgeLenDetector : public CErrorDetector
{
public:

	CPolyEdgeLenDetector();
	const char* GetName() const				{return "Poly Edge Length";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------POINTS OFF GRID DETECTOR--------------------------------
class CPointsOffGridDetector : public CErrorDetector
{
public:

	CPointsOffGridDetector();
	const char* GetName() const				{return "Points Off Grid";}
	void		OnBrush(CEditBrush* pBrush);
};

//----------------------------------REDUNDANT POINT DETECTOR----------------------------------
class CRedundantPointsDetector : public CErrorDetector
{
public:

	CRedundantPointsDetector();
	const char* GetName() const				{return "Redundant Points";}
	void		OnBrush(CEditBrush* pBrush);
};

//----------------------------------SMALL POLY SIZE DETECTOR----------------------------------
class CSmallPolySizeDetector : public CErrorDetector
{
public:

	CSmallPolySizeDetector();
	const char* GetName() const				{return "Small Poly Size";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------INVALID POLY COUNT DETECTOR--------------------------------
class CInvalidPolyCountDetector : public CErrorDetector
{
public:

	CInvalidPolyCountDetector();
	const char* GetName() const				{return "Invalid Poly Count";}
	void		OnBrush(CEditBrush* pBrush);
};

//----------------------------------LARGE LIGHTMAPPED POLIES DETECTOR--------------------------
class CLargeLightmappedPoliesDetector : public CErrorDetector
{
public:

	CLargeLightmappedPoliesDetector();
	const char* GetName() const				{return "Large Lightmapped Polies";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------SMALL LMGRIDSIZE DETECTOR--------------------------------
class CSmallLMGridSizeDetector : public CErrorDetector
{
public:

	CSmallLMGridSizeDetector();
	const char* GetName() const				{return "Small Lightmap Grid Size";}
	void		OnBrush(CEditBrush* pBrush);
};

//----------------------------------INVALID SUNLIGHT DETECTOR--------------------------------
class CInvalidSunlightDetector : public CErrorDetector
{
public:

	CInvalidSunlightDetector();
	const char* GetName() const				{return "Invalid Sunlight";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);
	void		OnObject(CBaseEditObj* pObj);
	void		OnBrush(CEditBrush* pBrush);

private:
	uint32		m_nNumSunlights;		//number of sunlights
	uint32		m_nNumSkyPortals;		//number of sky portals
};

//---------------------------LIGHTMAPPED SKYPORTAL DETECTOR------------------------------
class CLightmappedSkyportalDetector : public CErrorDetector
{
public:

	CLightmappedSkyportalDetector();
	const char* GetName() const				{return "Lightmapped Skyportal";}
	void		OnBrush(CEditBrush* pBrush);
};

//---------------------------OBJECTS OUTSIDE LEVEL DETECTOR------------------------------
class CObjectsOutsideLevelDetector : public CErrorDetector
{
public:

	CObjectsOutsideLevelDetector();
	const char* GetName() const				{return "Objects Outside Level";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);
	void		OnBrush(CEditBrush* pBrush);
	void		OnObject(CBaseEditObj* pObj);

private:

	//level bounding box
	LTVector	m_vBoxMin;
	LTVector	m_vBoxMax;
};

//----------------------------------MULTIPLE SKY DIMS DETECTOR--------------------------------
class CMultipleSkyDimsDetector : public CErrorDetector
{
public:

	CMultipleSkyDimsDetector();
	const char* GetName() const				{return "Multiple Sky Dims";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);
	void		OnObject(CBaseEditObj* pObj);

private:
	bool		m_bHitSkyDims;
};

//----------------------------------DUPLICATE SKY INDEX DETECTOR------------------------------
class CDuplicateSkyIndexDetector : public CErrorDetector
{
public:

	CDuplicateSkyIndexDetector();
	const char* GetName() const				{return "Duplicate Sky Indices";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);
	void		OnObject(CBaseEditObj* pObj);

private:
	CMoArray<uint32>	m_SkyIndices;
};

//----------------------------------DUPLICATE OBJECT NAME DETECTOR---------------------------
class CDuplicateObjectNameDetector : public CErrorDetector
{
public:

	CDuplicateObjectNameDetector();
	const char* GetName() const				{return "Duplicate Object Names";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);

private:
};

//----------------------------------UNTEXTURED POLIES DETECTOR--------------------------------
class CUntexturedPoliesDetector : public CErrorDetector
{
public:

	CUntexturedPoliesDetector();
	const char* GetName() const				{return "Untextured Polies";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//----------------------------------MISSING TEXTURES DETECTOR--------------------------------
class CMissingTexturesDetector : public CErrorDetector
{
public:

	CMissingTexturesDetector();
	const char* GetName() const				{return "Missing Textures";}
	bool		OnPoly(CEditBrush* pBrush, CEditPoly* pPoly);
};

//-----------------------------MULTIPLE NON DETAIL BRUSH DETECTOR----------------------------
class CInvalidNonDetailBrushDetector : public CErrorDetector
{
public:

	CInvalidNonDetailBrushDetector();
	const char* GetName() const				{return "Invalid Non Detail";}
	bool		InternalBuildErrorList(CRegionDoc* pDoc);
	void		OnBrush(CEditBrush* pBrush);

private:

	CEditBrush*		m_pFirstBrush;
	bool			m_bReportedFirst;
};

//--------------------------------SHADOW MESH BRUSH DETECTOR---------------------------------
class CShadowMeshBrushDetector : public CErrorDetector
{
public:

	CShadowMeshBrushDetector();
	const char* GetName() const				{return "Shadow Mesh";}
	void		OnBrush(CEditBrush* pBrush);
};


#endif

