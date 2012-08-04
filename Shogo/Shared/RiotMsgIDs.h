// ----------------------------------------------------------------------- //
//
// MODULE  : RiotMsgIDs.h
//
// PURPOSE : Riot message ids
//
// CREATED : 9/22/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOT_MSG_IDS_H__
#define __RIOT_MSG_IDS_H__

#include "SFXMsgIds.h"				// Special fx message ids

// General Client <-> Server messages

#define MID_PLAYER_UPDATE				100
#define MID_PLAYER_ONGROUND				101
#define MID_PLAYER_MODECHANGE			102
#define MID_PLAYER_EXITLEVEL			103
#define MID_PLAYER_INFOCHANGE			104
#define	MID_PLAYER_VELMAGNITUDE			105
#define	MID_PLAYER_STATE_CHANGE			106
#define MID_PLAYER_MESSAGE				107
#define MID_PLAYER_CHEAT				108
#define MID_PLAYER_ORIENTATION			109
#define MID_PLAYER_ADDED				110
#define MID_PLAYER_REMOVED				111
#define MID_PLAYER_FRAGGED				112
#define MID_PLAYER_INITVARS				113
#define MID_PLAYER_3RDPERSON			114
#define MID_PLAYER_RESPAWN				115
#define MID_PLAYER_MULTIPLAYER_INIT		116
#define MID_PLAYER_SINGLEPLAYER_INIT	117
#define MID_PLAYER_LOADCLIENT			118
#define MID_PLAYER_DAMAGE				119

#define MID_CAMERA_FOV					120
#define MID_CAMERA_RECT					121
#define MID_CAMERA_CIRCLE				122
#define MID_CAMERA_CHASEVIEW			123

#define MID_WEAPON_CHANGE				130
#define MID_WEAPON_STATE				131
#define MID_WEAPON_SOUND				132
#define MID_WEAPON_FIRE					133

#define MID_COMMAND_TOGGLE				140
#define MID_COMMAND_SHOWGAMEMSG			141
#define MID_COMMAND_SHOWDLG				142
#define MID_COMMAND_TRANSMISSION		143
#define MID_COMMAND_OBJECTIVE			144

#define MID_MUSIC						153
#define MID_DIALOG_CLOSE				154

#define MID_GAME_PAUSE					160
#define MID_GAME_UNPAUSE				161

#define MID_POWERUP_PICKEDUP			170
#define MID_POWERUP_EXPIRED				171

#define MID_LOAD_GAME					180
#define MID_SAVE_GAME					181

#define MID_SERVER_ERROR				190

#define MID_CHANGING_LEVELS				200

#define	MID_SINGLEPLAYER_START			210

#define MID_PHYSICS_UPDATE				212	// Server to client
#define MID_FRAG_SELF					213 // Client to server
#define MID_TRACTORBEAM_POS				214 // Client to server
#define MID_SERVERFORCEPOS				215 // Server to client
#define MID_TIMEOFDAYCOLOR				216 // Server to client
#define MID_PINGTIMES					217 // Server to client
#define MID_PLAYER_AUTOSAVE				218	// Server to client
#define MID_SFX_REG						219

// Sent from client to server when a transimission trigger ends
#define MID_TRANSMISSIONENDED			220

// Object <-> Object messages...

#define MID_PLAYDIALOG					2000

// Interface Change ids shared between client and server...

#define IC_AMMO_ID				1
#define IC_HEALTH_ID			2
#define IC_ARMOR_ID				3
#define IC_WEAPON_ID			4
#define IC_WEAPON_PICKUP_ID		5
#define IC_AIRLEVEL_ID			6
#define IC_ROCKETLOCK_ID		7
#define IC_OUTOFAMMO_ID			8


// Client update flags

#define	CLIENTUPDATE_PLAYERROT		(1<<0)
#define	CLIENTUPDATE_WEAPONROT		(1<<1)
#define	CLIENTUPDATE_CONTCODE		(1<<2)
#define	CLIENTUPDATE_3RDPERSON		(1<<3)
#define	CLIENTUPDATE_3RDPERVAL		(1<<4)
#define CLIENTUPDATE_ALLOWINPUT		(1<<5)
#define CLIENTUPDATE_EXTERNALCAMERA	(1<<6)

// Music commands
enum MusicCommand { MUSICCMD_LEVEL, MUSICCMD_MOTIF, MUSICCMD_MOTIF_LOOP, MUSICCMD_MOTIF_STOP, MUSICCMD_BREAK };



#endif // __RIOT_MSG_IDS_H__