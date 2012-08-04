// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToShove.cpp
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReactToShove.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToShove, kAct_ReactToShove );

// If the AI was shoved longer ago than this, it is too late to respond.  
// This should only occur if the AI was doing something more important than 
// responding to being shoved, such as responding to a player action with a 
// recoil

static float gk_flShovedTimeDelta = 0.1f;

static EnumAnimProp GetMovementDirProp( const LTVector& vDirection, const LTVector& vForward, const LTVector& vRight )
{
	// Direction has no velocity

	LTVector vMovementDir2D( vDirection.x, 0.0, vDirection.z );
	if ( LTVector::GetIdentity() == vMovementDir2D )
	{
		return kAP_MDIR_Forward;
	}

	vMovementDir2D.Normalize();

	// Check for a shove in Front/Back

	LTVector vForward2D = vForward;
	vForward2D.y = 0.0f;
	if ( LTVector::GetIdentity() != vForward2D )
	{
		vForward2D.Normalize();
		float flAngle = vForward2D.Dot( vMovementDir2D );
		if ( flAngle > 0.5 )
		{
			return kAP_MDIR_Forward;
		}
		else if ( flAngle < -0.5 )
		{
			return kAP_MDIR_Backward;
		}
	}

	// Check in Left/Right

	LTVector vRight2D = vRight;
	vRight2D.y = 0.0f;
	if ( LTVector::GetIdentity() != vRight2D )
	{
		vRight2D.Normalize();
		float flAngle = vRight2D.Dot( vMovementDir2D );
		if ( flAngle >= 0.0 )
		{
			return kAP_MDIR_Right;
		}
		if ( flAngle < 0.0 )
		{
			return kAP_MDIR_Left;
		}
	}

	// Failed to find a valid direction; use forward as a fallback.

	return kAP_MDIR_Forward;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReactToShove::CAIActionReactToShove()
{
}

CAIActionReactToShove::~CAIActionReactToShove()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToShove::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to being shoved.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Shoved );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToShove::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// AI is not solid to AIs.

	if ( !pAI->IsSolidToAI() )
	{
		return false;
	}

	// AI hasn't been shoved.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Shoved );

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		return false;
	}

	// AI wasn't shoved recently enough to respond.

	if ( pFact->GetTime() < g_pLTServer->GetTime() - gk_flShovedTimeDelta )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToShove::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Bail if the fact does not exist.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Shoved );

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		return;
	}

	// Determine the direction from the shover to the reacting AI.

	EnumAnimProp eMoveDir = kAP_MDIR_Forward;
	LTVector vShoverPos;
	if ( LT_OK == g_pLTServer->GetObjectPos( pFact->GetSourceObject(), &vShoverPos ) )
	{
		eMoveDir = GetMovementDirProp( 
			pAI->GetPosition() - vShoverPos,
			pAI->GetAIMovement()->GetForward(), pAI->GetAIMovement()->GetRight() );
	}

	// Set up the AIs state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );
	pStateUseSmartObject->SetProp( kAPG_MovementDir, eMoveDir );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Become temporarily non solid to pathing AIs.

	pAI->SetSolidToAI( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToShove::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Become solid again (this is safe as this action will fail to activate 
	// if the AI is not initially solid).

	pAI->SetSolidToAI( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToShove::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToShove::IsActionComplete( CAI* pAI )
{
	// Recoiling is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Recoiling is not complete.

	return false;
}

