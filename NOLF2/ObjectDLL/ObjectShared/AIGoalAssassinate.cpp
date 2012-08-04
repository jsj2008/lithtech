// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAssassinate.cpp
//
// PURPOSE : AIGoalAssassinate implementation
//
// CREATED : 7/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAssassinate.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINode.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAssassinate, kGoal_Assassinate);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAssassinate::CAIGoalAssassinate()
{
	m_hNodeAssassinate	= LTNULL;
	m_bNewTarget		= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAssassinate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hNodeAssassinate );
	SAVE_BOOL( m_bNewTarget );
}

void CAIGoalAssassinate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hNodeAssassinate );
	LOAD_BOOL( m_bNewTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAssassinate::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNodeAssassinate != LTNULL, m_pAI->m_hObject, "CAIGoalAssassinate::ActivateGoal: AINodeAssassinate is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags(kSense_SeeEnemy);

	m_pAI->SetState( kState_HumanAssassinate );

	// Set target.
	m_pAI->Target(m_hTarget);

	m_bNewTarget = LTFALSE;

	AINodeAssassinate* pNodeAssassinate = (AINodeAssassinate*)g_pLTServer->HandleToObject(m_hNodeAssassinate);

	CAIHumanStateAssassinate* pStateAssassinate = (CAIHumanStateAssassinate*)(m_pAI->GetState());
	pStateAssassinate->SetNode(pNodeAssassinate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAssassinate::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAssassinate:
			HandleStateAssassinate();
			break;

		// Unexpected State.
		default: ASSERT(!"CAIGoalAssassinate::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::HandleStateAssassinate
//
//	PURPOSE:	Determine what to do when in state Assassinate.
//
// ----------------------------------------------------------------------- //

void CAIGoalAssassinate::HandleStateAssassinate()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalAssassinate::HandleStateAssassinate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAssassinate::HandleGoalAttractors()
{
	// Default Target to player if not set.
	if(m_bNewTarget && (m_hTarget == LTNULL))
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		m_hTarget = pPlayer->m_hObject;
	}

	AINode* pNode = LTNULL;

	// Only look for atractors if there is a target set.
	if(m_hTarget != LTNULL)
	{
		AINode* pNode = super::HandleGoalAttractors();
		if(pNode != LTNULL)
		{
			AIASSERT(pNode->GetType() == kNode_Assassinate, m_pAI->m_hObject, "CAIGoalAssassinate::HandleGoalAttractors: AINode is not of type Assassinate.");
			m_hNodeAssassinate = pNode->m_hObject;
		}
		else {
			m_hNodeAssassinate = LTNULL;
		}
	}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAssassinate::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAssassinate::HandleNameValuePair(const char *szName, const char *szValue)
{
	if( super::HandleNameValuePair(szName, szValue) )
	{
		m_bNewTarget = LTTRUE;
		return LTTRUE;
	}

	return LTFALSE;
}


