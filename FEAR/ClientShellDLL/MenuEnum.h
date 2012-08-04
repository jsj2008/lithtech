// ----------------------------------------------------------------------- //
//
// MODULE  : MenuEnum.h
//
// PURPOSE : Enums and string constants for menus.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//


#undef ADD_MENU

#if defined(INCLUDE_AS_ENUM)
	#define ADD_MENU(id,name) MENU_ID_##id,
#elif defined(INCLUDE_AS_STRING)
	#define ADD_MENU(id,name) #name,
#else
	#error	To use this include file, first define either INCLUDE_AS_ENUM or INCLUDE_AS_STRING, to include the menus as enums, or string constants.
#endif


ADD_MENU(NONE,GenericMenu)
ADD_MENU(SYSTEM,MenuSystem)
ADD_MENU(ENDROUND,MenuEndRound)
ADD_MENU(MISSION,MenuMission)

//this must be the last id
ADD_MENU(UNASSIGNED,GenericMenu)

