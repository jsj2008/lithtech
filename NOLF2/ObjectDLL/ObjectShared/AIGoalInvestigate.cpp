// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalInvestigate.cpp
//
// PURPOSE : AIGoalInvestigate implementation
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalInvestigate.h"
#include "AIGoalMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHumanState.h"
#include "AIHuman.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "AINodeMgr.h"
#include "AIGoalButeMgr.h"
#include "AIStimulusMgr.h"
#include "AISounds.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AIPathMgr.h"
#include "AICentralKnowledgeMgr.h"
#include "AITarget.h"

extern CAIStimulusMgr* g_pAIStimulusMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalInvestigate, kGoal_Investigate);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalInvestigate::CAIGoalInvestigate()
{
	m_eDisturbanceSenseType = kSense_InvalidType;
	m_eDisturbanceStimulusType = kStim_InvalidType;
	m_vStimulusPos			= LTVector( 0.0f, 0.0f, 0.0f );
	m_vStimulusDir			= LTVector( 0.0f, 0.0f, 0.0f );
	m_nStimulusAlarmLevel	= 0;
	m_eStimulusID			= kStimID_Unset;
	m_eAISound				= kAIS_None;
	m_bInvestigationFailed	= LTFALSE;
	m_bLookOnly				= LTFALSE;

	m_bFaceAllyForward = LTFALSE;
	m_bEnforceGuardLimit = LTTRUE;

	m_hStimulusTarget = NULL;
	m_hNodeUseObject = NULL;

	m_pInvestigationVolume = LTNULL;

	m_mapInvestMemory.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eDisturbanceSenseType);
	SAVE_DWORD(m_eDisturbanceStimulusType);
	SAVE_HOBJECT(m_hStimulusTarget);
	SAVE_VECTOR(m_vStimulusPos);
	SAVE_VECTOR(m_vStimulusDir);
	SAVE_DWORD(m_nStimulusAlarmLevel);
	SAVE_DWORD(m_eStimulusID);
	SAVE_HOBJECT(m_hNodeUseObject);
	SAVE_DWORD(m_eAISound);
	SAVE_BOOL(m_bInvestigationFailed);
	SAVE_COBJECT(m_pInvestigationVolume);
	SAVE_BOOL(m_bEnforceGuardLimit);
	SAVE_BOOL(m_bFaceAllyForward);
	SAVE_VECTOR(m_vAllyForward);
	SAVE_BOOL(m_bLookOnly);

	AI_INVESTIGATION_MEMORY_MAP::iterator it;
	SAVE_DWORD(m_mapInvestMemory.size());
	for( it = m_mapInvestMemory.begin(); it != m_mapInvestMemory.end(); ++it )
	{
		SAVE_DWORD(it->first);
		SAVE_DWORD(it->second);
	}
}

void CAIGoalInvestigate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eDisturbanceSenseType, EnumAISenseType);
	LOAD_DWORD_CAST(m_eDisturbanceStimulusType, EnumAIStimulusType);
	LOAD_HOBJECT(m_hStimulusTarget);
	LOAD_VECTOR(m_vStimulusPos);
	LOAD_VECTOR(m_vStimulusDir);
	LOAD_DWORD(m_nStimulusAlarmLevel);
	LOAD_DWORD_CAST(m_eStimulusID, EnumAIStimulusID);
	LOAD_HOBJECT(m_hNodeUseObject);
	LOAD_DWORD_CAST(m_eAISound, EnumAISoundType);
	LOAD_BOOL(m_bInvestigationFailed);
	LOAD_COBJECT(m_pInvestigationVolume, AIVolume);
	LOAD_BOOL(m_bEnforceGuardLimit);
	LOAD_BOOL(m_bFaceAllyForward);
	LOAD_VECTOR(m_vAllyForward);
	LOAD_BOOL(m_bLookOnly);

	uint32 cMemories;
	EnumAITargetMatchID eTargetMatchID;
	EnumAISenseType eSenseType;
	LOAD_DWORD(cMemories);
	for( uint32 iMemory=0; iMemory < cMemories; ++iMemory )
	{
		LOAD_DWORD_CAST(eTargetMatchID, EnumAITargetMatchID);
		LOAD_DWORD_CAST(eSenseType, EnumAISenseType);
		m_mapInvestMemory.insert( AI_INVESTIGATION_MEMORY_MAP::value_type( eTargetMatchID, eSenseType ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::ActivateGoal()
{
	super::ActivateGoal();
	
	AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Activating Goal Investigate: AlarmLevel %d", m_pAI->GetAlarmLevel() ) );

	// If no higher priority goal handled SeeEnemy, Investigate simply
	// turns the AI to face the enemy.
	// This can occur when an enemy stands behind an AI in his PersonalBubble,
	// activating SeeEnemy as a 6th sense.

	if( ( m_eDisturbanceSenseType == kSense_SeeEnemy ) &&
		( m_vStimulusPos.DistSqr( m_pAI->GetPosition() ) < g_pAIButeMgr->GetSenses()->fPersonalBubbleDistanceSqr ) )
	{
		m_pAI->FaceObject( m_hStimulusSource );
		m_fCurImportance = 0.f;
		return;
	}

	// Register a disturbance stimulus if AI was not triggered by seeing a disturbed ally.
	// Stimulus is slightly less alarming than whatever stimulated AI. This ensures that 
	// an AI will look towards an enemy before looking towards an ally. Only look towards
	// the ally if the enemy is out of range.

	if( ( m_eDisturbanceSenseType != kSense_SeeAllyDisturbance ) && 
		( m_eDisturbanceSenseType != kSense_HearAllyDisturbance ) )
	{
		uint32 nAlarmLevel = ( m_nStimulusAlarmLevel > 1) ? m_nStimulusAlarmLevel - 1 : 1;
		g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDisturbanceSound, nAlarmLevel, m_pAI->m_hObject, LTNULL, m_pAI->GetPosition(), 1.f );
	}

	// Lock goal.

	m_pGoalMgr->LockGoal(this);

	// Clear old investigation knowledge.

	if( m_pInvestigationVolume )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_InvestigatingVolume, m_pAI, m_pInvestigationVolume );
	}

	// Check if there is a UseObject node pointing to the stimulus.

	if( m_hStimulusTarget )
	{
		HOBJECT hObj = m_hStimulusTarget;
		AINode* pNode = g_pAINodeMgr->FindUseObjectNode( kNode_Disturbance, hObj, LTFALSE );
		m_hStimulusTarget = hObj;
		m_hNodeUseObject = ( pNode ) ? pNode->m_hObject : NULL;
	}

	// Investigate a UseObjectNode.

	if( m_hNodeUseObject != NULL )
	{
		m_pAI->SetState( kState_HumanUseObject );

		AINodeUseObject* pNode = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
		if( !pNode )
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalInvestigate::ActivateGoal: Cannot find UseObject node." );
		}

		CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
		pStateUseObject->SetAwareness(kAP_Investigate);
		pStateUseObject->SetNode(pNode);
		pStateUseObject->SetWeaponPosition(kAP_Up);
		pStateUseObject->TurnOnLights( LTTRUE );
		pStateUseObject->SetAlertFirst( LTTRUE );

		// Find volume containing node.

		AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume( m_pAI->m_hObject, pNode->GetPos(), eAxisAll, m_pAI->GetDims().y*2.0f, (AISpatialRepresentation*)m_pAI->GetLastVolume() );
		if( pVolume )
		{
			m_pInvestigationVolume = pVolume;
		}

		// Find a Disturbance command string.

		HSTRING hstrCmd = pNode->GetSmartObjectCommand( kNode_Disturbance );
		if(hstrCmd != LTNULL)
		{
			pStateUseObject->SetSmartObjectCommand(hstrCmd);
		}
		
		AIASSERT(hstrCmd != LTNULL, m_pAI->m_hObject, "CAIGoalInvestigate::ActivateGoal: No command string found for attractors.");
	}

	// Investigate a stimulus.

	else if( m_hStimulusSource ) 
	{
		SetStateInvestigate();
	}

	// Uh oh, no stimulus.  Shouldn't get here... but if we do, search.
	
	else {
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Activating Goal Investigate: No Stimulus!" ) );
		SearchOrAware( LTFALSE );
	}

	// Just face the stimulus if someone else is already investigating in the destination volume.

	if( ReserveInvestigationVolume( m_pAI->GetCurrentVolume() ) )
	{
		m_pAI->FacePos( m_vStimulusPos );
	}

	m_pAI->SetAwareness( kAware_Suspicious );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear old investigation knowledge.

	m_nStimulusAlarmLevel	= 0; 
	m_eStimulusID			= kStimID_Unset; 

	if( m_pInvestigationVolume )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_InvestigatingVolume, m_pAI, m_pInvestigationVolume );
		m_pInvestigationVolume = LTNULL;
	}

	if( m_bInvestigationFailed )
	{
		m_pAI->PlaySound( kAIS_InvestigateFail, LTFALSE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::SetStateInvestigate
//
//	PURPOSE:	Set state to Investigate.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::SetStateInvestigate()
{
	m_bLookOnly = LTFALSE;

	// If disturbance is in view of AI, and he is not yet majorly disturbed,
	// just look at it.
	// Always walk to investigate a coin.

	if( ( !m_pAI->IsMajorlyAlarmed() ) &&
		( m_eDisturbanceStimulusType != kStim_EnemyCoinSound ) &&
		( ( m_pAI->GetLastRelaxedTime() == 0.f ) ||
		  ( m_pAI->GetLastRelaxedTime() + 30.f < g_pLTServer->GetTime() ) ) )
	{
		// Disturbance is already in view.

		if( m_pAI->IsPositionVisibleFromEye( CAI::DefaultFilterFn, NULL, m_vStimulusPos, 1000000.f, LTTRUE, LTTRUE ) )
		{
			m_pAI->SetState( kState_HumanAware );
			m_pGoalMgr->UnlockGoal(this);
			return;
		}

		uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder | 
									AIVolume::kVolumeType_Stairs |
									AIVolume::kVolumeType_JumpOver | 
									AIVolume::kVolumeType_JumpUp | 
									AIVolume::kVolumeType_AmbientLife |
									AIVolume::kVolumeType_Teleport;

		if( g_pAIVolumeMgr->StraightPathExists( m_pAI, m_pAI->GetPosition(), m_vStimulusPos, m_pAI->GetVerticalThreshold(), dwExcludeVolumes, m_pAI->GetLastVolume() ) )
		{
			// Look at a disturbance.
				
			m_pAI->SetState( kState_HumanLookAt );
			CAIHumanStateLookAt* pLookAtState = (CAIHumanStateLookAt*)m_pAI->GetState();
			pLookAtState->SetPos(m_vStimulusPos);
			m_bLookOnly = LTTRUE;
			return;
		}
	}


	// Assume gunfire is coming at me, to help find the nearest 
	// volume pos to the shooter.

	switch( m_eDisturbanceSenseType )
	{
		case kSense_HearEnemyWeaponFire:
		case kSense_HearEnemyWeaponImpact:
			if( !m_hStimulusTarget )
			{
				m_vStimulusDir = m_pAI->GetPosition() - m_vStimulusPos;
				m_vStimulusDir.Normalize();
			}
			break;
	}


	// Go to actually investigate.

	m_pAI->ClearAndSetState( kState_HumanInvestigate );

	m_bFaceAllyForward = LTFALSE;

	// Get pointer to ally, if disturbance caused by AI.

	CAI* pAlly = LTNULL;
	if( IsAI( m_hStimulusSource ) )
	{
		pAlly = (CAI*)g_pLTServer->HandleToObject( m_hStimulusSource );
	}

	if( pAlly )
	{
		// Investigate what alerted an ally.

		if( pAlly->HasTarget() && ( pAlly->GetAwareness() == kAware_Alert ) )
		{
			m_vStimulusPos = pAlly->GetTarget()->GetVisiblePosition();
			m_vStimulusDir = pAlly->GetPosition() - m_vStimulusPos;
			m_vStimulusDir.Normalize();
		}

		// Look at whatever the ally was looking at.

		else {
			m_bFaceAllyForward = LTTRUE;
			m_vAllyForward = pAlly->GetTorsoForward();
		}
	}

	// Do not try to investigate of there is no path to the stimulus.

	if( !g_pAIPathMgr->HasPath( m_pAI, m_vStimulusPos, m_vStimulusDir ) )
	{
		SearchOrAware( LTTRUE );
		m_pAI->FacePos( m_vStimulusPos );
		return;
	}

	// Setup the Investigate state.

	CAIHumanStateInvestigate* pState = (CAIHumanStateInvestigate*)m_pAI->GetState();
	pState->Reset(m_hStimulusSource, m_eDisturbanceSenseType, m_eAISound, m_vStimulusPos, m_vStimulusDir);
	pState->SetSearch(m_bSearch);

	// Pause before moving if not investigating gunfire.

	if( m_eDisturbanceSenseType != kSense_HearEnemyWeaponFire )
	{
		pState->SetPause( LTTRUE );
	}


	// Find volume containing stimulus.

	LTVector vVolumePos;
	LTVector vRayEnd = m_vStimulusPos + ( m_vStimulusDir * 5000.f );
	AIVolume *pVolume = g_pAIVolumeMgr->FindNearestIntersectingVolume( m_vStimulusPos, vRayEnd, m_pAI->GetDims().z, m_pAI->GetVerticalThreshold(), &vVolumePos);
	if( pVolume )
	{
		m_pInvestigationVolume = pVolume;
	}

	// If investigating an ally disturbance, walk within some radius of ally.

	if( IsCharacter( m_hStimulusSource ) )
	{
		CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hStimulusSource );
		if( pChar )
		{
			CharacterAlignment eAlignment = GetAlignment( pChar->GetRelationSet(), m_pAI->GetRelationData() );
			if( eAlignment == LIKE )
			{
				LTFLOAT fCloseEnoughSqr = 2.f * ( m_pAI->GetRadius() + pChar->GetRadius() );
				fCloseEnoughSqr *= fCloseEnoughSqr;
				pState->SetCloseEnoughDistSqr( fCloseEnoughSqr );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanInvestigate:
			HandleStateInvestigate();
			break;

		case kState_HumanUseObject:
			HandleStateUseObject();
			break;

		case kState_HumanAware:
			if( m_pGoalMgr->IsGoalLocked( this ) )
			{
				m_pGoalMgr->UnlockGoal( this );
			}
			break;

		case kState_HumanLookAt:
			HandleStateLookAt();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanSearch:
			if( !HandleGuardLimit() )
			{
				HandleStateSearch();
			}
			break;

		// Unexpected State.
		default: AIASSERT1(0, m_pAI->m_hObject, "CAIGoalInvestigate::UpdateGoal: Unexpected State: %s", s_aszStateTypes[pState->GetStateType()] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleStateInvestigate
//
//	PURPOSE:	Determine what to do when in state Investigate.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleStateInvestigate()
{
	// If we are guarding, check if we have strayed too far.

	if( HandleGuardLimit() )
	{
		return;
	}

	// Check the status of the investigate state.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_pAI->SetState( kState_HumanAware );
			m_pGoalMgr->UnlockGoal(this);
			break;

		case kSStat_FailedSetPath:
			{
				if( m_eDisturbanceSenseType != kSense_SeeEnemy )
				{
					// Look at a disturbance.
				
					m_pAI->SetState( kState_HumanLookAt );
					CAIHumanStateLookAt* pLookAtState = (CAIHumanStateLookAt*)m_pAI->GetState();
					pLookAtState->SetPos(m_vStimulusPos);
					return;
				}

				// Go aware.

				SearchOrAware( LTTRUE );
			}
			break;

		case kSStat_PathComplete:
			{
				if( m_bFaceAllyForward )
				{
					m_pAI->FaceDir( m_vAllyForward );
				}

				// Search or go aware.

				SearchOrAware( LTFALSE );
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalInvestigate::HandleStateInvestigate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleStateUseObject()
{
	// Check if smart object is no longer disturbed.

	AINodeUseObject* pNode = (AINodeUseObject*)g_pLTServer->HandleToObject(m_hNodeUseObject);
	if( !pNode )
	{
		return;
	}

	// If we are guarding, check if we have strayed too far.

	if( HandleGuardLimit() )
	{
		return;
	}

	// Check the status of the UseObject state.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			if( pNode->GetSmartObjectState() != kState_SmartObjectDisturbed )
			{
				SetStateInvestigate();
				return;
			}
			break;

		case kSStat_PathComplete:
			break;

		case kSStat_StateComplete:
			{
				m_hNodeUseObject = LTNULL;

				// Search or go aware.

				SearchOrAware( LTFALSE );
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalInvestigate::HandleStateUseObject: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleStateLookAt
//
//	PURPOSE:	Determine what to do when in state LookAt.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleStateLookAt()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			{
				if(	m_bLookOnly )
				{
					m_bLookOnly = LTFALSE;
					m_fCurImportance = 0.f;
					return;
				}

				// Search or go aware.

				SearchOrAware( LTFALSE );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalInvestigate::HandleStateLookAt: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleGuardLimit
//
//	PURPOSE:	Check if we have strayed too far from guard node.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalInvestigate::HandleGuardLimit()
{
	// Ignore guard limit if AI is not coming from a
	// relaxed state of awareness.

	if( !m_bEnforceGuardLimit )
	{
		return LTFALSE;
	}

	// Do not stop in the middle of a door volume.

	if( m_pAI->HasCurrentVolume() &&
		m_pAI->GetCurrentVolume()->HasDoors() )
	{
		return LTFALSE;
	}

	// Ignore guard radius while AI owns a Talk node (and has a Talk goal).

	if( g_pAINodeMgr->FindOwnedNode( kNode_Talk, m_pAI->m_hObject ) )
	{
		return LTFALSE;
	}

	// If we are guarding, check if we have strayed too far.

	AINodeGuard* pNodeGuard = (AINodeGuard*)g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject );
	if( pNodeGuard )
	{
		if( pNodeGuard->GetPos().DistSqr( m_pAI->GetPosition() ) > pNodeGuard->GetRadiusSqr() )
		{
			// Search or go aware.

			SearchOrAware( LTTRUE );

			// Do not play investigate failed sound, when we fail due to radius constraint.

			m_bInvestigationFailed = LTFALSE;

			LTVector vDir = m_vStimulusPos - m_pAI->GetPosition();
			vDir.Normalize();

			if( m_pAI->GetForwardVector().Dot( vDir ) < c_fFOV90 )
			{
				m_pAI->FacePos( m_vStimulusPos );
			}
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleVolume*
//
//	PURPOSE:	Only allow 1 AI to investigate a volume at a time.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleVolumeEnter(AIVolume* pVolume)
{
	// Only care if we're heading towards the disturbance.

	if( ( m_pAI->GetState()->GetStateType() != kState_HumanInvestigate ) && 
		( m_pAI->GetState()->GetStateType() != kState_HumanUseObject ) )
	{
		return;
	}

	ReserveInvestigationVolume( pVolume );
}

// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleVolumeExit(AIVolume* pVolume)
{
	if( !m_pInvestigationVolume )
	{
		return;
	}

	// Only care if we are leaving the disturbance.

	if( m_pAI->GetState()->GetStateType() != kState_HumanSearch ) 
	{
		return;
	}

	if( pVolume && ( pVolume == m_pInvestigationVolume ) )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_InvestigatingVolume, m_pAI, m_pInvestigationVolume );
		m_pInvestigationVolume = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::ReserveInvestigationVolume
//
//	PURPOSE:	Reserve volume or return TRUE if alreay reserved.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalInvestigate::ReserveInvestigationVolume(AIVolume* pVolume)
{
	if( !m_pInvestigationVolume )
	{
		return LTFALSE;
	}

	if( pVolume && ( pVolume == m_pInvestigationVolume ) )
	{
		// Someone is already investigating this volume.

		if( g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_InvestigatingVolume, m_pAI, m_pInvestigationVolume ) > 0 )
		{
			m_pInvestigationVolume = LTNULL;
			SearchOrAware( LTFALSE );
			return LTTRUE;
		}
		
		// Register the first AI to investigate this volume.
		// Only one AI may investigate a volume, to prevent clumping.

		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_InvestigatingVolume, m_pAI, m_pInvestigationVolume, LTTRUE );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::SearchOrAware
//
//	PURPOSE:	Search if possible, else go aware.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::SearchOrAware(LTBOOL bOnlyAware)
{
	// Clear investigation records.

	m_nStimulusAlarmLevel	= 0; 
	m_eStimulusID			= kStimID_Unset; 

	// Search if appropriate.

	if( ( !bOnlyAware ) && 
		( (m_pAI->IsMajorlyAlarmed() || m_bSearch) && m_pAI->CanSearch() ) )
	{
		SetStateSearch();
	}
	else {

		// Turn away from whatever was being investigated.
		// (e.g. so that AI won't be standing there facing file cabinet.)

		if( m_pAI->GetState()->GetStateType() == kState_HumanUseObject )
		{
			m_pAI->FaceDir( -m_pAI->GetForwardVector() );
		}

		m_pAI->SetState( kState_HumanAware );
		m_pGoalMgr->UnlockGoal(this);
		m_bInvestigationFailed = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::SelectTriggeredAISound
//
//	PURPOSE:	Select appropriate AI sound.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalInvestigate::SelectTriggeredAISound(EnumAISoundType* peAISoundType)
{
	// Do not play any disturbance sound if AI is already alert.
	// Do not play any disturbance sound if AI is transitioning (e.g. getting up).
	// Do not play any disturbance sound if AI is already investigating
	// or aware.

	if( ( m_pAI->GetAwareness() == kAware_Alert ) ||
		( m_pAI->GetAnimationContext()->IsTransitioning() ) ||
		( m_pGoalMgr->IsCurGoal( this ) && ( m_pAI->GetState()->GetStateType() != kState_HumanSearch ) ) )
	{
		return LTFALSE;
	}

	// Do not play any disturbance sound if AI is already talking.

	if( ( m_eDisturbanceSenseType != kSense_SeeAllyDisturbance ) &&
		( m_pAI->IsPlayingDialogSound() ) )
	{
		return LTFALSE;
	}

	EnumAISoundType	eAISound = kAIS_None;

	// Determine which disturbance sound to use.
	// NOTE: It would be cool if senses just knew if they we
	//       audio or visual.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	switch( m_eDisturbanceSenseType )
	{
		case kSense_SeeAllyDeath:
			// CheckBody goal will take care of the sound for this.
			break;

		case kSense_HearAllyDeath:
			{
				// Only play a sound for HearAllyDeath if there's not a straight 
				// path to the stimulus.  If there is a straight path, the AI
				// will probably check the body, and it makes more sense
				// to hear the check body sound first.

				uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder | 
											AIVolume::kVolumeType_Stairs |
											AIVolume::kVolumeType_JumpOver | 
											AIVolume::kVolumeType_JumpUp | 
											AIVolume::kVolumeType_AmbientLife |
											AIVolume::kVolumeType_Teleport;

				if( !g_pAIVolumeMgr->StraightPathExists( m_pAI, m_pAI->GetPosition(), m_vStimulusPos, m_pAI->GetVerticalThreshold(), dwExcludeVolumes, m_pAI->GetLastVolume() ) )
				{
					eAISound = kAIS_DisturbanceHeardAlarming;
				}
			}
			break;

		case kSense_SeeEnemy:
			eAISound = kAIS_DisturbanceSeenAlarming;
			break;

		case kSense_SeeEnemyLean:
			eAISound = kAIS_DisturbanceSeenUnsure;
			break;

		case kSense_SeeEnemyDisturbance:
		case kSense_SeeEnemyLightDisturbance:
			if( pAIHuman->GetAlarmLevel() >= m_pAI->GetBrain()->GetMajorAlarmThreshold() )
			{
				eAISound = kAIS_DisturbanceSeenMajor;
			}
			else if( IsAIVolume( m_hStimulusSource ) )
			{
				AIVolume* pVolume = (AIVolume*)g_pLTServer->HandleToObject( m_hStimulusSource );
				if( pVolume->IsLit() )
				{
					eAISound = kAIS_LightsOn;
				}
				else {
					eAISound = kAIS_LightsOff;
				}
			}
			else {
				eAISound = kAIS_DisturbanceSeenMinor;
			}
			break;

		case kSense_SeeEnemyWeaponImpact:
			eAISound = kAIS_DisturbanceSeenMajor;
			break;

		case kSense_SeeAllySpecialDamage:
			eAISound = kAIS_AssistTrappedAlly;
			break;

		// If an ally causes the disturbance, it is actually the ally who speaks.

		case kSense_SeeAllyDisturbance:
			if( IsAI( m_hStimulusSource ) )
			{
				CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hStimulusSource );
				if( ( !pAI->IsPlayingDialogSound() ) &&
					( !pAI->GetDamageFlags() ) )
				{
					if( pAI->GetAwareness() == kAware_Alert )
					{
						pAI->PlaySound( kAIS_AlertAllyMajor, LTFALSE );
					}
					else {
						pAI->PlaySound( kAIS_AlertAllyMinor, LTFALSE );
					}
				}
			}
			break;
	}

	if( eAISound != kAIS_None )
	{
		*peAISoundType = eAISound;
		return LTTRUE;
	}


	// Do not play a disturbance sound for the following senses if 
	// the disturbance is outside of the AI's guard radius.

	AINodeGuard* pNodeGuard = (AINodeGuard*)g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject );
	if( pNodeGuard )
	{
		if( pNodeGuard->GetPos().DistSqr( m_vStimulusPos ) > pNodeGuard->GetRadiusSqr() )
		{
			return LTFALSE;
		}
	}


	switch( m_eDisturbanceSenseType )
	{
		case kSense_HearEnemyWeaponFire:
		case kSense_HearEnemyWeaponImpact:
		case kSense_HearAllyPain:
		case kSense_HearAllyWeaponFire:
			eAISound = kAIS_DisturbanceHeardAlarming;
			break;

		case kSense_HearEnemyFootstep:
		case kSense_HearEnemyDisturbance:
			if( pAIHuman->GetAlarmLevel() >= m_pAI->GetBrain()->GetMajorAlarmThreshold() )
			{
				eAISound = kAIS_DisturbanceHeardMajor;
			}
			else {
				eAISound = kAIS_DisturbanceHeardMinor;
			}
			break;

	}

	*peAISoundType = eAISound;
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalInvestigate::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage( damage );

	// Record whatever triggered this goal.

	AIBM_Stimulus* pStimulus = g_pAIButeMgr->GetStimulus( kStim_EnemyWeaponImpactSound );
	if( pStimulus )
	{
		m_nStimulusAlarmLevel	= pStimulus->nAlarmLevel;
		m_eDisturbanceSenseType = kSense_HearEnemyWeaponImpact;
		m_eDisturbanceStimulusType = kStim_InvalidType;

		if( m_hStimulusSource )
		{
			g_pLTServer->GetObjectPos( m_hStimulusSource , &m_vStimulusPos );
		}
	}

	// Do not investigate without a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
	{
		m_pAI->IncrementAlarmLevel( m_nStimulusAlarmLevel );
		return LTFALSE;
	}

	// Activate the goal.

	m_bEnforceGuardLimit = LTFALSE;
	m_bRequiresImmediateResponse = LTTRUE;
	SetCurToBaseImportance();

	// Reactivate if already active.

	if( m_pGoalMgr->IsCurGoal( this ) )
	{
		m_pAI->ClearState();
		ActivateGoal();
	}

	// Always return false to allow normal damage handling.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalInvestigate::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Ignore Guard radius if AI sees player, or hears an alarm.

	if( pSenseRecord->eSenseType & 
		( kSense_SeeEnemy | kSense_HearEnemyAlarm ) )
	{
		m_bEnforceGuardLimit = LTFALSE;
	}

	// Handle trigger senses.

	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		LTBOOL bNewDisturbance = LTFALSE;

		// Do not investigate without a weapon.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
		{
			m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );
			return LTFALSE;
		}

		// Only enforce the guard radius when the AI is investigating from 
		// a relaxed state of awareness.

		if( m_pAI->GetAwareness() == kAware_Relaxed )
		{
			m_bEnforceGuardLimit = LTTRUE;
		}


		// Screen out senses under certain conditions.

		switch( pSenseRecord->eSenseType )
		{	
			// Ignore disturbed allies if already investigating.
		
			case kSense_SeeAllyDisturbance:
			case kSense_HearAllyDisturbance:
				if( ( m_pGoalMgr->IsCurGoal( this ) || 
					( m_fCurImportance == m_fBaseImportance ) ) ||
					m_pAI->IsSuspicious() )
				{
					return LTFALSE;
				}
				break;

			// Ignore weapon impacts on myself.
			// They are already handled by HandleDamage.

			case kSense_SeeEnemyWeaponImpact:
				if( pSenseRecord->hLastStimulusTarget == m_pAI->m_hObject )
				{
					return LTFALSE;
				}
				break;
		}

		
		// If stimulus has no duration (meaning it lasts forever, i.e. visibile 
		// stimuli like an open drawer), ignore if it occured before I 
		// returned to a relaxed state.

		if( pSenseRecord->pAIBM_Last_Stimulus && 
			( pSenseRecord->pAIBM_Last_Stimulus->fDuration == 0.f ) &&
			( m_pAI->GetLastRelaxedTime() > pSenseRecord->fLastStimulationTime ) )
		{
			return LTFALSE;
		}

		// Start a new investigation.

		LTBOOL bRepeatedDisturbance = LTFALSE;

		HOBJECT hTarget = pSenseRecord->hLastStimulusTarget;
		EnumAITargetMatchID eTargetMatchID = pSenseRecord->eLastTargetMatchID;

		// AI has never noticed this target to be disturbed before.

		if( m_mapInvestMemory.find( eTargetMatchID ) == m_mapInvestMemory.end() )
		{
			bNewDisturbance = LTTRUE;
		}

		// AI has noticed this target to be disturbed previously.

		else {

			// If we have investigated this target before, for a different type
			// of sense, ignore it.  (e.g. we heard the drawer open, and then we
			// saw the same drawer open).
			// If we have investigated this target before for the same
			// type of sense, pay attention because this is a new instance.
			// (e.g. someone opened the drawer again).

			EnumAISenseType eSenseType;
			AI_INVESTIGATION_MEMORY_MAP::iterator it;
			for( it = m_mapInvestMemory.lower_bound( eTargetMatchID ); it != m_mapInvestMemory.upper_bound( eTargetMatchID ); ++it )
			{
				eSenseType = it->second;
				if( pSenseRecord->eSenseType == eSenseType )
				{
					bRepeatedDisturbance = LTTRUE;
					break;
				}
			}
		}

		// Ignore this disturbance if it is old news.
		// Bail so that we do not increment the alarm level.

		if( !( bNewDisturbance || bRepeatedDisturbance ) )
		{
			// Commit the new sense type for disturbance target to memory.
			// (e.g. record that we have now seen and heard the drawer open).

			m_mapInvestMemory.insert( AI_INVESTIGATION_MEMORY_MAP::value_type( eTargetMatchID, pSenseRecord->eSenseType ) );

			// Remember the more alarming stimulus.

			if( pSenseRecord->nLastStimulusAlarmLevel > m_nStimulusAlarmLevel )
			{
				m_eStimulusID = pSenseRecord->eLastStimulusID; 
			}

			return LTFALSE;
		}


		// Update the alarm level with new stimulus.
		// AI elevates his alarm level as high as an ally's.

		uint32 nAlarmLevel;
		if( ( pSenseRecord->eSenseType == kSense_SeeAllyDisturbance ) || 
			( pSenseRecord->eSenseType == kSense_HearAllyDisturbance ) )
		{
			CAI* pAlly = (CAI*)g_pLTServer->HandleToObject( m_hStimulusSource );
			if( pAlly )
			{
				m_pAI->SetAlarmLevel( Max( pAlly->GetAlarmLevel(), m_pAI->GetAlarmLevel() ) );
			
				// Do not record any significant alarm level if stimulated by an ally.

				nAlarmLevel = 1;
			}
		}
		else {
			nAlarmLevel = pSenseRecord->nLastStimulusAlarmLevel;
			m_pAI->IncrementAlarmLevel( nAlarmLevel );
		}

		// Focus on more alarming stimulus.

		if( ( m_pAI->GetState()->GetStateType() == kState_HumanInvestigate ) && 
			( ( nAlarmLevel < m_nStimulusAlarmLevel ) ||
			  ( m_eStimulusID == pSenseRecord->eLastStimulusID ) ) )
		{
			return LTFALSE;
		}

		m_bInvestigationFailed = LTFALSE;

		// Record whatever triggered this goal.
		m_eDisturbanceSenseType	= pSenseRecord->eSenseType;
		m_hStimulusTarget		= hTarget;
		m_vStimulusPos			= pSenseRecord->vLastStimulusPos;
		m_vStimulusDir			= pSenseRecord->vLastStimulusDir;
		m_nStimulusAlarmLevel	= nAlarmLevel; 
		m_eStimulusID			= pSenseRecord->eLastStimulusID; 

		if( pSenseRecord->pAIBM_Last_Stimulus )
		{
			m_eDisturbanceStimulusType = pSenseRecord->pAIBM_Last_Stimulus->eStimulusType;
		}
		else {
			m_eDisturbanceStimulusType = kStim_InvalidType;
		}

		// HACK:  for TO2 to make AI shutup after killing someone one co-op.

		if( IsPlayer( m_hStimulusSource ) )
		{
			CCharacter* pPlayer = (CCharacter*)g_pLTServer->HandleToObject( m_hStimulusSource );
			if( !pPlayer->IsDead() )
			{
				m_pAI->MuteAISounds( LTFALSE );
			}
		}
		
		// END HACK


		// Commit new stimulus to memory.

		if( hTarget && bNewDisturbance )
		{
			m_mapInvestMemory.insert( AI_INVESTIGATION_MEMORY_MAP::value_type( eTargetMatchID, pSenseRecord->eSenseType ) );
		}

		// Immediately kill any dialogue.

		m_pGoalMgr->KillDialogue();

		return LTTRUE;
	}

	return LTFALSE;
}

