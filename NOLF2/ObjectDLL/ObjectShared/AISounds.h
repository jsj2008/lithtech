// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_SOUNDS_H__
#define __AI_SOUNDS_H__

#include "CharacterAlignment.h"
#include "Character.h"
#include "AISenseRecorderAbstract.h"

class CAI;

//
// ENUM: Types of AI sounds.
//
enum EnumAISoundType
{
	kAIS_InvalidType = -1,

	#define AISOUND_TYPE_AS_ENUM 1
	#include "AISoundTypeEnums.h"
	#undef AISOUND_TYPE_AS_ENUM

	kAIS_Count,
};

//
// STRINGS: const strings for AI sound types.
//
static const char* s_aszAISoundTypes[] =
{
	#define AISOUND_TYPE_AS_STRING 1
	#include "AISoundTypeEnums.h"
	#undef AISOUND_TYPE_AS_STRING
};


char* GetSound(CCharacter* pCharacter, EnumAISoundType eSound);
CharacterSoundType GetCharacterSoundType(EnumAISoundType eSound);

#endif // __AI_SOUNDS_H__
