// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAlarm.cpp
//
// PURPOSE : AIGoalAlarm implementation
//
// CREATED : 10/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAlarm.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AI.h"
#include "AINode.h"
#include "AIHumanState.h"
#include "AITarget.h"
#include "AICentralKnowledgeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAlarm, kGoal_Alarm);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAlarm::CAIGoalAlarm()
{
	m_eWeaponPosition = kAP_Up;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAlarm::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vEnemySeenPos);
}

void CAIGoalAlarm::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vEnemySeenPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlarm::ActivateGoal()
{
	// Force alarm nodes to Run, regardless of how they were set.
	// Override sound, because goal already played alarm sound.

	AIASSERT( m_hNodeUseObject, m_pAI->m_hObject, "CAIGoalAlarm::ActivateGoal: No UseObject node." );
	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hNodeUseObject );
	AIASSERT( pNodeUseObject, m_pAI->m_hObject, "CAIGoalAlarm::ActivateGoal: UseObject node is NULL." );
	if( pNodeUseObject )
	{
		// Bail if someone else claimed this node at the same time.

		if( pNodeUseObject->IsLockedDisabledOrTimedOut() )
		{
			m_fCurImportance = 0.f;
			return;
		}

		pNodeUseObject->SetMovement( kAP_Run );
		pNodeUseObject->SetFirstSound( kAIS_None );
	}

	super::ActivateGoal();

	// Ignore all stimulus.

	m_pAI->SetCurSenseFlags( kSense_None );

	AIASSERT( m_hStimulusSource, m_pAI->m_hObject, "CAIGoalAlarm::ActivateGoal: Stimulus is NULL" );
	m_pAI->Target( m_hStimulusSource );

	m_pAI->SetAwareness( kAware_Alert );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlarm::UpdateGoal()
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

		default:
			super::UpdateGoal();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlarm::HandleStateUseObject()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Moving:
			{
				// Control global alarm frequency.
				// No AI can sound an alarm within 30 sec of the last.
	
				LTFLOAT fNextAlarmTime = g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_NextAlarmTime, LTNULL );
				if( ( fNextAlarmTime != 0.f ) &&
					( fNextAlarmTime > g_pLTServer->GetTime() ) )
				{
					m_fCurImportance = 0.f;
					return;
				}

				// Bail if target gets in the way of the path to the alarm.

				AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hNodeUseObject );
				if( ( !pNodeUseObject ) || 
					( pNodeUseObject->GetStatus( m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject() ) != kStatus_Ok ) )
				{
					m_fCurImportance = 0.f;
				}
			}
			break;

		case kSStat_StateComplete:
			{
				if( IsCharacter( m_pAI->GetTarget()->GetObject() ) )
				{
					CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_pAI->GetTarget()->GetObject() );
					g_pAICentralKnowledgeMgr->ReplaceKnowledge( kCK_NextAlarmTime, m_pAI, pChar, LTFALSE, g_pLTServer->GetTime() + 30.f, LTTRUE );
				}

				// Ignore senses other than SeeEnemy.

				m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile  );

				// Run back to where enemy was last seen.
			
				m_pAI->SetState( kState_HumanGoto );
				CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
				pStateGoto->SetDest( m_vEnemySeenPos );
				pStateGoto->SetMovement( kAP_Run );
				pStateGoto->SetWeaponPosition( kAP_Up );
			}
			break;

		default:
			super::HandleStateUseObject();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlarm::HandleStateGoto()
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
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAlarm::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::FindNearestAttractorNode
//
//	PURPOSE:	Find attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAlarm::FindNearestAttractorNode()
{
	// Control global alarm frequency.
	// No AI can sound an alarm within 30 sec of the last.

	LTFLOAT fNextAlarmTime = g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_NextAlarmTime, LTNULL );
	if( ( fNextAlarmTime != 0.f ) &&
		( fNextAlarmTime > g_pLTServer->GetTime() ) )
	{
		return LTNULL;
	}

	// Find an unowned node.

	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)FindGoalAttractors( LTFALSE, LTNULL );
	if( pNodeUseObject && 
		( pNodeUseObject->GetStatus( m_pAI->GetPosition(), m_hStimulusSource ) == kStatus_Ok ) )
	{
		return pNodeUseObject;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::SelectTriggeredAISound
//
//	PURPOSE:	Select appropriate AI sound.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAlarm::SelectTriggeredAISound(EnumAISoundType* peAISoundType)
{
	if( !m_pAI->IsPlayingDialogSound() )
	{
		*peAISoundType = kAIS_Alarm;
	}
	else {
		*peAISoundType = kAIS_None;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlarm::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAlarm::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( CAIGoalAbstract::HandleGoalSenseTrigger( pSenseRecord ) )
	{
		super::HandleGoalSenseTrigger( pSenseRecord );

		// Activate the goal if the enemy is seen.

		if( ( !m_pGoalMgr->IsCurGoal( this ) ) && 
			( pSenseRecord->eSenseType == kSense_SeeEnemy ) )
		{
			m_vEnemySeenPos = pSenseRecord->vLastStimulusPos;
	
			// Force an extra immediate call to HandleGoalAtractors.
			// This is because HandleGoalAttractors will not get called 
			// if another goal is currently locked.

			if( HandleGoalAttractors() )
			{
				return LTTRUE;
			}
		}

		// Deactivate the goal if alarm has already been sounded, and 
		// AI is looking for the target.

		else if( m_pGoalMgr->IsCurGoal( this ) && 
				( m_pAI->GetState()->GetStateType() != kState_HumanUseObject ) )
		{
			m_hStimulusSource = LTNULL;
			m_fCurImportance = 0.f;
		}
	}

	return LTFALSE;
}
