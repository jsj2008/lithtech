// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGetOutOfTheWay.cpp
//
// PURPOSE : Contains the implementation of the "get out of the way" action.
//
// CREATED : 10/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGetOutOfTheWay.h"
#include "AIStateGoto.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGetOutOfTheWay, kAct_GetOutOfTheWay );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGetOutOfTheWay::CAIActionGetOutOfTheWay()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGetOutOfTheWay::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Position is valid

	m_wsWorldStateEffects.SetWSProp( kWSK_PositionIsValid, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGetOutOfTheWay::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Return false if the baseclass fails.

	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Only check if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Bail if AI is at a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		return false;
	}

	// Bail if no desire exists.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_GetOutOfTheWay );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Bail if no target character exists.

	HOBJECT hChar = pFact->GetTargetObject();
	if( !IsCharacter( hChar ) )
	{
		return false;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	LTVector vCharPos;
	g_pLTServer->GetObjectPos( hChar, &vCharPos );

	// Do not check for a straight path if we're standing in the same spot.

	bool bSuccess = false;
	float fRadius = pSmartObjectRecord->fMinDist;
	if( pAI->GetPosition() != vCharPos )
	{
		// Calculate the direction from the target to myself.

		LTVector vDir = pAI->GetPosition() - vCharPos;
		vDir.y = 0.f;
		vDir.Normalize();

		// Find a clear path in that direction.

		LTVector vDest = pAI->GetPosition() + ( vDir * fRadius );
		if( g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
		{
			bSuccess = true;
		}
	}

	// If a clear path does not exists, bail if there's no escape path.

	if( !( bSuccess || g_pAIPathMgrNavMesh->EscapePathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vCharPos, fRadius, pAI->GetLastNavMeshPoly(), NULL ) ) )
	{
		return false;
	}

	// AI can get out of the way.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGetOutOfTheWay::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if no desire exists.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_GetOutOfTheWay );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Bail if no target character exists.

	HOBJECT hChar = pFact->GetTargetObject();
	if( !IsCharacter( hChar ) )
	{
		return;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	LTVector vCharPos;
	g_pLTServer->GetObjectPos( hChar, &vCharPos );

	// Do not check for a straight path if we're standing in the same spot.

	LTVector vDest;
	bool bSuccess = false;
	float fRadius = pSmartObjectRecord->fMinDist;
	if( pAI->GetPosition() != vCharPos )
	{
		// Calculate the direction from the target to myself.

		LTVector vDir = pAI->GetPosition() - vCharPos;
		vDir.y = 0.f;
		vDir.Normalize();

		// Find a clear path in that direction.

		vDest = pAI->GetPosition() + ( vDir * fRadius );
		if( g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
		{
			bSuccess = true;
		}
	}

	// If a clear path does not exists, bail if there's no escape path.

	if( !( bSuccess || g_pAIPathMgrNavMesh->EscapePathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vCharPos, fRadius, pAI->GetLastNavMeshPoly(), &vDest ) ) )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );

	// Set the destination.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
	pGoto->SetDest( vDest );

	// Step aside facing the Player.

	pGoto->SetActivityProp( kAP_ATVT_StepAside );
	pGoto->SetDirectionProp( kAP_MDIR_Backward );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGetOutOfTheWay::IsActionComplete( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return true;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return true;
	}

	// If the desire no longer exists, we're done.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_GetOutOfTheWay );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return true;
	}

	// Bail if the character has strayed a long way from us.

	LTVector vCharPos;
	float fRadius = pSmartObjectRecord->fMaxDist;
	HOBJECT hChar = pFact->GetTargetObject();
	g_pLTServer->GetObjectPos( hChar, &vCharPos );

	if( vCharPos.DistSqr( pAI->GetPosition() ) > fRadius * fRadius )
	{
		return true;
	}

	// Goto is complete if state is complete.

	if( !pAI->GetState() )
	{
		AIASSERT(0, pAI->GetHOBJECT(), "CAIActionGetOutOfTheWay::IsActionComplete : AI has no state.");
		return false;
	}

	// Goto is complete.

	if ( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGetOutOfTheWay::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionGetOutOfTheWay::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// If the desire no longer exists, we're done.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_GetOutOfTheWay );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Face the player.

	LTVector vCharPos;
	HOBJECT hChar = pFact->GetTargetObject();
	g_pLTServer->GetObjectPos( hChar, &vCharPos );
	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hChar );

	// Consider a Player too close if the space between us is under 25% 
	// of the sum of our radii.

	float fMinDist = pAI->GetRadius() + pChar->GetRadius();
	fMinDist += fMinDist * 0.25f;

	// Bail if Player is still too close.

	if( vCharPos.DistSqr( pAI->GetPosition() ) < fMinDist * fMinDist )
	{
		return;
	}

	// Face the Player who bumped into me, to acknowledge their presence.

	pAI->GetAIBlackBoard()->SetBBFaceObject( pFact->GetTargetObject() );

	// Clear the desire to get out of the way.

	pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

