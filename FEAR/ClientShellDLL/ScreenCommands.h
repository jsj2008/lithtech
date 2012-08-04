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
	CMD_UP,
	CMD_DOWN,
	CMD_MAIN,
	CMD_OK,
	CMD_CANCEL,
	CMD_UPDATE,

	CMD_OPTIONS,

//  single
	CMD_NEW_GAME,
	CMD_LEVEL1,
	CMD_LEVEL2,
	CMD_LEVEL3,
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

	//	multiplayer commands.
	CMD_EDIT_BANDWIDTH,
	CMD_LAUNCH,
	CMD_WEAPONS,


//	multi-host-levels
	CMD_ADD_LEVEL,
	CMD_REMOVE_LEVEL,
	CMD_ADD_ALL,
	CMD_REMOVE_ALL,

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
	CMD_GPU,
	CMD_CPU,
	CMD_ADVANCED_CPU,
	CMD_ADVANCED_GPU,
	CMD_AUTO,

// options-performance-cpu
	CMD_PHYSICS,
	CMD_SOUNDS,

// options-performance-gpu
	CMD_EFFECTS,
	CMD_GRAPHICS,

// options-controls-configure
	CMD_MOVE_COM,
	CMD_INV_COM,
	CMD_VIEW_COM,
	CMD_MISC_COM,
	CMD_WEAP_COM,
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
