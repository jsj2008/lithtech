// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIButeMgr.h"
#include "CommonUtilities.h"


// Globals/statics

CAIButeMgr* g_pAIButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

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

LTBOOL CAIButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pAIButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

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

	SetSenses();
	SetAttract();
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
//	ROUTINE:	CAIButeMgr::Set*
//
//	PURPOSE:	Sets an attribute
//
// ----------------------------------------------------------------------- //

void CAIButeMgr::SetTemplate(int iTemplate)
{
	sprintf(s_aTagName, "AttributeTemplate%d", iTemplate);

	strcpy(m_aTemplates[iTemplate].szName, m_buteMgr.GetString(s_aTagName, "Name"));

	char szAlignment[128];
	strcpy(szAlignment, m_buteMgr.GetString(s_aTagName, "Alignment"));
	if ( !strcmp(szAlignment, "GOOD") )		m_aTemplates[iTemplate].ccAlignment = GOOD;
	else if ( !strcmp(szAlignment, "BAD") )		m_aTemplates[iTemplate].ccAlignment = BAD;
	else if ( !strcmp(szAlignment, "NEUTRAL") )	m_aTemplates[iTemplate].ccAlignment = NEUTRAL;
    else g_pLTServer->CPrint("Illegal alignment attribute for template %d", iTemplate);

    m_aTemplates[iTemplate].fRunSpeed                       = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "RunSpeed");
    m_aTemplates[iTemplate].fCreepSpeed                     = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "CreepSpeed");
    m_aTemplates[iTemplate].fWalkSpeed                      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "WalkSpeed");
    m_aTemplates[iTemplate].fRollSpeed                      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "RollSpeed");
    m_aTemplates[iTemplate].fSwimSpeed                      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SwimSpeed");
    m_aTemplates[iTemplate].fFlySpeed                       = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "FlySpeed");
    m_aTemplates[iTemplate].fMarksmanship                   = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Marksmanship");
    m_aTemplates[iTemplate].fBravado                        = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Bravado");
    m_aTemplates[iTemplate].fAwareness                      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Awareness");
    m_aTemplates[iTemplate].fSoundRadius                    = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SoundRadius");
    m_aTemplates[iTemplate].fHitPoints                      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HitPoints");
    m_aTemplates[iTemplate].fArmor							= (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Armor");
    m_aTemplates[iTemplate].fAccuracy                       = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Accuracy");
    m_aTemplates[iTemplate].fAccuracyIncreaseRate           = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "AccuracyIncreaseRate");
    m_aTemplates[iTemplate].fAccuracyDecreaseRate           = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "AccuracyDecreaseRate");
    m_aTemplates[iTemplate].fLag                            = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "Lag");
    m_aTemplates[iTemplate].fLagIncreaseRate                = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "LagIncreaseRate");
    m_aTemplates[iTemplate].fLagDecreaseRate                = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "LagDecreaseRate");
    m_aTemplates[iTemplate].bCanSeeEnemy                    = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanSeeEnemy");
    m_aTemplates[iTemplate].fSeeEnemyDistance               = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SeeEnemyDistance");
    m_aTemplates[iTemplate].bCanSeeAllyDeath                = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanSeeAllyDeath");
    m_aTemplates[iTemplate].fSeeAllyDeathDistance           = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SeeAllyDeathDistance");
    m_aTemplates[iTemplate].bCanHearAllyDeath               = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearAllyDeath");
    m_aTemplates[iTemplate].fHearAllyDeathDistance          = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearAllyDeathDistance");
    m_aTemplates[iTemplate].bCanHearAllyPain                = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearAllyPain");
    m_aTemplates[iTemplate].fHearAllyPainDistance           = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearAllyPainDistance");
    m_aTemplates[iTemplate].bCanHearEnemyFootstep           = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearEnemyFootstep");
    m_aTemplates[iTemplate].fHearEnemyFootstepDistance      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearEnemyFootstepDistance");
    m_aTemplates[iTemplate].bCanSeeEnemyFootprint           = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanSeeEnemyFootprint");
    m_aTemplates[iTemplate].fSeeEnemyFootprintDistance      = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SeeEnemyFootprintDistance");
    m_aTemplates[iTemplate].bCanHearEnemyWeaponImpact       = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearEnemyWeaponImpact");
    m_aTemplates[iTemplate].fHearEnemyWeaponImpactDistance  = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearEnemyWeaponImpactDistance");
    m_aTemplates[iTemplate].bCanHearEnemyWeaponFire         = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearEnemyWeaponFire");
    m_aTemplates[iTemplate].fHearEnemyWeaponFireDistance    = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearEnemyWeaponFireDistance");
    m_aTemplates[iTemplate].bCanHearAllyWeaponFire			= (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearAllyWeaponFire");
    m_aTemplates[iTemplate].fHearAllyWeaponFireDistance		= (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearAllyWeaponFireDistance");
    m_aTemplates[iTemplate].bCanSeeEnemyFlashlight          = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanSeeEnemyFlashlight");
    m_aTemplates[iTemplate].fSeeEnemyFlashlightDistance     = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "SeeEnemyFlashlightDistance");
    m_aTemplates[iTemplate].bCanHearEnemyDisturbance        = (LTBOOL)m_buteMgr.GetBool(s_aTagName,      "CanHearEnemyDisturbance");
    m_aTemplates[iTemplate].fHearEnemyDisturbanceDistance   = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "HearEnemyDisturbanceDistance");
    m_aTemplates[iTemplate].fBarkDistance                   = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "BarkDistance");
    m_aTemplates[iTemplate].fAttackDistance                 = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName,   "AttackDistance");
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

	strcpy(m_aBrains[iBrain].szName, m_buteMgr.GetString(s_aTagName, "Name"));

    m_aBrains[iBrain].fDodgeVectorCheckTime				= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorCheckTime");
    m_aBrains[iBrain].fDodgeVectorCheckChance			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorCheckChance");

    m_aBrains[iBrain].fDodgeVectorShuffleChance			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorShuffleChance");
    m_aBrains[iBrain].fDodgeVectorRollChance			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorRollChance");
    m_aBrains[iBrain].fDodgeVectorCoverChance			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorCoverChance");

    m_aBrains[iBrain].fDodgeVectorDelayTime				= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeVectorDelayTime");

    m_aBrains[iBrain].fDodgeProjectileCheckTime			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeProjectileCheckTime");
    m_aBrains[iBrain].fDodgeProjectileCheckChance		= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeProjectileCheckChance");
    m_aBrains[iBrain].fDodgeProjectileDelayTime			= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeProjectileDelayTime");
    m_aBrains[iBrain].fDodgeProjectileFleeChance		= (LTFLOAT)m_buteMgr.GetDouble	(s_aTagName,"DodgeProjectileFleeChance");

	m_aBrains[iBrain].fAttackVectorRangeMin				= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackVectorRange").GetMin();
    m_aBrains[iBrain].fAttackVectorRangeMax				= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackVectorRange").GetMax();
	m_aBrains[iBrain].fAttackProjectileRangeMin			= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackProjectileRange").GetMin();
    m_aBrains[iBrain].fAttackProjectileRangeMax			= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackProjectileRange").GetMax();
	m_aBrains[iBrain].fAttackMeleeRangeMin				= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackMeleeRange").GetMin();
    m_aBrains[iBrain].fAttackMeleeRangeMax				= (LTFLOAT)m_buteMgr.GetRange	(s_aTagName,"AttackMeleeRange").GetMax();
    m_aBrains[iBrain].fAttackChaseTime					= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackChaseTime");
    m_aBrains[iBrain].fAttackChaseTimeRandomMin			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"AttackChaseTimeRandom").GetMin();
    m_aBrains[iBrain].fAttackChaseTimeRandomMax			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"AttackChaseTimeRandom").GetMax();
    m_aBrains[iBrain].fAttackPoseCrouchChance			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackPoseCrouchChance");
    m_aBrains[iBrain].fAttackGrenadeThrowTime			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackGrenadeThrowTime");
    m_aBrains[iBrain].fAttackGrenadeThrowChance			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackGrenadeThrowChance");
    m_aBrains[iBrain].fAttackCoverCheckTime				= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackCoverCheckTime");
    m_aBrains[iBrain].fAttackCoverCheckChance			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackCoverCheckChance");
    m_aBrains[iBrain].fAttackSoundChance				= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackSoundChance");

    m_aBrains[iBrain].fAttackFromCoverCoverTime         = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverCoverTime");
    m_aBrains[iBrain].fAttackFromCoverCoverTimeRandom   = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverCoverTimeRandom");
    m_aBrains[iBrain].fAttackFromCoverUncoverTime       = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverUncoverTime");
    m_aBrains[iBrain].fAttackFromCoverUncoverTimeRandom = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverUncoverTimeRandom");
    m_aBrains[iBrain].fAttackFromCoverReactionTime      = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverReactionTime");
	m_aBrains[iBrain].nAttackFromCoverMaxRetries		= m_buteMgr.GetInt				(s_aTagName,"AttackFromCoverMaxRetries");
    m_aBrains[iBrain].fAttackFromCoverSoundChance		= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromCoverSoundChance");

    m_aBrains[iBrain].fAttackFromVantageAttackTime      = (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromVantageAttackTime");
    m_aBrains[iBrain].fAttackFromVantageAttackTimeRandom= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromVantageAttackTimeRandom");

    m_aBrains[iBrain].fAttackFromViewChaseTime			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"AttackFromViewChaseTime");

    m_aBrains[iBrain].fChaseSoundTime					= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"ChaseSoundTime");
    m_aBrains[iBrain].fChaseSoundTimeRandomMin			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"ChaseSoundTimeRandom").GetMin();
    m_aBrains[iBrain].fChaseSoundTimeRandomMax			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"ChaseSoundTimeRandom").GetMax();
    m_aBrains[iBrain].fChaseFoundSoundChance			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"ChaseFoundSoundChance");
    m_aBrains[iBrain].fChaseExtraTime					= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"ChaseExtraTime");
    m_aBrains[iBrain].fChaseExtraTimeRandomMin			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"ChaseExtraTimeRandom").GetMin();
    m_aBrains[iBrain].fChaseExtraTimeRandomMax			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"ChaseExtraTimeRandom").GetMax();
    m_aBrains[iBrain].fChaseSearchTime					= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"ChaseSearchTime");

    m_aBrains[iBrain].fPatrolSoundTime					= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"PatrolSoundTime");
    m_aBrains[iBrain].fPatrolSoundTimeRandomMin			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"PatrolSoundTimeRandom").GetMin();
    m_aBrains[iBrain].fPatrolSoundTimeRandomMax			= (LTFLOAT)m_buteMgr.GetRange   (s_aTagName,"PatrolSoundTimeRandom").GetMax();
    m_aBrains[iBrain].fPatrolSoundChance				= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"PatrolSoundChance");

    m_aBrains[iBrain].fDistressFacingThreshhold			= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"DistressFacingThreshhold");
    m_aBrains[iBrain].fDistressIncreaseRate				= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"DistressIncreaseRate");
    m_aBrains[iBrain].fDistressDecreaseRate				= (LTFLOAT)m_buteMgr.GetDouble  (s_aTagName,"DistressDecreaseRate");
	m_aBrains[iBrain].nDistressLevels					= m_buteMgr.GetInt				(s_aTagName,"DistressLevels");
}

#define SENSE_STIMULATION_READ(sense) \
    m_Senses.f##sense##StimulationIncreaseRateAlert     = (LTFLOAT)m_buteMgr.GetDouble("Senses", #sense"StimulationIncreaseRateAlert");  \
    m_Senses.f##sense##StimulationIncreaseRateUnalert   = (LTFLOAT)m_buteMgr.GetDouble("Senses", #sense"StimulationIncreaseRateUnalert");  \
    m_Senses.f##sense##StimulationDecreaseRateAlert     = (LTFLOAT)m_buteMgr.GetDouble("Senses", #sense"StimulationDecreaseRateAlert");  \
    m_Senses.f##sense##StimulationDecreaseRateUnalert   = (LTFLOAT)m_buteMgr.GetDouble("Senses", #sense"StimulationDecreaseRateUnalert");  \
    m_Senses.rng##sense##StimulationThreshhold.Set((LTFLOAT)m_buteMgr.GetRange("Senses", #sense"StimulationThreshhold").GetMin(), \
                                                   (LTFLOAT)m_buteMgr.GetRange("Senses", #sense"StimulationThreshhold").GetMax()); \
	m_Senses.n##sense##FalseStimulationLimit			= (int)m_buteMgr.GetDouble("Senses", #sense"FalseStimulationLimit");

#define SENSE_DELAY_READ(sense) \
    m_Senses.f##sense##ReactionDelay        = (LTFLOAT)m_buteMgr.GetDouble("Senses", #sense"ReactionDelay");  \

void CAIButeMgr::SetSenses()
{
    m_Senses.fAllyDeathNoiseDistance        = (LTFLOAT)m_buteMgr.GetDouble("Senses", "AllyDeathNoiseDistance");
    m_Senses.fAllyPainNoiseDistance         = (LTFLOAT)m_buteMgr.GetDouble("Senses", "AllyPainNoiseDistance");
    m_Senses.fEnemyMovementNoiseDistance    = (LTFLOAT)m_buteMgr.GetDouble("Senses", "EnemyMovementNoiseDistance");
    m_Senses.fCoinNoiseDistance             = (LTFLOAT)m_buteMgr.GetDouble("Senses", "CoinNoiseDistance");
    m_Senses.fNoFOVDistanceSqr              = (LTFLOAT)m_buteMgr.GetDouble("Senses", "NoFOVDistance"); m_Senses.fNoFOVDistanceSqr *= m_Senses.fNoFOVDistanceSqr;
    m_Senses.fInstantSeeDistanceSqr         = (LTFLOAT)m_buteMgr.GetDouble("Senses", "InstantSeeDistance"); m_Senses.fInstantSeeDistanceSqr *= m_Senses.fInstantSeeDistanceSqr;

	SENSE_STIMULATION_READ(SeeEnemy);
	SENSE_DELAY_READ(SeeEnemyFootprint);
	SENSE_STIMULATION_READ(SeeEnemyFlashlight);
	SENSE_DELAY_READ(HearEnemyWeaponFire);
	SENSE_DELAY_READ(HearEnemyWeaponImpact);
	SENSE_STIMULATION_READ(HearEnemyFootstep);
	SENSE_DELAY_READ(HearEnemyDisturbance);
	SENSE_DELAY_READ(SeeAllyDeath);
	SENSE_DELAY_READ(HearAllyDeath);
	SENSE_DELAY_READ(HearAllyPain);
	SENSE_DELAY_READ(HearAllyWeaponFire);
}

void CAIButeMgr::SetAttract()
{
    m_Attract.fNoTargetTime = (LTFLOAT)m_buteMgr.GetDouble("Attract", "NoTargetTime");
}

void CAIButeMgr::SetDEdit()
{
}