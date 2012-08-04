// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationPropStrings.h
//
// PURPOSE : String conversions of Anim Prop enums.
//
// CREATED : 6/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
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

// Anim Props.
static const char* s_aszAnimProp[] =
{
	#define ANIM_PROP_AS_STRING 1
	#include "AnimationPropEnums.h"
	#undef ANIM_PROP_AS_STRING
};

#endif