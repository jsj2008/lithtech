// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeTypes.cpp
//
// PURPOSE : 
//
// CREATED : 3/15/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeTypes.h"

//
// STRINGS: const strings for AI node types.
//
static const char* s_aszAINodeTypes[] =
{
	#define AINODE_TYPE_AS_STRING 1
	#include "AINodeTypeEnums.h"
	#undef AINODE_TYPE_AS_STRING
};

EnumAINodeType AINodeUtils::GetNodeType(const char* const pszTypeName)
{
	if (NULL == pszTypeName)
	{
		return kNode_InvalidType;
	}

	for (int i = 0; i < kNode_Count; ++i)
	{
		if (LTStrIEquals(s_aszAINodeTypes[i], pszTypeName))
		{
			return (EnumAINodeType)i;
		}
	}

	return kNode_InvalidType;
}


const char* const AINodeUtils::GetNodeTypeName( EnumAINodeType eNodeType )
{
	if ( kNode_InvalidType == eNodeType )
		return "Invalid";

	if ( eNodeType < 0 || eNodeType >= kNode_Count )
	{
		return "OutOfBounds";
	}

	return s_aszAINodeTypes[eNodeType];
}
