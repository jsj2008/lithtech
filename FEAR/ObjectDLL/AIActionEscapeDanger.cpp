// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionEscapeDanger.cpp
//
// PURPOSE : AIActionEscapeDanger implementation.
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionEscapeDanger.h"
#include "AI.h"
#include "AIStateGoto.h"
#include "AIPathMgrNavMesh.h"
#include "AIWorkingMemory.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "AINodeStimulus.h"
#include "AINodeMgr.h"
#include "PlayerObj.h"
#include "Projectile.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionEscapeDanger, kAct_EscapeDanger );

#define ESCAPE_PAUSE_TIME	2.f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionEscapeDanger::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionEscapeDanger::CAIActionEscapeDanger()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionEscapeDanger::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionEscapeDanger::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Position is valid

	m_wsWorldStateEffects.SetWSProp( kWSK_DisturbanceExists, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionEscapeDanger::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionEscapeDanger::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
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

	// Get a safe distance away from the danger!
	// Factor in the current distance from the danger, to ensure AI
	// runs somewhere if possible.

	float fRadius = pAI->GetPosition().Dist( pFact->GetPos() );
	fRadius += pFact->GetRadius();

	// Bail if no clear path can be found.

	if( !g_pAIPathMgrNavMesh->EscapePathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), pFact->GetPos(), fRadius, pAI->GetLastNavMeshPoly(), NULL ) )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionEscapeDanger::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionEscapeDanger::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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

	// Get a safe distance away from the danger!
	// Factor in the current distance from the danger, to ensure AI
	// runs somewhere if possible.

	LTVector vDangerPos = pFact->GetPos();
	float fRadius = pAI->GetPosition().Dist( vDangerPos );
	fRadius += pFact->GetRadius();

	// Bail if no clear path can be found.

	LTVector vDest;
	if( !g_pAIPathMgrNavMesh->EscapePathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDangerPos, fRadius, pAI->GetLastNavMeshPoly(), &vDest ) )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );

	// Set the destination.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
	pGoto->SetDest( vDest );

	// Replace the actual danger position with the position that AI 
	// started running from danger from. This gives the AI a valid place 
	// to come back and search.

	pFact->SetPos( pAI->GetPosition(), 1.f );

	// Pause before starting to fire.

	pAI->GetAIBlackBoard()->SetBBFirePauseTimeLimit( g_pLTServer->GetTime() + ESCAPE_PAUSE_TIME );

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

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionEscapeDanger::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Bail if we are not aware of danger.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Search for the source of the danger.

	LTVector vDangerPos = pFact->GetPos();
	SearchForDangerOrigin( pAI, vDangerPos );

	// Clear memory of danger.

	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToDanger::SearchForDangerOrigin
//
//	PURPOSE:	Search for the origin of the danger.
//
// ----------------------------------------------------------------------- //

void CAIActionEscapeDanger::SearchForDangerOrigin( CAI* pAI, const LTVector& vDangerPos )
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
//	ROUTINE:	CAIActionEscapeDanger::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionEscapeDanger::IsActionComplete( CAI* pAI )
{
	// Goto is complete if state is complete.

	if( !pAI->GetState() )
	{
		AIASSERT(0, pAI->GetHOBJECT(), "CAIActionEscapeDanger::IsActionComplete : AI has no state.");
		return false;
	}

	// Goto is complete.

	if ( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionEscapeDanger::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionEscapeDanger::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
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

