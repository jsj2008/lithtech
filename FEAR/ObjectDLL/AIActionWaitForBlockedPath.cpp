// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionWaitForBlockedPath.cpp
//
// PURPOSE : 
//
// CREATED : 10/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionWaitForBlockedPath.h"
#include "AIStateAnimate.h"
#include "AIMovementUtils.h"
#include "AIWorkingMemoryCentral.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionWaitForBlockedPath, kAct_WaitForBlockedPath );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionWaitForBlockedPath::CAIActionWaitForBlockedPath()
{
}

CAIActionWaitForBlockedPath::~CAIActionWaitForBlockedPath()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForBlockedPath::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Position is valid

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_BlockedPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid..
//
// ----------------------------------------------------------------------- //

bool CAIActionWaitForBlockedPath::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if their is not a blocking AI

	CAIWMFact queryBlockedPathFact;
	queryBlockedPathFact.SetFactType( kFact_Knowledge );
	queryBlockedPathFact.SetKnowledgeType( kKnowledge_BlockedPath );
	CAIWMFact* pBlockedPathFact = pAI->GetAIWorkingMemory()->FindWMFact( queryBlockedPathFact );
	if ( !pBlockedPathFact )
	{
		return false;
	}

	// Fail if the blocking character is NULL.

	HOBJECT hBlockingCharacter = pBlockedPathFact->GetTargetObject();
	if ( NULL == hBlockingCharacter )
	{
		return false;
	}

	// Fail if the blocking character is already waiting on someone

	CAIWMFact queryWaitFirBlockedPathFact;
	queryWaitFirBlockedPathFact.SetFactType( kFact_Knowledge );
	queryWaitFirBlockedPathFact.SetKnowledgeType( kKnowledge_WaitForBlockedPath );
	queryWaitFirBlockedPathFact.SetSourceObject( pBlockedPathFact->GetTargetObject() );
	if ( NULL != g_pAIWorkingMemoryCentral->FindWMFact( queryWaitFirBlockedPathFact ) )
	{
		return false;
	}

	// Fail if the blocking character is a player (as we can't assume players 
	// will move out of our way soon).

	if ( IsPlayer(hBlockingCharacter) )
	{
		return false;
	}

	// Fail if the blocker is not an AI.

	CAI* pBlockingAI = CAI::DynamicCast( hBlockingCharacter );
	if ( !pBlockingAI )
	{
		return false;
	}

	// Fail if the AI is a hostile and we have a weapon (we should just attack instead)

	EnumCharacterStance eStance = g_pCharacterDB->GetStance( 
		pAI->GetAlignment(), pBlockingAI->GetAlignment() );
	if ( kCharStance_Hate == eStance )
	{
		return false;
	}

	// Fail if the AI is a traitor.

	CAIWMFact queryTraitorFact;
	queryTraitorFact.SetFactType( kFact_Character );
	queryTraitorFact.SetTargetObject( hBlockingCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryTraitorFact );
	if ( pFact && ( 0 != ( pFact->GetFactFlags() & kFactFlag_CharacterIsTraitor ) ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForBlockedPath::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set reload animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, LOOP );

	// Register a fact in central working memory to allow other AIs to 
	// determine we are waiting for a path.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if ( pFact )
	{
		pFact->SetFactType( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_WaitForBlockedPath );
		pFact->SetSourceObject( pAI->GetHOBJECT() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForBlockedPath::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	if ( IsActionComplete( pAI ) )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BlockedPath );
		pAI->GetAIWorkingMemory()->ClearWMFacts( queryFact );

		pAI->GetAIBlackBoard()->SetBBMovementCollisionFlags( 
			(~kAIMovementFlag_BlockedPath) & pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() );
	}

	// Clear the fact in central working memory so that other AIs know we
	// are no longer waiting.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_WaitForBlockedPath );
	queryFact.SetSourceObject( pAI->GetHOBJECT() );
	g_pAIWorkingMemoryCentral->ClearWMFact( queryFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForBlockedPath::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionWaitForBlockedPath::IsActionComplete( CAI* pAI )
{
	// Action is complete when the blocking character has moved at least 25 
	// units in X/Z or is dead.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BlockedPath );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact )
	{
		return true;
	}

	if ( IsDeadAI( pFact->GetTargetObject() ) )
	{
		return true;
	}

	HOBJECT hBlocker = pFact->GetTargetObject();

	LTVector vCharacterPos, vCharacterDims;
	g_pLTServer->GetObjectPos( hBlocker, &vCharacterPos );
	g_pPhysicsLT->GetObjectDims( hBlocker, &vCharacterDims );
	float flCharacterRadiusSqr = 50.f;
	flCharacterRadiusSqr *= flCharacterRadiusSqr;

	LTVector vTesterPos, vTesterDims;
	g_pLTServer->GetObjectPos( pAI->GetHOBJECT(), &vTesterPos );
	g_pPhysicsLT->GetObjectDims( pAI->GetHOBJECT(), &vTesterDims );
	float flTesterRadiusSqr = pAI->GetRadius();
	flTesterRadiusSqr *= flTesterRadiusSqr;

	LTVector vMovement = pFact->GetPos() - pAI->GetPosition();
	vMovement.SetMagnitude( 50.0f );

	if ( !AIMovementUtils::Collides( 
		vCharacterPos, vCharacterDims, flCharacterRadiusSqr,
		vTesterPos + vMovement, vTesterDims, flTesterRadiusSqr,
		AIMovementUtils::GetRadiusBuffer( hBlocker ) ) )
	{
		return true;
	}

	// Reacting is not complete.

	return false;
}
