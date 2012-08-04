// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenCommands.h
//
// PURPOSE : Enumerate interface screen commands
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCREENCOMMANDS_H
#define SCREENCOMMANDS_H

///////////////////////////////////////
// The command IDs for the screen options

enum eScreenCmds
{
	CMD_NONE,

//	Reusable
	CMD_BACK,
	CMD_CONTINUE,
	CMD_LEFT,
	CMD_RIGHT,
	CMD_MAIN,
	CMD_OK,
	CMD_CANCEL,
	CMD_UPDATE,

//  Main menu
	CMD_SINGLE_PLAYER,
	CMD_CONTINUE_GAME,
	CMD_MULTI_PLAYER,
	CMD_MULTI_PLAYER_LAN,
	CMD_COOP,
	CMD_DM,
	CMD_TEAM_DM,
	CMD_DOOM,
	CMD_OPTIONS,
	CMD_RESUME,
	CMD_PROFILE,
	CMD_EXIT, 
	CMD_QUIT,

//  single
	CMD_NEW_GAME,
	CMD_LOAD_GAME,
	CMD_SAVE_GAME,
	CMD_CUSTOM_LEVEL,
	CMD_CHAPTER,
	CMD_EASY,
	CMD_MEDIUM,
	CMD_HARD,
	CMD_INSANE,

//  single-save
	CMD_OVERWRITE,
	CMD_EDIT_NAME,

//  multi
	CMD_PLAYER,
	CMD_HOST,
	CMD_JOIN,
	CMD_JOIN_LAN,

//	multi-player
	CMD_SKILLS,

//	multi-host
	CMD_EDIT_PASS,
	CMD_EDIT_SCMDPASS,
	CMD_EDIT_PORT,
	CMD_TOGGLE_PASS,
	CMD_TOGGLE_SCMDPASS,
	CMD_SET_OPTIONS,
	CMD_CHOOSE_CAMPAIGN,
	CMD_SET_LEVELS,
	CMD_EDIT_BANDWIDTH,
	CMD_LAUNCH,
	CMD_WEAPONS,

//	multi-host-levels
	CMD_ADD_LEVEL,
	CMD_REMOVE_LEVEL,
	CMD_ADD_ALL,
	CMD_REMOVE_ALL,

//	multi-join
	CMD_SEARCH,
	CMD_EDIT_CDKEY,

//	options
	CMD_DISPLAY,	
	CMD_AUDIO,		
	CMD_CONTROLS,	
	CMD_GAME,		
	CMD_PERFORMANCE,

//	options-controls
	CMD_CONFIGURE,
	CMD_MOUSE,
	CMD_KEYBOARD,
	CMD_JOYSTICK,
	CMD_RESET_DEFAULTS,

// options-game
	CMD_CROSSHAIR,

// options-performance
	CMD_SFX,
	CMD_TEXTURE,

// options-controls-configure
	CMD_MOVE_COM,
	CMD_INV_COM,
	CMD_VIEW_COM,
	CMD_MISC_COM,
	CMD_CHANGE_CONTROL,

// profile
	CMD_LOAD,
	CMD_CONFIRM,
	CMD_DELETE,
	CMD_CREATE,
	CMD_RENAME,
	CMD_EDIT,

	CMD_CUSTOM, //this needs to be the last cmd, so that custom commands may be defined after it
};



#endif // SCREENCOMMANDS_H
