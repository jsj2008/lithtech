// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIUtils.h"
#include "AIHuman.h"
#include "AISense.h"
#include "AIState.h"
#include "DeathScene.h"
#include "AINodeMgr.h"
#include "PlayerObj.h"
  
static char s_szTrigger[1024];

HOBJECT GetEnemyTargetObject(CAISense* pAISense)
{
	HOBJECT hTarget = LTNULL;

	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyWeaponFire:
		{
			hTarget = pAISense->GetStimulus();
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			CDeathScene* pDeathScene = (CDeathScene*)g_pLTServer->HandleToObject(pAISense->GetStimulus());
			if ( pDeathScene->GetObject() )
			{
				Body* pBody = (Body*)g_pLTServer->HandleToObject(pDeathScene->GetObject());
				hTarget = pBody->GetLastDamager();
			}
		}
		break;

		case stHearAllyPain:
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pAISense->GetStimulus());
			hTarget = pCharacter->GetLastDamager();
		}
		break;
	}

	if ( !hTarget || (hTarget == pAISense->GetAI()->GetObject()) )
	{
		if ( BAD == pAISense->GetAI()->GetCharacterClass() )
		{
			g_pLTServer->CPrint("Danger, could not resolve target properly, using player.");
			return g_pCharacterMgr->FindPlayer()->m_hObject;
		}
		else
		{
			g_pLTServer->CPrint("Danger, could not resolve target properly, using NULL.");
			return LTNULL;
		}
	}
	else
	{
		return hTarget;
	}
}

const char* GetEnemyTargetName(CAISense* pAISense)
{
	HOBJECT hTarget = LTNULL;

	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyWeaponFire:
		{
			hTarget = pAISense->GetStimulus();
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			CDeathScene* pDeathScene = (CDeathScene*)g_pLTServer->HandleToObject(pAISense->GetStimulus());
			if ( pDeathScene->GetObject() )
			{
				Body* pBody = (Body*)g_pLTServer->HandleToObject(pDeathScene->GetObject());
				hTarget = pBody->GetLastDamager();
			}
		}
		break;

		case stHearAllyPain:
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pAISense->GetStimulus());
			hTarget = pCharacter->GetLastDamager();
		}
		break;
	}

	if ( !hTarget || (hTarget == pAISense->GetAI()->GetObject()) )
	{
		if ( BAD == pAISense->GetAI()->GetCharacterClass() )
		{
			g_pLTServer->CPrint("Danger, could not resolve target properly, using player.");
			return "Player";
		}
		else
		{
			g_pLTServer->CPrint("Danger, could not resolve target properly, using no one.");
			return "noone";
		}
	}
	else
	{
		return g_pLTServer->GetObjectName(hTarget);
	}
}

// ----------------------------------------------------------------------- //
//
// SoundAlarm
//
// ----------------------------------------------------------------------- //

void CAIHuman::SoundAlarm(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			CAINode* pAINode = g_pAINodeMgr->FindNearestUseObject(m_vPos, "Alarm");
			if ( !pAINode )
			{
				sprintf(s_szTrigger, "TARGET (%s);ATTACK FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
			}
			else
			{
				sprintf(s_szTrigger, "TARGET (%s);USEOBJECT DEST=%s MOVE=RUN FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetStringData(pAINode->GetName()), bIndividual ? "TRUE" : "FALSE");
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// GetBackup
//
// ----------------------------------------------------------------------- //

void CAIHuman::GetBackup(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			CAINode* pAINode = g_pAINodeMgr->FindNearestBackup(m_vPos);
			if ( !pAINode )
			{
				sprintf(s_szTrigger, "TARGET (%s);ATTACK FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
			}
			else
			{
				sprintf(s_szTrigger, "TARGET (%s);GETBACKUP DEST=%s MOVE=RUN FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetStringData(pAINode->GetName()), bIndividual ? "TRUE" : "FALSE");
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// HitSwitch
//
// ----------------------------------------------------------------------- //

void CAIHuman::HitSwitch(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			CAINode* pAINode = g_pAINodeMgr->FindNearestUseObject(m_vPos, "Switch");
			if ( !pAINode )
			{
				sprintf(s_szTrigger, "TARGET (%s);ATTACK %s", ::GetEnemyTargetName(pAISense));
			}
			else
			{
				sprintf(s_szTrigger, "TARGET (%s);USEOBJECT DEST=%s FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetStringData(pAINode->GetName()), bIndividual ? "TRUE" : "FALSE");
			}

		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// GoForCover
//
// ----------------------------------------------------------------------- //

void CAIHuman::GoForCover(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);COVER", ::GetEnemyTargetName(pAISense));
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// TrainingFailure
//
// ----------------------------------------------------------------------- //

void CAIHuman::TrainingFailure(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			CAINode* pAINode = g_pAINodeMgr->FindNearestTrainingFailure(m_vPos);
			if ( !pAINode )
			{
				sprintf(s_szTrigger, "PING");
			}
			else
			{
				sprintf(s_szTrigger, "%s", g_pLTServer->GetStringData(pAINode->GetTrainingFailureCmd()));
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// Panic
//
// ----------------------------------------------------------------------- //

void CAIHuman::Panic(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);PANIC", ::GetEnemyTargetName(pAISense));
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// Distress
//
// ----------------------------------------------------------------------- //

void CAIHuman::Distress(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);DISTRESS", ::GetEnemyTargetName(pAISense));
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// Surrender
//
// ----------------------------------------------------------------------- //

void CAIHuman::Surrender(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);PANIC", ::GetEnemyTargetName(pAISense));
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// Charge
//
// ----------------------------------------------------------------------- //

void CAIHuman::Charge(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			HOBJECT hEnemy = ::GetEnemyTargetObject(pAISense);
			if ( hEnemy == NULL )
			{
				sprintf(s_szTrigger, "AWARE");
			}
			else
			{
				LTVector vEnemyPos;
				g_pLTServer->GetObjectPos(hEnemy, &vEnemyPos);
				LTFLOAT fEnemyDist = vEnemyPos.Dist(m_vPos);
				sprintf(s_szTrigger, "TARGET (%s);CHARGE ATTACKDIST=%f YELLDIST=%f STOPDIST=%f", g_pLTServer->GetObjectName(hEnemy), fEnemyDist*.85f, fEnemyDist, 100.0f);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// DrawWeaponAndAttack
//
// ----------------------------------------------------------------------- //

void CAIHuman::DrawWeaponAndAttack(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);DRAW FIRSTOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// DrawWeaponAndAttackFromCover
//
// ----------------------------------------------------------------------- //

void CAIHuman::DrawWeaponAndAttackFromCover(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);DRAW FIRSTOUND=%s NEXT=(ATTACKFROMCOVER RETRIES=0)", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// DrawWeaponAndAttackFromVantage
//
// ----------------------------------------------------------------------- //

void CAIHuman::DrawWeaponAndAttackFromVantage(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);DRAW FIRSTOUND=%s NEXT=(ATTACKFROMVANTAGE)", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// Attack
//
// ----------------------------------------------------------------------- //

void CAIHuman::Attack(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACK FIRSTOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// AttackStay
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackStay(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACK CHASE=FALSE FIRSTOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// AttackFromCover
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackFromCover(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACKFROMCOVER RETRIES=0 FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// AttackFromCoverStay
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackFromCoverStay(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACKFROMCOVER RETRIES=-1 FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// AttackFromCoverAlwaysRetry
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackFromCoverAlwaysRetry(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACKFROMCOVER RETRIES=99999 FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// AttackFromCoverRetryOnce
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackFromCoverRetryOnce(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			int nMaxRetries = ((CAIHuman*)pAISense->GetAI())->GetBrain()->GetAttackFromCoverMaxRetries();
			sprintf(s_szTrigger, "TARGET (%s);ATTACKFROMCOVER RETRIES=%d FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), nMaxRetries-1, bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// AttackOnSight
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackOnSight(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACKONSIGHT FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// AttackFromVantage
//
// ----------------------------------------------------------------------- //

void CAIHuman::AttackFromVantage(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "TARGET (%s);ATTACKFROMVANTAGE FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// CallOut
//
// ----------------------------------------------------------------------- //

void CAIHuman::CallOut(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// ShineFlashlight
//
// ----------------------------------------------------------------------- //

void CAIHuman::ShineFlashlight(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// LookAt
//
// ----------------------------------------------------------------------- //

void CAIHuman::LookAt(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "LOOKAT POSITION=%f,%f,%f SENSE=%d FIRSTSOUND=%s", EXPANDVEC(pAISense->GetStimulusPosition()), pAISense->GetType(), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

	AppendReturnString();
}

// ----------------------------------------------------------------------- //
//
// BecomeAlert
//
// ----------------------------------------------------------------------- //

void CAIHuman::BecomeAlert(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
	        sprintf(s_szTrigger, "AWARE FIRSTSOUND=%s", bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// InvestigateAndSearch
//
// ----------------------------------------------------------------------- //

void CAIHuman::InvestigateAndSearch(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "INVESTIGATE ENEMY=(%s) POSITION=%f,%f,%f SENSE=%d SEARCH=TRUE FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), EXPANDVEC(pAISense->GetStimulusPosition()), pAISense->GetType(), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			HOBJECT hBody = pAISense->GetStimulus();
			sprintf(s_szTrigger, "TARGET (%s);CHECKBODY BODY=(%s) SEARCH=TRUE FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetObjectName(hBody), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeEnemyFootprint:
		{
	        sprintf(s_szTrigger, "TARGET (%s);FOLLOWFOOTPRINT SEARCH=TRUE FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// InvestigateAndStay
//
// ----------------------------------------------------------------------- //

void CAIHuman::InvestigateAndStay(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "INVESTIGATE ENEMY=(%s) POSITION=%f,%f,%f SENSE=%d FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), EXPANDVEC(pAISense->GetStimulusPosition()), pAISense->GetType(), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			HOBJECT hBody = pAISense->GetStimulus();
			sprintf(s_szTrigger, "TARGET (%s);CHECKBODY BODY=(%s) FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetObjectName(hBody), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeEnemyFootprint:
		{
	        sprintf(s_szTrigger, "TARGET (%s);FOLLOWFOOTPRINT FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

}

// ----------------------------------------------------------------------- //
//
// InvestigateAndReturn
//
// ----------------------------------------------------------------------- //

void CAIHuman::InvestigateAndReturn(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "INVESTIGATE ENEMY=(%s) POSITION=%f,%f,%f SENSE=%d FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), EXPANDVEC(pAISense->GetStimulusPosition()), pAISense->GetType(), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			HOBJECT hBody = pAISense->GetStimulus();
			sprintf(s_szTrigger, "TARGET (%s);CHECKBODY BODY=(%s) FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), g_pLTServer->GetObjectName(hBody), bIndividual ? "TRUE" : "FALSE");
		}
		break;

		case stSeeEnemyFootprint:
		{
			sprintf(s_szTrigger, "TARGET (%s);FOLLOWFOOTPRINT FIRSTSOUND=%s", ::GetEnemyTargetName(pAISense), bIndividual ? "TRUE" : "FALSE");
		}
		break;
	}

	AppendReturnString();
}

// ----------------------------------------------------------------------- //
//
// PassOut
//
// ----------------------------------------------------------------------- //

void CAIHuman::PassOut(CAISense* pAISense, LTBOOL bIndividual)
{
	switch ( pAISense->GetType() )
	{
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stSeeEnemy:
		case stSeeAllyDeath:
		case stSeeEnemyFootprint:
		case stSeeEnemyFlashlight:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		{
			sprintf(s_szTrigger, "UNCONSCIOUS AWARE=FALSE TIME=%f", GetRandom(15.0f, 30.0f));
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
// SoundAlarm
//
// ----------------------------------------------------------------------- //

void CAIHuman::AppendReturnString()
{
	if ( m_pState && m_pState->CanReturn() )
	{
		HSTRING hstrReturn = m_pState->CreateReturnString();
		char szReturn[1024];
		sprintf(szReturn, " RETURN=(%s)", g_pLTServer->GetStringData(hstrReturn));
		strcat(s_szTrigger, szReturn);
		FREE_HSTRING(hstrReturn);
	}
}

// DoReaction

void CAIHuman::DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual)
{
	if ( !hstrReaction ) return;

	sprintf(s_szTrigger, "PING");

	g_pLTServer->CPrint("%s doing reaction \"%s\" to %s %s", GetName(), g_pLTServer->GetStringData(hstrReaction), pAISense->GetName(), pAISense->GetOutcome() == soFalseStimulation ? "(false alarm)" : "");

    char* szReaction = g_pLTServer->GetStringData(hstrReaction);
	_ASSERT(szReaction);

	if ( !_stricmp(szReaction, c_szNoReaction) )
		return;
	else if ( !_stricmp("Sound alarm", szReaction) )
		SoundAlarm(pAISense, bIndividual);
	else if ( !_stricmp("Get backup", szReaction) )
		GetBackup(pAISense, bIndividual);
	else if ( !_stricmp("Hit switch", szReaction) )
		HitSwitch(pAISense, bIndividual);
	else if ( !_stricmp("Go for cover", szReaction) )
		GoForCover(pAISense, bIndividual);
	else if ( !_stricmp("Panic", szReaction) )
		Panic(pAISense, bIndividual);
	else if ( !_stricmp("Distress", szReaction) )
		Distress(pAISense, bIndividual);
	else if ( !_stricmp("Surrender", szReaction) )
		Surrender(pAISense, bIndividual);
	else if ( !_stricmp("Charge", szReaction) )
		Charge(pAISense, bIndividual);
	else if ( !_stricmp("Draw weapon and attack", szReaction) )
		DrawWeaponAndAttack(pAISense, bIndividual);
	else if ( !_stricmp("Draw weapon and attack from cover", szReaction) )
		DrawWeaponAndAttackFromCover(pAISense, bIndividual);
	else if ( !_stricmp("Draw weapon and attack from vantage", szReaction) )
		DrawWeaponAndAttackFromVantage(pAISense, bIndividual);
	else if ( !_stricmp("Attack", szReaction) )
		Attack(pAISense, bIndividual);
	else if ( !_stricmp("Attack (stay)", szReaction) )
		AttackStay(pAISense, bIndividual);
	else if ( !_stricmp("Attack from cover", szReaction) )
		AttackFromCover(pAISense, bIndividual);
	else if ( !_stricmp("Attack from cover (stay)", szReaction) )
		AttackFromCoverStay(pAISense, bIndividual);
	else if ( !_stricmp("Attack from cover (always retry)", szReaction) )
		AttackFromCoverAlwaysRetry(pAISense, bIndividual);
	else if ( !_stricmp("Attack from cover (retry once)", szReaction) )
		AttackFromCoverRetryOnce(pAISense, bIndividual);
	else if ( !_stricmp("Attack on sight", szReaction) )
		AttackOnSight(pAISense, bIndividual);
	else if ( !_stricmp("Attack from vantage", szReaction) )
		AttackFromVantage(pAISense, bIndividual);
	else if ( !_stricmp("Call out", szReaction) )
		CallOut(pAISense, bIndividual);
	else if ( !_stricmp("Shine flashlight", szReaction) )
		ShineFlashlight(pAISense, bIndividual);
	else if ( !_stricmp("Look at", szReaction) )
		LookAt(pAISense, bIndividual);
	else if ( !_stricmp("Become alert", szReaction) )
		BecomeAlert(pAISense, bIndividual);
	else if ( !_stricmp("Investigate and search", szReaction) )
		InvestigateAndSearch(pAISense, bIndividual);
	else if ( !_stricmp("Investigate and stay", szReaction) )
		InvestigateAndStay(pAISense, bIndividual);
	else if ( !_stricmp("Investigate and return", szReaction) )
		InvestigateAndReturn(pAISense, bIndividual);
	else if ( !_stricmp("Pass out", szReaction) )
		PassOut(pAISense, bIndividual);
	else if ( !_stricmp("Training failure", szReaction) )
		TrainingFailure(pAISense, bIndividual);

	/*** DEPRECATED ***/
	
	else if ( !_stricmp(szReaction, "Check body and stay") )
	{
		g_pLTServer->CPrint("Using deprecated reaction: %s", szReaction);
		InvestigateAndStay(pAISense, bIndividual);
	}
	else if ( !_stricmp(szReaction, "Check body and return") )
	{
		g_pLTServer->CPrint("Using deprecated reaction: %s", szReaction);
		InvestigateAndReturn(pAISense, bIndividual);
	}
	else if ( !_stricmp(szReaction, "Follow footprints and stay") || !_stricmp(szReaction, "Check footprints and stay") )
	{
		g_pLTServer->CPrint("Using deprecated reaction: %s", szReaction);
		InvestigateAndStay(pAISense, bIndividual);
	}
	else if ( !_stricmp(szReaction, "Follow footprints and return") || !_stricmp(szReaction, "Check footprints and return") )
	{
		g_pLTServer->CPrint("Using deprecated reaction: %s", szReaction);
		InvestigateAndReturn(pAISense, bIndividual);
	}

	/*** .......... ***/

	else
	{
		// If it doesn't match one of these, it must be a command

        sprintf(s_szTrigger, g_pLTServer->GetStringData(hstrReaction));
	}

	DidReaction(pAISense, bIndividual);
	ChangeState(s_szTrigger);
	GetSenseMgr()->StopUpdating();
}

/*
		group 1:

		"Sound alarm",
		"Get backup",
		"Hit switch",

		group 2:

		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",

		group 3:

		"Charge",
		"Draw weapon and attack",  (1st only)
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",

		group 4:

		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",

		group 5:

		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
*/

#define ADD_DEPRECATED_REACTIONS() \
	"---DO NOT USE THESE REACTIONS---",\
	"Check body and stay",\
	"Check body and return",\
	"Follow footprints and stay",\
	"Follow footprints and return",\

REACTIONSTRUCT g_aAIHumanReactions[] =
{
	{ "ISE1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Pass out",
		"Training failure",
	},

	{ "ISE",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Pass out",
		"Training failure",
	},

	{ "ISEFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFlashlight1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFlashlight",

		c_szNoReaction,
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFlashlightFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFlashlightFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEFootstep1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEFootstep",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEFootstepFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEFootstepFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEWeaponFire1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHEWeaponFire",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHAWeaponFire1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHAWeaponFire",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHEWeaponImpact1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEWeaponImpact",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISADeath1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISADeath",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFootprint1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "ISEFootprint",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEDisturbance1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHEDisturbance",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHAPain1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHAPain",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Pass out",
		"Training failure",
	},

	{ "IHADeath1st",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "IHADeath",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		"Pass out",
		"Training failure",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSE1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
	},

	{ "GSE",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
	},

	{ "GSEFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFlashlight1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFlashlight",

		c_szNoReaction,
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFlashlightFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFlashlightFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEFootstep1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEFootstep",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEFootstepFalse1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEFootstepFalse",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEWeaponFire1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHEWeaponFire",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHAWeaponFire1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHAWeaponFire",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHEWeaponImpact1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEWeaponImpact",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSADeath1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSADeath",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFootprint1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GSEFootprint",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEDisturbance1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHEDisturbance",

		c_szNoReaction,
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHAPain1st",

		c_szNoReaction,
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHAPain",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Charge",
		"Attack",
		"Attack (stay)",
		"Attack from cover",
		"Attack from cover (stay)",
		"Attack from cover (always retry)",
		"Attack from cover (retry once)",
		"Attack on sight",
		"Attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
	},

	{ "GHADeath1st",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Sound alarm",
		"Get backup",
		"Hit switch",
		"Draw weapon and attack",
		"Draw weapon and attack from cover",
		"Draw weapon and attack from vantage",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},

	{ "GHADeath",

		c_szNoReaction,
		"Go for cover",
		"Panic",
		"Distress",
		"Surrender",
		"Call out",
		"Shine flashlight",
		"Look at",
		"Become alert",
		"Investigate and search",
		"Investigate and stay",
		"Investigate and return",
		ADD_DEPRECATED_REACTIONS()
	},
};

int g_cAIHumanReactions = sizeof(g_aAIHumanReactions)/sizeof(REACTIONSTRUCT);