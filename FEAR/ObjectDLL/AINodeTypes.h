// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeTypes.h
//
// PURPOSE : This file defines the AINodeType strings and enums.  It was
//			 moved out of AINode.h to break a few cyclic dependencies.
//
// CREATED : 2/02/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODETYPES_H_
#define _AINODETYPES_H_

//
// ENUM: Types of AI nodes.
//
enum EnumAINodeType
{
	kNode_InvalidType = -1,

	#define AINODE_TYPE_AS_ENUM 1
	#include "AINodeTypeEnums.h"
	#undef AINODE_TYPE_AS_ENUM

	kNode_Count,
};

// --------------------------------------------------------------------------

//
// ENUM: Status of AI nodes.
//
enum EnumNodeStatusFlag
{
	#define NODESTATUS_TYPE_AS_FLAG 1
	#include "AIEnumNodeStatus.h"
	#undef NODESTATUS_TYPE_AS_FLAG

	kNodeStatus_All = 0xffffffff,
};

// Expand as enums to automatically get the kNodeStatus_Count.  The enum values
// here are not used.
enum 
{
	#define NODESTATUS_AS_ENUM 1
	#include "AIEnumNodeStatus.h"
	#undef NODESTATUS_AS_ENUM

	kNodeStatus_Count,
};


//
// STRINGS: const strings for AI node types.
//
static const char* s_aszAINodeStatusFlag[] =
{
	#define NODESTATUS_TYPE_AS_STRING 1
	#include "AIEnumNodeStatus.h"
	#undef NODESTATUS_TYPE_AS_STRING
};

// --------------------------------------------------------------------------

struct AINodeUtils
{
	static EnumAINodeType GetNodeType(const char* const pszTypeName);
	static const char* const GetNodeTypeName( EnumAINodeType eNodeType );
};

#endif // _AINODETYPES_H_
