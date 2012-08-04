// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenPoly.h
//
// PURPOSE : AI NavMesh generator polygon class definition
//
// CREATED : 11/02
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_POLY_H_
#define _AI_NAVMESH_GEN_POLY_H_

#include "AINavMeshGenTypes.h"
#include "IGameWorldPacker.h"

// Forward declaration.

class	CAINavMeshGen;


//-----------------------------------------------------------------

class CAINavMeshGenPoly
{
public:

	 CAINavMeshGenPoly( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback );
	~CAINavMeshGenPoly();

	void	InitNMGPoly();

	// Construction.

	void	SetNMGPolyID( ENUM_NMGPolyID eID ) { m_eNMGPolyID = eID; }
	void	SetNMGConvertedPolyID( ENUM_NMGConvertedPolyID eID ) { m_eNMGConvertedPolyID = eID; }
	void	SetNMGParentNavMeshID( ENUM_NMGNavMeshID eID ) { m_eNMGParentNavMeshID = eID; }
	void	SetNMGComponentID( ENUM_NMGComponentID eID ) { m_eNMGComponentID = eID; }
	void	SetNMGCharTypeMask( uint32 dwCharTypeMask ) { m_dwNMGCharTypeMask = dwCharTypeMask; }
	void	SetNMGLinkID( ENUM_NMGLinkID eID ) { m_eNMGLinkID = eID; }
	void	AddNMGVert( ENUM_NMGVertID eVert );
	void	RemoveNMGVert( const int iVert );
	void	InsertNMGVert( ENUM_NMGVertID eVert, ENUM_NMGVertID ePrevVert, ENUM_NMGVertID eNextVert );
	void	CopyNMGVerts( CAINavMeshGenPoly* pPolySource, ENUM_NMGVertID eVertStart );
	void	SimplifyNMGPoly();
	bool	IsNMGVertSuperfluous( ENUM_NMGVertID eVertPrev, ENUM_NMGVertID eVertCur, ENUM_NMGVertID eVertNext );
	bool	IsNMGPolyConvex();
	bool	IsNMGPolyAreaGreaterThan( float fThreshold );
	bool	PolyHasDuplicateVerts();
	float	ComputeNMGPolyTriArea( const LTVector& vVert0,  const LTVector& vVert1, const LTVector& vVert2 );
	void	GetNeighborNMGVerts( ENUM_NMGVertID eVert, ENUM_NMGVertID* peVertPrev, ENUM_NMGVertID* peVertNext );
	bool	SharesNMGEdge( ENUM_NMGVertID eVertShared, ENUM_NMGPolyID eNMGPolyID );
	bool	IsValidForMerge32( SAINAVMESHGEN_MERGE_3_2_RESULTS* pmrg32, CAINavMeshGenPoly* pPolyB, CAINavMeshGen* pAINavMeshGen );
	ENUM_NMGPolyID FindSharedNeighborPoly( ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB );
	bool	IsPolyOnOneSideOfRay( SAINAVMESHGEN_POLY_INTERSECT* pintersect );
	bool	IsPolyOnOutsideOfRay( SAINAVMESHGEN_POLY_INTERSECT* pintersect, const LTVector& vInside );
	bool	CoplanarRayIntersectPoly( SAINAVMESHGEN_POLY_INTERSECT* pintersect );
	void	SplitPoly( SAINAVMESHGEN_POLY_INTERSECT* pintersect, CAINavMeshGenPoly*& pPolyA, CAINavMeshGenPoly*& pPolyB );
	bool	FindAdjacentNMGEdges( CAINavMeshGenPoly* pPolyB, SAINAVMESHGEN_EDGE*& pEdgeA, SAINAVMESHGEN_EDGE*& pEdgeB );
	void	SplitAdjacentNMGEdge( SAINAVMESHGEN_EDGE* pEdgeA, SAINAVMESHGEN_EDGE* pEdgeB, ENUM_NMGVertID* peSplittingVertMin, ENUM_NMGVertID* peSplittingVertMax );
	bool	SplitPolyOnVert( ENUM_NMGVertID eVert, AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts, CAINavMeshGenPoly*& pPolyA, CAINavMeshGenPoly*& pPolyB );
	bool	ContainsPoint2D( const LTVector& vPos );
	bool	FindNMGOverlappingVert( CAINavMeshGenPoly* pPolyB, LTVector* pvOverlap );
	void	SetNumNMGPolyEdges( uint32 cEdges ) { m_cNMGPolyEdges = cEdges; }
	void	SetNumNMGRegions( uint32 cRegions ) { m_cNMGRegions = cRegions; }

	// Data access.

	const LTVector&			GetNMGPolyCenter() const { return m_vNMGPolyCenter; }
	const int				GetNumNMGVerts() const { return m_lstNMGVerts.size(); }
	ENUM_NMGVertID			GetNMGVert( const int iVert ) const { return m_lstNMGVerts[iVert]; }
	bool					ContainsNMGVert( ENUM_NMGVertID eVert );
	ENUM_NMGPolyID			GetNMGPolyID() const { return m_eNMGPolyID; }
	ENUM_NMGConvertedPolyID	GetNMGConvertedPolyID() const { return m_eNMGConvertedPolyID; }
	ENUM_NMGPolyID			GetNMGParentPolyID() const { return m_eNMGParentPolyID; }
	ENUM_NMGNavMeshID		GetNMGParentNavMeshID() const { return m_eNMGParentNavMeshID; }
	ENUM_NMGComponentID		GetNMGComponentID() const { return m_eNMGComponentID; }
	uint32					GetNMGCharTypeMask() const { return m_dwNMGCharTypeMask; }
	ENUM_NMGLinkID			GetNMGLinkID() const { return m_eNMGLinkID; }
	ENUM_NMGNormalID		GetNMGNormalID() const { return m_eNMGNormalID; }
	SAINAVMESHGEN_AABB*		GetAABB() { return &m_aabbNMGPolyBounds; }
	SAINAVMESHGEN_PLANE*	GetNMGPlane() { return &m_plnNMGPolyPlane; }
	const int				GetNumNMGPolyNeighbors() const { return m_lstNMGVerts.size(); }
	ENUM_NMGPolyID			GetNMGPolyNeighbor( int iNeighbor ) const;
	uint32					GetNumNMGPolyEdges() const { return m_cNMGPolyEdges; }
	uint32					GetNumNMGRegions() const { return m_cNMGRegions; }
	float					GetNMGBoundingRadius() const;

	// Debug.

	void	PrintNMGVerts();
	void	PrintActualVerts();

protected:

	int		FindNextOriginalVert( int iStart, int cSkip, int nIncr, AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts );

protected:

	NMGVERT_LIST			m_lstNMGVerts;
	LTVector				m_vNMGPolyCenter;
	ENUM_NMGPolyID			m_eNMGPolyID;
	ENUM_NMGConvertedPolyID	m_eNMGConvertedPolyID;
	ENUM_NMGPolyID			m_eNMGParentPolyID;
	ENUM_NMGNavMeshID		m_eNMGParentNavMeshID;
	ENUM_NMGComponentID		m_eNMGComponentID;
	uint32					m_dwNMGCharTypeMask;
	ENUM_NMGLinkID			m_eNMGLinkID;
	ENUM_NMGNormalID		m_eNMGNormalID;
	SAINAVMESHGEN_AABB		m_aabbNMGPolyBounds;
	SAINAVMESHGEN_PLANE		m_plnNMGPolyPlane;
	uint32					m_cNMGPolyEdges;
	uint32					m_cNMGRegions;

	// error callback used when running from the World Packer
	IGameWorldPacker::PFNERRORCALLBACK	m_pfnErrorCallback;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_POLY_H_

