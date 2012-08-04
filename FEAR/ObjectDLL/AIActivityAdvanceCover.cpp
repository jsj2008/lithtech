// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityAdvanceCover.cpp
//
// PURPOSE : AIActivityAdvanceCover class implementation
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivityAdvanceCover.h"
#include "AI.h"
#include "AINode.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include "AIWorkingMemoryCentral.h"
#include "AISquad.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityAdvanceCover, kActivity_AdvanceCover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAdvanceCover::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivityAdvanceCover::CAIActivityAdvanceCover()
{
	m_nActivityPriority = 1;
}

CAIActivityAdvanceCover::~CAIActivityAdvanceCover()
{
}

void CAIActivityAdvanceCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIActivityAdvanceCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAdvanceCover::FindActivityParticipants
//
//	PURPOSE:	Return true if successfully found participants.
//
// ----------------------------------------------------------------------- //

bool CAIActivityAdvanceCover::FindActivityParticipants()
{
	// Intentionally do NOT call super::FindActivityParticipants().
	// AI advance for different reasons than GetToCover.

	CAI* pCurAI;
	AINode* pNode;

	//
	// Look for someone in need of advancing cover.
	//

	m_hTarget = NULL;
	uint32 iPotential;
	HOBJECT hNeedsCover = NULL;
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

		// Skip if AI is not in cover.

		pNode = AtCoverNode( pCurAI, CHECK_AMBUSH );
		if( !pNode )
		{
			continue;
		}

		// Skip AI that have a scripted Task (cover or ambush).

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetFactFlags( kFactFlag_Scripted );
		if( pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			continue;
		}

		// Skip if AI already has valid cover with respect to the BoundaryRadius.

		if( pNode->IsNodeValid( pCurAI, pCurAI->GetPosition(), hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatOutsideBoundary ) )
		{
			continue;
		}

		// Skip if AI has no valid cover to go to.

		hNeedsCover = pCurAI->m_hObject;
		if( !FindValidCoverNodeFact( pCurAI, hTarget, !CHECK_AMBUSH ) )
		{
			continue;
		}

		// Found an AI who needs to advance cover.

		m_hCoverAI[0] = pCurAI->m_hObject;
		m_hTarget = hTarget;
		break;
	}

	// Bail if no AI found that needs to advance cover and has available cover.

	if( !m_hTarget )
	{
		PlaySquadFailureAISound( NULL, hNeedsCover );
		return false;
	}


	//
	// Look for someone to lay suppression fire.
	// This must be someone who is already in cover.
	//

	CAIWMFact* pTimeoutFact;
	CAIWMFact factTimeoutQuery;
	factTimeoutQuery.SetFactType( kFact_Knowledge );
	factTimeoutQuery.SetKnowledgeType( kKnowledge_NextSuppressTime );

	for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// Skip AI who are not targeting the same enemy.

		if( !pCurAI->HasTarget( kTarget_Character | kTarget_Object ) )
		{
			continue;
		}
		HOBJECT hTarget = pCurAI->GetAIBlackBoard()->GetBBTargetObject();
		if( hTarget != m_hTarget )
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

		// Stop searching if we've found an AI that can
		// lay suppression fire from cover.

		pNode = AtCoverNode( pCurAI, !CHECK_AMBUSH );
		if( pNode )
		{
			// Lay suppression from a valid cover node.

			if( IsCoverNodeValid( pCurAI, pNode, m_hTarget, VALID_STRICT ) )
			{
				m_hSuppressAI = pCurAI->m_hObject;
				break;
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
	// Find all AI who have available valid cover to advance to.
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

		// Ignore AI that are already at valid cover nodes with respect to BoundaryRadius.

		pNode = AtCoverNode( pCurAI, CHECK_AMBUSH );
		if( pNode && 
			( pNode->IsNodeValid( pCurAI, pCurAI->GetPosition(), m_hTarget, kThreatPos_TargetPos, kNodeStatus_ThreatOutsideBoundary ) ) )
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

		else if( FindValidCoverNodeFact( pCurAI, m_hTarget, CHECK_AMBUSH ) )
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

AITRACE( AIShowGoals, ( m_hCoverAI[0], "ADVANCE COVER VALID" ) );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAdvanceCover::PlaySquadFailureAISound
//
//	PURPOSE:	Play appropriate AISound for situation.
//
// ----------------------------------------------------------------------- //

void CAIActivityAdvanceCover::PlaySquadFailureAISound( HOBJECT hHasCover, HOBJECT hNeedsCover )
{
	// Sanity check.

	if( !hNeedsCover )
	{
		return;
	}

	// No one to talk to.

	if( m_cPotentialParticipants == 1 )
	{
		return;
	}

	// Bail if no ally to give orders.

	HOBJECT hAlly = g_pAICoordinator->FindAlly( hNeedsCover, NULL );
	if( !hAlly )
	{
		return;
	}

	// Only verbally refuse to advance if someone has seen 
	// a squad member die.

	CAI* pAlly = (CAI*)g_pLTServer->HandleToObject( hAlly );
	if( pAlly->GetAIBlackBoard()->GetBBBodyCount() == 0 )
	{
		return;
	}

	// Small chance of playing this.

	if( GetRandom( 0.f, 1.f ) > 0.5f )
	{
		// Time stamp sounds so AI does not try to play them again too soon.

		g_pAISoundMgr->SkipAISound( hAlly, kAIS_OrderAdvance );
		g_pAISoundMgr->SkipAISound( hNeedsCover, kAIS_NegativeStrong );
		return;
	}

	// "Move up!"
	// "No fucking way!"

	g_pAISoundMgr->RequestAISound( hAlly, kAIS_OrderAdvance, kAISndCat_Event, NULL, 0.f );
	g_pAISoundMgr->RequestAISoundSequence( hNeedsCover, kAIS_NegativeStrong, hAlly, kAIS_OrderAdvance, kAIS_OrderAdvance, kAISndCat_Event, NULL, 0.5f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAdvanceCover::PlaySquadAISound
//
//	PURPOSE:	Play appropriate AISound for situation.
//
// ----------------------------------------------------------------------- //

void CAIActivityAdvanceCover::PlaySquadAISound( uint32 cCovering )
{
	// Don't play any sound if only 1 AI in squad.

	if( ( m_cPotentialParticipants == 1 ) ||
		( cCovering == 0 ) )
	{
		return;
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

	//
	// Multiple AI advancing cover.
	//

	if( cCovering > 1 )
	{
		// "Move up!"

		if( m_hSuppressAI )
		{
			g_pAISoundMgr->RequestAISound( m_hSuppressAI, kAIS_OrderAdvance, kAISndCat_Event, m_hTarget, 0.f );
		}

		// "Let's go!"

		else {
			g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_RequestAdvance, kAISndCat_Event, m_hTarget, 0.f );
		}

		return;
	}

	//
	// Single AI advancing cover.
	//

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );
	if( !pAI )
	{
		return;
	}

	// "Move up!"

	if( m_hSuppressAI )
	{
		g_pAISoundMgr->RequestAISound( m_hSuppressAI, kAIS_OrderAdvance, kAISndCat_Event, m_hTarget, 0.f );
	}

	// "Advancing"

	else {
		g_pAISoundMgr->RequestAISound( hSpeaker, kAIS_Advancing, kAISndCat_Event, m_hTarget, 0.f );
	}
}

