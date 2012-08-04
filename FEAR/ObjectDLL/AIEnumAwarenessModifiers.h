// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumAwarenessModifiers.h
//
// PURPOSE : Enums and string constants for AI awareness modifiers.
//
// CREATED : 08/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_AWARENESSMODS_H__
#define __AI_ENUM_AWARENESSMODS_H__


//
// ENUM: Awareness modifiers.
//
enum EnumAIAwarenessMod
{
	kAwarenessMod_Invalid = -1,
	#define AWARENESSMOD_AS_ENUM 1
	#include "AIEnumAwarenessModifierValues.h"
	#undef AWARENESSMOD_AS_ENUM

	kAwarenessMod_Count,
};

//
// STRINGS: const strings for awareness modifiers.
//
static const char* s_aszAIAwarenessMods[] =
{
	#define AWARENESSMOD_AS_STRING 1
	#include "AIEnumAwarenessModifierValues.h"
	#undef AWARENESSMOD_AS_STRING
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StringToAwarenessMod
//
//	PURPOSE:	Handle converting an AwarenessMod string to an 
//			enum.  If the enum value is 'None', kAwarenessMod_Invalid
//			is returned.  If the value is not found in the list,
//			an assert is triggered and kAwarenessMod_Invalid 
//			returned.
//
// ----------------------------------------------------------------------- //

static EnumAIAwarenessMod StringToAwarenessMod( const char* pszAwareness )
{
	// String pointer is invalid

	if ( !pszAwareness )
	{
		AIASSERT( pszAwareness, NULL, "StringToAwarenessMod : Invalid Awareness pointer." );
		return kAwarenessMod_Invalid;
	}

	// Check for the 'invalid' awareness mod.

	if( LTStrIEquals( pszAwareness, "None" ) )
	{
		return kAwarenessMod_Invalid;
	}

	// Find a match.

	for( uint32 iMod=0; iMod < kAwarenessMod_Count; ++iMod )
	{
		if( LTStrIEquals( pszAwareness, s_aszAIAwarenessMods[iMod] ) )
		{
			return (EnumAIAwarenessMod)iMod;
		}
	}

	// No match.  Assert, as either None or one of the valid names is required.

	AIASSERT1( 0, NULL, "StringToAwarenessMod : Unrecognized awareness modifier '%s'", pszAwareness );
	return kAwarenessMod_Invalid;
}

#endif // __AI_ENUM_AWARENESSMOD_H__
