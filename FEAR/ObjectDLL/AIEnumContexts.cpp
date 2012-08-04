// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumContexts.cpp
//
// PURPOSE : 
//
// CREATED : 2/26/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

//
// STRINGS: const strings for contexts.
//
static const char* s_aszAIContexts[] =
{
	#define CONTEXT_AS_STRING 1
	#include "AIEnumContextValues.h"
	#undef CONTEXT_AS_STRING
};

static int kAP_Count = LTARRAYSIZE(s_aszAIContexts);

AIContextUtils::DynamicContextMapType AIContextUtils::ms_DynamicContextMap;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIContextUtils::Count()
//              
//	PURPOSE:	Returns the number of AIContexts.  This number may 
//				change as additional dynamic Contexts may be added.
//              
//----------------------------------------------------------------------------

int AIContextUtils::Count()
{
	return kAP_Count + ms_DynamicContextMap.size();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIContextUtils::String()
//              
//	PURPOSE:	Returns the name of the passed in Context, <invalid> 
//				if there is no such Context.
//              
//----------------------------------------------------------------------------

const char* const AIContextUtils::String( EnumAIContext eContext )
{
	// s_aszAIContexts contains the hard coded Contexts

	if ( eContext >= 0 && eContext < kAP_Count )
	{
		return s_aszAIContexts[eContext];
	}

	// ms_DynamicContextMap contains the dynamic Contexts

	if ( eContext < Count() )
	{
		int iOffset = eContext - kAP_Count;
		return ms_DynamicContextMap[iOffset].first.c_str();
	}

	// Context is invalid (out of range)

	return "<invalid>";
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIContextUtils::Enum()
//              
//	PURPOSE:	Returns the EnumAIContext enum associated with the passed in 
//				string.  If there are no current Contexts with this name, a new
//				dynamic Context will be added and its new id returned.  Any time
//				a dynamic Context is added, a message will be printed to the 
//				console which can be copy and pasted into the enum list.
//              
//----------------------------------------------------------------------------

EnumAIContext AIContextUtils::Enum( const char* const pszContextName, AIContextUtils::NotFoundAction eAction )
{
	// Passed in name is invalid.

	if( !pszContextName || !pszContextName[0] )
	{
		LTERROR( "Invalid Context name." );
		return kContext_Invalid;
	}

	// Context is a hard coded Context

	for( int iContext = 0; iContext < kAP_Count; ++iContext )
	{
		if( LTStrIEquals( pszContextName, s_aszAIContexts[iContext] ))
		{
			return (EnumAIContext)iContext;
		}
	}
	
	// Context is an existing dynamic Context

	for ( uint32 iDynamicContext = 0; iDynamicContext < ms_DynamicContextMap.size(); ++iDynamicContext )
	{
		if( LTStrIEquals( pszContextName, ms_DynamicContextMap[iDynamicContext].first.c_str() ))
		{
			return ms_DynamicContextMap[iDynamicContext].second;
		}
	}

	switch ( eAction )
	{
	case kNotFound_ReturnInvalid:
		{
			return kContext_Invalid;
		}

	case kNotFound_Add:
		{
			// Context is a new AIContext

			EnumAIContext eNewContext = (EnumAIContext)(kAP_Count + ms_DynamicContextMap.size());
			ms_DynamicContextMap.push_back( std::make_pair( pszContextName, eNewContext ) );

			// Check to make sure we aren't running under another app (ie WorldEdit) 
			// before printing out the message.

			if ( g_pLTBase )
			{
				g_pLTBase->CPrint( "ADD_CONTEXT(%s)			// kContext_%s", pszContextName, pszContextName );
			}

			return eNewContext;
		}
	default:
		{
			AIASSERT1( 0, NULL, "AIContextUtils::Enum: Invalid 'context not found' action: %d", eAction );
			return kContext_Invalid;
		}
	}
}
