// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "AIBrain.h"
#include "AI.h"
#include "AITarget.h"
#include "ProjectileTypes.h"
#include "AINodeMgr.h"
#include "CharacterMgr.h"
#include "AnimatorPlayer.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS(CAIBrain);


CAIBrain::CAIBrain()
{
	m_szName[0] = 0;
	m_pBrain = NULL;
	m_pAI = NULL;
}

CAIBrain::~CAIBrain()
{
}

void CAIBrain::Init(CAI* pAI, const char* szName)
{
	m_pAI = pAI;

	LTStrCpy(m_szName, szName,LTARRAYSIZE(m_szName));
	int32 iBrain = g_pAIDB->GetAIBrainRecordID(szName);
	if ( -1 != iBrain )
	{
		m_pBrain = g_pAIDB->GetAIBrainRecord(iBrain);
	}
	else
	{
		g_pLTServer->CPrint("DANGER!!!! AI had no brain, but settled for default!!!!");
		m_pBrain = g_pAIDB->GetAIBrainRecord(0);
	}

	if ( !m_pBrain )
	{
		g_pLTServer->CPrint("DANGER!!!! AI has no brain!!!!");
		return;
	}
}



