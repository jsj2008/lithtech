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

#define NGE_FRAGS			1				// NGE: net game end
#define NGE_TIME			2
#define NGE_FRAGSANDTIME	3
#define NGE_NEVER			4

#define NGM_STANDARDUPDATE	0				// NGM: net generic message
#define NGM_LEVELCHANGED	1
#define NGM_CONSOLEMSG		2
#define NGM_LEVELCHANGING	3
#define NGM_LEVELCHANGESTOP	4

#define MAX_PLAYER_NAME		32
#define MAX_GAME_LEVELS		50

#define MAX_MULTI_PLAYERS			128
#define MAX_MULTI_PLAYERS_DISPLAY	16


// Structures...

typedef struct NetGame_t
{
	BYTE	m_byType;
	BYTE	m_byEnd;
	DWORD	m_dwEndFrags;
	DWORD	m_dwEndTime;
	BYTE	m_byNumLevels;
	char	m_sLevels[MAX_GAME_LEVELS][NML_LEVEL];

}	NetGame;

typedef struct ServerOptions_t
{
	BOOL	m_bTractorBeam;
	BOOL	m_bDoubleJump;
	BOOL	m_bRammingDamage;
	float	m_fRunSpeed;
	float	m_fMissileSpeed;
	float	m_fRespawnScale;
	float	m_fHealScale;
	float	m_fWorldTimeSpeed;
	char	m_sWorldNightColor[32];

}	ServerOptions;


// EOF...

#endif
