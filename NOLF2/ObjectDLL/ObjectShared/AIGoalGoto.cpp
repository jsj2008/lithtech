// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGoto.cpp
//
// PURPOSE : AIGoalGoto implementation
//
// CREATED : 8/07/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalGoto.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGoto, kGoal_Goto);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalGoto::CAIGoalGoto()
{
	m_eMovement = kAP_Walk;
	m_eAwareness = kAP_None;
	m_hDestNode = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hDestNode);
	SAVE_DWORD(m_eMovement);
	SAVE_DWORD(m_eAwareness);
}

void CAIGoalGoto::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hDestNode);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_DWORD_CAST(m_eAwareness, EnumAnimProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->SetState( kState_HumanGoto );
	CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
	pGoto->SetDestNode(m_hDestNode);
	pGoto->SetMovement(m_eMovement);
	pGoto->SetAwareness(m_eAwareness);

	m_pGoalMgr->LockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanGoto:
			HandleStateGoto();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGoto::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			// Lose interest when arrived at destination.
			m_fCurImportance = 0.f;
			m_pGoalMgr->UnlockGoal(this);
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGoto::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalGoto::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "NODE") )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GOTO setting node=%s", szValue ) );
		AINode* pNode = g_pAINodeMgr->GetNode( szValue );
		if( pNode )
		{
			m_hDestNode = pNode->m_hObject;

			// If Goal was already active (walking to previous goto node)
			// Reset the goal.

			if( m_pGoalMgr->IsCurGoal( this ) && ( m_pAI->GetState()->GetStateType() == kState_HumanGoto ) )
			{
				CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
				pGoto->SetDestNode( m_hDestNode );
			}
		}
		else
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAIGoalGoto::HandleNameValuePair: Cannot find node '%s'", szValue );
		}
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "MOVEMENT") )
	{
		m_eMovement = CAnimationMgrList::GetPropFromName( szValue );
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "AWARENESS") )
	{
		m_eAwareness = CAnimationMgrList::GetPropFromName( szValue );
		return LTTRUE;
	}

	return LTFALSE;
}

