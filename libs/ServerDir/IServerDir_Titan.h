//////////////////////////////////////////////////////////////////////////////
// Titan Server directory interface header

#ifndef __ISERVERDIR_TITAN_H__
#define __ISERVERDIR_TITAN_H__

#include "iltmessage.h"

#include <string>
#include <vector>

/* Titan Server directory interface
	Design goals:
		Specific datastructures needed when writing to Titan service.
*/

struct StartupInfo_Titan
{
	StartupInfo_Titan( ) { }

	StartupInfo_Titan( StartupInfo_Titan const& other )
	{
		Copy( other );
	}

	StartupInfo_Titan& operator=( StartupInfo_Titan const& other )
	{
		return Copy( other );
	}

	StartupInfo_Titan& Copy( StartupInfo_Titan const& other )
	{
		if( this == &other )
			return *this;

		m_sGameSpyName = other.m_sGameSpyName;
		m_sGameSpySecretKey = other.m_sGameSpySecretKey;
		
		return *this;
	}

	std::string		m_sGameSpyName;
	std::string		m_sGameSpySecretKey;
};

struct PeerInfo_Service_Titan
{
	struct Player
	{
		Player( ) { };
		Player( Player const& other )
		{
			Copy( other );
		}
		Player& operator=( Player const& other )
		{
			return Copy( other );
		}
		Player& Copy( Player const& other )
		{
			if( this == &other )
				return *this;

			m_sName = other.m_sName;
			m_nScore = other.m_nScore;
			m_nPing = other.m_nPing;

			return *this;
		}

		std::string		m_sName;
		int				m_nScore;
		uint16			m_nPing;
	};

	PeerInfo_Service_Titan( ) { }

	PeerInfo_Service_Titan( PeerInfo_Service_Titan const& other )
	{
		Copy( other );
	}

	PeerInfo_Service_Titan& operator=( PeerInfo_Service_Titan const& other )
	{
		return Copy( other );
	}

	PeerInfo_Service_Titan& Copy( PeerInfo_Service_Titan const& other )
	{
		if( this == &other )
			return *this;

		m_sHostName = other.m_sHostName;
		m_sCurWorld = other.m_sCurWorld;
		m_nCurNumPlayers = other.m_nCurNumPlayers;
		m_nMaxNumPlayers = other.m_nMaxNumPlayers;
		m_bUsePassword = other.m_bUsePassword;
		m_sGameType = other.m_sGameType;
		m_sServerOptions = other.m_sServerOptions;
		m_nScoreLimit = other.m_nScoreLimit;
		m_nTimeLimit = other.m_nTimeLimit;

		m_PlayerList.clear( );
		PlayerList::const_iterator iter = other.m_PlayerList.begin( );
		while( iter != other.m_PlayerList.end( ))
		{
			m_PlayerList.push_back( *iter );
			iter++;
		}
		
		return *this;
	}

	std::string		m_sHostName;
	std::string		m_sCurWorld;
	uint8			m_nCurNumPlayers;
	uint8			m_nMaxNumPlayers;
	bool			m_bUsePassword;
	std::string		m_sGameType;
	std::string		m_sServerOptions;
	uint32			m_nScoreLimit;
	uint32			m_nTimeLimit;

	typedef std::vector< Player > PlayerList;
	PlayerList		m_PlayerList;
};

#endif //__ISERVERDIR_TITAN_H__