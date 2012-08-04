// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePackedTypes.h
//
// PURPOSE : AnimationTreePackedTypes
//
// CREATED : 6/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIMATION_TREE_PACKED_TYPES_H__
#define __ANIMATION_TREE_PACKED_TYPES_H__


//
// Enums.
//

enum AT_TREE_ID { kATTreeID_Invalid = -1 };
enum AT_ANIMATION_ID { kATAnimID_Invalid = -1 };
enum AT_TRANSITION_ID { kATTransID_Invalid = -1 };
enum AT_GLOBAL_TRANSITION_ID { kATGlobalTransID_Invalid = -1 };


//
// Lists.
//

class CAnimationTreePacked;
typedef std::vector<CAnimationTreePacked*> ANIM_TREE_PACKED_LIST;


#endif





