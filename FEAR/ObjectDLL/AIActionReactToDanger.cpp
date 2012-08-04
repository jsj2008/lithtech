// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToDanger.cpp
//
// PURPOSE : AIActionReactToDanger implementation.
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReactToDanger.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AISoundMgr.h"
#include "AIUtils.h"
#include "AICoordinator.h"
#include "PlayerObj.h"
#include "AINodeStimulus.h"
#include "AINodeMgr.h"
#include "Projectile.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToDanger, kAct_ReactToDanger );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReactToDanger::CAIActionReactToDanger()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToDanger::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Position is valid

	m_wsWorldStateEffects.SetWSProp( kWSK_DisturbanceExists, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToDanger::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Return false if the baseclass fails.

	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Bail if we are not aware of danger.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToDanger::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if we are not aware of danger.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set flinch animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Crouch );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Activity, kAP_ATVT_Distress );
	animProps.Set( kAPG_Action, kAP_ACT_Idle );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Do NOT play a threat sound if threatened by a Turret.
	// Instead, allow turret targeting to play something appropriate.

	if( IsPlayer( pFact->GetSourceObject() ) )
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pFact->GetSourceObject() );
		if( pPlayer && pPlayer->GetTurret() )
		{
			return;
		}
	}

	// Play threat sound.

	// "Fire!"

	if( IsAINode( pFact->GetSourceObject() ) )
	{
		EnumAISoundType eAISound = kAIS_Danger;
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetSourceObject() );
		if( pNode && pNode->GetType() == kNode_Stimulus )
		{
			AINodeStimulus* pNodeStim = (AINodeStimulus*)pNode;
			if( pNodeStim )
			{
				eAISound = pNodeStim->GetAISoundType();
			}
		}
		g_pAISoundMgr->RequestAISound( pAI->m_hObject, eAISound, kAISndCat_Event, NULL, 0.f );
	}

	// "Watch out grenade!"
	// "Shit!"

	else if( IsKindOf( pFact->GetSourceObject(), "CProjectile" ) )
	{
		// Don't say anything if ally threw grenade.

		CProjectile* pProjectile = (CProjectile*)g_pLTServer->HandleToObject( pFact->GetSourceObject() );
		if( pProjectile && IsCharacter( pProjectile->GetFiredFrom() ) )
		{
			CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( pProjectile->GetFiredFrom() );
			if( pChar && kCharStance_Like != g_pCharacterDB->GetStance( pAI->GetAlignment(), pChar->GetAlignment() ) )
			{
				ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
				CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
				if( pSquad && pSquad->GetNumSquadMembers() > 1 )
				{
					g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_GrenadeThreat, kAISndCat_Event, NULL, 0.f );
				}
				else {
					g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_GrenadeThreatAlone, kAISndCat_Event, NULL, 0.f );
				}
			}
		}
	}

	// "Shit"

	else 
	{
		g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_GrenadeThreatAlone, kAISndCat_Event, NULL, 0.f );
	}



	// Search for the source of the danger.

	LTVector vDangerPos = pFact->GetPos();
	SearchForDangerOrigin( pAI, vDangerPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToDanger::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// HACK: Ensure AI doesn't get stuck in his flinch animation.

	pAI->ClearState();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::SearchForDangerOrigin
//
//	PURPOSE:	Search for the origin of the danger.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToDanger::SearchForDangerOrigin( CAI* pAI, const LTVector& vDangerPos )
{
	// Sanity check.

	if( !( pAI && pAI->CanSearch() ) )
	{
		return;
	}

	// Find an existing memory for the desire to search, or create one.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Search);
	CAIWMFact* pFactSearch = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFactSearch )
	{
		pFactSearch = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
	}

	// Search from the nearest search node to the danger position.

	LTVector vSearchOrigin = vDangerPos;
	AINode* pNode = g_pAINodeMgr->FindNearestNodeInRadius( pAI, kNode_Search, vDangerPos, 1000.f, true );
	if( pNode )
	{
		vSearchOrigin = pNode->GetPos();
	}

	// Setup the current desire.

	if( pFactSearch )
	{
		pFactSearch->SetDesireType( kDesire_Search, 1.f );
		pFactSearch->SetPos( vSearchOrigin, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToDanger::IsActionComplete( CAI* pAI )
{
	// Bail after flinching for 1 second.

	if( pAI->GetAIBlackBoard()->GetBBStateChangeTime() < g_pLTServer->GetTime() - 1.f )
	{
		return true;
	}

	// Reacting is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Reacting is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToDanger::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !( pAI && pwsWorldStateCur && pwsWorldStateGoal ) )
	{
		return;
	}

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	// Only clear the disturbance world state if AI was reacting to a grenade or AINode.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	if( IsAINode( pFact->GetSourceObject() ) ||
		IsKindOf( pFact->GetSourceObject(), "CProjectile" ) )
	{
		ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
	}
}

