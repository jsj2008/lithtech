// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AISounds.h"
#include "AI.h"
#include "ServerUtilities.h"
#include "iltserver.h"
#include "SoundMgr.h"

// Tags - this must be kept in the same order as the AISound enumeration

const static char* s_aszTags[] =
{
	"Death",
	"DeathQuiet",
	"Pain",
	"Patrol",
	"Charge",
	"Backup",
	"CombAggr",
	"CombDef",
	"Disturb",
	"GaveUp",
	"Search",
	"SeeBody",

	// Specific

	"Bark",
	"Bite",
	"Tail",
	"IngeSing",
	"PoodleRun",
	"PoodleSpray",
	"Ticket",
};

const static uint32 s_cTags = sizeof(s_aszTags)/sizeof(const char*);

// Return buffer

extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

void FudgeModelId(ModelId& eModelId, ModelStyle& eModelStyle)
{
	// Fudge whacky model/style combinations into generic modelbute entries
	// in order to fake nationalities

	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMission = pMissionData->GetMissionNum();
	int nLevel = pMissionData->GetLevelNum();

/** PER LEVEL NATIONALITY OVERRIDES

HARMGUARDS:

m10s01 - american
m10s02 - american
m10s03 - american
m10s04 - ameriacn

DOCKGUARDS

m10s01 - american

**/

	switch ( eModelId )
	{
		case 15: // DOCKGUARD
		{
			switch (nMission)
			{
				case 18: // M10
				{
					switch (nLevel)
					{
						case 0: // S01
						case 1: // S02
						case 2: // S03
						case 3: // S04
							eModelId = (ModelId)38; // american
							break;
					}
				}
				break;
/*
				case 8: // M05
				{
					switch (nLevel)
					{
						case 0: // S01
						case 1: // S02
						case 2: // S03
						case 3: // S04
						case 4: // S05
							eModelId = (ModelId)40; // germanic
							break;
					}
				}
				break;
*/
			}
		}
		break;

		case 14: // HARMGUARD
		{
			switch (nMission)
			{
				case 16: // M09
				{
					switch (nLevel)
					{
						case 2: // S03
							eModelId = (ModelId)38; // american
							break;
					}
				}
				break;

				case 18: // M10
				{
					switch (nLevel)
					{
						case 0: // S01
							eModelId = (ModelId)38; // american
							break;
					}
				}
				break;

			}

		}
		break;

		case 27: // MALEBYSTANDER
		{
			switch ( eModelStyle )
			{
				case 14:
					eModelId = (ModelId)41; // moroccan
					return;

				case 0:
					eModelId = (ModelId)38; // american
					return;

				case 2:
					eModelId = (ModelId)39; // english
					return;

				case 12:
					if ( nMission == 12 && nLevel == 0 ) // m07s01
					{
						eModelId = (ModelId)39; // english
					}
					else if ( nMission == 18 && nLevel == 3 ) // m10s04
					{
						eModelId = (ModelId)38; // american
					}
					else
					{
						eModelId = (ModelId)40; // germanic
					}
					return;

				case 3:
					eModelId = (ModelId)40; // germanic
					return;
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSound()
//
//	PURPOSE:	Get the particular sound
//
// ----------------------------------------------------------------------- //

char* GetSound(CCharacter* pCharacter, AISound ais)
{
	if (g_pVersionMgr->IsLowViolence())
	{
		if ( (ais == aisDeath) || (ais == aisDeathQuiet) || (ais == aisPain) )
		{
			return BUILD_NOPAIN_WAV;
		}
	}

	if (!pCharacter || !g_pServerSoundMgr) return LTNULL;

	ModelId eModelId = pCharacter->GetModelId();
	ModelStyle eModelStyle = pCharacter->GetModelStyle();

	FudgeModelId(eModelId, eModelStyle);

	const char* szCharacterName = g_pModelButeMgr->GetModelName(eModelId);
	if (!szCharacterName) return LTNULL;

	CString str = g_pServerSoundMgr->GetRandomSoundFilename(szCharacterName, s_aszTags[ais]);

	if (str.GetLength() > 0)
	{
		strcpy(s_FileBuffer, str.GetBuffer(0));
		return s_FileBuffer;
	}
	else
	{
		const char* szAINationality = g_pModelButeMgr->GetModelNationality(eModelId);
		if (!szAINationality) return LTNULL;

		str = g_pServerSoundMgr->GetRandomSoundFilename(szAINationality, s_aszTags[ais]);

		if (str.GetLength() > 0)
		{
			strcpy(s_FileBuffer, str.GetBuffer(0));
			return s_FileBuffer;
		}
		else
		{
			const char* szAISex = g_pModelButeMgr->GetModelSex(eModelId);
			if (!szAISex) return LTNULL;

			str = g_pServerSoundMgr->GetRandomSoundFilename(szAISex, s_aszTags[ais]);

			if (str.GetLength() > 0)
			{
				strcpy(s_FileBuffer, str.GetBuffer(0));
				return s_FileBuffer;
			}
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheAISounds()
//
//	PURPOSE:	Cache all the sounds used by the particular AI
//
// ----------------------------------------------------------------------- //

void CacheAISounds(CCharacter* pCharacter)
{
	if (!g_pServerSoundMgr || !pCharacter) return;

	ModelId eModelId = pCharacter->GetModelId();
	ModelStyle eModelStyle = pCharacter->GetModelStyle();

	FudgeModelId(eModelId, eModelStyle);

	char* szCharacterName = (char*)g_pModelButeMgr->GetModelName(eModelId);
	char* szSex = (char*)g_pModelButeMgr->GetModelSex(eModelId);
	char* szNationality = (char*)g_pModelButeMgr->GetModelNationality(eModelId);

	for ( uint32 iTag = 0 ; iTag < s_cTags ; iTag++ )
	{
		if ( szCharacterName )
		{
			g_pServerSoundMgr->CacheSounds(szCharacterName, s_aszTags[iTag]);
		}

		if ( szNationality )
		{
			g_pServerSoundMgr->CacheSounds(szNationality, s_aszTags[iTag]);
		}

		if ( szSex )
		{
			g_pServerSoundMgr->CacheSounds(szSex, s_aszTags[iTag]);
		}
	}
}

AISound SenseToFailSound(SenseType st)
{
// $SOUND 
	return aisGaveUp;
/*
	switch ( st )
	{
		case stSeeEnemy:
		case stSeeEnemyFlashlight:
		case stSeeEnemyFootprint:
		case stSeeAllyDeath:
			return aisCheckEnemyFail;
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		case stHearAllyDeath:
		case stHearAllyPain:
		case stHearAllyWeaponFire:
		default:
			return aisCheckNoiseFail;
	}
*/
}

CCharacter::CharacterSoundType GetCharacterSoundType(AISound ais)
{
	switch ( ais )
	{
		case aisDeath:
		case aisDeathQuiet:
		case aisPain:
		case aisBark:
		case aisBite:
		case aisTail:
		case aisIngeSing:
		case aisPoodleRun:
		case aisPoodleSpray:
		case aisTicket:
			return CCharacter::CST_DIALOG;
			break;

		case aisPatrol:
		case aisCharge:
		case aisBackup:
		case aisCombAggr:
		case aisCombDef:
		case aisDisturb:
		case aisGaveUp:
		case aisSearch:
		case aisSeeBody:
			return CCharacter::CST_AI_SOUND;
			break;
	}

	return CCharacter::CST_DIALOG;
}
