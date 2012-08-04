// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPassTarget.cpp
//
// PURPOSE : AISensorPassTarget class implementation
//
// CREATED : 11/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorPassTarget.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "AINavMesh.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPassTarget, kSensor_PassTarget );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorPassTarget::CAISensorPassTarget()
{
	m_bHoldingPosition = false;
	m_hVerifiedNode = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorPassTarget::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bHoldingPosition );
	SAVE_HOBJECT( m_hVerifiedNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorPassTarget::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_bool( m_bHoldingPosition );
	LOAD_HOBJECT( m_hVerifiedNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorPassTarget::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// We are NOT yet holding position.

	bool bCalledFindPath = false;
	if( !m_bHoldingPosition )
	{
		// Start holding position if necessary.

		if( NeedToHoldPosition( &bCalledFindPath ) )
		{
			StartHoldingPosition();
			return true;
		}

		return bCalledFindPath;
	}

	// We are currently holding position.
	// Stop holding position when possible.

	if( !ContinueHoldingPosition( &bCalledFindPath ) )
	{
		StopHoldingPosition();
		return bCalledFindPath;
	}

	return bCalledFindPath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::NeedToHoldPosition
//
//	PURPOSE:	Return true if AI needs to hold his position.
//
// ----------------------------------------------------------------------- //

bool CAISensorPassTarget::NeedToHoldPosition( bool* pbCalledFindPath ) 
{
	// Sanity check.

	if( !pbCalledFindPath )
	{
		return false;	
	}

	// No need to hold position if not targeting a character.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		m_hVerifiedNode = NULL;
		return false;
	}

	// Only check for passing the target if we are within some distance of the target.

	LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
	float fDistSqr = vTargetPos.DistSqr( m_pAI->GetPosition() );
	if( fDistSqr > g_pAIDB->GetAIConstantsRecord()->fHoldPositionDistanceSqr )
	{
		m_hVerifiedNode = NULL;
		return false;	
	}

	// Character is outside of the NavMesh.

	ENUM_NMPolyID ePolyTarget = GetTargetNavMeshPoly();
	if( ePolyTarget == kNMPoly_Invalid )
	{
		m_hVerifiedNode = NULL;
		return false;
	}

	// Bail if AI is not going for cover or ambush.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Cover );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		factQuery.SetTaskType( kTask_Ambush );
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	}
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		m_hVerifiedNode = NULL;
		return false;
	}

	// Bail if AI is scripted to go for cover.

	if( pFact->GetFactFlags() & kFactFlag_Scripted )
	{
		m_hVerifiedNode = NULL;
		return false;
	}

	// We have already determined that we can get to this node without
	// crossing the target's position.

	HOBJECT hNode = pFact->GetTargetObject();
	if( hNode == m_hVerifiedNode )
	{
		// AI is not going anywhere.

		CAIPathNavMesh* pNMPath = m_pAI->GetAINavigationMgr()->GetNMPath();
		if( ( !pNMPath ) ||
			( !m_pAI->GetAINavigationMgr()->IsNavSet() ) ||
			( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set ) )
		{
			return false;
		}

		// Existing path is still valid.  Path does not cross target.

		if( !PathIncludesPoly( *pNMPath, ePolyTarget ) )
		{
			return false;
		}
	}

	// No path exists to node.

	*pbCalledFindPath = true;
	static CAIPathNavMesh NMPath;
	if( !FindPathToNode( hNode, &NMPath ) )
	{
		return false;
	}

	// Path does cross target's position.

	if( PathIncludesPoly( NMPath, ePolyTarget ) )
	{
		AvoidNode( hNode );
		return true;
	}

	// Path does not cross target's position.

	m_hVerifiedNode = hNode;
	return false; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::ContinueHoldingPosition
//
//	PURPOSE:	Return true if AI needs to continue holding his position.
//
// ----------------------------------------------------------------------- //

bool CAISensorPassTarget::ContinueHoldingPosition( bool* pbCalledFindPath ) 
{ 
	// Continue holding if AI does not have a new cover or ambush dest.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Cover );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		factQuery.SetTaskType( kTask_Ambush );
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	}
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		return true;
	}

	// Stop holding position if AI is scripted to go for cover.

	if( pFact->GetFactFlags() & kFactFlag_Scripted )
	{
		return false;
	}

	// Continue holding if cover dest is not valid.

	HOBJECT hNode = pFact->GetTargetObject();
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
	if( !( pNode && pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), NULL, kThreatPos_TargetPos, kNodeStatus_Avoid ) ) )
	{
		return true;
	}

	// Continue holding if no path exists to the node.

	*pbCalledFindPath = true;
	static CAIPathNavMesh NMPath;
	if( !FindPathToNode( hNode, &NMPath ) )
	{
		return true;
	}

	// Continue holding if path crosses target's position.

	ENUM_NMPolyID ePolyTarget = GetTargetNavMeshPoly();
	if( PathIncludesPoly( NMPath, ePolyTarget ) )
	{
		AvoidNode( hNode );
		return true;
	}

	// Stop holding position!
	// We are ready to move somewhere new for cover or ambush.

	return false; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::StartHoldingPosition
//
//	PURPOSE:	Start holding a position.
//
// ----------------------------------------------------------------------- //

void CAISensorPassTarget::StartHoldingPosition() 
{
	m_bHoldingPosition = true;
	m_hVerifiedNode = NULL;

	// Create Desire to HoldPosition.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_HoldPosition );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		pFact->SetDesireType( kDesire_HoldPosition, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::StartHoldingPosition
//
//	PURPOSE:	Stop holding a position.
//
// ----------------------------------------------------------------------- //

void CAISensorPassTarget::StopHoldingPosition() 
{
	m_bHoldingPosition = false;
	m_hVerifiedNode = NULL;

	// Clear Desire to HoldPosition.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_HoldPosition );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::GetTargetNavMeshPoly
//
//	PURPOSE:	Return the target's current NavMesh poly.
//
// ----------------------------------------------------------------------- //

ENUM_NMPolyID CAISensorPassTarget::GetTargetNavMeshPoly() 
{
	// Target is not a character.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !IsCharacter( hTarget ) )
	{
		return kNMPoly_Invalid;
	}

	// Return character's NavMesh poly.

	CCharacter* pTargetChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
	return pTargetChar->GetCurrentNavMeshPoly();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::FindPathToNode
//
//	PURPOSE:	Return true if a path exists to the node.
//
// ----------------------------------------------------------------------- //

bool CAISensorPassTarget::FindPathToNode( HOBJECT hNode, CAIPathNavMesh* pNMPATH ) 
{
	// Sanity check.

	if( !IsAINode( hNode ) )
	{
		return false;
	}

	LTVector vNodePos;
	g_pLTServer->GetObjectPos( hNode, &vNodePos );

	ENUM_NMPolyID ePolySource = m_pAI->GetLastNavMeshPoly();
	ENUM_NMPolyID ePolyDest = g_pAIQuadTree->GetContainingNMPoly( vNodePos, m_pAI->GetCharTypeMask(), ePolySource );

	// Return the result of FindPath.

	return g_pAIPathMgrNavMesh->FindPath( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vNodePos, ePolySource, ePolyDest, DO_NOT_PULL_STRINGS, kPath_Safe, pNMPATH );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::PathIncludesPoly
//
//	PURPOSE:	Return true if path includes the specified poly.
//
// ----------------------------------------------------------------------- //

bool CAISensorPassTarget::PathIncludesPoly( CAIPathNavMesh& NMPath, ENUM_NMPolyID ePoly ) 
{
	// Sanity check.

	if( ePoly == kNMPoly_Invalid )
	{
		return false;
	}

	CAINavMeshPoly* pNMPoly = g_pAINavMesh->GetNMPoly( ePoly );
	if( !pNMPoly )
	{
		return false;
	}

	// Path has not been set.

	uint32 cPathNodes = NMPath.GetPathLength();
	if( cPathNodes == 0 )
	{
		return false;
	}

	// Return true if path includes the specified poly.
	// We don't care if the path starts or ends at the poly,
	// just if the poly falls in the middle of the path.

	SPATH_NODE* pPathNode = NMPath.GetPathNode( 0 );
	ENUM_NMPolyID ePolyFirst = pPathNode ? pPathNode->ePoly : kNMPoly_Invalid;
	for( uint32 iPathNode=2; iPathNode < cPathNodes - 1; ++iPathNode )	
	{
		pPathNode = NMPath.GetPathNode( iPathNode );

		// Something is wrong with this path!

		if( !pPathNode )
		{
			return false;
		}

		// Path includes poly.

		if( pPathNode->ePoly == ePoly )
		{
			return true;
		}

		// Path passes through immediate neighbor of poly.
		// Don't bother with this check if the path starts in
		// the target poly.

		if( ( ePoly != ePolyFirst ) && 
			( pNMPoly->GetNMPolyNeighborEdge( pPathNode->ePoly ) ) )
		{
			return true;
		}
	}

	// Path does not include poly.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPassTarget::AvoidNode
//
//	PURPOSE:	Record working memory about node to avoid.
//
// ----------------------------------------------------------------------- //

void CAISensorPassTarget::AvoidNode( HOBJECT hNode ) 
{
	// Sanity check.

	if( !IsAINode( hNode ) )
	{
		return;
	}

	// Record node to avoid in AI's own working memory.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_AvoidNode );
	factQuery.SetTargetObject( hNode );

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
	}

	// Set the expiration time to stop avoiding this node.

	if( pFact )
	{
		float fDelay = g_pAIDB->GetAIConstantsRecord()->fHoldPositionTime;

		pFact->SetKnowledgeType( kKnowledge_AvoidNode, 1.f );
		pFact->SetTargetObject( hNode, 1.f );
		pFact->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );
	}
}

