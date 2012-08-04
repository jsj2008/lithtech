
//----------------------------------------------------------------------------
//
//	MODULE:		AISpatialRepresentationMgr.cpp
//
//	PURPOSE:	- implementation
//
//	CREATED:	18.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AISPATIALREPRESENTATIONMGR_H__
#include "AISpatialRepresentationMgr.h"
#endif

#include "AI.h"
#include "AIPath.h"
#include "FastHeap.h"
#include "WorldProperties.h"
#include "AIUtils.h"

#include <algorithm>

// Forward declarations

// Globals

// Statics



//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::CAISpatialRepresentationMgr()
//
//	PURPOSE:	Initializes the SpatialRepMgr
//
//----------------------------------------------------------------------------
CAISpatialRepresentationMgr::CAISpatialRepresentationMgr()
{
	m_bInitialized = LTFALSE;
	m_bDrawingVolumes = LTFALSE;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::~CAISpatialRepresentationMgr()
//
//	PURPOSE:	Cleans up the SpatialRepMgr
//
//----------------------------------------------------------------------------
CAISpatialRepresentationMgr::~CAISpatialRepresentationMgr()
{
}

void CAISpatialRepresentationMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_BOOL(m_bInitialized);

	uint32 nVolumes;
	LOAD_INT(nVolumes);
	m_listpVolumes.clear( );
	m_listpVolumes.resize(nVolumes);

    for ( uint32 iVolume = 0 ; iVolume < nVolumes; iVolume++ )
	{
		LOAD_COBJECT(m_listpVolumes[iVolume], AISpatialRepresentation);
	}
}

void CAISpatialRepresentationMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL(m_bInitialized);

	SAVE_INT(m_listpVolumes.size());
	for ( uint32 iVolume = 0; iVolume < m_listpVolumes.size(); iVolume++ )
	{
		SAVE_COBJECT(m_listpVolumes[iVolume]);
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::Term()
//
//	PURPOSE:	Deinitializes the CAISpatialRepresentationMgr
//
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::Term()
{
	m_bInitialized = LTFALSE;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::CountInstances()
//
//	PURPOSE:	Counts the instances of a class of objects.
//
//----------------------------------------------------------------------------
int CAISpatialRepresentationMgr::CountInstances(const char* const szClass) const
{
	int nInstances = 0;
	HCLASS  hClass = g_pLTServer->GetClass((char*)szClass);
	HOBJECT	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
		{
			nInstances++;
		}
	}

	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
		{
			nInstances++;
		}
	}

	nInstances += g_pGameServerShell->GetLiteObjectMgr()->GetObjectsOfClass(hClass, LTNULL);

	return nInstances;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::SetupInstanceArray()
//
//	PURPOSE:	Sets the Vector of pointers to all instances of type Volume
//
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::SetupInstanceArray(const char* const szClass)
{
	uint32 nId = 0;
	HCLASS hClass = g_pLTServer->GetClass((char*)szClass);
	HOBJECT hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
		{
			// Setup the volume
			m_listpVolumes[nId] = (AISpatialRepresentation*)g_pLTServer->HandleToObject(hCurObject);
			nId++;
		}
	}

	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
		{
			// Setup the volume

			m_listpVolumes[nId] = (AISpatialRepresentation*)g_pLTServer->HandleToObject(hCurObject);
			nId++;
		}
	}

	CLiteObjectMgr::TObjectList aLiteObjects;
	g_pGameServerShell->GetLiteObjectMgr()->GetObjectsOfClass(hClass, &aLiteObjects);
	CLiteObjectMgr::TObjectList::iterator iCurObj = aLiteObjects.begin();
	for (; iCurObj != aLiteObjects.end(); ++iCurObj)
	{
		m_listpVolumes[nId] = (AISpatialRepresentation*)*iCurObj;
		nId++;
	}
}

struct SetupNeighbors : public std::binary_function< AISpatialRepresentation*, CAISpatialRepresentationMgr*, bool >
{
	bool operator() ( AISpatialRepresentation* pVolume, CAISpatialRepresentationMgr* VolumeMgr ) const
	{
		VolumeMgr->SetupVolumesNeighbors(pVolume);
		return true;
	}
};

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAISpatialRepresentationMgr::Init()
//
//	PURPOSE:	Initializes the CAISpatialRepresentationMgr to use a specific
//				type of volume the passed classname.  Sets up the Vector of
//				instance pointers, and calls Init() on all members
//
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::Init(const char* const szClass)
{
	Term();

	// First, we count up the number of szClass instances in the level

	m_listpVolumes.resize( CountInstances(szClass) );

	// Now we put the Volumes int32o our array

	SetupInstanceArray( szClass );

	// Initialize all of the volumes

	std::for_each(
		m_listpVolumes.begin(),
		m_listpVolumes.end(),
		std::mem_fun( &AISpatialRepresentation::Init ) );

	// Build the neighboring connections

	std::for_each(
		m_listpVolumes.begin(),
		m_listpVolumes.end(),
		std::bind2nd( SetupNeighbors(), this ) );

	m_bInitialized = LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::SetupVolumesNeighbors()
//              
//	PURPOSE:	Sets up and initializes a volumes neighbors.  This 
//              
//	NOTE:		This funtionality may belong in the volumes themselves.
//
//----------------------------------------------------------------------------
int CAISpatialRepresentationMgr::SetupVolumesNeighbors(AISpatialRepresentation* pVolume)
{
	int32 cNeighbors = 0;
	AISpatialRepresentation* apVolumeNeighbors[kMaxNeighbors];

	// Get Neighbors by shared edge
	for ( uint32 iNeighborVolume = 0 ; iNeighborVolume < m_listpVolumes.size(); iNeighborVolume++ )
	{
		// Do not allow neighboring self
		if ( pVolume == m_listpVolumes[iNeighborVolume] )
			continue;

		// If they do not allow unconnected volumes to be neighbors
		if ( !pVolume->IsConnected( m_listpVolumes[iNeighborVolume] ) )
			continue;

		/*	char szDebug[128];
			sprint32f(szDebug, "%s neighbors %s", m_apVolumes[iVolume]->GetName(), m_apVolumes[iNeighborVolume].GetName());
			OutputDebugString(szDebug);
		*/

		if ( cNeighbors >= kMaxNeighbors )
		{
			g_pLTServer->CPrint("WARNING: Volume '%s' has exceeded the max (%d) number of neighboring volumes!!!!",
				pVolume->GetName(), kMaxNeighbors );
			break;
		}

		// Add the volume to our list
		apVolumeNeighbors[cNeighbors++] = m_listpVolumes[iNeighborVolume];
	}

	pVolume->InitNeighbors((AISpatialRepresentation**)apVolumeNeighbors, cNeighbors);

	/*	g_pLTServer->CPrint("restated: %s has %d neighbors", m_apVolumes[iVolume]->GetName(), m_apVolumes[iVolume]->GetNumNeighbors());
	{for ( int32 iNeighborVolume = 0 ; iNeighborVolume < m_apVolumes[iVolume]->GetNumNeighbors() ; iNeighborVolume++ )
	{
		int32 iNeighbor = m_apVolumes[iVolume]->GetNeighborByIndex(iNeighborVolume)->GetVolumeId();
        g_pLTServer->CPrint("           %s", m_apVolumes[iNeighbor].GetName());
	}}*/
	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::FindContainingVolumeBruteForce()
//              
//	PURPOSE:	Returns a pointer to the volume containing the passed position
//              
//----------------------------------------------------------------------------
AISpatialRepresentation* CAISpatialRepresentationMgr::FindContainingVolumeBruteForce(HOBJECT hObject,
																					 const LTVector& vPos,
																					 int iAxisMask,
																					 LTFLOAT fVerticalThreshhold)
{
	// Set use flags if hObject was passed in.
	// If no hObject was provided, match all volumes.

	uint32 dwUseBy = AISpatialRepresentation::kUseBy_All;
	if( hObject )
	{
		if( IsAI( hObject ) )
		{
			dwUseBy = AISpatialRepresentation::kUseBy_AI;
		}
		else if( IsPlayer( hObject ) )
		{
			dwUseBy = AISpatialRepresentation::kUseBy_Player;
		}
	}

	// The really, really, stupid way.

	for ( uint32 iVolume = 0 ; iVolume < m_listpVolumes.size(); iVolume++ )
	{
		// Skip disabled volumes.

		if( !m_listpVolumes[iVolume]->IsVolumeEnabled() )
		{
			continue;
		}

		if ( ( m_listpVolumes[iVolume]->GetUseFlags() & dwUseBy ) && 
			m_listpVolumes[iVolume]->InsideMasked(vPos, iAxisMask, fVerticalThreshhold) )
		{
			return m_listpVolumes[iVolume];
		}
	}

	return LTNULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::RayIntersectVolume()
//              
//	PURPOSE:	Returns LTTrue if the Volume is intersected by a ray, false if
//				it is not.
//              
//----------------------------------------------------------------------------
LTBOOL CAISpatialRepresentationMgr::RayIntersectVolume(AISpatialRepresentation* pVolume,
														 const LTVector& vOrigin,
														 const LTVector& vDest,
														 LTFLOAT fVerticalThreshhold,
														 LTVector* pvIntersection)
{
	LTVector vMin = pVolume->GetBackBottomLeft();
	LTVector vMax = pVolume->GetFrontTopRight();

	vMin.y -= fVerticalThreshhold;
	vMax.y += fVerticalThreshhold;

	return RayIntersectBox( vMin, vMax, vOrigin, vDest, pvIntersection );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::FindNeighbor()
//              
//	PURPOSE:	Returns the AISpatialNeighbor if there is one between
//				the passed in volumes
//              
//----------------------------------------------------------------------------
AISpatialNeighbor* CAISpatialRepresentationMgr::FindNeighbor(CAI* pAI,
																	AISpatialRepresentation* pVolume,
																	AISpatialRepresentation* pVolumeNeighbor)
{
	AIASSERT( pVolume && pVolumeNeighbor, pAI->m_hObject, "CVolumeMgr::FindNeighbor: Volume is NULL");
	for ( uint32 iNeighbor = 0 ; iNeighbor < pVolume->GetNumNeighbors() ; iNeighbor++ )
	{
		if ( pVolume->GetSpatialNeighborByIndex(iNeighbor)->GetSpatialVolume() == pVolumeNeighbor )
		{
			return pVolume->GetSpatialNeighborByIndex(iNeighbor);
		}
	}

	return LTNULL;
}

//----------------------------------------------------------------------------

LTBOOL CAISpatialRepresentationMgr::FindDangerScatterPosition(AISpatialRepresentation* pVolume,
															  const LTVector& vAIPos,
															  const LTVector& vDangerPos,
															  LTFLOAT fDangerDistanceSqr,
															  LTVector* pvScatterPosition,
															  LTBOOL bNeighbor /* = LTFALSE */)
{
	// Is there a Position in this volume that is sufficiently far from the position in question?

	LTVector avCorners[] = 
	{
		pVolume->GetBackBottomLeft(),
		pVolume->GetBackBottomRight(),
		pVolume->GetFrontBottomLeft(),
		pVolume->GetFrontBottomRight() 
	};

	LTBOOL abCornerValid[] =
	{
		LTFALSE,
		LTFALSE,
		LTFALSE,
		LTFALSE
	};

	LTFLOAT fRandomRadiusModifier = GetRandom(1.0f, 1.0f);

	// Find all valid corners

	{for ( uint iCorner = 0 ; iCorner < 4 ; iCorner++ )
	{
		if ( avCorners[iCorner].DistSqr(vDangerPos) > fDangerDistanceSqr*fRandomRadiusModifier )
		{
			abCornerValid[iCorner] = LTTRUE;
		}
	}}

	// Decide which, if any, of the valid corners, is best for us to use (ie, don't run through the danger radius to get there)

	LTFLOAT fMinimumDistanceSqr = (LTFLOAT)INT_MAX;
	uint32 iCornerBest = -1;

	{for ( uint iCorner = 0 ; iCorner < 4 ; iCorner++ )
	{
		if ( !abCornerValid[iCorner] ) continue;

		LTFLOAT fDistanceSqr = avCorners[iCorner].DistSqr(vAIPos);
		if ( fDistanceSqr < fMinimumDistanceSqr )
		{
			iCornerBest = iCorner;
			fMinimumDistanceSqr = fDistanceSqr;
		}
	}}

	if ( iCornerBest != -1 )
	{
		// Find opposite corner

		_ASSERT(iCornerBest >= 0 && iCornerBest <= 3);

		uint32 iCornerOpposite;

		switch ( iCornerBest )
		{
			case 0:
				iCornerOpposite = 3;
				break;

			case 1:
				iCornerOpposite = 2;
				break;

			case 2:
				iCornerOpposite = 1;
				break;

			case 3:
				iCornerOpposite = 0;
				break;
		}

		// Extend towards opposite corner slightly

		LTVector vOffset = avCorners[iCornerOpposite] - avCorners[iCornerBest];
		vOffset.Normalize();
		vOffset *= 50.0f;

		*pvScatterPosition = avCorners[iCornerBest] + vOffset;

		return LTTRUE;
	}

	// No - so look into all the neighbors (only 1 deep!)

	if ( !bNeighbor )
	{
		for ( uint32 iNeighbor = 0 ; iNeighbor < pVolume->GetNumNeighbors() ; iNeighbor++ )
		{
			AISpatialRepresentation* pVol = pVolume->GetSpatialNeighborByIndex(iNeighbor)->GetSpatialVolume();

			if ( FindDangerScatterPosition(pVol, vAIPos, vDangerPos, fDangerDistanceSqr, pvScatterPosition, LTTRUE) )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

AISpatialRepresentation* CAISpatialRepresentationMgr::FindNearestIntersectingVolume(const LTVector& vOrigin,
																					const LTVector& vDest,
																					LTFLOAT fWidth,
																					LTFLOAT fVerticalThreshhold,																					
																					LTVector* pvIntersection)
{
	AISpatialRepresentation* pVolume = LTNULL;
	LTFLOAT fDistSqr;
	LTFLOAT fMinDistSqr = 9999999.f;
	LTVector vIntersection;
	for ( uint32 iVolume = 0 ; iVolume < m_listpVolumes.size(); ++iVolume )
	{
		if( RayIntersectVolume(m_listpVolumes[iVolume], vOrigin, vDest, fVerticalThreshhold, &vIntersection) )
		{
			if( vIntersection == vOrigin )
			{
				*pvIntersection = vOrigin;
				pVolume = m_listpVolumes[iVolume];
				break;
			}

			fDistSqr = vIntersection.DistSqr( vOrigin );
			if( fDistSqr < fMinDistSqr )
			{
				fMinDistSqr = fDistSqr;
				*pvIntersection = vIntersection;
				pVolume = m_listpVolumes[iVolume];
			}
		}
	}

	if ( pVolume == NULL )
	{
		Warn("CVolumeMgr::FindNearestIntersectingVolume: No valid Intersecting Volume found");
		*pvIntersection = LTVector(0,0,0); 
		return NULL;
	}

	// Bound the intersection point into the volume by some width.

	LTVector vMin = pVolume->GetBackBottomLeft();
	LTVector vMax = pVolume->GetFrontTopRight();

	if( (pvIntersection->x - fWidth) < vMin.x )
	{
		pvIntersection->x = vMin.x + fWidth;
	}
	else if ( (pvIntersection->x + fWidth) > vMax.x )
	{
		pvIntersection->x = vMax.x - fWidth;
	}

	if( (pvIntersection->z - fWidth) < vMin.z )
	{
		pvIntersection->z = vMin.z + fWidth;
	}
	else if ( (pvIntersection->z + fWidth) > vMax.z )
	{
		pvIntersection->z = vMax.z - fWidth;
	}

	return pVolume;
}


AISpatialRepresentation* CAISpatialRepresentationMgr::FindContainingVolume(HOBJECT hObject, 
												  const LTVector& vPos,
												  int iAxisMask,
												  LTFLOAT fVerticalThreshhold,
												  AISpatialRepresentation* pVolumeStart,
												  LTBOOL bBruteForce )
{
	if ( !pVolumeStart )
	{
		return FindContainingVolumeBruteForce(hObject, vPos, iAxisMask, fVerticalThreshhold );
	}
	else
	{
		// Set use flags if hObject was passed in.
		// If no hObject was provided, match all volumes.

		uint32 dwUseBy = AISpatialRepresentation::kUseBy_All;
		if( hObject )
		{
			if( IsAI( hObject ) )
			{
				dwUseBy = AISpatialRepresentation::kUseBy_AI;
			}
			else if( IsPlayer( hObject ) )
			{
				dwUseBy = AISpatialRepresentation::kUseBy_Player;
			}
		}

		// We can use the starting volume as a good hint to where our new volume is.

		if( ( pVolumeStart->GetUseFlags() & dwUseBy ) && 
			( pVolumeStart->InsideMasked(vPos, iAxisMask, fVerticalThreshhold) ) &&
			( pVolumeStart->IsVolumeEnabled() ) )
		{
			return pVolumeStart;
		}
		else
		{
			// Look at all the neighbors

			for ( uint32 iNeighbor = 0 ; iNeighbor < pVolumeStart->GetNumNeighbors() ; iNeighbor++ )
			{
				AISpatialRepresentation* pVolume = pVolumeStart->GetSpatialNeighborByIndex(iNeighbor)->GetSpatialVolume();

				// Skip disabled volumes.

				if( !pVolume->IsVolumeEnabled() )
				{
					continue;
				}

				if ( ( pVolumeStart->GetUseFlags() & dwUseBy ) && 
					pVolume->InsideMasked(vPos, iAxisMask, fVerticalThreshhold ) )
				{
					return pVolume;
				}
			}
		}

		if ( bBruteForce )
		{
			// Give up and brute force it. In the future, we can do a limited breadth first search through
			// the neighbors.

			return FindContainingVolumeBruteForce(hObject, vPos, iAxisMask, fVerticalThreshhold );
		}
		else
		{
			return LTNULL;
		}
	}
}

AISpatialRepresentation* CAISpatialRepresentationMgr::GetVolume(const char* szVolume)
{
	if ( !szVolume )
	{
		return LTNULL;
	}

	for ( uint32 iVolume = 0 ; iVolume < m_listpVolumes.size(); iVolume++ )
	{
		if ( !_stricmp(szVolume, m_listpVolumes[iVolume]->GetName()) )
		{
			return m_listpVolumes[iVolume];
		}
	}

	return LTNULL;
}

AISpatialRepresentation* CAISpatialRepresentationMgr::GetVolume(uint32 iVolume)
{ 
	if ( !IsInitialized() || (iVolume >= GetNumVolumes()) )
	{
		return LTNULL;
	}

	return m_listpVolumes[iVolume]; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::UpdateDebugRendering()
//              
//	PURPOSE:	Draw or hide volumes.
//              
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::UpdateDebugRendering(LTFLOAT fVarTrack)
{
	if( !m_bDrawingVolumes && fVarTrack )
	{
		DrawVolumes();
	}
	else if( m_bDrawingVolumes && !fVarTrack )
	{
		HideVolumes();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::DrawVolumes()
//              
//	PURPOSE:	Sets all Volumes to draw and remembers that the volumes are 
//				being drawn.
//              
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::DrawVolumes()
{
	std::for_each(
		GetContainer()->begin(),
		GetContainer()->end(),
		std::mem_fun( &AISpatialRepresentation::DrawSelf ));

	m_bDrawingVolumes = LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISpatialRepresentationMgr::HideVolumes()
//              
//	PURPOSE:	Sets all Volumes to Hide
//              
//----------------------------------------------------------------------------
void CAISpatialRepresentationMgr::HideVolumes()
{
	std::for_each(
		GetContainer()->begin(),
		GetContainer()->end(),
		std::mem_fun( &AISpatialRepresentation::HideSelf ));

	m_bDrawingVolumes = LTFALSE;
}
