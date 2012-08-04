//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalSentryMark.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	10.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIGOALSENTRYMARK_H__
#include "AIGoalSentryMark.h"		
#endif

#ifndef __AI_H__
#include "AI.h"
#endif

#ifndef _AI_BRAIN_H_
#include "AIBrain.h"
#endif

#ifndef __AIHUMANSTATESENTRYCHALLENGE_H__
#include "AIHumanStateSentryChallenge.h"
#endif

#ifndef __AI_UTILS_H__
#include "AIUtils.h"
#endif

#ifndef __AIGOAL_MGR_H__
#include "AIGoalMgr.h"
#endif

#ifndef __WEAPON_H__
#include "Weapon.h"
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif

// Forward declarations

// Globals

// Statics
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSentryMark, kGoal_SentryMark);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::[~]CAIGoalSentryMark()
//              
//	PURPOSE:	Creates/Destroys the goal
//              
//----------------------------------------------------------------------------
CAIGoalSentryMark::CAIGoalSentryMark()
{
	SetRelationChangeResponse(eIgnoreChange);
}
/*virtual*/ CAIGoalSentryMark::~CAIGoalSentryMark()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::Save()
//              
//	PURPOSE:	Save and load -- call the base, as there is nothing to save 
//				or load yet for this class.
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalSentryMark::Save(ILTMessage_Write *pMsg)
{
	CAIGoalAbstractStimulated::Save(pMsg);
}

/*virtual*/ void CAIGoalSentryMark::Load(ILTMessage_Read *pMsg)
{
	CAIGoalAbstractStimulated::Load(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::ActivateGoal()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::ActivateGoal()
{
	CAIGoalAbstractStimulated::ActivateGoal();

	if(m_pAI->GetCurrentWeapon())
	{
		SetStateSentryMark();
	}
	else
	{
		SetStateDraw();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::DeactivateGoal()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::DeactivateGoal()
{
	// 
	CAIGoalAbstractStimulated::DeactivateGoal();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::UpdateGoal()
//              
//	PURPOSE:	Handle the AIs state
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::UpdateGoal()
{
	CAIGoalAbstractStimulated::UpdateGoal();

	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
	case kState_HumanDraw:
		HandleStateDraw();
		break;

	case kState_HumanSentryChallenge:
		HandleStateSentryMark();
		break;

	case kState_HumanIdle:
		break;

	default:
		AIASSERT(0, m_pAI->m_hObject, "CAIGoalSentryChallenge::UpdateGoal: Unexpected State.");
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::HandleStateDraw()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::HandleStateDraw()
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
			SetStateSentryMark();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryMark::HandleStateSentryMark
//
//	PURPOSE:	Determine what to do when in state SentryMark.  Currently
//				only the entrance and exit states are handled.  The AIState
//				itself handles all other conditions.
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryMark::HandleStateSentryMark()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			m_pAI->SetState( kState_HumanIdle );
			break;

		case kSStat_FailedComplete:
			// Reset AIs default senses, from aibutes.txt.
			m_pGoalMgr->UnlockGoal( this );
			m_pAI->ResetBaseSenseFlags();
			m_fCurImportance = 0.f;
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default:
			AIASSERT(0, m_pAI->m_hObject, "CAIGoalSentryMark::HandleStateSentryMark: Unexpected State Status.");
			break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::HandleGoalSenseTrigger()
//              
//	PURPOSE:	Returns LTTRUE if the AI should enter this Goal, LTFALSE if 
//				it should not.  Distance and Visibility(?) are used for this
//				determination.
//              
//----------------------------------------------------------------------------
LTBOOL CAIGoalSentryMark::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) == LTFALSE)
	{
		return LTFALSE;
	}

	// If we are not currently doing this already...

	// [kml] 01/23/01 Jeff removed the CAIGoalAbstract::IsThisGoalAIsCurrentGoal()
	// function, so I inlined it here
	UBER_ASSERT( m_pAI, "Testing AIs Current goal without AI" );
	UBER_ASSERT( m_pAI, "Testing AIs Current goal without goal mgr" );
	const CAIGoalAbstract* pCurGoal = m_pAI->GetGoalMgr()->GetCurrentGoal();
	bool bIsThisGoalAIsCurrentGoal = ( (pCurGoal==NULL) ? (false) : (pCurGoal->GetGoalType()==GetGoalType()) );
	if ( bIsThisGoalAIsCurrentGoal )
	{
		return LTFALSE;
	}

	// Check range (default to 1000, allow the AI to optionally override it)

	// Get the distance to the source of the stimulus
	float fDistToStimulus;
	LTVector vOurPos;
	LTVector vStimulusPos;

	g_pLTServer->GetObjectPos( m_pAI->m_hObject, &vOurPos );
	g_pLTServer->GetObjectPos( pSenseRecord->hLastStimulusSource, &vStimulusPos );
	fDistToStimulus = (vOurPos - vStimulusPos).Length();

	// Check the distance to the stimulusSource
	if ( fDistToStimulus > m_pAI->GetSentryMarkDistMax() )
	{
		return LTFALSE;
	}

	// Require line of sight to enter goal?
	Warn( "CAIGoalSentryMark: Does AI require line of sight to enter this goal?" );
	// Record whatever triggered this goal.
	m_hStimulusSource = pSenseRecord->hLastStimulusSource;
	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::HandleNameValuePair()
//              
//              
//----------------------------------------------------------------------------
LTBOOL CAIGoalSentryMark::HandleNameValuePair(const char *szName,const char *szValue)
{
	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryChallenge::SetStateDraw()
//              
//	PURPOSE:	Transitions the AI to the Draw state
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::SetStateDraw()
{
	m_pGoalMgr->LockGoal( this );
	m_pAI->SetState( kState_HumanDraw );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryMark::SetStateSentryMark()
//              
//	PURPOSE:	Transitions the AI to the Challenge state
//              
//----------------------------------------------------------------------------
void CAIGoalSentryMark::SetStateSentryMark()
{
	m_pGoalMgr->LockGoal( this );

	AIASSERT( m_pAI->GetCurrentWeapon(), m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );
	AIASSERT( (static_cast<CAIHuman*>(m_pAI))->GetWeaponProp( m_pAI->GetCurrentWeapon()->GetWeaponData()) != kAP_Weapon3, m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );

	m_pAI->SetAwareness( kAware_Alert );

	// Start the Sentry Challenge State
	m_pAI->ClearAndSetState( kState_HumanSentryChallenge );
	CAIHumanStateSentryChallenge* pChallengeState = (CAIHumanStateSentryChallenge*)m_pAI->GetState();

	Challenge Failed;
	Failed.SetResult( Challenge::kCR_Fail );
	pChallengeState->SetChallengeResult( Failed );

	// This is what the AI is going to be challenging.
	pChallengeState->SetObjectToSentryChallenge( m_hStimulusSource );

	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kChallengeAction, kAPG_Posture, kAP_Stand );
	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kChallengeAction, kAPG_WeaponPosition, kAP_Up );
	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kChallengeAction, kAPG_Action, kAP_Challenge );

	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kPassedAction, kAPG_Posture, kAP_Stand );
	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kPassedAction, kAPG_Mood, kAP_Happy);

	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kFailedAction, kAPG_Posture, kAP_Stand );
	pChallengeState->SetAnimProp( CAIHumanStateSentryChallenge::kFailedAction, kAPG_Action, kAP_Mark);
}