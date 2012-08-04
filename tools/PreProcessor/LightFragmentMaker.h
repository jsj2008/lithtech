//////////////////////////////////////////////////////////////////////////////
// Polygon subdivision lighting engine
// The current version makes the following requirements/restrictions
//	- DirLights are not supported
//	- Sunlight is not supported
//	- Polygons must not have more than one neighbor per edge
//  - Silhouette corners are not guaranteed to be handled properly
//  - The order of the edge manipulations is very very high, so don't use high-poly levels yet
//  - This may create some triangles which could be removed without any effect

#ifndef __LIGHTFRAGMENTMAKER_H__
#define __LIGHTFRAGMENTMAKER_H__

#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////////
// Forward declarations

// The light definition class, which is defined in LightMapMaker.h
class CLightDef;
// The lightmap maker, which handles the lighting of the points.  Also in LightMapMaker.h
class CLightMapMaker;

// World polygons
class CPrePoly;

#define INVALID_INDEX 0xFFFFFFFF

//////////////////////////////////////////////////////////////////////////////
// Local classes

// Outline-based poly structure
// Poly is defined by a plane, a set of vertices, and a neighbor list
struct SLFM_Outline
{
public:
	// Initialize the outline based on a CPrePoly
	SLFM_Outline();
	SLFM_Outline(CPrePoly *pPoly);

	// Swap with another outline (to avoid the list copies...)
	void Swap(SLFM_Outline &sOther) {
		std::swap(m_pPrePoly, sOther.m_pPrePoly);
		std::swap(m_cPlane, sOther.m_cPlane);
		m_aVertices.swap(sOther.m_aVertices);
		m_aNeighbors.swap(sOther.m_aNeighbors);
	}

	void RecalcExtents(float fExpandExtents = 0.01f);
	bool OverlapExtents(const SLFM_Outline &sOther) const {
		return (m_vExtentsMax.x >= sOther.m_vExtentsMin.x) &&
			(m_vExtentsMax.y >= sOther.m_vExtentsMin.y) &&
			(m_vExtentsMax.z >= sOther.m_vExtentsMin.z) &&
			(m_vExtentsMin.x <= sOther.m_vExtentsMax.x) &&
			(m_vExtentsMin.y <= sOther.m_vExtentsMax.y) &&
			(m_vExtentsMin.z <= sOther.m_vExtentsMax.z);
	}

public:
	// Remember which poly this came from
	CPrePoly *m_pPrePoly;

	// Remember the extents of this poly
	LTVector m_vExtentsMin, m_vExtentsMax;
	
	LTPlane m_cPlane;
	typedef std::vector<LTVector> TVertList;
	TVertList m_aVertices;
	typedef std::vector<SLFM_Outline *> TNeighborList;
	TNeighborList m_aNeighbors;
};
typedef std::vector<SLFM_Outline> TSLFM_OutlineList;

// Edge description structure (for silhouette edges)
struct SLFM_Edge
{
public:
	SLFM_Edge() { m_pOutlines[0] = m_pOutlines[1] = 0; }
	SLFM_Edge(
		const LTVector &vPt1, const LTVector &vPt2, 
		const SLFM_Outline *pNeighbor0 = 0, const SLFM_Outline *pNeighbor1 = 0);
	SLFM_Edge(const SLFM_Edge &cOther);
	SLFM_Edge &operator=(const SLFM_Edge &cOther);

	// Create a plane from a point and this edge
	LTPlane CreatePlane(const LTVector &vPt) const;
	// Get the plane status of this edge
	PolySide GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront) const;
	// Split this edge by a plane
	// If it intersected the plane, the return value is true
	// pSideResult will be filled in with which side of the plane the original edge is on
	bool SplitPlane(const LTPlane &cPlane, SLFM_Edge *pNewEdge, PolySide *pSideResult, bool *pStartFront);
	// Split this edge at a given point
	void SplitPt(const LTVector &vPt, SLFM_Edge *pNewEdge);
	// Project the edge onto a plane using projection points for starting and ending points
	// fRayEdgeLen is used to calculate a semi-infinite edge for the case when the projection is a ray
	bool ProjectPlane(const LTPlane &cPlane, const LTVector &vStartPt, const LTVector &vEndPt, float fRayEdgeLen = 10000.0f);
	// Re-calc the direction and length of the edge based on the start & end points
	void Recalc();
	// Is this edge a neighboring edge to the provided outline?
	bool IsNeighbor(const SLFM_Outline *pOutline) const { return (pOutline == m_pOutlines[0]) || (pOutline == m_pOutlines[1]); }
public:
	LTVector m_vStart, m_vEnd;
	const SLFM_Outline *m_pOutlines[2];
};
typedef std::vector<SLFM_Edge> TSLFM_EdgeList;

// Shadow edge description structure
struct SLFM_ShadowEdge
{
public:
	SLFM_ShadowEdge() {}
	SLFM_ShadowEdge(const LTVector &vStart, const LTVector &vEnd, const CLightDef *pLight, float fStart, float fEnd);
	SLFM_ShadowEdge(const SLFM_ShadowEdge &cOther);
	SLFM_ShadowEdge &operator=(const SLFM_ShadowEdge &cOther);
	PolySide GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront) const;
	void Split(float fInterpolant, SLFM_ShadowEdge *pStartResult, SLFM_ShadowEdge *pEndResult) const;
	PolySide SplitPlane(const LTPlane &cPlane, SLFM_ShadowEdge *pResultFront, SLFM_ShadowEdge *pResultBack) const;
public:
	// The extents of the edge
	LTVector m_vStart, m_vEnd;
	// Which light cast the shadow
	const CLightDef *m_pLight;
	// How much lighting should fall on this edge [0-1]
	float m_fStartLight, m_fEndLight;
};
typedef std::vector<SLFM_ShadowEdge> TSLFM_ShadowEdgeList;

// Final vertices for copying into the finalized triangulation
struct SLFM_FinalLightingVert
{
public:
	SLFM_FinalLightingVert() {}
	SLFM_FinalLightingVert(const SLFM_FinalLightingVert &sOther) : m_vPos(sOther.m_vPos), m_vNormal(sOther.m_vNormal), m_aLightIntensities(sOther.m_aLightIntensities) {}
	SLFM_FinalLightingVert(const LTVector &vPos, const LTVector &vNormal) : m_vPos(vPos), m_vNormal(vNormal) {}
	SLFM_FinalLightingVert &operator=(const SLFM_FinalLightingVert &sOther) {
		m_vPos = sOther.m_vPos;
		m_vNormal = sOther.m_vNormal;
		m_aLightIntensities = sOther.m_aLightIntensities;
		return *this;
	}

public:
	// Note that if the light is already associated with the vertex, it will use the minimum of
	// the provided intensity and the existing intensity
	void SetLightIntensity(const CLightDef *pLight, float fIntensity);
	float GetLightIntensity(const CLightDef *pLight, float fDefault = 0.0f) const;

	// Interpolate between two vertices
	void Lerp(const SLFM_FinalLightingVert &sStart, const SLFM_FinalLightingVert &sEnd, float fInterpolant);
	void BiLerp(const SLFM_FinalLightingVert &sStart, const SLFM_FinalLightingVert &sEndU, const SLFM_FinalLightingVert &sEndV, float fInterpolantU, float fInterpolantV);

	// Are the two vertices equal?  (Mostly..)
	bool Equals(const SLFM_FinalLightingVert &sOther) const;

	// Copy the lighting (otherwise known as extra data)
	void CopyExtra(const SLFM_FinalLightingVert &sOther);
public:
	LTVector m_vPos, m_vNormal;
	// Yes, this map is evil.  But initial, prototype implementations often are
	typedef std::map<const CLightDef *, float> TLightIntensityMap;
	TLightIntensityMap m_aLightIntensities;
	// Note : This member is not updated.  (It's not even copied)
	LTVector m_vColor;
};

// Finalized polygon triangulation
/*
	T_Vert must have the following members:
		LTVector m_vPos -
			Position of the vertex
		void Lerp(const T_Vert &sStart, const T_Vert &sEnd, float fInterpolant) - 
			Assign *this to a linear interpolation between sStart and sEnd
		void BiLerp(const T_Vert &sStart, const T_Vert &sEndU, const T_Vert &sEndV, float fInterpolantU, float fInterpolantV) -
			Assign *this to a bilinear interpolation between sStart, sEndU and sEndV
*/
// Note : The template is so the projection routine can use this for a CSG
template <class T_Vert>
struct SLFM_Triangulation
{
public:
	typedef T_Vert SVert;

	struct SEdge
	{
		SEdge(uint32 nStart = 0, uint32 nEnd = 0, bool bHardEdge = false) : 
			m_nStart(nStart), m_nEnd(nEnd), m_bHardEdge(bHardEdge) {}
		SEdge(const SEdge &sOther) : 
			m_nStart(sOther.m_nStart), m_nEnd(sOther.m_nEnd), m_bHardEdge(sOther.m_bHardEdge) {}
		SEdge &operator=(const SEdge &sOther) {
			m_nStart = sOther.m_nStart;
			m_nEnd = sOther.m_nEnd;
			m_bHardEdge = sOther.m_bHardEdge;
			return *this;
		}
		uint32 m_nStart, m_nEnd;
		bool m_bHardEdge;
	};

	struct STriangle
	{
		STriangle(uint32 nEdge0 = 0, uint32 nEdge1 = 0, uint32 nEdge2 = 0) :
			m_nEdge0(nEdge0), m_nEdge1(nEdge1), m_nEdge2(nEdge2) {
			ASSERT((m_nEdge0 != m_nEdge1) && (m_nEdge0 != m_nEdge2) && (m_nEdge1 != m_nEdge2));
		}
		STriangle(const STriangle &sOther) :
			m_nEdge0(sOther.m_nEdge0), m_nEdge1(sOther.m_nEdge1), m_nEdge2(sOther.m_nEdge2) {}
		STriangle &operator=(const STriangle &sOther) {
			m_nEdge0 = sOther.m_nEdge0;
			m_nEdge1 = sOther.m_nEdge1;
			m_nEdge2 = sOther.m_nEdge2;
			return *this;
		}
		// Index-based edge accessors
		uint32 &Edge(uint32 nEdgeIndex) { return (nEdgeIndex == 0) ? m_nEdge0 : ((nEdgeIndex == 1) ? m_nEdge1 : m_nEdge2); }
		const uint32 &Edge(uint32 nEdgeIndex) const { return (nEdgeIndex == 0) ? m_nEdge0 : ((nEdgeIndex == 1) ? m_nEdge1 : m_nEdge2); }
		// Edge index retreival
		uint32 EdgeIndex(uint32 nEdge) const { return (nEdge == m_nEdge0) ? 0 : (nEdge == m_nEdge1) ? 1 : (nEdge == m_nEdge2) ? 2 : 3; }
		// Does this triangle contain the specified edge?
		bool HasEdge(uint32 nEdge) const { return (nEdge == m_nEdge0) || (nEdge == m_nEdge1) || (nEdge == m_nEdge2); }

		uint32 m_nEdge0, m_nEdge1, m_nEdge2;
	};

public:
	// Find/create a vertex at the given position.
	// If it needs to create a vertex, it will calculate a proper normal and split
	// any triangles which will contain the point
	// Note : This will invalidate any iterators
	uint32 GetVertIndex(const SVert &vPos);
	// Find an existing, exactly equal vertex.
	// Returns INVALID_INDEX if not found
	uint32 GetExistingVertIndex(const SVert &vPos);

	SVert &GetVert(uint32 nIndex) { return m_aVertices[nIndex]; }
	const SVert &GetVert(uint32 nIndex) const { return m_aVertices[nIndex]; }

	// Find an edge with the given indices
	// Note : This will invalidate any iterators
	bool FindEdge(uint32 nStart, uint32 nEnd, uint32 *pResultEdge, uint32 nStartSearch = 0) const;

	// Insert a hard edge and triangulate appropriately
	// Notes : This will invalidate any iterators.  This will not split hard edges.  This
	// will take a long time and not look for duplicates, so call FindEdge first.
	bool InsertHardEdge(uint32 nStart, uint32 nEnd);

	// Split an edge at a point, and maintain proper triangulation
	// Returns the index of the new vertex
	// Note : This will invalidate any iterators
	uint32 SplitEdge(uint32 nEdgeIndex, const T_Vert &vPos, float fInterpolant = -1.0f);

	// Get the vertex indices associated with a triangle
	void GetTriVerts(const STriangle &sTri, uint32 *pVert0, uint32 *pVert1, uint32 *pVert2) const;

	// Check for a CW winding order given 3 vertex indices and a poly normal
	bool IsFrontFacing(uint32 nIndex0, uint32 nIndex1, uint32 nIndex2, const LTVector &vNormal) const;
public:
	typedef std::vector<SVert> TVertList;
	typedef std::vector<STriangle> TTriangleList;
	typedef std::vector<SEdge> TEdgeList;

	LTPlane m_cPlane;

	TVertList m_aVertices;
	TEdgeList m_aEdges;
	TTriangleList m_aTriangles;
};

class CLightFragmentMaker
{
public:
	CLightFragmentMaker(const CLightMapMaker &cLMM);
	~CLightFragmentMaker();

	// Load a set of polygons into the fragment maker
	// Note : These polys aren't constant, because that's where the results go
	void LoadPolys(CPrePoly * const *pPolyArray, uint32 nPolyCount);
	// Load the set of lights that are going to be used
	void LoadLights(const CLightDef * const *pLights, uint32 nLightCount);
	// Subdivide the poly set and generate the lit fragments
	void Execute();

private:
	//////////////////////////////////////////////////////////////////////////////
	// Internal types

	typedef std::vector<CPrePoly*> TPrePolyList;
	typedef std::vector<const CLightDef *> TLightDefList;
	typedef std::vector<SLFM_Outline *> TOutlineList;
	typedef TSLFM_EdgeList TEdgeList;
	typedef std::vector<TEdgeList> TEdgeListList;
	typedef std::vector<SLFM_ShadowEdge> TShadowEdgeList;
	typedef TSLFM_ShadowEdgeList TShadowEdgeList;
	typedef std::vector<TShadowEdgeList> TShadowEdgeListList;
	typedef std::vector<LTVector> TVectorList;
	typedef std::vector<SLFM_Triangulation<SLFM_FinalLightingVert> > TTriangulationList;

private:
	//////////////////////////////////////////////////////////////////////////////
	// Internal implementation functions

	// Build the silhouettes from the world poly array
	void BuildOutlines();

	// Clear out the silhouette array
	void ClearOutlines();

	// Count how many lights are going to be casting shadows (so we can skip stuff if necessary)
	void CountCastingLights();

	// Put together the neighbor lists in the outlines
	void FindNeighbors();

	// Find the silhouette edges for a light position
	void FindSilhouetteEdges(const CLightDef *pLightDef, TEdgeList *pEdges) const;

	// Project the edge list onto the polygons in the scene
	void ProjectEdges(const CLightDef *pLightDef, const TEdgeList &aEdges);

	// Does the light possibly touch the specified poly?
	// Note : This is not a guarantee that the light touches the poly
	bool LightTouchesPoly(const CLightDef *pLightDef, const SLFM_Outline &sOutline) const;

	// Fix edges that cross each other
	void FixCrossedEdges(const LTPlane &cPlane, const TShadowEdgeList &cEdges, TShadowEdgeList *pResultEdges) const;

	// Triangulate a polygon
	// Note : Crossed edges are not legal to send into this function
	void Triangulate(const SLFM_Outline &cPoly, const TShadowEdgeList &cEdges, SLFM_Triangulation<SLFM_FinalLightingVert> *pTriangulation);

	// Write the provided triangulation into the provided polygon's fragments
	void WriteTriangulation(const SLFM_Triangulation<SLFM_FinalLightingVert> &cTriangulation, CPrePoly *pPoly);
private:
	//////////////////////////////////////////////////////////////////////////////
	// Class data

	// Our lightmap maker class, which we use to light points
	const CLightMapMaker &m_cLMM;

	// The main world poly set
	TPrePolyList m_aWorldPolys;

	// The lights
	TLightDefList m_aWorldLights;
	uint32 m_nCastingLights;

	// Outline data
	TOutlineList m_aOutlines;
	uint32 m_nReceiverCount;

	// Edges
	TShadowEdgeListList m_aFragmentEdges;

	// Triangulation output
	TTriangulationList m_aTriangulations;
};

#endif //__LIGHTFRAGMENTMAKER_H__