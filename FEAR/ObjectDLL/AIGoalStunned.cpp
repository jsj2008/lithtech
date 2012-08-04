// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalStunned.cpp
//
// PURPOSE : AIGoalStunned class implementation
//
// CREATED : 09/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalStunned.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalStunned, kGoal_Stunned );

#define FLASHLIGHT_FLICKER_TIME		"StunnedFlashlightFlickerTime"
#define FLASHLIGHT_DISABLE_TIME		"StunnedFlashlightDisableTime"
#define SLOWMO_RECORD				"StunnedSlowMo"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalStunned::CAIGoalStunned()
{
}

CAIGoalStunned::~CAIGoalStunned()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalStunned::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalPatrol
//              
//----------------------------------------------------------------------------

void CAIGoalStunned::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalStunned::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalStunned::CalculateGoalRelevance()
{
	// Goal is relevant if we have been stunned.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) >= 1.f )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// We have not been stunned.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalStunned::ActivateGoal()
{
	super::ActivateGoal();

	// Kick stunner into slo-mo.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Only do slow-mo if stunner is a player.

	HOBJECT hStunner = pFact->GetTargetObject();
	if( !IsPlayer( hStunner ) )
	{
		return;
	}
	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hStunner );

	// Enter slow-mo.

	HRECORD hSlowMoRecord = g_pAIDB->GetMiscRecordLink( SLOWMO_RECORD );
	if( hSlowMoRecord )
	{
		g_pGameServerShell->EnterSlowMo( hSlowMoRecord, -1.0f, NULL, CPlayerObj::kTransition | CPlayerObj::kUsePlayerTimeScale );
	}

	// Disable the flashlight immediately after some flickering.

	ILTBaseClass* pStunner = g_pLTServer->HandleToObject( hStunner );
	float fFlickerTime = g_pAIDB->GetMiscFloat( FLASHLIGHT_FLICKER_TIME );

	char szCommand[128];
	LTSNPrintF( szCommand, LTARRAYSIZE( szCommand ), "FLASHLIGHT DISABLE %f", fFlickerTime );
	g_pCmdMgr->QueueMessage( m_pAI, pStunner, szCommand );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalStunned::DeactivateGoal()
{
	super::DeactivateGoal();

	// Disable the flashlight of whoever stunned me.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		// Enable the flashlight after some delay.

		char szName[64];
		HOBJECT hStunner = pFact->GetTargetObject();
		ILTBaseClass* pStunner = g_pLTServer->HandleToObject( hStunner );
		g_pLTServer->GetObjectName( hStunner, szName, LTARRAYSIZE(szName) );

		float fDelay = g_pAIDB->GetMiscFloat( FLASHLIGHT_DISABLE_TIME );

		char szCommand[128];
		LTSNPrintF( szCommand, LTARRAYSIZE( szCommand ), "delay %.2f (msg %s (FLASHLIGHT ENABLE))", fDelay, szName );
		g_pCmdMgr->QueueCommand( szCommand, m_pAI, pStunner );
	}

	// Clear the knowledge of reacting to getting stunned.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Invalid );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalStunned::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStunned::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalStunned::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// We have handled getting stunned.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject );
	if( pProp && ( pProp->eAIWorldStateEventWSValue == kWSE_Damage ) )
	{
		return true;
	}

	// We have not handled getting stunned.

	return false;
}


