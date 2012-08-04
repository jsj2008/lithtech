// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackProp.cpp
//
// PURPOSE : AIGoalAttackProp implementation
//
// CREATED : 7/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackProp.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AIHuman.h"
#include "AIGoalButeMgr.h"
#include "Prop.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackProp, kGoal_AttackProp);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackProp::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNode != LTNULL, m_pAI->m_hObject, "CAIGoalAttackProp::ActivateGoal: AINodeUseObject is NULL.");

	if(m_pAI->GetCurrentWeapon())
	{
		SetStateAttack();
	}
	else {
		m_pGoalMgr->LockGoal( this );
		m_pAI->SetState( kState_HumanDraw );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::SetStateAttack
//
//	PURPOSE:	Set Attack state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::SetStateAttack()
{
	m_pAI->SetCurrentWeapon( m_eWeaponType );

	if( m_pAI->GetState()->GetStateType() != kState_HumanAttackProp )
	{
		m_pAI->SetState( kState_HumanAttackProp );

		// Set object to attack.

		HOBJECT hObject;
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject(m_hNode);
		CAIHumanStateAttackProp* pStateAttackProp = (CAIHumanStateAttackProp*)(m_pAI->GetState());
		if ( LT_OK == FindNamedObject(pNode->GetObject(), hObject) )
		{
			pStateAttackProp->SetProp(hObject);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAttackProp:
			HandleStateAttackProp();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		// Unexpected state.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackProp::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::HandleStateAttackProp
//
//	PURPOSE:	Determine what to do when in state AttackProp.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::HandleStateAttackProp()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:	
			{
				// Disable node when prop has been destroyed.

				AINode* pNode = (AINode*)g_pLTServer->HandleToObject(m_hNode);
				pNode->Disable();
				m_hNode = LTNULL;
				m_fCurImportance = 0.f;
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackProp::HandleStateAttackProp: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProp::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal( this );
			SetStateAttack();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProp::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackProp::HandleGoalAttractors()
{
	// Check if already attacking a prop.
	if( m_pAI->GetState()->GetStateType() != kState_HumanAttackProp )
	{
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( pAIHuman->HasHolsterString() || pAIHuman->GetPrimaryWeapon())
		{
			AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
			AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalAbstract::HandleGoalAttractors: Goal has no attractors.");

			// Check if attractors are triggering activateability.
			AINode* pNode;
			for(uint32 iAttractor=0; iAttractor < pTemplate->cAttractors; ++iAttractor)
			{
				pNode = g_pAINodeMgr->FindNearestNodeInRadius(m_pAI, pTemplate->aAttractors[iAttractor], m_pAI->GetPosition(), pTemplate->fAttractorDistSqr * m_fBaseImportance, LTTRUE);
				if(pNode != LTNULL)
				{
					HOBJECT hObject;
					if ( LT_OK == FindNamedObject(pNode->GetObject(), hObject) )
					{
						Prop* pProp = (Prop*)g_pLTServer->HandleToObject(hObject);
						if(pProp->GetState() != kState_PropDestroyed)
						{
							AIASSERT(pNode->GetType() == kNode_UseObject, m_pAI->m_hObject, "CAIGoalAttackProp::HandleGoalAttractors: AINode is not of type UseObject.");
							m_hNode = pNode->m_hObject;
							SetCurToBaseImportance();
							return pNode;
						}
					}
					
					// Disable node if prop has been destroyed.
					pNode->Disable();				
				}
			}
		}

		m_hNode = LTNULL;
	}
	return LTNULL;
}

