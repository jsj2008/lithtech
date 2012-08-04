// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPickupWeapon.cpp
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalPickupWeapon.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"

LINKFROM_MODULE(AIGoalPickupWeapon);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalPickupWeapon, kGoal_PickupWeapon );

static CAIWMFact* GetWeaponItemTaskFact( CAI* pAI )
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_PickupWeapon );
	return pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPickupWeapon::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalPickupWeapon::CAIGoalPickupWeapon()
{
}

CAIGoalPickupWeapon::~CAIGoalPickupWeapon()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPickupWeapon::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalPickupWeapon
//              
//----------------------------------------------------------------------------

void CAIGoalPickupWeapon::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hWeaponItem );
}

void CAIGoalPickupWeapon::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hWeaponItem );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPickupWeapon::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalPickupWeapon::CalculateGoalRelevance()
{
	// Current target is not a Weapon and no PickupWeapon task.

	if ( !m_pAI->HasTarget( kTarget_WeaponItem ) 
		&& NULL == GetWeaponItemTaskFact( m_pAI ) )
	{
		m_fGoalRelevance = 0.f;	
		return;
	}

	// Goal is relevant.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPickupWeapon::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPickupWeapon::ActivateGoal()
{
	super::ActivateGoal();
	
	CAIWMFact* pPickupWeaponTaskFact = GetWeaponItemTaskFact( m_pAI );
	if ( pPickupWeaponTaskFact )
	{
		m_hWeaponItem = pPickupWeaponTaskFact->GetTargetObject();
	}
	else
	{
		m_hWeaponItem = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	// Add an ownership claim on the WeaponItem.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if ( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_Ownership );
		pFact->SetSourceObject( m_pAI->GetHOBJECT() );
		pFact->SetTargetObject( m_hWeaponItem );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPickupWeapon::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPickupWeapon::DeactivateGoal()
{
	super::DeactivateGoal();

	// Remove any PickupWeapon tasks

	CAIWMFact queryPickupWeaponFact;
	queryPickupWeaponFact.SetFactType( kFact_Task );
	queryPickupWeaponFact.SetTaskType( kTask_PickupWeapon );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( queryPickupWeaponFact );

	// Remove the ownership claim on the WeaponItem.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_Ownership );
	queryFact.SetSourceObject( m_pAI->GetHOBJECT() );
	queryFact.SetTargetObject( m_hWeaponItem );
	g_pAIWorkingMemoryCentral->ClearWMFact( queryFact );

	// Nullify the handle to the target WeaponItem.

	m_hWeaponItem = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPickupWeapon::HandleBuildPlanFailure
//              
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//              
//----------------------------------------------------------------------------

void CAIGoalPickupWeapon::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Remove any PickupWeapon tasks

	CAIWMFact queryPickupWeaponFact;
	queryPickupWeaponFact.SetFactType( kFact_Task );
	queryPickupWeaponFact.SetTaskType( kTask_PickupWeapon );
	g_pAIWorkingMemoryCentral->ClearWMFact( queryPickupWeaponFact );

	// Delay re-evaluation of this WeaponItem

	CAIWMFact queryKnowledgeFact;
	queryKnowledgeFact.SetFactType(kFact_Knowledge);
	queryKnowledgeFact.SetKnowledgeType(kKnowledge_WeaponItem);
	queryKnowledgeFact.SetTargetObject(m_pAI->GetAIBlackBoard()->GetBBTargetObject());
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact(queryKnowledgeFact);
	if (pFact)
	{
		// TODO: If this needs to get modified, move it to the the Const 
		// record in the database, along with any other SetTime modifications
		// of this fact.
		pFact->SetTime(g_pLTServer->GetTime() + 2.0f);
	}

	// Remove a task, if one exists, about this weapon.

	CAIWMFact queryTaskFact;
	queryTaskFact.SetFactType( kFact_Task );
	queryTaskFact.SetTaskType( kTask_ExchangeWeapon );
	queryTaskFact.SetTargetObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( queryTaskFact );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPickupWeapon::SetWSSatisfaction
//              
//	PURPOSE:	Set the WorldState satisfaction conditions.
//              
//----------------------------------------------------------------------------

void CAIGoalPickupWeapon::SetWSSatisfaction( CAIWorldState& WorldState )
{
	CAIWMFact* pTaskFact = GetWeaponItemTaskFact( m_pAI );
	if ( pTaskFact )
	{
		WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, pTaskFact->GetTargetObject() );
	}
	else
	{
		CAIWMFact queryKnowledgeFact;
		queryKnowledgeFact.SetFactType(kFact_Knowledge);
		queryKnowledgeFact.SetKnowledgeType(kKnowledge_WeaponItem);
		queryKnowledgeFact.SetTargetObject(m_pAI->GetAIBlackBoard()->GetBBTargetObject());
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact(queryKnowledgeFact);
		if ( pFact )
		{
			if ( pFact->GetSourceObject() )
			{
				WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, pFact->GetSourceObject() );
			}
			else
			{
				WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, pFact->GetTargetObject() );
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPickupWeapon::HandleBuildPlanFailure
//              
//	PURPOSE:	Return true if the world state satisfies the goal.
//              
//----------------------------------------------------------------------------

bool CAIGoalPickupWeapon::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	if ( m_hWeaponItem )
	{
		// There is a scripted weapon item to pick up.  The goal is satisfied 
		// only if don't match, as the goal should be reevalated at this point.

		CAIWMFact* pFact = GetWeaponItemTaskFact( m_pAI );
		if ( pFact )
		{
			return ( m_hWeaponItem != pFact->GetTargetObject() );
		}

		// There is a targeted weapon item to pick up.  The goal is satisfied 
		// only if don't match, as the goal should be reevalated at this point.

		else if ( m_hWeaponItem != m_pAI->GetAIBlackBoard()->GetBBTargetObject() )
		{
			return true;
		}

		return false;
	}
	else
	{
		// Goal does not specify a weapon item.  If:
		// 1) A weapon item is not targetted
		// 2) There are no tasks to pick up a weapon item
		// ...then this goal is satisfied.

		if ( !m_pAI->HasTarget( kTarget_WeaponItem ) && !GetWeaponItemTaskFact( m_pAI ) )
		{
			return true;
		}
	}

	return false;
}

