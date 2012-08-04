// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWork.cpp
//
// PURPOSE : AIGoalWork implementation
//
// CREATED : 7/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalWork.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIHumanState.h"
#include "AnimationMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalWork,			kGoal_Work);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPlacePoster,	kGoal_PlacePoster);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalEnjoyPoster,	kGoal_EnjoyPoster);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDestroy,		kGoal_Destroy);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRide,			kGoal_Ride);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalWork::CAIGoalWork()
{
	m_bRequireBareHands = LTTRUE;
	m_bAllowDialogue = LTTRUE;
	m_bTurnOffLights = LTTRUE;
	m_bTurnOnLights = LTTRUE;

	m_eDeactivationState = kWDS_None;
}

CAIGoalWork::~CAIGoalWork()
{
	// If destructing because the AI died,
	// and no special death was played,
	// and node is only locked by this goal,
	// then turn off the object.

	if( m_pAI->IsDead() && 
		m_hNodeUseObject &&
		m_bLockedNode &&
		!m_bPlayedSpecialDeathAnim )
	{
		AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
		AIASSERT( pNodeUseObject && pNodeUseObject->IsLocked(), m_pAI->m_hObject, "CAIGoalAbstractUseObject::DeactivateGoal: Node is NULL or not locked" );
		if( pNodeUseObject && 
			pNodeUseObject->GetHObject() &&
			( pNodeUseObject->GetLockCount() == 1 ) )
		{
			SendTriggerMsgToObject( m_pAI, pNodeUseObject->GetHObject(), LTFALSE, "OFF" );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalWork::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_eDeactivationState );
}

void CAIGoalWork::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_eDeactivationState, EnumWorkDeactivationState );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalWork::ActivateGoal()
{
	super::ActivateGoal();

	// Clear any object AI was previously interacting with.

	m_pAI->SetAnimObject( LTNULL );

	m_pAI->SetAwareness( kAware_Relaxed );
	
	m_eDeactivationState = kWDS_None;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalWork::DeactivateGoal()
{
	// Check if deactivation has already started, to handle special cases
	// where AI is interrupted or shot with special damage.

	switch( m_eDeactivationState )
	{
		case kWDS_Interrupting:
			// We have already started deactivating if we are in the Animate state, 
			// animating an interruption.

			if( m_pAI->GetState()->GetStateType() == kState_HumanAnimate )
			{
				// The interruption animation has finished, so deactivation is complete.

				if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
				{
					m_pGoalMgr->UnlockGoal( this );
				}
			}
			else {
				m_pGoalMgr->UnlockGoal( this );
			}
			return;

		case kWDS_Transitioning:
			if( !m_pAI->GetAnimationContext()->IsTransitioning() )
			{
				m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
				m_eDeactivationState = kWDS_DoneTransitioning;
			}
			return;

		case kWDS_DoneTransitioning:
			if( HandleSpecialDamageDeactivation() )
			{
				m_pGoalMgr->UnlockGoal( this );
			}
			else if( !HandleWorkInterruptionDeactivation() )
			{
				m_pGoalMgr->UnlockGoal( this );
			}
			return;
	}


	// Start deactivating.

	super::DeactivateGoal();

	// Rapidly finish a transition.

	if( m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		m_eDeactivationState = kWDS_Transitioning;
		m_pAI->GetAnimationContext()->SetAnimRate( 4.f );
		m_pGoalMgr->LockGoal(this);
		return;
	}

	// Special damage may take over the animation without letting 
	// the AI exit his work animations (e.g. passing out at a desk).

	if( HandleSpecialDamageDeactivation() )
	{
		return;
	}

	// Work may play a special interruption animation.

	HandleWorkInterruptionDeactivation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::HandleSpecialDamageDeactivation
//
//	PURPOSE:	Determine if we need to handle exiting specially for special damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalWork::HandleSpecialDamageDeactivation()
{
	// No special damage applied.

	if( !m_pAI->GetDamageFlags() )
	{
		return LTFALSE;
	}

	// Get the anim prop for the damage type.

	EnumAnimProp eDamage = kAP_None;
	switch( DamageFlagToType( m_pAI->GetDamageFlags() ) )
	{
		case DT_SLEEPING:
			eDamage = kAP_DamageSleeping;
			break;

		case DT_STUN:
			eDamage = kAP_DamageStunned;
			break;

		case DT_BEAR_TRAP:
			eDamage = kAP_DamageTrapped;
			break;

		case DT_GLUE:
			eDamage = kAP_DamageGlued;
			break;

		case DT_LAUGHING:
			eDamage = kAP_DamageLaughing;
			break;

		case DT_SLIPPERY:
			eDamage = kAP_DamageSlipping;
			break;
	}

	// Check if a damage animation exists for the current activity.

	if( ( m_pAI->GetState()->GetStateType() == kState_HumanUseObject ) &&
		( m_pAI->GetState()->GetStateStatus() == kSStat_PathComplete ) )
	{
		CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
		EnumAnimProp eActivity = pStateUseObject->GetActivity();

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, pStateUseObject->GetPose() );
		animProps.Set( kAPG_Weapon, kAP_Weapon3 );
		animProps.Set( kAPG_WeaponPosition, kAP_Lower );
		animProps.Set( kAPG_Awareness, eActivity );
		animProps.Set( kAPG_Action, kAP_Idle );
		animProps.Set( kAPG_Mood, eDamage );

		// This activity has a special damage animation,

		if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
		{	
			return LTTRUE;
		}
	}

	// This activity does not have a special damage animation,

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::HandleWorkInterruptionDeactivation
//
//	PURPOSE:	Determine if we need to handle exiting specially for interrupting work.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalWork::HandleWorkInterruptionDeactivation()
{
	// Were interrupted while in the middle of an activity?

	if( ( m_pAI->GetState()->GetStateType() == kState_HumanUseObject ) &&
		( m_pAI->GetState()->GetStateStatus() == kSStat_PathComplete ) )
	{
		CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
		EnumAnimProp eActivity = pStateUseObject->GetActivity();

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, pStateUseObject->GetPose() );
		animProps.Set( kAPG_Weapon, kAP_Weapon3 );
		animProps.Set( kAPG_WeaponPosition, kAP_Lower );
		animProps.Set( kAPG_Awareness, eActivity );
		animProps.Set( kAPG_Action, kAP_Interrupt );

		// If this activity has an interruption animation,
		// play it and lock deactivation until complete.

		if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
		{
			m_pAI->SetState( kState_HumanAnimate );
			CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
			pStateAnimate->SetAnimation( animProps, LTFALSE );
			m_eDeactivationState = kWDS_Interrupting;
			m_pGoalMgr->LockGoal(this);
			return LTTRUE;
		}
	}

	return LTFALSE;
}

