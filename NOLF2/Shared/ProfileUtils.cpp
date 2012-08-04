#include "stdafx.h"
#include "ProfileUtils.h"
#include "ButeMgr.h"
#include "ltbasedefs.h"
#include "ModelButeMgr.h"
#include "CommonUtilities.h"
#include "ResShared.h"
#ifdef _CLIENTBUILD
#include "ClientUtilities.h"
#include "ClientResShared.h"
#endif //_CLIENTBUILD


#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
#define DEFAULT_SESSION ""

namespace
{

#ifdef _CLIENTBUILD
	int nDefTeamID[2] = {IDS_TEAM1_NAME,IDS_TEAM2_NAME};
#endif
}

ServerGameOptions::ServerGameOptions( )
{
	Clear( );
}

void ServerGameOptions::Clear( )
{
	m_sProfileName = "";

	m_eGameType = eGameTypeCooperative;

	m_sPassword = "";
	m_bUsePassword = false;
	m_sScmdPassword = "";
	m_bAllowScmdCommands = false;

	m_bDedicated = false;
	m_bLANOnly = false;
	m_bPreCacheAssets = true;
	m_bPerformanceTest = false;
	m_nPort = DEFAULT_PORT;
	m_nBandwidthServer = 3;
	m_nBandwidthServerCustom = g_BandwidthServer[m_nBandwidthServer];
	m_setRestrictedWeapons.clear( );
	m_setRestrictedAmmo.clear( );
	m_setRestrictedGear.clear( );
	m_sModName = "";

	m_Coop.Clear();
	m_DM.Clear();
	m_TeamDM.Clear();
	m_DD.Clear();
	
}

static void StringSetToDelimitedString( StringSet const& sSet, std::string& sDelimitedString, char const* pszDelimiter )
{
	sDelimitedString.clear( );

	// Save set to a string delimted by commas.
	std::string sValue = "";
	StringSet::const_iterator iter = sSet.begin( );
	while( iter != sSet.end( ))
	{
		std::string const& sItem = *iter;
		sDelimitedString += sItem;
		sDelimitedString += pszDelimiter;

		iter++;
	}
}

static void DelimitedStringToStringSet( std::string const& sDelimitedString, StringSet& sSet, char const* pszDelimiter )
{
	sSet.clear( );

	// Find all items delimited by commas.
	uint32 nPos = 0;
	while( nPos < sDelimitedString.size( ))
	{
		uint32 nNextPos = sDelimitedString.find( ',', nPos );
		if( nNextPos == std::string::npos )
			nNextPos = sDelimitedString.size( );

		uint32 nNumChars = nNextPos - nPos;
		std::string sItem = sDelimitedString.substr( nPos, nNumChars );

		// Add the item to the set.
		if( !sItem.empty( ))
		{
			sSet.insert( sItem );
		}


		nPos = nNextPos + 1;
	}
}

bool ServerGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Multiplayer";
	std::string sAttName = "";

	char szString[256];
	
	m_eGameType = ( GameType )bute.GetInt( szTagName,"GameType", eGameTypeCooperative );
	m_bUsePassword = bute.GetBool( szTagName, "UsePassword", false );
	bute.GetString( szTagName, "Password", m_sPassword.c_str( ), szString, ARRAY_LEN( szString ));
	m_sPassword = szString;
	bute.GetString( szTagName, "ScmdPassword", m_sScmdPassword.c_str( ), szString, ARRAY_LEN( szString ));
	m_sScmdPassword = szString;
	m_bAllowScmdCommands = bute.GetBool( szTagName, "AllowScmdCommands", false );

	m_nPort = ( uint16 )bute.GetInt( szTagName,"Port", DEFAULT_PORT );

	m_nBandwidthServer = (uint8)bute.GetInt(szTagName,"BandwidthServer",3);
	m_nBandwidthServerCustom = (uint16)bute.GetInt(szTagName,"BandwidthServerCustom",g_BandwidthServer[3]);
	
	m_bLANOnly = bute.GetBool( szTagName, "LANOnly", false );
	m_bDedicated = bute.GetBool( szTagName, "Dedicated", false );

	std::string sDelimitedString = "";

	sDelimitedString = bute.GetString( szTagName, "WeaponRestrictions", "" );
	DelimitedStringToStringSet( sDelimitedString, m_setRestrictedWeapons, "," );
	sDelimitedString = bute.GetString( szTagName, "AmmoRestrictions", "" );
	DelimitedStringToStringSet( sDelimitedString, m_setRestrictedAmmo, "," );
	sDelimitedString = bute.GetString( szTagName, "GearRestrictions", "" );
	DelimitedStringToStringSet( sDelimitedString, m_setRestrictedGear, "," );

	m_Coop.LoadFromBute( bute );
	m_DM.LoadFromBute( bute );
	m_TeamDM.LoadFromBute( bute );
	m_DD.LoadFromBute( bute );

	
	return true;
}

bool ServerGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Multiplayer";

	bute.SetInt(szTagName,"GameType", m_eGameType);

	bute.SetBool(szTagName,"UsePassword",m_bUsePassword);
	bute.SetString(szTagName, "Password",m_sPassword.c_str());

	bute.SetString(szTagName, "ScmdPassword",m_sScmdPassword.c_str());
	bute.SetBool(szTagName,"AllowScmdCommands",m_bAllowScmdCommands);

	bute.SetInt(szTagName,"Port",(int)m_nPort);

	bute.SetInt( szTagName, "BandwidthServer", ( int )m_nBandwidthServer );
	bute.SetInt( szTagName, "BandwidthServerCustom", ( int )m_nBandwidthServerCustom );
	
	bute.SetBool( szTagName, "LANOnly", m_bLANOnly );
	bute.SetBool( szTagName, "Dedicated", m_bDedicated );

	std::string sDelimitedString = "";

	StringSetToDelimitedString( m_setRestrictedWeapons, sDelimitedString, "," );
	bute.SetString( szTagName, "WeaponRestrictions", sDelimitedString.c_str( ));
	StringSetToDelimitedString( m_setRestrictedAmmo, sDelimitedString, "," );
	bute.SetString( szTagName, "AmmoRestrictions", sDelimitedString.c_str( ));
	StringSetToDelimitedString( m_setRestrictedGear, sDelimitedString, "," );
	bute.SetString( szTagName, "GearRestrictions", sDelimitedString.c_str( ));

	m_Coop.SaveToBute( bute );
	m_DM.SaveToBute( bute );
	m_TeamDM.SaveToBute( bute );
	m_DD.SaveToBute( bute );

	return true;
}

ServerGameOptions& ServerGameOptions::Copy( ServerGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sProfileName = other.m_sProfileName;

	m_eGameType = other.m_eGameType;
	m_bUsePassword = other.m_bUsePassword;
	m_sPassword = other.m_sPassword;
	m_sScmdPassword = other.m_sScmdPassword;
	m_bAllowScmdCommands = other.m_bAllowScmdCommands;

	m_bDedicated = other.m_bDedicated;
	m_bLANOnly = other.m_bLANOnly;
	m_bPreCacheAssets = other.m_bPreCacheAssets;
	m_bPerformanceTest = other.m_bPerformanceTest;

	m_nPort = other.m_nPort;
	m_nBandwidthServer = other.m_nBandwidthServer;
	m_nBandwidthServerCustom = other.m_nBandwidthServerCustom;

	m_setRestrictedWeapons = other.m_setRestrictedWeapons;
	m_setRestrictedAmmo = other.m_setRestrictedAmmo;
	m_setRestrictedGear = other.m_setRestrictedGear;

	m_sModName = other.m_sModName;

	m_Coop.Copy(other.m_Coop);
	m_DM.Copy(other.m_DM);
	m_TeamDM.Copy(other.m_TeamDM);
	m_DD.Copy(other.m_DD);

	return *this;
}

bool ServerGameOptions::IsDefaultCampaign() const
{
	return (stricmp(GetCampaignName(), DEFAULT_CAMPAIGN ) == 0);
}

const char*	ServerGameOptions::GetCampaignName() const
{

	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		return m_Coop.m_sCampaignName.c_str();
		break;
	case eGameTypeDeathmatch:
		return m_DM.m_sCampaignName.c_str();
		break;
	case eGameTypeTeamDeathmatch:
		return m_TeamDM.m_sCampaignName.c_str();
		break;
	case eGameTypeDoomsDay:
		return m_DD.m_sCampaignName.c_str();
		break;
		
	};

	return DEFAULT_CAMPAIGN;
}
void ServerGameOptions::SetCampaignName(const char*	pszName)
{
	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		m_Coop.m_sCampaignName = pszName;
		break;
	case eGameTypeDeathmatch:
		m_DM.m_sCampaignName = pszName;
		break;
	case eGameTypeTeamDeathmatch:
		m_TeamDM.m_sCampaignName = pszName;
		break;
	case eGameTypeDoomsDay:
		m_DD.m_sCampaignName = pszName;
		break;
	};
}



const char*	ServerGameOptions::GetSessionName() const
{
	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		return m_Coop.m_sSessionName.c_str();
		break;
	case eGameTypeDeathmatch:
		return m_DM.m_sSessionName.c_str();
		break;
	case eGameTypeTeamDeathmatch:
		return m_TeamDM.m_sSessionName.c_str();
		break;
	case eGameTypeDoomsDay:
		return m_DD.m_sSessionName.c_str();
		break;
		
	};


	return DEFAULT_SESSION;
}
void ServerGameOptions::SetSessionName(const char*	pszName)
{
	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		m_Coop.m_sSessionName = pszName;
		break;
	case eGameTypeDeathmatch:
		m_DM.m_sSessionName = pszName;
		break;
	case eGameTypeTeamDeathmatch:
		m_TeamDM.m_sSessionName = pszName;
		break;
	case eGameTypeDoomsDay:
		m_DD.m_sSessionName = pszName;
		break;
	};
}


uint8 ServerGameOptions::GetMaxPlayers() const
{
	switch (m_eGameType)
	{
		case eGameTypeCooperative:
		{
			return m_Coop.m_nMaxPlayers;
		}
		break;

		case eGameTypeDeathmatch:
		{	
			return m_DM.m_nMaxPlayers;
		}
		break;
		
		case eGameTypeTeamDeathmatch:
		{
			return m_TeamDM.m_nMaxPlayers;
		}
		break;

		case eGameTypeDoomsDay:
		{
			return m_DD.m_nMaxPlayers;
		}
		break;
		
		default:
		{
			return 0;
		}
		break;
	};
}

int ServerGameOptions::GetGameTypeStringID() const
{
	switch (m_eGameType)
	{
		case eGameTypeCooperative:
		{
			return IDS_COOPERATIVE;
		}
		break;

		case eGameTypeDeathmatch:
		{	
			return IDS_DEATHMATCH;
		}
		break;
		
		case eGameTypeTeamDeathmatch:
		{
			return IDS_TEAMDEATHMATCH;
		}
		break;

		case eGameTypeDoomsDay:
		{
			return IDS_DOOMSDAY;
		}
		break;
		
		default:
		{
			return 0;
		}
		break;
	};
}

/************************************************************************************
**** Cooperative game options
************************************************************************************/

CoopGameOptions::CoopGameOptions( )
{
}

void CoopGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 4;
	m_bUseSkills = true;
	m_bFriendlyFire = true;
	m_nDifficulty = 1;
	m_fPlayerDiffFactor = 1.0f;
}

bool CoopGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Cooperative";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 4 );
	m_bUseSkills = bute.GetBool(szTagName,"UseSkills",true);
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);
	m_nDifficulty = (uint8)bute.GetInt(szTagName,"MPDifficulty",1);
	m_fPlayerDiffFactor = bute.GetFloat(szTagName,"MPPlayerDiffFactor",0.1f);

	return true;
}

bool CoopGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Cooperative";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetBool(szTagName,"UseSkills",m_bUseSkills);
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);
	bute.SetInt(szTagName,"MPDifficulty",(int)m_nDifficulty);
	bute.SetFloat(szTagName,"MPPlayerDiffFactor",m_fPlayerDiffFactor);

	return true;
}

CoopGameOptions& CoopGameOptions::Copy( CoopGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_bUseSkills = other.m_bUseSkills;
	m_bFriendlyFire = other.m_bFriendlyFire;
	m_nDifficulty = other.m_nDifficulty;
	m_fPlayerDiffFactor = other.m_fPlayerDiffFactor;

	return *this;
}


/************************************************************************************
**** Deathmatch game options
************************************************************************************/

DMGameOptions::DMGameOptions( )
{
}

void DMGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 8;
	m_nRunSpeed = 130;
	m_nScoreLimit = 25;
	m_nTimeLimit = 10;
	m_nRounds = 1;

	m_nFragScore = 2;
	m_nTagScore = 1;
}


bool DMGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Deathmatch";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nScoreLimit = (uint8)bute.GetInt(szTagName,"ScoreLimit",25);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",10);
	m_nRounds = (uint8)bute.GetInt( szTagName, "Rounds", 0 );

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	return true;
}

bool DMGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Deathmatch";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"ScoreLimit",m_nScoreLimit);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds",m_nRounds);

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	return true;
}

// for transmitting options that may change mid game
void DMGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nScoreLimit);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nRounds );

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

}

// for transmitting options that may change mid game
void DMGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nScoreLimit = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

}


DMGameOptions& DMGameOptions::Copy( DMGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nScoreLimit = other.m_nScoreLimit;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;
	
	return *this;
}



/************************************************************************************
**** TeamDeathmatch game options
************************************************************************************/

TeamDMGameOptions::TeamDMGameOptions( )
{

}

void TeamDMGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 16;
	m_nRunSpeed = 130;
	m_nScoreLimit = 50;
	m_nTimeLimit = 10;
	m_nRounds = 1;
	m_bFriendlyFire = true;

	m_nNumTeams = 2;

	char szTemp[32] = {0};
	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = i;
		sprintf( szTemp, "Team %i", i+1 );
		m_sTeamName[i] = szTemp;
	}

	m_nFragScore = 2;
	m_nTagScore = 1;
	m_nRevivingScore = 1;
}

bool TeamDMGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "TeamDM";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nScoreLimit = (uint8)bute.GetInt(szTagName,"ScoreLimit",100);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",10);
	m_nRounds = (uint8)bute.GetInt( szTagName, "Rounds", 1 );
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);	

	m_nNumTeams = bute.GetInt( szTagName, "NumTeams", 2 );

	char szAttName[32] = {0};
	char szTemp[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		int defModel = i;
		sprintf( szTemp, "Team %i", i+1 );
#ifdef _CLIENTBUILD
		defModel = g_pModelButeMgr->GetTeamDefaultModel(i);
		LoadString(nDefTeamID[i],szTemp,sizeof(szTemp));
#endif

		sprintf( szAttName, "Team%iModel", i );
		m_nTeamModel[i] = bute.GetInt( szTagName, szAttName, defModel );

		sprintf( szAttName, "Team%iName", i );
		bute.GetString( szTagName, szAttName, szTemp, szString, ARRAY_LEN( szString ));
		m_sTeamName[i] = szString;
	}

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	m_nRevivingScore = ( uint8 )bute.GetInt( szTagName, "ReviveScore", 1 );

	return true;
}

bool TeamDMGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "TeamDM";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));


	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"ScoreLimit",m_nScoreLimit);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds", m_nRounds );
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);

	
	bute.SetInt( szTagName, "NumTeams", m_nNumTeams );
	
	char szAttName[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		sprintf( szAttName, "Team%iModel", i );
		bute.SetInt( szTagName, szAttName, m_nTeamModel[i] );

		sprintf( szAttName, "Team%iName", i );
		bute.SetString( szTagName, szAttName, m_sTeamName[i].c_str() );
	}

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	bute.SetInt( szTagName, "ReviveScore", m_nRevivingScore );

	return true;

}


// for transmitting options that may change mid game
void TeamDMGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nScoreLimit);
	pMsg->Writeuint8(m_nRounds );
	pMsg->Writebool(m_bFriendlyFire);

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

}

// for transmitting options that may change mid game
void TeamDMGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nScoreLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();
	m_bFriendlyFire = pMsg->Readbool();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

}


TeamDMGameOptions& TeamDMGameOptions::Copy( TeamDMGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nScoreLimit = other.m_nScoreLimit;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_bFriendlyFire = other.m_bFriendlyFire;

	m_nNumTeams = other.m_nNumTeams;

	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = other.m_nTeamModel[i];
		m_sTeamName[i] = other.m_sTeamName[i];
	}

	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;

	m_nRevivingScore = other.m_nRevivingScore;

	return *this;
}


/************************************************************************************
**** Cooperative game options
************************************************************************************/

DoomsdayGameOptions::DoomsdayGameOptions( )
{
}


void DoomsdayGameOptions::Clear( )
{
	m_sSessionName = "";
	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_nMaxPlayers = 16;
	m_nRunSpeed = 130;
	m_nTimeLimit = 20;
	m_nRounds = 1;
	m_bFriendlyFire = true;

	m_nNumTeams = 2;

	char szTemp[32] = {0};
	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = i;
		sprintf( szTemp, "Team %i", i+1 );
		m_sTeamName[i] = szTemp;
	}

	m_nFragScore = 2;
	m_nTagScore = 1;
	m_nDeviceCompletedScore = 10;
	m_nLightPiecePlacedScore = 3;
	m_nHeavyPiecePlacedScore = 5;
	m_nPieceRemovedScore = 2;
	m_nRevivingScore = 1;
}

bool DoomsdayGameOptions::LoadFromBute( CButeMgr& bute )
{
	char const szTagName[] = "Doomsday";
	std::string sAttName = "";

	char szString[256];
	bute.GetString( szTagName, "SessionName", m_sSessionName.c_str( ), szString, ARRAY_LEN( szString ));
	m_sSessionName = szString;

	bute.GetString( szTagName, "CampaignName", DEFAULT_CAMPAIGN, szString, ARRAY_LEN( szString ));
	m_sCampaignName = szString;

	m_nMaxPlayers = (uint8)bute.GetInt( szTagName, "MaxPlayers", 16 );
	m_nRunSpeed = (uint8)bute.GetInt(szTagName,"RunSpeed",130);
	m_nTimeLimit = (uint8)bute.GetInt(szTagName,"TimeLimit",20);
	m_nRounds = (uint8)bute.GetInt(szTagName, "Rounds", 1 );
	m_bFriendlyFire = bute.GetBool(szTagName,"FriendlyFire",true);	

	m_nNumTeams = bute.GetInt( szTagName, "NumTeams", 2 );

	char szAttName[32] = {0};
	char szTemp[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		int defModel = i;
		sprintf( szTemp, "Team %i", i+1 );
#ifdef _CLIENTBUILD
		defModel = g_pModelButeMgr->GetTeamDefaultModel(i);
		LoadString(nDefTeamID[i],szTemp,sizeof(szTemp));
#endif

		sprintf( szAttName, "Team%iModel", i );
		m_nTeamModel[i] = bute.GetInt( szTagName, szAttName, defModel );

		sprintf( szAttName, "Team%iName", i );
		bute.GetString( szTagName, szAttName, szTemp, szString, ARRAY_LEN( szString ));
		m_sTeamName[i] = szString;
	}

	m_nFragScore = bute.GetInt(szTagName,"FragScore", 2);
	m_nTagScore = bute.GetInt(szTagName,"TagScore", 1);

	m_nDeviceCompletedScore = ( uint8 )bute.GetInt( szTagName, "DeviceCompletedScore", 10 );
	m_nLightPiecePlacedScore = ( uint8 )bute.GetInt( szTagName, "LightPiecePlacedScore", 3 );
	m_nHeavyPiecePlacedScore = ( uint8 )bute.GetInt( szTagName, "HeavyPiecePlacedScore", 5 );
	m_nPieceRemovedScore = ( uint8 )bute.GetInt( szTagName, "PieceRemovedScore", 2 );
	m_nRevivingScore = ( uint8 )bute.GetInt( szTagName, "ReviveScore", 1 );

	return true;
}


// for transmitting options that may change mid game
void DoomsdayGameOptions::Write( ILTMessage_Write *pMsg )
{

	pMsg->Writeuint8(m_nRunSpeed);
	pMsg->Writeuint8(m_nTimeLimit);
	pMsg->Writeuint8(m_nRounds );
	pMsg->Writebool(m_bFriendlyFire);

	pMsg->Writeuint8(m_nFragScore);
	pMsg->Writeuint8(m_nTagScore);

	pMsg->Writeuint8( m_nDeviceCompletedScore );
	pMsg->Writeuint8( m_nLightPiecePlacedScore );
	pMsg->Writeuint8( m_nHeavyPiecePlacedScore );
	pMsg->Writeuint8( m_nPieceRemovedScore );
	pMsg->Writeuint8( m_nRevivingScore );

}

// for transmitting options that may change mid game
void DoomsdayGameOptions::Read( ILTMessage_Read *pMsg )
{
	m_nRunSpeed = pMsg->Readuint8();
	m_nTimeLimit = pMsg->Readuint8();
	m_nRounds = pMsg->Readuint8();
	m_bFriendlyFire = pMsg->Readbool();

	m_nFragScore = pMsg->Readuint8();
	m_nTagScore = pMsg->Readuint8();

	m_nDeviceCompletedScore = pMsg->Readuint8();
	m_nLightPiecePlacedScore = pMsg->Readuint8();
	m_nHeavyPiecePlacedScore = pMsg->Readuint8();
	m_nPieceRemovedScore = pMsg->Readuint8();
	m_nRevivingScore = pMsg->Readuint8();

}

bool DoomsdayGameOptions::SaveToBute( CButeMgr& bute )
{
	char const szTagName[] = "Doomsday";

	bute.SetString( szTagName, "SessionName", m_sSessionName.c_str());
	bute.SetString( szTagName, "CampaignName", m_sCampaignName.c_str( ));

	bute.SetInt(szTagName,"MaxPlayers",(int)m_nMaxPlayers);
	bute.SetInt(szTagName,"RunSpeed",m_nRunSpeed);
	bute.SetInt(szTagName,"TimeLimit",m_nTimeLimit);
	bute.SetInt(szTagName,"Rounds", m_nRounds );
	bute.SetBool(szTagName,"FriendlyFire",m_bFriendlyFire);

	bute.SetInt( szTagName, "NumTeams", m_nNumTeams );
	
	char szAttName[32] = {0};
	for( int i = 0; i < MAX_TEAMS; ++i )
	{
		sprintf( szAttName, "Team%iModel", i );
		bute.SetInt( szTagName, szAttName, m_nTeamModel[i] );

		sprintf( szAttName, "Team%iName", i );
		bute.SetString( szTagName, szAttName, m_sTeamName[i].c_str() );
	}

	bute.SetInt(szTagName,"FragScore",m_nFragScore);
	bute.SetInt(szTagName,"TagScore",m_nTagScore);

	bute.SetInt( szTagName, "DeviceCompletedScore", m_nDeviceCompletedScore );
	bute.SetInt( szTagName, "LightPiecePlacedScore", m_nLightPiecePlacedScore );
	bute.SetInt( szTagName, "HeavyPiecePlacedScore", m_nHeavyPiecePlacedScore );
	bute.SetInt( szTagName, "PieceRemovedScore", m_nPieceRemovedScore );
	bute.SetInt( szTagName, "ReviveScore", m_nRevivingScore );

	return true;

}


DoomsdayGameOptions& DoomsdayGameOptions::Copy( DoomsdayGameOptions const& other )
{
	if( &other == this )
		return *this;

	m_sSessionName = other.m_sSessionName;
	m_sCampaignName = other.m_sCampaignName;

	m_nMaxPlayers = other.m_nMaxPlayers;
	m_nRunSpeed = other.m_nRunSpeed;
	m_nTimeLimit = other.m_nTimeLimit;
	m_nRounds = other.m_nRounds;
	m_bFriendlyFire = other.m_bFriendlyFire;

	m_nNumTeams = other.m_nNumTeams;

	for(int i = 0; i < MAX_TEAMS; ++i )
	{
		m_nTeamModel[i] = other.m_nTeamModel[i];
		m_sTeamName[i] = other.m_sTeamName[i];
	}

	m_nFragScore = other.m_nFragScore;
	m_nTagScore = other.m_nTagScore;

	m_nDeviceCompletedScore = other.m_nDeviceCompletedScore;
	m_nLightPiecePlacedScore = other.m_nLightPiecePlacedScore;
	m_nHeavyPiecePlacedScore = other.m_nHeavyPiecePlacedScore;
	m_nPieceRemovedScore = other.m_nPieceRemovedScore;
	m_nRevivingScore = other.m_nRevivingScore;

	return *this;

}


/***********************************************************************************/

const char*	GetProfileDir( char const* pszProfileName ) 
{
	static std::string directory;
	directory = PROFILE_DIR;
	directory += pszProfileName;
	return directory.c_str();
}

const char*	GetProfileFile( char const* pszProfileName ) 
{
	static std::string sProfileFile;
	sProfileFile = PROFILE_DIR;
	sProfileFile += pszProfileName;
	sProfileFile += PROFILE_EXT;
	return sProfileFile.c_str();
}

static const char szCampaignDirs[g_knNumGameTypes][32] = 
{
	"\\",
	"\\Campaigns\\",
	"\\DM_Campaigns\\",
	"\\TDM_Campaigns\\",
	"\\DD_Campaigns\\",
};


const char*	GetCampaignDir( char const* pszProfileName, GameType eGameType ) 
{
	ASSERT( eGameType >= 0 && eGameType < g_knNumGameTypes );
	
	static std::string sCampaignDir;
	sCampaignDir = GetProfileDir( pszProfileName );
	sCampaignDir += szCampaignDirs[eGameType]; 

	return sCampaignDir.c_str();
}

const char*	GetCampaignFile( ServerGameOptions const& serverGameOptions ) 
{
	static std::string sCampaignFile;

	if (serverGameOptions.m_eGameType == eGameTypeSingle)
		return "";

	if( serverGameOptions.IsDefaultCampaign() )
	{
		sCampaignFile = GetCampaignDir( serverGameOptions.m_sProfileName.c_str( ), serverGameOptions.m_eGameType );
		sCampaignFile += DEFAULT_CAMPAIGN_FILE;
	}
	else
	{
		sCampaignFile = GetCampaignDir( serverGameOptions.m_sProfileName.c_str( ), serverGameOptions.m_eGameType );
		sCampaignFile += serverGameOptions.GetCampaignName(); 
		sCampaignFile += ".txt";
	}

	return sCampaignFile.c_str();
}

