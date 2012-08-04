// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromRoofVantage.cpp
//
// PURPOSE : AIGoalAttackFromRoofVantage implementation
//
// CREATED : 5/18/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackFromRoofVantage.h"
#include "AI.h"
#include "AIHumanState.h"
#include "AITarget.h"
#include "AICentralKnowledgeMgr.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromRoofVantage, kGoal_AttackFromRoofVantage);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromVantage::SetStateAttack
//
//	PURPOSE:	Set AttackFromVantage state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromRoofVantage::SetStateAttack()
{
	HOBJECT hVantageNode = LTNULL;

	// If setting a new roof vantage node, count it.

	if( m_pAI->GetState()->GetStateType() == kState_HumanAttackFromVantage )
	{
		CAIHumanStateAttackFromVantage* pAttackState = (CAIHumanStateAttackFromVantage*)m_pAI->GetState();
		hVantageNode = pAttackState->GetVantageNode();
	}

	if( ( hVantageNode != m_hNode ) && ( m_pAI->HasTarget() ) )
	{
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackFromRoof, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()) ), m_pAI->m_hObject, "CAIGoalAttackFromRoofVantage::SetStateAttack: Already registered roof attack count!" );
		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_AttackFromRoof, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()), LTTRUE );
	}

	super::SetStateAttack();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRoofVantage::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromRoofVantage::DeactivateGoal()
{
	super::DeactivateGoal();

	// Decrement counters.

	g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AttackFromRoof, m_pAI );
	AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackFromRoof, m_pAI ), m_pAI->m_hObject, "CAIGoalAttackFromRoofVantage::DeactivateGoal: Too many roof attackers registered!" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromRoofVantage::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackFromRoofVantage::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	if( !m_pAI->HasTarget() )
	{
		return LTNULL;
	}

	// Only use roof vantage if there are at least 3 AIs attacking the same target.

	uint32 cAttackers = g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_Attacking, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()) );
	if( cAttackers < 3 )
	{
		return LTNULL;
	}

	// Only one AI at a time may attack from roof.

	cAttackers = g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AttackFromRoof, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()) );
	if( cAttackers != 0 )
	{
		return LTNULL;
	}

	return super::FindNearestAttractorNode( eNodeType, vPos, fDistSqr );
}

