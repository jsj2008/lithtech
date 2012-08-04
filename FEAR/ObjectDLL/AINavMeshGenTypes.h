// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenTypes.h
//
// PURPOSE : AI NavMesh generator type definitions.
//
// CREATED : 11/02
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_TYPES_H_
#define _AI_NAVMESH_GEN_TYPES_H_

//-----------------------------------------------------------------

#include <map>

// Forward declarations.

class	CAINavMeshGenPoly;
class	CAINavMeshGenCarver;
class	CAINavMeshGenQuadTreeNode;
class	SAINAVMESHGEN_VERT;
class	SAINAVMESHGEN_EDGE;
class	SAINAVMESHGEN_REGION; 
class	SAINAVMESHGEN_COMPONENT; 
struct  SAINAVMESHGEN_LINK;
struct	SAINAVMESHGEN_SPLITTING_VERT;
struct  SAINAVMESHGEN_NODE;
struct  SAINAVMESHGEN_RESTRICTIONS;
struct  SAINODECLUSTER_MERGE;

//-----------------------------------------------------------------

// Enums.

enum ENUM_NMGNavMeshID { kNMGNavMesh_Invalid = -1, };
enum ENUM_NMGPolyID { kNMGPoly_Invalid = -1, };
enum ENUM_NMGConvertedPolyID { kNMGConvertedPoly_Invalid = -1, };
enum ENUM_NMGLinkID { kNMGLink_Invalid = -1, };
enum ENUM_NMGVertID { kNMGVert_Invalid = -1, };
enum ENUM_NMGNormalID { kNMGNormal_Invalid = -1, };
enum ENUM_NMGEdgeID { kNMGEdge_Invalid = -1, };
enum ENUM_NMGConvertedEdgeID { kNMGConvertedEdge_Invalid = -1, };
enum ENUM_NMGCarverID { kNMGCarver_Invalid = -1, };
enum ENUM_NMGRegionID { kNMGRegion_Invalid = -1, };
enum ENUM_NMGComponentID { kNMGComponent_Invalid = -1, };
enum ENUM_NMGSensoryComponentID { kNMGSensoryComponent_Invalid = -1, };
enum ENUM_NMGQTNodeID { kNMGQTNode_Invalid = -1, };
enum ENUM_NMGNodeID { kNMGNode_Invalid = -1, };
enum ENUM_NMGNodeClusterID { kNMGNodeCluster_Invalid = -1, };

enum ENUM_NMGEdgeType
{
	kNMGEdgeType_Invalid = -1,
	kNMGEdgeType_Border,
	kNMGEdgeType_Shared,
};


// Lists.

typedef std::vector<
			LTVector, 
			LTAllocator<LTVector, LT_MEM_TYPE_OBJECTSHELL> 
		> VECTOR_LIST;
typedef std::vector<
			ENUM_NMGPolyID, 
			LTAllocator<ENUM_NMGPolyID, LT_MEM_TYPE_OBJECTSHELL> 
		> NMGPOLY_LIST;
typedef std::vector<
			ENUM_NMGVertID,
			LTAllocator<ENUM_NMGVertID, LT_MEM_TYPE_OBJECTSHELL> 
		> NMGVERT_LIST;
typedef std::vector<
			ENUM_NMGRegionID,
			LTAllocator<ENUM_NMGRegionID, LT_MEM_TYPE_OBJECTSHELL> 
		> NMGREGION_LIST;
typedef std::vector<
			CAINavMeshGenPoly*,
			LTAllocator<CAINavMeshGenPoly*, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_POLY_LIST;
typedef std::vector<
			CAINavMeshGenCarver*,
			LTAllocator<CAINavMeshGenCarver*, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_CARVER_LIST;
typedef std::vector<
			SAINAVMESHGEN_REGION*,
			LTAllocator<SAINAVMESHGEN_REGION*, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_REGION_LIST;
typedef std::vector<
			SAINAVMESHGEN_COMPONENT*,
			LTAllocator<SAINAVMESHGEN_COMPONENT*, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_COMPONENT_LIST;
typedef std::vector<
			SAINAVMESHGEN_SPLITTING_VERT,
			LTAllocator<SAINAVMESHGEN_SPLITTING_VERT, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_SPLITTING_VERT_LIST;
typedef std::vector<
			SAINAVMESHGEN_LINK,
			LTAllocator<SAINAVMESHGEN_LINK, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_LINK_LIST;
typedef std::vector<
			SAINAVMESHGEN_NODE,
			LTAllocator<SAINAVMESHGEN_NODE, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_NODE_LIST;
typedef std::vector<
			SAINAVMESHGEN_NODE*,
			LTAllocator<SAINAVMESHGEN_NODE*, LT_MEM_TYPE_OBJECTSHELL> 
	> AINAVMESHGEN_NODE_PTR_LIST;
typedef std::vector<
			SAINAVMESHGEN_RESTRICTIONS,
			LTAllocator<SAINAVMESHGEN_RESTRICTIONS, LT_MEM_TYPE_OBJECTSHELL> 
	> AINAVMESHGEN_RESTRICTIONS_LIST;
typedef std::vector<
			SAINODECLUSTER_MERGE,
			LTAllocator<SAINODECLUSTER_MERGE, LT_MEM_TYPE_OBJECTSHELL> 
		> AINODECLUSTER_MERGE_LIST;
typedef std::vector<
			CAINavMeshGenQuadTreeNode*, 
			LTAllocator<CAINavMeshGenQuadTreeNode*, LT_MEM_TYPE_OBJECTSHELL> 
		> NMQUAD_TREE_LIST;

// Maps.

typedef std::map<
			ENUM_NMGVertID, 
			SAINAVMESHGEN_VERT*,
			std::less<ENUM_NMGVertID>,
			LTAllocator<std::pair<ENUM_NMGVertID, SAINAVMESHGEN_VERT*>, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_VERT_MAP;	// Sorted by vertID.
typedef std::multimap<
			ENUM_NMGVertID, 
			SAINAVMESHGEN_EDGE*,
			std::less<ENUM_NMGVertID>,
			LTAllocator<std::pair<ENUM_NMGVertID, SAINAVMESHGEN_EDGE*>, LT_MEM_TYPE_OBJECTSHELL> 
		> AINAVMESHGEN_EDGE_MAP;	// Sorted by min index.

// Bounds.

typedef std::pair<
			AINAVMESHGEN_EDGE_MAP::iterator, 
			AINAVMESHGEN_EDGE_MAP::iterator
		> AINAVMESHGEN_EDGE_BOUNDS;


//-----------------------------------------------------------------

struct SAINAVMESHGEN_AABB
{
	SAINAVMESHGEN_AABB() { InitAABB(); }

	void InitAABB();
	void GrowAABB( const LTVector& vVert );
	bool IntersectAABB( const SAINAVMESHGEN_AABB& aabb );
	bool ContainsPoint( const LTVector& vPos );

	LTVector vMin;
	LTVector vMax;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_MERGE_3_2_RESULTS
{
	SAINAVMESHGEN_MERGE_3_2_RESULTS() 
	{
		Init();
	}

	void Init()
	{
		ePolyA = kNMGPoly_Invalid;
		ePolyB = kNMGPoly_Invalid;
		ePolyC = kNMGPoly_Invalid;

		eVert1 = kNMGVert_Invalid;
		eVert2 = kNMGVert_Invalid;
		eVert3 = kNMGVert_Invalid;
		eVert4 = kNMGVert_Invalid;
		eVert5 = kNMGVert_Invalid;
		eVert6 = kNMGVert_Invalid;
		eVert7 = kNMGVert_Invalid;

		pPolyB1 = NULL;
		pPolyB2 = NULL;
	}

	ENUM_NMGPolyID	ePolyA;
	ENUM_NMGPolyID	ePolyB;
	ENUM_NMGPolyID	ePolyC;

	ENUM_NMGVertID	eVert1;
	ENUM_NMGVertID	eVert2;
	ENUM_NMGVertID	eVert3;
	ENUM_NMGVertID	eVert4;
	ENUM_NMGVertID	eVert5;
	ENUM_NMGVertID	eVert6;
	ENUM_NMGVertID	eVert7;

	CAINavMeshGenPoly*	pPolyB1;
	CAINavMeshGenPoly*	pPolyB2;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_POLY_INTERSECT
{
	SAINAVMESHGEN_POLY_INTERSECT()
	{
		v0.Init();
		v1.Init();
		eVertIntersectNear = kNMGVert_Invalid;
		eVertIntersectFar = kNMGVert_Invalid;
		eVertEdgeNear0 = kNMGVert_Invalid;
		eVertEdgeNear1 = kNMGVert_Invalid;
		eVertEdgeFar0 = kNMGVert_Invalid;
		eVertEdgeFar1 = kNMGVert_Invalid;
	}

	LTVector	v0;
	LTVector	v1;
	ENUM_NMGVertID	eVertIntersectNear;
	ENUM_NMGVertID	eVertIntersectFar;
	ENUM_NMGVertID	eVertEdgeNear0;
	ENUM_NMGVertID	eVertEdgeNear1;
	ENUM_NMGVertID	eVertEdgeFar0;
	ENUM_NMGVertID	eVertEdgeFar1;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_SPLITTING_VERT
{
	SAINAVMESHGEN_SPLITTING_VERT()
	{
		eVert = kNMGVert_Invalid;
		ePoly = kNMGPoly_Invalid;
	}

	ENUM_NMGVertID	eVert;
	ENUM_NMGPolyID	ePoly;
};

//-----------------------------------------------------------------

class SAINAVMESHGEN_VERT
{
public:
	SAINAVMESHGEN_VERT()
	{
		vVert.Init();
		lstPolyRefs.resize( 0 );
		bIsBorderVert = false; 
	}

	LTVector		vVert;
	NMGPOLY_LIST	lstPolyRefs;
	bool			bIsBorderVert;
};

//-----------------------------------------------------------------

class SAINAVMESHGEN_EDGE
{
public:
	SAINAVMESHGEN_EDGE() 
	{
		eNMGEdgeID = kNMGEdge_Invalid;

		eNMGEdgeType = kNMGEdgeType_Invalid;

		eVertMin = kNMGVert_Invalid;
		eVertMax = kNMGVert_Invalid;
		ePolyID1 = kNMGPoly_Invalid;
		ePolyID2 = kNMGPoly_Invalid;

		bTooManyNeighbors = false;
	}

	bool IntersectsNMGEdge( ENUM_NMGVertID eVert );
	void PrintNMGEdge();

	ENUM_NMGEdgeID		eNMGEdgeID;

	ENUM_NMGEdgeType	eNMGEdgeType;

	ENUM_NMGVertID		eVertMin;
	ENUM_NMGVertID		eVertMax;
	ENUM_NMGPolyID		ePolyID1;
	ENUM_NMGPolyID		ePolyID2;

	bool				bTooManyNeighbors;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_PLANE
{
	SAINAVMESHGEN_PLANE() { eNMGPlaneNormal = kNMGNormal_Invalid; D = 0.f; }

	void InitNMGPlane( ENUM_NMGNormalID eNormalID, const LTVector& vVert );
	bool IsCoplanar( const LTVector& vPt );
	bool RayIntersectNMGPlane( const LTVector& vRay0, const LTVector& vRay1, LTVector* pvIntersect );

	ENUM_NMGNormalID	eNMGPlaneNormal;
	float				D;
};

//-----------------------------------------------------------------

class SAINAVMESHGEN_REGION
{
public:
	SAINAVMESHGEN_REGION() 
	{
		eNMGRegionID = kNMGRegion_Invalid; 
		lstCarvers.resize( 0 ); 
		cNMGPolys = 0;
		fBoundingSphereRadius = 0;
	}

	ENUM_NMGRegionID			eNMGRegionID;
	AINAVMESHGEN_CARVER_LIST	lstCarvers;
	uint32						cNMGPolys;
	LTVector					vBoundingSphereCenter;
	float						fBoundingSphereRadius;
};

//-----------------------------------------------------------------

class SAINAVMESHGEN_COMPONENT
{
public:
	SAINAVMESHGEN_COMPONENT() 
	{
		eNMGComponentID = kNMGComponent_Invalid; 
		eNMGSensoryComponentID = kNMGSensoryComponent_Invalid;
		eNMGLinkID = kNMGLink_Invalid;
	}

	ENUM_NMGComponentID			eNMGComponentID;
	ENUM_NMGSensoryComponentID	eNMGSensoryComponentID;
	ENUM_NMGLinkID				eNMGLinkID;
	NMGPOLY_LIST				lstNeighborComponentPolys;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_LINK
{
	SAINAVMESHGEN_LINK()
	{
		eNMGLinkID = kNMGLink_Invalid;
		bIsSensoryLink = false;
	}

	ENUM_NMGLinkID			eNMGLinkID;
	bool					bIsSensoryLink;
	std::vector<LTVector>	lstLinkBounds;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_NODE
{
	SAINAVMESHGEN_NODE()
	{
		eNMGNodeID = kNMGNode_Invalid;
		eNMGNodeClusterID = kNMGNodeCluster_Invalid;
	}

	std::string				strName;
	ENUM_NMGNodeID			eNMGNodeID;
	ENUM_NMGNodeClusterID	eNMGNodeClusterID;
	LTVector				vPos;
};

//-----------------------------------------------------------------

struct SAINAVMESHGEN_RESTRICTIONS
{
	SAINAVMESHGEN_RESTRICTIONS()
	{
		eNavMeshID = kNMGNavMesh_Invalid;
		dwCharTypeMask = 0;
	}

	ENUM_NMGNavMeshID	eNavMeshID;
	uint32				dwCharTypeMask;
};

//-----------------------------------------------------------------

struct SAINODECLUSTER_MERGE
{
	SAINODECLUSTER_MERGE()
	{
		eClusterFrom = kNMGNodeCluster_Invalid;
		eClusterTo = kNMGNodeCluster_Invalid;
	}

	ENUM_NMGNodeClusterID eClusterFrom;
	ENUM_NMGNodeClusterID eClusterTo;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_TYPES_H_
