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

#ifndef __MSGIDS_H__
#define __MSGIDS_H__

#include "SFXMsgIds.h"              // Special fx message ids

// General Client <-> Server messages

#define MID_PLAYER_UPDATE				100

#define MID_PLAYER_SUMMARY				102 
#define MID_PLAYER_INFOCHANGE			103 // Both ways
#define MID_PLAYER_SCORE				104
#define	MID_PLAYER_STATE_CHANGE			105
#define MID_PLAYER_MESSAGE				106
#define MID_PLAYER_CHEAT				107
#define MID_PLAYER_LADDER				108
#define MID_PLAYER_INFO					109
	//player info sub-messages
	#define	MID_PI_JOIN					0	// this player is joining
	#define	MID_PI_EXIST				1	// this is info about an existing player sent to someone joining
	#define MID_PI_UPDATE				2	// this is an update of existing players

#define MID_PLAYER_REMOVED				110
#define MID_PLAYER_SCORED				111
#define MID_PLAYER_RESPAWN				112

#define MID_PLAYER_SPECTATORMODE		113

#define MID_PLAYER_GOTO_NODE			114
#define MID_PLAYER_ARRIVED_AT_NODE		115 

#define MID_PLAYER_LOADCLIENT			116
#define MID_PLAYER_DAMAGE				117
#define MID_PLAYER_ACTIVATE				118
//Activation sub-messages
	#define MID_ACTIVATE_NORMAL			0
	#define MID_ACTIVATE_WAKEUP			1
	#define MID_ACTIVATE_LADDER			2
	#define MID_ACTIVATE_SPECIALMOVE	3
	#define MID_ACTIVATE_TURRET			4
	#define MID_ACTIVATE_TAKE			5
	#define MID_ACTIVATE_SWAP			6
	#define MID_ACTIVATE_OPEN			7
	#define MID_ACTIVATE_CLOSE			8
	#define MID_ACTIVATE_READ			9
	#define MID_ACTIVATE_MOVE			10

	// Entry tools.
	#define MID_ACTIVATE_ENTRY_MIN		11
	#define MID_ACTIVATE_CHOP			12
	#define MID_ACTIVATE_PRY			13
	#define MID_ACTIVATE_BREAK			14
	#define MID_ACTIVATE_EXTINGUISH		15
	#define MID_ACTIVATE_DIG			16
	#define MID_ACTIVATE_ACTIVATE		17
	#define MID_ACTIVATE_DEACTIVATE		18
	#define MID_ACTIVATE_ENTRY_MAX		19

#define IS_ACTIVATE_ENTRY(flag) \
	(((flag) > MID_ACTIVATE_ENTRY_MIN) && ((flag) < MID_ACTIVATE_ENTRY_MAX))

	// Forensics.
	#define MID_ACTIVATE_FORENSIC_MIN	20
	#define MID_ACTIVATE_SCAN			21
	#define MID_ACTIVATE_SAMPLE			22
	#define MID_ACTIVATE_TEST			23
	#define MID_ACTIVATE_PHOTOGRAPH		24
	#define MID_ACTIVATE_FORENSIC_MAX	29

#define IS_ACTIVATE_FORENSIC(flag) \
	(((flag) > MID_ACTIVATE_FORENSIC_MIN) && ((flag) < MID_ACTIVATE_FORENSIC_MAX))

	#define MID_ACTIVATE_SURFACESND		30

#define MID_PLAYER_CLIENTMSG			119	// Client to server
#define MID_PLAYER_LEASH				120 // Server to client
#define MID_PLAYER_TELEPORT				121 // Client to server
#define MID_PLAYER_ANIMTRACKERS			122 // Client to server
	// Animation trackers sub-messages
	#define	MID_ANIMTRACKERS_REMOVE		0
	#define MID_ANIMTRACKERS_ADD		1

#define MID_PLAYER_EVENT				123
enum PlayerEventMsgType
{
	kPETransmission,
	kPESignal,
	kPEFlashlight,
	kPEOverlay,
	kPECrosshair,
	kPEHeartBeat,
	kPEBreathing,
	kPECarry,
	kPEBlocked,
	kPEEndRoundCondition,
	kPEStoryMode,
	kPEHealthTestMode,
	kPEWeaponEffect,
	kPESyncAction,
	kPEObjective,
	kPEMission,
	kPEReward,
	kPENextSpawnPoint,
	kPEPrevSpawnPoint,
	kPEDropWeapon,
	kPEAutobalance,
};


#define MID_PLAYER_CHATMODE				124	// Client to server
#define MID_PLAYER_GHOSTMESSAGE			125
#define MID_PLAYER_TEXT					126
#define MID_PLAYER_TEAM					127
#define MID_DROP_GRENADE				128
#define MID_WEAPON_CHANGE				129	  // Both ways
#define MID_WEAPON_LAST					130   // Server to client
#define MID_WEAPON_SOUND				131
#define MID_WEAPON_SOUND_LOOP			132
#define MID_WEAPON_FIRE					133
#define MID_WEAPON_RELOAD				134
#define MID_WEAPON_SWAP					135
#define MID_WEAPON_FIRE_FX				136
#define MID_WEAPON_DAMAGE_PLAYER		137 // Server to client
#define MID_DYNAMIC_SECTOR				138
#define	MID_WEAPON_PRIORITY				139


#define MID_SWITCHINGWORLDSSTATE		140
enum SwitchingWorldsState
{
	eSwitchingWorldsStateNone,
	eSwitchingWorldsStatePlayerHookup,
	eSwitchingWorldsStateWaitForClient,
	eSwitchingWorldsStateReloadSave,
	eSwitchingWorldsStateFinished,
};

#define MID_CLIENT_READY_FOR_AUTOLOAD			141	// Client to Server- Lets server know the client is ready to begin an automatic load.

#define MID_SIMULATION_TIMER					142 // Simulation timer scale change.

#define MID_PLAYER_BODY							143 // Message to the player body...
enum PlayerBodyMessage
{
	kPlayerBodyAnimate,
	kPlayerBodyAnimateStop,
	kPlayerBodyAnimateLoop,
	kPlayerBodyAnimateLinger,
	kPlayerBodyAnimateCamRot,
	kPlayerBodyWeightSet,
};

#define MID_SONIC								144
enum SonicMessage
{
	kSonicUpdateBreath,
	kSonicUpdateSkills,
	kSonicIntone,
};

#define MID_PLAYER_INTERFACE			145


#define MID_PLAYER_BROADCAST						146
#define MID_MULTIPLAYER_UPDATE					147
#define MID_MULTIPLAYER_OPTIONS					148
#define	MID_TEAM_INFO							149
	#define MTEAM_FULL							0
	#define MTEAM_SCORE							1
#define	MID_TEAM_ADD							150
#define MID_WEAPON_FINISH						151
#define MID_WEAPON_FINISH_RAGDOLL				152

#define MID_MIXER								154	// i'll assume 153 - 157 are for sound..
#define MID_SOUND_FILTER						155
#define MID_SOUND_BROADCAST_DB					156
#define MID_SOUND_MISC							157

// MID_SOUND_MISC IDs
	#define MID_SOUND_MISC_KILL_EARRING_EFFECT		1


#define MID_QUERY_TARGET_PROPERTIES				158 // Client <-> Server
// Target Property ID's:
	#define TARGET_PROP_ENERGY_REQUIRED			1
	#define TARGET_PROP_HEALTH					2

#define MID_OBJECT_MESSAGE						159 // Client -> Server
	
#define MID_GAME_PAUSE							160
#define MID_GAME_UNPAUSE						161
#define MID_DYNANIMPROP							162 // both ways
#define MID_GORE_SETTING						163 // Client -> Server
#define MID_PERFORMANCE_SETTING					164 // Client -> Server
enum PerformanceSettingMsg
{
	kPerfSettingMsg_WorldDetail,

	kPerfSettingMsg_Count,
};

#define MID_MAPLIST								165 // Server -> Client

#define MID_PLAYER_GEAR							170 // Client <-> Server
enum GearMsgType
{
	kGearAdd,
	kGearRemove,
	kGearUse
};

#define MID_PICKUPITEM_ACTIVATE							171 // Client -> Server
#define MID_PICKUPITEM_ACTIVATE_EX						172 // Client -> Server

#define MID_WEAPON_BREAK_WARN					173	// Server -> Client

#define MID_SAVE_GAME							181

#define MID_LOAD_GAME							182
enum LoadGameType
{
	kLoadGameTypeSlot,
	kLoadGameTypeQuick,
	kLoadGameTypeReload,
	kLoadGameTypeCheckpointSave,
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

#define MID_SCMD_COMMAND						188
#define MID_SCMD_RESPONSE						189

#define MID_LOAD_FAILED							190
#define MID_VOTE								191
#define MID_SLOWMO								193
enum SlowMoMsgType
{
	kSlowMoInit,
	kSlowMoStart,
	kSlowMoEnd
};

#define MID_PHYSICSCOLLISION					194

#define MID_SERVERGAMESTATE						195

#define MID_MELEEATTACK							196
#define MID_MELEEBLOCK							197
#define MID_INSTANTNAVMARKER					198

#define MID_PUNKBUSTER_MSG						201
#define MID_MULTIPLAYER_DATA					202

#define MID_CLIENT_PLAYER_UPDATE				212	// Server to client
#define MID_FRAG_SELF							213 // Client to server
#define MID_PINGTIMES							214 // Server to client
#define MID_CHANGE_WORLDPROPERTIES				216	// Server to client
#define MID_DO_DAMAGE							217 // Client to server

#define MID_SHATTER_WORLD_MODEL					229	// Server to client when a world model needs to be shattered
#define MID_CLIENTFX_CREATE						230 // Server to client
#define MID_SFX_MESSAGE							231 // Server to client
#define MID_SFX_MESSAGE_OVERRIDE				232 // Server to client
#define MID_APPLY_DECAL							233 // Server to client

#define MID_CONSOLE_TRIGGER						235 // Client to server
#define MID_CONSOLE_COMMAND						236 // Client to server

#define MID_DECISION							239	// Both ways
#define MID_DIFFICULTY							240 // Client to server
#define MID_GAMETEXMGR							241	// Server to client
#define MID_DISPLAY_METER						243	// Server to client						

#define MID_CLIENTCONNECTION					245 // Both ways, client connection message

#define MID_CLEAR_PROGRESSIVE_DAMAGE			247 // Client to server
#define MID_ADD_PUSHER							248 // Server to client

// AI message IDs 249 - 255
#define MID_AIDBUG								249 // Client to server					
#define MID_STIMULUS							250 // Client to server
#define MID_RENDER_STIMULUS						251 // Client to server
#define MID_ADD_GOAL							252 // Client to server
#define MID_REMOVE_GOAL							253 // Client to server
#define MID_OBJECT_ALPHA						254 // Client to server


// Object <-> Object messages...


// Interface Change ids shared between client and server...These are
// used with above MID_PLAYER_INFOCHANGE

enum IC_Ids
{
	IC_AMMO_ID,
	IC_HEALTH_ID,
	IC_ARMOR_ID,
	IC_DEFAULTWEAPON_ID,
	IC_WEAPON_PICKUP_ID,
	IC_AIRLEVEL_ID,
	IC_OUTOFAMMO_ID,
	IC_MOD_PICKUP_ID,
	IC_FADE_SCREEN_ID,
	IC_RESET_INVENTORY_ID,
	IC_MISSION_FAILED_ID,
	IC_MAX_HEALTH_ID,
	IC_MAX_ARMOR_ID,
	IC_WEAPON_OBTAIN_ID,
	IC_OPTION_ID,
	IC_PARAMETER_ID,
	IC_WEAPONCAP_ID,
	IC_WEAPON_REMOVE_ID,

	IC_NumIds
};



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
	CP_LADDER_SLIDE,
	CP_STORY_CANCEL
};

// CP_MOTION_STATUS, Motion Status is used to notify the server if the motion of the player
// has changed (did he jump/land/fall)....
enum CPMotionStatusTypes
{
	MS_NONE,
	MS_JUMPED,
	MS_FALLING,
	MS_LANDED,

	MS_COUNT,
};

// CP_DAMAGE, Damage is used to inflict damage the player (e.g., fall damage)...

// CP_WEAPON_STATUS, Weapon Status is used to tell the server what animation the player-view
// weapon is playing...
enum CPWeaponStatusTyps
{
	WS_NONE,
	WS_RELOADING,
	WS_BLOCKING,
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
// and remove the stimuli when they start to lean back in. 
enum CPPlayerLeanTypes
{
	PL_LEFT,
	PL_RIGHT,
	PL_CENTER,
};

// CP_PHYSICSMODEL, New model is defined by PlayerPhysicsModel...

// CP_DAMAGE_VEHICLE_IMPACT, Damage from vehicle impact

// Client update flags

#define CLIENTUPDATE_3RDPERSON			(1<<0)
#define CLIENTUPDATE_3RDPERVAL			(1<<1)
#define CLIENTUPDATE_ALLOWINPUT			(1<<2)
#define CLIENTUPDATE_CAMERAINFO			(1<<3)
#define CLIENTUPDATE_ANIMATION			(1<<4)

#endif // __MSGIDS_H__
