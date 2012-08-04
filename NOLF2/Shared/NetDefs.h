/****************************************************************************
;
;	 MODULE:		NetDefs (.H)
;
;	PURPOSE:		Network game definitions
;
;	HISTORY:		07/09/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998-2002, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _NETDEFS_H_
#define _NETDEFS_H_

enum GameType
{
	eGameTypeSingle,
	eGameTypeCooperative,
	eGameTypeDeathmatch,
	eGameTypeTeamDeathmatch,
	eGameTypeDoomsDay,
	g_knNumGameTypes
};

enum BandwidthSettings
{
	eBandwidth_56K,
	eBandwidth_DSL_Low,
	eBandwidth_DSL_High,
	eBandwidth_Cable,
	eBandwidth_T1,
	eBandwidth_LAN,
	eBandwidth_Custom,
};
extern const uint16 g_BandwidthServer[eBandwidth_Custom];
extern const uint16 g_BandwidthClient[eBandwidth_Custom];

//approx 10 megabit
#define k_fMaxBandwidth 10240000

const char* GameTypeToString(GameType eType);
GameType GameStringTypeToGameType( char const* pszGameType );

// Defines...

#define DEFAULT_PLAYERNAME		"Player"
#define NET_NAME_LENGTH			30

#define MAX_PLAYER_NAME		16
#define MAX_GAME_LEVELS		50
#define MAX_PASSWORD		16
#define MAX_SESSION_NAME	25

#define MAX_MULTI_PLAYERS			16

#define DEFAULT_PORT		27888

// Structures...

struct NetClientData
{
	char	m_szName[MAX_PLAYER_NAME];
	char	m_szPlayerGuid[64];
	uint8	m_ePlayerModelId;

};

// Messages sent from servershell to the serverapp.
enum ServerApp
{
	SERVERAPP_INIT,
	SERVERAPP_ADDCLIENT,
	SERVERAPP_REMOVECLIENT,
	SERVERAPP_SHELLUPDATE,
	SERVERAPP_PRELOADWORLD,
	SERVERAPP_POSTLOADWORLD,
	SERVERAPP_CONSOLEMESSAGE,
	SERVERAPP_MISSIONFAILED,
};

// Messages sent from serverapp to the servershell.
enum ServerShell
{
	SERVERSHELL_INIT,
	SERVERSHELL_NEXTWORLD,
	SERVERSHELL_SETWORLD,
	SERVERSHELL_MESSAGE,
	SERVERSHELL_MISSIONFAILED,
};


#endif