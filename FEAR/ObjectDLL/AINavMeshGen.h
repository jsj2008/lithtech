// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGen.h
//
// PURPOSE : AI NavMesh generator class definition
//
// CREATED : 11/02
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_H_
#define _AI_NAVMESH_GEN_H_

#include "AINavMeshGenTypes.h"
#include "IGameWorldPacker.h"

//-----------------------------------------------------------------

// Forward Declarations.

class  AINavMesh;
class  CAINavMesh;
class  CAIQuadTree;
class  CAINavMeshGenQuadTreeNode;
class  ILTOutConverter;

//-----------------------------------------------------------------

// Region Maps.
// Get the polys in an AIRegion, or get the AIRegions that a poly is in.

typedef std::multimap< 
			ENUM_NMGPolyID, 
			ENUM_NMGRegionID,
			std::less<ENUM_NMGPolyID>, 
			LTAllocator<std::pair<ENUM_NMGPolyID, ENUM_NMGRegionID>, LT_MEM_TYPE_OBJECTSHELL> 
		>  AINAVMESHGEN_POLY_TO_REGION_MAP;

typedef std::multimap< 
			ENUM_NMGRegionID,
			ENUM_NMGPolyID, 
			std::less<ENUM_NMGRegionID>, 
			LTAllocator<std::pair<ENUM_NMGRegionID, ENUM_NMGPolyID>, LT_MEM_TYPE_OBJECTSHELL> 
		>  AINAVMESHGEN_REGION_TO_POLY_MAP;

// Bounds.

typedef std::pair<
			AINAVMESHGEN_POLY_TO_REGION_MAP::iterator, 
			AINAVMESHGEN_POLY_TO_REGION_MAP::iterator
		> AINAVMESHGEN_POLY_TO_REGION_BOUNDS;

typedef std::pair<
			AINAVMESHGEN_REGION_TO_POLY_MAP::iterator, 
			AINAVMESHGEN_REGION_TO_POLY_MAP::iterator
		> AINAVMESHGEN_REGION_TO_POLY_BOUNDS;


// Conversion Maps.
// Convert between NavMeshGen IDs and NavMesh IDs.

typedef std::map< 
			ENUM_NMGEdgeID, 
			ENUM_NMGConvertedEdgeID,
			std::less<ENUM_NMGEdgeID>, 
			LTAllocator<std::pair<ENUM_NMGEdgeID, ENUM_NMGConvertedEdgeID>, LT_MEM_TYPE_OBJECTSHELL> 
		>  NMEDGEID_CONVERT_MAP;
typedef std::map< 
			ENUM_NMGPolyID, 
			ENUM_NMGConvertedPolyID,
			std::less<ENUM_NMGPolyID>, 
			LTAllocator<std::pair<ENUM_NMGPolyID, ENUM_NMGConvertedPolyID>, LT_MEM_TYPE_OBJECTSHELL> 
		>  NMPOLYID_CONVERT_MAP;

//-----------------------------------------------------------------

class CAINavMeshGen
{
public:

	CAINavMeshGen( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback );
	~CAINavMeshGen();

	bool	ImportRawNavMesh( const uint8* blindData );
	bool	ExportPackedNavMesh( ILTOutConverter& Converter );

	void	InitNavMeshGen( const LTVector& vWorldOffset );
	void	TermNavMeshGen();

	// Singleton access.

	static CAINavMeshGen* GetAINavMeshGen();

	// Pool access.

	void	GetActualVertFromPool( ENUM_NMGVertID eVert, LTVector* pvVert );
	void	GetActualNormalFromPool( ENUM_NMGNormalID eNormal, LTVector* pvNormal );

	ENUM_NMGVertID		GetExistingVertInPool( const LTVector& vVert );
	ENUM_NMGVertID		AddVertToPool( const LTVector& vVert );
	ENUM_NMGNormalID	AddNormalToPool( const LTVector& vNormal );

	// Edge access.

	SAINAVMESHGEN_EDGE*	AddNMGEdge( ENUM_NMGPolyID ePolyID, ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB );
	void				RemoveNMGEdge( ENUM_NMGPolyID ePolyID, ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB );

	// QuadTree access.

	CAINavMeshGenQuadTreeNode*	CreateNMGQTNode();
	void						DestroyNMGQTNode( CAINavMeshGenQuadTreeNode* pNMGQTNode );

	// Conversion access.

	ENUM_NMGConvertedEdgeID		ConvertToNMEdgeID( ENUM_NMGEdgeID eNMGEdgeID );
	ENUM_NMGConvertedPolyID		ConvertToNMPolyID( ENUM_NMGPolyID eNMGPolyID );

	// Restrictions access.

	uint32						GetCharTypeMask( ENUM_NMGNavMeshID eNavMeshID );

	// Data access.

	SAINAVMESHGEN_VERT*			GetNMGVert( ENUM_NMGVertID eVert );
	SAINAVMESHGEN_EDGE*			GetNMGEdge( ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB );
	CAINavMeshGenPoly*			GetNMGPoly( ENUM_NMGPolyID ePolyID );
	SAINAVMESHGEN_LINK*			GetNMGLink( ENUM_NMGLinkID eLinkID );
	CAINavMeshGenQuadTreeNode*	GetNMGQuadTree() const { return m_pNMGQuadTree; }

	// Node clustering.

	void	ClusterAINodes();

	// RunTime.

	bool	PackRunTimeNavMesh( ILTOutConverter& Converter );

protected:

	// NavMesh construction.

	void	AssignCharTypeMasks();

	void	AddNMGEdges( CAINavMeshGenPoly* pPoly );
	void	RemoveNMGEdges( CAINavMeshGenPoly* pPoly );
	void	CompactNMGEdgeList();

	void	AddNMGPoly( CAINavMeshGenPoly* pPoly );
	void	RemoveNMGPoly( CAINavMeshGenPoly* pPoly );

	void	AddPolyToAIRegion( ENUM_NMGPolyID ePolyID, ENUM_NMGRegionID eRegionID );
	void	AddPolyToParentsAIRegions( CAINavMeshGenPoly* pPoly );
	void	RemovePolyFromAIRegions( ENUM_NMGPolyID ePolyID );
	void	CompactAIRegions();

	void	AccessNMGVert( ENUM_NMGVertID eVert, ENUM_NMGPolyID ePoly );
	void	ReleaseNMGVert( ENUM_NMGVertID eVert, ENUM_NMGPolyID ePoly );
	void	AccessNMGVerts( CAINavMeshGenPoly* pPoly );
	void	ReleaseNMGVerts( CAINavMeshGenPoly* pPoly );
	void	CompactNMGVertPool();

	// Carvers.

	void	AddNMGCarver( CAINavMeshGenCarver* pCarver );
	void	CarveNMGCarvers( AINAVMESHGEN_CARVER_LIST* plstNMGCarvers, ENUM_NMGRegionID eRegionID );

	// AIRegions.

	void	AddNMGRegion( SAINAVMESHGEN_REGION* pAIRegion );
	void	CarveNMGRegions();

	// Merge polys.

	void	MergeNMGPolys( ENUM_NMGVertID eVert0, ENUM_NMGVertID eVert1, CAINavMeshGenPoly* pPolyA, CAINavMeshGenPoly* pPolyB, CAINavMeshGenPoly* pPolyC );
	void	MergeNeighbors_HertelMehlhorn();
	void	MergeNeighbors_3_2();

	// TrivialPolys.

	void	DiscardTrivialNMGPolys();

	// Adjacency.

	void	FindAdjacencies( AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts );
	void	SplitAdjacentPolys( AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts );

	// Connected components.

	void						CreateConnectedComponents();
	void						FloodFillComponentID( CAINavMeshGenPoly* pPoly );
	SAINAVMESHGEN_COMPONENT*	AddNMGComponent( ENUM_NMGComponentID eComponent );
	SAINAVMESHGEN_COMPONENT*	GetNMGComponent( ENUM_NMGComponentID eComponent );

	// Sensory components.

	void	CreateSensoryComponents();
	void	FloodFillSensoryComponentID( SAINAVMESHGEN_COMPONENT* pComponent );

	// Border Verts.

	void	MarkBorderVerts();

public:

	static const float kfEpsilon;
	static const float kfAreaThreshold;

protected:

	static CAINavMeshGen*				m_pAINavMeshGen;

	bool								m_bNMGInitialized;

	uint32								m_nAINavMeshVersion;

	LTVector							m_vWorldOffset;
	SAINAVMESHGEN_AABB					m_aabbWorldBox;

	int									m_nNextNMGPolyID;
	int									m_nNextNMGCarverID;
	int									m_nNextNMGRegionID;
	int									m_nNextNMGVertID;
	int									m_nNextNMGEdgeID;
	int									m_nNextNMGQTNodeID;

	AINAVMESHGEN_POLY_LIST				m_lstNMGPolys;
	AINAVMESHGEN_LINK_LIST				m_lstNMGLinks;
	AINAVMESHGEN_VERT_MAP				m_mapNMGVertPool;
	VECTOR_LIST							m_lstNMGNormalPool;
	AINAVMESHGEN_EDGE_MAP				m_mapNMGEdges;
	AINAVMESHGEN_CARVER_LIST			m_lstNMGCarvers;
	AINAVMESHGEN_REGION_LIST			m_lstNMGRegions;
	AINAVMESHGEN_COMPONENT_LIST			m_lstNMGComponents;
	CAINavMeshGenQuadTreeNode*			m_pNMGQuadTree;
	NMQUAD_TREE_LIST					m_lstNMGQuadTreeNodes;

	NMEDGEID_CONVERT_MAP				m_mapConvertEdgeIDs;
	NMPOLYID_CONVERT_MAP				m_mapConvertPolyIDs;

	AINAVMESHGEN_POLY_TO_REGION_MAP		m_mapNMGPolyToAIRegion;
	AINAVMESHGEN_REGION_TO_POLY_MAP		m_mapAIRegionToNMGPoly;
	NMGPOLY_LIST						m_lstDeletedAIRegionPolys;

	AINAVMESHGEN_SPLITTING_VERT_LIST	m_lstSplittingVerts;

	AINAVMESHGEN_NODE_LIST				m_lstNMGNodes;

	AINAVMESHGEN_RESTRICTIONS_LIST		m_lstNMGRestrictions;

	// error callback used when running from the World Packer
	IGameWorldPacker::PFNERRORCALLBACK	m_pfnErrorCallback;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_H_
