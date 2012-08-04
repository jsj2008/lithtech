// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "AI.h"
#include "AITarget.h"
#include "AISensorMgr.h"
#include "AIUtils.h"
#include "AIBrain.h"
#include "CharacterMgr.h" 
#include "CharacterDB.h" 
#include "AIWorkingMemory.h" 
#include "AIMovement.h" 
#include "AIBlackBoard.h" 
#include "AIStimulusMgr.h" 
#include "AISoundMgr.h" 
#include "NodeTrackerContext.h"
#include "AIWeaponUtils.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMeshLinkPlayer.h"
#include "CharacterHitBox.h"
#include "AICombatOpportunity.h"
#include "AICoordinator.h"

DEFINE_AI_FACTORY_CLASS(CAITarget);


#define MAX_PHASE				8

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Constructor/Destructor
//
//	PURPOSE:	Factory con/destructor
//
// ----------------------------------------------------------------------- //

CAITarget::CAITarget()
{
	m_hVisionBlocker = NULL;

	m_fCurMovementInaccuracy = 0.f;

	m_bTriggerTracker = false;
	m_bTrackingLastVisible = false;

	m_eAITargetSelection = kTargetSelect_InvalidType;
	m_eTargetAlignment = kCharAlignment_Invalid;

	m_vTargetVelocity.Init();
	m_vVisiblePosition.Init();

	m_fTargetDistSqr = 0.f;

	m_bCanUpdateVisibility = true;

	m_dwLastTargetPosTrackingFlags = kTargetTrack_Normal;

	m_fPushSpeed = 0.f;
	m_fPushMinDist = 0.f;
	m_fPushMinDistSqr = 0.f;
	m_fPushThreshold = 64.f;

	m_nPhase = 2;
	m_fPhaseStep = 0.f;

	m_pAI = NULL;

	ClearTarget( NULL );

	m_bPrimaryOnLeft = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Save(ILTMessage_Write *pMsg)
{
    if ( !g_pLTServer || !pMsg ) return;

	SAVE_BOOL(m_bCanUpdateVisibility);
	SAVE_HOBJECT(m_hVisionBlocker);

	SAVE_DWORD( m_eAITargetSelection );
	SAVE_DWORD( m_eTargetAlignment );

	SAVE_BOOL( m_bTriggerTracker );
	SAVE_BOOL( m_bTrackingLastVisible );

	SAVE_FLOAT(m_fCurMovementInaccuracy);

	SAVE_DWORD(m_dwLastTargetPosTrackingFlags);

	SAVE_VECTOR(m_vTargetVelocity);
	SAVE_VECTOR(m_vVisiblePosition);
	SAVE_FLOAT(m_fTargetDistSqr);
	SAVE_INT(m_nPhase);
	SAVE_FLOAT(m_fPhaseStep);

	SAVE_FLOAT(m_fPushSpeed);
	SAVE_FLOAT(m_fPushMinDist);
	SAVE_FLOAT(m_fPushMinDistSqr);
	SAVE_FLOAT(m_fPushThreshold);

	SAVE_BOOL(m_bPrimaryOnLeft);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Load
//
//	PURPOSE:	Loads our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Load(ILTMessage_Read *pMsg)
{
    if ( !g_pLTServer || !pMsg ) return;

	LOAD_BOOL(m_bCanUpdateVisibility);
	LOAD_HOBJECT(m_hVisionBlocker);

	LOAD_DWORD_CAST( m_eAITargetSelection, EnumAITargetSelectType );
	LOAD_DWORD_CAST( m_eTargetAlignment, EnumCharacterAlignment );

	LOAD_BOOL( m_bTriggerTracker );
	LOAD_BOOL( m_bTrackingLastVisible );

	LOAD_FLOAT(m_fCurMovementInaccuracy);

	LOAD_DWORD(m_dwLastTargetPosTrackingFlags);

	LOAD_VECTOR(m_vTargetVelocity);
	LOAD_VECTOR(m_vVisiblePosition);
	LOAD_FLOAT(m_fTargetDistSqr);
	LOAD_INT(m_nPhase);
	LOAD_FLOAT(m_fPhaseStep);

	LOAD_FLOAT(m_fPushSpeed);
	LOAD_FLOAT(m_fPushMinDist);
	LOAD_FLOAT(m_fPushMinDistSqr);
	LOAD_FLOAT(m_fPushThreshold);

	LOAD_BOOL(m_bPrimaryOnLeft);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Init
//
//	PURPOSE:	Intialized targeting for an AI.
//
// ----------------------------------------------------------------------- //

void CAITarget::InitTarget(CAI* pAI)
{
	m_pAI = pAI;

	if( m_pAI )
	{
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromEye( false );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromWeapon( false );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibilityConfidence( 0.f );
		m_pAI->GetAIBlackBoard()->SetBBTargetLastVisibleTime( 0.f );
		m_pAI->GetAIBlackBoard()->SetBBTargetTrueNavMeshPoly( kNMPoly_Invalid );
		m_pAI->GetAIBlackBoard()->SetBBTargetReachableNavMeshPoly( kNMPoly_Invalid );

		LTVector vDims;
		vDims.Init();
		m_pAI->GetAIBlackBoard()->SetBBTargetDims( vDims );
	}

	m_hVisionBlocker = NULL;
	m_bCanUpdateVisibility = true;
	m_eTargetAlignment = kCharAlignment_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::ClearTarget
//
//	PURPOSE:	Clear current target.
//
// ----------------------------------------------------------------------- //

void CAITarget::ClearTarget(CAI* pAI)
{
	InitTarget( pAI );
	if( pAI )
	{
		if( m_pAI->GetAIBlackBoard()->GetBBTargetObject() )
		{
			AITRACE( AIShowTarget, ( pAI->m_hObject, "Clearing Target." ) );
		}

		m_pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_None );
		m_pAI->GetAIBlackBoard()->SetBBTargetObject( NULL );
		m_pAI->GetAIBlackBoard()->SetBBTargetStimulusID( kStimID_Unset );
		m_pAI->GetAIBlackBoard()->SetBBTargetStimulusType( kStim_InvalidType );
		m_pAI->GetAIBlackBoard()->SetBBTargetPosUpdateTime( 0.f );
	}

	DeselectAITargetSelection();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::ClearInvalidCharacterFacts
//
//	PURPOSE:	Marks invalid character facts for deletion.  This ought 
//				to be done in the sensors that manage these facts; we 
//				need to know immediately prior to the use of these facts
//				if they are valid however.
//
// ----------------------------------------------------------------------- //

void CAITarget::ClearInvalidCharacterFacts()
{
	AIWORKING_MEMORY_FACT_LIST::const_iterator itFact;
	const AIWORKING_MEMORY_FACT_LIST* pFactList = m_pAI->GetAIWorkingMemory()->GetFactList();
	for( itFact = pFactList->begin(); itFact != pFactList->end(); ++itFact )
	{
		// Ignore deleted facts.

		CAIWMFact* pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about characters.

		if ( pFact->GetFactType() != kFact_Character )
		{
			continue;
		}

		// Delete facts about dead characters.

		if( IsDeadCharacter( pFact->GetTargetObject() ) )
		{
			m_pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
			continue;
		}

		// Delete facts about characters whose alignments are unexpected.

		CCharacter* pCharacter = CCharacter::DynamicCast( pFact->GetTargetObject() );
		if ( pCharacter )
		{
			EnumCharacterStance eExpectedStance =
				( pFact->GetFactFlags() & kFactFlag_CharacterIsTraitor ) ? kCharStance_Like : kCharStance_Hate;

			EnumCharacterStance eActualStance = 
				g_pCharacterDB->GetStance( m_pAI->GetAlignment(), pCharacter->GetAlignment() );

			if ( eExpectedStance != eActualStance )
			{
				m_pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
				continue;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateTarget
//
//	PURPOSE:	Select the best target.
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateTarget()
{
	uint32 dwCharTargets = kTarget_Character | kTarget_Leader | kTarget_Follower;
	ENUM_AI_TARGET_TYPE eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	if ( eTargetType & dwCharTargets )
	{
		// Clear dead targets.

		if ( IsDeadAI( hTarget ) )
		{
			ClearTarget( m_pAI );

			// Clear memories of dead or removed characters.

			CAIWMFact factTargetQuery;
			factTargetQuery.SetFactType( kFact_Character );
			factTargetQuery.SetTargetObject( NULL );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factTargetQuery );
		}

		// Clear targets whose alignment changed

		if ( m_eTargetAlignment != kCharAlignment_Invalid )
		{
			CCharacter* pChar = CCharacter::DynamicCast( hTarget );
			if ( pChar->GetAlignment() != m_eTargetAlignment )
			{
				ClearTarget( m_pAI );
			}
		}
	}

	// Mark invalid tasks for deletion.

	ClearInvalidCharacterFacts();

	// Force target re-selection.

	if( m_pAI->GetAIBlackBoard()->GetBBInvalidateTarget() )
	{
		DeselectAITargetSelection();
		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( false );
	}


	// We are currently targeting something.

	CAITargetSelectAbstract* pTargetSelect;
	if( m_eAITargetSelection != kTargetSelect_InvalidType )
	{
		// Current target has been invalidated.

		pTargetSelect = g_pAITargetSelectMgr->GetAITargetSelect( m_eAITargetSelection );
		if( !( pTargetSelect && pTargetSelect->Validate( m_pAI ) ) )
		{
			DeselectAITargetSelection();
		}
	}


	// We are not currently targeting anything.

	ENUM_AI_TARGET_TYPE ePreviousTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	double fCurTime = g_pLTServer->GetTime();
	if( ( m_eAITargetSelection == kTargetSelect_InvalidType ) &&
		( m_pAI->GetAIBlackBoard()->GetBBSelectTarget() ) )
	{
		// Select a new targeting selection object.

		m_eAITargetSelection = SelectAITargetSelection();
		m_pAI->GetAIBlackBoard()->SetBBSelectTarget( false );

		// Target something.

		if( m_eAITargetSelection != kTargetSelect_InvalidType )
		{
			pTargetSelect = g_pAITargetSelectMgr->GetAITargetSelect( m_eAITargetSelection );
			if( pTargetSelect )
			{
				// Dummy world state used as required parameter for planning functions.
				// This is necessary because AITarget is emulating planning.

				AITRACE( AIShowTarget, ( m_pAI->m_hObject, "Activating Target Select: %s", s_aszTargetSelectTypes[pTargetSelect->GetTargetSelectRecord()->eTargetSelectType] ) );
				pTargetSelect->Activate( m_pAI );

				// Kill dialogue that may be playing when targeting 
				// something for the first time.

				if( m_pAI->IsControlledByDialogue() )
				{
					m_pAI->KillDialogueSound();
					m_pAI->StopDialogue();
				}
			}
		}

		// We found nothing to target.

		else if( ePreviousTargetType != kTarget_None )
		{
			ClearTarget( m_pAI );
			m_pAI->GetAIBlackBoard()->SetBBTargetChangeTime( fCurTime );
		}
	}


	// Target changed.

	eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	if( m_pAI->GetAIBlackBoard()->GetBBTargetChangeTime() == fCurTime )
	{
		// Setup new target.

		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		switch( eTargetType )
		{
			case kTarget_Berserker:
			case kTarget_Character:
			case kTarget_CombatOpportunity:
			case kTarget_Disturbance:
			case kTarget_Interest:
			case kTarget_Leader:
			case kTarget_Follower:
			case kTarget_Object:
			case kTarget_WeaponItem:
				InitTarget( m_pAI );
				SetupTargetObject( hTarget );
				break;

			default:
				ClearTarget( m_pAI );
				break;
		}

		// Reset the target firing count.

		m_pAI->GetAIBlackBoard()->SetBBShotsFiredAtTarget( 0 );

		// Reset node tracking.

		if( eTargetType != kTarget_None )
		{
			TrackTargetPosition();
		}
		ResetNodeTrackingTarget();

		// Record type targeted in the AIs mask, so it may determine 
		// what types of objects it has targeted in the past at some later
		// point in time.the fact that the AI has now targeted this type.

		m_pAI->GetAIBlackBoard()->SetBBTargetedTypeMask( m_pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() | eTargetType );

		// Re-evaluate targets.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}


	// Track a target's position.

	else if( ( eTargetType != kTarget_None ) &&
			 ( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() ) )
	{
		TrackTargetPosition();

		if( m_dwLastTargetPosTrackingFlags == kTargetTrack_None )
		{
			ResetNodeTrackingTarget();
		}
	}

	m_dwLastTargetPosTrackingFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();

	// Repeatedly update the node tracking target position if tracking
	// the last visible position instead of tracking the actual position
	// of the head node.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetTrackerTrackLastVisible() )
	{
		m_bTrackingLastVisible = true;
		ResetNodeTrackingTarget();
	}
	else if( m_bTrackingLastVisible )
	{
		m_bTrackingLastVisible = false;
		ResetNodeTrackingTarget();
	}

	// Update node tracking.

	UpdateNodeTracking();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::SelectAITargetSelection
//
//	PURPOSE:	Return the cheapest valid target selection.
//
// ----------------------------------------------------------------------- //

EnumAITargetSelectType CAITarget::SelectAITargetSelection()
{
	// TargetSelect set undefined.

	ENUM_AITargetSelectSet eSet = m_pAI->GetAIBlackBoard()->GetBBAITargetSelectSet();
	if( eSet == kAITargetSelectSet_Invalid )
	{
		return kTargetSelect_InvalidType;
	}

	// Search for the cheapest valid TargetSelect.

	float fMinCost = FLT_MAX;
	EnumAITargetSelectType eMinTargetSelect = kTargetSelect_InvalidType;

	EnumAITargetSelectType eTargetSelect;
	CAITargetSelectAbstract* pTargetSelect;
	uint32 cTargetSelects = g_pAITargetSelectMgr->GetNumAITargetSelects();
	for( uint32 iTargetSelect=0; iTargetSelect < cTargetSelects; ++iTargetSelect )
	{
		// TargetSelect is not in target selection set.

		eTargetSelect = (EnumAITargetSelectType)iTargetSelect;
		if( !g_pAITargetSelectMgr->IsTargetSelectInAITargetSelectSet( eSet, eTargetSelect ) )
		{
			continue;
		}

		// TargetSelect does not exist.

		pTargetSelect = g_pAITargetSelectMgr->GetAITargetSelect( eTargetSelect );
		if( !pTargetSelect )
		{
			continue;
		}

		// Already found a cheaper TargetSelect.

		if( pTargetSelect->GetCost() >= fMinCost )
		{
			continue;
		}

		// Found a cheaper valid TargetSelect.

		if( pTargetSelect->ValidatePreconditions( m_pAI ) )
		{
			eMinTargetSelect = eTargetSelect;
			fMinCost = pTargetSelect->GetCost();
		}
	}

	// Return the cheapest valid TargetSelect.

	return eMinTargetSelect;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::DeselectAITargetSelection
//
//	PURPOSE:	Deactivate the current target selection TargetSelect if one exists.
//
// ----------------------------------------------------------------------- //

void CAITarget::DeselectAITargetSelection()
{
	// We are not currently targeting anything.

	if( m_eAITargetSelection == kTargetSelect_InvalidType )
	{
		if( m_pAI )
		{
			m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
		}

		return;
	}

	// Deactivate the current target selection TargetSelect.

	CAITargetSelectAbstract* pTargetSelect = g_pAITargetSelectMgr->GetAITargetSelect( m_eAITargetSelection );
	if( pTargetSelect )
	{
		AITRACE( AIShowTarget, ( m_pAI->m_hObject, "Deactivating Targeting TargetSelect: %s", s_aszTargetSelectTypes[pTargetSelect->GetTargetSelectRecord()->eTargetSelectType] ) );
		pTargetSelect->Deactivate( m_pAI );
	}

	// Current target has been invalidated.

	m_eAITargetSelection = kTargetSelect_InvalidType;

	if( m_pAI )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::TrackTargetPosition
//
//	PURPOSE:	Track the true and NavMesh position of the current target.
//
// ----------------------------------------------------------------------- //

void CAITarget::TrackTargetPosition()
{
	// Bail if tracking soley based on squad's knowledge of target position,
	// and squad cannot see the target.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() == kTargetTrack_Squad )
	{
		ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
		CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
		if( !( pSquad && pSquad->SquadCanSeeTarget( kTarget_Character ) ) )
		{
			return;
		}
	}

	// Always clear the Track_Once flag.  This only tracks for a single update.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Once )
	{
		uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
		dwFlags = dwFlags & ~kTargetTrack_Once;
		m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );
	}

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	ENUM_NMPolyID eHintPoly = kNMPoly_Invalid;
	LTVector vTargetPos;


	//
	// Determine the target's current actual position.
	//

	ENUM_AI_TARGET_TYPE eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	switch( eTargetType )
	{
		// Targeting a character.

			case kTarget_Berserker:
			case kTarget_Character:
			case kTarget_Leader:
			case kTarget_Follower:
			{
				if( !IsCharacter( hTarget ) )
				{
					AIASSERT( 0, m_pAI->m_hObject, "CAITarget::TrackTargetPosition: CharacterTarget is not a character!." );
					return;
				}
				eHintPoly = m_pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();

				// Consider the target's position to be the position of 
				// character's view transform.  This allows AI to aim at
				// a Player while leaning.

				LTRigidTransform tfView;
				CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
				pChar->GetViewTransform( tfView );
				vTargetPos = tfView.m_vPos;
			}
			break;

		// Targeting a disturbance.

		case kTarget_Disturbance:
			{
				// Try to get the stimulus position from the fact.
				// If the fact no longer exists, maintain its last known 
				// position; this is required as external systems may clear 
				// out this fact.  If this causes a problem, we could have 
				// the target selection fail validation if this occurs.

				CAIWMFact factQuery;
				factQuery.SetFactType( kFact_Disturbance );
				factQuery.SetTargetObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
				factQuery.SetStimulus( m_pAI->GetAIBlackBoard()->GetBBTargetStimulusType(), m_pAI->GetAIBlackBoard()->GetBBTargetStimulusID() );
				CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
				if ( pFact )
				{
					vTargetPos = pFact->GetPos();
				}
				else
				{
					vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
				}

				eHintPoly = m_pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();
			}
			break;

		// Targeting an arbitrary object (non-character).

		case kTarget_Object:
		case kTarget_Interest:
			{
				g_pLTServer->GetObjectPos( hTarget, &vTargetPos );
			}
			break;

		// Targeting a combat opportunity.

		case kTarget_CombatOpportunity:
			{
				g_pLTServer->GetObjectPos( hTarget, &vTargetPos );
			}
			break;

		// Targeting a combat opportunity.

		case kTarget_WeaponItem:
			{
				g_pLTServer->GetObjectPos( hTarget, &vTargetPos );
			}
			break;

		// Targeting something else.

		default:
			AIASSERT( 0, m_pAI->m_hObject, "CAITarget::TrackTargetPosition: Unhandled target type." );
			return;
	}


	// Update the target's position.  Tracks the actual last position
	// of the target.

	m_pAI->GetAIBlackBoard()->SetBBTargetPosition( vTargetPos );
	m_pAI->GetAIBlackBoard()->SetBBTargetPosUpdateTime( g_pLTServer->GetTime() );

	// If the target's position is inside the nav mesh, and the nav mesh
	// poly is pathable from the AI's current NavMesh poly, then update the 
	// blackboards target position.  This tracks the last location of the 
	// target object in a position the AI can move to.


	ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vTargetPos, m_pAI->GetCharTypeMask(), eHintPoly );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrueNavMeshPoly( ePoly );

	// AI has a path to target character's NavMesh position.

	if( ( ePoly != kNMPoly_Invalid ) &&
		( g_pAIPathMgrNavMesh->HasPath( m_pAI, m_pAI->GetCharTypeMask(), ePoly ) ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBTargetReachableNavMeshPoly( ePoly );
		m_pAI->GetAIBlackBoard()->SetBBTargetReachableNavMeshPosition( vTargetPos );
	}

	// Target might be in an AINavMeshLinkPlayer.

	else {
		CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
		if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
		{
			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
			if( pLink && ( pLink->GetNMLinkType() == kLink_Player ) )
			{
				LTVector vNavMeshPos;
				AINavMeshLinkPlayer* pLinkPlayer = (AINavMeshLinkPlayer*)pLink;
				if( pLinkPlayer->FindNearestNavMeshPos( m_pAI, vTargetPos, &vNavMeshPos, &ePoly ) )
				{
					m_pAI->GetAIBlackBoard()->SetBBTargetReachableNavMeshPoly( ePoly );
					m_pAI->GetAIBlackBoard()->SetBBTargetReachableNavMeshPosition( vNavMeshPos );
				}
			}
		}
	}

	// TODO: FIND NEAREST INTERSECTING NMPOLY.
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::ResetNodeTrackingTarget()
//
//	PURPOSE:	Turn on Head and Torso tracking.
//				Return TRUE if AI is facing his target.
//
// ----------------------------------------------------------------------- //

void CAITarget::ResetNodeTrackingTarget()
{
	// No target.

	ENUM_AI_TARGET_TYPE eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	if( eTargetType == kTarget_None )
	{
		return;
	}

	// Target a character's head, or a position in space.

	switch( eTargetType )
	{
		case kTarget_Character:
			{
				// AI is node tracking the last visible position, rather than the
				// actual position.  This is useful for blind firing, and firing 
				// where the target was, instead of at a wall.

				if( m_pAI->GetAIBlackBoard()->GetBBTargetTrackerTrackLastVisible() )
				{
					HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Character );
					factQuery.SetTargetObject( hTarget );
					CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
					if( pFact )
					{
						m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_World );
						m_pAI->GetAIBlackBoard()->SetBBTargetTrackerPos( pFact->GetPos() );
						return;
					}
				}
			}
			// Intentionally fall thru.

		case kTarget_Berserker:
		case kTarget_Leader:
		case kTarget_Follower:
			{
				HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
					
				HMODELNODE hNodeHead;
				g_pModelLT->GetNode( hTarget, "Eye_cam", hNodeHead);
				if( hNodeHead == INVALID_MODEL_NODE )
				{
					g_pModelLT->GetNode( hTarget, "Head", hNodeHead);
				}

				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Node );
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModelNode( hNodeHead );
			}
			break;

		case kTarget_Disturbance:
		case kTarget_Interest:
			{
				LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();

				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_World );
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerPos( vTargetPos );
			}
			break;

		case kTarget_Object:
			{
				HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Object );
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
			}
			break;

		case kTarget_CombatOpportunity:
			{
				HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Object );
				m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateNodeTracking()
//
//	PURPOSE:	Update head and torso tracking, and body facing.
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateNodeTracking()
{
	// Tracker settings from commands override those set by the AITarget.
	
	uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTriggerTrackerFlags();
	if( dwFlags != kTrackerFlag_None )
	{	
		m_bTriggerTracker = true;
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( dwFlags );
		return;
	}

	// Reset tracker target if previously using a trigger tracker.

	if( m_bTriggerTracker )
	{
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		ResetNodeTrackingTarget();
		m_bTriggerTracker = false;
	}

	// Do not use tracking nodes if AI has no target.

	ENUM_AI_TARGET_TYPE eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	if( eTargetType == kTarget_None )
	{
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		return;
	}

	// Track the last visible position if the AI has lost track of its target.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() == kTargetTrack_None )
	{
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_World );
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerPos( m_vVisiblePosition );
		return;
	}

	// Track the target's last known position, if one exists.

	LTVector vTargetPos;
	if(	m_pAI->GetAIBlackBoard()->GetBBFaceTargetKnownPos() )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Character );
		factQuery.SetTargetObject( hTarget );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && pFact->GetConfidence( CAIWMFact::kFactMask_Position ) < 1.f )
		{
			vTargetPos = pFact->GetPos();
			m_pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
			m_pAI->GetAIBlackBoard()->SetBBFacePos( vTargetPos );
			return;
		}
	}

	bool bFace = m_pAI->GetAIBlackBoard()->GetBBFaceTarget();

	// Face without using tracking.

	bool bCanTrack = !!( m_pAI->GetAIBlackBoard()->GetBBTargetTrackerFlags() & kTrackerFlag_AimAt );
	if( bFace && !bCanTrack )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		m_pAI->GetAIBlackBoard()->SetBBFaceObject( hTarget );
		return;
	}

	// Do not use tracking if the target is too close.
/**
	vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
	if( bFace && 
		( m_eTargetType == kTarget_Character ) &&
		( m_pAI->GetPosition().DistSqr( vTargetPos ) < 128.f*128.f ) )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		m_pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		m_pAI->GetAIBlackBoard()->SetBBFaceObject( hTarget );
		return;
	}
**/

	// Face target if we hit our tracking limit.

	if( bFace && m_pAI->GetAIBlackBoard()->GetBBTargetTrackerAtLimitX() )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		m_pAI->GetAIBlackBoard()->SetBBFaceObject( hTarget );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateVisibility
//
//	PURPOSE:	Updates the target's visibility
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateVisibility(const LTVector& vCheckPos)
{
	// Do not update visibility again until something external says it's OK.
	// AI sets this to TRUE when it has had a sense update.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !( m_bCanUpdateVisibility && hTarget ) )
	{
		return;
	}

	m_bCanUpdateVisibility = false;

	// Do not see dead targets.
	// This is important for AI behavior in multiplayer, where a 
	// dead players stick around so the HOBJECTs do not get NULLed.

	ENUM_AI_TARGET_TYPE eTargetType = m_pAI->GetAIBlackBoard()->GetBBTargetType();
	CCharacter* pChar = NULL;
	if( ( eTargetType == kTarget_Character ) && IsCharacter( hTarget ) )
	{
		pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
		if( pChar && !pChar->IsAlive() )
		{
			// Invalidate a dead target.

			ClearTarget( m_pAI );
			return;
		}
	}


	//
	// Predict the target's next position based on target's observed velocity.
	//

	LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();

	double fCurTime = g_pLTServer->GetTime();
	float fTimeDelta = (float)(fCurTime - m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime());

	if( fTimeDelta > 0.f )
	{
		LTVector vPredictedPos = m_vVisiblePosition + ( m_vTargetVelocity * fTimeDelta);
		LTVector vDiff = vTargetPos - vPredictedPos;
		
		float fInaccuracyDist = LTMIN( vDiff.Mag(), m_pAI->GetMaxMovementAccuracyPerturb() );
		m_fCurMovementInaccuracy = LTMAX( fInaccuracyDist, m_fCurMovementInaccuracy - ( m_pAI->GetMovementAccuracyPerturbDecay() * fTimeDelta ) );

		m_vTargetVelocity = ( vTargetPos - m_vVisiblePosition ) / fTimeDelta;
	}

	// Keep track of the distance to the target.

	m_fTargetDistSqr = m_pAI->GetPosition().DistSqr( vTargetPos );

	// Incrementally scan the target vertically until he can be seen.

	LTVector vDims = m_pAI->GetAIBlackBoard()->GetBBTargetDims();
	vTargetPos.y += ( vDims.y - ( float(m_nPhase) * m_fPhaseStep ) );


	bool bTargetVisibleFromEye = false;
	bool bTargetVisibleFromWeapon = false;
	float fTargetVisibilityConfidence = 0.f;
	float fTargetRadius = 0.f;

	// Character targets have cached visibility which decays.

	CAIWMFact* pFact = NULL;
	if (eTargetType == kTarget_Character)
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Character);
		factQuery.SetTargetObject(hTarget);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			fTargetVisibilityConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus );
			if( fTargetVisibilityConfidence >= 0.2f )
			{
				bTargetVisibleFromEye = true;
			}
			fTargetRadius = pFact->GetRadius();
		}
		else
		{
			// No memory of the target exists.  Clear it.
			ClearTarget( m_pAI );
			return;
		}
	}

	// If the target is a combat opportunity, assume it is permanently 
	// visible from the eye, as it doesn't move.  If we need to decay
	// this visibility, we can later.

	if( ( eTargetType == kTarget_CombatOpportunity ) ||
		( eTargetType == kTarget_Object ) )
	{
		bTargetVisibleFromEye = true;
		fTargetRadius = vDims.Mag();
		g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

		// Uncomment to draw the visibility test for the CombatOpportunity.
		//
		//DebugLineSystem& sys = LineSystem::GetSystem(this, "vis_test");
		//sys.AddBox(vTargetPos, m_vTargetDims );
		//sys.AddLine(vTargetPos, vTargetPos + LTVector(1000,0,0) );
		//sys.AddLine(vTargetPos, vTargetPos + LTVector(0,1000,0) );
		//sys.AddLine(vTargetPos, vTargetPos + LTVector(0,0,1000) );

		fTargetVisibilityConfidence = 1.0f;
	}

	HOBJECT hVisionBlocker = NULL;

	if( bTargetVisibleFromEye )
	{
	    float fSeeEnemyDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fTargetRadius;
		fSeeEnemyDistanceSqr *= fSeeEnemyDistanceSqr;

		// Check visibility.

		// Verify that this vector is going to hit the character anyway.  No 
		// point in doing an expensive intersect segment if it is a miss.

		bool bHitnodeMissed = false;
		if (IsCharacter(hTarget))
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
			if (pCharacter)
			{
				LTVector vDir = (vTargetPos - vCheckPos).GetUnit( );
				if( !g_pModelsDB->GetSkeletonNodeAlongPath( hTarget, pCharacter->GetModelSkeleton( ), vCheckPos, vDir ))
				{
					bHitnodeMissed = true;
				}
			}
		}

		if (!bHitnodeMissed)
		{
			bool bIsAlert = ( GetAI()->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert ) ||
							( GetAI()->GetAIBlackBoard()->GetBBAwareness() == kAware_Suspicious );

			if ( !GetAI()->CanShootThrough() )
			{
				if ( GetAI()->IsObjectPositionVisible(CAI::DefaultFilterFn, NULL, vCheckPos, hTarget, vTargetPos, fSeeEnemyDistanceSqr, !bIsAlert, false, &hVisionBlocker ) )
				{
					bTargetVisibleFromWeapon = true;
				}
			}
			else
			{
				if( GetAI()->IsObjectPositionVisible(CAI::ShootThroughFilterFn, CAI::ShootThroughPolyFilterFn, vCheckPos, hTarget, vTargetPos, fSeeEnemyDistanceSqr, !bIsAlert, true, &hVisionBlocker ) )
				{
					CAI* pTargetAI = NULL;
					if( IsAI( hTarget ) )
					{
						pTargetAI = (CAI*)g_pLTServer->HandleToObject( hTarget );
					}

					CAI* pVisionBlocker = NULL;
					if( !g_pCharacterMgr->RayIntersectAI( vCheckPos, vTargetPos, GetAI(), pTargetAI, &pVisionBlocker, 2500.f ) )
					{
						bTargetVisibleFromWeapon = true;
					}
					else if( pVisionBlocker )
					{
						hVisionBlocker = pVisionBlocker->m_hObject;
					}
				}

				// Ignore AI that are not currently client-solid.
				// (e.g. AI that are knocked out).

				else if( IsAI( hVisionBlocker ) )
				{
					uint32 dwFlags;
					g_pCommonLT->GetObjectFlags( hVisionBlocker, OFT_User, dwFlags);
					if( ! ( dwFlags & USRFLG_AI_CLIENT_SOLID ) )
					{
						hVisionBlocker = NULL;
					}
				}
			}
		}
	}

	if( bTargetVisibleFromEye )
	{
        m_pAI->GetAIBlackBoard()->SetBBTargetLastVisibleTime( fCurTime );

		if( eTargetType == kTarget_Character )
		{
			ENUM_NMPolyID ePoly = m_pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();
			m_pAI->GetAIBlackBoard()->SetBBTargetLastVisibleNavMeshPoly( ePoly );

			LTVector vPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
			m_pAI->GetAIBlackBoard()->SetBBTargetLastVisiblePosition( vPos );		

			if( m_pAI->GetAIBlackBoard()->GetBBTargetFirstThreatTime() == 0.f )
			{
				m_pAI->GetAIBlackBoard()->SetBBTargetFirstThreatTime( fCurTime );
			}
		}
	}

	if( bTargetVisibleFromWeapon )
	{
		m_vVisiblePosition = vTargetPos;

		m_hVisionBlocker = NULL;
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromEye( bTargetVisibleFromEye );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromWeapon( bTargetVisibleFromWeapon );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibilityConfidence( fTargetVisibilityConfidence );
	}
	else 
	{
		// We have an enemy, but can't see him.

		if( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_SeekEnemy )
		{
			m_vVisiblePosition = vTargetPos;
		}

		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromEye( bTargetVisibleFromEye );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibleFromWeapon( bTargetVisibleFromWeapon );
		m_pAI->GetAIBlackBoard()->SetBBTargetVisibilityConfidence( fTargetVisibilityConfidence );
		m_hVisionBlocker = hVisionBlocker;

		// Only change the phase if target is not visible.
		// Minimize the phase to one, so that we always test visibility 
		// within the dims rather than testing an extent (which will surely miss).

		m_nPhase = (m_nPhase+1) % MAX_PHASE;
		m_nPhase = LTMAX( 1, m_nPhase );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdatePush
//
//	PURPOSE:	Push the target back if it is too close.
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdatePush( CCharacter* pChar )
{
	if( m_fPushSpeed > 0.f )
	{
		// Handle collisions by pushing the target back.

		LTVector vPos = GetAI()->GetPosition();
		if( vPos.DistSqr( m_vVisiblePosition ) < m_fPushMinDistSqr )
		{
			pChar->PushCharacter(vPos, m_fPushMinDist + m_fPushThreshold, 0.f, GetAI()->GetSenseUpdateRate(), m_fPushSpeed );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::SetupTargetObject
//
//	PURPOSE:	Internal function for handling setting a target object.
//
// ----------------------------------------------------------------------- //

void CAITarget::SetupTargetObject(HOBJECT hObj)
{
	m_pAI->GetAIBlackBoard()->SetBBTargetObject( hObj );

	AITRACE( AIShowTarget, ( m_pAI->m_hObject, "Setting new Target: %s", GetObjectName( hObj ) ) );

	LTVector vDims;
	if( hObj )
	{
		g_pPhysicsLT->GetObjectDims( hObj, &vDims );
		m_fPhaseStep = ( vDims.y * 2.f ) / (float)MAX_PHASE;
		m_nPhase = 2;
	}
	else {
		vDims.Init();
		m_fPhaseStep = 0.f;
	}

	m_pAI->GetAIBlackBoard()->SetBBTargetDims( vDims );

	// If we are targetting a character, cache their alignment.  If this 
	// changes, we need to reevaluate the target as the actions/behaviors
	// in use may no longer be valid.

	if ( IsCharacter( hObj ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hObj );
		if ( pChar )
		{
			m_eTargetAlignment = pChar->GetAlignment();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::GetDebugInfoString
//
//	PURPOSE:	Append to the string any information about the state of 
//				the AITarget.
//
// ----------------------------------------------------------------------- //

void CAITarget::GetDebugInfoString(std::string& OutInfo)
{
	// Display the target TargetSelect name.

	OutInfo += "AT : ";
	if (m_eAITargetSelection != kTargetSelect_InvalidType)
	{
		OutInfo += s_aszTargetSelectTypes[m_eAITargetSelection];
	}
	else
	{
		OutInfo += "None";
	}
	OutInfo += "\n";


	// Display the target object name

	OutInfo += "TO : ";
	if ( NULL != m_pAI->GetAIBlackBoard()->GetBBTargetObject() )
	{
		char szName[48];
		if ( LT_OK == g_pLTServer->GetObjectName(
			m_pAI->GetAIBlackBoard()->GetBBTargetObject(),
			&szName[0], LTARRAYSIZE(szName)) )
		{
			OutInfo += szName;
		}
		else
		{
			OutInfo += "None";
		}
	}
	else
	{
		OutInfo += "None";
	}
	OutInfo += "\n";

	// Display visibility information

	OutInfo += "VE : ";
	OutInfo += m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() ? "1" : "0";
	OutInfo += "\n";

	OutInfo += "VW : ";
	OutInfo += m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ? "1" : "0";
	OutInfo += "\n";

	char szBuf[16];
	float fVisibilityConfidence = m_pAI->GetAIBlackBoard()->GetBBTargetVisibilityConfidence();
	LTSNPrintF( szBuf, LTARRAYSIZE(szBuf), "%f", fVisibilityConfidence );
	OutInfo += "VC : ";
	OutInfo += szBuf;
	OutInfo += "\n";
}

