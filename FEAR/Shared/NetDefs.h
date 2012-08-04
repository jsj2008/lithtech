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

#include "ModelsDB.h"

enum GameType
{
	eGameTypeSingle,
	eGameTypeDeathmatch,
	eGameTypeTeamDeathmatch,
	g_knNumGameTypes
};

enum BandwidthSettings
{
//	eBandwidth_56K,
	eBandwidth_DSL_Low,
	eBandwidth_DSL_High,
	eBandwidth_Cable,
	eBandwidth_T1,
	eBandwidth_LAN,
	eBandwidth_Custom,
};
extern const uint16 g_BandwidthServer[eBandwidth_Custom];

struct BandwidthMaxPlayers
{
	uint16 m_nBandwidth;
	uint8  m_nMaxPlayers;
};
extern const BandwidthMaxPlayers g_BandwidthMaxPlayers[];
extern const uint32 g_BandwidthMaxPlayersSize;

struct BandWidthDefaults
{
	// Used to set bandwidthtargetclient.
	uint32 m_nBandwidthTargetClient;

	// Used to set CSendRate.
	uint16 m_nCSendRate;
};
extern const BandWidthDefaults g_BandwidthClient[eBandwidth_Custom];

//approx 10 megabit
#define k_nMaxBandwidth		10240000

const char* GameTypeToString(GameType eType);
GameType GameStringTypeToGameType( char const* pszGameType );

// Defines...

#define DEFAULT_PLAYERNAME		"Player"
#define NET_NAME_LENGTH			30

#define MAX_PLAYER_NAME		16
#define MAX_GAME_LEVELS		50
#define MAX_PASSWORD		16
#define MAX_SESSION_NAME	25
#define PLAYER_GUID_LENGTH  64

#define MAX_MULTI_PLAYERS			16

#define DEFAULT_PORT		27888

#define INVALID_CLIENT		0xFF

// Structures...

struct NetClientData
{
	wchar_t	m_szName[MAX_PLAYER_NAME];
	LTGUID	m_PlayerGuid;
	uint8	m_nDMModelIndex;
	uint8	m_nTeamModelIndex;
};

// Value passed back by server after initializing.  Only
// way to pass error back to client during startgame.
enum EServerStartResult
{
	eServerStartResult_None,
	eServerStartResult_Success,
	eServerStartResult_NetworkError,
	eServerStartResult_Failed,
};


// Transferred from hosting client to server.
struct NetGameInfo
{
	NetGameInfo( )
	{
		m_bPerformanceTest = false;
		m_eServerStartResult = eServerStartResult_None;
		m_bLinux = false;
		m_bDedicated = false;
	}

	std::string		m_sModName;
	std::string		m_sGameMode;
	std::string		m_sServerOptionsFile;
	std::string		m_sProfileName;
	std::string		m_sMissionOrder;
	std::string		m_sCustomizationsFile;
	bool			m_bPerformanceTest;
	bool			m_bLinux;
	bool			m_bDedicated;

	// Server fills in with start result.
	EServerStartResult m_eServerStartResult;
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
	SERVERAPP_FAILEDTOSTART,
	SERVERAPP_FAILEDTOLOADWORLD,
	SERVERAPP_MULTIPLAYEROPTIONS,
};

// Messages sent from serverapp to the servershell.
enum ServerShell
{
	SERVERSHELL_INIT,
	SERVERSHELL_NEXTWORLD,
	SERVERSHELL_SETWORLD,
	SERVERSHELL_MESSAGE,
	SERVERSHELL_MISSIONFAILED,
	// Tells gameserver about a user message change.
	SERVERSHELL_USERMESSAGE,
};


#endif
