// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIPathMgr.h"
#include "AIPath.h"
#include "AI.h"
#include "AIRegionMgr.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "FastHeap.h"
#include "ProjectileTypes.h"
#include "Door.h"
#include "AIInformationVolumeMgr.h"
#include "AIBrain.h"
#include "AIMovement.h"
#include "AIPathKnowledgeMgr.h"

#define CURVE_SUBDIVISION_DEPTH 5
#define CURVE_MIN_ANGLE_DELTA_SLOW 10.f
#define CURVE_MIN_ANGLE_DELTA_FAST 40.f


// Functors

struct CompareVolumeDistance
{
	// The overloaded () operator should return true if pVolume1 has a higher heap
	// key than pVolume2. In the case of these Volumes, though, a shorter distance
	// means a higher heap key, so we are doing a "less than"

	inline bool operator()(const AIVolume* pVolume1, const AIVolume* pVolume2) const
	{
		return pVolume1->GetShortestEstimate() < pVolume2->GetShortestEstimate();
	}
};

//
// PATH_INFO
//
// Simple data struct to group arguments for clarity
//
struct PATH_INFO
{
public:
	CAI* m_pAI;
	CAIPath* m_pPath;
	AIVolume* m_pSourceVolume;
	AIVolume* m_pDesinationVolume;
	LTVector m_vPosition;
	LTVector m_vDestination;
	LTBOOL m_bDivergePaths;
};


// Globals

CAIPathMgr* g_pAIPathMgr = LTNULL;

// Statics

// Methods

CAIPathMgr::CAIPathMgr()
{
	g_pAIPathMgr = this;

    m_bInitialized = LTFALSE;
	m_nPathIndex = 0;	
	m_nWaypointID = 1;
	m_nPathKnowledgeIndex = 0;

	m_fMinCurveAngleDelta = CURVE_MIN_ANGLE_DELTA_SLOW;

	m_pAINodeMgr = debug_new( CAINodeMgr );
	m_pAIVolumeMgr = debug_new( CAIVolumeMgr );
	m_pAIInformationVolumeMgr = debug_new( CAIInformationVolumeMgr );
	m_pAIRegionMgr = debug_new( CAIRegionMgr );
}

CAIPathMgr::~CAIPathMgr()
{
	Term();

	debug_delete( m_pAINodeMgr );
	debug_delete( m_pAIVolumeMgr );
	debug_delete( m_pAIInformationVolumeMgr );
	debug_delete( m_pAIRegionMgr );
}

void CAIPathMgr::Term()
{
	g_lstGrenades.Clear();

	m_pAIRegionMgr->Term();
	m_pAIVolumeMgr->Term();
	m_pAIInformationVolumeMgr->Term();
	m_pAINodeMgr->Term();

    m_bInitialized = LTFALSE;
}

void CAIPathMgr::Init()
{
	Term();

	g_lstGrenades.Init();

	m_pAIVolumeMgr->Init();
	m_pAINodeMgr->Init();
	m_pAIInformationVolumeMgr->Init();
	m_pAIRegionMgr->Init();

#ifndef _FINAL
	m_pAINodeMgr->Verify();
#endif

    m_bInitialized = LTTRUE;
}

void CAIPathMgr::Load(ILTMessage_Read *pMsg)
{
	m_pAIRegionMgr->Load(pMsg);
	m_pAIVolumeMgr->Load(pMsg);
	m_pAIInformationVolumeMgr->Load(pMsg);
	m_pAINodeMgr->Load(pMsg);

	LOAD_FLOAT(m_fMinCurveAngleDelta);
	LOAD_BOOL(m_bInitialized);
	LOAD_DWORD(m_nPathIndex);
	LOAD_DWORD(m_nWaypointID);
	LOAD_DWORD(m_nPathKnowledgeIndex);
}

void CAIPathMgr::Save(ILTMessage_Write *pMsg)
{
	m_pAIRegionMgr->Save(pMsg);
	m_pAIVolumeMgr->Save(pMsg);
	m_pAIInformationVolumeMgr->Save(pMsg);
	m_pAINodeMgr->Save(pMsg);

	SAVE_FLOAT(m_fMinCurveAngleDelta);
	SAVE_BOOL(m_bInitialized);
	SAVE_DWORD(m_nPathIndex);
	SAVE_DWORD(m_nWaypointID);
	SAVE_DWORD(m_nPathKnowledgeIndex);
}

LTBOOL CAIPathMgr::RandomPath(CAI* pAI, AIVolume* pVolumeSrcPrev, AIVolume* pVolumeSrc, AIVolume* pVolumeSrcNext, LTFLOAT fLength, CAIPath* pPath)
{
	pPath->ClearWaypoints();
	++m_nPathIndex;

	// Square the length.

	fLength *= fLength;

	// Setup all Volumes

	for ( uint32 iVolume = 0 ; iVolume < g_pAIVolumeMgr->GetNumVolumes() ; iVolume++ )
	{
		// Initialized values, we know shortest estimate for source Volume is 0

		AIVolume* pVolume = g_pAIVolumeMgr->GetVolume(iVolume);

		if ( pVolume == pVolumeSrcPrev )
		{
			pVolume->SetShortestEstimate(0.0f);
			pVolume->SetEntryPosition(pVolumeSrc->GetCenter());
			pVolume->SetWalkthroughPosition(pVolumeSrc->GetCenter());
		}
		else
		{
			pVolume->SetShortestEstimate((float)INT_MAX);
		}

        pVolume->SetPreviousVolume(LTNULL);
	}
	
	AIVolume *pVolumePrev = pVolumeSrcPrev;
	AIVolume *pVolumeCurr = pVolumeSrc;
	AIVolume *pVolumeNext = LTNULL;

	LTBOOL bFirst = LTTRUE;

	pVolumeCurr->SetPathIndex( m_nPathIndex );

	while ( fLength > 0.0f )
	{
		LTFLOAT afDistances[16];
		AIVolume* apVolumes[16];
		uint32 cVolumes = 0;

		for ( uint32 iNeighbor = 0 ; iNeighbor < pVolumeCurr->GetNumNeighbors() ; iNeighbor++ )
		{
			AIVolumeNeighbor* pNeighbor = pVolumeCurr->GetNeighborByIndex(iNeighbor);
			pVolumeNext = pNeighbor->GetVolume();

			// The first "next" needs to be the specified next.
			if( bFirst && pVolumeSrcNext )
			{
				if( pVolumeNext != pVolumeSrcNext )
				{
					continue;
				}
			}

			// Don't go through locked doors

			if ( pVolumeNext->HasDoors() && pVolumeNext->AreDoorsLocked( pAI->m_hObject ) )
			{
				continue;
			}

			// Don't go back to our previous volume

			if ( pVolumeNext == pVolumePrev )
			{
				continue;
			}

			// Don't path into dead-ends.

			if( pVolumeNext->GetNumNeighbors() == 1 )
			{
				continue;
			}

			// Don't create loops.

			if ( pVolumeNext->GetPathIndex() == m_nPathIndex )
			{
				continue;
			}

			pNeighbor->GetVolume()->SetEntryPosition(pNeighbor->GetConnectionPos());

			// Stick it in the array of potential volumes to go to

			afDistances[cVolumes] = pVolumeCurr->GetEntryPosition().DistSqr((LTVector)pNeighbor->GetConnectionPos());
			apVolumes[cVolumes] = pVolumeNext;

			cVolumes++;
		}

		bFirst = LTFALSE;

		if ( 0 == cVolumes )
		{
			fLength = 0.0f;
		}
		else
		{
			uint32 iRandVolume = GetRandom(0, cVolumes-1);
			pVolumeNext = apVolumes[iRandVolume];
			fLength -= afDistances[iRandVolume];

			pVolumeNext->SetPathIndex( m_nPathIndex );
			pVolumeNext->SetPreviousVolume(pVolumeCurr);

			pVolumePrev = pVolumeCurr;
			pVolumeCurr = pVolumeNext;
		}
	}

	// Change previous pointers to next pointers since we need to walk the path in forward order

	ReversePath(pVolumeCurr);

	// Build the path

	BuildPath(pAI, pPath, pVolumeSrc, pVolumeCurr->GetCenter());

    return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::FindRandomPosition()
//              
//	PURPOSE:	Finds a random position at least some dist away. 
//              
//----------------------------------------------------------------------------

LTBOOL CAIPathMgr::FindRandomPosition(CAI* pAI, AIVolume* pVolume, const LTVector& vStartPos, const LTFLOAT fMinDist, LTVector* pvRandomPos)
{
	// No position to fill in.

	if( !pvRandomPos )
	{
		AIASSERT( 0, pAI->m_hObject, "CAIPathMgr::FindRandomPosition: RandomPos is NULL" );
		return LTFALSE;
	}

	// No starting volume.

	if( !pVolume )
	{
		AIASSERT( 0, pAI->m_hObject, "CAIPathMgr::FindRandomPosition: Volume is NULL" );
		return LTFALSE;
	}

	// Find a random position far enough away.

	++m_nPathIndex;
	return FindRandomPosition( pAI, pVolume, vStartPos, vStartPos, fMinDist*fMinDist, 25, 0, pvRandomPos );
}

LTBOOL CAIPathMgr::FindRandomPosition(CAI* pAI, AIVolume* pVolume, const LTVector& vStartPos, const LTVector& vEntryPoint, const LTFLOAT fMinDistSqr, const uint8 nMaxDepth, uint8 nCurDepth, LTVector* pvRandomPos)
{
	if( !pvRandomPos )
	{
		AIASSERT( 0, pAI->m_hObject, "CAIPathMgr::FindRandomPosition: RandomPos is NULL" );
		return LTFALSE;
	}

	// Index volumes to avoid loops.

	pVolume->SetPathIndex( m_nPathIndex );


	// Do not go thru doors or disabled volumes.

	if( ( pVolume->GetVolumeType() == AIVolume::kVolumeType_AmbientLife ) ||
		( pVolume->HasDoors() ) ||
		( !pVolume->IsVolumeEnabled() ) )
	{
		return LTFALSE;
	}


	// Do not choose a destination in a volume with special pathing properties.

	uint32 dwExcludeVolumes = AIVolume::kVolumeType_Ladder | 
							  AIVolume::kVolumeType_JumpUp | 
							  AIVolume::kVolumeType_JumpOver | 
							  AIVolume::kVolumeType_Teleport;

	if( !( pVolume->GetVolumeType() & dwExcludeVolumes ) )
	{
		// Check if this volume's entry point is far enough from the start point.

		if( vStartPos.DistSqr( vEntryPoint ) > fMinDistSqr )
		{
			// Get a random point in the volume.

			LTVector vDims = pVolume->GetDims();
			LTVector vCenter = pVolume->GetCenter();
			pvRandomPos->x = GetRandom( vCenter.x - vDims.x, vCenter.x + vDims.x );
			pvRandomPos->z = GetRandom( vCenter.z - vDims.z, vCenter.z + vDims.z );
			pvRandomPos->y = vCenter.y + vDims.y;

			return LTTRUE;
		}
	}
	else {
		--nCurDepth;
	}

	// Check recursion depth.

	if( nCurDepth >= nMaxDepth )
	{
		return LTFALSE;
	}

	// Randomize neighbors.

	uint8 iRand;
	uint8 nNeighbors = pVolume->GetNumNeighbors();
	AIVolumeNeighbor* apNeighbors[CAISpatialRepresentationMgr::kMaxNeighbors];
	for( uint8 iNeighbor=0; iNeighbor < pVolume->GetNumNeighbors(); ++iNeighbor )
	{
		apNeighbors[iNeighbor] = LTNULL;
	}

	uint8 cBlanks, iSpot;
	for(int iNeighbor=0; iNeighbor < pVolume->GetNumNeighbors(); ++iNeighbor )
	{
		iRand = GetRandom( 0, nNeighbors - 1 );
		cBlanks = 0;

		for( iSpot=0; iSpot < pVolume->GetNumNeighbors(); ++iSpot )
		{
			if( apNeighbors[iSpot] == LTNULL )
			{
				++cBlanks;
			}

			if( cBlanks > iRand )
			{
				apNeighbors[iSpot] = pVolume->GetNeighborByIndex( iNeighbor );
				--nNeighbors;
				break;
			}
		}
	}

	// Recurse through neighbors.

	AIVolume* pVol;
	for(int iNeighbor = 0; iNeighbor < pVolume->GetNumNeighbors(); ++iNeighbor )
	{
		// Do not recurse through previously visited volumes (pathIndex).
		// Do not go thru doors.

		pVol = apNeighbors[iNeighbor]->GetVolume();
		if( ( pVol->GetPathIndex() != m_nPathIndex ) &&
			FindRandomPosition( pAI, pVol, vStartPos, apNeighbors[iNeighbor]->GetConnectionPos(), fMinDistSqr, nMaxDepth, nCurDepth + 1, pvRandomPos ) )
		{
			return LTTRUE;
		}
	}

	// No valid point found.

	return LTFALSE;
}


//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::FindPath()
//              
//	PURPOSE:	The FindPath family of functions attempts to fill the a
//				CAIPath instance with a path from the specified location to
//				the specified location.
//              
//----------------------------------------------------------------------------
LTBOOL CAIPathMgr::FindPath(CAI* pAI, const LTVector& vPosDest, LTBOOL bDivergePaths, CAIPath* pPath)
{
	pPath->ClearWaypoints();
	PATH_INFO PathInfo;
	InitPathInfo( pAI, vPosDest, bDivergePaths, pPath, &PathInfo );
	return FindPath( &PathInfo );
}
LTBOOL CAIPathMgr::FindPath(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir, LTBOOL bDivergePaths, CAIPath* pPath)
{
	pPath->ClearWaypoints();
	PATH_INFO PathInfo;
	InitPathInfo( pAI, vPosDest, vDir, bDivergePaths, pPath, &PathInfo );
	return FindPath( &PathInfo );
}
LTBOOL CAIPathMgr::FindPath(CAI* pAI, AINode* pNodeDest, LTBOOL bDivergePaths, CAIPath* pPath)
{
	pPath->ClearWaypoints();
	PATH_INFO PathInfo;
	InitPathInfo(pAI, pNodeDest, bDivergePaths, pPath, &PathInfo);
	return FindPath( &PathInfo );
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, AIVolume* pVolumeDest, LTBOOL bDivergePaths, CAIPath* pPath)
{
	pPath->ClearWaypoints();
	PATH_INFO PathInfo;
	InitPathInfo(pAI, pVolumeDest, bDivergePaths, pPath, &PathInfo);
	return FindPath( &PathInfo );
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::HasPath()
//              
//	PURPOSE:	The HasPath family returns LTTRUE if a path from the specified
//				location to the specified location exists, false if it does
//				not.
//              
//----------------------------------------------------------------------------
LTBOOL CAIPathMgr::HasPath(CAI* pAI, const LTVector& vPosDest)
{
	PATH_INFO PathInfo;
	InitPathInfo( pAI, vPosDest, LTFALSE, LTNULL, &PathInfo );
	return HasPath( &PathInfo );
}
LTBOOL CAIPathMgr::HasPath(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir)
{
	PATH_INFO PathInfo;
	InitPathInfo( pAI, vPosDest, vDir, LTFALSE, LTNULL, &PathInfo );
	return HasPath( &PathInfo );
}
LTBOOL CAIPathMgr::HasPath(CAI* pAI, AINode* pNodeDest)
{
	PATH_INFO PathInfo;
	InitPathInfo( pAI, pNodeDest, LTFALSE, LTNULL, &PathInfo );
	return HasPath( &PathInfo );
}
LTBOOL CAIPathMgr::HasPath(CAI* pAI, AIVolume* pVolumeDest)
{
	PATH_INFO PathInfo;
	InitPathInfo( pAI, pVolumeDest, LTFALSE, LTNULL, &PathInfo );
	return HasPath( &PathInfo );
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::InitPathInfo()
//              
//	PURPOSE:	The InitPathInfo family of functions takes passed in variables 
//				and initializes a PATH_INFO structure with them.
//              
//----------------------------------------------------------------------------
void CAIPathMgr::InitPathInfo(CAI* pAI, const LTVector& vPosDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo)
{
	pPathInfo->m_pAI = pAI;
	pPathInfo->m_pPath = pPath;
	pPathInfo->m_pDesinationVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPosDest, eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume() ); 
	pPathInfo->m_pSourceVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, pAI->GetPathingPosition(), eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume() );
	pPathInfo->m_vDestination = vPosDest;
	pPathInfo->m_vPosition = pAI->GetPosition();
	pPathInfo->m_bDivergePaths = bDivergePaths;
}
void CAIPathMgr::InitPathInfo(CAI* pAI, const LTVector& vPosDest, const LTVector& vDir, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo)
{
	LTVector vPosOrigin = vPosDest + ( vDir * 5000.f );
	LTVector vNewDest;
	AIVolume* pVolumeDest;
	pVolumeDest = g_pAIVolumeMgr->FindNearestIntersectingVolume(vPosDest, vPosOrigin, pAI->GetDims().z, pAI->GetVerticalThreshold(), &vNewDest);

	pPathInfo->m_pAI = pAI;
	pPathInfo->m_pPath = pPath;
	pPathInfo->m_pDesinationVolume = pVolumeDest; 
	pPathInfo->m_pSourceVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, pAI->GetPathingPosition(), eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume());
	pPathInfo->m_vDestination = vNewDest;
	pPathInfo->m_vPosition = pAI->GetPosition();
	pPathInfo->m_bDivergePaths = bDivergePaths;
}
void CAIPathMgr::InitPathInfo(CAI* pAI, AINode* pNodeDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo)
{
	pPathInfo->m_pAI = pAI;
	pPathInfo->m_pPath = pPath;
	pPathInfo->m_pDesinationVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, pNodeDest->GetPos(), eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume()); 
	pPathInfo->m_pSourceVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, pAI->GetPathingPosition(), eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume());
	pPathInfo->m_vDestination = pNodeDest->GetPos();
	pPathInfo->m_vPosition = pAI->GetPosition();
	pPathInfo->m_bDivergePaths = bDivergePaths;
}
void CAIPathMgr::InitPathInfo(CAI* pAI, AIVolume* pVolumeDest, LTBOOL bDivergePaths, CAIPath* pPath, PATH_INFO* pPathInfo)
{
	pPathInfo->m_pAI = pAI;
	pPathInfo->m_pPath = pPath;
	pPathInfo->m_pDesinationVolume = pVolumeDest;
	pPathInfo->m_pSourceVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, pAI->GetPathingPosition(), eAxisAll, pAI->GetVerticalThreshold(), pAI->GetLastVolume());
	pPathInfo->m_vDestination = pVolumeDest ? pVolumeDest->GetCenter() : pAI->GetPathingPosition();
	pPathInfo->m_vPosition = pAI->GetPosition();
	pPathInfo->m_bDivergePaths = bDivergePaths;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::HasPath()
//              
//	PURPOSE:	Returns LTRUE if a path exists (based on volume traversal)
//				and false if it does not.  Skips the waypoint construction,
//				as it cannot fail!
//              
//----------------------------------------------------------------------------
LTBOOL CAIPathMgr::HasPath(PATH_INFO* pPathInfo)
{
	// First check if AI has previously searched for a path to this
	// same destination.  If so, return the results of that search.

	if( pPathInfo->m_pAI && pPathInfo->m_pAI->GetPathKnowledgeMgr() && pPathInfo->m_pDesinationVolume )
	{
		EnumPathBuildStatus eStatus = pPathInfo->m_pAI->GetPathKnowledgeMgr()->GetPathKnowledge( pPathInfo->m_pDesinationVolume );
		if( eStatus != kPath_Unknown )
		{
			return ( eStatus == kPath_PathFound );
		}
	}

	// No previous search results exist, so find a path.

	if ( kPath_NoPathFound == BuildVolumePath(pPathInfo))
	{
		return LTFALSE;
	}
	return LTTRUE;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::FindPath()
//              
//	PURPOSE:	Returns LTRUE if a path was constructed, LTFALSE if it was not.
//				Handles the constructions of a full path, from Volumes to
//				Waypoints
//              
//----------------------------------------------------------------------------
LTBOOL CAIPathMgr::FindPath(PATH_INFO* pPathInfo)
{
	AIASSERT( !pPathInfo->m_pAI->GetAIMovement()->IsMovementLocked(), pPathInfo->m_pAI->m_hObject, "CAIPathMgr::FindPath: Movement is locked!" );

	// First check if AI has previously searched for a path to this
	// same destination.  If so, and none was found, bail!

	if( pPathInfo->m_pAI && pPathInfo->m_pAI->GetPathKnowledgeMgr() && pPathInfo->m_pDesinationVolume )
	{
		EnumPathBuildStatus eStatus = pPathInfo->m_pAI->GetPathKnowledgeMgr()->GetPathKnowledge( pPathInfo->m_pDesinationVolume );
		if( eStatus == kPath_NoPathFound )
		{
			return LTFALSE;
		}
	}

	// No previous failed search results exist, so find a path.

	if ( kPath_NoPathFound == BuildVolumePath(pPathInfo))
	{
		return LTFALSE;
	}

	BuildWaypointPath(pPathInfo);

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::BuildVolumePath()
//              
//	PURPOSE:	Builds a series of volumes connecting a source and destination
//				Returns kNoPathFound if there is no such connecting path,
//				kPathFound if a path was found and volume pointers were set.
//              
//----------------------------------------------------------------------------
CAIPathMgr::EnumPathBuildStatus CAIPathMgr::BuildVolumePath(PATH_INFO* pPathInfo)
{
	CAI* pAI = pPathInfo->m_pAI;

	EnumPathBuildStatus eStatus = kPath_Unknown;

	if( ( !pPathInfo->m_pDesinationVolume ) || 
		( !pPathInfo->m_pDesinationVolume->CanBePathDest() ) ||
		( !pPathInfo->m_pSourceVolume && !pAI->HasLastVolume() ) )
	{
		// If there's no dest volume we're screwed. 
		// If the dest volume cannot be a path destination, we're screwed.
		// If there's no src volume, we do have a chance...
		// g_pLTServer->CPrint("pathfinding failed (0 iterations)");
        eStatus = kPath_NoPathFound;
	}

	else if ( pPathInfo->m_pSourceVolume == pPathInfo->m_pDesinationVolume )
	{
		// Just go there if we're in the same volume.
		// g_pLTServer->CPrint("pathfinding took 0 iterations");
        eStatus = kPath_PathFound;
	}

	// Check if we already know that we cannot find a path to this dest volume.

	else if( pAI->GetPathKnowledgeMgr() && pPathInfo->m_pDesinationVolume &&
			 ( pAI->GetPathKnowledgeMgr()->GetPathKnowledge( pPathInfo->m_pDesinationVolume ) == kPath_NoPathFound ) )
	{
        eStatus = kPath_NoPathFound;
	}

	// Make sure we're not going to blow the heap

	else if ( g_pAIVolumeMgr->GetNumVolumes() >= 1024 )
	{
		Warn( "**** Number of volumes exceed the max count (1024)!  No pathfinding possible! ****" );
		AIASSERT( 0, pAI->m_hObject, "More than 1024 volumes -- Up the heap size to a new max" );
		eStatus = kPath_NoPathFound;
	}

	// Early out if no djikstra's is necessary.

	if( ( eStatus != kPath_Unknown ) && pAI->GetPathKnowledgeMgr() )
	{
		if( pPathInfo->m_pDesinationVolume )
		{
			pAI->GetPathKnowledgeMgr()->RegisterPathKnowledge( pPathInfo->m_pDesinationVolume, eStatus );
		}

		return eStatus;
	}


	// pq is the priority queue (heap) used for dijkstra's

	CFastHeap<AIVolume, 1024, CompareVolumeDistance> pq;


	// Setup all Volumes

	for ( uint32 iVolume = 0 ; iVolume < g_pAIVolumeMgr->GetNumVolumes() ; iVolume++ )
	{
		// Initialized values, we know shortest estimate for source Volume is 0

		AIVolume* pVolume = g_pAIVolumeMgr->GetVolume(iVolume);

		if ( pVolume == pPathInfo->m_pSourceVolume )
		{
			pVolume->SetShortestEstimate(0.0f);
			pVolume->SetEntryPosition(pPathInfo->m_vPosition);
			pVolume->SetWalkthroughPosition(pPathInfo->m_vPosition);
		}
		else
		{
			pVolume->SetShortestEstimate(FLT_MAX);
		}

        pVolume->SetPreviousVolume(LTNULL);

		// Add to priority queue

		pq.Push(pVolume);
	}

	// Some AI cannot open doors.

	LTBOOL bUseDoors = LTTRUE;
	if( pAI->GetBrain()->GetAIDataExist( kAIData_CannotPathThruDoors ) )
	{
		if( pAI->GetBrain()->GetAIData( kAIData_CannotPathThruDoors ) == 1.f )
		{
			bUseDoors = LTFALSE;
		}
	}

	// Do djikstra's

//	int32 nPathfindingIterations = 0;
//	int32 nNeighborTouches = 0;

	AIASSERT( pAI->GetBrain(), pAI->m_hObject, "CAIPathMgr::FindPath: AI is brainless!" );
	LTFLOAT fMinPathWeight = 0.f;
	if( pAI->GetBrain() && pAI->GetBrain()->GetAIDataExist( kAIData_MinPathWeight ) )
	{
		fMinPathWeight = pAI->GetBrain()->GetAIData( kAIData_MinPathWeight );
	}

	while ( !pq.IsEmpty() )
	{
//		nPathfindingIterations++;

		// Extract the Volume on the fringe with the minimum estimate

		AIVolume *pCurrentVolume = pq.Pop();

		// If the highest priority volume already costs more than the shortest known path, this whole path is bogus

		if ( pCurrentVolume->GetShortestEstimate() > pPathInfo->m_pDesinationVolume->GetShortestEstimate() )
		{
			continue;
		}

		// Relax all the neighbors

		for ( uint32 iNeighbor = 0 ; iNeighbor < pCurrentVolume->GetNumNeighbors() ; iNeighbor++ )
		{
//			nNeighborTouches++;

			AIVolumeNeighbor* pVolumeNeighbor = pCurrentVolume->GetNeighborByIndex(iNeighbor);
			AIVolume *pNeighborVolume = pVolumeNeighbor->GetVolume();

			// AI may be resticted to using lower weighted (more preferred) volumes.

			if( ( fMinPathWeight > 0.f ) && ( pNeighborVolume->GetPathWeight( LTTRUE, LTFALSE ) > fMinPathWeight ) )
			{
				continue;
			}

			LTVector vNeighborConnection = pVolumeNeighbor->GetConnectionPos();

			// AI ignore preferred path weighting when alert.

            LTFLOAT fDistance = pCurrentVolume->GetShortestEstimate() + ( pCurrentVolume->GetPathWeight( ( pAI->GetAwareness() != kAware_Alert ), pPathInfo->m_bDivergePaths ) * pCurrentVolume->GetEntryPosition().DistSqr(vNeighborConnection) );

			if ( (pNeighborVolume->GetShortestEstimate() > fDistance) && (fDistance < pPathInfo->m_pDesinationVolume->GetShortestEstimate()) )
			{
				// Do not path thru disabled volumes.

				if( !pNeighborVolume->IsVolumeEnabled() )
				{
					continue;
				}

				if ( pNeighborVolume->HasDoors() )
				{
					// Some AI cannot use doors.

					if( !bUseDoors )
					{
						continue;
					}

					// If this is a door volume, make sure the doors aren't locked.
					// Only consider doors locked if they are not open, and are locked.
					// LevelDesigners sometimes need to lock doors in the open state.

					LTBOOL bLocked = LTFALSE;

					for ( uint32 iDoor = 0 ; iDoor < 2 ; iDoor++ )
					{
						HOBJECT hDoor = pNeighborVolume->GetDoor(iDoor);
						if ( hDoor )
						{
							Door* pDoor = (Door*)g_pLTServer->HandleToObject(hDoor);
							if( pDoor->IsLockedForCharacter(pAI->m_hObject) &&
								( pDoor->GetState() != DOORSTATE_OPEN ) )
							{
								bLocked = LTTRUE;
								break;
							}
						}	
					}

					if ( bLocked ) 
					{
						continue;
					}
				}

				// Check special properties of volume.

				if( !pCurrentVolume->CanBuildPathTo( pAI, pNeighborVolume ) )
				{
					continue;
				}

				if( !pNeighborVolume->CanBuildPathFrom( pAI, pCurrentVolume ) )
				{
					continue;
				}

				if ( !pCurrentVolume->CanBuildPathThrough( pAI, pCurrentVolume->GetPreviousVolume(), pNeighborVolume ) )
				{
					continue;
				}


				pNeighborVolume->SetShortestEstimate(fDistance);
				pNeighborVolume->SetPreviousVolume(pCurrentVolume);
				pNeighborVolume->SetEntryPosition(vNeighborConnection);
				pq.Update(pNeighborVolume);
			}
		}
	}

	// Uncomment this for debugging.
	/****
	
	AITRACE( AIShowPaths, ( pAI->m_hObject, "Built Volume Path:") ); 
	AIVolume* pVolumeDebug;
	for( pVolumeDebug = pPathInfo->m_pDesinationVolume; pVolumeDebug; pVolumeDebug = pVolumeDebug->GetPreviousVolume() )
	{
		AITRACE( AIShowPaths, ( pAI->m_hObject, "   %s", pVolumeDebug->GetName() ) ); 
	}
	****/


//	g_pLTServer->CPrint("pathfinding took %d iterations, %d neighbor touches", nPathfindingIterations, nNeighborTouches);

	if ( !pPathInfo->m_pDesinationVolume->GetPreviousVolume() )
	{
		if( pPathInfo->m_pSourceVolume )
		{
			AITRACE( AIShowVolumes, ( pAI->m_hObject, "No Path Found from %s to %s", pPathInfo->m_pSourceVolume->GetName(), pPathInfo->m_pDesinationVolume->GetName() ) );
		}
		else {
			AITRACE( AIShowVolumes, ( pAI->m_hObject, "No source volume found! No Path Found to %s.", pPathInfo->m_pDesinationVolume->GetName() ) );
		}
        eStatus = kPath_NoPathFound;
	}
	else {
        eStatus = kPath_PathFound;	
	}

	if( pAI->GetPathKnowledgeMgr() )
	{
		pAI->GetPathKnowledgeMgr()->RegisterPathKnowledge( pPathInfo->m_pDesinationVolume, eStatus );
	}

	return eStatus;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::BuildWaypointPath()
//              
//	PURPOSE:	Given volume information, constructs a series of waypoints for
//				the AI to navigate.
//              
//----------------------------------------------------------------------------
void CAIPathMgr::BuildWaypointPath(PATH_INFO* pPathInfo)
{
	if ( !pPathInfo->m_pSourceVolume )
	{
		// Get us back into the volume.. hope this works...

		pPathInfo->m_pSourceVolume = pPathInfo->m_pAI->GetLastVolume();
		CAIPathWaypoint* pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
		pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveTo );
		pWaypt->SetArgumentVector1( pPathInfo->m_pAI->GetLastVolumePos() );
		pPathInfo->m_pPath->AddWaypoint( pWaypt );
	}

	if ( pPathInfo->m_pSourceVolume == pPathInfo->m_pDesinationVolume )
	{
		// Just go there if we're in the same volume.
		CAIPathWaypoint* pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
		pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveTo );
		pWaypt->SetArgumentVector1( pPathInfo->m_vDestination );
		pPathInfo->m_pPath->AddWaypoint( pWaypt );
        return;
	}

	// Change Previous pointers to next pointers since we need to walk the path in forward order

	ReversePath(pPathInfo->m_pDesinationVolume);

	// Build the path

	BuildPath(pPathInfo->m_pAI, pPathInfo->m_pPath, pPathInfo->m_pSourceVolume, pPathInfo->m_vDestination);

/*	g_pLTServer->CPrint("path =");
	for ( int iWaypoint = 0 ; iWaypoint < pPath->GetNumWaypoints() ; iWaypoint++ )
	{
		g_pLTServer->CPrint("   pt%d = %f,%f,%f", iWaypoint, EXPANDVEC(pPath->GetWaypoint(iWaypoint)->GetArgumentVector1()));
	}*/

	return;
}


/*////////////////////////////////////////////////////

path find context

	b/vPosSrc	
	b/pVolumeSrc
	b/pRegionSrc

	b/vPosDest
	b/pVolumeDest
	b/pRegionDest

	fBiasDoorLocked
	fBiasDoorClosed
	fBiasDoorOpened
	fBiasStairs
	fBiasLadder

	fMaximumLength
	fMaximumTime

	eMovement
	eUrgency

path follow context

	fLength
	fTime

	aWaypoints
	cWaypoints
	iWaypoint


/*/////////////////////////////////////////////////////


void CAIPathMgr::ReversePath(AIVolume* pVolume, AIVolume* pVolumeNext /* = LTNULL */)
{
	if ( !pVolume )
	{
		return;
	}
	else
	{
		ReversePath(pVolume->GetPreviousVolume(), pVolume);
		pVolume->SetNextVolume(pVolumeNext);
	}
}


void CAIPathMgr::BuildPath(CAI* pAI, CAIPath* pPath, AIVolume* pVolume, const LTVector& vPosDest)
{
	_ASSERT(pPath->HasAI());

	// Connections are divided into openings of EntryWidth size.
	// These openings may be larger than the gates, which are about
	// a character width wide.
	// AI will attempt to enter a volume thru the closest opening.

    LTFLOAT fRadius = pAI->GetRadius()*1.25f;
	LTFLOAT fWidth = 2.f * fRadius;
	LTFLOAT fEntryWidth = fWidth;

	AIVolume* pPrevVolume = LTNULL;
	AIVolume* pNextVolume;
	AIVolumeNeighbor* pVolumeNeighbor;
	AIVolumeNeighbor* pLastVolumeNeighbor;
	LTVector vEntryPoint;
	VolumeGate eVolumeGate;

	LTVector vPosLast = pAI->GetPosition();
	LTVector vPosFinal = vPosDest;

	CAIPathWaypoint* pWaypt;
	CAIPathWaypoint* pLastWaypt;

	// Uncomment this for debugging.
	/***

	AITRACE( AIShowPaths, ( pAI->m_hObject, "Planning Path:") ); 
	AITRACE( AIShowPaths, ( pAI->m_hObject, "   Start: %.2f, %.2f, %.2f", vPosLast.x, vPosLast.y, vPosLast.z ) ); 
	AIVolume* pVolumeDebug;
	for( pVolumeDebug = pVolume; pVolumeDebug; pVolumeDebug = pVolumeDebug->GetNextVolume() )
	{
		AITRACE( AIShowPaths, ( pAI->m_hObject, "   %s (%.2f, %.2f, %.2f)", 
			pVolumeDebug->GetName(), pVolumeDebug->GetCenter().x, pVolumeDebug->GetCenter().y, pVolumeDebug->GetCenter().z ) ); 
	}
	***/	

	//
	// Find path control points.
	//

	static POINT_LIST s_lstControlPoints;
	s_lstControlPoints.clear();

	// Mark our starting point.

	pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
	pWaypt->SetControlPoint( pAI, CAIPathWaypoint::eInstructionMoveTo, vPosLast, 0 );
	s_lstControlPoints.push_back( vPosLast );
	pPath->AddWaypoint( pWaypt );
	pLastWaypt = pWaypt;

	AIVolume* pVolumeCur;
	for( pVolumeCur = pVolume; pVolumeCur->GetNextVolume(); pVolumeCur = pVolumeCur->GetNextVolume() )
	{
		// Get the next volume, and the neighbor data.

		pNextVolume = pVolumeCur->GetNextVolume();
		pVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor( pAI, pVolumeCur, pNextVolume );

		// Find the nearest entry into the next volume, that accomodates the 
		// requested width.  Use the gate closest to the chosen entry point.

		if( pNextVolume->GetNextVolume() )
		{
			pVolumeNeighbor->FindNearestEntryPoint( vPosLast, &vEntryPoint, fEntryWidth );
		}
		else {

			// If the next volume is the last, choose the opening closest
			// to the destination.

			pVolumeNeighbor->FindNearestEntryPoint( vPosDest, &vEntryPoint, fEntryWidth );
		}

		// The connection is divided into gates, about the width of a character.
		// Find the nearest gate to our chosen EntryPoint.
		// Only move the point if we did not get the best gate.
		// This maximizes the chance of walking in a straight line.

		LTBOOL bGotBest;
		eVolumeGate = pVolumeNeighbor->AllocateGate(vEntryPoint, &bGotBest);
		if( !bGotBest )
		{
			vEntryPoint = pVolumeNeighbor->GetGatePosition(eVolumeGate);
		}

		pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
		pWaypt->SetControlPoint( pAI, CAIPathWaypoint::eInstructionMoveTo, vEntryPoint, s_lstControlPoints.size() );

		// Add context-dependent waypoints, depending on the type of volume.

		switch( pVolumeCur->GetVolumeType() )
		{
			// Ladder waypoints.

			case AIVolume::kVolumeType_Ladder:
				BuildLadderPath( pAI, pPath, fRadius, pVolumeCur, vEntryPoint, pWaypt, pLastWaypt );
				break;

			// Jump Over waypoints.

			case AIVolume::kVolumeType_JumpOver:
				BuildJumpOverPath(pAI, pPath, fRadius, pVolumeCur, pVolumeNeighbor->GetConnectionPerpDir(), pWaypt, pLastWaypt);
				break;

			case AIVolume::kVolumeType_Teleport:
				AIASSERT(!pVolumeCur->HasDoors(), LTNULL, "Teleport volumes do not support doors" );
				BuildTeleportPath( pAI, pPath, fRadius, pVolumeCur, pWaypt, pLastWaypt );
				break;

			// Jump Up waypoints.
			// JumpUp volumes can be used for normal pathing, if not valid for a jump.

			case AIVolume::kVolumeType_JumpUp:
				if( pVolumeCur->IsValidForPath( pAI, pPrevVolume ) )
				{
					BuildJumpUpPath(pAI, pPath, fRadius, pVolumeCur, pLastVolumeNeighbor, pVolumeNeighbor->GetConnectionPerpDir(), pWaypt, pLastWaypt);
					break;
				}
				// Fall thru on purpose if not valid for jumping.

			// Default MoveTo pathing.

			default:

				// Door waypoints.
				
				if( pVolumeCur->HasDoors() )
				{
					BuildDoorPath( pAI, pPath, fRadius, pVolumeCur, pVolumeNeighbor->GetConnectionPerpDir(), pWaypt, pLastWaypt );
					s_lstControlPoints.back() = pLastWaypt->GetArgumentVector1();
				}
				break;
		}

		// Add and remember the last control point.

		pPath->AddWaypoint( pWaypt );
		s_lstControlPoints.push_back( pWaypt->GetArgumentVector1() );
		pLastWaypt = pWaypt;

		// Add a ReleaseGate waypoint.

		pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
		pWaypt->SetReleaseGatePoint( pAI, eVolumeGate, pVolumeCur, pNextVolume );
		pPath->AddWaypoint( pWaypt );

		// Remember the previous volume.

		pPrevVolume = pVolumeCur;
		pLastVolumeNeighbor = pVolumeNeighbor;
		vPosLast = vEntryPoint;
	}
	
	// Check special properties of final volume.
	// Sort of a hack to force AIs not to end in a JumpUp or Door volume.  yuk.

	switch( pVolumeCur->GetVolumeType() )
	{
		case AIVolume::kVolumeType_JumpUp:
			if( pVolumeCur->IsValidForPath( pAI, pPrevVolume ) )
			{
				pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
				BuildJumpUpPath(pAI, pPath, fRadius, pVolumeCur, pLastVolumeNeighbor, -pLastVolumeNeighbor->GetConnectionPerpDir(), pWaypt, pLastWaypt);
				pPath->AddWaypoint( pWaypt );
				s_lstControlPoints.push_back( pWaypt->GetArgumentVector1() );
			}
			break;

		default:

			// Door waypoints.
				
			if( pVolumeCur->HasDoors() )
			{
				LTVector vFTL = pVolumeCur->GetFrontTopLeft();
				LTVector vBTR = pVolumeCur->GetBackTopRight();

				if( vEntryPoint.x == vFTL.x )
				{
					vEntryPoint.x = vBTR.x;
				}
				else if( vEntryPoint.x == vBTR.x )
				{
					vEntryPoint.x = vFTL.x;
				}
				else if( vEntryPoint.z == vFTL.z )
				{
					vEntryPoint.z = vBTR.z;
				}
				else
				{
					vEntryPoint.z = vFTL.z;
				}

				pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
				pWaypt->SetControlPoint( pAI, CAIPathWaypoint::eInstructionMoveTo, vEntryPoint, s_lstControlPoints.size() );
				BuildDoorPath( pAI, pPath, fRadius, pVolumeCur, pLastVolumeNeighbor->GetConnectionPerpDir(), pWaypt, pLastWaypt );
				s_lstControlPoints.back() = pLastWaypt->GetArgumentVector1();
				pPath->AddWaypoint( pWaypt );
				vPosFinal = pWaypt->GetArgumentVector1();
				s_lstControlPoints.push_back( vPosFinal );
			}
			break;
	}

	// Add MoveTo waypoint to the final dest.

	pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
	pWaypt->SetControlPoint( pAI, CAIPathWaypoint::eInstructionMoveTo, vPosFinal, s_lstControlPoints.size() );
	s_lstControlPoints.push_back( vPosFinal );
	pPath->AddWaypoint( pWaypt );

  	AI_WAYPOINT_LIST::iterator it;


	//
	// Calculate curve waypoints.
	//

	// Iterate through control points, and insert waypoints to smooth path.
	// NOTES:
	// This step may be removed completely if curves are not desired.
	// This step may be run incrementally as the AI approaches a new volume.

	if( pAI->GetAwareness() == kAware_Relaxed )
	{
		m_fMinCurveAngleDelta = CURVE_MIN_ANGLE_DELTA_SLOW;
	}
	else {
		m_fMinCurveAngleDelta = CURVE_MIN_ANGLE_DELTA_FAST;
	}
	AI_WAYPOINT_LIST::iterator itFrom = pPath->Begin();
	for( it = pPath->Begin() + 1; it != pPath->End(); ++it )
	{
		// Is this waypoint a curve control point?

		if( (*it)->GetControlPointIndex() != -1 )
		{
			// Are curves turned on for this volume?
			// By default, curves are on, but some volumes turn them off
			// (e.g. jumping volumes, ladder volumes).

			if( (*itFrom)->GetCalculateCurve() )
			{
				// Create a hermite curve by subdividing the segement between control points.

				it = GenerateHermiteCurvePoints( pAI, pPath, s_lstControlPoints, (*itFrom)->GetControlPointIndex(), it );
			}
			itFrom = it;
		}
	}



	//
	// Bound path points by volumes and connections,
	// to ensure AI does not clip walls.
	//

	BOUND_PATH_STRUCT bps;
	bps.pAI = pAI;
	bps.pPath = pPath;
	bps.pVolumePrev = LTNULL;
	bps.pVolumeCur = pVolume;
	bps.fRadius = fRadius;
	bps.itFrom = pPath->Begin();
	bps.nLastID = (*bps.itFrom)->GetWaypointID();

	// Iterate through waypoints.
	// ReleaseGate instructions denote volume crossings.
	// NOTE:
	// This step is required, even without curves, but it may
	// be run incrementally as the AI approaches new volumes.

	uint32 nCurID;
	for( it = pPath->Begin(); it != pPath->End(); ++it )
	{
		switch( (*it)->GetInstruction() )
		{
			// Record the last seen moving waypoint.

			case CAIPathWaypoint::eInstructionMoveTo:
			case CAIPathWaypoint::eInstructionClimbUpTo:
			case CAIPathWaypoint::eInstructionClimbDownTo:
			case CAIPathWaypoint::eInstructionJumpUpTo:
			case CAIPathWaypoint::eInstructionJumpDownTo:
			case CAIPathWaypoint::eInstructionJumpOver:
			case CAIPathWaypoint::eInstructionMoveToTeleport:
			case CAIPathWaypoint::eInstructionMoveFromTeleport:
				bps.nLastID = (*it)->GetWaypointID();
				break;

			// ReleaseGate instructions appear between volumes.
			// When crossing to the next volume, bound all of the
			// waypoints in the current volume, to ensure AI will
			// not clip walls.

			case CAIPathWaypoint::eInstructionReleaseGate:
				nCurID = (*it)->GetWaypointID();
				it = BoundWaypointsToVolume( bps );
				bps.pVolumePrev = bps.pVolumeCur;
				bps.pVolumeCur = bps.pVolumeCur->GetNextVolume();
				bps.itFrom = it;
				while( (*it)->GetWaypointID() != nCurID )
				{
					++it;
				}
				break;
		}
	}
}

//------------------------------------------------------------------------------------------------------------

void CAIPathMgr::BuildDoorPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt)
{
	// Do not curve points in door volume.

	pLastWaypt->SetCalculateCurve( LTFALSE );

	// Scoot the last control point back from the door.
	// Do not scoot waypoint if AI is currently already near the door,
	// as this may cause him to turn around the wrong direction while 
	// going thru the door.

	LTVector vToDoor = pLastWaypt->GetArgumentVector1() - pAI->GetPosition();
	vToDoor.y = 0.f;
	if( vToDoor.MagSqr() > fRadius * fRadius )
	{
		LTVector vLastPos = pLastWaypt->GetArgumentVector1();
		vLastPos += vDir * fRadius;
		pLastWaypt->SetArgumentVector1( vLastPos );
	}

	LTVector vEntryPos = pWaypt->GetArgumentVector1();

	// First, face through the door

	CAIPathWaypoint* pWayptDoor = AI_FACTORY_NEW( CAIPathWaypoint );
	pWayptDoor->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceDoor, -vDir );
	pWayptDoor->SetArgumentObject1( g_pLTServer->HandleToObject( pVolume->GetDoor(0) ) );
	pWayptDoor->SetArgumentObject2( g_pLTServer->HandleToObject( pVolume->GetDoor(1) ) );

	pPath->AddWaypoint( pWayptDoor );

	// Now, open the door if it's a rotating door, or just wait for it to slide open if it's not

	pWayptDoor = AI_FACTORY_NEW( CAIPathWaypoint );
	pWayptDoor->SetAI(pAI);
	pWayptDoor->SetArgumentObject1( g_pLTServer->HandleToObject( pVolume->GetDoor(0) ) );
	pWayptDoor->SetArgumentObject2( g_pLTServer->HandleToObject( pVolume->GetDoor(1) ) );

	if ( pVolume->GetDoor(0) && IsKindOf(pVolume->GetDoor(0), "RotatingDoor") )
	{
		pWayptDoor->SetInstruction( CAIPathWaypoint::eInstructionOpenDoors );
	}
	else
	{
		pWayptDoor->SetInstruction( CAIPathWaypoint::eInstructionWaitForDoors );
	}

	pPath->AddWaypoint( pWayptDoor );

	// Scoot the final waypoint away from the other side of the door.

	vEntryPos += -vDir * fRadius;

	pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveTo );
	pWaypt->SetArgumentVector1( vEntryPos );
}

//------------------------------------------------------------------------------------------------------------

void CAIPathMgr::BuildLadderPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, 
								 AIVolume* pVolume, const LTVector& vDestPoint, 
								 CAIPathWaypoint* pWaypt, 
								 CAIPathWaypoint* pLastWaypt)
{
	// Do not curve points in ladder volume.

	pLastWaypt->SetCalculateCurve( LTFALSE );

	LTVector vCenter = (pVolume->GetFrontBottomRight() + pVolume->GetBackBottomLeft() ) * 0.5f;

	LTVector vLadderBottom = vCenter;
	vLadderBottom.y += pAI->GetDims().y;

	LTVector vLadderTop = vCenter;
	vLadderTop.y = pVolume->GetFrontTopRight().y;

	CAIPathWaypoint* pLadderWaypt;

	AIVolume* pNextVolume = pVolume->GetNextVolume();
	if ( pNextVolume )
	{
		// Vertical volume connects us to a volume above.  Go up!

		if ( pNextVolume->GetFrontBottomRight().y > pVolume->GetFrontBottomRight().y + pAI->GetDims().y )
		{
			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionLockedMoveTo, vLadderBottom );
			pPath->AddWaypoint( pLadderWaypt );

			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceLadder, pVolume->GetForward() );
			pPath->AddWaypoint( pLadderWaypt );

			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetArgumentVector1( vLadderTop );
			pLadderWaypt->SetInstruction( CAIPathWaypoint::eInstructionClimbUpTo );
			pPath->AddWaypoint( pLadderWaypt );

			pWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionGetOffLadder, vDestPoint );	
		}

		// Vertical volume either connects to one below, or is our starting volume.
		// If this is the first volume, do not treat it as a ladder.

		else if ( pPath->GetNumWaypoints() > 1 )
		{
			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionGetOnLadder, vLadderTop );
			pPath->AddWaypoint( pLadderWaypt );

			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceLadder, pVolume->GetForward() );
			pPath->AddWaypoint( pLadderWaypt );

			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetArgumentVector1( vLadderBottom );
			pLadderWaypt->SetInstruction( CAIPathWaypoint::eInstructionClimbDownTo );
			pPath->AddWaypoint( pLadderWaypt );

			LTVector vDir = vDestPoint - vLadderBottom;
			vDir.y = 0.f;
			vDir.Normalize();
			pLadderWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
			pLadderWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceLadder, vDir );
			pPath->AddWaypoint( pLadderWaypt );

			pWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionLockedMoveTo, vDestPoint );	
		}
	}
	else
	{
		Warn("Ladder volume is a dead end!!!");
	}
}

//------------------------------------------------------------------------------------------------------------

void CAIPathMgr::BuildJumpUpPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, AIVolumeNeighbor* pLastVolumeNeighbor, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt)
{
	// Do not curve points in JumpUp volume.

	pLastWaypt->SetCalculateCurve( LTFALSE );

	LTVector vFaceDir;
	if( pLastVolumeNeighbor )
	{
		vFaceDir = pLastVolumeNeighbor->GetConnectionPerpDir();
	}
	else {
		vFaceDir = vDir;
	}

	LTVector vOrigin = pLastWaypt->GetArgumentVector1();
	LTVector vDest = vOrigin - (vFaceDir * AIVolumeJumpUp::kJumpUpWidth );


	CAIPathWaypoint::Instruction eInstruction;
	AIVolume* pDestVolume = pVolume->GetNextVolume();
	if ( pDestVolume && ( pDestVolume->GetFrontBottomRight().y > vOrigin.y + pAI->GetDims().y ) )
	{
		eInstruction = CAIPathWaypoint::eInstructionJumpUpTo;
		vDest.y = pVolume->GetBackTopLeft().y - AIVolumeJumpUp::kJumpUpHeightThreshold + pAI->GetDims().y;

		// Find the actual height of the destination.

		LTVector vHeightTest = vOrigin - (vFaceDir * 91.f );
		vHeightTest.y = vDest.y;

		LTFLOAT fFloorHeight;
		if( pAI->FindFloorHeight( vHeightTest, &fFloorHeight) )
		{
			vDest.y = fFloorHeight;
		}
	}
	else
	{
		eInstruction = CAIPathWaypoint::eInstructionJumpDownTo;
		vDest.y = pVolume->GetBackBottomLeft().y + pAI->GetDims().y;

		// Scoot the last control point back from edge.

		LTVector vOffset = vFaceDir * AIVolumeJumpUp::kJumpUpStepWidth;
		pLastWaypt->SetArgumentVector1( vOrigin + vOffset );
	}

	CAIPathWaypoint* pJumpWaypt;

	pJumpWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
	pJumpWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceJumpLand, -vFaceDir );
	pPath->AddWaypoint( pJumpWaypt );

	pWaypt->SetInstruction( eInstruction );
    pWaypt->SetArgumentVector1( vDest );

	// Check if jumping down from a volume with a railing.

	if( eInstruction == CAIPathWaypoint::eInstructionJumpDownTo )
	{
		AIVolumeJumpUp* pVolJumpUp = (AIVolumeJumpUp*)pVolume;
		if( pVolJumpUp->HasRailing() )
		{
			pWaypt->SetAnimProp( kAP_OverRailing );
		}
	}

	// Do not curve in volume after jumping.
	// Jumping lands in the next volume.

	pWaypt->SetCalculateCurve( LTFALSE );
}

//------------------------------------------------------------------------------------------------------------

void CAIPathMgr::BuildJumpOverPath(CAI* pAI, CAIPath* pPath, LTFLOAT fRadius, AIVolume* pVolume, const LTVector& vDir, CAIPathWaypoint* pWaypt, CAIPathWaypoint* pLastWaypt)
{
	// Do not curve points in JumpOver volume.

	pLastWaypt->SetCalculateCurve( LTFALSE );

	LTVector vJumpingVolDims = pVolume->GetDims();

	LTVector vOrigin = pLastWaypt->GetArgumentVector1();

	LTFLOAT fLen = (vDir.x != 0.f) ? ( vJumpingVolDims.x * 2.f ) : ( vJumpingVolDims.z * 2.f );
	LTVector vDest = vOrigin - (vDir * ( fLen + 1.f ) );

	// Dest's y is set to the jumping parabola's peak height, which
	// matches the jumping volume's top.

	vDest.y = pVolume->GetBackTopLeft().y;

	CAIPathWaypoint *pJumpWaypt;

	pJumpWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
	pJumpWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionFaceJumpLand, -vDir );
	pPath->AddWaypoint( pJumpWaypt );

	pWaypt->SetInstruction( CAIPathWaypoint::eInstructionJumpOver );
    pWaypt->SetArgumentVector1( vDest );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgr::BuildTeleportPath()
//              
//	PURPOSE:	Creates a teleport path between two volumes, disabling waypoint
//				curve generation as is needed.
//              
//----------------------------------------------------------------------------
void CAIPathMgr::BuildTeleportPath(CAI* pAI, CAIPath* pPath,
								   LTFLOAT fRadius,
								   AIVolume* pVolume,
								   CAIPathWaypoint* pWaypt,
								   CAIPathWaypoint* pLastWaypt)
{
	pLastWaypt->SetCalculateCurve( LTFALSE );

	if (pVolume->GetNextVolume())
	{
		// If the next volume is a teleport volume too, then we are
		// teleporting to it, so disable pathing and set the waypoints
		// correctly.
		if ( pVolume->GetNextVolume()->GetVolumeType() == AIVolume::kVolumeType_Teleport )
		{
			// This is the boundry waypoint that we are entering from.  Between
			// this entry, and the point we are going to do the teleport from,
			// there should be no waypoints so disable the curve calculation.

			CAIPathWaypoint* pEntry = AI_FACTORY_NEW( CAIPathWaypoint );
			pEntry->SetWaypoint( pAI, CAIPathWaypoint::eInstructionMoveTo, pLastWaypt->GetArgumentVector1() );
			pEntry->SetCalculateCurve( LTFALSE );
			pPath->AddWaypoint( pEntry );

			// Set a waypoint at the center of the volume to be moved towards

			// Set the waypoint to do the teleport
			pWaypt->SetWaypoint( pAI, CAIPathWaypoint::eInstructionMoveToTeleport, pVolume->GetCenter() );
			// Save off the start and end volumes for the teleport path
			pWaypt->SetArgumentObject1( pVolume );
			pWaypt->SetArgumentObject2( pVolume->GetNextVolume() );
			pWaypt->SetCalculateCurve( LTFALSE );
		}
	}

	if (pLastWaypt->GetInstruction() == CAIPathWaypoint::eInstructionMoveToTeleport )
	{
		// If the last instruction we had was to Teleport, then we just teleported --
		// handle this by insuring movement to the origin, and then disabling the curve.
		pWaypt->SetCalculateCurve( LTFALSE );
		pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveFromTeleport );
	}
}


//------------------------------------------------------------------------------------------------------------

AI_WAYPOINT_LIST::iterator CAIPathMgr::GenerateHermiteCurvePoints(CAI* pAI, CAIPath* pPath, POINT_LIST& lstControlPoints, uint32 iControlPoint0, AI_WAYPOINT_LIST::iterator itTo)
{
	// Ensure that last waypoint has a position.

	switch( (*itTo)->GetInstruction() )
	{
			case CAIPathWaypoint::eInstructionMoveTo:
			case CAIPathWaypoint::eInstructionMoveToTeleport:
			case CAIPathWaypoint::eInstructionMoveFromTeleport:
			case CAIPathWaypoint::eInstructionClimbUpTo:
			case CAIPathWaypoint::eInstructionClimbDownTo:
			case CAIPathWaypoint::eInstructionJumpUpTo:
			case CAIPathWaypoint::eInstructionJumpDownTo:
			case CAIPathWaypoint::eInstructionJumpOver:
				break;

			default:
				AIASSERT( 0, pAI->m_hObject, "CAIPathMgr::GenerateHermiteCurvePoints: First point must be a moving waypoint." );
				break;
	}

	// Set endpoints of curve.

	LTVector p0 = lstControlPoints[iControlPoint0];
	LTVector p1 = lstControlPoints[iControlPoint0 + 1];


	// Calculate tangent vectors.

	LTVector v0, v1;
	LTVector h0, h1;
	if( iControlPoint0 > 0 )
	{
		v0 = lstControlPoints[iControlPoint0 + 1] - lstControlPoints[iControlPoint0 - 1];
		v0.Normalize();

		h0 = lstControlPoints[iControlPoint0 + 1] - lstControlPoints[iControlPoint0];
		h1 = lstControlPoints[iControlPoint0] - lstControlPoints[iControlPoint0 - 1];
		
		v0 *= Min( h0.Mag(), h1.Mag() );
	}
	else {
		v0 = lstControlPoints[iControlPoint0 + 1] - lstControlPoints[iControlPoint0];
	}

	if( iControlPoint0 < lstControlPoints.size() - 2 )
	{
		v1 = lstControlPoints[iControlPoint0 + 2] - lstControlPoints[iControlPoint0];
		v1.Normalize();

		h0 = lstControlPoints[iControlPoint0 + 2] - lstControlPoints[iControlPoint0 + 1];
		h1 = lstControlPoints[iControlPoint0 + 1] - lstControlPoints[iControlPoint0];
		
		v1 *= Min( h0.Mag(), h1.Mag() );
	}
	else {
		v1 = lstControlPoints[iControlPoint0 + 1] - lstControlPoints[iControlPoint0];
	}


	// Calculate coefficients.

	HERMITE_COEFFICIENTS_STRUCT hcs;

	// a = 2p0 - 2p1 + v0 + v1
	hcs.a = (p0 + p0) - (p1 + p1 ) + v0 + v1;

	// b = -3p0 + 3p1 -2v0 - v1
	hcs.b = -(p0 + p0 + p0) + (p1 + p1 + p1) - (v0 + v0) - v1;

	// c = v0
	hcs.c = v0;

	// d = p0
	hcs.d = p0;

	// Generate curve points.

	itTo = SubdivideHermiteCurve( pAI, pPath, itTo, CURVE_SUBDIVISION_DEPTH, hcs, 0.f, p0, 1.f, p1, LTFALSE );

	return itTo;
}

//------------------------------------------------------------------------------------------------------------

AI_WAYPOINT_LIST::iterator CAIPathMgr::SubdivideHermiteCurve(CAI* pAI, CAIPath* pPath, AI_WAYPOINT_LIST::iterator itTo,
														 int32 nLevel, const HERMITE_COEFFICIENTS_STRUCT& hcs,
														 LTFLOAT t0, const LTVector& p0, 
														 LTFLOAT t1, const LTVector& p1, 
														 LTBOOL bSubdivided)
{
	// Quit if we've recursed too deep.

	if( nLevel > 0 )
	{
		// Determine a new point at time tm, 1/3 of the way from t0 to t1.

		LTFLOAT tm = t0 + ( ( t1 - t0 ) * 0.33f );
		LTVector pm = ( ( ( hcs.a ) * tm + hcs.b ) * tm + hcs.c ) * tm + hcs.d;		

		LTFLOAT tm2 = t0 + ( ( t1 - t0 ) * 0.66f );
		LTVector pm2 = ( ( ( hcs.a ) * tm2 + hcs.b ) * tm2 + hcs.c ) * tm2 + hcs.d;		

		// If flat enough, stop subdividing and add the endpoint.

		if( FlatEnough( p0, p1, pm ) && FlatEnough( p0, p1, pm2 ) ) 
		{
			// Only add a point if this is a new point.
			// Otherwise, path will get duplicates.

			if( bSubdivided )
			{
				CAIPathWaypoint* pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
				pWaypt->SetAI( pAI );
				pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveTo );
				pWaypt->SetArgumentVector1( p1 );
				itTo = pPath->InsertWaypoint( itTo, pWaypt );
				++itTo;
			}
		}

		// Subdivide.

		else {

			// Determine a new point at time tm, 1/2 of the way from t0 to t1.

			tm = t0 + ( ( t1 - t0 ) * 0.5f );
			pm = ( ( ( hcs.a ) * tm + hcs.b ) * tm + hcs.c ) * tm + hcs.d;		

			// Subdivide segments on either side of the new point.

			itTo = SubdivideHermiteCurve( pAI, pPath, itTo, nLevel-1, hcs, t0, p0, tm, pm, LTTRUE);
			itTo = SubdivideHermiteCurve( pAI, pPath, itTo, nLevel-1, hcs, tm, pm, t1, p1, bSubdivided);
		}
	}

	return itTo;
}

//------------------------------------------------------------------------------------------------------------

LTBOOL CAIPathMgr::FlatEnough(const LTVector& p0, const LTVector& p1, const LTVector& pm) 
{
	LTVector v0 = pm;
	v0 -= p0;
	v0.y = 0.f;
	v0.Normalize();

	LTVector v1 = p1;
	v1 -= p0;
	v1.y = 0.f;
	v1.Normalize();

	// Segment is flat enough if the angle between the segment and the new
	// point are less than a specified angle.

	LTFLOAT fDot = v0.Dot( v1 );
	if( acos( fDot ) < MATH_DEGREES_TO_RADIANS( m_fMinCurveAngleDelta ) )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

//------------------------------------------------------------------------------------------------------------

AI_WAYPOINT_LIST::iterator CAIPathMgr::BoundWaypointsToVolume(const BOUND_PATH_STRUCT& bps)
{
	// Ensure that first waypoint has a position.

	switch( (*bps.itFrom)->GetInstruction() )
	{
			case CAIPathWaypoint::eInstructionMoveTo:
			case CAIPathWaypoint::eInstructionMoveToTeleport:
			case CAIPathWaypoint::eInstructionMoveFromTeleport:
			case CAIPathWaypoint::eInstructionClimbUpTo:
			case CAIPathWaypoint::eInstructionClimbDownTo:
			case CAIPathWaypoint::eInstructionJumpUpTo:
			case CAIPathWaypoint::eInstructionJumpDownTo:
			case CAIPathWaypoint::eInstructionJumpOver:
				break;

			default:
				AIASSERT( 0, bps.pAI->m_hObject, "CAIPathMgr::BoundWaypointsToVolume: First point must be a moving waypoint." );
				break;
	}


	LTVector vFrontLeft = bps.pVolumeCur->GetFrontTopLeft();
	LTVector vBackRight = bps.pVolumeCur->GetBackTopRight();

	// Setup struct used for checking instersections between the next path segment
	// and the boundaries of the connections.

	INTERSECT_CONNECTION_STRUCT ics;
	ics.fRadius = bps.fRadius;
	ics.vPointLast =  (*bps.itFrom)->GetArgumentVector1();

	if( bps.pVolumeCur->GetNextVolume() )
	{
		ics.pVolumeNeighborNext = g_pAIVolumeMgr->FindNeighbor( bps.pAI, bps.pVolumeCur, bps.pVolumeCur->GetNextVolume() );
	}

	if( bps.pVolumePrev )
	{
		ics.bInEntrance = LTTRUE;
		ics.pVolumeNeighborPrev = g_pAIVolumeMgr->FindNeighbor( bps.pAI, bps.pVolumePrev, bps.pVolumeCur );
	}

	// Iterate through all segments within the current volume.

	AI_WAYPOINT_LIST::iterator it;
	for( it = bps.itFrom + 1; (*it)->GetWaypointID() != bps.nLastID; ++it )
	{
		// Skip waypoints other than MoveTos.

		if( (*it)->GetInstruction() != CAIPathWaypoint::eInstructionMoveTo )
		{
			continue;
		}

		ics.vPointCur = (*it)->GetArgumentVector1();

		//---

		// Check intersection with entrance connection.

		if( ics.bInEntrance )
		{
			// If segment intersects, replace all previous points in this volume
			// with a new point that avoids the entrance boundaries.

			if( IntersectConnectionEdges( &ics, kEntrance ) )
			{
				it = ReplaceMoveTos( bps.pAI, bps.pPath, bps.itFrom + 1, (*it)->GetWaypointID(), ics.vPointNew );
				ics.vPointCur = ics.vPointNew;
			}
		}

		//---

		// Check intersection with exit connection.

		if( ics.pVolumeNeighborNext )
		{
			// If segment intersects, replace all following points in this volume
			// with a new point that avoids the exit boundaries.

			if( IntersectConnectionEdges( &ics, kExit ) )
			{
				it = ReplaceMoveTos( bps.pAI, bps.pPath, it, bps.nLastID, ics.vPointNew );
				ics.vPointCur = ics.vPointNew;
			}
		}

		//---

		// Do not bound the last point.

		if( (*it)->GetWaypointID() == bps.nLastID )
		{
			return it;
		}

		ics.vPointLast = ics.vPointCur;

		//---

		// Set flags indicating which volume boundaries to check point against.
		// Do not check volume boundaries where point is within a connection.

		long dwCheck = 0xffffffff;
		if( ics.bInEntrance )
		{
			dwCheck &= ~ics.pVolumeNeighborPrev->GetConnectionLocation();
		}
		if( ics.bInExit )
		{
			// Locations in next volume are reversed, because they are relative to the
			// next volume itself.  (So the next volume's left side is the current
			// volume's right side, etc.)

			switch( ics.pVolumeNeighborNext->GetConnectionLocation() )
			{
				case eVolumeConnectionLocationLeft:
				case eVolumeConnectionLocationRight:
					dwCheck &= ~( ( ics.pVolumeNeighborNext->GetConnectionLocation() == eVolumeConnectionLocationLeft ) ? eVolumeConnectionLocationRight : eVolumeConnectionLocationLeft );
					break;

				case eVolumeConnectionLocationFront:
				case eVolumeConnectionLocationBack:
					dwCheck &= ~( ( ics.pVolumeNeighborNext->GetConnectionLocation() == eVolumeConnectionLocationFront ) ? eVolumeConnectionLocationBack : eVolumeConnectionLocationFront );
					break;
			}
		}

		//---

		// Check the point against each side of the volume to determine if it is 
		// too close (based on the specified radius).

		if( ( dwCheck & eVolumeConnectionLocationLeft ) && ( ics.vPointCur.x < vFrontLeft.x + ics.fRadius ) )
		{
			ics.vPointCur.x = vFrontLeft.x + ics.fRadius;
			(*it)->SetArgumentVector1( ics.vPointCur );
		}
		else if( ( dwCheck & eVolumeConnectionLocationRight ) && ( ics.vPointCur.x > vBackRight.x - ics.fRadius ) )
		{
			ics.vPointCur.x = vBackRight.x - ics.fRadius;
			(*it)->SetArgumentVector1( ics.vPointCur );
		}

		if( ( dwCheck & eVolumeConnectionLocationFront ) && ( ics.vPointCur.z > vFrontLeft.z - ics.fRadius ) )
		{
			ics.vPointCur.z = vFrontLeft.z - ics.fRadius;
			(*it)->SetArgumentVector1( ics.vPointCur );
		}
		else if( ( dwCheck & eVolumeConnectionLocationBack ) && ( ics.vPointCur.z < vBackRight.z + ics.fRadius ) )
		{
			ics.vPointCur.z = vBackRight.z + ics.fRadius;
			(*it)->SetArgumentVector1( ics.vPointCur );
		}
	}

	return it;
}

//------------------------------------------------------------------------------------------------------------

LTBOOL CAIPathMgr::IntersectConnectionEdges(INTERSECT_CONNECTION_STRUCT* pics,
										EnumConnectionCheck eCC)

{
	AIVolumeNeighbor* pVolumeNeighbor;
	LTBOOL bWasInConnection;
	LTBOOL* pbInConnection;

	// Check the given path segment against the 
	// entrance or exit connection.

	if( eCC == kEntrance )
	{
		pVolumeNeighbor = pics->pVolumeNeighborPrev;
		bWasInConnection = pics->bInEntrance;
		pbInConnection = &( pics->bInEntrance );
	}
	else {
		pVolumeNeighbor = pics->pVolumeNeighborNext;
		bWasInConnection = pics->bInExit;
		pbInConnection = &( pics->bInExit );
	}

	// Get connection endpoints.

	LTVector vConnect1 = pVolumeNeighbor->GetConnectionEndpoint1();
	LTVector vConnect2 = pVolumeNeighbor->GetConnectionEndpoint2();
	LTFLOAT fMin, fMax;

	VolumeConnectionType eConnectionType = pVolumeNeighbor->GetConnectionType();

	switch( eConnectionType )
	{
		// Vertical connection.

		case eVolumeConnectionTypeVertical:
		{
			// Sort connection endpoints.

			if( vConnect1.z < vConnect2.z )
			{
				fMin = vConnect1.z;
				fMax = vConnect2.z;
			}
			else {
				fMin = vConnect2.z;
				fMax = vConnect1.z;
			}

			// Determine which side of the connection the point is on.
			// Point may be above, below, or inside the connection.
			// Record which z edge is crossed, if any.

			LTFLOAT z;
			if( pics->vPointCur.z <= fMin )
			{
				z = fMin;
				*pbInConnection = LTFALSE;
			}
			else if( pics->vPointCur.z >= fMax )
			{
				z = fMax;
				*pbInConnection = LTFALSE;
			}
			else {
				if( pics->vPointLast.z <= fMin )
				{
					z = fMin;
				}
				else if( pics->vPointLast.z >= fMax )
				{
					z = fMax;
				}
				else {
					bWasInConnection = LTTRUE;
				}

				*pbInConnection = LTTRUE;
			}

			// Check if the current point entered or exited the connection,
			// with respect to the last point.

			if( bWasInConnection != *pbInConnection )
			{
				VolumeConnectionLocation eConnectionLocation;
				LTVector vInner, vOuter;

				// Inner and outer points depend one whether we
				// are leaving the entrance connection, or entering 
				// the exit connection.

				if( eCC == kEntrance )
				{
					vInner = pics->vPointLast;
					vOuter = pics->vPointCur;
					eConnectionLocation = pVolumeNeighbor->GetConnectionLocation();
				}
				else {

					// Only check an exit if the current point is in the exit.

					if( !pics->bInExit )
					{
						return LTFALSE;
					}

					vInner = pics->vPointCur;
					vOuter = pics->vPointLast;
					eConnectionLocation = ( pVolumeNeighbor->GetConnectionLocation() == eVolumeConnectionLocationLeft ) ? eVolumeConnectionLocationRight : eVolumeConnectionLocationLeft;
				}

				// Don't bother checking if outer point is exactly on the 
				// connection edge.  The bounding code will push it out.

				if( vOuter.z == z )
				{
					return LTFALSE;
				}

				// Determine the minimum distance from the volume edge the inner 
				// point must be for a potential collision with the connection edge.

				LTFLOAT fBoundary;
				if( eConnectionLocation == eVolumeConnectionLocationLeft )
				{
					fBoundary = vConnect1.x + pics->fRadius;
				}
				else {
					fBoundary = vConnect1.x - pics->fRadius;
				}

				// Check if the inner point is too close to the connection.

				if( ( ( eConnectionLocation == eVolumeConnectionLocationLeft ) && ( vInner.x < fBoundary ) )
					|| ( ( eConnectionLocation == eVolumeConnectionLocationRight ) && ( vInner.x > fBoundary ) ) )
				{
					// Get the x intercept of the path segment.

					LTFLOAT fSlope = ( vOuter.z - vInner.z ) / ( vOuter.x - vInner.x );
					LTFLOAT x = ( ( z - vInner.z ) / fSlope ) + vInner.x;

					// Check if the path segment crosses the connection edge too close
					// to one side of the volume.  If so, create a new point that is far
					// enough away.

					if( ( ( eConnectionLocation == eVolumeConnectionLocationLeft ) && ( x < fBoundary ) )
						|| ( ( eConnectionLocation == eVolumeConnectionLocationRight ) && ( x > fBoundary ) ) )
					{
						pics->vPointNew.x = fBoundary;
						pics->vPointNew.y = vOuter.y;
						pics->vPointNew.z = z;
						return true;
					}
				}
			}
		}
		break;

		// Horizontal connection.

		case eVolumeConnectionTypeHorizontal:
		{
			// Sort connection endpoints.

			if( vConnect1.x < vConnect2.x )
			{
				fMin = vConnect1.x;
				fMax = vConnect2.x;
			}
			else {
				fMin = vConnect2.x;
				fMax = vConnect1.x;
			}

			// Determine which side of the connection the point is on.
			// Point may be left, right, or inside the connection.
			// Record which x edge is crossed, if any.

			LTFLOAT x;
			if( pics->vPointCur.x <= fMin )
			{
				x = fMin;
				*pbInConnection = LTFALSE;
			}
			else if( pics->vPointCur.x >= fMax )
			{
				x = fMax;
				*pbInConnection = LTFALSE;
			}
			else {
				if( pics->vPointLast.x <= fMin )
				{
					x = fMin;
				}
				else if( pics->vPointLast.x >= fMax )
				{
					x = fMax;
				}
				else {
					bWasInConnection = LTTRUE;
				}
				*pbInConnection = LTTRUE;
			}

			// Check if the current point entered or exited the connection,
			// with respect to the last point.

			if( bWasInConnection != *pbInConnection )
			{
				VolumeConnectionLocation eConnectionLocation;
				LTVector vInner, vOuter;

				// Inner and outer points depend one whether we
				// are leaving the entrance connection, or entering 
				// the exit connection.

				if( eCC == kEntrance )
				{
					vInner = pics->vPointLast;
					vOuter = pics->vPointCur;
					eConnectionLocation = pVolumeNeighbor->GetConnectionLocation();
				}
				else {

					// Only check an exit if the current point is in the exit.

					if( !pics->bInExit )
					{
						return LTFALSE;
					}

					vInner = pics->vPointCur;
					vOuter = pics->vPointLast;
					eConnectionLocation = ( pVolumeNeighbor->GetConnectionLocation() == eVolumeConnectionLocationFront ) ? eVolumeConnectionLocationBack : eVolumeConnectionLocationFront;
				}

				// Don't bother checking if outer point is exactly on the 
				// connection edge.  The bounding code will push it out.

				if( vOuter.x == x )
				{
					return LTFALSE;
				}

				// Determine the minimum distance from the volume edge the inner 
				// point must be for a potential collision with the connection edge.

				float fBoundary;
				if( eConnectionLocation == eVolumeConnectionLocationFront )
				{
					fBoundary = vConnect1.z - pics->fRadius;
				}
				else {
					fBoundary = vConnect1.z + pics->fRadius;
				}

				// Check if the inner point is too close to the connection.

				if( ( ( eConnectionLocation == eVolumeConnectionLocationFront ) && ( vInner.z > fBoundary ) )
					|| ( ( eConnectionLocation == eVolumeConnectionLocationBack ) && ( vInner.z < fBoundary ) ) )
				{
					// Get the z intercept of the path segment.

					LTFLOAT fSlope = ( vOuter.z - vInner.z ) / ( vOuter.x - vInner.x );
					LTFLOAT z = ( ( x - vInner.x ) * fSlope ) + vInner.z;

					// Check if the path segment crosses the connection edge too close
					// to one side of the volume.  If so, create a new point that is far
					// enough away.

					if( ( ( eConnectionLocation == eVolumeConnectionLocationFront ) && ( z > fBoundary ) )
						|| ( ( eConnectionLocation == eVolumeConnectionLocationBack ) && ( z < fBoundary ) ) )
					{
						pics->vPointNew.x = x;
						pics->vPointNew.y = vOuter.y;
						pics->vPointNew.z = fBoundary;
						return LTTRUE;
					}
				}
			}
		}
		break;
	}

	return LTFALSE;
}

//------------------------------------------------------------------------------------------------------------

AI_WAYPOINT_LIST::iterator CAIPathMgr::ReplaceMoveTos(CAI* pAI, CAIPath* pPath, AI_WAYPOINT_LIST::iterator it, uint32 nEndID, const LTVector& vPointNew)
{
	// Remove MoveTo waypoints between it and waypoint with ID == nEndID.

	while( (*it)->GetWaypointID() != nEndID )
	{
		if( (*it)->GetInstruction() == CAIPathWaypoint::eInstructionMoveTo )
		{
			it = pPath->RemoveWaypoint( it );
		}
		else {
			++it;
		}
	}

	// Insert a new MoveTo.

	CAIPathWaypoint* pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
	pWaypt->SetAI( pAI );
	pWaypt->SetInstruction( CAIPathWaypoint::eInstructionMoveTo );
	pWaypt->SetArgumentVector1( vPointNew );
	it = pPath->InsertWaypoint( it, pWaypt );

	return it;
}

//------------------------------------------------------------------------------------------------------------


