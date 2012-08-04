// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNodeStatus.h
//
// PURPOSE : Enums and string constants for node status.
//
// CREATED : 2/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_STATUS_TYPE
	#undef ADD_STATUS_TYPE
#endif
 
#if NODESTATUS_TYPE_AS_STRING
	#define ADD_STATUS_TYPE(label, flag) #label,
#elif NODESTATUS_TYPE_AS_FLAG
	#define ADD_STATUS_TYPE(label, flag) kNodeStatus_##label = (1 << flag),
#elif NODESTATUS_AS_ENUM
	#define ADD_STATUS_TYPE(label, flag) kNodeStatus_DoNotUseThisValueForAutoCountOnly_##label,
#else
	#error	To use this include file, first define either STATUS_TYPE_AS_ENUM or STATUS_TYPE_AS_STRING, to include the status as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_STATUS_TYPE(x) where 
// x is the name of the enum without the "kNodeStatus_" prefix.
// --------------------------------------------------------------------------

ADD_STATUS_TYPE(None,								0)		// kNodeStatus_None
ADD_STATUS_TYPE(TooFar,								1)		// kNodeStatus_TooFar
ADD_STATUS_TYPE(Damaged,							2)		// kNodeStatus_Damaged
ADD_STATUS_TYPE(Disabled,							3)		// kNodeStatus_Disabled
ADD_STATUS_TYPE(LockedByOther,						4)		// kNodeStatus_LockedByOther
ADD_STATUS_TYPE(ThreatOutsideFOV,					5)		// kNodeStatus_ThreatOutsideFOV
ADD_STATUS_TYPE(ThreatInsideRadius,					6)		// kNodeStatus_ThreatInsideRadius
ADD_STATUS_TYPE(ThreatOutsideBoundary,				7)		// kNodeStatus_ThreatOutsideBoundary
ADD_STATUS_TYPE(ThreatOutsideRegions,				8)		// kNodeStatus_ThreatOutsideRegions
ADD_STATUS_TYPE(ThreatBlockingPath,					9)		// kNodeStatus_ThreatBlockingPath
ADD_STATUS_TYPE(ThreatAimingAtNode,					10)		// kNodeStatus_ThreatAimingAtNode
ADD_STATUS_TYPE(ThreatLookingAtNode,				11)		// kNodeStatus_ThreatLookingAtNode
ADD_STATUS_TYPE(ThreatUnseen,						12)		// kNodeStatus_ThreatUnseen
ADD_STATUS_TYPE(SearchedRecently,					13)		// kNodeStatus_SearchedRecently
ADD_STATUS_TYPE(RequiresPartner,					14)		// kNodeStatus_RequiresPartner
ADD_STATUS_TYPE(Expired,							15)		// kNodeStatus_Expired
ADD_STATUS_TYPE(ThreatInAIFOV,						16)		// kNodeStatus_ThreatInAIFOV
ADD_STATUS_TYPE(ThreatIsAtDisadvantage,				17)		// kNodeStatus_ThreatIsAtDisadvantage
ADD_STATUS_TYPE(ThreatInUnexpectedLocation,			18)		// kNodeStatus_ThreatInUnexpectedLocation
ADD_STATUS_TYPE(ThreatEngagedInMelee,				19)		// kNodeStatus_ThreatEngagedInMelee
ADD_STATUS_TYPE(ThreatLookingAtNodeOutsideNodeFOV,	20)		// kNodeStatus_ThreatLookingAtNodeOutsideNodeFOV
ADD_STATUS_TYPE(ThreatAwareOfPosition,				21)		// kNodeStatus_ThreatAwareOfPosition
ADD_STATUS_TYPE(CombatOpportunity,					22)		// kNodeStatus_CombatOpportunity
ADD_STATUS_TYPE(TargetisCombatOpportunity,			23)		// kNodeStatus_TargetisCombatOpportunity
ADD_STATUS_TYPE(AIOutsideFOV,						24)		// kNodeStatus_AIOutsideFOV
ADD_STATUS_TYPE(AIOutsideRadiusOrRegion,			25)		// kNodeStatus_AIOutsideRadiusOrRegion
ADD_STATUS_TYPE(AIBackToNode,						26)		// kNodeStatus_AIBackToNode
ADD_STATUS_TYPE(OwnerNotLocked,						27)		// kNodeStatus_OwnerNotLocked
ADD_STATUS_TYPE(PlayerOnNode,						28)		// kNodeStatus_PlayerOnNode
ADD_STATUS_TYPE(ValidForFollow,						29)		// kNodeStatus_ValidForFollow
ADD_STATUS_TYPE(Avoid,								30)		// kNodeStatus_Avoid
