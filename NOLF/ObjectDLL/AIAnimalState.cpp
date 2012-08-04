// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIAnimal.h"
#include "AIAnimalState.h"
#include "AIAnimalStrategy.h"
#include "AITarget.h"
#include "Alarm.h"
#include "AIDog.h"
#include "AIPoodle.h"
#include "AIShark.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "Spawner.h"
#include "AISense.h"
#include "AINodeMgr.h"

IMPLEMENT_FACTORY(CAIDogStateIdle, 0)
IMPLEMENT_FACTORY(CAIDogStateBark, 0)
IMPLEMENT_FACTORY(CAIDogStateExcited, 0)
IMPLEMENT_FACTORY(CAIDogStateHeat, 0)

IMPLEMENT_FACTORY(CAIPoodleStateShutdown, 0)
IMPLEMENT_FACTORY(CAIPoodleStateSeduce, 0)
IMPLEMENT_FACTORY(CAIPoodleStateDischarge, 0)

IMPLEMENT_FACTORY(CAISharkStateIdle, 0)
IMPLEMENT_FACTORY(CAISharkStateChase, 0)
IMPLEMENT_FACTORY(CAISharkStateGoto, 0)
IMPLEMENT_FACTORY(CAISharkStateBite, 0)
IMPLEMENT_FACTORY(CAISharkStateMaul, 0)
IMPLEMENT_FACTORY(CAISharkStateWait, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimalState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIAnimalState::Constructor()
{
	super::Constructor();

    m_pAIAnimal = LTNULL;
}

void CAIAnimalState::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimalState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIAnimalState::Init(CAIAnimal* pAIAnimal)
{
	if ( !super::Init(pAIAnimal) )
	{
        return LTFALSE;
	}

	m_pAIAnimal = pAIAnimal;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogState::Constructor()
{
	super::Constructor();

    m_pAIDog = LTNULL;
    m_pStrategyFollowPath = LTNULL;
    m_pStrategyOneShotAni = LTNULL;
}

void CAIDogState::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogState::Init(AI_Dog* pAIDog)
{
	if ( !super::Init(pAIDog) )
	{
        return LTFALSE;
	}

	m_pAIDog = pAIDog;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogState::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIDogState::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogState::UpdateSenses
//
//	PURPOSE:	Updates the AIs senses
//
// ----------------------------------------------------------------------- //

void CAIDogState::UpdateSenses()
{
	static SenseType astSenses[10] =
		{ stSeeEnemy,
//		  stSeeEnemyFootprint,
//		  stSeeEnemyFlashlight,
//		  stHearEnemyWeaponFire,
//		  stHearEnemyWeaponImpact,
//		  stHearEnemyFootstep,
//		  stHearEnemyDisturbance,
//		  stSeeAllyDeath,
//		  stHearAllyDeath,
//		  stHearAllyPain
//		  stHearAllyWeaponFire,
		};

	static int cSenses = sizeof(astSenses)/sizeof(SenseType);

	for ( int iSense = 0 ; iSense < cSenses ; iSense++ )
	{
		GetAI()->GetSenseMgr()->UpdateSense(astSenses[iSense]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateIdle::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStateIdle::Constructor()
{
	super::Constructor();

	m_ePose = CAnimatorAIAnimal::eIdleStand;
}

void CAIDogStateIdle::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateIdle::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIDogStateIdle::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_ePose, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateIdle::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIDogStateIdle::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_ePose);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateIdle::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIDogStateIdle::Update()
{
	super::Update();

	GetAI()->GetAnimator()->SetMain(m_ePose);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateIdle::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair
//
// ----------------------------------------------------------------------- //

void CAIDogStateIdle::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "POSE") )
	{
		if ( !_stricmp(szValue, "SIT") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleSit;
		}
		else if ( !_stricmp(szValue, "LAY") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleLay;
		}
		else if ( !_stricmp(szValue, "STAND") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleStand;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStateBark::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIDogStrategyFollowPath);
	m_pStrategyOneShotAni = FACTORY_NEW(CAIDogStrategyOneShotAni);

	m_dwNode = CAINode::kInvalidNodeID;
	m_eState = eMove;

	m_fPause = 0.0f;
}

void CAIDogStateBark::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
	FACTORY_DELETE(m_pStrategyOneShotAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStateBark::Init(AI_Dog* pAIDog)
{
	if ( !super::Init(pAIDog) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIDog) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIDog) )
	{
        return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(CAnimatorAIAnimal::eRunning);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIDogStateBark::Update()
{
	super::Update();

	if ( m_bFirstUpdate )
	{
		m_fPause = GetRandom(0.0f, 1.0f);
	}

	if ( GetElapsedTime() < m_fPause )
	{
		return;
	}

	if ( m_eState == eMove )
	{
		if ( m_pStrategyFollowPath->IsUnset() )
		{
			CAINode* pAINode = g_pAINodeMgr->FindNearestNode(GetAI()->GetPosition());
			if ( !pAINode )
			{
				goto Error;
			}

			m_dwNode = pAINode->GetID();

			if ( !m_pStrategyFollowPath->Set(pAINode) )
			{
				goto Error;
			}
		}

		if ( !m_pStrategyFollowPath->Update() )
		{
			goto Error;
		}

		if ( m_pStrategyFollowPath->IsDone() )
		{
			m_eState = eJump;

			GetAI()->Stop();

			if ( !m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eFenceJump) )
			{
				goto Error;
			}
		}
	}

	if ( m_eState == eMove )
	{
		return;
	}
	else
	{
		CAINode* pAINode = g_pAINodeMgr->GetNode(m_dwNode);
		if ( pAINode )
		{
			GetAI()->FaceDir(pAINode->GetForward());
		}
	}

	if ( m_eState == eJump )
	{
		if ( !m_pStrategyOneShotAni->Update() )
		{
			goto Error;
		}

		if ( !m_pStrategyOneShotAni->IsAnimating() )
		{
			m_eState = eBark;

			if ( !m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eFenceBark) )
			{
				goto Error;
			}
		}
	}

	if ( m_eState == eBark )
	{
		if ( GetAI()->HasBarked() )
		{
			if ( m_dwNode != CAINode::kInvalidNodeID )
			{
				CAINode* pAINode = g_pAINodeMgr->GetNode(m_dwNode);
				if ( pAINode && !!pAINode->GetBackupCmd() )
				{
					SendMixedTriggerMsgToObject(GetAI(), GetAI()->GetObject(), pAINode->GetBackupCmd());
					m_dwNode = CAINode::kInvalidNodeID;
				}
			}
		}

		if ( !m_pStrategyOneShotAni->Update() )
		{
			goto Error;
		}

		if ( !m_pStrategyOneShotAni->IsAnimating() )
		{
			m_eState = eUnjump;

			if ( !m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eFenceUnjump) )
			{
				goto Error;
			}
		}
	}

	if ( m_eState == eUnjump )
	{
		if ( !m_pStrategyOneShotAni->Update() )
		{
			goto Error;
		}

		if ( !m_pStrategyOneShotAni->IsAnimating() )
		{
			m_eState = eJump;

			if ( !m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eFenceJump) )
			{
				goto Error;
			}
		}
	}

	return;

Error:

	GetAI()->ChangeState("IDLE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIDogStateBark::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	m_pStrategyOneShotAni->Load(hRead);

	LOAD_DWORD(m_dwNode);
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_FLOAT(m_fPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIDogStateBark::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_DWORD(m_dwNode);
	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateBark::GetDeathAni
//
//	PURPOSE:	Gets our death ani
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIDogStateBark::GetDeathAni(LTBOOL bFront)
{
	static const char* s_aszDeaths[] =
	{
		"Death2", "Death2_sit", "Death2_laydown",
	};
	static const uint32 s_cDeaths = sizeof(s_aszDeaths)/sizeof(const char*);

	return g_pLTServer->GetAnimIndex(GetAI()->GetObject(), (char*)s_aszDeaths[GetRandom(0, s_cDeaths-1)]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateExcited::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStateExcited::Constructor()
{
	super::Constructor();
}

void CAIDogStateExcited::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateExcited::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIDogStateExcited::Update()
{
	super::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateHeat::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStateHeat::Constructor()
{
	super::Constructor();
}

void CAIDogStateHeat::Destructor()
{
	GetAI()->DestroyHearts();

	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateHeat::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStateHeat::Init(AI_Dog* pAIDog)
{
	if ( !super::Init(pAIDog) )
	{
        return LTFALSE;
	}

	GetAI()->CreateHearts();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStateHeat::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIDogStateHeat::Update()
{
	super::Update();

	GetAI()->GetAnimator()->SetMain(CAnimatorAIAnimal::eSniffPoodle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleState::Constructor()
{
	super::Constructor();

    m_pAIPoodle = LTNULL;
    m_pStrategyFollowPath = LTNULL;
}

void CAIPoodleState::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleState::Init(AI_Poodle* pAIPoodle)
{
	if ( !super::Init(pAIPoodle) )
	{
        return LTFALSE;
	}

	m_pAIPoodle = pAIPoodle;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleState::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIPoodleState::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleState::UpdateSenses
//
//	PURPOSE:	Updates the AIs senses
//
// ----------------------------------------------------------------------- //

void CAIPoodleState::UpdateSenses()
{
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateSeduce::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateSeduce::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIPoodleStrategyFollowPath);
	m_dwNode = CAINode::kInvalidNodeID;
}

void CAIPoodleStateSeduce::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateSeduce::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStateSeduce::Init(AI_Poodle* pAIPoodle)
{
	if ( !super::Init(pAIPoodle) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIPoodle) )
	{
        return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(CAnimatorAIAnimal::eRunning);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateSeduce::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateSeduce::Update()
{
	super::Update();

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		CAINode* pAINode = g_pAINodeMgr->FindNearestPoodleNode(GetAI()->GetPosition());
		if ( !pAINode || !m_pStrategyFollowPath->Set(pAINode) )
		{
			goto Error;
		}

		m_dwNode = pAINode->GetID();
	}

	if ( m_pStrategyFollowPath->IsSet() && !m_pStrategyFollowPath->Update() )
	{
		goto Error;
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		CAINode* pAINode = g_pAINodeMgr->GetNode(m_dwNode);
		if ( !pAINode )
		{
			goto Error;
		}

		if ( GetAI()->FaceDir(pAINode->GetForward()) )
		{
			GetAI()->ChangeState("DISCHARGE");
		}
	}

	return;

Error:

	GetAI()->ChangeState("SHUTDOWN");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateSeduce::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateSeduce::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_DWORD(m_dwNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateSeduce::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateSeduce::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_DWORD(m_dwNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateDischarge::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateDischarge::Constructor()
{
	super::Constructor();

    m_bDischarged = LTFALSE;
}

void CAIPoodleStateDischarge::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateDischarge::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStateDischarge::Init(AI_Poodle* pAIPoodle)
{
	if ( !super::Init(pAIPoodle) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateDischarge::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateDischarge::Update()
{
	super::Update();

	CTList<AI_Dog*> lstDogs;
	AI_Dog** pCur;

	if ( !m_bDischarged )
	{
		g_pServerSoundMgr->PlaySoundFromObject(GetAI()->GetObject(), "chars\\snd\\poodle\\fairmonerelease.wav",
			1000.0f, SOUNDPRIORITY_MISC_LOW, 0);

		g_pCharacterMgr->FindDogsInRadius(lstDogs, 500.0f);

		pCur = lstDogs.GetItem(TLIT_FIRST);

		while (pCur && *pCur)
		{
            SendTriggerMsgToObject(GetAI(), (*pCur)->m_hObject, LTFALSE, "HEAT");

			pCur = lstDogs.GetItem(TLIT_NEXT);
		}

        m_bDischarged = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateDischarge::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateDischarge::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_BOOL(m_bDischarged);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateDischarge::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateDischarge::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_BOOL(m_bDischarged);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateShutdown::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateShutdown::Constructor()
{
	super::Constructor();
}

void CAIPoodleStateShutdown::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStateShutdown::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIPoodleStateShutdown::Update()
{
	super::Update();

	char szSpawn[128] = "WeaponItem AmmoAmount 1;WeaponType Poodle,Poodle";
	BaseClass* pObj = SpawnObject(szSpawn, GetAI()->GetPosition(), GetAI()->GetRotation());

	GetAI()->ChangeState("REMOVE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkState::Constructor()
{
	super::Constructor();

    m_pAIShark = LTNULL;
    m_pStrategyFollowPath = LTNULL;
    m_pStrategyOneShotAni = LTNULL;
}

void CAISharkState::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkState::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	m_pAIShark = pAIShark;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkState::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAISharkState::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateIdle::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateIdle::Constructor()
{
	super::Constructor();

	m_ePose = CAnimatorAIAnimal::eIdleStand;
}

void CAISharkStateIdle::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateIdle::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateIdle::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_DWORD_CAST(m_ePose, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateIdle::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateIdle::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_DWORD(m_ePose);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateIdle::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateIdle::Update()
{
	super::Update();

	GetAI()->GetAnimator()->SetMain(m_ePose);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateIdle::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateIdle::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "POSE") )
	{
		if ( !_stricmp(szValue, "SIT") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleSit;
		}
		else if ( !_stricmp(szValue, "LAY") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleLay;
		}
		else if ( !_stricmp(szValue, "STAND") )
		{
			m_ePose = CAnimatorAIAnimal::eIdleStand;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateChase::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAISharkStrategyFollowPath);
}

void CAISharkStateChase::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStateChase::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIShark) )
	{
        return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(CAnimatorAIAnimal::eSwimming);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::HandleTouch
//
//	PURPOSE:	Handles getting touch notifies
//
// ----------------------------------------------------------------------- //

void CAISharkStateChase::HandleTouch(HOBJECT hObject)
{
	super::HandleTouch(hObject);

	if ( GetAI()->HasTarget() )
	{
		if ( GetAI()->GetTarget()->GetObject() == hObject )
		{
			GetAI()->ChangeState("BITE");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateChase::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_pStrategyFollowPath->IsUnset() )
	{
        CCharacter* pTarget = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());

        LTVector vTargetPos = pTarget->GetLastVolumePos();
//		vTargetPos.y = GetAI()->GetPosition().y;

		if ( !m_pStrategyFollowPath->Set(vTargetPos) )
		{
			GetAI()->ChangeState("WAIT");
			return;
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		if ( !m_pStrategyFollowPath->Update() )
		{
			GetAI()->ChangeState("WAIT");
			return;
		}
	}

	static int nTimes = 0;
	nTimes++;

	if ( m_pStrategyFollowPath->IsDone() || nTimes == 10 )
	{
		nTimes= 0;
		GetAI()->ChangeState("CHASE");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateChase::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateChase::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateChase::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateGoto::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateGoto::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAISharkStrategyFollowPath);

    m_vDest = LTVector(0,0,0);
	m_cNodes = 0;
	m_iNextNode = 0;
    m_bLoop = LTFALSE;
}

void CAISharkStateGoto::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateGoto::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStateGoto::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIShark) )
	{
        return LTFALSE;
	}

	// Set up our default movement speed

	m_pStrategyFollowPath->SetMovement(CAnimatorAIAnimal::eSwimming);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateGoto::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateGoto::Update()
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

					NextOr("IDLE");
					return;
				}
			}

			// Advance the next node

			m_iNextNode++;

			// Get the node

			CAINode *pNode = g_pAINodeMgr->GetNode(m_adwNodes[iNode]);

			if ( !pNode || !m_pStrategyFollowPath->Set(pNode) )
			{
				GetAI()->ChangeState("IDLE");
				return;
			}
		}
	}

	if ( !m_pStrategyFollowPath->Update() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateGoto::HandleNameValuePair
//
//	PURPOSE:	Sets data from name/value pairs
//
// ----------------------------------------------------------------------- //

void CAISharkStateGoto::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PT") )
	{
		sscanf(szValue, "%f,%f,%f", &m_vDest.x, &m_vDest.y, &m_vDest.z);
		m_cNodes = 0;
	}
	else if ( !_stricmp(szName, "OBJ") )
	{
		HOBJECT hObject;
		if ( LT_OK != FindNamedObject(szValue, hObject) )
		{
            g_pLTServer->CPrint("GOTO OBJ=%s -- this object does not exist!", szValue);
			return;
		}
		g_pLTServer->GetObjectPos(hObject, &m_vDest);
		m_cNodes = 0;
	}
	else if ( !_stricmp(szName, "PTS") )
	{
		m_cNodes = 0;

		char *szPoint = strtok(szValue, ",");
		while ( szPoint )
		{
			if ( m_cNodes == CAISharkStateGoto::kMaxGotoNodes )
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
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateGoto::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateGoto::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

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
//
//	ROUTINE:	CAISharkStateGoto::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateGoto::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

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
//
//	ROUTINE:	CAISharkStateBite::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateBite::Constructor()
{
	super::Constructor();

	m_pStrategyOneShotAni = FACTORY_NEW(CAISharkStrategyOneShotAni);

    m_bBiting = LTFALSE;
}

void CAISharkStateBite::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyOneShotAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateBite::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStateBite::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIShark) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateBite::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateBite::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
	}
	else if ( m_bBiting && !m_pStrategyOneShotAni->IsAnimating() )
	{
        LTVector vPos;
        g_pLTServer->GetObjectPos(GetAI()->GetTarget()->GetObject(), &vPos);

		if ( VEC_DISTSQR(GetAI()->GetPosition(), vPos) > 5625.0f )
		{
			GetAI()->ChangeState("CHASE");
		}
	}
	else if ( m_bBiting )
	{
		GetAI()->FaceTarget();
	}
	else if ( !m_bBiting )
	{
        m_bBiting = LTTRUE;
	}

	m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eBite);
	if ( !m_pStrategyOneShotAni->Update() )
	{

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateBite::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateBite::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyOneShotAni->Load(hRead);

	LOAD_BOOL(m_bBiting);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateBite::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateBite::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_BOOL(m_bBiting);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateMaul::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateMaul::Constructor()
{
	super::Constructor();

	m_pStrategyOneShotAni = FACTORY_NEW(CAISharkStrategyOneShotAni);

	m_hTarget = LTNULL;
    m_bBiting = LTFALSE;
	m_bDone = LTFALSE;
	m_fMaulTime = 15.0f;
}

void CAISharkStateMaul::Destructor()
{
	super::Destructor();

	GetAI()->Unlink(m_hTarget);

	FACTORY_DELETE(m_pStrategyOneShotAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateMaul::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStateMaul::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyOneShotAni->Init(pAIShark) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateMaul::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateMaul::Update()
{
	super::Update();

	if ( !m_hTarget )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	if ( m_bFirstUpdate )
	{
		g_pLTServer->SetModelAnimation(m_hTarget, g_pLTServer->GetAnimIndex(m_hTarget, "DSharkMaul"));
		g_pLTServer->SetModelLooping(m_hTarget, LTTRUE);

		HATTACHMENT hAttachment;
		LTRESULT dResult = g_pLTServer->CreateAttachment(GetAI()->GetObject(), m_hTarget, "Maul", &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);
		_ASSERT(LT_OK == dResult);
	}

	if ( m_bBiting && !m_pStrategyOneShotAni->IsAnimating() )
	{
		if ( m_fElapsedTime > m_fMaulTime )
		{
			// Remove the attached dude

			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(GetAI()->GetObject(), m_hTarget, &hAttachment) )
			{
				if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
				{
				}
			}

			m_bDone = LTTRUE;

			g_pLTServer->SetModelAnimation(m_hTarget, g_pLTServer->GetAnimIndex(m_hTarget, "DSharkDie"));
			g_pLTServer->SetModelLooping(m_hTarget, LTTRUE);

			GetAI()->ChangeState("TARGETPLAYER;CHASE");

			return;
		}
	}
	else if ( m_bBiting )
	{
		GetAI()->FaceObject(m_hTarget);
	}
	else if ( !m_bBiting )
	{
        m_bBiting = LTTRUE;
	}

	m_pStrategyOneShotAni->Set(CAnimatorAIAnimal::eBite);
	if ( !m_pStrategyOneShotAni->Update() )
	{

	}
}
// ----------------------------------------------------------------------- //

void CAISharkStateMaul::HandleBrokenLink(HOBJECT hObject)
{
	super::HandleBrokenLink(hObject);

	if ( m_hTarget == hObject )
	{
		m_hTarget = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void CAISharkStateMaul::HandleNameValuePair(char *szName, char *szValue)
{
	super::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "TIME") )
	{
		m_fMaulTime = (LTFLOAT)atof(szValue);
	}
	else if ( !_stricmp(szName, "TARGET") )
	{
		GetAI()->Unlink(m_hTarget);
		m_hTarget = LTNULL;

		if ( LT_OK != FindNamedObject(szValue, m_hTarget) )
		{
            g_pLTServer->CPrint("MAUL TARGET=%s -- this object does not exist!", szValue);
			return;
		}

		GetAI()->Link(m_hTarget);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateMaul::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateMaul::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyOneShotAni->Load(hRead);

	LOAD_HOBJECT(m_hTarget);
	LOAD_BOOL(m_bBiting);
	LOAD_BOOL(m_bDone);
	LOAD_FLOAT(m_fMaulTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateMaul::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateMaul::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyOneShotAni->Save(hWrite);

	SAVE_HOBJECT(m_hTarget);
	SAVE_BOOL(m_bBiting);
	SAVE_BOOL(m_bDone);
	SAVE_FLOAT(m_fMaulTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateWait::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStateWait::Constructor()
{
	super::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAISharkStrategyFollowPath);
	m_dwNode = CAINode::kInvalidNodeID;
}

void CAISharkStateWait::Destructor()
{
	super::Destructor();

	FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateWait::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStateWait::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIShark) )
	{
        return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(CAnimatorAIAnimal::eSwimming);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateWait::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAISharkStateWait::Update()
{
	super::Update();

	if ( !GetAI()->HasTarget() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

    CCharacter* pTarget = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
    LTVector vTargetPos = pTarget->GetLastVolumePos();

	if ( m_pStrategyFollowPath->IsUnset() )
	{
		if ( m_pStrategyFollowPath->Set(vTargetPos) )
		{
			GetAI()->ChangeState("CHASE");
			return;
		}
		else
		{
			CAINode* pNode = g_pAINodeMgr->FindNearestVantage(vTargetPos);
			if ( !pNode )
			{
				GetAI()->ChangeState("CHASE");
				return;
			}
			else
			{
				if ( pNode->GetID() == m_dwNode )
				{

				}
				else if ( !m_pStrategyFollowPath->Set(pNode) )
				{
					GetAI()->ChangeState("CHASE");
					return;
				}

				m_dwNode = pNode->GetID();
			}
		}
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		if ( !m_pStrategyFollowPath->Update() )
		{
			GetAI()->ChangeState("IDLE");
		}
	}

	if ( m_pStrategyFollowPath->IsDone() )
	{
		m_pStrategyFollowPath->Stop();
		GetAI()->FaceTarget();
	}

	static int nTimes = 0;
	nTimes++;

	if ( m_pStrategyFollowPath->IsDone() || nTimes == 10 )
	{
		CAINode* pNode = g_pAINodeMgr->FindNearestVantage(vTargetPos);
		nTimes= 0;

		if ( pNode && (m_dwNode == pNode->GetID()) )
		{

		}
		else
		{
			GetAI()->ChangeState("WAIT");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateWait::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateWait::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);
	LOAD_DWORD(m_dwNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStateWait::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAISharkStateWait::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);
	SAVE_DWORD(m_dwNode);
}