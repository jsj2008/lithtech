// ----------------------------------------------------------------------- //
//
// MODULE  : GameModeMgr.cpp
//
// PURPOSE : Manager of game rule data.
//
// CREATED : 09/07/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "GameModeMgr.h"
#include "ltprofileutils.h"
#include "NetDefs.h"
#include "WinUtil.h"
#include "ltfileoperations.h"
#include "CTFDB.h"
#include "ControlPointDB.h"
#include "iltfilemgr.h"

#define SERVEROPTIONS_DIR "ServerOptions" FILE_PATH_SEPARATOR

// default multiplayer mode is Deathmatch
#define DEFAULT_MULTIPLAYER_MODE		"Deathmatch"
#define DEFAULT_PER_CLIENT_DOWNLOAD		40960

HATTRIBUTE GameRule::GetStruct() const
{
	return g_pLTDatabase->GetAttribute( GameModeMgr::Instance().GetGameModeRecord(), GetAttribName( ));
}

void GameRule::SetDirty( bool bDirty ) 
{ 
	m_bDirty = bDirty; 
	if( m_bDirty )
		GameModeMgr::Instance().SetBrowserDirty( true );
}

// this is necessary for the Linux build, since the dynamic linker does not
// properly locate the game rule list when it is declared as a global and 
// linked into both a shared library and the main executable.
GameRule::GameRuleList& GameRule::GetGameRuleList()
{
	static GameRuleList lstGameRuleList;
	
	return lstGameRuleList;
}

void GameRuleFloat::ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const 
{ 
	uint32 nPrecision = g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Precision" ), 0, 0 );
	LTSNPrintF( pszString, nStringLen, L"%.*f", nPrecision, m_fValue ); 
}

void GameRuleBool::ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const 
{ 
	if( bLocalized )
	{
		LTStrCpy( pszString, LoadString( g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( 
			GetStruct( ), 0, m_bValue ? "TrueString" : "FalseString" ), 0, "" )), nStringLen );
	}
	else
	{
		LTSNPrintF( pszString, nStringLen, L"%d", m_bValue ? 1 : 0 ); 
	}
}

void GameRuleBool::FromString( wchar_t const* pszString, bool bLocalized ) 
{ 
	if( bLocalized )
	{
		m_bValue = LTStrEquals( pszString, LoadString( g_pLTDatabase->GetString( 
			CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "TrueString" ), 0, "" )));
	}
	else
	{
		m_bValue = ( LTStrToLong( pszString ) != 0 ) ? true : false;
	}
	SetDirty( true );
}

// Gets the index into the list of values given the unlocalized label value.  Returns -1 on error.
uint32 GameRuleEnum::GetRawValueToIndex( char const* pszRawValue )
{
	// Get the struct of values.
	HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Values" );
	if( !hValues )
	{
		return (uint32)-1;
	}

	// Iterate through the values and find the label that matches the input value.
	uint32 nNumValues = g_pLTDatabase->GetNumValues( hValues );
	for( uint32 nIndex = 0; nIndex < nNumValues; nIndex++ )
	{
		char const* pszLabel = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "Label" ), 0, "" );
		if( LTStrEquals( pszRawValue, pszLabel ))
		{
			return nIndex;
		}
	}

	return (uint32)-1;
}

// Gets the unlocalized label value given an index into the values.
char const* GameRuleEnum::GetIndexToRawValue( uint32 nIndex )
{
	// Get the struct of values.
	HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Values" );
	if( !hValues )
	{
		return "";
	}

	// Iterate through the values and find the label that matches the input value.
	return g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "Label" ), 0, "" );
}

// Gets the stringid given an index into the values.
char const* GameRuleEnum::GetIndexToStringId( uint32 nIndex )
{
	// Get the struct of values.
	HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Values" );
	if( !hValues )
	{
		return "";
	}

	// Iterate through the values and find the label that matches the input value.
	return g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "String" ), 0, "" );
}

void GameRuleEnum::ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const 
{ 
	if( bLocalized )
	{
		// Get the struct of values.
		HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Values" );
		if( !hValues )
		{
			LTStrCpy( pszString, L"", nStringLen );
			return;
		}

		// Iterate through the values and find the label that matches our current value.
		uint32 nNumValues = g_pLTDatabase->GetNumValues( hValues );
		for( uint32 nIndex = 0; nIndex < nNumValues; nIndex++ )
		{
			char const* pszLabel = g_pLTDatabase->GetString( 
				CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "Label" ), 0, "" );
			if( LTStrEquals( *this, pszLabel ))
			{
				// Return the localize string.
				HATTRIBUTE hString = CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "String" );
				LTStrCpy( pszString, LoadString( g_pLTDatabase->GetString( hString, 0, "" )), nStringLen );
				break;
			}
		}
	}
	else
	{
		// Return the current value label.
		GameRuleString::ToString( pszString, nStringLen, bLocalized );
	}
}

void GameRuleEnum::FromString( wchar_t const* pszString, bool bLocalized ) 
{ 
	if( bLocalized )
	{
		// Get the struct of values.
		HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Values" );
		if( !hValues )
		{
			return;
		}

		// Iterate through the values and find the localized string that matches the input value.
		uint32 nNumValues = g_pLTDatabase->GetNumValues( hValues );
		for( uint32 nIndex = 0; nIndex < nNumValues; nIndex++ )
		{
			char const* pszStringId = g_pLTDatabase->GetString( 
				CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "String" ), 0, "" );
			if( LTStrEquals( pszString, LoadString( pszStringId )))
			{
				// Match found, set our current value to the value label.
				HATTRIBUTE hLabel = CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "Label" );
				GameRuleString::FromString( MPA2W( g_pLTDatabase->GetString( hLabel, 0, "" )).c_str( ), bLocalized );
				break;
			}
		}
	}
	else
	{
		// Set our value label.
		GameRuleString::FromString( pszString, bLocalized );
	}
}


const wchar_t* const SS_ServerSettings = L"ServerSettings";
	const wchar_t* const SS_nGameType = L"GameType";
	const wchar_t* const SS_wServerMessage = L"ServerMessage";
	const wchar_t* const SS_wBriefingOverrideMessage = L"BriefingOverrideMessage";
	const wchar_t* const SS_bUsePassword = L"UsePassword";
	const wchar_t* const SS_wPassword = L"Password";
	const wchar_t* const SS_wScmdPassword = L"ScmdPassword";
	const wchar_t* const SS_bAllowScmdCommands = L"AllowScmdCommands";
	const wchar_t* const SS_nPort = L"Port";
	const wchar_t* const SS_sBindToAddr = L"BindToAddr";
	const wchar_t* const SS_nBandwidthServer = L"BandwidthServer";
	const wchar_t* const SS_nBandwidthServerCustom = L"BandwidthServerCustom";
	const wchar_t* const SS_bLANOnly = L"LANOnly";
	const wchar_t* const SS_bDedicated = L"Dedicated";
	const wchar_t* const SS_bAllowContentDownload = L"AllowContentDownload";
	const wchar_t* const SS_nMaxDownloadRatePerClient = L"MaxDownloadRatePerClient";
	const wchar_t* const SS_nMaxDownloadRateAllClients = L"MaxDownloadRateAllClients";
	const wchar_t* const SS_nMaxSimultaneousDownloads = L"MaxSimultaneousDownloads";
	const wchar_t* const SS_nMaxDownloadSize = L"MaxDownloadSize";
	const wchar_t* const SS_sRedirectURLs = L"RedirectURLs";
	const wchar_t* const SS_wContentDownloadMessage = L"ContentDownloadMessage";
	const wchar_t* const SS_sWeaponRestrictions = L"WeaponRestrictions";
	const wchar_t* const SS_sAmmoRestrictions = L"AmmoRestrictions";
	const wchar_t* const SS_sGearRestrictions = L"GearRestrictions";
	const wchar_t* const SS_sEnableScoringLog = L"EnableScoringLog";
	const wchar_t* const SS_sMaxScoringLogFileAge = L"MaxScoringLogFileAge"; // days

	const wchar_t* const SS_bAllowVoteKick = L"AllowVoteKick";
	const wchar_t* const SS_bAllowVoteTeamKick = L"AllowVoteTeamKick";
	const wchar_t* const SS_bAllowVoteBan = L"AllowVoteBan";
	const wchar_t* const SS_bAllowVoteNextMap = L"AllowVoteNextMap";
	const wchar_t* const SS_bAllowVoteNextRound = L"AllowVoteNextRound";
	const wchar_t* const SS_bAllowVoteSelectMap = L"AllowVoteSelectMap";
	const wchar_t* const SS_nMinPlayersForVote = L"MinPlayersForVote";
	const wchar_t* const SS_nMinPlayersForTeamVote = L"MinPlayersForTeamVote";
	const wchar_t* const SS_nVoteLifetime = L"VoteLifetime";
	const wchar_t* const SS_nVoteBanDuration = L"VoteBanDuration";
	const wchar_t* const SS_nVoteDelay = L"VoteDelay";
	const wchar_t* const SS_bUsePunkbuster = L"UsePunkBuster";


ServerSettings::ServerSettings( )
{
#if !defined(PLATFORM_LINUX)
	Clear( );
#endif
}

void ServerSettings::Clear( )
{
	m_sGameMode = GameModeMgr::GetSinglePlayerRecordName( );

	m_sServerMessage = L"";
	m_sBriefingOverrideMessage = L"";
	m_sPassword = L"";
	m_bUsePassword = false;
	m_sScmdPassword = L"";
	m_bAllowScmdCommands = false;

	m_bDedicated = false;
	m_bLANOnly = false;
	m_nPort = DEFAULT_PORT;
	m_sBindToAddr = L"";
	m_nBandwidthServer = 3;
	m_nBandwidthServerCustom = g_BandwidthServer[m_nBandwidthServer];
	m_bAllowContentDownload = true;
	m_nMaxDownloadRatePerClient = DEFAULT_PER_CLIENT_DOWNLOAD;
	m_nMaxDownloadRateAllClients = GetMaxPlayersForBandwidth() * m_nMaxDownloadRatePerClient;
	m_nMaxSimultaneousDownloads = GetMaxPlayersForBandwidth();
	m_nMaxDownloadSize = (uint32)-1;
	m_sRedirectURLs.clear();
	m_sContentDownloadMessage = L"";
	m_bEnableScoringLog = false;
	m_nMaxScoringLogFileAge = 0;

	for (uint8 i =0; i < kNumVoteTypes; ++i)
	{
		m_bAllowVote[i] = true;
	}

	m_nMinPlayersForVote = 5;
	m_nMinPlayersForTeamVote = 3;
	m_nVoteLifetime = 30;		//seconds
	m_nVoteBanDuration = 60;	//minutes
	m_nVoteDelay = 0;		//seconds
	m_bUsePunkbuster = false;
}


bool ServerSettings::Load(const char* pszFN )
{
	wchar_t wszValue[256] = L"";
	wchar_t wszDefault[256] = L"";

	if (!pszFN || !pszFN[0]) return false;
	wchar_t wszFile[MAX_PATH*2];
	LTStrCpy( wszFile, MPA2W( pszFN ).c_str( ), LTARRAYSIZE( wszFile ));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nGameType,MPA2W( m_sGameMode.c_str( )).c_str( ),wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_sGameMode = MPW2A( wszValue ).c_str( );

	// Load the servermessage.
	wchar_t wszTemp[GameModeMgr::kMaxUserMessageLen+1] = L"";
	LTProfileUtils::ReadString(SS_ServerSettings,SS_wServerMessage,L"",wszTemp,LTARRAYSIZE(wszTemp),wszFile);
	uint32 nMessageLen = LTStrLen( wszTemp );
	// See if there is any message.
	if( nMessageLen )
	{
		wchar_t wszConverted[GameModeMgr::kMaxUserMessageLen+1] = L"";
		// Convert string in place into std::string.
		GameModeMgr::UserMessageConvertFromProfile( wszTemp, wszConverted, LTARRAYSIZE( wszConverted ));
		m_sServerMessage = wszConverted;
	}
	else
	{
		m_sServerMessage.clear();
	}

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bUsePassword,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bUsePassword	= !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_wPassword,LoadString("IDS_PASSWORD_DEFAULT"),wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_sPassword = wszValue;

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowScmdCommands,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowScmdCommands = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_wScmdPassword,LoadString("IDS_PASSWORD_DEFAULT"),wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_sScmdPassword = wszValue;

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nPort,L"27888",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nPort = (uint16)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_sBindToAddr,L"",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_sBindToAddr = wszValue;

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nBandwidthServer,L"3",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nBandwidthServer = (uint8)LTStrToLong(wszValue);

	LTSNPrintF(wszDefault,LTARRAYSIZE(wszDefault),L"%d",g_BandwidthServer[3]);
	LTProfileUtils::ReadString(SS_ServerSettings,SS_nBandwidthServerCustom,wszDefault,wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nBandwidthServerCustom = (uint16)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bLANOnly,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bLANOnly = !!(LTStrToLong(wszValue));

// Lan games not allowed in demos, since they cannot be turned off.
#ifdef _DEMO
	m_bLANOnly = false;
#endif // _FINAL

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bDedicated,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bDedicated = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowContentDownload,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowContentDownload = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMaxDownloadRatePerClient,L"-1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMaxDownloadRatePerClient = (uint32)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMaxDownloadRateAllClients,L"-1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMaxDownloadRateAllClients = (uint32)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMaxSimultaneousDownloads,L"-1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMaxSimultaneousDownloads = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMaxDownloadSize,L"-1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMaxDownloadSize = (uint32)LTStrToLong(wszValue);

	std::string sDelimitedString = "";
	LTProfileUtils::ReadString(SS_ServerSettings,SS_sRedirectURLs,L"",wszValue,LTARRAYSIZE(wszValue),wszFile);
	sDelimitedString = MPW2A(wszValue).c_str();
	DelimitedStringToStringContainer( sDelimitedString, m_sRedirectURLs, "," );

	LTProfileUtils::ReadString(SS_ServerSettings,SS_wContentDownloadMessage,L"",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_sContentDownloadMessage = wszValue;

	LTProfileUtils::ReadString(SS_ServerSettings,SS_sEnableScoringLog,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bEnableScoringLog = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_sMaxScoringLogFileAge,L"5",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMaxScoringLogFileAge = (uint32)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteKick,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_Kick] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteTeamKick,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_TeamKick] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteBan,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_Ban] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteNextRound,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_NextRound] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteNextMap,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_NextMap] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bAllowVoteSelectMap,L"1",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_bAllowVote[eVote_SelectMap] = !!(LTStrToLong(wszValue));

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMinPlayersForVote,L"5",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMinPlayersForVote = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nMinPlayersForTeamVote,L"3",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nMinPlayersForTeamVote = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nVoteLifetime,L"30",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nVoteLifetime = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nVoteBanDuration,L"60",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nVoteBanDuration = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_nVoteDelay,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	m_nVoteDelay = (uint8)LTStrToLong(wszValue);

	LTProfileUtils::ReadString(SS_ServerSettings,SS_bUsePunkbuster,L"0",wszValue,LTARRAYSIZE(wszValue),wszFile);
	SetUsePunkbuster( !!(LTStrToLong(wszValue)));

	return true;
}

bool ServerSettings::Save(const char* pszFN )
{
	wchar_t wszValue[256] = L"";

	if (!pszFN || !pszFN[0]) return false;
	wchar_t wszFile[MAX_PATH*2];
	LTStrCpy( wszFile, MPA2W( pszFN ).c_str( ), LTARRAYSIZE( wszFile ));

	LTProfileUtils::WriteString(SS_ServerSettings,SS_nGameType, MPA2W( m_sGameMode.c_str()), wszFile);

	// Write the server message.
	wchar_t wszTemp[GameModeMgr::kMaxUserMessageLen+1] = L"";
	if( !m_sServerMessage.empty( ))
	{
		GameModeMgr::UserMessageConvertToProfile( m_sServerMessage.c_str(), wszTemp, LTARRAYSIZE( wszTemp ));
	}
	LTProfileUtils::WriteString(SS_ServerSettings,SS_wServerMessage,wszTemp,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bUsePassword));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bUsePassword,wszValue,wszFile);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_wPassword,m_sPassword.c_str(),wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowScmdCommands));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowScmdCommands,wszValue,wszFile);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_wScmdPassword,m_sScmdPassword.c_str(),wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nPort);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nPort,wszValue,wszFile);

	LTProfileUtils::WriteString(SS_ServerSettings,SS_sBindToAddr,m_sBindToAddr.c_str(),wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nBandwidthServer);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nBandwidthServer,wszValue,wszFile);
	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nBandwidthServerCustom);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nBandwidthServerCustom,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bLANOnly));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bLANOnly,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bDedicated));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bDedicated,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowContentDownload));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowContentDownload,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMaxDownloadRatePerClient);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMaxDownloadRatePerClient,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMaxDownloadRateAllClients);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMaxDownloadRateAllClients,wszValue,wszFile);	

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMaxSimultaneousDownloads);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMaxSimultaneousDownloads,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMaxDownloadSize);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMaxDownloadSize,wszValue,wszFile);

	std::string sDelimitedString = "";
	StringContainerToDelimitedString( m_sRedirectURLs, sDelimitedString, "," );
	LTProfileUtils::WriteString(SS_ServerSettings,SS_sRedirectURLs,MPA2W(sDelimitedString.c_str()).c_str(),wszFile);

	LTProfileUtils::WriteString(SS_ServerSettings,SS_wContentDownloadMessage,m_sContentDownloadMessage.c_str(),wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bEnableScoringLog));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_sEnableScoringLog,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMaxScoringLogFileAge);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_sMaxScoringLogFileAge,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_Kick]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteKick,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_TeamKick]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteTeamKick,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_Ban]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteBan,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_NextRound]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteNextRound,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_NextMap]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteNextMap,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(m_bAllowVote[eVote_SelectMap]));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bAllowVoteSelectMap,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMinPlayersForVote);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMinPlayersForVote,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nMinPlayersForTeamVote);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nMinPlayersForTeamVote,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nVoteLifetime);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nVoteLifetime,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nVoteBanDuration);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nVoteBanDuration,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",m_nVoteDelay);
	LTProfileUtils::WriteString(SS_ServerSettings,SS_nVoteDelay,wszValue,wszFile);

	LTSNPrintF(wszValue,LTARRAYSIZE(wszValue),L"%d",uint8(GetUsePunkbuster( )));
	LTProfileUtils::WriteString(SS_ServerSettings,SS_bUsePunkbuster,wszValue,wszFile);

	return true;
}

uint8 ServerSettings::GetMaxPlayersForBandwidth() const
{
	uint16 nBandwidth = 0;
	if (m_bLANOnly)
	{
		nBandwidth = g_BandwidthServer[eBandwidth_LAN];
	}
	else
	{
		if ( m_nBandwidthServer >= eBandwidth_Custom )
		{
			nBandwidth = m_nBandwidthServerCustom;
		}
		else
		{
			nBandwidth = g_BandwidthServer[m_nBandwidthServer];
		}
	}

	// Find the bandwidth in the table.  Keep upping the max players
	// until our bandwidth can't reach the bandwidth in the table entry.  Which
	// means we'll just use the player count from the previous entry.
	uint8 nMaxPlayers = 1;
	uint8 nIndex = 0;
	for( uint8 nIndex = 0; nIndex < g_BandwidthMaxPlayersSize; nIndex++ )
	{
		nMaxPlayers = g_BandwidthMaxPlayers[nIndex].m_nMaxPlayers;
		if( nBandwidth <= g_BandwidthMaxPlayers[nIndex].m_nBandwidth )
		{
			break;
		}
	}

	// Clamp to the hardcoded max.
	nMaxPlayers = LTMIN( nMaxPlayers, MAX_MULTI_PLAYERS );

	return (nMaxPlayers);
}


uint32 ServerSettings::GetDefaultPerClientDownload() const
{
	return GetMaxPerClientDownload() / 2;
}

//return bps for download based on the current bandwidth
uint32 ServerSettings::GetMaxPerClientDownload() const
{
	uint16 nBandwidth = 0;
	if (m_bLANOnly)
	{
		nBandwidth = g_BandwidthServer[eBandwidth_LAN];
	}
	else
	{
		if ( m_nBandwidthServer >= eBandwidth_Custom )
		{
			nBandwidth = m_nBandwidthServerCustom;
		}
		else
		{
			nBandwidth = g_BandwidthServer[m_nBandwidthServer];
		}
	}

	uint8 nMaxPlayers = GetMaxPlayersForBandwidth();

	//nBandwidth is in kilobits per sec, this return value is in bits per sec
	uint32 nBpS = nBandwidth * 1024;
	
	//no single client should get more than (1/number of players) of the bandwidth or 10kb/sec whichever is lower
	return LTMIN((nBpS / nMaxPlayers), 81920);

}

//return bps for download based on the current bandwidth
uint32 ServerSettings::GetMaxOverallDownload() const
{
	return GetMaxPerClientDownload() * GetMaxPlayersForBandwidth();
}

GameModeMgr& GameModeMgr::Instance()
{
	if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
	{
		static GameModeMgr instance;
		return instance;
	}
	else
	{
		static GameModeMgr instance;
		return instance;
	}
}

GameModeMgr::GameModeMgr( )
{
	m_hGameModeRecord = NULL;
	m_bBrowserDirty = false;

	m_grfRunSpeed.Init( "RunSpeed", 1.0f );
	m_grwsSessionName.Init( "SessionName" );
	m_grwsBriefingStringId.Init( "BriefingStringId" );
	m_grbFriendlyFire.Init( "FriendlyFire", false );
	m_grbWeaponsStay.Init( "WeaponsStay", false );
	m_grbUseLoadout.Init( "UseLoadout", true );
	m_grfTeamReflectDamage.Init( "TeamReflectDamage", 0.0f );
	m_grfTeamDamagePercent.Init( "TeamDamagePercent", 1.0f );
	m_grnScoreLimit.Init( "ScoreLimit", 0 );
	m_grnTimeLimit.Init( "TimeLimit", 0 );
	m_grnSuddenDeathTimeLimit.Init( "SuddenDeathTimeLimit", 0 );
	m_grnMaxWeapons.Init( "MaxWeapons", 2 );
	m_grnNumRounds.Init( "NumRounds", 1 );
	m_grnFragScorePlayer.Init( "FragScorePlayer", 1 );
	m_grnFragScoreTeam.Init( "FragScoreTeam", 1 );
	m_grnDeathScorePlayer.Init( "DeathScorePlayer", 0 );
	m_grnDeathScoreTeam.Init( "DeathScoreTeam", 0 );
	m_grnTKScore.Init( "TeamKillScore", 0 );
	m_grnSuicideScorePlayer.Init( "SuicideScorePlayer", 0 );
	m_grnSuicideScoreTeam.Init( "SuicideScoreTeam", 0 );
	m_grnMaxPlayers.Init( "MaxPlayers", MAX_MULTI_PLAYERS );
	m_grbUseTeams.Init( "UseTeams", true );
	m_grbSwitchTeamsBetweenRounds.Init( "SwitchTeamsBetweenRounds", false );
	m_grnRespawnWaitTime.Init( "RespawnWaitTime", 8 );
	m_grbUseRespawnWaves.Init( "UseRespawnWaves", false );
	m_grnTeamKillRespawnPenalty.Init( "TeamKillRespawnPenalty", 0 );
	m_grbAccumulateRespawnPenalty.Init( "AccumulateRespawnPenalty", false );
	m_grbUseWeaponRestrictions.Init( "UseWeaponRestrictions", false );
	m_greSpawnPointSelect.Init( "SpawnPointSelect", "First" );
	m_grbUsesDifficulty.Init( "UsesDifficulty", true );
	m_grbAllowRespawnFromDeath.Init( "AllowRespawnFromDeath", true );
	m_grbEliminationWin.Init( "EliminationWin", false );
	m_grnJoinGracePeriod.Init( "JoinGracePeriod", 0 );
	m_grsRestrictedWeapons.Init( "RestrictedWeapons" );
	m_grsRestrictedGear.Init( "RestrictedGear" );
	m_grbUseSlowMo.Init( "UseSlowMo", false );
	m_grbSlowMoPersistsAcrossDeath.Init( "SlowMoPersistsAcrossDeath", false );
	m_grbSlowMoRespawnAfterUse.Init( "SlowMoRespawnAfterUse", false );
	m_grbSlowMoNavMarker.Init( "SlowMoNavMarker", false );
	m_grnSlowMoHoldScorePeriod.Init( "SlowMoHoldScorePeriod", 0 );
	m_grnSlowMoHoldScorePlayer.Init( "SlowMoHoldScorePlayer", 0 );
	m_grnSlowMoHoldScoreTeam.Init( "SlowMoHoldScoreTeam", 0 );
	m_grsBroadcastSet.Init( "BroadcastSet" );
	m_grbAllowSpectatorToLiveChatting.Init( "AllowSpectatorToLiveChatting", true );
	m_grbAllowDeadVoting.Init( "AllowDeadVoting", true );

	m_grsCTFRules.Init( "CTFRules" );
	m_grnCTFDefendFlagBaseScorePlayer.Init( "CTFDefendFlagBaseScorePlayer" );
	m_grnCTFDefendFlagBaseScoreTeam.Init( "CTFDefendFlagBaseScoreTeam" );
	m_grnCTFDefendFlagBaseRadius.Init( "CTFDefendFlagBaseRadius" );
	m_grnCTFDefendFlagCarrierScorePlayer.Init( "CTFDefendFlagCarrierScorePlayer" );
	m_grnCTFDefendFlagCarrierScoreTeam.Init( "CTFDefendFlagCarrierScoreTeam" );
	m_grnCTFDefendFlagCarrierRadius.Init( "CTFDefendFlagCarrierRadius" );
	m_grnCTFKillFlagCarrierScorePlayer.Init( "CTFKillFlagCarrierScorePlayer" );
	m_grnCTFKillFlagCarrierScoreTeam.Init( "CTFKillFlagCarrierScoreTeam" );
	m_grnCTFTeamKillFlagCarrierPenalty.Init( "CTFTeamKillFlagCarrierPenalty" );
	m_grnCTFReturnFlagScorePlayer.Init( "CTFReturnFlagScorePlayer" );
	m_grnCTFReturnFlagScoreTeam.Init( "CTFReturnFlagScoreTeam" );
	m_grnCTFStealFlagScorePlayer.Init( "CTFStealFlagScorePlayer" );
	m_grnCTFStealFlagScoreTeam.Init( "CTFStealFlagScoreTeam" );
	m_grnCTFPickupFlagScorePlayer.Init( "CTFPickupFlagScorePlayer" );
	m_grnCTFPickupFlagScoreTeam.Init( "CTFPickupFlagScoreTeam" );
	m_grnCTFCaptureFlagScorePlayer.Init( "CTFCaptureFlagScorePlayer" );
	m_grnCTFCaptureFlagScoreTeam.Init( "CTFCaptureFlagScoreTeam" );
	m_grfCTFCaptureAssistTimeout.Init( "CTFCaptureAssistTimeout" );
	m_grnCTFCaptureAssistScorePlayer.Init( "CTFCaptureAssistScorePlayer" );
	m_grnCTFCaptureAssistScoreTeam.Init( "CTFCaptureAssistScoreTeam" );
	m_grfCTFFlagLooseTimeout.Init( "CTFFlagLooseTimeout" );
	m_grfCTFFlagMovementLimit.Init( "CTFFlagMovementLimit" );
	m_grsRequiredMapFeatures.Init( "RequiredMapFeatures" );

	m_grsCPRules.Init( "CPRules" );
	m_grfCPCapturingTime.Init( "CPCapturingTime" );
	m_grfCPGroupCaptureFactor.Init( "CPGroupCaptureFactor" );
	m_grnCPDefendScorePlayer.Init( "CPDefendScorePlayer" );
	m_grnCPDefendScoreTeam.Init( "CPDefendScoreTeam" );
	m_grnCPDefendRadius.Init( "CPDefendRadius" );
	m_grnCPOwnedScoreAmountTeam.Init( "CPOwnedScoreAmountTeam" );
	m_grfCPOwnedScorePeriod.Init( "CPOwnedScorePeriod" );
	m_grnCPScoreLoseTeam.Init( "CPScoreLoseTeam" );
	m_grnCPScoreNeutralizeTeam.Init( "CPScoreNeutralizeTeam" );
	m_grnCPScoreCaptureTeam.Init( "CPScoreCaptureTeam" );
	m_grnCPScoreNeutralizePlayer.Init( "CPScoreNeutralizePlayer" );
	m_grnCPScoreCapturePlayer.Init( "CPScoreCapturePlayer" );
	m_grbCPConquestWin.Init( "CPConquestWin" );

	m_grsHUDTeamScoreLayout.Init("HUDTeamScoreLayout");

	m_greTeamSizeBalancing.Init( "TeamSizeBalancing", "Never" );
	m_greTeamScoreBalancing.Init( "TeamScoreBalancing", "Never" );
	m_grfTeamScoreBalancingPercent.Init( "TeamScoreBalancingPercent", 2.0f );
	m_grfEndRoundMessageTime.Init( "EndRoundMessageTime", 5.0f );
	m_grfEndRoundScoreScreenTime.Init( "EndRoundScoreScreenTime", 10.0f );

	m_grbAllowKillerTrackSpectating.Init( "AllowKillerTrackSpectating", true );

}

GameModeMgr::~GameModeMgr( )
{
}

bool GameModeMgr::ResetToMode( HRECORD hRecord )
{
	if( !hRecord )
		return false;

	m_hGameModeRecord = hRecord;

	// SP doesn't use serversettings.
	if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( m_hGameModeRecord, Multiplayer ))
	{
		m_ServerSettings.Clear();
	}

	// Make sure the server settings are still set to the correct mode.
	m_ServerSettings.m_sGameMode = g_pLTDatabase->GetRecordName( hRecord );

	// Set all gamerules to the default setting.
	if( !SetRulesToDefault( ))
		return false;

#if !defined(DISABLE_ASSERTS) && !defined(_DEBUG)
	// Mute asserts in mp.
	if( DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( m_hGameModeRecord, Multiplayer ))
	{
		LTAssert::SetDisplayAssert( LTAssert::eDisplay_Never );
	}
#endif // !defined(_FINAL) && !defined(_DEBUG)

	// We are now exactly the default setting, so we're not dirty.
	SetBrowserDirty( false );

	return true;
}

bool GameModeMgr::WriteToMsg( ILTMessage_Write& msg )
{
	// We don't need to send most of the servergameoptions, since those are
	// server specific.

	// Write out the game rules so the client can operate correctly.
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		// Don't need to send the immutable one's since the client can read his own
		// gdb for them.
		if( !pGameRule->IsCanModify( ))
			continue;

		pGameRule->WriteToMsg( msg );
	}

	for (uint8 n = 0; n < kNumVoteTypes; n++)
	{
		msg.Writebool(m_ServerSettings.m_bAllowVote[n]);
	}

	return true;
}

bool GameModeMgr::ReadFromMsg( ILTMessage_Read& msg )
{
	// Read all the game rules.
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		if( !pGameRule->IsCanModify( ))
			continue;

		pGameRule->ReadFromMsg( msg );
	}

	for (uint8 n = 0; n < kNumVoteTypes; n++)
	{
		m_ServerSettings.m_bAllowVote[n] = msg.Readbool();
	}
	return true;
}

bool GameModeMgr::WriteToOptionsFile( char const* pszFileTitle, bool bIncludeServerSettings /* = true */ )
{
	// Get absolute path for passing into profile functions.
	char szAbsoluteFilePath[MAX_PATH*2];
	if( !GetOptionsFilePath( pszFileTitle, szAbsoluteFilePath, LTARRAYSIZE( szAbsoluteFilePath ), kAbsolutePath ))
		return false;
// Allow the win32 platform to create a blank unicode file.  The private profile functions will only create
// an ANSI text file.  We need the file to be Unicode to support wide characters in the fields.
#ifdef PLATFORM_WIN32
	if( !LTFileOperations::FileExists( szAbsoluteFilePath ))
	{
		// Create a file using relative path to user folder.
		char szRelativeFilePath[MAX_PATH*2];
		if( !GetOptionsFilePath( pszFileTitle, szRelativeFilePath, LTARRAYSIZE( szRelativeFilePath ), kRelativePath ))
			return false;
		ILTOutStream* pOutStream = g_pLTBase->FileMgr()->OpenUserFileForWriting( szRelativeFilePath );
		if( !pOutStream )
			return false;
		// Write out the Unicode BOM to make the new blank file Unicode.
		uint8 aUnicodeBOM[2] = { 0xFF, 0xFE };
		pOutStream->Write( aUnicodeBOM, LTARRAYSIZE( aUnicodeBOM ));
		pOutStream->Release();
		pOutStream = NULL;
	}
#endif // PLATFORM_WIN32

	// Save the server specific settings.
	if( bIncludeServerSettings )
		m_ServerSettings.Save( szAbsoluteFilePath );

	// Convert over to wide since we'll need to use them alot in the loop.
	wchar_t wszModeName[256];
	LTStrCpy( wszModeName, MPA2W( m_ServerSettings.m_sGameMode.c_str( )).c_str( ), LTARRAYSIZE( wszModeName ));
	wchar_t wszPath[MAX_PATH*2];
	LTStrCpy( wszPath, MPA2W( szAbsoluteFilePath ).c_str( ), LTARRAYSIZE( wszPath ));

	// Write the briefing message.  This is a special case setting that is actually per game mode.
	// Because of its potential size, it is not going to be using the gamerule mechanism, which is
	// design to send itself to browsing clients.
	wchar_t wszTemp[kMaxUserMessageLen+1] = L"";
	if( !m_ServerSettings.m_sBriefingOverrideMessage.empty( ))
	{
		GameModeMgr::UserMessageConvertToProfile( m_ServerSettings.m_sBriefingOverrideMessage.c_str(), wszTemp, LTARRAYSIZE( wszTemp ));
	}
	LTProfileUtils::WriteString( wszModeName,SS_wBriefingOverrideMessage,wszTemp,wszPath);

	// Write out each of the game rules.
	wchar_t wszValue[256];
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		if( !pGameRule->IsCanModify( ))
			continue;

		pGameRule->ToString( wszValue, LTARRAYSIZE( wszValue ), false );
		LTProfileUtils::WriteString( wszModeName, MPA2W( pGameRule->GetAttribName( )).c_str( ), wszValue, wszPath );
	}

	return true;
}

bool GameModeMgr::SetRulesToDefault( )
{
	// Clear out the briefing message, which is gamemode specific.
	m_ServerSettings.m_sBriefingOverrideMessage.clear( );

	// Set all the rules up to the default.
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		// Set us up with the default value.
		pGameRule->SetToDefault();
	}

	return true;
}

bool GameModeMgr::SetRulesToMultiplayerDefault( )
{
	// get the database record associated with this mode
	HRECORD hDefaultMode = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory(), DEFAULT_MULTIPLAYER_MODE );

	ResetToMode( hDefaultMode );

	return true;
}


bool GameModeMgr::ReadFromOptionsFile( HRECORD hOverrideGameMode, char const* pszFileTitle )
{
	char szFilePath[MAX_PATH*2];
	if( !GetOptionsFilePath( pszFileTitle, szFilePath, LTARRAYSIZE( szFilePath )))
		return false;

	if( !LTFileOperations::FileExists( szFilePath ))
		return false;

	m_ServerSettings.Load( szFilePath );
	m_hGameModeRecord = hOverrideGameMode ? hOverrideGameMode : g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory(), 
		m_ServerSettings.m_sGameMode.c_str( ));
	
	const char* pszGameMode = g_pLTDatabase->GetRecordName( m_hGameModeRecord );
	if (pszGameMode)
	{
		m_ServerSettings.m_sGameMode = pszGameMode;
	}
	else
	{
		m_ServerSettings.m_sGameMode = "";
	}

	SetRulesToDefault( );
	SetBrowserDirty( false );

	wchar_t wszModeName[256];
	LTStrCpy( wszModeName, MPA2W( m_ServerSettings.m_sGameMode.c_str( )).c_str( ), LTARRAYSIZE( wszModeName ));
	wchar_t wszPath[MAX_PATH*2];
	LTStrCpy( wszPath, MPA2W( szFilePath ).c_str( ), LTARRAYSIZE( wszPath ));

	// Load the briefing message.  This is a special case setting that is actually per game mode.
	// Because of its potential size, it is not going to be using the gamerule mechanism, which is
	// design to send itself to browsing clients.
	wchar_t wszTemp[kMaxUserMessageLen+1] = L"";
	LTProfileUtils::ReadString( wszModeName,SS_wBriefingOverrideMessage,L"",wszTemp,LTARRAYSIZE(wszTemp),wszPath);
	uint32 nMessageLen = LTStrLen( wszTemp );
	// See if there is any message.
	if( nMessageLen )
	{
		wchar_t wszConverted[GameModeMgr::kMaxUserMessageLen+1] = L"";
		// Convert string in place into std::string.
		GameModeMgr::UserMessageConvertFromProfile( wszTemp, wszConverted, LTARRAYSIZE( wszConverted ));
		m_ServerSettings.m_sBriefingOverrideMessage = wszConverted;
	}
	else
	{
		m_ServerSettings.m_sBriefingOverrideMessage.clear();
	}

	wchar_t wszValue[256];
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		// Set us up with the default value.
		pGameRule->SetToDefault();

		// If we can't be modified, then just stick with the default.
		if( !pGameRule->IsCanModify( ))
			continue;

		pGameRule->ToString( wszValue, LTARRAYSIZE( wszValue ), false );
		LTProfileUtils::ReadString( wszModeName, MPA2W( pGameRule->GetAttribName( )).c_str(), 
			wszValue, wszValue, LTARRAYSIZE( wszValue ), wszPath );
		pGameRule->FromString( wszValue, false );
	}

#if !defined(DISABLE_ASSERTS) && !defined(_DEBUG)
	// Mute asserts in mp.
	if( DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( m_hGameModeRecord, Multiplayer ))
	{
		LTAssert::SetDisplayAssert( LTAssert::eDisplay_Never );
	}
#endif // !defined(_FINAL) && !defined(_DEBUG)

	return true;
}

// Gets the full path to an options file given just the title.
// e.g. pszFileTitle = "MyOptions", pszFilePath = "serveroptions\MyOptions.txt"
bool GameModeMgr::GetOptionsFilePath( char const* pszFileTitle, char* pszFilePath, uint32 nFilePathSize, ePathType pathType ) const
{
	if( !pszFileTitle || !pszFilePath )
		return false;

	// determine if a directory was provided in the file name
	char szDirectory[MAX_PATH] = { 0 };
	char szFileName[MAX_PATH] = { 0 };
	char szSuffix[MAX_PATH] = { 0 };
	LTFileOperations::SplitPath(pszFileTitle, szDirectory, szFileName, szSuffix);

	if (szDirectory[0])
	{
		// a directory was provided, use this path as specified
		LTStrCpy(pszFilePath, pszFileTitle, nFilePathSize);
	}
	else
	{
		// retrieve the options folder and use that as the path
		LTSNPrintF( pszFilePath, nFilePathSize, "%s%s.txt", GetOptionsFolder( pathType ), pszFileTitle );
	}

	return true;
}

char const* GameModeMgr::GetOptionsFolder( ePathType pathType ) const 
{
	static char szUserDirectory[MAX_PATH*2];

	szUserDirectory[0] = '\0';

	// If they wan't the absolute path, then prepend the user folder path.
	if ( kAbsolutePath == pathType )
	{
		LTFileOperations::GetUserDirectory( szUserDirectory, MAX_PATH );
	}

	LTStrCat( szUserDirectory, SERVEROPTIONS_DIR, LTARRAYSIZE( szUserDirectory ));

	// Make sure the serveroptions folder exists.
	if( !LTStrEmpty( szUserDirectory ))
	{
		LTFileOperations::CreateNewDirectory( szUserDirectory );
	}

	return szUserDirectory; 
}

bool GameModeMgr::GetMissionByIndex( char const* pszOptionsFileTitle, uint32 nIndex, char* pszMission, uint32 nMissionSize )
{
	if( !pszMission )
		return false;

	// Read the mission value out.
	char szFilePath[MAX_PATH*2];
	GameModeMgr::Instance( ).GetOptionsFilePath( pszOptionsFileTitle, szFilePath, LTARRAYSIZE( szFilePath ));
	char szMissionKey[128];
	LTSNPrintF( szMissionKey, LTARRAYSIZE( szMissionKey ), "Mission%d", nIndex );
	LTProfileUtils::ReadString( m_ServerSettings.m_sGameMode.c_str( ), szMissionKey, "", pszMission, nMissionSize, szFilePath );
	return true;
}

// Set number of missions.  This is important so it doesn't leave a bunch of extra missions off the end.
bool GameModeMgr::SetNumMissions( char const* pszOptionsFileTitle, uint32 nNumMissions )
{
	// Write the mission value out.
	char szFilePath[MAX_PATH*2];
	GameModeMgr::Instance( ).GetOptionsFilePath( pszOptionsFileTitle, szFilePath, LTARRAYSIZE( szFilePath ));

	// Erase all the keys after the last mission index.
	char szMissionKey[128];
	char szMission[MAX_PATH*2];
	for( uint32 nIndex = nNumMissions; ; nIndex++ )
	{
		// Check if the key exists.
		LTSNPrintF( szMissionKey, LTARRAYSIZE( szMissionKey ), "Mission%d", nIndex );
		LTProfileUtils::ReadString( m_ServerSettings.m_sGameMode.c_str( ), 
			szMissionKey, "*end*", szMission, LTARRAYSIZE( szMission ), szFilePath );
		// Key doesn't exist, quit erasing.
		if( LTStrEquals( szMission, "*end*" ))
			break;

		// Write out a NULL value which erases the entry.
		LTProfileUtils::WriteString( m_ServerSettings.m_sGameMode.c_str( ), szMissionKey, NULL, szFilePath );
	}

	return true;
}

// Sets a mission by index.
bool GameModeMgr::SetMissionByIndex( char const* pszOptionsFileTitle, uint32 nIndex, char const* pszMission )
{
	if( !pszMission )
		return false;

	// Write the mission value out.
	char szFilePath[MAX_PATH*2];
	GameModeMgr::Instance( ).GetOptionsFilePath( pszOptionsFileTitle, szFilePath, LTARRAYSIZE( szFilePath ));
	char szMissionKey[128];
	LTSNPrintF( szMissionKey, LTARRAYSIZE( szMissionKey ), "Mission%d", nIndex );
	LTProfileUtils::WriteString( m_ServerSettings.m_sGameMode.c_str( ), szMissionKey, pszMission, szFilePath );
	return true;
}

// Get the CTFRules record if specified.
HRECORD GameModeMgr::GetCTFRulesRecord( ) const
{
	if( LTStrEmpty( m_grsCTFRules ))
		return NULL;

	return DATABASE_CATEGORY( CTFRules ).GetRecordByName( m_grsCTFRules );
}

// Get the CPRules record if specified.
HRECORD GameModeMgr::GetCPRulesRecord( ) const
{
	if( LTStrEmpty( m_grsCPRules ))
		return NULL;

	return DATABASE_CATEGORY( CPRules ).GetRecordByName( m_grsCPRules );
}


// Functions to manipulate user messages that have special escape sequences.
// "\n" is converted to/from "carriage return, line feed".
// "\\" is converted to/from "\"
void GameModeMgr::UserMessageConvertFromProfile( wchar_t const* pwszSourceWithEscapeSequences, 
										   wchar_t* pwszDest, uint32 nDestLen )
{
	if( !nDestLen )
		return;

	// Iterate over the source characters.
	uint32 nDestIndex = 0;
	for( uint32 nSrcIndex = 0; ( nDestIndex < ( nDestLen - 1 )) && pwszSourceWithEscapeSequences[nSrcIndex]; 
		nSrcIndex++ )
	{
		switch( pwszSourceWithEscapeSequences[nSrcIndex] )
		{
			// Check if it's the escape sequence.
			case L'\\':
			{
				// Advance to the command character.
				nSrcIndex++;
				if( !pwszSourceWithEscapeSequences[nSrcIndex] )
					break;

				// Handle the different command characters.
				switch( pwszSourceWithEscapeSequences[nSrcIndex] )
				{
					// Handle carriage return.
					case L'n':
					case L'N':
					{
						if( nDestIndex < nDestLen - 1 )
						{
							pwszDest[nDestIndex] = L'\n';
							nDestIndex++;
						}
					}
					break;
					// Handle the backslash.
					case L'\\':
					{
						pwszDest[nDestIndex] = L'\\';
						nDestIndex++;
					}
					break;
					// Everything else is ignored.
					default:
					break;
				}
			}
			break;
			// Handle regular character.
			default:
			{
				pwszDest[nDestIndex] = pwszSourceWithEscapeSequences[nSrcIndex];
				nDestIndex++;
			}
			break;
		}
	}

	// Make sure destination string is zero terminated.
	nDestIndex = LTCLAMP( nDestIndex, 0, nDestLen - 1 );
	pwszDest[nDestIndex] = L'\0';
}

// Functions to manipulate user messages that have special escape sequences.
// "\n" is converted to/from "carriage return, line feed".
// "\\" is converted to/from "\"
void GameModeMgr::UserMessageConvertToProfile( wchar_t const* pwszSource, 
										 wchar_t* pwszDestWithEscapeSequences, uint32 nDestLen )
{
	if( !nDestLen )
		return;

	// Iterate over the source characters.
	uint32 nDestIndex = 0;
	for( uint32 nSrcIndex = 0; ( nDestIndex < ( nDestLen - 1 )) && pwszSource[nSrcIndex]; 
		nSrcIndex++ )
	{
		switch( pwszSource[nSrcIndex] )
		{
			// Check if it's the carriage-return.
			case L'\n':
			{
				// See if we reached the end.
				if( !pwszSource[nSrcIndex] )
					break;

				// See if the dest has room for the whole thing.
				if( nDestIndex >= nDestLen - 2 )
					continue;
				// Write out "\n".
				pwszDestWithEscapeSequences[nDestIndex] = L'\\';
				nDestIndex++;
				pwszDestWithEscapeSequences[nDestIndex] = L'n';
				nDestIndex++;
			}
			break;
			// Check if it's the backslash.
			case L'\\':
			{
				// See if the dest has room for the whole thing.
				if( nDestIndex >= nDestLen - 2 )
					continue;
				// Write out "\\".
				pwszDestWithEscapeSequences[nDestIndex] = L'\\';
				nDestIndex++;
				pwszDestWithEscapeSequences[nDestIndex] = L'\\';
				nDestIndex++;
			}
			break;
			// Handle regular character.
			default:
			{
				// Skip other control characters.
				if( iswcntrl( pwszSource[nSrcIndex] ))
					continue;

				// Copy the source.
				pwszDestWithEscapeSequences[nDestIndex] = pwszSource[nSrcIndex];
				nDestIndex++;
			}
			break;
		}
	}

	// Make sure destination string is zero terminated.
	nDestIndex = LTCLAMP( nDestIndex, 0, nDestLen - 1 );
	pwszDestWithEscapeSequences[nDestIndex] = L'\0';
}
