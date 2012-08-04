// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractStimulated.cpp
//
// PURPOSE : AIGoalAbstractStimulated abstract class implementation
//
// CREATED : 8/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAbstractStimulated.h"
#include "AISenseRecorderAbstract.h"
#include "AIStimulusMgr.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIUtils.h"
#include "AIMovement.h"
#include "ObjectRelationMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAbstractStimulated::CAIGoalAbstractStimulated()
{
	m_eSenseType = kSense_InvalidType;
	m_hStimulusSource = LTNULL;
	m_hStimulusTarget = LTNULL;
	m_fStimulusTime = 0.f;
	m_eOnRelationChangeAction = eNullImportance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalAbstractStimulated::InitGoal(CAI* pAI)
{
	super::InitGoal( pAI );

	m_hStimulusSource.SetReceiver( *this );
	m_hStimulusTarget.SetReceiver( *this );

	m_hRelationNotifier.SetObserver( this );
	m_hRelationNotifier.SetSubject( pAI->GetRelationMgr() );
}

void CAIGoalAbstractStimulated::InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime)
{
	super::InitGoal( pAI, fImportance, fTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractStimulated::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eSenseType);
	SAVE_HOBJECT(m_hStimulusSource);
	SAVE_HOBJECT(m_hStimulusTarget);
	SAVE_TIME(m_fStimulusTime);
	SAVE_BYTE(m_eOnRelationChangeAction);
}

void CAIGoalAbstractStimulated::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eSenseType, EnumAISenseType);
	LOAD_HOBJECT(m_hStimulusSource);
	LOAD_HOBJECT(m_hStimulusTarget);
	LOAD_TIME(m_fStimulusTime);
	LOAD_BYTE_CAST(m_eOnRelationChangeAction, RelationReactions);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractStimulated::ActivateGoal()
{
	super::ActivateGoal();

	// Only set a new response index if the AI is witnessing a new stimulus,
	// rather than seeing an ally who is disturbed.

	if( ( m_eSenseType != kSense_SeeAllyDisturbance ) && 
		( m_eSenseType != kSense_HearAllyDisturbance ) )
	{
		m_pAI->SetLastStimulusTime( m_fStimulusTime );
	}

	// Copy an ally's stimulus time.

	else if( m_hStimulusSource && IsAI( m_hStimulusSource ) ) 
	{
		CAI* pAlly = (CAI*)( g_pLTServer->HandleToObject( m_hStimulusSource ) );
		if( pAlly )
		{
			m_pAI->SetLastStimulusTime( pAlly->GetLastStimulusTime() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstractStimulated::HandleDamage(const DamageStruct& damage)
{
	// Stimuluated by whomever shot me.

	m_hStimulusSource = damage.hDamager;

	// Update the alarm level with new stimulus.

	AIBM_Stimulus* pStimulus = g_pAIButeMgr->GetStimulus( kStim_EnemyWeaponImpactVisible );
	if( pStimulus )
	{
		m_eSenseType		= pStimulus->eSenseType;
		m_hStimulusTarget	= LTNULL;
		m_pAI->IncrementAlarmLevel( pStimulus->nAlarmLevel );

		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "AI was shot!" ) );
	}

	// Always return false to allow normal damage handling.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstractStimulated::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Record whatever triggered this goal.
		m_eSenseType		= pSenseRecord->eSenseType;
		m_hStimulusSource	= pSenseRecord->hLastStimulusSource;
		m_hStimulusTarget	= pSenseRecord->hLastStimulusTarget;
		m_fStimulusTime		= pSenseRecord->fLastStimulationTime;
		return LTTRUE;
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstractStimulated::OnRelationshipBroken()
//              
//	PURPOSE:	When a relationship changed, give the goal a chance to adjust
//				itself to accordingly.  Currently, just make the goal 
//				unimportant
//              
//----------------------------------------------------------------------------
/*virtual*/ int CAIGoalAbstractStimulated::OnRelationChange(HOBJECT hObject)
{
	if( (m_eOnRelationChangeAction == eNullImportance) &&
		(m_hStimulusSource == hObject || m_hStimulusTarget == hObject) )
	{
		m_hStimulusTarget = NULL;
		m_hStimulusSource = NULL;
		SetCurImportance( 0.0f );
	}
	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstractStimulated::SetRelationChangeResponse()
//              
//	PURPOSE:	Function to set the Goals response to the receiving 
//				notification that the it has a handle to something which just
//				underwent a relationship change.
//              
//----------------------------------------------------------------------------
void CAIGoalAbstractStimulated::SetRelationChangeResponse(RelationReactions eReaction)
{
	m_eOnRelationChangeAction = eReaction;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::OnLinkBroken
//
//	PURPOSE:	Handles a deleted object reference.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractStimulated::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( &m_hStimulusSource == pRef || &m_hStimulusTarget == pRef )
		SetCurImportance( 0.0f );
}

