// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityGetToCover.cpp
//
// PURPOSE : AIActivityGetToCover class implementation
//
// CREATED : 6/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivityGetToCover.h"
#include "AI.h"
#include "AINodeMgr.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"
#include "AIWorldState.h"
#include "AIWorkingMemory.h"
#include "AISoundMgr.h"
#include "AITarget.h"
#include "AISquad.h"
#include "AIUtils.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityGetToCover, kActivity_GetToCover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivityGetToCover::CAIActivityGetToCover()
{
	m_nActivityPriority = 2;

	m_fActivityUpdateRate = 0.5f;
	m_fActivityTimeOut = 10.f;

	m_hTarget = NULL;
	m_hSuppressAI = NULL;

	m_hHeavyArmor = NULL;

	m_bPlayedFailureSound = false;
	m_bSquadReloaded = false;

	m_cBlockedNodes = 0;
	for( int iCover=0; iCover < MAX_AI_GET_COVER; ++iCover )
	{
		m_hCoverAI[iCover] = NULL;

		// Each AI can lock two nodes: cover and dependency.

		m_aBlockedNodes[2 * iCover] = NULL;
		m_aBlockedNodes[(2 * iCover) + 1] = NULL;
	}
}

CAIActivityGetToCover::~CAIActivityGetToCover()
{
}

void CAIActivityGetToCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hTarget);
	SAVE_HOBJECT(m_hSuppressAI);

	SAVE_HOBJECT(m_hHeavyArmor);

	SAVE_bool(m_bPlayedFailureSound);
	SAVE_bool(m_bSquadReloaded);

	for (int i = 0 ; i < MAX_AI_GET_COVER; ++i)
	{
		SAVE_HOBJECT(m_hCoverAI[i]);
	}
}

void CAIActivityGetToCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hTarget);
	LOAD_HOBJECT(m_hSuppressAI);

	LOAD_HOBJECT(m_hHeavyArmor);

	LOAD_bool(m_bPlayedFailureSound);
	LOAD_bool(m_bSquadReloaded);

	for (int i = 0 ; i < MAX_AI_GET_COVER; ++i)
	{
		LOAD_HOBJECT(m_hCoverAI[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::InitActivity
//
//	PURPOSE:	Initialize the activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::InitActivity()
{
	super::InitActivity();

	m_hTarget = NULL;
	m_hSuppressAI = NULL;

	m_bPlayedFailureSound = false;
	m_bSquadReloaded = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::FindActivityParticipants
//
//	PURPOSE:	Return true if successfully found participants.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::FindActivityParticipants()
{
	CAI* pCurAI = NULL;
	AINode* pNode;


	//
	// Look for someone in need of cover.
	//

	HOBJECT hHasCover = NULL;
	HOBJECT hNeedsCover = NULL;
	m_hTarget = NULL;
	uint32 iPotential;
	for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Skip if AI has no target.

		if( !pCurAI->HasTarget( kTarget_Character | kTarget_Object ) )
		{
			continue;
		}
		HOBJECT hTarget = pCurAI->GetAIBlackBoard()->GetBBTargetObject();

		// Skip if the AI does not have access to a ranged weapon

		if (!AIWeaponUtils::HasWeaponType(pCurAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER))
		{
			continue;
		}

		// Skip if AI already has valid cover.

		pNode = AtCoverNode( pCurAI, !CHECK_AMBUSH );
		if( pNode && IsCoverNodeValid( pCurAI, pNode, hTarget, VALID_STRICT ) )
		{
			hHasCover = pCurAI->m_hObject;
			continue;
		}

		// Skip AI who have a scripted task (ambush or cover).

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetFactFlags( kFactFlag_Scripted );
		if( pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			continue;
		}

		// Only go to an ambush node if cover is strictly invalid.
		// Don't go to ambush from cover if someone is aiming at me.

		bool bCheckAmbush = true;
		if( pNode && IsCoverNodeValid( pCurAI, pNode, hTarget, VALID_LENIENT ) )
		{
			bCheckAmbush = false;
		}

		// Skip if AI has no valid cover.

		if( !FindValidCoverNodeFact( pCurAI, hTarget, bCheckAmbush ) )
		{
			hNeedsCover = pCurAI->m_hObject;
			continue;
		}

		// Found an AI who needs cover.

		m_hCoverAI[0] = pCurAI->m_hObject;
		m_hTarget = hTarget;
		break;
	}

	// Bail if no AI found that needs cover and has available cover.

	if( !m_hTarget )
	{
		PlaySquadFailureAISound( hHasCover, hNeedsCover );
		return false;
	}

	// AI is going to cover, and firing again.
	// They are no longer holding positions.

	m_bPlayedFailureSound = false;


	//
	// Look for someone to lay suppression fire.
	// Preferably someone who is already in cover.
	// Don't lay supression fire against a turret.
	//

	CAIWMFact* pTimeoutFact;
	CAIWMFact factTimeoutQuery;
	factTimeoutQuery.SetFactType( kFact_Knowledge );
	factTimeoutQuery.SetKnowledgeType( kKnowledge_NextSuppressTime );

	if( !IsTurret( m_hTarget ) )
	{
		for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
		{
			pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
			if( !pCurAI )
			{
				continue;
			}

			// Skip AI who are not targeting a character.
			// Only suppress against a character (and not against a turret!)

			if( !pCurAI->HasTarget( kTarget_Character ) )
			{
				continue;
			}
			HOBJECT hTarget = pCurAI->GetAIBlackBoard()->GetBBTargetObject();
			if( hTarget != m_hTarget )
			{
				continue;
			}

			// Skip AI who are playing a locked animation (e.g. flipping something).

			if( pCurAI->GetAnimationContext()->IsLocked() )
			{
				continue;
			}

			// Skip if AI cannot see its target.

			if( !pCurAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
			{
				continue;
			}

			// Skip if the AI does not have access to a ranged weapon

			if (!AIWeaponUtils::HasWeaponType(pCurAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER))
			{
				continue;
			}

			// Skip AI who have suppressed too recently.

			pTimeoutFact = pCurAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
			if( pTimeoutFact && ( pTimeoutFact->GetTime() > g_pLTServer->GetTime() ) )
			{
				continue;
			}

			// Skip AI who are already en route to a cover node.

			if( EnRouteToCoverNode( pCurAI, !CHECK_AMBUSH ) )
			{
				continue;
			}

			m_hSuppressAI = pCurAI->m_hObject;

			// Stop searching if we've found an AI that can
			// lay suppression fire from cover.

			pNode = AtCoverNode( pCurAI, !CHECK_AMBUSH );
			if( pNode )
			{
				// Lay suppression from a valid cover node.

				if( IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_STRICT ) )
				{
					break;
				}

				// Don't lay suppression from an invalid cover node.

				else {
					m_hSuppressAI = NULL;
				}
			}
		}
	}

	//
	// Clear the list of participating AI.
	//

	int iGetCover;
	for( iGetCover=0; iGetCover < MAX_AI_GET_COVER; ++iGetCover )
	{
		m_hCoverAI[iGetCover] = NULL;
	}


	//
	// Find all AI who have available valid cover from enemy.
	//

	iGetCover = 0;
	for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Ignore the AI that has already been selected to
		// lay suppression fire.

		if( pCurAI->m_hObject == m_hSuppressAI )
		{
			continue;
		}
		
		// Skip if the AI does not have access to a ranged weapon

		if (!AIWeaponUtils::HasWeaponType(pCurAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER))
		{
			continue;
		}

		// Ignore AI that are already at valid cover nodes.

		pNode = AtCoverNode( pCurAI, !CHECK_AMBUSH );
		if( pNode && IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_STRICT ) )
		{
			continue;
		}

		// Only go to an ambush node if cover is strictly invalid.
		// Don't go to ambush from cover if someone is aiming at me.

		bool bCheckAmbush = true;
		if( pNode && IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_LENIENT ) )
		{
			bCheckAmbush = false;
		}

		// Ignore AI that have a scripted Task (cover or ambush).

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetFactFlags( kFactFlag_Scripted );
		if( pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			continue;
		}

		// Record AI that are en route to cover.

		if( EnRouteToCoverNode( pCurAI, !CHECK_AMBUSH ) )
		{
			m_hCoverAI[iGetCover] = pCurAI->m_hObject;
			++iGetCover;
			if( iGetCover == MAX_AI_GET_COVER )
			{
				break;
			}
		}

		// Record AI that have valid cover available.

		else if( FindValidCoverNodeFact( pCurAI, m_hTarget, bCheckAmbush ) )
		{
			m_hCoverAI[iGetCover] = pCurAI->m_hObject;
			++iGetCover;
			if( iGetCover == MAX_AI_GET_COVER )
			{
				break;
			}
		}
	}

	// If only one AI needs cover, no one should lay suppression fire.

	if( iGetCover == 0 )
	{
		m_hCoverAI[0] = m_hSuppressAI;
		m_hSuppressAI = NULL;
	}

	// Record the squad's target.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_SquadTarget );
	factQuery.SetIndex( m_pSquad->GetSquadID() );

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
	if (!pFact)
	{
		pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);
	}

	if (pFact)
	{
		pFact->SetKnowledgeType( kKnowledge_SquadTarget, 1.f );
		pFact->SetIndex( m_pSquad->GetSquadID(), 1.f );
		pFact->SetTargetObject( m_hTarget, 1.f );
	}

	// Found participants.

AITRACE( AIShowGoals, ( m_hCoverAI[0], "GET TO COVER VALID" ) );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::ActivateActivity
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::ActivateActivity()
{
	if( !super::ActivateActivity() )
	{
		return false;
	}

	// Ensure that entire squad reacts.

	AlertSquadMembers();

	// Bail if participants are missing.

	if( !ValidParticipantsStillExist() )
	{
		return false;
	}

	// Order suppressing AI to lay suppression fire.

	if( m_hSuppressAI )
	{
		CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hSuppressAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SUPPRESSIONFIRE" );
		AITRACE( AIShowActivities, ( m_hSuppressAI, "Ordered to Suppress" ) );
	}

	// Initialize activity.

	m_eActStatus = kActStatus_Initialized;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::DeactivateActivity
//
//	PURPOSE:	Deactivate an active activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::DeactivateActivity()
{
	super::DeactivateActivity();

	CAI* pAI;

	m_hTarget = NULL;

	if( m_hSuppressAI )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hSuppressAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "CLEARTASK" );
		m_hSuppressAI = NULL;
	}

	m_hHeavyArmor = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::AlertSquadMembers
//
//	PURPOSE:	Ensure everyone is aware of a threat.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::AlertSquadMembers()
{
	CAI* pCurAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Ensure everyone is alert!

		pCurAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::UpdateActivity
//
//	PURPOSE:	Return true if Activity should continue running.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::UpdateActivity()
{
	if( !super::UpdateActivity() )
	{
		return false;
	}

	// Bail if participants are missing.

	if( !ValidParticipantsStillExist() )
	{
		m_eActStatus = kActStatus_Failed;
		return false;
	}

	//
	// Check progress.
	//

	switch( m_eActStatus )
	{
		//
		// Waiting for suppressing AI to start supressing.
		//

		case kActStatus_Initialized:
		{
			// Wait until the suppressing AI has started suppressing.
			// (He might have been pre-occupied reloading when the order came in).
			// If there is no suppressing AI, just tell everyone to go to cover.

			if( !WaitingForSuppressionFire() )
			{
				// Advance the activity status.

				m_eActStatus = kActStatus_Suppressing; 

				// Order other AI to cover nodes.
				
				uint32 cCovering = IssueCoverOrders();
				if( cCovering == 0 )
				{
					m_eActStatus = kActStatus_Failed;
					return false;
				}

				// Play an AI sound for the squad.

				PlaySquadAISound( cCovering );
			}
		}
		break;

		//
		// Waiting for AI to get to cover.
		//

		case kActStatus_Suppressing:
			{
				// Bail if any AI are at or heading to invalid cover.

				if( !DestinationsAreValid() )
				{
					m_eActStatus = kActStatus_Failed;
					return false;
				}

				// We are done when no one is left going for cover.

				uint32 cCovering = ClearCompleted();
				if( cCovering == 0 )
				{					
					AITRACE( AIShowActivities, ( m_hSuppressAI, "Done suppressing. Activity complete." ) );
					m_eActStatus = kActStatus_Complete;
					return false;
				}
			}
			break;
	}

	//
	// Continue running.
	//

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::ClearDeadAI
//
//	PURPOSE:	Clear handles to dead AI.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::ClearDeadAI()
{
	super::ClearDeadAI();

	if( m_hTarget && IsDeadAI( m_hTarget ) )
	{
		m_hTarget = NULL;
	}

	if( m_hSuppressAI && IsDeadAI( m_hSuppressAI ) )
	{
		m_hSuppressAI = NULL;
	}

	for( uint32 iCover=0; iCover < MAX_AI_GET_COVER; ++iCover )
	{
		if( m_hCoverAI[iCover] && IsDeadAI( m_hCoverAI[iCover] ) )
		{
			m_hCoverAI[iCover] = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::WaitingForSuppressionFire
//
//	PURPOSE:	Return true if we are still waiting for suppression fire to commence.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::WaitingForSuppressionFire()
{
	// Don't wait if there is no suppressor.

	if( !m_hSuppressAI )
	{
		return false;
	}

	CAI* pSuppressor = (CAI*)g_pLTServer->HandleToObject( m_hSuppressAI );
	if( !pSuppressor )
	{
		return false;
	}

	// Don't wait if suppressor's vision is obscured by an ally (because 
	// this could lead to waiting infinitely).

	if( !pSuppressor->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Don't wait if AI has started suppressing.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Knowledge);
	factQuery.SetKnowledgeType(kKnowledge_Suppressing);
	factQuery.SetSourceObject(m_hSuppressAI);

	if( g_pAIWorkingMemoryCentral->CountMatches( factQuery ) )
	{
		return false;
	}

	// Keep waiting.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::ValidParticipantsStillExist
//
//	PURPOSE:	Return true if valid participants still exist.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::ValidParticipantsStillExist()
{
	int iGetCover;
	for( iGetCover=0; iGetCover < MAX_AI_GET_COVER; ++iGetCover )
	{
		if( m_hCoverAI[iGetCover] )
		{
			break;
		}
	}

	// There is no one left to get to cover.

	if( iGetCover == MAX_AI_GET_COVER )
	{
		return false;
	}

	// There are still AI left to get to cover.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::DestinationsAreValid
//
//	PURPOSE:	Return true if Squad members are at or going to valid cover.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::DestinationsAreValid()
{
	CAI* pCurAI;
	AINode* pNode;

	// Iterate over squad members who may be participating.

	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Ignore AI that have a scripted Task (cover or ambush).

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetFactFlags( kFactFlag_Scripted );
		if( pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			continue;
		}

		// Activity is invalid if AI is heading to invalid cover.

		pNode = EnRouteToCoverNode( pCurAI, CHECK_AMBUSH );
		if( pNode )
		{
			if( !IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_STRICT ) )
			{
				// Clear invalidated cover from memory.

				ClearCoverTasks( pCurAI );
				return false;
			}
		}

		// Activity is invalid if AI is at an invalid node.

		else 
		{
			pNode = AtCoverNode( pCurAI, CHECK_AMBUSH );
			if( pNode && !IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_STRICT ) )
			{
				return false;
			}
		}
	}

	// Activity is invalid if suppressing AI has been suppressing for too long.

	if( m_hSuppressAI )
	{
		CAI* pSuppressAI = (CAI*)g_pLTServer->HandleToObject( m_hSuppressAI );
		if( pSuppressAI && ( pSuppressAI->GetAIBlackBoard()->GetBBStateChangeTime() < g_pLTServer->GetTime() - MAX_SUPPRESSION_TIME ) )
		{
			return false;
		}
	}

	// Activity is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::IssueCoverOrders
//
//	PURPOSE:	Return the number of AI going for cover.
//
// ----------------------------------------------------------------------- //

uint32 CAIActivityGetToCover::IssueCoverOrders()
{
	CAI* pAI;
	AINode* pNode;
	CAIWMFact* pFact;

	bool bCoverAvailable;

	uint32 cCovering = 0;
	for( int iGetCover=0; iGetCover < MAX_AI_GET_COVER; ++iGetCover )
	{
		bCoverAvailable = false;

		if( m_hCoverAI[iGetCover] )
		{
			// Bail if no AI.

			pAI = (CAI*)g_pLTServer->HandleToObject( m_hCoverAI[iGetCover] );
			if( !pAI )
			{
				continue;
			}

			// If AI is en route to valid cover, keep going there.

			pNode = EnRouteToCoverNode( pAI, !CHECK_AMBUSH );
			if( pNode )
			{
				if( IsCoverNodeValid( pAI, pNode, m_hTarget, VALID_STRICT ) )
				{
					bCoverAvailable = true;

					// Lock node so others will not claim it.

					if( !pNode->IsNodeLocked() )
					{
						BlockNode( m_hCoverAI[iGetCover], pNode );
					}
				}

				// Clear invalidated cover from memory.

				else {
					ClearCoverTasks( pAI );
				}
			}

			// AI is NOT en route to somewhere valid, so find a node to goto.

			if( !bCoverAvailable )
			{
				// Only go to an ambush node if cover is strictly invalid.
				// Don't go to ambush from cover if someone is aiming at me.

				bool bCheckAmbush = true;
				pNode = AtCoverNode( pAI, !CHECK_AMBUSH );
				if( pNode && IsCoverNodeValid( pAI, pNode, m_hTarget, VALID_LENIENT ) )
				{
					bCheckAmbush = false;
				}

				// If we are already en route to an ambush node, do not consider
				// going to another one.  Only change destinations if a cover
				// node is now available.

				if( EnRouteToCoverNode( pAI, CHECK_AMBUSH ) )
				{
					bCoverAvailable = true;
					bCheckAmbush = false;
				}

				pFact = FindValidCoverNodeFact( pAI, m_hTarget, bCheckAmbush );
				if( pFact )
				{
					pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
					if( pNode )
					{
						// Lock node so others will not claim it.

						BlockNode( m_hCoverAI[iGetCover], pNode );

						// Send order to AI.

						char szMsg[128];
						if( pNode->GetType() == kNode_Cover )
						{
							LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "PRIVATE_COVER %s", pNode->GetNodeName() );		
							g_pCmdMgr->QueueMessage( pAI, pAI, szMsg );
							AITRACE( AIShowActivities, ( m_hCoverAI[iGetCover], "Ordered to Cover" ) );

							AINodeCover* pNodeCover = (AINodeCover*)pNode;
							if( pNodeCover->GetDependency() && IsAINodeSmartObject( pNodeCover->GetDependency() ) )
							{
								AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pNodeCover->GetDependency() );
								if( pNodeSmartObject->GetSmartObject() &&
									pNodeSmartObject->GetSmartObject()->eDependencyType == kDependency_Occupied )
								{
									m_hHeavyArmor = pNodeSmartObject->GetLockingAI();
								}
							}
						}
						else if( pNode->GetType() == kNode_Ambush )
						{
							LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "PRIVATE_AMBUSH %s", pNode->GetNodeName() );		
							g_pCmdMgr->QueueMessage( pAI, pAI, szMsg );
							AITRACE( AIShowActivities, ( m_hCoverAI[iGetCover], "Ordered to Ambush" ) );
						}

						bCoverAvailable = true;
					}
				}
			}

			// Found available cover.

			if( bCoverAvailable )
			{
				// At least 1 AI is going for cover.

				++cCovering;
			}
			else {
				m_hCoverAI[iGetCover] = NULL;
			}
		}
	}

	// Unlock any nodes blocked during searches.

	UnblockNodes();

	return cCovering;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::ClearCompleted
//
//	PURPOSE:	Return the number of AI still going for cover.
//
// ----------------------------------------------------------------------- //

uint32 CAIActivityGetToCover::ClearCompleted()
{
	// Iterate over all AI who are going to cover.

	CAI* pAI;
	AINode* pNode;
	uint32 cCovering = 0;
	for( int iGetCover=0; iGetCover < MAX_AI_GET_COVER; ++iGetCover )
	{
		if( m_hCoverAI[iGetCover] )
		{
			// If AI has reached a cover node, he is no 
			// longer a participant.

			pAI = (CAI*)g_pLTServer->HandleToObject( m_hCoverAI[iGetCover] );
			pNode = AtCoverNode( pAI, CHECK_AMBUSH );
			if( IsCoverNodeValid( pAI, pNode, m_hTarget, VALID_STRICT ) )
			{
				AITRACE( AIShowActivities, ( m_hCoverAI[iGetCover], "Reached Cover" ) );
				m_hCoverAI[iGetCover] = NULL;
			}

			// Count how many AI are still in the process of going for cover.

			else {
				++cCovering;
			}
		}
	}

	return cCovering;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::PlaySquadFailureAISound
//
//	PURPOSE:	Play appropriate AISound for situation.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::PlaySquadFailureAISound( HOBJECT hHasCover, HOBJECT hNeedsCover )
{
	if( m_bPlayedFailureSound )
	{
		return;
	}

	// No one to talk to.

	if( m_cPotentialParticipants == 1 )
	{
		return;
	}

	// AI in cover cannot see the enemy.

	if( hHasCover && SquadHasEverSeenThreat() )
	{
		HOBJECT hSeeEnemy = FindAllySeesThreat( NULL, !VISIBLE_STRICT );
		if( !hSeeEnemy )
		{
			CAI* pCurAI;
			bool bEnoughReloads = false;
			HOBJECT hKnowsLocation = NULL;
			EnumAISoundType eLocation = kAIS_InvalidType;
			HOBJECT hOther = NULL;
			for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
			{
				pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
				if( !pCurAI )
				{
					continue;
				}

				if( pCurAI->GetAIBlackBoard()->GetBBTargetLocation() != kAIS_InvalidType )
				{
					hKnowsLocation = m_aPotentialParticipants[iPotential];
					eLocation = pCurAI->GetAIBlackBoard()->GetBBTargetLocation();
				}

				if( pCurAI->GetAIBlackBoard()->GetBBRoundsFired() >= 3 )
				{
					bEnoughReloads = true;
				}

				if( m_aPotentialParticipants[iPotential] != hHasCover )
				{
					hOther = m_aPotentialParticipants[iPotential];
				}
			}

			// "Where is he?"
			// "Behind the column!"

			if( eLocation != kAIS_InvalidType )
			{
				if( GetRandom( 0.f, 1.f ) > 0.5f )
				{
					g_pAISoundMgr->RequestAISound( hKnowsLocation, eLocation, kAISndCat_Location, NULL, 0.f );
					g_pAISoundMgr->SkipAISound( hOther, kAIS_WhereIsHe );
				}
				else 
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_WhereIsHe, kAISndCat_Event, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hKnowsLocation, eLocation, hOther, kAIS_WhereIsHe, kAIS_WhereIsHe, kAISndCat_Location, NULL, 0.5f );
				}
			}

			// "I need ammo"
			// "Here ya go"
			// "Thanks"

			else if( bEnoughReloads && !m_bSquadReloaded )
			{
				g_pAISoundMgr->RequestAISound( hOther, kAIS_RequestAmmo, kAISndCat_Event, NULL, 0.f );
				g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_Give, hOther, kAIS_RequestAmmo, kAIS_RequestAmmo, kAISndCat_Event, NULL, 0.5f );
				g_pAISoundMgr->RequestAISoundSequence( hOther, kAIS_Thanks, hHasCover, kAIS_Give, kAIS_RequestAmmo, kAISndCat_Event, NULL, 0.5f );
				m_bSquadReloaded = true;
			}

			// Random squad communication.

			else if( SquadHasEverSeenThreat() )
			{
				float fRand = GetRandom( 0.f, 1.f );

				// "Hold your position"
				// "Roger"

				if( fRand < 0.2f )
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_HoldPosition, kAISndCat_CheckIn, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_Affirmative, hOther, kAIS_HoldPosition, kAIS_HoldPosition, kAISndCat_Always, NULL, 0.5f );
				}

				// "Do you see him?"
				// "Negative"

				else if( fRand < 0.4f )
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_DoYouSeeHim, kAISndCat_CheckIn, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_Negative, hOther, kAIS_DoYouSeeHim, kAIS_DoYouSeeHim, kAISndCat_Always, NULL, 0.5f );
				}

				// "Fire!"
				// "I can't get a shot."

				else if( ( fRand < 0.6f ) && SquadHasSeenThreatRecently() )
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_Attack, kAISndCat_CheckIn, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_NoShot, hOther, kAIS_Attack, kAIS_Attack, kAISndCat_Always, NULL, 0.5f );
				}
 
				// "Squad check in"
				// "Roger"

				else if( fRand < 0.8f )
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_CheckIn, kAISndCat_CheckIn, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_Affirmative, hOther, kAIS_CheckIn, kAIS_CheckIn, kAISndCat_Always, NULL, 0.5f );
				}

				// "Anyone see him?"
				// "Shut the fuck up!"

				else
				{
					g_pAISoundMgr->RequestAISound( hOther, kAIS_RandomChatter, kAISndCat_CheckIn, NULL, 0.f );
					g_pAISoundMgr->RequestAISoundSequence( hHasCover, kAIS_ShutUp, hOther, kAIS_RandomChatter, kAIS_RandomChatter, kAISndCat_Always, NULL, 0.3f );
				}
			}

			m_bPlayedFailureSound = true;
			return;
		}
	}

	// Bail if no one currently needs cover.

	if( !hNeedsCover )
	{
		return;
	}

	// Find an AI in invalid cover.

	CAI* pAINeedsCover = NULL;
	HOBJECT hTarget;
	CAI* pCurAI;
	AINode* pNodeInvalidCover;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}
		if( pCurAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() < g_pLTServer->GetTime() - 0.5f )
		{
			continue;
		}

		hTarget = pCurAI->GetAIBlackBoard()->GetBBTargetObject();
		pNodeInvalidCover = AtCoverNode( pCurAI, CHECK_AMBUSH );
		if( pNodeInvalidCover && ( !IsCoverNodeValid( pCurAI, pNodeInvalidCover, hTarget, VALID_STRICT ) ) )
		{
			pAINeedsCover = pCurAI;
			break;
		}
	}
	
	// No AI is in cover, and in need of new cover.

	if( !pAINeedsCover )
	{
		return;
	}

	// No witness exists.

	HOBJECT hWitness = FindAllySeesThreat( pAINeedsCover->m_hObject, VISIBLE_STRICT );
	if( !hWitness )
	{
		// "He's rushing me!"

		if( !pNodeInvalidCover->IsNodeValid( pAINeedsCover, pAINeedsCover->GetPosition(), hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatInsideRadius ) )
		{
			g_pAISoundMgr->RequestAISound( pAINeedsCover->m_hObject, kAIS_RequestCoverRadius, kAISndCat_Event, hTarget, 0.f );
		}

		// "Under fire!"

		else if( !pNodeInvalidCover->IsNodeValid( pAINeedsCover, pAINeedsCover->GetPosition(), hTarget, kThreatPos_TargetPos, kNodeStatus_Damaged ) )
		{
			g_pAISoundMgr->RequestAISound( pAINeedsCover->m_hObject, kAIS_UnderFire, kAISndCat_Event, hTarget, 0.f );
		}

		m_bPlayedFailureSound = true;
		return;
	}

	// An ally witness does exist.

	// Only speak if someone is aiming at the AI in invalid cover.

	if( pNodeInvalidCover->IsNodeValid( pAINeedsCover, pAINeedsCover->GetPosition(), hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatAimingAtNode ) )
	{
		return;
	}

	// Base dialogue on the presence of recent weaponfire from the threat.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Disturbance );
	factQuery.SetTargetObject( hTarget );
	CAIWMFact* pFact = pAINeedsCover->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && pFact->GetUpdateTime() >= g_pLTServer->GetTime() - 0.5f )
	{
		// "Get out of there!"
		// "He's got me pinned down!"

		g_pAISoundMgr->RequestAISound( hWitness, kAIS_OrderMove, kAISndCat_LimitedWarnAlly, NULL, 0.f );
		g_pAISoundMgr->RequestAISoundSequence( pAINeedsCover->m_hObject, kAIS_PinnedDown, hWitness, kAIS_OrderMove, kAIS_OrderMove, kAISndCat_TargetVisible, NULL, 0.2f );
	}

	// Threat has not fired recently.

	else if( GetRandom( 0.f, 1.f ) > 0.6f )
	{
		// "Get out of there!"
		// "I've got nowhere to go!"

		g_pAISoundMgr->RequestAISound( hWitness, kAIS_OrderMove, kAISndCat_LimitedWarnAlly, NULL, 0.f );
		g_pAISoundMgr->RequestAISoundSequence( pAINeedsCover->m_hObject, kAIS_NowhereToGo, hWitness, kAIS_OrderMove, kAIS_OrderMove, kAISndCat_TargetVisible, NULL, 0.2f );
	}
	else
	{
		// "He's aiming at you"
		// "Where should I go?"

		g_pAISoundMgr->RequestAISound( hWitness, kAIS_OrderCoverAim, kAISndCat_LimitedWarnAlly, NULL, 0.f );
		if( GetRandom( 0.f, 1.f ) > 0.5f )
		{
			g_pAISoundMgr->RequestAISoundSequence( pAINeedsCover->m_hObject, kAIS_WhereShouldIGo, hWitness, kAIS_OrderCoverAim, kAIS_OrderCoverAim, kAISndCat_TargetVisible, NULL, 0.2f );
		}
	}

	m_bPlayedFailureSound = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::PlaySquadAISound
//
//	PURPOSE:	Play appropriate AISound for situation.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::PlaySquadAISound( uint32 cCovering )
{
	// Don't play any sound if only 1 AI in squad.

	if( ( m_cPotentialParticipants == 1 && !m_hHeavyArmor ) ||
		( cCovering == 0 ) )
	{
		return;
	}

	// Don't play any sound if no one can see the threat.

	HOBJECT hSeesEnemy = FindAllySeesThreat( NULL, VISIBLE_STRICT );
	if( !hSeesEnemy )
	{
		return;
	}

	// If only one AI can see the threat, play a target spotted sound.

	if( ( !m_hHeavyArmor ) &&
		( !FindAllySeesThreat( hSeesEnemy, VISIBLE_STRICT ) ) )
	{
		// Only play target spotted sound if spotting first threat.

		CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSeesEnemy );
		if( pAI && pAI->GetAIBlackBoard()->GetBBTargetFirstThreatTime() >= g_pLTServer->GetTime() - 2.f )
		{
			if( pAI->GetAIBlackBoard()->GetBBTargetPosition().DistSqr( pAI->GetPosition() ) > g_pAIDB->GetAIConstantsRecord()->fAlertImmediateThreatInstantSeeDistanceSqr )
			{
				g_pAISoundMgr->RequestAISound( hSeesEnemy, kAIS_DisturbanceSeenAlarmingFar, kAISndCat_Event, m_hTarget, 0.f );
			}
			else {
				g_pAISoundMgr->RequestAISound( hSeesEnemy, kAIS_DisturbanceSeenAlarming, kAISndCat_Event, m_hTarget, 0.f );
			}
			return;
		}
	}

	// Choose a speaker.

	HOBJECT hSpeaker;
	for( int iGetCover=0; iGetCover < MAX_AI_GET_COVER; ++iGetCover )
	{
		if( m_hCoverAI[iGetCover] )
		{
			hSpeaker = m_hCoverAI[iGetCover];
			break;
		}
	}

	// Find another AI that can see the threat.

	HOBJECT hSeeEnemy = FindAllySeesThreat( hSpeaker, VISIBLE_STRICT );


	//
	// AI getting behind HeavyArmor.
	//

	if( m_hHeavyArmor )
	{
		// "Get behind him!"

		if( hSeeEnemy )
		{
			g_pAISoundMgr->RequestAISound( hSeeEnemy, kAIS_GetBehindHim, kAISndCat_Event, m_hTarget, 0.f );
		}

		// "Get behind me!"

		else {
			g_pAISoundMgr->RequestAISound( m_hHeavyArmor, kAIS_GetBehindHim, kAISndCat_Event, m_hTarget, 0.f );
		}

		m_hHeavyArmor = NULL;
		return;
	}


	//
	// Multiple AI going for cover.
	//

	if( cCovering > 1 )
	{
		// "Suppresion fire"

		if( m_hSuppressAI )
		{
			g_pAISoundMgr->RequestAISound( m_hSuppressAI, kAIS_SuppressionFire, kAISndCat_Event, m_hTarget, 0.f );
		}

		// "Take cover!"

		else {
			g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_OrderCover, kAISndCat_Event, m_hTarget, 0.f );
		}

		return;
	}

	//
	// Single AI going for cover.
	//

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );
	if( !pAI )
	{
		return;
	}

	// Determine if AI is leaving a previous cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( pNode && ( pNode->GetType() == kNode_Cover ) )
	{
		// "Under fire!"

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), m_hTarget, kThreatPos_TargetPos, kNodeStatus_Damaged ) )
		{
			// Damage must be recent.

			if( pAI->GetDestructible()->GetLastDamageTime() > g_pLTServer->GetTime() - 0.7f )
			{
				g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_UnderFire, kAISndCat_Event, m_hTarget, 0.f );
				return;
			}
		}

		// FOV invalidated.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), m_hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatOutsideFOV ) )
		{
			// "Get out of there!"

			if( hSeeEnemy )
			{
				g_pAISoundMgr->RequestAISound( hSeeEnemy, kAIS_OrderCoverFOV, kAISndCat_Event, m_hTarget, 0.f );
			}

			// "I'm exposed!"

			else {
				g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_RequestCoverFOV, kAISndCat_Event, m_hTarget, 0.f );
			}
			return;
		}

		// ThreatRadius invalidated.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), m_hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatInsideRadius ) )
		{
			// "Get out of there!"

			if( hSeeEnemy )
			{
				g_pAISoundMgr->RequestAISound( hSeeEnemy, kAIS_OrderCoverRadius, kAISndCat_Event, m_hTarget, 0.f );
			}

			// "He's rushing me!"

			else {
				g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_RequestCoverRadius, kAISndCat_Event, m_hTarget, 0.f );
			}
			return;
		}

		// Threat aiming at AI.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), m_hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatAimingAtNode ) )
		{
			// "He's aiming right at you!"

			if( hSeeEnemy )
			{
				g_pAISoundMgr->RequestAISound( hSeeEnemy, kAIS_OrderCoverAim, kAISndCat_Event, m_hTarget, 0.f );
			}

			// "I'm in trouble!"

			else {
				g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_RequestCoverAim, kAISndCat_Event, m_hTarget, 0.f );
			}
			return;
		}
	}

	// Not in cover.

	// "Suppresion fire"

	if( m_hSuppressAI )
	{
		g_pAISoundMgr->RequestAISound( m_hSuppressAI, kAIS_SuppressionFire, kAISndCat_Event, m_hTarget, 0.f );
	}

	// "Take cover!"
	// "Where should I go?"

	else if( hSeeEnemy )
	{
		g_pAISoundMgr->RequestAISound( hSeeEnemy, kAIS_OrderCover, kAISndCat_Event, NULL, 0.f );
		g_pAISoundMgr->RequestAISoundSequence( hSpeaker, kAIS_NowhereToGo, hSeeEnemy, kAIS_OrderCover, kAIS_OrderCover, kAISndCat_Event, NULL, 0.2f );
	}

	// "I need cover!"

	else {
		g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_RequestCover, kAISndCat_Event, m_hTarget, 0.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::FindValidCoverNodeFact
//
//	PURPOSE:	Return fact for a valid node.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIActivityGetToCover::FindValidCoverNodeFact( CAI* pAI, HOBJECT hTarget, bool bCheckAmbush )
{
	// Find a valid cover node.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Cover, kNodeStatus_All, NULL, hTarget );

	// Find a valid ambush node.

	if( bCheckAmbush && !pFact )
	{
		pFact = pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Ambush, kNodeStatus_All, NULL, hTarget );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::AtCoverNode
//
//	PURPOSE:	Return node that AI is at.
//
// ----------------------------------------------------------------------- //

AINode* CAIActivityGetToCover::AtCoverNode( CAI* pAI, bool bCheckAmbush )
{
	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}

	// AI is at a cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode && 
			( ( pNode->GetType() == kNode_Cover ) ||
			  ( bCheckAmbush && pNode->GetType() == kNode_Ambush ) ) )
		{
			return pNode;
		}
	}

	// AI is not at a cover node.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::EnRouteToCoverNode
//
//	PURPOSE:	Return node AI is en route to.
//
// ----------------------------------------------------------------------- //

AINode* CAIActivityGetToCover::EnRouteToCoverNode( CAI* pAI, bool bCheckAmbush )
{
	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}

	// AI is en route to a cover node.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Cover );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		return (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	}

	// AI is en route to an ambush node.

	if( bCheckAmbush )
	{
		factQuery.SetTaskType( kTask_Ambush );
		pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			return (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		}
	}

	// Not en route to a node.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::IsCoverNodeValid
//
//	PURPOSE:	Return true if node is valid.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::IsCoverNodeValid( CAI* pAI, AINode* pNode, HOBJECT hTarget, uint32 dwValidation )
{
	// Sanity check.

	if( !( pAI && pNode ) )
	{
		return false;
	}

	switch( pNode->GetType() )
	{
		// Cover node validity.

		case kNode_Cover:
			return pNode->IsNodeValid( pAI, pAI->GetPosition(), hTarget, kThreatPos_TargetPos, dwValidation );

		// Ambush node validity.

		case kNode_Ambush:
			return pNode->IsNodeValid( pAI, pAI->GetPosition(), hTarget, kThreatPos_TargetPos, dwValidation );
	}

	// Not a valid node.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::SquadHasEverSeenThreat
//
//	PURPOSE:	Return true if any squad member has ever seen the threat.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::SquadHasEverSeenThreat()
{
	CAI* pCurAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		if( IsDeadAI( m_aPotentialParticipants[iPotential] ) )
		{
			continue;
		}

		// Someone alive has seen the target.

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( pCurAI && pCurAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() > 0.f )
		{
			return true;
		}

		// Someone at some point communicated the target's position.

		if( pCurAI && pCurAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Squad )
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::SquadHasSeenThreatRecently
//
//	PURPOSE:	Return true if any squad member has seen the threat recently.
//
// ----------------------------------------------------------------------- //

bool CAIActivityGetToCover::SquadHasSeenThreatRecently()
{
	CAI* pCurAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		if( IsDeadAI( m_aPotentialParticipants[iPotential] ) )
		{
			continue;
		}

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( pCurAI && pCurAI->GetAIBlackBoard()->GetBBTargetVisibilityConfidence() > 0.5f )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::FindAllySeesThreat
//
//	PURPOSE:	Find another AI that can see the threat.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIActivityGetToCover::FindAllySeesThreat( HOBJECT hExclude, bool bVisibleStrict )
{
	// Find another AI that can see the threat.

	CAI* pCurAI;
	HOBJECT hSeeEnemy = NULL;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		if( m_aPotentialParticipants[iPotential] == hExclude )
		{
			continue;
		}

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Strictly visible.

		if( bVisibleStrict && pCurAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
		{
			hSeeEnemy = pCurAI->m_hObject;
			break;
		}

		// More leniently visible.

		else if( pCurAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() >= g_pLTServer->GetTime() - 0.5f )
		{
			hSeeEnemy = pCurAI->m_hObject;
			break;
		}
	}

	return hSeeEnemy;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::ClearCoverTasks
//
//	PURPOSE:	Clear any cover tasks from memory.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::ClearCoverTasks( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear cover tasks.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Cover );
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Clear ambush tasks.

	factQuery.SetTaskType( kTask_Ambush );
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::BlockNode
//
//	PURPOSE:	Block a node from future searches.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::BlockNode( HOBJECT hAI, AINode* pNode )
{
	// Sanity check.

	if( !( hAI && pNode ) )
	{
		return;
	}

	// Lock node so others will not claim it.

	pNode->LockNode( hAI );
	m_aBlockedNodes[m_cBlockedNodes] = pNode;
	++m_cBlockedNodes;

	// Lock any dependency this node has.

	if( pNode->GetType() == kNode_Cover )
	{
		AINodeCover* pNodeCover = (AINodeCover*)pNode;
		if( pNodeCover->GetDependency() )
		{
			AINode* pDependency = (AINode*)g_pLTServer->HandleToObject( pNodeCover->GetDependency() );
			if( pDependency && !pDependency->IsNodeLocked() )
			{
				pDependency->LockNode( hAI );
				m_aBlockedNodes[m_cBlockedNodes] = pDependency;
				++m_cBlockedNodes;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityGetToCover::UnblockNodes
//
//	PURPOSE:	Unblock a nodes that were blocked from searches.
//
// ----------------------------------------------------------------------- //

void CAIActivityGetToCover::UnblockNodes()
{
	// Unlock any nodes blocked during searches.

	AINode* pNode;
	for( uint32 iBlocked=0; iBlocked < m_cBlockedNodes; ++iBlocked )
	{
		pNode = m_aBlockedNodes[iBlocked];
		if( pNode )
		{
			pNode->UnlockNode( pNode->GetLockingAI() );
		}
	}

	m_cBlockedNodes = 0;
}
