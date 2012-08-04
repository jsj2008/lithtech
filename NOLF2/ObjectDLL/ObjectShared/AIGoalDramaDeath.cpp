// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDramaDeath.cpp
//
// PURPOSE : AIGoalDramaDeath implementation
//
// CREATED : 2/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalDramaDeath.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIHumanStateLaunch.h"
#include "AINodeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDramaDeath, kGoal_DramaDeath);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDramaDeath::CAIGoalDramaDeath()
{
	m_hDamager = LTNULL;
	m_hNodeDeath = LTNULL;
	m_bLaunch = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hDamager );
	SAVE_HOBJECT( m_hNodeDeath );
	SAVE_BOOL( m_bLaunch );
}

void CAIGoalDramaDeath::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hDamager );
	LOAD_HOBJECT( m_hNodeDeath );
	LOAD_BOOL( m_bLaunch );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT( m_hNodeDeath, m_pAI->m_hObject, "CAIGoalDramaDeath::ActivateGoal: Death Node is NULL" );

	AINodeDeath* pNodeDeath = (AINodeDeath*)g_pLTServer->HandleToObject( m_hNodeDeath );
	if( pNodeDeath )
	{
		m_pGoalMgr->LockGoal(this);

		// Ignore all senses.
	
		m_pAI->SetCurSenseFlags(kSense_None);

		// Launch AI in parabola to death node.

		if( m_bLaunch )
		{
			m_pAI->SetState( kState_HumanLaunch );

			// Variables could be parameterized.
	
			LTFLOAT fLaunchHeight = 100.f;
			LTFLOAT fLaunchSpeed = 200.f;

			LTVector vAIPos = m_pAI->GetPosition();
			LTVector vNodePos = pNodeDeath->GetPos();

			// Calculate how much above or below death node AI is.
	
			LTFLOAT fDiffBelow = Max( vNodePos.y - vAIPos.y, 0.f );
			LTFLOAT fDiffAbove = Max( vAIPos.y - vNodePos.y, 0.f );

			// Set up the launch.

			CAIHumanStateLaunch* pStateLaunch = (CAIHumanStateLaunch*)(m_pAI->GetState());
			pStateLaunch->SetLaunchDest( vNodePos );
			pStateLaunch->SetLaunchSpeed( fLaunchSpeed );
			pStateLaunch->SetLaunchHeight( Max( ( fDiffBelow + ( fLaunchHeight - fDiffAbove ) ), 10.f ) );
			pStateLaunch->SetLaunchMovement( kAP_JumpFly );
		}

		// Run to death node.

		else {
			m_pAI->SetState( kState_HumanGoto );
			CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
			pStateGoto->SetDestNode( pNodeDeath->m_hObject );
			pStateGoto->SetMovement( kAP_Run );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanLaunch:
			HandleStateLaunch();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanAnimate:
			HandleStateAnimate();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDramaDeath::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::HandleStateLaunch
//
//	PURPOSE:	Determine what to do when in state Launch.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::HandleStateLaunch()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			break;

		case kSStat_StateComplete:
			SetDeathAnimation();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "void CAIGoalDramaDeath::HandleStateLaunch: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			SetDeathAnimation();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDramaDeath::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::SetDeathAnimation
//
//	PURPOSE:	Start the death animation.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::SetDeathAnimation()
{
	// Play a dramatic death animation.

	m_pAI->SetState( kState_HumanAnimate );

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_Stand );
	animProps.Set( kAPG_Weapon, kAP_Weapon1 );
	animProps.Set( kAPG_WeaponPosition, kAP_Down );
	animProps.Set( kAPG_Action, kAP_Death );

	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
	pStateAnimate->SetAnimation( animProps, LTFALSE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalDramaDeath::HandleStateAnimate()
{
	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());

	switch( pStateAnimate->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		// Death animation finished.

		case kSStat_StateComplete:
			{
				// Kill AI for real.

				CDestructible* pDestructable = m_pAI->GetDestructible();
				pDestructable->HandleDestruction( m_hDamager );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDramaDeath::HandleStateAnimate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::HandleDamage
//
//	PURPOSE:	Handle damage by powering down or dying.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDramaDeath::HandleDamage(const DamageStruct& damage)
{
	CDestructible* pDestructable = m_pAI->GetDestructible();

	// Do not let AI die yet.  First launch him to a death node.

	if( pDestructable->IsDead() )
	{
		AINodeDeath* pNodeDeath = (AINodeDeath*)g_pAINodeMgr->FindNearestNode( m_pAI, kNode_Death, m_pAI->GetPosition(), LTFALSE, LTFALSE );
		if( pNodeDeath )
		{
			m_hNodeDeath = pNodeDeath->m_hObject;

			pDestructable->SetNeverDestroy( LTTRUE );
			pDestructable->Heal( pDestructable->GetMaxHitPoints() );

			m_hDamager = damage.hDamager;

			// Activate goal to launch AI.
	
			SetCurToBaseImportance();

			return LTTRUE;
		}

		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalDramaDeath::HandleDamage: Could not find Death node." );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDramaDeath::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDramaDeath::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "LAUNCH") )
	{
		m_bLaunch = IsTrueChar( szValue[0] );
		return LTTRUE;
	}

	return LTFALSE;
}

