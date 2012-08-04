// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNodeDependencyTypes.h
//
// PURPOSE : Enums and string constants for AI node dependency types.
//
// CREATED : 08/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_NODE_DEPENDENCY_TYPES_H__
#define __AI_ENUM_NODE_DEPENDENCY_TYPES_H__


//
// ENUM: Types of node dependency type.
//
enum EnumAINodeDependencyType
{
	kDependency_InvalidType = -1,
	#define DEPENDENCY_TYPE_AS_ENUM 1
	#include "AIEnumNodeDependencyTypeValues.h"
	#undef DEPENDENCY_TYPE_AS_ENUM

	kDependency_Count,
};

//
// STRINGS: const strings for node dependency types.
//
static const char* s_aszNodeDependencyTypes[] =
{
	#define DEPENDENCY_TYPE_AS_STRING 1
	#include "AIEnumNodeDependencyTypeValues.h"
	#undef DEPENDENCY_TYPE_AS_STRING
};

#endif // __AI_ENUM_NODE_DEPENDENCY_TYPES_H__
