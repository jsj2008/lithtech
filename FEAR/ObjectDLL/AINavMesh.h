// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMesh.h
//
// PURPOSE : AI NavMesh class definition
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_H_
#define _AI_NAVMESH_H_

#include "GameBase.h"
#include "AIAssert.h"
#include "AIEnumNavMeshTypes.h"
#include "AIEnumNavMeshLinkTypes.h"
#include "AIRegion.h"
#include "AICharacterTypeRestrictions.h"

LINKTO_MODULE( AINavMesh );

//-----------------------------------------------------------------

#define AINAVMESH_VERSION_NUMBER	6				// This must be synced to the version in WorldPacker.

//-----------------------------------------------------------------

// Forward declarations.

class	CAINavMeshGen;
class	CAINavMeshGenQuadTreeNode;
class	CAINavMeshPoly;
class	CAINavMeshEdge;
class	CAINavMeshComponent;
class	CAIQuadTreeNode;
class	AINavMeshLinkAbstract;


// Externs

extern class CAINavMesh* g_pAINavMesh;


// Typedefs

typedef std::vector<AINavMeshLinkAbstract*, LTAllocator<AINavMeshLinkAbstract*, LT_MEM_TYPE_OBJECTSHELL> >	AINAVMESH_LINK_LIST;
typedef std::vector<AIRegion*, LTAllocator<AIRegion*, LT_MEM_TYPE_OBJECTSHELL> >	AIREGION_LIST;
typedef std::vector<uint32, LTAllocator<uint32, LT_MEM_TYPE_OBJECTSHELL> >	CHAR_TYPE_MASK_LIST;

//-----------------------------------------------------------------

// The only purpose for this class is to have an object in WorldEdit to
// bind brushes to.

class AINavMesh : public GameBase
{
public:
	typedef GameBase super;

	AINavMesh();
	~AINavMesh();

	// Engine

	virtual uint32 EngineMessageFn(uint32 messageID, void *pvData, float fData);
	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	void OnNavMeshCreated(int nPolyCount);

	__inline void SetNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
	{
		if( m_pdwAINavMeshPolyFlags &&
			( ePoly > kNMPoly_Invalid ) && 
			( ePoly < m_nAINavMeshPolyFlagsCount ) )
		{
			m_pdwAINavMeshPolyFlags[ePoly] |= dwFlags;
			return;
		}

		AIASSERT(0, NULL, "Failed to set navmesh poly flags.");
		return;
	}

	__inline bool TestNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
	{
		if( m_pdwAINavMeshPolyFlags &&
			( ePoly > kNMPoly_Invalid ) && 
			( ePoly < m_nAINavMeshPolyFlagsCount ) )
		{
			return ( ( m_pdwAINavMeshPolyFlags[ePoly] & dwFlags ) == dwFlags );
		}

		AIASSERT(0, NULL, "Failed to test navmesh poly flags.");
		return false;
	}

	__inline void ClearNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
	{
		if( m_pdwAINavMeshPolyFlags &&
			( ePoly > kNMPoly_Invalid ) && 
			( ePoly < m_nAINavMeshPolyFlagsCount ) )
		{
			m_pdwAINavMeshPolyFlags[ePoly] = m_pdwAINavMeshPolyFlags[ePoly] & ~dwFlags;
			return;
		}

		AIASSERT(0, NULL, "Failed to clear navmesh poly flags.");
		return;
	}

public:
	unsigned int		m_nBlindDataIndex;
	ENUM_NMNavMeshID	m_eNavMeshID;
	uint32*				m_pdwAINavMeshPolyFlags;
	int32				m_nAINavMeshPolyFlagsCount;
	uint32				m_iNavMeshCharacterTypeMask;

	CAICharacterTypeRestrictions	m_CharTypeRestrictions;
};

class AINavMeshPlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

private:

	static bool							sm_bInitted;
	CAICharacterTypeRestrictionsPlugin	m_AICharTypeRestrictionsPlugin;
};

//-----------------------------------------------------------------

class CAINavMesh
{
friend class CAINavMeshGen;
public:

	 CAINavMesh();
	~CAINavMesh();

	void	InitNavMesh();
	void	TermNavMesh();

	// Read Data.

	void	SetAINavMeshObject( AINavMesh* pAINavMesh ) { m_pAINavMeshObject = pAINavMesh; }
	void	AddNMCharTypeMask( uint32 iMaskIndex, uint32 dwMask );
	void	SortAINavMeshLinks();
	void	RuntimeSetup( uint8* pData, bool bDelete );

	void	AddAINavMeshLink( AINavMeshLinkAbstract* pLink );
	void	AddAIRegion( AIRegion* pAIRegion );

	// Flags.

	void					SetNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags );
	bool					TestNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags );
	void					ClearNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags );

	// Query.

	bool					IsNavMeshInitialized() const { return m_bNMInitialized; }

	uint32					GetNMCharTypeMask( uint32 iMaskIndex );

	int						GetNumNMPolys() { return m_cAINavMeshPolys; }
	CAINavMeshPoly*			GetNMPoly( ENUM_NMPolyID ePoly );

	int						GetNumNMPolyNormals() { return m_cAINavMeshPolyNormals; }
	bool					GetNMPolyNormal( ENUM_NMNormalID eNormal, LTVector* pvNormal );

	int						GetNumNMLinks() { return m_lstAINavMeshLinks.size(); }
	AINavMeshLinkAbstract*	GetNMLink( ENUM_NMLinkID eLink );

	CAINavMeshComponent*	GetNMComponent( ENUM_NMComponentID eComponent );

	int						GetNumNMEdges() { return m_cAINavMeshEdges; }
	CAINavMeshEdge*			GetNMEdge( ENUM_NMEdgeID eEdge );

	AIRegion*				GetAIRegion( ENUM_AIRegionID eRegion );

	static bool				GetNavMeshBlindObjectData( uint8*& blindData, uint32& blindDataSize );
	static bool				IsNavMeshBlindDataProcessed( uint8* blindData, uint32 nSize );


	// Debug rendering.

	void	UpdateDebugRendering( float fNavMeshVarTrack, float fAIRegionsVarTrack );
	void	DrawNavMesh();
	void	HideNavMesh();
	void	DrawAIRegions();
	void	HideAIRegions();
	bool	IsDrawing() const;
	void	RedrawPoly(ENUM_NMPolyID ePoly);

protected:

	void	FixUpNavMeshPointers();

	void	ConnectNMLinks();

protected:

	bool					m_bNMInitialized;
	bool					m_bDrawingNavMesh;
	bool					m_bDrawingAIRegions;
	bool*					m_pbAINavMeshFixedUp;

	AINavMesh*				m_pAINavMeshObject;
	uint8*					m_pPackedNavMeshData;
	bool					m_bDeletePackedNavMeshData;

	CHAR_TYPE_MASK_LIST		m_lstAINavMeshCharTypeMasks;

	int						m_cAINavMeshPolys;
	CAINavMeshPoly*			m_pAINavMeshPolys;

	int						m_cAINavMeshPolyNormals;
	LTVector*				m_pAINavMeshPolyNormals;

	AINAVMESH_LINK_LIST		m_lstAINavMeshLinks;
	uint32					m_cAINavMeshLinks;
	NAVMESH_LINK_DATA*		m_pAINavMeshLinkData;

	uint32					m_cAINavMeshLinkBoundaryVerts;
	LTVector*				m_pAINavMeshLinkBoundaryVerts;

	AIREGION_LIST			m_lstAIRegions;
	uint32					m_cAIRegions;
	AIREGION_DATA*			m_pAIRegionData;

	int						m_cAINavMeshComponents;
	CAINavMeshComponent*	m_pAINavMeshComponents;

	int						m_cAINavMeshComponentNeighborLists;
	ENUM_NMComponentID*		m_pAINavMeshComponentNeighborLists;	

	int						m_cAINavMeshEdges;
	CAINavMeshEdge*			m_pAINavMeshEdges;

	int						m_cAINavMeshEdgeLists;
	ENUM_NMEdgeID*			m_pAINavMeshEdgeLists;

	int						m_cAIRegionLists;
	ENUM_AIRegionID*		m_pAIRegionLists;

	int						m_cNMPolyLists;
	ENUM_NMPolyID*			m_pNMPolyLists;

	uint32					m_cClusteredAINodes;
	AINODE_CLUSTER_DATA*	m_pClusteredAINodes;

	const char*				m_pszClusteredAINodeNameList;

	uint32					m_cAIQuadTreeNodes;
	CAIQuadTreeNode*		m_pAIQuadTreeNodes;

	uint32					m_cAIQuadTreeNMPolyLists;
	ENUM_NMPolyID*			m_pAIQuadTreeNMPolyLists;
};

//-----------------------------------------------------------------

class CAINavMeshPoly
{
friend class CAINavMeshGen;
friend class CAINavMeshGenQuadTreeNode;
public:

	CAINavMeshPoly();

	// NavMesh construction.

	void				FixUpNavMeshPointers( ENUM_NMEdgeID* pAINavMeshEdgeLists, ENUM_AIRegionID* pAIRegionLists );

	// Data Access.

	ENUM_NMPolyID		GetNMPolyID() const { return m_eNMPolyID; }
	ENUM_NMNormalID		GetNMNormalID() const { return m_eNMNormalID; }
	ENUM_NMComponentID	GetNMComponentID() const { return m_eNMComponentID; }
	ENUM_NMLinkID		GetNMLinkID() const { return m_eNMLinkID; }
	uint32				GetNMCharTypeMask() const { return m_dwNMCharTypeMask; }
	const LTVector&		GetNMPolyCenter() const { return m_vNMPolyCenter; }
	float				GetNMBoundingRadius() const { return (m_aabbNMPolyBounds.vMax - m_aabbNMPolyBounds.vMin).Mag() / 2.f; }
	SAABB*				GetNMPolyAABB() { return &m_aabbNMPolyBounds; }

	// Neighbor Access.

	int				GetNumNMPolyEdges() const { return m_cNMPolyEdges; }
	CAINavMeshEdge*	GetNMPolyEdge( int iEdge );
	CAINavMeshPoly*	GetNMPolyNeighborAtEdge( int iNeighbor );
	CAINavMeshEdge*	GetNMPolyNeighborEdge( ENUM_NMPolyID eNMPolyNeighbor );

	// AIRegion Access.

	int				GetNumAIRegions() const { return m_cAIRegions; }
	ENUM_AIRegionID	GetAIRegion( int iAIRegion );
	bool			IsContainedByAIRegion( ENUM_AIRegionID eAIRegion );

	// Containment Testing.

	bool			ContainsPoint2D( const LTVector& vPos );

	// Intersection testing.

	bool			RayIntersectPoly2D( const LTVector& vRay0, const LTVector& vRay1, LTVector* pvIntersect );

	// Debug rendering.

	void	PrintVerts();
	void	DrawSelf();
	void	HideSelf();
	void	DrawSelfInAIRegion( AIRegion* pAIRegion, bool bDrawName );
	void	HideSelfInAIRegion();

protected:

	// IMPORTANT:  This data must remain in sync with data packed from NavMeshGen.

	ENUM_NMPolyID		m_eNMPolyID;
	ENUM_NMNormalID		m_eNMNormalID;
	ENUM_NMComponentID	m_eNMComponentID;
	ENUM_NMLinkID		m_eNMLinkID;

	uint32				m_dwNMCharTypeMask;

	LTVector			m_vNMPolyCenter;
	SAABB				m_aabbNMPolyBounds;

	int					m_cNMPolyEdges;
	ENUM_NMEdgeID*		m_pNMPolyEdgeList;

	int					m_cAIRegions;
	ENUM_AIRegionID*	m_pAIRegionList;
};

//-----------------------------------------------------------------

class CAINavMeshEdge
{
friend class CAINavMeshGen;
public:

	CAINavMeshEdge();

	// Data Access.

	ENUM_NMEdgeID	GetNMEdgeID() const { return m_eNMEdgeID; }
	ENUM_NMEdgeType	GetNMEdgeType() const { return m_eNMEdgeType; }
	ENUM_NMPolyID	GetNMPolyIDA() const { return m_eNMPolyIDA; }
	ENUM_NMPolyID	GetNMPolyIDB() const { return m_eNMPolyIDB; }
	const LTVector&	GetNMEdgeMidPt() const { return m_vNMEdgeMidPt; }
	const LTVector&	GetNMEdge0() const { return m_vNMEdge0; }
	const LTVector&	GetNMEdge1() const { return m_vNMEdge1; }
	bool			GetNMEdgeN( ENUM_NMPolyID eNMPolyID, LTVector* pvEdgeN );
	bool			IsNMBorderVert0() const { return ( m_bIsBorderVert0 != 0 ); }
	bool			IsNMBorderVert1() const { return ( m_bIsBorderVert1 != 0 ); }

protected:

	// IMPORTANT:  This data must remain in sync with data packed from NavMeshGen.

	ENUM_NMEdgeID	m_eNMEdgeID;

	ENUM_NMEdgeType	m_eNMEdgeType;

	LTVector		m_vNMEdge0;
	uint32			m_bIsBorderVert0;

	LTVector		m_vNMEdge1;
	uint32			m_bIsBorderVert1;

	LTVector		m_vNMEdgeMidPt;

	ENUM_NMPolyID	m_eNMPolyIDA;
	ENUM_NMPolyID	m_eNMPolyIDB;

	LTVector		m_vNMEdgeN_A;
};

//-----------------------------------------------------------------

class CAINavMeshComponent
{
	friend class CAINavMeshGen;
public:

	CAINavMeshComponent();

	// NavMesh construction.

	void						FixUpNavMeshPointers( ENUM_NMComponentID* pAINavMeshComponentNeighborLists );

	// Neighbor Access.

	int							GetNumNMComponentNeighbors() const { return m_cNMComponentNeighbors; }
	CAINavMeshComponent*		GetNMComponentNeighbor( int iNeighbor );

	// Data Access.

	ENUM_NMComponentID			GetNMComponentID() const { return m_eNMComponentID; }
	ENUM_NMSensoryComponentID	GetNMSensoryComponentID() const { return m_eNMSensoryComponentID; }

protected:

	// IMPORTANT:  This data must remain in sync with data packed from NavMeshGen.

	ENUM_NMComponentID			m_eNMComponentID;
	ENUM_NMSensoryComponentID	m_eNMSensoryComponentID;
	
	int							m_cNMComponentNeighbors;
	ENUM_NMComponentID*			m_pNMComponentNeighborList;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_H_
