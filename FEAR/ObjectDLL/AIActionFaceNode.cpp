// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFaceNode.cpp
//
// PURPOSE : AIActionFaceNode class implementation
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFaceNode.h"
#include "AIStateAnimate.h"
#include "ServerSoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFaceNode, kAct_FaceNode );

#define TURNING_SOUND	"TurningSound"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFaceNode::CAIActionFaceNode()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFaceNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// The node to use is a variable. Which node depends on the goal 
	// or action we are trying to satisfy.

	// No preconditions.

	// Set effects.
	// Using the object.  

	m_wsWorldStateEffects.SetWSProp( kWSK_UsingObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFaceNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Find which node we are facing from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}

	// Set animate state.

	pAI->SetState( kState_Animate );

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, kAP_ACT_Alert );
	
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, LOOP );

	// Face the node.

	pAI->GetAIBlackBoard()->SetBBFaceObject( pProp->hWSValue );

	// We are departing a previous node.

	pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, 0 );

	// We have a new destination.

	pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Set );

	// Loop turning sound.

	HRECORD hRecord = g_pAIDB->GetMiscRecordLink( TURNING_SOUND );
	const char* pszSoundFile = g_pLTDatabase->GetRecordName( hRecord );
	HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pszSoundFile );

	HLTSOUND hLoopSound = g_pServerSoundMgr->PlayDBSoundFromObject( pAI->m_hObject, hSR,
		-1.0f, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE,
		SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
		DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);

	pAI->GetAIBlackBoard()->SetBBLoopingSound( hLoopSound );

	// Head tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFaceNode::DeactivateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Kill the looping turning sound.

	HLTSOUND hLoopSound = pAI->GetAIBlackBoard()->GetBBLoopingSound();
	if( hLoopSound )
	{
		g_pLTServer->SoundMgr()->KillSound( hLoopSound );
		pAI->GetAIBlackBoard()->SetBBLoopingSound( NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionFaceNode::IsActionComplete( CAI* pAI )
{
	// Facing was just set.

	if( pAI->GetAIBlackBoard()->GetBBFaceType() != kFaceType_None )
	{
		return false;
	}

	// Facing is complete.

	if( pAI->GetAIMovement()->GetTargetForward() == pAI->GetForwardVector() )
	{
		return true;
	}

	// Facing is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFaceNode::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionFaceNode::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !( pAI && pwsWorldStateCur && pwsWorldStateGoal ) )
	{
		return;
	}

	// Bail if WorldState has no knowledge of AINode being used.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return;
	}

	// We have reached our destination.

	pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Done );

	// Record the node we used in the world state.

	pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, pProp->hWSValue );
}
