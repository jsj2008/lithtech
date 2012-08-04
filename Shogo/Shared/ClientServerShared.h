#ifndef __CLIENTSERVERSHARED_H
#define __CLIENTSERVERSHARED_H

// Shared client/server enumerations...

// Player states
enum PlayerState { PS_UNKNOWN=0, PS_ALIVE, PS_DEAD, PS_DYING };

// Single player game difficulty

enum GameDifficulty
{
	GD_EASY=0,
	GD_NORMAL,
	GD_HARD,
	GD_VERYHARD
};

// Strafe flags...

#define SF_LEFT		1
#define SF_RIGHT	2
#define SF_FORWARD	4
#define SF_BACKWARD	8

// Powerup types

enum PickupItemType 
{ 
	PIT_UNKNOWN=0, 
	
	// Armor...

	PIT_ARMOR_REPAIR100,
	PIT_ARMOR_REPAIR250,
	PIT_ARMOR_REPAIR500,
	PIT_ARMOR_BODY50,
	PIT_ARMOR_BODY100,
	PIT_ARMOR_BODY200,

	// Enhancements...

	PIT_ENHANCEMENT_DAMAGE,
	PIT_ENHANCEMENT_MELEEDAMAGE,
	PIT_ENHANCEMENT_PROTECTION,
	PIT_ENHANCEMENT_ENERGYPROTECTION,
	PIT_ENHANCEMENT_PROJECTILEPROTECTION,
	PIT_ENHANCEMENT_EXPLOSIVEPROTECTION,
	PIT_ENHANCEMENT_REGEN,
	PIT_ENHANCEMENT_HEALTH,
	PIT_ENHANCEMENT_ARMOR,

	// First aid...

	PIT_FIRSTAID_10,
	PIT_FIRSTAID_15,
	PIT_FIRSTAID_25,
	PIT_FIRSTAID_50,

	PIT_POWERSURGE_50,
	PIT_POWERSURGE_100,
	PIT_POWERSURGE_150,
	PIT_POWERSURGE_250,

	// Ultra Powerups...

	PIT_ULTRA_DAMAGE, 
	PIT_ULTRA_HEALTH, 
	PIT_ULTRA_POWERSURGE, 
	PIT_ULTRA_SHIELD, 
	PIT_ULTRA_STEALTH, 
	PIT_ULTRA_REFLECT, 
	PIT_ULTRA_NIGHTVISION, 
	PIT_ULTRA_INFRARED, 
	PIT_ULTRA_SILENCER, 
	PIT_ULTRA_RESTORE,

	// Uprades...

	PIT_UPGRADE_DAMAGE,
	PIT_UPGRADE_PROTECTION,
	PIT_UPGRADE_REGEN,
	PIT_UPGRADE_HEALTH,
	PIT_UPGRADE_ARMOR,
	PIT_UPGRADE_TARGETING,

	// Misc...

	PIT_CAT,
	PIT_SHOGO_S,
	PIT_SHOGO_H,
	PIT_SHOGO_O,
	PIT_SHOGO_G,

	PIT_MAX			// Always last enum
};			  
	

// Riot object user flags -
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

#define USRFLG_MODELADD					(1<<6)
#define USRFLG_GLOW						(1<<7)


// Used with the player object only...

#define USRFLG_PLAYER_TEARS				(1<<8)
#define USRFLG_PLAYER_UNDERWATER		(1<<9)
#define USRFLG_PLAYER_VEHICLE			(1<<10)
#define USRFLG_PLAYER_DUCK				(1<<11)

#define USRFLG_CHARACTER				(1<<12)
#define USRFLG_MOVEABLE					(1<<13)

// Used with pickup items only...

#define USRFLG_PICKUP_ROTATE			(1<<8)
#define USRFLG_PICKUP_BOUNCE			(1<<9)
#define USRFLG_PICKUP_RESPAWN			(1<<10)


#define CT_FULLSCREEN			0
#define CT_CINEMATIC			1


// Load/Save game defines...

#define LOAD_NEW_GAME			1	// Start from scratch - no saving or restoring
#define LOAD_NEW_LEVEL			2	// Save keep alives and level objects
#define LOAD_RESTORE_GAME		3	// No saving, but restore saved object

#define SERROR_LOADGAME			1
#define SERROR_SAVEGAME			2

#define KEEPALIVE_FILENAME		"Save\\Kpalv.sav"
#define RELOADLEVEL_FILENAME	"Save\\Reload.sav"
#define QUICKSAVE_FILENAME		"Save\\Quick.sav"
#define SAVEGAMEINI_FILENAME	"Save\\Save21.ini"  // v2.1


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

#endif
