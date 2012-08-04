// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationPropStrings.h
//
// PURPOSE : String conversions of Anim Prop enums.
//
// CREATED : 6/13/01
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIM_PROP_STRINGS_H__
#define __ANIM_PROP_STRINGS_H__

// Anim Prop Groups.
static const char* s_aszAnimPropGroup[] =
{
	#define ANIM_PROP_GROUP_AS_STRING 1
	#include "AnimationPropGroupEnums.h"
	#undef ANIM_PROP_GROUP_AS_STRING
};

//----------------------------------------------------------------------------
//              
//	ROUTINE:	GetAnimationPropGroupFromName()
//              
//	PURPOSE:	Returns the AniPropGroup with the passed in name, or Asserts
//				if there is not a match
//              
//----------------------------------------------------------------------------
static EnumAnimPropGroup GetAnimationPropGroupFromName( const char *pszName )
{
	if( !pszName || !pszName[0] )
	{
		LTERROR( "Invalid prop group name." );
		return kAPG_Invalid;
	}

	// Search group name in hash table.
	for( uint32 iPropGroup=0; iPropGroup < kAPG_Count; ++iPropGroup )
	{
		if( LTStrIEquals( pszName, s_aszAnimPropGroup[iPropGroup] ))
		{
			return (EnumAnimPropGroup)iPropGroup;
		}
	}

	char szError[1024];
	LTSNPrintF( szError, LTARRAYSIZE(szError), "Invalid prop group name %s", pszName );
	LTERROR( szError );

	return kAPG_Invalid;
}

//----------------------------------------------------------------------------
//              
//	CLASS:		AnimPropUtils
//              
//	PURPOSE:	Handle wrapping various animation prop manipulations.
//              
//----------------------------------------------------------------------------

struct AnimPropUtils
{
	typedef std::vector< std::pair< std::string, EnumAnimProp > > DynamicPropMapType;

public:
	// Returns the number of animation props.  This number may change as 
	// additional dynamic props may be added.
	static int					Count();

	// Returns the name of the passed in animation prop, <invalid> if there is 
	// no such prop.
	static const char* const	String( EnumAnimProp eProp );

	// Returns the EnumAnimProp enum associated with the passed in string.  If 
	// there are no current props with this name, a new dynamic prop will be 
	// added and its new id returned.  Any time a dynamic prop is added, a 
	// message will be printed to the console which can be copy and pasted into
	// the enum list.
	static EnumAnimProp			Enum( const char* const pszPropName );

	// Used to synchronize animprops between the client and server.
	static void					Sync( const char* const pszPropName, EnumAnimProp eProp, int iOffset );

private:

	// Accessor used to wrap client/server access (to mask duplicity issues with Xenon)
	static DynamicPropMapType& GetDynamicPropMap();

#if defined(_CLIENTBUILD)
	static DynamicPropMapType	ms_ClientDynamicPropMap;
#endif

#if defined(_SERVERBUILD)
	static DynamicPropMapType	ms_ServerDynamicPropMap;
#endif

};

// Keep this around to avoid changelist co-mingling.

static EnumAnimProp GetAnimationPropFromName( const char* const pszString )
{
	return AnimPropUtils::Enum( pszString );
}

#endif
