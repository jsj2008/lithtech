// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSuppressionFireFromCover.cpp
//
// PURPOSE : AIActionSuppressionFireFromCover class implementation
//
// CREATED : 01/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionSuppressionFireFromCover.h"
#include "AIActionSuppressionFire.h"
#include "AI.h"
#include "AINode.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "NodeTrackerContext.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSuppressionFireFromCover, kAct_SuppressionFireFromCover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionSuppressionFireFromCover::CAIActionSuppressionFireFromCover()
{
	// Suppression AI sounds are handled by AIActivities.

	m_bPlayAISound = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFireFromCover::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Remove super classes' precondition.
	// Weapon does not need to be loaded -- AI will 
	// automatically reload while suppressing.

	m_wsWorldStatePreconditions.ClearWSProp( kWSK_WeaponLoaded, NULL );

	// Set effects.
	// Target is suppressed.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsSuppressed, NULL, kWST_bool, true );

	// Remove super classes' effect.

	m_wsWorldStateEffects.ClearWSProp( kWSK_TargetIsDead, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionSuppressionFireFromCover::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// No Target.

	if( !pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFireFromCover::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Automatically reload behind the scenes while suppressing.
	// This prevents the reloading animation from interfering with 
	// timing of coordinated events.

	pAI->GetAIBlackBoard()->SetBBAutoReload( true );
	pAI->GetAIBlackBoard()->SetBBSuppressionFire( true );

	// Keep track of who's suppressing who.

	if( pAI->HasTarget( kTarget_Character ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if (pChar)
		{
			CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);
			if (pFact)
			{
				pFact->SetTargetObject( pChar->GetHOBJECT(), 1.f );
				pFact->SetSourceObject( pAI->GetHOBJECT(), 1.f );
				pFact->SetKnowledgeType( kKnowledge_Suppressing, 1.f );
			}
		}
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
	pAI->GetAIBlackBoard()->SetBBFaceTargetKnownPos( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFireFromCover::DeactivateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Turn off automatic reloading.

	pAI->GetAIBlackBoard()->SetBBAutoReload( false );
	pAI->GetAIBlackBoard()->SetBBSuppressionFire( false );
	pAI->GetAIBlackBoard()->SetBBFaceTargetKnownPos( false );

	// Remove knowledge of suppressor.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Knowledge);
	factQuery.SetSourceObject(pAI->GetHOBJECT());
	g_pAIWorkingMemoryCentral->ClearWMFact(factQuery);

	// Do not suppress again for some time.

	CAIWMFact factTimeoutQuery;
	factTimeoutQuery.SetFactType( kFact_Knowledge );
	factTimeoutQuery.SetKnowledgeType( kKnowledge_NextSuppressTime );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
	if( !pFact )
	{
		pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
	}
	if( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_NextSuppressTime, 1.f );
		pFact->SetTime( g_pLTServer->GetTime() + SUPPRESSION_TIMEOUT, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFireFromCover::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionSuppressionFireFromCover::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateContextPreconditions().
	// Target does not need to be visible or in range.
	// AI will automatically reload while suppressing.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	// Action is valid if the node FOV is valid in relation to the threat.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
			{
				return false;
			}
		}
	}

	return true;
}

