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


// Defines...

#define NGT_SINGLE			0				// NGT: net game type
#define	NGT_DEATHMATCH		1
#define NGT_CAPTUREFLAG		2
#define NGT_COOPERATIVE		3
#define NGT_TEAMS			4
#define NGT_SOCCER			5
#define NGT_TOETAG			6

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

#define NMT_ORDOG			1				// NMT: net mech type
#define NMT_ENFORCER		2
#define NMT_PREDATOR		3
#define NMT_AKUMA			4

#define NGE_FRAGS			1				// NGE: net game end
#define NGE_TIME			2
#define NGE_FRAGSANDTIME	3
#define NGE_NEVER			4

#define NGM_STANDARDUPDATE	0				// NGM: net generic message
#define NGM_LEVELCHANGED	1
#define NGM_CONSOLEMSG		2
#define NGM_LEVELCHANGING	3

#define NPC_BLACK			1				// NPC: net player color
#define NPC_DARKRED			2
#define NPC_DARKGREEN		3
#define NPC_DARKBLUE		4
#define NPC_DARKPURPLE		5
#define NPC_BROWN			6
#define NPC_GRAY			7
#define NPC_BRIGHTRED		8
#define NPC_BRIGHTGREEN		9
#define NPC_BRIGHTBLUE		10
#define NPC_BRIGHTPURPLE	11
#define NPC_YELLOW			12
#define NPC_WHITE			13

#define LEVEL_NONE			0
#define LEVEL_HALF			1
#define LEVEL_NORMAL		2
#define LEVEL_DOUBLE		3
#define LEVEL_INSANE		4

#define HEAL_NONE			0
#define HEAL_REALLYSLOW		1
#define HEAL_SLOW			2
#define HEAL_NORMAL			3
#define HEAL_FAST			4
#define HEAL_REALLYFAST		5

#define TEAM_AUTO			0
#define TEAM_1				1
#define TEAM_2				2

#define MAX_PLAYER_NAME		32
#define MAX_CONFIG_NAME		64
#define MAX_GAME_LEVELS		50

#define MAX_MULTI_PLAYERS			128
#define MAX_MULTI_PLAYERS_DISPLAY	16

#define SOCBALL_SKIN_SOCCER	0
#define SOCBALL_SKIN_ZOMBIE	1


// Structures...

typedef struct NetPlayer_t
{
	char	m_sName[MAX_PLAYER_NAME];
	char	m_sConfig[MAX_CONFIG_NAME];
	DDWORD	m_dwTeam;
	DDWORD	m_dwLatency;

}	NetPlayer;

typedef struct NetGame_t
{
	DBYTE	m_byType;
	DBYTE	m_byEnd;
	DDWORD	m_dwEndFrags;
	DDWORD	m_dwEndTime;
	DBYTE	m_byNumLevels;
	char	m_sLevels[MAX_GAME_LEVELS][NML_LEVEL];
	int		m_nAmmoLevel;
	int		m_nAmmoRespawn;
	int		m_nArmorLevel;
	int		m_nArmorRespawn;
	int		m_nHealthLevel;
	int		m_nHealthRespawn;
	int		m_nPowerupsLevel;
	int		m_nPowerupsRespawn;
	int		m_nHealingRate;
	int		m_nFlagValue;
	int		m_nGoalValue;
	DBOOL	m_bFallDamage;
	DBOOL	m_bFriendlyFire;
	DBOOL	m_bNegTeamFrags;
	DBOOL	m_bOnlyFlagScores;
	DBOOL	m_bOnlyGoalScores;
	DBOOL	m_bUseTeamSize;
	int		m_nSocBallSkin;

}	NetGame;

typedef struct NetClientData_t
{
	DDWORD	m_dwTeam;

}	NetClientData;

typedef struct ServerOptions_t
{
	DFLOAT	m_fRunSpeed;
	DFLOAT	m_fMissileSpeed;
	DFLOAT	m_fWorldTimeSpeed;
	char	m_sWorldNightColor[32];

}	ServerOptions;


// EOF...

#endif
