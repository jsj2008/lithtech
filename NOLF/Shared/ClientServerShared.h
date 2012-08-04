// ----------------------------------------------------------------------- //
//
// MODULE  : ClientServerShared.h
//
// PURPOSE : Types and enums shared between the client and the server
//
// CREATED : 11/25/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SERVER_SHARED_H__
#define __CLIENT_SERVER_SHARED_H__

// Shared client/server enumerations...

// Player states

enum PlayerState { PS_UNKNOWN=0, PS_ALIVE, PS_DEAD, PS_DYING, PS_GHOST };

// Level ending flag for multiplayer
enum LevelEnd
{
	LE_UNKNOWN = 0,
	LE_TEAM1_WIN,
	LE_TEAM2_WIN,
	LE_DRAW,
	LE_TIMELIMIT,
	LE_FRAGLIMIT
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
	PSI_JUMP,
	PSI_LAND
};

inline LTBOOL IsWeaponSound(PlayerSoundId ePSI)
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
			return LTTRUE;
		break;

		default : break;
	}

	return LTFALSE;
}

// Single player game difficulty

enum GameDifficulty
{
	GD_EASY=0,
	GD_NORMAL,
	GD_HARD,
	GD_VERYHARD
};



// Light waveform types

enum WaveTypes
{
	WAVE_NONE,
	WAVE_SQUARE,
	WAVE_SAW,
	WAVE_RAMPUP,
	WAVE_RAMPDOWN,
	WAVE_SINE,
	WAVE_FLICKER1,
	WAVE_FLICKER2,
	WAVE_FLICKER3,
	WAVE_FLICKER4,
	WAVE_STROBE,
	WAVE_SEARCH
};


enum HitLocation
{
	HL_UNKNOWN	= 0,
	HL_HEAD,
	HL_TORSO,
	HL_ARM,
	HL_LEG,
	HL_NUM_LOCS,
};

// Strafe flags...

#define SF_LEFT		1
#define SF_RIGHT	2
#define SF_FORWARD	4
#define SF_BACKWARD	8


// Object user flags -
// NOTE: The top byte is reserved for surface flags

#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_NIGHT_INFRARED			(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

// Used with player attachments (weapon / flash).  XXX_HIDE1 indicates that this
// object is hidden on the client when in first person mode.  XXX_HIDE1SHOW3
// indicates that this object is hidden on the client in first person mode,
// and shown on the client in 3rd person mode....

#define USRFLG_ATTACH_HIDE1				(1<<3)
#define USRFLG_ATTACH_HIDE1SHOW3		(1<<4)

// Used with Camera objects.  This specifies if a specific camera is "live"

#define USRFLG_CAMERA_LIVE				(1<<5)

// Can this object move...

#define USRFLG_MOVEABLE					(1<<5)

#define USRFLG_MODELADD					(1<<6)
#define USRFLG_GLOW						(1<<7)

// Used with world models only (server-side only).  This determines if
// the model can crush other objects...

#define USRFLG_CANT_CRUSH				(1<<8)

// Specifies that this object can be activated...

#define USRFLG_CAN_ACTIVATE				(1<<21)

// Used with the player object only (checked only on the client, and only
// on the player model)...

#define USRFLG_PLAYER_UNDERWATER		(1<<8)
#define USRFLG_PLAYER_DUCK				(1<<9)
#define USRFLG_PLAYER_MOTORCYCLE		(1<<10)
#define USRFLG_PLAYER_SNOWMOBILE		(1<<11)

// Used with character objects...(The USRFLG_CHAR_XXX can overlap the
// USRFLG_GADGET_XXX flags)

#define USRFLG_CHARACTER				(1<<12)

#define USRFLG_CHAR_LASER				(1<<13)
#define USRFLG_CHAR_BLEEDING			(1<<14)
#define USRFLG_CHAR_POISONED			(1<<15)
#define USRFLG_CHAR_STUNNED				(1<<16)
#define USRFLG_CHAR_SLEEPING			(1<<17)
#define USRFLG_CHAR_CHOKE				(1<<18)
#define USRFLG_CHAR_BURN				(1<<19)
#define USRFLG_CHAR_ELECTROCUTE			(1<<20)


// Used with non-character (i.e., ~USRFLG_CHARACTER) objects that can
// interact with gadgets...

#define USRFLG_GADGET_CAMERA_DISABLER	(1<<13)
#define USRFLG_GADGET_CODE_DECIPHERER	(1<<14)
#define USRFLG_GADGET_LOCK_PICK			(1<<15)
#define USRFLG_GADGET_WELDER			(1<<16)
#define USRFLG_GADGET_INTELLIGENCE		(1<<17)
#define USRFLG_GADGET_ZIPCORD			(1<<18)
#define USRFLG_GADGET_LIGHTER			(1<<19)

#define USRFLG_GADGET_UNUSED2			(1<<20)

// Used with AI objects.

#define USRFLG_AI_CLIENT_SOLID			(1<<3)
#define USRFLG_AI_UNUSED				(1<<4)
#define USRFLG_AI_FLASHLIGHT			(1<<22)

// Used when a GameBase object is active on the server

#define USRFLG_GAMEBASE_ACTIVE			(1<<23)

// ALL USRFLG's available!!!!


// Camera related flags...

#define CT_FULLSCREEN				0
#define CT_CINEMATIC				1


// Load/Save game defines...

#define LOAD_NEW_GAME				1	// Start from scratch - no saving or restoring
#define LOAD_NEW_LEVEL				2	// Save keep alives and level objects
#define LOAD_RESTORE_GAME			3	// No saving, but restore saved object

#define SERROR_LOADGAME				1
#define SERROR_SAVEGAME				2

#define GAME_NAME					"NOLF"
#define KEEPALIVE_FILENAME			"Save\\Kpalv.sav"
#define RELOADLEVEL_FILENAME		"Save\\Reload.sav"
#define QUICKSAVE_FILENAME			"Save\\Quick.sav"
#define SAVEGAMEINI_FILENAME		"Save\\Save1001.ini"  // v1.001
#define PLAYERSUMMARY_FILENAME		"Save\\Summary.sav"

#ifndef _DEMO
#define GAME_HANDSHAKE_VER_MAJOR	1 // GAME_HANDSHAKE_VER_MAJOR.GAME_HANDSHAKE_VER_MINOR - 1
#define GAME_HANDSHAKE_VER_MINOR	4 // 1.003
#else
#define GAME_HANDSHAKE_VER_MAJOR	0 // Demo = 0.002
#define GAME_HANDSHAKE_VER_MINOR	3    
#endif

#define GAME_HANDSHAKE_VER			((GAME_HANDSHAKE_VER_MAJOR << 8) + GAME_HANDSHAKE_VER_MINOR) 

#define GAME_HANDSHAKE_PASSWORD		0x40404040
#define GAME_HANDSHAKE_MASK			0x73173173

#define GAME_DISCON_BADHANDSHAKE	1 // Disconnect due to a bad handshake response
#define GAME_DISCON_BADWEAPONS		2 // Disconnect due to a weapons.txt mismatch
#define GAME_DISCON_BADCSHELL		3 // Disconnect due to a cshell.dll mismatch

#define INFINITE_MASS				100000.0f

#endif  // __CLIENT_SERVER_SHARED_H__