// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumStateTypes.h
//
// PURPOSE : Enums and string constants for AI state types.
//
// CREATED : 06/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_STATE_TYPES_H__
#define __AI_ENUM_STATE_TYPES_H__

//
// ENUM: Types of states.
//
enum EnumAIStateType
{
#define STATE_TYPE_AS_ENUM 1
#include "AIEnumStateTypeValues.h"
#undef STATE_TYPE_AS_ENUM

kState_Count,
};

//
// STRINGS: const strings for state types.
//
static const char* s_aszStateTypes[] =
{
#define STATE_TYPE_AS_STRING 1
#include "AIEnumStateTypeValues.h"
#undef STATE_TYPE_AS_STRING
};

//
// ENUM: State Status.
//
enum ENUM_AIStateStatus
{
kAIStateStatus_Initialized,
kAIStateStatus_Complete,
kAIStateStatus_Failed,
kAIStateStatus_Uninitialized,
};

#endif // __AI_ENUM_STATE_TYPES_H__
