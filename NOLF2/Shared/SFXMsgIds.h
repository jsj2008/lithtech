// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMsgIDs.h
//
// PURPOSE : Special FX message ids
//
// CREATED : 10/28/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MSG_IDS_H__
#define __SFX_MSG_IDS_H__

// SpecialFX types...
//
// The first BYTE in the message associated with the ILTMessage_Read* parameter 
// passed to CSFXMgr::HandleSFXMsg() must be one of the following values:

// NOTE:  It is important that these remain in this order, SFXMgr.cpp assumes
// this order for making optimizations...New FX should be added at the end
// and SFX_TOTAL_NUMBER should be adjusted to account for them (also, the
// s_nDynArrayMaxNums[] should be updated in SFXMgr.cpp).


#define SFX_POLYGRID_ID				1
#define SFX_PARTICLETRAIL_ID		2
#define	SFX_PARTICLESYSTEM_ID		3
#define SFX_PARTICLESHOWER_ID		4
#define SFX_TRACER_ID				5
#define SFX_WEAPON_ID				6
#define SFX_DYNAMICLIGHT_ID			7
#define SFX_PARTICLETRAILSEG_ID		8
#define SFX_SMOKE_ID				9
#define SFX_BULLETTRAIL_ID			10
#define SFX_VOLUMEBRUSH_ID			11
#define SFX_SHELLCASING_ID			12
#define SFX_CAMERA_ID				13
#define SFX_PARTICLEEXPLOSION_ID	14
#define SFX_SCALE_ID				15 // Sprites/Models	
#define SFX_DEBRIS_ID				16
#define SFX_DEATH_ID				17
#define SFX_GIB_ID					18
#define SFX_PROJECTILE_ID			19
#define SFX_MARK_ID					20
#define SFX_LIGHT_ID				21
#define SFX_RANDOMSPARKS_ID			22
#define SFX_PICKUPITEM_ID			23
#define SFX_CHARACTER_ID			24
#define SFX_PLAYERSOUND_ID			25
#define SFX_NODELINES_ID			26
#define SFX_WEATHER_ID				27
#define SFX_LIGHTNING_ID			28
#define SFX_SPRINKLES_ID			29
#define	SFX_FIRE_ID					30
#define SFX_LENSFLARE_ID			31
#define SFX_MUZZLEFLASH_ID			32
#define SFX_SEARCHLIGHT_ID			33
#define SFX_POLYDEBRIS_ID			34
#define SFX_STEAM_ID				35
#define SFX_EXPLOSION_ID			36
#define SFX_POLYLINE_ID				37
#define SFX_BODY_ID					38
#define SFX_LASERTRIGGER_ID			39
#define SFX_MINE_ID					40
#define SFX_BEAM_ID					41
#define SFX_PLAYERVEHICLE_ID		42
#define SFX_SOUND_ID				43
#define SFX_OBJSPRITE_ID			44
#define SFX_LIGHTGROUP_ID			45
#define SFX_DEBUGLINE_ID			46
#define SFX_TEXTUREFX_ID			47
#define SFX_SNOW_ID					48
#define SFX_JUMPVOLUME_ID			49
#define SFX_PLAYERLURE_ID			50
#define SFX_GADGETTARGET_ID			51	
#define SFX_DISPLAYTIMER_ID			52
#define SFX_DYNAMIC_OCCLUDER_ID		53
#define SFX_SCATTER_ID				54
#define SFX_TRIGGER_ID				55
#define SFX_RADAROBJECT_ID			56
#define SFX_ACTIVATEOBJECT_ID		57
#define SFX_DOOMSDAYPIECE_ID		58
#define SFX_TOTAL_NUMBER			58	// Important that this is the same as the last FX defined...

// Shared fx related stuff...


// CharacterFX messages...

#define CFX_NODECONTROL_LIP_SYNC		2
#define CFX_NODECONTROL_HEAD_FOLLOW_OBJ	3
#define CFX_CINEMATICAI_MSG				4
#define CFX_NODECONTROL_SCRIPT			5
#define CFX_STEALTH_MSG					6
#define CFX_RESET_TRACKER				7
#define CFX_CLIENTID_MSG				8
#define CFX_CHAT_MSG					9
#define CFX_FLASHLIGHT_CREATE_MSG		10
#define CFX_FLASHLIGHT_DESTROY_MSG		11
#define CFX_DMGFLAGS_MSG				12
#define CFX_CIGARETTE_CREATE_MSG		13
#define CFX_CIGARETTE_DESTROY_MSG		14
#define CFX_CIGARETTESMOKE_CREATE_MSG	15
#define CFX_CIGARETTESMOKE_DESTROY_MSG	16
#define CFX_ALLFX_MSG					17
#define CFX_DIALOGUE_MSG				18
#define CFX_CROSSHAIR_MSG				19
#define CFX_ZZZ_CREATE_MSG				20
#define CFX_ZZZ_DESTROY_MSG				21
#define CFX_TAUNT_MSG					22
#define CFX_INFO_STRING					23
#define CFX_WEAPON_SOUND_LOOP_MSG		24
#define CFX_TRACK_TARGET_MSG			25
#define CFX_PITCH						26
#define CFX_HITBOX_MSG					27
#define CFX_CAN_CARRY					28
#define	CFX_PLAYER_DEAD					29
#define CFX_PLAYER_REVIVED				30
#define CFX_PLAYER_RESPAWN				31
#define	CFX_INSTANTDMGFLAGS_MSG			32
#define CFX_CHARACTER_TRACKING			33
#define CFX_UPDATE_ATTACHMENTS			34
#define CFX_CAN_WAKE					35
#define CFX_CHARACTER_RADAR				36
#define CFX_CARRY						37
//sub-messages for CFX_CARRY
	#define	CFX_CARRY_NONE					0
	#define	CFX_CARRY_BODY					1
	#define	CFX_CARRY_DD_TRAN				2					
	#define	CFX_CARRY_DD_BATT				3
	#define	CFX_CARRY_DD_CORE				4					


// CameraFX messages...

#define CAMFX_FOV						1

// BodyFX messages...

#define BFX_TWITCH_MSG					1
#define BFX_FADE_MSG					2
#define BFX_HITBOX_MSG					3
#define BFX_CAN_CARRY					4
#define BFX_CARRIED						5
#define BFX_DAMAGEFX_MSG				6
#define BFX_PERMANENT					7
#define BFX_CANREVIVE					8

// TriggerFX messages...

#define	TRIGFX_ALLFX_MSG				1
#define TRIGFX_LOCKED_MSG				2

// ActivateFX messages...

#define ACTIVATEFX_DISABLED				1
#define	ACTIVATEFX_STATE				2

// PickupItemFX messages...

#define PUFX_CLIENTFX					1
#define PUFX_TEAMID						2

// BodyState values..

enum BodyState
{
	eBodyStateNormal		= 0,	// Normal death (shot, etc)
	eBodyStateChair			= 1,	// Killed in a chair
	eBodyStateLedge			= 2,	// Falling off a ledge
	eBodyStateLaser			= 3,	// Vap-o-rized by a lazer
	eBodyStateExplode		= 4,	// Hit by an explosion
	eBodyStateStuck			= 5,	// Stuck to a wall w/ xbow/spear
	eBodyStateUnderwater	= 6,	// Killed underwater
	eBodyStateStairs		= 7,	// Fall down stairs (and break hip)
	eBodyStateFly			= 8,	// Flying through the air
	eBodyStateDecay			= 9,	// Decay powder dumped on body
	eBodyStateCrush			= 10,	// Crushed
	eBodyStatePoison		= 11,	// Killed by poison
	eBodyStateAcid			= 12,	// Killed by acid
	eBodyStateArrow			= 13,	// Killed by arrow (wall stick)
	eBodyStateFade			= 14,	// Fade for low-violence setting
	eBodyStateLadder		= 15,	// Falling off a ladder
};

// PolyLine values...

enum PolyLineWidthStyle 
{
	PLWS_BIG_TO_SMALL=0,
	PLWS_SMALL_TO_BIG,
	PLWS_SMALL_TO_SMALL,
	PLWS_CONSTANT
};

// Used with SFX_PLAYERLURE_ID
enum PlayerLureFlags
{
	ePlayerLureFlagsAllowWeapon		= (1<<0),
	ePlayerLureFlagsRetainOffsets	= (1<<1),
	ePlayerLureFlagsBicycle			= (1<<2),
};
enum PlayerLureCameraFreedom
{
	kPlayerLureCameraFreedomNone,
	kPlayerLureCameraFreedomLimited,
	kPlayerLureCameraFreedomUnlimited
};

#endif // __SFX_MSG_IDS_H__
