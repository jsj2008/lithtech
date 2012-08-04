// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShortRecoil.cpp
//
// PURPOSE : AIActionShortRecoil class implementation
//
// CREATED : 03/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionShortRecoil.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "AIStateUseSmartObject.h"
#include "AIWorkingMemoryCentral.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShortRecoil, kAct_ShortRecoil );

#define MIN_STUMBLE_RECOIL_DIST	80.f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionShortRecoil::CAIActionShortRecoil()
{
	m_bFourQuadrantDirectionalRecoils = false;
	m_bApplyContextEffect = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Don't recoil if AI is already recoiling.

	if ( !g_pAIDB->GetAIConstantsRecord()->bAllowRecoilsToInterruptRecoils )
	{
		EnumAnimProp eAction = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
		if( ( eAction == kAP_ACT_ShortRecoil ) 
			|| ( eAction == kAP_ACT_LongRecoil ) 
			|| ( eAction == kAP_ACT_DefeatedRecoil ) 
			)
		{
			return false;
		}
	}
	EnumAnimProp eActivity = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Activity );
	if( eActivity == kAP_ATVT_KnockDown )
	{
		return false;
	}

	// Get a damage fact to use for the prop generation.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if (!pFact || !DidDamage(pAI, pFact))
	{
		return false;
	}

	// Bail if damage was not recent.
	// Perform this check during planning only, as some derived classes
	// rely on action chaining to get to this action.  If this test occurs
	// at runtime, the AI will fail once it starts the recoil.

	if ( bIsPlanning )
	{
		if( pFact->GetUpdateTime() + 0.3f < g_pLTServer->GetTime() )
		{
			return false;
		}
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Bail if a damage type is specified and it is the wrong type.

	DamageType eDamageType;
	LTVector vDamageDir;
	pFact->GetDamage( &eDamageType, NULL, &vDamageDir );

	EnumAnimProp eDamageProp = DamageTypeToDamageProp( eDamageType );
	if( pSmartObjectRecord->Props.Get( kAPG_Damage ) != kAP_None 
		&& eDamageProp != pSmartObjectRecord->Props.Get( kAPG_Damage ) )
	{
		return false;
	}

	// Use the position of the damager when determining the direction of the 
	// damage, if possible.

	LTVector vDamagerPos;
	if ( LT_OK == g_pLTServer->GetObjectPos( pFact->GetTargetObject(), &vDamagerPos ) )
	{
		if ( pAI->GetPosition() != vDamagerPos  )
		{
			vDamageDir = ( pAI->GetPosition() - vDamagerPos ).GetUnit();	
		}
	}


	// If a timeout has been set, only one AI may recoil within some frequency.

	if( !CanAIRecoil( pAI, pSmartObjectRecord ) )
	{
		return false;
	}

	// Bail if we fail to create the associated anim props.

	CAnimationProps AnimProps;
	if (!GetRecoilProps(pAI, pFact, AnimProps))
	{
		return false;
	}

	// Animation does not exist. Check for a stumbling version.

	uint32 nAnimations = pAI->GetAnimationContext()->CountAnimations(AnimProps);
	if (0 == nAnimations)
	{

#ifndef _FINAL
#if defined(PROJECT_DARK)
		// For designer debugging, print out the properties any time we fail 
		// to find a recoil.  While this is allowed in general, in our game, 
		// it should never happen.

		if ( kAP_ACT_ShortRecoil == AnimProps.Get( kAPG_Action ) 
			|| kAP_ACT_DefeatedRecoil == AnimProps.Get( kAPG_Action ) )
		{
			// Do an animation lookup to force an error printing.
			ANIM_QUERY_RESULTS Results;
			pAI->GetAnimationContext()->FindAnimation( AnimProps, Results, FAILURE_IS_ERROR );
		}

#endif // defined(PROJECT_DARK)
#endif // _FINAL

		// Bail if AI also does not have a stumbling recoil animation, or 
		// does not have room to play the recoil animation.

		if( !IsStumbleRecoilValid( pAI, vDamageDir, AnimProps ) )
		{
			return false;
		}
	}

	// Special-case check for knock downs.

	if( AnimProps.Get( kAPG_Activity ) == kAP_ATVT_KnockDown )
	{
		if( !IsKnockDownValid( pAI, vDamageDir ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is already recoiling.

	if ( !g_pAIDB->GetAIConstantsRecord()->bAllowRecoilsToInterruptRecoils )
	{
		EnumAnimProp eAction = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
		if( ( eAction == kAP_ACT_ShortRecoil ) 
			|| ( eAction == kAP_ACT_LongRecoil ) 
			|| ( eAction == kAP_ACT_DefeatedRecoil ) 
			)
		{
			return;
		}
	}

	// Bail if no damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact || !DidDamage(pAI, pFact))
	{
		return;
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Get the animation props.

	CAnimationProps AnimProps;
	if (!GetRecoilProps(pAI, pFact, AnimProps))
	{
		return;
	}

	// Play a stumbling version of the recoil if possible.
	// Randomize whether or not to play stumble.

	LTVector vDamageDir;
	pFact->GetDamage( NULL, NULL, &vDamageDir );

	// Use the position of the damager when determining the direction of the 
	// damage, if possible.

	LTVector vDamagerPos;
	if ( LT_OK == g_pLTServer->GetObjectPos( pFact->GetTargetObject(), &vDamagerPos ) )
	{
		if ( pAI->GetPosition() != vDamagerPos  )
		{
			vDamageDir = ( pAI->GetPosition() - vDamagerPos ).GetUnit();	
		}
	}


	bool bMoving = ( GetRandom( 0.f, 1.f ) > 0.5f ) && 
					IsStumbleRecoilValid( pAI, vDamageDir, AnimProps );

	// Set SmartObject state.
	
	pAI->SetState( kState_UseSmartObject );

	// Set smartobject.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Set additional props for the damage.

	pStateUseSmartObject->SetProp( kAPG_Damage, AnimProps.Get( kAPG_Damage ) );
	pStateUseSmartObject->SetProp( kAPG_DamageDir, AnimProps.Get( kAPG_DamageDir ) );
	pStateUseSmartObject->SetProp( kAPG_Body, AnimProps.Get( kAPG_Body ) );
	pStateUseSmartObject->SetProp( kAPG_Movement, AnimProps.Get( kAPG_Movement )  );
	pStateUseSmartObject->SetProp( kAPG_Activity, AnimProps.Get( kAPG_Activity )  );
	if(	bMoving )
	{
		pStateUseSmartObject->SetProp( kAPG_Movement, kAP_MOV_Stumble );
	}

	// Set the weapon property manually.  This will insure it doesn't change 
	// during the duration of the animation; for instance, if the AI drops 
	// his weapon mid animation, we don't want to change to a different 
	// recoil, we want to finish the current recoil.

	pStateUseSmartObject->SetProp( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp()  );

	// Face towards or away from the blow.

	if( AnimProps.Get( kAPG_DamageDir ) == kAP_DDIR_Back )
	{
		pAI->GetAIBlackBoard()->SetBBFaceDir( vDamageDir );	
	}
	else {
		pAI->GetAIBlackBoard()->SetBBFaceDir( -vDamageDir );	
	}

	// Record who is recoiling on the blackboard.
	// Only one AI may recoil at a time.

	UpdateRecoilKnowledge( pAI, pSmartObjectRecord );

	// AI is incapacitated.

	pAI->GetAIBlackBoard()->SetBBIncapacitated( true );

	// Stop any dialogue sound that may be playing.

	pAI->StopDialogue();
	pAI->KillDialogueSound();

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Allow multiple transitions to be played in a row.  This is required for 
	// recoils which start with a transition.  As Fear is close to shipping 
	// and this change may negatively impact them, it is disabled for this 
	// project.

#ifndef PROJECT_FEAR
	pAI->GetAnimationContext()->ClearLock();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::DeactivateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Allow other AI to recoil.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	ClearRecoilKnowledge( pAI, pSmartObjectRecord );

	// AI is no longer incapacitated.

	pAI->GetAIBlackBoard()->SetBBIncapacitated( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::IsActionComplete( CAI* pAI )
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	if ( m_bApplyContextEffect )
	{
		ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::GetRecoilProps
//
//	PURPOSE:	Returns by parameter the animation props for a recoil
//				animation, given an AI and a fact.  Returns true if the
//				props are set up, false if they were not.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::GetRecoilProps(CAI* pAI, CAIWMFact* pFact, CAnimationProps& outAnimProps)
{
	if (!pAI || !pFact)
	{
		return false;
	}

	// Determine which body part was hit.

	EnumAnimProp eBodyAnipProp = kAP_None;
	ModelsDB::HNODE hModelNode = pAI->GetModelNodeLastHit();
	if( hModelNode )
	{
		eBodyAnipProp = g_pModelsDB->GetNodeBodyAnimProp( hModelNode );
		if( eBodyAnipProp == kAP_Invalid )
		{
			return false;
		}
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Determine if AI was hit from the front or back.

	DamageType eDamageType;
	LTVector vDamageDir;
	pFact->GetDamage( &eDamageType, NULL, &vDamageDir );
	EnumAnimProp eDamageDirAnimProp = kAP_DDIR_Front;
	if( !pAI->HitFromFront( vDamageDir ) )
	{
		eDamageDirAnimProp = kAP_DDIR_Back;
	}

	if ( m_bFourQuadrantDirectionalRecoils )
	{
		// Support 4 directions instead of just 2.  Front/Back has already
		// been handled.  Change to left/right if closer to this axis.
		// If not, leave the value alone.

		LTRotation rRot;
		g_pLTServer->GetObjectRotation( pAI->GetHOBJECT(), &rRot );

		LTVector vAIRight2D = rRot.Right();
		vAIRight2D.y = 0;

		LTVector vDamageDir2D = vDamageDir;
		vDamageDir2D.y = 0;

		if ( vDamageDir2D != LTVector::GetIdentity() 
			&& vAIRight2D != LTVector::GetIdentity() )
		{
			vDamageDir2D.Normalize();
			vAIRight2D.Normalize();

			float flForwardDot = vDamageDir2D.Dot( vAIRight2D );
			if ( flForwardDot > 0.5f )
			{
				eDamageDirAnimProp = kAP_DDIR_Right;
			}
			else if ( flForwardDot < -0.5f )
			{
				eDamageDirAnimProp = kAP_DDIR_Left;
			}
		}
	}

	// return false if the damage type is invalid, as if it is, there is no need to
	// search the animation tree as there will be no match.

	EnumAnimProp eDamageProp = DamageTypeToDamageProp( eDamageType );
	if (kAP_Invalid == eDamageProp)
	{
		return false;
	}

	EnumAnimProp ePosture = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
	
	outAnimProps = pSmartObjectRecord->Props;
	outAnimProps.Set( kAPG_Posture, ePosture );
	outAnimProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	outAnimProps.Set( kAPG_Body, eBodyAnipProp );
	outAnimProps.Set( kAPG_Damage, eDamageProp );
	outAnimProps.Set( kAPG_DamageDir, eDamageDirAnimProp );
	outAnimProps.Set( kAPG_Movement, pAI->GetAnimationContext()->GetCurrentProp( kAPG_Movement ) );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::IsStumbleRecoilValid
//
//	PURPOSE:	Returns true if AI has a stumbling recoil animation,
//				and there is enough space to play it.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::IsStumbleRecoilValid( CAI* pAI, const LTVector& vDir, CAnimationProps& AnimProps )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// No direction - nowhere to stumble towards.

	if ( LTVector(0.f, 0.f, 0.f ) == vDir )
	{
		return false;
	}

	// Bail if AI also does not have a stumbling recoil animation.

	AnimProps.Set( kAPG_Movement, kAP_MOV_Stumble );
	if( 0 == pAI->GetAnimationContext()->CountAnimations(AnimProps) )
	{
		AnimProps.Set( kAPG_Movement, pAI->GetAnimationContext()->GetProp( kAPG_Movement ) );
		return false;
	}

	// Bail if AI does not have enough room to play a moving recoil.

	LTVector vDest = pAI->GetPosition() + ( vDir * MIN_STUMBLE_RECOIL_DIST );
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// AI can play a stumbling recoil!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::IsKnockDownValid
//
//	PURPOSE:	Returns true if there is enough space to play a knock down.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::IsKnockDownValid( CAI* pAI, const LTVector& vDir )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if AI does not have enough room to play a knock down.
	// Check both directions.

	LTVector vDest = pAI->GetPosition() + ( vDir * MIN_STUMBLE_RECOIL_DIST );
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	vDest = pAI->GetPosition() - ( vDir * MIN_STUMBLE_RECOIL_DIST );
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Knock down can play.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::CanAIRecoil
//
//	PURPOSE:	Return true if the AI may recoil.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoil::CanAIRecoil( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord )
{
	// Sanity check.

	if( !( pAI && pSmartObjectRecord ) )
	{
		return false;
	}

	// Bail if someone else has long recoiled too recently.

	CAIWMFact factOnlyOneQuery, factLastTimeQuery;
	factOnlyOneQuery.SetFactType( kFact_Knowledge );
	factLastTimeQuery.SetFactType( kFact_Knowledge );
	if( pSmartObjectRecord->Props.Get( kAPG_Action ) == kAP_ACT_LongRecoil )
	{
		factOnlyOneQuery.SetKnowledgeType( kKnowledge_LongRecoiling );
		factLastTimeQuery.SetKnowledgeType( kKnowledge_LastLongRecoilTime );
	}

	// Bail if someone else has gotten knocked out too recently.

	else if( pSmartObjectRecord->Props.Get( kAPG_Activity ) == kAP_ATVT_KnockDown )
	{
		factOnlyOneQuery.SetKnowledgeType( kKnowledge_KnockedDown );
		factLastTimeQuery.SetKnowledgeType( kKnowledge_LastKnockDownTime );
	}

	// Not a recoil that supports limiting.

	else 
	{
		return true;
	}

	// Bail if someone is already recoiling.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factOnlyOneQuery );
	if( pFact )
	{
		// Clear records of dead AI.

		if( IsDeadAI( pFact->GetSourceObject() ) )
		{
			g_pAIWorkingMemoryCentral->ClearWMFacts( factOnlyOneQuery );
		}
		else return false;
	}

	// Bail if someone has recoiled too recently.

	if( pSmartObjectRecord->fTimeout > 0.f )
	{
		pFact = g_pAIWorkingMemoryCentral->FindWMFact( factLastTimeQuery );
		double fLastTime = pFact ? pFact->GetTime() : 0.f;
		if( fLastTime + pSmartObjectRecord->fTimeout > g_pLTServer->GetTime() )
		{
			return false;
		}
	}

	// It is time to recoil.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::UpdateRecoilKnowledge
//
//	PURPOSE:	Update working memories about recoiling AI.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::UpdateRecoilKnowledge( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord )
{
	// Sanity check.

	if( !( pAI && pSmartObjectRecord ) )
	{
		return;
	}

	// Record knowledge for long recoils and knock downs.

	ENUM_AIWMKNOWLEDGE_TYPE eKnowledge = kKnowledge_InvalidType;
	if( pSmartObjectRecord->Props.Get( kAPG_Action ) == kAP_ACT_LongRecoil )
	{
		eKnowledge = kKnowledge_LongRecoiling;
	}
	else if( pSmartObjectRecord->Props.Get( kAPG_Activity ) == kAP_ATVT_KnockDown )
	{
		eKnowledge = kKnowledge_KnockedDown;
	}

	// Do not record knowledge for this type of recoil.

	if( eKnowledge == kKnowledge_InvalidType )
	{
		return;
	}

	// Only one AI may recoil at a time.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if (pFact)
	{
		pFact->SetKnowledgeType( eKnowledge, 1.f );
		pFact->SetSourceObject( pAI->m_hObject, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::ClearRecoilKnowledge
//
//	PURPOSE:	Clear working memories about recoiling AI.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::ClearRecoilKnowledge( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord )
{
	// Sanity check.

	if( !( pAI && pSmartObjectRecord ) )
	{
		return;
	}

	// Clear knowledge for long recoils and knock downs.

	ENUM_AIWMKNOWLEDGE_TYPE eKnowledge = kKnowledge_InvalidType;
	if( pSmartObjectRecord->Props.Get( kAPG_Action ) == kAP_ACT_LongRecoil )
	{
		eKnowledge = kKnowledge_LongRecoiling;
	}
	else if( pSmartObjectRecord->Props.Get( kAPG_Activity ) == kAP_ATVT_KnockDown )
	{
		eKnowledge = kKnowledge_KnockedDown;
	}

	// Do not clear knowledge for this type of recoil.

	if( eKnowledge == kKnowledge_InvalidType )
	{
		return;
	}

	// Remove knowledge of recoiling AI.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( eKnowledge );
	factQuery.SetSourceObject( pAI->m_hObject );
	g_pAIWorkingMemoryCentral->ClearWMFact( factQuery );


	// Do not allow anyone to recoil again too soon.

	if( pSmartObjectRecord->fTimeout )
	{
		CAIWMFact factQueryLast;
		factQueryLast.SetFactType( kFact_Knowledge );
		if( eKnowledge == kKnowledge_LongRecoiling )
		{
			factQueryLast.SetKnowledgeType( kKnowledge_LastLongRecoilTime );
		}
		else if( eKnowledge == kKnowledge_KnockedDown )
		{
			factQueryLast.SetKnowledgeType( kKnowledge_LastKnockDownTime );
		}

		CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQueryLast );
		if( !pFact )
		{
			pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
			pFact->SetKnowledgeType( factQueryLast.GetKnowledgeType(), 1.f );
		}
		if( pFact )
		{
			pFact->SetTime( g_pLTServer->GetTime() );	
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::SetFourQuadrantDirectionalRecoils
//
//	PURPOSE:	Enable configuration of this action to support recoiling 
//				in 4 directions.  In the past, only 2 were supported; as 
//				this increases the numbers of animations needed, it should 
//				be enabled selectively.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::SetFourQuadrantDirectionalRecoils( bool b )
{
	m_bFourQuadrantDirectionalRecoils = b;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoil::SetFourQuadrantDirectionalRecoils
//
//	PURPOSE:	Enable derived actions to determine if the context effect 
//				should be applied or not.  By default, the context is 
//				applied.  For two stage recoils, the first stage may not 
//				want the context applied.  If the AI restarts the recoil
//				during the second stage, the first stage will not be 
//				played if it is applied.
//
// ----------------------------------------------------------------------- //

void CAIActionShortRecoil::SetApplyContextEffect( bool bApply )
{
	m_bApplyContextEffect = bApply;
}
