// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCombatOpportunity.cpp
//
// PURPOSE : 
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCombatOpportunity.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AICombatOpportunity.h"

LINKFROM_MODULE(AITargetSelectCombatOpportunity);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCombatOpportunity, kTargetSelect_CombatOpportunity );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCombatOpportunity::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectCombatOpportunity::CAITargetSelectCombatOpportunity()
{
}

CAITargetSelectCombatOpportunity::~CAITargetSelectCombatOpportunity()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCombatOpportunity::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCombatOpportunity::ValidatePreconditions( CAI* pAI )
{
	if( !super::ValidatePreconditions( pAI ) )
	{
		return false;
	}

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Determine the object the AI is dealing with.

	HOBJECT hCombatOpportunityTarget = NULL;
	switch(pAI->GetAIBlackBoard()->GetBBTargetType())
	{
	case kTarget_Character:
	case kTarget_Disturbance:
		{
			hCombatOpportunityTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		}
		break;

	case kTarget_CombatOpportunity:
		hCombatOpportunityTarget = pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget();
		break;
	}

	// Nothing to use the CombatOpportunity against.

	if (!hCombatOpportunityTarget)
	{
		return false;
	}

	// Check to see if the AI has a combat opportunity which targets the current
	// enemy.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_UsableCombatOpportunity );
	factQuery.SetTargetObject( hCombatOpportunityTarget );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);

	// Failed to find a CombatOpportunity our sensor determined is usable.

	if (!pFact)
	{
		return false;
	}

	// Skip AI who have targeted a combat opportunity too recently.

	if( g_pAIDB->GetAIConstantsRecord()->fCombatOpportunityFrequency > 0.f )
	{
		CAIWMFact factTimeoutQuery;
		factTimeoutQuery.SetFactType( kFact_Knowledge );
		factTimeoutQuery.SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime );
		CAIWMFact* pTimeoutFact = pAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
		if( pTimeoutFact && ( pTimeoutFact->GetTime() > g_pLTServer->GetTime() ) )
		{
			return false;
		}
	}

	// Combat opportunity object does not exist.

	AICombatOpportunity* pCombatOpportunity = AICombatOpportunity::HandleToObject(pFact->GetSourceObject());
	if (!pCombatOpportunity)
	{
		return false;
	}

	// Do not allow anyone to shoot at the combat opportunity until ally
	// finishes announcing it.  (e.g. "Shoot the power box!")

	if( IsAI( pCombatOpportunity->GetAllySpeaker() ) )
	{
		CAI* pAlly = (CAI*)g_pLTServer->HandleToObject( pCombatOpportunity->GetAllySpeaker() );
		if( pAlly && pAlly->IsPlayingDialogSound() )
		{
			return false;
		}
	}

	// Combat opportunity is locked by someone else.

	if( pCombatOpportunity->IsCombatOpportunityLocked() &&
		( pCombatOpportunity->GetLockingAI() != pAI->GetHOBJECT() ) )
	{
		return false;
	}

	// Combat opportunity's target is dead or destroyed.

	if (!pCombatOpportunity->GetRangedTargetObject())
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCombatOpportunity::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	SetTargetCombatOpportunity( pAI );

	// Do not target a combat opportunity again for some time.

	if( g_pAIDB->GetAIConstantsRecord()->fCombatOpportunityFrequency > 0.f )
	{
		CAIWMFact factTimeoutQuery;
		factTimeoutQuery.SetFactType( kFact_Knowledge );
		factTimeoutQuery.SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime );
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
		if( !pFact )
		{
			pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		}
		if( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime, 1.f );
			pFact->SetTime( g_pLTServer->GetTime() + g_pAIDB->GetAIConstantsRecord()->fCombatOpportunityFrequency, 1.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCombatOpportunity::Deactivate
//
//	PURPOSE:	Deactivate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCombatOpportunity::Deactivate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Find the combat opportunity to unlock.

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_CombatOpportunity);
	queryFact.SetTargetObject(pAI->GetAIBlackBoard()->GetBBTargetObject());

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);

	// Failed to find a fact about this combat opportunity.

	if (!pFact)
	{
		return;
	}

	// Source Object is not an instance of AICombatOpportunity.

	if (!IsKindOf(pFact->GetSourceObject(), "AICombatOpportunity"))
	{
		return;
	}

	// Failed to convert the object to a AICombatOpportunity

	AICombatOpportunity* pCombatOpportunity = AICombatOpportunity::HandleToObject(pFact->GetSourceObject());
	if (!pCombatOpportunity)
	{
		return;
	}

	// Clear any speaker associated with this combat opportunity.

	pCombatOpportunity->SetAllySpeaker( NULL );

	// Unlock the CombatOpportunity.

	pCombatOpportunity->UnlockCombatOpportunity( pAI->GetHOBJECT() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCombatOpportunity::SetTargetCombatOpportunity
//
//	PURPOSE:	Set a TargetOpportunity as the current target.  This allows
//				AIs to shoot at non-character objects.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCombatOpportunity::SetTargetCombatOpportunity( CAI* pAI )
{
	// Determine the object the AI is dealing with.

	HOBJECT hCombatOpportunityTarget = NULL;
	switch(pAI->GetAIBlackBoard()->GetBBTargetType())
	{
	case kTarget_Character:
	case kTarget_Disturbance:
		{
			hCombatOpportunityTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		}
		break;

	case kTarget_CombatOpportunity:
		hCombatOpportunityTarget = pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget();
		break;
	}

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_UsableCombatOpportunity );
	factQuery.SetTargetObject( hCombatOpportunityTarget );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);

	if (!pFact)
	{
		return;
	}

	AICombatOpportunity* pCombatOpportunity = AICombatOpportunity::HandleToObject(pFact->GetSourceObject());
	if (!pCombatOpportunity)
	{
		return;
	}

	if (!pCombatOpportunity->GetRangedTargetObject())
	{
		return;
	}

	// Lock the CombatOpportunity.

	pCombatOpportunity->LockCombatOpportunity( pAI->GetHOBJECT() );

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_CombatOpportunity );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( pCombatOpportunity->GetRangedTargetObject() );

	// Update the target position.  Tracks the actual last position
	// of the target.

	LTVector vTargetPos;
	g_pLTServer->GetObjectPos( pCombatOpportunity->GetRangedTargetObject(), &vTargetPos );
	pAI->GetAIBlackBoard()->SetBBTargetPosition( vTargetPos );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCombatOpportunity::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCombatOpportunity::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fact is timed out - it was evaluated recently, and failed.

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_CombatOpportunity);
	queryFact.SetTargetObject(pAI->GetAIBlackBoard()->GetBBTargetObject());

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);

	// Failed to find a fact about this combat opportunity.

	if (!pFact)
	{
		return false;
	}

	// Timed out.

	if (g_pLTServer->GetTime() < pFact->GetTime())
	{
		return false;
	}

	// Failsafes to prevent AI from targeting something he can't hit for too long.
	// These should only affect an AI firing at the combat op, and should have no effect
	// on AI going to a view node, or a SmartObject node to exploit a combat op.

	// Bail if the target is obscured.

	if( pAI->GetTarget()->GetVisionBlocker() )
	{
		return false;
	}

	// Bail if we have been firing for a while.

	if( pAI->GetAIBlackBoard()->GetBBShotsFiredAtTarget() > 3 )
	{
		return false;
	}


	// Combat Opportunity does not exist.

	if (!pFact->GetSourceObject())
	{
		return false;
	}

	// Source Object is not an instead of AICombatOpportunity
	if (!IsKindOf(pFact->GetSourceObject(), "AICombatOpportunity"))
	{
		return false;
	}
	
	// The object the AI intends to kill by using this CombatOpportunity 
	// no longer exists.

	if( IsDeadCharacter( pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget() ) )
	{
		return false;
	}

	// Failed to convert the object to a AICombatOpportunity

	AICombatOpportunity* pCombatOpportunity = AICombatOpportunity::HandleToObject(pFact->GetSourceObject());
	if (!pCombatOpportunity)
	{
		return false;
	}

	// AICombatOpportunity is not valid.

	if ( !pCombatOpportunity->IsValid(pAI->GetHOBJECT(), pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget(), 
		AICombatOpportunity::kStatusFlag_ThreatPosition | AICombatOpportunity::kStatusFlag_QueryObjectInEnemyArea) )
	{
		return false;
	}

	// Fail if the position is no longer shootable (ie it was when targeted,
	// but is no longer).

	LTVector vTargetOrigin;
	if ( LT_OK != g_pLTServer->GetObjectPos(pFact->GetTargetObject(), &vTargetOrigin))
	{
		return false;
	}

	if ( !AIUtil_PositionShootable(pAI, vTargetOrigin) )
	{
		return false;
	}

	return true;
}
