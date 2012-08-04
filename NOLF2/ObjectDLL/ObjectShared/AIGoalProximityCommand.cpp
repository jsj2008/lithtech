// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalProximityCommand.cpp
//
// PURPOSE : AIGoalProximityCommand implementation
//
// CREATED : 6/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalProximityCommand.h"
#include "AIHuman.h"
#include "AIBrain.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalProximityCommand, kGoal_ProximityCommand);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalProximityCommand::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalProximityCommand::CAIGoalProximityCommand()
{
	m_fProximityDistSqr = 0.f;
	m_bSentCommand = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalProximityCommand::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalProximityCommand::InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime)
{
	super::InitGoal(pAI, fImportance, fTime);

	m_fProximityDistSqr = pAI->GetBrain()->GetAIData(kAIData_GoalProximityDist);
	m_fProximityDistSqr *= m_fProximityDistSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalProximityCommand::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalProximityCommand::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT( m_fProximityDistSqr );
	SAVE_BOOL( m_bSentCommand );
}

void CAIGoalProximityCommand::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT( m_fProximityDistSqr );
	LOAD_BOOL( m_bSentCommand );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalProximityCommand::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalProximityCommand::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		if( ( !m_bSentCommand ) && 
			( m_pAI->GetPosition().DistSqr( pSenseRecord->vLastStimulusPos ) <= m_fProximityDistSqr ) )
		{
			// Run the command once, until goal is reset.
			
			if( m_pAI->GetCommandProximityGoal() )
			{
				// We need to Queue the command, because sending it right away may
				// cause changes to goals while we're iterating over goals == crash!

				AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Running ProximityGoal command: %s", ::ToString( m_pAI->GetCommandProximityGoal() ) ) );
				m_pAI->QueueTriggerMsg( g_pLTServer->GetStringData( m_pAI->GetCommandProximityGoal() ) );
				m_bSentCommand = LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalProximityCommand::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalProximityCommand::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "RESET") )
	{
		m_bSentCommand = LTFALSE;
		return LTTRUE;
	}

	return LTFALSE;
}

