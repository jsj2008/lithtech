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
	#define ADD_AINODE_TYPE(label) kNode_##label,
#elif AINODE_TYPE_AS_STRING
	#define ADD_AINODE_TYPE(label) #label,
#else
	#error ! To use this include file, first define either AINODE_TYPE_AS_ENUM or AINODE_TYPE_AS_STRING, to include the AINode types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AINODE_TYPE(x) 
// where x is the name of the enum without the "kNode" prefix.
// --------------------------------------------------------------------------

ADD_AINODE_TYPE(Base)					// kNode_Base					0
ADD_AINODE_TYPE(Cover)					// kNode_Cover					1
ADD_AINODE_TYPE(SmartObject)			// kNode_SmartObject			2
ADD_AINODE_TYPE(View)					// kNode_View					3
ADD_AINODE_TYPE(Patrol)					// kNode_Patrol					4
ADD_AINODE_TYPE(Guard)					// kNode_Guard					5
ADD_AINODE_TYPE(Goto)					// kNode_Goto					6
ADD_AINODE_TYPE(Attackable)				// kNode_Attackable				7
ADD_AINODE_TYPE(Weapon)					// kNode_Weapon					8
ADD_AINODE_TYPE(WorkItem)				// kNode_WorkItem				9
ADD_AINODE_TYPE(Smashable)				// kNode_Smashable				10
ADD_AINODE_TYPE(DamageType)				// kNode_DamageType				11
ADD_AINODE_TYPE(Flipable)				// kNode_Flipable				12
ADD_AINODE_TYPE(MenacePlace)			// kNode_MenacePlace			13
ADD_AINODE_TYPE(Search)					// kNode_Search					14
ADD_AINODE_TYPE(NavMeshLink)			// kNode_NavMeshLink			15
ADD_AINODE_TYPE(NavMeshLinkClimb)		// kNode_NavMeshLinkClimb		16
ADD_AINODE_TYPE(NavMeshLinkCrawl)		// kNode_NavMeshLinkCrawl		17
ADD_AINODE_TYPE(NavMeshLinkDiveThru)	// kNode_NavMeshLinkDiveThru	18
ADD_AINODE_TYPE(NavMeshLinkDoor)		// kNode_NavMeshLinkDoor		19
ADD_AINODE_TYPE(NavMeshLinkDuckUnder)	// kNode_NavMeshLinkDuckUnder	20
ADD_AINODE_TYPE(NavMeshLinkJump)		// kNode_NavMeshLinkJump		21
ADD_AINODE_TYPE(NavMeshLinkJumpOver)	// kNode_NavMeshLinkJumpOver	22
ADD_AINODE_TYPE(NavMeshLinkStairs)		// kNode_NavMeshLinkStairs		23
ADD_AINODE_TYPE(PickupWeapon)			// kNode_PickupWeapon			24
ADD_AINODE_TYPE(Vehicle)				// kNode_Vehicle				25
ADD_AINODE_TYPE(Ambush)					// kNode_Ambush					26
ADD_AINODE_TYPE(Action)					// kNode_Action					27
ADD_AINODE_TYPE(Hide)					// kNode_Hide					28
ADD_AINODE_TYPE(Stalk)					// kNode_Stalk					29
ADD_AINODE_TYPE(CombatOpportunity)		// kNode_CombatOpportunity		30
ADD_AINODE_TYPE(CombatOpportunityView)	// kNode_CombatOpportunityView	31
ADD_AINODE_TYPE(Armored)				// kNode_Armored				32
ADD_AINODE_TYPE(Follow)					// kNode_Follow					33
ADD_AINODE_TYPE(DarkChant)				// kNode_DarkChant				34
ADD_AINODE_TYPE(DarkWait)				// kNode_DarkWait				35
ADD_AINODE_TYPE(NavMeshLinkFlyThru)		// kNode_NavMeshLinkDiveThru	36
ADD_AINODE_TYPE(Scanner)				// kNode_Scanner				37
ADD_AINODE_TYPE(FallBack)				// kNode_FallBack				38
ADD_AINODE_TYPE(Stimulus)				// kNode_Stimulus				39
ADD_AINODE_TYPE(Interest)				// kNode_Interest				40
ADD_AINODE_TYPE(Safety)					// kNode_Safety					41
ADD_AINODE_TYPE(SafetyFirePosition)		// kNode_SafetyFirePosition		42
ADD_AINODE_TYPE(Intro)					// kNode_Intro					43
ADD_AINODE_TYPE(Surprise)				// kNode_Surprise				44
ADD_AINODE_TYPE(NavMeshLinkDuckRun)		// kNode_NavMeshLinkDuckRun		45
ADD_AINODE_TYPE(Lead)					// kNode_Lead					46
