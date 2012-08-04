// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceSquad.cpp
//
// PURPOSE : AITargetSelectDisturbanceSquad class definition
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDisturbanceSquad.h"
#include "AI.h"
#include "AITarget.h" 
#include "AICoordinator.h" 
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceSquad, kTargetSelect_DisturbanceSquad );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceSquad::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbanceSquad::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI is already targeting something.

	if( pAI->HasTarget( kTarget_All ) )
	{
		return false;
	}

	// AI is not in a squad.

	ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
	if( !pSquad )
	{
		return false;
	}

	// Find a squad member targeting some disturbance.

	CAI* pMember = NULL;
	bool bTargetingDisturbance = false;
	uint32 cMembers = pSquad->GetNumSquadMembers();
	LTObjRef* pMembers = pSquad->GetSquadMembers();
	if( pMembers )
	{
		for( uint32 iMember=0; iMember < cMembers; ++iMember )
		{
			pMember = (CAI*)g_pLTServer->HandleToObject( pMembers[iMember] );
			if( pMember && pMember->HasTarget( kTarget_Disturbance ) )
			{
				bTargetingDisturbance = true;
				break;
			}
		}
	}

	// Preconditions are met if the squad is targeting a disturbance.

	return bTargetingDisturbance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceSquad::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbanceSquad::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not in a squad.

	ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
	if( !pSquad )
	{
		return;
	}

	// Find a squad member targeting a disturbance.

	CAI* pMember = NULL;
	bool bTargetingDisturbance = false;
	uint32 cMembers = pSquad->GetNumSquadMembers();
	LTObjRef* pMembers = pSquad->GetSquadMembers();
	if( pMembers )
	{
		for( uint32 iMember=0; iMember < cMembers; ++iMember )
		{
			pMember = (CAI*)g_pLTServer->HandleToObject( pMembers[iMember] );
			if( pMember && pMember->HasTarget( kTarget_Disturbance ) )
			{
				break;
			}
		}
	}

	// Bail if we failed to find a squad member.

	if( !pMember )
	{
		return;
	}

	// Bail if squad member is not targeting a disturbance.

	if( !pMember->HasTarget( kTarget_Disturbance ) )
	{
		return;
	}
	HOBJECT hTarget = pMember->GetAIBlackBoard()->GetBBTargetObject();

	// Bail if squad member has no memory of the character.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Disturbance );
	factQuery.SetTargetObject( hTarget );
	CAIWMFact* pFact = pMember->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Create a memory for this character.

	CAIWMFact* pTargetFact;
	pTargetFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Disturbance );
	pTargetFact->SetTargetObject( hTarget, 1.f );

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	pTargetFact->SetStimulus( eStimulusType, eStimulusID, 0.f );
	pTargetFact->SetPos( pFact->GetPos(), 1.f );
	pTargetFact->SetRadius( 0.f, 1.f );

	// Target the disturbance.

	TargetDisturbance( pAI, pFact );
}
