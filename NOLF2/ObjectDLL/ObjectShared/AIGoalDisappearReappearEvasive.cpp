// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDisappearReappearEvasive.cpp
//
// PURPOSE : AIGoalDisappearReappearEvasive implementation
//
// CREATED : 11/12/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalDisappearReappearEvasive.h"
#include "AIHumanStateDisappearReappear.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AICentralKnowledgeMgr.h"
#include "AIBrain.h"
#include "AIState.h"
#include "AIMovement.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDisappearReappearEvasive, kGoal_DisappearReappearEvasive);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDisappearReappearEvasive::CAIGoalDisappearReappearEvasive()
{
	m_bFirstTime = LTTRUE;
	m_bForceDisappear = LTFALSE;
	m_fReappearDistOverride = 0.f;
	m_fReappearDelay = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL( m_bFirstTime );
	SAVE_BOOL( m_bForceDisappear );
	SAVE_FLOAT( m_fReappearDistOverride );
	SAVE_FLOAT( m_fReappearDelay );
}

void CAIGoalDisappearReappearEvasive::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL( m_bFirstTime );
	LOAD_BOOL( m_bForceDisappear );
	LOAD_FLOAT( m_fReappearDistOverride );
	LOAD_FLOAT( m_fReappearDelay );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::ActivateGoal()
{
	if( m_bForceDisappear ||
		( g_pAICentralKnowledgeMgr->CountMatches( kCK_NextDisappearTime ) == 0 ) ||
		( g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_NextDisappearTime, LTNULL ) < g_pLTServer->GetTime() ) )
	{
		m_bForceDisappear = LTFALSE;
		ResetNextDisappearTime();
		super::ActivateGoal();

		if( m_pAI->GetState()->GetStateType() == kState_HumanDisappearReappear )
		{
			CAIHumanStateDisappearReappear* pState = (CAIHumanStateDisappearReappear*)m_pAI->GetState();

			if( m_fReappearDistOverride > 0.f )
			{
				pState->SetReappearDistOverride( m_fReappearDistOverride );
			}

			if( m_fReappearDelay > 0.f )
			{
				pState->SetReappearDelay( m_fReappearDelay );
			}
		}
	}
	else {
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::DeactivateGoal()
{
	super::DeactivateGoal();

	// Goals DisappearReappear and DisappearReappearEvasive should share timing info.

	CAIGoalAbstract* pGoal = m_pGoalMgr->FindGoalByType( kGoal_DisappearReappear );
	if( pGoal && ( m_fNextUpdateTime > pGoal->GetNextUpdateTime() ) )
	{
		pGoal->SetNextUpdateTime( m_fNextUpdateTime ); 
	}

	// All AIs using this goal share timing info.

	ResetNextDisappearTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::HandleStateDisappearReappear
//
//	PURPOSE:	Determine what to do when in state DisappearReappear.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::HandleStateDisappearReappear()
{
	// Always reappear with a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( ( m_pAI->GetState()->GetStateStatus() == kSStat_Moving ) &&
		( !m_pAI->GetPrimaryWeapon() ) &&
		( !pAIHuman->HasHolsterString() ) &&
		m_pAI->HasBackupHolsterString() )
	{
		m_pAI->SetHolsterString( m_pAI->GetBackupHolsterString() );
	}

	super::HandleStateDisappearReappear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::ResetNextDisappearTime
//
//	PURPOSE:	Reset the next valid disappearance time.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::ResetNextDisappearTime()
{
	// All AIs using this goal share timing info.
	
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

	LTFLOAT fNextDisappearTime = 0.f;
	if( pTemplate->fFrequencyMax > 0.f )
	{
		LTFLOAT fCurTime = g_pLTServer->GetTime();
		fNextDisappearTime = fCurTime + GetRandom(pTemplate->fFrequencyMin, pTemplate->fFrequencyMax);
	}

	g_pAICentralKnowledgeMgr->RemoveAllKnowledge( kCK_NextDisappearTime, m_pAI );
	g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_NextDisappearTime, m_pAI, g_pLTServer->HandleToObject(m_hStimulusSource), LTFALSE, fNextDisappearTime, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::RecalcImportance
//
//	PURPOSE:	Disappear and reappear with a weapon if disarmed.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappearEvasive::RecalcImportance()
{
	// If we're unarmed, disappear and reappear with a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( ( !m_pAI->GetPrimaryWeapon() ) &&
		( !pAIHuman->HasHolsterString() ) &&
		m_pAI->HasBackupHolsterString() )
	{
		m_bForceDisappear = LTTRUE;
		m_fNextUpdateTime = 0.f;
		SetCurToBaseImportance();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDisappearReappearEvasive::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "NOW") )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GoalDisappearReappearEvasive: NOW=1" ) );

		m_bForceDisappear = LTTRUE;
		m_bRequiresImmediateResponse = LTTRUE;
		SetCurToBaseImportance();
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "OVERRIDEDIST") )
	{
		m_fReappearDistOverride = (LTFLOAT)atof( szValue );
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GoalDisappearReappearEvasive: OVERRIDEDIST=%.2f", m_fReappearDistOverride ) );
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "DELAY") )
	{
		m_fReappearDelay = (LTFLOAT)atof( szValue );
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GoalDisappearReappearEvasive: DELAY=%.2f", m_fReappearDelay ) );
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappearEvasive::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDisappearReappearEvasive::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Do not use the super classes' HandleGoalSenseTrigger.

	if( CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Do not disappear while jumping.

		if( m_pAI->GetAIMovement()->IsMovementLocked() )
		{
			return LTFALSE;
		}

		// Check the distance.

		LTFLOAT fDistSqr = m_pAI->GetPosition().DistSqr( pSenseRecord->vLastStimulusPos );
		if(	( fDistSqr >= m_fDisappearDistMinSqr ) && ( fDistSqr <= m_fDisappearDistMaxSqr ) )
		{
			// Check if we're being aimed at.

			m_pAI->Target( m_hStimulusSource );
			if( m_pAI->GetBrain()->TargetIsAimingAtMe() )
			{
				// Do not disappear the very first time we're aimed at.

				if( m_bFirstTime )
				{
					m_bFirstTime = LTFALSE;
					SetRandomNextUpdate();
					return LTFALSE;
				}

				// Only activate if centralized timing says no one else has disappeared too recently.

				if( ( g_pAICentralKnowledgeMgr->CountMatches( kCK_NextDisappearTime ) == 0 ) ||
					( g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_NextDisappearTime, LTNULL ) < g_pLTServer->GetTime() ) )
				{
					return LTTRUE;
				}
			}
		}
	}

	return LTFALSE;
}
