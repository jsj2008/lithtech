// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// CREATED : 9/17/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "CommandIDs.h"
#include "ServerUtilities.h"
#include "Trigger.h"
#include "ObjectMsgs.h"
#include <stdio.h>
#include "WorldProperties.h"
#include "GameBase.h"
#include "Camera.h"
#include "ServerSpecialFX.h"
#include "AI.h"
#include "AIStimulusMgr.h"
#include "AIGoalAbstract.h"
#include "DecisionObject.h"
#include "AINodeMgr.h"
#include "AIUtils.h"
#include "AINavMesh.h"
#include "GlobalServerMgr.h"
#include "ObjectTemplateMgr.h"
#include "AIMgr.h"
#include "BanIPMgr.h"
#include "BanUserMgr.h"
#include "ServerMissionMgr.h"
#include "ServerSaveLoadMgr.h"
#include "CharacterMgr.h"
#include "VersionMgr.h"
#include "PlayerButes.h"
#include "VarTrack.h"
#include "ParsedMsg.h"
#include "ScmdServer.h"
#include "ServerPhysicsCollisionMgr.h"
#include "CommandDB.h"
#include "ServerDB.h"
#include "EngineTimer.h"
#include "lttimeutils.h"
#include "ltfileoperations.h"
#include "sys/win/mpstrconv.h"
#include "ServerConnectionMgr.h"
#include "iltfilemgr.h"
#include "GameModeMgr.h"
#include "ltguid.h"
#include "PlayerNodeGoto.h"
#include <malloc.h>
#include "ltfileread.h"
#include "ObjectTransformHistory.h"
#include "SlowMoDB.h"
#include "AnimationPropStrings.h"
#include "DamageFxDB.h"
#include "LightEditor.h"
#include "CLTFileToILTInStream.h"
#include "ServerVoteMgr.h"
#include "TeamBalancer.h"

#include <time.h>
#include <algorithm>
#if !defined(PLATFORM_XENON)
#include "igamespy.h"
#endif // !PLATFORM_XENON

#include "ipunkbuster.h"

#if !defined(PLATFORM_LINUX)
#include "AssertMgr.h"
#endif // !PLATFORM_LINUX

#define CLIENT_PING_UPDATE_RATE		3.0f
#define UPDATENAME_INTERVAL			10.0f

#define GAME_VERSION				1

// Refresh every 5 seconds
const uint32 k_nRepublishDelay = 5000;

//#define _DEGUG

// debug output messages to sever console
#ifdef _DEBUG				
	enum EDebugOutput {eAlways = 0, eSomeTimes = 1, eRarely = 2};

	// if true all server messages will be output to console
	bool			g_debugOutputMessages(false);
	EDebugOutput	g_debugOutputLevel(eRarely);
		
	#define DEBUG_CONSOLE_OUTPUT(output, level)						\
		if (g_debugOutputMessages && level >= g_debugOutputLevel)	\
			g_pLTServer->CPrint(output)								
#else 
	#define DEBUG_CONSOLE_OUTPUT(output, level)
#endif // _DEBUG


// the ILTServerContentTransfer interface
#include "iltservercontenttransfer.h"
static ILTServerContentTransfer* g_pLTServerContentTransfer;
define_holder(ILTServerContentTransfer, g_pLTServerContentTransfer);

#if !defined(PLATFORM_XENON)
#include "iltgameutil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

// Stuff to create a GameServerShell.

//SETUP_SERVERSHELL()

VarTrack			g_ShowTimingTrack;
VarTrack			g_ShowMessagesTrack;
VarTrack			g_ShowMessagesFilter;
VarTrack			g_ShowNodesTrack;
VarTrack			g_ShowNavMeshTrack;
VarTrack			g_ShowAIRegionsTrack;
VarTrack			g_ShowAIPathTrack;
VarTrack			g_ClearLinesTrack;
VarTrack			g_DamageScale;
VarTrack			g_HealScale;
VarTrack			g_StairStepHeight;

VarTrack			g_vtAIHitPtsPlayerIncreasePercent;
VarTrack			g_vtAIArmorPtsPlayerIncreasePercent;

VarTrack			g_vtPhysicsSettleTime;
VarTrack			g_vtPhysicsSettleRate;

CGameServerShell*   g_pGameServerShell = NULL;

extern VarTrack g_vtFlickerDisableMessages;

static void BuildSendInstantDamageTypeList( CGameServerShell::InstantDamageTypes& lstInstantDamageTypes );
static void BuildSendDeathDamageTypeList( CGameServerShell::DeathDamageTypes& lstDeathDamageTypes );

LTRESULT CGameServerShell::OnServerInitialized()
{
	g_pGameServerShell = this;

	//////////////////////////////////////////////////////////////////////////////
	// Set up all the stuff that everything relies on

	// check to see if we need to change the default user directory 
	HCONSOLEVAR hUserDirectoryVar = g_pLTServer->GetConsoleVariable("UserDirectory");
	const char* pszUserDirectory = g_pLTServer->GetConsoleVariableString(hUserDirectoryVar);
	if (pszUserDirectory)
	{
		LTFileOperations::SetUserDirectory(pszUserDirectory);
	}

	// Redirect our asserts as specified by the assert convar

#if !defined(PLATFORM_LINUX)
	CAssertMgr::Enable();
#endif // !PLATFORM_LINUX

	// Set up the base global interfaces if we're not in SEM mode
#if !defined(PLATFORM_SEM)
	g_pModelLT = g_pLTServer->GetModelLT();
	g_pPhysicsLT = g_pLTServer->Physics();
	g_pLTBase = static_cast<ILTCSBase*>(g_pLTServer);
	g_pCommonLT = g_pLTServer->Common();
#endif // PLATFORM_SEM

	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	g_pVersionMgr->Init( );

	// Set up global console trackers...

	g_ShowTimingTrack.Init(g_pLTServer, "ShowTiming", "0", 0.0f);
	g_ShowMessagesTrack.Init(g_pLTServer, "ShowMessages", "0", 0.0f);
	g_ShowMessagesFilter.Init(g_pLTServer, "ShowMessagesFilter", "", 0.0f);
	g_ShowNodesTrack.Init(g_pLTServer, "ShowAINodes", "0", 0.0f);
	g_ShowNavMeshTrack.Init(g_pLTServer, "ShowAINavMesh", "0", 0.0f);
	g_ShowAIRegionsTrack.Init(g_pLTServer, "ShowAIRegions", "0", 0.0f);
	g_ShowAIPathTrack.Init(g_pLTServer, "ShowAIPath", "0", 0.0f);
	g_ClearLinesTrack.Init(g_pLTServer, "ClearLines", NULL, 0.0f);
	g_DamageScale.Init(g_pLTServer, "DamageScale", NULL, 1.0f);
	g_HealScale.Init(g_pLTServer, "HealScale", NULL, 1.0f);
	g_vtAIHitPtsPlayerIncreasePercent.Init( g_pLTServer, "AIHitPointsPlayerIncreasePercent", NULL, 0.25f );
	g_vtAIArmorPtsPlayerIncreasePercent.Init( g_pLTServer, "AIArmorPointsPlayerIncreasePercent", NULL, 0.25f );
	g_StairStepHeight.Init(g_pLTServer, STAIR_STEP_HEIGHT_CVAR, NULL, DEFAULT_STAIRSTEP_HEIGHT);
	g_vtMuteAIAssertsVar.Init(g_pLTServer, "MuteAIAsserts", NULL, 0.0f);

	// The amount of time to spend settling physics at the beginning of the level.
	g_vtPhysicsSettleTime.Init( g_pLTServer, "PhysicsSettleTime", NULL, 2.0f );
	// The frame rate of the physics settling time.  This determines how fine a timeslice to give.
	g_vtPhysicsSettleRate.Init( g_pLTServer, "PhysicsSettleRate", NULL, 1.0f / 30.0f );

	// Make sure we are using autodeactivation...

	g_pLTServer->SetConsoleVariableFloat("autodeactivate", 1.0f);

	g_pPhysicsLT->SetStairHeight( g_StairStepHeight.GetFloat() );


	//////////////////////////////////////////////////////////////////////////////
	// Do all the stuff that used to be in the ctor for the server

	SetUpdateGameServ();

	NetGameInfo** ppNetGameInfo = NULL;
    uint32 dwLen = sizeof( NetGameInfo* );
    g_pLTServer->GetGameInfo((void**)&ppNetGameInfo, &dwLen);
	if( !ppNetGameInfo || !( *ppNetGameInfo ))
		return LT_ERROR;

	NetGameInfo* pNetGameInfo = *ppNetGameInfo;

	if( !InitGameType( *pNetGameInfo ))
		return LT_ERROR;

	// Default to failing to initialize.
	pNetGameInfo->m_eServerStartResult = eServerStartResult_Failed;

		
	// Create the AI class factory.
	if ( 0 == m_pAIClassFactory )
	{
		m_pAIClassFactory = debug_new( CAIClassFactory );
		ASSERT( 0 != m_pAIClassFactory );
	}

	SetLGFlags( LOAD_NEW_GAME );
	m_bFirstUpdate = true;

	m_ClientPingSendCounter = 0.0f;

	m_sWorld.clear( );

	// Initialize all the globals...
	if ( m_pGlobalMgr == 0 )
	{
		m_pGlobalMgr = debug_new( CGlobalServerMgr );
		ASSERT( 0 != m_pGlobalMgr );
	}
	m_pGlobalMgr->Init();

	// if this is a multiplayer game server check for and load overrides, and initialize
	// the scoring log if necessary.  We can't call IsMultiplayerGame here because GameModeMgr isn't setup yet.
	if( !LTStrIEquals( pNetGameInfo->m_sGameMode.c_str( ), GameModeMgr::GetSinglePlayerRecordName( )))
	{
		// attempt to open the overrides file
		std::string strCustomizationsFile;

		if ((*ppNetGameInfo)->m_sCustomizationsFile.empty())
		{
			// use the default file name
			strCustomizationsFile = g_pszOverridesFilename;
		}
		else
		{
			// use the user specified name
			strCustomizationsFile = (*ppNetGameInfo)->m_sCustomizationsFile;
		}

		// if a path was specified then load the file directly from the filename string, otherwise 
		// load it from the user directory
		ILTInStream* pOverridesStream = NULL;

		char szDirectory[MAX_PATH] = { 0 };
		LTFileOperations::SplitPath(strCustomizationsFile.c_str(), szDirectory, NULL, NULL);
		if (szDirectory[0])
		{
			// make sure the file exists
			if (LTFileOperations::FileExists(strCustomizationsFile.c_str()))
			{
				// create a CLTFileToLTInStream wrapper and open the file
				CLTFileToILTInStream* pOverridesFile = NULL;
				LT_MEM_TRACK_ALLOC(pOverridesFile = new CLTFileToILTInStream(), LT_MEM_TYPE_GAMECODE);
				if (!pOverridesFile->Open(strCustomizationsFile.c_str()))
				{
					delete pOverridesFile;
				}
				else
				{
					pOverridesStream = pOverridesFile;
				}
			}
		}
		else
		{
			pOverridesStream = g_pLTBase->FileMgr()->OpenUserFileForReading(strCustomizationsFile.c_str());
		}

		if (pOverridesStream)
		{
			// file exists so load the overrides
			bool bResult = LoadMultiplayerOverrides(*pOverridesStream);
			pOverridesStream->Release();

			if (!bResult)
			{
				return LT_ERROR;
			}
		}

		// Set rules to default mp.
		GameModeMgr::Instance( ).SetRulesToMultiplayerDefault( );
	}

	// Initialize the gamemode.
	if( pNetGameInfo->m_sServerOptionsFile.empty( ))
	{
		// If no options file, then just reset to the default settings for the game mode specified.
		HRECORD hGameModeRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory(), 
			pNetGameInfo->m_sGameMode.c_str( ));
		if( !GameModeMgr::Instance( ).ResetToMode( hGameModeRecord ))
		{
			LTERROR( "Invalid game mode" );
			return LT_ERROR;
		}
	}
	else
	{
		// Initialize from the option file.
		if( !GameModeMgr::Instance( ).ReadFromOptionsFile( NULL, pNetGameInfo->m_sServerOptionsFile.c_str( )))
		{
			 LTERROR( "Invalid game mode" );
			 return LT_ERROR;
		}
	}


	m_pServerMissionMgr = debug_new( CServerMissionMgr );
	if( !m_pServerMissionMgr || !m_pServerMissionMgr->Init( ))
	{
		LTERROR( "Could not initialize ServerMissionMgr." );
		return LT_ERROR;
	}

	ServerVoteMgr::Instance().Init();
	TeamBalancer::Instance().Init();

	if (IsMultiplayerGameServer())
	{
		// prepare the file archives for automatic content download - if we're allowing downloads
		// from the server then we need to compress them
		bool bCompressFileArchives = GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload;

		if (g_pLTServerContentTransfer->PrepareFileArchives(bCompressFileArchives) != LT_OK)
		{
			return LT_ERROR;
		}

		// enable scoring log if necessary
		if (GameModeMgr::Instance().m_ServerSettings.m_bEnableScoringLog)
		{
			if (!InitializeMultiplayerScoringLog())
			{
				return LT_ERROR;
			}
		}
	}


	// Create AI systems.

	if ( m_pAIMgr == 0 )
	{
		m_pAIMgr = debug_new( CAIMgr );
		ASSERT( 0 != m_pAIMgr );
	}
	m_pAIMgr->InitAI();

	if ( m_pCharacterMgr == 0 )
	{
		m_pCharacterMgr = debug_new( CCharacterMgr );
		ASSERT( 0 != m_pCharacterMgr );
	}

	if( !m_pServerSaveLoadMgr )
	{
		m_pServerSaveLoadMgr = debug_new( CServerSaveLoadMgr );
		if( !m_pServerSaveLoadMgr )
		{
			return LT_ERROR;
		}
	}
	if( !m_pServerSaveLoadMgr->Init( m_NetGameInfo.m_sProfileName.c_str( ), IsMultiplayerGameServer( )))
	{
		return LT_ERROR;
	}

	if( !ServerPhysicsCollisionMgr::Instance().Init( ))
	{
		return LT_ERROR;
	}

	m_nPauseCount = 0;
	m_bClientPaused = false;

	m_eSwitchingWorldsState = eSwitchingWorldsStateNone;

	// Tracks time we stay in slowmo.
	m_SlowMoTimer.SetEngineTimer( GameTimeTimer::Instance( ));
	m_hSlowMoRecord = NULL;

	// Use max on LAN games
	uint32 nMinBandwidth = ( GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly ) ? k_nMaxBandwidth : 0;

	if( GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServer < eBandwidth_Custom)
	{
		WriteConsoleFloat("BandwidthTargetServer",
			(float)LTCLAMP((uint32)g_BandwidthServer[GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServer] * 1024, nMinBandwidth, k_nMaxBandwidth) );
	}
	else
	{
		WriteConsoleFloat("BandwidthTargetServer",
			(float)LTCLAMP((uint32)GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServerCustom * 1024, nMinBandwidth, k_nMaxBandwidth) );
	}

	// Initialize the scmd handler.
	if( !ScmdServer::Instance( ).Init( ))
		return LT_ERROR;

	// Initialize the BanIPMgr.
	if( !BanIPMgr::Instance( ).Init( ))
		return LT_ERROR;

	// initialize BanUserMgr
	BanUserMgr::Instance().Init();

	// Do any game specific initialization.
	if( GameModeMgr::Instance( ).m_grbUseWeaponRestrictions )
	{
		ApplyWeaponRestrictions( );
	}

	g_pPhysicsLT->SetStairHeight( g_StairStepHeight.GetFloat() );


	// Seed the random number generator so GetRandom() isn't the same each game.
	// IMPORTANT: reseeding the random number generator should not happen all the time
	// as it can lead to GetRandom() not acting as random as you might think.
	if( pNetGameInfo->m_bPerformanceTest )
	{
		srand(123); // Same as client, doesn't really matter...
	}
	else
	{
		srand(time(NULL));
	}

	// Build the list of instant damage types we need to send to the client when characters take that type of damage.
	BuildSendInstantDamageTypeList( m_lstSendInstantDamageTypes );

	// Build the list of damage types we need to send to the client when characters die from that type of damage.
	BuildSendDeathDamageTypeList( m_lstSendDeathDamageTypes );

	// Don't do anything special if we're playing single-player
	if (!IsMultiplayerGameServer( ))
	{
		// Say we were successful.
		pNetGameInfo->m_eServerStartResult = eServerStartResult_Success;
		return LT_OK;
	}

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// get our ip address and port to give to gamespy.
	char szIpAddr[16];
	uint16 nPort = 0;
	if( LT_OK != g_pLTServer->GetTcpIpAddress( szIpAddr, ARRAY_LEN( szIpAddr ), nPort ))
	{
		pNetGameInfo->m_eServerStartResult = eServerStartResult_NetworkError;
		return LT_ERROR;
	}
	SOCKET socket;
	if( LT_OK != g_pLTServer->GetUDPSocket( socket ))
	{
		pNetGameInfo->m_eServerStartResult = eServerStartResult_NetworkError;
		return LT_ERROR;
	}

	// Create a gamespy server.
	IGameSpyServer::StartupInfo startupInfo;
	startupInfo.m_bPublic = !GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly;

// Lan games not allowed in demos, since they cannot be turned off.
#ifdef _DEMO
	startupInfo.m_bPublic = true;
#endif // _FINAL

	startupInfo.m_nPort = nPort;
	startupInfo.m_sIpAddress = szIpAddr;
	startupInfo.m_Socket = socket;
	startupInfo.m_sVersion = g_pVersionMgr->GetNetVersion();
	m_pGameSpyServer = g_pLTGameUtil->CreateGameSpyServer( startupInfo );
	if( !m_pGameSpyServer )
	{	
		ASSERT( !"Couldn't create gamespy server." );
		pNetGameInfo->m_eServerStartResult = eServerStartResult_NetworkError;
		return LT_ERROR;
	}

	// Register some reserved keys.
	m_pGameSpyServer->RegisterKey( "hostname" );
	m_pGameSpyServer->RegisterKey( "mapname" );
	m_pGameSpyServer->RegisterKey( "mappath" );
	m_pGameSpyServer->RegisterKey( "numplayers" );
	m_pGameSpyServer->RegisterKey( "maxplayers" );
	m_pGameSpyServer->RegisterKey( "gametype" );
	m_pGameSpyServer->RegisterKey( "gamemode" );
	m_pGameSpyServer->RegisterKey( "password" );
	m_pGameSpyServer->RegisterKey( "fraglimit" );
	m_pGameSpyServer->RegisterKey( "timelimit" );
	m_pGameSpyServer->RegisterKey( "player_" );
	m_pGameSpyServer->RegisterKey( "score_" );
	m_pGameSpyServer->RegisterKey( "ping_" );
	m_pGameSpyServer->RegisterKey( "gamevariant" );
	m_pGameSpyServer->RegisterKey( "options" );
	m_pGameSpyServer->RegisterKey( "hasoverrides" );
	m_pGameSpyServer->RegisterKey( "downloadablefiles" );
	m_pGameSpyServer->RegisterKey( "overridesdata" );
	m_pGameSpyServer->RegisterKey( "dedicated" );
	m_pGameSpyServer->RegisterKey( "linux" );
	m_pGameSpyServer->RegisterKey( "punkbuster" );

	// Initialize some of the properties.
	char szString[256];
	m_pGameSpyServer->SetServerProperty( "hostname", MPW2A( GameModeMgr::Instance( ).m_grwsSessionName ).c_str( ));
	m_pGameSpyServer->SetServerProperty( "gametype", g_pLTDatabase->GetRecordName( GameModeMgr::Instance().GetGameModeRecord()));
	m_pGameSpyServer->SetServerProperty( "gamemode", "openplaying" );
	LTSNPrintF( szString, LTARRAYSIZE( szString ), "%d", ( uint32 )GameModeMgr::Instance( ).m_grnMaxPlayers );
	m_pGameSpyServer->SetServerProperty( "maxplayers", szString );
	m_pGameSpyServer->SetServerProperty( "password", GameModeMgr::Instance( ).m_ServerSettings.m_bUsePassword ? "1" : "0" );
	ModNameToGameVariant( m_NetGameInfo.m_sModName.c_str( ), szString, LTARRAYSIZE( szString ));
	m_pGameSpyServer->SetServerProperty( "gamevariant", szString );
	m_pGameSpyServer->SetServerProperty( "hasoverrides", m_hOverridesDatabase ? "1" : "0" );
	m_pGameSpyServer->SetServerProperty( "dedicated", m_NetGameInfo.m_bDedicated ? "1" : "0" );
	m_pGameSpyServer->SetServerProperty( "linux", m_NetGameInfo.m_bLinux ? "1" : "0" );

	// build list of downloadable archive files
	std::string strDownloadableArchiveFiles;
	if (!BuildDownloadableArchiveFilesString(strDownloadableArchiveFiles))
	{
		return LT_ERROR;
	}

	m_pGameSpyServer->SetServerProperty( "downloadablefiles", strDownloadableArchiveFiles.c_str());
	m_pGameSpyServer->SetServerProperty( "overridesdata", m_pszServerOverrides);
	m_pGameSpyServer->SetNumPlayers( 0 );
	m_pGameSpyServer->SetNumTeams( 0 );

	// Publish the the server.
	if( !m_pGameSpyServer->Publish( ))
	{	
		pNetGameInfo->m_eServerStartResult = eServerStartResult_NetworkError;
		return LT_ERROR;
	}

	// Initialize PunkBuster
	IPunkBusterServer::StartupInfo PBstartupInfo;
	PBstartupInfo.m_PBServerCallBacks.GetClientInfo = PunkBusterGetClientInfoCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetServerAddr = PunkBusterGetServerAddressCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetVersion = PunkBusterGetGameVersionCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetClientStats = PunkBusterGetClientStatsCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetMaxClients = PunkBusterGetMaxClientsCallback ;
	PBstartupInfo.m_PBServerCallBacks.DropClient = PunkBusterDropClientCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetSocket = PunkBusterGetSocketCallback ;
	PBstartupInfo.m_PBServerCallBacks.SendGameMessage = PunkBusterSendGameMessageCallback ;
	PBstartupInfo.m_PBServerCallBacks.DisplayMessage = PunkBusterDisplayMessageCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetMapName = PunkBusterGetMapNameCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetServerHostName = PunkBusterGetServerHostNameCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetGameName = PunkBusterGetGameNameCallback ;
	PBstartupInfo.m_PBServerCallBacks.GetCvarValue = PunkBusterGetConsoleVariableCallback ;
	PBstartupInfo.m_PBServerCallBacks.RestartMap = PunkBusterRestartMapCallback ;
	PBstartupInfo.m_PBServerCallBacks.ChangeMap = PunkBusterChangeMapCallback ;
	PBstartupInfo.m_PBServerCallBacks.HandleAdminCommand = PunkBusterHandleAdminCommandCallback ;
	m_pPunkBusterServer = g_pLTGameUtil->CreatePunkBusterServer( PBstartupInfo );

	// initialize PunkBuster server
	m_pPunkBusterServer->Initialize();

	// Enable or disable pb based on settings.
	if( GameModeMgr::Instance( ).m_ServerSettings.GetUsePunkbuster() && !m_pPunkBusterServer->IsEnabled( ))
	{
		m_pPunkBusterServer->Enable( );
	}
	else if( !GameModeMgr::Instance( ).m_ServerSettings.GetUsePunkbuster() && m_pPunkBusterServer->IsEnabled( ))
	{
		m_pPunkBusterServer->Disable( );
	}

	m_pGameSpyServer->SetServerProperty( "punkbuster", m_pPunkBusterServer->IsEnabled() ? "1" : "0" );

	// initialize the SCMD console interface for PunkBuster web access
	ScmdConsole::Instance().Init(m_cScmdPunkBusterDriver);

#endif // !PLATFORM_XENON

	pNetGameInfo->m_eServerStartResult = eServerStartResult_Success;


	//hack to disable flicker lights in MP games [jrg - 12/09/04]
	if(!g_vtFlickerDisableMessages.IsInitted())
	{
		g_vtFlickerDisableMessages.Init(g_pLTServer, "FlickerDisableMessages", NULL, 0.0f);
	}

	if (IsMultiplayerGameServer())
	{
		g_vtFlickerDisableMessages.SetFloat(1.0f);
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnServerTerm()
//
//	PURPOSE:	Server is shutting down...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnServerTerm()
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

#if !defined(PLATFORM_LINUX)
	CAssertMgr::Disable();
#endif // !PLATFORM_LINUX

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	if( m_pGameSpyServer )
	{
		g_pLTGameUtil->DestroyGameSpyServer( m_pGameSpyServer );
		m_pGameSpyServer = NULL;
	}

#endif // !PLATFORM_XENON

	if ( m_pPunkBusterServer )
	{
		g_pLTGameUtil->DestroyPunkBusterServer( m_pPunkBusterServer );
		m_pPunkBusterServer = NULL;
	}

	CLightEditor::Singleton().Term();

	ServerPhysicsCollisionMgr::Instance().Term( );

	if( m_pServerSaveLoadMgr )
	{
		debug_delete( m_pServerSaveLoadMgr );
		m_pServerSaveLoadMgr = NULL;
	}

	if ( m_pCharacterMgr != 0 )
	{
		debug_delete( m_pCharacterMgr );
		m_pCharacterMgr = 0;
	}

	if ( m_pAIMgr != 0 )
	{
		m_pAIMgr->TermAI();
		debug_delete( m_pAIMgr );
		m_pAIMgr = 0;
	}

	if ( m_pGlobalMgr != 0 )
	{
		debug_delete( m_pGlobalMgr );
		m_pGlobalMgr = 0;
	}

	// Delete the AI class factory LAST.
	if ( m_pAIClassFactory != 0 )
	{
		debug_delete( m_pAIClassFactory );
		m_pAIClassFactory = 0;
	}

	// restore database if necessary
	if (m_hGameDatabase && m_hOverridesDatabase)
	{
		g_pLTDatabase->SwapDatabaseValues(m_hOverridesDatabase, m_hGameDatabase);
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		m_hOverridesDatabase = NULL;
	}

	if (IsMultiplayerGameServer() && GameModeMgr::Instance().m_ServerSettings.m_bEnableScoringLog)
	{
		// write to scoring log
		WriteMultiplayerScoringLogEntry("Server stopped.");
	}

	// free overrides buffers
	delete [] m_pCompressedOverridesBuffer;
	delete [] m_pszServerOverrides;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CGameServerShell::CGameServerShell() :
	  m_pAIClassFactory( 0 )
	, m_pGlobalMgr( 0 )
	, m_pCharacterMgr( 0 )
	, m_nLastPublishTime(0)
	, m_nUniqueObjectID(0)
	, m_pGameSpyServer( NULL )
	, m_bInfiniteAmmo( false )
	, m_hGameDatabase(NULL)
	, m_hOverridesDatabase(NULL)
	, m_nDecompressedOverridesSize(0)
	, m_pCompressedOverridesBuffer(NULL)
	, m_nCompressedOverridesSize(0)
	, m_pszServerOverrides(NULL)
	, m_bGoreAllowed(true)
	, m_eWorldObjectsLOD( eEngineLOD_Low )
	, m_pPunkBusterServer(NULL)
{
	m_pAIMgr = NULL;

	// Create the object template mgr
	m_pObjectTemplates = debug_new( CObjectTemplateMgr );

	// Note : ctor stuff is now all handled by ::OnServerInitialized

	m_pServerMissionMgr = NULL;

	m_nEntryCount = 0;

	m_hSlowMoRecord = NULL;

	m_nSlowMoActivatorTeamId = INVALID_TEAM;

	m_fLastPlayerDeathTime = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::~CGameServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CGameServerShell::~CGameServerShell()
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Get rid of the object template mgr
	debug_delete( m_pObjectTemplates );
	m_pObjectTemplates = 0;

	// Toss the server missionmgr.
	if( m_pServerMissionMgr )
	{
		debug_delete( m_pServerMissionMgr );
		m_pServerMissionMgr = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnAddClient()
//
//	PURPOSE:	Called when a client connects to a server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnAddClient(HCLIENT hClient)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	ServerConnectionMgr::Instance().OnAddClient( hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnRemoveClient()
//
//	PURPOSE:	Called when a client disconnects from a server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnRemoveClient(HCLIENT hClient)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Notify observers about client getting removed.
	RemoveClientNotifyParams cParams(RemoveClient, hClient);
	RemoveClient.DoNotify(cParams);

	ServerConnectionMgr::Instance().OnRemoveClient( hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

LPBASECLASS	CGameServerShell::OnClientEnterWorld(HCLIENT hClient)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

    if( !hClient )
	{
		LTERROR( "Invalid client." );
		return NULL;
	}
	
	// Get the gameclientdata associated with this client.
	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( hClient );
	if( !pGameClientData )
	{
		LTERROR( "Invalid client." );
		return NULL;
	}

	// See if we can use a player from one of the clientrefs.
	CPlayerObj* pPlayer = PickClientRefPlayer( hClient );

	// If we found a player, then respawn, otherwise create a new player.
	if( !pPlayer )
	{
		pPlayer = CreatePlayer( pGameClientData );
		if( !pPlayer )
		{
			LTERROR( "Could not create player." );
			return NULL;
		}
	}

	pGameClientData->SetPlayer( pPlayer->m_hObject );

	// Player object meet client, client meet player object...
	pPlayer->SetClient(hClient);

	// Add this client to our local list...
	SetUpdateGameServ();

	if( IsMultiplayerGameServer( ))
	{
		// Forward the notification to the player objects
		for ( HCLIENTREF hCurClient = g_pLTServer->GetNextClientRef( NULL ); 
			hCurClient; 
			hCurClient= g_pLTServer->GetNextClientRef( hCurClient )
			)
		{
			// Get the player associated with this clientref.
			HOBJECT hObject = g_pLTServer->GetClientRefObject( hCurClient );
			CPlayerObj* pCurPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObject ));
			if( !pCurPlayer )
				continue;

			// Tell them someone new arrived
			pCurPlayer->OnClientEnterWorld(hClient, pPlayer);
		}
	}

	// Check if we have client save data to tell the client.
	if( pPlayer->GetClientSaveData( ))
	{
		CAutoMessage cLoadMsg;
		cLoadMsg.Writeuint8( MID_PLAYER_LOADCLIENT );
		cLoadMsg.Writeuint32( g_pVersionMgr->GetCurrentSaveVersion( ));
		cLoadMsg.Writeuint8( g_pVersionMgr->GetLGFlags() );
		cLoadMsg.WriteMessage( pPlayer->GetClientSaveData( ));
		pPlayer->SetClientSaveData( NULL );
		g_pLTServer->SendToClient(cLoadMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED);
	}

	// All done...
	return pPlayer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientExitWorld()
//
//	PURPOSE:	remove a client
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnClientExitWorld(HCLIENT hClient)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Remove this client from our local list...

	SetUpdateGameServ();


	// Remove the player object...

    CPlayerObj* pPlayer = GetPlayerFromHClient( hClient );
	if (pPlayer)
	{
		// Clear our client pointer.
		pPlayer->SetClient( NULL );
        g_pLTServer->RemoveObject(pPlayer->m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PreStartWorld()
//
//	PURPOSE:	Handle pre start world
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PreStartWorld(bool bSwitchingWorlds)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Make sure the client doesn't hold a pause on us.
	ClientPauseGame( false );

	// Not switching worlds yet.
	SetSwitchingWorldsState( eSwitchingWorldsStateNone );

	// Make sure we're at realtime.
	ExitSlowMo( false, 0.0f );
	SetSimulationTimerScale( 1, 1 );

	//shut down any light editing that may have been done
	CLightEditor::Singleton().Term();

	// No commands yet...
	m_CmdMgr.Clear();

	m_pCharacterMgr->PreStartWorld(m_nLastLGFlags);

	m_pAIMgr->PreStartWorld( bSwitchingWorlds );

	// Tell the server app.
	ServerAppPreStartWorld( );


	// Make sure we reset any necessary globals...

	Camera::ClearActiveCameras();

	// Clear the object template mgr
	m_pObjectTemplates->Clear();

	ServerPhysicsCollisionMgr::Instance().PreStartWorld();

	g_pServerSaveLoadMgr->PreStartWorld( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PostStartWorld()
//
//	PURPOSE:	Handle post switch world
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PostStartWorld()
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Start the switching worlds state machine.
	StartSwitchingWorlds( );

	ServerConnectionMgr::Instance().PostStartWorld();

	m_fLastPlayerDeathTime = 0.0f;

	g_pServerMissionMgr->PostStartWorld();

	m_pCharacterMgr->PostStartWorld(m_nLastLGFlags);

	// Load any commands we might have for this level...

	if( g_pCommandDB )
		g_pCommandDB->PostStartWorld( m_nLastLGFlags );

	// Tell the server app.
	ServerAppPostStartWorld( );

	// Give the AIMgr a chance to do post-start world stuff
	m_pAIMgr->PostStartWorld();

	ServerPhysicsCollisionMgr::Instance().PostStartWorld( );

	CCharacter::ClearDeathCaps();

	//shut down any light editing that may have been done
	CLightEditor::Singleton().Init(m_sWorld.c_str());
}


void CGameServerShell::SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, uint8 nInfoType)
{
	HCLIENT hClient	= pPlayer->GetClient();
	if (!hClient) return;

    uint32  nClientID   = g_pLTServer->GetClientID(hClient);

	bool bIsAdmin = false;
	if( ScmdServer::Instance( ).GetAdminControl( ) == ScmdServer::kAdminControlClient && 
		hClient == ScmdServer::Instance( ).GetAdminClient( ))
	{
		bIsAdmin = true;
	}

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayer->GetClient( ));
	if( !pGameClientData )
		return;

	NetClientData ncd;
	if( !pGameClientData->GetNetClientData( ncd ))
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_INFO);
	cMsg.WriteWString(pPlayer->GetNetUniqueName());
	// Send patch as file title only to reduce bandwidth.
	char szPatchTitle[MAX_PATH] = "";
	LTFileOperations::SplitPath( pPlayer->GetPatch(), NULL, szPatchTitle, NULL );
	cMsg.WriteString(szPatchTitle);
	cMsg.Writeuint32(nClientID);
	cMsg.WriteObject(pPlayer->m_hObject);
	cMsg.Writeuint8( GameModeMgr::Instance().m_grbUseTeams ? ncd.m_nTeamModelIndex : ncd.m_nDMModelIndex );
	cMsg.Writeuint8(pPlayer->GetTeamID( ));
	cMsg.Writebool( bIsAdmin );
    cMsg.Writeuint8(nInfoType);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);

	pGameClientData->GetPlayerScore()->UpdateClients(hClients);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::ApplyDecal
//
//  PURPOSE:	Send decal messages to client.  Necessary since decals
//				can only be applied on the client, yet rigid body collision
//				notification only happen on the server (for performance).
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ApplyDecal(HOBJECT hModel, HMODELNODE hNode, HRECORD hDecalType, const LTVector& vPos, const LTVector& vDir)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_APPLY_DECAL);
	cMsg.WriteObject(hModel);
	cMsg.Writeuint32(hNode);
	cMsg.WriteDatabaseRecord(g_pLTDatabase, hDecalType);
	cMsg.WriteLTVector(vPos);
	cMsg.WriteLTVector(vDir);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, 0);
}

/***************************************************************************/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnMessage()
//
//	PURPOSE:	Routes messages to a specific handler in an effort to reduce 
//				ungodly sized switch cases.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	if( !pMsg )
		return;

	if( ServerConnectionMgr::Instance().OnMessage( hSender, *pMsg ))
		return;

	// Let the server missionmgr handle its messages.
	if( g_pServerMissionMgr )
	{
		if( g_pServerMissionMgr->OnMessage( hSender, *pMsg ))
			return;
	}

	// Let the server saveloadmgr handle its messages.
	if( g_pServerSaveLoadMgr )
	{
		if( g_pServerSaveLoadMgr->OnMessage( hSender, *pMsg ))
			return;
	}

	// Check for a SCMD command
	if( ScmdServer::Instance( ).OnMessage( hSender, *pMsg ))
	{
		return;
	}

	// Check for a SCMD response
	pMsg->SeekTo(0);
	if( ScmdConsole::Instance( ).OnMessage( pMsg->Readuint8(), *pMsg ))
	{
		return;
	}

	pMsg->SeekTo(0);
	uint8 messageID = pMsg->Readuint8();

	switch (messageID)
	{
		case MID_PLAYER_UPDATE:				HandlePlayerUpdate				(hSender, pMsg);	break;
		case MID_WEAPON_FIRE:				HandleWeaponFire				(hSender, pMsg);	break;
		case MID_WEAPON_FINISH:				HandleWeaponFinish				(hSender, pMsg);	break;
		case MID_WEAPON_FINISH_RAGDOLL:		HandleWeaponFinishRagdoll		(hSender, pMsg);	break;
		case MID_WEAPON_RELOAD:				HandleWeaponReload				(hSender, pMsg);	break;
		case MID_WEAPON_SWAP:				HandleWeaponSwap				(hSender, pMsg);	break;
		case MID_WEAPON_SOUND:				HandleWeaponSound				(hSender, pMsg);	break;
		case MID_WEAPON_SOUND_LOOP:			HandleWeaponSoundLoop			(hSender, pMsg);	break;
		case MID_WEAPON_CHANGE:				HandleWeaponChange				(hSender, pMsg);	break;
		case MID_PLAYER_ACTIVATE:			HandlePlayerActivate			(hSender, pMsg);	break;
		case MID_PLAYER_CLIENTMSG:			HandlePlayerClientMsg			(hSender, pMsg);	break;
		case MID_FRAG_SELF:					HandleFragSelf					(hSender, pMsg);	break;
		case MID_DO_DAMAGE:					HandleDoDamage					(hSender, pMsg);	break;
		case MID_PLAYER_RESPAWN:			HandlePlayerRespawn				(hSender, pMsg);	break;
		case MID_PLAYER_EVENT:				HandlePlayerEvent				(hSender, pMsg);	break;
		case MID_PLAYER_SPECTATORMODE:		HandlePlayerRequestSpectatorMode(hSender, pMsg);	break;
		case MID_PLAYER_MESSAGE:			HandlePlayerMessage				(hSender, pMsg);	break;
		case MID_PLAYER_CHEAT:				HandlePlayerCheat				(hSender, pMsg);	break;
		case MID_PLAYER_TELEPORT:			HandlePlayerTeleport			(hSender, pMsg);	break;
		case MID_GAME_PAUSE:				HandleGamePause					(hSender, pMsg);	break;
		case MID_GAME_UNPAUSE:				HandleGameUnpause				(hSender, pMsg);	break;
		case MID_CONSOLE_TRIGGER:			HandleConsoleTrigger			(hSender, pMsg);	break;
		case MID_CONSOLE_COMMAND:			HandleConsoleCommand			(hSender, pMsg);	break;
		case MID_DIFFICULTY:				HandleDifficulty				(hSender, pMsg);	break;
		case MID_STIMULUS:					HandleStimulus					(hSender, pMsg);	break;
		case MID_RENDER_STIMULUS:			HandleRenderStimulus			(hSender, pMsg);	break;
		case MID_OBJECT_ALPHA:				HandleObjectAlpha				(hSender, pMsg);	break;
		case MID_ADD_GOAL:					HandleAddGoal					(hSender, pMsg);	break;
		case MID_REMOVE_GOAL:				HandleRemoveGoal				(hSender, pMsg);	break;
		case MID_AIDBUG:					HandleAIDebug					(hSender, pMsg);	break;				
		case MID_DECISION:					HandleDecision					(hSender, pMsg);	break;
		case MID_PLAYER_BROADCAST:			HandlePlayerBroadcast			(hSender, pMsg);	break;
		case MID_PLAYER_GEAR:				HandlePlayerGear				(hSender, pMsg);	break;
		case MID_SLOWMO:					HandlePlayerSlowMo				(hSender, pMsg);	break;
		case MID_MULTIPLAYER_UPDATE:		HandleMultiplayerUpdate			(hSender, pMsg);	break;
		case MID_PLAYER_INFOCHANGE:			HandleMultiplayerPlayerUpdate	(hSender, pMsg);	break;
		case MID_PICKUPITEM_ACTIVATE:		HandlePickupItemActivate		(hSender, pMsg);	break;
		case MID_PICKUPITEM_ACTIVATE_EX:	HandlePickupItemActivateEx		(hSender, pMsg);	break;
		case MID_PLAYER_GHOSTMESSAGE:		HandlePlayerGhostMessage		(hSender, pMsg);	break;
		case MID_PLAYER_CHATMODE:			HandlePlayerChatMode			(hSender, pMsg);	break;
		case MID_CLEAR_PROGRESSIVE_DAMAGE:	HandleClearProgressiveDamage	(hSender, pMsg);	break;
		case MID_SAVE_GAME:					HandleSaveGame					(hSender, pMsg);	break;
		case MID_OBJECT_MESSAGE:			HandleObjectMessage				(hSender, pMsg);	break;
		case MID_PLAYER_ANIMTRACKERS:		HandleAnimTrackersMessage		(hSender, pMsg);	break;
		case MID_WEAPON_PRIORITY:			HandleWeaponPriority			(hSender, pMsg);	break;
		case MID_SONIC:						HandleSonic						(hSender, pMsg);	break;
		case MID_DYNANIMPROP:				HandleDynAnimProp				(hSender, pMsg);	break;
		case MID_DROP_GRENADE:				HandleDropGrenade				(hSender, pMsg);	break;
		case MID_PLAYER_ARRIVED_AT_NODE:	HandleArrivedAtNode				(hSender, pMsg);	break;
		case MID_SOUND_BROADCAST_DB:		HandleSoundBroadcastDB			(hSender, pMsg);	break;
		case MID_GORE_SETTING:				HandleGoreSettingMessage		(hSender, pMsg);	break;
		case MID_PERFORMANCE_SETTING:		HandlePerformanceSettingMessage	(hSender, pMsg);	break;
		case MID_PUNKBUSTER_MSG:			HandlePunkBusterMessage			(hSender, pMsg);	break;
		case MID_VOTE:				ServerVoteMgr::Instance().HandleMsgVote	(hSender, pMsg);	break;
		default:																				break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerUpdate ()
//
//	PURPOSE:	Pass player update off to player.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerUpdate (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Update the player...

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		if (pPlayer->ClientUpdate(pMsg))
		{
			// Merged player position and update messages.
			pPlayer->HandlePlayerPositionMessage(pMsg); 
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Handle ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponFire (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	
	if (pPlayer)
	{
		pPlayer->HandleWeaponFireMessage (pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Handle ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponFinish (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		pPlayer->HandleWeaponFinishMessage (pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Handle ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponFinishRagdoll (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		pPlayer->HandleWeaponFinishRagdollMessage (pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDropGrenade ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleDropGrenade( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	CPlayerObj *pPlayer = GetPlayerFromHClient( hSender );

	if( pPlayer )
	{
		pPlayer->HandleDropGrenadeMessage( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDropGrenade ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleArrivedAtNode( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	CPlayerObj *pPlayer = GetPlayerFromHClient( hSender );
	if( !pPlayer )
		return;

	HOBJECT hNode = pMsg->ReadObject( );
	if( !hNode )
		return;

	PlayerNodeGoto *pNode = dynamic_cast<PlayerNodeGoto*>(g_pLTServer->HandleToObject( hNode ));
	if( pNode )
	{
		pNode->HandlePlayerArrival( pPlayer->GetHOBJECT( ));
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleSoundBroadcastDB
//
//  PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleSoundBroadcastDB(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CAutoMessage cMsg;
	HRECORD hSound;
	cMsg.Writeuint8(MID_SOUND_BROADCAST_DB);	
	hSound = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pSoundDB->GetSoundCategory());
	cMsg.WriteDatabaseRecord(g_pLTDatabase, hSound);
	cMsg.WriteObject(pMsg->ReadObject());
	SendToClientsExcept(*cMsg.Read(), hSender, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleWeaponReload
//
//  PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponReload(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj *pPlayer	= GetPlayerFromHClient( hSender );
	if( pPlayer )
	{
		HWEAPON hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ));
		HAMMO hAmmo = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
		pPlayer->DoWeaponReload( hWeapon, hAmmo );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleWeaponSwap
//
//  PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponSwap(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj *pPlayer	= GetPlayerFromHClient( hSender );

	if( pPlayer )
	{
		HWEAPON hFromWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
		HWEAPON hToWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );

		pPlayer->DoWeaponSwap( hFromWeapon, hToWeapon );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleWeaponSound ()
//
//	PURPOSE:	Handle sound message from weapon.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponSound (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		pPlayer->HandleWeaponSoundMessage (pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleWeaponSoundLoop
//
//  PURPOSE:	Relays message to specific player to handle
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponSoundLoop( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	CPlayerObj *pPlayer = GetPlayerFromHClient( hSender );

	if( pPlayer )
	{
		pPlayer->HandleWeaponSoundLoopMessage( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleWeaponChange ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponChange (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		HWEAPON	hWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
		HAMMO	hAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
		
		pPlayer->DoWeaponChange( hWeapon, hAmmo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerActivate ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerActivate (HCLIENT hSender, ILTMessage_Read *pMsg)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		pPlayer->HandleActivateMessage(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerClientMsg ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerClientMsg (HCLIENT hSender, ILTMessage_Read *pMsg)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	
	if (pPlayer)
	{
		pPlayer->HandleClientMsg(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerTeleport ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerTeleport (HCLIENT hSender, ILTMessage_Read *pMsg)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	
	if (pPlayer)
	{
		pPlayer->HandleTeleportMsg(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleFragSelf ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleFragSelf (HCLIENT hSender, ILTMessage_Read* /*pMsg*/)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	
	if (pPlayer)
	{
		DamageStruct damage;

		damage.eType	= DT_EXPLODE;
		damage.fDamage	= damage.kInfiniteDamage;
		damage.hDamager = pPlayer->m_hObject;

		damage.DoDamage(pPlayer->m_hObject, pPlayer->m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDoDamage ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleDoDamage (HCLIENT hSender, ILTMessage_Read* pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		char szTargetName[256];
		pMsg->ReadString(szTargetName, ARRAY_LEN(szTargetName));

		const float fAmount = pMsg->Readfloat();

		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(szTargetName, objArray);

		DamageStruct damage;
		damage.eType	= DT_EXPLODE;
		damage.fDamage	= fAmount;
		damage.hDamager = pPlayer->m_hObject;

		for( uint32 i = 0; i < objArray.NumObjects(); ++i )
		{
			damage.DoDamage(pPlayer->m_hObject, objArray.GetObject(i));
		}

		CAutoMessage cReplyMsg;
		cReplyMsg.Writeuint8(MID_DO_DAMAGE);
		cReplyMsg.WriteString(szTargetName);
		cReplyMsg.Writeuint8(objArray.NumObjects());
		cReplyMsg.Writefloat(fAmount);
		g_pLTServer->SendToClient(cReplyMsg.Read(), hSender, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerRespawn ()
//
//	PURPOSE:	when the player dies he is respawned back into the world in
//				one of two states. the first state is ePlayerState_Alive where the 
//				player can participate in the match. the second state is
//				ePlayerState_Spectator where the player has no interaction with the 
//				in-game world and basically acts as a spectator.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerRespawn (HCLIENT hSender, ILTMessage_Read* /*pMsg*/)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	// Only allowed to respawn while playing or ending round.
	if( g_pServerMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath )
		return;
	
	// don't respawn player if he is alive (may cause problems for end of match spawns) 
	if (pPlayer && pPlayer->CanRespawn( ))
	{
		// Tell them about slowmo every time they start up a player.
		if( IsMultiplayerGameServer( ) && IsInSlowMo( ))
		{
			pPlayer->EnterSlowMo( GetSlowMoRecord(), NULL, m_nSlowMoActivatorTeamId, 0 );
		}

		pPlayer->Respawn( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerEvent
//
//	PURPOSE:	Handle a player event message
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerEvent(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		pPlayer->HandlePlayerEventMsg(pMsg);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerRequestSpectatorMode()
//
//	PURPOSE:	Handles a request of changing the spectator mode.
//
// ----------------------------------------------------------------------- //
void CGameServerShell::HandlePlayerRequestSpectatorMode(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	// Only allowed while playing or ending round.
	if( g_pServerMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath )
		return;

	SpectatorMode eSpectatorMode = ( SpectatorMode )pMsg->Readuint8( );
	pPlayer->SetSpectatorMode( eSpectatorMode, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerBroadcast ()
//
//	PURPOSE:	when we get an audio broadcast from a player we first check to
//				see if we have audio broadcasts turned off here on the server.
//				if not we forward the broadcast to the clients
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerBroadcast (HCLIENT hSender, ILTMessage_Read *pMsg)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	pPlayer->HandleBroadcastMsg(pMsg);
} 

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerGear()
//
//	PURPOSE:	pass gear message off to player
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerGear(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	pPlayer->GetInventory()->HandleGearMsg(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerSlowMo()
//
//	PURPOSE:	pass slow mo message off to player
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerSlowMo(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Only allowed while playing or ending round.
	if( g_pServerMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath )
		return;

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	pPlayer->HandlePlayerSlowMoMsg(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleWeaponPriority()
//
//	PURPOSE:	pass weapon priority message off to player
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleWeaponPriority(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	pPlayer->GetInventory()->HandlePriorityMsg(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleSonic()
//
//	PURPOSE:	pass weapon priority message off to player
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleSonic( HCLIENT hSender, ILTMessage_Read* pMsg )
{
	CPlayerObj* pPlayer = GetPlayerFromHClient( hSender );

	if( !pPlayer )
		return;

	pPlayer->HandleSonicMsg( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDynAnimProp()
//
//	PURPOSE:	Handle client telling us about a new dynamic animation propery
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleDynAnimProp( HCLIENT hSender, ILTMessage_Read* pMsg )
{
	char pszPropName[256];
	pMsg->ReadString(pszPropName,LTARRAYSIZE(pszPropName));
	EnumAnimProp eNewProp = (EnumAnimProp)pMsg->Readuint32();
	int iOffset = pMsg->Readuint32();
	AnimPropUtils::Sync(pszPropName,eNewProp,iOffset);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleMultiplayerUpdate ()
//
//	PURPOSE:	send all of the player states to everyone on the server. this
//				is basically just a state update.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleMultiplayerUpdate (HCLIENT hSender, ILTMessage_Read* /*pMsg*/)
{
    CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	// Tell the new Client about all the clients already
	// on the server...

	for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pExistingPlayer = *iter;
		if( pExistingPlayer && pExistingPlayer->GetClient())
		{
			SendPlayerInfoMsgToClients(pPlayer->GetClient(), pExistingPlayer, MID_PI_UPDATE);
		}
	}
} 

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleMultiplayerPlayerUpdate ()
//
//	PURPOSE:	updates a clients player info and informs all other clients of the changes.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleMultiplayerPlayerUpdate (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if( !pPlayer )
		return;

		// try and update player from message
	if( !pPlayer->MultiplayerUpdate( pMsg ))
		return;

	// Send a message to all clients, letting them know a
	// new client has joined the game...

    SendPlayerInfoMsgToClients(NULL, pPlayer, MID_PI_UPDATE);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePickupItemActivate()
//
//	PURPOSE:	handles the players request to swap the current weapon
//					with one on the ground
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePickupItemActivate(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if( !pPlayer )
		return;

	// Only allowed while playing or ending round.
	if( g_pServerMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath )
		return;

	// try and update player from message
	HOBJECT hTarget = pMsg->ReadObject( );
	if( !hTarget )
		return;

	if( pMsg->Readbool( ))
	{
		LTRigidTransform tPickup;
		g_pLTServer->GetObjectTransform( hTarget, &tPickup );
		pPlayer->WeaponSwap( hTarget, tPickup );
	}
	else
	{
		g_pCmdMgr->QueueMessage( pPlayer, g_pLTServer->HandleToObject( hTarget ), "ACTIVATE" );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePickupItemActivateEx()
//
//	PURPOSE:	handles the players request to swap the current weapon
//					with one on the ground
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePickupItemActivateEx(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if( !pPlayer )
		return;

	// Only allowed while playing or ending round.
	if( g_pServerMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pServerMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath )
		return;

	// try and update player from message
	HOBJECT hTarget = pMsg->ReadObject( );
	if( !hTarget )
		return;

	if( pMsg->Readbool( ))
	{
		LTRigidTransform tPickup = pMsg->ReadLTRigidTransform();
		pPlayer->WeaponSwap( hTarget, tPickup );
	}
	else
	{
		g_pCmdMgr->QueueMessage( pPlayer, g_pLTServer->HandleToObject( hTarget ), "ACTIVATE" );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerMessage ()
//
//	PURPOSE:	this handler is called when a player sends out a text message
//				which routes it to the rest of the player
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerMessage (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// retrieve the string they sent
	wchar_t szString[256];
	szString[0] = 0;
	pMsg->ReadWString(szString, LTARRAYSIZE(szString));
	bool bTeamMsg = pMsg->Readbool();

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer )
		return;

	uint8 nTeam = INVALID_TEAM;
	if (GameModeMgr::Instance( ).m_grbUseTeams && bTeamMsg && pPlayer)
		nTeam = pPlayer->GetTeamID();
	uint32 clientID = g_pLTServer->GetClientID(hSender);

	// So it shows up in GameSrv..
    if(szString[0] && !(g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL))
	{
		wchar_t szMsg[256] = {0};
		LTSNPrintF( szMsg, LTARRAYSIZE(szMsg), L"%ls: %ls", pPlayer->GetNetUniqueName(), szString );

		g_pLTServer->CPrintNoArgs( MPW2A(szMsg).c_str() );
	}

	// now send the string to all clients

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_MESSAGE);
	cMsg.WriteWString(szString);
	cMsg.Writeuint32(clientID);
	cMsg.Writeuint8(nTeam);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerGhostMessage ()
//
//	PURPOSE:	this handler is called when a player sends out a text message
//				which only routes it to other spectators
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerGhostMessage (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// retrieve the string they sent
	wchar_t szString[100];
	szString[0] = 0;
	pMsg->ReadWString(szString, LTARRAYSIZE(szString));
	bool bTeamMsg = pMsg->Readbool();

	//do nothing if player isn't a ghost
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if( !pPlayer || !pPlayer->IsSpectating())
		return;

	uint8 nTeam = INVALID_TEAM;
	if (GameModeMgr::Instance( ).m_grbUseTeams && bTeamMsg && pPlayer)
		nTeam = pPlayer->GetTeamID();
	uint32 clientID = g_pLTServer->GetClientID(hSender);

	// So it shows up in GameSrv..
    if(szString[0] && !(g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL))
	{
		wchar_t szMsg[256] = {0};
		LTSNPrintF( szMsg, LTARRAYSIZE(szMsg), L"%ls: %ls", pPlayer->GetNetUniqueName(), szString );

		g_pLTServer->CPrintNoArgs( MPW2A(szMsg).c_str() );
	}

	// Send the message to all clients if allowed, or just other spectators.
	bool bAllClients = GameModeMgr::Instance( ).m_grbAllowSpectatorToLiveChatting;
	for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pCurPlayer = *iter;
		if (pCurPlayer && pCurPlayer->GetClient() && ( bAllClients || pCurPlayer->IsSpectating()))
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_GHOSTMESSAGE);
			cMsg.WriteWString(szString);
			cMsg.Writeuint32(clientID);
			cMsg.Writeuint8(nTeam);
			g_pLTServer->SendToClient(cMsg.Read(), pCurPlayer->GetClient(), MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerChatMode ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerChatMode (HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (pPlayer)
	{
		bool bChatting = !!pMsg->Readuint8();
		pPlayer->SetChatting(bChatting);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerCheat ()
//
//	PURPOSE:	this handler is called when the player enters a cheat code
//				into the console. it checks some validity issues and then 
//				routes the cheat to the appropriat handler
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerCheat (HCLIENT hSender, ILTMessage_Read *pMsg)
{

#ifdef _DEMO
	return;  // No cheats in demo mode
#endif

	// get a pointer to the sender's player object

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (!pPlayer) return;

	// retrieve message data

    CheatCode nCheatCode = (CheatCode) pMsg->Readuint8();
    
	// now deal with the specific cheat code

	if (nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX) return; // wont the default case catch these?

#ifdef _FINAL
	if (IsMultiplayerGameServer( )) // No cheats in multiplayer - oh yeah?! :{
	{
		switch (nCheatCode)				// why always send the data? it was easier to cut & paste
		{
			case CHEAT_NEXTMISSION:
			{
				uint32 nData = pMsg->Readuint32();
				CheatNextMission( pPlayer, nData );
			}
			break;

			case CHEAT_BOOT:
			{
				uint32 nData = pMsg->Readuint32();
				CheatBootPlayer( pPlayer, nData );
			}
			break;
		}
		return;
	}

#else

	// Only allow cheats in MP if console variable turned on.
	if( IsMultiplayerGameServer( ))
	{
		bool bAllowMPCheats = !!GetConsoleInt( "AllowMPCheats", 0 );
		if( !bAllowMPCheats )
			return;
	}

#endif

	switch (nCheatCode)
	{	
		case CHEAT_GOD:
		{
			uint32 nData = pMsg->Readuint32();
			CheatGod( pPlayer, nData );
		}
		break;

		case CHEAT_AMMO:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatAmmo( pPlayer, nData );	
		}
		break;

		case CHEAT_ARMOR:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatArmor( pPlayer, nData );	
		}
		break;

		case CHEAT_HEALTH:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatHealth	( pPlayer, nData );
		}
		break;

		case CHEAT_CLIP:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatClip( pPlayer, nData );	
		}
		break;

		case CHEAT_INVISIBLE:
		{
			uint32 nData = pMsg->Readuint32();
			CheatInvisible( pPlayer, nData );
		}
		break;

		case CHEAT_TELEPORT:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatTeleport( pPlayer, nData );	
		}
		break;

		case CHEAT_FULL_WEAPONS:	
		{
			uint32 nData = pMsg->Readuint32();
			CheatWeapons( pPlayer, nData );	
		}
		break;

		case CHEAT_TEARS:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatTears( pPlayer, nData );
		}
		break;

		case CHEAT_REMOVEAI:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatRemoveAI( pPlayer, nData );	
		}
		break;

		case CHEAT_MODSQUAD:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatModSquad( pPlayer, nData );
		}
		break;

		case CHEAT_FULL_GEAR:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatFullGear( pPlayer, nData );
		}
		break;

		case CHEAT_EXITLEVEL:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatExitLevel( pPlayer, nData );	
		}
		break;

		case CHEAT_NEXTMISSION:		
		{
			uint32 nData = pMsg->Readuint32();
			CheatNextMission( pPlayer, nData );	
		}
		break;

		case CHEAT_BOOT:			
		{
			uint32 nData = pMsg->Readuint32();
			CheatBootPlayer( pPlayer, nData );	
		}
		break;

		case CHEAT_GIMMEGUN:
		{
			HWEAPON hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
			CheatGimmeGun( pPlayer, hWeapon );	
		}
		break;

		case CHEAT_GIMMEMOD:		
		{
			HMOD hMod = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
			CheatGimmeMod( pPlayer, hMod );	
		}
		break;

		case CHEAT_GIMMEGEAR:
		{
			HGEAR hGear = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetGearCategory() );
			CheatGimmeGear( pPlayer, hGear );
		}
		break;

		case CHEAT_GIMMEAMMO:
		{
			HAMMO hAmmo = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
			CheatGimmeAmmo( pPlayer, hAmmo );
		}
		break;

		case CHEAT_BODYGOLFING:	
		{
			uint32 nData = pMsg->Readuint32();
			CheatBodyGolfing( pPlayer, nData );	
		}
		break;

		default:					
		{
		}
		break; // good way to catch bugs, we hate those default cheaters
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleGamePause ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleGamePause (HCLIENT hSender, ILTMessage_Read* /*pMsg*/)
{
	ClientPauseGame( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleGameUnpause ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleGameUnpause (HCLIENT hSender, ILTMessage_Read* /*pMsg*/)
{
	ClientPauseGame( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleTrigger ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleTrigger (HCLIENT hSender, ILTMessage_Read* pMsg)
{
	// Only accept console triggers from the local host.
	bool bIsHost = g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL;
	if( !bIsHost ) 
		return;

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if (!pPlayer)
	{
		return;
	}

	// Read the message data...
	char szName[256];
	char szCommand[256];

	pMsg->ReadString(szName,LTARRAYSIZE(szName));
	pMsg->ReadString(szCommand,LTARRAYSIZE(szCommand));

	ILTBaseClass *pTarget = NULL;
	FindNamedObject( szName, pTarget, false );

	// Special case if we're supposed to list objects of a certain type...

	if (LTSubStrICmp(szCommand, "LIST", 4) == 0)
	{
		ConParse parse;
		parse.Init(szCommand);

        bool bNoObjects = true;

		if (g_pCommonLT->Parse(&parse) == LT_OK)
		{
			if (parse.m_nArgs > 1)
			{
                g_pLTServer->CPrint("Listing objects of type '%s'", parse.m_Args[1]);

				LTVector vPos;
                HCLASS  hClass = g_pLTServer->GetClass(parse.m_Args[1]);

				// Get the names of all the objects of the specified class...

                HOBJECT hObj = g_pLTServer->GetNextObject(NULL);
				while (hObj)
				{
                    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
					{
						g_pLTServer->GetObjectPos( hObj, &vPos );
                        g_pLTServer->CPrint("%s (active) Pos<%.3f,%.3f,%.3f>", GetObjectName(hObj), 
							VEC_EXPAND( vPos ));
                        bNoObjects = false;
					}

                    hObj = g_pLTServer->GetNextObject(hObj);
				}

                hObj = g_pLTServer->GetNextInactiveObject(NULL);
				while (hObj)
				{
                    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
					{
						g_pLTServer->GetObjectPos( hObj, &vPos );
                        g_pLTServer->CPrint("%s (inactive) Pos<%.3f,%.3f,%.3f>", GetObjectName(hObj), 
							VEC_EXPAND( vPos ));
                        bNoObjects = false;
					}

                    hObj = g_pLTServer->GetNextInactiveObject(hObj);
				}

				if (bNoObjects)
				{
                    g_pLTServer->CPrint("No objects of type '%s' exist (NOTE: object type IS case-sensitive)", parse.m_Args[1]);
				}
			}
		}
	}
	// Send the message to all appropriate objects...
	else if( g_pCmdMgr->QueueMessage( pPlayer, pTarget, szCommand ))
	{
		g_pLTServer->CPrint("Sent '%s' Msg '%s'", szName[0] ? szName : "Invalid Object!", szCommand[0] ? szCommand : "Empty Message!!!");
	}
	else
	{
        g_pLTServer->CPrint("Failed to Send '%s' Msg '%s'!", szName[0] ? szName : "Invalid Object!", szCommand[0] ? szCommand : "Empty Message!!!");
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleCommand ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleCommand (HCLIENT hSender, ILTMessage_Read* pMsg)
{

#ifdef _FINAL
	// Only accept console triggers from the local host.
	bool bIsHost = g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL;
	if( !bIsHost ) 
		return;
#endif // _FINAL

	char szCommand[256];
	pMsg->ReadString(szCommand,LTARRAYSIZE(szCommand));

	// Get the player of the sending client.
	CPlayerObj *pPlayer = NULL;
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( hSender );
	if( pGameClientData )
	{
   		pPlayer = static_cast< CPlayerObj* >( g_pLTServer->HandleToObject( pGameClientData->GetPlayer( )));
	}
	if( m_CmdMgr.QueueCommand(szCommand, pPlayer, (ILTBaseClass*)NULL))
	{
        g_pLTServer->CPrint("Sent Command '%s'", szCommand);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDifficulty ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleDifficulty (HCLIENT hSender, ILTMessage_Read *pMsg)
{

	if (!GameModeMgr::Instance( ).m_grbUsesDifficulty) return;

	bool bIsHost = g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL;
	if (!bIsHost) return;


	m_eDifficulty = (GameDifficulty)pMsg->Readuint8();
	DebugCPrint(1,"Difficulty:%d",(int)m_eDifficulty);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleGoreSettingMessage ()
//
//	PURPOSE:	Handle a gore setting message from the clients...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleGoreSettingMessage( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	// Ignore the gore setting in multiplayer games...
	if( IsMultiplayerGameServer( ))
		return;

	m_bGoreAllowed = pMsg->Readbool( );

	// After the auto save has been created remove all objects based on new gore settings...
	if( GetSwitchingWorldsState( ) == eSwitchingWorldsStateFinished )
	{
		RemoveObjectsBasedOnClientSettings( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePerformanceSettingMessage ()
//
//	PURPOSE:	Handle a performance setting message from the clients...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePerformanceSettingMessage( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	// Ignore the performance setting in multiplayer games...
	if( IsMultiplayerGameServer( ))
		return;

	// After the auto save has been created remove all objects based on new object detail settings...
	m_eWorldObjectsLOD = static_cast<EEngineLOD>(pMsg->ReadBits( FNumBitsInclusive<eEngineLOD_NumLODTypes>::k_nValue ));
	if( GetSwitchingWorldsState( ) == eSwitchingWorldsStateFinished )
	{
		RemoveObjectsBasedOnClientSettings( );
	}	

	WriteConsoleFloat( "BodyCapRadius", ( float )pMsg->Readuint32( ));
	WriteConsoleFloat( "BodyCapRadiusCount", pMsg->Readuint8( ));
	WriteConsoleFloat( "BodyCapTotalCount", pMsg->Readuint8( ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePunkBusterMessage
//
//	PURPOSE:	Handle a PunkBuster message from a client.
//
// ----------------------------------------------------------------------- //
void CGameServerShell::HandlePunkBusterMessage(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	if (!pMsg)
	{
		return;
	}

	if (!m_pPunkBusterServer)
	{
		return;
	}

	// get the server's address and port
	uint8  aClientAddress[4] = { 0 };
	uint16 nPort = 0;
	g_pLTServer->GetClientAddr(hSender, aClientAddress, &nPort);

	// extract the message data
	uint32 nDataLen = (pMsg->Size() - pMsg->Tell()) / 8;
	uint8* pData = ( uint8* )alloca( nDataLen );
	pMsg->ReadData( pData, nDataLen * 8 );

	// send it to PunkBuster
	m_pPunkBusterServer->HandleNetMessage(pData, nDataLen, aClientAddress, nPort);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleStimulus()
//
//	PURPOSE:	Handle a stimulus message by placing one in the world.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleStimulus(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	uint8 cArgs = pMsg->Readuint8();

	char szStimType[64];
	pMsg->ReadString(szStimType, 64);
	EnumAIStimulusType eStimType = CAIStimulusMgr::StimulusFromString(szStimType);

	float fMultiplier = (cArgs == 2) ? pMsg->Readfloat() : 1.f;

	// Place stimulus at the location of the player.
	LTVector vPos;
	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if (pPlayer)
	{
		g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vPos);

		StimulusRecordCreateStruct scs( eStimType, pPlayer->GetAlignment(), vPos, pPlayer->GetHOBJECT());
		scs.m_flRadiusScalar = fMultiplier;
		g_pAIStimulusMgr->RegisterStimulus( scs );

		g_pLTServer->CPrint("Stimulus: %s at (%.2f, %.2f, %.2f) x%.2f", szStimType, vPos.x, vPos.y, vPos.z, fMultiplier);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleRenderStimulus()
//
//	PURPOSE:	Handle a render stimulus message by toggling stimulus rendering.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleRenderStimulus(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	bool bRender = !!pMsg->Readuint8();

	g_pAIStimulusMgr->RenderStimulus(bRender);

	g_pLTServer->CPrint("Stimulus Rendering: %d", bRender);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleObjectAlpha()
//
//	PURPOSE:	Handle an object alpha message by setting the alpha for the object.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleObjectAlpha(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	char szName[64];
	pMsg->ReadString(szName, 64);

	float fAlpha = pMsg->Readfloat();

	HOBJECT hObject;
	if ( LT_OK == FindNamedObject(szName, hObject) )
	{
		g_pLTServer->SetObjectColor(hObject, 0.f, 0.f, 0.f, fAlpha);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleAddGoal()
//
//	PURPOSE:	Handle an add goal message by adding a goal to an AI.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleAddGoal(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	HOBJECT hAI;

	char szAIName[32];
	pMsg->ReadString(szAIName, 32);
	if ( LT_OK == FindNamedObject(szAIName, hAI) )
	{
		char szGoalType[64];
		pMsg->ReadString(szGoalType, 64);

		char szMsg[128];
		LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "AddGoal %s", szGoalType );

		char szNameValuePair[64];
		uint32 cNameValuePairs = pMsg->Readuint32();
		for(uint32 iNameValuePair=0; iNameValuePair < cNameValuePairs; ++iNameValuePair)
		{
			pMsg->ReadString(szNameValuePair, 64);
			strcat(szMsg, " ");
			strcat(szMsg, szNameValuePair);
		}

		CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hAI);
		g_pCmdMgr->QueueMessage( pAI, pAI, szMsg );
	}
	else
	{
		g_pLTServer->CPrint("Could not find AI named: %s", szAIName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleRemoveGoal()
//
//	PURPOSE:	Handle a remove goal message by removing a goal from an AI.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleRemoveGoal(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	HOBJECT hAI;

	char szAIName[32];
	pMsg->ReadString(szAIName, 32);
	if ( LT_OK == FindNamedObject(szAIName, hAI) )
	{
		char szGoalType[64];
		pMsg->ReadString(szGoalType, 64);

		char szMsg[128];
		LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "RemoveGoal %s", szGoalType );

		CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hAI);
		g_pCmdMgr->QueueMessage( pAI, pAI, szMsg );
	}
	else 
	{
		g_pLTServer->CPrint("Could not find AI named: %s", szAIName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleAIDebug()
//
//	PURPOSE:	Handle routing an AIDebug command to the AI subsystems.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleAIDebug(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Insure there is a valid sender

	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if ( !pPlayer )
	{
		g_pLTServer->CPrint( "CGameServerShell::HandleAIDebug : No sender" );
		return;
	}
	HOBJECT hSendingObject = pPlayer->GetHOBJECT();

	// Build the parsed message into a token
	
	char szCommand[256];
	int nCommandCharacters = pMsg->ReadString( &szCommand[0], LTARRAYSIZE(szCommand) );
	if ( nCommandCharacters >= LTARRAYSIZE(szCommand) )
	{
		g_pLTServer->CPrint( "CGameServerShell::HandleAIDebug : Console command exceeded max length: %s", szCommand);
		return;
	}

	ConParse parse;
	parse.Init( szCommand );
	while ( g_pCommonLT->Parse(&parse) == LT_OK )
	{
		// Send the command on to the debug system.

		CParsedMsg cParsedMsg( parse.m_nArgs, parse.m_Args );
		m_pAIMgr->OnDebugCmd( hSendingObject, cParsedMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleDecision()
//
//	PURPOSE:	Handle Decision object choices
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleDecision(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	HOBJECT hObj = pMsg->ReadObject();
	uint8 nChoice = pMsg->Readuint8();

	DecisionObject *pDecisionObject = (DecisionObject*)g_pLTServer->HandleToObject( hObj );
	if( pDecisionObject )
	{
		pDecisionObject->HandleChoose(pPlayer->m_hObject, nChoice);
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleClearDamage
//
//  PURPOSE:	Clear the specified damage for the client
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleClearProgressiveDamage( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if( pPlayer )
	{
		CDestructible	*pDamage = 	pPlayer->GetDestructible();
		DamageFlags		nDmgFlags;
		pMsg->ReadType( &nDmgFlags );
	
		pDamage->ClearProgressiveDamage( nDmgFlags );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::HandleAnimTrackersMessage
//
//  PURPOSE:	Handle an animation tracker message...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleAnimTrackersMessage( HCLIENT hSender, ILTMessage_Read *pMsg )
{
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);

	if( pPlayer )
	{
		pPlayer->HandleAnimTrackerMsg( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatGod (CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return; // yeah, this is redundant but hell, cheats aren't called every frame

	pPlayer->ToggleGodMode ();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatAmmo (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	pPlayer->FullAmmoCheat ();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatArmor (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	pPlayer->RepairArmorCheat ();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatHealth (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	pPlayer->HealCheat ();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatClip()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatClip (CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return;

	pPlayer->SetSpectatorMode(( SpectatorMode )nData, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatInvisible()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatInvisible(CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return;

	pPlayer->SetInvisibleMode( !!nData );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatTeleport (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	pPlayer->Respawn ();	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatWeapons (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	pPlayer->FullWeaponCheat ();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatTears (CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	m_bInfiniteAmmo = !m_bInfiniteAmmo;
	if( m_bInfiniteAmmo ) 
		pPlayer->FullWeaponCheat();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatRemoveAI (CPlayerObj* pPlayer, uint32 nData)
{
	HandleCheatRemoveAI(nData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatModSquad (CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return;

	pPlayer->FullModsCheat();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Cheat ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatFullGear (CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return;

	pPlayer->FullGearCheat();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatExitLevel()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatExitLevel(CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return;

	bool bIsHost = g_pLTServer->GetClientInfoFlags(pPlayer->GetClient()) & CIF_LOCAL;
	if (!bIsHost) return;

	g_pServerMissionMgr->ExitLevelSwitch( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatNextMission()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatNextMission(CPlayerObj* pPlayer, uint32 /*nData*/)
{
	if (!pPlayer) return;

	bool bIsHost = g_pLTServer->GetClientInfoFlags(pPlayer->GetClient()) & CIF_LOCAL;
	if (!bIsHost) return;


	g_pServerMissionMgr->NextMission( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatBootPlayer()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatBootPlayer(CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer || !IsMultiplayerGameServer() ) return;

	bool bIsHost = g_pLTServer->GetClientInfoFlags(pPlayer->GetClient()) & CIF_LOCAL;
	if (!bIsHost) return;

	//don't boot yourself
	uint32 clientID = g_pLTServer->GetClientID(pPlayer->GetClient());
	if (clientID == nData) return;

	HCLIENT hClient = g_pLTServer->GetClientHandle( nData );
	if( hClient )
		g_pLTServer->KickClient(hClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatGimmeGun()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- 

void CGameServerShell::CheatGimmeGun( CPlayerObj *pPlayer, HWEAPON hWeapon )
{
	if( !pPlayer || !hWeapon )
		return;

	pPlayer->GimmeGunCheat( hWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatGimmeMod()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- 

void CGameServerShell::CheatGimmeMod( CPlayerObj *pPlayer, HMOD hMod )
{
	if( !pPlayer || !hMod )
		return;

	pPlayer->GimmeModCheat( hMod );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatGimmeGear()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- 

void CGameServerShell::CheatGimmeGear( CPlayerObj *pPlayer, HGEAR hGear )
{
	if( !pPlayer || !hGear )
		return;

	pPlayer->GimmeGearCheat( hGear );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatGimmeAmmo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- 

void CGameServerShell::CheatGimmeAmmo( CPlayerObj *pPlayer, HAMMO hAmmo )
{
	if( !pPlayer || !hAmmo )
		return;

	pPlayer->GimmeAmmoCheat( hAmmo );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheatBodyGolfing ()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheatBodyGolfing (CPlayerObj* pPlayer, uint32 nData)
{
	if (!pPlayer) return; 

	//currently unimplemented... perhaps reduce dead characters mass and friction?
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleCheatRemoveAI()
//
//	PURPOSE:	Handle the remove ai cheat
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleCheatRemoveAI(uint32 nData)
{
    HOBJECT hObj   = g_pLTServer->GetNextObject(NULL);
    HCLASS  hClass = g_pLTServer->GetClass("CAI");

	// Remove all the ai objects...

    HOBJECT hRemoveObj = NULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextObject(hObj);

		if (hRemoveObj)
		{
			DamageStruct damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = hRemoveObj;

			damage.DoDamage(hRemoveObj, hRemoveObj);
            hRemoveObj = NULL;
		}
	}


    hObj = g_pLTServer->GetNextInactiveObject(NULL);
    hRemoveObj = NULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);

		if (hRemoveObj)
		{
			DamageStruct damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = hRemoveObj;

			damage.DoDamage(hRemoveObj, hRemoveObj);

            hRemoveObj = NULL;
		}
	}
}

void CGameServerShell::EnterServerShell()
{
#if defined(PLATFORM_SEM)
	if (m_nEntryCount == 0)
	{
		// We shouldn't already have an external scope base interface value if we're entering this scope
		LTASSERT(m_pExternalScopeBaseInterface == NULL, "Scope entry with non-NULL external interface pointer encountered");
		// Save the external scope
		m_pExternalScopeBaseInterface = g_pLTBase;
		// Switch the interfaces over to the server
		g_pModelLT = g_pLTServer->GetModelLT();
		g_pPhysicsLT = g_pLTServer->Physics();
		g_pLTBase = static_cast<ILTCSBase*>(g_pLTServer);
		g_pCommonLT = g_pLTServer->Common();
	}
	// Track our entry level for proper re-entrant behavior
	++m_nEntryCount;
	LTASSERT(g_pLTBase == g_pLTServer, "Scope entry encountered with invalid interface");
#endif // PLATFORM_SEM
}

void CGameServerShell::ExitServerShell()
{
#if defined(PLATFORM_SEM)
	// If we're exiting the context, fix up the interfaces
	if (m_nEntryCount == 1)
	{
		// If we previosly had an external scope, restore the interfaces
		if (m_pExternalScopeBaseInterface != NULL)
		{
			g_pModelLT = m_pExternalScopeBaseInterface->GetModelLT();
			g_pPhysicsLT = m_pExternalScopeBaseInterface->Physics();
			g_pLTBase = static_cast<ILTCSBase*>(m_pExternalScopeBaseInterface);
			g_pCommonLT = m_pExternalScopeBaseInterface->Common();
			m_pExternalScopeBaseInterface = NULL;
		}
		// Otherwise set them to null
		else
		{
			g_pModelLT = NULL;
			g_pPhysicsLT = NULL;
			g_pLTBase = NULL;
			g_pCommonLT = NULL;
		}
	}
	// Track the entry count for proper re-entrancy handling
	if (m_nEntryCount != 0)
		--m_nEntryCount;
#endif // PLATFORM_SEM
}

void CGameServerShell::UpdateClientPingTimes()
{
	float ping;
    uint32 clientID;
	HCLIENT hClient;

	m_ClientPingSendCounter += RealTimeTimer::Instance().GetTimerElapsedS();
	if(m_ClientPingSendCounter > CLIENT_PING_UPDATE_RATE)
	{
		CAutoMessage cMsg;

		cMsg.Writeuint8(MID_PINGTIMES);

        hClient = NULL;
        for(;;)
		{
	        hClient = g_pLTServer->GetNextClient(hClient);
			if( !hClient )
				break;
            clientID = g_pLTServer->GetClientID(hClient);
            g_pLTServer->GetClientPing(hClient, ping);

			cMsg.Writeuint16((uint16)clientID);
			cMsg.Writeuint16((uint16)(ping + 0.5f));
		}

		cMsg.Writeuint16(0xFFFF);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, 0);

		m_ClientPingSendCounter = 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetSwitchingWorldsState
//
//	PURPOSE:	Sets the switching worlds state and tells it to the clients.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetSwitchingWorldsState( SwitchingWorldsState eSwitchingWorldsState )
{
	if( m_eSwitchingWorldsState == eSwitchingWorldsState )
		return;

	// If we're switching back to the beginning, and we aren't
	// finished, then we need to remove the pause added in StartSwitchingWorlds.
	if( eSwitchingWorldsState == eSwitchingWorldsStateNone &&
		m_eSwitchingWorldsState != eSwitchingWorldsStateFinished )
	{
		PauseGame( false );
	}

	m_eSwitchingWorldsState = eSwitchingWorldsState;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SWITCHINGWORLDSSTATE );
	cMsg.Writeuint8( eSwitchingWorldsState );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::StartSwitchingWorlds
//
//	PURPOSE:	Finish our switch world process.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::StartSwitchingWorlds( )
{
	// If we aren't already switching worlds, then we need
	// to add a pause.
	if( m_eSwitchingWorldsState == eSwitchingWorldsStateFinished ||
		m_eSwitchingWorldsState == eSwitchingWorldsStateNone )
	{
		// Pause the game until all the clients are in.  There is an unpause
		// in FinishSwitchingWorlds.
		PauseGame( true );
	}

	SetSwitchingWorldsState( eSwitchingWorldsStatePlayerHookup );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FinishSwitchingWorlds
//
//	PURPOSE:	Finish our switch world process.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::FinishSwitchingWorlds( )
{
	// Step the physics simulation a few times to let objects in the level settle.
	TLTPrecisionTime start = LTTimeUtils::GetPrecisionTime();
	float fPhysicsSettleTime = g_vtPhysicsSettleTime.GetFloat();
	float fPhysicsSettleRate = g_vtPhysicsSettleRate.GetFloat();
	g_pLTServer->PhysicsSim()->StepSimulationForward( fPhysicsSettleTime, fPhysicsSettleRate, 
		( uint32 )( fPhysicsSettleTime / fPhysicsSettleRate + 0.5f ));
	double fPreSteppingTime = LTTimeUtils::GetPrecisionTimeIntervalS( start, LTTimeUtils::GetPrecisionTime());
	DebugCPrint( 1, "Pre-stepping physics took %.3f seconds", fPreSteppingTime );

	// It's safe for objects to be removed since the auto save has been created...
	RemoveObjectsBasedOnClientSettings( );

	// Remove our pause we put on in StartSwitchingWorlds.
	PauseGame( false );

	// Done switching worlds.
	SetSwitchingWorldsState( eSwitchingWorldsStateFinished );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateSwitchingWorlds
//
//	PURPOSE:	Update our switch world state machine.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::UpdateSwitchingWorlds( )
{
	switch( m_eSwitchingWorldsState )
	{
		case eSwitchingWorldsStateNone:
		{
			return;
		}
		break;

		// Waiting for clients from the previous level to 
		// get hooked up to players in this level.
		case eSwitchingWorldsStatePlayerHookup:
		{
			// Check if we can't save the game.  If we can't that means we don't have to sync
			// up all the clients to players before moving on.  If we can, that means
			// we're doing a coop mp game and all players need to come over before we can 
			// go to the next state.
			if( !g_pServerSaveLoadMgr->CanSaveGame( ))
			{
				SetSwitchingWorldsState( eSwitchingWorldsStateWaitForClient );
				return;
			}

			// Loop through the clients that don't have players bound to them.  If they
			// match any of the clientrefs, we have to wait until they are loaded.
			// If none of the clients match a clientref, then we're done with the switchworld.
			bool bWaitLonger = false;
			HCLIENT hNextClient = g_pLTServer->GetNextClient( NULL );

			// If we don't have any clients, then there's no reason to proceed yet.  This
			// avoids a server constantly switching levels.
			if( !hNextClient )
				bWaitLonger = true;

			while( hNextClient )
			{
				// Copy the current client and advance to the next.  Makes body of loop easier.
				HCLIENT hClient = hNextClient;
				hNextClient = g_pLTServer->GetNextClient( hClient );

				// Kick the client if they aren't communicating.
				if( ServerConnectionMgr::Instance( ).ClientConnectionInTrouble( hClient ))
				{
					// Kick the client if they are too slow to play.
					g_pLTServer->KickClient( hClient );
					continue;
				}

				// If the client is in the world, we don't need to wait for them.
				GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( hClient );
				if( pGameClientData )
				{
					if( pGameClientData->GetClientConnectionState() == eClientConnectionState_InWorld )
					{
						continue;
					}

					// A player hasn't respawned, we need to wait until they do.
					bWaitLonger = true;
					break;
				}

				// If there are no clientrefs, then we didn't come from a previous
				// level with players. This guy is just joining and 
				// we need to wait longer for him to finish handshaking.
				HCLIENTREF hNextClientRef = g_pLTServer->GetNextClientRef(NULL);
				if (!hNextClientRef)
				{
					bWaitLonger = true;
					break;
				}

				// See if this client will match any clientrefs.  If so, then we have to wait
				// for the OnClientEnterWorld before we can finish the switch world.
				CPlayerObj* pUsedPlayer = PickClientRefPlayer( hClient );
				if( pUsedPlayer )
				{
					bWaitLonger = true;
					break;
				}
			}

			// See if we need to wait longer before everyone's hooked up.
			if( bWaitLonger )
				return;

			// Wait for player loaded.
			SetSwitchingWorldsState( eSwitchingWorldsStateWaitForClient );

			return;
		}
		break;

		// Wait for clients to be loaded before staring game.
		case eSwitchingWorldsStateWaitForClient:
		{
			// Iterate through all the clients and see if anyone is ready to play.
			ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
			ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
			for( ; iter != gameClientDataList.end( ); iter++ )
			{
				GameClientData* pGameClientData = *iter;
				if( !pGameClientData->GetClient( ))
					continue;

				// Check the client's connection.  We don't want to stall all the players
				// to wait for one bad connection.
				if( ServerConnectionMgr::Instance( ).ClientConnectionInTrouble( pGameClientData->GetClient( )))
				{
					// Kick the client if they aren't communicating.
					g_pLTServer->KickClient( pGameClientData->GetClient( ));
					continue;
				}

				// Skip clients that aren't ready to play yet.
				if( pGameClientData->GetClientConnectionState() != eClientConnectionState_InWorld )
					continue;

				// Client must have reached the inworld state.
				if( !pGameClientData->IsClientInWorld( ))
					continue;

				// Count them as ready.
				// Do an reloadsave if we aren't loading from a save game.
				if( g_pServerSaveLoadMgr->CanSaveGame( ) && m_nLastLGFlags != LOAD_RESTORE_GAME )
				{
					// Do an reloadsave.
					g_pServerSaveLoadMgr->ReloadSave( g_pServerMissionMgr->GetCurrentMission( ), 
						g_pServerMissionMgr->GetCurrentLevel( ));

					SetSwitchingWorldsState( eSwitchingWorldsStateReloadSave );
				}
				else
				{
					FinishSwitchingWorlds();
				}
			}
		}
		break;

		// Wait for the reloadsave to finish.
		case eSwitchingWorldsStateReloadSave:
		{
			// See if we're done with the reloadsave.
			if( g_pServerSaveLoadMgr->GetSaveDataState( ) == eSaveDataStateNone )
			{
				FinishSwitchingWorlds();
				return;
			}
		}
		break;

		case eSwitchingWorldsStateFinished:
		{
			return;
		}
		break;

		default:
		{
			LTERROR( "Invalid switching worlds state." );
			return;
		}
		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PreUpdate
//
//	PURPOSE:	Pre-Update server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PreUpdate()
{
	// Note : This extra server shell scope update makes sure that the object updates are also covered
	// in the server shell scope
	EnterServerShell();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Update
//
//	PURPOSE:	Update servier stuff periodically
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Update(float /*timeElapsed*/)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Check for first update...

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = false;
		FirstUpdate();
	}

	// Update the switching worlds state machine.
	UpdateSwitchingWorlds();

	// Update the AI systems.

	m_pAIMgr->Update();

	// Update the command mgr...

	m_CmdMgr.Update();

	// Update the light editor
	CLightEditor::Singleton().Update();

	// Should the stair height change...

	if( g_StairStepHeight.GetFloat() != DEFAULT_STAIRSTEP_HEIGHT )
	{
		g_pPhysicsLT->SetStairHeight( g_StairStepHeight.GetFloat() );
	}

#ifndef _FINAL
	if( g_pAINodeMgr )
	{
		g_pAINodeMgr->UpdateDebugRendering( g_ShowNodesTrack.GetFloat() );
	}

	if( g_pAINavMesh )
	{
		g_pAINavMesh->UpdateDebugRendering( g_ShowNavMeshTrack.GetFloat(), g_ShowAIRegionsTrack.GetFloat() );
	}
#endif

	if( g_ClearLinesTrack.GetFloat() > 0.0f )
	{
		LineSystem::RemoveAll();
		g_ClearLinesTrack.SetFloat(0.0f);
	}

	// Did the server want to say something?

	const char *pSay = m_SayTrack.GetStr("");
	if(pSay && pSay[0] != 0)
	{
		wchar_t fullMsg[512];

		LTSNPrintF(fullMsg, LTARRAYSIZE(fullMsg), L"HOST: %ls", MPA2W(pSay).c_str());

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_MESSAGE);
		cMsg.WriteWString(fullMsg);
		cMsg.Writeuint32((uint32)-1);
		cMsg.Writeuint8(INVALID_TEAM);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		m_SayTrack.SetStr("");
	}

	g_pServerMissionMgr->Update( );

	g_pServerSaveLoadMgr->Update( );

	ServerPhysicsCollisionMgr::Instance().Update( );

	// Update our slowmo state.
	UpdateSlowMo();

	ServerConnectionMgr::Instance().Update( );


	if( IsMultiplayerGameServer( ))
	{
		// Update multiplayer stuff...
		UpdateMultiplayer();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PostUpdate
//
//	PURPOSE:	Post-Update server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PostUpdate()
{
	// Note : This extra server shell scope update makes sure that the object updates are also covered
	// in the server shell scope
	ExitServerShell();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FirstUpdate
//
//	PURPOSE:	Do the first update
//
// ----------------------------------------------------------------------- //

void CGameServerShell::FirstUpdate()
{
	m_SayTrack.Init(g_pLTServer, "Say", "", 0.0f);

	// Init some console vars...

	extern VarTrack g_vtAIDamageAdjustEasy;
	extern VarTrack g_vtAIDamageAdjustNormal;
	extern VarTrack g_vtAIDamageAdjustHard;
	extern VarTrack g_vtAIDamageAdjustVeryHard;
	extern VarTrack g_vtDamageDebug;
	extern VarTrack g_vtDemoModeInvulnerable;
	extern VarTrack g_vtHealthTest;


	float fVal = 0.0f;
	if( !g_vtAIDamageAdjustEasy.IsInitted() )
	{
		fVal = g_pServerDB->GetDamageAdjustment(GD_EASY);
		g_vtAIDamageAdjustEasy.Init( g_pLTServer, "AIDamageAdjustEasy", NULL, fVal );
	}
	if( !g_vtAIDamageAdjustNormal.IsInitted() )
	{
		fVal = g_pServerDB->GetDamageAdjustment(GD_NORMAL);
		g_vtAIDamageAdjustNormal.Init( g_pLTServer, "AIDamageAdjustNormal", NULL, fVal );
	}
	if( !g_vtAIDamageAdjustHard.IsInitted() )
	{
		fVal = g_pServerDB->GetDamageAdjustment(GD_HARD);
		g_vtAIDamageAdjustHard.Init( g_pLTServer, "AIDamageAdjustHard", NULL, fVal );
	}
	if( !g_vtAIDamageAdjustVeryHard.IsInitted() )
	{
		fVal = g_pServerDB->GetDamageAdjustment(GD_VERYHARD);
		g_vtAIDamageAdjustVeryHard.Init( g_pLTServer, "AIDamageAdjustVeryHard", NULL, fVal );
	}
	if ( !g_vtDamageDebug.IsInitted() )
	{
		g_vtDamageDebug.Init( g_pLTServer, "DamageDebug", NULL, 0.0f );
	}

	if ( !g_vtDemoModeInvulnerable.IsInitted() )
	{
		g_vtDemoModeInvulnerable.Init( g_pLTServer, "DemoModeInvulnerable", NULL, 0.0f );
	}
	if ( !g_vtHealthTest.IsInitted() )
	{
		g_vtHealthTest.Init( g_pLTServer, "HealthTest", NULL, 0.0f );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateGameServer
//
//	PURPOSE:	Updates a stand-alone server with game info if necessary
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::UpdateGameServer()
{
    // Check if we need to update...

	if (!m_bUpdateGameServ)
	{
        return(false);
	}

    m_bUpdateGameServ = false;


	// Make sure we are actually being hosted via GameServ...

	if( !m_NetGameInfo.m_bDedicated )
	{
        return(false);
	}

	ServerAppShellUpdate( );

	// All done...
    return(true);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::InitGameType
//
//	PURPOSE:	Setup game type.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::InitGameType( NetGameInfo const& netGameInfo )
{
	m_NetGameInfo = netGameInfo;

	// Write out the missionorder record to use.
	if( !m_NetGameInfo.m_sMissionOrder.empty( ))
		WriteConsoleString( "MissionOrder", m_NetGameInfo.m_sMissionOrder.c_str( ));

	// Toss any old missionmgr.
	if( m_pServerMissionMgr )
	{
		debug_delete( m_pServerMissionMgr );
		m_pServerMissionMgr = NULL;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateMultiplayer
//
//	PURPOSE:	Determine if it is time to change levels
//
// ----------------------------------------------------------------------- //
void CGameServerShell::UpdateMultiplayer()
{
	if (!IsMultiplayerGameServer( )) return;

	// Update client ping times
	UpdateClientPingTimes();

	// Update game server info...
	UpdateGameServer();

	// Update the gamespy info.
	UpdateGameSpy( );

	if ( m_pPunkBusterServer )
		m_pPunkBusterServer->ProcessEvents();

	// Update the object transform history...
	CObjectTransformHistoryMgr::Instance( ).Update( );

	ServerVoteMgr::Instance().Update();
	TeamBalancer::Instance().Update();

}

bool CGameServerShell::UpdateGameSpy( )
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// No need to continue unless we have a gamespyserver.
	if( !m_pGameSpyServer )
		return false;

	// Publish the server if we've waited long enough since the last directory update
	uint32 nCurTime = (uint32)LTTimeUtils::GetTimeMS();
	if ((m_nLastPublishTime == 0) || 
		((nCurTime - m_nLastPublishTime) > k_nRepublishDelay))
	{
		m_nLastPublishTime = nCurTime;
		uint32 nMax = 0;
		g_pLTServer->GetMaxConnections(nMax);

		// If not run by a dedicated server, we need to add one connection
		// for the local host.
		if( !m_NetGameInfo.m_bDedicated )
			nMax++;

		char szProp[256];
		char szValue[256];
		char szPath[256];

		// Update the details
		ServerMissionSettings sms = g_pServerMissionMgr->GetServerSettings();
		LTFileOperations::SplitPath(GetCurLevel(), szPath, szValue, NULL );
		m_pGameSpyServer->SetServerProperty( "mappath", szPath );
		m_pGameSpyServer->SetServerProperty( "mapname", szValue );

		LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%d", (uint32)GameModeMgr::Instance( ).m_grnScoreLimit );
		m_pGameSpyServer->SetServerProperty( "fraglimit", szValue );
		LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%d", (uint32)GameModeMgr::Instance( ).m_grnTimeLimit );
		m_pGameSpyServer->SetServerProperty( "timelimit", szValue );

		// Only do the options if the options are dirty.
		if( GameModeMgr::Instance().IsBrowserDirty( ))
		{
			// Iterate through the list of gamerules.  Only show the one's that can be modified.
			char szOptions[2048] = "";
			GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( );
			uint32 nGameRuleIndex = 0;
			for( ; iter != GameRule::GetGameRuleList().end( ); iter++, nGameRuleIndex++ )
			{
				GameRule* pGameRule = *iter;

				// Skip rules not shown on the list.
				if( !pGameRule->IsCanModify( ) || !pGameRule->IsShowInOptions( ))
					continue;

				// If this isn't the first option, then separate the option with a token.
				if( szOptions[0] )
					LTStrCat( szOptions, ";", LTARRAYSIZE( szOptions ));

				// Create the format string using the passed in format for the value.
				wchar_t wszRuleValue[256];
				pGameRule->ToString( wszRuleValue, LTARRAYSIZE( wszRuleValue ), false );
				LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "(%d)(%s)", nGameRuleIndex, MPW2A( wszRuleValue ).c_str( ));
				LTStrCat( szOptions, szValue, LTARRAYSIZE( szOptions ));
			}
			m_pGameSpyServer->SetServerProperty( "options", szOptions );

			// Not dirty anymore.
			GameModeMgr::Instance().SetBrowserDirty( false );
		}

		std::string sNetUniqueName;

		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		uint32 nIndex;
		for( nIndex = 0; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
		{
			CPlayerObj* pPlayerObj = *iter;

			// only update the player if we have a valid client handle
			if (pPlayerObj->GetClient())
			{
				GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
				if( !pGameClientData )
					continue;

				LTSNPrintF( szProp, LTARRAYSIZE( szProp ), "player_%d", nIndex );
				sNetUniqueName = MPW2A( pPlayerObj->GetNetUniqueName( ));
				m_pGameSpyServer->SetServerProperty( szProp, sNetUniqueName.c_str( ));
				LTSNPrintF( szProp, LTARRAYSIZE( szProp ), "score_%d", nIndex );
				LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%d", pGameClientData->GetPlayerScore()->GetScore( ));
				m_pGameSpyServer->SetServerProperty( szProp, szValue );

				float fPing;
				g_pLTServer->GetClientPing( pPlayerObj->GetClient( ), fPing );
				uint16 nPing = ( uint16 )( fPing + 0.5f );
				LTSNPrintF( szProp, LTARRAYSIZE( szProp ), "ping_%d", nIndex );
				LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%d", nPing );
				m_pGameSpyServer->SetServerProperty( szProp, szValue );

				LTSNPrintF( szProp, LTARRAYSIZE( szProp ), "score_%d", nIndex );
				LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%i", pGameClientData->GetPlayerScore()->GetScore( ));
				m_pGameSpyServer->SetServerProperty( szProp, szValue );

				nIndex++;
			}
		}

		uint32 nNumClients = ServerConnectionMgr::Instance().GetGameClientDataList( ).size( );
		m_pGameSpyServer->SetNumPlayers( nNumClients );
		LTSNPrintF( szValue, LTARRAYSIZE( szValue ), "%d", nNumClients );
		m_pGameSpyServer->SetServerProperty( "numplayers", szValue );
	}

#endif // !PLATFORM_XENON

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::StartNextMultiplayerLevel
//
//	PURPOSE:	Start the next multiplayer level
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::StartNextMultiplayerLevel()
{
	return g_pServerMissionMgr->NextMission( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppMessageFn
//
//	PURPOSE:	Server app message function
//
// ----------------------------------------------------------------------- //

LTRESULT CGameServerShell::ServerAppMessageFn( ILTMessage_Read& msg )
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

	// Let the scmd check if it needs this message.
	if( ScmdServer::Instance( ).OnMessage( NULL, msg ))
	{
		return LT_OK;
	}

	msg.SeekTo(0);
	switch( msg.Readuint8( ))
	{
		case SERVERSHELL_INIT:
		{
			if( !OnServerShellInit( ))
				return LT_ERROR;
		}
			break;

		case SERVERSHELL_NEXTWORLD:
		{	
			if( !StartNextMultiplayerLevel( ))
				return LT_ERROR;
		}
			break;

		case SERVERSHELL_SETWORLD:
		{	
			// Read in the world index.  Go back one, since StartNextMultiplayerLevel
			// will preincrement the curlevel.
			int nNewMission;
			nNewMission = msg.Readuint32( );
			if( !g_pServerMissionMgr->SwitchToCampaignIndex( nNewMission ))
				return LT_ERROR;
		}
		break;

		case SERVERSHELL_MESSAGE:
		{
			char szMsg[256] = {0};
			msg.ReadString( szMsg, ARRAY_LEN( szMsg ));

			// Intercept PunkBuster Console Commands
			if ( !LTSubStrICmp ( szMsg, "pb_", 3 ) )
				if ( m_pPunkBusterServer )
					m_pPunkBusterServer->ProcessCmd( szMsg );
			g_pLTServer->RunConsoleCommand( szMsg );

		}
		break;
		
		case SERVERSHELL_MISSIONFAILED:
		{
			// We have a failed mission from the server app...
			// Wait for client responses to begin the auto load.
			
			g_pServerSaveLoadMgr->WaitForAutoLoadResponses();
		}
			break;

		case SERVERSHELL_USERMESSAGE:
			{	
				GameModeMgr& gameModeMgr = GameModeMgr::Instance();
				wchar_t wszUserMessage[GameModeMgr::kMaxUserMessageLen+1] = L"";
				msg.ReadWString( wszUserMessage, LTARRAYSIZE( wszUserMessage ));
				gameModeMgr.m_ServerSettings.m_sServerMessage = wszUserMessage;
				wszUserMessage[0] = L'\0';
				msg.ReadWString( wszUserMessage, LTARRAYSIZE( wszUserMessage ));
				gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage = wszUserMessage;
			}
			break;


		// Invalid message.
		default:
		{
			ASSERT( FALSE );
			return LT_ERROR;
		}
			break;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnServerShellInit
//
//	PURPOSE:	Initializes the server servershell from serverapp.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::OnServerShellInit( )
{
	if (m_NetGameInfo.m_bDedicated)
	{
		//force construction of DM mission file
		g_pMissionDB->CreateMPDB();
	}

	// Initialize the server game.  Need to do this first to init servermissionmgr.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_GAME );
	CLTMsgRef_Read msgRead = cMsg.Read( );
	if( !g_pServerMissionMgr->OnMessage( NULL, *msgRead ))
		return false;

	// First we'll tell the serverapp about the missions in the campaign.
	CAutoMessage cLevelsMsg;
	cLevelsMsg.Writeuint8( SERVERAPP_INIT );

	char szFirstMission[MAX_PATH*2] = "";

	Campaign& campaign = g_pServerMissionMgr->GetCampaign( );
	cLevelsMsg.Writeuint8( campaign.size( ));
	for( Campaign::iterator iter = campaign.begin( ); iter != campaign.end( ); iter++ )
	{
		// Get the mission in the campaign.
		int nMission = *iter;
		HRECORD hMission = g_pMissionDB->GetMission( nMission );
		if( !hMission )
			continue;

		// Use the first level of the mission as the name of the mission.
		HRECORD hLevel = g_pMissionDB->GetLevel(hMission,0);
		if( !hLevel )
			continue;

		if( !szFirstMission[0] )
		{
			LTStrCpy( szFirstMission, g_pMissionDB->GetWorldName(hLevel,true), LTARRAYSIZE( szFirstMission ));
		}

 		cLevelsMsg.WriteString( g_pMissionDB->GetWorldName(hLevel,false) );
	}

	// Send the message to the server app.
	CLTMsgRef_Read msgRefReadLevels = cLevelsMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefReadLevels );

	// Start the first level.
	CAutoMessage cMsgStartLevel;
	cMsgStartLevel.Writeuint8( MID_START_LEVEL );
	cMsgStartLevel.WriteString( szFirstMission );
	CLTMsgRef_Read msgReadStartLevel = cMsgStartLevel.Read( );
	if( !g_pServerMissionMgr->OnMessage( NULL, *msgReadStartLevel ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PauseGame
//
//	PURPOSE:	Pause/unpause the game.  bForce overrides the counted pauses.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PauseGame( bool bPause, bool bForce )
{
	// Reset the pause count if forced.
	if( bForce )
	{
		m_nPauseCount = 0;
	}

	PauseGame( bPause );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PauseGame
//
//	PURPOSE:	Pause/unpause the game using reference counts.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PauseGame( bool bPause )
{
	if( bPause )
		m_nPauseCount++;
	else
		m_nPauseCount--;

	// Make sure we don't go negative.
	m_nPauseCount = LTMAX( 0, m_nPauseCount );

	//determine what state we are in
	bool bInPauseState = (m_nPauseCount > 0);

	//if we are not paused, forget whether the client had paused us
	if (!bInPauseState)
		m_bClientPaused = false;


	//alright, we have changed pause state, so let us notify the server and its objects
	SimulationTimer::Instance( ).PauseTimer( bInPauseState );
	GameTimeTimer::Instance( ).PauseTimer( bInPauseState );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_GAME_PAUSE );
	cMsg.Writebool( bInPauseState );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ClientPauseGame( )
//
//	PURPOSE:	Modifies the client's pause on the game.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ClientPauseGame( bool bPause )
{
	if( bPause )
	{
		if( m_bClientPaused )
			return;

		m_bClientPaused = true;
		PauseGame( true );
	}
	else
	{
		if( m_bClientPaused )
		{
			m_bClientPaused = false;
			PauseGame( false );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleCmdMsg()
//
//	PURPOSE:	Handle console cmd messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleCmdMsg(HCLIENT /*hSender*/, ILTMessage_Read *pMsg)
{
	char szCommand[256];
	pMsg->ReadString(szCommand,LTARRAYSIZE(szCommand));


    if (m_CmdMgr.QueueCommand(szCommand, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL))
	{
        g_pLTServer->CPrint("Sent Command '%s'", szCommand);
	}

	
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleSaveData
//
//	PURPOSE:	Handle savegame command from client.
//
// --------------------------------------------------------------------------- //

void CGameServerShell::HandleSaveGame( HCLIENT hSender, ILTMessage_Read* pMsg )
{
	if( !pMsg )
		return;

	// Ignore save requests from non-local clients.
	if( !( g_pLTServer->GetClientInfoFlags( hSender ) & CIF_LOCAL ))
		return;

	int nSlot = pMsg->Readuint8( );

	// Check if this is a quicksave.
	if( nSlot == 0 )
	{
		g_pServerSaveLoadMgr->QuickSave( g_pServerMissionMgr->GetCurrentMission( ), 
			g_pServerMissionMgr->GetCurrentLevel( ));
	}
	else
	{
		// Get the savegame name.
		wchar_t wszSaveName[MAX_PATH];
		pMsg->ReadWString( wszSaveName, LTARRAYSIZE( wszSaveName ));
		g_pServerSaveLoadMgr->SaveGameSlot( nSlot, wszSaveName );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleObjectMessage
//
//	PURPOSE:	Handles generic message from the client to a specific object
//
// ----------------------------------------------------------------------- //
void CGameServerShell::HandleObjectMessage(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	// Sanity check
	if(!pMsg)
		return;

	// Grab the object and make sure it exists
	HOBJECT hObj = pMsg->ReadObject();
	if(!hObj)
		return;

	// Cast it
	GameBase *pObj = (GameBase *)g_pLTServer->HandleToObject(hObj);
	if(!pObj)
		return;

	// Check for the player object
	CPlayerObj* pPlayer = GetPlayerFromHClient(hSender);
	if(!pPlayer)
	{
        return;
	}

	// Call the object's objectmessagefn and forward the message
	pObj->ObjectMessageFn(pPlayer->m_hObject, CLTMsgRef_Read(pMsg->SubMsg( pMsg->Tell( ))));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Save
//
//	PURPOSE:	Save the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	LTASSERT(pMsg, "Attempting to save server shell to a null message.");
	if (!pMsg) return;

	SAVE_DWORD(g_pVersionMgr->GetSaveVersion());

	// Do NOT save m_nUniqueObjectID here.
	// Instead save it in the Player so that it
	// can remain unique as the player traverses TransAms.

	m_pCharacterMgr->Save(pMsg);
	m_CmdMgr.Save(pMsg);
	m_pAIMgr->Save(pMsg);
	m_pObjectTemplates->Save( pMsg );

	g_pServerMissionMgr->Save( *pMsg, dwSaveFlags );

	m_SlowMoTimer.Save( *pMsg );
	SAVE_HRECORD( m_hSlowMoRecord );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Load
//
//	PURPOSE:	Load the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	LTASSERT(pMsg, "Attempting to load server shell from a null message.");
	if (!pMsg) return;

	uint32 nSaveVersion;
	LOAD_DWORD(nSaveVersion);
	g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );
	
	// Do NOT load m_nUniqueObjectID here.
	// Instead load it in the Player so that it
	// can remain unique as the player traverses TransAms.

	// Tell the persistent timers about the load.
	SerializeableEngineTimer::OnLoad( false );

	// Send the timescale to the client.
	SendSimulationTimerScale( NULL );

	m_pCharacterMgr->Load(pMsg);
	m_CmdMgr.Load(pMsg);
	m_pAIMgr->Load(pMsg);
	m_pObjectTemplates->Load( pMsg );

	g_pServerMissionMgr->Load( *pMsg, dwLoadFlags );

	m_SlowMoTimer.Load( *pMsg );
	LOAD_HRECORD( m_hSlowMoRecord, DATABASE_CATEGORY( SlowMo ).GetCategory( ));
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::GetPlayerPing
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 CGameServerShell::GetPlayerPing(CPlayerObj* pPlayer)
{
	if (!pPlayer) return(0);

	float ping = 0.0f;

	HCLIENT hClient = pPlayer->GetClient();
	if (!hClient) return(0);

	g_pLTServer->GetClientPing(hClient, ping);

	return((uint16)(ping + 0.5f));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::GetCurLevel
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char const* CGameServerShell::GetCurLevel()
{
	return(m_sWorld.c_str());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetCurLevel
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetCurLevel( char const* pszWorldName )
{
	m_sWorld = pszWorldName;
	
#if defined(PLATFORM_LINUX)
	// make sure the world name is Win32 compatible
	for (uint32 nIndex = 0; nIndex < LTStrLen(pszWorldName); ++nIndex)
	{
		if (m_sWorld[nIndex] == '/')
		{
			m_sWorld[nIndex] = '\\';
		}
	}
#endif // PLATFORM_LINUX
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppAddClient
//
//	PURPOSE:	The server app is the dedicated server application and is
//				usefull when running a dedicated server. on a regular server
//				SendToServerApp returns LT_NOTFOUND and nothing actually 
//				gets sent. inorder for SendToServerApp to return LT_OK we
//				need to call CreateServer to get a ServerInterface* and 
//				register a ServerAppHandler to it.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ServerAppAddClient( HCLIENT hClient )
{
	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( hClient );
	if( !pGameClientData )
		return;

	NetClientData ncd;
	if( !pGameClientData->GetNetClientData( ncd ))
		return;

	// Write out the player information.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERAPP_ADDCLIENT );
	cMsg.Writeuint16( g_pLTServer->GetClientID( hClient ));
	cMsg.WriteData(( void* )&ncd.m_PlayerGuid, sizeof( ncd.m_PlayerGuid ) * 8 );

	// Send to the server app.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefRead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppRemoveClient
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ServerAppRemoveClient( HCLIENT hClient )
{
	// Write out the player information.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERAPP_REMOVECLIENT );
	cMsg.Writeuint16( g_pLTServer->GetClientID( hClient ));

	// Send to the server app.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefRead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppShellUpdate
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ServerAppShellUpdate( )
{
	// Write the message id.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERAPP_SHELLUPDATE );

	// Add info for each player.
	ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
	ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
	for( ; iter != gameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( !pGameClientData )
			continue;

		cMsg.Writeuint16( g_pLTServer->GetClientID( pGameClientData->GetClient()));
		cMsg.WriteWString( pGameClientData->GetUniqueName());
		cMsg.Writeint16( pGameClientData->GetPlayerScore()->GetEventCount(CPlayerScore::eKill));
		cMsg.Writeint16( pGameClientData->GetPlayerScore()->GetEventCount(CPlayerScore::eDeath));
		cMsg.Writeint16( pGameClientData->GetPlayerScore()->GetScore( ));
	}

	// Signal end of player list.
	cMsg.Writeuint16(( uint16 )-1 );

	// Send to the server app.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefRead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppPreStartWorld
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ServerAppPreStartWorld( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERAPP_PRELOADWORLD );

	// Send to the server app.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefRead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppPostStartWorld
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ServerAppPostStartWorld( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( SERVERAPP_POSTLOADWORLD );
	cMsg.Writeuint16(( uint16 )g_pServerMissionMgr->GetCurrentCampaignIndex( ));

	char fname[_MAX_FNAME] = "";
	LTFileOperations::SplitPath( GetCurLevel( ), NULL, fname, NULL );
 	cMsg.WriteString( fname );

	// Send to the server app.
	CLTMsgRef_Read msgRefRead = cMsg.Read( );
	g_pLTServer->SendToServerApp( *msgRefRead );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::MatchClientNameToPlayer
//
//	PURPOSE:	Matches a client name to a clientref and returns the
//				playerobj of the clientref.  If it can't match the client
//				to the player, it returns null.
//
// --------------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::MatchClientNameToPlayer( char const* pszClientName )
{
	// Check inputs.
	if( !pszClientName )
	{
		ASSERT( !"CGameServerShell::MatchClientToPlayer: Invalid inputs." );
		return NULL;
	}

	char szClientRefName[MAX_CLIENT_NAME_LENGTH] = "";

	// Search through all the clientrefs to find the one we were using when the player
	// got saved.  This is done by matching our IP addresses together.
    HCLIENTREF hNextClientRef = g_pLTServer->GetNextClientRef(NULL);
	while (hNextClientRef)
	{
		// Copy the current clientref and advance to the next.  Makes body of loop easier.
		HCLIENTREF hClientRef = hNextClientRef;
		hNextClientRef = g_pLTServer->GetNextClientRef(hClientRef);

		// Get the player associated with this clientref.
		HOBJECT hObject = g_pLTServer->GetClientRefObject( hClientRef );
		CPlayerObj* pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObject ));
		if( !pPlayer )
			continue;

		// Get the clientname from the clientref and compare it to the input clientname.
		g_pLTServer->GetClientRefName( hClientRef, szClientRefName, ARRAY_LEN( szClientRefName ));
		if( LTStrICmp( pszClientName, szClientRefName ) != 0 )
			continue;

		// Found a matching player.
		return pPlayer;
	}

	// No player found.
	return NULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FindFreePlayerFromClientRefs
//
//	PURPOSE:	Look through all the clientrefs and see if there is any
//				that doesn't match a different client.  
//
// --------------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::FindFreePlayerFromClientRefs( )
{
	char szClientName[MAX_CLIENT_NAME_LENGTH] = "";

	// Iterate through the clientrefs.  Check if the clientref's playerobj
	// doesn't match a client that doesn't have a playerobj associated with it yet.
	HCLIENTREF hNextClientRef = g_pLTServer->GetNextClientRef(NULL);
	while( hNextClientRef )
	{
		// Copy the current clientref and advance to the next.  Makes body of loop easier.
		HCLIENTREF hClientRef = hNextClientRef;
		hNextClientRef = g_pLTServer->GetNextClientRef( hClientRef );

		// Get the player associated with this clientref.
		HOBJECT hObject = g_pLTServer->GetClientRefObject( hClientRef );
		CPlayerObj* pPlayerRef = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObject ));
		if( !pPlayerRef )
			continue;

		// Loop through the other clients and see if they match this clientref.
		HCLIENT hNextClient = g_pLTServer->GetNextClient( NULL );
		bool bMatched = false;
		while( hNextClient )
		{
			// Copy the current client and advance to the next.  Makes body of loop easier.
			HCLIENT hOtherClient = hNextClient;
			hNextClient = g_pLTServer->GetNextClient( hOtherClient );

			// Check if this client has already been hooked up to a player.  If so, skip it.
			CPlayerObj* pUsedPlayer = GetPlayerFromHClient( hOtherClient );
			if( pUsedPlayer )
				continue;

			// Get the client name.
			g_pLTServer->GetClientName( hOtherClient, szClientName, ARRAY_LEN( szClientName ));

			// If this client doesn't match any clientrefs, then skip it.
			CPlayerObj* pClientMatch = MatchClientNameToPlayer( szClientName );
			if( !pClientMatch )
				continue;

			// Check if this client matches a different clientref.  If so, skip it.
			if( pClientMatch != pPlayerRef )
				continue;

			// This client matches this clientref.  Stop looking and go to the next clientref.
			bMatched = true;				
			break;
		}

		// If no client matched this clientref, then we can take it.
		if( !bMatched )
		{
			// No client matched this clientref, we can use it.
			if( pPlayerRef )
				return pPlayerRef;
		}
	}

	// Didn't find any free clientref players.
	return NULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PickClientRefPlayer
//
//	PURPOSE:	Find the clientref for this client based on the set clientname.
//
// --------------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::PickClientRefPlayer( HCLIENT hClient )
{
	// Check inputs.
	if( !hClient )
	{
		ASSERT( !"CGameServerShell::FindPlayerFromClientRefs: Invalid inputs." );
		return NULL;
	}
	
	// Get the clientname.  Must be set before entering this function.
	char szClientName[MAX_CLIENT_NAME_LENGTH] = "";
	g_pLTServer->GetClientName( hClient, szClientName, ARRAY_LEN( szClientName ));
	if( !szClientName[0] )
	{
		// If we've got a client without a name, they're not ready yet...
		return NULL;
	}

	// See if we have a direct match between our client and a player.
	CPlayerObj* pPlayer = MatchClientNameToPlayer( szClientName );
	if( !pPlayer )
		return NULL;

	return pPlayer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ApplyWeaponRestrictions
//
//	PURPOSE:	Apply the server game option weapon restrictions.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ApplyWeaponRestrictions( )
{
	StringSet setRestrictedWeapons;
	StringSet setRestrictedGear;

	DelimitedStringToStringContainer(GameModeMgr::Instance( ).m_grsRestrictedWeapons.GetValue(),setRestrictedWeapons,",");
	DelimitedStringToStringContainer(GameModeMgr::Instance( ).m_grsRestrictedGear.GetValue(),setRestrictedGear,",");

	StringSet::iterator iter = setRestrictedWeapons.begin( );
	while( iter != setRestrictedWeapons.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( sValue.c_str( ));
		if( !hWeapon  )
			continue;

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->GetBool( hWpnData, WDB_ALL_bCanServerRestrict ))
			g_pWeaponDB->Restrict( hWpnData );
	}

	iter = setRestrictedGear.begin( );
	while( iter != setRestrictedGear.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		HGEAR hGear = g_pWeaponDB->GetGearRecord( sValue.c_str( ));
		if( !hGear )
			continue;

		if( g_pWeaponDB->GetBool( hGear, WDB_ALL_bCanServerRestrict ))
			g_pWeaponDB->Restrict( hGear );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::LoadMultiplayerOverrides
//
//	PURPOSE:	Load multiplayer overrides.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::LoadMultiplayerOverrides(ILTInStream& OverridesStream)
{
	// get the game database
	if ((m_hGameDatabase = OpenGameDatabase(DB_Default_File)) == NULL)
	{
		// failed opening game database
		return false;
	}

	// get the overrides database
	if ((m_hOverridesDatabase = OpenOverridesDatabase(OverridesStream, g_pszConstraintsFilename)) == NULL)
	{
		// failed opening the overrides database
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;
		return false;
	}

	// if there are no categories in the overrides database we need to stop processing - this is not an
	// error, it merely indicates an overrides file with no actual data, most likely since it's all comments.
	if (g_pLTDatabase->GetNumCategories(m_hOverridesDatabase) == 0)
	{
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		m_hOverridesDatabase = NULL;
		return true;
	}

	// swap the values
	if (!g_pLTDatabase->SwapDatabaseValues(m_hOverridesDatabase, m_hGameDatabase))
	{
		// overrides failed
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		m_hOverridesDatabase = NULL;
		return false;
	}

	// store the length of the decompressed data
	m_nDecompressedOverridesSize = (uint32)OverridesStream.GetLen();

	// get the overrides data from the stream
	OverridesStream.SeekTo(0);
	char* pDecompressedOverridesBuffer = NULL;
	LT_MEM_TRACK_ALLOC(pDecompressedOverridesBuffer = new char[m_nDecompressedOverridesSize + 1], LT_MEM_TYPE_GAMECODE);
	if (OverridesStream.Read(pDecompressedOverridesBuffer, m_nDecompressedOverridesSize) != LT_OK)
	{
		// failed to read stream
		delete [] pDecompressedOverridesBuffer;
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		m_hOverridesDatabase = NULL;
		return false;
	}

	// null terminate
	pDecompressedOverridesBuffer[m_nDecompressedOverridesSize] = 0;

	// compress the overrides stream data - start by retrieving the size of the buffer
	// we need to allocate to store the compressed data
	uint32 nCompressedBufferMaxSize = 0;
	if (g_pLTServer->GetCompressedBufferMaxSize(m_nDecompressedOverridesSize, nCompressedBufferMaxSize) != LT_OK)
	{
		// failed to read stream
		delete [] pDecompressedOverridesBuffer;
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		return false;
	}

	// allocate the destination buffer
	LT_MEM_TRACK_ALLOC(m_pCompressedOverridesBuffer = new uint8[nCompressedBufferMaxSize], LT_MEM_TYPE_GAMECODE);
	if (!m_pCompressedOverridesBuffer)
	{
		// failed to allocate
		delete [] pDecompressedOverridesBuffer;
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		return false;
	}
	
	// now compress the buffer
    if (g_pLTServer->CompressBuffer((uint8*)pDecompressedOverridesBuffer, m_nDecompressedOverridesSize, m_pCompressedOverridesBuffer, nCompressedBufferMaxSize, m_nCompressedOverridesSize) != LT_OK)
	{
		// failed to compress
		delete [] pDecompressedOverridesBuffer;
		delete [] m_pCompressedOverridesBuffer;
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		return false;
	}

	// create an abbreviated string containing the category, record, attribute IDs and values
	// for each override.  This is submitted to GameSpy so we can display the customizers on the
	// server browser.
	LT_MEM_TRACK_ALLOC(m_pszServerOverrides = new char[m_nDecompressedOverridesSize], LT_MEM_TYPE_GAMECODE);
	::memset(m_pszServerOverrides, 0, m_nDecompressedOverridesSize);

	// walk the unpacked database and build a string containing the database indexes of the
	// customized entries from the main database, and the customized values.
	uint32 nNumOverrideCategories = g_pLTDatabase->GetNumCategories(m_hOverridesDatabase);
	
	for (uint32 nOverrideCategoryIndex = 0; nOverrideCategoryIndex < nNumOverrideCategories; ++nOverrideCategoryIndex)
	{
		// get the name of the category from the overrides database
		HCATEGORY hOverrideDatabaseCategory = g_pLTDatabase->GetCategoryByIndex(m_hOverridesDatabase, nOverrideCategoryIndex);
		const char* pszCategoryName = g_pLTDatabase->GetCategoryName(hOverrideDatabaseCategory);

		// get the category with the same name from the main database
		HCATEGORY hMainDatabaseCategory = g_pLTDatabase->GetCategory(m_hGameDatabase, pszCategoryName);

		// get the index of this category
		uint32 nMainDatabaseCategoryIndex = g_pLTDatabase->GetCategoryIndex(hMainDatabaseCategory);

		// get the number of records in this category
		uint32 nNumOverrideRecords = g_pLTDatabase->GetNumRecords(hOverrideDatabaseCategory);

		// process each record
		for (uint32 nOverrideRecordIndex = 0; nOverrideRecordIndex < nNumOverrideRecords; ++nOverrideRecordIndex)
		{
			// get the name of the record from the overrides database
			HRECORD hOverrideDatabaseRecord = g_pLTDatabase->GetRecordByIndex(hOverrideDatabaseCategory, nOverrideRecordIndex);
			const char* pszRecordName = g_pLTDatabase->GetRecordName(hOverrideDatabaseRecord);

			// get the record with the same name from the main database
			HRECORD hMainDatabaseRecord = g_pLTDatabase->GetRecord(hMainDatabaseCategory, pszRecordName);

			// get the index of this record
			uint32 nMainDatabaseRecordIndex = g_pLTDatabase->GetRecordIndex(hMainDatabaseRecord);

			// get the number of attributes in this record
			uint32 nNumOverrideAttributes = g_pLTDatabase->GetNumAttributes(hOverrideDatabaseRecord);

			// process each attribute
			for (uint32 nOverrideAttributeIndex = 0; nOverrideAttributeIndex < nNumOverrideAttributes; ++nOverrideAttributeIndex)
			{
				// get the name of the attribute from the overrides database
				HATTRIBUTE hOverrideDatabaseAttribute = g_pLTDatabase->GetAttributeByIndex(hOverrideDatabaseRecord, nOverrideAttributeIndex);
				const char* pszAttributeName = g_pLTDatabase->GetAttributeName(hOverrideDatabaseAttribute);

				// get the attribute with the same name from the main database
				HATTRIBUTE hMainDatabaseAttribute = g_pLTDatabase->GetAttribute(hMainDatabaseRecord, pszAttributeName);

				// get the index of this attribute
				uint32 nMainDatabaseAttributeIndex = g_pLTDatabase->GetAttributeIndex(hMainDatabaseAttribute);

				// get the attribute type from the main database 
				EAttributeType eAttributeType = g_pLTDatabase->GetAttributeType(hMainDatabaseAttribute);

				// get the number of values associated with this attribute
				uint32 nNumOverrideValues = g_pLTDatabase->GetNumValues(hMainDatabaseAttribute);

				// add the category, record, and attribute to the string
				uint32 nServerOverridesLen = LTStrLen(m_pszServerOverrides);
				LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
						   m_nDecompressedOverridesSize - nServerOverridesLen, 
						   "%d.%d.%d=", nMainDatabaseCategoryIndex, nMainDatabaseRecordIndex, nMainDatabaseAttributeIndex);

				// process each value
				for (uint32 nValueIndex = 0; nValueIndex < nNumOverrideValues; ++nValueIndex)
				{
					// get the value and convert it to a string
					switch (eAttributeType)
					{
					case eAttributeType_Bool:
						nServerOverridesLen = LTStrLen(m_pszServerOverrides);
						LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
						  		   m_nDecompressedOverridesSize - nServerOverridesLen, 
								   "%d", g_pLTDatabase->GetBool(hMainDatabaseAttribute, nValueIndex, false));
						break;
					case eAttributeType_Float:
						nServerOverridesLen = LTStrLen(m_pszServerOverrides);
						LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
						  		   m_nDecompressedOverridesSize - nServerOverridesLen, 
								   "%f", g_pLTDatabase->GetFloat(hMainDatabaseAttribute, nValueIndex, 0.0f));
						break;
					case eAttributeType_Int32:
						nServerOverridesLen = LTStrLen(m_pszServerOverrides);
						LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
						  		   m_nDecompressedOverridesSize - nServerOverridesLen, 
								   "%d", g_pLTDatabase->GetInt32(hMainDatabaseAttribute, nValueIndex, 0));
						break;
					case eAttributeType_String:
						nServerOverridesLen = LTStrLen(m_pszServerOverrides);
						LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
						  		   m_nDecompressedOverridesSize - nServerOverridesLen, 
								   "%s", g_pLTDatabase->GetString(hMainDatabaseAttribute, nValueIndex, NULL));
						break;
					case eAttributeType_Vector2:
						{					
							LTVector2 vEmptyVector;
							LTVector2 vValue = g_pLTDatabase->GetVector2(hMainDatabaseAttribute, nValueIndex, vEmptyVector);
							nServerOverridesLen = LTStrLen(m_pszServerOverrides);
							LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
								m_nDecompressedOverridesSize - nServerOverridesLen, 
									   "[%f,%f]", vValue.x, vValue.y);
						}
						break;
					case eAttributeType_Vector3:
						{					
							LTVector vEmptyVector;
							LTVector vValue = g_pLTDatabase->GetVector3(hMainDatabaseAttribute, nValueIndex, vEmptyVector);
							nServerOverridesLen = LTStrLen(m_pszServerOverrides);
							LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
								m_nDecompressedOverridesSize - nServerOverridesLen, 
									   "[%f,%f,%f]", vValue.x, vValue.y, vValue.z);
						}	
						break;
					case eAttributeType_Vector4:
						{					
							LTVector4 vEmptyVector;
							LTVector4 vValue = g_pLTDatabase->GetVector4(hMainDatabaseAttribute, nValueIndex, vEmptyVector);
							nServerOverridesLen = LTStrLen(m_pszServerOverrides);
							LTSNPrintF(m_pszServerOverrides + nServerOverridesLen, 
								m_nDecompressedOverridesSize - nServerOverridesLen, 
									   "[%f,%f,%f,%f]", vValue.x, vValue.y, vValue.z, vValue.w);
						}
						break;
					}

					// add a value separator if necessary
					if (nValueIndex != (nNumOverrideValues - 1))
					{
						LTStrCat(m_pszServerOverrides, ",", m_nDecompressedOverridesSize);
					}
				}	

				// add the attribute separator
				LTStrCat(m_pszServerOverrides, ";", m_nDecompressedOverridesSize);
			}
		}

	}

	// clean up
	delete [] pDecompressedOverridesBuffer;

	// success
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::InitializeMultiplayerScoringLog
//
//	PURPOSE:	Activate server side multiplayer score logging.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::InitializeMultiplayerScoringLog()
{
	// if there is an existing scoring file, open it and read the timestamp
	bool bWriteLogHeader = false;

	char szScoringLogFileName[MAX_PATH] = { 0 };
	LTFileOperations::GetUserDirectory(szScoringLogFileName, LTARRAYSIZE(szScoringLogFileName));
	LTStrCat(szScoringLogFileName, g_pszScoringLogFilename, LTARRAYSIZE(szScoringLogFileName));

	if (LTFileOperations::FileExists(szScoringLogFileName))
	{
		// get the maximum file age from the options (convert to seconds)
		uint32 nMaxFileAge = GameModeMgr::Instance().m_ServerSettings.m_nMaxScoringLogFileAge * 86400;

		// get the current time
		time_t currentTime;
		time(&currentTime);

		// read the file age from the first part of the file (10 bytes)
		CLTFileRead cFileRead;
		if (!cFileRead.Open(szScoringLogFileName))
		{
			return false;
		}

		char szTime[g_nScoringTimeBufferSize] = { 0 };
		if (!cFileRead.Read(szTime, 10))
		{
			return false;
		}

		uint32 nFileCreationTime = 0;
		sscanf(szTime, "%d", &nFileCreationTime);
	
		// if the difference exceeds the maximum age, we need to recycle this file
		if ((currentTime - nFileCreationTime) > nMaxFileAge)
		{
			// copy the current log file to the backup file
			uint64 nFileSize = 0;
			cFileRead.GetFileSize(nFileSize);
	
			cFileRead.Seek(0);
			uint8* pFileData = NULL;
			LT_MEM_TRACK_ALLOC(pFileData = new uint8[(uint32)nFileSize], LT_MEM_TYPE_GAMECODE);

			if (!cFileRead.Read(pFileData, (uint32)nFileSize))
			{
				return false;
			}

			char szScoringLogBackupFileName[MAX_PATH] = { 0 };
			LTFileOperations::GetUserDirectory(szScoringLogBackupFileName, LTARRAYSIZE(szScoringLogBackupFileName));
			LTStrCat(szScoringLogBackupFileName, g_pszScoringLogBackupFilename, LTARRAYSIZE(szScoringLogBackupFileName));

			CLTFileWrite cFileWrite;
			if (!cFileWrite.Open(szScoringLogBackupFileName, false))
			{
				return false;
			}

			if (!cFileWrite.Write(pFileData, (uint32)nFileSize))
			{
				return false;
			}

			// cleanup the buffer
			delete [] pFileData;

			// delete the existing file and buffer
			cFileRead.Close();

			if (!LTFileOperations::DeleteFile(szScoringLogFileName))
			{
				return false;
			}

			bWriteLogHeader = true;
		}
	}
	else
	{
		bWriteLogHeader = true;
	}

	if (bWriteLogHeader)
	{
		// write the current timestamp
		time_t currentTime;
		time(&currentTime);

		char szTime[g_nScoringTimeBufferSize] = { 0 };
		LTSNPrintF(szTime, LTARRAYSIZE(szTime), "%10d" FILE_NEWLINE, currentTime);

		CLTFileWrite cFileWrite;
		if (!cFileWrite.Open(szScoringLogFileName, false))
		{
			return false;
		}
	
		if (!cFileWrite.Write(szTime, LTStrLen(szTime)))
		{
			return false;
		}
	}

	// open the file member (in append mode) and write an entry for server startup
	m_cMultiplayerScoringLogFile.Open(szScoringLogFileName, true);
	WriteMultiplayerScoringLogEntry("");
	WriteMultiplayerScoringLogEntry("------------------------------------------");
	WriteMultiplayerScoringLogEntry("Server started.");
	
	// success
	return true;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::WriteMultiplayerScoringLogEntry
//
//	PURPOSE:	Write an entry to the scoring log
//
// ----------------------------------------------------------------------- //

void CGameServerShell::WriteMultiplayerScoringLogEntry(const char* pszEntry)
{
	LTASSERT(LTStrLen(pszEntry) < g_nMaximumEntryLength, "scoring log entry exceeds maximum size");

	char szEntryBuffer[g_nScoringEntryBufferSize] = {0};

	// get the current time
	char szTimeBuffer[g_nScoringTimeBufferSize];
	time_t currentTime;
	time(&currentTime);
	LTStrCpy(szTimeBuffer, ctime(&currentTime), LTARRAYSIZE(szTimeBuffer));

	// remove newline
	szTimeBuffer[LTStrLen(szTimeBuffer) - 1] = 0;

	// write the entry
	LTSNPrintF(szEntryBuffer, LTARRAYSIZE(szEntryBuffer), "[%s] %s" FILE_NEWLINE, szTimeBuffer, pszEntry);
	m_cMultiplayerScoringLogFile.Write(szEntryBuffer, LTStrLen(szEntryBuffer));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::BuildDownloadableArchiveFilesString
//
//	PURPOSE:	Builds a string containing the GUIDs and sizes of all
//			    downloadable archive files in use by the server.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::BuildDownloadableArchiveFilesString(std::string& strDownloadableArchiveFiles)
{
	char szDownloadableArchiveFiles[g_nMaxDownloadableFileStringLength];
	::memset(szDownloadableArchiveFiles, 0, LTARRAYSIZE(szDownloadableArchiveFiles));

	// get the number of non-retail resource files from the engine
	uint32 nNumDownloadableArchiveFiles = 0;
	g_pLTServer->GetNumDownloadableArchiveFiles(nNumDownloadableArchiveFiles);

	// if there are non-retail files, build a string containing the GUIDs and sizes 
	if (nNumDownloadableArchiveFiles > 0)
	{
		// first part of the string is the number of files
		LTSNPrintF(szDownloadableArchiveFiles, LTARRAYSIZE(szDownloadableArchiveFiles), "%d%s", 
				   nNumDownloadableArchiveFiles, 
				   g_pszDownloadableFileCountSeparator);

		for (uint32 nIndex = 0; nIndex < nNumDownloadableArchiveFiles; ++nIndex)
		{
			LTGUID cGUID;
			uint64 nSize;

			// get the GUID and size of this file from the engine
			if (g_pLTServer->GetDownloadableArchiveFileInfo(nIndex, cGUID, nSize) != LT_OK)
			{
				return false;
			}

			// get the GUID in string form
			char szGUIDString[LTGUID_STRING_SIZE+1];
			LTGUIDToString(szGUIDString, LTARRAYSIZE(szGUIDString), cGUID);
			
			// append the size
			char szEntryString[g_nMaxDownloadableFileEntryLength + 1];
			LTSNPrintF(szEntryString, LTARRAYSIZE(szEntryString), "%s%s%8d%s", 
					   szGUIDString, 
					   g_pszDownloadableFileSizeSeparator,
					   (uint32)nSize,
					   g_pszDownloadableFileEntrySeparator);

			// add this entry to the list
			LTStrCat(szDownloadableArchiveFiles, szEntryString, LTARRAYSIZE(szDownloadableArchiveFiles));
		}
	}

	strDownloadableArchiveFiles = szDownloadableArchiveFiles;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetLGFlags
//
//	PURPOSE:	Set the load game flags for...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetLGFlags( uint8 nLGFlags )
{
	m_nLastLGFlags = nLGFlags;
	
	if( g_pVersionMgr )
		g_pVersionMgr->SetLGFlags( m_nLastLGFlags );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetUniqueObjectID
//
//	PURPOSE:	Set the unique object ID, but only if it is higher 
//				than the current ID.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetUniqueObjectID( uint32 nID )
{
	// Only set the unique ID if it is higher than the currently 
	// existing value.  This allows the ID to remain unique as the 
	// Player transitions across TransAms.

	if( nID > m_nUniqueObjectID )
		m_nUniqueObjectID = nID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::CreatePlayer( GameClientData* pGameClientData )
{
	ObjectCreateStruct theStruct;

	theStruct.m_Rotation.Init();
	theStruct.m_Pos.Init();
	theStruct.m_Flags = 0;

	HCLASS hClass = g_pLTServer->GetClass("CPlayerObj");

	CPlayerObj* pPlayer = NULL;
	if (hClass)
	{
		theStruct.m_UserData = ( uint32 )pGameClientData;
		pPlayer = (CPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
	}

	return pPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ProcessPacket
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTRESULT CGameServerShell::ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort)
{
	// Track the current execution shell scope for proper SEM behavior
	CServerShellScopeTracker cScopeTracker;

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

		// Read the data out of the message.
		uint32 nDataLen = pMsg->Size() / 8;
		uint8* pData = ( uint8* )alloca( nDataLen );
		pMsg->ReadData( pData, nDataLen * 8 );

	// check for a PunkBuster packet
	if (m_pPunkBusterServer)
	{
		// See if the PunkBuster server wants it.
		if (m_pPunkBusterServer->HandleNetMessage(pData, nDataLen, senderAddr, senderPort))
		{
			return LT_OK;
		}
	}
	
	// Check if we have a gamespy server.
	if( m_pGameSpyServer )
	{
		// See if the gamespyserver wants it.
		m_pGameSpyServer->HandleNetMessage( pData, nDataLen, senderAddr, senderPort );
	}

#endif // !PLATFORM_XENON

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGameServerShell::SetSimulationTimerScale
//
//  PURPOSE:	Accessor the simulation timer scale.  All simulation timer changes should go through these
//				accessores rather than the engine directly.
// ----------------------------------------------------------------------- //
bool CGameServerShell::SetSimulationTimerScale( uint32 nNumerator, uint32 nDenominator )
{
	uint32 nAdjustedNumerator = LTMAX( nNumerator, 0 );
	uint32 nAdjustedDenominator = LTMAX( nDenominator, 0 );

	if( !SimulationTimer::Instance( ).SetTimerTimeScale( nAdjustedNumerator, nAdjustedDenominator ))
	{
		LTERROR( "CGameServerShell::SetSimulationTimerScale - Invalid timer information." );
		return false;
	}

	// Tell the current clients about the change.
	if( !SendSimulationTimerScale( NULL ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SendSimulationTimerScale
//
//	PURPOSE:	Sends the simulation timer scale to the client.
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::SendSimulationTimerScale( HCLIENT hClient )
{
	uint32 nNumerator;
	uint32 nDenominator;
	if( !SimulationTimer::Instance().GetTimerTimeScale( nNumerator, nDenominator ))
		return false;
    
	// Check if the scale is set to real time, which means we don't have to 
	// send the exact data down.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SIMULATION_TIMER );
	bool bRealTime = ( nNumerator == nDenominator );
	cMsg.Writebool( bRealTime );
	if( !bRealTime )
	{
		LTASSERT( nNumerator == ( uint16 )nNumerator, "CGameServerShell::SendSimulationTimerScale - Invalid timer scale." );
		LTASSERT( nDenominator == ( uint16 )nDenominator, "CGameServerShell::SendSimulationTimerScale - Invalid timer scale." );
		cMsg.Writeuint16(( uint16 )nNumerator );
		cMsg.Writeuint16(( uint16 )nDenominator );
	}
	g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED);
	
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetMaxClientsCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the maximum number
//				of clients allowed on the server.
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetMaxClientsCallback(int& nMaxClients)
{
	// get the maximum number of clients from GameModeMgr
	nMaxClients = GameModeMgr::Instance().m_grnMaxPlayers;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServertShell::PunkBusterGetServerAddressCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the server's
//				address.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetServerAddressCallback(char* pszServerAddressBuffer, int nServerAddressBufferLength)
{

	char szIpAddr[16];
	uint16 nPort = 0;
	g_pLTServer->GetTcpIpAddress( szIpAddr, ARRAY_LEN( szIpAddr ), nPort );

	LTSNPrintF( pszServerAddressBuffer, nServerAddressBufferLength, "%s:%d", szIpAddr, nPort );

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterDropClientsCallback()
//
//	PURPOSE:	PunkBuster callback function for dropping a player.
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterDropClientCallback(const int nIndex, const char* pszReason)
{
	// call ServerConnectionMgr to get the client data
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nIndex);
	if (!pGameClientData)
	{
		// no client data for this client Id
		return;
	}

	// tell ServerConnectionMgr to boot the client
	ServerConnectionMgr::Instance().BootWithReason(*pGameClientData, eClientConnectionError_PunkBuster, pszReason);
}	

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetClientStatsCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving client stats.
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetClientStatsCallback(const int nIndex, char* pszStatsBuffer, int nStatsBufferLength)
{
	*pszStatsBuffer = 0;

	// call ServerConnectionMgr to get the client data
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nIndex);
	if (!pGameClientData)
	{
		// no client data for this client Id
		return;
	}

	// get the player object for this client
	CPlayerObj* pPlayerObj = GetPlayerFromClientId(nIndex);
	if (!pPlayerObj)
	{
		// no player for this client Id
		return;
	}

	// get the player score data and the team Id
	CPlayerScore* pPlayerScore = pGameClientData->GetPlayerScore();
	uint8 nTeamId = pPlayerObj->GetTeamID();

	// build the stats string
	LTSNPrintF(pszStatsBuffer, nStatsBufferLength, "team=%d score=%d deaths=%d tk=%ld", 
			   nTeamId,
			   pPlayerScore->GetScore(),
			   pPlayerScore->GetEventCount(CPlayerScore::eDeath),
			   pPlayerScore->GetEventCount(CPlayerScore::eTeamKill));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetGameVersionCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the game
//				version string.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetGameVersionCallback(char* pszGameVersionBuffer, int nGameVersionBufferLength)
{
	LTStrCpy(pszGameVersionBuffer, g_pVersionMgr->GetNetVersion(), nGameVersionBufferLength);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetClientInfoCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving information about
//				about a specific client.
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetClientInfoCallback(const int nIndex, 
													   char* pszNameBuffer, int nNameBufferLength, 
													   char* pszGUIDBuffer, int nGUIDBufferLength, 
													   char* pszAddrBuffer, int nAddrBufferLength)

{
	*pszNameBuffer = 0;
	*pszGUIDBuffer = 0;
	*pszAddrBuffer = 0;

	// call ServerConnectionMgr to get the client data
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nIndex);
	if (!pGameClientData)
	{
		// no client data for this client Id
		return;
	}

	// client must be in the world before we can report information
	if (!pGameClientData->IsClientInWorld())
	{
		return;
	}

	// copy the player name
	LTStrCpy(pszNameBuffer, MPW2A(pGameClientData->GetUniqueName()).c_str(), nNameBufferLength);

	// copy the unique CD key hash
	const char* pszUserCDKeyHash = NULL;
	g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(nIndex, pszUserCDKeyHash);
	LTStrCpy(pszGUIDBuffer, pszUserCDKeyHash, nGUIDBufferLength);

	// copy the IP address
	uint8  aAddressBuffer[4];
	uint16 nPort;
	g_pLTServer->GetClientAddr(pGameClientData->GetClient(), aAddressBuffer, &nPort);
	LTSNPrintF(pszAddrBuffer, nAddrBufferLength, "%d.%d.%d.%d:%d",
			   aAddressBuffer[0], aAddressBuffer[1], aAddressBuffer[2], aAddressBuffer[3], nPort);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetSocketCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving server socket.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetSocketCallback(SOCKET *sock)
{
	if (g_pLTServer->GetUDPSocket(*sock) != LT_OK)
	{
		*sock = INVALID_SOCKET;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterSendGameMessageCallback()
//
//	PURPOSE:	PunkBuster callback function for sending a game-stream network
//              message.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterSendGameMessageCallback(const int nIndex, char *data, int datlen)
{
	// call ServerConnectionMgr to get the client data
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nIndex);
	if (!pGameClientData)
	{
		// no client data for this client Id
		return;
	}

	// build the message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PUNKBUSTER_MSG);
	cMsg.WriteData(data, datlen*8);

	// send it to the client (unguaranteed, since PunkBuster manages retries internally)
	g_pLTServer->SendToClient(cMsg.Read(), pGameClientData->GetClient(), 0);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterDisplayMessageCallback()
//
//	PURPOSE:	PunkBuster callback function for displaying a HUD message.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterDisplayMessageCallback(char *data, int datlen)
{
	// copy the string
	uint32 nDataCopyLength = LTStrLen(data);
	char* pDataCopy = (char*)alloca(nDataCopyLength + 1);
	LTStrCpy(pDataCopy, data, nDataCopyLength + 1);

	// convert CR characters to spaces
	for (uint32 nIndex = 0; nIndex < nDataCopyLength; ++nIndex)
	{
		if (pDataCopy[nIndex] == '\r')
		{
			pDataCopy[nIndex] = ' ';
		}
	}

	g_pLTServer->CPrint ( data ) ;

	if ( g_pGameServerShell->GetPunkBusterServer() )
		g_pGameServerShell->GetPunkBusterServer()->CaptureConsoleOutput ( data, datlen ) ;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetMapNameCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the current map name.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetMapNameCallback(char *data, int datlen)
{

	static char level[256];

	LTFileOperations::SplitPath( g_pGameServerShell->GetCurLevel(), NULL, level, NULL );
	strncpy ( data, level, datlen );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetServerHostNameCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the server hostname.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetServerHostNameCallback(char *data, int datlen)
{
	strncpy ( data, MPW2A( GameModeMgr::Instance( ).m_grwsSessionName ).c_str( ), datlen ) ;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetGameNameCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the game/mod name.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetGameNameCallback(char *data, int datlen)
{
	LTStrCpy(data, g_pGameServerShell->m_NetGameInfo.m_sModName.c_str(), datlen);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetConsoleVariableCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the game/mod name.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterGetConsoleVariableCallback(const char* pszName, char* pszValueBuffer, int nValueBufferLength)
{
	*pszValueBuffer = 0;

	if (!pszName)
	{
		return;
	}

	HCONSOLEVAR hVariable = g_pLTServer->GetConsoleVariable(pszName);
	if (hVariable)
	{	
		LTStrCpy(pszValueBuffer, g_pLTServer->GetConsoleVariableString(hVariable), nValueBufferLength);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterRestartMapCallback()
//
//	PURPOSE:	PunkBuster callback function for restarting the current map.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterRestartMapCallback()
{
	if (!g_pServerMissionMgr)
	{
		return;
	}

	// get the current mission world
	const char* pszCurrentWorld = g_pServerMissionMgr->GetLevelFromMission(g_pServerMissionMgr->GetCurrentMission(), g_pServerMissionMgr->GetCurrentLevel());

	// tell CServerMissionMgr to switch to it
	g_pServerMissionMgr->ExitLevelToLevel(pszCurrentWorld);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterChangeMapCallback()
//
//	PURPOSE:	PunkBuster callback function for changing to a new map.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterChangeMapCallback(const char* pszName)
{
	if (!g_pServerMissionMgr)
	{
		return;
	}

	// switch to the new level
	g_pServerMissionMgr->ExitLevelToLevel(pszName);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterHandleAdminCommandCallback()
//
//	PURPOSE:	PunkBuster callback function for handling admin (scmd) commands.
//
//
// --------------------------------------------------------------------------- //


void CGameServerShell::PunkBusterHandleAdminCommandCallback(const char* pszCommand)
{
	// parse the command
	ConParseW cParse(MPA2W(pszCommand).c_str());
	if (LT_OK == g_pCommonLT->Parse(&cParse))
	{
		// convert it to a messasge
		CParsedMsgW cParsedMsg(cParse.m_nArgs, cParse.m_Args);

		// send it to the SCMD console
		ScmdConsole::Instance().SendParsedCommand(cParsedMsg);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterHandleAdminCommandCallback()
//
//	PURPOSE:	PunkBuster callback function for locking a player into spectator
//				mode.
//
//
// --------------------------------------------------------------------------- //

void CGameServerShell::PunkBusterForceSpectatorModeCallback(const int nIndex, bool bForceSpectatorMode)
{
	if (!g_pGameServerShell)
	{
		return;
	}

	// get the player associated with this slot
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nIndex);
	if (!pGameClientData)
	{
		// no client data for this client Id
		return;
	}

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pGameClientData->GetPlayer());
	if (!pPlayer)
	{
		// no player object
		return;
	}

	// set the lock spectator flag
	pPlayer->SetLockSpectatorMode(bForceSpectatorMode);

	// if we're forcing to spectator mode make sure the player is in spectator, otherwise
	// force them to the alive state
	if (bForceSpectatorMode)
	{
		if (!pPlayer->IsSpectating())
		{
			pPlayer->SetSpectatorMode(eSpectatorMode_Fixed, true);
		}
	}
	else
	{
		pPlayer->Respawn();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::EnterSlowMo()
//
//	PURPOSE:	Go into slowmo mode.
//
// --------------------------------------------------------------------------- //
bool CGameServerShell::EnterSlowMo( HRECORD hSlowMoRecord, float fSlowMoCharge, HCLIENT hActivator, uint32 nSlowMoFlags )
{
	// See if a valid slowmo was specified.
	if( !hSlowMoRecord )
	{
		return false;
	}

	// Don't enter slow-mo if the game mode doesn't use it...
	if( !GameModeMgr::Instance( ).m_grbUseSlowMo )
		return false;

	if( fSlowMoCharge <= -1.0f )
		fSlowMoCharge = GETCATRECORDATTRIB( SlowMo, hSlowMoRecord, Period );

	if( IsMultiplayerGameServer())
	{
		// Get the activator object.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( hActivator );
		if( !pGameClientData )
			return false;

		m_nSlowMoActivatorTeamId = pGameClientData->GetLastTeamId( );
	}

	// Start the timer.
	m_SlowMoTimer.Start( fSlowMoCharge );

	// Change the time scales.
	LTVector2 v2SimulationTimeScale = GETCATRECORDATTRIB( SlowMo, hSlowMoRecord, SimulationTimeScale );
	g_pGameServerShell->SetSimulationTimerScale(( uint32 )v2SimulationTimeScale.x, ( uint32 )v2SimulationTimeScale.y );

	// Store the record while were in it.
	m_hSlowMoRecord = hSlowMoRecord;

	// Each player is affected by slow-mo in some way, run through them all.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	for( ; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pPlayerObj = *iter;

		// Set everyone to use the same type and amount of slow-mo...
		pPlayerObj->EnterSlowMo( m_hSlowMoRecord, hActivator, m_nSlowMoActivatorTeamId, nSlowMoFlags );
	}


	// Start the slowmo effect to all clients.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SLOWMO );
	cMsg.Writeuint8( kSlowMoStart );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, GetSlowMoRecord( ));
	cMsg.Writebool( !!( nSlowMoFlags & CPlayerObj::kTransition ));
	cMsg.Writedouble( GetSlowMoTimer( ).GetDuration( ));
	cMsg.Writebool( !!( nSlowMoFlags & CPlayerObj::kPlayerControlled ));
	cMsg.Writeuint32( g_pLTServer->GetClientID( hActivator ));
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ExitSlowMo()
//
//	PURPOSE:	Exit slowmo state.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ExitSlowMo( bool bDoTransition, float fCharge )
{
	// Check if we're not in slowmo.
	if( !IsInSlowMo( ))
		return;

	// Exit slowmo.
	m_SlowMoTimer.Stop( );

	// Change the time scales.
	g_pGameServerShell->SetSimulationTimerScale( 1, 1 );

	// Told to turn off.  If the slowmo has a transition period, then wait to fully exit 
	// until after the transition period.  Otherwise, kill it now.
	float fTransitionPeriod = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, TransitionPeriod );
	if (fCharge > 0.0f && fCharge < fTransitionPeriod)
	{
		fTransitionPeriod = fCharge;
	}
	if( bDoTransition && fTransitionPeriod <= 0.0f )
	{
		bDoTransition = false;
	}

	// Start a timer for the remainder of the slowmo.
	if( bDoTransition )
		m_SlowMoTimer.Start( fTransitionPeriod );

	for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pPlayerObj = *iter;
		if( !pPlayerObj )
			continue;

		pPlayerObj->ExitSlowMo( bDoTransition );
	}

	m_hSlowMoRecord = NULL;

	// End the slowmo effect.
//	DebugCPrint(0,"%s - sending MID_SLOWMO,kSlowMoEnd", __FUNCTION__);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SLOWMO );
	cMsg.Writeuint8( kSlowMoEnd );
	cMsg.Writebool( bDoTransition );
	cMsg.Writefloat( fCharge );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateSlowMo()
//
//	PURPOSE:	Update our slowmo state.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::UpdateSlowMo()
{
	// Check if we're not in slowmo.
	if( !IsInSlowMo( ))
		return;

	// Check if we're not timed out yet.
	if( m_SlowMoTimer.GetDuration() <= 0.0f || !m_SlowMoTimer.IsTimedOut( ))
		return;

	// Leave slowmo state.
	ExitSlowMo( true, 0.0f );
}

static void BuildSendInstantDamageTypeList( CGameServerShell::InstantDamageTypes& lstInstantDamageTypes )
{
	// Clear list so it starts fresh.
	lstInstantDamageTypes.clear( );

	// Iterate through all the damagefx and see if they have attributes that the client would use when damage occurs.
	HCATEGORY hCategory = DATABASE_CATEGORY( DamageFxDB ).GetCategory();
	uint32 nNumRecords = DATABASE_CATEGORY( DamageFxDB ).GetNumRecords();
	for( uint32 nRec = 0; nRec < nNumRecords; nRec++ )
	{
		HRECORD hRec = DATABASE_CATEGORY( DamageFxDB ).GetRecordByIndex( nRec );
		char const* pszFirstPersonInstantFxName = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB( hRec, FirstPersonInstantFX );
		if( pszFirstPersonInstantFxName && pszFirstPersonInstantFxName[0] )
		{
			HRECORD hDT = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hRec,DamageType);
			uint32 nDamageFlag = g_pDTDB->GetDamageFlag(hDT);
			lstInstantDamageTypes.push_back( nDamageFlag );
			continue;
		}
		char const* pszThirdPersonInstantFxName = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB( hRec, ThirdPersonInstantFX );
		if( pszThirdPersonInstantFxName && pszThirdPersonInstantFxName[0] )
		{
			HRECORD hDT = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hRec,DamageType);
			uint32 nDamageFlag = g_pDTDB->GetDamageFlag(hDT);
			lstInstantDamageTypes.push_back( nDamageFlag );
			continue;
		}
	}
}

bool CGameServerShell::ShouldSendInstantDamageToClient( uint16 nInstantDamageType )
{
	for( InstantDamageTypes::iterator iter = m_lstSendInstantDamageTypes.begin( ); 
		iter != m_lstSendInstantDamageTypes.end( ); iter++ )
	{
		if( nInstantDamageType == *iter )
			return true;
	}

	return false;
}

static void BuildSendDeathDamageTypeList( CGameServerShell::DeathDamageTypes& lstDeathDamageTypes )
{
	// Clear list so it starts fresh.
	lstDeathDamageTypes.clear( );

	// Iterate through all the damagefx and see if they have attributes that the client would use when damage occurs.
	HCATEGORY hCategory = DATABASE_CATEGORY( DamageFxDB ).GetCategory();
	uint32 nNumRecords = DATABASE_CATEGORY( DamageFxDB ).GetNumRecords();
	for( uint32 nRec = 0; nRec < nNumRecords; nRec++ )
	{
		HRECORD hRec = DATABASE_CATEGORY( DamageFxDB ).GetRecordByIndex( nRec );
		char const* pszFirstPersonDeathFxName = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB( hRec, FirstPersonDeathFX );
		if( pszFirstPersonDeathFxName && pszFirstPersonDeathFxName[0] )
		{
			HRECORD hDT = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hRec,DamageType);
			uint32 nDamageFlag = g_pDTDB->GetDamageFlag(hDT);
			lstDeathDamageTypes.push_back( nDamageFlag );
			continue;
		}
	}
}

bool CGameServerShell::ShouldSendDeathDamageToClient( uint16 nDeathDamageType )
{
	for( DeathDamageTypes::iterator iter = m_lstSendDeathDamageTypes.begin( ); 
		iter != m_lstSendDeathDamageTypes.end( ); iter++ )
	{
		if( nDeathDamageType == *iter )
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::RemoveObjectsBasedOnClientSettings()
//
//	PURPOSE:	Removes object that should not be in the world due to client settings...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::RemoveObjectsBasedOnClientSettings( )
{
	// Don't remove anything in multiplayer games...
	if( IsMultiplayerGameServer( ))
		return;

	// Run through each object and test it's settings against the clients settings for removal conditions...
	HOBJECT hObject = g_pLTServer->GetNextObject( INVALID_HOBJECT );
	while( hObject != INVALID_HOBJECT )
	{
		if( ShouldRemoveObjectBasedOnClientSettings( hObject ))
			g_pLTServer->RemoveObject( hObject );

		hObject = g_pLTServer->GetNextObject( hObject );
	}

	// Now the inactive objects must be tested...
	hObject = g_pLTServer->GetNextInactiveObject( INVALID_HOBJECT );
	while( hObject != INVALID_HOBJECT )
	{
		if( ShouldRemoveObjectBasedOnClientSettings( hObject ))
			g_pLTServer->RemoveObject( hObject );

		hObject = g_pLTServer->GetNextInactiveObject( hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ShouldRemoveObjectBasedOnClientSettings()
//
//	PURPOSE:	Determines if objects should be removed based on client settings...
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::ShouldRemoveObjectBasedOnClientSettings( HOBJECT hObject )
{
	if( hObject == INVALID_HOBJECT )
		return false;

	uint32 nUserFlags = 0;
	g_pCommonLT->GetObjectFlags( hObject, OFT_User, nUserFlags );

	return ShouldRemoveBasedOnClientSettings( nUserFlags );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ShouldRemoveBasedOnClientSettings()
//
//	PURPOSE:	Determines if an object, with the specified user settings,
//				should be in the world due to client settings...
//
// ----------------------------------------------------------------------- //

bool CGameServerShell::ShouldRemoveBasedOnClientSettings( uint32 nUserFlags )
{
	// The gore setting takes priority over detail settings...
	if( !m_bGoreAllowed && (nUserFlags & USRFLG_GORE ))
		return true;

	EEngineLOD eObjectLOD = UserFlagToObjectLOD( nUserFlags );
	if( eObjectLOD > m_eWorldObjectsLOD )
		return true;

	return false;
}

// EOF
