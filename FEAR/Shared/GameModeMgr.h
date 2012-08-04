// ----------------------------------------------------------------------- //
//
// MODULE  : GameModeMgr.h
//
// PURPOSE : Manager of game rule data.
//
// CREATED : 09/07/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef GAMEMODEMGR_H
#define GAMEMODEMGR_H

// ----------------------------------------------------------------------- //
//
//	CLASS:		GameRule
//
//	PURPOSE:	Base class for all GameRules supported by GameModeMgr.
//
// ----------------------------------------------------------------------- //

#include "GameModesDB.h"
#include "sys/win/mpstrconv.h"
#include "StringUtilities.h"
#include "ParsedMsg.h"
#include "ProfileUtils.h"
#include "SharedVoting.h"
#include <algorithm>

#define SERVEROPTIONS_EXAMPLE "ExampleServerOptions"

class GameModeMgr;

class GameRule
{
	public:

		GameRule( ) { m_pszAttribName = NULL; }
		virtual ~GameRule( ) { UnRegister( ); }

		virtual void Init( char const* pszAttribName ) { m_pszAttribName = pszAttribName; m_bDirty = true; Register( ); }

		char const* GetAttribName( ) const { return m_pszAttribName; }
		HATTRIBUTE GetStruct( ) const;

		bool IsDirty( ) const { return m_bDirty; }
		void SetDirty( bool bDirty );

		bool IsCanModify( ) const { return DATABASE_CATEGORY( GameModes ).GetBool( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "CanModify" ), 0, false ); }
		bool IsCanModifyAtRuntime( ) const { return DATABASE_CATEGORY( GameModes ).GetBool( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "CanModifyAtRuntime" ), 0, false ); }
		bool IsShowInOptions( ) const { return DATABASE_CATEGORY( GameModes ).GetBool( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "ShowInOptions" ), 0, false ); }

		typedef std::vector< GameRule* > GameRuleList;
		static GameRuleList& GetGameRuleList( );

	public:

		virtual void SetToDefault( ) = 0;

		virtual void WriteToMsg( ILTMessage_Write& msg ) const = 0;
		virtual void ReadFromMsg( ILTMessage_Read& msg ) = 0;

		virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const = 0;
		virtual void FromString( wchar_t const* pszString, bool bLocalized ) = 0;

	private:

		void Register( )
		{
			GetGameRuleList().push_back( this );
		}

		void UnRegister( )
		{
			GameRuleList& lstGameRuleList = GetGameRuleList();
			GameRuleList::iterator iter = std::find( lstGameRuleList.begin(), lstGameRuleList.end( ), this );
			if( iter != lstGameRuleList.end( ))
				lstGameRuleList.erase( iter );
		}

	private:

		char const* m_pszAttribName;
		bool m_bDirty;
		
};

class GameRuleFloat : public GameRule
{
	public:

		GameRuleFloat( ) { m_fValue = 0.0f; }
		void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); m_fValue = 0.0f; }
		void Init( char const* pszAttribName, float fValue ) { GameRule::Init( pszAttribName ); m_fValue = fValue; }
		GameRuleFloat& operator=( float fValue ) { SetValue( fValue ); return *this; }
		operator float( ) const { return m_fValue; }
		float& GetValue( ) { return m_fValue; }
		float GetDefault( ) const
		{
			return DATABASE_CATEGORY( GameModes ).GetFloat( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, 0.0f );
		}

	public:

		virtual void SetToDefault( ) { SetValue( GetDefault( )); }

		virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.Writefloat( m_fValue ); }
		virtual void ReadFromMsg( ILTMessage_Read& msg ) { m_fValue = msg.Readfloat( ); SetDirty( true ); }

		virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const;
		virtual void FromString( wchar_t const* pszString, bool bLocalized ) { SetValue(( float )LTStrToDouble( pszString )); }

	protected:

		void SetValue( float fValue )
		{
			LTVector2 vRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Range" ), 0, LTVector2( 0.0f, 0.0f ));
			m_fValue = LTCLAMP( fValue, vRange.x, vRange.y );
			SetDirty( true );
		}

	private:

		float m_fValue;
};

class GameRuleUint32 : public GameRule
{
public:

	GameRuleUint32( ) { m_nValue = 0; }
	void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); m_nValue = 0; }
	void Init( char const* pszAttribName, uint32 nValue ) { GameRule::Init( pszAttribName ); m_nValue = nValue; }
	GameRuleUint32& operator=( uint32 nValue ) { SetValue( nValue ); return *this; }
	operator uint32( ) const { return m_nValue; }
	uint32& GetValue( ) { return m_nValue; }
	uint32 GetDefault( ) const
	{
		return ( uint32 )DATABASE_CATEGORY( GameModes ).GetInt32( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, 0 );
	}
	LTVector2 GetRange( ) const
	{
		return g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Range" ), 0, LTVector2( 0.0f, 0.0f ));
	}

public:

	virtual void SetToDefault( ) 
	{ 
		SetValue( GetDefault( )); 
	}

	virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.Writeuint32( m_nValue ); }
	virtual void ReadFromMsg( ILTMessage_Read& msg ) { SetValue( msg.Readuint32( )); }

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const { LTSNPrintF( pszString, nStringLen, L"%d", m_nValue ); }
	virtual void FromString( wchar_t const* pszString, bool bLocalized ) { SetValue( LTStrToLong( pszString )); }

protected:

	void SetValue( uint32 nValue )
	{
		LTVector2 vRange = GetRange( );
		m_nValue = LTCLAMP( nValue, ( uint32 )vRange.x, ( uint32 )vRange.y );
		SetDirty( true );
	}

private:

	uint32 m_nValue;
};

class GameRuleInt32 : public GameRule
{
public:

	GameRuleInt32( ) { m_nValue = 0; }
	void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); m_nValue = 0; }
	void Init( char const* pszAttribName, int32 nValue ) { GameRule::Init( pszAttribName ); m_nValue = nValue; }
	GameRuleInt32& operator=( int32 nValue ) { SetValue( nValue ); return *this; }
	operator int32( ) const { return m_nValue; }
	int32 GetDefault( ) const
	{
		return DATABASE_CATEGORY( GameModes ).GetInt32( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, 0 );
	}

public:

	virtual void SetToDefault( ) { SetValue( GetDefault( )); }

	virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.Writeint32( m_nValue ); }
	virtual void ReadFromMsg( ILTMessage_Read& msg ) { SetValue( msg.Readint32( )); }

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const { LTSNPrintF( pszString, nStringLen, L"%i", m_nValue ); }
	virtual void FromString( wchar_t const* pszString, bool bLocalized ) { SetValue( LTStrToLong( pszString )); }

protected:

	void SetValue( int32 nValue )
	{
		LTVector2 vRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Range" ), 0, LTVector2( 0.0f, 0.0f ));
		m_nValue = LTCLAMP( nValue, ( int32 )vRange.x, ( int32 )vRange.y );
		SetDirty( true );
	}

private:

	int32 m_nValue;
};

class GameRuleString : public GameRule
{
public:

	GameRuleString( ) { }
	void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); }
	void Init( char const* pszAttribName, char const* pszValue ) { GameRule::Init( pszAttribName ); m_sValue = ( pszValue ) ? pszValue : ""; }
	GameRuleString& operator=( char const* pszValue ) { m_sValue = ( pszValue ) ? pszValue : ""; SetDirty( true ); return *this; }
	operator char const*( ) const { return m_sValue.c_str( ); }
	std::string& GetValue( ) { return m_sValue; }
	char const* GetDefault( ) const
	{
		return DATABASE_CATEGORY( GameModes ).GetString( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, "" );
	}

public:

	virtual void SetToDefault( ) { m_sValue = GetDefault( ); }

	virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.WriteString( m_sValue.c_str( )); }
	virtual void ReadFromMsg( ILTMessage_Read& msg ) { uint32 nLen = msg.PeekString( NULL, 0 ); m_sValue.resize( nLen + 1 ); msg.ReadString( &m_sValue[0], nLen + 1 ); }

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const { LTStrCpy( pszString, MPA2W( m_sValue.c_str()).c_str( ), nStringLen ); }
	virtual void FromString( wchar_t const* pszString, bool bLocalized ) { m_sValue = MPW2A( pszString ).c_str( ); SetDirty( true ); }

private:

	std::string m_sValue;
};

// Base class of wstring types.  Do not create this type directly.
class GameRuleWString : public GameRule
{
public:
	GameRuleWString( ) { }
	void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); }
	void Init( char const* pszAttribName, wchar_t const* pwszValue ) { GameRule::Init( pszAttribName ); m_wsValue = ( pwszValue ) ? pwszValue : L""; }
	GameRuleWString& operator=( wchar_t const* pwszValue ) { SetValue(( pwszValue ) ? pwszValue : L"" ); return *this; }
	operator wchar_t const*( ) const { return m_wsValue.c_str( ); }
	std::wstring& GetValue( ) { return m_wsValue; }
	wchar_t const* GetDefault( ) const
	{
		return LoadString( DATABASE_CATEGORY( GameModes ).GetString( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, "" ));
	}

	uint32 GetMaxLength( ) const
	{
		return g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "MaxLength" ), 0, 0 );
	}

public:

	virtual void SetToDefault( ) { m_wsValue = GetDefault( ); }

	virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.WriteWString( m_wsValue.c_str( )); }
	virtual void ReadFromMsg( ILTMessage_Read& msg ) 
	{ 
		uint32 nLen = msg.PeekWString( NULL, 0 ); 
		if (GetMaxLength() > 0)
		{
			nLen = LTMIN( nLen, GetMaxLength( ));
		}		
		m_wsValue.resize( nLen + 1 ); 
		msg.ReadWString( &m_wsValue[0], nLen + 1 ); 
		SetDirty( true );
	}

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const { LTStrCpy( pszString, m_wsValue.c_str(), nStringLen ); }
	virtual void FromString( wchar_t const* pszString, bool bLocalized ) { SetValue( pszString ); }

protected:

	void SetValue( wchar_t const* pszValue )
	{
		uint32 nMaxLength = GetMaxLength( );
		m_wsValue = pszValue;
		if( nMaxLength && m_wsValue.length() > nMaxLength )
			m_wsValue.resize( nMaxLength );
		SetDirty( true );
	}

private:

	std::wstring m_wsValue;
};

class GameRuleBool : public GameRule
{
public:

	GameRuleBool( ) { m_bValue = false; }
	void Init( char const* pszAttribName ) { GameRule::Init( pszAttribName ); m_bValue = false; }
	void Init( char const* pszAttribName, bool const& value ) { GameRule::Init( pszAttribName ); m_bValue = value; }
	GameRuleBool& operator=( bool const& value ) { m_bValue = value; SetDirty( true ); return *this; }
	operator bool( ) const { return m_bValue; }
	bool& GetValue( ) { return m_bValue; }
	bool GetDefault( ) const
	{
		return DATABASE_CATEGORY( GameModes ).GetBool( CGameDatabaseReader::GetStructAttribute( GetStruct( ), 0, "Default" ), 0, false );
	}

public:

	virtual void SetToDefault( ) { m_bValue = GetDefault( ); }

	virtual void WriteToMsg( ILTMessage_Write& msg ) const { msg.Writebool( m_bValue ); }
	virtual void ReadFromMsg( ILTMessage_Read& msg ) { m_bValue = msg.Readbool( ); }

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const;
	virtual void FromString( wchar_t const* pszString, bool bLocalized );

private:

	bool m_bValue;
};

class GameRuleEnum : public GameRuleString
{
public:

	GameRuleEnum( ) { }
	void Init( char const* pszAttribName ) { GameRuleString::Init( pszAttribName ); }
	void Init( char const* pszAttribName, char const* pszValue ) { GameRuleString::Init( pszAttribName, pszValue ); }
	GameRuleEnum& operator=( char const* pszValue ) { GameRuleString::operator=( pszValue ); return *this; }
	operator char const*( ) const { return GameRuleString::operator char const*( ); }

	// Gets the index into the list of values given the unlocalized label value.  Returns -1 on error.
	uint32 GetRawValueToIndex( char const* pszRawValue );

	// Gets the unlocalized label value given an index into the values.
	char const* GetIndexToRawValue( uint32 nIndex );

	// Gets the stringid given an index into the values.
	char const* GetIndexToStringId( uint32 nIndex );

public:

	virtual void ToString( wchar_t* pszString, uint32 nStringLen, bool bLocalized ) const;
	virtual void FromString( wchar_t const* pszString, bool bLocalized );
};


class ServerSettings
{
	public:

		ServerSettings( );

		void	Clear( );

		bool	Load(const char* pszFN );
		bool	Save(const char* pszFN );

		void	SetUsePunkbuster( bool bUsePunkbuster ) { m_bUsePunkbuster = bUsePunkbuster; }
		bool	GetUsePunkbuster( ) const { return m_bUsePunkbuster; }

		// Saved data: all game types
		std::string		m_sGameMode;
		bool			m_bUsePassword;
		std::wstring	m_sServerMessage;
		// Special case server setting that is saved in each gamemode.
		std::wstring	m_sBriefingOverrideMessage;
		std::wstring	m_sPassword;
		std::wstring	m_sScmdPassword;
		bool			m_bAllowScmdCommands;
		uint16			m_nPort;
		std::wstring	m_sBindToAddr;
		uint8			m_nBandwidthServer;
		uint16			m_nBandwidthServerCustom;
		bool			m_bLANOnly;
		bool			m_bDedicated;
		bool		    m_bAllowContentDownload;
		uint32			m_nMaxDownloadRatePerClient;
		uint32			m_nMaxDownloadRateAllClients;
		uint8			m_nMaxSimultaneousDownloads;
		uint32			m_nMaxDownloadSize;
		StringArray		m_sRedirectURLs;
		std::wstring	m_sContentDownloadMessage;
		bool			m_bEnableScoringLog;
		uint8			m_nMaxScoringLogFileAge;

		bool			m_bAllowVote[kNumVoteTypes];
		uint8			m_nMinPlayersForVote;
		uint8			m_nMinPlayersForTeamVote;
		uint8			m_nVoteLifetime;		//seconds
		uint8			m_nVoteBanDuration;		//minutes
		uint8			m_nVoteDelay;		//seconds

		const char*	GetGameTypeStringID() const;
		uint8		GetMaxPlayersForBandwidth() const;

		//return bps for download based on the current bandwidth
		uint32		GetDefaultPerClientDownload() const;
		uint32		GetMaxPerClientDownload()  const;
		uint32		GetMaxOverallDownload()  const;

private:

	bool			m_bUsePunkbuster;

};


class GameModeMgr
{
	private:

		GameModeMgr();
		~GameModeMgr();

	public:

		static GameModeMgr& Instance();

		// Round ending conditions.
		enum EndRoundCondition
		{
			eEndRoundCondition_None,
			eEndRoundCondition_TimeLimit,
			eEndRoundCondition_ScoreLimit,
			eEndRoundCondition_Elimination,
			eEndRoundCondition_Restart,
			eEndRoundCondition_SuddenDeathScore,
			eEndRoundCondition_Conquest,
		};

		enum ePathType
		{
			kRelativePath = false,
			kAbsolutePath = true
		};

		enum eUserMessage
		{
			// Maximum length of a user message for server message and briefing message.
			kMaxUserMessageLen = 1024,
		};

		// Resets settings to gamemode.
		bool ResetToMode( HRECORD hGameModeRecord );

		HRECORD GetGameModeRecord( ) const { return m_hGameModeRecord; }
		char const* GetGameModeName( ) const { return g_pLTDatabase->GetRecordName( GetGameModeRecord( )); }
		static char const* GetSinglePlayerRecordName( ) { return "SinglePlayer"; }

		bool SetRulesToDefault( );
		bool SetRulesToMultiplayerDefault( );
	
		bool WriteToMsg( ILTMessage_Write& msg );
		bool ReadFromMsg( ILTMessage_Read& msg );

		// Gets the folder to where options are kept.
		char const* GetOptionsFolder( ePathType pathType = kAbsolutePath ) const;

		// Gets the full path to an options file given just the title.
		// e.g. pszFileTitle = "MyOptions", pszFilePath = "serveroptions\MyOptions.txt"
		bool GetOptionsFilePath( char const* pszFileTitle, char* pszFilePath, uint32 nFilePathSize, ePathType pathType = kAbsolutePath ) const;

		// Write/Read options to a ini file.  Provide only the title of path.  "MyOptions", not
		// "serveroptions\MyOptions.txt"
		bool WriteToOptionsFile( char const* pszFileTitle, bool bIncludeServerSettings = true );
		bool ReadFromOptionsFile( HRECORD hOverrideGameMode, char const* pszFileTitle );

		bool IsBrowserDirty( ) { return m_bBrowserDirty; }
		void SetBrowserDirty( bool bBrowserDirty ) { m_bBrowserDirty = bBrowserDirty; }

		// Gets a mission by index.  Returns false if one doesn't exist.  The first index
		// to fail indicates the end of the list.
		bool GetMissionByIndex( char const* pszOptionsFileTitle, uint32 nIndex, char* pszMission, uint32 nMissionSize );
		// Set number of missions.  This is important so it doesn't leave a bunch of extra missions off the end.
		bool SetNumMissions( char const* pszOptionsFileTitle, uint32 nNumMissions );
		// Sets a mission by index.
		bool SetMissionByIndex( char const* pszOptionsFileTitle, uint32 nIndex, char const* pszMission );

		// Get the CTFRules record if specified.
		HRECORD GetCTFRulesRecord( ) const;

		// Get the CPRules record if specified.
		HRECORD GetCPRulesRecord( ) const;

		// Functions to manipulate user messages that have special escape sequences.
		// "\n" is converted to/from "carriage return, line feed".
		// "\\" is converted to/from "\"
		static void UserMessageConvertFromProfile( wchar_t const* pwszSourceWithEscapeSequences, 
			wchar_t* pwszDest, uint32 nDestLen );
		static void UserMessageConvertToProfile( wchar_t const* pwszSource, 
			wchar_t* pwszDestWithEscapeSequences, uint32 nDestLen );

		// Game rules.
		GameRuleFloat	m_grfRunSpeed;
		GameRuleWString m_grwsSessionName;
		GameRuleWString	m_grwsBriefingStringId;
		GameRuleBool	m_grbFriendlyFire;
		GameRuleBool	m_grbWeaponsStay;
		GameRuleBool	m_grbUseLoadout;
		GameRuleFloat	m_grfTeamReflectDamage;
		GameRuleFloat	m_grfTeamDamagePercent;
		GameRuleUint32	m_grnScoreLimit;
		GameRuleUint32	m_grnTimeLimit;
		GameRuleUint32	m_grnSuddenDeathTimeLimit;
		GameRuleUint32	m_grnMaxWeapons;
		GameRuleUint32	m_grnNumRounds;
		GameRuleUint32	m_grnFragScorePlayer;
		GameRuleUint32	m_grnFragScoreTeam;
		GameRuleInt32	m_grnDeathScorePlayer;
		GameRuleInt32	m_grnDeathScoreTeam;
		GameRuleInt32	m_grnTKScore;
		GameRuleInt32	m_grnSuicideScorePlayer;
		GameRuleInt32	m_grnSuicideScoreTeam;
		GameRuleUint32	m_grnMaxPlayers;
		GameRuleBool	m_grbUseTeams;
		GameRuleBool	m_grbSwitchTeamsBetweenRounds;
		GameRuleUint32	m_grnRespawnWaitTime;
		GameRuleBool	m_grbUseRespawnWaves;
		GameRuleUint32	m_grnTeamKillRespawnPenalty;
		GameRuleBool	m_grbAccumulateRespawnPenalty;
		GameRuleBool	m_grbUseWeaponRestrictions;
		GameRuleEnum	m_greSpawnPointSelect;
		GameRuleBool	m_grbUsesDifficulty;
		GameRuleBool	m_grbAllowRespawnFromDeath;
		GameRuleBool	m_grbEliminationWin;
		GameRuleUint32  m_grnJoinGracePeriod;
		GameRuleString	m_grsRestrictedWeapons;
		GameRuleString	m_grsRestrictedGear;
		GameRuleBool	m_grbUseSlowMo;
		GameRuleBool	m_grbSlowMoPersistsAcrossDeath;
		GameRuleBool	m_grbSlowMoRespawnAfterUse;
		GameRuleBool	m_grbSlowMoNavMarker;
		GameRuleUint32	m_grnSlowMoHoldScorePeriod;
		GameRuleUint32	m_grnSlowMoHoldScorePlayer;
		GameRuleUint32	m_grnSlowMoHoldScoreTeam;
		GameRuleString	m_grsBroadcastSet;
		GameRuleBool	m_grbAllowSpectatorToLiveChatting;
		GameRuleBool	m_grbAllowDeadVoting;
		GameRuleString	m_grsCTFRules;
		GameRuleUint32	m_grnCTFDefendFlagBaseScorePlayer;
		GameRuleUint32	m_grnCTFDefendFlagBaseScoreTeam;
		GameRuleUint32	m_grnCTFDefendFlagBaseRadius;
		GameRuleUint32	m_grnCTFDefendFlagCarrierScorePlayer;
		GameRuleUint32	m_grnCTFDefendFlagCarrierScoreTeam;
		GameRuleUint32	m_grnCTFDefendFlagCarrierRadius;
		GameRuleUint32	m_grnCTFKillFlagCarrierScorePlayer;
		GameRuleUint32	m_grnCTFKillFlagCarrierScoreTeam;
		GameRuleUint32	m_grnCTFTeamKillFlagCarrierPenalty;
		GameRuleUint32	m_grnCTFReturnFlagScorePlayer;
		GameRuleUint32	m_grnCTFReturnFlagScoreTeam;
		GameRuleUint32	m_grnCTFStealFlagScorePlayer;
		GameRuleUint32	m_grnCTFStealFlagScoreTeam;
		GameRuleUint32	m_grnCTFPickupFlagScorePlayer;
		GameRuleUint32	m_grnCTFPickupFlagScoreTeam;
		GameRuleUint32	m_grnCTFCaptureFlagScorePlayer;
		GameRuleUint32	m_grnCTFCaptureFlagScoreTeam;
		GameRuleFloat	m_grfCTFCaptureAssistTimeout;
		GameRuleUint32	m_grnCTFCaptureAssistScorePlayer;
		GameRuleUint32	m_grnCTFCaptureAssistScoreTeam;
		GameRuleFloat	m_grfCTFFlagLooseTimeout;
		GameRuleFloat	m_grfCTFFlagMovementLimit;
		GameRuleString	m_grsRequiredMapFeatures;
		GameRuleString	m_grsCPRules;
		GameRuleFloat	m_grfCPCapturingTime;
		GameRuleFloat	m_grfCPGroupCaptureFactor;
		GameRuleUint32	m_grnCPDefendScorePlayer;
		GameRuleUint32	m_grnCPDefendScoreTeam;
		GameRuleUint32	m_grnCPDefendRadius;
		GameRuleUint32	m_grnCPOwnedScoreAmountTeam;
		GameRuleFloat	m_grfCPOwnedScorePeriod;
		GameRuleUint32	m_grnCPScoreLoseTeam;
		GameRuleUint32	m_grnCPScoreNeutralizeTeam;
		GameRuleUint32	m_grnCPScoreCaptureTeam;
		GameRuleUint32	m_grnCPScoreNeutralizePlayer;
		GameRuleUint32	m_grnCPScoreCapturePlayer;
		GameRuleBool	m_grbCPConquestWin;
		GameRuleString	m_grsHUDTeamScoreLayout;
		GameRuleEnum	m_greTeamSizeBalancing;
		GameRuleEnum	m_greTeamScoreBalancing;
		GameRuleFloat	m_grfTeamScoreBalancingPercent;
		GameRuleFloat	m_grfEndRoundMessageTime;
		GameRuleFloat	m_grfEndRoundScoreScreenTime;
		GameRuleBool	m_grbAllowKillerTrackSpectating;

#ifdef _CLIENTBUILD
		bool CreateHostOptionCtrls( CBaseScreen& parent, CLTGUIListCtrl& lstCtrl );
#endif // _CLIENTBUILD

		ServerSettings	m_ServerSettings;

	private:

		HRECORD m_hGameModeRecord;
		bool m_bBrowserDirty;

	private:

		PREVENT_OBJECT_COPYING( GameModeMgr );

};

template<typename TContainer>
void StringContainerToDelimitedString( TContainer const& sContainer, std::string& sDelimitedString, char const* pszDelimiter )
{
	sDelimitedString.clear( );

	// Save set to a string delimted by commas.
	typename TContainer::const_iterator iter = sContainer.begin( );

	while( iter != sContainer.end( ))
	{
		std::string const& sItem = *iter;
		sDelimitedString += sItem;
		sDelimitedString += pszDelimiter;

		iter++;
	}
}

template<typename TContainer>
void DelimitedStringToStringContainer( std::string const& sDelimitedString, TContainer& sContainer, char const* pszDelimiter )
{
	if( !pszDelimiter )
		return;

	sContainer.clear( );

	// Find all delimited items.
	uint32 nPos = 0;
	while( nPos < sDelimitedString.size( ))
	{
		uint32 nNextPos = sDelimitedString.find( pszDelimiter[0], nPos );
		if( nNextPos == std::string::npos )
			nNextPos = sDelimitedString.size( );

		uint32 nNumChars = nNextPos - nPos;
		std::string sItem = sDelimitedString.substr( nPos, nNumChars );

		// Add the item to the set.
		if( !sItem.empty( ))
		{
			// NOTE: this will insert properly based on the type of container being used, e.g., vector
			//       will insert on the end(), and list will insert at the appropriate position using 
			//       end() as the starting point.
			sContainer.insert( sContainer.end(), sItem );
		}


		nPos = nNextPos + 1;
	}
}

#endif // GAMEMODEMGR_H
