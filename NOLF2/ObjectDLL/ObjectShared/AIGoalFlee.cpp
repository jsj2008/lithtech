// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlee.cpp
//
// PURPOSE : AIGoalFlee implementation
//
// CREATED : 7/24/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalFlee.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AIPathMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AITarget.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFlee, kGoal_Flee);

#define PANIC_RESET_TIME  30.f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFlee::CAIGoalFlee()
{
	m_hDestNode = NULL;
	m_hDangerObject = NULL;
	m_fRelaxTime = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hDestNode );
	SAVE_HOBJECT( m_hDangerObject );
	SAVE_TIME( m_fRelaxTime );
}

void CAIGoalFlee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hDestNode );
	LOAD_HOBJECT( m_hDangerObject );
	LOAD_TIME( m_fRelaxTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::ActivateGoal()
{
	super::ActivateGoal();

	if(m_hDestNode != LTNULL)
	{
		// Ignore senses.
		m_pAI->SetCurSenseFlags( kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject(m_hDestNode);
		LTVector vDest = pNode->GetPos();
		m_pAI->SetState( kState_HumanFlee );
		((CAIHumanStateFlee*)(m_pAI->GetState()))->SetDest(vDest);
		((CAIHumanStateFlee*)(m_pAI->GetState()))->SetDanger(m_hDangerObject);
		return;
	}

	//
	// No dest node was set, so just run... away.
	//

	// First try to find a panic node.

	if( m_pAI->HasTarget() )
	{
		AINodePanic* pPanicNode = (AINodePanic*)g_pAINodeMgr->FindNearestNodeFromThreat( m_pAI, kNode_Panic, m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject(), 1.f );
		if( pPanicNode )
		{
			m_fRelaxTime = g_pLTServer->GetTime() + PANIC_RESET_TIME;

			m_pAI->SetState( kState_HumanPanic );
			CAIHumanStatePanic* pStatePanic = (CAIHumanStatePanic*)m_pAI->GetState();
			pStatePanic->SetPanicNode( pPanicNode );
			m_pGoalMgr->LockGoal( this );
			return;
		}
	}
	
	// No panic node, so just randomly run away.

	m_pAI->SetAwareness( kAware_Alert );

	LTVector vDest;
	if( g_pAIPathMgr->FindRandomPosition( m_pAI, m_pAI->GetLastVolume(), m_pAI->GetPosition(), 512.f, &vDest ) )
	{
		m_pAI->SetState( kState_HumanGoto );
		CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
		pGoto->SetDest( vDest );
		pGoto->SetMovement( kAP_Run );

		m_pGoalMgr->LockGoal(this);
		return;
	}

	// Could not find a random path, so panic in place.

	m_pAI->SetState( kState_HumanPanic );
	m_pGoalMgr->LockGoal( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanFlee:
			HandleStateFlee();
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanAware:
			break;

		// State should only be idle or flee.
		default: AIASSERT1(0, m_pAI->m_hObject, "CAIGoalFlee::UpdateGoal: Unexpected State '%s'.", s_aszStateTypes[pState->GetStateType()] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleStateFlee
//
//	PURPOSE:	Determine what to do when in state Flee.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::HandleStateFlee()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalFlee::HandleStateFlee: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleStatePanic
//
//	PURPOSE:	Determine what to do when in state Panic.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::HandleStatePanic()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:

			// Unlock goal if nothing has been sensed for 30 secs.

			if( m_pGoalMgr->IsGoalLocked( this )  )
			{
				if( m_pAI->GetSenseRecorder()->HasAnyStimulation( kSense_All ) )
				{
					m_fRelaxTime = g_pLTServer->GetTime() + PANIC_RESET_TIME;
				}
				else if( g_pLTServer->GetTime() > m_fRelaxTime )
				{
					if( m_pAI->HasBackupHolsterString() )
					{
						m_pAI->SetHolsterString( m_pAI->GetBackupHolsterString() );
					}
					m_pGoalMgr->UnlockGoal( this );
				}
			}
			break;

		case kSStat_FailedComplete:
			{
				// Try to find a new panic node.
			
				AINodePanic* pPanicNode = (AINodePanic*)g_pAINodeMgr->FindNearestNodeFromThreat( m_pAI, kNode_Panic, m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject(), 1.f );
				if( pPanicNode )
				{
					m_fRelaxTime = g_pLTServer->GetTime() + PANIC_RESET_TIME;
	
					CAIHumanStatePanic* pStatePanic = (CAIHumanStatePanic*)m_pAI->GetState();
					pStatePanic->SetPanicNode( pPanicNode );
				}
				else {
					m_pGoalMgr->UnlockGoal( this );
				}
			}
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalFlee::HandleStatePanic: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_pAI->SetState( kState_HumanAware );
			m_pGoalMgr->UnlockGoal(this);
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalFlee::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::Relax
//
//	PURPOSE:	Stop panicking.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::Relax()
{
	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFlee::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Do not flee if we are armed.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() )
		{
			return LTFALSE;
		}

		m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );

		// Do not re-trigger while running away.

		if( m_pGoalMgr->IsCurGoal( this ) && 
			( ( m_pAI->GetState()->GetStateType() == kState_HumanPanic ) ||
			( m_pAI->GetState()->GetStateType() == kState_HumanGoto ) ) )
		{
			return LTFALSE;
		}

		// Neutrals do not flee when they see enemies.
		// They only flee when they see/hear gunfire.

		if( ( pSenseRecord->eSenseType == kSense_SeeEnemy ) && IsCharacter( m_hStimulusSource ) )
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject( m_hStimulusSource );

			if ( NEUTRAL == GetRelativeAlignment( m_pAI->GetRelationSet(), pCharacter->GetRelationData()) )
			{
				return LTFALSE;
			}
		}

		m_pAI->Target( m_hStimulusSource );

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFlee::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "NODE") )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(szValue);
		AIASSERT(pNode != LTNULL, m_pAI->m_hObject, "CAIGoalFlee::HandleNameValuePair: Dest node is NULL.");
		m_hDestNode = pNode ? pNode->m_hObject : NULL;
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "OBJECT") )
	{
		HOBJECT hObj;
		DWORD dwResult = FindNamedObject(szValue, hObj);
		m_hDangerObject = hObj;
		AIASSERT(dwResult == LT_OK, m_pAI->m_hObject, "CAIGoalFlee::HandleNameValuePair: Can't find danger object.");
		return LTTRUE;
	}

	return LTFALSE;
}

