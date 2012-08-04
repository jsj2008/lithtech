//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalSentryChallenge.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	04.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Goes to idle when the sentry state is complete to prevent the
//				AI from looping the last animation played.  Draws the weapon if
//				it is required.
//              
//	NOTES:		Only one AI is allowed to scan at a time.  The current static 
//				locking method is absolutely ugly -- ought to be replaced with 
//				a structured system for handling it.
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "AIGoalSentryChallenge.h"		
#include "AnimationProp.h"
#include "AIHumanStateSentryChallenge.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AISenseRecorderAbstract.h"
#include "AITarget.h"
#include "Weapon.h"
#include "AIUtils.h"

// Forward declarations

// Globals

// Statics
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSentryChallenge, kGoal_SentryChallenge);

LTBOOL CAIGoalSentryChallenge::sm_bGlobalScanLocked = LTFALSE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //
CAIGoalSentryChallenge::CAIGoalSentryChallenge()
{
	SetRelationChangeResponse(eIgnoreChange);
	m_bHasLockedGoal = LTFALSE;
}

/*virtual*/ CAIGoalSentryChallenge::~CAIGoalSentryChallenge()
{
	if ( m_bHasLockedGoal )
	{
		sm_bGlobalScanLocked = LTFALSE;
		m_bHasLockedGoal = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryChallenge::Save(ILTMessage_Write *pMsg)
{
	CAIGoalAbstractStimulated::Save(pMsg);

	SAVE_BOOL(m_bHasLockedGoal);
}

void CAIGoalSentryChallenge::Load(ILTMessage_Read *pMsg)
{
	CAIGoalAbstractStimulated::Load(pMsg);

	LOAD_BOOL(m_bHasLockedGoal);

	// If the AI owned the goal, then they still own it -- globally.
	if ( m_bHasLockedGoal )
	{
		AIASSERT( sm_bGlobalScanLocked==LTFALSE, m_pAI->GetHOBJECT(), "sm_bGlobalScanLocked already owned!" );
		sm_bGlobalScanLocked = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryChallenge::ActivateGoal()
{
	CAIGoalAbstractStimulated::ActivateGoal();

	if(m_pAI->GetCurrentWeapon())
	{
		SetStateChallenge();
	}
	else
	{
		SetStateDraw();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryChallenge::DeactivateGoal()
{
	if ( m_bHasLockedGoal )
	{
		sm_bGlobalScanLocked = LTFALSE;
		m_bHasLockedGoal = LTFALSE;
	}

	CAIGoalAbstractStimulated::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryChallenge::UpdateGoal()
{
	CAIGoalAbstractStimulated::UpdateGoal();

	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
	case kState_HumanSentryChallenge:
		HandleStateSentryChallenge();
		break;

	case kState_HumanDraw:
		HandleStateDraw();
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
//	ROUTINE:	CAIGoalSentryChallenge::HandleStateDraw()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalSentryChallenge::HandleStateDraw()
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
			SetStateChallenge();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::HandleStateDraw: Unexpected State Status.");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::HandleStateSentryChallenge
//
//	PURPOSE:	Determine what to do when in state SentryChallenge.
//
// ----------------------------------------------------------------------- //
void CAIGoalSentryChallenge::HandleStateSentryChallenge()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if ( m_bHasLockedGoal )
			{
				sm_bGlobalScanLocked = LTFALSE;
				m_bHasLockedGoal = LTFALSE;
			}

			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			m_pAI->SetState( kState_HumanIdle );
			break;

		case kSStat_FailedComplete:
			if ( m_bHasLockedGoal )
			{
				sm_bGlobalScanLocked = LTFALSE;
				m_bHasLockedGoal = LTFALSE;
			}
			// Reset AIs default senses, from aibutes.txt.
			m_pGoalMgr->UnlockGoal( this );
			m_pAI->ResetBaseSenseFlags();
			m_fCurImportance = 0.f;
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default:
			AIASSERT(0, NULL, "CAIGoalSentryChallenge::HandleStateSentryChallenge: Unexpected State Status.");
			break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryChallenge::HandleGoalSenseTrigger()
//              
//	PURPOSE:	React to a Sense
//              
//----------------------------------------------------------------------------
LTBOOL CAIGoalSentryChallenge::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// If we are not currently doing this already...
		if ( m_pAI->GetAnimationContext()->IsLocked() )
		{
			return LTFALSE;
		}

		if ( m_pAI->GetState() && m_pAI->GetState()->GetStateType() == kState_HumanSentryChallenge )
		{
			return LTFALSE;
		}

		// Check range (default to 500, allow the AI to optionally override it)

		// Get the distance to the source of the stimulus
		float fDistToStimulus;
		LTVector vOurPos;
		LTVector vStimulusPos;

		g_pLTServer->GetObjectPos( m_pAI->m_hObject, &vOurPos );
		g_pLTServer->GetObjectPos( pSenseRecord->hLastStimulusSource, &vStimulusPos );
		fDistToStimulus = (vOurPos - vStimulusPos).Length();

		// Check the distance to the stimulusSource
		if ( fDistToStimulus < m_pAI->GetSentryChallengeScanDistMax() )
		{
			// Record whatever triggered this goal.
			m_hStimulusSource = pSenseRecord->hLastStimulusSource;
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSentryChallenge::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIGoalSentryChallenge::HandleNameValuePair(const char *szName, const char *szValue)
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
void CAIGoalSentryChallenge::SetStateDraw()
{
	m_pGoalMgr->LockGoal( this );
	m_pAI->SetState( kState_HumanDraw );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSentryChallenge::SetStateChallenge()
//              
//	PURPOSE:	Transitions the AI to the Challenge state
//              
//----------------------------------------------------------------------------
void CAIGoalSentryChallenge::SetStateChallenge()
{
	// Check to see if someone is already doing it.  If they are, then abort!
	if ( sm_bGlobalScanLocked == LTTRUE )
	{
		m_pGoalMgr->UnlockGoal( this );
		m_fCurImportance = 0.f;
		return;
	}
	else
	{
		sm_bGlobalScanLocked = LTTRUE;
		m_bHasLockedGoal = LTTRUE;
	}

	m_pGoalMgr->LockGoal( this );

	AIASSERT( m_pAI->GetCurrentWeapon(), m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );
	AIASSERT( (static_cast<CAIHuman*>(m_pAI))->GetWeaponProp( m_pAI->GetCurrentWeapon()->GetWeaponData()) != kAP_Weapon3, m_pAI->m_hObject, "CAIGoalSentryChallenge::SetStateChallenge: AI challenging without weapon" );

	m_pAI->SetAwareness( kAware_Suspicious );

	// Start the Sentry Challenge State
	m_pAI->SetState( kState_HumanSentryChallenge );
	CAIHumanStateSentryChallenge* pChallengeState = (CAIHumanStateSentryChallenge*)m_pAI->GetState();

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