// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVolumeMgr.h"
#include "AIPath.h"
#include "FastHeap.h"
#include "WorldProperties.h"

// Globals

CAIVolumeMgr* g_pAIVolumeMgr = LTNULL;

// Methods

CAIVolumeMgr::CAIVolumeMgr()
{
	g_pAIVolumeMgr = this;
	m_cVolumes = 0;
	m_bInitialized = LTFALSE;
	m_aVolumes = LTNULL;
}

CAIVolumeMgr::~CAIVolumeMgr()
{
	g_pAIVolumeMgr = LTNULL;
	Term();
}

void CAIVolumeMgr::Term()
{
	m_cVolumes = 0;
	m_bInitialized = LTFALSE;
	if ( m_aVolumes )
	{
		debug_deletea(m_aVolumes);
		m_aVolumes = LTNULL;
	}
}

void CAIVolumeMgr::Init()
{
	Term();

	// First, we count up the number of volumes in the level

    HCLASS  hAIVolume = g_pLTServer->GetClass("AIVolume");
	HOBJECT	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIVolume))
		{
			m_cVolumes++;
		}
	}

	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIVolume))
		{
			m_cVolumes++;
		}
	}

	if ( 0 == m_cVolumes ) return;

	m_aVolumes = debug_newa(CAIVolume, m_cVolumes);

	int32 iVolume = 0;

	// Now we put the Volumes int32o our array

	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIVolume))
		{
			// Setup the volume

            m_aVolumes[iVolume].Init(iVolume, *(AIVolume*)g_pLTServer->HandleToObject(hCurObject));
			iVolume++;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

	hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hAIVolume))
		{
			// Setup the volume

            m_aVolumes[iVolume].Init(iVolume, *(AIVolume*)g_pLTServer->HandleToObject(hCurObject));
			iVolume++;

			// Remove the object

            g_pLTServer->RemoveObject(hCurObject);
		}
	}

	// Build the neighboring connections

	int32 cTotalNeighbors = 0;

	{for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		const static int32 c_nMaxNeighbors = 16;
		CAIVolume* apVolumeNeighbors[c_nMaxNeighbors];
		int32 cNeighbors = 0;

		{for ( int32 iNeighborVolume = 0 ; iNeighborVolume < m_cVolumes ; iNeighborVolume++ )
		{
			if ( iVolume != iNeighborVolume )
			{
				if ( m_aVolumes[iVolume].Intersects(m_aVolumes[iNeighborVolume]) )
				{
/*					char szDebug[128];
					sprint32f(szDebug, "%s neighbors %s", m_aVolumes[iVolume].GetName(), m_aVolumes[iNeighborVolume].GetName());
					OutputDebugString(szDebug);
*/
					if ( cNeighbors >= c_nMaxNeighbors )
					{
						_ASSERT(!"Max number of neighboring volumes exceeded!!!!");
                        g_pLTServer->CPrint("Max number of neighboring volumes exceeded!!!!");
						break;
					}

					apVolumeNeighbors[cNeighbors++] = &m_aVolumes[iNeighborVolume];
					cTotalNeighbors++;
				}
			}
		}}

		m_aVolumes[iVolume].InitNeighbors(apVolumeNeighbors, cNeighbors);
/*
        g_pLTServer->CPrint("restated: %s has %d neighbors", m_aVolumes[iVolume].GetName(), m_aVolumes[iVolume].GetNumNeighbors());
		{for ( int32 iNeighborVolume = 0 ; iNeighborVolume < m_aVolumes[iVolume].GetNumNeighbors() ; iNeighborVolume++ )
		{
			int32 iNeighbor = m_aVolumes[iVolume].GetNeighborByIndex(iNeighborVolume)->GetIndex();
            g_pLTServer->CPrint("           %s", m_aVolumes[iNeighbor].GetName());
		}}*/
	}}

	// All done
#ifndef _FINAL
    g_pLTServer->CPrint("Added %d volumes, %d connections", m_cVolumes, cTotalNeighbors);
#endif
	m_bInitialized = LTTRUE;
}

CAIVolume* CAIVolumeMgr::GetVolumeByName(const char* szVolume)
{
	if ( !szVolume ) return LTNULL;

	for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		if ( !_stricmp(szVolume, m_aVolumes[iVolume].GetName()) )
		{
			return &m_aVolumes[iVolume];
		}
	}

	return LTNULL;
}
		
void CAIVolumeMgr::Link(HOBJECT hObject)
{
	// Link via the worldprops

	if ( g_pWorldProperties && hObject )
	{
        g_pLTServer->CreateInterObjectLink(g_pWorldProperties->m_hObject, hObject);
	}
}

void CAIVolumeMgr::Unlink(HOBJECT hObject)
{
	// Unink via the worldprops

	if ( g_pWorldProperties && hObject )
	{
        g_pLTServer->BreakInterObjectLink(g_pWorldProperties->m_hObject, hObject);
	}
}

void CAIVolumeMgr::HandleBrokenLink(HOBJECT hObject)
{
	for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		m_aVolumes[iVolume].HandleBrokenLink(hObject);
	}
}

void CAIVolumeMgr::Load(HMESSAGEREAD hRead)
{
	LOAD_BOOL(m_bInitialized);
	LOAD_INT(m_cVolumes);

	if ( 0 == m_cVolumes ) 
	{
		if ( m_aVolumes )
		{
			debug_deletea(m_aVolumes); 
			m_aVolumes = LTNULL;
		}

		return;
	}

	m_aVolumes = debug_newa(CAIVolume, m_cVolumes);
	for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		m_aVolumes[iVolume].Load(hRead);
	}
}

void CAIVolumeMgr::Save(HMESSAGEWRITE hWrite)
{
	SAVE_BOOL(m_bInitialized);
	SAVE_INT(m_cVolumes);
	for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		m_aVolumes[iVolume].Save(hWrite);
	}
}

CAIVolume* CAIVolumeMgr::FindContainingVolume(const LTVector& vPos, LTFLOAT fVerticalThreshhold /*= 0.0f*/, CAIVolume* pVolumeStart /* = LTNULL */, LTBOOL bBruteForce /* = LTTRUE */)
{
	if ( !pVolumeStart )
	{
		return FindContainingVolumeBruteForce(vPos, fVerticalThreshhold);
	}
	else
	{
		// We can use the starting volume as a good hint to where our new volume is.

		if ( pVolumeStart->Inside(vPos, fVerticalThreshhold) )
		{
			return pVolumeStart;
		}
		else
		{
			// Look at all the neighbors

			for ( int32 iNeighbor = 0 ; iNeighbor < pVolumeStart->GetNumNeighbors() ; iNeighbor++ )
			{
				int32 iVolume = pVolumeStart->GetNeighborByIndex(iNeighbor)->GetIndex();

				if ( m_aVolumes[iVolume].Inside(vPos, fVerticalThreshhold) )
				{
					return &m_aVolumes[iVolume];
				}
			}
		}

		if ( bBruteForce )
		{
			// Give up and brute force it. In the future, we can do a limited breadth first search through
			// the neighbors.

			return FindContainingVolumeBruteForce(vPos, fVerticalThreshhold);
		}
		else
		{
			return LTNULL;
		}
	}
}

CAIVolume* CAIVolumeMgr::FindContainingVolumeBruteForce(const LTVector& vPos, LTFLOAT fVerticalThreshhold)
{
	// The really, really, stupid way.

	for ( int32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		if ( m_aVolumes[iVolume].Inside(vPos, fVerticalThreshhold) )
		{
			return &m_aVolumes[iVolume];
		}
	}

	return LTNULL;
}

CAIVolumeNeighbor* CAIVolumeMgr::FindNeighbor(CAIVolume* pVolume, CAIVolume* pVolumeNeighbor)
{
	for ( int32 iNeighbor = 0 ; iNeighbor < pVolume->GetNumNeighbors() ; iNeighbor++ )
	{
		if ( &m_aVolumes[pVolume->GetNeighborByIndex(iNeighbor)->GetIndex()] == pVolumeNeighbor )
		{
			return pVolume->GetNeighborByIndex(iNeighbor);
		}
	}

	return LTNULL;
}

LTBOOL CAIVolumeMgr::FindDangerScatterPosition(CAIVolume* pVolume, const LTVector& vAIPos, const LTVector& vDangerPos, LTFLOAT fDangerDistanceSqr, LTVector* pvScatterPosition, LTBOOL bNeighbor /* = LTFALSE */)
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
		vOffset.Norm();
		vOffset *= 50.0f;

		*pvScatterPosition = avCorners[iCornerBest] + vOffset;

		return LTTRUE;
	}

	// No - so look into all the neighbors (only 1 deep!)

	if ( !bNeighbor )
	{
		for ( int32 iNeighbor = 0 ; iNeighbor < pVolume->GetNumNeighbors() ; iNeighbor++ )
		{
			int32 iVolume = pVolume->GetNeighborByIndex(iNeighbor)->GetIndex();

			if ( FindDangerScatterPosition(&m_aVolumes[iVolume], vAIPos, vDangerPos, fDangerDistanceSqr, pvScatterPosition, LTTRUE) )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

CAIVolume* CAIVolumeMgr::GetVolumeByIndex(int32 iVolume)
{ 
	if ( !IsInitialized() || (iVolume >= GetNumVolumes()) || (iVolume < 0) ) return LTNULL;

	return &m_aVolumes[iVolume]; 
}
