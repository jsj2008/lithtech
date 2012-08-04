// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSuppressionFire.cpp
//
// PURPOSE : AIActionSuppressionFire class implementation
//
// CREATED : 5/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionSuppressionFire.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSuppressionFire, kAct_SuppressionFire );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFire::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionSuppressionFire::CAIActionSuppressionFire()
{
	// Suppression AI sounds are handled by AIActivities.

	m_bPlayAISound = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFire::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFire::InitAction( AIDB_ActionRecord* pActionRecord )
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
//	ROUTINE:	CAIActionSuppressionFire::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionSuppressionFire::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Intentionally do not call super::ValidateContextPreconditions().
	// Target does not need to be in range.

	if( !pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFire::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFire::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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
//	ROUTINE:	CAIActionSuppressionFire::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFire::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	pProps->Set( kAPG_Posture, kAP_POS_Crouch );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFire::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSuppressionFire::DeactivateAction( CAI* pAI )
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
	factQuery.SetKnowledgeType( kKnowledge_Suppressing );
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
//	ROUTINE:	CAIActionSuppressionFire::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionSuppressionFire::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateAction().
	// Target does not need to be visible or in range.
	// AI will automatically reload while suppressing.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	return true;
}

