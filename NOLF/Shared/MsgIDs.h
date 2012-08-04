// ----------------------------------------------------------------------- //
//
// MODULE  : MsgIDs.h
//
// PURPOSE : Message ids
//
// CREATED : 9/22/97
//
// ----------------------------------------------------------------------- //

#ifndef __MSG_IDS_H__
#define __MSG_IDS_H__

#include "SFXMsgIds.h"				// Special fx message ids

// General Client <-> Server messages

#define MID_PLAYER_UPDATE				100
#define MID_PLAYER_SUMMARY				101 // Both ways
#define MID_PLAYER_EXITLEVEL			102 // Both ways
#define MID_PLAYER_INFOCHANGE			103
#define MID_PLAYER_AUTOSAVE				104	// Server to client
#define	MID_PLAYER_STATE_CHANGE			105
#define MID_PLAYER_MESSAGE				106
#define MID_PLAYER_CHEAT				107
#define MID_PLAYER_ORIENTATION			108
#define MID_PLAYER_ADDED				109
#define MID_PLAYER_REMOVED				110
#define MID_PLAYER_FRAGGED				111
#define MID_PLAYER_INITVARS				112
#define MID_PLAYER_RESPAWN				113
#define MID_PLAYER_MULTIPLAYER_INIT		114
#define MID_PLAYER_SINGLEPLAYER_INIT	115
#define MID_PLAYER_LOADCLIENT			116
#define MID_PLAYER_DAMAGE				117
#define MID_PLAYER_ACTIVATE				118
#define MID_PLAYER_CLIENTMSG			119	// Client to server
#define MID_PLAYER_CHANGETEAM			120 // Server to client
#define MID_PLAYER_TEAMMESSAGE			121
#define MID_PLAYER_MULTIPLAYER_CHANGE	122	// Client to server
#define MID_PLAYER_TRANSMISSION			123 // Server to client
#define MID_PLAYER_CHATMODE				124	// Client to server
#define MID_PLAYER_POPUPTEXT			125 // Server to client
#define MID_PLAYER_GHOSTMESSAGE			126
#define MID_PLAYER_CREDITS				127
#define MID_UPDATE_OPTIONS				128 // Client to server
#define MID_PLAYER_CONNECTED			129

#define MID_WEAPON_CHANGE				130	  // Both ways
#define MID_WEAPON_SOUND				131
#define MID_WEAPON_FIRE					132

#define MID_TEAM_SCORED					135
#define MID_PLAYER_TAUNT				136
#define	MID_MULTIPLAYER_UPDATE			137

#define MID_COMMAND_TOGGLE				140

#define MID_MUSIC						153

#define MID_GAME_PAUSE					160
#define MID_GAME_UNPAUSE				161

#define MID_GEAR_PICKEDUP				170

#define MID_LOAD_GAME					180 // Client to server
#define MID_SAVE_GAME					181 // Client to server

#define MID_MISSION_INFO				185	// Client to server

#define MID_SERVER_ERROR				190

#define MID_CHANGING_LEVELS				200
#define	MID_SINGLEPLAYER_START			201
#define	MID_MULTIPLAYER_DATA			202

#define MID_PHYSICS_UPDATE				212	// Server to client
#define MID_FRAG_SELF					213 // Client to server
#define MID_SERVERFORCEPOS				214 // Server to client
#define MID_TIMEOFDAYCOLOR				215 // Server to client
#define MID_PINGTIMES					216 // Server to client
#define MID_SHAKE_SCREEN				217 // Client to server
#define MID_CHANGE_FOG					218	// Server to client

#define MID_CLIENTFX_CREATE				230 // Server to client
#define MID_SFX_MESSAGE					231 // Server to client

#define	MID_CONSOLE_TRIGGER				235	// Client to server
#define MID_EDIT_OBJECTINFO				236	// Server to client
#define MID_CONSOLE_COMMAND				237	// Client to server

#define MID_DIFFICULTY					240 // Client to server
#define MID_GAMETEXMGR					241	// Server to client
#define MID_DISPLAY_TIMER				242	// Server to client						
#define MID_DISPLAY_METER				243	// Server to client						
#define MID_FADEBODIES					244 // Client to server

#define MID_HANDSHAKE					245 // Both ways, handshake negotiation
// Handshake sub-messages
	#define MID_HANDSHAKE_HELLO			0 // Hello! (version verification)  Both ways
	#define MID_HANDSHAKE_PASSWORD		1 // Wassa password?  Server to client
	#define MID_HANDSHAKE_LETMEIN		2 // What, this password?  Client to server
	#define MID_HANDSHAKE_DONE			3 // You'll do just fine.  Server to client
	// Handshake version state

// Object <-> Object messages...


// Interface Change ids shared between client and server...These are
// used with above MID_PLAYER_INFOCHANGE

#define IC_AMMO_ID				1
#define IC_HEALTH_ID			2
#define IC_ARMOR_ID				3
#define IC_WEAPON_ID			4
#define IC_WEAPON_PICKUP_ID		5
#define IC_AIRLEVEL_ID			6
#define IC_ROCKETLOCK_ID		7
#define IC_OUTOFAMMO_ID			8
#define IC_OBJECTIVE_ID			9
#define IC_MOD_PICKUP_ID		10
#define IC_FADE_SCREEN_ID		11
#define IC_RESET_INVENTORY_ID	12
#define IC_MISSION_TEXT_ID		13
#define IC_MISSION_FAILED_ID	14
#define IC_MAX_HEALTH_ID		15
#define IC_MAX_ARMOR_ID			16
#define IC_WEAPON_OBTAIN_ID		17
#define IC_INTEL_PICKUP_ID		18

#define OBJECTIVE_ADD_ID		0
#define OBJECTIVE_REMOVE_ID		1
#define OBJECTIVE_COMPLETE_ID	2
#define OBJECTIVE_CLEAR_ID		3


// Client to player messages...These are used with MID_PLAYER_CLIENTMSG

// Motion Status is used to notify the server if the motion of the player
// has changed (did he jump/land/fall)....
#define CP_MOTION_STATUS		1
	#define MS_NONE				0
	#define MS_JUMPED			1
	#define MS_FALLING			2
	#define MS_LANDED			3

// Damage is used to inflict damage the player (e.g., fall damage)...

#define CP_DAMAGE				2

// Weapon Status is used to tell the server what animation the player-view
// weapon is playing...

#define CP_WEAPON_STATUS		3
	#define WS_NONE				0
	#define WS_RELOADING		1
	#define WS_SELECT			2
	#define WS_DESELECT			3

// Flashlight is used to simply send new info regarding the location of the 
// flashlight beam impact

#define CP_FLASHLIGHT			4
	#define FL_ON				0
	#define FL_OFF				1
	#define FL_UPDATE			2

// Zipcord is used to notify the server that the zipcord is being turned on/off

#define CP_ZIPCORD				5
	#define ZC_ON				0
	#define ZC_OFF				1

// New model is defined by PlayerPhysicsModel...

#define CP_PHYSICSMODEL			6

// Damage from vehicle impact

#define CP_DAMAGE_VEHICLE_IMPACT	7

// Client update flags

#define	CLIENTUPDATE_PLAYERROT		(1<<0)
#define	CLIENTUPDATE_CONTCODE		(1<<1)
#define	CLIENTUPDATE_3RDPERSON		(1<<2)
#define	CLIENTUPDATE_3RDPERVAL		(1<<3)
#define CLIENTUPDATE_ALLOWINPUT		(1<<4)

#endif // __MSG_IDS_H__