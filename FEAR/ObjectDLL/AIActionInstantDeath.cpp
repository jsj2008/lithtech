// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInstantDeath.cpp
//
// PURPOSE : AIActionInstantDeath class implementation
//
// CREATED : 03/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionInstantDeath.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AnimationContext.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInstantDeath, kAct_InstantDeath );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInstantDeath::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionInstantDeath::CAIActionInstantDeath()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInstantDeath::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionInstantDeath::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI recated to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInstantDeath::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionInstantDeath::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// No destructible.

	CDestructible* pDestructible = pAI->GetDestructible();
	if( !pDestructible )
	{
		return false;
	}

	// Don't die instantly if NeverDestroy is set.

	if( pDestructible->GetNeverDestroy() )
	{
		return false;
	}

	// Don't die if damage was not recent.

	if( pDestructible->GetLastDamageTime() + 0.3f < g_pLTServer->GetTime() )
	{
		return false;
	}

	// Die instantly if this AI is flagged to die right away.
	
	if ( pAI->GetAIBlackBoard()->GetBBInstantDeath() )
	{
		return true;
	}

	// Die instantly if taken by surprise and hit in the head or torso.

	if( pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Relaxed )
	{
		// Determine which body part was hit.

		ModelsDB::HNODE hModelNode = pAI->GetModelNodeLastHit();
		if( hModelNode )
		{
			EnumAnimProp eBodyAnipProp = g_pModelsDB->GetNodeBodyAnimProp( hModelNode );
			if( ( eBodyAnipProp == kAP_BODY_Head ) ||
				( eBodyAnipProp == kAP_BODY_Torso ) )
			{
				return true;
			}
		}
	}

	// Die instantly if hit by TASER_UPGRADE damage while either on stairs 
	// or in the defeated state

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( pFact && DidDamage(pAI, pFact) )
	{
		DamageType eDamageType;
		pFact->GetDamage( &eDamageType, NULL, NULL );

		if ( DT_TASER_UPGRADE == eDamageType || DT_TASER == eDamageType )
		{
			if ( pAI->HasCurrentNavMeshLink() )
			{
				AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetCurrentNavMeshLink() );
				if ( pLink )
				{
					EnumAINavMeshLinkType eNMLType = pLink->GetNMLinkType();
					if ( kLink_Stairs == eNMLType
						|| kLink_DiveThru == eNMLType
						|| kLink_Climb == eNMLType 
						|| kLink_Jump == eNMLType )
					{
						return true;
					}
				}
			}

			// In the 'defeated state'

			EnumAnimProp eAction = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
			if ( kAP_ACT_DefeatedRecoil == eAction )
			{
				return true;
			}
		}
	}


	// Do not die instantly.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInstantDeath::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionInstantDeath::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if no damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact || !DidDamage(pAI, pFact) )
	{
		return;
	}

	// Die instantly.  Use the data from the last damage received -- just up
	// the amount of damage to infinite.  This will insure this action behaves 
	// exactly like a standard death without any additional prompting.  It is 
	// possible that this DamageStruct will be TOO recent; as this behavior is 
	// delayed up to 0.3 seconds, there probably isn't a good way of handling 
	// this, so we will go with the simple solution for now.

	DamageStruct Damage = pAI->GetAIBlackBoard()->GetBBLastDamage();
	Damage.fDamage = Damage.kInfiniteDamage;
	Damage.DoDamage( pAI->GetHOBJECT(), pAI->GetHOBJECT() );
}
