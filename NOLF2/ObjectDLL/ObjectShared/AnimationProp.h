// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationProp.h
//
// PURPOSE : Enums for animation propertires and groups.
//
// CREATED : 6/14/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#ifndef __ANIMATION_PROP_H__
#define __ANIMATION_PROP_H__

//
// ENUM: Animation Property Groups.
//
enum EnumAnimPropGroup {
	kAPG_Invalid = -1,
	#define ANIM_PROP_GROUP_AS_ENUM 1
	#include "AnimationPropGroupEnums.h"
	#undef ANIM_PROP_GROUP_AS_ENUM 
	kAPG_Count,
}; 

//
// ENUM: Animation Properties.
//
enum EnumAnimProp {
	kAP_Invalid = -1,
	#define ANIM_PROP_AS_ENUM 1
	#include "AnimationPropEnums.h"
	#undef ANIM_PROP_AS_ENUM 
	kAP_Count,
	kAP_MoodFirst	= kAP_Silent,
	kAP_MoodLast	= kAP_Disagree,
}; 


#endif