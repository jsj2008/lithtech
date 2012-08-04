// ----------------------------------------------------------------------- //
//
// MODULE  : AISounds.cpp
//
// PURPOSE : AISounds implementation - Handle AI Sounds
//
// CREATED : 12/02/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISounds.h"
#include "AI.h"
#include "ServerUtilities.h"
#include "iltserver.h"
#include "SoundMgr.h"
#include "../Shared/VersionMgr.h"
#include "ServerSoundMgr.h"
#include "AISoundDB.h"

// Return buffer

extern char s_FileBuffer[_MAX_PATH];

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

	if (!pCharacter || !g_pServerSoundMgr) return NULL;
	if (eSound == kAIS_InvalidType ) return NULL;

	ModelsDB::HMODEL hModel = pCharacter->GetModel();

	HRECORD hSoundTemplate = g_pModelsDB->GetModelSoundTemplate(hModel);
	if( !hSoundTemplate )
		return NULL;

	// Look for sounds in a heirarchical manner: First look for the sound in the sound 
	// template, if it isn't found there look in the parent sound template, and so on...

	char szStr[128] = "";
	g_pAISoundDB->GetRandomSoundFilename(hSoundTemplate, AISoundUtils::String( eSound ), szStr, sizeof(szStr));

	if (!LTStrEmpty(szStr))
	{
		LTStrCpy(s_FileBuffer, szStr, LTARRAYSIZE(s_FileBuffer));
		return s_FileBuffer;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSound()
//
//	PURPOSE:	Get the particular sound from the passed in soundset
//
// ----------------------------------------------------------------------- //

char* GetSound( HRECORD hAISoundSet )
{
	g_pAISoundDB->GetRandomSoundFilename( hAISoundSet, s_FileBuffer, LTARRAYSIZE(s_FileBuffer) );
	return s_FileBuffer;
}

CharacterSoundType GetCharacterSoundType(EnumAISoundType eSound)
{	
	switch ( eSound )
	{
		case kAIS_Death:
		case kAIS_DeathUnderWater:
		case kAIS_DeathQuiet:
			return CST_DEATH;
			break;

		case kAIS_Pain:
		case kAIS_PainUnderWater:
		case kAIS_Burning:
		case kAIS_Crush:
		case kAIS_Explode:
		case kAIS_Fall:
		case kAIS_FallStairs:
			return CST_DAMAGE;
			break;

		case kAIS_DamageStun:
			return CST_DIALOG;
			break;

		default:
			return CST_AI_SOUND;
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISoundUtils::String()
//
//	PURPOSE:	Converts the passed in AISoundType enum to a string.  If 
//				the sound is out of range or invalid, an empty string is 
//				returned.
//
// ----------------------------------------------------------------------- //

const char*	AISoundUtils::String( EnumAISoundType eAISoundType )
{
	if ( eAISoundType < 0 || eAISoundType >= kAIS_Count )
	{
		return "";
	}

	return s_aszAISoundTypes[eAISoundType];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISoundUtils::Enum()
//
//	PURPOSE:	Converts the passed in AISoundType name to an 
//				EnumAISoundType enum.  If there is no matching enum, 
//				kAIS_InvalidType is returned.
//
// ----------------------------------------------------------------------- //

EnumAISoundType	AISoundUtils::Enum( const char* pszName )
{
	for ( int i = 0; i < LTARRAYSIZE(s_aszAISoundTypes); ++i )
	{
		if ( LTStrIEquals( pszName, s_aszAISoundTypes[i] ) )
		{
			return (EnumAISoundType)i;
		}
	}

	return kAIS_InvalidType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISoundUtils::AddToStringList()
//
//	PURPOSE:	Helper function for WorldEdit string list construction.  
//				This function adds any strings starting with the passed 
//				in filter to the passed in list.  If no filter is 
//				specified, all AISoundTypes are added.
//
// ----------------------------------------------------------------------- //

void AISoundUtils::AddToStringList( char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength, const char* pszPrefixFilter )
{
	int nFilterLen = LTStrLen( pszPrefixFilter );
	for ( int i = 0; ( ( i<kAIS_Count) && ((*pcStrings)+1<cMaxStrings) ); ++i )
	{
		const char* pszAISoundTypeName = AISoundUtils::String((EnumAISoundType)i);

		if ( nFilterLen > 0 
			&& !LTSubStrIEquals( pszPrefixFilter, pszAISoundTypeName, nFilterLen ) 
			&& ( i != kAIS_None ) )
		{
			continue;
		}

		LTStrCpy( aszStrings[(*pcStrings)++], pszAISoundTypeName, cMaxStringLength );
	}
}
