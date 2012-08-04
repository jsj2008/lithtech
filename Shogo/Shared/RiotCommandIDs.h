// ----------------------------------------------------------------------- //
//
// MODULE  : RiotCommandIDs.h
//
// PURPOSE : Riot command ids
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOT_COMMAND_IDS_H__
#define __RIOT_COMMAND_IDS_H__
	

// Player movement commands ids...

#define COMMAND_ID_FORWARD			0
#define COMMAND_ID_REVERSE			1
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
#define COMMAND_ID_SPECIAL_MOVE		15
#define COMMAND_ID_LOOKUP			16
#define COMMAND_ID_LOOKDOWN			17
#define COMMAND_ID_DOUBLEJUMP		18

#define COMMAND_ID_DECSCREENRECT	20
#define COMMAND_ID_INCSCREENRECT	21

#define COMMAND_ID_SCREENSHOT		61
#define COMMAND_ID_QUICKSAVE		62
#define COMMAND_ID_QUICKLOAD		63

// Weapon selection command ids...

#define COMMAND_ID_PREV_WEAPON		68
#define COMMAND_ID_NEXT_WEAPON		69
#define COMMAND_ID_WEAPON_1			70
#define COMMAND_ID_WEAPON_2			71
#define COMMAND_ID_WEAPON_3			72
#define COMMAND_ID_WEAPON_4			73
#define COMMAND_ID_WEAPON_5			74
#define COMMAND_ID_WEAPON_6			75
#define COMMAND_ID_WEAPON_7			76
#define COMMAND_ID_WEAPON_8			77
#define COMMAND_ID_WEAPON_9			78
#define COMMAND_ID_WEAPON_10		79


// Misc. command ids...

#define COMMAND_ID_CAMERACIRCLE		50
#define COMMAND_ID_FRAGCOUNT		84
#define COMMAND_ID_SHOWORDINANCE	87
#define COMMAND_ID_INTERFACETOGGLE	88
#define COMMAND_ID_MOUSEAIMTOGGLE	90
#define COMMAND_ID_CROSSHAIRTOGGLE	91
#define COMMAND_ID_CENTERVIEW		92
#define COMMAND_ID_CHASEVIEWTOGGLE	93
#define COMMAND_ID_MENUTOGGLE		94
#define COMMAND_ID_POSE				95
#define COMMAND_ID_MESSAGE			96
#define COMMAND_ID_DROPUPGRADE		97
#define COMMAND_ID_VEHICLETOGGLE	98
#define COMMAND_ID_UNASSIGNED		99	// always leave this unassigned - used in input code

#define CHEAT_ID_FLASHLIGHT			48

#endif // __RIOT_COMMAND_IDS_H__