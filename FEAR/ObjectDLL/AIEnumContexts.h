// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumContexts.h
//
// PURPOSE : Enums and string constants for AI contexts.
//
// CREATED : 08/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_CONTEXTS_H__
#define __AI_ENUM_CONTEXTS_H__


//
// ENUM: Contexts.
//
enum EnumAIContext
{
	kContext_Invalid = -1,
	#define CONTEXT_AS_ENUM 1
	#include "AIEnumContextValues.h"
	#undef CONTEXT_AS_ENUM
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AIContextUtils
//              
//	PURPOSE:	Handle wrapping various animation Context manipulations.
//              
//----------------------------------------------------------------------------

struct AIContextUtils
{
	typedef std::vector< std::pair< std::string, EnumAIContext > > DynamicContextMapType;

public:

	// Returns the number of animation Contexts.  This number may change as 
	// additional dynamic Contexts may be added.
	static int					Count();

	// Returns the name of the passed in animation Context, <invalid> if there is 
	// no such Context.
	static const char* const	String( EnumAIContext eContext );

	// Returns the EnumAIContext enum associated with the passed in string.  If 
	// there are no current Contexts with this name, a new dynamic Context will be 
	// added and its new id returned.  Any time a dynamic Context is added, a 
	// message will be printed to the console which can be copy and pasted into
	// the enum list.
	enum NotFoundAction
	{
		kNotFound_Add,
		kNotFound_ReturnInvalid
	};

	static EnumAIContext			Enum( const char* const pszContextName, NotFoundAction eAction );

private:

	static DynamicContextMapType	ms_DynamicContextMap;
};
#endif // __AI_ENUM_CONTEXTS_H__
