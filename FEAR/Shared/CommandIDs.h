// ----------------------------------------------------------------------- //
//
// MODULE  : CommandIDs.h
//
// PURPOSE : Command ids
//
// CREATED : 9/18/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMANDIDS_H__
#define __COMMANDIDS_H__


// Player movement commands ids...

#define COMMAND_ID_FORWARD			0
#define COMMAND_ID_REVERSE			1
#define COMMAND_ID_FORWARD_AXIS		2
#define COMMAND_ID_STRAFE_LEFT		3
#define COMMAND_ID_STRAFE_RIGHT		4
#define COMMAND_ID_STRAFE_AXIS		5
#define COMMAND_ID_STRAFE			6
#define COMMAND_ID_PITCH_POS		7
#define COMMAND_ID_PITCH_NEG		8
#define COMMAND_ID_YAW_POS			9
#define COMMAND_ID_YAW_NEG			10
#define COMMAND_ID_PITCH			11
#define COMMAND_ID_YAW				12
#define COMMAND_ID_MENU				13	// Console "start button" command
#define COMMAND_ID_DUCK				14
#define COMMAND_ID_JUMP				15
#define COMMAND_ID_RUN				16
#define COMMAND_ID_FIRING			17
#define COMMAND_ID_RUNLOCK			18
#define COMMAND_ID_ALT_FIRING		19
#define COMMAND_ID_LEAN_LEFT		20
#define COMMAND_ID_LEAN_RIGHT		21
#define COMMAND_ID_PITCH_ACCEL		22
#define COMMAND_ID_YAW_ACCEL		23
#define COMMAND_ID_ACCEL_TURN		24
#define COMMAND_ID_STAND			25
#define COMMAND_ID_KNEEL			26
#define COMMAND_ID_FOCUS			27
#define COMMAND_ID_BLOCK			28
#define COMMAND_ID_CELLPHONE		29	// the "other button" (back on xbox, select on ps2, etc) that's the companion of "start"

// 30 - 59 are reserved for weapons...

#define COMMAND_ID_WEAPON_BASE		30 // Up to 10 Weapon Slots
#define COMMAND_ID_WEAPON_MAX		39

#define COMMAND_ID_GRENADE_BASE		40 // 3 Grenade Slots
#define COMMAND_ID_GRENADE_MAX		42

#define COMMAND_ID_HOLSTER			50


// Sonics

#define COMMAND_ID_TOGGLEMELEE		60
#define COMMAND_ID_AMMOCHECK		61
#define COMMAND_ID_STUNGUN			62


// Misc. Command ids...

#define COMMAND_ID_MEDKIT			70
#define COMMAND_ID_ZOOM_IN			71
#define COMMAND_ID_ZOOM_OUT			72
#define COMMAND_ID_NEXT_GRENADE		73
#define COMMAND_ID_QUICKSAVE		74
#define COMMAND_ID_QUICKLOAD		75
#define COMMAND_ID_PREV_WEAPON		76
#define COMMAND_ID_NEXT_WEAPON		77
#define COMMAND_ID_MISSION			78
#define COMMAND_ID_STATUS			79
#define COMMAND_ID_TOGGLE_NAVMARKER	80
#define COMMAND_ID_THROW_GRENADE	81
#define COMMAND_ID_CENTERVIEW		82
#define COMMAND_ID_MESSAGE			84
#define COMMAND_ID_NEXT_LAYOUT		85
#define COMMAND_ID_PREV_LAYOUT		86
#define COMMAND_ID_ACTIVATE			87
#define COMMAND_ID_RELOAD			88
#define COMMAND_ID_DEBUGMSG			89

#define COMMAND_ID_CHOOSE_1			91
#define COMMAND_ID_CHOOSE_2			92
#define COMMAND_ID_CHOOSE_3			93
#define COMMAND_ID_CHOOSE_4			94
#define COMMAND_ID_CHOOSE_5			95
#define COMMAND_ID_CHOOSE_6			96
#define COMMAND_ID_CHOOSE_7			97
#define COMMAND_ID_CHOOSE_8			98
#define COMMAND_ID_CHOOSE_9			99
#define COMMAND_ID_CHOOSE_0			100

#define COMMAND_ID_DROPWEAPON		102
#define COMMAND_ID_LASTWEAPON		103
#define COMMAND_ID_TEAM_MESSAGE		104
#define COMMAND_ID_MANUALAIM		105
#define COMMAND_ID_SLOWMO			106

#define COMMAND_ID_FLASHLIGHT		114
#define COMMAND_ID_RADIO			115
#define COMMAND_ID_TOOLS			116

// Special command ids...

#define COMMAND_ID_UNASSIGNED		201	// always leave this unassigned - used in input code


#endif//__COMMANDIDS_H__

