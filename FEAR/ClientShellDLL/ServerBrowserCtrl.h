// ----------------------------------------------------------------------- //
//
// MODULE  : ServerBrowserCtrl.h
//
// PURPOSE : Control to browse multiplayer servers.
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SERVERBROWSERCTRL_H_
#define _SERVERBROWSERCTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IGameSpy.h"

// PlayerEntry
// Defines information about a player on a server.
struct PlayerEntry
{
	PlayerEntry( )
	{
		m_nPing = 0;
		m_nScore = 0;
	}

	std::string m_sName;
	uint16 m_nPing;
	int32 m_nScore;
};
typedef std::vector< PlayerEntry > PlayerEntryList;

// ServerEntry
// Defines information about a server.
struct ServerEntry
{
	ServerEntry( )
	{
		m_nNumPlayers = 0;
		m_nMaxPlayers = 0;
		m_nPing = 0;
		m_bUsePassword = false;
		m_hGameModeRecord = NULL;
		m_bDirectConnect = true;
		m_bConnectViaPublic = true;
		m_bLan = true;
		m_pColumnCtrl = NULL;
		m_bHasOverrides = false;
		m_nRequiredDownloadSize = 0;
		m_bDedicated = true;
		m_bLinuxServer = false;
		m_bHasDetails = true;
		m_bUsePunkbuster = false;
	}

	std::string m_sPublicAddress;
	std::string m_sPrivateAddress;
	std::string m_sName;
	std::wstring m_sMission;
	std::string m_sVersion;
	std::string m_sModName;
	uint32 m_nNumPlayers;
	uint32 m_nMaxPlayers;
	uint16 m_nPing;
	bool m_bUsePassword;
	bool m_bDedicated;
	bool m_bLinuxServer;
	bool m_bUsePunkbuster;
	HRECORD m_hGameModeRecord;
	std::string m_sOptions;
	PlayerEntryList m_lstPlayerEntry;
	bool m_bDirectConnect;
	bool m_bConnectViaPublic;
	bool m_bLan;
	CLTGUIColumnCtrlEx* m_pColumnCtrl;
	bool m_bHasOverrides;
	std::string m_sDownloadableFiles;
	std::string m_sOverridesData;
	uint32 m_nRequiredDownloadSize;
	bool m_bHasDetails;
};
typedef stdext::hash_map<std::string, ServerEntry, hash_map_stdstring_nocase> TServerEntryMap;


class ServerBrowserCtrl
{

// Types.
public:

	// Sorting criteria.
	enum EColumn
	{
		eColumn_Lock = 0,
		eColumn_Platform,
		eColumn_Punkbuster,
		eColumn_Customized,
		eColumn_RequiresDownload,
		eColumn_Name,
		eColumn_Ping,
		eColumn_Player,
		eColumn_Type,
		eColumn_Mission,

		eColumnCount
	};

// Methods.
public:

	// Reads the server entry from gamespy and fills in the serverentry object.
	static bool ReadServerEntry( IGameSpyBrowser& gameSpyBrowser, IGameSpyBrowser::HGAMESERVER hGameServer, ServerEntry& serverEntry );

	// Sets the summary info on the column control.
	static bool SetSummaryInfo( CLTGUIColumnCtrlEx& columnCtrl, ServerEntry const& serverEntry );

private:

	static uint32 DetermineRequiredDownloadSize(const std::string& strDownloadableFilesString);

};

#endif //_SERVERBROWSERCTRL_H_
