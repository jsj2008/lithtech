// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeUncloaked.cpp
//
// PURPOSE : AIActionAttackLungeUncloaked class implementation
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackLungeUncloaked.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIPathMgrNavMesh.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeUncloaked, kAct_AttackLungeUncloaked );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackLungeUncloaked::CAIActionAttackLungeUncloaked()
{
	SetValidateVisibility(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackLungeUncloaked::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// AI doesn't have a target.

	if (!pAI->HasTarget( kTarget_Character ))
	{
		return false;
	}

	// Target is not visible.

	if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// AI does not have a weapon of the correct type

	if (!AIWeaponUtils::HasWeaponType(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// AI does not have any ammo for this weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// Someone else is lunging.  Only one AI may lunge at a time.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Lunging );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact )
	{
		// Clear records of dead AI.

		if( IsDeadAI( pFact->GetSourceObject() ) )
		{
			g_pAIWorkingMemoryCentral->ClearWMFacts( factQuery );
		}
		else return false;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}


	// Someone has lunged too recently.

	if( pSmartObjectRecord->fTimeout > 0.f )
	{
		factQuery.SetFactType( kFact_Knowledge );
		factQuery.SetKnowledgeType( kKnowledge_NextLungeTime );
		pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
		if( pFact && ( pFact->GetTime() > g_pLTServer->GetTime() ) )
		{
			return false;
		}
	}

	// Bail if the AI does not have the desire to lunge.
	// The desire indicates the max range of the lunge.

	CAIWMFact factDesireQuery;
	factDesireQuery.SetFactType( kFact_Desire );
	factDesireQuery.SetDesireType( kDesire_Lunge );
	CAIWMFact* pDesireFact = pAI->GetAIWorkingMemory()->FindWMFact( factDesireQuery );
	if( !pDesireFact )
	{
		return false;
	}

	// Target must be in range.

	LTVector vTarget = pAI->GetAIBlackBoard()->GetBBTargetPosition();
	float fDistSqr = vTarget.DistSqr( pAI->GetPosition() );

	// Target too close.

	if( fDistSqr < pSmartObjectRecord->fMinDist * pSmartObjectRecord->fMinDist )
	{
		return false;
	}

	// Target too far.

	float fMaxDist = GetLungeMaxDist( pAI, pDesireFact );
	if( fDistSqr > fMaxDist * fMaxDist )
	{
		return false;
	}

	// No straight path to the target.

	LTVector vDir = vTarget - pAI->GetPosition();
	vDir.Normalize();
	LTVector vDest = pAI->GetPosition() + ( vDir * ( fMaxDist + 50.f ) );
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Lunge!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::GetLungeMaxDist
//
//	PURPOSE:	Return the max distance AI can lunge.
//
// ----------------------------------------------------------------------- //

float CAIActionAttackLungeUncloaked::GetLungeMaxDist( CAI* pAI, CAIWMFact* pDesireFact )
{
	if( !( pAI && pDesireFact ) )
	{
		return 0.f;
	}

	return pDesireFact->GetRadius();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeUncloaked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Record who is lunging on the blackboard.
	// Only one AI may lunge at a time.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if (pFact)
	{
		pFact->SetKnowledgeType( kKnowledge_Lunging, 1.f );
		pFact->SetSourceObject( pAI->m_hObject, 1.f );
	}

	// Depart from a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			pNode->HandleAIDeparture( pAI );
		}
	}

	// Turn on touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_Damage );

	// Do not move thru the target.

	pAI->GetAIMovement()->AllowTargetPenetration( false );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );

	// Turn towards target, but don't track.

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	pAI->GetAIBlackBoard()->SetBBFaceObject( hTarget );

	// This Actions does not Uncloak the AI.
	// It is assumed that the AI will uncloak thru a
	// DESIRE modelstring in the lunge animation.
	// This allows the AI to uncloak in mid-lunge.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeUncloaked::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Intentionally do NOT call super::SetAttackAnimProps.
	// Get animation props from SmartObject.

	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( pSmartObjectRecord )
	{
		*pProps = pSmartObjectRecord->Props;
		pProps->Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeUncloaked::DeactivateAction( CAI* pAI )
{
	// Remove knowledge of lunging AI.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Lunging );
	factQuery.SetSourceObject( pAI->m_hObject );
	g_pAIWorkingMemoryCentral->ClearWMFact( factQuery );

	// Do not allow anyone to lunge again too soon.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( pSmartObjectRecord && pSmartObjectRecord->fTimeout )
	{
		CAIWMFact factQueryNext;
		factQueryNext.SetFactType( kFact_Knowledge );
		factQueryNext.SetKnowledgeType( kKnowledge_NextLungeTime );
		CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQueryNext);
		if( !pFact )
		{
			pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
			pFact->SetKnowledgeType( kKnowledge_NextLungeTime, 1.f );
		}
		if( pFact )
		{
			pFact->SetTime( g_pLTServer->GetTime() + pSmartObjectRecord->fTimeout );	
		}
	}

	// Turn off touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_None );

	// Do not move thru the target.

	pAI->GetAIMovement()->AllowTargetPenetration( true );

	// Remove the desire to uncloak.

	CAIWMFact factUncloakQuery;
	factUncloakQuery.SetFactType(kFact_Desire);
	factUncloakQuery.SetDesireType(kDesire_Uncloak);
	pAI->GetAIWorkingMemory()->ClearWMFacts( factUncloakQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackLungeUncloaked::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateAction(). 
	// Action is always valid until completion.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	return true;
}

