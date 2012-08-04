// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLoveKitty.cpp
//
// PURPOSE : AIGoalLoveKitty implementation
//
// CREATED : 2/14/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalLoveKitty.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIHumanState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLoveKitty, kGoal_LoveKitty);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalLoveKitty::CAIGoalLoveKitty()
{
	m_bIHateCats = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalLoveKitty::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR( m_vDest );
	SAVE_BOOL( m_bIHateCats );
}

void CAIGoalLoveKitty::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR( m_vDest );
	LOAD_BOOL( m_bIHateCats );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLoveKitty::ActivateGoal()
{
	super::ActivateGoal();

	if( !m_pGoalMgr->IsGoalLocked( this ) )
	{
		m_pGoalMgr->LockGoal(this);

		m_pAI->SetState( kState_HumanAnimate );

		m_pAI->FaceObject( m_hStimulusSource );

		// Setup animprops for the discovery animation.
	
		CAnimationProps	animProps;
		animProps.Set( kAPG_Posture, kAP_Stand );
		animProps.Set( kAPG_Weapon, kAP_Weapon1 );
		animProps.Set( kAPG_WeaponPosition, kAP_Down );
		animProps.Set( kAPG_Action, kAP_Discover );

		CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
		pStateAnimate->SetAnimation( animProps, LTFALSE );

		m_pAI->PlaySound( kAIS_DiscoveryGood, LTFALSE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLoveKitty::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAnimate:
			HandleStateAnimate();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalLoveKitty::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalLoveKitty::HandleStateAnimate()
{
	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());

	switch( pStateAnimate->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		// Discovery animation finished.

		case kSStat_StateComplete:
			{
				m_pAI->SetState( kState_HumanGoto );
				CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
				pGoto->SetDest(m_vDest);
				pGoto->SetAwareness( kAP_Attracted );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalLoveKitty::HandleStateAnimate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalLoveKitty::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			// Lose interest when arrived at destination.
			m_fCurImportance = 0.f;
			m_pGoalMgr->UnlockGoal(this);
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalLoveKitty::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::HandleDamage
//
//	PURPOSE:	Learn to hate kitties if that's what damaged me.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalLoveKitty::HandleDamage(const DamageStruct& damage)
{
	// Learn to hate cats.

	if( m_hStimulusSource == (HOBJECT)damage.hDamager )
	{
		m_bIHateCats = LTTRUE;
	}

	// Always allow normal damage handling.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLoveKitty::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalLoveKitty::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Only return TRUE if the stimulus came from an AngryKitty mine.

		if( pSenseRecord->hLastStimulusTarget )
		{
			static HCLASS hTest  = g_pLTServer->GetClass( "CKitty" );
			HCLASS hClass = g_pLTServer->GetObjectClass( pSenseRecord->hLastStimulusTarget );
			if( g_pLTServer->IsKindOf( hClass, hTest ) )
			{
				// Only attracted if a kitty has not blown up before.

				if( !m_bIHateCats )
				{
					m_vDest = pSenseRecord->vLastStimulusPos;
					return LTTRUE;
				}
				else {
					m_pAI->PlaySound( kAIS_DiscoveryBad, LTFALSE );
				}
			}
		}
	}

	return LTFALSE;
}


