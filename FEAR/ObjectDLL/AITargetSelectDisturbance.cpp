// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbance.cpp
//
// PURPOSE : AITargetSelectDisturbance class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDisturbance.h"
#include "AI.h"
#include "AITarget.h" 
#include "AIStimulusMgr.h" 
#include "AISoundMgr.h" 
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkPlayer.h"
#include "AIPathMgrNavMesh.h"
#include "AIQuadTree.h"
#include "AIWorkingMemory.h"
#include "AICoordinator.h"
#include "CharacterDB.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbance, kTargetSelect_Disturbance );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::Constructor
//
//	PURPOSE:	Factory Constructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectDisturbance::CAITargetSelectDisturbance()
{
	m_fMinCharacterConfidence = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbance::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI is not aware of any disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return false;
	}

	// HACK:  FEAR AI need to ignore bodies while searching.

	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, NULL );
	if( eStimulusType == kStim_DeathVisible )
	{
		ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
		CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
		if( pSquad && pSquad->GetCurrentActivity() &&
			pSquad->GetCurrentActivity()->GetActivityClassType() == kActivity_Search )
		{
			return false;
		}
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbance::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Ensure AI is at least suspicious.

	if( pAI->GetAIBlackBoard()->GetBBAwareness() < kAware_Suspicious )
	{
		pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Suspicious );
	}

	// Bail if AI is not aware of any disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return;
	}

	// Bail if the AI is already targeting this disturbance.

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	if( pAI->GetAIBlackBoard()->GetBBTargetStimulusID() == eStimulusID 
		&& kTarget_Disturbance == pAI->GetAIBlackBoard()->GetBBTargetType(  ) )
	{
		return;
	}

	// Target the disturbance.

	TargetDisturbance( pAI, pFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::Deactivate
//
//	PURPOSE:	Deactivate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbance::Deactivate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear any pending sounds, since we are no longer targeting the disturbance.
	// Sound weird when AI says "What was that?" while firing at you!

	g_pAISoundMgr->ClearPendingAISounds( pAI->m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::TargetDisturbance
//
//	PURPOSE:	Target a disturbance and record targeting data.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbance::TargetDisturbance( CAI* pAI, CAIWMFact* pFact )
{
	// Sanity check.

	if( !( pAI && pFact ) )
	{
		return;
	}

	// Record the existence of a disturbance in the AI's world state.

	pAI->GetAIWorldState()->SetWSProp( kWSK_DisturbanceExists, pAI->m_hObject, kWST_bool, true );


	// Play a sound corresponding to the type of stimulus.

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	switch( eStimulusType )
	{
		case kStim_WeaponFireSound:
		case kStim_WeaponImpactSound:
		case kStim_WeaponReloadSound:
		case kStim_DisturbanceSound:
		case kStim_FootstepSound:
		case kStim_DeathSound:
		case kStim_PainSound:
			{
				HOBJECT hAlly = g_pAICoordinator->FindAlly( pAI->m_hObject, NULL );
				if( hAlly )
				{
					// "Check it out!"
					// "Roger!"

					g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_OrderInvestigate, kAISndCat_DisturbanceHeard, NULL, 1.f );
					g_pAISoundMgr->RequestAISoundSequence( hAlly, kAIS_Affirmative, pAI->m_hObject, kAIS_OrderInvestigate, kAIS_OrderInvestigate, kAISndCat_Event, NULL, 0.3f );
				}
				else {
					// "What was that?"

					g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_DisturbanceHeardAlarming, kAISndCat_DisturbanceHeard, NULL, 1.f );
				}
			}
			break;

		// "Flashlight!"

		case kStim_FlashlightBeamVisible:
			{
				g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_DisturbanceSeenFlashlight, kAISndCat_Event, NULL, 0.5f );

				HOBJECT hAlly = g_pAICoordinator->FindAlly( pAI->m_hObject, NULL );
				if( hAlly )
				{
					// "Check it out!"
					// "Roger!"

					g_pAISoundMgr->RequestAISoundSequence( hAlly, kAIS_OrderInvestigate, pAI->m_hObject, kAIS_DisturbanceSeenFlashlight, kAIS_DisturbanceSeenFlashlight, kAISndCat_DisturbanceHeard, NULL, 0.3f );
					g_pAISoundMgr->RequestAISoundSequence( pAI->m_hObject, kAIS_Affirmative, hAlly, kAIS_OrderInvestigate, kAIS_DisturbanceSeenFlashlight, kAISndCat_Event, NULL, 0.3f );
				}
			}
			break;
	}

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Disturbance );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( eStimulusType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( eStimulusID );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( pFact->GetTargetObject() );


	// Record initial disturbance position.
	// If the stimulus is dynamic, AITarget will track its movement.

	LTVector vTargetPos = pFact->GetPos();
	pAI->GetAIBlackBoard()->SetBBTargetPosition( vTargetPos );

	// If the disturbance is coming from an ally's weapon fire sound,
	// then treat the disturbance position as the position of whatever
	// the ally is firing at.

	if( IsAI( pFact->GetTargetObject() ) )
	{
		CAI* pOtherAI = (CAI*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( pOtherAI && 
			pOtherAI->HasTarget( kTarget_Character ) &&
			( eStimulusType == kStim_WeaponFireSound ) &&
			( kCharStance_Like == g_pCharacterDB->GetStance( pAI->GetAlignment(), pOtherAI->GetAlignment() ) ) )
		{
			LTVector vTargetPos = pOtherAI->GetAIBlackBoard()->GetBBTargetPosition();
			pAI->GetAIBlackBoard()->SetBBTargetPosition( vTargetPos );
			pFact->SetPos( vTargetPos, 1.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbance::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if there is a 'better' disturbance target (more recent or more 
	// alarming)

	CAIWMFact* pMaxDisturbanceFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( pMaxDisturbanceFact )
	{
		EnumAIStimulusID eStimulusID;
		EnumAIStimulusType eStimulusType;
		pMaxDisturbanceFact->GetStimulus( &eStimulusType, &eStimulusID );
		if( pAI->GetAIBlackBoard()->GetBBTargetStimulusID() != eStimulusID )
		{
			return false;
		}
	}

	// Keep targeting the disturbance.

	return true;
}
