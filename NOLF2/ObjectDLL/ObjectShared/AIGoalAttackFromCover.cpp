// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromCover.cpp
//
// PURPOSE : AIGoalAttackFromCover implementation
//
// CREATED : 7/31/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackFromCover.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIMovement.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromCover, kGoal_AttackFromCover);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackFromCover::CAIGoalAttackFromCover()
{
	m_bNodeBased	= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackFromCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNode != LTNULL, m_pAI->m_hObject, "CAIGoalAttackFromCover::ActivateGoal: AINodeCover is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::SetStateAttack
//
//	PURPOSE:	Set AttackFromCover state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::SetStateAttack()
{
	m_pAI->SetCurrentWeapon( m_eWeaponType );

	m_pAI->SetAwareness( kAware_Alert );

	// Get a pointer to the node.

	AINodeCover* pCoverNode = (AINodeCover*)g_pLTServer->HandleToObject( m_hNode );
	if( !pCoverNode )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAttackFromCover::SetStateAttack: Cover node is NULL." );
		return;
	}

	// The node has an object.
		
	if( pCoverNode->HasObject() )
	{
		// Bail if already in progress.

		if( m_pAI->GetState()->GetStateType() == kState_HumanUseObject )
		{
			return;
		}

		HOBJECT hObject;
		if ( LT_OK == FindNamedObject( pCoverNode->GetObject(), hObject ) )
		{
			// The object is pointed to by a UseObject node with a cover SmartObject.

			AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pAINodeMgr->FindUseObjectNode( kNode_Coverable, hObject, LTTRUE );
			if( pNodeUseObject )
			{
				// Force running to cover.

				pNodeUseObject->SetMovement( kAP_Run );

				// First use the object, then run for cover.

				m_pAI->SetState( kState_HumanUseObject );

				CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
				pStateUseObject->SetNode( pNodeUseObject );
				pStateUseObject->SetWeaponPosition( kAP_Up );

				// Find a command string.

				HSTRING hstrCmd = pNodeUseObject->GetSmartObjectCommand( kNode_Coverable );
				if(hstrCmd != LTNULL)
				{
					pStateUseObject->SetSmartObjectCommand( hstrCmd );
				}
		
				AIASSERT(hstrCmd != LTNULL, m_pAI->m_hObject, "CAIGoalAttackFromCover::SetStateAttack: No command string found for cover smartobject.");

				// Lock cover node while using object.

				pCoverNode->Lock( m_pAI->m_hObject );

				// Set to AttackFromCover after done using object.

				return;
			}
		}
	}

	// No object (or an object but no node), so immediately AttackFromCover.

	if( m_pAI->GetState()->GetStateType() != kState_HumanAttackFromCover )
	{
		m_pAI->SetState( kState_HumanAttackFromCover );

		CAIHumanStateAttackFromCover* pAttackState = (CAIHumanStateAttackFromCover*)m_pAI->GetState();
		pAttackState->SetCoverNode( m_hNode );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanUseObject:
			HandleStateUseObject();
			break;

		case kState_HumanAttackFromCover:
			HandleStateAttackFromCover();
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

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromCover::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::HandleStateUseObject()
{
	AINodeCover* pCoverNode = (AINodeCover*)g_pLTServer->HandleToObject( m_hNode );
	if( !pCoverNode )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAttackFromCover::HandleStateUseObject: Cover node is NULL." );
		return;
	}

	// Abandon the state if target is blocking the path to the node.

	EnumNodeStatus eCoverStatus = pCoverNode->GetStatus( m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject() );
	if( ( kStatus_ThreatBlockingPath == eCoverStatus ) &&
		( !m_pAI->GetAIMovement()->IsMovementLocked() ) )
	{
		m_hFailedNode = m_hNode;
		m_hNode = LTNULL;
			
		// Done covering, so charge!

		CoverOrCharge();
		return;
	}

	// Check the status of the UseObject state.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			break;

		case kSStat_PathComplete:
			break;

		case kSStat_StateComplete:
			{
				// We are done using the object.

				pCoverNode->ClearObject();

				// Unlock cover node from using object, StateAttackFromCover will re-lock it.

				pCoverNode->Unlock( m_pAI->m_hObject );

				SetStateAttack();
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAttackFromCover::HandleStateUseObject: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::HandleStateAttackFromCover
//
//	PURPOSE:	Determine what to do when in state AttackFromCover.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::HandleStateAttackFromCover()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_FindCover:
			break;

		case kSStat_GotoCover:
			break;

		case kSStat_UseCover:
			{
				// If the node has a weapon string, and if the AI does not 
				// match the string AND if the AI has a weapon which defines a
				// transition at this location, then do a weapon change.
				
				HandlePotentialWeaponChange();
			}
			break;

		case kSStat_FailedComplete:
			m_hFailedNode = m_hNode;
			m_hNode = LTNULL;
			
			// Done covering, so charge!

			CoverOrCharge();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromCover::HandleStateAttackFromCover: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::CoverOrCharge
//
//	PURPOSE:	Find new cover or charge.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::CoverOrCharge()
{
	// If there are no other cover nodes, charge!

	m_bForceSearch = LTTRUE;

	m_pAI->SetState( kState_HumanCharge );
	if( HandleGoalAttractors() )
	{
		SetStateAttack();
	}

	m_bForceSearch = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::HandleStateCharge
//
//	PURPOSE:	Determine what to do when in state Charge.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromCover::HandleStateCharge()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if( m_pGoalMgr->IsGoalLocked(this) )
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromCover::HandleStateCharge: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromCover::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackFromCover::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	// If the AI was just shot, look 25% further for cover.
	// Maybe this should be buted.

	LTFLOAT fSearchFactor = 1.f;
	if( ( g_pLTServer->GetTime() - m_pAI->GetLastPainTime() ) < 1.f )
	{
		fSearchFactor = 1.25f;
	}

	return g_pAINodeMgr->FindNearestNodeFromThreat(m_pAI, eNodeType, vPos, m_hStimulusSource, fSearchFactor);
}