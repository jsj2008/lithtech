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

enum SFXMsgId
{
	SFX_POLYGRID_ID,
	SFX_RENDERTARGET_ID,
	SFX_RENDERTARGETGROUP_ID,
	SFX_PARTICLETRAIL_ID,
	SFX_WEAPON_ID,
	SFX_VOLUMEBRUSH_ID,
	SFX_SHELLCASING_ID,
	SFX_CAMERA_ID,
	SFX_PROJECTILE_ID,
	SFX_MARK_ID,
	SFX_PICKUPITEM_ID,
	SFX_CHARACTER_ID,
	SFX_PLAYERSOUND_ID,
	SFX_SEARCHLIGHT_ID,
	SFX_STEAM_ID,
	SFX_EXPLOSION_ID,
	SFX_SOUND_ID,
	SFX_DEBUGLINE_ID,
	SFX_SNOW_ID,
	SFX_JUMPVOLUME_ID,
	SFX_PLAYERLURE_ID,
	SFX_DISPLAYTIMER_ID,
	SFX_DYNAMIC_SECTOR_ID,
	SFX_SCATTER_ID,
	SFX_TRIGGER_ID,
	SFX_AIMMAGNET_ID,
	SFX_NAVMARKER_ID,
	SFX_LADDER_ID,
	SFX_SPECIALMOVE_ID,
	SFX_FINISHINGMOVE_ID,
	SFX_FORENSICOBJECT_ID,
	SFX_TURRET_ID,
	SFX_SOUND_NONPOINT_ID,
	SFX_VOLUMETRICLIGHT_ID,
	SFX_ENTRYTOOLLOCK_ID,
	SFX_SCREENEFFECT_ID,
	SFX_ACTIVATEOBJECT_ID,
	SFX_CTFFLAGBASE_ID,
	SFX_CTFFLAG_ID,
	SFX_TEAMCLIENTFX_ID,
	SFX_PHYSICS_CONSTRAINT_ID,
	SFX_PHYSICS_COLLISION_SYSTEM_ID,
	SFX_CONTROLPOINT_ID,

	SFX_TOTAL_NUMBER	// Important that this is the last ID defined...
};


// Shared fx related stuff...


// CharacterFX messages...
enum CFXMsgId
{
	CFX_NODECONTROL_LIP_SYNC		= 2,
	CFX_CINEMATICAI_MSG,
	CFX_STEALTH_MSG,
	CFX_RESET_TRACKER,
	CFX_CLIENTID_MSG,
	CFX_CHAT_MSG,
	CFX_FLASHLIGHT_CREATE_MSG,
	CFX_FLASHLIGHT_DESTROY_MSG,
	CFX_DMGFLAGS_MSG,
	CFX_ALLFX_MSG,
	CFX_DIALOGUE_MSG,
	CFX_CROSSHAIR_MSG,
	CFX_BROADCAST_MSG,
	CFX_BODYSTATE_MSG,
	CFX_INFO_STRING,
	CFX_WEAPON_SOUND_MSG,
	CFX_WEAPON_SOUND_LOOP_MSG,
	CFX_TRACK_TARGET_MSG,
	CFX_TRACKER_TARGET_MSG,
	CFX_MELEE_CONTROLLER_MSG,
	CFX_HITBOX_MSG,
	CFX_PLAYER_RESPAWN,
	CFX_INSTANTDMGFLAGS_MSG,
	CFX_UPDATE_ATTACHMENTS,
	CFX_PLAYER_PHYSICS_MODEL,
	CFX_PLAYER_TIMER,
	CFX_FADE_MSG,
	CFX_DEAD,
	CFX_SEVER,
	CFX_BLOCKWINDOW_MSG,
	CFX_DODGEWINDOW_MSG,
	CFX_BERSERKERKICK_MSG,
	CFX_FINISHINGMOVE_MSG,
	CFX_KILLPROJECTILE_MSG,
	CFX_CREATE_LOOP_FX_MSG,
	CFX_KILL_LOOP_FX_MSG,
	CFX_SLIDE,
	CFX_SPECTATE,
	CFX_UPDATE_WEAPON_MODEL,
	CFX_UPDATE_WEAPON_TRANSFORM,
	CFX_RAGDOLL,
	CFX_UPDATE_FORENSIC_MASK,
	CFX_CHANGE_WEAPON,
	CFX_FX_MSG,
	CFX_USE_GEAR,
	CFX_SHOW_ATTACH_FX,
	CFX_HIDE_ATTACH_FX,
	CFX_MOTION_STATUS_FX,

	CFX_COUNT,
};

enum ForensicFxMsgId
{
	FORENSICFX_SEND_FORENSIC_DIST,
	FORENSICFX_FORENSIC_TOOL_SELECTED,
	FORENSICFX_FORENSIC_TOOL_ACTIVATE,
	FORENSICFX_REQUEST_FORENSIC_DIST,
};

// Melee Controller messages...
enum MeleeControllerMsgId
{
	MELEE_FORCE_BLOCKING_MSG	= 1,
};

// CameraFX messages...
enum CAMFXMsgId
{
	CAMFX_FOV		= 1,
};


// TriggerFX messages...

enum TRIGFXMsgId
{
	TRIGFX_ALLFX_MSG	= 1,
	TRIGFX_LOCKED_MSG,
};

// PickupItemFX messages...

enum PUFXMsgId
{
	PUFX_CLIENTFX	= 1,
	PUFX_TEAMID,
	PUFX_ENDFX,
	PUFX_DROPPEDCLIENTFX,
	PUFX_LOCKED,
	PUFX_RESPAWN,
	PUFX_PICKEDUP,
	PUFX_FIREDFROM,
	PUFX_RECOVERABLE
};

// SnowFX messages...

enum SVFXMsgId
{
	SVFX_TURNON		= 1,
};

// SoundNonPointFX messages...

enum SNPFXMsgId
{
	SNPFX_ALLFX_MSG	= 1,
	SNPFX_TOGGLE_MSG,
};

// ActivateFX messages...

enum ActivateFXMsgId
{
	ACTIVATEFX_STATE = 1,
	ACTIVATEFX_DISABLED,
	ACTIVATEFX_MAX,
};

// SpecialMoveFXMsgId...

enum SpecialMoveFXMsgId
{
	SPECIALMOVEFX_ACTIVATE = ACTIVATEFX_MAX,
	SPECIALMOVEFX_ON,
	SPECIALMOVEFX_OFF,
	SPECIALMOVEFX_ACTIVATED,
	SPECIALMOVEFX_RELEASED,
	SPECIALMOVEFX_LOOKEDAT,
	SPECIALMOVEFX_DESTROY,
};

//
// BodyState values..
//
enum BodyState 
{
	eBodyStateInvalid		= -1,
#define BODYSTATE_AS_ENUM 1
#include "BodyStateEnums.h"
#undef BODYSTATE_AS_ENUM
	eBodyStateCount,
}; 

// Helper function for converting from a body state name to an enum.
BodyState StringToBodyState( const char* pszBodyState );

enum PlayerLureId
{
	PlayerLureId_Invalid,
};

enum PlayerLureCameraFreedom
{
	kPlayerLureCameraFreedomNone,
	kPlayerLureCameraFreedomLimited,
	kPlayerLureCameraFreedomUnlimited
};

enum PickupItemType
{
	kPickupItemType_Unknown,
	kPickupItemType_Weapon,
	kPickupItemType_AmmoBox,
	kPickupItemType_Mod,
	kPickupItemType_Gear,

	kPickupItemType_NumPickupTypes,
};

enum TurretFXMsgId
{
	kTurretFXMsg_All,
	kTurretFXMsg_Deactivate,
	kTurretFXMsg_RemoteActivate,
	kTurretFXMsg_SwitchToTurret,
	kTurretFXMsg_Damage,
};

enum CTFFlagBaseState
{
	kCTFFlagBaseState_HasFlag,
	kCTFFlagBaseState_NoFlag,

	kCTFFlagBaseState_NumStates,
};

enum CTFFlagState
{
	kCTFFlagState_InBase,
	kCTFFlagState_Carried,
	kCTFFlagState_Loose,

	kCTFFlagState_NumStates,
};

enum CTFFlagFXMsgId
{
	kCTFFlagFXMsg_Steal,
	kCTFFlagFXMsg_PickedUp,
	kCTFFlagFXMsg_Return,
	kCTFFlagFXMsg_AutoReturn,
	kCTFFlagFXMsg_Dropped,
	kCTFFlagFXMsg_Capture,
	kCTFFlagFXMsg_FlagDefend,
	kCTFFlagFXMsg_FlagCarrierDefend,
	kCTFFlagFXMsg_FlagCarrierKill,
	kCTFFlagFXMsg_CaptureAssist,

	kCTFFlagFXMsg_NumIds,
};

enum DisplayTimerMsgId
{
	kDisplayTimerMsgId_Main,
	kDisplayTimerMsgId_Team0,
	kDisplayTimerMsgId_Team1,
	kDisplayTimerMsgId_SuddenDeath,
};

enum EConstraintType
{
	kConstraintType_Hinge,
	kConstraintType_LimitedHinge,
	kConstraintType_Point,
	kConstraintType_Prismatic,
	kConstraintType_Ragdoll,
	kConstraintType_StiffSpring,
	kConstraintType_Wheel,

	kConstraintType_NumTypes,
};

enum EVolumeBrushMsgId
{
	kVolumeBrush_AllowSwimming,

	kVolumeBrush_NumMsgs
};

enum EControlPointConstants
{
	// Maximum number of control points allowed in a level.
	MAX_CONTROLPOINT_OBJECTS = 9,
	// Invalid CP id
	CONTROLPOINT_INVALID_ID = 0,
};

enum EControlPointState
{
	kControlPointState_Team0,
	kControlPointState_Team1,
	kControlPointState_Neutral,

	kControlPointState_NumStates
};

enum EControlPointFXMsgId
{
	kControlPointFXMsg_ControlLevel,
	kControlPointFXMsg_PlayerCapture,
	kControlPointFXMsg_PlayerFixup,
	kControlPointFXMsg_PlayerNeutralize,
	kControlPointFXMsg_PlayerControlPointDefend,

	kControlPointFXMsg_NumIds,
};



#endif // __SFX_MSG_IDS_H__
