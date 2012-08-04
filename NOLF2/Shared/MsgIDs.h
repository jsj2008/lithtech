// ----------------------------------------------------------------------- //
//
// MODULE  : MsgIDs.h
//
// PURPOSE : Message ids
//
// CREATED : 9/22/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MSG_IDS_H__
#define __MSG_IDS_H__

#include "SFXMsgIds.h"              // Special fx message ids

// General Client <-> Server messages

#define MID_PLAYER_UPDATE				100
#define MID_PLAYER_SKILLS				101 // Both ways
#define MID_PLAYER_SUMMARY				102 
#define MID_PLAYER_INFOCHANGE			103 // Both ways
#define MID_PLAYER_SCORE				104
#define	MID_PLAYER_STATE_CHANGE			105
#define MID_PLAYER_MESSAGE				106
#define MID_PLAYER_CHEAT				107

#define MID_PLAYER_ORIENTATION			108	// Server to client
// Orientation sub-messages
	#define MID_ORIENTATION_ALL			0	// Pitch, Yaw, Roll
	#define MID_ORIENTATION_YAW			1	// Yaw only! (Pitch amd Roll preserved on client)

#define MID_PLAYER_INFO					109
	//player info sub-messages
	#define	MID_PI_JOIN					0	// this player is joining
	#define	MID_PI_EXIST				1	// this is info about an existing player sent to someone joining
	#define MID_PI_UPDATE				2	// this is an update of existing players

#define MID_PLAYER_REMOVED				110
#define MID_PLAYER_SCORED				111
#define MID_PLAYER_RESPAWN				112
#define MID_PLAYER_CANCELREVIVE			113
#define MID_PLAYER_MULTIPLAYER_INIT		114
#define MID_PLAYER_SINGLEPLAYER_INIT	115
#define MID_PLAYER_LOADCLIENT			116
#define MID_PLAYER_DAMAGE				117
#define MID_PLAYER_ACTIVATE				118
//Activation sub-messages
	#define MID_ACTIVATE_NORMAL			0
	#define MID_ACTIVATE_SEARCH			1
	#define MID_ACTIVATE_MOVE			2
	#define MID_ACTIVATE_REVIVE			3
	#define MID_ACTIVATE_WAKEUP			4

#define MID_PLAYER_CLIENTMSG			119	// Client to server
#define MID_PLAYER_OVERLAY				120 // Server to client
#define MID_PLAYER_TELEPORT				121 // Client to server

#define MID_PLAYER_TRANSMISSION			123 // Server to client
#define MID_PLAYER_CHATMODE				124	// Client to server
#define MID_PLAYER_GHOSTMESSAGE			125
#define MID_PLAYER_CREDITS				126
#define MID_PLAYER_TEAM					127
#define MID_PLAYER_CONNECTED			128

#define MID_WEAPON_CHANGE				130	  // Both ways
#define MID_WEAPON_SOUND				131
#define MID_WEAPON_SOUND_LOOP			132
#define MID_WEAPON_FIRE					133
#define MID_WEAPON_RELOAD				134


#define MID_COMPILE						135	// Client to Server (ratings update)

	#define MWEAPFIRE_VECTOR              0
	#define MWEAPFIRE_PROJECTILE          1
	#define MWEAPFIRE_DISC                2

///////////////////////////////////////////
// Light cycle messages for TRON
#define MID_LIGHT_CYCLE_INFO			136	// Client <-> Server

	#define LCI_ADD_CYCLIST				1
	#define LCI_REMOVE_CYCLIST			2
	#define LCI_DEREZ_CYCLIST			3
///////////////////////////////////////////

#define	MID_PERFORMANCE					137


// Messages regarding projectiles
#define MID_PROJECTILE                          138
	#define MPROJ_UPDATE_POINT_TARGET             0 // Client to Server- give a projectile a new point target
	#define MPROJ_UPDATE_OBJECT_TARGET            1
	#define MPROJ_UPDATE_CONTROL_LINE             2 // Client to server- give a projectile a new control line
	#define MPROJ_RETURN                          3 // Client to Server- return the projectile to the owner, if applicable
	#define MPROJ_RETURNING                       4 // Server to Client- weapon is returning
	#define MPROJ_RETURNING_DISTANCE_THRESHOLD    5 // Server to Client- sent to an AI owner when returning disc is a certain distance from that owner
	#define MPROJ_RETURNED                        6 // Server to Client- weapon has returned, unlock it
	#define MPROJ_START_SENDING_MESSAGES          7
	#define MPROJ_STOP_SENDING_MESSAGES           8
	#define MPROJ_BLOCKED                         9 // Server to Client- player has successfully blocked the disc
	#define MPROJ_START_SWAT_BLOCK               10
	#define MPROJ_START_HOLD_BLOCK               11
	#define MPROJ_END_HOLD_BLOCK                 12
	#define MPROJ_START_ARM_BLOCK                13
	// types of Disc tracking
	#define MPROJ_DISC_TRACKING_HOMING          100
	#define MPROJ_DISC_TRACKING_STEADY          101
	#define MPROJ_DISC_TRACKING_POINT_GUIDED    102
	#define MPROJ_DISC_TRACKING_CONTROL_LINE    103

#define MID_CLIENT_LOADED						139
#define MID_SWITCHINGWORLDSSTATE				140
enum SwitchingWorldsState
{
	eSwitchingWorldsStateNone,
	eSwitchingWorldsStatePlayerHookup,
	eSwitchingWorldsStateAutoSave,
	eSwitchingWorldsStateWaitForClient,
	eSwitchingWorldsStateFinished,
};

#define MID_CLIENT_READY_FOR_AUTOLOAD			141	// Client to Server- Lets server know the client is ready to begin an automatic load.

// Info on doomsday objects...
#define MID_DOOMSDAY_MESSAGE					142 // Server -> Client
	#define	MID_DOOMSDAY_PIECE_PICKEDUP			0
	#define MID_DOOMSDAY_PIECE_DROPPED			1
	#define MID_DOOMSDAY_PIECE_PLACED			2
	#define MID_DOOMSDAY_PIECE_STOLEN			3	
	#define MID_DOOMSDAY_DEVICE_COMPLETED		4
	#define MID_DOOMSDAY_PIECE_RESPAWNED		5

#define MID_PLAYER_TAUNT                        146
#define MID_MULTIPLAYER_UPDATE                  147
#define MID_MULTIPLAYER_OPTIONS                 148
#define	MID_TEAM_INFO							149
	#define MTEAM_FULL							0
	#define MTEAM_SCORE							1
#define	MID_TEAM_ADD							150

#define MID_MUSIC                               153

#define MID_START_ENERGY_TRANSFER				155	// Server to Client
#define MID_STOP_ENERGY_TRANSFER				156	// Client to Server
#define MID_ENERGY_TRANSFER_COMPLETE			157 // Client to Server

#define MID_QUERY_TARGET_PROPERTIES				158 // Client <-> Server
// Target Property ID's:
	#define TARGET_PROP_ENERGY_REQUIRED			1
	#define TARGET_PROP_HEALTH					2

#define MID_OBJECT_MESSAGE						159 // Client -> Server
	
#define MID_GAME_PAUSE                          160
#define MID_GAME_UNPAUSE                        161

#define MID_GEAR_PICKEDUP                       170
#define MID_INTEL_PICKEDUP                      171

#define MID_SUBROUTINE_OBTAINED					172
#define MID_ADDITIVE_OBTAINED					173
#define MID_PROCEDURAL_OBTAINED					174
#define MID_PRIMITIVE_OBTAINED					175

#define MID_SAVE_GAME                           181

#define MID_LOAD_GAME							182
enum LoadGameType
{
	kLoadGameTypeSlot,
	kLoadGameTypeQuick,
	kLoadGameTypeReload,
	kLoadGameTypeContinue,
};

// First message sent from client to server to start the game.
#define MID_START_GAME							183

// Sent when the game is over.
#define MID_END_GAME							184

// Tell server to load a level from scratch.
#define MID_START_LEVEL							185

#define MID_EXIT_LEVEL							186

#define MID_SAVE_DATA							187

#define MID_SERVER_ERROR                        190
#define MID_LOAD_FAILED							191
#define MID_OBJECTIVES_DATA						192
#define MID_SCMD								193

#define MID_MULTIPLAYER_DATA                    202
#define MID_MULTIPLAYER_SERVERDIR				203 // server directory message

#define MID_CLIENT_PLAYER_UPDATE				212	// Server to client
#define MID_FRAG_SELF							213 // Client to server
#define MID_PINGTIMES							214 // Server to client
#define MID_SHAKE_SCREEN						215 // Client to server
#define MID_CHANGE_WORLDPROPERTIES				216	// Server to client

#define MID_CLIENTFX_CREATE                     230 // Server to client
#define MID_SFX_MESSAGE                         231 // Server to client

#define MID_CONSOLE_TRIGGER                     235 // Client to server
#define MID_CONSOLE_COMMAND                     236 // Client to server
#define MID_CONSOLE_CLIENTFX					237 // Client to server

#define MID_DECISION							239	// Both ways
#define MID_DIFFICULTY							240 // Client to server
#define MID_GAMETEXMGR							241	// Server to client
#define MID_DISPLAY_METER						243	// Server to client						

#define MID_HANDSHAKE                           245 // Both ways, handshake negotiation
// Handshake sub-messages
    #define MID_HANDSHAKE_HELLO                 0   // Hello! (version verification)  Both ways
    #define MID_HANDSHAKE_PASSWORD              1   // Wassa password?  Server to client
    #define MID_HANDSHAKE_LETMEIN               2   // What, this password?  Client to server
    #define MID_HANDSHAKE_DONE                  3   // You'll do just fine.  Server to client
    #define MID_HANDSHAKE_WRONGPASS             4   // Wrong password.  Server to client
    // Handshake version state

#define MID_GADGETTARGET						246 // Both ways, handling of a gadget target
#define MID_CLEAR_PROGRESSIVE_DAMAGE			247 // Client to server
#define MID_SEARCH								248 // Both ways, handling of searching
#define MID_ADD_PUSHER							249 // Server to client

// AI message IDs 250 - 255
#define MID_STIMULUS                            250 // Client to server
#define MID_RENDER_STIMULUS                     251 // Client to server
#define MID_ADD_GOAL                            252 // Client to server
#define MID_REMOVE_GOAL                         253 // Client to server
#define MID_OBJECT_ALPHA                        254 // Client to server


// Object <-> Object messages...


// Interface Change ids shared between client and server...These are
// used with above MID_PLAYER_INFOCHANGE

#define IC_AMMO_ID					1
#define IC_HEALTH_ID				2
#define IC_ARMOR_ID					3
#define IC_DEFAULTWEAPON_ID			4
#define IC_WEAPON_PICKUP_ID			5
#define IC_AIRLEVEL_ID				6
#define IC_ROCKETLOCK_ID			7
#define IC_OUTOFAMMO_ID				8
#define IC_OBJECTIVE_ID				9
#define IC_MOD_PICKUP_ID			10
#define IC_FADE_SCREEN_ID			11
#define IC_RESET_INVENTORY_ID		12
#define IC_MISSION_TEXT_ID			13
#define IC_MISSION_FAILED_ID		14
#define IC_MAX_HEALTH_ID			15
#define IC_MAX_ARMOR_ID				16
#define IC_WEAPON_OBTAIN_ID			17
#define IC_OPTION_ID				18
#define IC_PARAMETER_ID				19
#define IC_KEY_ID					20
#define IC_SKILLS_ID				21
#define IC_REGION_CHANGE			22
#define IC_MAX_ENERGY_ID			23
#define IC_ENERGY_ID				24
#define IC_PSETS_ID					25
#define IC_PERFORMANCE_RATING_ID	26
#define IC_BUILD_POINTS_ID			27

//helper id's that tell what to do with objectives, parameters, keys, etc.
#define ITEM_ADD_ID			0
#define ITEM_REMOVE_ID      1
#define ITEM_CLEAR_ID       2
#define ITEM_COMPLETE_ID    3

//helper id's to differentiate between different types of search messages
#define SEARCH_START		0
#define SEARCH_FOUND		1


// Client to player messages...These are used with MID_PLAYER_CLIENTMSG
enum ClientPlayerMessage
{
	CP_MOTION_STATUS = 1,
	CP_DAMAGE,
	CP_WEAPON_STATUS,
	CP_FLASHLIGHT,
	CP_PLAYER_LEAN,
	CP_PHYSICSMODEL,
	CP_DAMAGE_VEHICLE_IMPACT,
	CP_DEFLECT,
};

// CP_MOTION_STATUS, Motion Status is used to notify the server if the motion of the player
// has changed (did he jump/land/fall)....
enum CPMotionStatusTypes
{
	MS_NONE,
	MS_JUMPED,
	MS_FALLING,
	MS_LANDED,
};

// CP_DAMAGE, Damage is used to inflict damage the player (e.g., fall damage)...

// CP_WEAPON_STATUS, Weapon Status is used to tell the server what animation the player-view
// weapon is playing...
enum CPWeaponStatusTyps
{
	WS_NONE,
	WS_RELOADING,
	WS_SELECT,
	WS_DESELECT,
};

// CP_FLASHLIGHT, Flashlight is used to simply send new info regarding the location of the 
// flashlight beam impact
enum CPFlashLightTypes
{
	FL_ON,
	FL_OFF,
};

// CP_PLAYER_LEAN, Lean is used to register a stimulus when the player is fully leaned out
// and remove the stimulu when they start to lean back in. 
enum CPPlayerLeanTypes
{
	PL_LEFT,
	PL_RIGHT,
	PL_CENTER,
};

// CP_PHYSICSMODEL, New model is defined by PlayerPhysicsModel...

// CP_DAMAGE_VEHICLE_IMPACT, Damage from vehicle impact

// Client update flags

#define CLIENTUPDATE_CAMERAOFFSET            (1<<0)
#define CLIENTUPDATE_CONTCODE                (1<<1)
#define CLIENTUPDATE_3RDPERSON               (1<<2)
#define CLIENTUPDATE_3RDPERVAL               (1<<3)
#define CLIENTUPDATE_ALLOWINPUT              (1<<4)
#define CLIENTUPDATE_PLAYERROT               (1<<5)
#define CLIENTUPDATE_ACCURATEPLAYERROT       (1<<6)
#define CLIENTUPDATE_FULLPLAYERROT			 (1<<7)


#endif // __MSG_IDS_H__
