// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIPath.h"
#include "AI.h"
#include "AIVolumeMgr.h"
#include "AIPathMgr.h"

DEFINE_AI_FACTORY_CLASS(CAIPathWaypoint);
DEFINE_AI_FACTORY_CLASS(CAIPath);


// CAIPathWaypoint

CAIPathWaypoint::CAIPathWaypoint()
{
    m_pAI = LTNULL;
	m_eInstruction = eInstructionInvalid,
    m_vVector1 = LTVector(0,0,0);
    m_hObject1 = LTNULL;
	m_hObject1.SetReceiver(*this);
	m_pObject1 = LTNULL;
    m_hObject2 = LTNULL;
	m_hObject2.SetReceiver(*this);
	m_pObject2 = LTNULL;
	m_eVolumeGate = eVolumeGateInvalid;
	m_nWaypointID = 0;
	m_iControlPoint = -1;
	m_bCalculateCurve = LTFALSE;
	m_eProp = kAP_None;
}

CAIPathWaypoint::~CAIPathWaypoint()
{
	if ( m_pAI )
	{
		m_hObject1 = LTNULL;
		m_pObject1 = LTNULL;
		m_hObject2 = LTNULL;
		m_pObject2 = LTNULL;
	}
}

void CAIPathWaypoint::SetWaypoint(CAI* pAI, Instruction eInstruction, const LTVector& vVector1)
{
	m_pAI = pAI;
	m_eInstruction = eInstruction;
	m_vVector1 = vVector1;
}

void CAIPathWaypoint::SetControlPoint(CAI* pAI, Instruction eInstruction, const LTVector& vVector1, uint32 iControlPoint)
{
	m_pAI = pAI;
	m_eInstruction = eInstruction;
	m_vVector1 = vVector1;
	m_iControlPoint = iControlPoint;
	m_bCalculateCurve = LTTRUE;
}

void CAIPathWaypoint::SetReleaseGatePoint(CAI* pAI, VolumeGate eVolumeGate, ILTBaseClass* pObject1, ILTBaseClass* pObject2)
{
	m_eInstruction = eInstructionReleaseGate;

	m_pAI = pAI;
	m_eVolumeGate = eVolumeGate;
	m_pObject1 = pObject1;
	m_hObject1 = (pObject1) ? pObject1->m_hObject : LTNULL;
	m_pObject2 = pObject2;
	m_hObject2 = (pObject2) ? pObject2->m_hObject : LTNULL;
}

void CAIPathWaypoint::SetArgumentObject1(ILTBaseClass* pObject1)
{
	if ( m_pAI )
	{
		m_pObject1 = pObject1;
		m_hObject1 = (pObject1) ? pObject1->m_hObject : LTNULL;
	}
}

void CAIPathWaypoint::SetArgumentObject2(ILTBaseClass* pObject2)
{
	if ( m_pAI )
	{
		m_pObject2 = pObject2;
		m_hObject2 = (pObject2) ? pObject2->m_hObject : LTNULL;
	}
}

void CAIPathWaypoint::ReleaseGate()
{
	AIVolume* pVolume = (AIVolume*)m_pObject1;
	AIVolume* pVolumeNext = (AIVolume*)m_pObject2;

	if ( pVolume && pVolumeNext )
	{
		AIVolumeNeighbor* pVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor( m_pAI, pVolume, pVolumeNext );
		if ( pVolumeNeighbor )
		{
			pVolumeNeighbor->ReleaseGate( m_eVolumeGate );
		}
	}
}

void CAIPathWaypoint::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eInstruction, Instruction);
	LOAD_VECTOR(m_vVector1);
	LOAD_COBJECT(m_pObject1, ILTBaseClass);
	m_hObject1 = (m_pObject1) ? m_pObject1->m_hObject : LTNULL;
	LOAD_COBJECT(m_pObject2, ILTBaseClass);
	m_hObject2 = (m_pObject2) ? m_pObject2->m_hObject : LTNULL;
	LOAD_DWORD_CAST(m_eVolumeGate, VolumeGate);
	LOAD_DWORD(m_nWaypointID);
	LOAD_DWORD(m_iControlPoint);
	LOAD_BOOL(m_bCalculateCurve);
	LOAD_DWORD_CAST(m_eProp, EnumAnimProp);
}

void CAIPathWaypoint::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eInstruction);
	SAVE_VECTOR(m_vVector1);
	SAVE_COBJECT(m_pObject1);
	SAVE_COBJECT(m_pObject2);
	SAVE_DWORD(m_eVolumeGate);
	SAVE_DWORD(m_nWaypointID);
	SAVE_DWORD(m_iControlPoint);
	SAVE_BOOL(m_bCalculateCurve);
	SAVE_DWORD(m_eProp);
}

void CAIPathWaypoint::OnLinkBroken(LTObjRefNotifier* pRef, HOBJECT hObj)
{
	if (pRef == &m_hObject1)
		m_pObject1 = LTNULL;
	if (pRef == &m_hObject2)
		m_pObject2 = LTNULL;
}

// CAIPath

CAIPath::CAIPath()
{
	m_pAI = LTNULL;
	m_iWaypoint = 0;

	ClearWaypoints();
}

CAIPath::~CAIPath()
{
	ClearWaypoints();
}

void CAIPath::Init(CAI* pAI)
{
	m_pAI = pAI;
}

void CAIPath::Load(ILTMessage_Read *pMsg)
{
	int cWaypoints;
	LOAD_INT(cWaypoints);
	m_lstWaypoints.resize( cWaypoints );

	LOAD_INT(m_iWaypoint);

	for( int i = 0; i < cWaypoints; ++i )
	{
		CAIPathWaypoint* pWaypt = AI_FACTORY_NEW( CAIPathWaypoint );
		pWaypt->Load( pMsg );

		m_lstWaypoints[i] = pWaypt;
	}
}

void CAIPath::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_lstWaypoints.size());
	SAVE_INT(m_iWaypoint);

	AI_WAYPOINT_LIST::iterator it;
	for( it = m_lstWaypoints.begin(); it != m_lstWaypoints.end(); ++it )
	{
		(*it)->Save(pMsg);
	}
}

void CAIPath::ClearWaypoints()
{
	// Release any gates path was holding.

	AI_WAYPOINT_LIST::iterator it;
	for( it = m_lstWaypoints.begin(); it != m_lstWaypoints.end(); ++it )
	{
		if( (*it)->m_eInstruction == CAIPathWaypoint::eInstructionReleaseGate )
		{
			(*it)->ReleaseGate();
		}

		AI_FACTORY_DELETE( *it );
	}

	m_iWaypoint = 0;
	m_lstWaypoints.clear();
}

void CAIPath::AddWaypoint(CAIPathWaypoint* pWaypt)
{
	pWaypt->SetWaypointID( g_pAIPathMgr->GetNextWaypointID() );
	m_lstWaypoints.push_back( pWaypt );
}

AI_WAYPOINT_LIST::iterator CAIPath::RemoveWaypoint(AI_WAYPOINT_LIST::iterator it)
{
	AI_FACTORY_DELETE( *it );
	return m_lstWaypoints.erase( it );
}

AI_WAYPOINT_LIST::iterator CAIPath::InsertWaypoint(AI_WAYPOINT_LIST::iterator it, CAIPathWaypoint* pWaypt)
{
	pWaypt->SetWaypointID( g_pAIPathMgr->GetNextWaypointID() );
	return m_lstWaypoints.insert( it, pWaypt );
}

void CAIPath::GetInitialDir(LTVector* pvDir)
{
	if( !pvDir )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIPath::GetInitialDir: Vector is NULL." );
		return;
	}

	LTVector vPosFirst;
	LTBOOL bFirst = LTTRUE;

	CAIPathWaypoint* pWaypt;
	AI_WAYPOINT_LIST::iterator it;
	for( it = m_lstWaypoints.begin(); it != m_lstWaypoints.end(); ++it )
	{
		pWaypt = *it;
		if( pWaypt->GetControlPointIndex() != -1 )
		{
			if( bFirst )
			{
				vPosFirst = pWaypt->GetArgumentVector1();
				bFirst = LTFALSE;
			}
			else {
				*pvDir = pWaypt->GetArgumentVector1() - vPosFirst;
				pvDir->Normalize();
				break;
			}
		}
	}
}

void CAIPath::GetFinalDir(LTVector* pvDir)
{
	if( !pvDir )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIPath::GetFinalDir: Vector is NULL." );
		return;
	}

	LTBOOL bFirst = LTTRUE;
	LTVector v0, v1;

	CAIPathWaypoint* pWaypt;
	AI_WAYPOINT_LIST::reverse_iterator rit;
	for( rit = m_lstWaypoints.rbegin(); rit != m_lstWaypoints.rend(); ++rit )
	{
		pWaypt = *rit;

		switch( pWaypt->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			case CAIPathWaypoint::eInstructionCrawlTo:
			case CAIPathWaypoint::eInstructionClimbUpTo:
			case CAIPathWaypoint::eInstructionClimbDownTo:
			case CAIPathWaypoint::eInstructionJumpUpTo:
			case CAIPathWaypoint::eInstructionJumpDownTo:
			case CAIPathWaypoint::eInstructionJumpOver:
				if( bFirst )
				{
					v1 = pWaypt->GetArgumentVector1();
					bFirst = LTFALSE;
				}
				else {
					v0 = pWaypt->GetArgumentVector1();
					rit = m_lstWaypoints.rend() - 1;
				}
				break;
		}
	}

	*pvDir = v1 - v0;
}

AIVolume* CAIPath::GetNextVolume(AIVolume* pVolume, AIVolume::EnumVolumeType eVolumeType)
{
	if( !pVolume )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIPath::GetNextVolume: Volume is NULL." );
		return LTNULL;
	}

	LTBOOL bFoundSource = LTFALSE;
	AIVolume* pNext = LTNULL;
	CAIPathWaypoint* pWaypt;
	AI_WAYPOINT_LIST::iterator it;
	for( it = m_lstWaypoints.begin(); it != m_lstWaypoints.end(); ++it )
	{
		pWaypt = *it;

		if( pWaypt->GetInstruction() != CAIPathWaypoint::eInstructionReleaseGate )
		{
			continue;
		}

		if( pWaypt->GetArgumentObject1() == pVolume )
		{
			bFoundSource = LTTRUE;
		}

		if( bFoundSource )
		{
			pNext = (AIVolume*)pWaypt->GetArgumentObject2();
			if( pNext &&
				( ( eVolumeType == AIVolume::kVolumeType_None ) ||
				  ( eVolumeType == pNext->GetVolumeType() ) ) )
			{
				break;
			}
		}
	}

	return pNext;
}

AIVolume* CAIPath::GetLastVolume(uint32 nOffsetFromLast)
{
	AIVolume* pVolume = LTNULL;
	uint32 cVolumesFromLast = 0;

	// Iterate backwards thru the path waypoints.

	CAIPathWaypoint* pWaypt;
	AI_WAYPOINT_LIST::reverse_iterator rit;
	for( rit = m_lstWaypoints.rbegin(); rit != m_lstWaypoints.rend(); ++rit )
	{
		pWaypt = *rit;

		// ReleaseGate waypoints mark boundaries between volumes.

		if( pWaypt->GetInstruction() != CAIPathWaypoint::eInstructionReleaseGate )
		{
			continue;
		}

		// Count how many volumes we are from the last.

		++cVolumesFromLast;
		if( cVolumesFromLast == nOffsetFromLast )
		{
			// Record the volume.

			pVolume = (AIVolume*)pWaypt->GetArgumentObject1();
			break;
		}
	}

	return pVolume;
}

void CAIPath::Print()
{
	char szTemp[100];
	OutputDebugString("\nPATH:\n");
	AI_WAYPOINT_LIST::iterator it;
	for( it = m_lstWaypoints.begin(); it != m_lstWaypoints.end(); ++it )
	{
		sprintf( szTemp, "%d ", (*it)->GetWaypointID() );
		OutputDebugString( szTemp );

		switch( (*it)->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionInvalid:
				OutputDebugString("eInstructionInvalid");
				break;
			case CAIPathWaypoint::eInstructionMoveTo:
				OutputDebugString("eInstructionMoveTo");
				break;
			case CAIPathWaypoint::eInstructionCrawlTo:
				OutputDebugString("eInstructionCrawlTo");
				break;
			case CAIPathWaypoint::eInstructionClimbUpTo:
				OutputDebugString("eInstructionClimbUpTo");
				break;
			case CAIPathWaypoint::eInstructionClimbDownTo:
				OutputDebugString("eInstructionClimbDownTo");
				break;
			case CAIPathWaypoint::eInstructionFaceLadder:
				OutputDebugString("eInstructionFaceLadder");
				break;
			case CAIPathWaypoint::eInstructionJumpUpTo:
				OutputDebugString("eInstructionJumpUpTo");
				break;
			case CAIPathWaypoint::eInstructionJumpDownTo:
				OutputDebugString("eInstructionJumpDownTo");
				break;
			case CAIPathWaypoint::eInstructionJumpOver:
				OutputDebugString("eInstructionJumpOver");
				break;
			case CAIPathWaypoint::eInstructionFaceJumpLand:
				OutputDebugString("eInstructionFaceJumpLand");
				break;
			case CAIPathWaypoint::eInstructionWaitForLift:
				OutputDebugString("eInstructionWaitForLift");
				break;
			case CAIPathWaypoint::eInstructionEnterLift:
				OutputDebugString("eInstructionEnterLift");
				break;
			case CAIPathWaypoint::eInstructionRideLift:
				OutputDebugString("eInstructionRideLift");
				break;
			case CAIPathWaypoint::eInstructionExitLift:
				OutputDebugString("eInstructionExitLift");
				break;
			case CAIPathWaypoint::eInstructionOpenDoors:
				OutputDebugString("eInstructionOpenDoors");
				break;
			case CAIPathWaypoint::eInstructionWaitForDoors:
				OutputDebugString("eInstructionWaitForDoors");
				break;
			case CAIPathWaypoint::eInstructionFaceDoor:
				OutputDebugString("eInstructionFaceDoor");
				break;
			case CAIPathWaypoint::eInstructionReleaseGate:
				OutputDebugString("eInstructionReleaseGate");
				break;
			default:
				OutputDebugString("UnrecognizedInstruction");
				break;
		}

		LTVector v = (*it)->GetArgumentVector1();
		sprintf( szTemp, " %.2f %.2f %.2f\n", v.x, v.y, v.z );
		OutputDebugString( szTemp );
	}
}

