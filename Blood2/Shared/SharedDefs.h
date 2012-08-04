#ifndef __SHAREDEFS_H__
#define __SHAREDEFS_H__


#include "assert.h"
#include "NetDefs.h"

//**************************************************************************

#define B2_LITE

#ifdef B2_LITE
	#define NRES(x)				3
	#define B2_LITE_MAXSOUNDS	3
#else
	#define NRES(x) x
	#define B2_LITE_MAXSOUNDS	300
#endif

//**************************************************************************

#define MAX_PLAYERNAME_LEN	40

//#include "serverobj_de.h"

// Messages from server to client
enum ServerClientMsgs 
{
	SMSG_CONSOLEMESSAGE	= 100,
	SMSG_CONSOLEMESSAGE_ALL,
	SMSG_FIREDWEAPON,			// Player has fired his current weapon
	SMSG_VELMAGNITUDE,			// Velocity magnitude used for head bobbing
	SMSG_EYELEVEL,				// Player has changed eye levels
	SMSG_SPECTATORMODE,			// Specatator mode active/inactive
	SMSG_NEWWEAPON,				// Player has a new current weapon
	SMSG_DELETEWEAPON,	
	SMSG_DELETESPECIFICWEAPON,	// NEW MESSAGE BY ANDY
	SMSG_CHANGEWEAPON,	
	SMSG_WEAPONLOWERED,
	SMSG_WEAPONRAISED,
	SMSG_ITEMCHANGED,			// Inventory item changed
	SMSG_CONVERSATION,			// NEW MESSAGE BY ANDY to play a conversation dialog
	SMSG_OBJECTIVES,			// To change the dialog within the objectives screen
	SMSG_ZOOMVIEW,				// Zoom view for weapons that support it
	SMSG_RESPAWN,				// Tells the client the player has respawned
	SMSG_HEALTH,				// Updates status bar health display
	SMSG_ARMOR,					// Updates status bar armor display
	SMSG_AMMO,					// Updates status bar ammo display
	SMSG_FOCUS,			
	SMSG_ADDCAMERA,				// Adds an external camera to the client
	SMSG_ADDPLAYER,				// Adds a new player to the player list (multiplayer)
	SMSG_ADDPLAYERS,			// Adds a bunch of players to list (for new players)
	SMSG_UPDATEPLAYER,			// Updates player's multiplayer info
	SMSG_REMOVEPLAYER,			// Remove a player from the list
	SMSG_SHUTDOWN,				// HACK - server shutdown message
	SMSG_CAMERA_FOV,			// Change the camera FOV
	SMSG_CAMERA_RECT,			// Change the camera Rect box
	SMSG_CAMERA_RESET,   
	SMSG_CAMERA_SELECT,  
	SMSG_CUTSCENE_START, 
	SMSG_CUTSCENE_END,   
	SMSG_DISPLAYTEXT,    
	SMSG_SCREENFADE,    
	SMSG_ALL_SEEING_EYE,
	SMSG_BLIND,	
	SMSG_ORBCAM,
	SMSG_BURN,			
	SMSG_EXITWORLD,				// Player has hit an exit trigger
	SMSG_WORLDINFO,				// World name, objectives, etc.
	SMSG_FORCEROTATION,			// Set the player's pitch & roll to these values.
	SMSG_SAVEGAME_ACK,			// Sent in acknowledgement of CMSG_SAVEGAME.
	SMSG_LOADWORLD_ACK,			// Sent in acknowledgement of CMSG_LOADWORLD.
	SMSG_CLIENTSAVEDATA,		// Sent when playerobj is loaded
	SMSG_TOOKDAMAGE,			// Sent when damaged, tint screen & play sound, etc.
	SMSG_MUSIC,					// Music message
	SMSG_PLAYERINIT_MULTIPLAYER,
	SMSG_PLAYERINIT_SINGLEPLAYER,
	SMSG_THIEF_ATTACH,
	SMSG_HAND_ATTACH,
	SMSG_BONELEECH_ATTACH,
	SMSG_DETACH_AI,
	SMSG_PLAYER_FRAGGED,
	SMSG_MP_CHANGING_LEVELS,
	SMSG_THEVOICE,
	SMSG_DEAD,					// I just died
	SMSG_WONKYVISION,
	SMSG_PLAYERSTATSUPDATE,		// Updates player stats like armor & health.
	SMSG_THROWDISTANCE,
	SMSG_BOSSHEALTH,
	SMSG_VOICEMGR,
	SMSG_VOICEMGR_STOPALL,
	SMSG_INVITEMACTION,
	SMSG_WEAPONHANDMODEL,		// New weapon hand model
	SMSG_DOAUTOSAVE,			// Do an autosave	
	SMSG_PHYSICS_UPDATE,
	SMSG_FORCEPOS,
	SMSG_CAPTUREDTHEFLAG,		// A player successfully captured the flag in CTF play
	SMSG_GRABBEDTHEFLAG,		// The player has grabbed the other teams flag in CTF play
	SMSG_RETURNEDTHEFLAG,		// The player has returned their own flag in CTF play
	SMSG_DROPPEDTHEFLAG,		// The player no longer has the flag in CTF play
	SMSG_NEEDYOURFLAG,			// The player needs to have their flag in CTF play
	SMSG_PINGTIMES,				// Packet containing all player ping times
	SMSG_BOOTED,				// Player has been booted by the server
	SMSG_SERVERSHUTDOWN,		// Server has been shutdown
	SMSG_SOCCERGOAL,			// A player scored a goal
	SMSG_TRAPPED,				// The client is being held still by something (flayer)
	SMSG_MAXMESSAGES
};

// Messages from client to server
enum ClientServerMsgs
{
	CMSG_SINGLEPLAYER_INIT=100,
	CMSG_MULTIPLAYER_INIT,
	CMSG_PLAYERUPDATE,
	CMSG_PLAYERMESSAGE,
	CMSG_SETCURRENTINVITEM,
	CMSG_CHEAT,
	CMSG_GAME_PAUSE,
	CMSG_AUTOSAVE,
	CMSG_LOADWORLD,
	CMSG_SAVEGAME,
	CMSG_RESTOREGAME,
	CMSG_SETCURRENTWEAPON,
	CMSG_DETACH_AI,				//for creatures attaching SCHLEGZ
	CMSG_FRAGSELF,				// Frag yourself
	CMSG_ATTACH_ACK,			//acknowledgement to attach
	CMSG_PLAYERPOSITION,
	CMSG_WEAPON_FIRE,
	CMSG_WEAPON_SOUND,
	CMSG_WEAPON_STATE,
	CMSG_WEAPON_CHANGE,
	CMSG_NEXTLEVEL,
	CMSG_MAXMESSAGES,
	CMSG_RESETPLAYERINVENTORY
};


// Client data flags
#define CDATA_SHOWGUN		1	// Tells the player obj to hide it's view weapon
#define CDATA_RUNLOCK		2
#define CDATA_MOUSEAIMING	4
#define CDATA_CANMOVE		8

// player update flags
#define PLAYERUPDATE_FLAGS				1
#define PLAYERUPDATE_PITCH				2
#define PLAYERUPDATE_YAW				4
#define PLAYERUPDATE_MUZZLE				8
#define PLAYERUPDATE_LMUZZLE			16
#define PLAYERUPDATE_MOUSE				32
#define PLAYERUPDATE_CONTAINER			64	
#define PLAYERUPDATE_CONTAINERATBOTTOM	128	

// Player stats flags
#define PLAYERSTATS_TOOKDAMAGE	(1<<0)
#define PLAYERSTATS_HEALTH		(1<<1)
#define PLAYERSTATS_AMMO		(1<<2)
#define PLAYERSTATS_ALTAMMO		(1<<3)
#define PLAYERSTATS_ARMOR		(1<<4)
#define PLAYERSTATS_FOCUS		(1<<5)
#define PLAYERSTATS_AIRLEVEL	(1<<6)
#define PLAYERSTATS_ALL			0xff

// Inventory item change flags
#define ITEMF_CURRENTITEM			(1<<0)
#define ITEMF_PREVITEM				(1<<1)
#define ITEMF_NEXTITEM				(1<<2)
#define ITEMF_CURRENTITEMCHARGE		(1<<3)
#define ITEMF_PREVITEMCHARGE		(1<<4)
#define ITEMF_NEXTITEMCHARGE		(1<<5)
#define ITEMF_SHOWITEMS				(1<<7)

// Object to object message IDs
#define		MID_KEYPICKUP			3500
#define		MID_KEYQUERY			3501
#define		MID_KEYQUERYRESPONSE	3502
#define		MID_KEYREMOVE			3503
#define		MID_ADDPOWERUP			3504
#define		MID_IMMOBILIZE			3505
#define		MID_DOORBLOCK			3506
#define		MID_ATTACH				3507
#define		MID_UNATTACH			3508
#define		MID_GOAL				3509
#define		MID_INCONTAINER			3510

// sound types
#define		SOUND_GUNFIRE			0
#define		SOUND_GUNIMPACT			1
#define		SOUND_PLAYERSOUND		2

// weapon definitions
#define		TYPE_RIFLE				0
#define		TYPE_AUTORIFLE			1
#define		TYPE_PISTOL				2
#define		TYPE_MELEE				4
#define		TYPE_MAGIC				8

//PISTOLS
#define		WEAP_NONE				0
#define		WEAP_BERETTA			1
#define		WEAP_SUBMACHINEGUN		2
#define		WEAP_FLAREGUN			3
//RIFLES
#define		WEAP_SHOTGUN			4
#define		WEAP_SNIPERRIFLE		5
#define		WEAP_HOWITZER			6
#define		WEAP_NAPALMCANNON		7
#define		WEAP_SINGULARITY		8
//AUTO RIFLES
#define		WEAP_ASSAULTRIFLE		9
#define		WEAP_BUGSPRAY			10
#define		WEAP_MINIGUN			11
#define		WEAP_DEATHRAY			12
#define		WEAP_TESLACANNON		13
//OTHER
#define		WEAP_VOODOO				14
#define		WEAP_ORB				15
#define		WEAP_LIFELEECH			16

#define	    WEAP_MELEE				17

#ifdef _ADD_ON
	#define		WEAP_COMBATSHOTGUN		18
	#define		WEAP_FLAYER				19
	#define		WEAP_LASTPLAYERWEAPON	19
#else
	#define		WEAP_LASTPLAYERWEAPON	17
#endif

//CREATURE ATTACKS
#define		WEAP_SHIKARI_CLAW		WEAP_LASTPLAYERWEAPON + 1
#define		WEAP_SHIKARI_SPIT		WEAP_LASTPLAYERWEAPON + 2
#define		WEAP_SOUL_CROWBAR		WEAP_LASTPLAYERWEAPON + 3
#define		WEAP_SOUL_AXE			WEAP_LASTPLAYERWEAPON + 4
#define		WEAP_SOUL_PIPE			WEAP_LASTPLAYERWEAPON + 5
#define		WEAP_SOUL_HOOK			WEAP_LASTPLAYERWEAPON + 6
#define		WEAP_BEHEMOTH_CLAW		WEAP_LASTPLAYERWEAPON + 7
#define		WEAP_ZEALOT_HEAL		WEAP_LASTPLAYERWEAPON + 8
#define		WEAP_ZEALOT_SHIELD		WEAP_LASTPLAYERWEAPON + 9
#define		WEAP_ZEALOT_ENERGYBLAST	WEAP_LASTPLAYERWEAPON + 10
#define		WEAP_ZEALOT_GROUNDFIRE	WEAP_LASTPLAYERWEAPON + 11
#define		WEAP_ZEALOT_SHOCKWAVE	WEAP_LASTPLAYERWEAPON + 12
#define		WEAP_DRUDGE_FIREBALL	WEAP_LASTPLAYERWEAPON + 13
#define		WEAP_DRUDGE_LIGHTNING	WEAP_LASTPLAYERWEAPON + 14
#define		WEAP_HAND_SQUEEZE		WEAP_LASTPLAYERWEAPON + 15
#define		WEAP_BONELEECH_SUCK		WEAP_LASTPLAYERWEAPON + 16
#define		WEAP_THIEF_SUCK			WEAP_LASTPLAYERWEAPON + 17
#define		WEAP_NIGHTMARE_BITE		WEAP_LASTPLAYERWEAPON + 18
#define		WEAP_BEHEMOTH_SHOCKWAVE	WEAP_LASTPLAYERWEAPON + 19
#define		WEAP_DEATHSHROUD_ZAP	WEAP_LASTPLAYERWEAPON + 20
#define		WEAP_NAGA_EYEBEAM		WEAP_LASTPLAYERWEAPON + 21
#define		WEAP_NAGA_SPIKE			WEAP_LASTPLAYERWEAPON + 22
#define		WEAP_NAGA_DEBRIS		WEAP_LASTPLAYERWEAPON + 23
#define		WEAP_GIDEON_SHIELD		WEAP_LASTPLAYERWEAPON + 24
#define		WEAP_GIDEON_WIND		WEAP_LASTPLAYERWEAPON + 25
#define		WEAP_GIDEON_VOMIT		WEAP_LASTPLAYERWEAPON + 26
#define		WEAP_GIDEON_GOO			WEAP_LASTPLAYERWEAPON + 27
#define		WEAP_GIDEON_SPEAR		WEAP_LASTPLAYERWEAPON + 28
#define		WEAP_ANCIENTONE_BEAM	WEAP_LASTPLAYERWEAPON + 29
#define		WEAP_ANCIENTONE_TENTACLE WEAP_LASTPLAYERWEAPON + 30
#define		WEAP_SKULL				WEAP_LASTPLAYERWEAPON + 31

#ifdef _ADD_ON
	#define		WEAP_GREMLIN_ROCK			WEAP_LASTPLAYERWEAPON + 32
	#define		WEAP_NIGHTMARE_FIREBALLS	WEAP_LASTPLAYERWEAPON + 33
	#define		WEAP_LASTCREATUREWEAPON		WEAP_NIGHTMARE_FIREBALLS
#else
	#define		WEAP_LASTCREATUREWEAPON	WEAP_SKULL
#endif

// Inventory Weapons..
#define		WEAP_PROXIMITYBOMB		(WEAP_LASTCREATUREWEAPON + 1)
#define		WEAP_REMOTEBOMB			(WEAP_LASTCREATUREWEAPON + 2)
#define		WEAP_TIMEBOMB			(WEAP_LASTCREATUREWEAPON + 3)

#define		WEAP_MAXWEAPONTYPES	WEAP_LASTCREATUREWEAPON + 3
#define		WEAP_BASEINVENTORY  WEAP_PROXIMITYBOMB


#define		AMMO_NONE			0
#define		AMMO_BULLET			1
#define		AMMO_SHELL			2
#define		AMMO_BMG			3
#define		AMMO_FLARE			4
#define		AMMO_DIEBUGDIE		5
#define		AMMO_HOWITZER		6
#define		AMMO_FUEL			7
#define		AMMO_BATTERY		8
#define		AMMO_FOCUS			9
#define		AMMO_PROXIMITYBOMB	10
#define		AMMO_REMOTEBOMB		11
#define		AMMO_TIMEBOMB		12
#define		AMMO_MAXAMMOTYPES	12


#define		INV_NONE			0
#define		INV_MEDKIT			1
#define		INV_FLASHLIGHT		2
#define		INV_NIGHTGOGGLES	3
#define		INV_THEEYE			4
#define		INV_BINOCULARS		5
#define		INV_KEY				6
#define		INV_LASTINVITEM		6


#define		INV_BASEINVWEAPON	7
#define		INV_PROXIMITY		7
#define		INV_REMOTE			8
#define		INV_TIMEBOMB		9
#define		INV_LASTINVWEAPON	9

#define SLOTCOUNT_WEAPONS		10	// 10 weapons
#define SLOTCOUNT_INVWEAPONS	(INV_LASTINVWEAPON - INV_BASEINVWEAPON + 1)
#define SLOTCOUNT_TOTALWEAPONS	(SLOTCOUNT_WEAPONS + SLOTCOUNT_INVWEAPONS)

#define		AMMOCOUNT_BULLET		100
#define		AMMOCOUNT_SHELL			20
#define		AMMOCOUNT_BMG			20
#define		AMMOCOUNT_FLARE			20
#define		AMMOCOUNT_DIEBUGDIE		20
#define		AMMOCOUNT_HOWITZER		5
#define		AMMOCOUNT_FUEL			10
#define		AMMOCOUNT_BATTERY		40
#define		AMMOCOUNT_PROXIMITYBOMB	1
#define		AMMOCOUNT_REMOTEBOMB	1
#define		AMMOCOUNT_TIMEBOMB		1


#define		POWERUP_NONE				0
#define		POWERUP_HEALTH				1
#define		POWERUP_MEGAHEALTH			2
#define		POWERUP_WARD				3
#define		POWERUP_NECROWARD			4
#define		POWERUP_INVULNERABILITY		5
#define		POWERUP_STEALTH				6
#define		POWERUP_ANGER				7
#define		POWERUP_REVENANT			8
#define		POWERUP_MAXPOWERUPTYPES		9


#define		MIN_CHARACTER			0
#define		CHARACTER_CALEB			0
#define		CHARACTER_OPHELIA		1
#define		CHARACTER_ISHMAEL		2
#define		CHARACTER_GABREILLA		3

#ifdef _ADD_ON

	#define		CHARACTER_M_CULTIST		4
	#define		CHARACTER_F_CULTIST		5
	#define		CHARACTER_SOULDRUDGE	6
	#define		CHARACTER_PROPHET		7
	#define		MAX_CHARACTER			7
	#define		MAX_CHARACTERS			8

#else

	#define		MAX_CHARACTER			3
	#define		MAX_CHARACTERS			4

#endif

#define		MAX_ATTRIBUTES			12
#define		DEFAULT_ATTRIBUTE		3

#define		MULTIPLAY_SKIN_MIN		0
#define		MULTIPLAY_SKIN_NORMAL	0
#define		MULTIPLAY_SKIN_RED		1
#define		MULTIPLAY_SKIN_BLUE		2
#define		MULTIPLAY_SKIN_GREEN	3
#define		MULTIPLAY_SKIN_YELLOW	4
#define		MULTIPLAY_SKIN_MAX		6	// [blg] 01/04/99 added two more for team skins


// Lower 4 bits of damage indicate damage type.
#define DAMAGE_TYPE_NORMAL	     0
#define DAMAGE_TYPE_FIRE	     1
#define DAMAGE_TYPE_FREEZE	     2	    // Used by Carbon freezer
#define DAMAGE_TYPE_SINGULARITY  3	    // Used by singularity generator
#define DAMAGE_TYPE_MELEE        4	    // Used by melee weapon
#define DAMAGE_TYPE_EXPLODE      5		// Used for explosions, like grenade
#define DAMAGE_TYPE_SUFFOCATE	 6		// Drowning, smothering.
#define DAMAGE_TYPE_DEATH		 7		// Instant death
#define DAMAGE_TYPE_FREEFALL	 8		// Endless falling, eventual death
#define DAMAGE_TYPE_BLIND        9		// Used by Voodoo doll
#define DAMAGE_TYPE_SLOW         10		// Used by Voodoo doll
#define DAMAGE_TYPE_DROPWEAPON   11		// Used by Voodoo doll
#define DAMAGE_TYPE_VISION       12		// Used by Voodoo doll
#define DAMAGE_TYPE_ELECTRIC     13		// Used by laser and lightning
#define DAMAGE_TYPE_ACID         14		// Used by bug spray and loogies
#define DAMAGE_TYPE_LEECH		 15		// Used by the lifeleech primary

// Upper 4 bits are damage flags
#define DAMAGE_FLAG_AREAEFFECT	 16		// Damage is area of effect
#define DAMAGE_FLAG_1			 32
#define DAMAGE_FLAG_2			 64
#define DAMAGE_FLAG_3			 128


// Structures...
								
typedef struct B2C_struct
{
	DDWORD	dwCharacter;
	DDWORD	dwColor;
	DDWORD	dwStrength;
	DDWORD	dwSpeed;
	DDWORD	dwResist;
	DDWORD	dwFocus;
	DDWORD	dwWeap1;
	DDWORD	dwWeap2;
	DDWORD	dwWeap3;
	DDWORD	dwWeap4;
	DDWORD	dwWeap5;
	DDWORD	dwWeap6;
	DDWORD	dwWeap7;
	DDWORD	dwWeap8;
	DDWORD	dwWeap9;

}	B2C;



enum KeyTypes
{
	KEYTYPE_NONE = 0,
	KEYTYPE_1,
	KEYTYPE_2,
	KEYTYPE_3,
	KEYTYPE_CRANELEVER,			// Special key types
	KEYTYPE_MAX = KEYTYPE_CRANELEVER
};


// Cheat codes
enum CheatCodes
{
	CHEAT_NONE = -1,
	CHEAT_GOD,				// god mode toggle
	CHEAT_KFA,				// Monolith Productions kicks fucking ass!
	CHEAT_AMMO,				// full ammo
	CHEAT_CLIP,				// clipping toggle
	CHEAT_HEALTH,			// Full health
	CHEAT_WHEREAMI,			// Displays position
	CHEAT_STEALTH,			// Gives invisibility powerup
	CHEAT_TRIPLEDAMAGE,		// Gives triple damage powerup
	CHEAT_KILLALLAI,		// Kill all AIs
	CHEAT_INCSPEED,			// Increase speed attribute
	CHEAT_INCSTRENGTH,		// Increase strength attribute
	CHEAT_CALEB,			// Set to these characters
	CHEAT_OPHELIA,
	CHEAT_ISHMAEL,
	CHEAT_GABRIELLA,
	CHEAT_BERETTA,			// Weapon pickup cheats
	CHEAT_MAC10,			// sub-machine gun
	CHEAT_FLAREGUN,
	CHEAT_SHOTGUN,
	CHEAT_SNIPERRIFLE,
	CHEAT_HOWITZER,
	CHEAT_NAPALMCANNON,
	CHEAT_SINGULARITY,
	CHEAT_ASSAULTRIFLE,		// m-16
	CHEAT_BUGSPRAY,
	CHEAT_MINIGUN,
	CHEAT_DEATHRAY,
	CHEAT_TESLACANNON,
	CHEAT_VOODOO,
	CHEAT_ORB,
	CHEAT_LIFELEECH,
	CHEAT_KNIFE,
#ifdef _ADD_ON
	CHEAT_COMBATSHOTGUN,
	CHEAT_FLAYER,
#endif
	CHEAT_GOBLE,
	CHEAT_SCORPIO,
	CHEAT_TOTARO,
	CHEAT_DEMOLEVEL2,		// go to second demo level
	CHEAT_GIVEALLINV,		// give all inventory items
	CHEAT_POW_HEALTH,		// health powerup
	CHEAT_POW_MEGAHEALTH,
	CHEAT_POW_WARD,
	CHEAT_POW_NECROWARD,
	CHEAT_POW_INVULN,
	CHEAT_POW_STEALTH,
	CHEAT_POW_ANGER,
	CHEAT_KFA2,				// god mode toggle (different weapons)
	CHEAT_CHASEVIEW,		// Chase view
	CHEAT_MAX
};

#define CHEAT_FIRSTWEAPON	CHEAT_BERETTA

#ifdef _ADD_ON
	#define CHEAT_LASTWEAPON	CHEAT_FLAYER
#else
	#define CHEAT_LASTWEAPON	CHEAT_LIFELEECH
#endif

// Base surface types for when we hit something.  There can be some subtypes
enum SurfaceType
{
	SURFTYPE_UNKNOWN = 0,	// Could be anything
	SURFTYPE_STONE   = 10,	// Stone
	SURFTYPE_METAL   = 20,	// Metal
	SURFTYPE_WOOD    = 30,	// Wood
	SURFTYPE_ENERGY	 = 40,	// Energy (Force fields, etc)
	SURFTYPE_GLASS   = 50,  // Glass
	SURFTYPE_BUILDING= 60,
	SURFTYPE_TERRAIN = 70,	// Dirt, ice, etc.
	SURFTYPE_CLOTH   = 80,	// Cloth, carpet, furniture
	SURFTYPE_PLASTIC = 90,
	SURFTYPE_FLESH   = 100,
	SURFTYPE_SKY	 = 110,	// Sky textures
	SURFTYPE_FIRETHROUGH = 120,	// Vector weapons can fire through
	SURFTYPE_LIQUID  = 200,
	SURFTYPE_MAX,
};


enum GameType
{
	GAMETYPE_SINGLE = NGT_SINGLE,			// Single player game
	GAMETYPE_BLOODBATH = NGT_DEATHMATCH,	// Multiplayer game
	GAMETYPE_CTF = NGT_CAPTUREFLAG,			// Capture the Flag
	GAMETYPE_COOP = NGT_COOPERATIVE,		// Coop play
	GAMETYPE_TEAMPLAY = NGT_TEAMS,			// Team play
	GAMETYPE_SOCCER = NGT_SOCCER,			// Zombie head soccer
	GAMETYPE_TAG = NGT_TOETAG,				// Tag
	GAMETYPE_LEVELRACE,						// Level racing
	GAMETYPE_ACTION,						// Action mode (single player, no story elements)
	GAMETYPE_CUSTOM,						// Custom level (single player)
};


// Load/Save game defines...
#define LOADTYPE_NEW_GAME			1	// Start from scratch - no saving or restoring
#define LOADTYPE_NEW_LEVEL			2	// Save keep alives and level objects
#define LOADTYPE_RESTORESAVE		3	// Don't save. Restore a saved world
#define LOADTYPE_RESTOREAUTOSAVE	4	// Don't save. Restore a saved world

// Save types
#define SAVETYPE_AUTO		0
#define SAVETYPE_CURRENT	1

// Team IDs
#define TEAM_1				1
#define TEAM_2				2


enum RestoreTypes
{
};


enum TeamplayOptions
{
	TEAMOPT_NOFRIENDLYFIRE = 0,
	TEAMOPT_FRIENDLYFIRE,
	TEAMOPT_REDUCEDDAMAGE,
	TEAMOPT_ARMORDAMAGEONLY
};


enum SkillLevels
{
	DIFFICULTY_EASY = 0,
	DIFFICULTY_MEDIUM,
	DIFFICULTY_HARD,
	DIFFICULTY_INSANE
};


enum AmmoLevels
{
	AMMO_REDUCED = 0,
	AMMO_NORMAL,
	AMMO_ABUNDANT,
	AMMO_EXCESSIVE
};


enum ItemOptions
{
	ITEMOPT_RESPAWN = 0,
	ITEMOPT_NORESPAWN,
	ITEMOPT_RESPAWN_MARKER,
	ITEMOPT_RESPAWN_RANDOM
};



// Music commands
enum MusicCommand
{ 
	MUSICCMD_LEVEL, 
	MUSICCMD_MOTIF, 
	MUSICCMD_MOTIF_LOOP, 
	MUSICCMD_MOTIF_STOP, 
	MUSICCMD_BREAK 
};


// "The Voice" type ids
enum TheVoice
{
	VOICE_START_BB,
	VOICE_START_CTF,
	VOICE_KILL,
	VOICE_SUICIDE,
	VOICE_FINISHHIM,
};


#define VOICEFLAG_NONE				0
#define VOICEFLAG_MALE_ATTACKER		1
#define VOICEFLAG_FEMALE_ATTACKER	2
#define VOICEFLAG_MALE_VICTIM		4
#define VOICEFLAG_FEMALE_VICTIM		8
#define VOICEFLAG_ALL_ATTACKER		(VOICEFLAG_MALE_ATTACKER | VOICEFLAG_FEMALE_ATTACKER)
#define VOICEFLAG_ALL_VICTIM		(VOICEFLAG_MALE_VICTIM | VOICEFLAG_FEMALE_VICTIM)
#define VOICEFLAG_ALL				(VOICEFLAG_ALL_VICTIM | VOICEFLAG_ALL_ATTACKER)


// Wonky vision defines

#define WONKY_VISION_SAFE_TIME		0.5f
#define WONKY_VISION_STAY_TIME		5.0f
#define WONKY_VISION_DEFAULT_TIME	15.0f


// Various useful defines

#define PI				(DFLOAT)3.14159
#define PIx2			(DFLOAT)PI*2
#define DEG2RAD(x)		((x*PI)/180)
#define RAD2DEG(x)		((x*180)/PI)

#define VEC_EQU(v1, v2) (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
#define ROTN_EQU(r1, r2) (VEC_EQU(r1.m_Vec, r2.m_Vec) && r1.m_Spin == r2.m_Spin)
#define ROTN_SUB(d, r1, r2) VEC_SUB(d.m_Vec, r1.m_Vec, r2.m_Vec); d.m_Spin = r1.m_Spin - r2.m_Spin;
#define ROTN_ADD(d, r1, r2) VEC_ADD(d.m_Vec, r1.m_Vec, r2.m_Vec); d.m_Spin = r1.m_Spin + r2.m_Spin;


#define CLIPLOWHIGH(x,l,h)  if (x < l) x = l; if (x > h) x = h;


// SFX_CAMERA_ID flags
#define CAMERASFXFLAG_ALLOWPLAYERMOVEMENT		(1<<0)
#define CAMERASFXFLAG_HIDEPLAYER				(1<<1)
#define CAMERASFXFLAG_ISLISTENER				(1<<2)

#endif  // __SHAREDEFS_H__