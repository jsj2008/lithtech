// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractUseObject.cpp
//
// PURPOSE : AIGoalAbstractUseObject implementation
//
// CREATED : 10/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAbstractUseObject.h"
#include "AISenseRecorderAbstract.h"
#include "AIHumanState.h"
#include "AINode.h"
#include "AINodeMgr.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AIHuman.h"
#include "AIUtils.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAbstractUseObject::CAIGoalAbstractUseObject()
{
	m_fStimTime	= LTNULL;

	m_bRequireBareHands = LTFALSE;
	m_bAllowDialogue = LTFALSE;
	m_bTurnOnLights = LTFALSE;
	m_bTurnOffLights = LTFALSE;
	m_bHolstered = LTFALSE;
	m_bLockedNode = LTFALSE;
	m_bPlayedSpecialDeathAnim = LTFALSE;

	m_eWeaponPosition = kAP_Down;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstractUseObject::~CAIGoalAbstractUseObject()
//              
//	PURPOSE:	Clean up any node locks still left active.
//              
//----------------------------------------------------------------------------
/*virtual*/ CAIGoalAbstractUseObject::~CAIGoalAbstractUseObject()
{
	if ( m_hNodeUseObject && m_bLockedNode )
	{
		AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
		AIASSERT( pNodeUseObject && pNodeUseObject->IsLocked(), m_pAI->m_hObject, "CAIGoalAbstractUseObject::DeactivateGoal: Node is NULL or not locked" );
		if( pNodeUseObject )
		{
			pNodeUseObject->Unlock( m_pAI->m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNodeUseObject);
	SAVE_HOBJECT(m_hLastNodeUseObject);
	SAVE_TIME(m_fStimTime);
	SAVE_DWORD(m_eWeaponPosition);
	SAVE_BOOL(m_bRequireBareHands);
	SAVE_BOOL(m_bAllowDialogue);
	SAVE_BOOL(m_bTurnOnLights);
	SAVE_BOOL(m_bTurnOffLights);
	SAVE_BOOL(m_bHolstered);
	SAVE_BOOL(m_bLockedNode);
	SAVE_BOOL(m_bPlayedSpecialDeathAnim);
}

void CAIGoalAbstractUseObject::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNodeUseObject);	
	LOAD_HOBJECT(m_hLastNodeUseObject);
	LOAD_TIME(m_fStimTime);
	LOAD_DWORD_CAST(m_eWeaponPosition, EnumAnimProp);
	LOAD_BOOL(m_bRequireBareHands);
	LOAD_BOOL(m_bAllowDialogue);
	LOAD_BOOL(m_bTurnOnLights);
	LOAD_BOOL(m_bTurnOffLights);
	LOAD_BOOL(m_bHolstered);
	LOAD_BOOL(m_bLockedNode);
	LOAD_BOOL(m_bPlayedSpecialDeathAnim);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNodeUseObject != LTNULL, m_pAI->m_hObject, "CAIGoalAbstractUseObject::ActivateGoal: AINodeUseObject is NULL.");

	m_bHolstered = LTFALSE;

	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
	AIASSERT( pNodeUseObject, m_pAI->m_hObject, "CAIGoalAbstractUseObject::ActivateGoal: AINodeUseObject is NULL.");

	// If the node was locked by someone else, bail.

	if( ( !pNodeUseObject ) || ( pNodeUseObject->IsLockedDisabledOrTimedOut() ) )
	{
		CompleteUseObject();
		return;
	}

	// Lock the node during the entire duration of the goal.
	// This includes time spent holstering and drawing weapons.

	if( pNodeUseObject )
	{
		pNodeUseObject->Lock( m_pAI->m_hObject );
		m_bLockedNode = LTTRUE;
	}

	m_pGoalMgr->LockGoal(this);

	SetStateUseObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::SetStateUseObject
//
//	PURPOSE:	Set state to UseObject.
//
// ----------------------------------------------------------------------- //

EnumAIStateType CAIGoalAbstractUseObject::GetUseObjectState()
{
	return kState_HumanUseObject; 
}

void CAIGoalAbstractUseObject::SetStateUseObject()
{
	m_pAI->SetState( GetUseObjectState() );

	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
	if( !pNodeUseObject )
	{
		AIASSERT( NULL, m_pAI->m_hObject, "CAIGoalAbstractUseObject::SetStateUseObject: AINodeUseObject is NULL.");
		return;
	}

	CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
	pStateUseObject->StateHandlesNodeLocking( LTFALSE );

	// Bail if there is any problem setting the node.

	if( !pStateUseObject->SetNode(pNodeUseObject) )
	{
		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Unable to set UseObject node %s.", ::ToString( pNodeUseObject->GetName() ) ) );
		m_hLastNodeUseObject = pNodeUseObject->m_hObject;
		CompleteUseObject();
		return;
	}

	pStateUseObject->SetWeaponPosition(m_eWeaponPosition);
	pStateUseObject->SetRequireBareHands( m_bRequireBareHands );
	pStateUseObject->SetAllowDialogue(m_bAllowDialogue);
	pStateUseObject->TurnOnLights( m_bTurnOnLights );
	pStateUseObject->TurnOffLights( m_bTurnOffLights );

	// Find command string for AINodeType matching UseObject.
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::ActivateGoal: Goal has no attractors.");

	HSTRING hstrCmd;
	for(uint32 iAttractor=0; iAttractor < pTemplate->cAttractors; ++iAttractor)
	{
		hstrCmd = pNodeUseObject->GetSmartObjectCommand(pTemplate->aAttractors[iAttractor]);
		if(hstrCmd != LTNULL)
		{
			pStateUseObject->SetSmartObjectCommand(hstrCmd);
			break;
		}
	}

	// Ensure that node is setup to be requested type of attractor.

	if( !hstrCmd )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::ActivateGoal: No command string found for attractors.");
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::DeactivateGoal()
{
	super::DeactivateGoal();

	// Unlock the node after the entire duration of the goal.
	// This includes time spent holstering and drawing weapons.

	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
	AIASSERT( pNodeUseObject && pNodeUseObject->IsLocked(), m_pAI->m_hObject, "CAIGoalAbstractUseObject::DeactivateGoal: Node is NULL or not locked" );
	if( m_bLockedNode && pNodeUseObject )
	{
		pNodeUseObject->Unlock( m_pAI->m_hObject );
		m_bLockedNode = LTFALSE;
	}

	// If we get drawn away to do something else, forget about 
	// the node, so that we will search again from where ever we
	// end up.
	// Do not set m_hLastNodeUseObject, because we may not
	// have finished using the object.

	ClearUseObjectNode();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::ClearUseObjectNode()
{
	// If there is no node, then importance MUST be zero.

	m_hNodeUseObject = LTNULL;
	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanHolster:
			HandleStateHolster();
			break;

		case kState_HumanUseObject:
			HandleStateUseObject();
			break;

		case kState_HumanIdle:
			if( !m_pAI->GetAnimationContext()->IsTransitioning() )
			{
				/*
				if( m_bHolstered )
				{
					m_pAI->SetState( kState_HumanDraw );
					m_pAI->GetState()->PlayFirstSound( LTFALSE );
				}
				else {
					CompleteUseObject();
				}
				*/

				CompleteUseObject();
			}
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: ASSERT(!"CAIGoalAbstractUseObject::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::HandleStateHolster
//
//	PURPOSE:	Determine what to do when in state Holster.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::HandleStateHolster()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::HandleStateHolster: Failed to holster weapon." );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			SetStateUseObject();
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalAbstractUseObject::HandleStateHolster: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			CompleteUseObject();
			break;

		case kSStat_StateComplete:
			CompleteUseObject();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::HandleStateUseObject()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Waiting:
			break;

		case kSStat_Moving:
			break;

		case kSStat_HolsterWeapon:
			m_bHolstered = LTTRUE;
			m_pAI->SetState( kState_HumanHolster );
			break;

		case kSStat_PathComplete:
			break;

		case kSStat_StateComplete:
			{
				// Stand up.

				CAIHumanState* pState = (CAIHumanState*)(m_pAI->GetState());
				pState->SetPose( kAP_Stand );

				// Go to idle, to ensure playing a xxx_out transition animation.
				// (Transitions are not played for locked animations).

				m_pAI->SetState( kState_HumanIdle );
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalAbstractUseObject::HandleStateAbstractUseObject: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::CompleteUseObject
//
//	PURPOSE:	Wrap up the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractUseObject::CompleteUseObject()
{
	// Do not allow the AI to immediately start using the same
	// node again, if the node does not have a reactivation time.

	AINodeUseObject* pNodeUseObject = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
	if( pNodeUseObject && ( pNodeUseObject->GetNodeReactivationTime() == 0.f ) )
	{
		m_hLastNodeUseObject = m_hNodeUseObject;				
	}

	m_fCurImportance = 0.f;
	m_pGoalMgr->UnlockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::GetAlternateDeathAnimation
//
//	PURPOSE:	Give goal a chance to choose an appropriate death animation.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIGoalAbstractUseObject::GetAlternateDeathAnimation()
{
	// Were killed in the middle of an activity?

	// jrg - 7/29/02 - don't count the activity if we haven't fully transitioned yet.
	if( ( m_pAI->GetState()->GetStateType() == kState_HumanUseObject ) &&
		( m_pAI->GetState()->GetStateStatus() == kSStat_PathComplete ) && 
		(!m_pAI->GetAnimationContext()->IsTransitioning())	) 
	{
		CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
		EnumAnimProp eActivity = pStateUseObject->GetActivity();

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, pStateUseObject->GetPose() );
		animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
		animProps.Set( kAPG_WeaponPosition, kAP_Lower );
		animProps.Set( kAPG_Awareness, eActivity );
		animProps.Set( kAPG_Action, kAP_Death );

		// Find a death animation for this activity.
		if( m_pAI->GetAnimationContext()->AnimationExists( animProps ))
		{
			m_bPlayedSpecialDeathAnim = LTTRUE;
			return m_pAI->GetAnimationContext()->GetAni( animProps );
		}
	}

	// No alternate.

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAbstractUseObject::HandleGoalAttractors()
{
	// Do not search for attractors if goal is already active.
	// Do not search on first update, to allow commands a chance to disable nodes.
	// Do not search if AI has any damage flags set (e.g. sleeping damage).

	if(	m_pGoalMgr->IsCurGoal(this) || 
		m_pAI->IsFirstUpdate() ||
		m_pAI->GetDamageFlags() )
	{
		return LTNULL;
	}

	// If this goal reacts to stimulus, check if it has been too 
	// long since stimulation.

	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	if( ( pTemplate->flagSenseTriggers != kSense_None )
		&& ( !m_hStimulusSource ) )
	{
		return LTNULL;
	}

	// Lock the last UseObject node, so that we don't try to use it again.
	BlockAttractorNodeFromSearch( m_hLastNodeUseObject );

	// Find the nearest attractor.
	AINode* pNode = FindNearestAttractorNode();
	if(pNode != LTNULL)
	{
		AIASSERT(pNode->GetType() == kNode_UseObject, m_pAI->m_hObject, "CAIGoalAbstractUseObject::HandleGoalAttractors: AINode is not of type UseObject.");

		AINodeUseObject* pNodeUseObject = (AINodeUseObject*)pNode;
		if( pNodeUseObject->HasObject() && !pNodeUseObject->GetHObject() )
		{
			pNode = LTNULL;
			m_fCurImportance = 0.f;
			AIASSERT( 0, pNodeUseObject->m_hObject, "CAIGoalAbstractUseObject::HandleGoalAttractors: AINodeUseObject points to invalid object" );
		}
		else if( pNodeUseObject->IsOneWay() && ( pNodeUseObject->GetForward().Dot( m_pAI->GetForwardVector() ) < 0.0f ) )
		{
			pNode = LTNULL;
			m_fCurImportance = 0.f;
		}
		else {
			AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Setting node: %s", ::ToString( pNode->GetName() ) ) );
			m_hNodeUseObject = pNode->m_hObject;
			SetCurToBaseImportance();
		}
	}

	if( !pNode )
	{
		ClearUseObjectNode();
		m_hStimulusSource = LTNULL;
	}

	// If we locked a node prior to the search, unlock it.
	UnblockAttractorNodeFromSearch( m_hLastNodeUseObject );

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::FindNearestAttractorNode
//
//	PURPOSE:	Find attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAbstractUseObject::FindNearestAttractorNode()
{
	// Do not use UseObject nodes while AI owns a talk node (and has a Talk goal).

	if( g_pAINodeMgr->FindOwnedNode( kNode_Talk, m_pAI->m_hObject ) )
	{
		return LTNULL;
	}

	// Find a guard node owned by this AI.

	AINode* pGuardNode = g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject );
	if( pGuardNode )
	{
		// Only find attractor nodes owned by the owner node.

		return FindGoalAttractors( LTTRUE, pGuardNode->m_hObject );
	}

	// Find an unowned node.

	return FindGoalAttractors( LTFALSE, LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractUseObject::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstractUseObject::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		m_fStimTime = g_pLTServer->GetTime();
	}

	// Always return false, and let the attractor handle activation.

	return LTFALSE;
}
