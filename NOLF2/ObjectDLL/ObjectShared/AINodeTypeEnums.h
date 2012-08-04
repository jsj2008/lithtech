// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeTypeEnums.h
//
// PURPOSE : Enums and string constants for AINodes.
//
// CREATED : 7/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AINODE_TYPE
	#undef ADD_AINODE_TYPE
#endif
 
#if AINODE_TYPE_AS_ENUM
	#define ADD_AINODE_TYPE(label) kNode_##label##,
#elif AINODE_TYPE_AS_STRING
	#define ADD_AINODE_TYPE(label) #label,
#else
	#error ! To use this include file, first define either AINODE_TYPE_AS_ENUM or AINODE_TYPE_AS_STRING, to include the AINode types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AINODE_TYPE(x) 
// where x is the name of the enum without the "kNode" prefix.
// --------------------------------------------------------------------------

ADD_AINODE_TYPE(Base)				// kNode_Base
ADD_AINODE_TYPE(Cover)				// kNode_Cover
ADD_AINODE_TYPE(Search)				// kNode_Search
ADD_AINODE_TYPE(Panic)				// kNode_Panic
ADD_AINODE_TYPE(Vantage)			// kNode_Vantage
ADD_AINODE_TYPE(VantageRoof)		// kNode_VantageRoof
ADD_AINODE_TYPE(UseObject)			// kNode_UseObject
ADD_AINODE_TYPE(PickupObject)		// kNode_PickupObject
ADD_AINODE_TYPE(Backup)				// kNode_Backup
ADD_AINODE_TYPE(Training)			// kNode_Training
ADD_AINODE_TYPE(TrainingFailure)	// kNode_TrainingFailure
ADD_AINODE_TYPE(TrainingSuccess)	// kNode_TrainingSuccess
ADD_AINODE_TYPE(Tail)				// kNode_Tail
ADD_AINODE_TYPE(View)				// kNode_View
ADD_AINODE_TYPE(Assassinate)		// kNode_Assassinate
ADD_AINODE_TYPE(Patrol)				// kNode_Patrol
ADD_AINODE_TYPE(Guard)				// kNode_Guard
ADD_AINODE_TYPE(Obstruct)			// kNode_Obstruct
ADD_AINODE_TYPE(Talk)				// kNode_Talk
ADD_AINODE_TYPE(ExitLevel)			// kNode_ExitLevel
ADD_AINODE_TYPE(Death)				// kNode_Death
ADD_AINODE_TYPE(Goto)				// kNode_Goto
ADD_AINODE_TYPE(Sensing)			// kNode_Sensing

//
// Special enums for types of UseObject nodes.
//

ADD_AINODE_TYPE(BeginUseObject)		// kNode_BeginUseObject
ADD_AINODE_TYPE(Alarm)				// kNode_Alarm
ADD_AINODE_TYPE(Attackable)			// kNode_Attackable
ADD_AINODE_TYPE(Disturbance)		// kNode_Disturbance
ADD_AINODE_TYPE(Edible)				// kNode_Edible
ADD_AINODE_TYPE(LightSwitch)		// kNode_LightSwitch
ADD_AINODE_TYPE(Weapon)				// kNode_Weapon
ADD_AINODE_TYPE(WorkItem)			// kNode_WorkItem
ADD_AINODE_TYPE(Bed)				// kNode_Bed
ADD_AINODE_TYPE(PostingPlace)		// kNode_PostingPlace
ADD_AINODE_TYPE(Examinable)			// kNode_Examinable
ADD_AINODE_TYPE(Smashable)			// kNode_Smashable
ADD_AINODE_TYPE(Ride)				// kNode_Ride
ADD_AINODE_TYPE(DamageType)			// kNode_DamageType
ADD_AINODE_TYPE(Sniper)				// kNode_Sniper
ADD_AINODE_TYPE(Coverable)			// kNode_Coverable
ADD_AINODE_TYPE(MenacePlace)		// kNode_MenacePlace
ADD_AINODE_TYPE(EndUseObject)		// kNode_EndUseObject

