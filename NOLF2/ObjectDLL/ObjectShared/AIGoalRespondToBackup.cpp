// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRespondToBackup.cpp
//
// PURPOSE : AIGoalRespondToBackup implementation
//
// CREATED : 11/15/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalRespondToBackup.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRespondToBackup, kGoal_RespondToBackup);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalRespondToBackup::CAIGoalRespondToBackup()
{
	m_hNodeBackup = LTNULL;
	m_cResponses = 0;
	m_cArrivalCount = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToBackup::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNodeBackup);
	SAVE_VECTOR(m_vEnemySeenPos);
	SAVE_DWORD(m_cResponses);
	SAVE_DWORD(m_cArrivalCount);
}

void CAIGoalRespondToBackup::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNodeBackup);
	LOAD_VECTOR(m_vEnemySeenPos);
	LOAD_DWORD(m_cResponses);
	LOAD_DWORD(m_cArrivalCount);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToBackup::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	// Run back to where enemy was last seen.

	m_hNodeBackup = LTNULL;			
	m_bRequiresUpdates = LTFALSE;

	m_pAI->SetState( kState_HumanGoto );
	CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
	pStateGoto->SetDest( m_vEnemySeenPos );
	pStateGoto->SetMovement( kAP_Run );
	pStateGoto->SetWeaponPosition( kAP_Up );

	// Keep track of the number of times we have responded to 
	// a call for backup.  If we have exceeded the limit set in
	// the node, we do not respond.

	++m_cResponses;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToBackup::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanAware:
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalRespondToBackup::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToBackup::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if(m_pAI->CanSearch())
			{
				SetStateSearch();
			}
			else {
				m_pGoalMgr->UnlockGoal(this);
				m_pAI->SetState( kState_HumanAware );
			}		
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGetBackup::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the state
//              of a backup node.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToBackup::RecalcImportance()
{
	// Goal activates when AI is stimulated by a backup node's AllyDistress, 
	// ally has arrived at (and unlocked) the node.

	if( m_hNodeBackup )
	{
		m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

		AINodeBackup* pNodeBackup = (AINodeBackup*)g_pLTServer->HandleToObject( m_hNodeBackup );

		if( !pNodeBackup->IsLocked() )
		{
			if( m_cArrivalCount >= pNodeBackup->GetArrivalCount() )
			{
				m_hNodeBackup = LTNULL;
				m_fCurImportance = 0.f;
				m_pAI->ResetBaseSenseFlags();
				return;
			}

			// Check if AI thinks ally is crying wolf.

			if( m_cResponses < pNodeBackup->GetCryWolfCount() )
			{
				m_vEnemySeenPos = pNodeBackup->GetEnemySeenPos();
				SetCurToBaseImportance();
				return;
			}
			else {
				m_pAI->PlaySound( kAIS_BackupCryWolf, LTFALSE );
				m_hNodeBackup = LTNULL;
				m_pAI->ResetBaseSenseFlags();
			}
		}
	}
	
	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToBackup::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalRespondToBackup::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if(super::HandleGoalSenseTrigger(pSenseRecord))
	{
		// Do not respond without a weapon.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
		{
			return LTFALSE;
		}

		// Reset the response counter if the enemy is actually seen.

		if( pSenseRecord->eSenseType == kSense_SeeEnemy )
		{
			m_cResponses = 0;
			m_fCurImportance = 0.f;

			if( m_hNodeBackup )
			{	
				m_hNodeBackup = LTNULL;
				m_pAI->ResetBaseSenseFlags();
				m_bRequiresUpdates = LTFALSE;
			}
		}

		else if( !m_pGoalMgr->IsCurGoal( this ) )
		{
			m_hNodeBackup = pSenseRecord->hLastStimulusTarget;

			AINodeBackup* pNodeBackup = (AINodeBackup*)g_pLTServer->HandleToObject( m_hNodeBackup );
			if( pNodeBackup )
			{
				m_cArrivalCount = pNodeBackup->GetArrivalCount();
			}

			m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );
			m_bRequiresUpdates = LTTRUE;
		}
	}

	// Always return false.  Only activate once the backup node is unlocked.

	return LTFALSE;
}

