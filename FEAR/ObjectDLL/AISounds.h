// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_SOUNDS_H__
#define __AI_SOUNDS_H__

class CCharacter;
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
char* GetSound( HRECORD hAISoundSet );
CharacterSoundType GetCharacterSoundType(EnumAISoundType eSound);

struct AISoundUtils
{
	// Returns the name of the passed in AISoundType.  If there is no match, 
	// an empty string is returned
	static const char*		String( EnumAISoundType eAISoundType );

	// Returns the AISoundType enum associated with the passed in name.  If 
	// no match exists, kAIS_InvalidType is returned.
	static EnumAISoundType	Enum( const char* pszAISoundTypeName );

	// Appends the all AISoundTypes to the passed in list.  If a filter string 
	// is specified, only None and AISoundTypes with this filter as a prefix 
	// will be added.
	static void				AddToStringList( char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength, const char* pszPrefixFilter );
};

#endif // __AI_SOUNDS_H__

