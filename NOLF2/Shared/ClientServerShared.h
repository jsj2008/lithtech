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

#include "LTObjRef.h"
#include <list>
#include "ltobjectcreate.h"
#include "CommonUtilities.h"
#include "NetDefs.h"

// Shared client/server enumerations...

// Player states

enum PlayerState { PS_UNKNOWN=0, PS_ALIVE, PS_DEAD, PS_DYING, PS_GHOST };

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
	PSI_LAND
};

enum CharacterSoundType { CST_NONE=0, CST_DAMAGE, CST_DEATH, CST_DIALOG, CST_EXCLAMATION, CST_AI_SOUND };

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
		case PSI_WEAPON_MISC1 :
		case PSI_WEAPON_MISC2 :
		case PSI_WEAPON_MISC3 :
		case PSI_WEAPON_MISC4 :
		case PSI_WEAPON_MISC5 :
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


// Sound class flags
enum
{
	DEFAULT_SOUND_CLASS = 0,
	WEAPONS_SOUND_CLASS,
	SPEECH_SOUND_CLASS,
	NUM_SOUND_CLASSES
};

// State of the save data when calling AskClientsForSaveData.
enum SaveDataState
{
	eSaveDataStateNone,
	eSaveDataStateSwitchLevels,
	eSaveDataStateTransitionLevels,
	eSaveDataSaveGameSlot,
	eSaveDataQuickSave,
	eSaveDataAutoSave,
};

// Doomsday pieces...
// NOTE: These MUST match the strings defined in the DoomsDayPiece.h file!!
enum DDPieceType
{
	kDoomsDay_transmitter,
	kDoomsDay_battery,
	kDoomsDay_Core,

	kDoomsDay_MAXTYPES
};

// Object user flags -
// NOTE: The top byte is reserved for surface flags

#define USRFLG_VISIBLE					(1<<0)
//#define USRFLG_SPY_VISION				(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

// Used with player attachments (weapon / flash).  XXX_HIDE1 indicates that this
// object is hidden on the client when in first person mode.  XXX_HIDE1SHOW3
// indicates that this object is hidden on the client in first person mode,
// and shown on the client in 3rd person mode....

#define USRFLG_ATTACH_HIDE1				(1<<3)
#define USRFLG_ATTACH_HIDE1SHOW3		(1<<4)

// Used with player attachments (specifically spears) to indicate that this attachment
// should not be shown on low-violence clients
#define USRFLG_ATTACH_HIDEGORE			(1<<19)

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

#define USRFLG_CAN_SEARCH				(1<<11)
#define USRFLG_REQUIRES_ENERGY			(1<<11) // Mutually exclusive to searching.
#define USRFLG_CAN_ACTIVATE				(1<<21)

// Used with the player object only (checked only on the client, and only
// on the player model)...

#define USRFLG_PLAYER_UNDERWATER		(1<<8)
#define USRFLG_PLAYER_DUCK				(1<<9)
#define USRFLG_PLAYER_SNOWMOBILE		(1<<10)
#define USRFLG_PLAYER_ALPHACYCLE		(1<<13)
#define USRFLG_PLAYER_LEANING			(1<<14)


// Used with character objects...(The USRFLG_CHAR_XXX can overlap the
// USRFLG_GADGET_XXX flags)

#define USRFLG_CHARACTER				(1<<12)

// Used with non-character (i.e., ~USRFLG_CHARACTER) objects that can
// interact with gadgets...

#define USRFLG_GADGET_BOMBABLE			(1<<9)
#define USRFLG_GADGET_INVISIBLE_INK		(1<<10)
#define USRFLG_GADGET_CAMERA_DISABLER	(1<<13)
#define USRFLG_GADGET_CODE_DECIPHERER	(1<<14)
#define USRFLG_GADGET_LOCK_PICK			(1<<15)
#define USRFLG_GADGET_WELDER			(1<<16)
#define USRFLG_GADGET_INTELLIGENCE		(1<<17)
#define USRFLG_GADGET_CAMERA			(1<<18)
#define USRFLG_GADGET_EAVESDROPBUG		(1<<20)

// Special objects that require permission sets.
// These duplicate some of the USRFLG_GADGET
// defines but that's okay because the objects
// are mutually exclusive.
#define USRFLG_PSET_1					(1<<13)
#define USRFLG_PSET_2					(1<<14)
#define USRFLG_PSET_3					(1<<15)
#define USRFLG_PSET_4					(1<<16)
#define USRFLG_PSET_5					(1<<17)
#define USRFLG_PSET_6					(1<<18)
#define USRFLG_PSET_7					(1<<19)
#define USRFLG_PSET_8					(1<<20)


#define USRFLG_GADGET_CAN_DISABLE	(USRFLG_GADGET_BOMBABLE | USRFLG_GADGET_INVISIBLE_INK |  \
									USRFLG_GADGET_CODE_DECIPHERER | USRFLG_GADGET_LOCK_PICK | USRFLG_GADGET_WELDER | \
									USRFLG_GADGET_INTELLIGENCE | USRFLG_GADGET_CAMERA | USRFLG_GADGET_EAVESDROPBUG )
		
// Used with AI objects.

#define USRFLG_AI_CLIENT_SOLID			(1<<3)


// Used when a GameBase object is active on the server

#define USRFLG_GAMEBASE_ACTIVE			(1<<22)

// Used with SpecialFX objects to determine, on the client,
// if a SpecialFX object is "on"

#define USRFLG_SFX_ON					(1<<23)

// Is this object a hitbox?  (Used for client-side hit detection)

#define USRFLG_HITBOX					(1<<24)

// ALL USRFLG's available!!!!


// These used to be part of User Flags but are now seperated out
// and sent to the clients through a MID_SFX_MESSAGE.

#define DMGFLG_CHAR_BLEEDING			(1<<0)
#define DMGFLG_CHAR_POISONED			(1<<1)
#define DMGFLG_CHAR_STUNNED				(1<<2)
#define DMGFLG_CHAR_SLEEPING			(1<<3)
#define DMGFLG_CHAR_CHOKE				(1<<4)
#define DMGFLG_CHAR_BURN				(1<<5)
#define DMGFLG_CHAR_ELECTROCUTE			(1<<6)
#define DMGFLG_CHAR_GLUE				(1<<7)
#define DMGFLG_CHAR_BEARTRAP			(1<<8)
#define DMGFLG_CHAR_LAUGHING			(1<<9)
#define DMGFLG_CHAR_ASSS				(1<<10)
#define DMGFLG_CHAR_FREEZING			(1<<11)
#define DMGFLG_CHAR_SLIPPING			(1<<12)



// Camera related flags...

#define CT_FULLSCREEN				0
#define CT_CINEMATIC				1


// Load/Save game defines...

#define LOAD_NEW_GAME				1	// Start from scratch - no saving or restoring
#define LOAD_NEW_LEVEL				2	// Save keep alives and level objects
#define LOAD_RESTORE_GAME			3	// No saving, but restore saved object
#define LOAD_TRANSITION				4	// Save transition objects to be loaded in new transition area

#define SERROR_LOADGAME				1
#define SERROR_SAVEGAME				2

#define INFINITE_MASS				100000.0f

#define MAX_DECISION_CHOICES		6

typedef std::vector<HSTRING> HStringArray;

typedef std::list< LTObjRef	> ObjRefList;
typedef std::list< LTObjRefNotifier > ObjRefNotifierList;

#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_MARKS_IN_REGION		10


class CButeMgr;

void ButeToConsoleFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ButeToConsoleString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ConsoleToButeFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );
void ConsoleToButeString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar );


#define DANGER(interface, engineer) \
interface->CPrint("----------------------------"); \
interface->CPrint("                            "); \
interface->CPrint("DDD   A  N   N GGGG EEE RR "); \
interface->CPrint("D  D A A NN  N G    E   R R"); \
interface->CPrint("D  D AAA N N N G GG EEE RR "); \
interface->CPrint("D  D A A N  NN G  G E   R R"); \
interface->CPrint("DDD  A A N   N GGGG EEE R R"); \
interface->CPrint("                            "); \
interface->CPrint("email "#engineer" immediately!!"); \
interface->CPrint("                            "); \
interface->CPrint("----------------------------");


// A quick accessor to the object type if you're absolutely sure the object is valid
inline uint32 GetObjectType(HOBJECT hObj)
{
	ASSERT(g_pCommonLT);

	uint32 nObjType;

	uint32 nResult = g_pCommonLT->GetObjectType(hObj, &nObjType);

	ASSERT(nResult == LT_OK);

	return nObjType;
}

// Utility functions for filtering intersection queries

//this will filter out all objects in the list, which is a list of HOBJECTs, terminated
//by NULL
inline bool ObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
            return false;
		++hList;
	}
    return true;
}

//Filters out anything but world models and the main world
inline bool WorldOnlyFilterFn(HOBJECT hTest, void *pUserData)
{
	//only accept it if it is a world model or if it is the main world
	if (IsMainWorld(hTest) || GetObjectType(hTest) == OT_WORLDMODEL)
	{
		//its a world
		return true;
	}
	//not either, don't want it
	return false;
}

// Old-style format for g_pCommonLT->SetObjectFilenames
inline LTRESULT SetObjectFilenames(HOBJECT hObj, const char *pFilename, const char *pSkinName)
{
	ObjectCreateStruct cOCS;
	cOCS.Clear();
	SAFE_STRCPY(cOCS.m_Filename, pFilename);
	SAFE_STRCPY(cOCS.m_SkinName, pSkinName);
	return g_pCommonLT->SetObjectFilenames(hObj, &cOCS);
}

// Sets a renderstyle of an object
#define RENDERSTYLE_MODEL_KEY "RS"
void SetObjectRenderStyle(HOBJECT hObj, uint8 nRenderStyleNum, char* szRenderStyleName);


// Useful macro for deleting HStrings
#ifdef _CLIENTBUILD
	#define TERMSTRING(str) if(str) { g_pLTClient->FreeString(str); str = NULL; }
#else
	#define TERMSTRING(str) if(str) { g_pLTServer->FreeString(str); str = NULL; }
#endif

// Gets the current game type.
GameType GetGameType( );

bool IsDeathmatchGameType();
bool IsCoopMultiplayerGameType();
bool IsTeamGameType();
bool IsRevivePlayerGameType( );
bool IsDifficultyGameType();

#ifdef SOURCE_RELEASE
#define GAME_HANDSHAKE_PASSWORD		0x40404040
#define GAME_HANDSHAKE_MASK			0x73173173
#endif // SOURCE_RELEASE

#endif  // __CLIENT_SERVER_SHARED_H__
