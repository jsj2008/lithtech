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

// Functors

struct CompareVolumeDistance
{
	// The overloaded () operator should return true if pVolume1 has a higher heap
	// key than pVolume2. In the case of these Volumes, though, a shorter distance
	// means a higher heap key, so we are doing a "less than"

	inline bool operator()(const CAIVolume* pVolume1, const CAIVolume* pVolume2) const
	{
		return pVolume1->GetShortestEstimate() < pVolume2->GetShortestEstimate();
	}
};

// Globals

CAIPathMgr* g_pAIPathMgr = LTNULL;

// Statics

static CAINodeMgr s_AINodeMgr;
static CAIVolumeMgr s_AIVolumeMgr;
static CAIRegionMgr s_AIRegionMgr;

// Methods

CAIPathMgr::CAIPathMgr()
{
	g_pAIPathMgr = this;

    m_bInitialized = LTFALSE;
}

CAIPathMgr::~CAIPathMgr()
{
	Term();
}

void CAIPathMgr::Term()
{
	g_lstGrenades.Clear();

	s_AIRegionMgr.Term();
	s_AIVolumeMgr.Term();
	s_AINodeMgr.Term();

    m_bInitialized = LTFALSE;
}

void CAIPathMgr::Init()
{
	Term();

	g_lstGrenades.Init();

	s_AINodeMgr.Init();
	s_AIVolumeMgr.Init();
	s_AIRegionMgr.Init();

#ifndef _FINAL
	s_AINodeMgr.Verify();
#endif

    m_bInitialized = LTTRUE;
}

void CAIPathMgr::Load(HMESSAGEREAD hRead)
{
	s_AIRegionMgr.Load(hRead);
	s_AIVolumeMgr.Load(hRead);
	s_AINodeMgr.Load(hRead);

	LOAD_BOOL(m_bInitialized);
}

void CAIPathMgr::Save(HMESSAGEWRITE hWrite)
{
	s_AIRegionMgr.Save(hWrite);
	s_AIVolumeMgr.Save(hWrite);
	s_AINodeMgr.Save(hWrite);

	SAVE_BOOL(m_bInitialized);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const LTVector& vPosDest, CAIPath* pPath)
{
	pPath->ClearWaypoints();

	CAIVolume* pVolumeSrc = g_pAIVolumeMgr->FindContainingVolume(pAI->GetPosition(), pAI->GetDims().y*2.0f);
	CAIVolume* pVolumeDest = g_pAIVolumeMgr->FindContainingVolume(vPosDest, pAI->GetDims().y*2.0f);

	return FindPath(pAI, pAI->GetPosition(), pVolumeSrc, vPosDest, pVolumeDest, pPath);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, CAINode* pNodeDest, CAIPath* pPath)
{
	pPath->ClearWaypoints();

	CAIVolume* pVolumeSrc = g_pAIVolumeMgr->FindContainingVolume(pAI->GetPosition(), pAI->GetDims().y*2.0f);
	CAIVolume* pVolumeDest = g_pAIVolumeMgr->FindContainingVolume(pNodeDest->GetPos(), pAI->GetDims().y*2.0f);

	return FindPath(pAI, pAI->GetPosition(), pVolumeSrc, pNodeDest->GetPos(), pVolumeDest, pPath);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, CAIVolume* pVolumeDest, CAIPath* pPath)
{
	pPath->ClearWaypoints();

	CAIVolume* pVolumeSrc = g_pAIVolumeMgr->FindContainingVolume(pAI->GetPosition(), pAI->GetDims().y*2.0f);
    LTVector vPosDest;

	return FindPath(pAI, pAI->GetPosition(), pVolumeSrc, vPosDest, pVolumeDest, pPath);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const LTVector& vPosSrc, CAIVolume* pVolumeSrc, const LTVector& vPosDest, CAIVolume* pVolumeDest, CAIPath* pPath)
{
	if ( !pVolumeDest || (!pVolumeSrc && !pAI->HasLastVolume()) )
	{
		// If there's no dest volume we're screwed. If there's no src volume, we do have a chance...

//		g_pLTServer->CPrint("pathfinding failed (0 iterations)");

        return LTFALSE;
	}

	if ( !pVolumeSrc )
	{
		// Get us back into the volume.. hope this works...

		pVolumeSrc = pAI->GetLastVolume();

		CAIPathWaypoint waypt;
		waypt.SetInstruction(CAIPathWaypoint::eInstructionMoveTo);
		waypt.SetArgumentVector1(pAI->GetLastVolumePos());
		pPath->AddWaypoint(waypt);
	}

	if ( pVolumeSrc == pVolumeDest )
	{
		// Just go there if we're in the same volume.

		CAIPathWaypoint waypt;
		waypt.SetInstruction(CAIPathWaypoint::eInstructionMoveTo);
		waypt.SetArgumentVector1(vPosDest);
		pPath->AddWaypoint(waypt);

//		g_pLTServer->CPrint("pathfinding took 0 iterations");

        return LTTRUE;
	}

	// pq is the priority queue (heap) used for dijkstra's

	CFastHeap<CAIVolume, 1024, CompareVolumeDistance> pq;

	// Make sure we're not going to blow the heap

	if ( g_pAIVolumeMgr->GetNumVolumes() >= 1024 )
	{
		return LTFALSE;
	}

	// Setup all Volumes

	for ( int iVolume = 0 ; iVolume < g_pAIVolumeMgr->GetNumVolumes() ; iVolume++ )
	{
		// Initialized values, we know shortest estimate for source Volume is 0

		CAIVolume* pVolume = g_pAIVolumeMgr->GetVolumeByIndex(iVolume);

		if ( pVolume == pVolumeSrc )
		{
			pVolume->SetShortestEstimate(0.0f);
			pVolume->SetEntryPosition(vPosSrc);
			pVolume->SetWalkthroughPosition(vPosSrc);
		}
		else
		{
			pVolume->SetShortestEstimate((float)INT_MAX);
		}

        pVolume->SetPreviousVolume(LTNULL);

		// Add to priority queue

		pq.Push(pVolume);
	}

	// Do djikstra's

//	int32 nPathfindingIterations = 0;
//	int32 nNeighborTouches = 0;

	while ( !pq.IsEmpty() )
	{
//		nPathfindingIterations++;

		// Extract the Volume on the fringe with the minimum estimate

		CAIVolume *pCurrentVolume = pq.Pop();

		// If the highest priority volume already costs more than the shortest known path, this whole path is bogus

		if ( pCurrentVolume->GetShortestEstimate() > pVolumeDest->GetShortestEstimate() )
		{
			continue;
		}

		// Relax all the neighbors

		for ( int iNeighbor = 0 ; iNeighbor < pCurrentVolume->GetNumNeighbors() ; iNeighbor++ )
		{
//			nNeighborTouches++;

			CAIVolumeNeighbor *pVolumeNeighbor = pCurrentVolume->GetNeighborByIndex(iNeighbor);
			CAIVolume *pNeighborVolume = g_pAIVolumeMgr->GetVolumeByIndex(pVolumeNeighbor->GetIndex());

            LTFLOAT fDistance = pCurrentVolume->GetShortestEstimate() + pCurrentVolume->GetEntryPosition().DistSqr((LTVector)pVolumeNeighbor->GetConnectionPos());

			if ( (pNeighborVolume->GetShortestEstimate() > fDistance) && (fDistance < pVolumeDest->GetShortestEstimate()) )
			{
				if ( pNeighborVolume->HasDoors() )
				{
					// If this is a door volume, make sure the doors aren't locked

					LTBOOL bLocked = LTFALSE;

					for ( uint32 iDoor = 0 ; iDoor < 2 ; iDoor++ )
					{
						HOBJECT hDoor = pNeighborVolume->GetDoor(iDoor);
						if ( hDoor )
						{
							Door* pDoor = (Door*)g_pLTServer->HandleToObject(hDoor);
							if ( pDoor->IsLocked() )
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

				pNeighborVolume->SetShortestEstimate(fDistance);
				pNeighborVolume->SetPreviousVolume(pCurrentVolume);
				pNeighborVolume->SetEntryPosition(pVolumeNeighbor->GetConnectionPos());
				pq.Update(pNeighborVolume);
			}
		}
	}

//	g_pLTServer->CPrint("pathfinding took %d iterations, %d neighbor touches", nPathfindingIterations, nNeighborTouches);

	if ( pVolumeDest->GetPreviousVolume() )
	{
		// Change Previous pointers to next pointers since we need to walk the path in forward order

		ReversePath(pVolumeDest);

		// Build the path

		BuildPath(pAI, pPath, pVolumeSrc, vPosDest);
/*
        g_pLTServer->CPrint("path =");
		for ( int iWaypoint = 0 ; iWaypoint < pPath->GetNumWaypoints() ; iWaypoint++ )
		{
            g_pLTServer->CPrint("   pt%d = %f,%f,%f", iWaypoint, EXPANDVEC(pPath->GetWaypoint(iWaypoint)->GetArgumentVector1()));
		}
*/
	}
	else
	{
		if ( pAI->GetDebugLevel() > 2 )
		{
	        g_pLTServer->CPrint("%s: No path from %s to %s", pAI->GetName(), pVolumeSrc->GetName(), pVolumeDest->GetName());
		}

//		g_pLTServer->CPrint("pathfinding failed (%d iterations)", nPathfindingIterations);

        return LTFALSE;
	}

    return LTTRUE;
}

void CAIPathMgr::ReversePath(CAIVolume* pVolume, CAIVolume* pVolumeNext /* = LTNULL */)
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

void CAIPathMgr::BuildPath(CAI* pAI, CAIPath* pPath, CAIVolume* pVolume, const LTVector& vPosDest)
{
	_ASSERT(pPath->HasAI());

	if ( !pVolume->GetNextVolume() )
	{
		CAIPathWaypoint waypt;
		waypt.SetInstruction(CAIPathWaypoint::eInstructionMoveTo);
		waypt.SetArgumentVector1(vPosDest);
		pPath->AddWaypoint(waypt);

		return;
	}
	else
	{
		CAIVolume* pNextVolume = pVolume->GetNextVolume();

		// Find closest point from Walkthrough point on segment connecting this volume to the next volume.
		// set the neighbor's Walkthrough point to that point
		// add a point on each side of the Walkthrough point to our list
		// this = next

		CAIVolumeNeighbor* pVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor(pVolume, pNextVolume);

        LTFLOAT fRadius = pAI->GetRadius()*1.25f;

        LTVector vEndpoint1 = pVolumeNeighbor->GetConnectionEndpoint1();
        LTVector vEndpoint2 = pVolumeNeighbor->GetConnectionEndpoint2();
        LTVector vEndpointDir = vEndpoint2 - vEndpoint1;

        LTFLOAT fLength = vEndpoint2.Dist(vEndpoint1);

		vEndpointDir /= fLength;

        LTVector vIntersectionPoint;

		// Calculate the intersection point

		{
            LTVector vNextWalkthroughPosition;

			// line :: X = Q + tw
			//
			//		Q :: pVolume->GetWalkthroughPosition()
			//		w :: NOMR(vNextWalkthroughPosition - pVolume->GetWalkthroughPosition())
			//
			// plane :: (X - P)v = 0
			//
			//		P :: pVolumeNeighbor->GetConnectionPoint()
			//		v :: pVolumeNeighbor->GetConnectionPerpDir()
			//
			// intersection = Q + ((P-Q)v/(wv))w


			if ( pNextVolume->GetNextVolume() )
			{
				CAIVolumeNeighbor* pNextVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor(pNextVolume, pNextVolume->GetNextVolume());
				vNextWalkthroughPosition = pNextVolumeNeighbor->GetConnectionPos();
			}
			else
			{
				vNextWalkthroughPosition = vPosDest;
			}

            LTVector vQ = pVolume->GetWalkthroughPosition();
			vQ.y = 0;
            LTVector vW = vNextWalkthroughPosition - vQ;
			vW.Norm();

            LTVector vv = pVolumeNeighbor->GetConnectionPerpDir();
            LTVector vP = pVolumeNeighbor->GetConnectionPos();

			vIntersectionPoint = vQ + vW*((vP-vQ).Dot(vv)/(vW.Dot(vv)));
		}

		// Clip it to the connection line segment

		LTBOOL bCorner = LTFALSE;
		{
            LTFLOAT fT = (vIntersectionPoint - vEndpoint1).Dot(vEndpointDir);

			if ( fT > (fLength-fRadius) )
			{
				// TODO: jitter away from corner
				fT = fLength-fRadius;
				bCorner = LTTRUE;
			}

			if ( fT < fRadius )
			{
				// TODO: jitter away from corner
				fT = fRadius;
				bCorner = LTTRUE;
			}

			vIntersectionPoint = vEndpoint1 + vEndpointDir*fT;
		}

		pNextVolume->SetWalkthroughPosition(vIntersectionPoint);

		CAIPathWaypoint waypt;
		waypt.SetAI(pAI);
		waypt.SetInstruction(CAIPathWaypoint::eInstructionMoveTo);

		// When we encounter a door volume, we skip the "interior" waypoints that
		// would get generated inside the volume. In the normal case of going
		// from volume A to B to C, we have 4 MoveTo waypoints, 1 in A, 2 in B,
		// and 1 in C. When B is a volume that has doors (ie, is a "door volume"),
		// then the path through A, B, and C is comprised of 1 MoveTo point in A,
		// 1 OpenDoors point in A, and 1 MoveTo point in C. We never generated any
		// waypoints inside of B (MoveTo or otherwise)

		if ( pVolume->HasDoors() || pVolume->HadDoors() )
		{

		}
		else if ( pVolume->IsVertical() )
		{

		}
		else if ( !pNextVolume->IsVertical() && (bCorner || pNextVolume->HasDoors()) )
		{
			waypt.SetArgumentVector1(vIntersectionPoint+pVolumeNeighbor->GetConnectionPerpDir()*fRadius);
			pPath->AddWaypoint(waypt);
		}

		if ( pNextVolume->HasDoors() )
		{
			CAIPathWaypoint wayptDoor;
			wayptDoor.SetAI(pAI);

			// First, face through the door

			wayptDoor.SetInstruction(CAIPathWaypoint::eInstructionFaceDoor);
			wayptDoor.SetArgumentVector1(-pVolumeNeighbor->GetConnectionPerpDir());
			wayptDoor.SetArgumentObject1(pNextVolume->GetDoor(0));
			wayptDoor.SetArgumentObject2(pNextVolume->GetDoor(1));

			pPath->AddWaypoint(wayptDoor);

			// Now, open the door if it's a rotating door, or just wait for it to slide open if it's not

			if ( pNextVolume->GetDoor(0) && IsKindOf(pNextVolume->GetDoor(0), "RotatingDoor") )
			{
				wayptDoor.SetInstruction(CAIPathWaypoint::eInstructionOpenDoors);
			}
			else
			{
				wayptDoor.SetInstruction(CAIPathWaypoint::eInstructionWaitForDoors);
			}

			// TODO: waypoints may have more than 2 doors in the future, waypoint will have to expand
			// to accommodate this and this code will need to change too.

			pPath->AddWaypoint(wayptDoor);
		}
		else if ( pNextVolume->HadDoors() )
		{
		}
		else if ( pNextVolume->IsVertical() )
		{
			float fY = 0.0f;

			CAIVolume* pNextNextVolume = pNextVolume->GetNextVolume();
			if ( pNextNextVolume )
			{
				// Vertical volume connects us to another volume... go right to it

				if ( pNextNextVolume->GetFrontBottomRight().y > pVolume->GetFrontBottomRight().y )
				{
					fY = pNextNextVolume->GetBackBottomRight().y + fRadius;
				}
				else
				{
					fY = pNextNextVolume->GetBackTopRight().y - fRadius;
				}
			}
			else
			{
				// We're ending in the vertical volume...

				if ( pNextVolume->GetFrontBottomRight().y > pVolume->GetFrontBottomRight().y )
				{
					fY = pNextVolume->GetBackBottomRight().y + fRadius;
				}
				else
				{
					fY = pNextVolume->GetBackTopRight().y - fRadius;
				}
			}

            waypt.SetArgumentVector1(LTVector(vIntersectionPoint.x, fY, vIntersectionPoint.z));
			pPath->AddWaypoint(waypt);
		}
		else if ( !pVolume->IsVertical() )
		{
			waypt.SetArgumentVector1(vIntersectionPoint-pVolumeNeighbor->GetConnectionPerpDir()*fRadius);
			pPath->AddWaypoint(waypt);
		}

		BuildPath(pAI, pPath, pVolume->GetNextVolume(), vPosDest);
	}
}