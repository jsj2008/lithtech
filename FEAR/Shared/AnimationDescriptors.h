// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationDescriptors.h
//
// PURPOSE : Enums for animation descriptors and groups.
//
// CREATED : 10/15/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#ifndef __ANIMATION_DESCRIPTORS_H__
#define __ANIMATION_DESCRIPTORS_H__

//
// ENUM: Animation Descriptor Groups.
//
enum EnumAnimDescGroup 
{
	kADG_Invalid = -1,
#define ANIM_DESC_GROUP_AS_ENUM 1
#include "AnimationDescriptorGroupEnums.h"
#undef ANIM_DESC_GROUP_AS_ENUM 
	kADG_Count,
}; 

//
// ENUM: Animation Descriptors.
//
enum EnumAnimDesc 
{
	kAD_Invalid = -1,
#define ANIM_DESC_AS_ENUM 1
#include "AnimationDescriptorEnums.h"
#undef ANIM_DESC_AS_ENUM 
	kAD_Count,
}; 

#define MAX_DESC_LIST	10

typedef	std::vector<EnumAnimDesc, LTAllocator<EnumAnimDesc, LT_MEM_TYPE_OBJECTSHELL> >	ANIM_DESC_LIST;

#endif // __ANIMATION_DESCRIPTORS_H__
