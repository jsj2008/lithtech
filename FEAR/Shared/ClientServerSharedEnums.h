// ----------------------------------------------------------------------- //
//
// MODULE  : ClientServerSharedEnums.h
//
// PURPOSE : Enums shared between the client and the server
//
// CREATED : 06/08/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SERVER_SHARED_ENUMS_H__
#define __CLIENT_SERVER_SHARED_ENUMS_H__

// Client connection states.
enum EClientConnectionState
{
	eClientConnectionState_None,
	eClientConnectionState_Tickle,
	eClientConnectionState_Error,
	eClientConnectionState_Hello,
	eClientConnectionState_KeyChallenge,
	eClientConnectionState_WaitingForAuth,
	eClientConnectionState_Overrides,
	eClientConnectionState_ContentTransfer,
	eClientConnectionState_CRCCheck,
	eClientConnectionState_LoggedIn,
	eClientConnectionState_Loading,
	eClientConnectionState_Loaded,
	eClientConnectionState_InWorld,
	eClientConnectionState_PostLoadWorld,
};

// ClientConnection error messages.
enum EClientConnectionError
{
	eClientConnectionError_WrongPassword,
	eClientConnectionError_BadCDKey,
	eClientConnectionError_ContentTransferFailed,
	eClientConnectionError_InvalidAssets,
	eClientConnectionError_Banned,
	eClientConnectionError_TimeOut,
	eClientConnectionError_PunkBuster
};

// Sonic types
enum SonicType
{
	eSonicType_None,
	eSonicType_Blast,
	eSonicType_Guide,
	eSonicType_Alter,
	eSonicType_Disabled,
};

// The game state the server is in.
enum EServerGameState
{
	EServerGameState_None,
	EServerGameState_Loading,
	EServerGameState_GracePeriod,
	EServerGameState_Playing,
	EServerGameState_PlayingSuddenDeath,
	EServerGameState_EndingRound,
	EServerGameState_ShowScore,
	EServerGameState_ExitingLevel,
};

// Player states

enum PlayerState 
{ 
	ePlayerState_None,
	ePlayerState_Alive, 
	ePlayerState_Dead, 
	ePlayerState_Dying_Stage1, 
	ePlayerState_Dying_Stage2, 
	ePlayerState_Spectator, 
};

// When playerstate is in spectator mode, it
// will be in one of these states.
enum SpectatorMode
{
	eSpectatorMode_None,
	eSpectatorMode_Fixed,
	eSpectatorMode_Tracking,
	eSpectatorMode_Clip,
	eSpectatorMode_Fly,
	eSpectatorMode_Follow,

	eSpectatorMode_Count
};

enum PlayerSoundId
{
	PSI_INVALID=0,
	PSI_RELOAD=1,
	PSI_RELOAD2,
	PSI_RELOAD3,
	PSI_SELECT,
	PSI_DESELECT,
	PSI_FIRE,
	PSI_DRY_FIRE,
	PSI_ALT_FIRE,
	PSI_SILENCED_FIRE,
	PSI_WEAPON_MISC1,
	PSI_WEAPON_MISC2,
	PSI_WEAPON_MISC3,
	PSI_WEAPON_MISC4,
	PSI_WEAPON_MISC5,
	PSI_JUMP,
	PSI_LAND,
	PSI_FIRE_LOOP,
	PSI_FIRE_LOOP_END,
};

enum CharacterSoundType { CST_NONE=0, CST_DAMAGE, CST_DEATH, CST_DIALOG, CST_EXCLAMATION, CST_AI_SOUND, CST_AI_SOUND_MODELKEY };

inline bool IsWeaponSound(PlayerSoundId ePSI)
{
	switch (ePSI)
	{
	case PSI_RELOAD :
	case PSI_RELOAD2 :
	case PSI_RELOAD3 :
	case PSI_SELECT :
	case PSI_DESELECT :
	case PSI_FIRE :
	case PSI_DRY_FIRE :
	case PSI_ALT_FIRE :
	case PSI_SILENCED_FIRE :
	case PSI_WEAPON_MISC1 :
	case PSI_WEAPON_MISC2 :
	case PSI_WEAPON_MISC3 :
	case PSI_WEAPON_MISC4 :
	case PSI_WEAPON_MISC5 :
	case PSI_FIRE_LOOP:
	case PSI_FIRE_LOOP_END:
		return true;
		break;

	default : break;
	}

	return false;
}

// Single player game difficulty

enum GameDifficulty
{
	GD_EASY=0,
	GD_NORMAL,
	GD_HARD,
	GD_VERYHARD,

	kNumDifficultyLevels
};

enum HitLocation
{
	HL_UNKNOWN	= 0,
	HL_HEAD,
	HL_TORSO,
	HL_ARM_LEFT,
	HL_ARM_RIGHT,
	HL_LEG_LEFT,
	HL_LEG_RIGHT,
	HL_NUM_LOCS,
};

// State of the save data when calling AskClientsForSaveData.
enum SaveDataState
{
	eSaveDataStateNone,
	eSaveDataStateSwitchLevels,
	eSaveDataStateTransitionLevels,
	eSaveDataSaveGameSlot,
	eSaveDataQuickSave,
	eSaveDataReloadSave,
	eSaveDataCheckpointSave,
};

#endif // __CLIENT_SERVER_SHARED_ENUMS_H__
