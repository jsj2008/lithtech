// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanState.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIVolumeMgr.h"
#include "PlayerObj.h"
#include "AIGoalMgr.h"
#include "AINodeMgr.h"
#include "AIVolumeMgr.h"
#include "CharacterMgr.h"
#include "AIPath.h"
#include "WeaponItems.h"
#include "CharacterHitBox.h"
#include "AIRegionMgr.h"
#include "AITypes.h"
#include "AISenseRecorderAbstract.h"
#include "Body.h"
#include "AIStimulusMgr.h"
#include "AIHumanStrategyToggleLights.h"
#include "AINodeGuard.h"
#include "Weapon.h"
#include "Attachments.h"
#include "ParsedMsg.h"
#include "TrackedNodeContext.h"
#include "AICentralKnowledgeMgr.h"
#include "AIUtils.h"
#include "AIMovement.h"

extern CAIStimulusMgr* g_pAIStimulusMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAnimate, kState_HumanAnimate);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAware, kState_HumanAware);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLookAt, kState_HumanLookAt);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAssassinate, kState_HumanAssassinate);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDraw, kState_HumanDraw);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateHolster, kState_HumanHolster);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttack, kState_HumanAttack);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromCover, kState_HumanAttackFromCover);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromVantage, kState_HumanAttackFromVantage);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromView, kState_HumanAttackFromView);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackProp, kState_HumanAttackProp);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCharge, kState_HumanCharge);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateChase, kState_HumanChase);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCheckBody, kState_HumanCheckBody);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateInvestigate, kState_HumanInvestigate);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCover, kState_HumanCover);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDistress, kState_HumanDistress);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFollow, kState_HumanFollow);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateIdle, kState_HumanIdle);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFollowFootprint, kState_HumanFollowFootprint);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateGetBackup, kState_HumanGetBackup);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateGoto, kState_HumanGoto);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFlee, kState_HumanFlee);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStatePanic, kState_HumanPanic);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStatePatrol, kState_HumanPatrol);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSearch, kState_HumanSearch);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateTail, kState_HumanTail);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateTalk, kState_HumanTalk);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateUseObject, kState_HumanUseObject);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLongJump, kState_HumanLongJump);


// ----------------------------------------------------------------------- //

CAIHumanState::CAIHumanState()
{
	m_pAIHuman = LTNULL;

	m_pStrategyFollowPath	= LTNULL;
	m_pStrategyDodge		= LTNULL;
	m_pStrategyCover		= LTNULL;
	m_pStrategyShoot		= LTNULL;
	m_pStrategyGrenade		= LTNULL;
	m_pStrategyOneShotAni	= LTNULL;
	m_pStrategyFlashlight	= LTNULL;
	m_pStrategyTaunt		= LTNULL;
	m_pStrategyToggleLights	= LTNULL;

	m_bInterrupt = LTTRUE;

	m_ePose = kAP_Any;
}

CAIHumanState::~CAIHumanState()
{
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanState::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pAIHuman = pAIHuman;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

CAnimationContext* CAIHumanState::GetAnimationContext()
{
	return GetAI()->GetAnimationContext();
}

// ----------------------------------------------------------------------- //

void CAIHumanState::PreUpdate()
{
	super::PreUpdate();

	// Note: this also happens in UpdateAnimation since there is a chance
	// it can occur before we get here

	if ( kAP_Any == m_ePose )
	{
		if ( GetAI()->GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Sit) )
		{
			m_ePose = kAP_Sit;
		}
		else
		{
			m_ePose = kAP_Stand;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::PostUpdate()
{
	super::PostUpdate();

	if ( m_pStrategyShoot )
	{
		m_pStrategyShoot->ClearFired();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::UpdateAnimation()
{
	super::UpdateAnimation();

	// This is a hack... do it here too since this can happen before our preupdate

	if ( kAP_Any == m_ePose )
	{
		if ( GetAI()->GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Sit) )
		{
			m_ePose = kAP_Sit;
		}
		else
		{
			m_ePose = kAP_Stand;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( m_pStrategyShoot )
	{
		m_pStrategyShoot->HandleModelString(pArgList);
	}

	if ( m_pStrategyGrenade )
	{
		m_pStrategyGrenade->HandleModelString(pArgList);
	}

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleModelString(pArgList);
	}

	if ( m_pStrategyFlashlight )
	{
		m_pStrategyFlashlight->HandleModelString(pArgList);
	}

	if ( m_pStrategyToggleLights )
	{
		m_pStrategyToggleLights->HandleModelString(pArgList);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanState::DelayChangeState()
{
	// FollowPath strategy can delay state changes

	return (m_pStrategyFollowPath ? m_pStrategyFollowPath->DelayChangeState() : LTFALSE) || super::DelayChangeState();
}

// ----------------------------------------------------------------------- //

void CAIHumanState::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_ePose);
	SAVE_BOOL(m_bInterrupt);
}

// ----------------------------------------------------------------------- //

void CAIHumanState::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_ePose, EnumAnimProp);
	LOAD_BOOL(m_bInterrupt);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateIdle::CAIHumanStateIdle()
{
	m_bNoCinematics = LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateIdle::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// DO NOT SET AWARENESS HERE.

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIdle::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAware::CAIHumanStateAware()
{
	m_bNoCinematics = LTFALSE;
	m_bPlayOnce		= LTFALSE;
	m_eAISound		= kAIS_None;

	m_bPlayFirstSound = LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAware::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Suspicious );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bPlayOnce);
	LOAD_DWORD_CAST( m_eAISound, EnumAISoundType );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bPlayOnce);
	SAVE_DWORD( m_eAISound );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Update()
{
	super::Update();

	if ( m_bPlayFirstSound && ( m_eAISound != kAIS_None ) && !GetAI()->IsControlledByDialogue() )
	{
		GetAI()->PlaySound( m_eAISound, LTFALSE );
	}

	if( m_bPlayOnce &&
		( !m_bFirstUpdate ) &&
		( !GetAnimationContext()->IsLocked() ) &&
		( !GetAnimationContext()->IsTransitioning() ) )
	{
		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
	GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);

	if( m_bPlayOnce && (m_eStateStatus != kSStat_StateComplete) )
	{
		GetAnimationContext()->Lock();
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateLookAt::CAIHumanStateLookAt()
{
	m_aniLook.Set(kAPG_Action, kAP_LookLeft);

	m_bPause = LTFALSE;

	m_eAISound = kAIS_None;
	m_bPlayFirstSound = LTTRUE;

	// TODO: should this state interrupt cinematics?
	m_bNoCinematics = LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateLookAt::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Suspicious );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_aniLook.Load(pMsg);
	LOAD_DWORD_CAST( m_eAISound, EnumAISoundType );
	LOAD_BOOL( m_bPause );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_aniLook.Save(pMsg);
	SAVE_DWORD( m_eAISound );
	SAVE_BOOL( m_bPause );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::SetPos(const LTVector& vPos)
{
	LTVector vDir = vPos - GetAI()->GetPosition();

	if ( vDir.Dot(GetAI()->GetRightVector()) > 0.0f )
	{
		m_aniLook.Set(kAPG_Action, kAP_LookRight);
	}
	else
	{
		m_aniLook.Set(kAPG_Action, kAP_LookLeft);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Update()
{
	super::Update();

	if ( m_bPlayFirstSound && ( m_eAISound != kAIS_None ) && !GetAI()->IsControlledByDialogue() )
	{
		GetAI()->PlaySearchSound( m_eAISound );
	}

	if( m_bPause )
	{
		if( !m_bFirstUpdate && !GetAnimationContext()->IsLocked() )
		{
			m_bPause = LTFALSE;
		}
		else return;
	}
	else if( !m_bFirstUpdate && !GetAnimationContext()->IsLocked() )
	{
		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);

	if( m_bPause )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);
		GetAnimationContext()->Lock();
		return;
	}

	GetAnimationContext()->SetProp(m_aniLook);

	if(m_eStateStatus != kSStat_StateComplete)
	{
		GetAnimationContext()->Lock();
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStatePatrol::CAIHumanStatePatrol()
{
	m_bNoCinematics = LTFALSE;

	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyToggleLights = AI_FACTORY_NEW(CAIHumanStrategyToggleLights);

	m_fWaitTimer = 3.0f + GetRandom(0.0f, 3.0f);

	m_bForward = LTTRUE;

	m_eAwareness = kAP_Patrol;

	m_pNode = LTNULL;
	m_pLastNode = LTNULL;
}

CAIHumanStatePatrol::~CAIHumanStatePatrol()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyToggleLights);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePatrol::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyToggleLights->Init( pAIHuman, m_pStrategyFollowPath ) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovementModifier( kAPG_Awareness, m_eAwareness );

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Relaxed );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::SetAwareness(EnumAnimProp eAwareness)
{
	m_eAwareness = eAwareness;
	if( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->SetMovementModifier( kAPG_Awareness, m_eAwareness );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::SetNode(AINodePatrol* pNode)
{
	ASSERT(pNode);
	m_pNode = pNode;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		LTFLOAT fPatrolSoundTime = GetAI()->GetBrain()->GetPatrolSoundTime();
		LTFLOAT fPatrolSoundTimeRandomMin = GetAI()->GetBrain()->GetPatrolSoundTimeRandomMin();
		LTFLOAT fPatrolSoundTimeRandomMax = GetAI()->GetBrain()->GetPatrolSoundTimeRandomMax();

		m_fTalkTimer = fPatrolSoundTime + GetRandom(fPatrolSoundTimeRandomMin, fPatrolSoundTimeRandomMax);
	}

	// Give a little breathing room after a goalset change before turning or
	// moving the AI.  This gives AI a chance to react to stimulus that may be
	// in the other direction from the patrol node.  (e.g. if an AIs dialogue
	// was interrupted when another AI was killed, AI may need to react to the body).

	if( GetAI()->GetGoalMgr()->GetGoalSetTime() + 2.f > g_pLTServer->GetTime() )
	{
		return;
	}

	if ( !m_pNode )
	{
        g_pLTServer->CPrint("AI had no patrol nodes.");

		// WE DIDN'T HAVE A VALID PATROL NODE
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Turn off lights in the source volume.
	// Turn on lights in the dest volume.

	if( m_pStrategyToggleLights->IsUnset() )
	{
		m_pStrategyToggleLights->Set( LTTRUE, LTTRUE, m_pNode->GetPos() );
	}

	if( m_pStrategyToggleLights->IsSet() )
	{
		m_pStrategyToggleLights->Update();
		return;
	}

	if ( m_fTalkTimer < 0.0f )
	{
		LTFLOAT fRandom = GetRandom(0.0f, 1.0f);
		if ( fRandom < GetAI()->GetBrain()->GetPatrolSoundChance() )
		{
			GetAI()->PlaySound( kAIS_Idle, LTFALSE );
		}

		LTFLOAT fPatrolSoundTime = GetAI()->GetBrain()->GetPatrolSoundTime();
		LTFLOAT fPatrolSoundTimeRandomMin = GetAI()->GetBrain()->GetPatrolSoundTimeRandomMin();
		LTFLOAT fPatrolSoundTimeRandomMax = GetAI()->GetBrain()->GetPatrolSoundTimeRandomMax();

		m_fTalkTimer = fPatrolSoundTime + GetRandom(fPatrolSoundTimeRandomMin, fPatrolSoundTimeRandomMax);
	}
	else
	{
        m_fTalkTimer -= g_pLTServer->GetFrameTime();
	}

	LTBOOL bGotoNextNode = LTFALSE;

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_pNode->ShouldFaceNodeForward() )
		{
			GetAI()->FaceDir(m_pNode->GetForward());
		}

		switch ( m_pNode->GetAction() )
		{
			case kAP_None:
				bGotoNextNode = LTTRUE;
				break;

			case kAP_Wait:
				bGotoNextNode = UpdateTaskWait();
				break;

			default:
				bGotoNextNode = UpdateTaskAnimate();
				break;
		}
	}
	else if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( !m_pStrategyFollowPath->Set(m_pNode, LTFALSE) )
		{
			// WE COULDN'T SET A PATH
			m_eStateStatus = kSStat_FailedSetPath;
			return;
		}
	}

	if ( bGotoNextNode )
	{
		m_pStrategyToggleLights->Reset();

		// Reset the wait timer

		m_fWaitTimer = 3.0f + GetRandom(0.0f, 3.0f);

		// Keep track of last node visited.

		m_pLastNode = m_pNode;

		// Run command at last node, if there is one.

		if( m_pLastNode->HasCmd() )
		{
			SendTriggerMsgToObject( m_pLastNode, GetAI()->m_hObject, m_pLastNode->GetCmd() );
		}

		// Get the next node

		if ( m_bForward )
		{
			if ( m_pNode->GetNext() )
			{
				m_pNode = m_pNode->GetNext();
			}
			else
			{
				m_pNode = m_pNode->GetPrev();
				m_bForward = LTFALSE;
			}
		}

		else
		{
			if ( m_pNode->GetPrev() )
			{
				m_pNode = m_pNode->GetPrev();
			}
			else
			{
				m_pNode = m_pNode->GetNext();
				m_bForward = LTTRUE;
			}
		}

		// Failed to find next or prev node.

		if( !m_pNode )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		// Set our path

		if ( !m_pStrategyFollowPath->Set(m_pNode, LTFALSE) )
		{
			AIError("AI \"%s\" - Cannot path from node \"%s\" (%s) to node \"%s\" (%s)",
				GetAI()->GetName(),
				::ToString((m_pLastNode)?m_pLastNode->GetName():LTNULL),
				(m_pLastNode && m_pLastNode->GetNodeContainingVolume())?m_pLastNode->GetNodeContainingVolume()->GetName():"",
				::ToString((m_pNode)?m_pNode->GetName():LTNULL),
				(m_pNode && m_pNode->GetNodeContainingVolume())?m_pNode->GetNodeContainingVolume()->GetName():"");

			// WE COULDN'T SET A PATH
			m_eStateStatus = kSStat_FailedSetPath;
			return;
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);

	if( m_pStrategyToggleLights->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		m_pStrategyToggleLights->UpdateAnimation();
		return;
	}

	if ( m_pNode && m_pStrategyFollowPath->IsDone() )
	{
		switch ( m_pNode->GetAction() )
		{
			case kAP_None:
			case kAP_Wait:
				return;

			default:
				GetAnimationContext()->SetProp(kAPG_Action, m_pNode->GetAction() );
				break;
		}

		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
		GetAnimationContext()->Lock();
	}
	else if( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePatrol::UpdateTaskWait()
{
	if ( m_fWaitTimer > 0.0f )
	{
		// We're waiting at our patrol point

        m_fWaitTimer -= g_pLTServer->GetFrameTime();
	}
	else
	{
		// We're done waiting, reset our wait timer and move to the next node

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePatrol::UpdateTaskAnimate()
{
	if ( !GetAnimationContext()->IsLocked() )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	if( m_pStrategyToggleLights->IsDone() )
	{
		m_pStrategyToggleLights->Reset();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyToggleLights->Load(pMsg);

	LOAD_FLOAT(m_fWaitTimer);
	LOAD_FLOAT(m_fTalkTimer);

	LOAD_BOOL(m_bForward);
	LOAD_DWORD_CAST(m_eAwareness, EnumAnimProp);
	LOAD_COBJECT(m_pNode, AINodePatrol);
	LOAD_COBJECT(m_pLastNode, AINodePatrol);
}


// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyToggleLights->Save(pMsg);

	SAVE_FLOAT(m_fWaitTimer);
	SAVE_FLOAT(m_fTalkTimer);

	SAVE_BOOL(m_bForward);
	SAVE_DWORD(m_eAwareness);
	SAVE_COBJECT(m_pNode);
	SAVE_COBJECT(m_pLastNode);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateGoto::CAIHumanStateGoto()
{
	m_bNoCinematics = LTFALSE;

	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyToggleLights = AI_FACTORY_NEW(CAIHumanStrategyToggleLights);

	m_vDest = LTVector(0,0,0);
	m_hDestNode = LTNULL;
	m_cNodes = 0;
	m_iNextNode = 0;
	m_bLoop = LTFALSE;

	m_eLoopingAISound = kAIS_None;

	m_eAwareness = kAP_None;
	m_eMood = kAP_None;
	m_eWeaponPosition = kAP_Down;
	m_eMovement = kAP_Walk;
	m_fCloseEnoughDistSqr = 0.f;
	m_bTurnOffLights = LTFALSE;
}

CAIHumanStateGoto::~CAIHumanStateGoto()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyToggleLights);

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
	if( pNode )
	{
		pNode->Unlock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateGoto::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyToggleLights->Init( pAIHuman, m_pStrategyFollowPath ) )
	{
		return LTFALSE;
	}

	// Set up our default movement speed

	if ( GetAI()->IsSwimming() )
	{
		m_pStrategyFollowPath->SetMovement(kAP_Swim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(m_eMovement);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::SetDestNode(HOBJECT hDestNode)
{
	m_hDestNode = hDestNode;
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
	if( pNode )
	{
		m_vDest = pNode->GetPos();
		m_eStateStatus = kSStat_Initialized;
		m_pStrategyFollowPath->Reset();
		pNode->Lock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

AINode* CAIHumanStateGoto::SafeGetGotoNode(int32 iGotoNode)
{
	if ( iGotoNode >= 0 && iGotoNode < m_cNodes )
	{
		return m_apNodes[iGotoNode];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Update()
{
	super::Update();

	// Turn off lights in the source volume.

	if( m_bTurnOffLights )
	{
		if( m_pStrategyToggleLights->IsUnset() )
		{
			m_pStrategyToggleLights->Set( LTTRUE, LTFALSE, m_vDest );
			if( m_pStrategyToggleLights->IsSet() )
			{
				AITRACE( AIShowStates, ( GetAI()->m_hObject, "HumanStateGoto: Set StrategyToggleLights." ) );
			}
		}

		if( m_pStrategyToggleLights->IsSet() )
		{
			m_pStrategyToggleLights->Update();
			return;
		}
	}

	// Play looping sound.

	if( ( m_eLoopingAISound != kAIS_None ) &&
		( !GetAI()->IsPlayingDialogSound() ) )
	{
		GetAI()->PlaySound( m_eLoopingAISound, LTFALSE );
	}

	// Check if we are within some radius of the dest.

	LTBOOL bCloseEnough = LTFALSE;
	if ( ( m_cNodes == 0 ) &&
		 ( m_fCloseEnoughDistSqr > 0.f ) &&
		   m_pStrategyFollowPath->IsSet() )
	{
		if( m_vDest.DistSqr( GetAI()->GetPosition() ) < m_fCloseEnoughDistSqr )
		{
			bCloseEnough = LTTRUE;
		}
	}

	// Check if we have reached our dest, or have not begun.

	if ( bCloseEnough || !m_pStrategyFollowPath->IsSet() )
	{
		if ( m_cNodes == 0 )
		{
			// If we're just going to a point...

			if ( bCloseEnough || m_pStrategyFollowPath->IsDone() )
			{
				// Set the status before running any node commands.
				// This allows nodes to set new goto nodes.

				m_eStateStatus = kSStat_StateComplete;

				// We successfully got there.

				if( m_hDestNode )
				{
					AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
					if( pNode && pNode->ShouldFaceNodeForward() )
					{
						GetAI()->FaceDir( pNode->GetForward() );
					}

					// If the dest node is a Goto node, run its command.

					if( pNode && ( pNode->GetType() == kNode_Goto ) && pNode->HasCmd() )
					{
						SendTriggerMsgToObject( pNode, GetAI()->m_hObject, pNode->GetCmd() );
					}
				}

				return;
			}
			else // if ( m_pStrategyFollowPath->IsUnset() )
			{
				if ( !m_pStrategyFollowPath->Set(m_vDest, LTFALSE) )
				{
					// WE COULDN'T SET A PATH

					if( m_hDestNode )
					{
						AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hDestNode );
						AITRACE( AIShowStates, ( GetAI()->m_hObject, "CAIHumanStateGoto::Update: Could not find path to Goto dest node: %s", ::ToString( pNode->GetName() ) ) );
					}
					else {
						LTFLOAT fSearchY = 64.f;
						AIVolume* pContainingVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, m_vDest, eAxisAll, fSearchY, LTNULL );
						AITRACE( AIShowStates, ( GetAI()->m_hObject, "CAIHumanStateGoto::Update: Could not find path to dest (%s).",
							pContainingVolume ? pContainingVolume->GetName() : "No Volume" ) );
					}

					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}
		}
		else
		{
			// See if we're done Goto-ing.

			int iNode = m_iNextNode;
			if ( iNode == m_cNodes )
			{
				if ( m_bLoop )
				{
					iNode = m_iNextNode = 0;
				}
				else
				{
					// We successfully got there

					AINode* pNode = SafeGetGotoNode(m_cNodes-1);
					if ( pNode && pNode->ShouldFaceNodeForward() )
					{
						GetAI()->FaceDir(pNode->GetForward());
					}

					// If the dest node is a Goto node, run its command.

					if( pNode && ( pNode->GetType() == kNode_Goto ) && pNode->HasCmd() )
					{
						SendTriggerMsgToObject( pNode, GetAI()->m_hObject, pNode->GetCmd() );
					}

					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}

			// Advance the next node

			m_iNextNode++;

			// Get the node

			AINode *pNode = SafeGetGotoNode(iNode);

			if ( !pNode || !m_pStrategyFollowPath->Set(pNode, LTFALSE) )
			{
				// WE COULDN'T SET A PATH OR WE DIDN'T HAVE ANOTHER NODE TO GO TO
				m_eStateStatus = kSStat_StateComplete;
				return;
			}
		}
	}

	if( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, m_eWeaponPosition);
	GetAnimationContext()->SetProp(kAPG_Awareness, m_eAwareness);
	GetAnimationContext()->SetProp(kAPG_Mood, m_eMood);

	if( m_pStrategyToggleLights->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		m_pStrategyToggleLights->UpdateAnimation();
		return;
	}

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	if( m_pStrategyToggleLights->IsDone() )
	{
		m_pStrategyToggleLights->Reset();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyToggleLights->Load(pMsg);

	LOAD_DWORD_CAST(m_eAwareness, EnumAnimProp);
	LOAD_DWORD_CAST(m_eMood, EnumAnimProp);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_DWORD_CAST(m_eWeaponPosition, EnumAnimProp);
	LOAD_DWORD_CAST(m_eLoopingAISound, EnumAISoundType);
	LOAD_VECTOR(m_vDest);
	LOAD_HOBJECT(m_hDestNode);
	LOAD_DWORD(m_cNodes);
	LOAD_DWORD(m_iNextNode);
	LOAD_BOOL(m_bLoop);

    int iNode;
    for ( iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		LOAD_COBJECT(m_apNodes[iNode], AINode);
	}

	for ( iNode = m_cNodes ; iNode < kMaxGotoNodes ; iNode++ )
	{
		m_apNodes[iNode] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyToggleLights->Save(pMsg);

	SAVE_DWORD(m_eAwareness);
	SAVE_DWORD(m_eMood);
	SAVE_DWORD(m_eMovement);
	SAVE_DWORD(m_eWeaponPosition);
	SAVE_DWORD(m_eLoopingAISound);
	SAVE_VECTOR(m_vDest);
	SAVE_HOBJECT(m_hDestNode);
	SAVE_DWORD(m_cNodes);
	SAVE_DWORD(m_iNextNode);
	SAVE_BOOL(m_bLoop);

	for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		SAVE_COBJECT(m_apNodes[iNode]);
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateFlee::CAIHumanStateFlee()
{
	m_hDanger = NULL;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateFlee::Init(CAIHuman* pAIHuman)
{
	if( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		// $SOUND GetAI()->PlaySound(aisPanic);
	}

	if ( !m_hDanger )
	{
		if ( GetAI()->HasTarget() )
		{
			GetAI()->FaceTarget();
		}

		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::UpdateAnimation()
{
	super::UpdateAnimation();

	// If follow path didn't set an action, raise weapon while moving.
	if(	GetAnimationContext()->GetProp(kAPG_Action) == kAP_None )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::SetDanger(HOBJECT hDanger)
{
	m_hDanger = hDanger;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hDanger);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hDanger);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateSearch::CAIHumanStateSearch()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyFlashlight = AI_FACTORY_NEW(CAIHumanStrategyFlashlight);
	m_pStrategyToggleLights = AI_FACTORY_NEW(CAIHumanStrategyToggleLights);

	m_bSearching = LTFALSE;
	m_pSearchNode = LTNULL;
	m_bFace = LTTRUE;
	m_bEngage = LTFALSE;
	m_bDone = LTFALSE;
	m_bAdded = LTFALSE;
	m_pSearchRegion = LTNULL;
	m_hDestRegion = LTNULL;
	m_bPause = LTFALSE;
	m_bIgnoreJunctions = LTTRUE;
	m_bLimitSearchCount = LTTRUE;
	m_cSearchedNodes = 0;

	m_bCheckedLantern = LTFALSE;

	m_fNextPause = 0.f;

	m_pLastVolume		= LTNULL;
	m_pJunctionVolume	= LTNULL;
}

// ----------------------------------------------------------------------- //

CAIHumanStateSearch::~CAIHumanStateSearch()
{
	if ( m_pSearchNode )
	{
		m_pSearchNode->Unlock( GetAI()->m_hObject );
	}

	if ( m_bAdded )
	{
		if ( m_pSearchRegion )
		{
			m_pSearchRegion->RemoveSearcher(GetAI());
		}
	}

	AI_FACTORY_DELETE(m_pStrategyFlashlight);
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyToggleLights);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSearch::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFlashlight->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyToggleLights->Init( pAIHuman, m_pStrategyFollowPath ) )
	{
		return LTFALSE;
	}

	// Run when AI has been alarmed enough.

	if( pAIHuman->IsImmediatelyAlarmed() )
	{
		m_pStrategyFollowPath->SetMovement(kAP_Run);
	}
	else {
		m_pStrategyFollowPath->SetMovement(kAP_Walk);
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::GetRandomSearch(AINodeSearch* pNode, CAnimationProps* paniSearch) const
{
	int nAvailableSearchs = 0;
	uint32 adwSearchs[AINodeSearch::kNumFlags];
	uint32 dwSearchFlags = pNode->GetFlags();

	paniSearch->Clear();

	for ( uint32 iSearch = 0 ; iSearch < AINodeSearch::kNumFlags ; iSearch++ )
	{
		if ( dwSearchFlags & (1 << iSearch) )
		{
			adwSearchs[nAvailableSearchs++] = (1 << iSearch);
		}
	}

	if ( 0 == nAvailableSearchs )
	{
        g_pLTServer->CPrint("GetRandomSearch - couldn't get random search action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
		paniSearch->Set(kAPG_WeaponPosition, kAP_Down);
		paniSearch->Set(kAPG_Action, kAP_Alert);
		return;
	}

	uint32 dwSearch = adwSearchs[GetRandom(0, nAvailableSearchs-1)];

	switch( dwSearch )
	{
		case AINodeSearch::kFlagShineFlashlight:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_Flashlight);
			return;

		case AINodeSearch::kFlagLookUnder:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_LookUnder);
			return;

		case AINodeSearch::kFlagLookOver:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_LookOver);
			return;

		case AINodeSearch::kFlagLookUp:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_LookUp);
			return;

		case AINodeSearch::kFlagLookLeft:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_LookLeft);
			return;

		case AINodeSearch::kFlagLookRight:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_LookRight);
			return;

		case AINodeSearch::kFlagKnockOnDoor:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Up);
			paniSearch->Set(kAPG_Action, kAP_KnockOnDoor);
			return;

		case AINodeSearch::kFlagAlert:
			paniSearch->Set(kAPG_WeaponPosition, kAP_Down);
			paniSearch->Set(kAPG_Action, kAP_Alert);
			return;
	}

    g_pLTServer->CPrint("GetRandomSearch - couldn't get search action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
	paniSearch->Set(kAPG_WeaponPosition, kAP_Down);
	paniSearch->Set(kAPG_Action, kAP_Alert);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Update()
{
	super::Update();

	m_pStrategyFlashlight->Update();

	if ( m_bDone )
	{
		GetAI()->PlaySearchSound( kAIS_SearchFail );

		// WE'RE DONE
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	if( m_bFirstUpdate )
	{
		GetAI()->PlaySearchSound(kAIS_Search);
	}

	// HACK: for TO2 ninjas with lanterns. transition animation method is not reliable.

	else if( !m_bCheckedLantern )
	{
		m_bCheckedLantern = LTTRUE;
		if( ( !GetAI()->GetAnimationContext()->IsTransitioning() ) &&
		    ( GetAI()->GetCurrentVolume() && !GetAI()->GetCurrentVolume()->IsLit() ) &&
			( GetAI()->GetBrain()->GetAIDataExist( kAIData_DisposeLantern ) ) )
		{
			char szAttachment[128];
			sprintf(szAttachment, "%s LIGHT Lantern", KEY_ATTACH);
			SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);
		}
	}

	// END HACK.

	// Handle our pause if we had to do one

	if ( (m_bFirstUpdate && m_bPause) || ( (m_fNextPause > 0.f) && (g_pLTServer->GetTime() > m_fNextPause) ) )
	{
		m_bPause = LTTRUE;
		m_fNextPause = 0.f;
		return;
	}
	else if ( m_bPause  )
	{
		return;
	}

	// Handle turning off/on lights.

	if( m_pStrategyToggleLights->IsSet() )
	{
		m_pStrategyToggleLights->Update();
		return;
	}

	if( m_pStrategyToggleLights->IsDone() && m_pSearchNode )
	{
		m_pStrategyFollowPath->Set( m_pSearchNode, LTFALSE );
		m_pStrategyToggleLights->Reset();
	}


	// Update our searching

	// Insure that we have a target before attempting to use it
	if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( m_bEngage )
		{
			// We CANNOT set the path if we don't have a target!
			// Both Node and Engage based checks require the use of the target.
			if ( !GetAI()->HasTarget() )
			{
				m_eStateStatus = kSStat_FailedEngage;
				return;
			}

			// If we're engaging, we have to run to the threat's region before we pick a search node

			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());

			// Check the target's last volume info

			LTVector vTargetPosition;
			if ( pCharacter->HasLastVolume() )
			{
				if ( pCharacter->GetLastVolume()->HasRegion() )
				{
					AIRegion* pRegion = pCharacter->GetLastVolume()->GetRegion();
					if ( pRegion->IsSearchable() )
					{
						vTargetPosition = pCharacter->GetLastVolumePos();
						if ( m_pStrategyFollowPath->Set(vTargetPosition, LTFALSE) )
						{
							m_pSearchRegion = pRegion;
							m_pStrategyFollowPath->SetMovement(kAP_Run);
							return;
						}
					}
				}
			}

			// If we get here we couldn't engage

			// COULD NOT ENGAGE OUR TARGET
			m_eStateStatus = kSStat_FailedEngage;
			return;
		}
		else if ( !FindNode() )
		{
			GetAI()->PlaySearchSound( kAIS_SearchFail );

			// COULD NOT FIND A VALID SEARCH NODE
			m_eStateStatus = kSStat_StateComplete;
			return;
		}
	}

	else if ( !m_pStrategyFollowPath->IsDone() )
	{
		m_pStrategyFollowPath->Update();

		// If we're engaging, see if we've reached the engage region

		if ( m_bEngage )
		{
			if ( GetAI()->HasLastVolume() )
			{
				if ( GetAI()->GetLastVolume()->HasRegion() )
				{
					if ( GetAI()->GetLastVolume()->GetRegion() == m_pSearchRegion )
					{
						if ( !FindNode() )
						{
							GetAI()->PlaySearchSound( kAIS_SearchFail );

							// COULD NOT FIND A VALID SEARCH NODE
							m_eStateStatus = kSStat_StateComplete;
						}
						else
						{
							m_bEngage = LTFALSE;
							m_pStrategyFollowPath->SetMovement(kAP_Walk);
						}

						return;
					}
				}
			}
		}
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// If we finished our path and were engaging, that's kind of weird, but just keep going

		if ( m_bEngage )
		{
			m_bEngage = LTFALSE;
			m_pStrategyFollowPath->SetMovement(kAP_Walk);
			FindNode();
			return;
		}

		// Update our searching animation

		if ( !m_pSearchNode )
		{
			GetAI()->PlaySearchSound( kAIS_SearchFail );

			// COULD NOT FIND A VALID SEARCH NODE
			m_eStateStatus = kSStat_StateComplete;
			return;
		}
		else if ( m_bSearching && !GetAnimationContext()->IsLocked() )
		{
			m_bSearching = LTFALSE;

			m_pSearchNode->Search();

			if( m_bLimitSearchCount && ( m_cSearchedNodes >= 2 ) )
			{
				m_bDone = LTTRUE;
				return;
			}

			if ( !FindNode() )
			{
				if ( m_pSearchRegion )
				{
					m_bDone = LTTRUE;

					HSTRING hstrPostSearchMsg = m_pSearchRegion->GetPostSearchMsg();
					if ( hstrPostSearchMsg )
					{
						// POST SEARCH MESSAGE
						ASSERT(!"CAIHumanStateSearch::Update: Need to handle this with goal system.");
					}
				}
				else
				{
					GetAI()->PlaySearchSound( kAIS_SearchFail );

					// DONE SEARCHING
					m_eStateStatus = kSStat_StateComplete;
				}

				return;
			}
		}
		else
		{
			if ( !m_bSearching )
			{
				// For Corner nodes, choose the appropriate direction and animation
				// depending on what side of the node we're facing.  Node points towards
				// a physical corner.

				if( m_pSearchNode->GetSearchType() == AINodeSearch::kSearch_Corner )
				{
					LTRotation rRot = LTRotation( m_pSearchNode->GetForward(), LTVector(0.0f, 1.0f, 0.0f) );

					if ( GetAI()->GetForwardVector().Dot( m_pSearchNode->GetRight() ) < 0.0f )
					{
						rRot.Rotate( LTVector(0.0f, 1.0f, 0.0f), MATH_DEGREES_TO_RADIANS( -135.f ) );
						m_aniSearch.Set( kAPG_Action, kAP_LookRight );
					}
					else {
						rRot.Rotate( LTVector(0.0f, 1.0f, 0.0f), MATH_DEGREES_TO_RADIANS( 135.f ) );
						GetAI()->FaceDir( m_pSearchNode->GetRight() );
						m_aniSearch.Set( kAPG_Action, kAP_LookLeft );
					}

					m_aniSearch.Set( kAPG_WeaponPosition, kAP_Up );
					GetAI()->FaceDir( rRot.Forward() );
				}

				// Default or OneWay nodes randomly choose an animation,
				// and face the direction of the node.

				else {
					GetRandomSearch(m_pSearchNode, &m_aniSearch);

					if ( m_bFace )
					{
						GetAI()->FaceDir(m_pSearchNode->GetForward());
					}
				}

				++m_cSearchedNodes;
			}

			m_bSearching = LTTRUE;
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSearch::FindNode()
{
	// Either use the AIs current region, or a region set
	// externally through SetRegion.

	AIRegion* pRegion = LTNULL;
	if( m_hDestRegion )
	{
		pRegion = (AIRegion*)g_pLTServer->HandleToObject( m_hDestRegion );
	}
	else if ( GetAI()->HasLastVolume() )
	{
		if ( GetAI()->GetLastVolume()->HasRegion() )
		{
			pRegion = GetAI()->GetLastVolume()->GetRegion();
		}
	}

	if ( pRegion && pRegion->IsSearchable() )
	{
		LTFLOAT fCurTime = g_pLTServer->GetTime();

		if ( m_pSearchNode )
		{
			m_pSearchNode->Unlock( GetAI()->m_hObject );
			m_pSearchNode->ResetActivationTime();
			m_pSearchNode = LTNULL;
		}

		AINodeSearch* pNode = pRegion->FindNearestSearchNode(GetAI()->GetPosition(), fCurTime);
		if ( !pNode )
		{
			return LTFALSE;
		}

		if( !m_pStrategyFollowPath->Set(pNode, LTFALSE) )
		{
			AITRACE( AIShowStates, ( m_pAI->m_hObject, "Could not find path to Search Node: %s", ::ToString( pNode->GetName() ) ) );

			// Reset the activation time slightly, so that
			// the AI will not find this node again until he
			// has visited other nodes.

			pNode->ResetActivationTime( 2.f );

			// return TRUE so that the state does not exit.

			return LTTRUE;
		}


		//
		// OneWay search nodes are only useable if the AI will arrive
		// at the node pointing in the same general direction that the
		// node points. This prevents an AI from arriving at a node and
		// immediately turning toward the direction he just came from.
		//
		// Corner nodes have the opposite requirement.  They are only
		// useable if the node is pointing opposite the general direction
		// the AI arrives from.
		//

		if( ( pNode->GetSearchType() == AINodeSearch::kSearch_OneWay ) ||
			( pNode->GetSearchType() == AINodeSearch::kSearch_Corner ) )
		{
			LTVector vDir;
			m_pStrategyFollowPath->GetFinalDir( &vDir );

			LTVector vCheckDir = pNode->GetForward();
			if( pNode->GetSearchType() == AINodeSearch::kSearch_Corner )
			{
				vCheckDir *= -1.f;
			}

			if ( vDir.Dot( vCheckDir ) < 0.0f )
			{
				// Reset the activation time slightly, so that
				// the AI will not find this node again until he
				// has visited a different node.

				pNode->ResetActivationTime( 2.f );

				// Reset the path, so that on the next update the AI
				// tries to find a new node.

				m_pStrategyFollowPath->Reset();

				AITRACE( AIShowStates, ( m_pAI->m_hObject, "SearchNode %s set to OneWay or Corner. Inaccessable now, will try again later.", ::ToString( pNode->GetName() ) ) );

				// return TRUE so that the state does not exit.

				return LTTRUE;
			}
		}

		m_pSearchNode = pNode;
		m_pSearchNode->Lock( GetAI()->m_hObject );

		if ( !m_bAdded )
		{
			pRegion->AddSearcher(GetAI());
			m_bAdded = LTTRUE;
			m_pSearchRegion = pRegion;
		}


		// Turn on lights in the destination volume.

		m_pStrategyToggleLights->Set( LTFALSE, LTTRUE, m_pSearchNode->GetPos() );
		if( m_pStrategyToggleLights->IsSet() )
		{
			m_pStrategyToggleLights->Update();
		}
		else {
			m_pStrategyToggleLights->Reset();
		}

		m_fNextPause = fCurTime + GetRandom(2.0f, 60.0f);
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	if ( m_bPause )
	{
		if( !GetAnimationContext()->IsTransitioning() )
		{
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
			GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);
			GetAnimationContext()->Lock();

			m_bPause = LTFALSE;
		}
	}
	else if( m_pStrategyToggleLights->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
		m_pStrategyToggleLights->UpdateAnimation();
		return;
	}
	else if ( m_bDone || m_pStrategyFollowPath->IsUnset() )
	{

	}
	else if ( m_pStrategyFollowPath->IsSet() )
	{
		if ( !m_bEngage )
		{
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
		}

		m_pStrategyFollowPath->UpdateAnimation();
	}
	else if ( m_pStrategyFollowPath->IsDone() )
	{
		_ASSERT(m_bSearching);
		m_aniSearch.Set(kAPG_Posture, kAP_Stand);
		GetAnimationContext()->SetProps(m_aniSearch);
		GetAnimationContext()->Lock();
	}

	if( GetAI()->GetLastVolume() && !GetAI()->GetLastVolume()->IsLit() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_InvestigateDark);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	if( m_bIgnoreJunctions )
	{



	// If we've reached the target's current volume, or a volume sharing her volume's lightswitch,
	// and she is hidden, give up the chase.

	if( GetAI()->HasTarget() && GetAI()->GetTarget()->GetCharacter()->IsHidden() )
	{
		AIVolume* pTargetVol = GetAI()->GetTarget()->GetCharacter()->GetCurrentVolume();
		if( pVolume == pTargetVol )
		{
			GetAI()->PlaySearchSound( kAIS_Search );
			m_bPause = LTTRUE;
			m_bCheckedLantern = LTFALSE;
			m_pStrategyFollowPath->Reset();
			return;
		}

		if( pTargetVol &&
			( pVolume->GetLightSwitchUseObjectNode() == pTargetVol->GetLightSwitchUseObjectNode() ) )
		{
			GetAI()->PlaySearchSound( kAIS_Search );
			m_bPause = LTTRUE;
			m_bCheckedLantern = LTFALSE;
			m_pStrategyFollowPath->Reset();
			return;
		}
	}



		return;
	}

	if ( pVolume->GetVolumeType() == AIVolume::kVolumeType_Junction )
	{
		AITRACE(AIShowJunctions, ( GetAI()->m_hObject, "Entered junction %s while searching.\n",
			 pVolume->GetName() ) );

		m_eStateStatus		= kSStat_Junction;
		m_pLastVolume		= GetAI()->GetLastVolume();
		m_pJunctionVolume	= pVolume;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyFlashlight->Load(pMsg);
	m_pStrategyToggleLights->Load(pMsg);
	m_aniSearch.Load(pMsg);

	LOAD_BOOL(m_bSearching);
	LOAD_COBJECT(m_pSearchNode, AINodeSearch);
	LOAD_BOOL(m_bFace);
	LOAD_DWORD(m_bEngage);
	LOAD_COBJECT(m_pSearchRegion, AIRegion);
	LOAD_HOBJECT(m_hDestRegion);
	LOAD_BOOL(m_bDone);
	LOAD_BOOL(m_bAdded);
	LOAD_BOOL(m_bPause);
	LOAD_BOOL(m_bIgnoreJunctions);
	LOAD_TIME(m_fNextPause);
	LOAD_COBJECT(m_pLastVolume, AIVolume);
	LOAD_COBJECT(m_pJunctionVolume, AIVolume);
	LOAD_BOOL(m_bLimitSearchCount);
	LOAD_DWORD(m_cSearchedNodes);
	LOAD_BOOL(m_bCheckedLantern);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyFlashlight->Save(pMsg);
	m_pStrategyToggleLights->Save(pMsg);
	m_aniSearch.Save(pMsg);

	SAVE_BOOL(m_bSearching);
	SAVE_COBJECT(m_pSearchNode);
	SAVE_BOOL(m_bFace);
	SAVE_DWORD(m_bEngage);
	SAVE_COBJECT(m_pSearchRegion);
	SAVE_HOBJECT(m_hDestRegion);
	SAVE_BOOL(m_bDone);
	SAVE_BOOL(m_bAdded);
	SAVE_BOOL(m_bPause);
	SAVE_BOOL(m_bIgnoreJunctions);
	SAVE_TIME(m_fNextPause);
	SAVE_COBJECT(m_pLastVolume);
	SAVE_COBJECT(m_pJunctionVolume);
	SAVE_BOOL(m_bLimitSearchCount);
	SAVE_DWORD(m_cSearchedNodes);
	SAVE_BOOL(m_bCheckedLantern);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateUseObject::CAIHumanStateUseObject()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = AI_FACTORY_NEW(CAIHumanStrategyOneShotAni);
	m_pStrategyToggleLights = AI_FACTORY_NEW(CAIHumanStrategyToggleLights);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_pUseNode = LTNULL;
	m_bLeaveNodeLocked = LTFALSE;

	// By default, the state handles locking/unlocking the node.
	// Some goals, (AIGoalAbstractUseObject) need the node to remain locked
	// during other activities (drawing/hostering), so the goal handles the
	// locking.

	m_bStateHandlesNodeLocking = LTTRUE;

	m_eAction			= kAP_None;
	m_eActivity			= kAP_None;
	m_eAwareness		= kAP_None;
	m_eWeaponPosition	= kAP_Down;
	m_eWeaponAction		= kAP_None;
	m_eMood				= kAP_None;
	m_ePose				= kAP_Stand;

	m_hstrSmartObjectCmd = LTNULL;

	m_fAnimTime = 0.f;
	m_fAnimTimer = 0.f;
	m_fMinFidgetTime = 0.f;
	m_fMaxFidgetTime = 0.f;
	m_fNextFidgetTime = 0.f;
	m_eLoopingAISound = kAIS_None;
	m_bPickedUp	= LTFALSE;

	m_bPlayFirstSound = LTTRUE;
	m_bPlayedFirstSound = LTFALSE;

	m_bRequireBareHands = LTFALSE;
	m_bTurnOffLights = LTFALSE;
	m_bTurnOnLights = LTFALSE;

	m_bVulnerable = LTFALSE;
	m_bAlertFirst = LTFALSE;

	m_hObject = NULL;
}

CAIHumanStateUseObject::~CAIHumanStateUseObject()
{
	if ( m_pUseNode && m_bStateHandlesNodeLocking )
	{
		m_pUseNode->Unlock( GetAI()->m_hObject );
	}

	AI_FACTORY_DELETE(m_pStrategyOneShotAni);
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyToggleLights);
	AI_FACTORY_DELETE(m_pStrategyShoot);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateUseObject::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyToggleLights->Init( pAIHuman, m_pStrategyFollowPath ) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init( pAIHuman ) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Pause(LTBOOL bPause)
{
	// Pausing is only allowed while the animation is playing.

	if( bPause )
	{
		if( m_eStateStatus == kSStat_PathComplete )
		{
			m_eStateStatus = kSStat_Paused;
			SetNextUpdate(GetAI()->m_hObject, UPDATE_NEVER);
		}

	}
	else {
		m_eStateStatus = kSStat_PathComplete;
		SetNextUpdate(GetAI()->m_hObject, c_fUpdateDelta);

		// Give a little extra time, so AI does not exit state right after unpausing.

		m_fAnimTimer -= 10.f;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Update()
{
	super::Update();

	if( m_eStateStatus == kSStat_Paused )
	{
		return;
	}

	// We have a use node.
	if( m_pUseNode )
	{
		// Go alert before walking anywhere.

		if( m_bAlertFirst )
		{
			GetAI()->FacePos( m_pUseNode->GetPos() );
			if( !m_bFirstUpdate && !GetAnimationContext()->IsLocked() )
			{
				m_bAlertFirst = LTFALSE;
			}
			else return;
		}

		// We are done if the node has been disabled.

		if( m_pUseNode->IsDisabled() )
		{
			m_eStateStatus = kSStat_StateComplete;
			return;
		}

		// Turn off lights in the source volume.
		// Turn on lights in the dest volume.

		if( m_bTurnOffLights || m_bTurnOnLights )
		{
			// Wait for AI to get back into the volumes (e.g. while getting
			// out of a bed or chair.

			if( ( m_eStateStatus == kSStat_Waiting ) &&
				( !GetAI()->GetCurrentVolume() ) )
			{
				return;
			}

			// Check if light state has changed.

			else if( m_pStrategyToggleLights->IsDone() )
			{
				if( m_pStrategyToggleLights->ResetIfDestStateChanged() )
				{
					if( m_eStateStatus == kSStat_PathComplete )
					{
						GetAI()->PlaySound( kAIS_LightsOff, LTFALSE );
					}

					if( GetAI()->GetCurrentVolume() )
					{
						m_eStateStatus = kSStat_Moving;
					}
					else {
						m_eStateStatus = kSStat_Waiting;
						return;
					}
				}
			}

			// Turn on/off lights.

			if( m_pStrategyToggleLights->IsUnset() )
			{
				m_pStrategyToggleLights->Set( m_bTurnOffLights, m_bTurnOnLights, m_pUseNode->GetPos() );
				if( m_eStateStatus == kSStat_Waiting )
				{
					m_eStateStatus = kSStat_Moving;
				}
			}

			if( m_pStrategyToggleLights->IsSet() )
			{
				m_pStrategyToggleLights->Update();
				return;
			}
		}
	}


	switch( m_eStateStatus )
	{
		//
		// Animating with no node. Start immediately.
		// If a node was provided, the status will be set to
		// kSStat_moving before the first update, from SetNode.
		//

		case kSStat_Initialized:
			StartAnimation();
			break;

		//
		// Moving to node for object.
		//

		case kSStat_Moving:
		{
			// Head and torso tracking.

			GetAI()->DisableNodeTracking();

			// Path may be unset if AI stopped to turn on a light.

			if( m_pStrategyFollowPath->IsUnset() )
			{
				if( !m_pStrategyFollowPath->Set( m_pUseNode, LTFALSE ) )
				{
					// CANNOT FIND PATH.

					if( m_pUseNode )
					{
						AITRACE( AIShowStates, ( GetAI()->m_hObject, "USEOBJECT OBJECT=%s -- unable to find path!", g_pLTServer->GetStringData(m_pUseNode->GetName()) ) );
					}
					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}

			if( m_pStrategyFollowPath->IsSet() )
			{
				// TODO: check for strategy failure
				m_pStrategyFollowPath->Update();
			}

			if ( m_pStrategyFollowPath->IsDone() )
			{
				// Just arrived at the object.

				// Attach childmodels.
				m_pUseNode->ApplyChildModels( GetAI()->m_hObject );
				GetAI()->GetAnimationContext()->Reset( GetAI()->m_hObject );
				//g_pLTServer->CPrint("resetting anims for %s", GetAI()->GetName());
				//g_pLTServer->CPrint("done");

				// Check if bare hands are required.

				if( m_bRequireBareHands && GetAI()->GetPrimaryWeapon() )
				{
					// HACK: for TO2 AI with grenades.

					if( m_pAI->GetPrimaryWeaponType() == kAIWeap_Thrown )
					{
					}

					// END HACK

					else {
						m_eStateStatus = kSStat_HolsterWeapon;
						return;
					}
				}

				// Pre-activate the node.

				m_pUseNode->PreActivate();

				// Do not play a transition in if the node is disturbed.
				// (e.g. do not open an open file cabinet).

				if( m_pUseNode->GetSmartObjectState() == kState_SmartObjectDisturbed )
				{
					GetAI()->GetAnimationContext()->PlayTransition( LTFALSE );
				}

				// start using object.

				StartAnimation();
			}
		}
		break;

		//
		// Using object.
		//

		case kSStat_PathComplete:
		{
			// Play first sound

			if( m_pUseNode &&
				( !m_bPlayedFirstSound ) &&
				( m_pUseNode->GetFirstSound() != kAIS_None ) &&
				( !m_pAI->GetAnimationContext()->IsTransitioning() ) )
			{
				GetAI()->PlaySound( m_pUseNode->GetFirstSound(), LTFALSE );
				m_bPlayedFirstSound = LTTRUE;
			}

			// Head and torso tracking.

			LTBOOL bEnableNodeTracking = LTTRUE;

			// Update non-looping animation.

			if( ( m_eActivity == kAP_None )
				&& ( ( m_eAction != kAP_None ) || ( m_eWeaponAction != kAP_None ) ) )
			{
				// TODO: check for strategy failure
				m_pStrategyOneShotAni->Update();

				// Check for completion.

				if(  m_pStrategyOneShotAni->IsDone() )
				{
					// Activate the node.

					if( m_pUseNode )
					{
						m_pUseNode->PostActivate();
					}

					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}


			// Update looping animation.

			else {

				if( m_fNextFidgetTime > 0.f )
				{
					// TODO: check for strategy failure
					m_pStrategyOneShotAni->Update();

					// Head and torso tracking.

					if( m_pStrategyOneShotAni->IsAnimating() )
					{
						bEnableNodeTracking = LTFALSE;
					}

					else if( m_pStrategyOneShotAni->IsDone() )
					{
						// Reset action timer.
						m_fNextFidgetTime = m_fAnimTimer + GetRandom( m_fMinFidgetTime, m_fMaxFidgetTime );
						m_pStrategyOneShotAni->Reset();
					}
					else if( m_pStrategyOneShotAni->IsUnset() && ( m_fNextFidgetTime <= m_fAnimTimer ) )
					{
						// Set an intermittent action.
						m_pStrategyOneShotAni->Set(kAPG_Action, kAP_Fidget);

						// Play a fidget sound.

						if( m_pUseNode && ( m_pUseNode->GetFidgetSound() != kAIS_None ) )
						{
							GetAI()->PlaySound( m_pUseNode->GetFidgetSound(), LTFALSE );
						}
					}
				}

				// Play looping sound.

				if( ( m_eLoopingAISound != kAIS_None ) &&
					( !GetAI()->IsPlayingDialogSound() ) &&
					( !GetAnimationContext()->IsPropSet( kAPG_Action, kAP_Fidget ) ) )
				{
					GetAI()->PlaySound( m_eLoopingAISound, LTTRUE );
				}

				// Check for completion.
				// AnimTime == 0 means anim loops infinitely.

				if( ( m_fAnimTime != 0.f ) &&
					( m_fAnimTimer >= m_fAnimTime ) &&
					( !m_pStrategyOneShotAni->IsSet() ) &&
					( !GetAnimationContext()->IsLocked() ) )
				{
					// Activate the node.

					if( m_pUseNode )
					{
						m_pUseNode->PostActivate();
					}

					// Head and torso tracking.

					GetAI()->DisableNodeTracking();

					m_eStateStatus = kSStat_StateComplete;
					return;
				}

				if( !GetAnimationContext()->IsTransitioning() )
				{
					// Decrement looping timer.
					m_fAnimTimer += g_pLTServer->GetFrameTime();
				}
				else {
					bEnableNodeTracking = LTFALSE;
				}

				// Head and torso tracking.

				if( m_pUseNode &&
					bEnableNodeTracking &&
					( GetAI()->GetTriggerNodeTrackingGroup() == kTrack_LookAt ) &&
					( GetAI()->GetActiveNodeTrackingGroup() != kTrack_LookAt ) )
				{
					SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, "TrackLookAt" );
				}
				else if( !bEnableNodeTracking )
				{
					GetAI()->DisableNodeTracking();
				}
			}

			// Continue using object.

			if( m_hObject )
			{
				// Set object to interact with.

				GetAI()->SetAnimObject( m_hObject );

				if( m_pUseNode && m_pUseNode->ShouldFaceNodeForward() )
				{
					GetAI()->FaceDir( m_pUseNode->GetForward() );
				}
				else {
					GetAI()->FaceObject(m_hObject);
				}

				// We got the pickup key, so pickup the object
				// if it still exists.
				if ( m_bPickedUp )
				{
					DoPickupObject();

					// Remove it
					g_pLTServer->RemoveObject(m_hObject);
				}
			}

			// There is no actual object, just a node.

			else if( m_pUseNode && m_pUseNode->ShouldFaceNodeForward() )
			{
				GetAI()->FaceDir( m_pUseNode->GetForward() );
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::StartAnimation()
{
	// start smartobject animation.

	SendTriggerMsgToObject(GetAI(), GetAI()->m_hObject, LTFALSE, g_pLTServer->GetStringData(m_hstrSmartObjectCmd));

	m_eStateStatus = kSStat_PathComplete;


	// Start a one-shot animation if not looping.

	if( ( m_eActivity == kAP_None )
		&& ( ( m_eAction != kAP_None ) || ( m_eWeaponAction != kAP_None) ) )
	{
		if( m_eAction != kAP_None )
		{
			m_pStrategyOneShotAni->Set(kAPG_Action, m_eAction);
		}
		else {
			m_pStrategyOneShotAni->Set(kAPG_WeaponAction, m_eWeaponAction);
		}
	}

	// Or set time for an intermittent action.

	else if( (m_fMinFidgetTime > 0.f) && (m_fMaxFidgetTime > 0.f) )
	{
		m_fNextFidgetTime = GetRandom( m_fMinFidgetTime, m_fMaxFidgetTime );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, m_eWeaponPosition);

	if( m_bAlertFirst )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);
		GetAnimationContext()->Lock();
		return;
	}


	if( m_pStrategyToggleLights->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		GetAnimationContext()->SetProp(kAPG_Awareness, m_eAwareness);
		m_pStrategyToggleLights->UpdateAnimation();
		return;
	}

	switch( m_eStateStatus )
	{
		// Waiting for AI to re-enter volumes (e.g. getting out of a bed or chair).

		case kSStat_Waiting:
			GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_Down );
			break;

		// Moving to node.

		case kSStat_Moving:
		{
			if( !m_pStrategyFollowPath->IsDone() )
			{
				m_pStrategyFollowPath->UpdateAnimation();
			}
			GetAnimationContext()->SetProp(kAPG_Awareness, m_eAwareness);
		}
		break;

		case kSStat_PathComplete:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
			GetAnimationContext()->SetProp(kAPG_Mood, m_eMood);

			if( ( m_eActivity == kAP_None ) &&
				( ( m_eAction != kAP_None ) || ( m_eWeaponAction != kAP_None ) ) )
			{
				m_pStrategyOneShotAni->UpdateAnimation();
			}

			else
			{
				GetAnimationContext()->SetProp(kAPG_Awareness, m_eActivity);

				if( m_pStrategyOneShotAni->IsSet() && !m_pStrategyOneShotAni->IsDone())
				{
					m_pStrategyOneShotAni->UpdateAnimation();
				}
				else {
					GetAnimationContext()->SetProp(kAPG_Action, kAP_Idle);
				}
			}
		}
		break;

		case kSStat_StateComplete:
		{
			// Keep playing a locked animation to prevent a pop on the last frame.

			if( m_eAction != kAP_None )
			{
				GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
				GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
				m_pStrategyOneShotAni->UpdateAnimation();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	if( m_pStrategyToggleLights->IsDone() )
	{
		m_pStrategyToggleLights->Reset();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	// These NameValue pairs come from trigger messages.

	//
	// UseObject can either play a single action:
	// ACTION=PushButton
	// or a looping base anim with intermittent actions:
	// ACTIVITY=Smoking LOOPTIME=[10.0,15.0] FIDGETFREQ=[3.0,5.0]
	//

	if ( !_stricmp(szName, "ACTION") )
	{
		m_eAction = CAnimationMgrList::GetPropFromName(szValue);
	}
	else if ( !_stricmp(szName, "WEAPONACTION") )
	{
		m_eWeaponAction = CAnimationMgrList::GetPropFromName(szValue);
	}
	else if ( !_stricmp(szName, "FIDGETFREQ") )
	{
		GetValueRange( GetAI(), szValue, &m_fMinFidgetTime, &m_fMaxFidgetTime );
	}
	else if ( !_stricmp(szName, "ACTIVITY") )
	{
		m_eActivity = CAnimationMgrList::GetPropFromName(szValue);
	}
	else if ( !_stricmp(szName, "POSE") )
	{
		m_ePose = CAnimationMgrList::GetPropFromName(szValue);
	}
	else if ( !_stricmp(szName, "MOOD") )
	{
		m_eMood = CAnimationMgrList::GetPropFromName(szValue);
	}
	else if ( !_stricmp(szName, "LOOPTIME") )
	{
		LTFLOAT fMin, fMax;
		GetValueRange( GetAI(), szValue, &fMin, &fMax );
		m_fAnimTime = GetRandom( fMin, fMax );
	}
	else if ( !_stricmp(szName, "LOCKNODE") )
	{
		// Increment the lock count so that the node will remain locked after state exits.
		AIASSERT(m_pUseNode != LTNULL, GetAI()->m_hObject, "CAIHumanStateUseObject::HandleNameValuePair: UseNode is NULL.");
		m_bLeaveNodeLocked = !_stricmp(szValue, "TRUE");
		if( m_pUseNode && m_bLeaveNodeLocked )
		{
			m_pUseNode->Lock( GetAI()->m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateUseObject::SetNode(AINodeUseObject* pUseNode)
{
	AIASSERT(pUseNode != LTNULL, GetAI()->m_hObject, "CAIHumanStateUseObject::SetNode: Node is NULL.");

	if( pUseNode == m_pUseNode )
	{
		return LTTRUE;
	}

	m_hObject = LTNULL;

	// Validate new node.

	if( m_bStateHandlesNodeLocking && pUseNode->IsLocked() )
	{
		AITRACE( AIShowStates, ( GetAI()->m_hObject, "USEOBJECT DEST=%s - use node is already locked!", g_pLTServer->GetStringData(pUseNode->GetName() ) ) );
		return LTFALSE;
	}

	if ( pUseNode->HasObject())
	{
		HOBJECT hObj = NULL;
		LTRESULT ltRes = FindNamedObject(pUseNode->GetObject(), hObj);
		m_hObject = hObj;
		if( LT_OK != ltRes )
		{
			AITRACE( AIShowStates, ( GetAI()->m_hObject, "USEOBJECT DEST=%s - this object does not exist!", g_pLTServer->GetStringData(pUseNode->GetName() ) ) );
			return LTFALSE;
		}
	}

	// Set path to new node.

	if ( m_pStrategyFollowPath->Set(pUseNode, LTFALSE) )
	{
		if ( m_pUseNode && m_bStateHandlesNodeLocking )
		{
			m_pUseNode->Unlock( GetAI()->m_hObject );
		}

		m_pUseNode = pUseNode;
		if( m_bStateHandlesNodeLocking )
		{
			pUseNode->Lock( GetAI()->m_hObject );
		}

		// Set Walk or Run.

		if( ( m_eAwareness == kAP_Investigate ) && ( GetAI()->IsImmediatelyAlarmed() ) )
		{
			m_pStrategyFollowPath->SetMovement(kAP_Run);
		}
		else {
			m_pStrategyFollowPath->SetMovement( m_pUseNode->GetMovement() );
		}

		m_eStateStatus = kSStat_Moving;
	}
	else
	{
		m_hObject = LTNULL;
		AITRACE( AIShowStates, ( GetAI()->m_hObject, "USEOBJECT DEST=%s - unable to find path!", g_pLTServer->GetStringData( pUseNode->GetName() ) ) );

		// FINISHED USING OBJECT
		m_eStateStatus = kSStat_StateComplete;

		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyPickUp) )
	{
		m_bPickedUp = LTTRUE;
	}

	else if ( !_stricmp(szKey, c_szKeyFireWeapon) )
	{
		if( GetAI()->GetCurrentWeapon() && m_hObject )
		{
			m_pStrategyShoot->ForceFire( m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if( m_bVulnerable )
	{
		switch ( damage.eType )
		{
			case DT_UNSPECIFIED:
			case DT_BLEEDING:
			case DT_BULLET:
			case DT_BURN:
			case DT_CHOKE:
			case DT_CRUSH:
			case DT_ELECTROCUTE:
			case DT_EXPLODE:
			case DT_FREEZE:
			case DT_POISON:
				SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, "DESTROY");
				break;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::DoPickupObject()
{
	if( !m_hObject )
		return;

	HCLASS hClass = g_pLTServer->GetObjectClass(m_hObject);

	// Only handle picking up weapons for now.
	AIASSERT(hClass == g_pLTServer->GetClass("WeaponItem"), GetAI()->m_hObject, "CAIHumanStateUseObject::DoPickupObject: Non-weapon pickups not handled.");
    if ( hClass == g_pLTServer->GetClass("WeaponItem") )
	{
	    WeaponItem* pWeaponItem = (WeaponItem*)g_pLTServer->HandleToObject(m_hObject);

		char* szWeapon = g_pWeaponMgr->GetWeapon(pWeaponItem->GetWeaponId())->szName;
		char* szAmmo = g_pWeaponMgr->GetAmmo(pWeaponItem->GetAmmoId())->szName;

		char szAttachment[128];
		sprintf(szAttachment, "%s RightHand (%s,%s)", KEY_ATTACH, szWeapon, szAmmo);

		SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyOneShotAni->Load(pMsg);
	m_pStrategyToggleLights->Load(pMsg);
	m_pStrategyShoot->Load(pMsg);

	LOAD_HOBJECT(m_hObject);
	LOAD_COBJECT(m_pUseNode, AINodeUseObject);

	LOAD_BOOL(m_bLeaveNodeLocked);
	LOAD_BOOL(m_bStateHandlesNodeLocking);
	LOAD_HSTRING(m_hstrSmartObjectCmd);
	LOAD_BOOL(m_bPickedUp);
	LOAD_DWORD_CAST(m_eAction, EnumAnimProp);
	LOAD_DWORD_CAST(m_eActivity, EnumAnimProp);
	LOAD_DWORD_CAST(m_eAwareness, EnumAnimProp);
	LOAD_DWORD_CAST(m_eWeaponPosition, EnumAnimProp);
	LOAD_DWORD_CAST(m_eWeaponAction, EnumAnimProp);
	LOAD_DWORD_CAST(m_eMood, EnumAnimProp);
	LOAD_FLOAT(m_fAnimTime);
	LOAD_FLOAT(m_fAnimTimer);
	LOAD_FLOAT(m_fMinFidgetTime);
	LOAD_FLOAT(m_fMaxFidgetTime);
	LOAD_FLOAT(m_fNextFidgetTime);
	LOAD_DWORD_CAST(m_eLoopingAISound, EnumAISoundType);
	LOAD_BOOL(m_bRequireBareHands);
	LOAD_BOOL(m_bTurnOffLights);
	LOAD_BOOL(m_bTurnOnLights);
	LOAD_BOOL(m_bVulnerable);
	LOAD_BOOL(m_bPlayedFirstSound);
	LOAD_BOOL(m_bAlertFirst);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyOneShotAni->Save(pMsg);
	m_pStrategyToggleLights->Save(pMsg);
	m_pStrategyShoot->Save(pMsg);

	SAVE_HOBJECT(m_hObject);
	SAVE_COBJECT(m_pUseNode);

	SAVE_BOOL(m_bLeaveNodeLocked);
	SAVE_BOOL(m_bStateHandlesNodeLocking);
	SAVE_HSTRING(m_hstrSmartObjectCmd);
	SAVE_BOOL(m_bPickedUp);
	SAVE_DWORD(m_eAction);
	SAVE_DWORD(m_eActivity);
	SAVE_DWORD(m_eAwareness);
	SAVE_DWORD(m_eWeaponPosition);
	SAVE_DWORD(m_eWeaponAction);
	SAVE_DWORD(m_eMood);
	SAVE_FLOAT(m_fAnimTime);
	SAVE_FLOAT(m_fAnimTimer);
	SAVE_FLOAT(m_fMinFidgetTime);
	SAVE_FLOAT(m_fMaxFidgetTime);
	SAVE_FLOAT(m_fNextFidgetTime);
	SAVE_DWORD(m_eLoopingAISound);
	SAVE_BOOL(m_bRequireBareHands);
	SAVE_BOOL(m_bTurnOffLights);
	SAVE_BOOL(m_bTurnOnLights);
	SAVE_BOOL(m_bVulnerable);
	SAVE_BOOL(m_bPlayedFirstSound);
	SAVE_BOOL(m_bAlertFirst);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateTail::CAIHumanStateTail()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_cTailNodes = 0;
	m_pTailNode = LTNULL;
}

// ----------------------------------------------------------------------- //

CAIHumanStateTail::~CAIHumanStateTail()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateTail::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( !_stricmp(pArgList->argv[0], "WHISTLE") && (GetRandom(0.0f, 1.0f) > 0.50f) )
	{
		GetAI()->PlaySound( kAIS_Tail, LTFALSE );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	LTVector vTargetPos;
    g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPos);

	AINodeTail* pNode = g_pAINodeMgr->FindTailNode( GetAI(), vTargetPos, GetAI()->GetPosition() );

	if ( !pNode )
	{
		if ( m_pTailNode != LTNULL )
		{
			pNode = m_pTailNode;
		}
		else
		{
			// COULD NOT FIND TAIL NODE
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	if ( pNode == m_pTailNode )
	{
		// We're en route to the right node... keep going

		if ( m_pStrategyFollowPath->IsDone() )
		{
			// We're at the spot... so do our animation, sound, face the given position

			m_eStateStatus = kSStat_Posing;

			GetAI()->FaceDir(pNode->GetForward());
		}
		else
		{
			// TODO: check for strategy failure
			m_pStrategyFollowPath->Update();
		}
	}
	else
	{
		m_pTailNode = pNode;

		if ( !m_pStrategyFollowPath->Set(pNode, LTFALSE) )
		{
			// COULD NOT SET PATH TO TAIL NODE
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		m_eStateStatus = kSStat_Moving;

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_eStateStatus )
	{
		case kSStat_Posing:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
			GetAnimationContext()->SetProp(kAPG_Action, kAP_Tail);
		}
		break;

		case kSStat_Moving:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
			m_pStrategyFollowPath->UpdateAnimation();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_COBJECT(m_pTailNode, AINodeTail);
	LOAD_DWORD(m_cTailNodes);

    uint32 iNode;
    for ( iNode = 0 ; iNode < m_cTailNodes ; iNode++ )
	{
		LOAD_COBJECT(m_apTailNodes[iNode], AINodeTail);
	}

	for ( iNode = m_cTailNodes ; iNode < kMaxTailNodes ; iNode++ )
	{
		m_apTailNodes[iNode] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_COBJECT(m_pTailNode);
	SAVE_DWORD(m_cTailNodes);

	for ( uint32 iNode = 0 ; iNode < m_cTailNodes ; iNode++ )
	{
		SAVE_COBJECT(m_apTailNodes[iNode]);
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateFollowFootprint::CAIHumanStateFollowFootprint()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_fLatestTimestamp = 0.0f;
	m_bSearch = LTFALSE;

	m_fNewTimestamp = 0.f;
	m_fLastTimestamp = 0.f;

	m_bPlayFirstSound = LTTRUE;
}

CAIHumanStateFollowFootprint::~CAIHumanStateFollowFootprint()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateFollowFootprint::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( m_bPlayFirstSound )
	{
		GetAI()->PlaySound( kAIS_FollowFootprint, LTFALSE );
	}

	LTBOOL bDone = LTFALSE;

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
	else if ( m_pStrategyFollowPath->IsUnset() || m_pStrategyFollowPath->IsDone() )
	{
		bDone = LTTRUE;

		if(m_fNewTimestamp > m_fLastTimestamp)
		{
			LTFLOAT fSeeEnemyFootprintDistanceSqr = GetAI()->GetSenseRecorder()->GetSenseDistanceSqr(kSense_SeeEnemyFootprint);

			if ( GetAI()->IsPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, m_vFootprintPos, fSeeEnemyFootprintDistanceSqr, LTFALSE, LTTRUE) )
			{
				if ( m_pStrategyFollowPath->Set(m_vFootprintPos, LTFALSE) )
				{
					m_fLastTimestamp = m_fNewTimestamp;
					bDone = LTFALSE;
				}
			}
		}
	}

	if ( bDone )
	{
		m_eStateStatus = kSStat_StateComplete;
		if ( m_bSearch == LTFALSE)
		{
			GetAI()->PlaySound( kAIS_InvestigateFail, LTFALSE );
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	if ( m_pStrategyFollowPath->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::ResetFootprint(LTFloat fTimestamp, const LTVector& vPos)
{
	m_fNewTimestamp = fTimestamp;
	m_vFootprintPos = vPos;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	LOAD_FLOAT(m_fLatestTimestamp);
	LOAD_BOOL(m_bSearch);

	LOAD_TIME(m_fNewTimestamp);
	LOAD_TIME(m_fLastTimestamp);
	LOAD_VECTOR(m_vFootprintPos);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	SAVE_FLOAT(m_fLatestTimestamp);
	SAVE_BOOL(m_bSearch);

	SAVE_TIME(m_fNewTimestamp);
	SAVE_TIME(m_fLastTimestamp);
	SAVE_VECTOR(m_vFootprintPos);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateInvestigate::CAIHumanStateInvestigate()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyToggleLights = AI_FACTORY_NEW(CAIHumanStrategyToggleLights);

	m_eSenseType = kSense_InvalidType;
	m_bSearch = LTFALSE;
	m_bPause = LTFALSE;

	m_bCheckedLantern = LTFALSE;

	m_fCloseEnoughDistSqr = 0.f;

	m_hEnemy = NULL;

	m_eAISound = kAIS_None;
	m_bPlayFirstSound = LTTRUE;
}

CAIHumanStateInvestigate::~CAIHumanStateInvestigate()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyToggleLights);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateInvestigate::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyToggleLights->Init(pAIHuman, m_pStrategyFollowPath) )
	{
		return LTFALSE;
	}

	// Run when AI has been alarmed enough.

	if( pAIHuman->IsImmediatelyAlarmed() )
	{
		m_pStrategyFollowPath->SetMovement(kAP_Run);
	}
	else {
		m_pStrategyFollowPath->SetMovement(kAP_Walk);
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Reset(HOBJECT hStimulusSource, EnumAISenseType eSenseType, EnumAISoundType eAISound, LTVector& vStimulusPos, LTVector& vStimulusDir)
{
	m_hEnemy	 = hStimulusSource;
	m_eSenseType = eSenseType;
	m_eAISound	 = eAISound;
	m_vPosition	 = vStimulusPos;
	m_vDirection = vStimulusDir;

	m_pStrategyFollowPath->Reset();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Update()
{
	super::Update();

	// HACK: for TO2 ninjas with lanterns. transition animation method is not reliable.

	if( ( !m_bFirstUpdate ) && ( !m_bCheckedLantern ) )
	{
		m_bCheckedLantern = LTTRUE;
		if( ( !GetAI()->GetAnimationContext()->IsTransitioning() ) &&
		    ( GetAI()->GetCurrentVolume() && !GetAI()->GetCurrentVolume()->IsLit() ) &&
			( GetAI()->GetBrain()->GetAIDataExist( kAIData_DisposeLantern ) ) )
		{
			char szAttachment[128];
			sprintf(szAttachment, "%s LIGHT Lantern", KEY_ATTACH);
			SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);
		}
	}

	// END HACK.

	// Make sure everything we need at this point is there

	if ( !m_hEnemy || (m_eSenseType == kSense_InvalidType) )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Pause before walking anywhere.

	if( m_bPause )
	{
		GetAI()->FacePos( m_vPosition );
		if( !m_bFirstUpdate && !GetAnimationContext()->IsLocked() )
		{
			m_bPause = LTFALSE;
		}
		else return;
	}

	// Turn on lights in the destination volume.

	if( m_pStrategyToggleLights->IsUnset() )
	{
		m_pStrategyToggleLights->Set( LTFALSE, LTTRUE, m_vPosition, m_vDirection );
	}

	if( m_pStrategyToggleLights->IsSet() )
	{
		m_pStrategyToggleLights->Update();
		return;
	}


	// Set our path if we haven't yet done so

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		LTVector vDestination;

		switch ( m_eSenseType )
		{
			case kSense_SeeEnemy:
		        g_pLTServer->GetObjectPos(m_hEnemy, &vDestination);
				break;

			default:
		        vDestination = m_vPosition;
				break;
		}

		LTBOOL bDivergePaths = GetAI()->GetBrain()->GetAIDataExist( kAIData_DivergePaths ) &&
			( GetAI()->GetBrain()->GetAIData( kAIData_DivergePaths ) > 0.f );

		if ( m_pStrategyFollowPath->Set( vDestination, m_vDirection, bDivergePaths ) == LTFALSE )
		{
			m_eStateStatus = kSStat_FailedSetPath;
			return;
		}
	}

	if( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	// Check if we are close enough to our dest.

	if( ( m_fCloseEnoughDistSqr > 0.f ) &&
		( m_vPosition.DistSqr( GetAI()->GetPosition() ) < m_fCloseEnoughDistSqr ) )
	{
		m_eStateStatus = kSStat_PathComplete;
	}

	else if ( m_pStrategyFollowPath->IsDone() )
	{
		m_eStateStatus = kSStat_PathComplete;
	}

	// We finished moving to where we saw the enemy

	if( m_eStateStatus == kSStat_PathComplete )
	{
		if( ( GetAI()->IsMajorlyAlarmed() || m_bSearch) && GetAI()->CanSearch() && IsCharacter( m_hEnemy ) )
		{
			GetAI()->Target(m_hEnemy);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	if( m_bPause )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);
		GetAnimationContext()->Lock();
		return;
	}

	if( m_pStrategyToggleLights->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
		m_pStrategyToggleLights->UpdateAnimation();
		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
		m_pStrategyFollowPath->UpdateAnimation();
	}

	if( GetAI()->GetLastVolume() && !GetAI()->GetLastVolume()->IsLit() )
	{
		GetAnimationContext()->SetProp(kAPG_Awareness, kAP_InvestigateDark);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyToggleLights->Load(pMsg);

	LOAD_DWORD_CAST(m_eSenseType, EnumAISenseType);
	LOAD_HOBJECT(m_hEnemy);
	LOAD_BOOL(m_bSearch);
	LOAD_VECTOR(m_vPosition);
	LOAD_DWORD_CAST(m_eSenseType, EnumAISenseType);
	LOAD_VECTOR(m_vDirection);
	LOAD_FLOAT(m_fCloseEnoughDistSqr);
	LOAD_DWORD_CAST(m_eAISound, EnumAISoundType);
	LOAD_BOOL(m_bPause);
	LOAD_BOOL(m_bCheckedLantern);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyToggleLights->Save(pMsg);

	SAVE_DWORD(m_eSenseType);
	SAVE_HOBJECT(m_hEnemy);
	SAVE_BOOL(m_bSearch);
	SAVE_VECTOR(m_vPosition);
	SAVE_DWORD(m_eSenseType);
	SAVE_VECTOR(m_vDirection);
	SAVE_FLOAT(m_fCloseEnoughDistSqr);
	SAVE_DWORD(m_eAISound);
	SAVE_BOOL(m_bPause);
	SAVE_BOOL(m_bCheckedLantern);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateCheckBody::CAIHumanStateCheckBody()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = AI_FACTORY_NEW(CAIHumanStrategyOneShotAni);

	m_hBody = NULL;
	m_eCheckAnim = kAP_KickBody;
	m_bPathToBody = LTTRUE;
}

CAIHumanStateCheckBody::~CAIHumanStateCheckBody()
{
	// Unregister self as checker.

	if ( m_hBody )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_CheckingBody, g_pLTServer->HandleToObject(m_hBody), GetAI() );

		if( IsAI( m_hBody ))
		{
			CAI *pAI = dynamic_cast<CAI*>(g_pLTServer->HandleToObject( m_hBody ));
			if( pAI )
			{
				pAI->SetCanCarry( pAI->IsUnconscious() );
			}
		}
		else if( IsBody( m_hBody ))
		{
			Body *pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( m_hBody ));
			if( pBody )
			{
				pBody->SetCanCarry( true );
			}
		}
	}

	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyOneShotAni);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCheckBody::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Update()
{
	super::Update();

	LTBOOL bSuccess = LTFALSE;

	// m_hBody may be a Body or a knocked out AI.

	ILTBaseClass *pChecker;

	Body* pBody = LTNULL;
	CAI* pAI = LTNULL;
	if( IsBody( m_hBody ) )
	{
		pBody = (Body*)g_pLTServer->HandleToObject(m_hBody);
		pChecker = g_pAICentralKnowledgeMgr->GetKnowledgeTarget( kCK_CheckingBody, pBody );
	}
	else if( IsAI( m_hBody ) )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hBody );
		pChecker = g_pAICentralKnowledgeMgr->GetKnowledgeTarget( kCK_CheckingBody, pAI );
	}

	// Bail if someone else is checking the body.

	if( pChecker && ( pChecker != GetAI() ) )
	{
		goto Done;
	}

	// Register self as checker.

	if( !pChecker )
	{
		if( pBody )
		{
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_CheckingBody, pBody, GetAI(), LTTRUE );
		}
		else if( pAI )
		{
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_CheckingBody, pAI, GetAI(), LTTRUE );
		}
	}


	if( pAI )
	{
		// Bail if AI just died, or got up.

		if( !pAI->GetDamageFlags() )
		{
			goto Done;
		}

		// Wait for AI to get in position.

		if( pAI->GetAnimationContext()->IsTransitioning() )
		{
			return;
		}
	}


	// Set our path if we haven't yet done so

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		FLOAT fOffset = 32.f;

		LTRotation rBodyRot;
		g_pLTServer->GetObjectRotation( m_hBody, &rBodyRot );

		LTVector vBodyPos;
		g_pLTServer->GetObjectPos( m_hBody, &vBodyPos );

		// Choose an animation.
		// Randomly kick or check pulse for dead bodies.
		// Always kick a knocked-out AI.

		CAnimationProps animProps;
		animProps.Set( kAPG_Posture, kAP_Stand );
		animProps.Set( kAPG_Weapon, GetAI()->GetCurrentWeaponProp() );
		animProps.Set( kAPG_Action, kAP_CheckPulse );

		// Choose a destination depending on the animation.

		LTVector vDest;
		if(	pBody && pBody->CanCheckPulse() &&
			( GetRandom( 0.f, 1.f) > 0.5f ) &&
			( GetAI()->GetAnimationContext()->AnimationExists( animProps ) ) )
		{
			m_eCheckAnim = kAP_CheckPulse;

			HMODELNODE hNeckNode;
			LTransform transform;

			if ( LT_OK != g_pModelLT->GetNode(m_hBody, "Neck", hNeckNode) )
			{
				goto Done;
			}

			g_pModelLT->GetNodeTransform(m_hBody, hNeckNode, transform, LTTRUE);
			g_pTransLT->GetPos(transform, vDest);

			vDest += ( fOffset * rBodyRot.Right() ) + ( 12.f * rBodyRot.Forward() );
		}

		else {
			m_eCheckAnim = kAP_KickBody;
			vDest = vBodyPos + ( fOffset * rBodyRot.Right() );
		}

		// Run to the dest.

		if ( !m_pStrategyFollowPath->Set( vDest, LTFALSE ) )
		{
			// If AI cannot find a path to the body, try to path to
			// the nearest volume location by using the direction from
			// the body to the AI.

			LTVector vDir = GetAI()->GetPosition() - vBodyPos;
			vDir.Normalize();

			m_bPathToBody = LTFALSE;
			m_eCheckAnim = kAP_Alert;
			if ( !m_pStrategyFollowPath->Set( vBodyPos, vDir, LTFALSE ) )
			{
				goto Done;
			}
		}
	}

	// See if we're done following the path to the object

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_pStrategyOneShotAni->IsDone() )
		{
			// We finished checking the Body

			bSuccess = LTTRUE;
			goto Done;
		}
		else
		{
			m_pStrategyOneShotAni->Update();
		}
	}
	else
	{
		// Move along our path

		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			// Just arrived at the object, now check the Body

			// If the body was picked up before we got to it end the check...

			if( (pAI && pAI->BeingCarried()) || (pBody && pBody->BeingCarried()) )
			{
				goto Done;
			}

			// Don't allow the body to be pickedup...

			if( pAI )
			{
				pAI->SetCanCarry( false );
			}
			else if( pBody )
			{
				pBody->SetCanCarry( false );
			}

			// Face the body.

			if( m_bPathToBody )
			{
				LTRotation rBodyRot;
					g_pLTServer->GetObjectRotation( m_hBody, &rBodyRot );

				GetAI()->FaceDir( -rBodyRot.Right() );
			}
			else {
				LTVector vBodyPos;
				g_pLTServer->GetObjectPos( m_hBody, &vBodyPos );

				GetAI()->FacePos( vBodyPos );
			}

			m_pStrategyOneShotAni->Set( kAPG_Action, m_eCheckAnim );

			// Play our sound

			if( !GetAI()->IsPlayingDialogSound() )
			{
				GetAI()->PlaySound( kAIS_SeeBody, LTFALSE );
			}

		}
	}

	return;

Done:

	if( pBody )
	{
		pBody->SetBodyResetTime( g_pLTServer->GetTime() + g_pAIButeMgr->GetResetTimers()->fBodyResetTimer );
	}

	if( bSuccess )
	{
		m_eStateStatus = kSStat_StateComplete;
	}
	else {
		m_eStateStatus = kSStat_FailedComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyFollowPath )
	{
		GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
		if ( m_pStrategyFollowPath->IsSet() )
		{
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
			m_pStrategyFollowPath->UpdateAnimation();
		}
		else if( m_pStrategyOneShotAni->IsSet() )
		{
			m_pStrategyOneShotAni->UpdateAnimation();
		}
		else {
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_Action, kAP_Alert);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::SetBody(HOBJECT hBody)
{
	m_hBody = hBody;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyOneShotAni->Load(pMsg);

	LOAD_HOBJECT(m_hBody);
	LOAD_DWORD_CAST(m_eCheckAnim, EnumAnimProp);
	LOAD_BOOL(m_bPathToBody);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyOneShotAni->Save(pMsg);

	SAVE_HOBJECT(m_hBody);
	SAVE_DWORD(m_eCheckAnim);
	SAVE_BOOL(m_bPathToBody);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateChase::CAIHumanStateChase()
{
	m_bSeen = LTFALSE;
	m_vSeenPosition = LTVector(0,0,0);

	m_pLastVolume			= LTNULL;
	m_pJunctionVolume		= LTNULL;
	m_pJunctionActionVolume	= LTNULL;
	m_pJunctionCorrectVolume= LTNULL;

	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = AI_FACTORY_NEW(CAIHumanStrategyOneShotAni);

	m_bStatusChanged = LTFALSE;
	m_bCanGetLost = LTFALSE;
	m_bUseStraightPath = LTFALSE;
	m_bSeekTarget = LTFALSE;

	m_bKeepDistance = LTTRUE;

	m_bOutOfBreath = LTFALSE;

	m_fSeeEnemyTime = 0.f;
	m_fEnduranceTime = 0.f;
	m_fVisionBlockedTime = 0.f;
}

CAIHumanStateChase::~CAIHumanStateChase()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyOneShotAni);

	if( m_pStrategyShoot )
	{
		AI_FACTORY_DELETE(m_pStrategyShoot);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateChase::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Only shoot while running if the brain allows it.

	if( GetAI()->GetBrain()->AttacksWhileMoving() )
	{
		m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
		if ( !m_pStrategyShoot->Init(pAIHuman) )
		{
			return LTFALSE;
		}
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	if ( GetAI()->IsSwimming() )
	{
		m_pStrategyFollowPath->SetMovement(kAP_Swim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(kAP_Run);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_eStateStatus = kSStat_Pursue;

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

void CAIHumanStateChase::SetStateStatus(EnumAIStateStatus eStateStatus)
{
	if( eStateStatus != m_eStateStatus )
	{
		super::SetStateStatus(eStateStatus);
		m_bStatusChanged = LTTRUE;
	}

	// Endurance.

	if( ( eStateStatus == kSStat_Pursue ) &&
		( GetAI()->GetBrain()->GetAIDataExist( kAIData_ChaseEndurance ) ) )
	{
		m_fEnduranceTime = g_pLTServer->GetTime() + GetAI()->GetBrain()->GetAIData( kAIData_ChaseEndurance );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Update()
{
	super::Update();

	// ---

	if ( !GetAI()->HasTarget() )
	{
		// NO TARGET
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	CAITarget* pTarget = GetAI()->GetTarget();

	// ---

	if ( m_pStrategyShoot && m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload(LTTRUE);
	}


	// Only TRON needs this block.

	if( g_pAIButeMgr->GetRules()->bEnableMeleeRepathing )
	{
		LTVector ToLastKnownPosition = m_pStrategyFollowPath->GetDest() - m_vSeenPosition;
		float flMovementDist = VEC_MAGSQR( ToLastKnownPosition );
		LTVector ToTarget = GetAI()->GetPosition() - m_vSeenPosition;
		float flRange = VEC_MAGSQR( ToTarget );
		if ( flMovementDist > flRange )
		{
			m_pStrategyFollowPath->Reset();

			CalculatePath();

			if ( m_eStateStatus == kSStat_StateComplete )
			{
				return;
			}
		}
	}

	// ---

	LTFLOAT fCurTime = g_pLTServer->GetTime();
	if( !pTarget->IsVisibleFromEye() )
	{
		m_fSeeEnemyTime = 0.f;
	}
	else if( m_fSeeEnemyTime == 0.f )
	{
		m_fSeeEnemyTime = fCurTime;
	}

	// ---

	if( m_eStateStatus == kSStat_Retry )
	{
		m_bCanGetLost = LTTRUE;
		m_pAIHuman->SetAwareness( kAware_Suspicious );

		// Head and Torso tracking.

		GetAI()->DisableNodeTracking();

		if( ( m_fSeeEnemyTime > 0.f ) && ( fCurTime - m_fSeeEnemyTime > 1.f ) )
		{
			m_eStateStatus = kSStat_Pursue;
		}
		else if(m_bStatusChanged)
		{
			GetAI()->PlaySearchSound( kAIS_SearchFail );

			if ( !m_pStrategyFollowPath->Set( m_pJunctionVolume->GetCenter(), LTFALSE ) )
			{
				// CAN'T SET A PATH TO THEIR LAST POSITION
				m_eStateStatus = kSStat_StateComplete;

				AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase Retry: Error setting path" ) );
				if( GetAI()->GetLastVolume() )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI in volume %s", GetAI()->GetLastVolume()->GetName() ) );
				}
				else {
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI not in volumes" ) );
				}

				return;
			}

			m_bStatusChanged = LTFALSE;
		}
		else if( m_pStrategyFollowPath->IsDone() )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	// --

	else if( m_eStateStatus == kSStat_Lost )
	{
		m_bCanGetLost = LTTRUE;
		m_pAIHuman->SetAwareness( kAware_Suspicious );

		// Head and Torso tracking.

		GetAI()->DisableNodeTracking();

		if( ( m_fSeeEnemyTime > 0.f ) && ( fCurTime - m_fSeeEnemyTime > 1.f ) )
		{
			m_eStateStatus = kSStat_Pursue;
		}
		else if(m_bStatusChanged)
		{
			if( !GetAI()->IsPlayingDialogSound() )
			{
				GetAI()->PlaySearchSound( kAIS_Search );
			}

			SetRandomPathFromJunction();
			m_bStatusChanged = LTFALSE;
		}
		else if( m_pStrategyFollowPath->IsDone() )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	// --

	if(m_eStateStatus == kSStat_Pursue)
	{
		// Some AI may get out of breath after running for some time.

		if( ( m_fEnduranceTime > 0.f ) && ( fCurTime > m_fEnduranceTime ) )
		{
			GetAI()->DisableNodeTracking();
			if( m_pStrategyOneShotAni->IsUnset() )
			{
				GetAI()->PlaySound( kAIS_CatchBreath, LTFALSE );

				m_pStrategyFollowPath->Reset();
				m_bSeen = LTFALSE;
				m_bOutOfBreath = LTTRUE;

				m_pStrategyOneShotAni->Set( kAPG_Action, kAP_CatchBreath );
				return;
			}
			else if( m_pStrategyOneShotAni->IsDone() )
			{
				m_bOutOfBreath = LTFALSE;
				m_pStrategyOneShotAni->Reset();
				m_fEnduranceTime = fCurTime + GetAI()->GetBrain()->GetAIData( kAIData_ChaseEndurance );
			}
			else {
				m_pStrategyOneShotAni->Update();
				return;
			}
		}

		if ( m_bFirstUpdate ||
			 m_bSeekTarget ||
			( ( m_fSeeEnemyTime > 0.f ) && ( fCurTime - m_fSeeEnemyTime > 1.f ) ) )
		{
			if( m_bFirstUpdate )
			{
				if( !GetAI()->IsPlayingDialogSound() )
				{
					GetAI()->PlayCombatSound( kAIS_Chase );
				}

				// Head and Torso tracking.

				GetAI()->SetNodeTrackingTarget( kTrack_AimAt, pTarget->GetObject(), "Head" );

				// Endurance.

				if( GetAI()->GetBrain()->GetAIDataExist( kAIData_ChaseEndurance ) )
				{
					m_fEnduranceTime = fCurTime + GetAI()->GetBrain()->GetAIData( kAIData_ChaseEndurance );
				}
			}

			if ( pTarget->GetCharacter()->HasLastVolume() )
			{
				m_vSeenPosition = pTarget->GetCharacter()->GetLastVolumePos();
				m_bSeen = LTTRUE;
				m_bCanGetLost = LTFALSE;
				m_pAIHuman->SetAwareness( kAware_Alert );
			}
		}

		// Head and Torso tracking.

		if( !m_bCanGetLost )
		{
			GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );
		}
		else {
			GetAI()->DisableNodeTracking();
		}

		// ---

		RangeStatus eRangeStatus = GetAI()->GetBrain()->GetRangeStatus();

		// ---

		if ( m_pStrategyShoot )
		{
			// Shoot blind while running.

			m_pStrategyShoot->SetShootBlind( m_pStrategyFollowPath->IsSet() );

			if ( m_pStrategyShoot->IsFiring() )
			{
				// Be sure to update the strategy if we are in the middle of
				// firing as this means that we recieved a firing key this frame
				// which has not yet been processed

				m_pStrategyShoot->Update();
			}
			else if ( ( eRangeStatus != eRangeStatusTooFar ) &&
					  ( pTarget->IsVisibleFromWeapon() ) )
			{
				m_pStrategyShoot->Update();
			}
		}

		// ---

		if ( pTarget->IsVisibleCompletely() )
		{
			// Stop seeking once target has been seen.
			// AI ignores junctions while seeking.

			m_bSeekTarget = LTFALSE;

			// If we can see them, and they are not too far away,
			// just stay here and fire at them.
			// Other goals should take over at this point, but the chase
			// goal should always stay resident as a fallback.

			if( m_bKeepDistance &&
				( eRangeStatus != eRangeStatusTooFar ) &&
				( !GetAI()->GetAIMovement()->IsMovementLocked() ) &&
				( !m_pAI->GetAnimationContext()->IsLocked() ) )
			{
				// Head and Torso tracking.

				GetAI()->EnableNodeTracking( kTrack_AimAt, pTarget->GetObject() );

				// WE CAN SEE OUR TARGET, AND IS IN RANGE.

				m_pStrategyFollowPath->Reset();

				// Endurance.

				if( GetAI()->GetBrain()->GetAIDataExist( kAIData_ChaseEndurance ) )
				{
					m_fEnduranceTime = fCurTime + GetAI()->GetBrain()->GetAIData( kAIData_ChaseEndurance );
				}
				return;
			}
		}

		// Stop if an ally gets in our way.  Ask him to dodge out of the
		// way if possible.

		else if( pTarget->GetVisionBlocker() &&
				IsAI( pTarget->GetVisionBlocker() ) &&
				( !m_pAI->GetAnimationContext()->IsLocked() ) &&
				( !m_pAI->GetAIMovement()->IsMovementLocked() ) )
		{
			CAI* pBlockingAI = (CAI*)g_pLTServer->HandleToObject( pTarget->GetVisionBlocker() );

			LTBOOL bOutOfBreath = LTFALSE;
			if( pBlockingAI->GetState()->GetStateType() == kState_HumanChase )
			{
				CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)pBlockingAI->GetState();
				bOutOfBreath = pStateChase->IsOutOfBreath();
			}

			if( ( !bOutOfBreath ) &&
				( GetAI()->GetForwardVector().Dot( pBlockingAI->GetPosition() - GetAI()->GetPosition() ) > 0.f ) )
			{
				if( pBlockingAI->GetPosition().DistSqr( GetAI()->GetPosition() ) < ( 64.f * 64.f ) )
				{
					// Head and Torso tracking.

					GetAI()->EnableNodeTracking( kTrack_AimAt, pTarget->GetObject() );

					m_pStrategyFollowPath->Reset();

					pBlockingAI->RequestDodge( GetAI()->m_hObject );

					m_fVisionBlockedTime = fCurTime;

					// Endurance.

					if( GetAI()->GetBrain()->GetAIDataExist( kAIData_ChaseEndurance ) )
					{
						m_fEnduranceTime = fCurTime + GetAI()->GetBrain()->GetAIData( kAIData_ChaseEndurance );
					}
					return;
				}
			}
		}

		// ---

		if( ( !GetAI()->GetAIMovement()->IsMovementLocked() ) &&
			( !m_pStrategyFollowPath->IsSet() ) &&
			( fCurTime > m_fVisionBlockedTime + 1.f ) )
		{
			// TODO: CHECK TO SEE IF WE'RE TOO CLOSE TO THEIR LAST VOLUME POSITION

			if( !CalculatePath() )
			{
				return;
			}
		}
	}

	// ---

	// If we are not done following the path, then follow it more
	if ( m_pStrategyFollowPath->IsSet() && !m_pStrategyFollowPath->IsDone() )
	{
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateChase::CalculatePath()
{
	// Cycle thru offsets to target position every time a path is set.
	// This keeps AIs from crowding each other when attacking, especially melee.
	// Static variable is shared between all callers to CalculatePath.

	static int iOffset = 0;
	LTVector vOffset(0.f, 0.f, 0.f);
	switch( iOffset )
	{
		case 0:	vOffset.x -= 96.f;
				break;

		case 1:	vOffset.x += 96.f;
				break;

		case 2:	vOffset.z -= 96.f;
				break;

		case 3:	vOffset.z += 96.f;
				break;
	}
	iOffset = ( iOffset + 1 ) % 4;


	m_pJunctionCorrectVolume = LTNULL;


	LTBOOL bDivergePaths = ( ( GetAI()->GetBrain()->GetAIDataExist( kAIData_DivergePaths ) ) &&
							 ( GetAI()->GetBrain()->GetAIData( kAIData_DivergePaths ) > 0.f ) );

	// Do not diverge paths if the enemy is in view.

	if( GetAI()->HasTarget() && GetAI()->GetTarget()->IsVisibleFromEye() )
	{
		bDivergePaths = LTFALSE;
	}


	// AI has seen target.

	if ( m_bSeen )
	{
		m_bSeen = LTFALSE;


		// Do NOT set the status to kSStat_StateComplete, even if a path cannot be found.
		// The chase goal needs to stay resident so that the AI remains aware of the enemy,
		// and stands alerts, aiming until in range.
		// Other goals should take over if the target is in range to attack, or a view
		// node is available to attack from.

		if ( !m_pStrategyFollowPath->Set(m_vSeenPosition + vOffset, bDivergePaths ) )
		{
			// Try path without offset.

			if ( !m_pStrategyFollowPath->Set(m_vSeenPosition, bDivergePaths ) )
			{
				// CAN'T SET A PATH TO THEIR LAST POSITION

				AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase saw enemy: Error setting path" ) );
				if( GetAI()->GetLastVolume() )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI in volume %s", GetAI()->GetLastVolume()->GetName() ) );
				}
				else {
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI not in volumes" ) );
				}

				AIVolume* pVolumeDest = g_pAIVolumeMgr->FindContainingVolume( LTNULL, m_vSeenPosition, eAxisAll, GetAI()->GetVerticalThreshold(), GetAI()->GetLastVolume() );
				if( pVolumeDest )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: Dest volume %s", pVolumeDest->GetName() ) );
				}
				else {
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: Dest not in volumes" ) );
				}

				return LTFALSE;
			}
		}
	}


	// AI has not seen target.

	else
	{
		m_bCanGetLost = LTTRUE;
		m_pAIHuman->SetAwareness( kAware_Suspicious );

		// Handle AIs that only use the preferred paths.

		if( GetAI()->GetBrain()->GetAIDataExist( kAIData_MinPathWeight ) )
		{
			LTFLOAT fMinPathWeight = GetAI()->GetBrain()->GetAIData( kAIData_MinPathWeight );
			if( fMinPathWeight > 0.f )
			{
				AIVolume* pVolumeLast = GetAI()->GetTarget()->GetCharacter()->GetLastVolume();
				if( ( !pVolumeLast ) ||
					( pVolumeLast->GetPathWeight( LTTRUE, LTFALSE ) > fMinPathWeight ) )
				{
					m_eStateStatus = kSStat_StateComplete;
					return LTFALSE;
				}
			}
		}

		// If the target is hidden, give up the chase.

		if( GetAI()->GetTarget()->GetCharacter()->IsHidden() )
		{
			GetAI()->PlaySearchSound( kAIS_Search );
			m_eStateStatus = kSStat_FailedComplete;
			return LTFALSE;
		}

		// Go to the targets last valid volume position.

		if ( !m_pStrategyFollowPath->Set( GetAI()->GetTarget()->GetCharacter()->GetLastVolumePos() + vOffset, bDivergePaths ) )
		{
			// Try path without offset.

			if ( !m_pStrategyFollowPath->Set( GetAI()->GetTarget()->GetCharacter()->GetLastVolumePos(), bDivergePaths ) )
			{
				// CAN'T SET A PATH TO THEIR LAST POSITION

				AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: Error setting path" ) );
				if( GetAI()->GetLastVolume() )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI in volume %s", GetAI()->GetLastVolume()->GetName() ) );
				}
				else {
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Chase: AI not in volumes" ) );
				}
				return LTFALSE;
			}
		}

		// Check if we were already at the destination.
		else if( m_pStrategyFollowPath->IsDone() )
		{
			// ALREADY AT THEIR LAST POSITION
			return LTTRUE;
		}


		// Cap path at next junction, so that AI runs straight instead of
		// turning right toward the player -- gives better odds of getting away.

		AIVolume* pNextJunctionVolume = m_pStrategyFollowPath->GetNextVolume( GetAI()->GetLastVolume(), AIVolume::kVolumeType_Junction );
		if( pNextJunctionVolume )
		{
			// Record the correct volume from the junction.

			m_pJunctionCorrectVolume = m_pStrategyFollowPath->GetNextVolume( pNextJunctionVolume, AIVolume::kVolumeType_None );
			m_pStrategyFollowPath->Set( pNextJunctionVolume, LTFALSE );
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	// The target got removed, probably due to going through a transam.
	if( !GetAI( )->HasTarget( ))
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// If we've reached the target's current volume, or a volume sharing her volume's lightswitch,
	// and she is hidden, give up the chase.

	if( GetAI()->GetTarget()->GetCharacter()->IsHidden() )
	{
		AIVolume* pTargetVol = GetAI()->GetTarget()->GetCharacter()->GetCurrentVolume();
		if( pVolume == pTargetVol )
		{
			GetAI()->PlaySearchSound( kAIS_Search );
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		if( pTargetVol &&
			( pVolume->GetLightSwitchUseObjectNode() == pTargetVol->GetLightSwitchUseObjectNode() ) )
		{
			GetAI()->PlaySearchSound( kAIS_Search );
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}


	m_pLastVolume = GetAI()->GetLastVolume();

	// Ignore junctions while returning to retry another option from previous junction.
	// Ignore junctions while seeking target.

	if( ( (m_eStateStatus == kSStat_Retry) && (pVolume != m_pJunctionVolume) ) ||
		m_bSeekTarget )
	{
		return;
	}

	if ( pVolume->GetVolumeType() != AIVolume::kVolumeType_Junction )
	{
		return;
	}

	if( ( !m_bCanGetLost ) || GetAI()->GetTarget()->IsVisibleFromEye() )
	{
		AITRACE(AIShowJunctions, ( GetAI()->m_hObject, "Saw target past junction %s. Runnning through.\n",
			 pVolume->GetName() ) );
		return;
	}


	// Some AIs ignore junctions.

	if( GetAI()->GetBrain()->GetAIDataExist( kAIData_IgnoreJunctions ) && (
		GetAI()->GetBrain()->GetAIData( kAIData_IgnoreJunctions ) > 0.f ) )
	{
		AITRACE(AIShowJunctions, ( GetAI()->m_hObject, "AI ignores junctions.  continuing thru %s\n",
					pVolume->GetName() ) );
		return;
	}

	// Play the dilemma sound.

	if( m_eStateStatus == kSStat_Pursue )
	{
		GetAI()->PlaySearchSound( kAIS_JunctionDilemma );
	}

	// Record the correct volume, which will actually take the AI to the target.

	if( !m_pJunctionCorrectVolume )
	{
		m_pJunctionCorrectVolume = m_pStrategyFollowPath->GetNextVolume( pVolume, AIVolume::kVolumeType_None );
	}

	// Record relevant junction info for goals querying the Chase state.

	AITRACE(AIShowJunctions, ( GetAI()->m_hObject, "Lost target in junction %s\n",
			 pVolume->GetName() ) );

	m_eStateStatus		= kSStat_Junction;
	m_pJunctionVolume	= pVolume;
	m_fSeeEnemyTime = 0.f;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::SetRandomPathFromJunction()
{
	if ( !m_pStrategyFollowPath->SetRandom(m_pLastVolume, m_pJunctionVolume, m_pJunctionActionVolume) )
	{
		// CAN'T SET A RANDOM PATH PAST THEIR LAST POSITION
		m_eStateStatus = kSStat_FailedComplete;
	}
	else {
		m_eStateStatus = kSStat_Lost;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::HandleVolumeExit(AIVolume* pVolume)
{
	super::HandleVolumeExit(pVolume);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	// Play an exhaustion animation.

	if( m_pStrategyOneShotAni->IsSet() )
	{
		m_pStrategyOneShotAni->UpdateAnimation();
		return;
	}

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->UpdateAnimation();
	}
	if( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}

	// See if we ought to upgrade our Run or Walk to a sprint.

	if ( !GetAI()->GetBrain()->GetAIDataExist(kAIData_SprintDist) )
		return;

	// ONLY override run and walk, as we are currently assuming they are
	// 'common' animations which have nothing unusual set about them.
	if ( !GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_Run) &&
		!GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_Walk) )
		return;

	// Do not override if someone locked anything at all.  Be careful about screwing with
	// animations
	if ( GetAnimationContext()->IsLocked() || GetAI()->GetAIMovement()->IsMovementLocked() )
		return;

	// If this AI supports sprinting, see if it ought to sprint at this
	// distance instead of whatever movement FollowPath set..?
	if ( GetAI()->HasTarget() && GetAI()->GetBrain()->GetAIDataExist(kAIData_SprintDist) )
	{
		HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
		LTVector vTargetPos;
		g_pLTServer->GetObjectPos(hTarget, &vTargetPos);
		LTFLOAT fTargetDistanceSqr = VEC_DISTSQR(vTargetPos, GetAI()->GetPosition());

		float flSprintDist = GetAI()->GetBrain()->GetAIData(kAIData_SprintDist);
		if ( flSprintDist*flSprintDist > fTargetDistanceSqr )
		{
			GetAnimationContext()->SetProp(kAPG_Movement, kAP_Sprint);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyOneShotAni->Load(pMsg);

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->Load(pMsg);
	}

	LOAD_BOOL(m_bSeen);
	LOAD_VECTOR(m_vSeenPosition);

	LOAD_BOOL(m_bCanGetLost);
	LOAD_BOOL(m_bStatusChanged);
	LOAD_COBJECT(m_pLastVolume, AIVolume);
	LOAD_COBJECT(m_pJunctionVolume, AIVolume);
	LOAD_COBJECT(m_pJunctionActionVolume, AIVolume);
	LOAD_COBJECT(m_pJunctionCorrectVolume, AIVolume);

	LOAD_BOOL(m_bUseStraightPath);
	LOAD_BOOL(m_bSeekTarget);

	LOAD_BOOL(m_bKeepDistance);

	LOAD_BOOL(m_bOutOfBreath);

	LOAD_TIME(m_fSeeEnemyTime);
	LOAD_TIME(m_fEnduranceTime);
	LOAD_TIME(m_fVisionBlockedTime);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyOneShotAni->Save(pMsg);

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->Save(pMsg);
	}

	SAVE_BOOL(m_bSeen);
	SAVE_VECTOR(m_vSeenPosition);

	SAVE_BOOL(m_bCanGetLost);
	SAVE_BOOL(m_bStatusChanged);
	SAVE_COBJECT(m_pLastVolume);
	SAVE_COBJECT(m_pJunctionVolume);
	SAVE_COBJECT(m_pJunctionActionVolume);
	SAVE_COBJECT(m_pJunctionCorrectVolume);

	SAVE_BOOL(m_bUseStraightPath);
	SAVE_BOOL(m_bSeekTarget);

	SAVE_BOOL(m_bKeepDistance);

	SAVE_BOOL(m_bOutOfBreath);

	SAVE_TIME(m_fSeeEnemyTime);
	SAVE_TIME(m_fEnduranceTime);
	SAVE_TIME(m_fVisionBlockedTime);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateDraw::CAIHumanStateDraw()
{
	m_bDrew = LTFALSE;
	m_eWeaponType = kAIWeap_Invalid;
	m_eWeaponProp = kAP_Weapon3;

	m_bPlayFirstSound = LTTRUE;
	m_bFired = LTFALSE;

	m_bFaceTarget = LTFALSE;

	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
}

CAIHumanStateDraw::~CAIHumanStateDraw()
{
	AI_FACTORY_DELETE(m_pStrategyShoot);

	// Clear any push that may have been set.

	if( GetAI()->HasTarget() )
	{
		GetAI()->GetTarget()->SetPushSpeed( 0.f );
		GetAI()->GetTarget()->SetPushMinDist( 0.f );
	}
}

// ------------------------------------------------------------------------ //

LTBOOL CAIHumanStateDraw::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Play draw animation for the weapon type being drawn.

	m_eWeaponType = pAIHuman->GetHolsterWeaponType();
	m_eWeaponProp = pAIHuman->GetWeaponProp( m_eWeaponType );

	// Create a shoot burst strategy so AI can injure targets while
	// drawing a weapon.

	if( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Update()
{
	super::Update();

	if ( (!m_bDrew) && (!GetAI()->HasHolsterString()) )
	{
		// NO WEAPON TO DRAW
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( GetAI()->HasTarget() && m_bFaceTarget )
	{
		if( m_bFirstUpdate )
		{
			GetAI()->FaceTarget();
		}

		// Knock target back if we're drawing a melee weapon.

		else if( ( !m_bFired ) &&
				 ( m_eWeaponType == kAIWeap_Melee ) )
		{
			GetAI()->GetTarget()->SetPushSpeed(800.f);
			GetAI()->GetTarget()->SetPushMinDist( GetAI()->GetDims().z * 2.f );

			if( m_bDrew &&
				( GetAI()->GetBrain()->GetRangeStatus( kAIWeap_Melee ) != eRangeStatusTooFar ) )
			{
				// AccuracyModifier is subtracted. Using 2 to make AI really
				// accurate when drawing the weapon.

				GetAI()->SetCurrentWeapon( kAIWeap_Melee );
				GetAI()->SetAccuracyModifier(2.f, 1.f);
				m_pStrategyShoot->ForceFire( LTNULL );
				m_bFired = LTTRUE;
			}
		}
	}

	if ( m_bDrew && !GetAnimationContext()->IsLocked() )
	{
		// DREW OUR WEAPON
		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);

	GetAnimationContext()->SetProp(kAPG_Weapon, m_eWeaponProp);
	GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Draw);
	GetAnimationContext()->Lock();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( GetAI()->HasHolsterString() && !_stricmp(pArgList->argv[0], "DRAW") )
	{
		char szHolsterRight[64];
		char szHolsterLeft[64];
		GetAI()->GetRightAndLeftHolsterStrings( szHolsterRight, szHolsterLeft, 64 );

		char szAttachment[128];


		//
		// Right Hand
		//

		sprintf(szAttachment, "%s RightHand (%s)", KEY_ATTACH, szHolsterRight);
		SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);

		// Optional comma separates a weapon from its ammo type.

		char* pComma = strchr(szHolsterRight, ',');
		if( pComma )
		{
			*pComma = '\0';
		}

		const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( szHolsterRight );
		if( pWeapon && pWeapon->szHolsterAttachment[0] )
		{
			char szTrigger[128];
			sprintf( szTrigger, "%s %s", KEY_DETACH, pWeapon->szHolsterAttachment );
			SendTriggerMsgToObject( GetAI(), GetAI()->GetObject(), LTFALSE, szTrigger );
		}


		//
		// Left Hand
		//

		if( szHolsterLeft[0] )
		{
			sprintf(szAttachment, "%s LeftHand (%s)", KEY_ATTACH, szHolsterLeft);
			SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);

			// Optional comma separates a weapon from its ammo type.

			pComma = strchr(szHolsterRight, ',');
			if( pComma )
			{
				*pComma = '\0';
			}

			pWeapon = g_pWeaponMgr->GetWeapon( szHolsterLeft );
			if( pWeapon && pWeapon->szHolsterAttachment[0] )
			{
				char szTrigger[128];
				sprintf( szTrigger, "%s %s", KEY_DETACH, pWeapon->szHolsterAttachment );
				SendTriggerMsgToObject( GetAI(), GetAI()->GetObject(), LTFALSE, szTrigger );
			}
		}

		// The tilda '~' indicates that this holster string was created from within
		// the code, and should be cleared after drawing a weapon.

		const char *szHolster = GetAI()->GetHolsterString();
		if( szHolster[0] == '~' )
		{
			GetAI()->ClearHolsterString();
		}

		m_bDrew = LTTRUE;

		// Re-init the strategy with whatever weapon was drawn.

		m_pStrategyShoot->Init( GetAI() );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load( pMsg );

	LOAD_BOOL(m_bDrew);
	LOAD_BOOL(m_bFired);
	LOAD_BOOL(m_bFaceTarget);
	LOAD_DWORD_CAST(m_eWeaponType, EnumAIWeaponType);
	LOAD_DWORD_CAST(m_eWeaponProp, EnumAnimProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save( pMsg );

	SAVE_BOOL(m_bDrew);
	SAVE_BOOL(m_bFired);
	SAVE_BOOL(m_bFaceTarget);
	SAVE_DWORD(m_eWeaponType);
	SAVE_DWORD(m_eWeaponProp);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateHolster::CAIHumanStateHolster()
{
	m_bHolstered = LTFALSE;
	m_eWeapon = kAP_None;
}

// ------------------------------------------------------------------------ //

LTBOOL CAIHumanStateHolster::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pAIHuman->SetAwareness( kAware_Relaxed );

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHolster::Update()
{
	super::Update();

	if( m_eWeapon == kAP_None )
	{
		m_eWeapon = GetAI()->GetCurrentWeaponProp();
	}

	if ( ( !m_bHolstered ) && ( !GetAI()->GetCurrentWeapon() ) )
	{
		// ALREADY HAS A HOLSTERED WEAPON
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( m_bHolstered && !GetAnimationContext()->IsLocked() )
	{
		// DREW OUR WEAPON
		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHolster::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);

	GetAnimationContext()->SetProp(kAPG_Weapon, m_eWeapon);
	GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Holster);
	GetAnimationContext()->Lock();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHolster::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	// Check for invalid arg list.

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] )
	{
		return;
	}

	// Check that AI has a weapon, and the correct model string.

	if ( !GetAI()->GetCurrentWeapon() || _stricmp(pArgList->argv[0], "HOLSTER") )
	{
		return;
	}

	// Look for a weapon in the right hand.

	CAttachment* pAttachment = GetAI()->GetAttachments()->GetAttachment("RightHand");
	if( !pAttachment || pAttachment->GetType() != ATTACHMENT_TYPE_WEAPON )
	{
		AIASSERT( NULL, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: No righthand weapon.");
		return;
	}
	
	char szTrigger[128];
	char szHolster[128];
	szHolster[0] = '\0';


	//
	// Right Hand
	//

	CAttachmentWeapon* pWeaponAttachment = (CAttachmentWeapon*)pAttachment;
	const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( pWeaponAttachment->GetWeapons()->GetCurWeaponId() );
	AIASSERT( pWeapon, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find weapon.");

	// Attach holstered model.

	if( pWeapon && pWeapon->szHolsterAttachment[0] )
	{
		sprintf( szTrigger, "%s %s", KEY_ATTACH, pWeapon->szHolsterAttachment );
		SendTriggerMsgToObject( GetAI(), GetAI()->GetObject(), LTFALSE, szTrigger );
	}

	// Create holster string for right hand.

	if( !GetAI()->HasHolsterString() )
	{
		// The tilda '~' indicates that this holster string was created from within
		// the code, and should be cleared after drawing a weapon.

		szHolster[0] = '~';
		strcpy( szHolster + 1, pWeapon->szName );		

		// Concatenate ammo type.

		if( pWeaponAttachment->GetWeapons()->GetCurWeapon() )
		{
			const AMMO* pAmmo = g_pWeaponMgr->GetAmmo( pWeaponAttachment->GetWeapons()->GetCurWeapon()->GetAmmoId() );
			if( pAmmo )
			{
				strcat( szHolster, "," );		
				strcat( szHolster, pAmmo->szName );
			}
			else {
				AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find right hand weapon ammo.");
			}
		}
		else {
			AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find right hand weapon.");
		}
	}

	// Detach weapon attachment from right hand.

	char szDetach[128];
	sprintf(szDetach, "%s RightHand", KEY_DETACH );		
	SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szDetach);

	// Detach flashlight (in case of rifle-mounted flashlights).

	sprintf( szDetach, "%s Light", KEY_DETACH );		
	SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetach );
	
	//
	// Left Hand
	//

	// Look for a weapon in the left hand.
	
	pAttachment = GetAI()->GetAttachments()->GetAttachment("LeftHand");
	if( pAttachment && ( pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON ) )
	{
		CAttachmentWeapon* pWeaponAttachment = (CAttachmentWeapon*)pAttachment;
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( pWeaponAttachment->GetWeapons()->GetCurWeaponId() );
		AIASSERT( pWeapon, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find weapon.");

		// Attach holstered model.

		if( pWeapon && pWeapon->szHolsterAttachment[0] )
		{
			sprintf( szTrigger, "%s %s", KEY_ATTACH, pWeapon->szHolsterAttachment );
			SendTriggerMsgToObject( GetAI(), GetAI()->GetObject(), LTFALSE, szTrigger );
		}

		if( szHolster[0] )
		{
			// Append holster string for left hand.

			strcat( szHolster, ";" );		
			strcat( szHolster, pWeapon->szName );
			
			// Concatenate ammo type.

			if( pWeaponAttachment->GetWeapons()->GetCurWeapon() )
			{
				const AMMO* pAmmo = g_pWeaponMgr->GetAmmo( pWeaponAttachment->GetWeapons()->GetCurWeapon()->GetAmmoId() );
				if( pAmmo )
				{
					strcat( szHolster, "," );		
					strcat( szHolster, pAmmo->szName );
				}
				else {
					AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find left hand weapon ammo.");
				}
			}
			else {
				AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateHolster::HandleModelString: Cannot find left hand weapon.");
			}
		}

		// Detach weapon attachment from left hand.

		sprintf(szDetach, "%s LeftHand", KEY_DETACH ); 
		SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szDetach);
	}
	
	if( szHolster[0] )
	{
		GetAI()->SetHolsterString( szHolster );
	}

	m_bHolstered = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHolster::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bHolstered);
	LOAD_DWORD_CAST(m_eWeapon, EnumAnimProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHolster::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bHolstered);
	SAVE_DWORD(m_eWeapon);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAttack::CAIHumanStateAttack()
{
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_pStrategyDodge = AI_FACTORY_NEW(CAIHumanStrategyDodge);
	m_pStrategyTaunt = AI_FACTORY_NEW(CAIHumanStrategyTaunt);

	m_aniPosture.Set(kAPG_Posture, kAP_Stand);

	m_fChaseTimer = 0.0f;
	m_fChaseDelay = 0.0f;

	m_fCrouchTimer = 0.0f;
	m_dwAttackFlags = kAttk_None;

	m_bPlayFirstSound = LTTRUE;
}

CAIHumanStateAttack::~CAIHumanStateAttack()
{
	AI_FACTORY_DELETE(m_pStrategyShoot);
	AI_FACTORY_DELETE(m_pStrategyDodge);
	AI_FACTORY_DELETE(m_pStrategyTaunt);

	if ( m_pStrategyGrenade )
	{
		AI_FACTORY_DELETE(m_pStrategyGrenade);
	}

	// Decrement counters.

	if( GetAI() )
	{
		if( m_dwAttackFlags & kAttk_Crouching )
		{
			g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AttackCroucher, GetAI() );
			AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackCroucher, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttack: Too many crouches registered!" );
		}

		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttack: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttack::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyDodge->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyTaunt->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( GetAI()->HasWeapon( kAIWeap_Thrown ) )
	{
		m_pStrategyGrenade = AI_FACTORY_NEW(CAIHumanStrategyGrenadeThrow);
		if ( !m_pStrategyGrenade->Init(pAIHuman) )
		{
			return LTFALSE;
		}
	}

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttack::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	// Dodge if we're getting pegged.

	if( GetAI()->GetBrain()->GetDodgeVectorCheckChance() > 0.f )
	{
		m_dwAttackFlags |= kAttk_ForceDodge;
	}

	// Reset our timers if we're getting pegged so we don't just stand there

	m_fChaseTimer = 0.0f;

	// Reset taunting.

	m_pStrategyTaunt->ResetTaunting();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::SetChaseDelay(LTFLOAT fDelay)
{
	m_fChaseDelay = fDelay + g_pLTServer->GetTime();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::SetPosture(EnumAnimProp ePosture)
{
	if(ePosture == kAP_Crouch)
	{
		m_aniPosture.Set(kAPG_Posture, kAP_Crouch);
	}
	else {
		m_aniPosture.Set(kAPG_Posture, kAP_Stand);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;		
		return;
	}

	HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
	HOBJECT hAI = GetAI()->m_hObject;

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, hTarget, "Head" );
		GetAI()->FaceTarget();
	}


	// Clear any push set by melee weapon attacks ( in StrategyShoot::Fire() ).

	GetAI()->GetTarget()->SetPushSpeed(0.f);
	GetAI()->GetTarget()->SetPushMinDist(0.f);

	// Make sure we have a weapon

	if ( !GetAI()->GetCurrentWeapon() )
	{
		m_eStateStatus = kSStat_FailedComplete;		
		return;
	}

	// Play our first sound if we should

	if( ( !GetAI()->IsPlayingDialogSound() ) &&
		( m_bPlayFirstSound || m_bFirstUpdate ) )
	{
		GetAI()->PlayCombatSound(kAIS_Attack);
	}

	// Select a posture

	if( ( GetAI()->GetCurrentWeaponType() != kAIWeap_Ranged ) ||
		( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Stand) ) ||
		( m_fCrouchTimer > GetAI()->GetBrain()->GetAttackPoseCrouchTime() ) )
	{
		m_aniPosture.Set(kAPG_Posture, kAP_Stand);

		if( m_dwAttackFlags & kAttk_Crouching )
		{
			g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AttackCroucher, GetAI() );
			AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget) ), hAI, "CAIHumanStateAttack::Update: Too many crouches registered!" );

			m_dwAttackFlags &= ~( kAttk_Crouching | kAttk_ForceCrouch);
		}
	}
	else if( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Crouch) )
	{
		m_aniPosture.Set(kAPG_Posture, kAP_Crouch);
		m_fCrouchTimer += g_pLTServer->GetFrameTime();

		if( !( m_dwAttackFlags & kAttk_Crouching ) )
		{
			AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget) ), hAI, "CAIHumanStateAttack::Update: Already registered crouch!" );
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget), LTTRUE );
			m_dwAttackFlags |= kAttk_Crouching;
		}
	}


	// Update taunting

	m_pStrategyTaunt->Update();
	if ( m_pStrategyTaunt->IsTaunting() )
	{
		// Head and Torso tracking.
	
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
		return;
	}

	// Update grenade throwing

	if ( m_pStrategyGrenade && m_pStrategyGrenade->IsThrowing() )
	{
		// Head and Torso tracking.
	
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );

		m_pStrategyGrenade->Update();
		return;
	}

	// Update dodging

	if ( m_pStrategyDodge->IsDodging() )
	{
		m_pStrategyDodge->Update();
		m_dwAttackFlags &= ~( kAttk_ForceDodge | kAttk_ForceCrouch );

		// Head and Torso tracking.
	
		if( ( m_pStrategyDodge->GetAction() == eDodgeActionBlock ) ||
			( m_pStrategyDodge->GetAction() == eDodgeActionShuffle ) )
		{
			GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
		}
		else {
			GetAI()->DisableNodeTracking();
			GetAI()->FaceTarget();
		}

		switch(m_pStrategyDodge->GetDodgeState())
		{
			case CAIHumanStrategyDodge::eStateFleeing:
				m_eStateStatus = kSStat_Flee;
				break;
		}

		return;
	}

	// Update shooting

	if ( m_pStrategyShoot->IsReloading() )
	{
		// Head and Torso tracking.
	
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );

		m_pStrategyShoot->Update();
		return;
	}

	// Crouch if we should.
	// Do not crouch if too many other AI are crouching.

	if( ( m_bFirstUpdate && 
		  ( g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget) ) == 0 ) &&
		  ( GetAI()->GetCurrentWeaponType() == kAIWeap_Ranged ) &&
		  ( GetRandom(0.0f, 1.0f) < GetAI()->GetBrain()->GetAttackPoseCrouchChance() ) )
		|| ( m_dwAttackFlags & kAttk_ForceCrouch ) )
	{
		m_aniPosture.Set(kAPG_Posture, kAP_Crouch);

		if( !( m_dwAttackFlags & kAttk_Crouching ) )
		{
			AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget) ), hAI, "CAIHumanStateAttack::Update: Already registered crouch!" );
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(hTarget), LTTRUE );
			m_dwAttackFlags |= kAttk_Crouching;
			m_dwAttackFlags &= ~kAttk_ForceDodge;
		}
	}

	// Check our range from the target

	RangeStatus eRangeStatus = CanChase(LTTRUE) ? GetAI()->GetBrain()->GetRangeStatus() : eRangeStatusOk;

	if ( eRangeStatus == eRangeStatusTooFar )
	{
		// Head and Torso tracking.
	
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );

		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// See if we should dodge

	if( ( !GetAI()->GetAnimationContext()->IsLocked() ) &&
		( ( m_aniPosture.GetProp() == kAP_Stand ) || ( m_dwAttackFlags & kAttk_ForceDodge ) ) )
	{
		if( m_dwAttackFlags & kAttk_ForceDodge )
		{
			m_aniPosture.Set(kAPG_Posture, kAP_Stand);
			m_pStrategyDodge->ForceDodge();
			m_dwAttackFlags &= ~kAttk_ForceDodge;
		}

		m_pStrategyDodge->Update();
		if( !m_bFirstUpdate )
		{
			switch(m_pStrategyDodge->GetDodgeState())
			{
				case CAIHumanStrategyDodge::eStateFleeing:
					m_eStateStatus = kSStat_Flee;
					return;
			}

			switch ( m_pStrategyDodge->GetStatus() )
			{
				case eDodgeStatusVector:
				case eDodgeStatusProjectile:
				{
					m_pStrategyDodge->Dodge();
					m_fCrouchTimer = 0.f;
					return;
				}
				break;
	
				case eDodgeStatusOk:
				{
				}
				break;
			}
		}
	}

	// Head and Torso tracking.

	if( m_dwAttackFlags & kAttk_Crouching )
	{
		GetAI()->DisableNodeTracking();
		GetAI()->FaceTarget();
	}
	else {
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
	}

	// Reload if we should

	if ( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload();

		return;
	}

	// Shoot at the target or get ready to chase it

	if ( m_pStrategyShoot->IsFiring() )
	{
		// Be sure to update the strategy if we are in the middle of
		// firing as this means that we recieved a firing key this frame
		// which has not yet been processed
		m_pStrategyShoot->Update();
		return;
	}

	// Get pointer to an AI that is blocking my view.

	CAITarget* pTarget = GetAI()->GetTarget();

	CAI* pBlockingAI = LTNULL;
	if( pTarget->GetVisionBlocker() && IsAI( pTarget->GetVisionBlocker() ) )
	{
		pBlockingAI = (CAI*)g_pLTServer->HandleToObject( pTarget->GetVisionBlocker() );
	}

	// Shoot if target is visible, or only blocked by a crouching ally and I'm not crouching.

	if ( pTarget->IsVisibleCompletely() || 
		( pBlockingAI && ( !( m_dwAttackFlags & kAttk_Crouching ) ) && pBlockingAI->GetAnimationContext() &&
		pBlockingAI->GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Crouch) ) )
	{
		if ( m_pStrategyGrenade && m_aniPosture.GetProp() == kAP_Stand && m_pStrategyGrenade->ShouldThrow() )
		{
			m_pStrategyGrenade->Throw();
		}
		else
		{
			m_pStrategyShoot->Update();
		}

		m_fChaseTimer = GetAI()->GetBrain()->GetAttackChaseTime() +
						GetRandom(GetAI()->GetBrain()->GetAttackChaseTimeRandomMin(),
								  GetAI()->GetBrain()->GetAttackChaseTimeRandomMax());
		return;
	}

	// Do not bail if vision is blocked by an ally.

	if( pBlockingAI && !( m_dwAttackFlags & kAttk_Crouching ) )
	{
		// Request that ally crouches, if I'm not crouching.

		if( !( m_dwAttackFlags & kAttk_Crouching ) && pBlockingAI->RequestCrouch( hAI ) )
		{
			return;
		}

		// If ally cannot crouch, dodge to get a clear shot.

		if( GetAI()->GetBrain()->GetDodgeVectorCheckChance() > 0.f )
		{
			m_dwAttackFlags |= kAttk_ForceDodge;
		}
		return;
	}

	// Bail if blocked by something other than AI.

	m_fChaseTimer -= g_pLTServer->GetFrameTime();

	if ( CanChase(LTFALSE) )
	{
		m_eStateStatus = kSStat_StateComplete;				
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttack::RequestCrouch(HOBJECT hRequestingAI)
{
	// Return TRUE only if a new crouch request will be granted.

	// AI cannot crouch.

	if( GetAI()->GetBrain()->GetAttackPoseCrouchChance() <= 0.f )
	{
		return LTFALSE;
	}

	// Do not crouch if not using a ranged weapon.

	if( GetAI()->GetCurrentWeaponType() != kAIWeap_Ranged )
	{
		return LTFALSE;
	}

	// AI has already crouched for too long.

	if( m_fCrouchTimer > GetAI()->GetBrain()->GetAttackPoseCrouchTime() )
	{
		return LTFALSE;
	}

	// Do not crouch if allies are too close together (inside each other).

	CAI* pAlly = (CAI*)g_pLTServer->HandleToObject( hRequestingAI );
	if( pAlly->GetPosition().DistSqr( GetAI()->GetPosition() ) < 48.f*48.f )
	{
		return LTFALSE;
	}

	// Requester yells "Get Down!"

	if( !( m_dwAttackFlags & kAttk_ForceCrouch ) )
	{
		m_dwAttackFlags |= kAttk_ForceCrouch;

		if( !pAlly->IsPlayingDialogSound() ) 
		{
			pAlly->PlaySound( kAIS_OrderCrouch, LTFALSE );
		}
	}

	// Grant the request.

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttack::RequestDodge(HOBJECT hRequestingAI)
{
	// Return TRUE only if a new dodge request will be granted.

	// AI cannot dodge.

	if( GetAI()->GetBrain()->GetDodgeVectorCheckChance() <= 0.f )
	{
		return LTFALSE;
	}

	m_dwAttackFlags |= kAttk_ForceDodge;

	// Grant the request.

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttack::CanChase(LTBOOL bOutOfRange)
{
	return ( ((m_fChaseTimer <= 0.0f) || bOutOfRange) && (g_pLTServer->GetTime() > m_fChaseDelay));
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyGrenade && m_pStrategyGrenade->IsThrowing() )
	{
		GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
		m_pStrategyGrenade->UpdateAnimation();
	}
	else
	{
		GetAnimationContext()->SetProp(m_aniPosture);
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

		if ( m_pStrategyTaunt->IsTaunting() )
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			m_pStrategyTaunt->UpdateAnimation();
		}
		else if ( m_pStrategyDodge->IsDodging() )
		{
			m_pStrategyDodge->UpdateAnimation();
		}
		else
		{
			m_pStrategyShoot->UpdateAnimation();
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	m_pStrategyDodge->Load(pMsg);
	m_pStrategyTaunt->Load(pMsg);

	EnumAIStrategyType eType;
	LOAD_DWORD_CAST(eType, EnumAIStrategyType);

	if( eType !=  kStrat_InvalidType )
	{
		m_pStrategyGrenade = (CAIHumanStrategyGrenade*)GetAI()->AI_FACTORY_NEW_Strategy( eType );
	}

	if ( m_pStrategyGrenade )
	{
		m_pStrategyGrenade->Init(GetAI());
		m_pStrategyGrenade->Load(pMsg);
	}

	m_aniPosture.Load(pMsg);

	LOAD_FLOAT(m_fChaseTimer);
	LOAD_TIME(m_fChaseDelay);

	LOAD_FLOAT(m_fCrouchTimer);
	LOAD_DWORD(m_dwAttackFlags);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	m_pStrategyDodge->Save(pMsg);
	m_pStrategyTaunt->Save(pMsg);

	if ( m_pStrategyGrenade )
	{
		SAVE_DWORD(m_pStrategyGrenade->GetStrategyType());
		m_pStrategyGrenade->Save(pMsg);
	}
	else
	{
		SAVE_DWORD(kStrat_InvalidType);
	}

	m_aniPosture.Save(pMsg);

	SAVE_FLOAT(m_fChaseTimer);
	SAVE_TIME(m_fChaseDelay);

	SAVE_FLOAT(m_fCrouchTimer);
	SAVE_DWORD(m_dwAttackFlags);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAttackProp::CAIHumanStateAttackProp()
{
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_hProp = NULL;
}

CAIHumanStateAttackProp::~CAIHumanStateAttackProp()
{
	AI_FACTORY_DELETE(m_pStrategyShoot);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackProp::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::SetProp(HOBJECT hProp)
{
	m_hProp = hProp;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Update()
{
	super::Update();

	if ( !m_hProp )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	Prop* pProp = (Prop*)g_pLTServer->HandleToObject(m_hProp);
	if( !pProp )
		return;

	if( pProp->GetState() == kState_PropDestroyed )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, m_hProp );
	}
	GetAI()->EnableNodeTracking( kTrack_AimAt, m_hProp );

	if ( m_pStrategyShoot->IsReloading() )
	{
		m_pStrategyShoot->Update(m_hProp);

		return;
	}

	if ( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload();

		return;
	}

	m_pStrategyShoot->Update(m_hProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	m_pStrategyShoot->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);

	LOAD_HOBJECT(m_hProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);

	SAVE_HOBJECT(m_hProp);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAssassinate::CAIHumanStateAssassinate()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_bIgnoreVisibility = LTTRUE;
}

CAIHumanStateAssassinate::~CAIHumanStateAssassinate()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);

	// Decrement counters.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAssassinate: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAssassinate::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAssassinate::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::SetNode(AINodeAssassinate* pNode)
{
	if ( !m_pStrategyFollowPath->Set(pNode, LTFALSE) )
	{
		m_eStateStatus = kSStat_StateComplete;
		g_pLTServer->CPrint("ASSASSINATE DEST=%s FollowPath failed", g_pLTServer->GetStringData(pNode->GetName()) );
	}

	m_pStrategyFollowPath->SetMovement( pNode->GetMovement() );

	m_bIgnoreVisibility = pNode->IgnoreVisibility();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget())
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Head and Torso tracking.

	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}

	// Shoot blind while running.

	m_pStrategyShoot->SetShootBlind( m_pStrategyFollowPath->IsSet() );

	if ( m_pStrategyShoot->IsReloading() )
	{
		GetAI()->FaceTarget();

		m_pStrategyShoot->Update();

		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// We haven't arrived at our "assassinate" dest yet, move there.

		m_pStrategyFollowPath->Update();

		// Head and Torso tracking.

		GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Our gun is drawn, start capping

		// Head and Torso tracking.
		// Face target if hit tracking limit.

		GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );

		if ( m_pStrategyShoot->IsFiring() )
		{
			// Be sure to update the strategy if we are in the middle of
			// firing as this means that we recieved a firing key this frame
			// which has not yet been processed
			m_pStrategyShoot->Update();
		}
		else if ( m_pStrategyShoot->ShouldReload() )
		{
			m_pStrategyShoot->Reload();
			return;
		}
		else if ( m_bIgnoreVisibility || GetAI()->GetTarget()->IsVisibleFromWeapon() )
		{
		// Only fire we if we can see our target or we're told to ignore visibility

			// TODO: check for strategy failure
			m_pStrategyShoot->Update();
		}
		else
		{
			// KEEP AIMING
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		m_pStrategyShoot->UpdateAnimation();
	}
	else
	{
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyShoot->Load(pMsg);

	LOAD_BOOL(m_bIgnoreVisibility);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyShoot->Save(pMsg);

	SAVE_BOOL(m_bIgnoreVisibility);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAttackFromCover::CAIHumanStateAttackFromCover()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_pStrategyCover = LTNULL;

	m_nRetries = 0;
	m_pCoverNode = LTNULL;

	m_bBoostedHitpoints = LTFALSE;

	m_bCovered = LTFALSE;

	m_bPlayFirstSound = LTTRUE;
}

CAIHumanStateAttackFromCover::~CAIHumanStateAttackFromCover()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);

	if ( m_pStrategyCover )
	{
		AI_FACTORY_DELETE(m_pStrategyCover);
	}

	if ( m_pCoverNode )
	{
		if ( m_bBoostedHitpoints && ( m_pCoverNode->GetHitpointsBoost() > 0.f ) )
		{
			GetAI()->BoostHitpoints(1.0f/m_pCoverNode->GetHitpointsBoost());
		}

		m_pCoverNode->Unlock( GetAI()->m_hObject );
	}

	// Decrement counters.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttackFromCover: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromCover::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	m_eStateStatus = kSStat_FindCover;

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttackFromCover::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Make sure we have a cover node.

	if ( !m_pCoverNode )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Make sure we have a weapon

	if ( !GetAI()->GetCurrentWeapon() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}

	// Play our first sound if we should

	if ( m_bPlayFirstSound || m_bFirstUpdate )
	{
		GetAI()->PlayCombatSound(kAIS_Cover);
	}

	// If we've had too many retries just attack

	if ( m_nRetries == (int32)GetAI()->GetBrain()->GetAttackFromCoverMaxRetries() )
	{
		// TOO MANY RETRIES
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Update accordingly with our state

	switch ( m_eStateStatus )
	{
		case kSStat_FindCover:
		{
			if( !m_pStrategyFollowPath->Set(m_pCoverNode->GetPos(), LTFALSE) )
			{
				m_eStateStatus = kSStat_FailedComplete;
				return;
			}

			// AIs that attack while moving may backup and fire.

			if( GetAI()->GetBrain()->AttacksWhileMoving() )
			{
				// Get the direction to the first waypoint.

				LTVector vPathDir;
				m_pStrategyFollowPath->GetInitialDir( &vPathDir );
				vPathDir.y = 0.f;
				vPathDir.Normalize();

				// If cover is behind AI, run backwards.

				if( vPathDir.Dot( GetAI()->GetForwardVector() ) < 0.f )
				{
					m_pStrategyFollowPath->SetMovement( kAP_BackUp );
				}
			}

			m_eStateStatus = kSStat_GotoCover;
			UpdateGotoCover();
		}
		break;

		case kSStat_GotoCover:
		{
			UpdateGotoCover();
		}
		break;

		case kSStat_UseCover:
		{
			UpdateUseCover();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateAnimation()
{
	super::UpdateAnimation();

	if( m_eStateStatus == kSStat_UseCover)
	{
		// Cover/fire strategies will override these if necessary

		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);

		m_pStrategyShoot->UpdateAnimation();
		m_pStrategyCover->UpdateAnimation();
	}
	else {
		GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

		m_pStrategyFollowPath->UpdateAnimation();

		// Fire while backing up.

		if( m_pStrategyFollowPath->GetMovement() == kAP_BackUp )
		{
			m_pStrategyShoot->UpdateAnimation();
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateGotoCover()
{
	// Abandon the state if target is blocking the path to the node.

	if( m_pCoverNode )
	{
		EnumNodeStatus eCoverStatus = m_pCoverNode->GetStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
		if( ( kStatus_ThreatBlockingPath == eCoverStatus ) &&
			( !GetAI()->GetAIMovement()->IsMovementLocked() ) )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	// TODO: check for strategy failure
	
	// Handle the 3 FollowPath state: Unset, Set, and Done.  Unset

	// Make sure that the path is not Unset
	AIASSERT( !m_pStrategyFollowPath->IsUnset(), GetAI()->m_hObject, "Path unset in goto cover!" );

	// Only attempt to follow a path if you have one.
	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->Update();

		// Fire while backing up.

		if( m_pStrategyFollowPath->GetMovement() == kAP_BackUp )
		{	
			// Turn around if target gets out of sight.

			if( !GetAI()->GetTarget()->IsVisibleFromEye() )
			{
				m_pStrategyFollowPath->SetMovement( kAP_Run );
			}
			else {
				if( m_pStrategyShoot->ShouldReload() )
				{
					m_pStrategyShoot->Reload(LTTRUE);
				}

				m_pStrategyShoot->Update();

				// Half of normal accuracy.
	
				GetAI()->SetAccuracyModifier( 0.5f, 2.0f);
			}
		}
	}

	// Head and Torso tracking.
	
	if( m_pStrategyFollowPath->GetMovement() == kAP_BackUp )
	{
		GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );
	}
	else {
		GetAI()->DisableNodeTracking();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// We've reached cover... clear out the cover strategy

		m_pStrategyCover->Clear();

		// Trigger the cover object if we need to

		if ( m_pCoverNode )
		{
			if ( m_pCoverNode->HasObject() )
			{
				HOBJECT hObject;
				if ( LT_OK == FindNamedObject(m_pCoverNode->GetObject(), hObject) )
				{
					SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
					m_pCoverNode->ClearObject();
				}
			}

			// Use the cover

			AIASSERT( !m_bBoostedHitpoints, GetAI()->m_hObject, "CAIHumanStateAttackFromCover::UpdateGotoCover: Hitpoints boosted more than once!" );
			if( m_pCoverNode->GetHitpointsBoost() > 0.f )
			{
				GetAI()->BoostHitpoints(m_pCoverNode->GetHitpointsBoost());
			}
			m_bBoostedHitpoints = LTTRUE;

			// Face the node's forward.

			GetAI()->FaceDir( m_pCoverNode->GetForward() );

			m_eStateStatus = kSStat_UseCover;
		}
		else
		{
			// COULD NOT FIND COVER
			m_eStateStatus = kSStat_FailedComplete;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateUseCover()
{
	_ASSERT(m_pStrategyCover);
	if ( !m_pStrategyCover ) return;

	// Never exit the state if an attack animation is in progress. 

	LTBOOL bAnimLocked = GetAI()->GetAnimationContext()->IsLocked();

	// TODO: check for strategy failure
	m_pStrategyCover->Update();

	if ( m_pStrategyCover->IsCovered() )
	{
		// Check to see if our cover is still valid

		if ( !m_pCoverNode ) return;

		if ( ( m_fElapsedTime > m_pCoverNode->GetTimeout() ) && !bAnimLocked )
		{
			// TIMED OUT AT COVER NODE
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		EnumNodeStatus eCoverStatus = m_pCoverNode->GetStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

		if( ( kStatus_ThreatInsideRadius == eCoverStatus ) && 
			( !bAnimLocked ) &&
			( !GetAI()->GetAIMovement()->IsMovementLocked() ) )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		if ( m_nRetries != -1 && (kStatus_Ok != eCoverStatus) && !bAnimLocked )
		{
			// TRY USING NEW COVER
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		if ( m_pStrategyShoot->ShouldReload() )
		{
			m_pStrategyShoot->Reload();
		}

		// If just finished attacking from cover, using one animation,
		// ensure that the shooting strategy is cleared to prevent shooting
		// at a wall.

		if( m_pStrategyCover->OneAnimCover() && 
			( !bAnimLocked ) &&
			( !m_bCovered || !GetAI()->GetTarget()->IsVisibleFromEye() ) )
		{
			m_pStrategyShoot->Clear();
		}

		if ( m_pStrategyShoot->IsFiring() )
		{
			// Be sure to update the strategy if we are in the middle of
			// firing as this means that we recieved a firing key this frame
			// which has not yet been processed
			m_pStrategyShoot->Update();
			m_pStrategyCover->Clear();
		}
		else if ( m_pStrategyShoot->IsReloading() )
		{
			// TODO: check for strategy failure
			m_pStrategyShoot->Update();
		}
		else if ( m_pStrategyShoot->ShouldReload() )
		{
			m_pStrategyShoot->Reload();
		}
	}

	if ( m_pStrategyCover->IsUncovered() )
	{		 
		// If we need to reload, go back to cover and then do so

		if ( m_pStrategyShoot->ShouldReload() )
		{
			m_pStrategyCover->Cover();
		}
		else if ( m_pStrategyShoot->IsFiring() )
		{
			// Be sure to update the strategy if we are in the middle of
			// firing as this means that we recieved a firing key this frame
			// which has not yet been processed
			m_pStrategyShoot->Update();
		}
		else if ( m_pStrategyCover->CanBlindFire() )
		{
			// If the attack from cover animation is all one animation,
			// then we're always blind firing because the animation
			// starts blind.  In that case, make it extremely accurate because
			// it takes the AI a long time to come out and take a shot.
			// This is a play-balancing cheat.

			if( m_pStrategyCover->OneAnimCover() )
			{
				GetAI()->SetAccuracyModifier( 2.f, 3.0f);
			}

			// If the animation is not all in one, then make the AI
			// less accurate momentarily when taking a shot from cover.

			else 
			{
				GetAI()->SetAccuracyModifier(-0.25f, 1.0f);
			}

			// TODO: check for strategy failure
			m_pStrategyShoot->Update(GetAI()->GetTarget()->GetObject());
		}
		else
		{
			// Shoot at our target if we can see it or if we've seen it recently

			if ( GetAI()->GetTarget()->IsVisibleFromEye() )
			{
				// TODO: check for strategy failure
				m_pStrategyShoot->Update();
			}
		}
	}

	m_bCovered = m_pStrategyCover->IsCovered();

	// Head and Torso tracking.

	GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::SetCoverNode(HOBJECT hCoverNode)
{
	AIASSERT( hCoverNode, GetAI()->m_hObject, "CAIHumanStateAttackFromCover::SetCoverNode: Cover node is NULL." );

	AINodeCover* pCoverNode = (AINodeCover*)g_pLTServer->HandleToObject( hCoverNode );
	AIASSERT( pCoverNode, GetAI()->m_hObject, "CAIHumanStateAttackFromCover::SetCoverNode: Cover node is NULL." );
	
	if( pCoverNode )
	{
		// Node should not be locked by anyone else.

		if( pCoverNode->IsLockedDisabledOrTimedOut() )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		// Release any node we're currently holding

		if ( m_pCoverNode )
		{
			m_pCoverNode->Unlock( GetAI()->m_hObject );
		}

		// Claim the node as ours
	
		m_pCoverNode = pCoverNode;
		m_pCoverNode->Lock( GetAI()->m_hObject );

		// Choose our cover strategy
	
		SetCoverStrategy(GetRandomCoverStrategy(pCoverNode));
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if ( kSStat_UseCover == m_eStateStatus && m_pStrategyCover && m_pStrategyCover->IsCovered() )
	{
		if ( m_pCoverNode )
		{
			m_eStateStatus = kSStat_FailedComplete;
		}
	}
}

// ----------------------------------------------------------------------- //

bool CAIHumanStateAttackFromCover::HandleCommand(const CParsedMsg &cMsg)
{
	if ( super::HandleCommand(cMsg) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

EnumAIStrategyType CAIHumanStateAttackFromCover::GetRandomCoverStrategy(AINodeCover* pNode)
{
	int nAvailableCovers = 0;
	uint32 adwCovers[AINodeCover::kNumFlags];
	uint32 dwCoverFlags = pNode->GetFlags();

	for ( uint32 iCover = 0 ; iCover < AINodeCover::kNumFlags ; iCover++ )
	{
		if ( dwCoverFlags & (1 << iCover) )
		{
			adwCovers[nAvailableCovers++] = (1 << iCover);
		}
	}

	if ( 0 == nAvailableCovers )
	{
		_ASSERT(LTFALSE);
        g_pLTServer->CPrint("GetRandomCoverStrategy - couldn't get random cover node for node %s", g_pLTServer->GetStringData(pNode->GetName()));
		return kStrat_HumanCoverBlind;
	}

	uint32 dwCover = adwCovers[GetRandom(0, nAvailableCovers-1)];

	if ( dwCover == AINodeCover::kFlagDuck )		return kStrat_HumanCoverDuck;
	if ( dwCover ==	AINodeCover::kFlagBlind )		return kStrat_HumanCoverBlind;
	if ( dwCover == AINodeCover::kFlag1WayCorner )	return kStrat_HumanCover1WayCorner;
	if ( dwCover == AINodeCover::kFlag2WayCorner )	return kStrat_HumanCover2WayCorner;

	_ASSERT(LTFALSE);
    g_pLTServer->CPrint("GetRandomCoverStrategy - couldn't get random cover node for node %s", g_pLTServer->GetStringData(pNode->GetName()));
	return kStrat_HumanCoverBlind;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromCover::SetCoverStrategy(EnumAIStrategyType eStrategy)
{
	if ( LTNULL == m_pCoverNode ) return LTFALSE;

	if ( m_pStrategyCover )
	{
		AI_FACTORY_DELETE(m_pStrategyCover);
		m_pStrategyCover = LTNULL;
	}

	m_pStrategyCover = (CAIHumanStrategyCover*)GetAI()->AI_FACTORY_NEW_Strategy( eStrategy );

	_ASSERT(m_pStrategyCover);

	if ( !m_pStrategyCover->Init(GetAI()) )
	{
		return LTFALSE;
	}

	m_pStrategyCover->SetCoverNode(m_pCoverNode);
	m_pStrategyCover->SetCoverTime(GetAI()->GetBrain()->GetAttackFromCoverCoverTime(), GetAI()->GetBrain()->GetAttackFromCoverCoverTimeRandom());
	m_pStrategyCover->SetUncoverTime(GetAI()->GetBrain()->GetAttackFromCoverUncoverTime(), GetAI()->GetBrain()->GetAttackFromCoverUncoverTimeRandom());

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	m_pStrategyFollowPath->Load(pMsg);

	LOAD_INT(m_nRetries);
	LOAD_BOOL(m_bBoostedHitpoints);
	LOAD_COBJECT(m_pCoverNode, AINodeCover);
	LOAD_BOOL(m_bCovered);

	// Instantiate the correct cover strategy

	LTBOOL bHadCoverStrategy;
	LOAD_BOOL(bHadCoverStrategy);

	if ( bHadCoverStrategy )
	{
		EnumAIStrategyType eStrategy;
		LOAD_DWORD_CAST(eStrategy, EnumAIStrategyType);

		if ( !SetCoverStrategy(eStrategy) )
		{
			return;
		}

		m_pStrategyCover->Load(pMsg);
	}
	else
	{
		if ( m_pStrategyCover )
		{
			AI_FACTORY_DELETE(m_pStrategyCover);
			m_pStrategyCover = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	m_pStrategyFollowPath->Save(pMsg);

	SAVE_INT(m_nRetries);
	SAVE_BOOL(m_bBoostedHitpoints);
	SAVE_COBJECT(m_pCoverNode);
	SAVE_BOOL(m_bCovered);

	// Write out the strategy type so we know which one to instantiate
	// Don't allow saving cover strategy if we're in a transition save,
	// because the cover node won't exist in the next level.
	uint8 nLoadFlags = g_pGameServerShell->GetLGFlags( );
	if ( m_pStrategyCover && nLoadFlags != LOAD_TRANSITION )
	{
		SAVE_BOOL(LTTRUE);
		SAVE_DWORD(m_pStrategyCover->GetStrategyType());
		m_pStrategyCover->Save(pMsg);
	}
	else
	{
		SAVE_BOOL(LTFALSE);
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAttackFromVantage::CAIHumanStateAttackFromVantage()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_pVantageNode = LTNULL;
	m_fAttackTimer = 0.0f;
	m_bFired = LTFALSE;

	m_bPlayFirstSound = LTTRUE;
	m_bIgnoreVisibility = LTFALSE;
}

CAIHumanStateAttackFromVantage::~CAIHumanStateAttackFromVantage()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);

	if ( m_pVantageNode) 
	{
		m_pVantageNode->Unlock( GetAI()->m_hObject );
	}

	// Decrement counters.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttackFromVantage: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromVantage::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttackFromVantage::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}


	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::CleanupState()
{
	// CleanupState handles cleanup when the state itself decided to exit.
	// If something external deletes the state, we do not necessarily want the cleanup.

	// Turn off object, if one was specified for the node.

	if ( m_pVantageNode && m_pVantageNode->HasObject() &&
		( !GetAI()->GetAnimationContext()->IsLocked() ) )
	{
		HOBJECT hObject;
		if ( LT_OK == FindNamedObject(m_pVantageNode->GetObject(), hObject) )
		{
			SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "OFF");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Make sure we have an enabled vantage node.

	if ( ( !m_pVantageNode ) || ( m_pVantageNode->IsDisabled() ) )
	{
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Make sure we have a weapon

	if ( !GetAI()->GetCurrentWeapon() )
	{
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}

	// Play our first sound if we should

	if ( m_bPlayFirstSound || m_bFirstUpdate )
	{
		GetAI()->PlayCombatSound(kAIS_Attack);
	}

	// Find a vantage node if we don't have one

	if ( m_eStateStatus == kSStat_Initialized )
	{
		if ( !m_pStrategyFollowPath->Set(m_pVantageNode->GetPos(), LTFALSE) )
		{
			// $STATECHANGE: COULD NOT REACH VANTAGE NODE
			CleanupState();
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		// Set our flags

		m_eStateStatus = kSStat_Moving;
	}

	// Update our movement if we're moving

	if ( m_eStateStatus == kSStat_Moving )
	{
		UpdateMoving();
	}

	// Update our attack if we're attacking

	if ( m_eStateStatus == kSStat_Attacking )
	{
		UpdateAttacking();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch ( m_eStateStatus )
	{
		case kSStat_Moving:
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		case kSStat_Attacking:
			m_pStrategyShoot->UpdateAnimation();
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateMoving()
{
	AIASSERT( !m_pStrategyFollowPath->IsUnset(), GetAI()->m_hObject, "CAIHumanStateAttackFromVantage::UpdateMoving::Path unset!" );

	// Abandon the state if target is blocking the path to the node.

	if( m_pVantageNode )
	{
		EnumNodeStatus eVantageStatus = m_pVantageNode->GetStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
		if( ( kStatus_ThreatBlockingPath == eVantageStatus ) &&
			( !GetAI()->GetAIMovement()->IsMovementLocked() ) )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	// Only attempt to follow a path if you have one.
	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->Update();

		// Head and Torso tracking.

		GetAI()->DisableNodeTracking();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Turn on object, if one was specified for the node.

		if ( m_pVantageNode && m_pVantageNode->HasObject() )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(m_pVantageNode->GetObject(), hObject) )
			{
				SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "ON");
			}
		}

		// Head and Torso tracking.
		// Face target if hit tracking limit.

		GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );

		m_eStateStatus = kSStat_Attacking;
		m_fAttackTimer = GetAI()->GetBrain()->GetAttackFromVantageAttackTime() + GetRandom(0.0f,GetAI()->GetBrain()->GetAttackFromVantageAttackTimeRandom());
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateAttacking()
{
	// Never exit the state if an attack animation is in progress. 

	LTBOOL bAnimLocked = GetAI()->GetAnimationContext()->IsLocked();

	// Check to see if our vantage is still valid

	if ( ( !m_pVantageNode ) && ( !bAnimLocked ) )
	{
		// $STATECHANGE: COULD NOT FIND VANTAGE NODE
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	EnumNodeStatus eVantageStatus = m_pVantageNode->GetStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

	if( ( kStatus_ThreatInsideRadius == eVantageStatus ) && 
		( !bAnimLocked ) &&
		( !GetAI()->GetAIMovement()->IsMovementLocked() ) )
	{
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Head and Torso tracking.
	// Face target if hit tracking limit.

	LTBOOL bFacingTarget = GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );

	if ( m_pStrategyShoot->IsReloading() )
	{
		m_pStrategyShoot->Update();
		return;
	}

	if ( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload();
		return;
	}

	if ( m_pStrategyShoot->IsFiring() )
	{
		// Be sure to update the strategy if we are in the middle of
		// firing as this means that we recieved a firing key this frame
		// which has not yet been processed
		m_pStrategyShoot->Update();
		m_bFired = LTTRUE;
	}
	else if ( GetAI()->HasTarget() &&
		( m_bIgnoreVisibility || GetAI()->GetTarget()->IsVisibleCompletely() ) )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
	else if( bFacingTarget && ( !bAnimLocked ) )
	{
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}
	
	if ( ( m_fAttackTimer < 0.0f ) && ( !bAnimLocked ) )
	{
		// Find another vantage node or exit state.
		CleanupState();
		m_eStateStatus = kSStat_FailedComplete;
	}

	// Only decrement timer after actually attacking.
	// This ensures that AI will get at least 1 shot off.

	if( m_bFired )
	{
		m_fAttackTimer -= g_pLTServer->GetFrameTime();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::SetVantageNode(HOBJECT hVantageNode)
{
	AIASSERT( hVantageNode, GetAI()->m_hObject, "CAIHumanStateAttackFromVantage::SetVantageNode: Vantage node is NULL." );

	AINodeVantage* pVantageNode = (AINodeVantage*)g_pLTServer->HandleToObject( hVantageNode );
	AIASSERT( pVantageNode, GetAI()->m_hObject, "CAIHumanStateAttackFromVantage::SetVantageNode: Vantage node is NULL." );
	
	if( pVantageNode )
	{
		// Node should not be locked by anyone else.

		if( pVantageNode->IsLockedDisabledOrTimedOut() )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		// Release any node we're currently holding

		if ( m_pVantageNode )
		{
			m_pVantageNode->Unlock( GetAI()->m_hObject );
		}

		// Claim the node as ours
	
		m_pVantageNode = pVantageNode;
		m_pVantageNode->Lock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

HOBJECT CAIHumanStateAttackFromVantage::GetVantageNode()
{
	if( m_pVantageNode )
	{
		return m_pVantageNode->m_hObject;
	}
	
	return LTNULL; 
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if ( m_eStateStatus == kSStat_Attacking  )
	{
		if( m_pVantageNode && ( !GetAI()->GetAnimationContext()->IsLocked() ) )
		{
			// Find another vantage node or exit state.
			CleanupState();
			m_eStateStatus = kSStat_FailedComplete;
		}
	}
}

// ----------------------------------------------------------------------- //

bool CAIHumanStateAttackFromVantage::HandleCommand(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_ReEvaluate("REEVALUATE");

	if ( super::HandleCommand(cMsg) )
	{
		return true;
	}

	if ( cMsg.GetArg(0) == s_cTok_ReEvaluate )
	{
		// Force a new vantage node selection next update
		m_fAttackTimer = -1.0f;

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	m_pStrategyFollowPath->Load(pMsg);

	LOAD_COBJECT(m_pVantageNode, AINodeVantage);
	LOAD_FLOAT(m_fAttackTimer);
	LOAD_BOOL(m_bIgnoreVisibility);
	LOAD_BOOL(m_bFired);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	m_pStrategyFollowPath->Save(pMsg);

	SAVE_COBJECT(m_pVantageNode);
	SAVE_FLOAT(m_fAttackTimer);
	SAVE_BOOL(m_bIgnoreVisibility);
	SAVE_BOOL(m_bFired);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAttackFromView::CAIHumanStateAttackFromView()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_pViewNode = LTNULL;
	m_fChaseTimer = 0.0f;
}

CAIHumanStateAttackFromView::~CAIHumanStateAttackFromView()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);

	if ( m_pViewNode )
	{
		m_pViewNode->Unlock( GetAI()->m_hObject );
	}

	// Decrement counters.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttackFromView: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromView::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttackFromView::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		// HAVE NO TARGET
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( m_pViewNode == LTNULL )
	{
		// COULD NOT FIND A VIEW NODE
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}

	// If we have a target start doing our stuff

	switch ( m_eStateStatus )
	{
		case kSStat_Moving:
		{
			UpdateMoving();
		}
		break;

		case kSStat_Attacking:
		{
			UpdateAttacking();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch ( m_eStateStatus )
	{
		case kSStat_Moving:
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		case kSStat_Attacking:
			m_pStrategyShoot->UpdateAnimation();
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::UpdateMoving()
{
	if( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		// Head and Torso tracking.
	
		GetAI()->DisableNodeTracking();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Attack

		m_eStateStatus = kSStat_Attacking;

		// Head and Torso tracking.
	
		GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::UpdateAttacking()
{
	// Head and Torso tracking.
	
	GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );

	if ( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload();
	}

	if ( m_pStrategyShoot->IsFiring() )
	{
		// Be sure to update the strategy if we are in the middle of
		// firing as this means that we recieved a firing key this frame
		// which has not yet been processed
		m_pStrategyShoot->Update();
	}
	else if ( m_pStrategyShoot->IsReloading() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
	else if ( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload();
	}
	else if( GetAI()->GetSenseRecorder()->HasAnyStimulation(kSense_SeeEnemy) ||
			 GetAI()->GetSenseRecorder()->HasAnyStimulation(kSense_SeeEnemyLean) )
	{
		// Invalidate hiding spots if AI is firing at player.

		if( IsPlayer( GetAI()->GetTarget()->GetObject() ) )
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( GetAI()->GetTarget()->GetObject() );
			if( pPlayer )
			{
				pPlayer->SetVisibleToEnemyAI( GetAI(), true ); 
			}
		}

		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
	else
	{
        m_fChaseTimer += g_pLTServer->GetFrameTime();
		if ( m_fChaseTimer > GetAI()->GetBrain()->GetAttackFromViewChaseTime() )
		{
			// Never exit the state if an attack animation is in progress. 
			
			if( GetAI()->GetAnimationContext()->IsLocked() )
			{
				// LOST SIGHT OF TARGET
				m_eStateStatus = kSStat_FailedComplete;
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::SetNode(AINode* pViewNode)
{
	if( !pViewNode )
	{
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateAttackFromView::SetNode: ViewNode is NULL." );
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( pViewNode->IsLockedDisabledOrTimedOut() )
	{
		g_pLTServer->CPrint("ATTACKFROMVIEW DEST - View node is already locked!");
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}
	else if ( !m_pStrategyFollowPath->Set(pViewNode->GetPos(), LTFALSE) )
	{
		g_pLTServer->CPrint("ATTACKFROMVIEW DEST - Could not set a path to node!");
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Release any node we're currently holding

	if ( m_pViewNode )
	{
		m_pViewNode->Unlock( GetAI()->m_hObject );
	}

	// Claim the node as ours

	m_pViewNode = pViewNode;
	m_pViewNode->Lock( GetAI()->m_hObject );

	// Goto the view node

	m_eStateStatus = kSStat_Moving;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	m_pStrategyFollowPath->Load(pMsg);

	LOAD_COBJECT(m_pViewNode, AINode);
	LOAD_FLOAT(m_fChaseTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	m_pStrategyFollowPath->Save(pMsg);

	SAVE_COBJECT(m_pViewNode);
	SAVE_FLOAT(m_fChaseTimer);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateCover::CAIHumanStateCover()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_pCoverNode = LTNULL;
}

CAIHumanStateCover::~CAIHumanStateCover()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);

	if ( m_pCoverNode )
	{
		m_pCoverNode->Unlock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCover::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Make sure we have a node.

	if( !m_pCoverNode )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	if ( m_bFirstUpdate )
	{
		GetAI()->PlaySound( kAIS_Cover, LTFALSE );
	}

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( !m_pStrategyFollowPath->Set( m_pCoverNode->GetPos(), LTFALSE ) )
		{
			// COULD NOT SET PATH TO COVER NODE
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			GetAI()->FaceTarget();
			m_fElapsedTime = 0.0f;
		}
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		m_eStateStatus = kSStat_UseCover;

		if ( m_pCoverNode && m_pCoverNode->HasObject() )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(m_pCoverNode->GetObject(), hObject) )
			{
				SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
				m_pCoverNode->ClearObject();
			}
		}

		if( m_fElapsedTime > m_pCoverNode->GetTimeout() )
		{
			// TIMED OUT AT COVER NODE
			m_eStateStatus = kSStat_StateComplete;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(kAPG_Posture, kAP_Crouch);
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
//		GetAnimationContext()->Lock();
	}
	else
	{
		GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
		GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);

		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::SetCoverNode(HOBJECT hCoverNode)
{
	AIASSERT( hCoverNode, GetAI()->m_hObject, "CAIHumanStateCover::SetCoverNode: Cover node is NULL." );

	AINodeCover* pCoverNode = (AINodeCover*)g_pLTServer->HandleToObject( hCoverNode );
	AIASSERT( pCoverNode, GetAI()->m_hObject, "CAIHumanStateCover::SetCoverNode: Cover node is NULL." );
	
	if( pCoverNode )
	{
		// Node should not be locked by anyone else.

		if( pCoverNode->IsLockedDisabledOrTimedOut() )
		{
			m_eStateStatus = kSStat_FailedComplete;
			return;
		}

		// Release any node we're currently holding

		if ( m_pCoverNode )
		{
			m_pCoverNode->Unlock( GetAI()->m_hObject );
		}

		// Claim the node as ours
	
		m_pCoverNode = pCoverNode;
		m_pCoverNode->Lock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if ( kSStat_UseCover == m_eStateStatus )
	{
		if ( m_pCoverNode )
		{
			m_eStateStatus = kSStat_FailedComplete;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_COBJECT(m_pCoverNode, AINodeCover);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_COBJECT(m_pCoverNode);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStatePanic::CAIHumanStatePanic()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_bAtPanicDestination = LTFALSE;
	m_pPanicNode = LTNULL;

	m_aniPanic.Set(kAPG_Posture, kAP_Crouch);

	m_bCanActivate = LTTRUE;
}

CAIHumanStatePanic::~CAIHumanStatePanic()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);

	if ( m_pPanicNode )
	{
		m_pPanicNode->Unlock( GetAI()->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePanic::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::SetPanicNode(AINodePanic* pPanicNode)
{
	if( !pPanicNode )
	{
		return;
	}

	if ( !m_pStrategyFollowPath->Set( pPanicNode->GetPos(), LTFALSE ) )
	{
		m_bAtPanicDestination = LTTRUE;
	}
	else
	{
		// Release any node we're currently holding

		if ( m_pPanicNode )
		{
			m_pPanicNode->Unlock( GetAI()->m_hObject );
		}

		m_bAtPanicDestination = LTFALSE;

		// Claim the node as ours

		m_pPanicNode = pPanicNode;
		m_pPanicNode->Lock( GetAI()->m_hObject );

		// Get the random panic animation

		GetRandomPanic(pPanicNode, &m_aniPanic);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Update()
{
	super::Update();

	// Make sure we still have our target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Has our panic node been disabled?

	if( m_pPanicNode && m_pPanicNode->IsDisabled() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	// Play our first sound

	if ( m_bFirstUpdate )
	{
		GetAI()->PlaySound( kAIS_Panic, LTTRUE );
	}

	if ( m_ePose == kAP_Sit )
	{
		m_bAtPanicDestination = LTTRUE;
	}

	// Pick our spot to run away to if we haven't done so yet

	if ( !m_bAtPanicDestination && m_pStrategyFollowPath->IsUnset() )
	{
		AINodePanic* pPanicNode = (AINodePanic*)g_pAINodeMgr->FindNearestNodeFromThreat( GetAI(), kNode_Panic, GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject(), 1.f);
		if ( pPanicNode )
		{
			SetPanicNode( pPanicNode );
		}
		else
		{
			m_bAtPanicDestination = LTTRUE;
		}
	}

	if ( m_bAtPanicDestination || m_pStrategyFollowPath->IsDone() )
	{
/*		if ( m_hPanicNode != LTNULL )
		{
			if ( AINode::HandleToObject(m_hPanicNode)->GetPanicFlags() & AINode::kPanicFlagCrouch )
			{
				// bleh
			}
		}
*/
		if ( m_ePose != kAP_Sit )
		{
			if( m_pPanicNode && m_pPanicNode->ShouldFaceNodeForward() )
			{
				GetAI()->FaceDir( m_pPanicNode->GetForward() );
			}

			else if ( GetAI()->GetTarget()->IsVisiblePartially() )
			{
				GetAI()->FaceTarget();
			}
		}

		// Play a sound if our target is threatening us

		if ( !GetAI()->IsPlayingDialogSound() && GetAI()->GetTarget()->IsAttacking() )
		{
			// $SOUND GetAI()->PlaySound(aisPanic);
		}
	}
	else if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_pPanicNode && m_pPanicNode->HasObject() )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(m_pPanicNode->GetObject(), hObject) )
			{
				SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
				m_pPanicNode->ClearObject();
			}
		}

		m_bAtPanicDestination = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Panic);

	switch ( m_ePose )
	{
		case kAP_Sit:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Sit);
		}
		break;

		case kAP_Stand:
		{
			if ( m_bAtPanicDestination )
			{
				GetAnimationContext()->SetProp(m_aniPanic);
			}
			else
			{
				GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
				m_pStrategyFollowPath->UpdateAnimation();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::GetRandomPanic(AINodePanic* pNode, CAnimationProp* pProp)
{
	int nAvailablePanics = 0;
	uint32 adwPanics[AINodePanic::kNumFlags];
	uint32 dwPanicFlags = pNode->GetFlags();

	for ( uint32 iPanic = 0 ; iPanic < AINodePanic::kNumFlags ; iPanic++ )
	{
		if ( dwPanicFlags & (1 << iPanic) )
		{
			adwPanics[nAvailablePanics++] = (1 << iPanic);
		}
	}

	if ( 0 == nAvailablePanics )
	{
        AITRACE( AIShowStates, ( GetAI()->m_hObject, "GetRandomPanic - couldn't get random Panic action for node %s", g_pLTServer->GetStringData(pNode->GetName() ) ) );
		pProp->Set(kAPG_Posture, kAP_Crouch);
		return;
	}

	uint32 dwPanic = adwPanics[GetRandom(0, nAvailablePanics-1)];

	if ( dwPanic == AINodePanic::kFlagStand )
	{
		pProp->Set(kAPG_Posture, kAP_Stand);
		return;
	}

	if ( dwPanic == AINodePanic::kFlagCrouch )
	{
		pProp->Set(kAPG_Posture, kAP_Crouch);
		return;
	}

    AITRACE( AIShowStates, ( GetAI()->m_hObject, "GetRandomPanic - couldn't get Panic action for node %s", g_pLTServer->GetStringData(pNode->GetName() ) ) );
	pProp->Set(kAPG_Posture, kAP_Crouch);
	return;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_aniPanic.Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_COBJECT(m_pPanicNode, AINodePanic);
	LOAD_BOOL(m_bAtPanicDestination);
	LOAD_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_aniPanic.Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_COBJECT(m_pPanicNode);
	SAVE_BOOL(m_bAtPanicDestination);
	SAVE_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateDistress::CAIHumanStateDistress()
{
	m_nDistressLevel = 0;
	m_fDistress = 0.0f;

	m_bCanActivate = LTTRUE;
}

// ------------------------------------------------------------------------ //

LTBOOL CAIHumanStateDistress::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Head and Torso tracking.
	// Face target if torso tracking does not reach.
	
	if( !GetAI()->IsNodeTrackingEnabled() )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_LookAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}
	GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );

	if( m_bFirstUpdate )
	{
		GetAI()->PlaySound( kAIS_Distress, LTTRUE );
	}

	// If our target is aiming at us, elevate distress

	LTBOOL bDistress = false;
	if ( GetAI()->GetTarget()->IsVisiblePartially() )
	{
        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		if ( pCharacter->HasDangerousWeapon() )
		{
			LTRotation rRot;
			LTVector vForward;
            g_pLTServer->GetObjectRotation(GetAI()->GetTarget()->GetObject(), &rRot);
			vForward = rRot.Forward();

			LTVector vDir;
			vDir = GetAI()->GetPosition() - GetAI()->GetTarget()->GetVisiblePosition();
			vDir.y = 0;
			vDir.Normalize();

			// TODO: bute this

			const static LTFLOAT fThreshhold = GetAI()->GetBrain()->GetDistressFacingThreshhold();

			if ( vDir.Dot(vForward) > fThreshhold )
			{
				bDistress = true;
			}
		}
	}

	// TODO: bute stimulation rates

	if ( bDistress )
	{
		// Only increase distress when enemy aims a dangerous weapon at you.

		LTFLOAT fIncreaseRate = GetAI()->GetBrain()->GetDistressIncreaseRate();
        m_fDistress += g_pLTServer->GetFrameTime()*fIncreaseRate;
	}
	else {
		LTFLOAT fDecreaseRate = GetAI()->GetBrain()->GetDistressDecreaseRate();
        m_fDistress = Max<LTFLOAT>(-3.0f, m_fDistress-g_pLTServer->GetFrameTime()*fDecreaseRate);
	}

	// See if we need to go to the next level

	if ( m_fDistress > 1.0f )
	{
		m_fDistress = 0.0f;
		m_nDistressLevel++;

		// Play sound

		// $SOUND GetAI()->PlaySound(aisPanic);
	}
	else if ( m_fDistress < -1.0f )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Craig says distressed AIs should not run away and panic, because it
	// can ruin the experience for the player -- can't talk to AI anymore.
/**
	// If distress level is max, panic

	const static int32 nLevels = GetAI()->GetBrain()->GetDistressLevels();
	if ( m_nDistressLevel == nLevels )
	{
		m_eStateStatus = kSStat_Panic;
		return;
	}
**/
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
	GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Distress);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT(m_nDistressLevel);
	LOAD_FLOAT(m_fDistress);
	LOAD_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT(m_nDistressLevel);
	SAVE_FLOAT(m_fDistress);
	SAVE_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateGetBackup::CAIHumanStateGetBackup()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_pBackupNode = LTNULL;

	m_bPlayFirstSound = LTTRUE;
}

// ----------------------------------------------------------------------- //

CAIHumanStateGetBackup::~CAIHumanStateGetBackup()
{
	if ( m_pBackupNode ) 
	{
		m_pBackupNode->Unlock( GetAI()->m_hObject );
		m_pBackupNode->ResetActivationTime();
	}

	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateGetBackup::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Update()
{
	super::Update();

	if( !m_pBackupNode )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Run command at last node, if there is one.

		if( m_pBackupNode->HasCmd() )
		{
			SendTriggerMsgToObject( m_pBackupNode, GetAI()->m_hObject, m_pBackupNode->GetCmd() );
		}

		m_eStateStatus = kSStat_StateComplete;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);

	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::SetDest(AINodeBackup* pNode)
{
	AIASSERT(pNode != LTNULL, GetAI()->m_hObject, "CAIHumanStateGetBackup::SetDest: Node is NULL.");

	if ( pNode->IsLockedDisabledOrTimedOut() )
	{
        AITRACE(AIShowStates, ( GetAI()->m_hObject, "GETBACKUP DEST - Backup node is already locked!" ) );
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if( m_pStrategyFollowPath->Set(pNode, LTFALSE) )
	{
		m_pBackupNode = pNode;
		pNode->Lock( GetAI()->m_hObject );

		m_pStrategyFollowPath->SetMovement( m_pBackupNode->GetMovement() );
	}
	else
	{
		AITRACE(AIShowStates, ( GetAI()->m_hObject, "GETBACKUP OBJECT -- unable to find path!" ) );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::SetEnemySeenPos( LTVector& vPos )
{
	// Node keeps track of where ally saw enemy.

	if( m_pBackupNode )
	{
		m_pBackupNode->SetEnemySeenPos( vPos );	
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);
	m_pStrategyShoot->Load(pMsg);

	LOAD_COBJECT(m_pBackupNode, AINodeBackup);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);
	m_pStrategyShoot->Save(pMsg);

	SAVE_COBJECT(m_pBackupNode);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateTalk::CAIHumanStateTalk()
{
	m_bNoCinematics = LTFALSE;

	m_eMood = kAP_Happy;

	m_fFaceTime = 15.0f;

	m_hFace = NULL;
}

CAIHumanStateTalk::~CAIHumanStateTalk()
{
	if( m_hFace )
	{
		GetAI()->FaceDir(m_vInitialForward);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateTalk::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_fFaceTimer = g_pLTServer->GetTime() + m_fFaceTime;
	m_vInitialForward = GetAI()->GetForwardVector();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::SetGesture(HSTRING hstrGesture)
{
	const char* pszGesture = g_pLTServer->GetStringData( hstrGesture );
	GetAI()->GetAnimationContext()->SetSpecial( pszGesture );
	GetAI()->GetAnimationContext()->PlaySpecial();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Update()
{
	if ( m_hFace )
	{
		if ( GetAI()->IsPlayingDialogSound() )
		{
			m_fFaceTimer = g_pLTServer->GetTime() + m_fFaceTime;
		}

		if ( g_pLTServer->GetTime() > m_fFaceTimer )
		{
			GetAI()->FaceDir(m_vInitialForward);
		}
		else
		{
			// Head and Torso tracking.
			// Face target if torso tracking does not reach.
	
			if( !GetAI()->IsNodeTrackingEnabled() )
			{
				GetAI()->SetNodeTrackingTarget( kTrack_LookAt, m_hFace, "Head" );
			}
			GetAI()->EnableNodeTracking( kTrack_LookAt, m_hFace );
		}
	}

	else if( m_hGuardNode )
	{
		AINodeGuard* pNode = (AINodeGuard*)g_pLTServer->HandleToObject( m_hGuardNode );
		if ( pNode->ShouldFaceNodeForward() )
		{
			GetAI()->FaceDir( pNode->GetForward() );
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, m_ePose);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
}


// ----------------------------------------------------------------------- //

bool CAIHumanStateTalk::HandleCommand(const CParsedMsg &cMsg)
{
	if ( super::HandleCommand(cMsg) )
	{
		return true;
	}

	m_eMood = CAnimationMgrList::GetPropFromName( cMsg.GetArg(0) );

	if(m_eMood != kAP_Invalid)
	{
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::SetFace(HOBJECT hFace)
{
	m_hFace = hFace; 
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::SetFaceTime(LTFLOAT fTime)
{
	m_fFaceTime = fTime;
	m_fFaceTimer = g_pLTServer->GetTime() + m_fFaceTime;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::SetGuardNode(HOBJECT hGuardNode)
{
	m_hGuardNode = hGuardNode; 
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eMood, EnumAnimProp);
	LOAD_HOBJECT(m_hFace);
	LOAD_TIME(m_fFaceTimer);
	LOAD_FLOAT(m_fFaceTime);
	LOAD_VECTOR(m_vInitialForward);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eMood);
	SAVE_HOBJECT(m_hFace);
	SAVE_TIME(m_fFaceTimer);
	SAVE_FLOAT(m_fFaceTime);
	SAVE_VECTOR(m_vInitialForward);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateCharge::CAIHumanStateCharge()
{
	m_bAttacking = LTFALSE;
	m_fAttackDistanceSqr = 750.f * 750.f;
	m_bYelled = LTFALSE;
	m_fYellDistanceSqr = 900.f * 900.f;
	m_fStopDistanceSqr = 200.f * 200.f;

	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
}

CAIHumanStateCharge::~CAIHumanStateCharge()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);

	if( m_pStrategyShoot )
	{
		AI_FACTORY_DELETE(m_pStrategyShoot);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCharge::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Only shoot while running if the brain allows it.

	if( GetAI()->GetBrain()->AttacksWhileMoving() )
	{
		m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
		if ( !m_pStrategyShoot->Init(pAIHuman) )
		{
			return LTFALSE;
		}
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	// Make sure we have a weapon

	if ( !GetAI()->GetCurrentWeapon() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	if( m_bFirstUpdate )
	{
		if( !GetAI()->IsPlayingDialogSound() )
		{
			GetAI()->PlayCombatSound(kAIS_Charge);
		}

		// Head and Torso tracking.
	
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
	}

	// Head and Torso tracking.
	
	GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );

	// Go into attack mode if we're too close or if we can't find a path to our target

	CAITarget* pTarget = GetAI()->GetTarget();
	HOBJECT hTarget = pTarget->GetObject();
	LTVector vTargetPos;

	if ( pTarget->GetCharacter()->HasLastVolume() )
	{
		vTargetPos = pTarget->GetCharacter()->GetLastVolumePos();
	}
	else {
		// COULD NOT SET PATH TO TARGET
		AITRACE( AIShowStates, ( GetAI()->m_hObject, "Cannot find path to target!" ) );
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

	LTFLOAT fTargetDistanceSqr = VEC_DISTSQR(vTargetPos, GetAI()->GetPosition());

	if( ( fTargetDistanceSqr < m_fStopDistanceSqr ) &&
		( !GetAI()->GetAIMovement()->IsMovementLocked() ) )
	{
		GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );

		// CLOSE NOW
		m_eStateStatus = kSStat_StateComplete;
		return;
	}
	else
	{
		if( !GetAI()->GetAIMovement()->IsMovementLocked() )
		{
			if( m_pStrategyFollowPath->IsUnset() || m_pStrategyFollowPath->IsDone() )
			{
				if ( !m_pStrategyFollowPath->Set(vTargetPos, LTFALSE) )
				{
					// COULD NOT SET PATH TO TARGET
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "Cannot find path to target!" ) );
					GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
					m_eStateStatus = kSStat_StateComplete;
					return;
				}
			}
			else if( m_pStrategyFollowPath->IsSet() )
			{
				// Check if we are within some radius of the dest.

				if( m_pStrategyFollowPath->GetDest().DistSqr( GetAI()->GetPosition() ) < ( 5.f * 5.f ) )
				{
					LTBOOL bDivergePaths = GetAI()->GetBrain()->GetAIDataExist( kAIData_DivergePaths ) && 
						( GetAI()->GetBrain()->GetAIData( kAIData_DivergePaths ) > 0.f );

					if ( !m_pStrategyFollowPath->Set(vTargetPos, bDivergePaths) )
					{
						// COULD NOT SET PATH TO TARGET
						AITRACE( AIShowStates, ( GetAI()->m_hObject, "Cannot find path to target!" ) );
						GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
						m_eStateStatus = kSStat_StateComplete;
						return;
					}
				}
			}
		}

		if( m_pStrategyFollowPath->IsSet() )
		{
			m_pStrategyFollowPath->Update();
		}
	}

	if ( !m_bAttacking )
	{
		// Go into attack mode if we should, otherwise yell if we should

		if ( m_pStrategyShoot && ( fTargetDistanceSqr < m_fAttackDistanceSqr ) )
		{
			m_bAttacking = LTTRUE;
		}
		else if ( !m_bYelled && (fTargetDistanceSqr < m_fYellDistanceSqr) )
		{
			m_bYelled = LTTRUE;
		}
	}

	// Shoot if we're attacking

	if ( m_pStrategyShoot )
	{
		// Shoot blind while running.

		m_pStrategyShoot->SetShootBlind( m_pStrategyFollowPath->IsSet() );

		if ( m_pStrategyShoot->IsFiring() )
		{
			// Be sure to update the strategy if we are in the middle of
			// firing as this means that we recieved a firing key this frame
			// which has not yet been processed
			m_pStrategyShoot->Update();
		}
		else if ( m_bAttacking )
		{
			// Cheat a reload if we should

			if ( m_pStrategyShoot->ShouldReload() )
			{
				m_pStrategyShoot->Reload(LTTRUE);
			}

			m_pStrategyShoot->Update();
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->UpdateAnimation();
	}
	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->Load(pMsg);
	}

	LOAD_BOOL(m_bAttacking);
	LOAD_FLOAT(m_fAttackDistanceSqr);
	LOAD_BOOL(m_bYelled);
	LOAD_FLOAT(m_fYellDistanceSqr);
	LOAD_BOOL(m_bStopped);
	LOAD_FLOAT(m_fStopDistanceSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	if( m_pStrategyShoot )
	{
		m_pStrategyShoot->Save(pMsg);
	}

	SAVE_BOOL(m_bAttacking);
	SAVE_FLOAT(m_fAttackDistanceSqr);
	SAVE_BOOL(m_bYelled);
	SAVE_FLOAT(m_fYellDistanceSqr);
	SAVE_BOOL(m_bStopped);
	SAVE_FLOAT(m_fStopDistanceSqr);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateAnimate::CAIHumanStateAnimate()
{
	m_pStrategyFlashlight = AI_FACTORY_NEW(CAIHumanStrategyFlashlight);

	m_bNoCinematics = LTFALSE;

	m_bLoop = LTFALSE;

	m_hstrAnim = LTNULL;

	m_bResetAnim = LTTRUE;
}

// ----------------------------------------------------------------------- //

CAIHumanStateAnimate::~CAIHumanStateAnimate()
{
	GetAI()->GetAnimationContext()->ClearSpecial();

	AI_FACTORY_DELETE(m_pStrategyFlashlight);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAnimate::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFlashlight->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::SetAnimation(HSTRING hstrAnim, LTBOOL bLoop)
{
	m_bLoop = bLoop;

	m_hstrAnim = hstrAnim;
	GetAI()->GetAnimationContext()->SetSpecial( g_pLTServer->GetStringData(m_hstrAnim) );

	m_eStateStatus = kSStat_Initialized;
	m_bResetAnim = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::SetAnimation(const CAnimationProps& animProps, LTBOOL bLoop)
{
	m_bLoop = bLoop;

	GetAI()->GetAnimationContext()->SetSpecial( animProps );

	m_eStateStatus = kSStat_Initialized;
	m_bResetAnim = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Update()
{
	super::Update();

	m_pStrategyFlashlight->Update();

	if ( m_bResetAnim )
	{
		if ( m_bLoop )
		{
			GetAI()->GetAnimationContext()->LoopSpecial();
		}
		else
		{
			GetAI()->GetAnimationContext()->LingerSpecial();
		}

		m_bResetAnim = LTFALSE;
	}

	if ( !m_bLoop && GetAI()->GetAnimationContext()->IsSpecialDone() )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}

}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFlashlight->Load(pMsg);

	LOAD_BOOL(m_bLoop);
	LOAD_HSTRING(m_hstrAnim);
	LOAD_BOOL(m_bResetAnim);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFlashlight->Save(pMsg);

	SAVE_BOOL(m_bLoop);
	SAVE_HSTRING(m_hstrAnim);
	SAVE_BOOL(m_bResetAnim);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateFollow::CAIHumanStateFollow()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_eStateStatus = kSStat_Following;

	m_fRangeTime = .25f;

	m_fTimer = 0.0f;

	m_fRangeSqr = 200.0f*200.0f;
}

// ----------------------------------------------------------------------- //

CAIHumanStateFollow::~CAIHumanStateFollow()
{
	if ( GetAI() )
	{
		GetAI()->SetHoverAcceleration( m_fBaseAIHoverAcceleration );
	}

	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateFollow::Init(CAIHuman* pAIHuman)
{
	if ( !CAIHumanState::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	if ( GetAI()->IsSwimming() )
	{
		m_pStrategyFollowPath->SetMovement(kAP_Swim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(kAP_Run);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	m_fRangeTimer = m_fRangeTime + g_pLTServer->GetTime();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}


	LTVector vTargetPosition;
    g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPosition);

	if ( !IsPlayer(GetAI()->GetTarget()->GetObject()) )
	{
		g_pLTServer->CPrint("NOT FOLLOWING PLAYER?!?!?!");
	}

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		// Head and Torso tracking.
	
		GetAI()->SetNodeTrackingTarget( kTrack_LookAt, GetAI()->GetTarget()->GetObject(), "Head" );

		if ( !m_pStrategyFollowPath->Set(vTargetPosition, LTFALSE) )
		{
			g_pLTServer->CPrint("Could not set path to target");
			
			// Head and Torso tracking.
			// Face target if at tracking limit.

			GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );
			return;
		}
	}

	if ( kSStat_Holding == m_eStateStatus )
	{
		if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) > (m_fRangeSqr+1000.0f) )
		{
			if ( g_pLTServer->GetTime() > m_fRangeTimer )
			{
			}
			else
			{
				// Head and Torso tracking.
				// Face target if at tracking limit.
	
				GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );
				return;
			}
		}
		else
		{
			m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;

			// Head and Torso tracking.
			// Face target if at tracking limit.

			GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );
			return;
		}
	}
	else if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < m_fRangeSqr )
	{
		m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;
		m_eStateStatus = kSStat_Holding;

		// Head and Torso tracking.
		// Face target if at tracking limit.

		GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );
		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_eStateStatus = kSStat_Following;

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		// Head and Torso tracking.

		GetAI()->EnableNodeTracking( kTrack_LookAt, LTNULL );
	}

    m_fTimer += g_pLTServer->GetFrameTime();

	if ( m_pStrategyFollowPath->IsDone() || m_fTimer > 1.0f )
	{
		m_fTimer = 0.0f;

		if ( !m_pStrategyFollowPath->Set(vTargetPosition, LTFALSE) )
		{
			m_eStateStatus = kSStat_Holding;

			// Head and Torso tracking.
			// Face target if at tracking limit.

			GetAI()->EnableNodeTracking( kTrack_LookAt, GetAI()->GetTarget()->GetObject() );
			return;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::HandleHolding()
//              
//	PURPOSE:	Updates the holding portion of the state.  AI will attempt to
//				stay within a certain range, moving to the follow substate
//				if too far away, and  waiting/facing who they are following
//				if close enough.
//              
//----------------------------------------------------------------------------
void CAIHumanStateFollow::HandleHolding()
	{
	// Get the perfect position of the target

	float f2DDistToTarget = Get2DDistance( GetTargetPosition() );

	// Check to see if this is a hovering AI.  A hovering AI may be drifting -- 
	// if it is, and if it has stopped, then clear out the destination it was
	// going to.
	if ( GetAI()->IsHovering() )
	{
		if ( m_pStrategyFollowPath->IsSet() )
		{
			if ( GetAI()->HoverIsDrifting() )
			{
				m_pStrategyFollowPath->Reset();
			}
			else
			{
				GetAI()->FaceTarget();
				return;
			}
		}

		if ( g_pLTServer->GetTime() < m_fRangeTimer )
		{
			m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;
			GetAI()->FaceTarget();
			return;
		}
	}
	else
	{
		m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;
		m_eStateStatus = kSStat_Holding;
		GetAI()->FaceTarget();
		return;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::HandleFollowing()
//              
//	PURPOSE:	Handles the following substate of the AI.  If close enough,
//				move to the hold state.  Otherwise move closer.  Resets on a
//				timer to keep the position the AI runs to current.
//              
//----------------------------------------------------------------------------
void CAIHumanStateFollow::HandleFollowing()
	{
	// If we are close enough, then move to the holding state stat.

	float f2DDistToTarget = Get2DDistance( GetTargetPosition() );

	// See if we should start Holding due to 'close enough'

	if ( f2DDistToTarget < m_fRangeSqr)
	{
		TransitionToHolding();
		return;
	}

	// See if we need to refresh our path.

	if (m_pStrategyFollowPath->IsUnset()	|| 
		m_pStrategyFollowPath->IsDone()		||
		m_fTimer > 1.0f)
	{
		m_fTimer = 0.0f;
		if ( !m_pStrategyFollowPath->Set(GetTargetPosition(), LTFALSE) )
		{
			// Unable to path to this point!  Start holding.
			g_pLTServer->CPrint("Could not set path to target");
			TransitionToHolding();
			return;
		}
	}

	// We have a path, so just update it and keep going.
	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_pStrategyFollowPath->Update();
		GetAI()->FaceTarget();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::TransitionToFollowing()
//              
//	PURPOSE:	Sets the state to Following, setting the acceleration to the 
//				AIs original
//              
//----------------------------------------------------------------------------
void CAIHumanStateFollow::TransitionToFollowing()
{
	if ( !IsPlayer( GetAI()->GetTarget()->GetObject()) )
	{
		g_pLTServer->CPrint("NOT FOLLOWING PLAYER?!?!?!");
	}

	GetAI()->SetHoverAcceleration( m_fBaseAIHoverAcceleration );

	m_eStateStatus = kSStat_Following;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::TransitionToHolding()
//              
//	PURPOSE:	Sets the state to Holding, clearing the path if not a hovering
//				character, and starting to decelerate if it is a hoveringing
//				character
//              
//----------------------------------------------------------------------------
void CAIHumanStateFollow::TransitionToHolding()
		{
			m_eStateStatus = kSStat_Holding;
			GetAI()->FaceTarget();
			return;
		}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::Get2DDistance()
//              
//	PURPOSE:	Returns the 2D distance between the passed in point and the AI
//              
//----------------------------------------------------------------------------
float CAIHumanStateFollow::Get2DDistance(const LTVector& vOtherPos) const
{
	LTVector v2DTargetPosition = vOtherPos;
	LTVector v2DAIPosition = GetAI()->GetPosition();
	v2DTargetPosition.y = v2DAIPosition.y = 0.0f;
	return ( v2DTargetPosition.DistSqr( v2DAIPosition ) );
	}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateFollow::GetTargetPosition()
//              
//	PURPOSE:	Returns the position of what the AI is following.  
//
//	NOTE:		Use to return the absolute position.  Currently returns the 
//				last visible position.  We MAY want to add a provision to see
//				if the Target SHOULD be followed even when unseen.
//              
//----------------------------------------------------------------------------
LTVector CAIHumanStateFollow::GetTargetPosition() const
{
	AIASSERT( GetAI()->HasTarget(), GetAI()->m_hObject, "GetTargetPositiong without a target" );
	return ( GetAI()->GetTarget()->GetVisiblePosition() );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::UpdateAnimation()
{
	CAIHumanState::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);

	if ( kSStat_Following == m_eStateStatus )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::SetMovement(EnumAnimProp eMovement)
{
	m_pStrategyFollowPath->SetMovement(eMovement);

	switch(eMovement)
	{
		case kAP_Walk:
		case kAP_Run:
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
			break;

		case kAP_Swim:
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Load(ILTMessage_Read *pMsg)
{
	CAIHumanState::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_TIME(m_fRangeTimer);
	LOAD_FLOAT(m_fRangeTime);
	LOAD_FLOAT(m_fTimer);
	LOAD_FLOAT(m_fRangeSqr);
	LOAD_FLOAT(m_fBaseAIHoverAcceleration);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Save(ILTMessage_Write *pMsg)
{
	CAIHumanState::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_TIME(m_fRangeTimer);
	SAVE_FLOAT(m_fRangeTime);
	SAVE_FLOAT(m_fTimer);
	SAVE_FLOAT(m_fRangeSqr);
	SAVE_FLOAT(m_fBaseAIHoverAcceleration);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

CAIHumanStateLongJump::CAIHumanStateLongJump()
{
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_bFired = LTFALSE;

	m_fLongJumpHeightMin = 5.f;
	m_fLongJumpHeightMax = 30.f;

	// AnimProp1 starts the jump.
	// AnimProp2 can be stretched to cover a variable distance.
	// AnimProp3 completes the jump.

	m_eAnimProp1 = kAP_None;
	m_eAnimProp2 = kAP_None;
	m_eAnimProp3 = kAP_None;

	m_iAnimRandomSeed = -1;

	m_bFaceDest = LTTRUE;

	m_fJumpSpeed = 0.f;

	m_bCountAttackers = LTFALSE;

	// Are we jumping to a target, or away.
	m_bJumpToTarget = LTTRUE;
	m_fLongJumpStopDist = 128.f;
}

// ----------------------------------------------------------------------- //

CAIHumanStateLongJump::~CAIHumanStateLongJump()
{
	AI_FACTORY_DELETE( m_pStrategyShoot );

	if( GetAI()->GetAIMovement()->IsMovementLocked() )
	{
		GetAnimationContext()->SetAnimRate( 1.f );
		GetAI()->GetAIMovement()->UnlockMovement();
	}

	CountAttackers( LTFALSE );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bFired);
	LOAD_BOOL(m_bJumpToTarget);
	LOAD_FLOAT(m_fLongJumpHeightMin);
	LOAD_FLOAT(m_fLongJumpHeightMax);
	LOAD_VECTOR(m_vLongJumpDest);
	LOAD_FLOAT(m_fLongJumpStopDist);
	LOAD_DWORD_CAST(m_eAnimProp1, EnumAnimProp);
	LOAD_DWORD_CAST(m_eAnimProp2, EnumAnimProp);
	LOAD_DWORD_CAST(m_eAnimProp3, EnumAnimProp);
	LOAD_DWORD(m_iAnimRandomSeed);
	LOAD_BOOL(m_bFaceDest);
	LOAD_FLOAT(m_fJumpSpeed);
	LOAD_BOOL(m_bCountAttackers);

	m_pStrategyShoot->Load(pMsg);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bFired);
	SAVE_BOOL(m_bJumpToTarget);
	SAVE_FLOAT(m_fLongJumpHeightMin);
	SAVE_FLOAT(m_fLongJumpHeightMax);
	SAVE_VECTOR(m_vLongJumpDest);
	SAVE_FLOAT(m_fLongJumpStopDist);
	SAVE_DWORD(m_eAnimProp1);
	SAVE_DWORD(m_eAnimProp2);
	SAVE_DWORD(m_eAnimProp3);
	SAVE_DWORD(m_iAnimRandomSeed);
	SAVE_BOOL(m_bFaceDest);
	SAVE_FLOAT(m_fJumpSpeed);
	SAVE_BOOL(m_bCountAttackers);

	m_pStrategyShoot->Save(pMsg);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateLongJump::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_fLongJumpHeightMin = pAIHuman->GetBrain()->GetAIData(kAIData_LongJumpHeightMin);
	m_fLongJumpHeightMax = pAIHuman->GetBrain()->GetAIData(kAIData_LongJumpHeightMax);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::CountAttackers(LTBOOL bCount)
{
	HOBJECT hAI = GetAI()->m_hObject;

	// Keep track of how many AI are attacking the same target.

	// Increment Attack counter.

	if( ( bCount && !m_bCountAttackers ) && GetAI()->HasTarget() )
	{
		m_bCountAttackers = LTTRUE;

		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI(), g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject()) ), hAI, "CAIHumanStateLongJump::CountAttackers: Already registered attacking count!" );
		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, GetAI(), g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject()), LTTRUE );
	}

	// Decrement Attack counter.

	else if( m_bCountAttackers && !bCount )
	{
		m_bCountAttackers = LTFALSE;

		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), hAI, "CAIHumanStateLongJump::CountAttackers: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::Update()
{
	super::Update();

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
			{
				// Make sure we have a target

				if ( !GetAI()->HasTarget() )
				{
					m_eStateStatus = kSStat_FailedComplete;
					return;
				}

				// Get a random seed to use when playing all 3 animations in the sequence.

				CAnimationProps Props;
				Props.Set(kAPG_Posture, kAP_Stand);
				Props.Set(kAPG_WeaponPosition, kAP_Up);
				Props.Set(kAPG_Movement, m_eAnimProp1);
				Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
				m_iAnimRandomSeed = GetAnimationContext()->ChooseRandomSeed( Props );

				// Setting a target push speed allows the AI to knock the player back,
				// if they collide while the AI is lunging.

				GetAI()->GetTarget()->SetPushSpeed( m_fJumpSpeed * 2.f );

				m_eStateStatus = kSStat_TakingOff;
			}
			break;

		case kSStat_TakingOff:
			if( !GetAnimationContext()->IsLocked() )
			{
				SetupLongJumpMovement();
				m_eStateStatus = kSStat_Moving;
			}
			break;

		case kSStat_Moving:

			// Do damage if weilding a melee weapon, and come in contact with target.

			if( (!m_bFired ) && GetAI()->HasWeapon( kAIWeap_Melee ) &&
				( GetAI()->GetBrain()->GetRangeStatus( kAIWeap_Melee ) == eRangeStatusOk ) )
			{				
				// AccuracyModifier is subtracted. Using 2 to make AI really
				// accurate when she slams into someone.

				GetAI()->SetCurrentWeapon( kAIWeap_Melee );
				GetAI()->SetAccuracyModifier(2.f, 1.f);
				m_pStrategyShoot->ForceFire( LTNULL );
				m_bFired = LTTRUE;
			}

			if( GetAI()->GetAIMovement()->IsDone() )
			{
				GetAnimationContext()->SetAnimRate( 1.f );
				m_eStateStatus = kSStat_Landing;
			}
			break;

		case kSStat_Landing:
			if( !GetAnimationContext()->IsLocked() )
			{
				GetAI()->GetAIMovement()->UnlockMovement();
				m_eStateStatus = kSStat_StateComplete;
			}
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyFireWeapon) )
	{
		if( GetAI()->HasWeapon( kAIWeap_Melee ) &&
			( GetAI()->GetBrain()->GetRangeStatus( kAIWeap_Melee ) == eRangeStatusOk ) )
		{
			// AccuracyModifier is subtracted. Using -2 to make AI really
			// accurate when she slams into someone.

			GetAI()->SetCurrentWeapon( kAIWeap_Melee );
			GetAI()->SetAccuracyModifier(2.f, 1.f);
			m_pStrategyShoot->ForceFire( LTNULL );
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::SetupLongJumpMovement()
{	
	LTVector vOrigin = GetAI()->GetPosition();

	// If we are jumping to a target, see if the target has moved, and
	// jump to a dest some offset from the target's actual pos.

	if( m_bJumpToTarget && GetAI()->HasTarget() )
	{
//		LTVector vTargetPos = GetAI()->GetTarget()->GetPosition();

		// TEMP FIX!!  This will keep characters from using the incorrect position
		// which Target->GetPosition returns.  The Target position is offset from the
		// origin of the character.  We want to jump at the characters origin, as that
		// is the only place we know is on the ground, safe, etc.  If the target
		// jumped, it might be problematic.
		// TODO: Handle this in a more consistant way!
		LTVector vTargetPos;
		g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPos);

		// Check if target has moved further away.

		if( vOrigin.DistSqr( m_vLongJumpDest ) < vOrigin.DistSqr( vTargetPos ) )
		{
			// Check if AI can take a straight line path to target's new position.
	
			if( g_pAIVolumeMgr->StraightRadiusPathExists( GetAI(),
														vOrigin, 
														vTargetPos, 
														GetAI()->GetRadius(), 
														GetAI()->GetVerticalThreshold(), 
														AIVolume::kVolumeType_Ladder | AIVolume::kVolumeType_JumpOver | AIVolume::kVolumeType_AmbientLife | AIVolume::kVolumeType_Teleport,
														GetAI()->GetLastVolume() ) )
			{
				// Ensure there are not AI in the way.

				if( !g_pCharacterMgr->RayIntersectAI( vOrigin, vTargetPos, GetAI(), LTNULL, LTNULL ) )
				{
					m_vLongJumpDest = vTargetPos;
				}
			}
		}

		// Find the location LongJumpStopDist from target.
	
		LTVector vDir = vOrigin - m_vLongJumpDest;
		vDir.Normalize();

		m_vLongJumpDest = m_vLongJumpDest + (vDir * m_fLongJumpStopDist);
	}

	// Vary the peak height of the parabola by distance covered.

	LTFLOAT fDist = vOrigin.Dist( m_vLongJumpDest );

	LTFLOAT fJumpHeight = 0.8f * fDist;
	if( fJumpHeight > m_fLongJumpHeightMax )
	{
		fJumpHeight = m_fLongJumpHeightMax;
	}
	else if( fJumpHeight < m_fLongJumpHeightMin )
	{
		fJumpHeight = m_fLongJumpHeightMin;
	}


	// Figure out how long it takes to fly to the dest.

	LTFLOAT fJumpTime = fDist / m_fJumpSpeed;

	// Find the length of the fly animation.

	CAnimationProps Props;
	Props.Set(kAPG_Posture, kAP_Stand);
	Props.Set(kAPG_WeaponPosition, kAP_Up);
	Props.Set(kAPG_Movement, m_eAnimProp2);
	Props.Set(kAPG_Weapon, GetAI()->GetCurrentWeaponProp());
	GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );
	LTFLOAT fAnimLength = GetAnimationContext()->GetAnimationLength( Props );

	// Calculate how fast to play the fly animation.

	LTFLOAT fAnimRate = fAnimLength / fJumpTime;

	// Setup the AI Movement.

	GetAI()->GetAIMovement()->SetMovementDest( m_vLongJumpDest );
	GetAI()->GetAIMovement()->FaceDest( m_bFaceDest );
	GetAI()->GetAIMovement()->SetSpeed( m_fJumpSpeed );
	GetAI()->GetAIMovement()->SetParabola( fJumpHeight );
	GetAI()->GetAIMovement()->IgnoreVolumes( LTTRUE );
	GetAI()->GetAIMovement()->LockMovement();

	GetAnimationContext()->SetAnimRate( fAnimRate );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateLongJump::SetStopDistance()
//              
//	PURPOSE:	Set the distance in front of the target/target location to
//				stop.  This must be a positive value, as we don't test
//				pathfinding for overshooting currently
//              
//----------------------------------------------------------------------------
void CAIHumanStateLongJump::SetStopDistance ( const float flDist )
{
	UBER_ASSERT( flDist >= 0, "Cannot set negitive stop distance" );
	m_fLongJumpStopDist = flDist;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLongJump::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch( m_eStateStatus )
	{
		case kSStat_TakingOff:
			GetAnimationContext()->SetProp(kAPG_Movement, m_eAnimProp1);
			GetAnimationContext()->Lock();
			break;

		case kSStat_Moving:
			{
				GetAnimationContext()->SetProp(kAPG_Movement, m_eAnimProp2);
			}
			break;

		case kSStat_Landing:
		case kSStat_StateComplete:
			{
				GetAnimationContext()->SetProp(kAPG_Movement, m_eAnimProp3);

				if( m_eStateStatus != kSStat_StateComplete )
				{
					GetAnimationContext()->Lock();
				}
			}
			break;
	}
}
