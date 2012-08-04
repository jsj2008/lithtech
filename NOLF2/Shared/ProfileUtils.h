// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileUtils.h
//
// PURPOSE : Profile utilities
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include "ltbasetypes.h"
#include "NetDefs.h"
#include "TeamMgr.h"
#include <string>

class CButeMgr;

#define DEFAULT_CAMPAIGN	".default"
#define DEFAULT_CAMPAIGN_FILE	"default_campaign.txt"
#define PROFILE_DIR "Profiles\\"
#define PROFILE_EXT ".txt"

#define MAX_PROFILE_NAME	16


class CoopGameOptions
{
	public:

		CoopGameOptions( );

		CoopGameOptions& Copy( CoopGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		bool			m_bUseSkills;
		bool			m_bFriendlyFire;
		uint8			m_nDifficulty;
		float			m_fPlayerDiffFactor;

};

class DMGameOptions
{
	public:

		DMGameOptions( );

		DMGameOptions& Copy( DMGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nScoreLimit;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;

		uint8			m_nFragScore;
		uint8			m_nTagScore;
};

class TeamDMGameOptions
{
	public:

		TeamDMGameOptions( );

		TeamDMGameOptions& Copy( TeamDMGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nScoreLimit;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;
		bool			m_bFriendlyFire;

		uint8			m_nNumTeams;	// Will this ever be anything but 2?
		uint32			m_nTeamModel[MAX_TEAMS];
		std::string		m_sTeamName[MAX_TEAMS];

		uint8			m_nFragScore;
		uint8			m_nTagScore;
		uint8			m_nRevivingScore;
};

class DoomsdayGameOptions
{
	public:

		DoomsdayGameOptions( );

		DoomsdayGameOptions& Copy( DoomsdayGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// for transmitting options that may change mid game
		void	Write( ILTMessage_Write *pMsg );
		void	Read( ILTMessage_Read *pMsg );

		std::string		m_sSessionName;
		std::string		m_sCampaignName;

		uint8			m_nMaxPlayers;
		uint8			m_nRunSpeed;
		uint8			m_nTimeLimit;
		uint8			m_nRounds;
		bool			m_bFriendlyFire;

		uint8			m_nNumTeams;	// Will this ever be anything but 2?
		uint32			m_nTeamModel[MAX_TEAMS];
		std::string		m_sTeamName[MAX_TEAMS];

		uint8			m_nFragScore;
		uint8			m_nTagScore;

		uint8			m_nDeviceCompletedScore;
		uint8			m_nLightPiecePlacedScore;
		uint8			m_nHeavyPiecePlacedScore;
		uint8			m_nPieceRemovedScore;

		uint8			m_nRevivingScore;
};


class ServerGameOptions
{
	public:

		ServerGameOptions( );

		ServerGameOptions( ServerGameOptions const& other )
		{
			Copy( other );
		}

		ServerGameOptions& operator=( ServerGameOptions const& other )
		{
			return Copy( other );
		}

		ServerGameOptions& Copy( ServerGameOptions const& other );

		void	Clear( );

		bool	LoadFromBute( CButeMgr& bute );
		bool	SaveToBute( CButeMgr& bute );

		// Saved data: all game types
		GameType		m_eGameType;
		bool			m_bUsePassword;
		std::string		m_sPassword;
		std::string		m_sScmdPassword;
		bool			m_bAllowScmdCommands;
		uint16			m_nPort;
		uint8			m_nBandwidthServer;
		uint16			m_nBandwidthServerCustom;
		bool			m_bLANOnly;
		bool			m_bDedicated;
		StringSet		m_setRestrictedWeapons;
		StringSet		m_setRestrictedAmmo;
		StringSet		m_setRestrictedGear;
		std::string		m_sModName;

		//for the current game type
		bool		IsDefaultCampaign() const;
		const char*	GetCampaignName() const;
		void		SetCampaignName(const char*	pszName);
		const char*	GetSessionName() const;
		void		SetSessionName(const char*	pszName);
		
		uint8		GetMaxPlayers() const;
		int			GetGameTypeStringID() const;

		// Unsaved data.
		std::string		m_sProfileName;
		bool			m_bPreCacheAssets;
		bool			m_bPerformanceTest;

		CoopGameOptions&		GetCoop() { return m_Coop;	}
		DMGameOptions&			GetDeathmatch() { return m_DM; }
		TeamDMGameOptions&		GetTeamDeathmatch() { return m_TeamDM; }
		DoomsdayGameOptions&	GetDoomsday() { return m_DD; }

	private:

		//Saved data: cooperative
		CoopGameOptions		m_Coop;

		//Saved data: deathmatch
		DMGameOptions		m_DM;

		//Saved data:  team deathmatch
		TeamDMGameOptions	m_TeamDM;
		
		//Saved data:  doomsday
		DoomsdayGameOptions	m_DD;
};



const char*	GetProfileDir( char const* pszProfileName );
const char*	GetProfileFile( char const* pszProfileName );
const char*	GetCampaignDir( char const* pszProfileName, GameType eGameType );
const char*	GetCampaignFile( ServerGameOptions const& serverGameOptions );


#endif // PROFILEUTILS_H

