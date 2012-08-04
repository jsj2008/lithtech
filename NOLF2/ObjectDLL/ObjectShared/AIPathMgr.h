// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_PATH_MGR_H__
#define __AI_PATH_MGR_H__

#include "AIPath.h"

class CAI;
class CAIPath;
class CAINodeMgr;
class CAIVolumeMgr;
class CAIRegionMgr;
class CAIInformationVolumeMgr;
struct PATH_INFO;

// Externs

extern class CAIPathMgr* g_pAIPathMgr;

// Structs

struct HERMITE_COEFFICIENTS_STRUCT
{
	LTVector a, b, c, d;
};

struct BOUND_PATH_STRUCT
{
	BOUND_PATH_STRUCT()
	{
		pAI = LTNULL;
		pPath = LTNULL;
		pVolumePrev = LTNULL;
		pVolumeCur = LTNULL;
		fRadius = 0.f;
		nLastID = 0;
	}

	CAI*		pAI;
	CAIPath*	pPath;
	AIVolume*	pVolumePrev;
	AIVolume*	pVolumeCur;
	LTFLOAT		fRadius;
	AI_WAYPOINT_LIST::iterator itFrom;
	uint32		nLastID;
};

struct INTERSECT_CONNECTION_STRUCT
{
	INTERSECT_CONNECTION_STRUCT()
	{
		pVolumeNeighborPrev = LTNULL;
		pVolumeNeighborNext = LTNULL;
		bInEntrance = LTFALSE;
		bInExit = LTFALSE;
		fRadius = 0.f;
	}

	LTVector			vPointLast;
	LTVector			vPointCur;
	LTVector			vPointNew;
	AIVolumeNeighbor*	pVolumeNeighborPrev;
	AIVolumeNeighbor*	pVolumeNeighborNext;
	LTBOOL				bInEntrance;
	LTBOOL				bInExit;
	LTFLOAT				fRadius;
};

// Constants

enum EnumConnectionCheck
{
	kEntrance,
	kExit,
};

// Classes

class CAIPathMgr
{
	public :

		// Ctors/Dtors/etc

		CAIPathMgr();
		~CAIPathMgr();

		void Init();
		void Term();

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Path finding methods

        LTBOOL FindPath(CAI* pAI, const LTVector& vPosDest, LTBOOL bDivergePaths, CAIPath* pPath);
		LTBOOL FindPath(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir, LTBOOL bDivergePaths, CAIPath* pPath);
        LTBOOL FindPath(CAI* pAI, AINode* pNodeDest, LTBOOL bDivergePaths, CAIPath* pPath);
        LTBOOL FindPath(CAI* pAI, AIVolume* pVolumeDest, LTBOOL bDivergePaths, CAIPath* pPath);

		LTBOOL RandomPath(CAI* pAI, AIVolume* pVolumeSrcPrev, AIVolume* pVolumeSrc, AIVolume* pVolumeSrcNext, LTFLOAT fLength, CAIPath* pPath);

		LTBOOL FindRandomPosition(CAI* pAI, AIVolume* pVolume, const LTVector& vStartPos, const LTFLOAT fMinDist, LTVector* pfRandomPos);
		LTBOOL FindRandomPosition(CAI* pAI, AIVolume* pVolume, const LTVector& vStartPos, const LTVector& vEntryPoint, const LTFLOAT fMinDistSqr, const uint8 nMaxDepth, uint8 nCurDepth, LTVector* pfRandomPos);

		// Determine if there is a path 

        LTBOOL HasPath(CAI* pAI, const LTVector& vPosDest);
		LTBOOL HasPath(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir);
        LTBOOL HasPath(CAI* pAI, AINode* pNodeDest);
        LTBOOL HasPath(CAI* pAI, AIVolume* pVolumeDest);
		
		// Unique waypoint IDs.

		uint32 GetNextWaypointID() { return m_nWaypointID++; }

		// Path Knowledge Index.
		// All AIs knowledge is invalidated by incrementing the global index.

		const uint32 GetPathKnowledgeIndex() const { return m_nPathKnowledgeIndex; }
		void InvalidatePathKnowledge() { ++m_nPathKnowledgeIndex; }

		// Simple accessors

        inline LTBOOL IsInitialized() { return m_bInitialized; }

	public :

		enum EnumPathBuildStatus
		{
			kPath_Unknown,
			kPath_NoPathFound,
			kPath_PathFound,
		};

	protected :

        LTBOOL HasPath(PATH_INFO* pPathInfo);
		LTBOOL FindPath(PATH_INFO* pPathInfo);
		void BuildWaypointPath(PATH_INFO* pPathInfo);
		EnumPathBuildStatus BuildVolumePath(PATH_INFO* pPathInfo);

		void InitPathInfo(CAI* pAI, AIVolume* pVolumeDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo);
		void InitPathInfo(CAI* pAI, AINode* pNodeDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo);
		void InitPathInfo(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo);
		void InitPathInfo(CAI* pAI, const LTVector& vPosDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo);


        LTBOOL EstimatePath(CAI* pAI, const LTVector& vPosSrc, AIVolume* pVolumeSrc, const LTVector& vPosDest, AIVolume* pVolumeDest, LTFLOAT* pfDistanceEstimate);
        void ReversePath(AIVolume* pVolume, AIVolume* pVolumeNext = LTNULL);
        void BuildPath(CAI* pAI, CAIPath* pPath, AIVolume* pVolume, const LTVector& vPosDest);
		void BuildEstimate(CAI* pAI, AIVolume* pVolume, const LTVector& vPosCurrent, const LTVector& vPosDest, LTFLOAT* pfDistanceEstimate);

		// Special volume paths.

		void BuildDoorPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt);
		void BuildLadderPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, const LTVector& vDestPoint, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt);
		void BuildJumpOverPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt);
		void BuildJumpUpPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, AIVolumeNeighbor* pLastVolumeNeighbor, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt);
		void BuildTeleportPath(CAI* pAI,CAIPath* pPath,LTFLOAT fRadius,AIVolume* pVolume, CAIPathWaypoint* pWaypt,CAIPathWaypoint* pLastWaypt);

		// Hermite curves.

		AI_WAYPOINT_LIST::iterator GenerateHermiteCurvePoints(CAI* pAI, CAIPath* pPath, POINT_LIST& lstControlPoints, uint32 i0, AI_WAYPOINT_LIST::iterator itTo);
		AI_WAYPOINT_LIST::iterator SubdivideHermiteCurve(CAI* pAI, CAIPath* pPath, AI_WAYPOINT_LIST::iterator itTo, int32 nLevel, const HERMITE_COEFFICIENTS_STRUCT& hcs,
														 LTFLOAT t0, const LTVector& p0, LTFLOAT t1, const LTVector& p1, LTBOOL bSubdivided);
		LTBOOL FlatEnough(const LTVector& p0, const LTVector& p1, const LTVector& p);

		// Bounding waypoints.

		AI_WAYPOINT_LIST::iterator BoundWaypointsToVolume(const BOUND_PATH_STRUCT& bps);
		LTBOOL IntersectConnectionEdges(INTERSECT_CONNECTION_STRUCT* pics, EnumConnectionCheck eCC);
		AI_WAYPOINT_LIST::iterator ReplaceMoveTos(CAI* pAI, CAIPath* pPath, AI_WAYPOINT_LIST::iterator it, uint32 nEndID, const LTVector& vPointNew);

	private :

        LTBOOL			m_bInitialized;
		uint32			m_nPathIndex;
		uint32			m_nWaypointID;
		uint32			m_nPathKnowledgeIndex;

		LTFLOAT			m_fMinCurveAngleDelta;

		CAINodeMgr*				m_pAINodeMgr;
		CAIInformationVolumeMgr* m_pAIInformationVolumeMgr;
		CAIVolumeMgr*			m_pAIVolumeMgr;
		CAIRegionMgr*			m_pAIRegionMgr;
};

#endif // __AI_PATH_MGR_H__
