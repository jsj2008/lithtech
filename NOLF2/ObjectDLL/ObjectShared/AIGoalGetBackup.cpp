// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGetBackup.cpp
//
// PURPOSE : AIGoalGetBackup implementation
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalGetBackup.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIStimulusMgr.h"
#include "AITarget.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGetBackup, kGoal_GetBackup);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalGetBackup::CAIGoalGetBackup()
{
	m_hNodeBackup = LTNULL;
	m_hFailedNode = NULL;
	m_eStimulusID = kStimID_Unset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNodeBackup);
	SAVE_HOBJECT(m_hFailedNode);
	SAVE_VECTOR(m_vEnemySeenPos);
	SAVE_DWORD(m_eStimulusID);
}

void CAIGoalGetBackup::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNodeBackup);
	LOAD_HOBJECT(m_hFailedNode);
	LOAD_VECTOR(m_vEnemySeenPos);
	LOAD_DWORD_CAST(m_eStimulusID, EnumAIStimulusID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNodeBackup != LTNULL, m_pAI->m_hObject, "CAIGoalGetBackup::ActivateGoal: AINodeBackup is NULL.");

	if( m_pAI->GetState()->GetStateType() != kState_HumanGetBackup )
	{
		// Ignore all senses.
		m_pAI->SetCurSenseFlags(kSense_None);

		m_pGoalMgr->LockGoal(this);

		AINodeBackup* pNode = (AINodeBackup*)g_pLTServer->HandleToObject(m_hNodeBackup);
		AIASSERT( pNode, m_pAI->m_hObject, "CAIGoalGetBackup::ActivateGoal: Node is NULL" );
		if( pNode )
		{
			m_pAI->SetState( kState_HumanGetBackup );

			CAIHumanStateGetBackup* pStateGetBackup = (CAIHumanStateGetBackup*)(m_pAI->GetState());
			pStateGetBackup->SetDest( pNode );
			pStateGetBackup->SetEnemySeenPos( m_vEnemySeenPos );

			if( m_eStimulusID )
			{
				// Remove the stimulus.
	
				g_pAIStimulusMgr->RemoveStimulus( m_eStimulusID );
			}

			m_eStimulusID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDistressVisible, m_pAI->m_hObject, pNode->m_hObject, CAIStimulusRecord::kDynamicPos_TrackTarget, pNode->GetStimulusRadius() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::DeactivateGoal()
{
	super::DeactivateGoal();

	if( m_eStimulusID )
	{
		// Remove the stimulus.
	
		g_pAIStimulusMgr->RemoveStimulus( m_eStimulusID );
		m_eStimulusID = kStimID_Unset;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanGetBackup:
			HandleStateGetBackup();
			break;

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
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGetBackup::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::HandleStateGetBackup
//
//	PURPOSE:	Determine what to do when in state GetBackup.
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::HandleStateGetBackup()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			{
				// Bail if target gets in the way of the path to the node.

				AINodeBackup* pNodeBackup = (AINodeBackup*)g_pLTServer->HandleToObject( m_hNodeBackup );
				if( ( !pNodeBackup ) || 
					( pNodeBackup->GetStatus( m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject() ) != kStatus_Ok ) )
				{
					m_fCurImportance = 0.f;
				}
			}
			break;

		case kSStat_FailedComplete:
			m_hFailedNode = m_hNodeBackup;
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			{
				AINodeBackup* pNodeBackup = (AINodeBackup*)g_pLTServer->HandleToObject( m_hNodeBackup );
				if( pNodeBackup )
				{
					pNodeBackup->IncrementArrivalCount();
				}


				m_hNodeBackup = LTNULL;

				uint32 cResponders = g_pAIStimulusMgr->GetNumResponders( m_eStimulusID );

				if( cResponders == 0 )
				{
					m_pAI->PlaySound( kAIS_BackupFail, LTFALSE );
				}
				else {
					m_pAI->PlaySound( kAIS_OrderBackup, LTFALSE );
				}

				// Only pay attention to really important senses.
				m_pAI->SetCurSenseFlags(
					kSense_SeeEnemy					|
					kSense_HearEnemyWeaponFire		|
					kSense_HearEnemyWeaponImpact	|
					kSense_HearAllyWeaponFire		|
					kSense_HearAllyPain				|
					kSense_SeeDangerousProjectile	|
					kSense_SeeCatchableProjectile);

				// Run back to where enemy was last seen.
			
				m_pAI->SetState( kState_HumanGoto );
				CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
				pStateGoto->SetDest( m_vEnemySeenPos );
				pStateGoto->SetMovement( kAP_Run );
				pStateGoto->SetWeaponPosition( kAP_Up );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGetBackup::HandleStateGetBackup: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalGetBackup::HandleStateGoto()
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
//	ROUTINE:	CAIGoalGetBackup::SelectTriggeredAISound
//
//	PURPOSE:	Select appropriate AI sound.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalGetBackup::SelectTriggeredAISound(EnumAISoundType* peAISoundType)
{
	*peAISoundType = kAIS_FindBackup;
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalGetBackup::HandleGoalAttractors()
{
	AINode* pNode = LTNULL;

	if(m_hStimulusSource != LTNULL)
	{
		if(m_hNodeBackup == LTNULL)
		{
			// Lock the failed cover node, so that we don't try to use it again.
			BlockAttractorNodeFromSearch( m_hFailedNode );

			// Look for a backup node.
			// If one is found, this goal activates.
			pNode = super::HandleGoalAttractors();
			if(pNode != LTNULL)	
			{
				// Do not go to a backup node if we are already in its inner radius.

				AINodeBackup* pNodeBackup = (AINodeBackup*)pNode;
				if( ( pNodeBackup->GetStimulusRadiusSqr() > pNodeBackup->GetPos().DistSqr( m_pAI->GetPosition() ) ) ||
					( pNodeBackup->GetStatus( m_pAI->GetPosition(), m_hStimulusSource ) != kStatus_Ok ) )
				{
					m_fCurImportance = 0.f;
					m_hNodeBackup = LTNULL;
					pNode = LTNULL;
				}
				else {
					m_hNodeBackup = pNode->m_hObject;
					m_hStimulusSource = LTNULL;
				}
			}

			// If we locked a node prior to the search, unlock it.
			UnblockAttractorNodeFromSearch( m_hFailedNode );
		}
	}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGetBackup::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalGetBackup::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// If this is already the current goal, and we've already reached our backup
	// node, exit the goal if anything is sensed.

	if( m_pGoalMgr->IsCurGoal(this) && 
		( m_pAI->GetState()->GetStateType() != kState_HumanGetBackup ) &&
		( pSenseRecord->nLastStimulusAlarmLevel >= 10 ) )
	{
		m_pGoalMgr->UnlockGoal(this);
		m_fCurImportance = 0.f;
	}

	else if(super::HandleGoalSenseTrigger(pSenseRecord))
	{
		m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );

		// Record the AIs position, because we know we can find a path to here.

		m_vEnemySeenPos = m_pAI->GetPosition();

		// Force an extra immediate call to HandleGoalAtractors.
		// This is because HandleGoalAtractors will not get called 
		// if another goal is currently locked.

		if( HandleGoalAttractors() != LTNULL )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

