// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTalk.cpp
//
// PURPOSE : AIGoalTalk implementation
//
// CREATED : 7/24/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalTalk.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AI.h"
#include "AINodeGuard.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalTalk, kGoal_Talk);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalTalk::CAIGoalTalk()
{
	m_eMood				= kAP_Happy;
	m_eMovement			= kAP_Walk;
	m_hstrGesture		= LTNULL;
	m_fFaceTime			= 15.0f;
	m_eNodeType			= kNode_Talk;

	m_bDisposableDialogue = LTFALSE;

	ResetDialogue();
}

CAIGoalTalk::~CAIGoalTalk()
{
	FREE_HSTRING( m_hstrGesture );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eMood);
	SAVE_DWORD(m_eMovement);
	SAVE_HSTRING(m_hstrGesture);
	SAVE_FLOAT(m_fFaceTime);
	SAVE_BOOL(m_bStartedDialog);
	SAVE_BOOL(m_bInTalkPosition);
	SAVE_BOOL(m_bRetriggerDialogue);
	SAVE_BOOL(m_bDisposableDialogue);
	SAVE_BOOL(m_bTalking);
	SAVE_HOBJECT(m_hDialogue);
}

void CAIGoalTalk::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eMood, EnumAnimProp);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_HSTRING(m_hstrGesture);
	LOAD_FLOAT(m_fFaceTime);
	LOAD_BOOL(m_bStartedDialog);
	LOAD_BOOL(m_bInTalkPosition);
	LOAD_BOOL(m_bRetriggerDialogue);
	LOAD_BOOL(m_bDisposableDialogue);
	LOAD_BOOL(m_bTalking);
	LOAD_HOBJECT(m_hDialogue);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::ActivateGoal()
{
	super::ActivateGoal();

	m_pGoalMgr->LockGoal( this );

	// GoalGuard starts the AI in the Idle state.
	// If a dialogue has already been triggered,
	// the AI may need to be in another state.

	if( m_bStartedDialog )
	{
		// Walk to the Talk node.

		if( !m_bInTalkPosition )
		{
			GotoNode();
		}

		// Animation talking.

		else if( m_bTalking )
		{
			SetStateTalk();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::DeactivateGoal()
{
	super::DeactivateGoal();

	// If another goal activated while the AI was in
	// conversation, stop the dialog and cleanup the
	// dialog object.

	if( m_bStartedDialog && m_pAI->IsControlledByDialogue() )
	{
		m_pAI->StopDialogue();
	}

	// Cut off gesture animations.

	if( m_pAI->GetAnimationContext()->IsPlayingSpecial() )
	{
		m_pAI->GetAnimationContext()->ClearSpecial();
	}
}	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::ResetDialogue
//
//	PURPOSE:	Reset dialogue.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::ResetDialogue()
{
	m_bStartedDialog	= LTFALSE;
	m_bTalking			= LTFALSE;
	m_bInTalkPosition	= LTFALSE;
	m_bRetriggerDialogue= LTFALSE;
	m_hDialogue			= LTNULL;

	SetGuardNode( LTNULL );

	if( m_pGoalMgr && m_pGoalMgr->IsGoalLocked( this ) )
	{
		m_pGoalMgr->UnlockGoal( this );
	}

	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		// If state is idle, and a guard node is set, set state to goto.
		case kState_HumanIdle:
			HandleStateIdle();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanTalk:
			HandleStateTalk();
			break;

		// State should only be idle or goto.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalTalk::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::HandleStateIdle
//
//	PURPOSE:	Determine what to do when in state Idle.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::HandleStateIdle()
{
	if( m_bInRadius )
	{
		HandleInRadius();
	}
	else if( !m_bStartedDialog )
	{
		super::HandleStateIdle();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::HandleStateTalk
//
//	PURPOSE:	Determine what to do when in state Talk.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::HandleStateTalk()
{
	if( m_hstrGesture )
	{
		CAIHumanStateTalk* pTalkState = (CAIHumanStateTalk*)(m_pAI->GetState());
		pTalkState->SetGesture( m_hstrGesture );
		FREE_HSTRING( m_hstrGesture );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::GotoNode
//
//	PURPOSE:	Goto specified node.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::GotoNode()
{
	super::GotoNode();

	if( m_pAI->GetState()->GetStateType() == kState_HumanGoto )
	{
		CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
		pStateGoto->SetMovement( m_eMovement );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_bInRadius = LTTRUE;
			HandleInRadius();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalTalk::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::HandleInRadius
//
//	PURPOSE:	AI is ready to start a dialogue.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::HandleInRadius()
{
	// Flag that AI is in position.

	if( m_bStartedDialog )
	{
		m_bInTalkPosition = LTTRUE;
	}

	// If Retrigger is TRUE, retrigger the dialogue object,
	// now that the AI is in position.

	else if( m_bRetriggerDialogue && m_hDialogue )
	{
		SendTriggerMsgToObject( m_pAI, m_hDialogue, LTFALSE, "On" );
	}

	// Default behavior.  AI can do other things (work) while in radius.

	else {
		m_fCurImportance = m_fMinImportance;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::RequestDialogue
//
//	PURPOSE:	Is AI ready to talk?
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalTalk::RequestDialogue(HOBJECT hDialogue)
{
	// Record which dialogue object requested to start.

	m_hDialogue = hDialogue;

	// Do not start if the AI is not relaxed.

	if( m_pAI->GetAwareness() != kAware_Relaxed )
	{
		return LTFALSE;
	}

	// Safeguard to ensure AI knows it is at the Talk node if they are at 
	// the same position but different Y elevations.

	AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject(m_hGuardNode);
	if( ( !m_bInRadius ) &&
		( pGuardNode ) &&
		( pGuardNode->GetRadiusSqr() == 0.f ) )
	{
		if( ( m_pAI->GetPosition().x == pGuardNode->GetPos().x ) &&
			( m_pAI->GetPosition().z == pGuardNode->GetPos().z ) &&
			( pGuardNode->GetPos().y >= m_pAI->GetPosition().y - m_pAI->GetDims().y ) &&
			( pGuardNode->GetPos().y <= m_pAI->GetPosition().y + m_pAI->GetDims().y ) )
		{
			m_bInRadius = LTTRUE;
		}
	}

	// Do not start if AI is not in radius.

	return m_bInRadius;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::StartDialogue
//
//	PURPOSE:	Start a dialogue.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::StartDialogue()
{
	// Flag that the process has been kicked off.
	// AI may need to get in position before starting to talk.

	if( !m_bStartedDialog )
	{
		SetCurToBaseImportance();
		if( m_pGoalMgr->IsCurGoal( this ) )
		{
			GotoNode();
		}

		m_bStartedDialog = LTTRUE;
		m_bInTalkPosition = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::StartTalking
//
//	PURPOSE:	Start talking.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::StartTalking()
{
	if( m_pGoalMgr->IsCurGoal( this ) && 
		( m_pAI->GetState()->GetStateType() != kState_HumanTalk ) )
	{
		SetStateTalk();
	}

	m_bTalking = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::StopTalking
//
//	PURPOSE:	Stop talking.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::StopTalking()
{
	m_bTalking = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::SetStateTalk
//
//	PURPOSE:	Start talking.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::SetStateTalk()
{
	m_pAI->SetState( kState_HumanTalk );
	CAIHumanStateTalk* pTalkState = (CAIHumanStateTalk*)(m_pAI->GetState());
	pTalkState->SetMood(m_eMood);
	pTalkState->SetFace(m_hTarget);
	pTalkState->SetFaceTime(m_fFaceTime);
	pTalkState->SetGuardNode(m_hGuardNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::FindGuardNode
//
//	PURPOSE:	Find nearest guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::FindGuardNode()
{
	// Intentionally override base-class without finding a node.
	// Node must be set manually thru name value pair.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the distance
//              to the guard node. Importance is high after conversation
//              has started.
//
// ----------------------------------------------------------------------- //

void CAIGoalTalk::RecalcImportance()
{
	if( !m_bStartedDialog )
	{
		super::RecalcImportance();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalTalk::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// If the dialogue was disposable, and AI sensed anything,
	// remove the Talk goal.

	if( m_bDisposableDialogue  )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Disposing Talk Goal." ) );
		m_pGoalMgr->DeleteGoalNextUpdate( kGoal_Talk );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTalk::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalTalk::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if( super::HandleNameValuePair(szName, szValue) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(szName, "MOOD") )
	{
		m_eMood = CAnimationMgrList::GetPropFromName( szValue );
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "FACETIME") )
	{
		m_fFaceTime = (LTFLOAT)atof(szValue);
		return LTTRUE;
	}

	// Retrigger tells the AI to retrigger the dialogue object
	// once he is in the radius.

	else if ( !_stricmp(szName, "RETRIGGER") )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalTalk: RETRIGGER=1" ) );
		m_bRetriggerDialogue = IsTrueChar(*szValue);
		return LTTRUE;
	}

	// Disposable tells the AI to remove the Talk goal
	// once he is out of the radius.

	else if ( !_stricmp(szName, "DISPOSABLE") )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalTalk: DISPOSABLE=1" ) );
		m_bDisposableDialogue = IsTrueChar(*szValue);
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "GESTURE") )
	{
		FREE_HSTRING( m_hstrGesture );
		m_hstrGesture = g_pLTServer->CreateString( szValue );
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "MOVEMENT") )
	{
		m_eMovement = CAnimationMgrList::GetPropFromName( szValue );
		return LTTRUE;
	}

	return LTFALSE;
}

