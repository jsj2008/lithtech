// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNavMeshLinkTypeValues.h
//
// PURPOSE : Values for NavMeshLinks.
//
// CREATED : 06/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_LINK_TYPE
#undef ADD_LINK_TYPE
#endif

#if LINK_TYPE_AS_ENUM
#define ADD_LINK_TYPE(label) kLink_##label,
#elif LINK_TYPE_AS_STRING
#define ADD_LINK_TYPE(label) #label,
#else
#error	To use this include file, first define either LINK_TYPE_AS_ENUM or LINK_TYPE_AS_STRING, to include the actions as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_LINK_TYPE(x) where 
// x is the name of the enum without the "kLink_" prefix.
// --------------------------------------------------------------------------

ADD_LINK_TYPE(Base)				// kLink_Base
ADD_LINK_TYPE(Climb)			// kLink_Climb
ADD_LINK_TYPE(Crawl)			// kLink_Crawl
ADD_LINK_TYPE(DiveThru)			// kLink_DiveThru
ADD_LINK_TYPE(Door)				// kLink_Door
ADD_LINK_TYPE(DuckRun)			// kLink_DuckRun
ADD_LINK_TYPE(DuckUnder)		// kLink_DuckUnder
ADD_LINK_TYPE(Exit)				// kLink_Exit
ADD_LINK_TYPE(FlyThru)			// kLink_FlyThru
ADD_LINK_TYPE(FlamePot)			// kLink_FlamePot
ADD_LINK_TYPE(Jump)				// kLink_Jump
ADD_LINK_TYPE(JumpOver)			// kLink_JumpOver
ADD_LINK_TYPE(LoseTarget)		// kLink_LoseTarget
ADD_LINK_TYPE(Player)			// kLink_Player
ADD_LINK_TYPE(Stairs)			// kLink_Stairs
