// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSpecialDamage.cpp
//
// PURPOSE : AIGoalSpecialDamage implementation
//
// CREATED : 3/21/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalSpecialDamage.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIHumanState.h"
#include "AIGoalButeMgr.h"
#include "AIPathKnowledgeMgr.h"
#include "AINode.h"
#include "AINodeMgr.h"
#include "AIStimulusMgr.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "Attachments.h"
#include "AIMovement.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSpecialDamage, kGoal_SpecialDamage);


// Labels correspond to SmartObjects in AIGoals.txt

#define SMART_OBJECT_SLEEPING			"KnockOut"
#define SMART_OBJECT_SLEEPING_INFINITE	"KnockOutInfinite"
#define SMART_OBJECT_STUNNED			"Stun"
#define SMART_OBJECT_TRAPPED			"BearTrap"
#define SMART_OBJECT_GLUED				"Glue"
#define SMART_OBJECT_LAUGHING			"LaughingGas"
#define SMART_OBJECT_SLIPPING			"Slip"
#define SMART_OBJECT_BLEEDING			"Bleed"
#define SMART_OBJECT_BURNING			"Burn"
#define SMART_OBJECT_CHOKING			"Choke"
#define SMART_OBJECT_POISONING			"Poison"
#define SMART_OBJECT_ELECTROCUTE		"Electrocute"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSpecialDamage::CAIGoalSpecialDamage()
{
	m_eDamageType = DT_UNSPECIFIED;
	m_eDamageAnim = kAP_None;
	m_eSpecialDamageStimID = kStimID_Unset;
	m_bIncapacitated = LTFALSE;
	m_bProgressiveDamage = LTFALSE;
	m_fProgressiveMinTime = 0.f;
	m_hDamager = LTNULL;
	m_bFleeing = LTFALSE;
	m_fRelaxTime = 0.f;
	m_bInfinite = LTFALSE;
	m_bNeedsClearState = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_eDamageType );
	SAVE_DWORD( m_eDamageAnim );
	SAVE_DWORD( m_eSpecialDamageStimID );
	SAVE_BOOL( m_bIncapacitated );
	SAVE_BOOL( m_bProgressiveDamage );
	SAVE_FLOAT( m_fProgressiveMinTime );
	SAVE_HOBJECT( m_hDamager );
	SAVE_BOOL( m_bFleeing );
	SAVE_TIME( m_fRelaxTime );
	SAVE_BOOL( m_bInfinite );
	SAVE_bool( m_bNeedsClearState );
}

void CAIGoalSpecialDamage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_eDamageType, DamageType );
	LOAD_DWORD_CAST( m_eDamageAnim, EnumAnimProp );
	LOAD_DWORD_CAST( m_eSpecialDamageStimID, EnumAIStimulusID );
	LOAD_BOOL( m_bIncapacitated );
	LOAD_BOOL( m_bProgressiveDamage );
	LOAD_FLOAT( m_fProgressiveMinTime );
	LOAD_HOBJECT( m_hDamager );
	LOAD_BOOL( m_bFleeing );
	LOAD_TIME( m_fRelaxTime );
	LOAD_BOOL( m_bInfinite );
	LOAD_bool( m_bNeedsClearState );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::ActivateGoal()
{
	super::ActivateGoal();

	m_pGoalMgr->LockGoal(this);

	m_bFleeing = LTFALSE;
	m_bIncapacitated = LTTRUE;
	m_pAI->SetCurSenseFlags( kSense_None ); 

	// Get the SmartObject corresponding to the damage type.

	LTBOOL bLoopingSound = LTFALSE;
	m_bProgressiveDamage = LTFALSE;
	EnumAISoundType	eSound;
	AIGBM_SmartObjectTemplate* pSmartObject = LTNULL;
	switch( m_eDamageType )
	{
		case DT_SLEEPING:
			if( m_bInfinite )
			{
				pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_SLEEPING_INFINITE );
			}
			else {
				pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_SLEEPING );
			}
			m_eDamageAnim = kAP_DamageSleeping;
			eSound = kAIS_DamageSleeping;
			break;

		case DT_STUN:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_STUNNED );
			m_eDamageAnim = kAP_DamageStunned;
			eSound = kAIS_DamageStun;
			break;

		case DT_BEAR_TRAP:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_TRAPPED );
			m_eDamageAnim = kAP_DamageTrapped;
			eSound = kAIS_DamageBearTrap;
			break;

		case DT_GLUE:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_GLUED );
			m_eDamageAnim = kAP_DamageGlued;
			eSound = kAIS_DamageGlue;
			break;

		case DT_LAUGHING:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_LAUGHING );
			m_eDamageAnim = kAP_DamageLaughing;
			eSound = kAIS_DamageLaughing;
			bLoopingSound = LTTRUE;
			break;
			
		case DT_SLIPPERY:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_SLIPPING );
			m_eDamageAnim = kAP_DamageSlipping;
			eSound = kAIS_DamageSlippery;
			break;

		case DT_BLEEDING:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_BLEEDING );
			m_eDamageAnim = kAP_DamageBleeding;
			eSound = kAIS_Bleeding;
			bLoopingSound = LTTRUE;
			m_bProgressiveDamage = LTTRUE;
			break;

		case DT_BURN:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_BURNING );
			m_eDamageAnim = kAP_DamageBurning;
			eSound = kAIS_Burning;
			m_bProgressiveDamage = LTTRUE;
			break;

		case DT_CHOKE:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_CHOKING );
			m_eDamageAnim = kAP_DamageChoking;
			eSound = kAIS_Choking;
			bLoopingSound = LTTRUE;
			m_bProgressiveDamage = LTTRUE;
			break;

		case DT_ASSS:
		case DT_POISON:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_POISONING );
			m_eDamageAnim = kAP_DamagePoisoning;
			eSound = kAIS_Poisoning;
			bLoopingSound = LTTRUE;
			m_bProgressiveDamage = LTTRUE;
			break;

		case DT_ELECTROCUTE:
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_ELECTROCUTE );
			m_eDamageAnim = kAP_DamageElectrocuting;
			eSound = kAIS_Electrocute;
			bLoopingSound = LTTRUE;
			m_bProgressiveDamage = LTTRUE;
			break;
	}


	// Register a new stimulus.

	if( m_eSpecialDamageStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eSpecialDamageStimID );
		m_eSpecialDamageStimID = kStimID_Unset;
	}

	// Use the DeathVisible stimulus for knocked out AIs, and SpecialDamageVisible for the rest.
	// Make knocked-out AI non-solid.

	switch( m_eDamageType )
	{
		case DT_SLEEPING:
		case DT_SLIPPERY:
			m_eSpecialDamageStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDeathVisible, m_pAI->m_hObject, m_pAI->GetPosition(), 1.f );
			m_pAI->SetClientSolid( LTFALSE );

			//make sure sleeping AI can be searched
			m_pAI->SetUnconscious( true );

			// AI that are sleeping forever cannot be woken.

			m_pAI->SetCanWake( !m_bInfinite );

			// Enemies ignore sleeping AI.
			
			m_pAI->RemovePersistentStimuli();

			break;

		default:
			m_eSpecialDamageStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllySpecialDamageVisible, m_pAI->m_hObject, m_pAI->GetPosition(), 1.f );
			break;
	}


	// AI is suspicious while incapacitated.
	// The main reason to do this is so that an AI that was 
	// chasing will lose interest until seeing the target again.

	m_pAI->SetAwareness( kAware_Suspicious );

	// Try to limp to a cover node for progressive damage.

	if( m_bProgressiveDamage )
	{
		// React for at least 4 seconds.

		m_fProgressiveMinTime = g_pLTServer->GetTime() + 4.f;

		// Do not use any special volume types while damaged.
		// AI needs to clear existing knowledge of paths.

		m_pAI->SetCurValidVolumeMask( AIVolume::kVolumeType_BaseVolume | AIVolume::kVolumeType_Junction );
		m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();

		AINode*	pNode = g_pAINodeMgr->FindNearestNodeFromThreat( m_pAI, kNode_Cover, m_pAI->GetPosition(), m_hDamager, FLT_MAX );
		if( pNode && ( pNode->GetPos().DistSqr( m_pAI->GetPosition() ) > 64.f*64.f ) )
		{
			m_pAI->ClearAndSetState( kState_HumanGoto );
			CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
			pStateGoto->SetDestNode( pNode->m_hObject );
			pStateGoto->SetMood( m_eDamageAnim );

			// Play damage sound.

			if( bLoopingSound )
			{
				pStateGoto->SetLoopingSound( eSound );
			}
			else {
				m_pAI->PlaySound( eSound, LTTRUE );
			}

			return;
		}
	}


	if( m_bNeedsClearState )
	{
		m_pAI->ClearState();
		m_bNeedsClearState = false;
	}

	// Set UseObject state.

	m_pAI->SetState( kState_HumanUseObject );
	CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
	pStateUseObject->SetAllowDialogue( LTTRUE );

	// Play damage sound.

	if( bLoopingSound )
	{
		pStateUseObject->SetLoopingSound( eSound );
	}
	else {
		m_pAI->PlaySound( eSound, LTTRUE );
	}

	// Get the SmartObject's command string.

	if( pSmartObject )
	{
		HSTRING hstrCmd = LTNULL;
		SMART_OBJECT_CMD_MAP::iterator it = pSmartObject->mapCmds.find( kNode_DamageType );
		if(it != pSmartObject->mapCmds.end())
		{
			hstrCmd = it->second;
		}

		if(hstrCmd != LTNULL)
		{
			pStateUseObject->SetSmartObjectCommand(hstrCmd);

			// Determine if we should play animations specific to some activity (e.g. deskwork).

			HandleActivity();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::DeactivateGoal()
{
	super::DeactivateGoal();

	// Remove stimulus.

	if( m_eSpecialDamageStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eSpecialDamageStimID );
		m_eSpecialDamageStimID = kStimID_Unset;
	}

	RestoreAwareness();

	m_pAI->ResetBaseValidVolumeMask();

	// Remove anything attached (e.g. BearTrap).
	// The animation takes care of this usually, but we may 
	// have gone from BearTrapped to KnockedOut.

	char szTrigger[128];
	sprintf( szTrigger, "%s LeftFoot", KEY_DETACH );
	SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szTrigger );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleActivity
//
//	PURPOSE:	Determine if there are special animations for the current activity.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleActivity()
{
	EnumAnimProp eActivity = m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_Awareness );
	EnumAnimProp ePose = m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture );

	// No activity or special pose.

	if( ( eActivity == kAP_None ) && ( ePose == kAP_Stand ) )
	{
		return;
	}

	CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

	CAnimationProps animProps;
	animProps.Set( kAPG_Posture, ePose );
	animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_Lower );
	animProps.Set( kAPG_Awareness, eActivity );
	animProps.Set( kAPG_Action, kAP_Idle );
	animProps.Set( kAPG_Mood, m_eDamageAnim );

	// If this activity has a special damage animation, use it.

	if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
	{
		pStateUseObject->SetActivity( eActivity );
		pStateUseObject->SetPose( ePose );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::RestoreAwareness
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::RestoreAwareness()
{
	m_bIncapacitated = LTFALSE;

	// Make AI solid.		
	m_pAI->SetClientSolid( LTTRUE );

	//make sure waking AI can no longer be searched
	m_pAI->SetUnconscious( false );

	// Reset AIs default senses, from aibutes.txt.
	m_pAI->ResetBaseSenseFlags();

	// Clear AIs damage flags
	m_pAI->SetDamageFlags( 0 );

	// Ensure AI is visible to enemies.
			
	m_pAI->RegisterPersistentStimuli();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanUseObject:
			HandleStateUseObject();
			break;

		case kState_HumanIdle:
			HandleStateIdle();
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanAware:
			HandleStateAware();
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSpecialDamage::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	void CAIGoalSpecialDamage::HandleStateGoto()
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			{
				CDestructible* pDamage = m_pAI->GetDestructible();
				if( ( !pDamage->IsTakingProgressiveDamage( m_eDamageType ) ) &&
					( m_fProgressiveMinTime < g_pLTServer->GetTime() ) )
				{
					CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)(m_pAI->GetState());
					pStateGoto->SetMood( kAP_None );
					pStateGoto->SetLoopingSound( kAIS_None );
					pStateGoto->SetMovement( kAP_Run );
					RestoreAwareness();
				}
			}
			break;

		case kSStat_StateComplete:
			{
				// AI needs to clear existing knowledge of paths.

				m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();

				RestoreAwareness();

				// Search.

				if( m_pAI->CanSearch() && !m_pAI->GetSenseRecorder()->HasFullStimulation( kSense_SeeEnemy ) )
				{
					SetStateSearch();
				}
				else {
					m_pAI->SetState( kState_HumanAware );
				}
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSpecialDamage::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	void CAIGoalSpecialDamage::HandleStateAware()
//
//	PURPOSE:	Determine what to do when in state Aware.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleStateAware()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			if( (!m_bFleeing) && m_pGoalMgr->IsGoalLocked( this ) && 
				( !m_pAI->GetAnimationContext()->IsTransitioning() ) )
			{
				if( m_pAI->GetPrimaryWeapon() || 
					m_pAI->HasHolsterString() || 
					(!m_pAI->HasBackupHolsterString()) )
				{
					m_pGoalMgr->UnlockGoal(this);
				}

				// AI has been disarmed and needs to re-arm at a panic node.

				else {

					// Get rid of possible LeftHand weapon (e.g. RifleButt) 
					// since we have no RightHand weapon.

					char szDetachMsg[128];
					sprintf( szDetachMsg, "%s LEFTHAND", KEY_DETACH );
					SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetachMsg );

					m_bFleeing = LTTRUE;
					CAIHumanStateAware* pStateAware = (CAIHumanStateAware*)m_pAI->GetState();
					pStateAware->SetPlayOnce( LTTRUE );

					// Say "Who took my weapon?!"

					m_pAI->PlaySound( kAIS_Disarmed, LTFALSE );
				}
			}
			break;

		case kSStat_StateComplete:

			// Flee to panic node, if AI does not have the ability to disappear.

			if( m_pAI->HasTarget() && 
				( !m_pAI->GetBrain()->GetAIDataExist( kAIData_DisappearDistMin ) ) )
			{
				AINodePanic* pPanicNode = (AINodePanic*)g_pAINodeMgr->FindNearestNodeFromThreat( m_pAI, kNode_Panic, m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject(), 1.f );
				if( pPanicNode )
				{
					m_fRelaxTime = g_pLTServer->GetTime() + 30.f;

					m_pAI->SetState( kState_HumanPanic );
					CAIHumanStatePanic* pStatePanic = (CAIHumanStatePanic*)m_pAI->GetState();
					pStatePanic->SetPanicNode( pPanicNode );
					m_pAI->SetAwareness( kAware_Suspicious );
					m_pGoalMgr->LockGoal( this );
					return;
				}
			}

			// Bail.

			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::HandleStatePanic
//
//	PURPOSE:	Determine what to do when in state Panic.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleStatePanic()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:

			// Unlock goal if AI can re-arm and nothing has been sensed for 30 secs.

			if( m_pGoalMgr->IsGoalLocked( this ) && m_pAI->HasBackupHolsterString() )
			{
				if( m_pAI->GetSenseRecorder()->HasAnyStimulation( kSense_All ) )
				{
					m_fRelaxTime = g_pLTServer->GetTime() + 30.f;
				}
				else if( g_pLTServer->GetTime() > m_fRelaxTime )
				{
					m_pAI->SetHolsterString( m_pAI->GetBackupHolsterString() );
					m_pGoalMgr->UnlockGoal( this );
				}
			}
			break;

		case kSStat_FailedComplete:
			{
				// Try to find a new panic node.
			
				AINodePanic* pPanicNode = (AINodePanic*)g_pAINodeMgr->FindNearestNodeFromThreat( m_pAI, kNode_Panic, m_pAI->GetPosition(), m_pAI->GetTarget()->GetObject(), 1.f );
				if( pPanicNode )
				{
					m_fRelaxTime = g_pLTServer->GetTime() + 30.f;
	
					CAIHumanStatePanic* pStatePanic = (CAIHumanStatePanic*)m_pAI->GetState();
					pStatePanic->SetPanicNode( pPanicNode );
				}
				else {
					m_pGoalMgr->UnlockGoal( this );
				}
			}
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSpecialDamage::HandleStatePanic: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleStateUseObject
//
//	PURPOSE:	Determine what to do when in state UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleStateUseObject()
{
	// Handle special case updates for damage types.

	switch( m_eDamageType )
	{
		case DT_SLEEPING:
		case DT_SLIPPERY:
			
			// Kill AI that are knocked out and dropped on stairs.

			if( m_pAI->HasCurrentVolume() && 
				( !m_bInfinite ) &&
				( m_pAI->GetCurrentVolume()->GetVolumeType() == AIVolume::kVolumeType_Stairs ) )
			{
				m_pAI->GetDestructible()->HandleDestruction( m_hDamager );
			}
			break;
	}

	// Handle the StateStatus.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Paused:
			break;

		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			break;

		case kSStat_HolsterWeapon:
			break;

		case kSStat_PathComplete:

			// Knocked out AIs do not wake up if a player is standing on them.
			// Do not check for other AIs because this could result in a deadlock 
			// if 2 knocked out AIs are near each other.

			if( ( m_eDamageType == DT_SLEEPING ) || ( m_eDamageType == DT_SLIPPERY ) )
			{
				CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
				if( pStateUseObject->GetAnimTimeRemaining() < 10.f )
				{
					LTVector vPos = m_pAI->GetPosition();
					if( g_pCharacterMgr->FindCharactersWithinRadius( LTNULL, vPos, m_pAI->GetRadius(), CCharacterMgr::kList_Players ) )
					{
						pStateUseObject->AddAnimTime( 30.f );
					}
				}
			}

			// Progressive damage sets time infinite on SmartObjects, and stops
			// animating when progressive damage stops.
			// Animate for a minimal amount of time, regardless of progressive damage.

			else if( m_bProgressiveDamage )
			{
				CDestructible* pDamage = m_pAI->GetDestructible();
				if( ( !pDamage->IsTakingProgressiveDamage( m_eDamageType ) ) &&
					( m_fProgressiveMinTime < g_pLTServer->GetTime() ) )
				{
					RestoreAwareness();

					// Search.

					if( m_pAI->CanSearch() && !m_pAI->GetSenseRecorder()->HasFullStimulation( kSense_SeeEnemy ) )
					{
						SetStateSearch();
					}
					else {
						m_pAI->SetState( kState_HumanAware );
					}
				}
			}
			break;

		case kSStat_StateComplete:
			{
				RestoreAwareness();

				if( !m_pAI->GetPrimaryWeapon() )
				{
					// Get rid of possible LeftHand weapon (e.g. RifleButt) 
					// since we have no RightHand weapon.

					char szDetachMsg[128];
					sprintf( szDetachMsg, "%s LEFTHAND", KEY_DETACH );
					SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetachMsg );
				}

				// Search.

				if( m_pAI->CanSearch() && !m_pAI->GetSenseRecorder()->HasFullStimulation( kSense_SeeEnemy ) )
				{
					SetStateSearch();
				}
				else {
					m_pAI->SetState( kState_HumanAware );
				}
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT( 0, m_pAI->m_hObject, "CAIGoalSpecialDamage::HandleStateUseObject: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleDamage
//
//	PURPOSE:	Handle special damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSpecialDamage::HandleDamage(const DamageStruct& damage)
{
	// Clear old damage type if AI is not knocked out.

	if( m_bIncapacitated && ( damage.eType != m_eDamageType ) && !m_pAI->GetDestructible()->IsDead() )
	{
		if( (m_eDamageType != DT_SLEEPING) && (m_eDamageType != DT_SLIPPERY) )
		{
			switch( damage.eType )
			{
				case DT_SLIPPERY:
				case DT_SLEEPING:
				case DT_STUN:
				case DT_LAUGHING:
				case DT_BLEEDING:
				case DT_ELECTROCUTE:
				case DT_BURN:
				case DT_CHOKE:
				case DT_POISON:
				case DT_ASSS:
				case DT_BEAR_TRAP:
				case DT_GLUE:
					RestoreAwareness();
					m_pAI->GetAnimationContext()->ClearCurAnimation();
					m_bNeedsClearState = true;
					
				default :
					break;
			}
		}
	}

	// Apply a new damage type.

	if( !m_bIncapacitated )
	{
		LTBOOL bRequiresMovement = LTFALSE;
		LTBOOL bRequiresStanding = LTFALSE;

		switch( damage.eType )
		{
			case DT_SLEEPING:
			case DT_STUN:
			case DT_LAUGHING:
			case DT_BLEEDING:
			case DT_ELECTROCUTE:
			case DT_BURN:
			case DT_CHOKE:
			case DT_POISON:
			case DT_ASSS:
			case DT_GLUE:
				break;

			case DT_SLIPPERY:
				bRequiresStanding = LTTRUE;
				break;

			case DT_BEAR_TRAP:
				bRequiresMovement = LTTRUE;
				break;

			default:
				return LTFALSE;
		}


		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Hit with SpecialDamage!" ) );

		// Damage types that require movement have no effect if AI 
		// is not walking or running.

		if( bRequiresMovement && !m_pAI->IsWalkingOrRunning() )
		{
			return LTTRUE;
		}

		if( bRequiresStanding && !m_pAI->IsStanding() )
		{
			return LTTRUE;
		}

		// Some AI are killed instantly by all SpecialDamage. (e.g. rats and rabbits)

		if( m_pAI->GetBrain()->GetAIDataExist( kAIData_LethalSpecialDamage ) )
		{
			if( damage.eType != DT_SLIPPERY )
			{
				m_pAI->GetDestructible()->HandleDestruction( damage.hDamager );
				return LTTRUE;
			}

			return LTFALSE;
		}

		// Handle special damage in special volumes.

		if( m_pAI->HasCurrentVolume() && ( !m_bInfinite ) )
		{
			switch( m_pAI->GetCurrentVolume()->GetVolumeType() )
			{
				// If an AI is on a ladder, either take him off them ladder
				// or kill him, depending on how high up he is.
			
				case AIVolume::kVolumeType_Ladder:
					{
						LTFLOAT fHeight = 0.f;
						m_pAI->FindFloorHeight( m_pAI->GetPosition(), &fHeight );
						if( ( m_pAI->GetPosition().y - fHeight ) < ( 2.f * m_pAI->GetDims().y ) )
						{
							m_pAI->GetAIMovement()->UnlockMovement();
						}
						else {
							m_pAI->GetDestructible()->HandleDestruction( damage.hDamager );
						}
					}
					break;

				// Kill AI who are knocked out on ledges.
				// Kill AI who are knocked out on stairs.

				case AIVolume::kVolumeType_Ledge:
				case AIVolume::kVolumeType_Stairs:
					if( ( damage.eType == DT_SLEEPING ) || ( damage.eType == DT_SLIPPERY ) )
					{
						m_pAI->GetDestructible()->HandleDestruction( damage.hDamager );
					}
					break;
			}
		}


		m_eDamageType = damage.eType;

		// Record who caused the damage.

		m_hDamager = damage.hDamager;
		m_pAI->Target( m_hDamager );

		// Set AIs damage flags
		
		m_pAI->SetDamageFlags( DamageTypeToFlag( m_eDamageType ) );

		// Handle the damage immediately.

		m_bRequiresImmediateResponse = LTTRUE;
		SetCurToBaseImportance();

		// Reactivate if already active.

		if( m_pGoalMgr->IsCurGoal( this ) )
		{
			ActivateGoal();
		}

		return LTTRUE;
	}
	else if( m_pGoalMgr->IsCurGoal( this ) )
	{
		// We are already affected by the damage so check for one shot one kill...

		switch( m_eDamageType )
		{
			case DT_SLEEPING:
			case DT_SLIPPERY:
				break;

			default :
				return LTFALSE;
		}

		// Make sure the AI can be damaged by the damagetype...

		if( m_pAI->GetDestructible()->IsCantDamageType( damage.eType ) || !m_pAI->GetDestructible()->GetCanDamage() || damage.fDamage <= 0.0f)
		{
			return LTFALSE;
		}
	
		// Kill it...

		m_pAI->GetDestructible()->HandleDestruction( damage.hDamager );

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::GetAlternateDeathAnimation
//
//	PURPOSE:	Give goal a chance to choose an appropriate death animation.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIGoalSpecialDamage::GetAlternateDeathAnimation()
{
	// Were killed in the middle of an activity?

	if( ( m_pAI->GetState()->GetStateType() == kState_HumanUseObject ) &&
		( m_pAI->GetState()->GetStateStatus() == kSStat_PathComplete ) )
	{
		CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)m_pAI->GetState();
		EnumAnimProp eActivity = pStateUseObject->GetActivity();
		EnumAnimProp eMood = pStateUseObject->GetMood();

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, pStateUseObject->GetPose() );
		animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
		animProps.Set( kAPG_WeaponPosition, kAP_Lower );
		animProps.Set( kAPG_Awareness, eActivity );
		animProps.Set( kAPG_Mood, eMood );
		animProps.Set( kAPG_Action, kAP_Death );

		// Find a death animation for this activity.

		if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
		{
			// Use the In-transition as the death if transitioning.

			if( m_bIncapacitated && m_pAI->GetAnimationContext()->IsTransitioning() )
			{
				switch( m_eDamageType )
				{
					case DT_SLEEPING:
					case DT_SLIPPERY:
						return g_pLTServer->GetModelAnimation( m_pAI->m_hObject );
				}
			}

			// Use the actual death animation.

			return m_pAI->GetAnimationContext()->GetAni( animProps );
		}
	}

	// Were killed while running for cover?

	else if( m_pAI->GetState()->GetStateType() == kState_HumanGoto )
	{
		CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)m_pAI->GetState();
		EnumAnimProp eMood = pStateGoto->GetMood();

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, kAP_Stand );
		animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
		animProps.Set( kAPG_WeaponPosition, kAP_Lower );
		animProps.Set( kAPG_Mood, eMood );
		animProps.Set( kAPG_Action, kAP_Death );

		// Find a death animation.

		if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
		{
			return m_pAI->GetAnimationContext()->GetAni( animProps );
		}
	}


	// No alternate.

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::InterruptSpecialDamage
//
//	PURPOSE:	Interrupt special damage.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::InterruptSpecialDamage(LTBOOL bSearch)
{
	if( !m_bIncapacitated )
	{
		return;
	}

	if( !m_pAI->GetPrimaryWeapon() )
	{
		// Get rid of possible LeftHand weapon (e.g. RifleButt) 
		// since we have no RightHand weapon.

		char szDetachMsg[128];
		sprintf( szDetachMsg, "%s LEFTHAND", KEY_DETACH );
		SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetachMsg );
	}

	RestoreAwareness();

	// Go idle before searching to ensure playing the out transition.

	if( bSearch )
	{
		m_pAI->SetState( kState_HumanIdle );
	}
	else {
		m_pGoalMgr->UnlockGoal(this);
		m_pAI->SetState( kState_HumanAware );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleStateIdle
//
//	PURPOSE:	Determine what to do when in state Idle.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::HandleStateIdle()
{
	// Wait until the out transition finishes.

	if( m_pAI->GetAnimationContext()->IsTransitioning() ||
		m_pAI->GetState()->IsFirstUpdate() )
	{
		return;
	}

	// Search.

	if( m_pAI->CanSearch() && !m_pAI->GetSenseRecorder()->HasFullStimulation( kSense_SeeEnemy ) )
	{
		SetStateSearch();
	}
	else {
		m_pGoalMgr->UnlockGoal(this);
		m_pAI->SetState( kState_HumanAware );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::PauseSpecialDamage
//
//	PURPOSE:	Pause or unpause SpecialDamage. For use while being moved.
//
// ----------------------------------------------------------------------- //

void CAIGoalSpecialDamage::PauseSpecialDamage(LTBOOL bPause)
{
	if( m_pAI->GetState()->GetStateType() != kState_HumanUseObject )
	{
		return;
	}

	CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
	pStateUseObject->Pause( bPause );

	//clear old stimulus

	if( m_eSpecialDamageStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eSpecialDamageStimID );
		m_eSpecialDamageStimID = kStimID_Unset;
	}

	if( bPause )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Pausing SpecialDamage" ) );
		return;
	}


	AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Unpausing SpecialDamage" ) );

	// AI needs to clear existing knowledge of paths.

	m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();

	// Reset last volume info.

	m_pAI->RecalcLastVolumePos();

	// If we are unpausing register a new stimulus.
	// Use the DeathVisible stimulus for knocked out AIs, and SpecialDamageVisible for the rest.

	switch( m_eDamageType )
	{
		case DT_SLEEPING:
		case DT_SLIPPERY:
			m_eSpecialDamageStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDeathVisible, m_pAI->m_hObject, m_pAI->GetPosition(), 1.f );
			break;

		default:
			m_eSpecialDamageStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllySpecialDamageVisible, m_pAI->m_hObject, m_pAI->GetPosition(), 1.f );
			break;
	}

	// Clear any activity that was set (e.g. working at a desk).

	m_pAI->GetAnimationContext()->ClearCurAnimation();
	pStateUseObject->SetPose( kAP_Stand );
	pStateUseObject->SetActivity( kAP_None );

	// Special case handling for being moved while knocked out from a banana.
	// Treat like the AI was tranquilized to keep him from re-slipping when dropped.

	if( m_eDamageType == DT_SLIPPERY )
	{
		m_eDamageType = DT_SLEEPING;
		m_pAI->SetDamageFlags( DamageTypeToFlag( m_eDamageType ) );

		if( m_pAI->GetState()->GetStateType() == kState_HumanUseObject )
		{
			AIGBM_SmartObjectTemplate* pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate( SMART_OBJECT_SLEEPING );
			if( pSmartObject )
			{
				HSTRING hstrCmd = LTNULL;
				SMART_OBJECT_CMD_MAP::iterator it = pSmartObject->mapCmds.find( kNode_DamageType );
				if(it != pSmartObject->mapCmds.end())
				{
					hstrCmd = it->second;
				}

				if(hstrCmd != LTNULL)
				{
					CAIHumanStateUseObject* pStateUseObject = (CAIHumanStateUseObject*)(m_pAI->GetState());
					pStateUseObject->SetSmartObjectCommand(hstrCmd);
					pStateUseObject->StartAnimation();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSpecialDamage::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "PAUSE") )
	{
		PauseSpecialDamage( IsTrueChar( szValue[0] ) );
		return LTTRUE;
	}

	if ( !_stricmp(szName, "SLEEPFOREVER") )
	{
		if( IsTrueChar( szValue[0] ) )
		{
			m_bInfinite = LTTRUE;
			m_eDamageType = DT_SLEEPING;
		
			m_pAI->SetDamageFlags( DamageTypeToFlag( DT_SLEEPING ) );

			// Handle the damage immediately.

			m_bRequiresImmediateResponse = LTTRUE;
			SetCurToBaseImportance();
		}

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSpecialDamage::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( m_pGoalMgr->IsCurGoal( this ) && ( !m_bIncapacitated ) && 
		super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Break out of search animations.

		if( ( !m_pAI->GetAnimationContext()->IsTransitioning() ) &&
			( m_pAI->GetAnimationContext()->IsLocked() ) &&
			( m_pAI->GetState()->GetStateType() == kState_HumanSearch ) )
		{
			m_pAI->GetAnimationContext()->ClearLock();
		}

		if( ( !m_pAI->GetPrimaryWeapon() ) && ( !m_pAI->HasHolsterString() ) )
		{
			// Get rid of possible LeftHand weapon (e.g. RifleButt) 
			// since we have no RightHand weapon.

			char szDetachMsg[128];
			sprintf( szDetachMsg, "%s LEFTHAND", KEY_DETACH );
			SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetachMsg );
		}

		// Only bail if we are armed, otherwise stay in goal to panic.

		else {
			switch( pSenseRecord->eSenseType )
			{
				// Do not bail if only sensing agitated allies.

				case kSense_SeeAllyDisturbance:
				case kSense_HearAllyDisturbance:
					break;

				default:
					AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalSpecialDamage: Bailing due to sensing %s", CAISenseRecorderAbstract::SenseToString( pSenseRecord->eSenseType ) ) );
					m_fCurImportance = 0.f;
					break;
			}
		}
	}

	return LTFALSE;
}

