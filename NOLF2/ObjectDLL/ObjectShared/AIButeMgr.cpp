// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIButeMgr.h"
#include "ButeTools.h"
#include "CommonUtilities.h"
#include "AISenseRecorderAbstract.h"
#include "AIStimulusMgr.h"
#include "WeaponMgr.h"
#include "AIVolume.h"
#include "AIUtils.h"
#include <time.h>

extern CWeaponMgr* g_pWeaponMgr;

// Globals/statics

CAIButeMgr* g_pAIButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];


// Enable this define to assert on Bute errors instead of printing out AI ERROR
// warnings whenever an undefined label is accessed.

// DON'T leave this define on in a distributed build, as DEdit will assert if 
// any bute values are missing. 

//#define _AI_DEBUG_BUTES_


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::CAIButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAIButeMgr::CAIButeMgr()
{
    m_aTemplates = LTNULL;
    m_aBrains = LTNULL;
    m_aSenseMasks = LTNULL;
    m_aDamageMasks = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::~CAIButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIButeMgr::~CAIButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CAIButeMgr::Init(const char* szAttributeFile)
{
    if (g_pAIButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile))
	{
		AIASSERT1( 0, NULL, "CAIGoalButeMgr::Init: Failed to parse %s", szAttributeFile );
		return LTFALSE;
	}

	// Initialize console variable here, because this is a centralized
	// location for AI that will only get run once.

	g_vtMuteAIAssertsVar.Init(g_pLTServer, "MuteAIAsserts", LTNULL, 0.0f);
	g_vtDifficultyFactorEasy.Init( g_pLTServer, "DifficultyFactorEasy", LTNULL, 0.8f );
	g_vtDifficultyFactorNormal.Init( g_pLTServer, "DifficultyFacterNormal", LTNULL, 0.9f );
	g_vtDifficultyFactorHard.Init( g_pLTServer, "DifficultyFactorHard", LTNULL, 1.0f );
	g_vtDifficultyFactorVeryHard.Init( g_pLTServer, "DifficultyFactorVeryHard", LTNULL, 1.1f );
	g_vtDifficultyFactorPlayerIncrease.Init( g_pLTServer, "DifficultyFactorPlayerIncrease", LTNULL, 0.1f );
	

	// Set up global pointer

	g_pAIButeMgr = this;

	// See how many attribute templates there are

	m_cTemplateID = 0;
	sprintf(s_aTagName, "%s%d", "AttributeTemplate", m_cTemplateID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cTemplateID++;
		sprintf(s_aTagName, "%s%d", "AttributeTemplate", m_cTemplateID);
	}

	m_aTemplates = debug_newa(AIBM_Template, m_cTemplateID);
	for ( int iTemplate = 0 ; iTemplate < m_cTemplateID ; iTemplate++ )
	{
		SetTemplate(iTemplate);
	}

	// See how many damage masks there are
	// (Must read DamageMasks before brains, because brains reference them).

	m_cDamageMaskID = 0;
	sprintf(s_aTagName, "%s%d", "DamageMask", m_cDamageMaskID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cDamageMaskID++;
		sprintf(s_aTagName, "%s%d", "DamageMask", m_cDamageMaskID);
	}

	m_aDamageMasks = debug_newa(AIBM_DamageMask, m_cDamageMaskID);
	for ( int iDamageMask = 0 ; iDamageMask < m_cDamageMaskID ; iDamageMask++ )
	{
		SetDamageMask(iDamageMask);
	}

	// See how many brains there are

	m_cBrainID = 0;
	sprintf(s_aTagName, "%s%d", "Brain", m_cBrainID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cBrainID++;
		sprintf(s_aTagName, "%s%d", "Brain", m_cBrainID);
	}

	m_aBrains = debug_newa(AIBM_Brain, m_cBrainID);
	for ( int iBrain = 0 ; iBrain < m_cBrainID ; iBrain++ )
	{
		SetBrain(iBrain);
	}

	// See how many sense masks there are

	m_cSenseMaskID = 0;
	sprintf(s_aTagName, "%s%d", "SenseMask", m_cSenseMaskID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cSenseMaskID++;
		sprintf(s_aTagName, "%s%d", "SenseMask", m_cSenseMaskID);
	}

	m_aSenseMasks = debug_newa(AIBM_SenseMask, m_cSenseMaskID);
	for ( int iSenseMask = 0 ; iSenseMask < m_cSenseMaskID ; iSenseMask++ )
	{
		SetSenseMask(iSenseMask);
	}

	// Read stimulus.

	uint8 iStimulus = 0;
	sprintf(s_aTagName, "%s%d", "Stimulus", iStimulus);
	while (m_buteMgr.Exist(s_aTagName))
	{
		SetStimulus(iStimulus);
		iStimulus++;
		sprintf(s_aTagName, "%s%d", "Stimulus", iStimulus);
	}

	SetResetTimers();
	SetTextIDs();
	SetSenses();
	SetAttract();
	SetRules();
	SetDEdit();

	m_buteMgr.Term();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::Term()
{
	if ( m_aTemplates )
	{
		debug_deletea(m_aTemplates);
        m_aTemplates = LTNULL;
	}

	if ( m_aBrains )
	{
		debug_deletea(m_aBrains);
        m_aBrains = LTNULL;
	}

	if ( m_aSenseMasks )
	{
		debug_deletea(m_aSenseMasks);
        m_aSenseMasks = LTNULL;
	}

	if ( m_aDamageMasks )
	{
		debug_deletea(m_aDamageMasks);
        m_aDamageMasks = LTNULL;
	}

	AIBM_STIMULUS_MAP::iterator it;
	for(it = m_mapStimuli.begin(); it != m_mapStimuli.end(); it++)
	{
		debug_delete(it->second);
	}
	m_mapStimuli.clear();

    g_pAIButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::GetTemplate*()
//
//	PURPOSE:	Gets an attribute template property or information
//
// ----------------------------------------------------------------------- //

int CAIButeMgr::GetTemplateIDByName(const char *szName)
{
	for ( int iTemplateID = 0 ; iTemplateID < m_cTemplateID ; iTemplateID++ )
	{
		if ( !strcmp(szName, GetTemplate(iTemplateID)->szName) )
		{
			return iTemplateID;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::GetBrain*()
//
//	PURPOSE:	Gets an attribute Brain property or information
//
// ----------------------------------------------------------------------- //

int CAIButeMgr::GetBrainIDByName(const char *szName)
{
	for ( int iBrainID = 0 ; iBrainID < m_cBrainID ; iBrainID++ )
	{
		if ( !strcmp(szName, GetBrain(iBrainID)->szName) )
		{
			return iBrainID;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::GetSenseMask*()
//
//	PURPOSE:	Gets an attribute SenseMask property or information
//
// ----------------------------------------------------------------------- //

int CAIButeMgr::GetSenseMaskIDByName(const char *szName)
{
	for ( int iSenseMaskID = 0 ; iSenseMaskID < m_cSenseMaskID ; iSenseMaskID++ )
	{
		if ( !stricmp(szName, GetSenseMask(iSenseMaskID)->szName) )
		{
			return iSenseMaskID;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::GetDamageMask*()
//
//	PURPOSE:	Gets an attribute DamageMask property or information
//
// ----------------------------------------------------------------------- //

int CAIButeMgr::GetDamageMaskIDByName(const char *szName)
{
	for ( int iDamageMaskID = 0 ; iDamageMaskID < m_cDamageMaskID ; iDamageMaskID++ )
	{
		if ( !stricmp(szName, GetDamageMask(iDamageMaskID)->szName) )
		{
			return iDamageMaskID;
		}
	}

	return -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetTemplate(int iTemplate)
{
	sprintf(s_aTagName, "AttributeTemplate%d", iTemplate);
	AIBM_Template& Template = m_aTemplates[iTemplate];

	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", Template.szName, sizeof(Template.szName));
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Alignment", Template.szAlignment, sizeof(Template.szAlignment));
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "HumanType", Template.szHumanType, sizeof(Template.szHumanType), "Ground");

	Template.fRunSpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "RunSpeed");
	Template.fJumpSpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "JumpSpeed");
	Template.fJumpOverSpeed					= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "JumpOverSpeed");
	Template.fFallSpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "FallSpeed");
	Template.fWalkSpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "WalkSpeed");
	Template.fSwimSpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "SwimSpeed");
	Template.fFlySpeed						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "FlySpeed");
	Template.fSoundRadius					= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "SoundRadius");
	Template.fHitPoints						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "HitPoints");
	Template.fArmor							= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "Armor");
	Template.fAccuracy						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "Accuracy");
	Template.fAccuracyIncreaseRate			= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "AccuracyIncreaseRate");
	Template.fAccuracyDecreaseRate			= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "AccuracyDecreaseRate");
	
	Template.fFullAccuracyRadiusSqr			= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "FullAccuracyRadius");
	Template.fAccuracyMissPerturb			= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "AccuracyMissPerturb");
	Template.fMaxMovementAccuracyPerturb	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "MaxMovementAccuracyPerturb");
	Template.fMovementAccuracyPerturbDecay	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "MovementAccuracyPerturbTime");

	Template.fFullAccuracyRadiusSqr	*= Template.fFullAccuracyRadiusSqr;
	Template.fMovementAccuracyPerturbDecay = Template.fMaxMovementAccuracyPerturb / Template.fMovementAccuracyPerturbDecay;

	// Value defaulted

	Template.fUpdateRate					= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "UpdateRate", 0.1);
	Template.fVerticalThreshold				= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "VerticalThreshold", 100);

	Template.fDefensePercentage				= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,	"DefensePercentage", 1.0f);

	Template.flSentryChallengeScanDistMax	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "SentryChallengeScanDistMax", 0.0f);
	Template.flSentryChallengeDistMax		= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "SentryChallengeDistMax", 0.0f);
	Template.flSentryMarkDistMax			= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,   "SentryMarkDistMax", 0.0f);

	// Create map of sense to sense distance.
	// Only add map entries if sense is set to TRUE.
	LTFLOAT fDistance;
	EnumAISenseType eSenseType;
	for(uint32 iSenseType=0; iSenseType < kSense_Count; ++iSenseType)
	{
		sprintf(s_aAttName, "Can%s", s_aszSenseTypes[iSenseType]);
		if( CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, s_aAttName, LTFALSE) )
		{
			sprintf(s_aAttName, "%sDistance", s_aszSenseTypes[iSenseType]);
			fDistance = CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, s_aAttName, 0.f);
			eSenseType = (EnumAISenseType)(1 << iSenseType);
			Template.mapSenseDistance.insert( AISenseDistanceMap::value_type(eSenseType, fDistance) );
		}
	}

	Template.iUseableVolumesMask = 0;
	for(uint32 nVolumeType=AIVolume::kVolumeType_None; nVolumeType < AIVolume::kVolumeType_Count; ++nVolumeType)
	{
		sprintf(s_aAttName, "CanUseVolume%s", AIVolume::ms_aszVolumeTypes[nVolumeType]);
		if ( LTTRUE == CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, s_aAttName, LTTRUE ) )
		{
			Template.iUseableVolumesMask |= (AIVolume::EnumVolumeType)(1 << nVolumeType);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetBrain(int iBrain)
{
	sprintf(s_aTagName, "Brain%d", iBrain);
	
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", m_aBrains[iBrain].szName, sizeof(m_aBrains[iBrain].szName) );

	m_aBrains[iBrain].fDodgeVectorCheckTime 			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorCheckTime");
	m_aBrains[iBrain].fDodgeVectorCheckChance			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorCheckChance");

	m_aBrains[iBrain].fDodgeVectorShuffleDist			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorShuffleDist");
	m_aBrains[iBrain].fDodgeVectorRollDist				= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorRollDist");
	m_aBrains[iBrain].fDodgeVectorShuffleChance 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorShuffleChance");
	m_aBrains[iBrain].fDodgeVectorRollChance			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorRollChance");
	m_aBrains[iBrain].fDodgeVectorBlockChance			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorBlockChance");

	m_aBrains[iBrain].fDodgeVectorDelayTime 			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeVectorDelayTime");

	m_aBrains[iBrain].fDodgeProjectileCheckTime 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeProjectileCheckTime");
	m_aBrains[iBrain].fDodgeProjectileCheckChance		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeProjectileCheckChance");
	m_aBrains[iBrain].fDodgeProjectileDelayTime 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeProjectileDelayTime");
	m_aBrains[iBrain].fDodgeProjectileFleeChance		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DodgeProjectileFleeChance");

	m_aBrains[iBrain].fAttackRangedRangeMin 			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackRangedRange").GetMin();
	m_aBrains[iBrain].fAttackRangedRangeMax 			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackRangedRange").GetMax();
	m_aBrains[iBrain].fAttackThrownRangeMin 			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackThrownRange").GetMin();
	m_aBrains[iBrain].fAttackThrownRangeMax 			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackThrownRange").GetMax();
	m_aBrains[iBrain].fAttackMeleeRangeMin				= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackMeleeRange").GetMin();
	m_aBrains[iBrain].fAttackMeleeRangeMax				= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackMeleeRange").GetMax();
	m_aBrains[iBrain].fAttackChaseTime					= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackChaseTime");
	m_aBrains[iBrain].fAttackChaseTimeRandomMin 		= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackChaseTimeRandom").GetMin();
	m_aBrains[iBrain].fAttackChaseTimeRandomMax 		= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"AttackChaseTimeRandom").GetMax();
	m_aBrains[iBrain].fAttackPoseCrouchChance			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackPoseCrouchChance");
	m_aBrains[iBrain].fAttackPoseCrouchTime				= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackPoseCrouchTime");
	m_aBrains[iBrain].fAttackGrenadeThrowTime			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackGrenadeThrowTime");
	m_aBrains[iBrain].fAttackGrenadeThrowChance 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackGrenadeThrowChance");
	m_aBrains[iBrain].fAttackCoverCheckTime 			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackCoverCheckTime");
	m_aBrains[iBrain].fAttackCoverCheckChance			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackCoverCheckChance");
	m_aBrains[iBrain].fAttackSoundTimeMin				= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackSoundTimeMin");
	m_aBrains[iBrain].bAttackWhileMoving				= CButeTools::GetValidatedBool (m_buteMgr, s_aTagName,"AttackWhileMoving");

	m_aBrains[iBrain].fAttackFromCoverCoverTime 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromCoverCoverTime");
	m_aBrains[iBrain].fAttackFromCoverCoverTimeRandom	= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromCoverCoverTimeRandom");
	m_aBrains[iBrain].fAttackFromCoverUncoverTime		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromCoverUncoverTime");
	m_aBrains[iBrain].fAttackFromCoverUncoverTimeRandom = CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromCoverUncoverTimeRandom");
	m_aBrains[iBrain].fAttackFromCoverReactionTime		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromCoverReactionTime");
	m_aBrains[iBrain].nAttackFromCoverMaxRetries		= CButeTools::GetValidatedInt (m_buteMgr, s_aTagName,"AttackFromCoverMaxRetries");

	m_aBrains[iBrain].fAttackFromVantageAttackTime		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromVantageAttackTime");
	m_aBrains[iBrain].fAttackFromVantageAttackTimeRandom= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromVantageAttackTimeRandom");

	m_aBrains[iBrain].fAttackFromViewChaseTime			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"AttackFromViewChaseTime");

	m_aBrains[iBrain].fChaseExtraTime					= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"ChaseExtraTime");
	m_aBrains[iBrain].fChaseExtraTimeRandomMin			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"ChaseExtraTimeRandom").GetMin();
	m_aBrains[iBrain].fChaseExtraTimeRandomMax			= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"ChaseExtraTimeRandom").GetMax();
	m_aBrains[iBrain].fChaseSearchTime					= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"ChaseSearchTime");

	m_aBrains[iBrain].fPatrolSoundTime					= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"PatrolSoundTime");
	m_aBrains[iBrain].fPatrolSoundTimeRandomMin 		= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"PatrolSoundTimeRandom").GetMin();
	m_aBrains[iBrain].fPatrolSoundTimeRandomMax 		= (LTFLOAT)CButeTools::GetValidatedRange (m_buteMgr, s_aTagName,"PatrolSoundTimeRandom").GetMax();
	m_aBrains[iBrain].fPatrolSoundChance				= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"PatrolSoundChance");

	m_aBrains[iBrain].fDistressFacingThreshhold 		= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DistressFacingThreshhold");
	m_aBrains[iBrain].fDistressIncreaseRate 			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DistressIncreaseRate");
	m_aBrains[iBrain].fDistressDecreaseRate 			= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"DistressDecreaseRate");
	m_aBrains[iBrain].nDistressLevels					= CButeTools::GetValidatedInt (m_buteMgr, s_aTagName,"DistressLevels");

	m_aBrains[iBrain].nMajorAlarmThreshold				= CButeTools::GetValidatedInt (m_buteMgr, s_aTagName,"MajorAlarmThreshold");
	m_aBrains[iBrain].nImmediateAlarmThreshold			= CButeTools::GetValidatedInt (m_buteMgr, s_aTagName,"ImmediateAlarmThreshold");

	m_aBrains[iBrain].fTauntDelay						= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"TauntDelay");
	m_aBrains[iBrain].fTauntChance						= CButeTools::GetValidatedDouble (m_buteMgr, s_aTagName,"TauntChance");

	m_aBrains[iBrain].bCanLipSync						= CButeTools::GetValidatedBool (m_buteMgr, s_aTagName,"CanLipSync");

	char szTemp[64];
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "DamageMask", szTemp, sizeof(szTemp) );
	m_aBrains[iBrain].nDamageMaskID = GetDamageMaskIDByName( szTemp );


	// Handle ammo generation limitations by setting flags corresponding to 
	// the ID of the ammo which has its generation suppressed by the AI.
	UBER_ASSERT( m_aBrains[iBrain].AmmoGenerationRestrictionMask.size() >= 
		static_cast<unsigned int>(g_pWeaponMgr->GetNumAmmoIds()), 
		"Too few bits to represent the full range of ammo for ammo restriction" );

	m_aBrains[iBrain].AmmoGenerationRestrictionMask.reset();

	char	szLabelName[100];
	char	szAmmoName[100];
	int		nAmmoLimiterCount = 0;

	while ( true )
	{
		// If we have no label with this value, then stop looking.
		sprintf(szLabelName, "PreventGenerationAmmo%d", nAmmoLimiterCount );
		if ( !m_buteMgr.Exist( s_aTagName, szLabelName ) )
		{
			break;
		}
		else
		{
			// Icrement the label generating value
			nAmmoLimiterCount++;
		}

		// while we have PreventGenerationAmmoX statments, get their values
		// and add to our mask the Ammo IDs which should not to generated
		// Get then name of the ammo type to be restricted, then get its 
		CButeTools::GetValidatedString( m_buteMgr, s_aTagName, szLabelName, szAmmoName, sizeof( szAmmoName ) );
		
		const AMMO* const pAmmo = g_pWeaponMgr->GetAmmo( szAmmoName );
		if ( !pAmmo )
		{
			// Do an AI error here, not an assert as we don't want to take
			// down Dedit or the game for something so non critical
			AIError( "Attempted to restrict ammo generation of unknown: %s", szAmmoName );
		}
		else
		{
			m_aBrains[iBrain].AmmoGenerationRestrictionMask.set( pAmmo->nId, true );
		}
	}	


	// Read AIData that is specific to this brain.

	uint32 iAIData = 0;

	char szData[64];
	char* tok;
	EnumAIDataType eAIData;
	LTFLOAT fValue;
	uint32 iAIDataType;
	while ( true )
	{
		sprintf(s_aAttName, "%s%d", "AIData", iAIData);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szData, sizeof(szData) );
		if( !m_buteMgr.Success( ))
			break;

		if( szData[0] )
		{
			tok = strtok(szData, "=");
			
			for(iAIDataType=0; iAIDataType < kAIData_Count; ++iAIDataType)
			{
				if( stricmp(tok, s_aszAIDataTypes[iAIDataType]) == 0 )
				{
					eAIData = (EnumAIDataType)iAIDataType;
					break;
				}
			}

			tok = strtok(LTNULL, "");
			fValue = (LTFLOAT)atof(tok);
			m_aBrains[iBrain].mapAIData.insert( AIDATA_MAP::value_type(eAIData, fValue) );
		}

		++iAIData;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetSenseMask(int iSenseMask)
{
	sprintf(s_aTagName, "SenseMask%d", iSenseMask);
	
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", m_aSenseMasks[iSenseMask].szName, sizeof(m_aSenseMasks[iSenseMask].szName) );

	m_aSenseMasks[iSenseMask].flagsBlockedSenses = kSense_None;

	char szTemp[64] = "";

	uint32 iSense = 0;
	while ( true )
	{
		sprintf(s_aAttName, "Sense%d", iSense);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szTemp, sizeof(szTemp) );
		if( !m_buteMgr.Success( ))
			break;

		for(uint32 iSenseType = 0; iSenseType < kSense_Count; ++iSenseType)
		{
			if( stricmp(szTemp, s_aszSenseTypes[iSenseType]) == 0 )
			{
				m_aSenseMasks[iSenseMask].flagsBlockedSenses |= (1 << iSenseType);
				break;
			}
		}

		++iSense;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetDamageMask(int iDamageMask)
{
	sprintf(s_aTagName, "DamageMask%d", iDamageMask);
	
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", m_aDamageMasks[iDamageMask].szName, sizeof(m_aDamageMasks[iDamageMask].szName) );

	m_aDamageMasks[iDamageMask].dfDamageTypes = 0;

	char szTemp[64] = "";

	uint32 iDT;
	uint32 iDamage = 0;
	while ( true )
	{
		sprintf(s_aAttName, "Damage%d", iDamage);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szTemp, sizeof(szTemp) );
		if( !m_buteMgr.Success( ))
			break;

		for( iDT = 0; iDT < kNumDamageTypes; ++iDT )
		{
			if( stricmp( szTemp, pDTNames[iDT] ) == 0 )
			{
				m_aDamageMasks[iDamageMask].dfDamageTypes |= DamageTypeToFlag( (DamageType)iDT );
				break;
			}
		}

		++iDamage;
	}

	// Always include DT_UNSPECIFIED, so that DESTROY command works.

	m_aDamageMasks[iDamageMask].dfDamageTypes |= DamageTypeToFlag( DT_UNSPECIFIED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetStimulus(int iStimulus)
{
	sprintf(s_aTagName, "Stimulus%d", iStimulus);

	AIBM_Stimulus* pStimulus = debug_new(AIBM_Stimulus);

	// Convert stimulusType enum from string.
	char szTemp[64] = "";
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name",szTemp,sizeof(szTemp));
	uint32 iStimulusType;
	for(iStimulusType = 0; iStimulusType < kStim_Count; ++iStimulusType)
	{
		if( stricmp(szTemp, s_aszStimulusTypes[iStimulusType]) == 0 )
		{
			pStimulus->eStimulusType = (EnumAIStimulusType)(1 << iStimulusType);
			m_mapStimuli.insert( AIBM_STIMULUS_MAP::value_type( pStimulus->eStimulusType, pStimulus ) );
			break;
		}
	}
	ASSERT(iStimulusType < kStim_Count);

	// Convert senseType enum from string.
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Sense",szTemp,sizeof(szTemp));
	uint32 iSenseType;
	for(iSenseType = 0; iSenseType < kSense_Count; ++iSenseType)
	{
		if( stricmp(szTemp, s_aszSenseTypes[iSenseType]) == 0 )
		{
			pStimulus->eSenseType = (EnumAISenseType)(1 << iSenseType);
			break;
		}
	}
	ASSERT(iSenseType < kSense_Count);

	CharacterAlignment AlignmentBuffer;
	char szAlignmentBuffer[80];
	char szLabelName[80];
	int nAlignment = 0;

	// Convert alignment from string.
	while ( true )
	{
		// If we have no label with this value, then stop looking.
		sprintf(szLabelName, "RequiredAlignment%d", nAlignment );
		if ( !m_buteMgr.Exist( s_aTagName, szLabelName ) )
		{
			break;
		}
		else
		{
			// Icrement the label generating value
			nAlignment++;
		}

		// Retreive and copy the alignment 
		CButeTools::GetValidatedString( m_buteMgr, s_aTagName, szLabelName, szAlignmentBuffer, sizeof( szAlignmentBuffer ) );
		AlignmentBuffer = ConvertAlignmentNameToEnum( szAlignmentBuffer );
		pStimulus->caRequiredAlignment.push_back( AlignmentBuffer );
	}

	// Read numeric values.
    pStimulus->fDistance						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"Distance");
	pStimulus->fVerticalRadius					= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"VerticalRadius", 0.f);
    pStimulus->fDuration						= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"Duration");
    pStimulus->nAlarmLevel						= CButeTools::GetValidatedInt(m_buteMgr, s_aTagName,"AlarmLevel");
	pStimulus->fStimulationIncreaseRateAlert	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"StimulationIncreaseRateAlert", 100.f);
	pStimulus->fStimulationIncreaseRateUnalert	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"StimulationIncreaseRateUnalert", 100.f);
	pStimulus->fStimulationDecreaseRateAlert	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"StimulationDecreaseRateAlert", 100.f);
	pStimulus->fStimulationDecreaseRateUnalert	= CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName,"StimulationDecreaseRateUnalert", 100.f);
	pStimulus->nFalseStimulationLimit			= CButeTools::GetValidatedInt(m_buteMgr, s_aTagName,"FalseStimulationLimit", 0);
	pStimulus->nMaxResponders					= CButeTools::GetValidatedInt(m_buteMgr, s_aTagName,"MaxResponders", 0);
	pStimulus->bIgnoreOldStimulus				= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName,"IgnoreOldStimulus", LTTRUE);

	pStimulus->eFalseStimulusSense = kSense_InvalidType;
	if( pStimulus->nFalseStimulationLimit > 0.f )
	{
		CButeTools::GetValidatedString( m_buteMgr, s_aTagName, "FalseStimulusSense", szTemp, sizeof(szTemp), "" );
		if( szTemp[0] )
		{
			pStimulus->eFalseStimulusSense = CAISenseRecorderAbstract::SenseFromString( szTemp );
		}
	}

	pStimulus->bRequireSourceIsNotSelf			= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName,"RequireSourceIsNotSelf", LTTRUE);
	pStimulus->bRequireSourceIsSelf				= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName,"RequireSourceIsSelf", LTFALSE);

	CARange rngTemp;
	CARange rngDefault(1.f, 1.f);
	rngTemp = CButeTools::GetValidatedRange(m_buteMgr, s_aTagName, "StimulationThreshhold", rngDefault);
	pStimulus->rngStimulationThreshhold.Set((LTFLOAT)rngTemp.GetMin(), (LTFLOAT)rngTemp.GetMax());

	rngDefault.Set(0.f, 0.f);
	rngTemp = CButeTools::GetValidatedRange(m_buteMgr, s_aTagName, "ReactionDelay", rngDefault);
	pStimulus->rngReactionDelay.Set((LTFLOAT)rngTemp.GetMin(), (LTFLOAT)rngTemp.GetMax());
}

// ----------------------------------------------------------------------- //

AIBM_Stimulus* CAIButeMgr::GetStimulus(EnumAIStimulusType eStimulusType)
{

	AIBM_STIMULUS_MAP::iterator it = m_mapStimuli.find(eStimulusType);
	if( it == m_mapStimuli.end( ))
	{
		return NULL;
	}

	return it->second; 
}

// ----------------------------------------------------------------------- //

void CAIButeMgr::SetResetTimers()
{
    m_ResetTimers.fJunctionResetTimer       = CButeTools::GetValidatedDouble(m_buteMgr, "ResetTimers", "JunctionResetTimer", 10.f);
    m_ResetTimers.fSearchNodeResetTimer		= CButeTools::GetValidatedDouble(m_buteMgr, "ResetTimers", "SearchNodeResetTimer", 10.f);
    m_ResetTimers.fBodyResetTimer			= CButeTools::GetValidatedDouble(m_buteMgr, "ResetTimers", "BodyResetTimer", 10.f);
}

void CAIButeMgr::SetTextIDs()
{
    m_TextIDs.nApprehendMissionFailedID     = CButeTools::GetValidatedInt(m_buteMgr, "TextIDs", "nApprehendMissionFailedID", 0);
}

void CAIButeMgr::SetSenses()
{
    m_Senses.fNoFOVDistanceSqr              = CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "NoFOVDistance"); 
    m_Senses.fInstantSeeDistanceSqr         = CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "InstantSeeDistance"); 
    m_Senses.fThreatTooCloseDistanceSqr     = CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "ThreatTooCloseDistance"); 
    m_Senses.fPersonalBubbleDistanceSqr     = CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "PersonalBubbleDistance"); 
    m_Senses.fPersonalBubbleMultiplier		= CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "PersonalBubbleMultiplier");
    m_Senses.fDarknessMultiplier			= CButeTools::GetValidatedDouble(m_buteMgr, "Senses", "DarknessMultiplier");

	m_Senses.fNoFOVDistanceSqr *= m_Senses.fNoFOVDistanceSqr;
	m_Senses.fInstantSeeDistanceSqr *= m_Senses.fInstantSeeDistanceSqr;
	m_Senses.fThreatTooCloseDistanceSqr *= m_Senses.fThreatTooCloseDistanceSqr;
	m_Senses.fPersonalBubbleDistanceSqr *= m_Senses.fPersonalBubbleDistanceSqr;
}

void CAIButeMgr::SetRules()
{
	m_Rules.bEnableMeleeRepathing			= CButeTools::GetValidatedBool(m_buteMgr, "Rules", "EnableDeltaRepathing");
	m_Rules.bKeepInvalidNodeHistory			= CButeTools::GetValidatedBool(m_buteMgr, "Rules", "KeepInvalidNodeHistory");
	m_Rules.bUseTrueWeaponPosition			= CButeTools::GetValidatedBool(m_buteMgr, "Rules", "UseTrueWeaponPosition");
}

void CAIButeMgr::SetAttract()
{
    m_Attract.fNoTargetTime = CButeTools::GetValidatedDouble(m_buteMgr, "Attract", "NoTargetTime");
}

void CAIButeMgr::SetDEdit()
{
}



