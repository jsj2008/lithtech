// ----------------------------------------------------------------------- //
//
// MODULE  : CommandIDs.h
//
// PURPOSE : Command ids
//
// CREATED : 9/18/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_IDS_H__
#define __COMMAND_IDS_H__
	

// Player movement commands ids...

#define COMMAND_ID_FORWARD			0
#define COMMAND_ID_REVERSE			1
#define COMMAND_ID_ACTIVATE			2
#define COMMAND_ID_RELOAD			3
#define COMMAND_ID_TURNAROUND		4
#define COMMAND_ID_DUCK				5
#define COMMAND_ID_JUMP				6
#define COMMAND_ID_RUN				7
#define COMMAND_ID_FIRING			8
#define COMMAND_ID_STRAFE			9
#define COMMAND_ID_LEFT				10
#define COMMAND_ID_RIGHT			11
#define COMMAND_ID_RUNLOCK			12
#define COMMAND_ID_STRAFE_LEFT		13
#define COMMAND_ID_STRAFE_RIGHT		14
#define COMMAND_ID_ALT_FIRING		15 // currently broken
#define COMMAND_ID_LOOKUP			16
#define COMMAND_ID_LOOKDOWN			17
#define COMMAND_ID_LEAN_LEFT		18
#define COMMAND_ID_LEAN_RIGHT		19


// 20 - 59 are reserved for weapons/gadgets...

#define COMMAND_ID_WEAPON_BASE		20  // 20-59
#define COMMAND_ID_WEAPON_MAX		59


// Misc. Command ids...

#define COMMAND_ID_NEXT_AMMO		60
#define COMMAND_ID_ZOOM_IN			61
#define COMMAND_ID_ZOOM_OUT			62
#define COMMAND_ID_QUICKSAVE		64
#define COMMAND_ID_QUICKLOAD		65
#define COMMAND_ID_PREV_WEAPON		66
#define COMMAND_ID_NEXT_WEAPON		67
#define COMMAND_ID_STATUS			68
#define COMMAND_ID_MISSION			69
#define COMMAND_ID_CENTERVIEW		72
#define COMMAND_ID_HOLSTER			73
#define COMMAND_ID_MESSAGE			74
#define COMMAND_ID_NEXT_LAYOUT		75
#define COMMAND_ID_PREV_LAYOUT		76
#define COMMAND_ID_INVENTORY		77
#define COMMAND_ID_NEXT_WEAPON_1	78
#define COMMAND_ID_NEXT_WEAPON_2	79
#define COMMAND_ID_NEXT_WEAPON_3	80
#define COMMAND_ID_NEXT_WEAPON_4	81
#define COMMAND_ID_NEXT_WEAPON_5	82
#define COMMAND_ID_NEXT_WEAPON_6	83
#define COMMAND_ID_CHOOSE_1			84
#define COMMAND_ID_CHOOSE_2			85
#define COMMAND_ID_CHOOSE_3			86
#define COMMAND_ID_CHOOSE_4			87
#define COMMAND_ID_CHOOSE_5			88
#define COMMAND_ID_CHOOSE_6			89
#define COMMAND_ID_NEXT_VISMODE		90
#define COMMAND_ID_PREV_VISMODE		91
#define COMMAND_ID_DUCKLOCK			92
#define COMMAND_ID_LASTWEAPON		93
#define COMMAND_ID_TEAM_MESSAGE		94

// TO2-specific commands should be moved to the 101-150 range of ID numbers
#define FIRST_TO2_COMMAND			101

#define COMMAND_ID_COMPASS			101
#define COMMAND_ID_KEYS				102
#define COMMAND_ID_INTEL			103
#define COMMAND_ID_MOVE_BODY		104
#define COMMAND_ID_FLASHLIGHT		105
#define COMMAND_ID_RADIO			106

// Tron-specific commands will be in the 151-200 range of ID numbers
#define FIRST_TRON_COMMAND			151

#define COMMAND_ID_CYCLE_HAND		152
#define COMMAND_ID_CYCLE_DISC		153
#define COMMAND_ID_CYCLE_ROD		154
#define COMMAND_ID_CYCLE_BALL		155
#define COMMAND_ID_CYCLE_MESH		156
#define COMMAND_ID_CYCLE_UTILITY	157
#define COMMAND_ID_TOGGLE_PROGRESS	158
#define COMMAND_ID_SUBROUTINE_MENU	159

// Special command ids...

#define COMMAND_ID_UNASSIGNED		201	// always leave this unassigned - used in input code


#endif // __COMMAND_IDS_H__