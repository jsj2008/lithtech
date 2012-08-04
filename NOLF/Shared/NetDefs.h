/****************************************************************************
;
;	 MODULE:		NetDefs (.H)
;
;	PURPOSE:		Network game definitions
;
;	HISTORY:		07/09/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _NETDEFS_H_
#define _NETDEFS_H_

#include "ltbasetypes.h"

extern LTGUID NOLFGUID;

enum GameType
{
	SINGLE=0,
	COOPERATIVE_ASSAULT,
	DEATHMATCH
};

#define NGT_FILTER_ALL		0		//NGT: net game type
#define NGT_FILTER_LAST		DEATHMATCH

extern const int g_knNumGameTypes;
const char* GameTypeToString(GameType eType);

// Defines...

#define SO_DEFAULT_FILE "Attributes\\ServerOptions.txt"

#define SO_MAX_STR_LENGTH  64
#define SO_MAX_STRINGS	   5
#define SO_MAX_SUB_SET	   8

#define MAX_WORLDTIME_COLOR 64

#define NML_NAME			128				// NML: net max len
#define NML_HOST			128
#define	NML_LEVEL			128
#define NML_PLAYERS			128
#define NML_GAMES			128

#define NST_GAMENAME		"NAME"			// NST: net string token
#define NST_GAMEHOST		"HOST"
#define NST_GAMETYPE		"TYPE"
#define NST_GAMELEVEL		"LEVL"
#define NST_GAMETIME		"TIME"
#define NST_PLRCOUNT		"PLRS"
#define NST_PLRNAME_BASE	"PLRN"
#define NST_PLRFRAG_BASE	"PLRF"
#define NST_PLRID_BASE		"PLRI"
#define NST_CURLEVEL		"CLEV"
#define NST_NEXTLEVEL		"NLEV"
#define NST_GENERICMESSAGE	"GMSG"
#define NST_CONSOLEMESSAGE	"CMSG"

#define NGE_FRAGS			0				// NGE: net game end
#define NGE_TIME			1
#define NGE_FRAGSANDTIME	2
#define NGE_NEVER			3

#define NGM_STANDARDUPDATE	0				// NGM: net generic message
#define NGM_LEVELCHANGED	1
#define NGM_CONSOLEMSG		2
#define NGM_LEVELCHANGING	3
#define NGM_LEVELCHANGESTOP	4

#define NPC_BLACK			1				// NPC: net player color
#define NPC_WHITE			2
#define NPC_RED				3
#define NPC_GREEN			4
#define NPC_BLUE			5
#define NPC_CYAN			6
#define NPC_YELLOW			7
#define NPC_PURPLE			8
#define NPC_DEFAULT			NPC_BLUE

#define TEAM_AUTO			0
#define TEAM_1				1
#define TEAM_2				2
#define TEAM_1_COLOR		NPC_RED
#define TEAM_2_COLOR		NPC_BLUE
#define NUM_TEAMS			2

#define MAX_PLAYER_NAME		16
#define MAX_GAME_LEVELS		50
#define MAX_GAME_OPTIONS	20
#define MAX_OPTION_NAME		25
#define MAX_PASSWORD		16
#define MAX_SESSION_NAME	25

#define MAX_MULTI_PLAYERS			128
#define MAX_MULTI_PLAYERS_DISPLAY	16


// Structures...

typedef struct NetPlayer_t
{
	char	m_sName[MAX_PLAYER_NAME];
    uint8   m_byColor;
    uint32  m_dwLatency;
    uint32  m_dwTeam;

}	NetPlayer;

typedef struct NetClientData_t
{
	char	m_sName[MAX_PLAYER_NAME];
    uint32  m_dwTeam;

}	NetClientData;


typedef struct NetGame_t
{
	char	m_sSession[MAX_SESSION_NAME];
    uint8   m_byType;
    uint8   m_byNumLevels;
	char	m_sLevels[MAX_GAME_LEVELS][NML_LEVEL];
    uint8   m_byNumOptions;
    LTFLOAT  m_fOptions[MAX_GAME_OPTIONS];
	LTBOOL	m_bUsePassword;
	char	m_sPassword[MAX_PASSWORD];

}	NetGame;

#define SO_OPTION_TAG					"Option"

#define SO_VARIABLE						"Variable"
#define SO_SERV_VARIABLE				"ServVariable"
#define SO_NAME							"NameId"
#define SO_HELP							"HelpId"
#define SO_TYPE							"Type"
#define SO_STRINGS						"StringIds"
#define SO_RANGE						"SliderRange"
#define SO_INCREMENT					"SliderInc"
#define SO_SCALE						"SliderScale"
#define SO_GAME_TYPE					"GameType"
#define SO_DEFAULT						"Default"


enum eOptionType
{
	SO_TOGGLE = 0,
	SO_CYCLE,
	SO_SLIDER,
	SO_SLIDER_NUM,
	SO_SPECIAL,
	SO_UNKNOWN
};

class CButeMgr;

struct ServerOption
{
	ServerOption( );

	LTBOOL			InitializeFromBute( CButeMgr &buteMgr, const char *pszTagName );

	int				nId;

	char			szVariable[MAX_OPTION_NAME];
	char			szServVariable[MAX_OPTION_NAME];

	int				nNameId;
	int				nHelpId;
	eOptionType		eType;
	GameType		eGameType;


	int				nNumStrings;
	int				nStringId[SO_MAX_STRINGS];

	int				nSliderMin;
	int				nSliderMax;
	int				nSliderInc;

    LTFLOAT          fSliderScale;

    LTFLOAT          fDefault;

};

typedef struct ServerOptions_t
{
    LTBOOL   m_bTractorBeam;
    LTFLOAT  m_fRunSpeed;
    LTFLOAT  m_fMissileSpeed;
    LTFLOAT  m_fRespawnScale;
    LTFLOAT  m_fHealScale;
    LTFLOAT  m_fWorldTimeSpeed;
	char	m_sWorldNightColor[32];

}	ServerOptions;

typedef struct GameData_struct
{
    LTFLOAT  m_fRunSpeed;
    LTFLOAT  m_fRespawnScale;
    uint8   m_byEnd;
    uint32  m_dwEndFrags;
    uint32  m_dwEndTime;
	LTBOOL	m_bUsePassword;
	char	m_szPassword[MAX_PASSWORD];

}	GAMEDATA;

enum ServerApp
{
	SERVERAPP_ADDCLIENT,
	SERVERAPP_REMOVECLIENT,
	SERVERAPP_SHELLUPDATE,
	SERVERAPP_PRELOADWORLD,
	SERVERAPP_POSTLOADWORLD,
	SERVERAPP_CONSOLEMESSAGE,
};

enum ServerShell
{
	SERVERSHELL_INIT,
	SERVERSHELL_NEXTWORLD,
	SERVERSHELL_SETWORLD
};

// EOF...

#endif