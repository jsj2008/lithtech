// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCheckBody.cpp
//
// PURPOSE : AIGoalCheckBody implementation
//
// CREATED : 7/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalCheckBody.h"
#include "AIGoalSpecialDamage.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AIStimulusMgr.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "Body.h"
#include "AIUtils.h"
#include "AICentralKnowledgeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCheckBody, kGoal_CheckBody);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCheckBody::CAIGoalCheckBody()
{
	m_hBody.SetReceiver( *this );
}

CAIGoalCheckBody::~CAIGoalCheckBody()
{
	m_lstSeenBodies.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vStimulusPos);
	SAVE_HOBJECT(m_hBody);

	SeenBodyList::iterator it;
	SAVE_DWORD(m_lstSeenBodies.size());
	for(it = m_lstSeenBodies.begin(); it != m_lstSeenBodies.end(); ++it)
	{
		SAVE_HOBJECT(*it);
	}
}

void CAIGoalCheckBody::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vStimulusPos);
	LOAD_HOBJECT(m_hBody);

	uint32 cBodies;
	SeenBodyList::iterator it;
	LOAD_DWORD(cBodies);
	m_lstSeenBodies.resize(cBodies);
	for(it = m_lstSeenBodies.begin(); it != m_lstSeenBodies.end(); ++it)
	{
		LTObjRefNotifier& ref = *it;
		ref.SetReceiver( *this );
		LOAD_HOBJECT(ref);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::ActivateGoal()
{
	super::ActivateGoal();

	m_pGoalMgr->LockGoal(this);

	// Only pay attention to really important senses.
	m_pAI->SetCurSenseFlags(
		kSense_SeeEnemy					|
		kSense_HearEnemyWeaponFire		|
		kSense_HearEnemyWeaponImpact	|
		kSense_HearAllyWeaponFire		|
		kSense_HearAllyPain				|
		kSense_SeeDangerousProjectile	|
		kSense_SeeCatchableProjectile);

	// Saw a new body, or returning to a previously seen one.
	// Record the handle to the body, because m_hStimulusSource may change if the 
	// AI encounters other bodies.
	// m_hBody may be a Body or a knocked out AI.

	m_hBody = LTNULL;
	if( m_hStimulusSource )
	{
		m_hBody = m_hStimulusSource;
	}
	else if( !m_lstSeenBodies.empty() )
	{
		m_hBody = m_lstSeenBodies.front();
	}
	
	// Bail if body is gone.

	if( !m_hBody )
	{
		m_fCurImportance = 0.f;
	}

	// Body already has a checker that is not me, so search.

	ILTBaseClass *pChecker = g_pAICentralKnowledgeMgr->GetKnowledgeTarget( kCK_CheckingBody, g_pLTServer->HandleToObject(m_hBody) );
	if( pChecker && ( pChecker != m_pAI ) )
	{
		SearchOrAware();
		return;
	}


	// Get body pos.

	g_pLTServer->GetObjectPos( m_hBody, &m_vStimulusPos );


	// Body is not being checked, so check it.

	if( m_hStimulusSource )
	{
		m_pAI->SetState( kState_HumanCheckBody );
	
		CAIHumanStateCheckBody* pStateCheckBody = (CAIHumanStateCheckBody*)(m_pAI->GetState());
		pStateCheckBody->SetBody( m_hBody );
		return;
	}

	// Return to an old, previously seen body to dispose it.
	
	else {
		m_pAI->SetState( kState_HumanGoto );
		CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
		pStateGoto->SetDest( m_vStimulusPos );
		pStateGoto->SetMovement( kAP_Run );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::DeactivateGoal()
{
	super::DeactivateGoal();

	if( m_hBody )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_CheckingBody, g_pLTServer->HandleToObject(m_hBody), m_pAI );
	}

	m_hBody = LTNULL;
	m_hStimulusSource = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanCheckBody:
			HandleStateCheckBody();
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanAnimate:
			HandleStateAnimate();
			break;

		case kState_HumanAware:
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCheckBody::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::HandleStateCheckBody
//
//	PURPOSE:	Determine what to do when in state CheckBody.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::HandleStateCheckBody()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			{
				// If body is a knocked-out AI, bail if he gets up himself.
				if( IsAI( m_hBody ) )
				{
					CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hBody );
					if( pAI )
					{
						CAIGoalSpecialDamage* pGoal = (CAIGoalSpecialDamage*)pAI->GetGoalMgr()->FindGoalByType( kGoal_SpecialDamage );
						if( !pAI->GetGoalMgr()->IsCurGoal( pGoal ) )
						{
							SearchOrAware();
						}
					}
				}
			}
			break;

		case kSStat_FailedComplete:
			SearchOrAware();
			break;

		case kSStat_StateComplete:
			DisposeOfBody();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCheckBody::HandleStateCheckBody: Unexpected State Status.");
	}
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //
	
void CAIGoalCheckBody::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			DisposeOfBody();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGoto::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::HandleStateAnimate()
{
	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());

	switch( pStateAnimate->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			{
				// The AI is finished with this body.
				RemoveBodyFromSeenList( m_hBody );
				Body* pBody = (Body*)g_pLTServer->HandleToObject( m_hBody );
				
				// NULLify the handle, and do not remove the CentralKnowledge about the body checker.
				// When the body finishes fading the knowledge will be removed.
				// Until then, the knowledge keeps other AI from checking the same body as it is fading.

				m_hBody = LTNULL;		
				
				if( pBody && !pBody->BeingCarried() )
				{
					pBody->SetState(kState_BodyFade);
				}

				SearchOrAware();
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCheckBody::HandleStateAnimate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::DisposeOfBody
//
//	PURPOSE:	Dispose of a checked body.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::DisposeOfBody()
{
	// If body magicly went away (sleeping AI died) search...

	if( !m_hBody )
	{
		SearchOrAware();
		return;
	}

	// Wake up a knocked-out AI.

	if( IsAI( m_hBody ) )
	{
		CAI* pAI = dynamic_cast<CAI*>(g_pLTServer->HandleToObject( m_hBody ));
		if( pAI && !pAI->BeingCarried() )
		{
			CAIGoalSpecialDamage* pGoal = (CAIGoalSpecialDamage*)pAI->GetGoalMgr()->FindGoalByType( kGoal_SpecialDamage );
			if( pGoal )
			{
				pGoal->InterruptSpecialDamage( LTTRUE );
			}
		}

		SearchOrAware();
	}

	// Get rid of a dead body.

	else
	{
		Body* pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( m_hBody ));
		if( pBody )
		{
			if( pBody->BeingCarried() )
			{
				SearchOrAware();
				return;
			}

			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_CheckingBody, pBody, m_pAI, LTTRUE );
		}
		
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( pAIHuman )
		{
			m_pAI->SetState( kState_HumanAnimate );
	
			if( !m_pAI->IsPlayingDialogSound() )
			{
				m_pAI->PlaySound( kAIS_DisposeBody, LTFALSE );
			}

			m_pAI->FaceObject( m_hBody );

			// Setup animprops for the body disposal animation.

			CAnimationProps	animProps;
			animProps.Set( kAPG_Posture, kAP_Stand );
			animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
			animProps.Set( kAPG_Action, kAP_DisposeBody );

			CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
			pStateAnimate->SetAnimation( animProps, LTFALSE );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::SearchOrAware
//
//	PURPOSE:	Search or stand alert.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::SearchOrAware()
{
	// Reset AIs default senses, from aibutes.txt.
	m_pAI->ResetBaseSenseFlags();

	// Search or go aware.

	if( (m_pAI->IsMajorlyAlarmed() || m_bSearch) && m_pAI->CanSearch())
	{
		SetStateSearch();
	}
	else {
		m_pAI->SetState( kState_HumanAware );
		m_pGoalMgr->UnlockGoal(this);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::OnLinkBroken
//
//	PURPOSE:	Handles a deleted object reference.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Do not call super::OnLinkBroken( ref ), because we want
	// the AI to continue searching after the body is removed.

	// Remove deleted body from list of seen bodies.
	SeenBodyList::iterator it;
	for(it = m_lstSeenBodies.begin(); it != m_lstSeenBodies.end(); ++it)
	{
		if( &(*it) == pRef )
		{
			m_lstSeenBodies.erase( it );
			break;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::AddBodyToSeenList
//
//	PURPOSE:	Keep track of bodies AI has seen.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::AddBodyToSeenList(HOBJECT hBody)
{
	// Record the body that was seen, so that AI can return to
	// it if he dosn't finish checking it right now.
	// Only record bodies, not knocked-out AIs, because AIs
	// will get up themselves eventually.
		
	if( IsBody( m_hStimulusSource ) )
	{
		LTObjRefNotifier ref = m_hStimulusSource;
		ref.SetReceiver( *this );
		m_lstSeenBodies.push_back( ref );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::RemoveBodyFromSeenList
//
//	PURPOSE:	Forget about a body the AI has seen.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::RemoveBodyFromSeenList(HOBJECT hBody)
{
	// Remove deleted body from list of seen bodies.

	SeenBodyList::iterator it;
	for(it = m_lstSeenBodies.begin(); it != m_lstSeenBodies.end(); ++it)
	{
		if( *it == hBody )
		{
			m_lstSeenBodies.erase( it );
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the presence
//              of seen bodies.
//
// ----------------------------------------------------------------------- //

void CAIGoalCheckBody::RecalcImportance()
{
	// If this goal is not active, and the AI is not sensing anything else,
	// reactivate the goal to continue checking bodies.

	if( ( m_lstSeenBodies.size() > 0 ) && 
		( !m_pGoalMgr->HasLockedGoal() ) && 
		( !m_pGoalMgr->IsCurGoal( this ) ) &&
		( !m_pAI->GetSenseRecorder()->HasAnyStimulation( kSense_All ) ) )
	{
		SetCurToBaseImportance();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCheckBody::HandleDamage(const DamageStruct& damage)
{
	m_fCurImportance = 0.f;

	return super::HandleDamage( damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCheckBody::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCheckBody::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Do not investigate without a weapon.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
		{
			m_hStimulusSource = LTNULL;
			return LTFALSE;
		}


		// Handle SeeAllyDeath sense.
		
		if( pSenseRecord->eSenseType == kSense_SeeAllyDeath )
		{
			m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );

			// Record the body that was seen, so that AI can return to
			// it if he dosn't finish checking it right now.

			AddBodyToSeenList( m_hStimulusSource );
			m_vStimulusPos = pSenseRecord->vLastStimulusPos;

			return LTTRUE;
		}

		// Handle any other sense.
		// If stimulus matches body, then ignore because it is pain sounds 
		// from the fallen AI.

		else if( ( m_fCurImportance > 0.f ) &&
				 ( pSenseRecord->hLastStimulusSource != m_hBody ) &&
				 ( pSenseRecord->hLastStimulusTarget != m_hBody ) ) 
		{
			// Bail from goal if AI senses something else (e.g. gunfire, etc).

			m_fCurImportance = 0.f;
		}
	}

	m_hStimulusSource = LTNULL;
	return LTFALSE;
}
