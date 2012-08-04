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

#define MAX_WORLDTIME_COLOR 64

#define NGT_SINGLE			0				// NGT: net game type
#define	NGT_DEATHMATCH		1
#define NGT_CAPTUREFLAG		2
#define NGT_COOPERATIVE		3

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

#define MAX_PLAYER_NAME		32
#define MAX_GAME_LEVELS		50

#define MAX_MULTI_PLAYERS			128
#define MAX_MULTI_PLAYERS_DISPLAY	16


// Structures...

typedef struct NetPlayer_t
{
	char	m_sName[MAX_PLAYER_NAME];
	uint8	m_byColor;
	uint8	m_byMech;
	uint32	m_dwLatency;

}	NetPlayer;

typedef struct NetGame_t
{
	uint8	m_byType;
	uint8	m_byEnd;
	uint32	m_dwEndFrags;
	uint32	m_dwEndTime;
	uint8	m_byNumLevels;
	char	m_sLevels[MAX_GAME_LEVELS][NML_LEVEL];

}	NetGame;

typedef struct ServerOptions_t
{
	LTBOOL	m_bTractorBeam;
	LTBOOL	m_bDoubleJump;
	LTBOOL	m_bRammingDamage;
	LTFLOAT	m_fRunSpeed;
	LTFLOAT	m_fMissileSpeed;
	LTFLOAT	m_fRespawnScale;
	LTFLOAT	m_fHealScale;
	LTFLOAT	m_fWorldTimeSpeed;
	char	m_sWorldNightColor[32];

}	ServerOptions;


// EOF...

#endif
