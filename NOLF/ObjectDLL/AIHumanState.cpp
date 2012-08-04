// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHuman.h"
#include "AIHumanState.h"
#include "AIHumanStrategy.h"
#include "AITarget.h"
#include "Alarm.h"
#include "Switch.h"
#include "AIGroup.h"
#include "CVarTrack.h"
#include "AIVolumeMgr.h"
#include "PlayerObj.h"
#include "Door.h"
#include "AINodeMgr.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "CharacterMgr.h"
#include "AIPath.h"
#include "AISense.h"
#include "WeaponItems.h"
#include "AIHelicopter.h"
#include "CharacterHitBox.h"
#include "CommandMgr.h"
#include "DebrisFuncs.h"
#include "ProjectileTypes.h"
#include "AIRegion.h"
#include "AIRegionMgr.h"
#include "AIPathMgr.h"

using namespace AnimationsHuman;

IMPLEMENT_FACTORY(CAIHumanStateAnimate, 0)
IMPLEMENT_FACTORY(CAIHumanStateAware, 0)
IMPLEMENT_FACTORY(CAIHumanStateLookAt, 0)
IMPLEMENT_FACTORY(CAIHumanStateAssassinate, 0)
IMPLEMENT_FACTORY(CAIHumanStateDraw, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttack, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttackFromCover, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttackFromVantage, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttackFromView, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttackOnSight, 0)
IMPLEMENT_FACTORY(CAIHumanStateAttackProp, 0)
IMPLEMENT_FACTORY(CAIHumanStateCharge, 0)
IMPLEMENT_FACTORY(CAIHumanStateChase, 0)
IMPLEMENT_FACTORY(CAIHumanStateCheckBody, 0)
IMPLEMENT_FACTORY(CAIHumanStateInvestigate, 0)
//IMPLEMENT_FACTORY(CAIHumanStateCome, 0)
IMPLEMENT_FACTORY(CAIHumanStateCover, 0)
IMPLEMENT_FACTORY(CAIHumanStateDistress, 0)
IMPLEMENT_FACTORY(CAIHumanStateDrowsy, 0)
IMPLEMENT_FACTORY(CAIHumanStateFollow, 0)
IMPLEMENT_FACTORY(CAIHumanStateHeliAttack, 0)
IMPLEMENT_FACTORY(CAIHumanStateIdle, 0)
IMPLEMENT_FACTORY(CAIHumanStateFollowFootprint, 0)
IMPLEMENT_FACTORY(CAIHumanStateGetBackup, 0)
IMPLEMENT_FACTORY(CAIHumanStateGoto, 0)
IMPLEMENT_FACTORY(CAIHumanStateFlee, 0)
IMPLEMENT_FACTORY(CAIHumanStatePanic, 0)
IMPLEMENT_FACTORY(CAIHumanStateParaDie, 0)
IMPLEMENT_FACTORY(CAIHumanStateParaDive, 0)
IMPLEMENT_FACTORY(CAIHumanStateParaEscape, 0)
IMPLEMENT_FACTORY(CAIHumanStateParaShoot, 0)
IMPLEMENT_FACTORY(CAIHumanStatePatrol, 0)
IMPLEMENT_FACTORY(CAIHumanStatePickupObject, 0)
IMPLEMENT_FACTORY(CAIHumanStateSearch, 0)
IMPLEMENT_FACTORY(CAIHumanStateTail, 0)
IMPLEMENT_FACTORY(CAIHumanStateTalk, 0)
IMPLEMENT_FACTORY(CAIHumanStateUnconscious, 0)
IMPLEMENT_FACTORY(CAIHumanStateStunned, 0)
IMPLEMENT_FACTORY(CAIHumanStateUseObject, 0)

IMPLEMENT_FACTORY(CAIHumanStateScotBox, 0)
IMPLEMENT_FACTORY(CAIHumanStateIngeSing, 0)

// ----------------------------------------------------------------------- //

void CAIHumanState::Constructor()
{
	super::Constructor();

	m_pAIHuman = LTNULL;

	m_pStrategyFollowPath	= LTNULL;
	m_pStrategyDodge		= LTNULL;
	m_pStrategyCover		= LTNULL;
	m_pStrategyShoot		= LTNULL;
	m_pStrategyGrenade		= LTNULL;
	m_pStrategyOneShotAni	= LTNULL;
	m_pStrategyFlashlight	= LTNULL;

	m_bInterrupt = LTTRUE;

	memset(m_abCanChangeToState, LTTRUE, sizeof(LTBOOL)*kNumStateTypes);

	m_ePose = ePoseDefault;
}

void CAIHumanState::Destructor()
{
	super::Destructor();
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

	if ( ePoseDefault == m_ePose )
	{
		if ( GetAI()->GetAnimationContext()->IsPropSet(aniSit) )
		{
			m_ePose = ePoseSit;
		}
		else
		{
			m_ePose = ePoseStand;
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

void CAIHumanState::UpdateSenses()
{
	if ( m_bInterrupt )
	{
		static SenseType astSenses[] =
		{
			stSeeEnemy,
			stSeeEnemyFootprint,
			stSeeEnemyFlashlight,
			stHearEnemyWeaponFire,
			stHearAllyWeaponFire,
			stHearEnemyWeaponImpact,
			stHearEnemyFootstep,
			stHearEnemyDisturbance,
			stSeeAllyDeath,
			stHearAllyDeath,
			stHearAllyPain
		};

		static int cSenses = sizeof(astSenses)/sizeof(SenseType);

		for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
		{
			GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::UpdateAnimation()
{
	super::UpdateAnimation();

	// This is a hack... do it here too since this can happen before our preupdate

	if ( ePoseDefault == m_ePose )
	{
		if ( GetAI()->GetAnimationContext()->IsPropSet(aniSit) )
		{
			m_ePose = ePoseSit;
		}
		else
		{
			m_ePose = ePoseStand;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "POSE") )
	{
		if ( !_stricmp(szValue, "STAND") ||
			 !_stricmp(szValue, "LEAN") ) // support LEAN for backwards compatibility
		{
			m_ePose = ePoseStand;
		}
		else if ( !_stricmp(szValue, "SIT") ||
				  !_stricmp(szValue, "CHAIR") ) // support CHAIR for backwards compatibility
		{
			m_ePose = ePoseSit;
		}
	}
	else if ( !_stricmp(szName, "INTERRUPT") )
	{
		m_bInterrupt = IsTrueChar(szValue[0]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleBrokenLink(hObject);
	}

	if ( m_pStrategyFlashlight )
	{
		m_pStrategyFlashlight->HandleBrokenLink(hObject);
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
}

// ----------------------------------------------------------------------- //

void CAIHumanState::SearchOr(const char* szOr)
{
	if ( GetAI()->HasLastVolume() )
	{
		if ( GetAI()->GetLastVolume()->HasRegion() )
		{
			CAIRegion* pRegion = GetAI()->GetLastVolume()->GetRegion();
			if ( pRegion->IsSearchable() )
			{
				GetAI()->ChangeState("SEARCH PAUSE=TRUE");
			}
		}
	}

	GetAI()->ChangeState(szOr);
}

// ----------------------------------------------------------------------- //

void CAIHumanState::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_ePose);
	SAVE_BOOL(m_bInterrupt);

	for ( uint32 iState = 0 ; iState < kNumStateTypes ; iState++ )
	{
		SAVE_BOOL(m_abCanChangeToState[iState]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanState::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_ePose, Pose);
	LOAD_BOOL(m_bInterrupt);

	for ( uint32 iState = 0 ; iState < kNumStateTypes ; iState++ )
	{
		LOAD_BOOL(m_abCanChangeToState[iState]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIdle::Constructor()
{
	super::Constructor();

	m_bNoCinematics = LTFALSE;
}

void CAIHumanStateIdle::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

HSTRING CAIHumanStateIdle::CreateReturnString()
{
	char szBuffer[256];
	sprintf(szBuffer, "GOTO PT=%f,%f,%f NEXT=(FACEDIR %f,%f,%f) NEXT=IDLE MOVE=WALK", EXPANDVEC(GetAI()->GetPosition()), EXPANDVEC(GetAI()->GetForwardVector()));

    return g_pLTServer->CreateString(szBuffer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIdle::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_ePose )
	{
		case ePoseSit:
			GetAnimationContext()->SetProp(aniSit);
			break;

		case ePoseStand:
			GetAnimationContext()->SetProp(aniStand);
			break;

		default:
			_ASSERT(LTFALSE);
			break;
	}

	GetAnimationContext()->SetProp(aniDown);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Constructor()
{
	super::Constructor();

	static const CAnimationProp	s_aaniAlerts[] =
	{
		aniAlert1, aniAlert2, aniAlert3
	};
	static const uint32 s_cAlerts = sizeof(s_aaniAlerts)/sizeof(CAnimationProp);

	m_aniAlert = s_aaniAlerts[GetRandom(0, s_cAlerts-1)];

	m_bNoCinematics = LTFALSE;

	// m_bAlert = LTTRUE;
}

void CAIHumanStateAware::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_aniAlert.Load(hRead);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_aniAlert.Save(hWrite);
}

// ----------------------------------------------------------------------- //

HSTRING CAIHumanStateAware::CreateReturnString()
{
	char szBuffer[256];
	sprintf(szBuffer, "GOTO PT=%f,%f,%f NEXT=(FACEDIR %f,%f,%f) NEXT=(AWARE) MOVE=WALK",
		EXPANDVEC(GetAI()->GetPosition()),
		EXPANDVEC(GetAI()->GetForwardVector()));

    return g_pLTServer->CreateString(szBuffer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::Update()
{
	super::Update();

	if ( m_bPlayFirstSound && !GetAI()->IsControlledByCinematicTrigger() )
	{
		GetAI()->PlaySound(aisDisturb);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAware::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);
	GetAnimationContext()->SetProp(m_aniAlert);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Constructor()
{
	super::Constructor();

	m_aniLook = aniLookLeft;

	// TODO: should this state interrupt cinematics?
	m_bNoCinematics = LTFALSE;

	// m_bAlert = LTTRUE;
}

void CAIHumanStateLookAt::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_aniLook.Load(hRead);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_aniLook.Save(hWrite);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "POSITION") )
	{
		LTVector vPos;
		sscanf(szValue, "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
		LTVector vDir = vPos - GetAI()->GetPosition();

		if ( vDir.Dot(GetAI()->GetRightVector()) > 0.0f )
		{
			m_aniLook = aniLookRight;
		}
		else
		{
			m_aniLook = aniLookLeft;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::Update()
{
	super::Update();

	if ( m_bPlayFirstSound && !GetAI()->IsControlledByCinematicTrigger() )
	{
		GetAI()->PlaySound(aisDisturb);
	}

	if ( !m_bFirstUpdate && !GetAnimationContext()->IsLocked() )
	{
		ReturnOr("IDLE");

		if ( !GetAI()->IsControlledByCinematicTrigger() )
		{
			GetAI()->PlaySound(aisGaveUp);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLookAt::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(m_aniLook);
	GetAnimationContext()->Lock();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Constructor()
{
	super::Constructor();

	m_eState = eStateAwake;
}

void CAIHumanStateDrowsy::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_eState);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		switch ( m_eState )
		{
			case eStateAwake:
				Asleep();
				break;
			case eStateAsleep:
				Awake();
				break;
		}
	}

	switch ( m_eState )
	{
		case eStateAwake:
		{
		}
		break;

		case eStateAsleep:
		{
			if ( !GetAI()->IsPlayingDialogSound() )
			{
				// TODO: sound
				//GetAI()->PlaySound(GetDrowsyAsleepSound(GetAI()));
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniDrowsy);

	switch ( m_ePose )
	{
		case ePoseSit:
			GetAnimationContext()->SetProp(aniSit);
			break;

		case ePoseStand:
			GetAnimationContext()->SetProp(aniStand);
			break;
	}

	switch ( m_eState )
	{
		case eStateAwake:
			GetAnimationContext()->SetProp(aniAwake);
			break;

		case eStateAsleep:
			GetAnimationContext()->SetProp(aniAsleep);
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::UpdateSenses()
{
	switch ( m_eState )
	{
		case eStateAwake:
			super::UpdateSenses();
			break;

		case eStateAsleep:
			GetAI()->GetSenseMgr()->UpdateSense(stHearEnemyWeaponFire);
			GetAI()->GetSenseMgr()->UpdateSense(stHearEnemyWeaponImpact);
			GetAI()->GetSenseMgr()->UpdateSense(stHearAllyPain);
			GetAI()->GetSenseMgr()->UpdateSense(stHearAllyWeaponFire);
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Awake()
{
	// TODO: sound
	//GetAI()->PlaySound(GetDrowsyAwakeSound(GetAI()));

	m_eState = eStateAwake;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::Asleep()
{
// TODO: sound
//	GetAI()->PlaySound(GetDrowsyAsleepSound(GetAI()));

	m_eState = eStateAsleep;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateDrowsy::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "AWAKE") )
	{
		Awake();

		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "ASLEEP") )
	{
		Asleep();

		return LTTRUE;
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDrowsy::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

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

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::Constructor()
{
	super::Constructor();

	m_eState = eStateUnconscious;
	m_bAware = LTTRUE;
	m_fUnconsciousTime = 15.0f;
}

void CAIHumanStateUnconscious::Destructor()
{
	super::Destructor();

	GetAI()->SetClientSolid(LTTRUE);
	GetAI()->SetBlinking(LTTRUE);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateUnconscious::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_fRegainConsciousnessTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_fUnconsciousTime);

	GetAI()->SetClientSolid(LTFALSE);
	GetAI()->SetBlinking(LTFALSE);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	// TODO: if someone shoots us, attack them when we wake up.

	if ( damage.eType == DT_SLEEPING )
	{
		m_fRegainConsciousnessTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_fUnconsciousTime);
		m_eState = eStateUnconscious;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "AWARE") )
	{
		m_bAware = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "TIME") )
	{
		m_fUnconsciousTime = (LTFLOAT)atof(szValue);
		m_fRegainConsciousnessTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_fUnconsciousTime);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->KillDlgSnd();
	}

	switch ( m_eState )
	{
		case eStateConscious:
		{
			if ( m_bAware )
			{
				SearchOr("AWARE");
			}
			else
			{
				GetAI()->ChangeState("IDLE");
			}
		}
		break;

		case eStateRegainingConsciousness:
		{
			if ( GetAnimationContext()->IsPropSet(aniStand) )
			{
				m_eState = eStateConscious;
			}
		}
		break;

		case eStateUnconscious:
		{
			if ( g_pLTServer->GetTime() > m_fRegainConsciousnessTime )
			{
				m_eState = eStateRegainingConsciousness;
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_eState )
	{
		case eStateUnconscious:
			GetAI()->GetAnimationContext()->SetProp(aniProne);
			GetAI()->GetAnimationContext()->SetProp(aniUnconscious);
			break;

		case eStateRegainingConsciousness:
		case eStateConscious:
			GetAI()->GetAnimationContext()->SetProp(aniStand);
			GetAI()->GetAnimationContext()->SetProp(aniDown);
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_FLOAT(m_fRegainConsciousnessTime);
	LOAD_BOOL(m_bAware);
	LOAD_FLOAT(m_fUnconsciousTime);
}

// ----------------------------------------------------------------------- //

HMODELANIM CAIHumanStateUnconscious::GetDeathAni(LTBOOL bFront)
{
	if ( !GetAnimationContext() || !GetAnimationContext()->IsPropSet(aniUnconscious) )
	{
		return INVALID_MODEL_ANIM;
	}
	else
	{
		return g_pLTServer->GetAnimIndex(GetAI()->GetObject(), "PrUn");
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUnconscious::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fRegainConsciousnessTime);
	SAVE_BOOL(m_bAware);
	SAVE_FLOAT(m_fUnconsciousTime);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateStunned::Constructor()
{
	super::Constructor();

	m_fYellTimer = 0.0f;
}

void CAIHumanStateStunned::Destructor()
{
	super::Destructor();

	GetAI()->SetBlinking(LTTRUE);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateStunned::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	GetAI()->SetBlinking(LTFALSE);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateStunned::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_FLOAT(m_fYellTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateStunned::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_FLOAT(m_fYellTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateStunned::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->KillDlgSnd();
	}

	if ( m_fElapsedTime > LOWER_BY_DIFFICULTY(15.0f) )
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}

	m_fYellTimer -= g_pLTServer->GetFrameTime();

	if ( m_fYellTimer <= 0.0f )
	{
		// TODO: Register last pain time on Character!!!!
		// $SOUND GetAI()->PlaySound(aisPain);
		m_fYellTimer = GetRandom(2.0f, 4.0f);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateStunned::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAI()->GetAnimationContext()->SetProp(aniStand);
	GetAI()->GetAnimationContext()->SetProp(aniStunned);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Constructor()
{
	super::Constructor();

	m_bNoCinematics = LTFALSE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_fWaitTimer = 3.0f + GetRandom(0.0f, 3.0f);

	m_bForward = LTTRUE;
	m_cNodes = 0;
	m_iNextNode = 0;

	m_bFace = LTTRUE;
	m_bLoop = LTTRUE;
	m_bCircle = LTFALSE;

	m_eTask = eTaskWait;
}

void CAIHumanStatePatrol::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePatrol::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);
	m_pStrategyFollowPath->SetMovementModifier(aniPatrol);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

CAINode* CAIHumanStatePatrol::SafeGetPatrolNode(int32 iPatrolNode)
{
	if ( iPatrolNode >= 0 && iPatrolNode < m_cNodes )
	{
		uint32 dwNode = m_adwNodes[iPatrolNode];
		if ( dwNode != CAINode::kInvalidNodeID )
		{
			CAINode* pNode = g_pAINodeMgr->GetNode(dwNode);
			if ( pNode )
			{
				return pNode;
			}
		}
	}

	return LTNULL;
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

	if ( 0 == m_cNodes || 1 == m_cNodes )
	{
        g_pLTServer->CPrint("AI had no patrol nodes.");
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_fTalkTimer < 0.0f )
	{
		LTFLOAT fRandom = GetRandom(0.0f, 1.0f);
		if ( fRandom < GetAI()->GetBrain()->GetPatrolSoundChance() )
		{
			GetAI()->PlaySound(aisPatrol);
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
		if ( !m_bLoop && m_iNextNode == -1 )
		{
			NextOr("IDLE");
			return;
		}

		if ( m_bFace )
		{
			CAINode* pNode = SafeGetPatrolNode(m_iDirNode);
			if ( pNode )
			{
				GetAI()->FaceDir(pNode->GetForward());
			}
		}

		switch ( m_eTask )
		{
			case eTaskWait:
				bGotoNextNode = UpdateTaskWait();
				break;
			case eTaskAnimate:
				bGotoNextNode = UpdateTaskAnimate();
				break;
		}
	}
	else if ( m_pStrategyFollowPath->IsUnset() )
	{
		bGotoNextNode = LTTRUE;
	}

	if ( bGotoNextNode )
	{
		// Reset the wait timer

		m_fWaitTimer = 3.0f + GetRandom(0.0f, 3.0f);

		int iNode = m_iNextNode;

		if ( m_bForward )
		{
			m_iNextNode++;
		}
		else
		{
			m_iNextNode--;
		}

		if ( m_iNextNode < 0 )
		{
			ASSERT(m_bLoop && !m_bCircle);
			m_bForward = LTTRUE;
			m_iNextNode = 1;
		}
		else if ( m_iNextNode >= m_cNodes )
		{
			if ( m_bLoop )
			{
				if ( m_bCircle )
				{
					m_iNextNode = 0;
				}
				else
				{
					m_iNextNode = m_cNodes-2;
					m_bForward = LTFALSE;
				}
			}
			else
			{
				m_iNextNode = -1;
			}
		}

		// Record what direction to face

		m_iDirNode = iNode;

		// Get the node

		CAINode *pNode = SafeGetPatrolNode(iNode);
		_ASSERT(pNode);

		if ( !m_pStrategyFollowPath->Set(pNode) )
		{
			GetAI()->ChangeState("IDLE");
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

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_eTask == eTaskAnimate )
		{
			GetAnimationContext()->SetProp(aniLower);
			GetAnimationContext()->SetProp(m_aniTask);
			GetAnimationContext()->Lock();
		}
	}
	else
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

void CAIHumanStatePatrol::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PTS") )
	{
		m_cNodes = 0;

		char *szPoint = strtok(szValue, ",");
		while ( szPoint )
		{
			if ( m_cNodes == kMaxPatrolNodes )
			{
                g_pLTServer->CPrint("Max # patrol waypoints exceeded %s=%s", szName, szValue);
			}

			CAINode* pNode = g_pAINodeMgr->GetNode(szPoint);

			if ( pNode )
			{
				m_adwNodes[m_cNodes++] = pNode->GetID();
			}
			else
			{
                g_pLTServer->CPrint("Unknown patrol waypoint ''%s''", szPoint);
			}

			szPoint = strtok(NULL, ",");
		}
	}
	else if ( !_stricmp(szName, "FACE") )
	{
		m_bFace = IsTrueChar(szValue[0]);
	}
	else if ( !_stricmp(szName, "TASK") )
	{
		if ( !_stricmp(szValue, "WAIT") )
		{
			m_eTask = eTaskWait;
		}
		else if ( !_stricmp(szValue, "CLIPBOARD") )
		{
			m_eTask = eTaskAnimate;
			m_aniTask = aniClipboard;
		}
		else if ( !_stricmp(szValue, "DUST") )
		{
			m_eTask = eTaskAnimate;
			m_aniTask = aniDust;
		}
		else if ( !_stricmp(szValue, "SWEEP") )
		{
			m_eTask = eTaskAnimate;
			m_aniTask = aniSweep;
		}
		else if ( !_stricmp(szValue, "WIPE") )
		{
			m_eTask = eTaskAnimate;
			m_aniTask = aniWipe;
		}
		else if ( !_stricmp(szValue, "TICKET") )
		{
			m_eTask = eTaskAnimate;
			m_aniTask = aniTicket;
		}
	}
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "CIRCLE") )
	{
		m_bCircle = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_FLOAT(m_fWaitTimer);
	LOAD_FLOAT(m_fTalkTimer);

	LOAD_BOOL(m_bForward);
	LOAD_DWORD(m_cNodes);
	LOAD_DWORD(m_iNextNode);
	LOAD_DWORD(m_iDirNode);

    int iNode;
    for ( iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		LOAD_DWORD(m_adwNodes[iNode]);
	}

	for ( iNode = m_cNodes ; iNode < kMaxPatrolNodes ; iNode++ )
	{
		m_adwNodes[iNode] = CAINode::kInvalidNodeID;
	}

	LOAD_BOOL(m_bFace);
	LOAD_DWORD_CAST(m_eTask, Task);
	LOAD_BOOL(m_bLoop);
	LOAD_BOOL(m_bCircle);

	m_aniTask.Load(hRead);
}


// ----------------------------------------------------------------------- //

void CAIHumanStatePatrol::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_FLOAT(m_fWaitTimer);
	SAVE_FLOAT(m_fTalkTimer);

	SAVE_BOOL(m_bForward);
	SAVE_DWORD(m_cNodes);
	SAVE_DWORD(m_iNextNode);
	SAVE_DWORD(m_iDirNode);

	for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		SAVE_DWORD(m_adwNodes[iNode]);
	}

	SAVE_BOOL(m_bFace);
	SAVE_DWORD(m_eTask);
	SAVE_BOOL(m_bLoop);
	SAVE_BOOL(m_bCircle);

	m_aniTask.Save(hWrite);
}

// ----------------------------------------------------------------------- //

HSTRING CAIHumanStatePatrol::CreateReturnString()
{
	_ASSERT(m_cNodes > 0);

	char szBuffer[1024];
	sprintf(szBuffer, "PATROL PTS=");

	for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
        strcat(szBuffer, g_pLTServer->GetStringData(g_pAINodeMgr->GetNode(m_adwNodes[iNode])->GetName()));
		strcat(szBuffer, ",");
	}

	szBuffer[strlen(szBuffer)-1] = 0;

    return g_pLTServer->CreateString(szBuffer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Constructor()
{
	super::Constructor();

	m_bNoCinematics = LTFALSE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_bFace = LTTRUE;
	m_vDest = LTVector(0,0,0);
	m_cNodes = 0;
	m_iNextNode = 0;
	m_bLoop = LTFALSE;
}

void CAIHumanStateGoto::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

HSTRING CAIHumanStateGoto::CreateReturnString()
{
	char szBuffer[1024];

	if ( m_cNodes != 0 )
	{
		sprintf(szBuffer, "GOTO PTS=");

		for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
		{
			CAINode* pNode = SafeGetGotoNode(m_iNextNode-1);

			if ( !pNode ) continue;

			if ( iNode != 0 )
			{
				strcat(szBuffer, ",");
			}

			strcat(szBuffer, g_pLTServer->GetStringData(pNode->GetName()));
		}
	}
	else
	{
		sprintf(szBuffer, "GOTO PT=%f,%f,%f", EXPANDVEC(m_vDest));
	}

	strcat(szBuffer, " LOOP=");
	strcat(szBuffer, m_bLoop ? "TRUE" : "FALSE");
	strcat(szBuffer, " FACE=");
	strcat(szBuffer, m_bFace ? "TRUE" : "FALSE");
	strcat(szBuffer, " MOVE=");

	CAnimationProp ani = m_pStrategyFollowPath->GetMovement();
	if ( ani == aniSwim )
	{
		strcat(szBuffer, "SWIM");
	}
	else if ( ani == aniWalk )
	{
		strcat(szBuffer, "WALK");
	}
	else // if ( ani == aniRun )
	{
		strcat(szBuffer, "RUN");
	}

    return g_pLTServer->CreateString(szBuffer);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateGoto::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Set up our default movement speed

	if ( GetAI()->IsScuba() )
	{
		m_pStrategyFollowPath->SetMovement(aniSwim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(aniWalk);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

CAINode* CAIHumanStateGoto::SafeGetGotoNode(int32 iGotoNode)
{
	if ( iGotoNode >= 0 && iGotoNode < m_cNodes )
	{
		uint32 dwNode = m_adwNodes[iGotoNode];
		if ( dwNode != CAINode::kInvalidNodeID )
		{
			CAINode* pNode = g_pAINodeMgr->GetNode(dwNode);
			if ( pNode )
			{
				return pNode;
			}
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Update()
{
	super::Update();

	if ( !m_pStrategyFollowPath->IsSet() )
	{
		if ( m_cNodes == 0 )
		{
			// If we're just going to a point...

			if ( m_pStrategyFollowPath->IsDone() )
			{
				// We successfully got there - do our "Next"

				NextOr("IDLE");
				return;
			}
			else // if ( m_pStrategyFollowPath->IsUnset() )
			{
				if ( !m_pStrategyFollowPath->Set(m_vDest) )
				{
					GetAI()->ChangeState("IDLE");
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
					// We successfully got there - do our "Next"

					if ( m_bFace )
					{
						CAINode* pNode = SafeGetGotoNode(m_cNodes-1);
						if ( pNode )
						{
							GetAI()->FaceDir(pNode->GetForward());
						}
					}

					NextOr("IDLE");
					return;
				}
			}

			// Advance the next node

			m_iNextNode++;

			// Get the node

			CAINode *pNode = SafeGetGotoNode(iNode);

			if ( !pNode || !m_pStrategyFollowPath->Set(pNode) )
			{
				GetAI()->ChangeState("IDLE");
				return;
			}
		}
	}

	// TODO: check for strategy failure
	m_pStrategyFollowPath->Update();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PT") )
	{
		sscanf(szValue, "%f,%f,%f", &m_vDest.x, &m_vDest.y, &m_vDest.z);
		m_cNodes = 0;
	}
	else if ( !_stricmp(szName, "PTS") )
	{
		m_cNodes = 0;

		char *szPoint = strtok(szValue, ",");
		while ( szPoint )
		{
			if ( m_cNodes == kMaxGotoNodes )
			{
                g_pLTServer->CPrint("Max # Goto waypoints exceeded %s=%s", szName, szValue);
			}

			CAINode* pNode = g_pAINodeMgr->GetNode(szPoint);

			if ( pNode )
			{
				m_adwNodes[m_cNodes++] = pNode->GetID();
			}
			else
			{
                g_pLTServer->CPrint("Unknown Goto waypoint ''%s''", szPoint);
			}

			szPoint = strtok(NULL, ",");
		}
	}
	if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
		}
		else if ( !_stricmp(szMove, "SWIM") )
		{
			m_pStrategyFollowPath->SetMovement(aniSwim);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
		}
	}
	else if ( !_stricmp(szName, "FACE") )
	{
		m_bFace = IsTrueChar(szValue[0]);
	}
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_BOOL(m_bFace);
	LOAD_VECTOR(m_vDest);
	LOAD_DWORD(m_cNodes);
	LOAD_DWORD(m_iNextNode);
	LOAD_BOOL(m_bLoop);

    int iNode;
    for ( iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		LOAD_DWORD(m_adwNodes[iNode]);
	}

	for ( iNode = m_cNodes ; iNode < kMaxGotoNodes ; iNode++ )
	{
		m_adwNodes[iNode] = CAINode::kInvalidNodeID;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGoto::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_BOOL(m_bFace);
	SAVE_VECTOR(m_vDest);
	SAVE_DWORD(m_cNodes);
	SAVE_DWORD(m_iNextNode);
	SAVE_BOOL(m_bLoop);

	for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		SAVE_DWORD(m_adwNodes[iNode]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Constructor()
{
	super::Constructor();

	m_hDanger = LTNULL;
}

void CAIHumanStateFlee::Destructor()
{
	GetAI()->Unlink(m_hDanger);

	super::Destructor();
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

		NextOr("IDLE");
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniUp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DANGER") )
	{
		GetAI()->Unlink(m_hDanger);
		m_hDanger = LTNULL;

		if ( LT_OK != FindNamedObject(szValue, m_hDanger) )
		{
            g_pLTServer->CPrint("FLEE DANGER=%s -- this object does not exist!", szValue);
			return;
		}

		GetAI()->Link(m_hDanger);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hDanger )
	{
		m_hDanger = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_HOBJECT(m_hDanger);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFlee::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_HOBJECT(m_hDanger);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyFlashlight = FACTORY_NEW(CAIHumanStrategyFlashlight);

	m_bSearching = LTFALSE;
	m_iSearchNode = CAINode::kInvalidNodeID;
	m_bFace = LTTRUE;
	m_bEngage = LTFALSE;
	m_bDone = LTFALSE;
	m_bAdded = LTFALSE;
	m_iSearchRegion = CAIRegion::kInvalidRegion;
	m_bPause = LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Destructor()
{
	g_pAINodeMgr->UnlockNode(m_iSearchNode);

	if ( m_bAdded )
	{
		CAIRegion* pRegion = g_pAIRegionMgr->GetRegionByIndex(m_iSearchRegion);
		if (pRegion)
		{
			pRegion->RemoveSearcher(GetAI());
		}
	}

	FACTORY_DELETE(m_pStrategyFlashlight);
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSearch::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFlashlight->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

const CAnimationProp& CAIHumanStateSearch::GetRandomSearch(CAINode* pNode) const
{
	int nAvailableSearchs = 0;
	uint32 adwSearchs[128];
	uint32 dwSearchFlags = pNode->GetSearchFlags();

	for ( uint32 iSearch = 0 ; iSearch < CAINode::kNumSearchFlags ; iSearch++ )
	{
		if ( dwSearchFlags & (1 << iSearch) )
		{
			adwSearchs[nAvailableSearchs++] = (1 << iSearch);
		}
	}

	if ( 0 == nAvailableSearchs )
	{
		_ASSERT(LTFALSE);
        g_pLTServer->CPrint("GetRandomSearch - couldn't get random search action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
		return aniAlert1;
	}

	uint32 dwSearch = adwSearchs[GetRandom(0, nAvailableSearchs-1)];

	if ( dwSearch == CAINode::kSearchFlagShineFlashlight )	return aniFlashlight;
	if ( dwSearch == CAINode::kSearchFlagLookUnder )		return aniLookUnder;
	if ( dwSearch == CAINode::kSearchFlagLookOver )			return aniLookOver;
	if ( dwSearch == CAINode::kSearchFlagLookLeft )			return aniLookLeft;
	if ( dwSearch == CAINode::kSearchFlagLookRight )		return aniLookRight;
	if ( dwSearch == CAINode::kSearchFlagAlert1 )			return aniAlert1;
	if ( dwSearch == CAINode::kSearchFlagAlert2 )			return aniAlert2;
	if ( dwSearch == CAINode::kSearchFlagAlert3 )			return aniAlert3;

	_ASSERT(LTFALSE);
    g_pLTServer->CPrint("GetRandomSearch - couldn't get search action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
	return aniAlert1;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Update()
{
	super::Update();

	m_pStrategyFlashlight->Update();

	if ( m_bDone || !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("AWARE");
		return;
	}

	// Handle our pause if we had to do one

	if ( m_bFirstUpdate && m_bPause )
	{
		return;
	}
	else if ( m_bPause )
	{
		if ( GetAnimationContext()->IsLocked() )
		{
			return;
		}
		else
		{
			m_bPause = LTFALSE;
		}
	}

	// Update our searching

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( m_bEngage )
		{
			// If we're engaging, we have to run to the threat's region before we pick a search node

			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());

			// Check the target's last volume info

			LTVector vTargetPosition;
			if ( pCharacter->HasLastVolume() )
			{
				if ( pCharacter->GetLastVolume()->HasRegion() )
				{
					CAIRegion* pRegion = pCharacter->GetLastVolume()->GetRegion();
					if ( pRegion->IsSearchable() )
					{
						vTargetPosition = pCharacter->GetLastVolumePos();
						if ( m_pStrategyFollowPath->Set(vTargetPosition) )
						{
							m_iSearchRegion = pRegion->GetIndex();
							m_pStrategyFollowPath->SetMovement(aniRun);
							return;
						}
					}
				}
			}

			// If we get here we couldn't engage

			GetAI()->ChangeState("CHASE");
			return;
		}
		else if ( !FindNode() )
		{
			GetAI()->ChangeState("AWARE");
			return;
		}
	}

	if ( !m_pStrategyFollowPath->IsDone() )
	{
		m_pStrategyFollowPath->Update();

		// If we're engaging, see if we've reached the engage region

		if ( m_bEngage )
		{
			if ( GetAI()->HasLastVolume() )
			{
				if ( GetAI()->GetLastVolume()->HasRegion() )
				{
					if ( GetAI()->GetLastVolume()->GetRegion()->GetIndex() == m_iSearchRegion )
					{
						if ( !FindNode() )
						{
							GetAI()->ChangeState("AWARE");
						}
						else
						{
							m_bEngage = LTFALSE;
							m_pStrategyFollowPath->SetMovement(aniWalk);
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
			m_pStrategyFollowPath->SetMovement(aniWalk);
			FindNode();
			return;
		}

		// Update our searching animation

		CAINode* pNode = g_pAINodeMgr->GetNode(m_iSearchNode);

		if ( !pNode )
		{
			GetAI()->ChangeState("AWARE");
			return;
		}
		else if ( m_bSearching && !GetAnimationContext()->IsLocked() )
		{
			m_bSearching = LTFALSE;

			pNode->Search();

			if ( !FindNode() )
			{
				CAIRegion* pRegion = g_pAIRegionMgr->GetRegionByIndex(m_iSearchRegion);
				if ( pRegion )
				{
					m_bDone = LTTRUE;

					HSTRING hstrPostSearchMsg = pRegion->GetPostSearchMsg();
					if ( hstrPostSearchMsg )
					{
						GetAI()->ChangeState(g_pLTServer->GetStringData(hstrPostSearchMsg));
					}
				}
				else
				{
					GetAI()->ChangeState("AWARE");
				}

				return;
			}
		}
		else
		{
			if ( !m_bSearching )
			{
				GetAI()->PlaySound(aisSearch);
			}

			if ( m_bFace )
			{
				GetAI()->FaceDir(pNode->GetForward());
			}

			m_aniSearch = GetRandomSearch(pNode);

			m_bSearching = LTTRUE;
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateSearch::FindNode()
{
	if ( GetAI()->HasLastVolume() )
	{
		if ( GetAI()->GetLastVolume()->HasRegion() )
		{
			CAIRegion* pRegion = GetAI()->GetLastVolume()->GetRegion();
			if ( pRegion && pRegion->IsSearchable() )
			{
				CAINode* pNode = pRegion->FindNearestSearchNode(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
				if ( !pNode || !m_pStrategyFollowPath->Set(pNode) )
				{
					return LTFALSE;
				}

				g_pAINodeMgr->UnlockNode(m_iSearchNode);
				m_iSearchNode = pNode->GetID();
				g_pAINodeMgr->LockNode(m_iSearchNode);

				if ( !m_bAdded )
				{
					pRegion->AddSearcher(GetAI());
					m_bAdded = LTTRUE;
					m_iSearchRegion = pRegion->GetIndex();
				}

				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	if ( m_bPause )
	{
		GetAnimationContext()->SetProp(aniAlert1);
		GetAnimationContext()->Lock();
	}
	else if ( m_bDone || m_pStrategyFollowPath->IsUnset() )
	{

	}
	else if ( m_pStrategyFollowPath->IsSet() )
	{
		if ( !m_bEngage )
		{
			GetAnimationContext()->SetProp(aniInvestigate);
		}

		m_pStrategyFollowPath->UpdateAnimation();
	}
	else if ( m_pStrategyFollowPath->IsDone() )
	{
		_ASSERT(m_bSearching);
		GetAnimationContext()->SetProp(m_aniSearch);
		GetAnimationContext()->Lock();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "ENGAGE") )
	{
		m_bEngage = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "FACE") )
	{
		m_bFace = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "PAUSE") )
	{
		m_bPause = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyFlashlight->Load(hRead);
	m_aniSearch.Load(hRead);

	LOAD_BOOL(m_bSearching);
	LOAD_DWORD(m_iSearchNode);
	LOAD_BOOL(m_bFace);
	LOAD_DWORD(m_bEngage);
	LOAD_DWORD(m_iSearchRegion);
	LOAD_BOOL(m_bDone);
	LOAD_BOOL(m_bAdded);
	LOAD_BOOL(m_bPause);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateSearch::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyFlashlight->Save(hWrite);
	m_aniSearch.Save(hWrite);

	SAVE_BOOL(m_bSearching);
	SAVE_DWORD(m_iSearchNode);
	SAVE_BOOL(m_bFace);
	SAVE_DWORD(m_bEngage);
	SAVE_DWORD(m_iSearchRegion);
	SAVE_BOOL(m_bDone);
	SAVE_BOOL(m_bAdded);
	SAVE_BOOL(m_bPause);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = FACTORY_NEW(CAIHumanStrategyOneShotAni);
	m_eObjectType = eNone;
	m_hObject = LTNULL;
	m_dwUseNode = CAINode::kInvalidNodeID;
	m_bAlert = LTTRUE;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateUseObject::Destructor()
{
	GetAI()->Unlink(m_hObject);

	g_pAINodeMgr->UnlockNode(m_dwUseNode);

	FACTORY_DELETE(m_pStrategyOneShotAni);
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateUseObject::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Update()
{
	super::Update();

	// Make sure everything we need at this point is there

	if ( !m_hObject || m_eObjectType == eNone || m_pStrategyFollowPath->IsUnset() )
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}

	// Play first sound

	if ( m_bPlayFirstSound )
	{
		GetAI()->PlaySound(aisBackup);
	}

	// See if we're done following the path to the object

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_pStrategyOneShotAni->IsDone() )
		{
			// We finished using the object, now send the message

			SendTriggerMsgToObject(GetAI(), m_hObject, LTFALSE, "ACTIVATE");

			NextOr("ATTACK");
			return;
		}
		else
		{
			GetAI()->FaceObject(m_hObject);

			// TODO: check for strategy failure
			m_pStrategyOneShotAni->Update();
		}
	}
	else
	{
		// Move along our path

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			// Just arrived at the object, now start using it

			switch ( m_eObjectType )
			{
				case CAIHumanStateUseObject::eAlarm:
				{
					m_pStrategyOneShotAni->Set(aniPushButton);
				}
				break;
				case CAIHumanStateUseObject::eSwitch:
				{
					m_pStrategyOneShotAni->Set(aniPushButton);
				}
				break;
				default:
				{
					_ASSERT(LTFALSE);
				}
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	m_pStrategyFollowPath->UpdateAnimation();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(aniLower);
		m_pStrategyOneShotAni->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		GetAI()->Unlink(m_hObject);
		m_hObject = LTNULL;

		CAINode* pAINode = g_pAINodeMgr->GetNode(szValue);
		if ( !pAINode )
		{
            g_pLTServer->CPrint("USEOBJECT DEST=%s -- unable to find node by this name!", szValue);
			return;
		}

		if ( !pAINode->HasUseObject() )
		{
            g_pLTServer->CPrint("USEOBJECT DEST=%s -- this node has no UseInfo!", szValue);
			return;
		}

		if ( pAINode->IsLocked() )
		{
            g_pLTServer->CPrint("USEOBJECT DEST=%s - use node is already locked!", szValue);
			return;
		}

		if ( LT_OK != FindNamedObject(pAINode->GetUseObject(), m_hObject) )
		{
            g_pLTServer->CPrint("USEOBJECT DEST=%s::%s -- this object does not exist!", szValue, g_pLTServer->GetStringData(pAINode->GetName()));
			return;
		}

        HCLASS hClass = g_pLTServer->GetObjectClass(m_hObject);

        if ( hClass == g_pLTServer->GetClass("Alarm") )
		{
            Alarm* pAlarm = (Alarm*)g_pLTServer->HandleToObject(m_hObject);
			m_eObjectType = eAlarm;

			if ( pAlarm->IsLocked() )
			{
				// This is okay. The alarm "system" is already sounding... just attack.

				m_eObjectType = eNone;
				return;
			}
		}
        else if ( hClass == g_pLTServer->GetClass("Switch") )
		{
            Switch* pSwitch = (Switch*)g_pLTServer->HandleToObject(m_hObject);
			m_eObjectType = eSwitch;
		}
		else
		{
            g_pLTServer->CPrint("USEOBJECT OBJECT=%s::%s -- object is not a usable type!", szValue, g_pLTServer->GetStringData(pAINode->GetName()));
			return;
		}

		if ( m_pStrategyFollowPath->Set(pAINode) )
		{
			GetAI()->Link(m_hObject);
			m_dwUseNode = pAINode->GetID();
			pAINode->Lock();
		}
		else
		{
			m_hObject = LTNULL;
			m_eObjectType = eNone;
            g_pLTServer->CPrint("USEOBJECT OBJECT=%s -- unable to find path!", szValue);
		}
	}
	else if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hObject )
	{
		m_hObject = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyOneShotAni->Load(hRead);

	LOAD_HOBJECT(m_hObject);
	LOAD_DWORD_CAST(m_eObjectType,CAIHumanStateUseObject::ObjectType);
	LOAD_DWORD(m_dwUseNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateUseObject::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_HOBJECT(m_hObject);
	SAVE_DWORD(m_eObjectType);
	SAVE_DWORD(m_dwUseNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = FACTORY_NEW(CAIHumanStrategyOneShotAni);
	m_bPickedUp = LTFALSE;
	m_eObjectType = eNone;
	m_hObject = LTNULL;
	m_dwPickupNode = CAINode::kInvalidNodeID;
	m_bAlert = LTTRUE;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStatePickupObject::Destructor()
{
	GetAI()->Unlink(m_hObject);

	g_pAINodeMgr->UnlockNode(m_dwPickupNode);

	FACTORY_DELETE(m_pStrategyOneShotAni);
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePickupObject::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::Update()
{
	super::Update();

	// If we picked the object up, it will be gone, but we need to wait for
	// the animation to finish to change states

	if ( !m_hObject && m_bPickedUp )
	{
		if ( m_pStrategyOneShotAni->IsDone() )
		{
			// We finished animating, now do our next thing

			NextOr("ATTACK");
			return;
		}
		else
		{
			m_pStrategyOneShotAni->Update();
			return;
		}
	}

	// Make sure the object didn't go away before we could pick it up

	if ( (!m_hObject && !m_bPickedUp) || m_eObjectType == eNone || m_pStrategyFollowPath->IsUnset() )
	{
		GetAI()->ChangeState("PANIC");
		return;
	}

	// See if we're done following the path to the object

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAI()->FaceObject(m_hObject);

		if ( m_bPickedUp )
		{
			// We got the pickup key, so pickup the object

			switch ( m_eObjectType )
			{
				case eWeapon:
					DoPickupWeapon();
					break;
				default :
					_ASSERT(LTFALSE);
					break;
			}

			// Remove it

            g_pLTServer->RemoveObject(m_hObject);
		}

		// TODO: check for strategy failure
		m_pStrategyOneShotAni->Update();
	}
	else
	{
		// Move along our path

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			// Just arrived at the object, now start using it

			switch ( m_eObjectType )
			{
				case CAIHumanStatePickupObject::eWeapon:
				{
					m_pStrategyOneShotAni->Set(aniPickup);
				}
				break;
				default:
				{
					_ASSERT(LTFALSE);
				}
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	m_pStrategyFollowPath->UpdateAnimation();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(aniLower);
		m_pStrategyOneShotAni->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::DoPickupWeapon()
{
    WeaponItem* pWeaponItem = (WeaponItem*)g_pLTServer->HandleToObject(m_hObject);

	char* szWeapon = g_pWeaponMgr->GetWeapon(pWeaponItem->GetWeaponId())->szName;
	char* szAmmo = g_pWeaponMgr->GetAmmo(pWeaponItem->GetAmmoId())->szName;

	char szAttachment[128];
	sprintf(szAttachment, "ATTACH RightHand (%s,%s)", szWeapon, szAmmo);

	SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyPickUp) )
	{
		m_bPickedUp = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		GetAI()->Unlink(m_hObject);
		m_hObject = LTNULL;

		CAINode* pAINode = g_pAINodeMgr->GetNode(szValue);
		if ( !pAINode )
		{
            g_pLTServer->CPrint("PICKUPOBJECT DEST=%s -- unable to find node by this name!", szValue);
			return;
		}

		if ( !pAINode->HasPickupObject() )
		{
            g_pLTServer->CPrint("PICKUPOBJECT DEST=%s -- this node has no PickupObject!", szValue);
			return;
		}

		if ( pAINode->IsLocked() )
		{
            g_pLTServer->CPrint("PICKUPOBJECT DEST=%s - pickup node is already locked!", szValue);
			return;
		}

		if ( LT_OK != FindNamedObject(pAINode->GetPickupObject(), m_hObject) )
		{
            g_pLTServer->CPrint("PICKUPOBJECT DEST=%s::%s -- this object does not exist!", szValue, g_pLTServer->GetStringData(pAINode->GetName()));
			return;
		}

        HCLASS hClass = g_pLTServer->GetObjectClass(m_hObject);

        if ( hClass == g_pLTServer->GetClass("WeaponItem") )
		{
            WeaponItem* pWeapon = (WeaponItem*)g_pLTServer->HandleToObject(m_hObject);
			m_eObjectType = eWeapon;
		}
		else
		{
            g_pLTServer->CPrint("PICKUPOBJECT OBJECT=%s::%s -- object is not a pickupable type!", szValue, g_pLTServer->GetStringData(pAINode->GetName()));
			return;
		}

		if ( m_pStrategyFollowPath->Set(pAINode) )
		{
			GetAI()->Link(m_hObject);
			m_dwPickupNode = pAINode->GetID();
			pAINode->Lock();
		}
		else
		{
			m_hObject = LTNULL;
			m_eObjectType = eNone;
            g_pLTServer->CPrint("PICKUPOBJECT OBJECT=%s -- unable to find path!", szValue);
		}
	}
	else if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hObject )
	{
		m_hObject = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyOneShotAni->Load(hRead);

	LOAD_BOOL(m_bPickedUp);
	LOAD_HOBJECT(m_hObject);
	LOAD_DWORD_CAST(m_eObjectType,CAIHumanStatePickupObject::ObjectType);
	LOAD_DWORD(m_dwPickupNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePickupObject::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_BOOL(m_bPickedUp);
	SAVE_HOBJECT(m_hObject);
	SAVE_DWORD(m_eObjectType);
	SAVE_DWORD(m_dwPickupNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_eState = eStatePosing;
	m_cTailNodes = 0;
	m_dwTailNode = CAINode::kInvalidNodeID;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateTail::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( !_stricmp(pArgList->argv[0], "WHISTLE") && (GetRandom(0.0f, 1.0f) > 0.50f) )
	{
		GetAI()->PlaySound(aisTail);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	LTVector vTargetPos;
    g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPos);

	CAINode* pNode = g_pAINodeMgr->FindTailNode(vTargetPos, GetAI()->GetPosition(), m_adwTailNodes, m_cTailNodes);

	if ( !pNode )
	{
		if ( m_dwTailNode != CAINode::kInvalidNodeID )
		{
			pNode = g_pAINodeMgr->GetNode(m_dwTailNode);
		}
		else
		{
			GetAI()->ChangeState("IDLE");
			return;
		}
	}

	if ( pNode->GetID() == m_dwTailNode )
	{
		// We're en route to the right node... keep going

		if ( m_pStrategyFollowPath->IsDone() )
		{
			// We're at the spot... so do our animation, sound, face the given position

			m_eState = eStatePosing;

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
		m_dwTailNode = pNode->GetID();

		if ( !m_pStrategyFollowPath->Set(pNode) )
		{
			GetAI()->ChangeState("IDLE");
			return;
		}

		m_eState = eStateMoving;

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_eState )
	{
		case eStatePosing:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniLower);
			GetAnimationContext()->SetProp(aniTail);
		}
		break;

		case eStateMoving:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniDown);
			m_pStrategyFollowPath->UpdateAnimation();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PTS") )
	{
		m_cTailNodes = 0;

		char *szPoint = strtok(szValue, ",");
		while ( szPoint )
		{
			if ( m_cTailNodes == kMaxTailNodes )
			{
                g_pLTServer->CPrint("Max # Tail waypoints exceeded %s=%s", szName, szValue);
			}

			CAINode* pNode = g_pAINodeMgr->GetNode(szPoint);

			if ( pNode )
			{
				m_adwTailNodes[m_cTailNodes++] = pNode->GetID();
			}
			else
			{
                g_pLTServer->CPrint("Unknown Tail waypoint ''%s''", szPoint);
			}

			szPoint = strtok(NULL, ",");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD(m_dwTailNode);
	LOAD_DWORD(m_cTailNodes);

    int iNode;
    for ( iNode = 0 ; iNode < m_cTailNodes ; iNode++ )
	{
		LOAD_DWORD(m_adwTailNodes[iNode]);
	}

	for ( iNode = m_cTailNodes ; iNode < kMaxTailNodes ; iNode++ )
	{
		m_adwTailNodes[iNode] = CAINode::kInvalidNodeID;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTail::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_dwTailNode);
	SAVE_DWORD(m_cTailNodes);

	for ( int iNode = 0 ; iNode < m_cTailNodes ; iNode++ )
	{
		SAVE_DWORD(m_adwTailNodes[iNode]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_fLatestTimestamp = 0.0f;
	m_bSearch = LTFALSE;
}

void CAIHumanStateFollowFootprint::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateFollowFootprint::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		ReturnOr("AWARE");
		return;
	}

	if ( m_bPlayFirstSound )
	{
		GetAI()->PlaySound(aisDisturb);
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

        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		_ASSERT(pCharacter);

		CTList<CharFootprintInfo*>* plistFootprints = pCharacter->GetFootprints();
		CharFootprintInfo** ppFootprint = plistFootprints->GetItem(TLIT_FIRST);

		while ( ppFootprint && *ppFootprint )
		{
			CharFootprintInfo* pFootprint = *ppFootprint;
			ppFootprint = plistFootprints->GetItem(TLIT_NEXT);

			LTFLOAT fSeeEnemyFootprintDistanceSqr = GetAI()->GetSenseMgr()->GetSense(stSeeEnemyFootprint)->GetDistanceSqr();

			if ( pFootprint->fTimeStamp <= m_fLatestTimestamp )
			{
				// We're looking at footprints we've already visited -- no new ones left. We're done.

				break;
			}

			if ( GetAI()->IsPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, pFootprint->vPos, fSeeEnemyFootprintDistanceSqr, LTFALSE) )
			{
				// We can see it, let's set a path to it.

				if ( !m_pStrategyFollowPath->Set(pFootprint->vPos) )
				{
					// No biggie -- just keep looking at other footsteps.
				}
				else
				{
					// We've got a path to the next one... record the timestamp and get going

					m_fLatestTimestamp = pFootprint->fTimeStamp;

					bDone = LTFALSE;

					break;
				}
			}
		}
	}

	if ( bDone )
	{
		if ( m_bSearch )
		{
			SearchOr("AWARE");
		}
		else
		{
			GetAI()->PlaySound(aisGaveUp);
			ReturnOr("AWARE");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	if ( m_pStrategyFollowPath->IsSet() )
	{
		GetAnimationContext()->SetProp(aniInvestigate);
		m_pStrategyFollowPath->UpdateAnimation();
	}
/*
	m_pStrategyOneShotAni->UpdateAnimation();
*/
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::UpdateSenses()
{
	static SenseType astSenses[] =
	{
		stSeeEnemy,
		stSeeEnemyFlashlight,
		stHearEnemyWeaponFire,
		stHearAllyWeaponFire,
		stHearEnemyWeaponImpact,
		stHearEnemyFootstep,
		stHearEnemyDisturbance,
		stSeeAllyDeath,
		stHearAllyDeath,
		stHearAllyPain
	};

	static int cSenses = sizeof(astSenses)/sizeof(SenseType);

	for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
	{
		GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "SEARCH") )
	{
		m_bSearch = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	LOAD_FLOAT(m_fLatestTimestamp);
	LOAD_BOOL(m_bSearch);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollowFootprint::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	SAVE_FLOAT(m_fLatestTimestamp);
	SAVE_BOOL(m_bSearch);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_hEnemy = LTNULL;
	m_stSenseType = stInvalid;
	m_bSearch = LTFALSE;
}

void CAIHumanStateInvestigate::Destructor()
{
	GetAI()->Unlink(m_hEnemy);

	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateInvestigate::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Update()
{
	super::Update();

	// Make sure everything we need at this point is there

	if ( !m_hEnemy || (m_stSenseType == stInvalid) )
	{
		ReturnOr("AWARE");
		return;
	}

	// Play our first sound

	if ( m_bPlayFirstSound )
	{
		GetAI()->PlaySound(aisDisturb);
	}

	// Set our path if we haven't yet done so

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		LTVector vDestination;

		switch ( m_stSenseType )
		{
			case stSeeEnemy:
			case stSeeEnemyFlashlight:
		        g_pLTServer->GetObjectPos(m_hEnemy, &vDestination);
				break;

			case stHearEnemyFootstep:
			case stHearEnemyDisturbance:
		        vDestination = m_vPosition;
				break;
		}

		if ( !m_pStrategyFollowPath->Set(vDestination) )
		{
			if ( stHearEnemyDisturbance == m_stSenseType )
			{
				// If we couldn't reach the coin, look at it.
				char szBuffer[1024];
				sprintf(szBuffer, "LOOKAT POSITION=%f,%f,%f SENSE=%d", EXPANDVEC(vDestination), stHearEnemyDisturbance);

				if ( m_hstrReturn )
				{
					strcat(szBuffer, " RETURN=(");
					strcat(szBuffer, g_pLTServer->GetStringData(m_hstrReturn));
					strcat(szBuffer, ")");
				}
				else if ( m_bSearch )
				{
					strcat(szBuffer, " RETURN=AWARE");
				}

				GetAI()->ChangeState(szBuffer);
			}
			else if ( m_bSearch )
			{
				GetAI()->Target(m_hEnemy);
				SearchOr("AWARE");
			}
			else
			{
				GetAI()->PlaySound(SenseToFailSound(m_stSenseType));
				ReturnOr("AWARE");
			}

			return;
		}
	}

	// TODO: check for strategy failure
	m_pStrategyFollowPath->Update();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// We finished moving to where we saw the enemy

		if ( m_bSearch )
		{
			GetAI()->Target(m_hEnemy);
			SearchOr("AWARE");
		}
		else
		{
			GetAI()->PlaySound(SenseToFailSound(m_stSenseType));
			ReturnOr("AWARE");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	if ( m_pStrategyFollowPath->IsSet() )
	{
		GetAnimationContext()->SetProp(aniInvestigate);
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::UpdateSenses()
{
	switch ( m_stSenseType )
	{
		case stSeeEnemy:
		case stSeeEnemyFlashlight:
		{
			static SenseType astSenses[] = // see
			{
				stSeeEnemy,
				stSeeEnemyFootprint,
				stSeeEnemyFlashlight,
				stHearEnemyWeaponFire,
				stHearAllyWeaponFire,
				stHearEnemyWeaponImpact,
				stHearEnemyFootstep,
				stHearEnemyDisturbance,
				stSeeAllyDeath,
				stHearAllyDeath,
				stHearAllyPain
			};

			static int cSenses = sizeof(astSenses)/sizeof(SenseType);

			for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
			{
				GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
			}
		}
		break;

		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		{
			static SenseType astSenses[] = // hear
			{
				stSeeEnemy,
				stSeeEnemyFootprint,
				stSeeEnemyFlashlight,
				stHearEnemyWeaponFire,
				stHearAllyWeaponFire,
				stHearEnemyWeaponImpact,
				stSeeAllyDeath,
				stHearAllyDeath,
				stHearAllyPain
			};

			static int cSenses = sizeof(astSenses)/sizeof(SenseType);

			for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
			{
				GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "ENEMY") )
	{
		GetAI()->Unlink(m_hEnemy);
		m_hEnemy = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hEnemy) )
		{
			if ( IsKindOf(m_hEnemy, "CCharacter") )
			{
				GetAI()->Link(m_hEnemy);
			}
			else
			{
                g_pLTServer->CPrint("Investigate ENEMY=%s -- this object is not a CCharacter!", szValue);
				m_hEnemy = LTNULL;
			}
		}
		else
		{
            g_pLTServer->CPrint("Investigate ENEMY=%s -- this object does not exist!", szName);
		}
	}
	else if ( !_stricmp(szName, "SENSE") )
	{
		m_stSenseType = (SenseType)atol(szValue);
	}
	else if ( !_stricmp(szName, "POSITION") )
	{
		sscanf(szValue, "%f,%f,%f", &m_vPosition.x, &m_vPosition.y, &m_vPosition.z);
	}
	else if ( !_stricmp(szName, "SEARCH") )
	{
		m_bSearch = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hEnemy )
	{
		m_hEnemy = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_stSenseType, SenseType);
	LOAD_HOBJECT(m_hEnemy);
	LOAD_BOOL(m_bSearch);
	LOAD_VECTOR(m_vPosition);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateInvestigate::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_stSenseType);
	SAVE_HOBJECT(m_hEnemy);
	SAVE_BOOL(m_bSearch);
	SAVE_VECTOR(m_vPosition);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyOneShotAni = FACTORY_NEW(CAIHumanStrategyOneShotAni);

	m_hBody = LTNULL;
	m_bSearch = LTFALSE;
}

void CAIHumanStateCheckBody::Destructor()
{
	if ( m_hBody )
	{
		Body* pBody = (Body*)g_pLTServer->HandleToObject(m_hBody);
		if ( pBody )
		{
			if ( pBody->HasChecker() && (pBody->GetChecker() == GetAI()->GetObject()) )
			{
				pBody->SetChecker(LTNULL);
			}
		}
	}

	GetAI()->Unlink(m_hBody);

	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyOneShotAni);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCheckBody::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Update()
{
	super::Update();

	// If we're not the checker or the body has gone away, then move on to something else...

	Body* pBody = m_hBody ? (Body*)g_pLTServer->HandleToObject(m_hBody) : LTNULL;
	if ( pBody )
	{
		if ( !pBody->CanCheckPulse() )
		{
			if ( m_bSearch )
			{
				SearchOr("AWARE FIRSTSOUND=TRUE");
			}
			else
			{
				// Well, this is a judgment call, but I don't think we should return to
				// our previous state, because it will look weird.
				// So if this is not "investigate and search", just go aware.

				GetAI()->ChangeState("AWARE FIRSTSOUND=TRUE");
			}

			return;
		}
		else if ( pBody->HasChecker() && (pBody->GetChecker() != GetAI()->GetObject()) )
		{
			// Can't check its pulse or somebody else is checking it... oops

			goto Done;
		}
		else
		{
			pBody->SetChecker(GetAI()->GetObject());
		}
	}
	else
	{
		goto Done;
	}

	// Set our path if we haven't yet done so

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		// Get the specified node (case insensitive).

		HMODELNODE hNeckNode;
		LTransform transform;
		LTVector vNeckPos;

		if ( LT_OK != g_pModelLT->GetNode(m_hBody, "Neck_node", hNeckNode) )
		{
			goto Done;
		}

		g_pModelLT->GetNodeTransform(m_hBody, hNeckNode, transform, LTTRUE);
		g_pTransLT->GetPos(transform, vNeckPos);

		if ( !m_pStrategyFollowPath->Set(vNeckPos) )
		{
			goto Done;
		}
	}

	// See if we're done following the path to the object

	if ( m_pStrategyFollowPath->IsDone() )
	{
		if ( m_pStrategyOneShotAni->IsDone() )
		{
			// We finished checking the Body

			goto Done;
		}
		else
		{
			HMODELNODE hNeckNode;
			HMODELNODE hHandNode;
			LTransform transform;
			LTVector vNeckPos;
			LTVector vHandPos;

			if ( LT_OK != g_pModelLT->GetNode(m_hBody, "Neck_node", hNeckNode) )
			{
				goto Done;
			}

			g_pModelLT->GetNodeTransform(m_hBody, hNeckNode, transform, LTTRUE);
			g_pTransLT->GetPos(transform, vNeckPos);

			if ( LT_OK != g_pModelLT->GetNode(GetAI()->GetObject(), "left_arml_hand_node", hHandNode) )
			{
				goto Done;
			}

			g_pModelLT->GetNodeTransform(GetAI()->GetObject(), hHandNode, transform, LTTRUE);
			g_pTransLT->GetPos(transform, vHandPos);

			LTVector vOffset;
			vOffset = vNeckPos - vHandPos;
			vOffset.y = 0;

			LTVector vPosition = GetAI()->GetPosition() + vOffset;

			if ( GetAI()->HasLastVolume() && GetAI()->GetLastVolume()->Inside2dLoose(vPosition, GetAI()->GetRadius()) )
			{
				GetAI()->Move(vPosition);
			}

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

			GetAI()->FaceObject(m_hBody);
			m_pStrategyOneShotAni->Set(aniCheckPulse);

			// Play our sound

			GetAI()->PlaySound(aisSeeBody);
		}
	}

	return;

Done:

	if ( m_bSearch )
	{
		SearchOr("AWARE");
	}
	else
	{
		ReturnOr("AWARE");
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyFollowPath )
	{
		if ( m_pStrategyFollowPath->IsDone() )
		{
			GetAnimationContext()->SetProp(aniStand);
			m_pStrategyOneShotAni->UpdateAnimation();
		}
		else
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniDown);
			m_pStrategyFollowPath->UpdateAnimation();
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::UpdateSenses()
{
	static SenseType astSenses[] =
	{
		stSeeEnemy,
		stSeeEnemyFlashlight,
		stHearEnemyWeaponFire,
		stHearAllyWeaponFire,
		stHearEnemyWeaponImpact,
		stHearAllyPain,
	};

	static int cSenses = sizeof(astSenses)/sizeof(SenseType);

	for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
	{
		GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "BODY") )
	{
		GetAI()->Unlink(m_hBody);
		m_hBody = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hBody) )
		{
			if ( IsKindOf(m_hBody, "Body") )
			{
				GetAI()->Link(m_hBody);
			}
			else
			{
                g_pLTServer->CPrint("CHECKBODY BODY=%s -- this object is not a Body!", szValue);
				m_hBody = LTNULL;
			}
		}
		else
		{
            g_pLTServer->CPrint("CHECKBODY BODY=%s -- this object does not exist!", szName);
		}
	}
	else if ( !_stricmp(szName, "SEARCH") )
	{
		m_bSearch = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hBody )
	{
		m_hBody = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyOneShotAni->Load(hRead);

	LOAD_HOBJECT(m_hBody);
	LOAD_BOOL(m_bSearch);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCheckBody::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_HOBJECT(m_hBody);
	SAVE_BOOL(m_bSearch);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_fStopTime = 0.0f;
	m_fVisibleTimer = 0.0f;

	m_bPlayedChaseSound = LTFALSE;
	m_bPlayedFoundSound = LTFALSE;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateChase::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateChase::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( GetAI()->IsScuba() )
	{
		m_pStrategyFollowPath->SetMovement(aniSwim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(aniRun);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
		m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Update()
{
	super::Update();

	// Do we have a target?

	if ( !GetAI()->HasTarget() )
	{
		// Target must have died or something

		GetAI()->ChangeState("AWARE");
		return;
	}

	if ( m_bFirstUpdate )
	{
		m_fStopTime = g_pLTServer->GetTime() + GetAI()->GetBrain()->GetChaseSearchTime();
		GetAI()->FaceTarget();
	}

	// Instant reload if we need to

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload(LTTRUE);
	}

	CAITarget *pTarget = GetAI()->GetTarget();

	RangeStatus eRangeStatus = GetAI()->GetBrain()->GetRangeStatus();

	if ( pTarget->IsVisibleCompletely() && (eRangeStatus != eRangeStatusTooFar) )
	{
        m_fVisibleTimer += g_pLTServer->GetFrameTime();

		if ( m_fVisibleTimer > GetAI()->GetBrain()->GetChaseExtraTime() || (eRangeStatus == eRangeStatusTooClose) )
		{
			//TODO: hack for E3
			{
				if ( m_bPlayedChaseSound && !m_bPlayedFoundSound )
				{
					if ( GetRandom(0.0f, 1.0f) < GetAI()->GetBrain()->GetChaseFoundSoundChance() )
					{
						// $SOUND GetAI()->PlaySound(aisChaseFound);
					}
				}
			}

			GetAI()->ChangeState("ATTACK");
			return;
		}
	}
	else
	{
		m_fVisibleTimer = -GetRandom(GetAI()->GetBrain()->GetChaseExtraTimeRandomMin(),
									 GetAI()->GetBrain()->GetChaseExtraTimeRandomMax());

		if ( pTarget->IsVisiblePartially() )
		{
			m_fStopTime = g_pLTServer->GetTime() + GetAI()->GetBrain()->GetChaseSearchTime();
		}
	}

	if ( pTarget->IsVisibleFromWeapon() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}

	//TODO: hack for E3
	{
		LTFLOAT fChaseSoundTime = GetAI()->GetBrain()->GetChaseSoundTime();
		LTFLOAT fChaseSoundTimeRandomMin = GetAI()->GetBrain()->GetChaseSoundTimeRandomMin();
		LTFLOAT fChaseSoundTimeRandomMax = GetAI()->GetBrain()->GetChaseSoundTimeRandomMax();

		if ( !m_bPlayedChaseSound && (m_fElapsedTime > (fChaseSoundTime + GetRandom(fChaseSoundTimeRandomMin, fChaseSoundTimeRandomMax))) )
		{
			// $SOUND GetAI()->PlaySound(aisChase);
			m_bPlayedChaseSound = LTTRUE;
		}
	}

	LTBOOL bMove = LTTRUE;

	// Set our path if we need to

	if ( !m_pStrategyFollowPath->IsSet() )
	{
		// If we have reached the end of our path and can see the enemy, go aggressive

		if ( pTarget->IsVisibleCompletely() && (eRangeStatus != eRangeStatusTooFar) )
		{
			//TODO: hack for E3
			{
				if ( m_bPlayedChaseSound && !m_bPlayedFoundSound )
				{
					if ( GetRandom(0.0f, 1.0f) < GetAI()->GetBrain()->GetChaseFoundSoundChance() )
					{
						// $SOUND GetAI()->PlaySound(aisChaseFound);
					}

					m_bPlayedFoundSound = LTTRUE;
				}
			}

			GetAI()->ChangeState("ATTACK");
			return;
		}

		// Get the last usable position

		CCharacter* pCharacter = pTarget->GetCharacter();

		// Check the target's last volume info

		LTVector vTargetPosition;
		if ( pCharacter->HasLastVolume() )
		{
			vTargetPosition = pCharacter->GetLastVolumePos();
			LTVector vDir = vTargetPosition - GetAI()->GetPosition();
			vDir.y = 0;

			if ( VEC_MAGSQR(vDir) < 100.0f )
			{
				GetAI()->ChangeState("ATTACKONSIGHT CHASEDELAY=TRUE");
				return;
			}
		}
		else
		{
			GetAI()->ChangeState("ATTACKONSIGHT CHASEDELAY=TRUE");
			return;
		}

		LTBOOL bCanFollow = m_pStrategyFollowPath->IsUnset() || (m_fStopTime > g_pLTServer->GetTime());

		if ( bCanFollow )
		{
			if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
			{
				// No path, try attacking from view node (this might not work)

				GetAI()->ChangeState("ATTACKFROMVIEW");
				return;
			}
		}
		else
		{
			// Duh... where'd they go? Start searching...

			GetAI()->ChangeState("SEARCH ENGAGE=TRUE");
			return;
		}
	}

	// Now follow our path

	if ( bMove )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	m_pStrategyShoot->UpdateAnimation();
	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyShoot->Load(hRead);

	LOAD_FLOAT(m_fStopTime);
	LOAD_FLOAT(m_fVisibleTimer);

	LOAD_BOOL(m_bPlayedChaseSound);
	LOAD_BOOL(m_bPlayedFoundSound);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateChase::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyShoot->Save(hWrite);

	SAVE_FLOAT(m_fStopTime);
	SAVE_FLOAT(m_fVisibleTimer);

	SAVE_BOOL(m_bPlayedChaseSound);
	SAVE_BOOL(m_bPlayedFoundSound);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;
	m_bDrew = LTFALSE;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateDraw::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("AWARE");
		return;
	}

	if ( !GetAI()->HasHolsterString() )
	{
		GetAI()->ChangeState("PANIC");
		return;
	}

	if ( m_bPlayFirstSound || (m_bFirstUpdate && (GetAI()->GetBrain()->GetAttackSoundChance() > GetRandom(0.0f, 1.0f))) )
	{
		GetAI()->PlaySound(aisCombAggr);
	}

	GetAI()->FaceTarget();

	if ( m_bDrew && !GetAnimationContext()->IsLocked() )
	{
		NextOr("ATTACK");
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDraw);
	GetAnimationContext()->Lock();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( GetAI()->HasHolsterString() && !_stricmp(pArgList->argv[0], "DRAW") )
	{
		char szAttachment[128];
		sprintf(szAttachment, "ATTACH RightHand (%s)", GetAI()->GetHolsterString());
		SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szAttachment);

		m_bDrew = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_BOOL(m_bDrew);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDraw::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_BOOL(m_bDrew);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_pStrategyDodge = FACTORY_NEW(CAIHumanStrategyDodge);

	m_aniPosture = aniStand;

	m_bChase = LTTRUE;
	m_fChaseTimer = 0.0f;
	m_fChaseDelay = 0.0f;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateAttack::Destructor()
{
	FACTORY_DELETE(m_pStrategyShoot);
	FACTORY_DELETE(m_pStrategyDodge);

	if ( m_pStrategyGrenade )
	{
		FACTORY_DELETE(m_pStrategyGrenade);
	}

	super::Destructor();
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

	if ( GetAI()->GetNumWeapons() == 2 )
	{
		m_pStrategyGrenade = FACTORY_NEW(CAIHumanStrategyGrenadeThrow);
		if ( !m_pStrategyGrenade->Init(pAIHuman) )
		{
			return LTFALSE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	// Reset our timers if we're getting pegged so we don't just stand there

	m_fChaseTimer = 0.0f;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "CHASE") )
	{
		m_bChase = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "CHASEDELAY") )
	{
		m_fChaseDelay = (LTFLOAT)atof(szValue) + g_pLTServer->GetTime();
	}
	else if ( !_stricmp(szName, "POSTURE") )
	{
		if ( !_stricmp(szValue, "CROUCH") )
		{
			m_aniPosture = aniCrouch;
		}
		else if ( !_stricmp(szValue, "STAND") )
		{
			m_aniPosture = aniStand;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Update()
{
	super::Update();

	if ( !GetAI()->GetCurrentWeapon() )
	{
		// If we don't have a weapon, panic.
		GetAI()->ChangeState("PANIC");
		return;
	}

	if ( m_bPlayFirstSound || (m_bFirstUpdate && (GetAI()->GetBrain()->GetAttackSoundChance() > GetRandom(0.0f, 1.0f))) )
	{
		GetAI()->PlaySound(aisCombAggr);
	}

	if ( GetAnimationContext()->IsPropSet(aniStand) )
	{
		m_aniPosture = aniStand;
	}
	else if ( GetAnimationContext()->IsPropSet(aniCrouch) )
	{
		m_aniPosture = aniCrouch;
	}

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("AWARE");
		return;
	}

	if ( m_pStrategyGrenade && m_pStrategyGrenade->IsThrowing() )
	{
		GetAI()->FaceTarget();

		// TODO: check for strategy failure
		m_pStrategyGrenade->Update();

		return;
	}

	if ( m_pStrategyDodge->IsDodging() )
	{
		GetAI()->FaceTarget();

		// TODO: check for strategy failure
		m_pStrategyDodge->Update();

		return;
	}

	if ( m_pStrategyShoot->IsReloading() )
	{
		GetAI()->FaceTarget();

		// TODO: check for strategy failure
		m_pStrategyShoot->Update();

		return;
	}

	// See if we should crouch

	if ( m_bFirstUpdate && (GetRandom(0.0f, 1.0f) <= GetAI()->GetBrain()->GetAttackPoseCrouchChance()) )
	{
		m_aniPosture = aniCrouch;
	}

	// Check our range from the target

	RangeStatus eRangeStatus = CanChase(LTTRUE) ? GetAI()->GetBrain()->GetRangeStatus() : eRangeStatusOk;

	if ( eRangeStatus == eRangeStatusTooFar )
	{
		if ( (0.25f > GetRandom(0.0, 1.0f)) && (GetAI()->GetHitPoints()/GetAI()->GetMaxHitPoints() < .25f) )
		{
			GetAI()->PlaySound(aisCharge);
		}

		GetAI()->ChangeState("CHASE");
		return;
	}

	if ( m_aniPosture == aniStand )
	{
		// See if we should dodge

		m_pStrategyDodge->Update();

		switch ( m_pStrategyDodge->GetStatus() )
		{
			case eDodgeStatusVector:
			case eDodgeStatusProjectile:
			{
				m_pStrategyDodge->Dodge();
				return;
			}
			break;

			case eDodgeStatusOk:
			{
			}
			break;
		}
	}

	// Face the target

	GetAI()->FaceTarget();

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload();

		// We're done

		return;
	}

	CAITarget *pTarget = GetAI()->GetTarget();
	HOBJECT hTarget = pTarget->GetObject();

	if ( pTarget->IsVisibleCompletely() )
	{
        CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
		if (pB && !pB->IsDead())
		{
			if ( m_pStrategyGrenade && m_aniPosture == aniStand && m_pStrategyGrenade->ShouldThrow() )
			{
				// TODO: check for strategy failure
				m_pStrategyGrenade->Throw();
			}
			else
			{
				// TODO: check for strategy failure
				m_pStrategyShoot->Update();
			}
		}

		m_fChaseTimer = GetAI()->GetBrain()->GetAttackChaseTime() +
						GetRandom(GetAI()->GetBrain()->GetAttackChaseTimeRandomMin(),
								  GetAI()->GetBrain()->GetAttackChaseTimeRandomMax());
	}
	else
	{
		// Can't see our target anymore and we're not waiting for one

        m_fChaseTimer -= g_pLTServer->GetFrameTime();

		if ( CanChase(LTFALSE) )
		{
			GetAI()->ChangeState("CHASE");
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

CNudge::Priority CAIHumanStateAttack::GetNudgePriority()
{
	if ( m_pStrategyDodge->IsDodging() )
	{
		return CNudge::ePriorityHigh;
	}
	else
	{
		return CNudge::ePriorityLow;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttack::CanChase(LTBOOL bOutOfRange)
{
	return (m_bChase && ((m_fChaseTimer <= 0.0f) || bOutOfRange) && (g_pLTServer->GetTime() > m_fChaseDelay));
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyGrenade && m_pStrategyGrenade->IsThrowing() )
	{
		m_pStrategyGrenade->UpdateAnimation();
	}
	else
	{
		GetAnimationContext()->SetProp(m_aniPosture);
		GetAnimationContext()->SetProp(aniUp);
//		GetAnimationContext()->SetProp(aniRun);

		if ( m_pStrategyDodge->IsDodging() )
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

void CAIHumanStateAttack::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);
	m_pStrategyDodge->Load(hRead);

	CAIHumanStrategy::AIHumanStrategyType eType;
	LOAD_DWORD_CAST(eType, CAIHumanStrategy::AIHumanStrategyType);

	m_pStrategyGrenade = CAIHumanStrategyGrenade::Create(eType);
	if ( m_pStrategyGrenade )
	{
		m_pStrategyGrenade->Init(GetAI());
		m_pStrategyGrenade->Load(hRead);
	}

	m_aniPosture.Load(hRead);

	LOAD_FLOAT(m_fChaseTimer);
	LOAD_BOOL(m_bChase);
	LOAD_FLOAT(m_fChaseDelay);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttack::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
	m_pStrategyDodge->Save(hWrite);

	if ( m_pStrategyGrenade )
	{
		SAVE_DWORD(m_pStrategyGrenade->GetType());
		m_pStrategyGrenade->Save(hWrite);
	}
	else
	{
		SAVE_DWORD(CAIHumanStrategy::eStrategyNone);
	}

	m_aniPosture.Save(hWrite);

	SAVE_FLOAT(m_fChaseTimer);
	SAVE_BOOL(m_bChase);
	SAVE_FLOAT(m_fChaseDelay);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_hProp = LTNULL;
}

void CAIHumanStateAttackProp::Destructor()
{
	GetAI()->Unlink(m_hProp);

	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
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

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hProp )
	{
		m_hProp = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PROP") )
	{
		GetAI()->Unlink(m_hProp);
		m_hProp = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hProp) )
		{
			if ( IsKindOf(m_hProp, "Prop") )
			{
				GetAI()->Link(m_hProp);
			}
			else
			{
                g_pLTServer->CPrint("ATTACKPROP PROP=%s -- this object is not a Prop!", szValue);
				m_hProp = LTNULL;
			}
		}
		else
		{
            g_pLTServer->CPrint("ATTACKPROP PROP=%s -- this object does not exist!", szName);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Update()
{
	//GetAI()->GetAnimationContext()->SetPosture(CAnimatorAI::eAim);

	super::Update();

	if ( !m_hProp )
	{
		NextOr("IDLE");
		return;
	}

	GetAI()->FaceObject(m_hProp);

	if ( m_pStrategyShoot->IsReloading() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update(m_hProp);

		// We're done

		return;
	}

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload();

		// We're done

		return;
	}

	// TODO: check for strategy failure
	m_pStrategyShoot->Update(m_hProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	m_pStrategyShoot->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);

	LOAD_HOBJECT(m_hProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProp::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);

	SAVE_HOBJECT(m_hProp);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_bChaseDelay = LTFALSE;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateAttackOnSight::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	// Go after our target

	GetAI()->ChangeState("ATTACK");
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "CHASEDELAY") )
	{
		m_bChaseDelay = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::Update()
{
	super::Update();

	if ( !GetAI()->GetCurrentWeapon() )
	{
		// If we don't have a weapon, panic.
		GetAI()->ChangeState("PANIC");
		return;
	}

	if ( m_bPlayFirstSound || (m_bFirstUpdate && (GetAI()->GetBrain()->GetAttackSoundChance() > GetRandom(0.0f, 1.0f))) )
	{
		GetAI()->PlaySound(aisCombAggr);
	}

	// If we can see our target or if it's attacking us...

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( GetAI()->GetTarget()->IsVisiblePartially() || GetAI()->GetTarget()->IsAttacking() )
	{
		if ( m_bChaseDelay )
		{
			// TODO: don't hardcode this
			GetAI()->ChangeState("ATTACK CHASEDELAY=10.0");
		}
		else
		{
			GetAI()->ChangeState("ATTACK");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_BOOL(m_bChaseDelay);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackOnSight::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_BOOL(m_bChaseDelay);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_bIgnoreVisibility = LTTRUE;
}

void CAIHumanStateAssassinate::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAssassinate::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniWalk);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		if ( !IsVector(szValue) )
		{
			CAINode *pNode = g_pAINodeMgr->GetNode(szValue);

			if ( !pNode )
			{
                g_pLTServer->CPrint("ASSASSINATE DEST=%s - Could not find a node by this name!", szValue);
			}
			else
			{
				if ( !m_pStrategyFollowPath->Set(pNode) )
				{
                    g_pLTServer->CPrint("ASSASSINATE DEST=%s FollowPath failed", szValue);
				}
			}
		}
		else
		{
			LTVector vDest;

			sscanf(szValue, "%f,%f,%f", &vDest.x, &vDest.y, &vDest.z);

			if ( !m_pStrategyFollowPath->Set(vDest) )
			{
                g_pLTServer->CPrint("ASSASSINATE DEST=%s FollowPath failed", szValue);
			}
		}
	}
	else if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
		}
	}
	else if ( !_stricmp(szName, "IGNOREVIS") )
	{
		m_bIgnoreVisibility = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_pStrategyShoot->IsReloading() )
	{
		GetAI()->FaceTarget();

		// TODO: check for strategy failure
		m_pStrategyShoot->Update();

		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// We haven't arrived at our "assassinate" dest yet, move there.

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Our gun is drawn, start capping

		GetAI()->FaceTarget();

		if ( m_pStrategyShoot->NeedsReload() )
		{
			m_pStrategyShoot->Reload();

			return;
		}

		// Only fire we if we can see our target or we're told to ignore visibility

		if ( m_bIgnoreVisibility || GetAI()->GetTarget()->IsVisibleFromWeapon() )
		{
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

	GetAnimationContext()->SetProp(aniStand);

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(aniUp);
		m_pStrategyShoot->UpdateAnimation();
	}
	else
	{
		GetAnimationContext()->SetProp(aniDown);
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyShoot->Load(hRead);

	LOAD_BOOL(m_bIgnoreVisibility);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAssassinate::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyShoot->Save(hWrite);

	SAVE_BOOL(m_bIgnoreVisibility);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_pStrategyCover = LTNULL;

	m_eState = eStateFindCover;

	m_nRetries = 0;
	m_dwCoverNode = CAINode::kInvalidNodeID;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateAttackFromCover::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	if ( m_pStrategyCover )
	{
		FACTORY_DELETE(m_pStrategyCover);
	}

	if ( m_eState == eStateUseCover )
	{
		CAINode* pNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
		if ( pNode )
		{
			GetAI()->BoostHitpoints(1.0f/pNode->GetCoverHitpointsBoost());
		}
	}

	g_pAINodeMgr->UnlockNode(m_dwCoverNode);

	super::Destructor();
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

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Update()
{
	super::Update();

	if ( !GetAI()->GetCurrentWeapon() )
	{
		// If we don't have a weapon, panic.
		GetAI()->ChangeState("PANIC");
		return;
	}

	if ( m_bPlayFirstSound || (m_bFirstUpdate && (GetAI()->GetBrain()->GetAttackFromCoverSoundChance() > GetRandom(0.0f, 1.0f))) )
	{
		GetAI()->PlaySound(aisCombDef);
	}

	if ( m_nRetries == (int32)GetAI()->GetBrain()->GetAttackFromCoverMaxRetries() )
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}

	// TODO: We need to wait for a target if we don't have one yet
	// TOOD: But right now we just go idle.

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	// If we have a target start doing our stuff

	switch ( m_eState )
	{
		case eStateFindCover:
		{
			UpdateFindCover();
		}
		break;

		case eStateGotoCover:
		{
			UpdateGotoCover();
		}
		break;

		case eStateUseCover:
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

	switch ( m_eState )
	{
		case eStateFindCover:
		case eStateGotoCover:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);

			m_pStrategyFollowPath->UpdateAnimation();
		}
		break;

		case eStateUseCover:
		{
			// Cover/fire strategies will override these if necessary

			GetAnimationContext()->SetProp(aniDown);

			m_pStrategyShoot->UpdateAnimation();
			m_pStrategyCover->UpdateAnimation();
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateFindCover()
{
	CAINode *pCoverNode = g_pAINodeMgr->FindNearestCoverFromThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
	if ( !pCoverNode )
	{
		goto Error;
	}
	else if ( !m_pStrategyFollowPath->Set(pCoverNode->GetPos()) )
	{
		goto Error;
	}

	// Release any node we're currently holding

	g_pAINodeMgr->UnlockNode(m_dwCoverNode);

	// Claim the node as ours

	m_dwCoverNode = pCoverNode->GetID();
	g_pAINodeMgr->LockNode(m_dwCoverNode);

	// Choose our cover strategy

	SetCoverStrategy(GetRandomCoverStrategy(pCoverNode));

	// Goto the cover

	m_eState = eStateGotoCover;

	return;

Error:

	GetAI()->ChangeState("ATTACK");
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateGotoCover()
{
	// TODO: check for strategy failure
	m_pStrategyFollowPath->Update();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// We've reached cover... clear out the cover strategy

		m_pStrategyCover->Clear();

		// Trigger the cover object if we need to

		CAINode *pNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
		if ( pNode )
		{
			if ( pNode->HasCoverObject() )
			{
				HOBJECT hObject;
				if ( LT_OK == FindNamedObject(pNode->GetCoverObject(), hObject) )
				{
					SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
					pNode->ClearCoverObject();
				}
			}

			// Use the cover

			GetAI()->BoostHitpoints(pNode->GetCoverHitpointsBoost());

			m_eState = eStateUseCover;
		}
		else
		{
			GetAI()->ChangeState("ATTACK");
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::UpdateUseCover()
{
	_ASSERT(m_pStrategyCover);
	if ( !m_pStrategyCover ) return;

	if ( m_pStrategyCover->IsCovered() )
	{
		// Check to see if our cover is still valid

		CAINode* pNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
		if (!pNode) return;

		if ( m_fElapsedTime > pNode->GetCoverTimeout() )
		{
			// If we've been here too long, send our timeoutcmd

			if ( pNode->GetCoverTimeoutCmd() )
			{
				GetAI()->ChangeState(g_pLTServer->GetStringData(pNode->GetCoverTimeoutCmd()));
				return;
			}
		}

		CoverStatus eCoverStatus = pNode->GetCoverStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

		if ( eCoverStatusThreatInsideRadius == eCoverStatus )
		{
			HSTRING hstrReaction = pNode->GetCoverThreatRadiusReaction();
			if ( hstrReaction )
			{
                GetAI()->ChangeState(g_pLTServer->GetStringData(hstrReaction));
				return;
			}
		}

		if ( m_nRetries != -1 && (eCoverStatusOk != eCoverStatus) )
		{
			char szRetry[128];
			sprintf(szRetry, "ATTACKFROMCOVER RETRIES=%d", m_nRetries+1);

			GetAI()->ChangeState(szRetry);
			return;
		}

		if ( m_pStrategyShoot->NeedsReload() )
		{
			GetAI()->FaceTarget();

			m_pStrategyShoot->Reload();
		}

		if ( m_pStrategyCover->IsCovered() )
		{
			if ( m_pStrategyShoot->IsReloading() )
			{
				GetAI()->FaceTarget();

				// TODO: check for strategy failure
				m_pStrategyShoot->Update();
			}
			else if ( m_pStrategyShoot->NeedsReload() )
			{
				GetAI()->FaceTarget();

				m_pStrategyShoot->Reload();
			}
			else if ( GetAI()->GetTarget()->IsVisiblePartially() )
			{
				// TODO: check for strategy failure
				m_pStrategyShoot->Update();

				m_pStrategyCover->Clear();
			}
		}
	}

	if ( m_pStrategyCover->IsUncovered() )
	{
		// If we need to reload, go back to cover and then do so

		if ( m_pStrategyShoot->NeedsReload() )
		{
			m_pStrategyCover->Cover();
		}
		else if ( m_pStrategyCover->CanBlindFire() )
		{
			// If we're a blind fire capable cover strategy, do it

			GetAI()->SetAccuracyModifier(.25f, 1.0f);

			// TODO: check for strategy failure
			m_pStrategyShoot->Update(GetAI()->GetTarget()->GetObject());
		}
		else
		{
			// Shoot at our target if we can see it or if we've seen it recently

			if ( GetAI()->GetTarget()->IsVisiblePartially() )
			{
				// TODO: check for strategy failure
				m_pStrategyShoot->Update();
			}
		}
	}

	GetAI()->FaceTarget();

	// TODO: check for strategy failure
	m_pStrategyCover->Update();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if ( eStateUseCover == m_eState && m_pStrategyCover && m_pStrategyCover->IsCovered() )
	{
		CAINode* pCoverNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
		if ( pCoverNode )
		{
			if ( pCoverNode->GetCoverDamageCmd() )
			{
				SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), pCoverNode->GetCoverDamageCmd());
			}
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromCover::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "REEVALUATE") )
	{
		// Regardless of the outcome, reset our elapsed time (a bit of a hack) to simulate having "re-entered" this state

		m_fElapsedTime = 0.0f;

		// See if we have a new cover node we can use

		if ( GetAI()->HasTarget() )
		{
			CAINode* pCoverNode = g_pAINodeMgr->FindNearestCoverFromThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

			if ( pCoverNode )
			{
				char szBuffer[1024];
				sprintf(szBuffer, "ATTACKFROMCOVER RETRIES=%d DEST=%s", m_nRetries, g_pLTServer->GetStringData(pCoverNode->GetName()));
				SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), LTFALSE, szBuffer);
			}
		}

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		if ( GetType() != eStateAttackFromCover )
		{
            g_pLTServer->CPrint("DEST= only valid with ATTACKFROMCOVER message!");
			return;
		}

		CAINode *pCoverNode = g_pAINodeMgr->GetNode(szValue);

		if ( !pCoverNode )
		{
            g_pLTServer->CPrint("ATTACKFROMCOVER DEST=%s - Could not find a node by this name!", szValue);
			return;
		}
		else
		{
			if ( !pCoverNode->IsCover() )
			{
                g_pLTServer->CPrint("ATTACKFROMCOVER DEST=%s - Node offers no cover!", szValue);
				return;
			}
			else if ( pCoverNode->IsLocked() )
			{
                g_pLTServer->CPrint("ATTACKFROMCOVER DEST=%s - Cover node is already locked!", szValue);
				return;
			}
			else if ( !m_pStrategyFollowPath->Set(pCoverNode->GetPos()) )
			{
                g_pLTServer->CPrint("ATTACKFROMCOVER DEST=%s - Could not set a path to node!", szValue);
				return;
			}

			// Release any node we're currently holding

			g_pAINodeMgr->UnlockNode(m_dwCoverNode);

			// Claim the node as ours

			m_dwCoverNode = pCoverNode->GetID();
			g_pAINodeMgr->LockNode(m_dwCoverNode);

			// Choose our cover strategy

			SetCoverStrategy(GetRandomCoverStrategy(pCoverNode));

			// Goto the cover

			m_eState = eStateGotoCover;
		}
	}
	else if ( !_stricmp(szName, "RETRIES") )
	{
		// FOR INTERNAL USE ONLY

		m_nRetries = atoi(szValue);
	}
}

// ----------------------------------------------------------------------- //

CAIHumanStrategy::AIHumanStrategyType CAIHumanStateAttackFromCover::GetRandomCoverStrategy(CAINode* pNode)
{
	int nAvailableCovers = 0;
	uint32 adwCovers[128];
	uint32 dwCoverFlags = pNode->GetCoverFlags();

	for ( uint32 iCover = 0 ; iCover < CAINode::kNumCoverFlags ; iCover++ )
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
		return CAIHumanStrategy::eStrategyCoverBlind;
	}

	uint32 dwCover = adwCovers[GetRandom(0, nAvailableCovers-1)];

	if ( dwCover == CAINode::kCoverFlagDuck )		return CAIHumanStrategy::eStrategyCoverDuck;
	if ( dwCover ==	CAINode::kCoverFlagBlind )		return CAIHumanStrategy::eStrategyCoverBlind;
	if ( dwCover == CAINode::kCoverFlag1WayCorner )	return CAIHumanStrategy::eStrategyCover1WayCorner;
	if ( dwCover == CAINode::kCoverFlag2WayCorner )	return CAIHumanStrategy::eStrategyCover2WayCorner;

	_ASSERT(LTFALSE);
    g_pLTServer->CPrint("GetRandomCoverStrategy - couldn't get random cover node for node %s", g_pLTServer->GetStringData(pNode->GetName()));
	return CAIHumanStrategy::eStrategyCoverBlind;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromCover::SetCoverStrategy(CAIHumanStrategy::AIHumanStrategyType eStrategy)
{
	_ASSERT(CAINode::kInvalidNodeID != m_dwCoverNode);
	if ( CAINode::kInvalidNodeID == m_dwCoverNode ) return LTFALSE;

	if ( m_pStrategyCover )
	{
		FACTORY_DELETE(m_pStrategyCover);
		m_pStrategyCover = LTNULL;
	}

	switch ( eStrategy )
	{
		case CAIHumanStrategy::eStrategyCoverDuck:
		{
			m_pStrategyCover = FACTORY_NEW(CAIHumanStrategyCoverDuck);
			break;
		}
		case CAIHumanStrategy::eStrategyCoverBlind:
		{
			m_pStrategyCover = FACTORY_NEW(CAIHumanStrategyCoverBlind);
			break;
		}
		case CAIHumanStrategy::eStrategyCover1WayCorner:
		{
			m_pStrategyCover = FACTORY_NEW(CAIHumanStrategyCover1WayCorner);
			break;
		}
		case CAIHumanStrategy::eStrategyCover2WayCorner:
		{
			m_pStrategyCover = FACTORY_NEW(CAIHumanStrategyCover2WayCorner);
			break;
		}
		default:
		{
			_ASSERT(LTFALSE);
			return(LTFALSE);
		}
	}

	_ASSERT(m_pStrategyCover);

	if ( !m_pStrategyCover->Init(GetAI()) )
	{
		return LTFALSE;
	}

	CAINode* pNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
	if ( !pNode )
	{
		return LTFALSE;
	}

	m_pStrategyCover->SetCoverNode(pNode);

	m_pStrategyCover->SetCoverTime(GetAI()->GetBrain()->GetAttackFromCoverCoverTime(), GetAI()->GetBrain()->GetAttackFromCoverCoverTimeRandom());
	m_pStrategyCover->SetUncoverTime(GetAI()->GetBrain()->GetAttackFromCoverUncoverTime(), GetAI()->GetBrain()->GetAttackFromCoverUncoverTimeRandom());

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);
	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_INT(m_nRetries);
	LOAD_DWORD(m_dwCoverNode);

	// Instantiate the correct cover strategy

	LTBOOL bHadCoverStrategy;
	LOAD_BOOL(bHadCoverStrategy);

	if ( bHadCoverStrategy )
	{
		CAIHumanStrategy::AIHumanStrategyType eStrategy;
		LOAD_DWORD_CAST(eStrategy,CAIHumanStrategy::AIHumanStrategyType);

		if ( !SetCoverStrategy(eStrategy) )
		{
			return;
		}

		m_pStrategyCover->Load(hRead);
	}
	else
	{
		if ( m_pStrategyCover )
		{
			FACTORY_DELETE(m_pStrategyCover);
			m_pStrategyCover = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromCover::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_INT(m_nRetries);
	SAVE_DWORD(m_dwCoverNode);

	// Write out the strategy type so we know which one to instantiate

	if ( m_pStrategyCover )
	{
		SAVE_BOOL(LTTRUE);
		SAVE_DWORD(m_pStrategyCover->GetType());
		m_pStrategyCover->Save(hWrite);
	}
	else
	{
		SAVE_BOOL(LTFALSE);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_dwVantageNode = CAINode::kInvalidNodeID;
	m_eState = eStateUnset;
	m_fAttackTimer = 0.0f;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateAttackFromVantage::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	g_pAINodeMgr->UnlockNode(m_dwVantageNode);

	super::Destructor();
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

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Update()
{
	super::Update();

	if ( !GetAI()->GetCurrentWeapon() )
	{
		// If we don't have a weapon, panic.
		GetAI()->ChangeState("PANIC");
		return;
	}

	if ( m_bPlayFirstSound || (m_bFirstUpdate && (GetAI()->GetBrain()->GetAttackSoundChance() > GetRandom(0.0f, 1.0f))) )
	{
		GetAI()->PlaySound(aisCombAggr);
	}

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( eStateUnset == m_eState )
	{
		// We won't pick the node we're currently at because it's locked (by us!)

		CAINode *pVantageNode = g_pAINodeMgr->FindNearestVantageToThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
		if ( !pVantageNode )
		{
			GetAI()->ChangeState("ATTACK");
			return;
		}
		else if ( !m_pStrategyFollowPath->Set(pVantageNode->GetPos()) )
		{
			GetAI()->ChangeState("ATTACK");
			return;
		}

		// Release any node we're currently holding

		g_pAINodeMgr->UnlockNode(m_dwVantageNode);

		// Claim the node as ours

		m_dwVantageNode = pVantageNode->GetID();
		g_pAINodeMgr->LockNode(m_dwVantageNode);

		// Set our flags

		m_eState = eStateMoving;
	}

	if ( eStateMoving == m_eState )
	{
		UpdateMoving();
	}

	if ( eStateAttacking == m_eState )
	{
		UpdateAttacking();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_eState )
	{
		case eStateUnset:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		case eStateMoving:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);

			m_pStrategyFollowPath->UpdateAnimation();
		}
		break;

		case eStateAttacking:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);

			m_pStrategyShoot->UpdateAnimation();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateMoving()
{
	// TODO: check for strategy failure
	m_pStrategyFollowPath->Update();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		m_eState = eStateAttacking;
		m_fAttackTimer = GetAI()->GetBrain()->GetAttackFromVantageAttackTime() + GetRandom(0.0f,GetAI()->GetBrain()->GetAttackFromVantageAttackTimeRandom());
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::UpdateAttacking()
{
	// Check to see if our vantage is still valid

	CAINode* pNode = g_pAINodeMgr->GetNode(m_dwVantageNode);
	if (!pNode)
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}

	VantageStatus eVantageStatus = pNode->GetVantageStatus(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

	if ( eVantageStatusThreatInsideRadius == eVantageStatus )
	{
		HSTRING hstrReaction = pNode->GetVantageThreatRadiusReaction();
		if ( hstrReaction )
		{
            GetAI()->ChangeState(g_pLTServer->GetStringData(hstrReaction));
			return;
		}
	}

    m_fAttackTimer -= g_pLTServer->GetFrameTime();
	GetAI()->FaceTarget();

	if ( m_pStrategyShoot->IsReloading() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();

		// We're done

		return;
	}


	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload();

		// We're done

		return;
	}

	CAITarget *pTarget = GetAI()->GetTarget();
	HOBJECT hTarget = pTarget->GetObject();

	if ( pTarget->IsVisibleCompletely() )
	{
        CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
		if (pB && !pB->IsDead())
		{
			// TODO: check for strategy failure
			m_pStrategyShoot->Update();
		}
	}

	if ( m_fAttackTimer < 0.0f )
	{
		m_eState = eStateUnset;
	}
}


// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	if ( eStateAttacking == m_eState  )
	{
		CAINode* pVantageNode = g_pAINodeMgr->GetNode(m_dwVantageNode);
		if ( pVantageNode )
		{
			if ( pVantageNode->GetVantageDamageCmd() )
			{
				SendTriggerMsgToObject(GetAI(), GetAI()->GetObject(), pVantageNode->GetVantageDamageCmd());
			}
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromVantage::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "REEVALUATE") )
	{
		// Force a new vantage node selection next update
		m_fAttackTimer = -1.0f;

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);
	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD(m_dwVantageNode);
	LOAD_FLOAT(m_fAttackTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromVantage::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_dwVantageNode);
	SAVE_FLOAT(m_fAttackTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_eState = eStateUnset;
	m_dwViewNode = CAINode::kInvalidNodeID;
	m_fChaseTimer = 0.0f;

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateAttackFromView::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	g_pAINodeMgr->UnlockNode(m_dwViewNode);

	super::Destructor();
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

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_dwViewNode == CAINode::kInvalidNodeID )
	{
		if ( !FindView() )
		{
			GetAI()->ChangeState("ATTACKONSIGHT CHASEDELAY=TRUE");
			return;
		}
	}

	// If we have a target start doing our stuff

	switch ( m_eState )
	{
		case eStateMoving:
		{
			UpdateMoving();
		}
		break;

		case eStateAttacking:
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

	switch ( m_eState )
	{
		case eStateMoving:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);

			m_pStrategyFollowPath->UpdateAnimation();
		}
		break;

		case eStateAttacking:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);

			m_pStrategyShoot->UpdateAnimation();
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackFromView::FindView()
{
    CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
	if ( !pCharacter->HasLastVolume() )
	{
		return LTFALSE;
	}

	CAINode* pViewNode = pCharacter->GetLastVolume()->FindViewNode();
	if ( !pViewNode )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Set(pViewNode) )
	{
		return LTFALSE;
	}

	// Release any node we're currently holding

	g_pAINodeMgr->UnlockNode(m_dwViewNode);

	// Claim the node as ours

	m_dwViewNode = pViewNode->GetID();
	g_pAINodeMgr->LockNode(m_dwViewNode);

	// Goto the node

	m_eState = eStateMoving;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::UpdateMoving()
{
	// TODO: check for strategy failure
	m_pStrategyFollowPath->Update();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// Attack

		m_eState = eStateAttacking;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::UpdateAttacking()
{
	GetAI()->FaceTarget();

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload();
	}

	if ( m_pStrategyShoot->IsReloading() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
	else if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload();
	}
	else if ( GetAI()->GetTarget()->IsVisiblePartially() )
	{
		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
	else
	{
        m_fChaseTimer += g_pLTServer->GetFrameTime();
		if ( m_fChaseTimer > GetAI()->GetBrain()->GetAttackFromViewChaseTime() )
		{
			GetAI()->ChangeState("CHASE");
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		CAINode *pViewNode = g_pAINodeMgr->GetNode(szValue);

		if ( !pViewNode )
		{
            g_pLTServer->CPrint("ATTACKFROMVIEW DEST=%s - Could not find a node by this name!", szValue);
			return;
		}
		else
		{
			if ( pViewNode->IsLocked() )
			{
                g_pLTServer->CPrint("ATTACKFROMVIEW DEST=%s - Cover node is already locked!", szValue);
				return;
			}
			else if ( !m_pStrategyFollowPath->Set(pViewNode->GetPos()) )
			{
                g_pLTServer->CPrint("ATTACKFROMVIEW DEST=%s - Could not set a path to node!", szValue);
				return;
			}

			// Release any node we're currently holding

			g_pAINodeMgr->UnlockNode(m_dwViewNode);

			// Claim the node as ours

			m_dwViewNode = pViewNode->GetID();
			g_pAINodeMgr->LockNode(m_dwViewNode);

			// Goto the view node

			m_eState = eStateMoving;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);
	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD(m_dwViewNode);
	LOAD_FLOAT(m_fChaseTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackFromView::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_dwViewNode);
	SAVE_FLOAT(m_fChaseTimer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_dwCoverNode = CAINode::kInvalidNodeID;
}

void CAIHumanStateCover::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	g_pAINodeMgr->UnlockNode(m_dwCoverNode);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCover::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //

void CAIHumanStateCover::UpdateSenses()
{
	// We only care about very important senses.

	static SenseType astSenses[] =
	{
		stSeeEnemy,
//		stHearEnemyWeaponFire,
//		stHearEnemyWeaponImpact,
//		stHearAllyWeaponFire,
	};

	static int cSenses = sizeof(astSenses)/sizeof(SenseType);

	for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
	{
		GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Update()
{
	super::Update();

	// TODO: We need to wait for a target if we don't have one yet
	// TOOD: But right now we just go idle.

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_bFirstUpdate )
	{
		// $SOUND GetAI()->PlaySound("PANIC");
	}

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		CAINode *pCoverNode = g_pAINodeMgr->FindNearestCoverFromThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());
		if ( !pCoverNode )
		{
			// Couldnt' find an appropriate cover node

			GetAI()->ChangeState("ATTACK");
			return;
		}
		else if ( !m_pStrategyFollowPath->Set(pCoverNode->GetPos()) )
		{
			GetAI()->ChangeState("ATTACK");
			return;
		}

		// Release any node we're currently holding

		g_pAINodeMgr->UnlockNode(m_dwCoverNode);

		// Claim the node as ours

		m_dwCoverNode = pCoverNode->GetID();
		g_pAINodeMgr->LockNode(m_dwCoverNode);
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			GetAI()->FaceTarget();
		}
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		CAINode *pNode = g_pAINodeMgr->GetNode(m_dwCoverNode);
		if ( pNode && pNode->HasCoverObject() )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(pNode->GetCoverObject(), hObject) )
			{
				SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
				pNode->ClearCoverObject();
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::UpdateAnimation()
{
	super::UpdateAnimation();

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAnimationContext()->SetProp(aniCrouch);
		GetAnimationContext()->SetProp(aniDown);
	}
	else
	{
		GetAnimationContext()->SetProp(aniStand);
		GetAnimationContext()->SetProp(aniDown);

		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD(m_dwCoverNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCover::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_dwCoverNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_bAtPanicDestination = LTFALSE;
	m_dwPanicNode = CAINode::kInvalidNodeID;

	m_aniPanic = aniCrouch;

	m_bCanActivate = LTFALSE;
}

void CAIHumanStatePanic::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	g_pAINodeMgr->UnlockNode(m_dwPanicNode);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStatePanic::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Update()
{
	super::Update();

	// Make sure we still have our target

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	// Play our first sound

	if ( m_bPlayFirstSound )
	{
		// $SOUND GetAI()->PlaySound(aisPanic);
	}

	if ( m_ePose == ePoseSit )
	{
		m_bAtPanicDestination = LTTRUE;
	}

	// Pick our spot to run away to if we haven't done so yet

	if ( !m_bAtPanicDestination && m_pStrategyFollowPath->IsUnset() )
	{
		CAINode *pPanicNode = g_pAINodeMgr->FindNearestPanicFromThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

		if ( pPanicNode )
		{
			if ( !m_pStrategyFollowPath->Set(pPanicNode->GetPos()) )
			{
				m_bAtPanicDestination = LTTRUE;
			}
			else
			{
				// Release any node we're currently holding

				g_pAINodeMgr->UnlockNode(m_dwPanicNode);

				// Claim the node as ours

				m_dwPanicNode = pPanicNode->GetID();
				g_pAINodeMgr->LockNode(m_dwPanicNode);

				// Get the random panic animation

				m_aniPanic = GetRandomPanic(pPanicNode);
			}
		}
		else
		{
			m_bAtPanicDestination = LTTRUE;
		}
	}

	if ( m_bAtPanicDestination || m_pStrategyFollowPath->IsDone() )
	{
/*		if ( m_dwPanicNode != CAINode::kInvalidNodeID )
		{
			if ( g_pAINodeMgr->GetNode(m_dwPanicNode)->GetPanicFlags() & CAINode::kPanicFlagCrouch )
			{
				// bleh
			}
		}
*/
		if ( m_ePose != ePoseSit )
		{
			if ( GetAI()->GetTarget()->IsVisiblePartially() )
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
		CAINode *pNode = g_pAINodeMgr->GetNode(m_dwPanicNode);
		if ( pNode && pNode->GetPanicObject() )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(pNode->GetPanicObject(), hObject) )
			{
				SendTriggerMsgToObject(GetAI(), hObject, LTFALSE, "TRIGGER");
				pNode->ClearPanicObject();
			}
		}

		m_bAtPanicDestination = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniPanic);

	switch ( m_ePose )
	{
		case ePoseSit:
		{
			GetAnimationContext()->SetProp(aniSit);
		}
		break;

		case ePoseStand:
		{
			if ( m_bAtPanicDestination )
			{
				GetAnimationContext()->SetProp(m_aniPanic);
			}
			else
			{
				GetAnimationContext()->SetProp(aniStand);
				m_pStrategyFollowPath->UpdateAnimation();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //

const CAnimationProp& CAIHumanStatePanic::GetRandomPanic(CAINode* pNode) const
{
	int nAvailablePanics = 0;
	uint32 adwPanics[128];
	uint32 dwPanicFlags = pNode->GetPanicFlags();

	for ( uint32 iPanic = 0 ; iPanic < CAINode::kNumPanicFlags ; iPanic++ )
	{
		if ( dwPanicFlags & (1 << iPanic) )
		{
			adwPanics[nAvailablePanics++] = (1 << iPanic);
		}
	}

	if ( 0 == nAvailablePanics )
	{
		_ASSERT(LTFALSE);
        g_pLTServer->CPrint("GetRandomPanic - couldn't get random Panic action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
		return aniAlert1;
	}

	uint32 dwPanic = adwPanics[GetRandom(0, nAvailablePanics-1)];

	if ( dwPanic == CAINode::kPanicFlagStand )			return aniStand;
	if ( dwPanic == CAINode::kPanicFlagCrouch )			return aniCrouch;

	_ASSERT(LTFALSE);
    g_pLTServer->CPrint("GetRandomPanic - couldn't get Panic action for node %s", g_pLTServer->GetStringData(pNode->GetName()));
	return aniCrouch;
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEST") )
	{
		CAINode *pPanicNode = g_pAINodeMgr->GetNode(szValue);

		if ( !pPanicNode )
		{
            g_pLTServer->CPrint("PANIC DEST=%s - Could not find a node by this name!", szValue);
			return;
		}
		else
		{
			if ( !pPanicNode->IsPanic() )
			{
                g_pLTServer->CPrint("PANIC DEST=%s - Node has no panic options!", szValue);
				return;
			}
			else if ( pPanicNode->IsLocked() )
			{
                g_pLTServer->CPrint("PANIC DEST=%s - Node is already locked!", szValue);
				return;
			}
			else if ( !m_pStrategyFollowPath->Set(pPanicNode->GetPos()) )
			{
                g_pLTServer->CPrint("PANIC DEST=%s - Could not set a path to node!", szValue);
				return;
			}

			// Release any node we're currently holding

			g_pAINodeMgr->UnlockNode(m_dwPanicNode);

			// Claim the node as ours

			m_dwPanicNode = pPanicNode->GetID();
			g_pAINodeMgr->LockNode(m_dwPanicNode);

			// Get the random panic animation

			m_aniPanic = GetRandomPanic(pPanicNode);
		}
	}
	else if ( !_stricmp(szName, "CANACTIVATE") )
	{
		m_bCanActivate = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_aniPanic.Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD(m_dwPanicNode);
	LOAD_BOOL(m_bAtPanicDestination);
	LOAD_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStatePanic::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_aniPanic.Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_dwPanicNode);
	SAVE_BOOL(m_bAtPanicDestination);
	SAVE_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_nDistressLevel = 0;
	m_fDistress = 0.0f;

	m_bCanActivate = LTFALSE;
}

void CAIHumanStateDistress::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_ePose != ePoseSit )
	{
		GetAI()->FaceTarget();
	}

	// If our target is aiming at us, elevate distress

	LTBOOL bDistress = false;
	if ( GetAI()->GetTarget()->IsVisiblePartially() )
	{
        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		if ( pCharacter->HasDangerousWeapon() )
		{
			LTRotation rRot;
			LTVector vNull, vForward;
            g_pLTServer->GetObjectRotation(GetAI()->GetTarget()->GetObject(), &rRot);
			g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

			LTVector vDir;
			vDir = GetAI()->GetPosition() - GetAI()->GetTarget()->GetPosition();
			vDir.y = 0;
			vDir.Norm();

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
		LTFLOAT fIncreaseRate = GetAI()->GetBrain()->GetDistressIncreaseRate();
        m_fDistress += g_pLTServer->GetFrameTime()*fIncreaseRate;
	}
	else
	{
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
		ReturnOr("IDLE");
		return;
	}

	// If distress level is max, panic

	const static int32 nLevels = GetAI()->GetBrain()->GetDistressLevels();
	if ( m_nDistressLevel == nLevels )
	{
		GetAI()->ChangeState("PANIC");
		return;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_ePose )
	{
		case ePoseSit:
			GetAnimationContext()->SetProp(aniSit);
			break;

		case ePoseStand:
			GetAnimationContext()->SetProp(aniStand);
			break;

		default:
			_ASSERT(LTFALSE);
			break;
	}

	GetAnimationContext()->SetProp(aniDistress);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "CANACTIVATE") )
	{
		m_bCanActivate = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_INT(m_nDistressLevel);
	LOAD_FLOAT(m_fDistress);
	LOAD_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDistress::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_INT(m_nDistressLevel);
	SAVE_FLOAT(m_fDistress);
	SAVE_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_bSendingTrigger = LTFALSE;

	m_dwNode = CAINode::kInvalidNodeID;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Destructor()
{
	g_pAINodeMgr->UnlockNode(m_dwNode);

	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateGetBackup::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Update()
{
	super::Update();

	if ( m_bPlayFirstSound )
	{
		GetAI()->PlaySound(aisBackup);
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		// If we're done, do the backup command

		CAINode* pAINode = g_pAINodeMgr->GetNode(m_dwNode);

		m_bSendingTrigger = LTTRUE;

		if ( pAINode )
		{
			SendMixedTriggerMsgToObject(GetAI(), GetAI()->GetObject(), pAINode->GetBackupCmd());
		}

		m_bSendingTrigger = LTFALSE;


		NextOr("ATTACK");
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( HasNext() )
	{
        g_pLTServer->CPrint("GetBackup -- NEXT= specified, this could have dangerous effects!!!!");
	}

	if ( !_stricmp(szName, "DEST") )
	{
		CAINode* pAINode = g_pAINodeMgr->GetNode(szValue);
		if ( !pAINode )
		{
            g_pLTServer->CPrint("GETBACKUP DEST=%s -- unable to find node by this name!", szValue);
			return;
		}

		if ( !pAINode->HasBackupCmd() )
		{
            g_pLTServer->CPrint("GETBACKUP DEST=%s -- this node has no backupcmd!", szValue);
			return;
		}

		if ( pAINode->IsLocked() )
		{
            g_pLTServer->CPrint("GETBACKUP DEST=%s - Cover node is already locked!", szValue);
			return;
		}

		if ( m_pStrategyFollowPath->Set(pAINode) )
		{
			m_dwNode = pAINode->GetID();
			pAINode->Lock();
		}
		else
		{
            g_pLTServer->CPrint("GETBACKUP OBJECT=%s -- unable to find path!", szValue);
		}
	}
	else if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateGetBackup::DelayChangeState()
{
	// Delay if we're sending

	return m_bSendingTrigger;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyShoot->Load(hRead);

	LOAD_DWORD(m_dwNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateGetBackup::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyShoot->Save(hWrite);

	SAVE_DWORD(m_dwNode);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Constructor()
{
	super::Constructor();

	m_bNoCinematics = LTFALSE;

	m_eMood = eMoodHappy;

	m_hFace = LTNULL;

	m_fFaceTime = 15.0f;
}

void CAIHumanStateTalk::Destructor()
{
	super::Destructor();

	GetAI()->FaceDir(m_vInitialForward);

	GetAI()->Unlink(m_hFace);
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
			GetAI()->FaceObject(m_hFace);
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::UpdateAnimation()
{
	super::UpdateAnimation();

	switch ( m_ePose )
	{
		case ePoseStand:
			GetAnimationContext()->SetProp(aniStand);
			break;

		case ePoseSit:
			GetAnimationContext()->SetProp(aniSit);
			break;
	}

	GetAnimationContext()->SetProp(aniLower);

	if ( GetAI()->IsPlayingDialogSound() )
	{
		switch ( m_eMood )
		{
			case eMoodHappy:
				GetAI()->GetAnimationContext()->SetProp(aniHappy);
				break;

			case eMoodAngry:
				GetAI()->GetAnimationContext()->SetProp(aniAngry);
				break;

			case eMoodSad:
				GetAI()->GetAnimationContext()->SetProp(aniSad);
				break;

			case eMoodTense:
				GetAI()->GetAnimationContext()->SetProp(aniTense);
				break;

			case eMoodAgree:
				GetAI()->GetAnimationContext()->SetProp(aniAgree);
				break;

			case eMoodDisagree:
				GetAI()->GetAnimationContext()->SetProp(aniDisagree);
				break;
		}
	}
	else
	{
		GetAI()->GetAnimationContext()->SetProp(aniSilent);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hFace )
	{
		m_hFace = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateTalk::HandleCommand(char** pTokens, int nArgs)
{
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "HAPPY") )
	{
		m_eMood = eMoodHappy;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "ANGRY") )
	{
		m_eMood = eMoodAngry;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "SAD") )
	{
		m_eMood = eMoodSad;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "TENSE") )
	{
		m_eMood = eMoodTense;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "AGREE") )
	{
		m_eMood = eMoodAgree;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "DISAGREE") )
	{
		m_eMood = eMoodDisagree;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "MOOD") )
	{
		if ( !_stricmp(szValue, "HAPPY") )
		{
			m_eMood = eMoodHappy;
		}
		else if ( !_stricmp(szValue, "ANGRY") )
		{
			m_eMood = eMoodAngry;
		}
		else if ( !_stricmp(szValue, "SAD") )
		{
			m_eMood = eMoodSad;
		}
		else if ( !_stricmp(szValue, "TENSE") )
		{
			m_eMood = eMoodTense;
		}
		else if ( !_stricmp(szValue, "AGREE") )
		{
			m_eMood = eMoodAgree;
		}
		else if ( !_stricmp(szValue, "DISAGREE") )
		{
			m_eMood = eMoodDisagree;
		}
	}
	else if ( !_stricmp(szName, "FACE") )
	{
		GetAI()->Unlink(m_hFace);
		m_hFace = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hFace) )
		{
			GetAI()->Link(m_hFace);
		}
	}
	else if ( !_stricmp(szName, "FACETIME") )
	{
		m_fFaceTime = (LTFLOAT)atof(szValue);
		m_fFaceTimer = g_pLTServer->GetTime() + m_fFaceTime;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_eMood, Mood);
	LOAD_HOBJECT(m_hFace);
	LOAD_FLOAT(m_fFaceTimer);
	LOAD_FLOAT(m_fFaceTime);
	LOAD_VECTOR(m_vInitialForward);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateTalk::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_eMood);
	SAVE_HOBJECT(m_hFace);
	SAVE_FLOAT(m_fFaceTimer);
	SAVE_FLOAT(m_fFaceTime);
	SAVE_VECTOR(m_vInitialForward);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_bAttacking = LTFALSE;
	m_fAttackDistanceSqr = 0.0f;
	m_bYelled = LTFALSE;
	m_fYellDistanceSqr = 0.0f;
	m_fStopDistanceSqr = 0.0f;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_abCanChangeToState[eStateSearch] = LTFALSE;
}

void CAIHumanStateCharge::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCharge::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);
	m_pStrategyFollowPath->SetUrgency(CAIHumanStrategyFollowPath::eUrgencyAggressive);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}

	// See how close we are to our target

	HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
	LTVector vTargetPos;
    g_pLTServer->GetObjectPos(hTarget, &vTargetPos);
	LTFLOAT fTargetDistanceSqr = VEC_DISTSQR(vTargetPos, GetAI()->GetPosition());

	if ( (fTargetDistanceSqr < m_fStopDistanceSqr) || m_pStrategyFollowPath->IsDone() )
	{
		GetAI()->ChangeState("ATTACK");
		return;
	}
	else
	{
		if ( m_pStrategyFollowPath->IsUnset() )
		{
			if ( !m_pStrategyFollowPath->Set(vTargetPos) )
			{
				GetAI()->ChangeState("ATTACK");
				return;
			}
		}

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( !m_bAttacking )
	{
		if ( fTargetDistanceSqr < m_fAttackDistanceSqr )
		{
			m_bAttacking = LTTRUE;
		}
		else
		{
			if ( !m_bYelled && (fTargetDistanceSqr < m_fYellDistanceSqr) )
			{
				m_bYelled = LTTRUE;
				// $SOUND GetAI()->PlaySound(aisCharge);
			}
		}
	}

	if ( m_bAttacking )
	{
		if ( m_pStrategyShoot->NeedsReload() )
		{
			m_pStrategyShoot->Reload(LTTRUE);
		}

		// TODO: check for strategy failure
		m_pStrategyShoot->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	m_pStrategyShoot->UpdateAnimation();
	m_pStrategyFollowPath->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "ATTACKDIST") )
	{
		m_fAttackDistanceSqr = (LTFLOAT)atof(szValue);
		m_fAttackDistanceSqr *= m_fAttackDistanceSqr;
	}
	else if ( !_stricmp(szName, "YELLDIST") )
	{
		m_fYellDistanceSqr = (LTFLOAT)atof(szValue);
		m_fYellDistanceSqr *= m_fYellDistanceSqr;
	}
	else if ( !_stricmp(szName, "STOPDIST") )
	{
		m_fStopDistanceSqr = (LTFLOAT)atof(szValue);
		m_fStopDistanceSqr *= m_fStopDistanceSqr;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyShoot->Load(hRead);

	LOAD_BOOL(m_bAttacking);
	LOAD_FLOAT(m_fAttackDistanceSqr);
	LOAD_BOOL(m_bYelled);
	LOAD_FLOAT(m_fYellDistanceSqr);
	LOAD_BOOL(m_bStopped);
	LOAD_FLOAT(m_fStopDistanceSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCharge::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyShoot->Save(hWrite);

	SAVE_BOOL(m_bAttacking);
	SAVE_FLOAT(m_fAttackDistanceSqr);
	SAVE_BOOL(m_bYelled);
	SAVE_FLOAT(m_fYellDistanceSqr);
	SAVE_BOOL(m_bStopped);
	SAVE_FLOAT(m_fStopDistanceSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Constructor()
{
	super::Constructor();

	m_pStrategyFlashlight = FACTORY_NEW(CAIHumanStrategyFlashlight);

	m_bNoCinematics = LTFALSE;

	m_bLoop = LTFALSE;

	m_hstrAnim = LTNULL;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Destructor()
{
	GetAI()->GetAnimationContext()->StopSpecial();

	FREE_HSTRING(m_hstrAnim);

	FACTORY_DELETE(m_pStrategyFlashlight);

	super::Destructor();
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

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

HSTRING CAIHumanStateAnimate::CreateReturnString()
{
	char szNext[2048];

	if ( HasNext() )
	{
		for ( int iNext = 0 ; iNext < m_cNexts ; iNext++ )
		{
			strcat(szNext, g_pLTServer->GetStringData(m_ahstrNexts[iNext]));
			strcat(szNext, ";");
		}

		szNext[strlen(szNext)-1] = 0;
	}
	else
	{
		strcpy(szNext, "NEXT");
	}

	char szBuffer[4096];
	sprintf(szBuffer, "GOTO PT=%f,%f,%f NEXT=(FACEDIR %f,%f,%f) MOVE=WALK NEXT=(ANIMATE ANIM=%s LOOP=%s NEXT=%s INTERRUPT=%s)",
		EXPANDVEC(GetAI()->GetPosition()),
		EXPANDVEC(GetAI()->GetForwardVector()),
		g_pLTServer->GetStringData(m_hstrAnim),
		m_bLoop ? "TRUE" : "FALSE",
		szNext,
		m_bInterrupt ? "TRUE" : "FALSE");

    return g_pLTServer->CreateString(szBuffer);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "ANIM") )
	{
		FREE_HSTRING(m_hstrAnim);
		m_hstrAnim = g_pLTServer->CreateString(szValue);

		GetAI()->GetAnimationContext()->SetSpecial(szValue);
	}
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(szValue[0]);
	}

	// Can't loop and have a NEXT=

	_ASSERT(!(m_bLoop && m_cNexts>0));
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Update()
{
	super::Update();

	m_pStrategyFlashlight->Update();

	if ( m_bFirstUpdate )
	{
		if ( m_bLoop )
		{
			GetAI()->GetAnimationContext()->LoopSpecial();
		}
		else
		{
			GetAI()->GetAnimationContext()->PlaySpecial();
		}
	}

	if ( !m_bLoop && GetAI()->GetAnimationContext()->IsSpecialDone() )
	{
		NextOr("IDLE");
		return;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFlashlight->Load(hRead);

	LOAD_BOOL(m_bLoop);
	LOAD_HSTRING(m_hstrAnim);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAnimate::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFlashlight->Save(hWrite);

	SAVE_BOOL(m_bLoop);
	SAVE_HSTRING(m_hstrAnim);
}
/*
// ----------------------------------------------------------------------- //

void CAIHumanStateCome::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
}

void CAIHumanStateCome::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateCome::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( GetAI()->IsScuba() )
	{
		m_pStrategyFollowPath->SetMovement(aniSwim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(aniWalk);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCome::Update()
{
	super::Update();

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		LTVector vPlayerPosition;
        g_pLTServer->GetObjectPos(g_pCharacterMgr->FindPlayer()->m_hObject, &vPlayerPosition);

		if ( !m_pStrategyFollowPath->Set(vPlayerPosition) )
		{
			_ASSERT(!"Could not set path to player");
			GetAI()->ChangeState("IDLE");
			return;
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCome::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCome::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateCome::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
}
*/
// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_eState = eStateFollowing;

	m_fRangeTime = .25f;

	m_fTimer = 0.0f;

	m_fRangeSqr = 100.0f*100.0f;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateFollow::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( GetAI()->IsScuba() )
	{
		m_pStrategyFollowPath->SetMovement(aniSwim);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
	}
	else
	{
		m_pStrategyFollowPath->SetMovement(aniRun);
		m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
	}

	m_fRangeTimer = m_fRangeTime + g_pLTServer->GetTime();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		// Target's dead???
		GetAI()->ChangeState("PANIC");
		return;
	}

	LTVector vTargetPosition;
    g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPosition);

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
		{
			_ASSERT(!"Could not set path to target");
			GetAI()->FaceTarget();
			return;
		}
	}

	if ( eStateHolding == m_eState )
	{
		if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) > (m_fRangeSqr+1000.0f) )
		{
			if ( g_pLTServer->GetTime() > m_fRangeTimer )
			{
			}
			else
			{
				GetAI()->FaceTarget();
				return;
			}
		}
		else
		{
			m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;
			GetAI()->FaceTarget();
			return;
		}
	}
	else if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < m_fRangeSqr )
	{
		m_fRangeTimer = g_pLTServer->GetTime() + m_fRangeTime;
		m_eState = eStateHolding;
		GetAI()->FaceTarget();
		return;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		m_eState = eStateFollowing;

		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}

    m_fTimer += g_pLTServer->GetFrameTime();

	if ( m_pStrategyFollowPath->IsDone() || m_fTimer > 1.0f )
	{
		m_fTimer = 0.0f;

		if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
		{
			m_eState = eStateHolding;
			GetAI()->FaceTarget();
			return;
		}
/*
		if ( m_pStrategyFollowPath->IsSet() )
		{
			if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < 10000.0f )
			{
				m_eState = eStateHolding;
				GetAI()->FaceTarget();
				return;
			}

			// TODO: check for strategy failure
			m_pStrategyFollowPath->Update();
		}
*/
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniDown);

	if ( eStateFollowing == m_eState )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "MOVE") )
	{
		char* szMove = strtok(szValue, ",");

		if ( !_stricmp(szMove, "WALK") )
		{
			m_pStrategyFollowPath->SetMovement(aniWalk);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
		}
		else if ( !_stricmp(szMove, "RUN") )
		{
			m_pStrategyFollowPath->SetMovement(aniRun);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
		}
		else if ( !_stricmp(szMove, "SWIM") )
		{
			m_pStrategyFollowPath->SetMovement(aniSwim);
			m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumUnderwater);
		}
	}
	else if ( !_stricmp(szName, "RANGETIME") )
	{
		m_fRangeTimer = m_fRangeTime = (LTFLOAT)atof(szValue);
	}
	else if ( !_stricmp(szName, "RANGE") )
	{
		m_fRangeSqr = (LTFLOAT)atof(szValue)*(LTFLOAT)atof(szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_FLOAT(m_fRangeTimer);
	LOAD_FLOAT(m_fRangeTime);
	LOAD_FLOAT(m_fTimer);
	LOAD_FLOAT(m_fRangeSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateFollow::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fRangeTimer);
	SAVE_FLOAT(m_fRangeTime);
	SAVE_FLOAT(m_fTimer);
	SAVE_FLOAT(m_fRangeSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDive::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;
}

void CAIHumanStateParaDive::Destructor()
{
	GetAI()->GetAnimationContext()->StopSpecial();

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateParaDive::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDive::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->GetAnimationContext()->SetSpecial("ParaDive");
		GetAI()->GetAnimationContext()->LoopSpecial();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDive::HandleTouch(HOBJECT hObject)
{
	super::HandleTouch(hObject);

	if ( IsCharacter(hObject) )
	{
		DamageStruct damage;

		damage.eType	= DT_CRUSH;
		damage.fDamage	= 25.0f + 25.0f*GetDifficultyFactor();
		damage.hDamager = GetAI()->GetObject();
		damage.vDir		= GetAI()->GetForwardVector();

		damage.DoDamage(GetAI(), hObject);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaShoot::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);
}

void CAIHumanStateParaShoot::Destructor()
{
	GetAI()->GetAnimationContext()->StopSpecial();

	FACTORY_DELETE(m_pStrategyShoot);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateParaShoot::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaShoot::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		_ASSERT(LTFALSE);
		return;
	}

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload(LTTRUE);
	}

	// TODO: check for strategy failure
	m_pStrategyShoot->Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->GetAnimationContext()->SetSpecial("ParaFire");
		GetAI()->GetAnimationContext()->LoopSpecial();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaShoot::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaShoot::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDie::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_eState = eStateOpenChute;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDie::Destructor()
{
	GetAI()->GetAnimationContext()->StopSpecial();

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateParaDie::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDie::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->GetAnimationContext()->SetSpecial("ParaDie");
		GetAI()->GetAnimationContext()->PlaySpecial();
	}

	if ( m_eState == eStateOpenChute )
	{
		CAttachmentPosition& Back = ((CHumanAttachments*)GetAI()->GetAttachments())->GetBack();
		if ( Back.HasAttachment() )
		{
			CAttachment* pAttachment = Back.GetAttachment();
			if ( !!pAttachment->GetModel() )
			{
				SendTriggerMsgToObject(GetAI(), pAttachment->GetModel(), LTFALSE, "ANIM OpenChute");
			}
		}

		m_eState = eStateDie;
	}

	if ( m_eState == eStateDie )
	{
		if ( GetAI()->GetAnimationContext()->IsSpecialDone() )
		{
			GetAI()->GetAnimationContext()->SetSpecial("ParaDied");
			GetAI()->GetAnimationContext()->LoopSpecial();

			m_eState = eStateDied;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDie::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaDie::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_eState);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaEscape::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_eState = eStateOpenChute;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaEscape::Destructor()
{
	GetAI()->GetAnimationContext()->StopSpecial();

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateParaEscape::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaEscape::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		GetAI()->GetAnimationContext()->SetSpecial("ParaEscape");
		GetAI()->GetAnimationContext()->PlaySpecial();
	}

	if ( m_eState == eStateOpenChute )
	{
		CAttachmentPosition& Back = ((CHumanAttachments*)GetAI()->GetAttachments())->GetBack();
		if ( Back.HasAttachment() )
		{
			CAttachment* pAttachment = Back.GetAttachment();
			if ( !!pAttachment->GetModel() )
			{
				SendTriggerMsgToObject(GetAI(), pAttachment->GetModel(), LTFALSE, "ANIM OpenChute");
			}
		}

		m_eState = eStateEscape;
	}

	if ( m_eState == eStateEscape )
	{
		if ( GetAI()->GetAnimationContext()->IsSpecialDone() )
		{
			GetAI()->GetAnimationContext()->SetSpecial("ParaEscaped");
			GetAI()->GetAnimationContext()->LoopSpecial();
			m_eState = eStateEscaped;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaEscape::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateParaEscape::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_eState);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::Constructor()
{
	super::Constructor();

	m_bAlert = LTTRUE;

	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_hHelicopter = LTNULL;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyShoot);

	GetAI()->Unlink(m_hHelicopter);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateHeliAttack::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyShoot->SetIgnoreFOV(LTTRUE);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::Update()
{
	super::Update();

	if ( GetAI()->GetHitPoints() <= 0.0f )
	{
		return;
	}

	if ( !GetAI()->HasTarget() || !m_hHelicopter )
	{
		return;
	}

    AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hHelicopter);

	if ( !pHelicopter->IsRightDoorOpen() )
	{
		return;
	}

	// Figure out if our target is in our fov

	LTVector vTargetPosition;
    g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPosition);

	LTVector vDir;
	vDir = vTargetPosition - pHelicopter->GetGunnerPosition();
	vDir.y = 0.0f;
	vDir.Norm();

	LTRotation rRot = pHelicopter->GetGunnerRotation();
	LTVector vForward, vNull;
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);
	vForward.y = 0.0f;
	vForward.Norm();

	if ( vDir.Dot(vForward) <= c_fFOV120 )
	{
		return;
	}

	if ( m_pStrategyShoot->NeedsReload() )
	{
		m_pStrategyShoot->Reload(LTTRUE);
	}

	m_pStrategyShoot->Update();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateHeliAttack::CanBeDamagedAsAttachment()
{
	if ( !m_hHelicopter ) return LTTRUE;

    AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hHelicopter);

	return super::CanBeDamagedAsAttachment() && pHelicopter->IsRightDoorOpen() && (GetAI()->GetHitPoints() > 0.0f);

}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniCrouch);
	GetAnimationContext()->SetProp(aniUp);

	m_pStrategyShoot->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyShoot->Load(hRead);

	LOAD_HOBJECT(m_hHelicopter);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);

	SAVE_HOBJECT(m_hHelicopter);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);
	if ( GetAI()->GetHitPoints() <= 0.0f )
	{
		// We're dead, play our death animation

		GetAI()->GetAnimationContext()->SetSpecial(GetAI()->GetCrouchDeathAni());
		GetAI()->GetAnimationContext()->LingerSpecial();
		GetAI()->SetInvincible(LTTRUE);
		return;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hHelicopter )
	{
		m_hHelicopter = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateHeliAttack::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( m_hHelicopter && !_stricmp(pTokens[0], "OPEN") )
	{
        AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hHelicopter);
		pHelicopter->OpenRightDoor();

		return LTTRUE;
	}
	else if ( m_hHelicopter && !_stricmp(pTokens[0], "CLOSE") )
	{
        AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hHelicopter);
		pHelicopter->CloseRightDoor();

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateHeliAttack::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "HELI") )
	{
		GetAI()->Unlink(m_hHelicopter);
		m_hHelicopter = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hHelicopter) )
		{
			if ( IsKindOf(m_hHelicopter, "AI_Helicopter") )
			{
				GetAI()->Link(m_hHelicopter);
			}
			else
			{
                g_pLTServer->CPrint("HELIATTACK HELI=%s -- this object is not an AI_Helicopter!", szValue);
				m_hHelicopter = LTNULL;
			}
		}
		else
		{
            g_pLTServer->CPrint("HELIATTACK HELI=%s::%s -- this object does not exist!", szValue, szValue);
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

static const LTFLOAT s_fAngerStimulationRate = 0.2f;	// per hit
static const LTFLOAT s_fAngerDecayRate = 0.10f;			// per second
static const LTFLOAT s_fAngerThreshhold = 5.0f;
static const LTFLOAT s_fBoxTime = 3.0f;

void CAIHumanStateScotBox::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyGrenade = FACTORY_NEW(CAIHumanStrategyGrenadeThrow);

	m_bAlert = LTTRUE;

	m_eState = eStateBoxing;

	m_fClosingTimer = 0.0f;
	m_fBoxTimer = s_fBoxTime;
	m_fAnger = 0.0f;

	m_hstrDefeat = LTNULL;

	m_hDamageMeter = LTNULL;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::Destructor()
{
	FREE_HSTRING(m_hstrDefeat);

	if ( m_hDamageMeter )
	{
		GetAI()->Unlink(m_hDamageMeter);
		g_pLTServer->RemoveObject(m_hDamageMeter);
		m_hDamageMeter = LTNULL;
	}

	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyGrenade);

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateScotBox::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyGrenade->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(aniRun);

	// Added checks to make load/save safe...

	CAIHuman* pAI = GetAI();
	if (!pAI) return LTFALSE;

	CWeapon* pWeapon = pAI->GetWeapon(0);
	if (pWeapon)
	{
		HOBJECT hScotPunch = pWeapon->GetModelObject();
		if (hScotPunch)
		{
			g_pLTServer->SetObjectFlags(hScotPunch, g_pLTServer->GetObjectFlags(hScotPunch) & ~FLAG_VISIBLE);
		}
	}

	pWeapon = pAI->GetWeapon(2);
	if (pWeapon)
	{
		HOBJECT hScotSlam = pWeapon->GetModelObject();
		if (hScotSlam)
		{
			g_pLTServer->SetObjectFlags(hScotSlam, g_pLTServer->GetObjectFlags(hScotSlam) & ~FLAG_VISIBLE);
		}
	}

	pAI->GetDestructible()->SetNeverDestroy(LTTRUE);
	pAI->SetCanShortRecoil(LTFALSE);
	pAI->GetDestructible()->SetCantDamageTypes(DamageTypeToFlag(DT_GADGET_DECAYPOWDER)|DamageTypeToFlag(DT_EXPLODE));

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_VERYHARD:
			m_fAnger += s_fAngerStimulationRate*0.5f;
			break;

		case GD_HARD:
			m_fAnger += s_fAngerStimulationRate*0.75f;
			break;

		case GD_EASY:
			m_fAnger += s_fAngerStimulationRate*1.5f;
			break;

		default :
		case GD_NORMAL:
			m_fAnger += s_fAngerStimulationRate*1.0f;
			break;

	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( !_stricmp(pArgList->argv[0], "PUNCH") )
	{
		if ( !GetAI()->HasTarget() ) return;

		WFireInfo fireInfo;
		fireInfo.hFiredFrom = GetAI()->GetObject();
		fireInfo.vPath		= GetAI()->GetForwardVector();
		fireInfo.vFirePos	= GetAI()->GetPosition();
		fireInfo.vFlashPos	= GetAI()->GetPosition();
		fireInfo.hTestObj	= GetAI()->GetTarget()->GetObject();
		fireInfo.fPerturbR	= 0;
		fireInfo.fPerturbU	= 0;

		if ( GetAI()->GetWeapon(0) )
		{
			GetAI()->GetWeapon(0)->UpdateWeapon(fireInfo, LTTRUE);
		}
	}
	else if ( !_stricmp(pArgList->argv[0], "SLAM") )
	{
		if ( !GetAI()->HasTarget() ) return;

        CPlayerObj* pPlayerObj = (CPlayerObj*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		if ( pPlayerObj->IsOnGround() )
		{
			WFireInfo fireInfo;
			fireInfo.hFiredFrom = GetAI()->GetObject();
			fireInfo.vPath		= GetAI()->GetForwardVector();
			fireInfo.vFirePos	= GetAI()->GetPosition();
			fireInfo.vFlashPos	= GetAI()->GetPosition();
			fireInfo.hTestObj	= GetAI()->GetTarget()->GetObject();
			fireInfo.fPerturbR	= 0;
			fireInfo.fPerturbU	= 0;

			if ( GetAI()->GetWeapon(2) )
			{
				GetAI()->GetWeapon(2)->UpdateWeapon(fireInfo, LTTRUE);
			}

			HCLIENT hClient = pPlayerObj->GetClient();
			if (hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SHAKE_SCREEN);
				g_pLTServer->WriteToMessageVector(hMessage, &LTVector(5, 5, 5));
				g_pLTServer->EndMessage(hMessage);
			}
		}

		static char* aszSlamSounds[] =  { "chars\\snd\\scothit.wav", "chars\\snd\\scothit2.wav" };
		uint32 cSlamSounds = (sizeof(aszSlamSounds)/sizeof(aszSlamSounds[0])) - 1;

        LTVector temp(GetAI()->GetPosition());
        g_pServerSoundMgr->PlaySoundFromPos(temp, aszSlamSounds[GetRandom(0, cSlamSounds)], 2000.0f, SOUNDPRIORITY_MISC_MEDIUM, 0);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( hObject == m_hDamageMeter )
	{
		m_hDamageMeter = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "DEFEAT") )
	{
		FREE_HSTRING(m_hstrDefeat);
        m_hstrDefeat = g_pLTServer->CreateString(szValue);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		sprintf(theStruct.m_Name, "scot-dm");
		theStruct.m_Pos = GetAI()->GetPosition();
		theStruct.m_Flags = 0;
		theStruct.m_ObjectType = OT_NORMAL;

		HCLASS hClass = g_pLTServer->GetClass("DisplayMeter");
		LPBASECLASS pDamageMeter = g_pLTServer->CreateObject(hClass, &theStruct);
		if ( pDamageMeter )
		{
			m_hDamageMeter = pDamageMeter->m_hObject;
			if ( m_hDamageMeter )
			{
				SendTriggerMsgToObject(GetAI(), m_hDamageMeter, LTFALSE, "set 100;show");
				GetAI()->Link(m_hDamageMeter);
			}
		}
	}

	if ( m_eState != eStateDefeat )
	{
	//	m_fAnger = Max<LTFLOAT>(m_fAnger - s_fAngerDecayRate*g_pLTServer->GetFrameTime(), 0.0f);
	//  g_pLTServer->CPrint("Anger = %f", m_fAnger);

		if ( !GetAI()->GetWeapon(0) || !GetAI()->GetWeapon(1) || !GetAI()->GetWeapon(2) )
		{
			g_pLTServer->CPrint("SCOT-BOX state needs proper weapons (Grenade, Scot Punch, and Scot Slam)");
			m_eState = eStateVictory;
		}
		else
		{
			// Need to do this here since weapons aren't saved/loaded....

			HOBJECT hWeaponModel = GetAI()->GetWeapon(0)->GetModelObject();
			if (hWeaponModel)
			{
				g_pLTServer->SetObjectFlags(hWeaponModel, g_pLTServer->GetObjectFlags(hWeaponModel) & ~FLAG_VISIBLE);
			}

			hWeaponModel = GetAI()->GetWeapon(1)->GetModelObject();
			if (hWeaponModel)
			{
				g_pLTServer->SetObjectFlags(hWeaponModel, g_pLTServer->GetObjectFlags(hWeaponModel) & ~FLAG_VISIBLE);
			}
		}


		if ( !GetAI()->HasTarget() )
		{
			g_pLTServer->CPrint("SCOT-BOX needs target (badly)");
			m_eState = eStateVictory;
		}
		else
		{
			CPlayerObj* pPlayerObj = (CPlayerObj*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
			if ( pPlayerObj->IsDead() )
			{
				m_eState = eStateVictory;
			}
		}
	}

	switch ( m_eState )
	{
		case eStateBoxing:		// Just standing there waiting to punch
			UpdateBoxing();
			break;

		case eStateVictory:		// After player loses
			UpdateVictory();
			break;

		case eStateDefeat:		// After scot loses
			UpdateDefeat();
			break;

		case eStateTaunting:	// After you lose
			UpdateTaunting();
			break;

		case eStatePunching:	// Throwing a punch
			UpdatePunching();
			break;

		case eStateClosing:		// Running to you after slamming the ground
			UpdateClosing();
			break;

		case eStateSlamming:	// Punching the ground
			UpdateSlamming();
			break;

		case eStateDynamite:	// Throwing dynamite
			UpdateDynamite();
			break;
	}

	if ( !GetAnimationContext()->IsLocked() && (m_fAnger > s_fAngerThreshhold) )
	{
		m_fAnger = s_fAngerThreshhold;
		m_eState = eStateDefeat;

		SendTriggerMsgToObject(GetAI(), m_hDamageMeter, LTFALSE, "set 0;hide");

		return;
	}

	if ( m_hDamageMeter && (m_eState != eStateDefeat) )
	{
		char szMessage[128];
		sprintf(szMessage, "set %d", Max<int>(1, int(100.0f*(1.0f - m_fAnger/s_fAngerThreshhold))));
		SendTriggerMsgToObject(GetAI(), m_hDamageMeter, LTFALSE, szMessage);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateDynamite()
{
	if ( m_pStrategyGrenade->IsThrowing() )
	{
		// TODO: check for strategy failure
		m_pStrategyGrenade->Update();

		return;
	}
	else
	{
		m_eState = eStateClosing;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateVictory()
{
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateDefeat()
{
	if ( m_hstrDefeat )
	{
        g_pCmdMgr->Process(g_pLTServer->GetStringData(m_hstrDefeat));
		FREE_HSTRING(m_hstrDefeat);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateTaunting()
{
	if ( !GetAnimationContext()->IsLocked() )
	{
		m_eState = eStateBoxing;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateBoxing()
{
	GetAI()->FaceTarget();

	const LTVector& vTargetPosition = GetAI()->GetTarget()->GetPosition();

	if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < 150.0f*150.0f )
	{
		static const CAnimationProp	s_aaniPunches[] =
		{
			aniPunch1, aniPunch2, aniPunch3, aniPunch4
		};
		static const uint32 s_cPunches = sizeof(s_aaniPunches)/sizeof(CAnimationProp);

		m_aniPunch = s_aaniPunches[GetRandom(0, s_cPunches-1)];
		m_eState = eStatePunching;
	}
	else
	{
		m_fBoxTimer -= g_pLTServer->GetFrameTime();
		if ( m_fBoxTimer < 0.0f )
		{
			m_fBoxTimer = s_fBoxTime;

			if ( GetRandom(0.0f, 1.0f) < 0.5f )
			{
				m_pStrategyGrenade->Throw(0.75f); // hangtime is 0.75 seconds
				m_eState = eStateDynamite;
			}
			else
			{
				m_eState = eStateSlamming;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdatePunching()
{
	GetAI()->FaceTarget();

	if ( !GetAnimationContext()->IsLocked() )
	{
		m_eState = eStateBoxing;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateSlamming()
{
	GetAI()->FaceTarget();

	if ( !GetAnimationContext()->IsLocked() )
	{
		m_fBoxTimer = s_fBoxTime;
		m_eState = eStateClosing;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateClosing()
{
	if ( GetRandom(0.0f, 1.0f) < .001f )
	{
		m_eState = eStateTaunting;
		return;
	}

	const LTVector& vTargetPosition = GetAI()->GetTarget()->GetPosition();
	if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < 75.0f*75.0f )
	{
		static const CAnimationProp	s_aaniPunches[] =
		{
			aniPunch1, aniPunch2, aniPunch3, aniPunch4
		};
		static const uint32 s_cPunches = sizeof(s_aaniPunches)/sizeof(CAnimationProp);

		m_aniPunch = s_aaniPunches[GetRandom(0, s_cPunches-1)];
		m_eState = eStatePunching;
	}
	else
	{
		// TAKE THIS FROM THE CAIHUMANSTATEFOLLOW::UPDATE

		LTVector vTargetPosition;
		g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPosition);

		if ( m_pStrategyFollowPath->IsUnset() )
		{
			if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
			{
				m_pStrategyGrenade->Throw(0.75f); // hangtime is 0.75 seconds
				m_eState = eStateDynamite;
				return;
			}
		}

		if ( m_pStrategyFollowPath->IsSet() )
		{
			// TODO: check for strategy failure
			m_pStrategyFollowPath->Update();
		}

		m_fClosingTimer += g_pLTServer->GetFrameTime();

		if ( m_pStrategyFollowPath->IsDone() || m_fClosingTimer > 1.0f )
		{
			m_fClosingTimer = 0.0f;

			if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
			{
				m_pStrategyGrenade->Throw(0.75f); // hangtime is 0.75 seconds
				m_eState = eStateDynamite;
				return;
			}

			if ( m_pStrategyFollowPath->IsSet() )
			{
				// TODO: check for strategy failure
				m_pStrategyFollowPath->Update();
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);

	switch ( m_eState )
	{
		case eStateDynamite:
			if ( m_pStrategyGrenade->IsThrowing() )
			{
				m_pStrategyGrenade->UpdateAnimation();
			}
			break;

		case eStateVictory:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniVictory);
			break;

		case eStateDefeat:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniDefeat);
			break;

		case eStateTaunting:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniTaunting);
			GetAnimationContext()->Lock();
			break;

		case eStateBoxing:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniBox);
			break;

		case eStatePunching:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(m_aniPunch);
			GetAnimationContext()->Lock();
			break;

		case eStateClosing:
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		case eStateSlamming:
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniSlam);
			GetAnimationContext()->Lock();
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyGrenade->Load(hRead);

	m_aniPunch.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_FLOAT(m_fClosingTimer);
	LOAD_FLOAT(m_fBoxTimer);
	LOAD_FLOAT(m_fAnger);
	LOAD_HOBJECT(m_hDamageMeter);
	LOAD_HSTRING(m_hstrDefeat);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateScotBox::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyGrenade->Save(hWrite);

	m_aniPunch.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fClosingTimer);
	SAVE_FLOAT(m_fBoxTimer);
	SAVE_FLOAT(m_fAnger);
	SAVE_HOBJECT(m_hDamageMeter);
	SAVE_HSTRING(m_hstrDefeat);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_bAlert = LTTRUE;

	m_eState = eStateClosing;

	m_bAtPower = LTFALSE;
	m_bPowerOn = LTTRUE;

	m_bSinging = LTFALSE;
	m_fSingingTimer = 0.0f;

	m_fClosingTimer = 0.0f;

	m_cHenchmen = 0;
	m_cActiveHenchmen = 0;
	m_iHenchmanNext = 0;

	for ( uint32 iHenchman = 0 ; iHenchman < kMaxHenchmen ; iHenchman++ )
	{
		m_ahHenchmen[iHenchman] = LTNULL;
	}

	m_cTeleporters = 0;

	for ( uint32 iTeleporter = 0 ; iTeleporter < kMaxTeleporters ; iTeleporter++ )
	{
		m_ahTeleporters[iTeleporter] = LTNULL;
	}

	m_cExplosions = 0;

	for ( uint32 iExplosion = 0 ; iExplosion < kMaxExplosions ; iExplosion++ )
	{
		m_ahExplosions[iExplosion] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	for ( uint32 iHenchman = 0 ; iHenchman < m_cHenchmen ; iHenchman++ )
	{
		if ( m_ahHenchmen[iHenchman] )
		{
			GetAI()->Unlink(m_ahHenchmen[iHenchman]);
			m_ahHenchmen[iHenchman] = LTNULL;
		}
	}

	for ( uint32 iTeleporter = 0 ; iTeleporter < m_cTeleporters ; iTeleporter++ )
	{
		if ( m_ahTeleporters[iTeleporter] )
		{
			GetAI()->Unlink(m_ahTeleporters[iTeleporter]);
			m_ahTeleporters[iTeleporter] = LTNULL;
		}
	}

	for ( uint32 iExplosion = 0 ; iExplosion < m_cExplosions ; iExplosion++ )
	{
		if ( m_ahExplosions[iExplosion] )
		{
			GetAI()->Unlink(m_ahExplosions[iExplosion]);
			m_ahExplosions[iExplosion] = LTNULL;
		}
	}

	super::Destructor();
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateIngeSing::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( GetAI()->GetWeapon(0) )
	{
		HOBJECT hGermanSword = GetAI()->GetWeapon(0)->GetModelObject();
		if ( hGermanSword )
		{
			g_pLTServer->SetObjectFlags(hGermanSword, g_pLTServer->GetObjectFlags(hGermanSword) & ~FLAG_VISIBLE);
		}
	}

	//TODO: maybe this could be cleaner
	GetAI()->GetDestructible()->SetCantDamageTypes(0xFFFFFFFF & ~DamageTypeToFlag(DT_ELECTROCUTE));
	GetAI()->SetCanShortRecoil(LTFALSE);

	g_pLTServer->SetObjectUserFlags(GetAI()->GetObject(), g_pLTServer->GetObjectUserFlags(GetAI()->GetObject()) | SurfaceToUserFlag((SurfaceType)20) );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateIngeSing::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
	if (!pTokens || nArgs < 1) return LTFALSE;

	if ( super::HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "POWERON") )
	{
		m_bPowerOn = LTTRUE;
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "POWEROFF") )
	{
		m_bPowerOn = LTFALSE;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "HENCHMEN") )
	{
		m_cHenchmen = 0;

		for ( uint32 iHenchman = 0 ; iHenchman < kMaxHenchmen ; iHenchman++ )
		{
			char szFullName[128];
			sprintf(szFullName, "%s%2.2d", szValue, iHenchman);

			if ( LT_OK == FindNamedObject(szFullName, m_ahHenchmen[iHenchman]) )
			{
				GetAI()->Unlink(m_ahHenchmen[iHenchman]);
				GetAI()->Link(m_ahHenchmen[iHenchman]);
				m_cHenchmen++;
			}
			else
			{
				break;
			}
		}
	}
	else if ( !_stricmp(szName, "TELEPORTERS") )
	{
		m_cTeleporters = 0;

		for ( uint32 iTeleporter = 0 ; iTeleporter < kMaxTeleporters ; iTeleporter++ )
		{
			char szFullName[128];
			sprintf(szFullName, "%s%2.2d", szValue, iTeleporter);

			if ( LT_OK == FindNamedObject(szFullName, m_ahTeleporters[iTeleporter]) )
			{
				GetAI()->Unlink(m_ahTeleporters[iTeleporter]);
				GetAI()->Link(m_ahTeleporters[iTeleporter]);
				m_cTeleporters++;
			}
			else
			{
				break;
			}
		}
	}
	else if ( !_stricmp(szName, "EXPLOSIONS") )
	{
		m_cExplosions = 0;

		for ( uint32 iExplosion = 0 ; iExplosion < kMaxExplosions ; iExplosion++ )
		{
			char szFullName[128];
			sprintf(szFullName, "%s%2.2d", szValue, iExplosion);

			if ( LT_OK == FindNamedObject(szFullName, m_ahExplosions[iExplosion]) )
			{
				GetAI()->Unlink(m_ahExplosions[iExplosion]);
				GetAI()->Link(m_ahExplosions[iExplosion]);
				m_cExplosions++;
			}
			else
			{
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	for ( uint32 iHenchman = 0 ; iHenchman < m_cHenchmen ; iHenchman++ )
	{
		if ( m_ahHenchmen[iHenchman] == hObject )
		{
			m_ahHenchmen[iHenchman] = LTNULL;

			m_cActiveHenchmen--;

			return;
		}
	}

	for ( uint32 iTeleporter = 0 ; iTeleporter < m_cTeleporters ; iTeleporter++ )
	{
		if ( m_ahTeleporters[iTeleporter] == hObject )
		{
			m_ahTeleporters[iTeleporter] = LTNULL;
			return;
		}
	}

	for ( uint32 iExplosion = 0 ; iExplosion < m_cExplosions ; iExplosion++ )
	{
		if ( m_ahExplosions[iExplosion] == hObject )
		{
			m_ahExplosions[iExplosion] = LTNULL;
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( !_stricmp(pArgList->argv[0], "SWING") )
	{
		if ( !GetAI()->HasTarget() ) return;

		WFireInfo fireInfo;
		fireInfo.hFiredFrom = GetAI()->GetObject();
		fireInfo.vPath		= GetAI()->GetForwardVector();
		fireInfo.vFirePos	= GetAI()->GetPosition();
		fireInfo.vFlashPos	= GetAI()->GetPosition();
		fireInfo.hTestObj	= GetAI()->GetTarget()->GetObject();
		fireInfo.fPerturbR	= 0;
		fireInfo.fPerturbU	= 0;

		if ( GetAI()->GetWeapon(0) )
		{
			GetAI()->GetWeapon(0)->UpdateWeapon(fireInfo, LTTRUE);
		}
	}
	if ( !_stricmp(pArgList->argv[0], "FOOTSTEP_KEY") )
	{
		CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
		if (pPlayer)
		{
			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
                HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SHAKE_SCREEN);
                g_pLTServer->WriteToMessageVector(hMessage, &LTVector(1, 1, 1));
                g_pLTServer->EndMessage(hMessage);
			}
		}

		static char* aszSlamSounds[] =  { "chars\\snd\\scothit.wav", "chars\\snd\\scothit2.wav" };
		uint32 cSlamSounds = (sizeof(aszSlamSounds)/sizeof(aszSlamSounds[0])) - 1;

        LTVector temp(GetAI()->GetPosition());
        g_pServerSoundMgr->PlaySoundFromPos(temp, aszSlamSounds[GetRandom(0, cSlamSounds)], 2000.0f, SOUNDPRIORITY_MISC_MEDIUM, 0);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() || !m_cHenchmen || !m_cTeleporters || !m_cExplosions )
	{
        g_pLTServer->CPrint("INGE-SING needs target, HENCHMEN= and TELEPORTERS= and EXPLOSIONS=");
	}
	else
	{
        CPlayerObj* pPlayerObj = (CPlayerObj*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		if ( pPlayerObj->IsDead() )
		{
		}
	}

	UpdateHenchmen();

	switch ( m_eState )
	{
		case eStatePower:
			UpdatePower();
			break;

		case eStateClosing:
			UpdateClosing();
			break;

		case eStateSwinging:
			UpdateSwinging();
			break;

		case eStateSinging:
			UpdateSinging();
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdateSwinging()
{
	if ( GetAI()->HasTarget() )
	{
		GetAI()->FaceTarget();
	}

	if ( !GetAnimationContext()->IsLocked() )
	{
		m_eState = eStateClosing;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdateSinging()
{
	if ( m_bSinging )
	{
		m_fSingingTimer += g_pLTServer->GetFrameTime();
		if ( !GetAI()->IsPlayingDialogSound() )
		{
			CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
			HCLIENT hClient = LTNULL;
			if (pPlayer)
			{
				hClient = pPlayer->GetClient();
			}

			for ( uint32 iExplosion = 0 ; iExplosion < kMaxExplosions ; iExplosion++ )
			{
				SendTriggerMsgToObject(GetAI(), m_ahExplosions[iExplosion], LTFALSE, "ON");

				if ( hClient )
				{
  //                  HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SHAKE_SCREEN);
//                    g_pLTServer->WriteToMessageVector(hMessage, &LTVector(.02f, .02f, .02f));
    //                g_pLTServer->EndMessage(hMessage);
				}
			}

			const static LTFLOAT s_fSingingTime = 10.0f;

			if ( m_bPowerOn && (m_fSingingTimer < s_fSingingTime) )
			{
				GetAI()->PlaySound(aisIngeSing);
			}
			else
			{
				CAINode* pNode = g_pAINodeMgr->GetNode("powernode");
				if ( !pNode ) return;

				m_pStrategyFollowPath->Set(pNode);

				for ( uint32 iHenchman = 0 ; iHenchman < m_iHenchmanNext ; iHenchman++ )
				{
					if ( m_ahHenchmen[iHenchman] )
					{
                        SendTriggerMsgToObject(GetAI(), m_ahHenchmen[iHenchman], LTFALSE, "TARGETPLAYER;ATTACK");
					}
				}

				m_fSingingTimer = 0;
				m_bSinging = LTFALSE;
				m_eState = eStateClosing;
			}
		}
	}
	else
	{
		GetAI()->PlaySound(aisIngeSing);

		for ( uint32 iHenchman = 0 ; iHenchman < m_iHenchmanNext ; iHenchman++ )
		{
			if ( m_ahHenchmen[iHenchman] )
			{
				char szMessage[1024];
				sprintf(szMessage, "FACEOBJECT (%s);ANIMATE ANIM=StPn LOOP=TRUE INTERRUPT=FALSE", g_pLTServer->GetObjectName(GetAI()->GetObject()));

                SendTriggerMsgToObject(GetAI(), m_ahHenchmen[iHenchman], LTFALSE, szMessage);
			}
		}

		m_bSinging = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdatePower()
{
	if ( m_bPowerOn )
	{
		m_bAtPower = LTFALSE;
		m_eState = eStateClosing;

		return;
	}

	if ( m_bAtPower )
	{
		CAINode* pNode = g_pAINodeMgr->GetNode("powernode");
		if ( !pNode ) return;

		GetAI()->FaceDir(pNode->GetForward());

		if ( !GetAnimationContext()->IsLocked() )
		{
			HOBJECT hSwitch;
            if ( LT_OK == FindNamedObject(g_pLTServer->GetStringData(pNode->GetUseObject()), hSwitch) )
			{
				SendTriggerMsgToObject(GetAI(), hSwitch, LTFALSE, "ACTIVATE");
			}
		}
	}
	else
	{
		m_pStrategyFollowPath->Update();

		if ( m_pStrategyFollowPath->IsDone() )
		{
			m_bAtPower = LTTRUE;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdateClosing()
{
	if ( GetRandom(0.0f, 1.0f) < .001f )
	{
		m_eState = eStateSinging;
		return;
	}

	if ( !m_bPowerOn )
	{
		CAINode* pNode = g_pAINodeMgr->GetNode("powernode");
		if ( !pNode ) return;

		m_pStrategyFollowPath->Set(pNode);

		m_eState = eStatePower;
	}

	const LTVector& vTargetPosition = GetAI()->GetTarget()->GetPosition();
	if ( vTargetPosition.DistSqr(GetAI()->GetPosition()) < 75.0f*75.0f )
	{
		static const CAnimationProp	s_aaniSwordes[] =
		{
			aniSword1, aniSword2,
		};
		static const uint32 s_cSwordes = sizeof(s_aaniSwordes)/sizeof(CAnimationProp);

		m_aniSword = s_aaniSwordes[GetRandom(0, s_cSwordes-1)];

		m_eState = eStateSwinging;
	}
	else
	{
		// TAKE THIS FROM THE CAIHUMANSTATEFOLLOW::UPDATE

		LTVector vTargetPosition;
		g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vTargetPosition);

		if ( m_pStrategyFollowPath->IsUnset() )
		{
			if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
			{
				_ASSERT(!"Could not set path to target");
				m_eState = eStateSinging;
				return;
			}
		}

		if ( m_pStrategyFollowPath->IsSet() )
		{
			// TODO: check for strategy failure
			m_pStrategyFollowPath->Update();
		}

		m_fClosingTimer += g_pLTServer->GetFrameTime();

		if ( m_pStrategyFollowPath->IsDone() || m_fClosingTimer > 1.0f )
		{
			m_fClosingTimer = 0.0f;

			if ( !m_pStrategyFollowPath->Set(vTargetPosition) )
			{
				_ASSERT(!"Could not set path to target");
				m_eState = eStateSinging;
				return;
			}

			if ( m_pStrategyFollowPath->IsSet() )
			{
				// TODO: check for strategy failure
				m_pStrategyFollowPath->Update();
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdateHenchmen()
{
	static LTFLOAT fSpawnTimer = 0.0f;
	const static uint32 s_nMaxActiveHenchmen = 3;
	const static LTFLOAT fSpawnTime = 2.0f;

	if ( m_cActiveHenchmen < s_nMaxActiveHenchmen )
	{
		fSpawnTimer += g_pLTServer->GetFrameTime();

		if ( fSpawnTimer > fSpawnTime )
		{
			if ( m_iHenchmanNext < m_cHenchmen )
			{
				HOBJECT hTeleporter = FindTeleporter();
				if ( hTeleporter )
				{
					const char* szName = g_pLTServer->GetObjectName(m_ahHenchmen[m_iHenchmanNext]);

					char szMessage[1024];
					if ( !m_bSinging )
					{
						sprintf(szMessage, "FACEOBJECT PLAYER;delay .1 (msg %s (TELEPORT %s));ANIMATE ANIM=LongFall LOOP=FALSE INTERRUPT=FALSE NEXT=TARGETPLAYER NEXT=ATTACK", szName, g_pLTServer->GetObjectName(hTeleporter));
					}
					else
					{
						sprintf(szMessage, "TARGETPLAYER;FACEOBJECT (%s);delay .1 (msg %s (TELEPORT %s));ANIMATE ANIM=LongFall LOOP=FALSE INTERRUPT=FALSE NEXT=TARGETPLAYER NEXT=(ANIMATE ANIM=StPn LOOP=TRUE INTERRUPT=FALSE)", g_pLTServer->GetObjectName(GetAI()->GetObject()), szName, g_pLTServer->GetObjectName(hTeleporter));
					}

					SendMixedTriggerMsgToObject(GetAI(), m_ahHenchmen[m_iHenchmanNext], LTFALSE, szMessage);

					m_cActiveHenchmen++;
					m_iHenchmanNext++;
				}
			}

			fSpawnTimer = 0.0f;
		}
	}
}

// ----------------------------------------------------------------------- //

HOBJECT CAIHumanStateIngeSing::FindTeleporter()
{
	HOBJECT hTeleporter = LTNULL;

	LTBOOL abValidTeleporters[kMaxTeleporters];
	memset(abValidTeleporters, LTTRUE, sizeof(LTBOOL)*kMaxTeleporters);

	uint32 cAttempts = 64;
	while ( cAttempts-- )
	{
		uint32 iTeleporter = GetRandom(0, m_cTeleporters-1);
		if ( !abValidTeleporters[iTeleporter] ) continue;

		HOBJECT hTeleporter = m_ahTeleporters[iTeleporter];
		if ( hTeleporter )
		{
			if ( IsTeleporterValid(hTeleporter) )
			{
				return hTeleporter;
			}
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateIngeSing::IsTeleporterValid(HOBJECT hTeleporter)
{
	LTVector vTeleporterPosition;
	LTFLOAT fTeleporterRadius = 48.0f;

	g_pLTServer->GetObjectPos(hTeleporter, &vTeleporterPosition);

    HCLASS hClass = g_pLTServer->GetClass("CCharacter");

    ObjectList* pObjectList = g_pLTServer->FindObjectsTouchingSphere(&vTeleporterPosition, fTeleporterRadius);
	ObjectLink* pObject = pObjectList ? pObjectList->m_pFirstLink : LTNULL;
	while ( pObject )
	{
		HOBJECT hObject = pObject->m_hObject;
        if ( g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), hClass) )
		{
			// A character is standing in this teleporter, so don't use it

			if ( pObjectList )
			{
				g_pLTServer->RelinquishList(pObjectList);
			}

			return LTFALSE;
		}

		pObject = pObject->m_pNext;
	}

	// We're clear, so use it

	if ( pObjectList )
	{
		g_pLTServer->RelinquishList(pObjectList);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniUp);

	switch ( m_eState )
	{
		case eStatePower:
			if ( m_bAtPower )
			{
				GetAnimationContext()->SetProp(aniLower);
				GetAnimationContext()->SetProp(aniPushButton);
				GetAnimationContext()->Lock();
			}
			else
			{
				m_pStrategyFollowPath->UpdateAnimation();
				GetAnimationContext()->Unlock();
			}
			break;

		case eStateClosing:
			GetAnimationContext()->SetProp(aniWalk);
			GetAnimationContext()->Unlock();
			break;

		case eStateSwinging:
			GetAnimationContext()->SetProp(m_aniSword);
			GetAnimationContext()->Lock();
			break;

		case eStateSinging:
			GetAnimationContext()->SetProp(aniSing);
			GetAnimationContext()->Unlock();
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	m_aniSword.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);

	LOAD_BOOL(m_bSinging);
	LOAD_FLOAT(m_fSingingTimer);

	LOAD_FLOAT(m_fClosingTimer);

	LOAD_BOOL(m_bAtPower);
	LOAD_BOOL(m_bPowerOn);

	LOAD_DWORD(m_cHenchmen);
	LOAD_DWORD(m_cActiveHenchmen);
	LOAD_DWORD(m_iHenchmanNext);

	for ( uint32 iHenchman = 0 ; iHenchman < kMaxHenchmen ; iHenchman++ )
	{
		LOAD_HOBJECT(m_ahHenchmen[iHenchman]);
	}

	LOAD_DWORD(m_cTeleporters);

	for ( uint32 iTeleporter = 0 ; iTeleporter < kMaxTeleporters ; iTeleporter++ )
	{
		LOAD_HOBJECT(m_ahTeleporters[iTeleporter]);
	}

	LOAD_DWORD(m_cExplosions);

	for ( uint32 iExplosion = 0 ; iExplosion < kMaxExplosions ; iExplosion++ )
	{
		LOAD_HOBJECT(m_ahExplosions[iExplosion]);
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateIngeSing::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	m_aniSword.Save(hWrite);

	SAVE_DWORD(m_eState);

	SAVE_BOOL(m_bSinging);
	SAVE_FLOAT(m_fSingingTimer);

	SAVE_FLOAT(m_fClosingTimer);

	SAVE_BOOL(m_bAtPower);
	SAVE_BOOL(m_bPowerOn);

	SAVE_DWORD(m_cHenchmen);
	SAVE_DWORD(m_cActiveHenchmen);
	SAVE_DWORD(m_iHenchmanNext);

	for ( uint32 iHenchman = 0 ; iHenchman < kMaxHenchmen ; iHenchman++ )
	{
		SAVE_HOBJECT(m_ahHenchmen[iHenchman]);
	}

	SAVE_DWORD(m_cTeleporters);

	for ( uint32 iTeleporter = 0 ; iTeleporter < kMaxTeleporters ; iTeleporter++ )
	{
		SAVE_HOBJECT(m_ahTeleporters[iTeleporter]);
	}

	SAVE_DWORD(m_cExplosions);

	for ( uint32 iExplosion = 0 ; iExplosion < kMaxExplosions ; iExplosion++ )
	{
		SAVE_HOBJECT(m_ahExplosions[iExplosion]);
	}
}
