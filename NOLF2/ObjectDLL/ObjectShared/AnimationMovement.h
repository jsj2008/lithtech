// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationMovement.h
//
// PURPOSE : Enums for animation movements.
//
// CREATED : 9/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#ifndef __ANIMATION_MOVEMENT_H__
#define __ANIMATION_MOVEMENT_H__

//
// ENUM: Animation Movements.
//
enum EnumAnimMovement {
	kAM_Invalid = -1,
	#define ANIM_MOVEMENT_AS_ENUM 1
	#include "AnimationMovementEnums.h"
	#undef ANIM_MOVEMENT_AS_ENUM 
	kAM_Count,
}; 

//
// STRINGS: Animation Movements.
//
static const char* s_aszAnimMovement[] =
{
	#define ANIM_MOVEMENT_AS_STRING 1
	#include "AnimationMovementEnums.h"
	#undef ANIM_MOVEMENT_AS_STRING
};

#endif