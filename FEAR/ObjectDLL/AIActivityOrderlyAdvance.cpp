// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityOrderlyAdvance.cpp
//
// PURPOSE : AIActivityOrderlyAdvance class implementation
//
// CREATED : 4/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivityOrderlyAdvance.h"
#include "AI.h"
#include "AINode.h"
#include "AIWorldState.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AISoundMgr.h"
#include "AIUtils.h"
#include <algorithm>

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityOrderlyAdvance, kActivity_OrderlyAdvance );

#define ARG_ZERO		" 0"
#define ARG_ONE			" 1"
#define ARG_TWO			" 2"
#define ARG_THREE		" 3"
#define ARG_END			" -1"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivityOrderlyAdvance::CAIActivityOrderlyAdvance()
{
	m_nActivityPriority = 4;
	m_fActivityUpdateRate = 0.5f;

	m_eTaskType = kTask_Advance;

	m_hAdvanceAI = NULL;
	m_hAdvanceNode = NULL;

	m_bAdvanceCovered = false;

	m_bEnemySpotted = false;

	m_cFollowAI = 0;
	for( uint32 iFollow=0; iFollow < MAX_AI_FOLLOW; ++iFollow )
	{
		m_aFollowAI[iFollow].hFollowAI = NULL;
		m_aFollowAI[iFollow].fDistSqr = 0.f;
	}
}

CAIActivityOrderlyAdvance::~CAIActivityOrderlyAdvance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAIActivityOrderlyAdvance::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hAdvanceAI );
	SAVE_HOBJECT( m_hAdvanceNode );

	SAVE_bool( m_bAdvanceCovered );

	SAVE_bool( m_bEnemySpotted );

	SAVE_INT( m_cFollowAI );
	for( uint32 iFollow=0; iFollow < MAX_AI_FOLLOW; ++iFollow )
	{
		SAVE_HOBJECT( m_aFollowAI[iFollow].hFollowAI );
		SAVE_FLOAT( m_aFollowAI[iFollow].fDistSqr );
	}
}

void CAIActivityOrderlyAdvance::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hAdvanceAI );
	LOAD_HOBJECT( m_hAdvanceNode );

	LOAD_bool( m_bAdvanceCovered );

	LOAD_bool( m_bEnemySpotted );

	LOAD_INT( m_cFollowAI );
	for( uint32 iFollow=0; iFollow < MAX_AI_FOLLOW; ++iFollow )
	{
		LOAD_HOBJECT( m_aFollowAI[iFollow].hFollowAI );
		LOAD_FLOAT( m_aFollowAI[iFollow].fDistSqr );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::FindActivityParticipants
//
//	PURPOSE:	Return true if successfully found participants.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::FindActivityParticipants()
{
	//
	// Look for someone who's been ordered to advance.
	//

	if( !FindLeader() )
	{
		return false;
	}

	// No one needs to advance.

	if( !IsAI( m_hAdvanceAI ) )
	{
		return false;
	}
	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );

	// Find followers.

	CAI* pCurAI;
	m_cFollowAI = 0;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		// Too many participants.
		if( m_cFollowAI >= MAX_AI_FOLLOW)
			break;

		// Skip if no AI.

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Do not orderly advance if enemy is in sight.

		if( pCurAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			return false;
		}

		// Skip leader.

		if( pCurAI->m_hObject == m_hAdvanceAI )
		{
			continue;
		}

		// Ignore AI that cannot find a path to the leader.
		// Check from the leader back to the followers to optimize caching.

		if( !g_pAIPathMgrNavMesh->HasPath( pLeader, pLeader->GetCharTypeMask(), pCurAI->GetLastNavMeshPoly() ) )
		{
			continue;
		}

		// Record follower.

		m_aFollowAI[m_cFollowAI].hFollowAI = pCurAI->m_hObject;
		++m_cFollowAI;
	}

	// Advance!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::FindLeader
//
//	PURPOSE:	Return true if successfully found leader.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::FindLeader()
{
	// Look for someone who's been ordered to advance.

	CAI* pCurAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		// Bail if no AI.

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Do not orderly advance if enemy is in sight.

		if( pCurAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			return false;
		}

		// We found someone with an Advance task.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( m_eTaskType );
		CAIWMFact* pFact = pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			HOBJECT hNode = pFact->GetTargetObject();
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
			if( pNode )
			{
				m_hAdvanceAI = pCurAI->m_hObject;
				m_hAdvanceNode = hNode;

				// Leader wants someone to cover his back.

				if( pFact->IsSet( CAIWMFact::kFactMask_DesireType ) && 
					( pFact->GetDesireType() == kDesire_Covered ) )
				{
					m_bAdvanceCovered = true;
				}
			}
			break;
		}
	}

	return !!m_hAdvanceAI;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIActivityOrderlyAdvance::CompareFollowers
//              
//	PURPOSE:	Compare function used to sort followers.
//              
//----------------------------------------------------------------------------

static bool CompareFollowers(const SAI_FOLLOWER& Follower1, const SAI_FOLLOWER& Follower2)
{
	// Followers are sorted by distance to their leader.

	if( Follower1.fDistSqr < Follower2.fDistSqr )
	{
		return true;
	}
	
	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIActivityOrderlyAdvance::SortFollowers
//              
//	PURPOSE:	Sort followers by distance to leader.
//              
//----------------------------------------------------------------------------

void CAIActivityOrderlyAdvance::SortFollowers()
{
	// Bail if no leader.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( !pLeader )
	{
		return;
	}
	
	LTVector vLeaderPos = pLeader->GetPosition();

	// Calculate distances.

	CAI* pAI;
	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		// AI does not exist.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		if( !pAI )
		{
			continue;
		}

		m_aFollowAI[iFollow].fDistSqr = vLeaderPos.DistSqr( pAI->GetPosition() );
	}

	// Sort!

	std::sort( &m_aFollowAI[0], &m_aFollowAI[m_cFollowAI], CompareFollowers );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::ActivateActivity
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::ActivateActivity()
{
	if( !super::ActivateActivity() )
	{
		return false;
	}

	// Missing requirements.

	if( !m_hAdvanceAI )
	{
		return false;
	}

	// Order advancing AI to goto the destination node.

	if( m_hAdvanceAI )
	{
		// AI does not exist.

		CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
		if( !pLeader )
		{
			return false;
		}

		// Sort followers by distance to leader.

		SortFollowers();

		// Send follow commands to followers.

		CAI* pAI;
		CAI* pLast = pLeader;
		std::string strCmd;
		for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
		{
			// AI does not exist.

			pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
			if( !pAI )
			{
				continue;
			}

			// Skip if follower is the leader.
			// This should never happen!

			if( pAI == pLeader )
			{
				continue;
			}

			// Send follow orders.

			strCmd = "FOLLOW ";
			strCmd += pLast->GetName();

			if( iFollow == m_cFollowAI - 1 )
			{
				if( m_bAdvanceCovered )
				{
					strCmd += ARG_END;
				}
				else {
					strCmd += ARG_THREE;
				}
			}
			else if( iFollow == 0 )
			{
				strCmd += ARG_ZERO;
			}
			else if( iFollow == 1 )
			{
				strCmd += ARG_ONE;
			}
			else if( iFollow == 2 )
			{
				strCmd += ARG_TWO;
			}

			strCmd += " ";
			strCmd += pLeader->GetName();

			g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );

			// Next AI will follow this AI.

			pLast = pAI;
		}
	}

	// Initialize activity.

	m_eActStatus = kActStatus_Initialized;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::DeactivateActivity
//
//	PURPOSE:	Deactivate an active activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityOrderlyAdvance::DeactivateActivity()
{
	super::DeactivateActivity();

	m_hAdvanceNode = NULL;

	// Clear orders.

	ClearOrders();

	// Someone in the squad saw the enemy, so tell everyone else.

	CAI* pAI;
	if( m_bEnemySpotted )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "SEEKSQUADENEMY" );
	}

	// Clear participants.

	m_hAdvanceAI = NULL;
	m_bAdvanceCovered = false;

	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		m_aFollowAI[iFollow].hFollowAI = NULL;

		// Someone in the squad saw the enemy, so tell everyone else.

		if( m_bEnemySpotted )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
			g_pCmdMgr->QueueMessage( pAI, pAI, "SEEKSQUADENEMY" );
		}
	}
	m_cFollowAI = 0;

	m_bEnemySpotted = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::ClearOrders
//
//	PURPOSE:	Clears orders issued during activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityOrderlyAdvance::ClearOrders()
{
	// Clear orders.

	CAI* pAI;
	if( m_hAdvanceAI )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "ADVANCE NONE" );
		g_pCmdMgr->QueueMessage( pAI, pAI, "GOTO NONE" );
	}

	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		// AI does not exist.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::UpdateActivity
//
//	PURPOSE:	Return true if Activity should continue running.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::UpdateActivity()
{
	if( !super::UpdateActivity() )
	{
		return false;
	}

	// Advancing AI disappeared.

	if( !m_hAdvanceAI )
	{
		m_eActStatus = kActStatus_Failed;
		return false;
	}

	// Bail if leader sees the enemy.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( pAI && pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
	{
		AITRACE( AIShowActivities, ( m_hAdvanceAI, "Enemy spotted! Activity bailing." ) );
		m_eActStatus = kActStatus_Failed;
		m_bEnemySpotted = true;
		return false;
	}

	// Bail if follower sees the enemy.

	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		// Do not orderly advance if enemy is in sight.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		if( pAI && pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			AITRACE( AIShowActivities, ( m_hAdvanceAI, "Enemy spotted! Activity bailing." ) );
			m_eActStatus = kActStatus_Failed;
			m_bEnemySpotted = true;
			return false;
		}
	}

	// Don't check for reaching the destination on the first update.

	if( m_eActStatus == kActStatus_Initialized )
	{
		return BeginAdvance();
	}

	// Head to destination.

	if( m_eActStatus == kActStatus_Advancing )
	{
		// Reached the destination.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
		if( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && ( pProp->hWSValue == m_hAdvanceNode ) )
			{
				return HandleAdvanceNodeArrival();
			}
		}

		// Leader heading somewhere else.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hAdvanceNode );
		if( pNode && ( pAI->GetAIBlackBoard()->GetBBDest() != pNode->GetPos() ) )
		{
			AITRACE( AIShowActivities, ( m_hAdvanceAI, "Done advancing. Activity complete." ) );
			m_eActStatus = kActStatus_Complete;
			return false;
		}
	}

	// Continue updating.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::BeginAdvance
//
//	PURPOSE:	Kick-off the advance.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::BeginAdvance()
{
	// Leader waits for followers to begin moving before advancing.

	if( !SquadRegroup() )
	{
		return true;
	}

	// AI does not exist.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( !pLeader )
	{
		return false;
	}

	// Bail if node does not exist.

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hAdvanceNode );
	if( !pNode )
	{
		return false;
	}

	// Send goto command to the leader.

	std::string strCmd = "GOTO ";
	strCmd += pNode->GetNodeName();
	g_pCmdMgr->QueueMessage( pLeader, pLeader, strCmd.c_str() );
	AITRACE( AIShowActivities, ( m_hAdvanceAI, "Ordered to Advance" ) );

	m_eActStatus = kActStatus_Advancing;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::SquadRegroup
//
//	PURPOSE:	Regroup the squad before advancing.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::SquadRegroup()
{
	// Leader waits for followers to begin moving before advancing.

	if( g_pLTServer->GetTime() == m_fActivityActivateTime )
	{
		return false;
	}

	// Squad is ready when all followers have gotten into position.

	CAI* pAI;
	bool bSquadReady = true;
	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		// AI does not exist.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		if( !pAI )
		{
			continue;
		}

		EnumAnimProp eProp = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Movement );
		if( eProp != kAP_None )
		{
			return false;
		}
	}

	// Followers are in range of the leader.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::HandleAdvanceNodeArrival
//
//	PURPOSE:	Handle arriving at the advance destination node.
//
// ----------------------------------------------------------------------- //

bool CAIActivityOrderlyAdvance::HandleAdvanceNodeArrival()
{
	g_pAISoundMgr->RequestAISound( m_hAdvanceAI, kAIS_FanOut, kAISndCat_Event, NULL, 0.f );

	AITRACE( AIShowActivities, ( m_hAdvanceAI, "Done advancing. Activity complete." ) );
	m_eActStatus = kActStatus_Complete;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityOrderlyAdvance::ClearDeadAI
//
//	PURPOSE:	Clear handles to dead AI.
//
// ----------------------------------------------------------------------- //

void CAIActivityOrderlyAdvance::ClearDeadAI()
{
	super::ClearDeadAI();

	if( m_hAdvanceAI && IsDeadAI( m_hAdvanceAI ) )
	{
		m_hAdvanceAI = NULL;
	}

	for( uint32 iFollow=0; iFollow < MAX_AI_FOLLOW; ++iFollow )
	{
		if( m_aFollowAI[iFollow].hFollowAI && IsDeadAI( m_aFollowAI[iFollow].hFollowAI ) )
		{
			m_aFollowAI[iFollow].hFollowAI = NULL;
		}
	}
}
