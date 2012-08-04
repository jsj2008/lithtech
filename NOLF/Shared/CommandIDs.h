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
#define COMMAND_ID_ALT_FIRING		15
#define COMMAND_ID_LOOKUP			16
#define COMMAND_ID_LOOKDOWN			17

// These are available for use...
#define COMMAND_ID_UNUSED			18
#define COMMAND_ID_UNUSED2			19


// 20 - 59 are reserved for weapons/gadgets...

#define COMMAND_ID_WEAPON_BASE		20  // 20-59
#define COMMAND_ID_WEAPON_MAX		59


// Misc. Command ids...

#define COMMAND_ID_NEXT_AMMO		60
#define COMMAND_ID_ZOOM_IN			61
#define COMMAND_ID_ZOOM_OUT			62
#define COMMAND_ID_FLASHLIGHT		63
#define COMMAND_ID_QUICKSAVE		64
#define COMMAND_ID_QUICKLOAD		65
#define COMMAND_ID_PREV_WEAPON		66
#define COMMAND_ID_NEXT_WEAPON		67
#define COMMAND_ID_FRAGCOUNT		68
#define COMMAND_ID_MISSION			69
#define COMMAND_ID_MOUSEAIMTOGGLE	70
#define COMMAND_ID_CROSSHAIRTOGGLE	71
#define COMMAND_ID_CENTERVIEW		72
#define COMMAND_ID_HOLSTER			73
#define COMMAND_ID_MESSAGE			74
#define COMMAND_ID_NEXT_LAYOUT		75
#define COMMAND_ID_PREV_LAYOUT		76
#define COMMAND_ID_TEAM_MESSAGE		77
#define COMMAND_ID_INVENTORY		78
#define COMMAND_ID_LEFT_SHORT		79

// Special command ids...

#define COMMAND_ID_UNASSIGNED		99	// always leave this unassigned - used in input code


#endif // __COMMAND_IDS_H__