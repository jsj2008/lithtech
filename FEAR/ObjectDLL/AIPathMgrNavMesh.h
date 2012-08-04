// ----------------------------------------------------------------------- //
//
// MODULE  : AIPathMgrNavMesh.h
//
// PURPOSE : PathMgr definition for finding paths on a NavMesh.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_PATHMGR_NAVMESH_H_
#define _AI_PATHMGR_NAVMESH_H_

#include "AIClassFactory.h"
#include "AIEnumNavMeshTypes.h"
#include "AIEnumNavMeshLinkTypes.h"


// Forward declarations.

class	CAI;
class	CAIPathMgrNavMesh;
class	CAIAStarMachine;
class	CAIAStarNodeAbstract;
class	CAIAStarMapNavMesh;
class	CAIAStarStorageNavMesh;
class	CAIAStarGoalNavMesh;
class	CAIAStarGoalNavMeshSafe;
class	CAIAStarMapNavMeshStraightPath;
class	CAIAStarGoalNavMeshEscape;
class	CAIAStarMapNavMeshEscape;

extern CAIPathMgrNavMesh *g_pAIPathMgrNavMesh;

#define DO_NOT_PULL_STRINGS	0

//-----------------------------------------------------------------

enum ENUM_AI_PATH_TYPE
{
	kPath_Default,
	kPath_Safe,
};

//-----------------------------------------------------------------

struct SPATH_CACHED_STRAIGHT_PATH
{
	LTObjRef	hAI;
	uint32		dwCharTypeMask;
	LTVector	vSource;
	LTVector	vDest;
	int			iPathKnowledgeIndex;
	bool		bResult;
};

//-----------------------------------------------------------------

struct SPATH_CACHED_ESCAPE_PATH
{
	LTObjRef	hAI;
	uint32		dwCharTypeMask;
	LTVector	vSource;
	LTVector	vDanger;
	float		fClearance;
	LTVector	vClearDest;
	int			iPathKnowledgeIndex;
	bool		bResult;
};

//-----------------------------------------------------------------

struct SPATH_NODE 
{
	ENUM_NMPolyID	ePoly;
	ENUM_NMEdgeID	eEdge;
	LTVector		vLink0;
	LTVector		vLink1;
	LTVector		vWayPt;
	LTVector		vAStarEntry;
	bool			bTrimLink0;
	bool			bTrimLink1;
	bool			bOptional;
	ENUM_NMLinkID	eOffsetEntryToLink;
};
typedef std::vector<SPATH_NODE, LTAllocator<SPATH_NODE, LT_MEM_TYPE_OBJECTSHELL> > NMPATH;

class CAIPathNavMesh : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAIPathNavMesh );

		// Ctors/Dtors/etc

		 CAIPathNavMesh();
		~CAIPathNavMesh();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		// Path generation.

		void	ResetPath( unsigned int cNodes );
		void	SetPathSource( const LTVector& vSource ) { m_vSource = vSource; }
		void	SetPathDest( const LTVector& vDest ) { m_vDest = vDest; }
		void	SetPathNodePoly( unsigned int iNode, ENUM_NMPolyID ePoly, const LTVector& vEntry );
		void	GeneratePathLinks( CAI* pAI );

		// Path optimization.

		void	PullStrings( CAI* pAI, uint32 nMaxIterations );

		// Path following.

		void	ReserveNavMeshLinks( CAI* pAI, bool bReserve );
		void	ApplyNMLinkDeathDelay( CAI* pAI );
		void	SkipOptionalPathNodes( CAI* pAI );
		void	IncrementCurPathNodeIndex( CAI* pAI );

		// Query.

		bool			IsPathComplete() const { return ( m_iCurPathNode >= m_NMPath.size() ); }
		unsigned int	GetCurPathNodeIndex() const { return m_iCurPathNode; }
		SPATH_NODE*		GetCurPathNode() { return ( m_iCurPathNode < m_NMPath.size() ) ? &( m_NMPath[m_iCurPathNode] ) : NULL; }
		SPATH_NODE*		GetPathNode( unsigned int iNode ) { return ( iNode < m_NMPath.size() ) ? &( m_NMPath[iNode] ) : NULL; }
		unsigned int	GetPathLength() const { return m_NMPath.size(); }
		const LTVector&	GetPathSource() const { return m_vSource; }
		const LTVector&	GetPathDest() const { return m_vDest; }

		float			GetPathDistance() const;

	protected:

		void			AdvanceNextNMLink( CAI* pAI, unsigned int iPathNode );

	protected:
		
		LTVector		m_vSource;
		LTVector		m_vDest;
		NMPATH			m_NMPath;
		unsigned int	m_iCurPathNode;
};

//-----------------------------------------------------------------

struct SPATH_INVALIDATION_REQUEST
{
	LTObjRef hInvalidator;
	double fTime;
};
typedef std::vector<SPATH_INVALIDATION_REQUEST, LTAllocator<SPATH_INVALIDATION_REQUEST, LT_MEM_TYPE_OBJECTSHELL> > PATH_INVALIDATION_REQUEST_LIST;


class CAIPathMgrNavMesh
{
	public :

		enum EnumPathBuildStatus
		{
			kPath_Unknown,
			kPath_NoPathFound,
			kPath_PathFound,
		};

	public:
	
		 CAIPathMgrNavMesh();
		~CAIPathMgrNavMesh();

		void	InitPathMgrNavMesh();
		void	TermPathMgrNavMesh();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		bool	HasPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vDest );
		bool	HasPath( CAI* pAI, uint32 dwCharTypeMask, ENUM_NMPolyID eNavMeshPolyDest );

		bool	FindPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, ENUM_NMPolyID eNavMeshPolySource, ENUM_NMPolyID eNavMeshPolyDest, uint32 nPullStringsMaxIters, ENUM_AI_PATH_TYPE ePathType, CAIPathNavMesh* pPath );

		bool	StraightPathExists( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, ENUM_NMPolyID eLastPoly, float fRadius );
		bool	EscapePathExists( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance, ENUM_NMPolyID eLastPoly, LTVector* pvClearDest );

		// Path Knowledge Index.
		// All AIs knowledge is invalidated by incrementing the global index.

		const uint32	GetPathKnowledgeIndex();
		void			InvalidatePathKnowledge( HOBJECT hInvalidator );
		void			PostPathKnowledgeInvalidationRequest( HOBJECT hInvalidator, double fTime );

	protected:

		CAIAStarNodeAbstract*	FindPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest,ENUM_NMPolyID eNavMeshPolySource, ENUM_NMPolyID eNavMeshPolyDest, CAIAStarGoalNavMesh* pAStarGoal );

		void					CacheStraightPathResult( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, bool bResult );
		void					CacheEscapePathResult( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance, const LTVector& vClearDest, bool bResult );

	protected:

		CAIAStarMachine*				m_pAStarMachine;
		CAIAStarMapNavMesh*				m_pAStarMap;
		CAIAStarStorageNavMesh*			m_pAStarStorage;
		CAIAStarGoalNavMesh*			m_pAStarGoal;

		CAIAStarGoalNavMeshSafe*		m_pAStarGoalSafe;

		CAIAStarMapNavMeshStraightPath*	m_pAStarMapStraightPath;
		SPATH_CACHED_STRAIGHT_PATH		m_CachedStraightPath;

		CAIAStarGoalNavMeshEscape*		m_pAStarGoalEscape;
		CAIAStarMapNavMeshEscape*		m_pAStarMapEscape;
		SPATH_CACHED_ESCAPE_PATH		m_CachedEscapePath;

		uint32							m_nPathKnowledgeIndex;
		PATH_INVALIDATION_REQUEST_LIST	m_lstPathInvalidationRequests;
};

//-----------------------------------------------------------------

#endif // _AI_PATHMGR_NAVMESH_H_
