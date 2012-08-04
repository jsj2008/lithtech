// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationDescriptorStrings.h
//
// PURPOSE : String conversions of Anim Desc enums.
//
// CREATED : 10/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIM_DESC_STRINGS_H__
#define __ANIM_DESC_STRINGS_H__

// Anim Prop Groups.
static const char* s_aszAnimDescGroup[] =
{
#define ANIM_DESC_GROUP_AS_STRING 1
	#include "AnimationDescriptorGroupEnums.h"
#undef ANIM_DESC_GROUP_AS_STRING
};

// Anim Props.
static const char* s_aszAnimDesc[] =
{
#define ANIM_DESC_AS_STRING 1
	#include "AnimationDescriptorEnums.h"
#undef ANIM_DESC_AS_STRING
};

//----------------------------------------------------------------------------
//              
//	ROUTINE:	GetAnimationDescriptorGroupFromName()
//              
//	PURPOSE:	Returns the AniPropGroup with the passed in name, or Asserts
//				if there is not a match
//              
//----------------------------------------------------------------------------
static EnumAnimDescGroup GetAnimationDescriptorGroupFromName( const char *pszName )
{
	if( !pszName || !pszName[0] )
	{
		LTERROR( "Invalid desc group name." );
		return kADG_Invalid;
	}

	// Search group name in hash table.
	for( uint32 iDescGroup=0; iDescGroup < kADG_Count; ++iDescGroup )
	{
		if( LTStrIEquals( pszName, s_aszAnimDescGroup[iDescGroup] ))
		{
			return (EnumAnimDescGroup)iDescGroup;
		}
	}

	char szError[1024];
	LTSNPrintF( szError, LTARRAYSIZE(szError), "Invalid desc group name %s", pszName );
	LTERROR( szError );

	return kADG_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	GetAnimationDescriptorFromName()
//              
//	PURPOSE:	Returns the descriptor based on the name passed in.  May return 
//				Invalid -- this is okay, because it may be a meta Descriptor name,
//				starting with Any*.
//              
//----------------------------------------------------------------------------
static EnumAnimDesc GetAnimationDescriptorFromName( const char *pszName )
{
	if( !pszName || !pszName[0] )
	{
		LTERROR( "Invalid desc name." );
		return kAD_Invalid;
	}

	// Search for the desc name...
	for( uint32 iDesc = 0; iDesc < kAD_Count; ++iDesc )
	{
		if( LTStrIEquals( pszName, s_aszAnimDesc[iDesc] ))
		{
			return EnumAnimDesc(iDesc);
		}
	}

	return kAD_Invalid;
}

#endif // __ANIM_DESC_STRINGS_H__
