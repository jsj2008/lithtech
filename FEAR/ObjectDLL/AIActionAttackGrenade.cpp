// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackGrenade.cpp
//
// PURPOSE : AIActionAttackGrenade class implementation
//
// CREATED : 4/08/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackGrenade.h"
#include "AI.h"
#include "AIBrain.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AISoundMgr.h"
#include "AIStateAnimate.h"
#include "AITarget.h"
#include "AIPathMgrNavMesh.h"
#include "AICoordinator.h"
#include "NodeTrackerContext.h"
#include "AIWorkingMemoryCentral.h"
#include "CharacterMgr.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackGrenade, kAct_AttackGrenade );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackGrenade::CAIActionAttackGrenade()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenade::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Weapon must be armed.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );

	// Set effects.
	// Target is dead.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsDead, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackGrenade::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Target is not visible.

	if( !pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		return false;
	}

	// AI does not have a weapon of the correct type

	if (!AIWeaponUtils::HasWeaponType(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// AI does not have any ammo for this weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// At a node that does not allow grenade throwing.

	bool bAtNode = false;
	bool bStraightPathCheckRequired = true;
	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( !pNode )
		{
			return false;
		}

		if( !pNode->AllowThrowGrenades() )
		{
			return false;
		}

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_ThreatOutsideFOV ) )
		{
			return false;
		}

		bStraightPathCheckRequired = pNode->RequiresStraightPathToThrowGrenades();
		bAtNode = true;
	}

	// Target is not in range.

	if (!AIWeaponUtils::IsInRange(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// Someone else has thrown a grenade recently.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Knowledge);
	factQuery.SetKnowledgeType(kKnowledge_NextGrenadeTime);
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if(pFact && g_pLTServer->GetTime() < pFact->GetTime() )
	{
		return false;
	}

	// Throw at the last known position.

	CAIWMFact factTargetQuery;
	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	factTargetQuery.SetFactType( kFact_Character );
	factTargetQuery.SetTargetObject( hTarget );
	pFact = pAI->GetAIWorkingMemory()->FindWMFact( factTargetQuery );
	if( !pFact )
	{
		return false;
	}
	LTVector vTargetPos = pFact->GetPos();

	/***
	// Only throw a grenade when not at a node when there is a 
	// straight path to the target.  This is to ensure the 
	// grenade won't bounce back and kill the thrower.

	if( bStraightPathCheckRequired )
	{
		if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vTargetPos, pAI->GetLastNavMeshPoly(), 0.f ) )
		{
			return false;
		}
	}
	***/

	/***
	// Don't throw if someone is closer to the target than me, 
	// because I might hit them!

	if( pAI != g_pCharacterMgr->FindNearestAIAlly( pAI, vTargetPos ) )
	{
		return false;
	}
	***/

	// Get the grenade explosion radius.

	HAMMO hAmmo = AIWeaponUtils::GetWeaponAmmo( pAI, GetWeaponType(), bIsPlanning );
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,true);
	float fRadius = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadius );
	fRadius *= 1.2f;

	// Don't throw if we're in the radius!

	if( pAI->GetPosition().DistSqr( vTargetPos ) < fRadius * fRadius )
	{
		return false;
	}

	// Don't throw if an ally is in the blast radius.

	if( g_pCharacterMgr->FindAIAllyInRadius( pAI, vTargetPos, fRadius ) )
	{
		return false;
	}

/***
	// Don't throw grenade at allies.

	CTList<CCharacter*> lstChars;
	if( g_pCharacterMgr->FindCharactersWithinRadius( &lstChars, vTargetPos, fRadius, CCharacterMgr::kList_AIs ) )
	{
		// Iterate over characters in grenade's radius.

		CCharacter** pCur = lstChars.GetItem(TLIT_FIRST);
		while( pCur )
		{
			CCharacter* pChar = (CCharacter*)*pCur;
			pCur = lstChars.GetItem(TLIT_NEXT);

			// Skip the AI himself.

			if( pChar == pAI )
			{
				continue;
			}

			// Action is invalid if grenade could hit someone 
			// that the AI does not hate.

			if( g_pCharacterDB->GetStance( pAI->GetAlignment(), pChar->GetAlignment() ) != kCharStance_Hate )
			{
				return false;
			}
		}
	}
***/

	// Throw a grenade.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::GetActionProbability
//
//	PURPOSE:	Return the probability of taking this action.
//
// ----------------------------------------------------------------------- //

float CAIActionAttackGrenade::GetActionProbability( CAI* pAI )
{
	return RAISE_BY_DIFFICULTY( super::GetActionProbability( pAI ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::FailActionProbability
//
//	PURPOSE:	Do something if the Action fails to activate due to probability.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenade::FailActionProbability( CAI* pAI )
{
	// Coordinate frequency of grenade attacks across all AIs.

	SetNextThrowTime( pAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::SetNextThrowTime
//
//	PURPOSE:	Set the next time an AI may throw a grenade.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenade::SetNextThrowTime( CAI* pAI )
{
	// Coordinate frequency of grenade attacks across all AIs.

	if( !pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		return;
	}

	float fDelay = GetRandom( pAI->GetBrain()->GetAttackGrenadeThrowTimeMin(), 
							  pAI->GetBrain()->GetAttackGrenadeThrowTimeMax() );
	double fTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY( fDelay );

	AITRACE( AIShowGoals, ( pAI->m_hObject, "NOW: %.2f  NEXT: %.2f", g_pLTServer->GetTime(), fTime ) );

	// Update the global fact tracking grenade throwing frequency, or create 
	// it if it does not yet exist.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Knowledge);
	factQuery.SetKnowledgeType(kKnowledge_NextGrenadeTime);
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);

	if (!pFact)
	{
		pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);
		pFact->SetKnowledgeType( kKnowledge_NextGrenadeTime, 1.f );
	}

	if (pFact)
	{
		pFact->SetTime( fTime );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenade::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set grenade as current weapon.

	pAI->SetCurrentWeapon( kAIWeaponType_Thrown );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set throw animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_Action, kAP_ACT_Throw );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Get the target.

	HOBJECT hTarget = NULL;
	if( pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	// Play grenade attack sound.

	if( g_pAICoordinator->FindAlly( pAI->m_hObject, NULL ) )
	{
		g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_Grenade, kAISndCat_Always, hTarget, 0.f );
	}

	// Coordinate frequency of grenade attacks across all AIs.

	SetNextThrowTime( pAI );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenade::DeactivateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Return to previous weapon.

	ENUM_AIWeaponID eLast = pAI->GetAIBlackBoard()->GetBBLastAIWeaponRecordID();
	const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord( eLast );
	if( pRecord )
	{
		pAI->SetCurrentWeapon( pRecord->eAIWeaponType );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenade::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackGrenade::IsActionComplete( CAI* pAI )
{
	// Attack is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Attack is not complete.

	return false;
}


