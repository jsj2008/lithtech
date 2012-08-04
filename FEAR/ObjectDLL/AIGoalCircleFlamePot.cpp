// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCircleFlamePot.cpp
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCircleFlamePot.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMesh.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCircleFlamePot, kGoal_CircleFlamePot );

// If an AI deactivates the CircleFlamePot goal within this distance of an AI
// who is not blitzing, the AI may attempt to blitz the player.
static float g_flTooCloseToEnemySqr = 100.f*100.f;

// If the AI starts circling with a FlamePot destination this far away or 
// less, the AI will select a new position.  This is used to allow AIs to 
// optionally persist locations between goal runs.  This helps reduce AI 
// collision.
static float g_flCirclingPositionAchievedDistSqr = 100.f*100.f;


// Returns a pointer to an AIs circleable FlamePot link, if a valid link 
// exists.  Returns NULL if there is no valid link.
static AINavMeshLinkAbstract* GetCircleableFlamePotLink( CAI* pAI )
{
	// Fail if the target is not a character.
	
	if ( !pAI->HasTarget( kTarget_Character ) )
	{
		return NULL;
	}

	CCharacter* pChar = CCharacter::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if ( NULL == pChar )
	{
		return NULL;
	}

	// Fail if the target is not in a flame pot poly.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pChar->GetCurrentNavMeshLink() );
	if ( !pLink 
		|| kLink_FlamePot != pLink->GetNMLinkType() 
		|| NULL == pLink->GetHOBJECT() )
	{
		return NULL;
	}

	// Fail if the link is not active.

	if ( !pLink->IsNMLinkActiveToAI( pAI ) )
	{
		return NULL;
	}

	return pLink;
}

// Returns true if the AI has a valid flame pot position for this link, false
// if it does not.
//
// Helper function for setting a flame pot position.  If the AI already has 
// a valid position, it will use it.  If the AI does not have a valid 
// position, a new position will be selected.
static bool SelectFlamePotPosition( CAI* pAI, AINavMeshLinkAbstract* pLink )
{
	// Sanity checks.

	if ( !pAI || !pLink )
	{
		return false;
	}

	//
	// Check to see if the AI already has a position.  If he does, use it.
	//

	CAIWMFact queryExistingFact;
	queryExistingFact.SetFactType( kFact_Knowledge );
	queryExistingFact.SetKnowledgeType( kKnowledge_FlamePotPosition );
	queryExistingFact.SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	queryExistingFact.SetTargetObject( pLink->GetHOBJECT() );
	CAIWMFact* pCurrentPosition = pAI->GetAIWorkingMemory()->FindWMFact( queryExistingFact );
	if ( NULL != pCurrentPosition )
	{
		// Ignore the point if the AI is already very close to it; this is 
		// likely an old position that was 'achieved'.

		LTVector vPosDelta2D = ( pAI->GetPosition() - pCurrentPosition->GetPos() );
		vPosDelta2D.y = 0.0f;
		float flDistanceSqr2D = vPosDelta2D.MagSqr();
		if ( flDistanceSqr2D >= g_flCirclingPositionAchievedDistSqr )
		{
			return true;
		}
	}

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pLink->GetNMPolyID() );
	if ( NULL == pPoly )
	{
		return false;
	}

	// Get the total length of the edge, so we can pick a number within the 
	// range [0, EdgeLength]

	int nEdges = pPoly->GetNumNMPolyEdges();
	float flEdgeLengthSum = 0.0f;
	for ( int i = 0; i < nEdges; ++i )
	{
		// Skip any edges which:
		// 1) Don't exist.
		// 2) Don't have polies on both sides.
		CAINavMeshEdge* pEdge = pPoly->GetNMPolyEdge( i );
		if ( !pEdge 
			|| kNMPoly_Invalid == pEdge->GetNMPolyIDA()
			|| kNMPoly_Invalid == pEdge->GetNMPolyIDB() )
		{
			continue;
		}

		flEdgeLengthSum += ( pEdge->GetNMEdge0() - pEdge->GetNMEdge1() ).Mag(); // TODO: Don't need to get the mag here.
	}

	// Fail if there are no valid positions.

	float flRandomDistanceAlongEdge = GetRandom( 0.0f, flEdgeLengthSum );
	if ( 0.0f == flRandomDistanceAlongEdge ) 
	{
		return false;
	}

	//
	// Select a position on the edge.
	//

	bool bFoundPosition = false;
	LTVector vSelectedPosition;
	float flRemainingDistance = flRandomDistanceAlongEdge;
	for ( int i = 0; i < nEdges; ++i )
	{
		// Skip any edges which:
		// 1) Don't exist.
		// 2) Don't have polies on both sides.
		CAINavMeshEdge* pEdge = pPoly->GetNMPolyEdge( i );
		if ( !pEdge 
			|| kNMPoly_Invalid == pEdge->GetNMPolyIDA()
			|| kNMPoly_Invalid == pEdge->GetNMPolyIDB() )
		{
			continue;
		}

		float flEdgeLength = ( pEdge->GetNMEdge0() - pEdge->GetNMEdge1() ).Mag(); // TODO: Don't need to get the mag here.
		if ( flRemainingDistance < flEdgeLength )
		{
			// Find the distance along the edge for the position.

			LTVector vEdgeDir = ( pEdge->GetNMEdge1() - pEdge->GetNMEdge0() ).GetUnit();
			vSelectedPosition = pEdge->GetNMEdge0() + ( vEdgeDir * flRemainingDistance );
		
			// Move the position 'out' of the link.  If we don't do this, the AI may 
			// end up standing in the flame pot link. If he does, the penalty 
			// for pathing through the link does not apply.

			LTVector vNormalOutOfLink;
			pEdge->GetNMEdgeN( pPoly->GetNMPolyID(), &vNormalOutOfLink );
			vNormalOutOfLink = -vNormalOutOfLink;

			vSelectedPosition += vNormalOutOfLink * 128.0f;

			bFoundPosition = true;
			break;
		}

		flRemainingDistance -= flEdgeLength;
	}

	if ( false == bFoundPosition )
	{
		return false;
	}

	//
	// Remember the position and objects involved so that the satisfying 
	// actions can use it, while validating that it is still valid.
	//

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_FlamePotPosition );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryExistingFact );
	if ( !pFact )
	{
		pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_FlamePotPosition );
		}
	}

	if ( pFact )
	{
		pFact->SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
		pFact->SetTargetObject( pLink->GetHOBJECT() );
		pFact->SetPos( vSelectedPosition );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCircleFlamePot::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCircleFlamePot::CAIGoalCircleFlamePot()
{
}

CAIGoalCircleFlamePot::~CAIGoalCircleFlamePot()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCircleFlamePot::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalCircleFlamePot
//              
//----------------------------------------------------------------------------

void CAIGoalCircleFlamePot::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalCircleFlamePot::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCircleFlamePot::CalculateGoalRelevance
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalCircleFlamePot::CalculateGoalRelevance()
{
	// Goal is in progress, valid, and not satisfied.  Continue executing.

	if ( m_pAI->GetGoalMgr()->IsCurGoal( this ) 
		&& !IsWSSatisfied( m_pAI->GetAIWorldState() ) 
		&& IsPlanValid() )
	{
	    m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Fail if the flame pot link does not exist or is not active.

	AINavMeshLinkAbstract* pLink = GetCircleableFlamePotLink( m_pAI );
	if ( NULL == pLink )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Fail if we cannot find a position to move to.

	if ( !SelectFlamePotPosition( m_pAI, pLink ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

    m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCircleFlamePot::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalCircleFlamePot::DeactivateGoal()
{
	super::DeactivateGoal();

	// If:
	// 1) The player is an enemy.
	// 2) There is another AI very close by
	// 3) That AI does not have a blitz task
	// ...this AI should blitz the player.  This is an anti-clumping measure.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

	bool bShouldBlitz = false;

	if ( m_pAI->HasTarget( kTarget_Character ) 
		&& IsPlayer( hTarget ) )
	{
		CAI::AIList::const_iterator itEachAI = CAI::GetAIList().begin();
		CAI::AIList::const_iterator itLastAI = CAI::GetAIList().end();
		for ( ; itEachAI != itLastAI; ++itEachAI )
		{
			CAI* pCurrentAI = *itEachAI;

			// Ignore NULL, self and dead AI.

			if ( NULL == pCurrentAI 
				|| pCurrentAI == m_pAI 
				|| IsDeadAI( pCurrentAI->GetHOBJECT() ) )
			{
				continue;
			}

			// Ignore AIs who are far away in 2D (false positives are okay).

			LTVector vDelta2D = ( pCurrentAI->GetPosition() - m_pAI->GetPosition() );
			vDelta2D.y = 0.0f;

			if ( vDelta2D.MagSqr() > g_flTooCloseToEnemySqr )
			{
				continue;
			}

			// Ignore AI who are already blitzing.

			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( kTask_BlitzCharacter );
			if ( pCurrentAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
			{
				continue;
			}

			// AI should blitz.

			bShouldBlitz = true;
			break;
		}
	}

	if ( bShouldBlitz || ( 0 == GetRandom( 0, 2 ) ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_BlitzCharacter );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if ( pFact )
		{
			pFact->SetTaskType( kTask_BlitzCharacter );
			pFact->SetTargetObject( hTarget );
			pFact->SetIndex( kContext_None );
			pFact->SetFactFlags( kFactFlag_Scripted, 1.f );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCircleFlamePot::SetWSSatisfaction
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalCircleFlamePot::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_EnemyInFlamePot );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCircleFlamePot::IsWSSatisfied
//              
//	PURPOSE:	Return true if the world state satisfies the goal.
//              
//----------------------------------------------------------------------------

bool CAIGoalCircleFlamePot::IsWSSatisfied( CAIWorldState* /*pwsWorldState*/ )
{
	// Fail if there is no longer a circleable flame pot link.

	AINavMeshLinkAbstract* pLink = GetCircleableFlamePotLink( m_pAI );
	if ( NULL == pLink )
	{
		return true;
	}

	return false;
}
