// ----------------------------------------------------------------------- //
//
// MODULE  : AISounds.cpp
//
// PURPOSE : AISounds implementation - Handle AI Sounds
//
// CREATED : 12/02/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AISounds.h"
#include "AI.h"
#include "ServerUtilities.h"
#include "iltserver.h"
#include "SoundMgr.h"
#include "..\shared\VersionMgr.h"
#include "ServerSoundMgr.h"

// Return buffer

extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSound()
//
//	PURPOSE:	Get the particular sound
//
// ----------------------------------------------------------------------- //

char* GetSound(CCharacter* pCharacter, EnumAISoundType eSound)
{
/*
	if (g_pVersionMgr->IsLowViolence())
	{
		if ( (eSound == kAIS_Death) || (eSound == kAIS_DeathQuiet) || (eSound == kAIS_Pain) )
		{
			return BUILD_NOPAIN_WAV;
		}
	}
*/

	if (!pCharacter || !g_pServerSoundMgr) return LTNULL;

	ModelId eModelId = pCharacter->GetModelId();

	char szSoundTemplate[128] = "";
	SAFE_STRCPY(szSoundTemplate,g_pModelButeMgr->GetModelSoundTemplate(eModelId));
	if (!strlen(szSoundTemplate)) return LTNULL;

	// Look for sounds in a heirarchical manner: First look for the sound in the sound 
	// template, if it isn't found there look in the parent sound template, and so on...

	char szStr[128] = "";
	g_pServerSoundMgr->GetRandomSoundFilename(szSoundTemplate, s_aszAISoundTypes[eSound], szStr, sizeof(szStr));

	// guard against loops...
	uint32 nRemainingAttempts = 10;

	while (!strlen(szStr) && nRemainingAttempts)
	{
		// Look in the parent template...
	
		char szParentTemplate[128] = "";
		g_pServerSoundMgr->GetParentSoundTemplate(szParentTemplate, 
			sizeof(szParentTemplate), szSoundTemplate);

		SAFE_STRCPY(szSoundTemplate,szParentTemplate);

		if (!strlen(szSoundTemplate)) return LTNULL;
		g_pServerSoundMgr->GetRandomSoundFilename(szSoundTemplate, s_aszAISoundTypes[eSound], szStr, sizeof(szStr));

		--nRemainingAttempts;
	}

	if (!nRemainingAttempts)
		ASSERT(!"Potential Loop in AI sound template hierarchy");

	if (strlen(szStr))
	{
		SAFE_STRCPY(s_FileBuffer, szStr);
		return s_FileBuffer;
	}

	return LTNULL;
}

CharacterSoundType GetCharacterSoundType(EnumAISoundType eSound)
{	
	switch ( eSound )
	{
		case kAIS_Death:
		case kAIS_DeathQuiet:
			return CST_DEATH;
			break;

		case kAIS_Pain:
		case kAIS_Bleeding:
		case kAIS_Burning:
		case kAIS_Choking:
		case kAIS_Poisoning:
		case kAIS_Crush:
		case kAIS_Electrocute:
		case kAIS_Explode:
		case kAIS_Fall:
		case kAIS_FallStairs:
			return CST_DAMAGE;
			break;

		case kAIS_DamageBearTrap:
		case kAIS_DamageGlue:
		case kAIS_DamageLaughing:
		case kAIS_DamageSleeping:
		case kAIS_DamageSlippery:
		case kAIS_DamageStun:
			return CST_DIALOG;
			break;

		default:
			return CST_AI_SOUND;
			break;
	}

	return CST_DIALOG;
}
