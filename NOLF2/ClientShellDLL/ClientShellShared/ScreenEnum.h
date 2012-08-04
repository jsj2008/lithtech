// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEnum.h
//
// PURPOSE : Enums and string constants for screens.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#undef ADD_SCREEN

#if defined(INCLUDE_AS_ENUM)
	#define ADD_SCREEN(id,name) SCREEN_ID_##id,
#elif defined(INCLUDE_AS_STRING)
	#define ADD_SCREEN(id,name) #name,
#else
	#error	To use this include file, first define either INCLUDE_AS_ENUM or INCLUDE_AS_STRING, to include the screens as enums, or string constants.
#endif


ADD_SCREEN(NONE,GenericScreen)
ADD_SCREEN(MAIN,ScreenMain)

//main
ADD_SCREEN(SINGLE,ScreenSingle)
ADD_SCREEN(MULTI,ScreenMulti)
ADD_SCREEN(OPTIONS,ScreenOptions)
ADD_SCREEN(PROFILE,ScreenProfile)

//single
ADD_SCREEN(LOAD,ScreenLoad)
ADD_SCREEN(SAVE,ScreenSave)

//multi
ADD_SCREEN(PLAYER,ScreenPlayer)
ADD_SCREEN(PLAYER_SKILLS,ScreenPlayerSkills)
ADD_SCREEN(HOST,ScreenHost)
ADD_SCREEN(JOIN,ScreenJoin)
ADD_SCREEN(JOIN_LAN,ScreenJoinLAN)
ADD_SCREEN(PLAYER_TEAM,ScreenPlayerTeam)

//options
ADD_SCREEN(DISPLAY,ScreenDisplay)
ADD_SCREEN(AUDIO,ScreenAudio)
ADD_SCREEN(GAME,ScreenGame)
ADD_SCREEN(PERFORMANCE,ScreenPerformance)
ADD_SCREEN(CONTROLS,ScreenControls)

//multi - host
ADD_SCREEN(HOST_OPTIONS,ScreenHostOptions)
ADD_SCREEN(HOST_DM_OPTIONS,ScreenHostDMOptions)
ADD_SCREEN(HOST_TDM_OPTIONS,ScreenHostTDMOptions)
ADD_SCREEN(HOST_DD_OPTIONS,ScreenHostDDOptions)
ADD_SCREEN(HOST_MISSION,ScreenHostMission)
ADD_SCREEN(HOST_LEVELS,ScreenHostLevels)
ADD_SCREEN(HOST_WEAPONS,ScreenHostWeapons)
ADD_SCREEN(TEAM,ScreenTeam)

//options - game
ADD_SCREEN(CROSSHAIR,ScreenCrosshair)

//options - controls
ADD_SCREEN(CONFIGURE,ScreenConfigure)
ADD_SCREEN(MOUSE,ScreenMouse)
ADD_SCREEN(KEYBOARD,ScreenKeyboard)
ADD_SCREEN(JOYSTICK,ScreenJoystick)

//not really standard screens, but needed for layout stuff
ADD_SCREEN(FAILURE,ScreenFailure)
ADD_SCREEN(END_MISSION,ScreenEndMission)
ADD_SCREEN(END_DM_MISSION,ScreenEndDMMission)
ADD_SCREEN(END_COOP_MISSION,ScreenEndCoopMission)
ADD_SCREEN(PRELOAD,ScreenPreload)
ADD_SCREEN(POSTLOAD,ScreenPostload)

// Tron's player management screen
ADD_SCREEN(SUBROUTINES,ScreenSubroutines)

//this must be the last id
ADD_SCREEN(UNASSIGNED,GenericScreen)

