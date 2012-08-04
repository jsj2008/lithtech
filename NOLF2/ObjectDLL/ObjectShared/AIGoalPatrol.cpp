// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPatrol.cpp
//
// PURPOSE : AIGoalPatrol implementation
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalPatrol.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AIPathMgr.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPatrol, kGoal_Patrol);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalPatrol::CAIGoalPatrol()
{
	m_hPatrolNode = LTNULL;
	
	m_eAwareness = kAP_Patrol;
}

CAIGoalPatrol::~CAIGoalPatrol()
{
	// Unlock any path held by this AI by setting patrol node to NULL.
	SetPatrolNode( LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hPatrolNode );
	SAVE_DWORD( m_eAwareness );
}

void CAIGoalPatrol::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hPatrolNode );
	LOAD_DWORD_CAST( m_eAwareness, EnumAnimProp );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::ActivateGoal()
{
	super::ActivateGoal();

	// Start out idle.
	m_pAI->SetState( kState_HumanIdle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::DeactivateGoal()
{
	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		// If state is idle, and a patrol node is set, set state to patrol.
		case kState_HumanIdle:
			HandleStateIdle();
			break;

		case kState_HumanPatrol:
			HandleStatePatrol();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// State should only be idle or patrol.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalPatrol::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::HandleStateIdle
//
//	PURPOSE:	Determine what to do when in state Idle.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::HandleStateIdle()
{
	// Do not patrol if AI owns a Talk node (and has a Talk goal).

	if( g_pAINodeMgr->FindOwnedNode( kNode_Talk, m_pAI->m_hObject ) )
	{
		return;
	}

	AINodePatrol* pPatrolNode = (AINodePatrol*)g_pLTServer->HandleToObject(m_hPatrolNode);

	// Find a patrol node if we don't have one.
	// Do not search for a node on first update.
	// Give nodes set through an initial command first dibs.

	if( ( !m_pAI->IsFirstUpdate() ) && ( pPatrolNode == LTNULL ) )
	{
		// If we already own a Guard node, we will Guard so bail.
		// An AI cannot both Patrol and Guard.

		if( g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject ) )
		{
			m_fCurImportance = 0.f;
			return;
		}

		pPatrolNode = (AINodePatrol*)g_pAINodeMgr->FindNearestNodeInRadius( m_pAI, kNode_Patrol, m_pAI->GetPosition(), NODEMGR_MAX_SEARCH, LTTRUE );
		SetPatrolNode( pPatrolNode );
	}

	// Use a patrol node.

	else if(pPatrolNode != LTNULL)
	{
		// If a patrol node has become unreachable, look for the next closest owned patrol node.

		if( !g_pAIPathMgr->HasPath( m_pAI, pPatrolNode ) )
		{
			pPatrolNode = (AINodePatrol*)g_pAINodeMgr->FindNearestOwnedNode( m_pAI, kNode_Patrol, m_pAI->GetPosition(), m_pAI->m_hObject );
			if( !pPatrolNode )
			{
				return;
			}
		}

		m_pAI->SetState( kState_HumanPatrol );
		CAIHumanStatePatrol* pStatePatrol = (CAIHumanStatePatrol*)(m_pAI->GetState());
		pStatePatrol->SetAwareness( m_eAwareness );
		pStatePatrol->SetNode( pPatrolNode );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::HandleStatePatrol
//
//	PURPOSE:	Determine what to do when in state Patrol.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::HandleStatePatrol()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			{
				// Keep track of the current patrol node, in case we get interrupted.

				AINodePatrol* pPatrolNode = ((CAIHumanStatePatrol*)(m_pAI->GetState()))->GetNode();

				if( pPatrolNode && ( pPatrolNode->m_hObject != m_hPatrolNode ) )
				{
					m_hPatrolNode = pPatrolNode->m_hObject;
				
					// If AI holstered his weapon from code (denoted with ~)
					// unholster it when he reaches his first patrol node.

					if( ( !m_pAI->GetPrimaryWeapon() ) &&
						( m_pAI->HasHolsterString() ) && 
						( m_pAI->GetHolsterString()[0] == '~' ) )
					{
						m_pAI->SetState( kState_HumanDraw );
						m_pAI->GetState()->PlayFirstSound( LTFALSE );
					}				
				}
			}
			break;

		case kSStat_StateComplete:
		case kSStat_FailedComplete:
		case kSStat_FailedSetPath:
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalPatrol::HandleStatePatrol: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_pAI->SetState( kState_HumanIdle );
			break;

		case kSStat_StateComplete:
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalPatrol::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::SetPatrolNode
//
//	PURPOSE:	Set node and lock path.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::SetPatrolNode(AINodePatrol* pNode)
{
	// Unlock any previous path.

	if( m_hPatrolNode )
	{
		AINodePatrol* pOldNodePatrol = (AINodePatrol*)g_pLTServer->HandleToObject( m_hPatrolNode );
		UnlockPatrolPath( pOldNodePatrol );
	}

	// Record current node's handle.

	m_hPatrolNode = pNode ? pNode->m_hObject : LTNULL;

	// Lock new path, if not already the owner.

	if( pNode && ( pNode->GetNodeOwner() != m_pAI->m_hObject ) )
	{
		LockPatrolPath( pNode );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::LockPatrolPath
//
//	PURPOSE:	Lock\unlock nodes on patrol path.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::LockPatrolPath(AINodePatrol* pNode, LTBOOL bLock)
{
	AINodePatrol* pNodeStart = pNode;
	
	static std::vector<AINodePatrol*> vVisited;
	static std::vector<AINodePatrol*>::iterator it;
	vVisited.clear();

	LTBOOL bDone = LTFALSE;

	// Lock all next nodes of NodeStart.

	while( pNode && !bDone )
	{
		vVisited.push_back( pNode);

		if( bLock )
		{
			pNode->Lock( m_pAI->m_hObject );
			pNode->SetNodeOwner( m_pAI->m_hObject );
		}
		else {
			pNode->Unlock( m_pAI->m_hObject );
			pNode->SetNodeOwner( LTNULL );
		}

		pNode = pNode->GetNext();

		// Check for loop.
		for(it = vVisited.begin(); it != vVisited.end(); ++it)
		{
			if( *it == pNode )
			{
				bDone = LTTRUE;
				break;
			}
		}
	}

	// Lock all previous nodes of NodeStart.
	pNode = pNodeStart->GetPrev();
	while( pNode )
	{
		// Check for loop.
		for(it = vVisited.begin(); it != vVisited.end(); ++it)
		{
			if( *it == pNode )
			{
				return;
			}
		}

		vVisited.push_back( pNode);

		if( bLock )
		{
			pNode->Lock( m_pAI->m_hObject );
			pNode->SetNodeOwner( m_pAI->m_hObject );
		}
		else {
			pNode->Unlock( m_pAI->m_hObject );
			pNode->SetNodeOwner( LTNULL );
		}

		pNode = pNode->GetPrev();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the presence of a guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::RecalcImportance()
{
	// If we already own a Guard node, we will Guard so bail.
	// An AI cannot both Patrol and Guard.

	if( g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject ) )
	{
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalPatrol::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "NODE") )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(szValue);
		AIASSERT1( pNode && (pNode->GetType() == kNode_Patrol), m_pAI->m_hObject, "CAIGoalPatrol::HandleNameValuePair: Could not find patrol node: %s", szValue );
		if( pNode )
		{
			SetPatrolNode( (AINodePatrol*)pNode );
		}
		return LTTRUE;
	}

	else if ( !_stricmp(szName, "AWARENESS") )
	{
		m_eAwareness = CAnimationMgrList::GetPropFromName( szValue );
		AIASSERT( m_eAwareness != kAP_Invalid, m_pAI->m_hObject, "CAIGoalPatrol::HandleNameValuePair: Awareness is None" );

		if( m_pAI->GetState()->GetStateType() == kState_HumanPatrol )
		{
			CAIHumanStatePatrol* pStatePatrol = (CAIHumanStatePatrol*)(m_pAI->GetState());
			pStatePatrol->SetAwareness( m_eAwareness );
		}
	}

	return LTFALSE;
}

