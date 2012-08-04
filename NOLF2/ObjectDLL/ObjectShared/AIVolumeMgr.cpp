// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AIVolumeNeighbor.h"
#include "AI.h"

// Globals

CAIVolumeMgr* g_pAIVolumeMgr = LTNULL;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::CAIVolumeMgr()
//              
//	PURPOSE:	Hook up the AIVolumeMgr
//              
//----------------------------------------------------------------------------
CAIVolumeMgr::CAIVolumeMgr()
{
	g_pAIVolumeMgr = this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::~CAIVolumeMgr()
//              
//	PURPOSE:	Unhook the AI 
//              
//----------------------------------------------------------------------------
CAIVolumeMgr::~CAIVolumeMgr()
{
	g_pAIVolumeMgr = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::Init()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIVolumeMgr::Init()
{
	CAISpatialRepresentationMgr::Init( "AIVolume" );
	m_bDrawingVolumes = LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIVolumeMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_BOOL(m_bDrawingVolumes);
	CAISpatialRepresentationMgr::Load(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIVolumeMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL(m_bDrawingVolumes);
	CAISpatialRepresentationMgr::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIVolumeMgr::StraightRadiusPathExists
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

LTBOOL CAIVolumeMgr::StraightRadiusPathExists(CAI* pAI, const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fRadius, LTFLOAT fVerticalThreshold, uint32 dwExcludeVolumes, AIVolume* pStartVolumeHint)
{
	// Calculate the right vector for the direction of the ray.

	LTVector vForward = vDest - vOrigin;
	vForward.y = 0.f;
	vForward.Normalize();

	LTVector vRight = vForward.Cross( LTVector( 0.f, 1.f, 0.f ) );
	vRight *= fRadius;	

	// Check straight lines from the extents of radius.

	LTVector vStart1, vStart2, vEnd1, vEnd2;

	vStart1 = vOrigin + vRight;
	vStart2 = vOrigin - vRight;
	vEnd1 = vDest + vRight;
	vEnd2 = vDest - vRight;	

	if( !StraightPathExists( pAI, vStart1, vEnd1, fVerticalThreshold, dwExcludeVolumes, pStartVolumeHint ) )
	{
		return LTFALSE;
	}

	return StraightPathExists( pAI, vStart2, vEnd2, fVerticalThreshold, dwExcludeVolumes, pStartVolumeHint );
}

//----------------------------------------------------------------------------

LTBOOL CAIVolumeMgr::StraightPathExists(CAI* pAI, const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fVerticalThreshold, uint32 dwExcludeVolumes, AIVolume* pStartVolumeHint)
{
	// Find volume containing the origin.

	AIVolume* pVolumeOrigin = FindContainingVolume( LTNULL, vOrigin, eAxisAll, fVerticalThreshold, pStartVolumeHint );		
	if( ( !pVolumeOrigin ) || 
		( pVolumeOrigin->GetVolumeType() & dwExcludeVolumes ) ||
		( !( pVolumeOrigin->GetVolumeType() & pAI->GetCurValidVolumeMask() ) ) )
	{
		return LTFALSE;
	}

	// Find volume containing the destination.

	AIVolume* pVolumeDest = FindContainingVolume( LTNULL, vDest, eAxisAll, fVerticalThreshold, pStartVolumeHint );		
	if( ( !pVolumeDest ) || 
		( pVolumeDest->GetVolumeType() & dwExcludeVolumes ) ||
		( !( pVolumeDest->GetVolumeType() & pAI->GetCurValidVolumeMask() ) ) )
	{
		return LTFALSE;
	}

	// If origin and dest are in the same volume, we are done.

	if( pVolumeOrigin == pVolumeDest )
	{
		return LTTRUE;
	}

	// Walk the neighbor volumes, checking if the ray intersects with each connection.

	LTVector vIntersection;
	AIVolumeNeighbor* pNeighbor;
	LTVector vEnd1, vEnd2;
	LTBOOL bFoundNext;

	AIVolume* pVolumeLast = LTNULL;
	AIVolume* pVolumeCur = pVolumeOrigin;
	while( pVolumeCur != pVolumeDest )
	{
		// If the ray does not intersect, we are done.

		if( !RayIntersectVolume( pVolumeCur, vDest, vOrigin, fVerticalThreshold, &vIntersection) )
		{
			return LTFALSE;
		}

		// Find a neighbor who's connection segment contains the volume's intersection point.

		bFoundNext = LTFALSE;

		uint32 cNeighbors = pVolumeCur->GetNumNeighbors();
		for( uint32 iNeighbor = 0; iNeighbor < cNeighbors; ++iNeighbor )
		{
			pNeighbor = pVolumeCur->GetNeighborByIndex( iNeighbor );

			// Ignore volume we came from.

			if( pNeighbor->GetVolume() == pVolumeLast )
			{
				continue;
			}

			// Ignore excluded volumes.

			if( pNeighbor->GetVolume()->GetVolumeType() & dwExcludeVolumes )
			{
				continue;
			}

			if( !( pNeighbor->GetVolume()->GetVolumeType() & pAI->GetCurValidVolumeMask() ) )
			{
				continue;
			}

			// Ignore volumes that contain doors.

			if( pNeighbor->GetVolume()->HasDoors() )
			{
				continue;
			}

			// Ignore disabled volumes.

			if( !pNeighbor->GetVolume()->IsVolumeEnabled() )
			{
				continue;
			}

			vEnd1 = pNeighbor->GetConnectionEndpoint1();
			vEnd2 = pNeighbor->GetConnectionEndpoint2();

			// Continue if intersection is not within connection.
	
			if( ( ( vIntersection.x > vEnd1.x ) && ( vIntersection.x > vEnd2.x ) ) ||
				( ( vIntersection.x < vEnd1.x ) && ( vIntersection.x < vEnd2.x ) ) ||
				( ( vIntersection.z > vEnd1.z ) && ( vIntersection.z > vEnd2.z ) ) ||
				( ( vIntersection.z < vEnd1.z ) && ( vIntersection.z < vEnd2.z ) ) )
			{
				continue;
			}

			// Intersection was in connection.
			// Move on to checking the next volume.

			pVolumeLast = pVolumeCur;
			pVolumeCur = pNeighbor->GetVolume();
			bFoundNext = LTTRUE;
			break;
		}

		// None of the neighbors' connections contained the intersection point.

		if( !bFoundNext )
		{
			return LTFALSE;
		}
	}

	// Found a volume path to the destination.

	return LTTRUE;
}



//----------------------------------------------------------------------------

AIVolumeNeighbor* CAIVolumeMgr::FindNeighbor(CAI* pAI, AIVolume* pVolume, AIVolume* pVolumeNeighbor)
{
	return (AIVolumeNeighbor*)CAISpatialRepresentationMgr::FindNeighbor( pAI, pVolume, pVolumeNeighbor );
}
AIVolume* CAIVolumeMgr::FindContainingVolumeBruteForce(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold)
{
	return (AIVolume*)CAISpatialRepresentationMgr::FindContainingVolumeBruteForce( hObject, vPos, iAxisMask, fVerticalThreshhold );
}
AIVolume* CAIVolumeMgr::FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold, AISpatialRepresentation* pVolumeStart, LTBOOL bBruteForce )
{
	return (AIVolume*)CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce );
}
AIVolume* CAIVolumeMgr::FindNearestIntersectingVolume(const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fWidth, LTFLOAT fVerticalThreshhold, LTVector* pvIntersection)
{
	return (AIVolume*)CAISpatialRepresentationMgr::FindNearestIntersectingVolume(vOrigin, vDest, fWidth, fVerticalThreshhold, pvIntersection);
}
AIVolume* CAIVolumeMgr::GetVolume(uint32 iVolume)
{
	return (AIVolume*)CAISpatialRepresentationMgr::GetVolume(iVolume);
}
AIVolume* CAIVolumeMgr::GetVolume(const char* szVolume)
{
	return(AIVolume*)CAISpatialRepresentationMgr::GetVolume(szVolume);
}
