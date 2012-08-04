// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_SOUNDS_H__
#define __AI_SOUNDS_H__

#include "CharacterAlignment.h"
#include "Character.h"

class CAI;

enum AISound
{
	aisDeath			= 0,
	aisDeathQuiet,
	aisPain,
	aisPatrol,
	aisCharge,
	aisBackup,
	aisCombAggr,
	aisCombDef,
	aisDisturb,
	aisGaveUp,
	aisSearch,
	aisSeeBody,

	// Specific

	aisBark,
	aisBite,
	aisTail,
	aisIngeSing,
	aisPoodleRun,
	aisPoodleSpray,
	aisTicket,
	aisLast,
};

enum
{
	kNumAISounds		= aisLast,
};

extern AISound SenseToFailSound(SenseType st);

char* GetSound(CCharacter* pCharacter, AISound ais);
CCharacter::CharacterSoundType GetCharacterSoundType(AISound ais);

void CacheAISounds(CCharacter* pCharacter);

#endif // __AI_SOUNDS_H__
