// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLongRecoilHelmetPiercing.cpp
//
// PURPOSE : AIActionLongRecoilHelmetPiercing class implementation
//
// CREATED : 02/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionLongRecoilHelmetPiercing.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLongRecoilHelmetPiercing, kAct_LongRecoilHelmetPiercing );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLongRecoilHelmetPiercing::GetActionProbability
//
//	PURPOSE:	Return the probability of taking this action.
//
// ----------------------------------------------------------------------- //

float CAIActionLongRecoilHelmetPiercing::GetActionProbability( CAI* pAI )
{
	// Intentionally do not call super.
	// Always return the unmodified probablity from the Action record.

	return m_pActionRecord->fActionProbability;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLongRecoilHelmetPiercing::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionLongRecoilHelmetPiercing::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Get a damage fact to use for the prop generation.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if (!pFact || !DidDamage(pAI, pFact))
	{
		return false;
	}

	// Bail if the damage type is not helmet piercing.

	DamageType eDamageType;
	pFact->GetDamage( &eDamageType, NULL, NULL );
	if( eDamageType != DT_HELMET_PIERCING &&
		eDamageType != DT_ENERGY
		)
	{
		return false;
	}

	// Bail if AI was not hit in the head.

	ModelsDB::HNODE hModelNode = pAI->GetModelNodeLastHit();
	if( !hModelNode )
	{
		return false;
	}
	EnumAnimProp eBodyAnipProp = g_pModelsDB->GetNodeBodyAnimProp( hModelNode );
	if( eBodyAnipProp != kAP_BODY_Head )
	{
		return false;
	}

	// Default behavior.

	return super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}





