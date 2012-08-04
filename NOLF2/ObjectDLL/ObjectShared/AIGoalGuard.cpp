// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGuard.cpp
//
// PURPOSE : AIGoalGuard implementation
//
// CREATED : 11/12/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalGuard.h"
#include "AINodeGuard.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGuard, kGoal_Guard);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalGuard::CAIGoalGuard()
{
	m_hGuardNode	= LTNULL;
	m_bInRadius		= LTFALSE;
	m_eNodeType		= kNode_Guard;
	m_fMinImportance= 1.f;
}

CAIGoalGuard::~CAIGoalGuard()
{
	SetGuardNode( LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hGuardNode );
	SAVE_BOOL( m_bInRadius );
}

void CAIGoalGuard::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hGuardNode );
	LOAD_BOOL( m_bInRadius );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::ActivateGoal()
{
	super::ActivateGoal();

	// Start out idle.
	m_pAI->SetState( kState_HumanIdle );

	m_pAI->SetAwareness( kAware_Relaxed );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::DeactivateGoal()
{
	super::DeactivateGoal();

	// If the AI has a guard node, then keep some minimal importance
	// for this goal, so that it can override very unimportant 
	// goals (e.g. ExitLevel) which they only do if there's nothing else to do.

	if( m_hGuardNode )
	{
		m_fCurImportance = m_fMinImportance;
	}
	else {
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::UpdateGoal()
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

		// State should only be idle or goto.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGuard::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::HandleStateIdle
//
//	PURPOSE:	Determine what to do when in state Idle.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::HandleStateIdle()
{
	if( !m_bInRadius )
	{
		GotoNode();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::GotoNode
//
//	PURPOSE:	Goto specified node.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::GotoNode()
{
	AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject(m_hGuardNode);

	if( pGuardNode )
	{
		m_pAI->SetState( kState_HumanGoto );
		CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
		pStateGoto->SetDestNode( pGuardNode->m_hObject );
		pStateGoto->TurnOffLights( LTTRUE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = m_fMinImportance;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalGuard::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::FindGuardNode
//
//	PURPOSE:	Find nearest guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::FindGuardNode()
{
	// Do not search for a node on first update.
	// Give nodes set through an initial command first dibs.

	if( m_pAI->IsFirstUpdate() )
	{
		return;
	}

	// If we already own a Patrol node, we will Patrol so bail.
	// An AI cannot both Patrol and Guard.

	if( g_pAINodeMgr->FindOwnedNode( kNode_Patrol, m_pAI->m_hObject ) )
	{
		m_fCurImportance = 0.f;
		return;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_pAI->m_hObject, &vPos);
	AINodeGuard* pGuardNode = (AINodeGuard*)g_pAINodeMgr->FindNearestNodeInRadius( m_pAI, m_eNodeType, vPos, NODEMGR_MAX_SEARCH, LTTRUE);

	SetGuardNode( pGuardNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::SetGuardNode
//
//	PURPOSE:	Set guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::SetGuardNode(AINode* pGuardNode)
{
	if( pGuardNode && ( pGuardNode->GetType() != m_eNodeType ))
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalGuard::SetGuardNode: Wrong node type.");
		return;
	}

	if( m_hGuardNode )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject(m_hGuardNode);
		if( pNode )
		{
			pNode->SetNodeOwner( LTNULL );
			pNode->Unlock( m_pAI->m_hObject );
		}
		m_hGuardNode = LTNULL;
	}

	if( pGuardNode )
	{
		m_hGuardNode = pGuardNode->m_hObject;
		pGuardNode->Lock( m_pAI->m_hObject );
		pGuardNode->SetNodeOwner( m_pAI->m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the distance
//              to the guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::RecalcImportance()
{
	// Find a node.

	if(!m_hGuardNode)
	{
		FindGuardNode();
	}

	// Check status.

	AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject(m_hGuardNode);
	
	if( pGuardNode )
	{
		m_bInRadius = LTTRUE;

		LTFLOAT fDistSqr = m_pAI->GetPosition().DistSqr( pGuardNode->GetPos() );

		// There is a guard node, and we are outside of the return radius,
		// so guard goal gets its base importance.

		if( fDistSqr > pGuardNode->GetRadiusSqr() )
		{
			// Ignore the Guard node's radius while AI is assigned a Talk node.

			if( ( GetGoalType() == kGoal_Guard ) &&
				( g_pAINodeMgr->FindOwnedNode( kNode_Talk, m_pAI->m_hObject ) ) )
			{
				m_fCurImportance = 0.f;
				return;
			}

			SetCurToBaseImportance();
			m_bInRadius = LTFALSE;
		}

		// We are inside the guard radius, so guard goal has minimum importance.
		// Only give goal any importance if it was previously at base importance.
		
		else if( m_fCurImportance > m_fMinImportance )
		{
			m_fCurImportance = m_fMinImportance;
		}

		return;
	}

	// No guard node.

	m_bInRadius = LTFALSE;
	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalGuard::HandleNameValuePair(const char *szName, const char *szValue)
{
	AIASSERT(szName && szValue, m_pAI->m_hObject, "CAIGoalGuard::HandleNameValuePair: Name or value is NULL.");

	if( super::HandleNameValuePair(szName, szValue) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(szName, "NODE") )
	{
		// If Goal was already active (walking to previous guard node)
		// Reset the goal.

		if( m_pGoalMgr->IsCurGoal( this ) )
		{
			m_pAI->SetState( kState_HumanIdle );
		}

		AINode* pNode = g_pAINodeMgr->GetNode(szValue);
		if( pNode )
		{
			SetGuardNode( pNode );
			RecalcImportance();

			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoal%s: NODE=%s", s_aszGoalTypes[GetGoalType()], ::ToString( pNode->GetName() ) ) );
		}
		else {
			AIError( "%s Cannot find node! CAIGoal%s: NODE=%s", m_pAI->GetName(), s_aszGoalTypes[GetGoalType()], szValue );
		}

		return LTTRUE;
	}

	return LTFALSE;
}

