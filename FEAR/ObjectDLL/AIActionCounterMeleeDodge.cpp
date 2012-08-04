// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionCounterMeleeDodge.cpp
//
// PURPOSE : 
//
// CREATED : 9/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionCounterMeleeDodge.h"
#include "AIDB.h"
#include "AIUtils.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionCounterMeleeDodge, kAct_CounterMeleeDodge );

static bool GetWorldSpaceTransform( CAI* pAI, const CAnimationProps& rProps, uint32 iSeed, LTVector& vOutWorldSpaceAnimationEndPosition )
{
	// Clearing the cache is required as the cache queries don't test for
	// different seed values.
	pAI->GetAnimationContext()->ClearCachedAni(); 

	// Get the animation cooresponding to this prop set/seed combination.
	// To avoid destroying the current seed, save and restore it.

	uint32 iCurrentRandomSeed = pAI->GetAnimationContext()->GetRandomSeed();
	pAI->GetAnimationContext()->SetRandomSeed( iSeed );
	HMODELANIM hAni = pAI->GetAnimationContext()->GetAni( rProps );
	pAI->GetAnimationContext()->SetRandomSeed( iCurrentRandomSeed );

	// Bail if the animation cannot be found.

	if( hAni == INVALID_MODEL_ANIM )
	{
		return false;
	}

	// Get the object space offset.  Check the cache first, as this is 
	// expensive to calculate.

	LTVector vObjectSpaceOffset;

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_AnimationOffset );
	queryFact.SetIndex( hAni );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact )
	{
		// Calculate the result, then cache it so that it doesn't need to be 
		// calculated again for this AI.
		
		LTRigidTransform rTransform;
		if ( !GetAnimationTransform( pAI, hAni, rTransform ) )
		{
			AIASSERT( 0, pAI->GetHOBJECT(), "GetWorldSpaceTransform : Failed to get transform.  Potential performance issue as this will fail caching." );
			return false;
		}

		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_AnimationOffset );
		pFact->SetIndex( hAni );
		pFact->SetDir( rTransform.m_vPos );

		vObjectSpaceOffset = rTransform.m_vPos;
	}
	else
	{
		vObjectSpaceOffset = pFact->GetDir();
	}

	// Rotate from object space into world space.

	LTRotation pAIRot;
	if ( LT_OK != g_pLTServer->GetObjectRotation( pAI->GetHOBJECT(), &pAIRot ) )
	{
		return false;
	}

	vOutWorldSpaceAnimationEndPosition = pAI->GetPosition() + pAIRot.RotateVector( vObjectSpaceOffset );
	return true;
}

static void GetDodgeProps( CAnimationProps& OutProps, CAI* pAI, const AIDB_SmartObjectRecord& rSmartObjectRecord )
{
	OutProps = rSmartObjectRecord.Props;
	OutProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
}

static uint32 GetDodgeAnimationSeed( CAI* pAI, const CAnimationProps& props )
{
	// Determine if one of the animations can be executed without leaving 
	// the nav mesh or colliding with nav mesh boundary.  If at least one can,
	// execute it.

	// Count the number of animation matches

	uint32 nDodgeAnimations = pAI->GetAnimationContext()->CountAnimations( props );

	// Build a seed index map to allow random animation selection.

	const int kMaxAnimations = 10;
	int aAnimationSeedMap[kMaxAnimations];

	if ( nDodgeAnimations >= LTARRAYSIZE( aAnimationSeedMap ) )
	{
		ObjectCPrint( pAI->GetHOBJECT(), "Too many dodge animations.");
		nDodgeAnimations = LTARRAYSIZE( aAnimationSeedMap );
	}

	for ( uint32 i = 0; i < nDodgeAnimations; ++i )
	{
		aAnimationSeedMap[i] = i;
	}
	std::random_shuffle( &aAnimationSeedMap[0], &aAnimationSeedMap[0] + nDodgeAnimations );

	//
	// Find the first match, searching through the randomized map.
	//

	for ( uint32 iAnimationSeedIndex = 0; iAnimationSeedIndex < nDodgeAnimations; ++iAnimationSeedIndex )
	{
		// Get the seed at this index from the map.

		uint32 iSeed = aAnimationSeedMap[iAnimationSeedIndex];

		// Fail if we cannot get the world space transform for this prop/seed combination.

		LTVector vEndPosition;
		if ( !GetWorldSpaceTransform( pAI, props, iSeed, vEndPosition ) )
		{
			continue;
		}

		// Fail if this dodge moves and there is no room for the movement.

		if ( pAI->GetPosition() != vEndPosition 
			&& !IsClearForMovement( pAI, pAI->GetPosition(), vEndPosition ) )
		{
			continue;
		}

		// Success!  A valid movement was found.

		return iSeed;
	}

	return CAnimationContext::GetInvalidRandomSeed();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeDodge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionCounterMeleeDodge::CAIActionCounterMeleeDodge()
{
}

CAIActionCounterMeleeDodge::~CAIActionCounterMeleeDodge()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeDodge::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to the incoming attack.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_IncomingMeleeAttack );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeDodge::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionCounterMeleeDodge::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Do not dodge if AI is attached to something.

	HOBJECT hAttached = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( hAttached )
	{
		return false;
	}

	// Do not dodge if AI is at some node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		return false;
	}

	// Only dodge if we are armed.

	pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return false;
	}

	// Do not dodge if the AI doesn't know about an incoming melee attack

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact || ( 1.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) ) )
	{
		return false;
	}

	// Do not dodge if the enemy is not a a character.

	if ( !pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// Do not dodge if the target is not exposed to dodging

	CCharacter* pChar = CCharacter::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if ( !pChar || !pChar->GetDodgeWindowOpen() )
	{
		return false;
	}

	// Fail to dodge if the target is a dodging AI.

	if ( CAI* pTargetAI = CAI::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		EnumAnimProp eAction = pTargetAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
		if ( eAction == kAP_ACT_Dodge )
		{
			return false;
		}
	}

	// Do not dodge if the action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionCounterMeleeDodge::ActivateAction : No SmartObjectRecord specified.");
	if( !pSmartObjectRecord )
	{
		return false;
	}
	CAnimationProps props;
	GetDodgeProps( props, pAI, *pSmartObjectRecord);
	int iSeed = GetDodgeAnimationSeed( pAI, props );

	// Do not dodge if there is not a valid dodge animation.

	if ( CAnimationContext::GetInvalidRandomSeed() == GetDodgeAnimationSeed( pAI, props ) )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeDodge::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeDodge::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Fail if there is not smartobject record for this action.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionCounterMeleeDodge::ActivateAction : No SmartObjectRecord specified.");
	if( !pSmartObjectRecord )
	{
		return;
	}
	CAnimationProps props;
	GetDodgeProps( props, pAI, *pSmartObjectRecord);
	int iSeed = GetDodgeAnimationSeed( pAI, props );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set dodge animation.

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( props, !LOOP );

	// Set the animation seed of the validated animation.

	pAI->GetAnimationContext()->SetRandomSeed( iSeed );

	// Aim at the target, but do not face the target.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeDodge::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionCounterMeleeDodge::IsActionComplete( CAI* pAI )
{
	// Dodging is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Dodging is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeDodge::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeDodge::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
