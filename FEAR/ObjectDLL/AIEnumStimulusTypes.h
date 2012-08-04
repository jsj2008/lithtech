// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumStimulusTypes.h
//
// PURPOSE : Enums and string constants for AI stimulus types.
//
// CREATED : 06/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_STIMULUS_TYPES_H__
#define __AI_ENUM_STIMULUS_TYPES_H__

//
// ENUM: Types of stimulus exclusive bitflags.
//
enum EnumAIStimulusType
{
	kStim_InvalidType = 0,

#define STIMULUS_TYPE_AS_FLAG 1
#include "AIEnumStimulusTypeValues.h"
#undef STIMULUS_TYPE_AS_FLAG
};

// Expand as enums to automatically get the kStim_Count.  The enum values
// here are not used.
enum 
{
#define STIMULUS_TYPE_AS_ENUM 1
#include "AIEnumStimulusTypeValues.h"
#undef STIMULUS_TYPE_AS_ENUM
	kStim_Count
};

enum EnumAIStimulusID
{
	kStimID_Invalid = -1,
	kStimID_Unset = 0,
};

enum EnumAITargetMatchID
{
	kTMID_Invalid = -1,
};

//
// STRINGS: const strings for stimulus types.
//
static const char* s_aszStimulusTypes[] =
{
#define STIMULUS_TYPE_AS_STRING 1
#include "AIEnumStimulusTypeValues.h"
#undef STIMULUS_TYPE_AS_STRING
};

#endif // __AI_ENUM_STIMULUS_TYPES_H__
