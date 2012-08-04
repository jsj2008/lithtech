// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumAIAwareness.h
//
// PURPOSE : Enums and string constants for AI awareness.
//
// CREATED : 06/15/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_AI_AWARENESS_H__
#define __AI_ENUM_AI_AWARENESS_H__

enum EnumAIAwareness
{
	kAware_Invalid = -1,		// Invalid awareness
	kAware_Relaxed,				// AIVolume StimulusMasks enabled, narrow FOV.
	kAware_Suspicious,			// AIVolume StimulusMasks disabled, narrow FOV, searches. 
	kAware_Alert,				// AIVolume StimulusMasks disabled, wide FOV, searches. 
	kAware_Count,				// Number of awareness values.
	kAware_Any = kAware_Count,	// Matches any awareness. 
};

#endif // __AI_ENUM_AI_AWARENESS_H__
