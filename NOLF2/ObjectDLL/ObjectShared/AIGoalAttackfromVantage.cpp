// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromVantage.cpp
//
// PURPOSE : AIGoalAttackFromVantage implementation
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackFromVantage.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromVantage, kGoal_AttackFromVantage);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackFromVantage::CAIGoalAttackFromVantage()
{
	m_bNodeBased	= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackFromVantage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNode != LTNULL, m_pAI->m_hObject, "CAIGoalAttackFromVantage::ActivateGoal: AINodeVantage is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags(kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile);

	m_pGoalMgr->LockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::SetStateAttack
//
//	PURPOSE:	Set AttackFromVantage state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::SetStateAttack()
{
	m_pAI->SetCurrentWeapon( m_eWeaponType );

	// Set new state, or reset state if node has changed.

	CAIHumanStateAttackFromVantage* pAttackState = LTNULL;
	if( m_pAI->GetState()->GetStateType() == kState_HumanAttackFromVantage )
	{
		pAttackState = (CAIHumanStateAttackFromVantage*)m_pAI->GetState();
	}

	if( ( !pAttackState ) || ( pAttackState->GetVantageNode() != m_hNode ) )
	{
		m_pAI->ClearAndSetState( kState_HumanAttackFromVantage );
		pAttackState = (CAIHumanStateAttackFromVantage*)m_pAI->GetState();
		pAttackState->SetVantageNode( m_hNode );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAttackFromVantage:
			HandleStateAttackFromVantage();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		case kState_HumanCharge:
			HandleStateCharge();
			break;

		case kState_HumanHolster:
			HandleStateHolster();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanIdle:
			// We need to figure out if the AI ought to be in idle here, and
			// if so, we need to be sure that we are handling it correctly!
			Warn( "AI in Goal:AttackFromVantage in unexpected idle state" );
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromVantage::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::HandleStateAttackFromVantage
//
//	PURPOSE:	Determine what to do when in state AttackFromVantage.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::HandleStateAttackFromVantage()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Attacking:
			{
				// If the node has a weapon string, and if the AI does not 
				// match the string AND if the AI has a weapon which defines a
				// transition at this location, then do a weapon change.
				
				HandlePotentialWeaponChange();
			}
			break;

		case kSStat_Moving:
			break;

		case kSStat_FailedComplete:
			m_hFailedNode = m_hNode;
			m_hNode = LTNULL;

			// Done attacking from vantage, so charge!
			// If there are other attractors (cover, view, vantage nodes),
			// a goal will re-activate instead of charging.

			m_pAI->SetState( kState_HumanCharge );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromVantage::HandleStateAttackFromVantage: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::HandleStateCharge
//
//	PURPOSE:	Determine what to do when in state Charge.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromVantage::HandleStateCharge()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if(m_pGoalMgr->IsGoalLocked(this))
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromVantage::HandleStateCharge: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackFromVantage::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	return g_pAINodeMgr->FindNearestNodeFromThreat(m_pAI, eNodeType, vPos, m_hStimulusSource, 1.f);
}


