// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIUtils.h"
#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "ProjectileTypes.h"

// Globals

int g_cIntersectSegmentCalls = 0;

// Constants

const LTFLOAT c_fFOV180  = 0.0f;
const LTFLOAT c_fFOV160  = 0.1736481776669f;
const LTFLOAT c_fFOV140  = 0.3420201433257f;
const LTFLOAT c_fFOV120  = 0.5f;
const LTFLOAT c_fFOV90   = 0.70710678118654752440084436210485f;
const LTFLOAT c_fFOV75   = 0.7933533402912351645797769615013f;
const LTFLOAT c_fFOV60   = 0.86602540378443864676372317075294f;
const LTFLOAT c_fFOV45   = 0.92387953251128675612818318939679f;
const LTFLOAT c_fFOV30   = 0.9659258262890682867497431997289f;

const LTFLOAT c_fUpdateDelta         = 0.01f;
const LTFLOAT c_fDeactivationTime    = 10.0f;

const LTFLOAT c_fFacingThreshhold        = .999999f;

const char c_szKeyPickUp[]			= "PICKUP";
const char c_szKeyFireWeapon[]		= "FIRE";
const char c_szKeyBodySlump[]		= "NOISE";

const char c_szActivate[]			= "ACTIVATE";

char c_szNoReaction[]			= "Nothing";

LTBOOL FindGrenadeDangerPosition(const LTVector& vPos, LTFLOAT fDangerRadiusSqr, LTVector* pvDangerPos, CGrenade** ppGrenadeDanger)
{
	_ASSERT(pvDangerPos);

	CGrenade** ppGrenade = g_lstGrenades.GetItem(TLIT_FIRST);
	while ( ppGrenade && *ppGrenade )
	{
		CGrenade* pGrenade = *ppGrenade;

		LTVector vGrenadePosition;
		g_pLTServer->GetObjectPos(pGrenade->m_hObject, &vGrenadePosition);

		if ( vPos.DistSqr(vGrenadePosition) < fDangerRadiusSqr )
		{
			*ppGrenadeDanger = pGrenade;
			*pvDangerPos = vGrenadePosition;
			return LTTRUE;
		}
		
		ppGrenade = g_lstGrenades.GetItem(TLIT_NEXT);
	}

	return LTFALSE;
}

void ReadPropAIReactions(GenericProp& genProp, CAIReactions* pAIReactions, const char* szName)
{
	char szProp[128];

	sprintf(szProp, "%sSE1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemy[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSE", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemy[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFalse1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFalse[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFalse", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFalse[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFlashlight1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFlashlight[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFlashlight", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFlashlight[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFlashlightFalse1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFlashlightFalse[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFlashlightFalse", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFlashlightFalse[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEFootstep1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyFootstep[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEFootstep", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyFootstep[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEFootstepFalse1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyFootstepFalse[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEFootstepFalse", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyFootstepFalse[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEWeaponFire1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyWeaponFire[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEWeaponFire", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyWeaponFire[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEWeaponImpact1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyWeaponImpact[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEWeaponImpact", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyWeaponImpact[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSADeath1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeAllyDeath[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSADeath", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeAllyDeath[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFootprint1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFootprint[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sSEFootprint", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrSeeEnemyFootprint[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEDisturbance1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyDisturbance[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHEDisturbance", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearEnemyDisturbance[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHAPain1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyPain[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHAPain", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyPain[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHADeath1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyDeath[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHADeath", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyDeath[0] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHAWeaponFire1st", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyWeaponFire[1] = g_pLTServer->CreateString( genProp.m_String );
	sprintf(szProp, "%sHAWeaponFire", szName);
    if ( (g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK) && genProp.m_String[0] && strcmp(genProp.m_String, c_szNoReaction)) \
        pAIReactions->m_ahstrHearAllyWeaponFire[0] = g_pLTServer->CreateString( genProp.m_String );
}

// GetDifficultyFactor

LTFLOAT GetDifficultyFactor()
{
	if (!g_pGameServerShell) return 1.0f;

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_EASY:
			return .80f;
		break;

		case GD_NORMAL:
			return .90f;
		break;

		case GD_VERYHARD:
			return 1.10f;
		break;

		case GD_HARD:
		default :
			return 1.0f;


		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WordFilterFn
//
//	PURPOSE:	Filters out everything but the world
//
// ----------------------------------------------------------------------- //

LTBOOL WorldFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( IsMainWorld(hObj) )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GroundFilterFn
//
//	PURPOSE:	Filters out everything but potential ground candidates
//
// ----------------------------------------------------------------------- //

LTBOOL GroundFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( IsMainWorld(hObj) || (OT_WORLDMODEL == g_pLTServer->GetObjectType(hObj)) )
	{
		return LTTRUE;
	}

	return LTFALSE;
}