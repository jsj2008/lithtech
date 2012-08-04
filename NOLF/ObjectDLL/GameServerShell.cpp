// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// CREATED : 9/17/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "CommandIDs.h"
#include "ServerUtilities.h"
#include "CachedFiles.h"
#include "Trigger.h"
#include "Sparam.h"
#include "ObjectMsgs.h"
#include <stdio.h>
#include "DialogueWindow.h"
#include "CSDefs.h"
#include "AssertMgr.h"
#include "WorldProperties.h"
#include "GameBase.h"
#include "MusicMgr.h"
#include "Camera.h"

#include "CRC32.h"

#define MAX_CLIENT_NAME_LENGTH		100
#define CLIENT_PING_UPDATE_RATE		3.0f
#define UPDATENAME_INTERVAL			10.0f

#define GAME_VERSION				1

#define GAMESPY_GAME_NAME			"nolf"
#define GAMESPY_SECRET_KEY			"Jn3Ab4"

#define DEFAULT_PORT				27888


// Stuff to create a GameServerShell.

SETUP_SERVERSHELL()

CDialogueWindow		g_DialogueWindow;
CVarTrack			g_CanShowDimsTrack;
CVarTrack			g_ShowDimsTrack;
CVarTrack			g_ShowTimingTrack;
CVarTrack			g_ShowTriggersTrack;
CVarTrack			g_ShowTriggersFilter;
CVarTrack			g_DamageScale;
CVarTrack			g_HealScale;

CVarTrack			g_MaxPlayers;
CVarTrack			g_NetDMGameEnd;
CVarTrack			g_NetCAGameEnd;
CVarTrack			g_NetEndTime;
CVarTrack			g_NetEndFrags;
CVarTrack			g_NetEndScore;
CVarTrack			g_NetUseSpawnLimit;
CVarTrack			g_NetSpawnLimit;
CVarTrack			g_NetFragScore;
CVarTrack			g_NetAudioTaunts;
CVarTrack			g_NetArmorHealthPercent;

CGameServerShell*   g_pGameServerShell = LTNULL;

IServerShell* CreateServerShell(ILTServer *pServerDE)
{
	//AfxSetAllocStop(29048);

	// Redirect our asserts as specified by the assert convar

	CAssertMgr::Enable();

	// Set our global pointer

    g_pLTServer = pServerDE;

	// Set up the LT subsystem pointers

    g_pMathLT = g_pLTServer->GetMathLT();
    g_pModelLT = g_pLTServer->GetModelLT();
    g_pTransLT = g_pLTServer->GetTransformLT();
    g_pPhysicsLT = g_pLTServer->Physics();
	g_pBaseLT = static_cast<ILTCSBase*>(g_pLTServer);
	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

	// Set up global console trackers...

    g_CanShowDimsTrack.Init(g_pLTServer, "CanShowDims", LTNULL, 0.0f);
    g_ShowDimsTrack.Init(g_pLTServer, "ShowDims", LTNULL, 0.0f);
    g_ShowTimingTrack.Init(g_pLTServer, "ShowTiming", "0", 0.0f);
    g_ShowTriggersTrack.Init(g_pLTServer, "ShowTriggers", "0", 0.0f);
    g_ShowTriggersFilter.Init(g_pLTServer, "FilterTriggerMsgs", "", 0.0f);
    g_DamageScale.Init(g_pLTServer, "DamageScale", LTNULL, 1.0f);
    g_HealScale.Init(g_pLTServer, "HealScale", LTNULL, 1.0f);
    g_MaxPlayers.Init(g_pLTServer, "NetMaxPlayers", LTNULL, (LTFLOAT)(MAX_MULTI_PLAYERS_DISPLAY));
    g_NetDMGameEnd.Init(g_pLTServer,"NetDMGameEnd",LTNULL,0.0f);
    g_NetCAGameEnd.Init(g_pLTServer,"NetCAGameEnd",LTNULL,1.0f);
    g_NetEndTime.Init(g_pLTServer,"NetEndTime",LTNULL,15.0f);
    g_NetEndFrags.Init(g_pLTServer,"NetEndFrags",LTNULL,25.0f);
    g_NetEndScore.Init(g_pLTServer,"NetEndScore",LTNULL,50.0f);
    g_NetUseSpawnLimit.Init(g_pLTServer,"NetUseSpawnLimit",LTNULL,0.0f);
    g_NetSpawnLimit.Init(g_pLTServer,"NetSpawnLimit",LTNULL,5.0f);
    g_NetFragScore.Init(g_pLTServer,"NetFragScore",LTNULL,1.0f);
	g_NetAudioTaunts.Init(g_pLTServer,"NetAudioTaunts",LTNULL,1.0f);
	g_NetArmorHealthPercent.Init(g_pLTServer, "NetArmorHealthPercent", LTNULL, 0.0f);

	// Make sure we are using autodeactivation...

    g_pLTServer->RunGameConString("autodeactivate 1.0");

	CGameServerShell *pShell = debug_new1(CGameServerShell, pServerDE);

	g_pGameServerShell = pShell;

    return (IServerShell*)pShell;
}

void DeleteServerShell(IServerShell *pInputShell)
{
	CGameServerShell *pShell = (CGameServerShell*)pInputShell;

    // g_pGameServerShell = LTNULL;
	// (kls - 2/22/98 - CreateSeverShell() is called BEFORE
	// DeleteServerShell() is called so we CAN'T set this to NULL)

	debug_delete(pShell);

	// Return the report hook to the normal CRT function

	CAssertMgr::Disable();
}

LTBOOL g_bInfiniteAmmo = LTFALSE;
LTBOOL g_bRobert = LTFALSE;




// ----------------------------------------------------------------------- //
// Helpers for world time conversion.
// ----------------------------------------------------------------------- //

inline float TODSecondsToHours(float time)
{
	return (float)(time / 3600.0);
}

inline float TODHoursToSeconds(float time)
{
	return (float)(time * 3600.0);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CGameServerShell::CGameServerShell(ILTServer *pServerDE)
{
	memset(&m_GameInfo, 0, sizeof(NetGame));

	m_nCurLevel			 = 0;

	ClearClientList();
	SetUpdateGameServ();

    m_bGameServHosted = LTFALSE;
	m_WorldTimeColor[0] = m_WorldTimeColor[1] = m_WorldTimeColor[2] = MAX_WORLDTIME_COLOR;
	m_SunVec[0] = m_SunVec[1] = m_SunVec[2] = 0;

	SetupGameInfo();

	m_nLastLGFlags  = LOAD_NEW_GAME;
    m_hSwitchWorldVar = LTNULL;
    m_bFirstUpdate = LTTRUE;

	m_TODSeconds = TODHoursToSeconds(12.0f);
    m_TODCounter = 0.0f;
	m_ClientPingSendCounter = 0.0f;
	m_iPrevRamp = 0;

    m_bUseMissionData = LTFALSE;

	m_bFadeBodies = LTTRUE;

	m_bDontUseGameSpy = LTFALSE;
	m_sWorld[0] = '\0';

	// Initialize all the globals...

	m_GlobalMgr.Init();

	if (GetGameType() == SINGLE)
	{
		m_bDontUseGameSpy = LTTRUE;
	}
    else
	{
        m_ServerOptionMgr.Init(g_pLTServer);
		ReadNetHostSettings( );
	}
	m_bShowMultiplayerSummary = LTFALSE;
	m_bStartNextMultiplayerLevel = LTFALSE;
	m_eLevelEnd = LE_UNKNOWN;
	m_fSummaryEndTime = 0.0f;
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
	if (GetGameType() != SINGLE)
	{
		// Term the server option manager.
		m_ServerOptionMgr.Term();

		// Term GameSpy.
		TermGameSpy();
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
	g_pLTServer->CPrint("OnAddClient");

	// Tell serverapp about it.
	ServerAppAddClient( hClient );
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
	// Clear our client data...

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);
	if (pPlayer)
	{
        pPlayer->SetClient(LTNULL);
	}

	// Tell server app about it.
	ServerAppRemoveClient( hClient );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

LPBASECLASS	CGameServerShell::OnClientEnterWorld(HCLIENT hClient, void *pClientData,
                                                 uint32 clientDataLen)
{
    if (!hClient) return LTNULL;
	g_pLTServer->CPrint("OnClientEnterWorld");

    CPlayerObj* pPlayer = LTNULL;

	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);


	// Send them the current time of day color.

	SendTimeOfDay(hClient);


    LTBOOL bFoundClient = LTFALSE;
	char sClientName[MAX_CLIENT_NAME_LENGTH];
	char sClientRefName[MAX_CLIENT_NAME_LENGTH];
	sClientName[0] = sClientRefName[0] = '\0';


    g_pLTServer->GetClientName(hClient, sClientName, MAX_CLIENT_NAME_LENGTH-1);


	// Search through the client refs to see if any of them match our
	// client...
    HCLIENTREF hClientRef = g_pLTServer->GetNextClientRef(LTNULL);
	while (hClientRef)
	{
		// See if this client reference is local or not...

        if (g_pLTServer->GetClientRefInfoFlags(hClientRef) & CIF_LOCAL)
		{
            bFoundClient = LTTRUE;
		}


		// Determine if there is a reference to a client with the same
		// name...

        if (!bFoundClient && g_pLTServer->GetClientRefName(hClientRef, sClientRefName, MAX_CLIENT_NAME_LENGTH-1))
		{
			if (sClientName[0] && sClientRefName[0])
			{
				if (_stricmp(sClientName, sClientRefName) == 0)
				{
                    bFoundClient = LTTRUE;
				}
			}
		}


		// See if we found the right client...

		if (bFoundClient)
		{
            HOBJECT hObject = g_pLTServer->GetClientRefObject(hClientRef);
            pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);

			if (pPlayer)
			{
				RespawnPlayer(pPlayer, hClient);
			}
			break;
		}

        hClientRef = g_pLTServer->GetNextClientRef(hClientRef);
	}

	uint32 dwTeamID = TEAM_AUTO;
	char szPlayerName[MAX_PLAYER_NAME] = "";
	if (pClientData && clientDataLen == sizeof(NetClientData))
	{
		NetClientData* pNcd = (NetClientData*)pClientData;

		dwTeamID = pNcd->m_dwTeam;
		strcpy(szPlayerName,pNcd->m_sName);
	}
	if (GetGameType() == COOPERATIVE_ASSAULT)
	{
		// Add this client to the appropriate team...

		if (dwTeamID == TEAM_AUTO)
		{
			CTeam* pTeam = m_TeamMgr.GetTeamWithLeastPlayers(TRUE);
			if (pTeam)
			{
				dwTeamID = pTeam->GetID();
			}
			else
			{
				if (GetRandom(0,1) == 1) dwTeamID = TEAM_2;
				else dwTeamID = TEAM_1;
			}
		}

        uint32 dwTransTeamID = m_TeamMgr.GetTeamTransID(g_pLTServer->GetClientID(hClient));
		CTeam* pTeam         = m_TeamMgr.GetTeam(dwTransTeamID);

		if (dwTransTeamID != TM_ID_NULL && pTeam)
		{
			dwTeamID = dwTransTeamID;
		}

        m_TeamMgr.AddPlayer(dwTeamID, g_pLTServer->GetClientID(hClient));
	}



	// See if we need to create a player (no matches found)...

	if (!pPlayer)
	{
		pPlayer = CreatePlayer(hClient);
	}
	else
	{
		pPlayer->UpdateTeamID();
	}


    g_pLTServer->SetClientInfoFlags(hClient, CIF_AUTOACTIVATEOBJECTS);



	// Add this client to our local list...

	AddClientToList(hClient);
	SetUpdateGameServ();

	if (GetGameType() != SINGLE)
	{
		SendGameDataToClient(hClient);
		SendServerOptionsToClient(hClient);

	    uint32  nClientID   = g_pLTServer->GetClientID(hClient);
	    HSTRING hClientName = g_pLTServer->CreateString(szPlayerName);
		int  nTeam1Score = 0;
		int  nTeam2Score = 0;
		if (pPlayer->GetTeamID())
		{
			CTeam* pTeam = g_pGameServerShell->GetTeamMgr()->GetTeam(TEAM_1);
			nTeam1Score = pTeam->GetScore();
			pTeam = g_pGameServerShell->GetTeamMgr()->GetTeam(TEAM_2);
			nTeam2Score = pTeam->GetScore();
		}

		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_PLAYER_CONNECTED);
	    g_pLTServer->WriteToMessageHString(hWrite, hClientName);
		g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nClientID);
	    g_pLTServer->WriteToMessageByte(hWrite, (uint8)pPlayer->GetTeamID());
	    g_pLTServer->WriteToMessageFloat(hWrite, (float)nTeam1Score);
	    g_pLTServer->WriteToMessageFloat(hWrite, (float)nTeam2Score);

	    g_pLTServer->EndMessage(hWrite);

		g_pLTServer->FreeString(hClientName);

		// Remember the player's name for later use
		pPlayer->SetNetName(szPlayerName);
	}

	// All done...

	return pPlayer;
}


// ----------------------------------------------------------------------- //
// Sends a time of day message to the specified client(s).
// ----------------------------------------------------------------------- //
void CGameServerShell::SendTimeOfDay(HCLIENT hClient)
{
	HMESSAGEWRITE hWrite;
    hWrite = g_pLTServer->StartMessage(hClient, MID_TIMEOFDAYCOLOR);

    g_pLTServer->WriteToMessageByte(hWrite, m_WorldTimeColor[0]);
    g_pLTServer->WriteToMessageByte(hWrite, m_WorldTimeColor[1]);
    g_pLTServer->WriteToMessageByte(hWrite, m_WorldTimeColor[2]);
    g_pLTServer->WriteToMessageByte(hWrite, m_SunVec[0]);
    g_pLTServer->WriteToMessageByte(hWrite, m_SunVec[1]);
    g_pLTServer->WriteToMessageByte(hWrite, m_SunVec[2]);

    g_pLTServer->WriteToMessageFloat(hWrite, TODSecondsToHours(m_TODSeconds));

    g_pLTServer->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
// Sends a game data message to the specified client.
// ----------------------------------------------------------------------- //
void CGameServerShell::SendGameDataToClient(HCLIENT hClient)
{
    void *pData = g_pLTServer->GetClientUserData(hClient);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;



	HMESSAGEWRITE hWrite;
    hWrite = g_pLTServer->StartMessage(hClient, MID_MULTIPLAYER_DATA);

    g_pLTServer->WriteToMessageByte(hWrite, (uint8)GetGameType());

	if (GetGameType() != SINGLE)
	{
		// Get our ip address and port...
		char sBuf[32];
		int  nBufSize = 30;
		WORD wPort = 0;
		g_pLTServer->GetTcpIpAddress(sBuf, nBufSize, wPort);
	    g_pLTServer->WriteToMessageString(hWrite, sBuf);
	    g_pLTServer->WriteToMessageDWord(hWrite,(uint32) wPort);
	}

    uint32 dwTeam = 0;
	if (pPlayer)
	{
		pPlayer->UpdateTeamID();
		dwTeam = pPlayer->GetTeamID();
	}
	int nNumObj = m_Objectives[0].nNumObjectives + m_Objectives[dwTeam].nNumObjectives;

	//send total number of objectives
    g_pLTServer->WriteToMessageByte(hWrite, (uint8)nNumObj);

	//send list of general objectives
	for (int i = 0; i < m_Objectives[0].nNumObjectives; i++)
	{
        g_pLTServer->WriteToMessageDWord(hWrite, (uint32)m_Objectives[0].dwObjectives[i]);
	}
	//send list of team objectives
	for ( i = 0; i < m_Objectives[dwTeam].nNumObjectives; i++)
	{
        g_pLTServer->WriteToMessageDWord(hWrite, (uint32)m_Objectives[dwTeam].dwObjectives[i]);
	}

	//send total number of completed objectives
	nNumObj = m_CompletedObjectives[0].nNumObjectives + m_CompletedObjectives[dwTeam].nNumObjectives;
    g_pLTServer->WriteToMessageByte(hWrite, (uint8)nNumObj);

	//send list of general completed objectives
	for (i = 0; i < m_CompletedObjectives[0].nNumObjectives; i++)
	{
        g_pLTServer->WriteToMessageDWord(hWrite, (uint32)m_CompletedObjectives[0].dwObjectives[i]);
	}
	//send list of team completed objectives
	for (i = 0; i < m_CompletedObjectives[dwTeam].nNumObjectives; i++)
	{
        g_pLTServer->WriteToMessageDWord(hWrite, (uint32)m_CompletedObjectives[dwTeam].dwObjectives[i]);
	}

	LTBOOL bFragScore = (g_NetFragScore.GetFloat() > 0.0f);
	g_pLTServer->WriteToMessageByte(hWrite, bFragScore);

    g_pLTServer->EndMessage(hWrite);
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
	if (GetGameType() != SINGLE)
	{
		// Send a message to all clients, letting them know this user is leaving the world

        uint32 nClientID = g_pLTServer->GetClientID(hClient);
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_REMOVED);
        g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nClientID);
        g_pLTServer->EndMessage(hWrite);

		// Remove this client from the team...
        m_TeamMgr.RemovePlayer(g_pLTServer->GetClientID(hClient));
	}

	// Remove this client from our local list...

	RemoveClientFromList(hClient);
	SetUpdateGameServ();


	// Remove the player object...

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);
	if (pPlayer)
	{
        // pPlayer->SetClient(LTNULL);
        //g_pLTServer->RemoveObject(pPlayer->m_hObject);
		pPlayer->RemoveObject();
	}

    g_pLTServer->SetClientUserData(hClient, LTNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PreStartWorld()
//
//	PURPOSE:	Handle pre start world
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PreStartWorld(LTBOOL bSwitchingWorlds)
{

	// No commands yet...
	m_CmdMgr.Clear();

	// Setup the teams for multiplayer
	m_TeamMgr.Term();
	m_TeamMgr.Init();
	m_TeamMgr.AddTeam(TEAM_1, "TEAM1");
	m_TeamMgr.AddTeam(TEAM_2, "TEAM2");

	m_charMgr.PreStartWorld();
	g_DialogueWindow.Term();

	// Tell the server app.
	ServerAppPreStartWorld( );

	// Make sure we reset any necessary globals...

	Camera::ClearActiveCameras();
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
	m_charMgr.PostStartWorld(m_nLastLGFlags);
	g_DialogueWindow.Init();

	static CVarTrack cvResetAIReactions;
	if (!cvResetAIReactions.IsInitted())
        cvResetAIReactions.Init(g_pLTServer, "ResetAIReactions", LTNULL, 0.0f);
	if ( cvResetAIReactions.GetFloat() > 0.0f )
	{
		g_pLTServer->CPrint("got ai reaction reset");
		g_pCharacterMgr->ResetAIReactions();
	}

	// Tell the server app.
	ServerAppPostStartWorld( );
}


void CGameServerShell::SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, LTBOOL bNewPlayer)
{
	HCLIENT hClient	= pPlayer->GetClient();
	if (!hClient) return;

    uint32  nClientID   = g_pLTServer->GetClientID(hClient);
    HSTRING hClientName = g_pLTServer->CreateString(pPlayer->GetNetName());

	uint8 nLives = 255;
	if (g_NetUseSpawnLimit.GetFloat() > 0.0f)
	{
		nLives = (1+(uint8)g_NetSpawnLimit.GetFloat()-pPlayer->GetRespawnCount());
	}

    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClients, MID_PLAYER_ADDED);
    g_pLTServer->WriteToMessageHString(hWrite, hClientName);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nClientID);
    g_pLTServer->WriteToMessageObject(hWrite, pPlayer->m_hObject);
    g_pLTServer->WriteToMessageFloat(hWrite, (float)pPlayer->GetFragCount());
    g_pLTServer->WriteToMessageByte(hWrite, (uint8)pPlayer->GetTeamID());
    g_pLTServer->WriteToMessageByte(hWrite, nLives);
    g_pLTServer->WriteToMessageByte(hWrite, (uint8)bNewPlayer);
    g_pLTServer->FreeString(hClientName);

    g_pLTServer->EndMessage(hWrite);
}


void CGameServerShell::ProcessHandshake(HCLIENT hClient, HMESSAGEREAD hMessage)
{
	// Get the player
	CPlayerObj *pPlayer = GetPlayerFromClientList(hClient);
	if (!pPlayer)
	{
		g_pLTServer->CPrint("Handshake failed for an unknown client!");

		// Uh...  Who are you again?
		g_pLTServer->KickClient(hClient, GAME_DISCON_BADHANDSHAKE);

		return;
	}

	int nHandshakeSub = (int)g_pLTServer->ReadFromMessageByte(hMessage);
	switch (nHandshakeSub)
	{
		case MID_HANDSHAKE_HELLO :
		{
			// Check the client's version
			int nHandshakeVer = (int)g_pLTServer->ReadFromMessageWord(hMessage);
			if (nHandshakeVer != GAME_HANDSHAKE_VER)
			{
				g_pLTServer->CPrint("Handshake failed for '%s'!", pPlayer->GetNetName());
				g_pLTServer->CPrint("  '%s' was booted due to an invalid version number (%d not %d)", 
					pPlayer->GetNetName(), nHandshakeVer, GAME_HANDSHAKE_VER);

				// If they got here, they ignored our different version number.  Boot 'em!
				g_pLTServer->KickClient(hClient, GAME_DISCON_BADHANDSHAKE);

				// Jump out of the handshake
				return;
			}

			// Read in the client's sooper-secret key
			uint32 nClientKey = (uint32)g_pLTServer->ReadFromMessageDWord(hMessage);

			// Write out a password message, encrypted with their key
			// Note : A key from them that null encrypts our key doesn't really effect
			// anything..  I mean, they DID send the key, and they're the ones that need
			// to use our key anyway..
			HMESSAGEWRITE hResponse = g_pLTServer->StartMessage(hClient, MID_HANDSHAKE);
			g_pLTServer->WriteToMessageByte(hResponse, MID_HANDSHAKE_PASSWORD);
			g_pLTServer->WriteToMessageDWord(hResponse, GAME_HANDSHAKE_PASSWORD);
			g_pLTServer->EndMessage(hResponse);
		}
		break;
		case MID_HANDSHAKE_LETMEIN :
		{
			// Get the client's password
			uint32 nClientPassword = g_pLTServer->ReadFromMessageDWord(hMessage);
			uint32 nXORMask = GAME_HANDSHAKE_MASK;
			nClientPassword ^= nXORMask;
			if (nClientPassword != GAME_HANDSHAKE_PASSWORD)
			{
				g_pLTServer->CPrint("Handshake failed for '%s'!", pPlayer->GetNetName());
				g_pLTServer->CPrint("  '%s' was booted due to an invalid password", 
					pPlayer->GetNetName());

				// They're a fake!!
				g_pLTServer->KickClient(hClient, GAME_DISCON_BADHANDSHAKE);

				return;
			}

			// Read in their weapons file CRC
			uint32 nWeaponCRC = g_pWeaponMgr->GetFileCRC();
			nWeaponCRC ^= nXORMask;
			uint32 nClientCRC = g_pLTServer->ReadFromMessageDWord(hMessage);
			if (nWeaponCRC != nClientCRC)
			{
				g_pLTServer->CPrint("Handshake failed for '%s'!", pPlayer->GetNetName());
				g_pLTServer->CPrint("  '%s' was booted due to an invalid weapon file", 
					pPlayer->GetNetName());

				// They're cheating!!
				g_pLTServer->KickClient(hClient, GAME_DISCON_BADWEAPONS);

				return;
			}

			// Read in their client file CRC
			uint32 nCShellCRC = CRC32::CalcRezFileCRC("cshell.dll");
			nCShellCRC ^= nXORMask;
			nClientCRC = g_pLTServer->ReadFromMessageDWord(hMessage);
			if (nCShellCRC != nClientCRC)
			{
				g_pLTServer->CPrint("Handshake failed for '%s'!", pPlayer->GetNetName());
				g_pLTServer->CPrint("  '%s' was booted due to an invalid client shell", 
					pPlayer->GetNetName());

				// They're cheating!!
				g_pLTServer->KickClient(hClient, GAME_DISCON_BADCSHELL);

				return;
			}

			// Tell the client they passed the test...
			HMESSAGEWRITE hResponse = g_pLTServer->StartMessage(hClient, MID_HANDSHAKE);
			g_pLTServer->WriteToMessageByte(hResponse, MID_HANDSHAKE_DONE);
			g_pLTServer->EndMessage(hResponse);

			// Unlock the player
			pPlayer->FinishHandshake();

			// Spawn them in..
			RespawnPlayer(pPlayer, hClient);
		}
		break;
		default :
		{
			g_pLTServer->CPrint("Handshake failed for '%s'!", pPlayer->GetNetName());
			g_pLTServer->CPrint("  '%s' was booted due to an invalid handshake message (%d)", 
				pPlayer->GetNetName(), nHandshakeSub);

			// Hmm..  They sent over an invalid handshake message.  They're naughty.
			g_pLTServer->KickClient(hClient, GAME_DISCON_BADHANDSHAKE);

			return;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnMessage(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage)
{
	switch (messageID)
	{
		case MID_PLAYER_UPDATE :
		{
			HandleUpdatePlayerMsg(hSender, hMessage);
		}
		break;

		case MID_WEAPON_FIRE:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponFireMessage(hMessage);
			}
		}
		break;

		case MID_WEAPON_SOUND:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponSoundMessage(hMessage);
			}
		}
		break;

		case MID_WEAPON_CHANGE:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
                uint8 nWeaponId = g_pLTServer->ReadFromMessageByte(hMessage);
				pPlayer->DoWeaponChange(nWeaponId);
			}
		}
		break;

		case MID_PLAYER_ACTIVATE:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleActivateMessage(hMessage);
			}
		}
		break;

		case MID_PLAYER_CLIENTMSG:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleClientMsg(hMessage);
			}
		}
		break;

		case MID_FRAG_SELF:
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
			if(pPlayer)
			{
                LPBASECLASS pClass = g_pLTServer->HandleToObject(pPlayer->m_hObject);
				if (pClass)
				{
					DamageStruct damage;

					damage.eType	= DT_EXPLODE;
					damage.fDamage	= damage.kInfiniteDamage;
					damage.hDamager = pPlayer->m_hObject;
					damage.vDir.Init(0, 1, 0);

					damage.DoDamage(pClass, pPlayer->m_hObject);
				}
			}
		}
		break;

		case MID_PLAYER_RESPAWN :
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer && pPlayer->GetState() == PS_DEAD)
			{
				LTBOOL bRespawn = LTTRUE;
				if (GetGameType() == COOPERATIVE_ASSAULT && g_NetUseSpawnLimit.GetFloat() > 0.0f)
				{
					pPlayer->IncRespawnCount();
					if (pPlayer->GetRespawnCount() > (int)g_NetSpawnLimit.GetFloat() )
					{
						pPlayer->ChangeState(PS_GHOST);
						bRespawn = LTFALSE;
					}

				}

				if (bRespawn)
				{
					pPlayer->Respawn();
				}
			}
		}
		break;

		case MID_PLAYER_TAUNT :
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			uint32 nTauntID = g_pLTServer->ReadFromMessageDWord(hMessage);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer && g_NetAudioTaunts.GetFloat() > 0.0f)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
				g_pLTServer->WriteToMessageObject(hMessage, pPlayer->m_hObject);
				g_pLTServer->WriteToMessageByte(hMessage, CFX_TAUNT_MSG);
				g_pLTServer->WriteToMessageDWord(hMessage, nTauntID);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
			}


		} 
		break;

		case MID_MULTIPLAYER_UPDATE:
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				// Tell the new Client about all the clients already
				// on the server...

				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					CPlayerObj* pCurPlayer = GetPlayerFromClientList(m_aClients[i]);
					if (pCurPlayer)
					{
						SendPlayerInfoMsgToClients(pPlayer->GetClient(), pCurPlayer, LTFALSE);
					}
				}
			}

		} 
		break;

		case MID_PLAYER_MULTIPLAYER_INIT :
		case MID_PLAYER_MULTIPLAYER_CHANGE :
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{

				if (pPlayer->MultiplayerInit(hMessage))
				{
					// Send a message to all clients, letting them know a
					// new client has joined the game...

                    SendPlayerInfoMsgToClients(LTNULL, pPlayer, (messageID == MID_PLAYER_MULTIPLAYER_INIT));

					// Tell the new Client about all the clients already
					// on the server...

					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						CPlayerObj* pCurPlayer = GetPlayerFromClientList(m_aClients[i]);
						if (pCurPlayer && pCurPlayer != pPlayer)
						{
                            SendPlayerInfoMsgToClients(pPlayer->GetClient(), pCurPlayer, LTFALSE);
						}
					}
					if (messageID == MID_PLAYER_MULTIPLAYER_INIT)
					{
						pPlayer->Respawn();
					}
				}
			}
		}
		break;

		case MID_HANDSHAKE :
		{
			ProcessHandshake(hSender, hMessage);
		}
		break;

		case MID_PLAYER_INITVARS :
		{
            LTBOOL bRunLock = (LTBOOL)g_pLTServer->ReadFromMessageByte(hMessage);

            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->SetRunLock (bRunLock);
			}
		}
		break;

		case MID_PLAYER_MESSAGE :
		{
			// retrieve the string they sent

			char szString[100];
			szString[0] = 0;
			hMessage->ReadStringFL(szString, sizeof(szString));

			// So it shows up in GameSrv..
            if(szString[0] && !(g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL))
			{
                g_pLTServer->CPrint(szString);
			}

			// now send the string to all clients

            HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_PLAYER_MESSAGE);
            g_pLTServer->WriteToMessageString(hMessage, szString);
            g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
		}
		break;

		case MID_PLAYER_TEAMMESSAGE :
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;

			//do nothing if player isn't on a team
			if (!pPlayer->GetTeamID()) return;


			// retrieve the string they sent
			char szString[100];
			szString[0] = 0;
			hMessage->ReadStringFL(szString, sizeof(szString));

			// So it shows up in GameSrv..
            if(szString[0] && !(g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL))
			{
                g_pLTServer->CPrint(szString);
			}


			// now send the string to all clients on the same team
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				CPlayerObj* pCurPlayer = GetPlayerFromClientList(m_aClients[i]);
				if (pCurPlayer &&  pCurPlayer->GetTeamID() == pPlayer->GetTeamID())
				{
                    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(pCurPlayer->GetClient(), MID_PLAYER_TEAMMESSAGE);
                    g_pLTServer->WriteToMessageString(hMessage, szString);
                    g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}

		}
		break;

		case MID_PLAYER_GHOSTMESSAGE :
		{
            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;

			//do nothing if player isn't a ghost
			if (pPlayer->GetState() != PS_GHOST) return;


			// retrieve the string they sent
			char szString[100];
			szString[0] = 0;
			hMessage->ReadStringFL(szString, sizeof(szString));

			// So it shows up in GameSrv..
            if(szString[0] && !(g_pLTServer->GetClientInfoFlags(hSender) & CIF_LOCAL))
			{
                g_pLTServer->CPrint(szString);
			}


			// now send the string to all clients on the same team
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				CPlayerObj* pCurPlayer = GetPlayerFromClientList(m_aClients[i]);
				if (pCurPlayer &&  pCurPlayer->GetState() == PS_GHOST)
				{
                    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(pCurPlayer->GetClient(), MID_PLAYER_GHOSTMESSAGE);
                    g_pLTServer->WriteToMessageString(hMessage, szString);
                    g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}

		}
		break;

		case MID_PLAYER_CHATMODE:
		{
            LTBOOL bChatting = (LTBOOL)g_pLTServer->ReadFromMessageByte(hMessage);

            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;

			if (pPlayer)
				pPlayer->SetChatting(bChatting);

		}
		break;

		case MID_PLAYER_CHEAT :
		{

#ifdef _DEMO
			return;  // No cheats in demo mode
#endif

			if (GetGameType() != SINGLE) return;  // No cheats in multiplayer

			// get a pointer to the sender's player object

            void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (!pPlayer) return;

			// retrieve message data

            CheatCode nCheatCode = (CheatCode) g_pLTServer->ReadFromMessageByte (hMessage);
            uint8 nData = g_pLTServer->ReadFromMessageByte (hMessage);

			// now deal with the specific cheat code

			HandleCheatCode(pPlayer, nCheatCode, nData);
		}
		break;

		case MID_PLAYER_EXITLEVEL:
		{
			HandlePlayerExitLevel(hSender, hMessage);
		}
		break;

		case MID_PLAYER_SUMMARY:
		{
			HandlePlayerSummary(hSender, hMessage);
		}
		break;

		case MID_PLAYER_CHANGETEAM:
		{
			HandlePlayerChangeTeam(hSender, hMessage);
		}
		break;

		case MID_UPDATE_OPTIONS:
		{
			HandleUpdateServerOptions(hMessage);
		}
		break;

		case MID_GAME_PAUSE:
		{
            PauseGame(LTTRUE);
		}
		break;

		case MID_GAME_UNPAUSE:
		{
            PauseGame(LTFALSE);
		}
		break;

		case MID_MISSION_INFO:
		{
			HandleMissionInfoMsg(hSender, hMessage);
		}
		break;

		case MID_LOAD_GAME :
		{
			HandleLoadGameMsg(hSender, hMessage);
		}
		break;

		case MID_SAVE_GAME :
		{
			HandleSaveGameMsg(hSender, hMessage);
		}
		break;

		case MID_SINGLEPLAYER_START :
		{
			if (GetGameType() == SINGLE)
			{
                void *pData = g_pLTServer->GetClientUserData(hSender);
				CPlayerObj* pPlayer = (CPlayerObj*)pData;
				if (pPlayer)
				{
					pPlayer->StartLevel();
				}
				//g_pLTServer->CPrint("Difficulty:%d",(int)m_eDifficulty);
				//g_pLTServer->CPrint("FadeBodies:%d",(int)m_bFadeBodies);

				g_pMusicMgr->Enable();
			}
		}
		break;

		case MID_CONSOLE_TRIGGER :
		{
			HandleConsoleTriggerMsg(hSender, hMessage);
		}
		break;

		case MID_CONSOLE_COMMAND :
		{
			HandleConsoleCmdMsg(hSender, hMessage);
		}
		break;

		case MID_DIFFICULTY:
		{
            m_eDifficulty = (GameDifficulty)g_pLTServer->ReadFromMessageByte(hMessage);
			g_pLTServer->CPrint("Difficulty:%d",(int)m_eDifficulty);
		}
		break;

		case MID_FADEBODIES:
		{
            m_bFadeBodies = (LTBOOL)g_pLTServer->ReadFromMessageByte(hMessage);
			g_pLTServer->CPrint("FadeBodies:%d",(int)m_bFadeBodies);

			if ( m_bFadeBodies )
			{
				// Wake up all bodies if we need to fade

				HCLASS  hBody = g_pLTServer->GetClass("Body");
				HOBJECT hCurObject = LTNULL;
				while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
				{
					if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hBody))
					{
						g_pLTServer->SetNextUpdate(hCurObject, .01f);
					}
				}

				hCurObject = LTNULL;
				while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
				{
					if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hBody))
					{
						g_pLTServer->SetNextUpdate(hCurObject, .01f);
					}
				}

			}
		}
		break;

		case CSM_DIALOGUE_DONE:
		{
			// Tell our global dialogue manager that the dialogue is done
			g_DialogueWindow.Finished();
		}
		break;

		case CSM_DIALOGUE_DONE_SELECTION:
		{
			// Tell our global dialogue manager that the dialogue is done
            unsigned char byDecision = g_pLTServer->ReadFromMessageByte(hMessage);
            uint32 dwID = g_pLTServer->ReadFromMessageDWord(hMessage);
			g_DialogueWindow.Finished(byDecision,dwID);
		}
		break;

		case CSM_DIALOGUE_STOP:
		{
			g_DialogueWindow.StopDialogue();
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleUpdatePlayerMsg()
//
//	PURPOSE:	Handle updating the player info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleUpdatePlayerMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	// Update the player...

    void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;

	if (pPlayer)
	{
		if (pPlayer->ClientUpdate(hMessage))
		{
			pPlayer->HandlePlayerPositionMessage(hMessage); // Merged player position and update messages.
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleCheatCode()
//
//	PURPOSE:	Handle the various cheat codes
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleCheatCode(CPlayerObj* pPlayer, CheatCode nCheatCode, uint8 nData)
{
	if (nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX) return;

	switch (nCheatCode)
	{
		case CHEAT_GOD :
		{
			pPlayer->ToggleGodMode();
		}
		break;

		case CHEAT_AMMO :
		{
			pPlayer->FullAmmoCheat();
		}
		break;

		case CHEAT_ARMOR :
		{
			pPlayer->RepairArmorCheat();
		}
		break;

		case CHEAT_HEALTH :
		{
			pPlayer->HealCheat();
		}
		break;

		case CHEAT_CLIP :
		{
			pPlayer->SetSpectatorMode(nData);
		}
		break;

		case CHEAT_TELEPORT :
		{
			pPlayer->Respawn();
		}
		break;

		case CHEAT_FULL_WEAPONS :
		{
			pPlayer->FullWeaponCheat();
		}
		break;

		case CHEAT_TEARS:
		{
			g_bInfiniteAmmo = !g_bInfiniteAmmo;

			if (g_bInfiniteAmmo)
			{
				pPlayer->FullWeaponCheat();
			}
		}
		break;

		case CHEAT_REMOVEAI:
		{
			HandleCheatRemoveAI(nData);
		}
		break;

		case CHEAT_MOTORCYCLE:
		{
            g_pLTServer->RunGameConString("spawnobject PlayerVehicle (VehicleType Motorcycle)");
		}
		break;

		case CHEAT_SNOWMOBILE:
		{
            g_pLTServer->RunGameConString("spawnobject PlayerVehicle (VehicleType Snowmobile)");
		}
		break;

		case CHEAT_MODSQUAD :
		{
			pPlayer->FullModsCheat();
		}
		break;

		case CHEAT_FULL_GEAR :
		{
			pPlayer->FullGearCheat();
		}
		break;

		default:
		break;
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnCommandOn()
//
//	PURPOSE:	Handle commands
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnCommandOn(HCLIENT hClient, int command)
{
	if (!hClient) return;

    void *pData = g_pLTServer->GetClientUserData(hClient);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (!pPlayer) return;

	switch (command)
	{
		case COMMAND_ID_RUNLOCK	: pPlayer->ToggleRunLock(); break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::CreatePlayer(HCLIENT hClient)
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

    HCLASS hClass = g_pLTServer->GetClass("CPlayerObj");

	GameStartPoint* pStartPoint = FindStartPoint(LTNULL);
	if (pStartPoint)
	{
      g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
	}

	CPlayerObj* pPlayer = NULL;
	if (hClass)
	{
        pPlayer = (CPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
		if (pPlayer)
		{
			RespawnPlayer(pPlayer, hClient);
		}
	}

	return pPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FindStartPoint()
//
//	PURPOSE:	Find a good start point.
//
// ----------------------------------------------------------------------- //

GameStartPoint* CGameServerShell::FindStartPoint(CPlayerObj* pPlayer)
{
	// Get all the start points...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	int numObjects, i;
    GameStartPoint** pStartPtArray = LTNULL;

    g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
	numObjects = objArray.NumObjects();

	if (!numObjects)
    {
        return LTNULL;
    }
    else
    {
		pStartPtArray = debug_newa(GameStartPoint*, numObjects);
    }

    if (!pStartPtArray) return LTNULL;

	int nCount = 0;
    GameStartPoint* pStartPt = LTNULL;
    GameStartPoint* pDefStartPt = (GameStartPoint*)g_pLTServer->HandleToObject(objArray.GetObject(0));

	for(i = 0; i < numObjects; i++)
	{
        pStartPt = (GameStartPoint*)g_pLTServer->HandleToObject(objArray.GetObject(i));
		if (pStartPt)
		{
			if (pStartPt->GetGameType() == GetGameType())
			{
				if (pPlayer && (GetGameType() == COOPERATIVE_ASSAULT))
				{
					if (pStartPt->GetTeam() == TEAM_AUTO || pStartPt->GetTeam() == pPlayer->GetTeamID())
					{
						pStartPtArray[nCount++] = pStartPt;
					}
				}
				else
				{
					pStartPtArray[nCount++] = pStartPt;
				}
			}
		}
	}

    pStartPt = LTNULL;

	if (nCount > 0 && nCount <= numObjects)
	{
		int nIndex = 0;
		switch (GetGameType())
		{
			case SINGLE :
			{
				nIndex = 0;
			}
			break;

			case COOPERATIVE_ASSAULT:
			case DEATHMATCH :
            {
				nIndex = nCount+1;
				int nRetries = nCount;
				LTBOOL bSafe = LTFALSE;
				while (nRetries && !bSafe)
				{
					if (nIndex > nCount)
					{
						nIndex = GetRandom(0, nCount-1);
					}
					else
					{
						nIndex = (nIndex+1)%nCount;
					}
					LTVector testPos;
				    g_pLTServer->GetObjectPos(pStartPtArray[nIndex]->m_hObject, &testPos);
					bSafe = !IsPositionOccupied(testPos,pPlayer);
					nRetries--;
				}
            }
			break;

		/* commented out spawn closest jrg - 9/23/00
			case COOPERATIVE_ASSAULT:
			{
				nIndex = GetRandom(0, nCount-1);

				if (!pPlayer) break;

				LTVector curPos;
				LTFLOAT	fDistance = -1.0f;
		        g_pLTServer->GetObjectPos(pPlayer->m_hObject, &curPos);

				for (int i=0; i < nCount; i++)
				{
					if (pStartPtArray[i])
					{
						LTVector testPos;
					    g_pLTServer->GetObjectPos(pStartPtArray[i]->m_hObject, &testPos);
						LTFLOAT fTestDist = curPos.Dist(testPos);

						if (fDistance < 0.0f || fTestDist < fDistance)
						{
							nIndex = i;
							fDistance = fTestDist;
						}
					}
				}
			}
			break;
			*/

			default : break;
		}

		pStartPt = pStartPtArray[nIndex];
	}
	else
	{
		pStartPt = pDefStartPt;
	}

	debug_deletea(pStartPtArray);

	return pStartPt;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::RespawnPlayer()
//
//	PURPOSE:	Respawn the player object
//
// ----------------------------------------------------------------------- //

void CGameServerShell::RespawnPlayer(CPlayerObj* pPlayer, HCLIENT hClient)
{
	if (!pPlayer || !hClient) return;

	// Player object meet client, client meet player object...

	pPlayer->SetClient(hClient);
    g_pLTServer->SetClientUserData(hClient, (void *)pPlayer);

	if (GetGameType() != SINGLE)
		pPlayer->PreMultiplayerInit();

	// If this is a multiplayer game wait until MULTIPLAYER_INIT message
	// to respawn the player...


	if (GetGameType() == SINGLE)
	{
		pPlayer->Respawn(m_nLastLGFlags);

		uint8 nMessage = MID_PLAYER_SINGLEPLAYER_INIT;
		// Tell the client to init us!

		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, nMessage);
		g_pLTServer->EndMessage(hWrite);

		// Unlock the player since we're in single player mode
		pPlayer->FinishHandshake();
	}
	else if (!pPlayer->HasDoneHandshake())
	{
		// Start a handshake with the client
		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_HANDSHAKE);
		g_pLTServer->WriteToMessageByte(hWrite, MID_HANDSHAKE_HELLO);
		g_pLTServer->WriteToMessageWord(hWrite, GAME_HANDSHAKE_VER);
		g_pLTServer->EndMessage(hWrite);
	}
	else
	{
	    uint8 nMessage = MID_PLAYER_MULTIPLAYER_INIT;

		// Tell the client to init us!

		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, nMessage);
		g_pLTServer->WriteToMessageDWord(hWrite, (uint32)g_pWorldProperties->GetMPMissionName());
		g_pLTServer->WriteToMessageDWord(hWrite, (uint32)g_pWorldProperties->GetMPMissionBriefing());
		g_pLTServer->EndMessage(hWrite);

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleCheatRemoveAI()
//
//	PURPOSE:	Handle the remove ai cheat
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleCheatRemoveAI(uint8 nData)
{
    HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hClass = g_pLTServer->GetClass("CAI");

	// Remove all the ai objects...

    LTBOOL bRemove = LTFALSE;

    HOBJECT hRemoveObj = LTNULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextObject(hObj);

		if (hRemoveObj)
		{
            LPBASECLASS pClass = g_pLTServer->HandleToObject(hRemoveObj);
			if (pClass)
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = hRemoveObj;

				damage.DoDamage(pClass, hRemoveObj);
			}

            hRemoveObj = LTNULL;
		}
	}


    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
    hRemoveObj = LTNULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);

		if (hRemoveObj)
		{
            LPBASECLASS pClass = g_pLTServer->HandleToObject(hRemoveObj);
			if (pClass)
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = hRemoveObj;

				damage.DoDamage(pClass, hRemoveObj);
			}

            hRemoveObj = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleMissionInfoMsg()
//
//	PURPOSE:	Handle the mission info message
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleMissionInfoMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	// Read the message data...

    m_MissionData.ReadFromMessage(g_pLTServer, hMessage);

	if (g_pLTServer->ReadFromMessageByte(hMessage))
	{
		void *pData = g_pLTServer->GetClientUserData(hSender);
		CPlayerObj* pPlayer = (CPlayerObj*)pData;
		if (pPlayer)
		{
			pPlayer->Setup(&m_MissionData);
		}
	}
	else
	{
		m_bUseMissionData = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleLoadGameMsg()
//
//	PURPOSE:	Handle loading a game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleLoadGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
    LTRESULT dResult = LT_OK;
    uint32 flags;

	// Read the message data...

    m_nLastLGFlags  = g_pLTServer->ReadFromMessageByte(hMessage);
    m_eDifficulty   = (GameDifficulty) g_pLTServer->ReadFromMessageByte(hMessage);
    m_bFadeBodies   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hMessage);
    HSTRING hLGName = g_pLTServer->ReadFromMessageHString(hMessage);
    HSTRING hSGName = g_pLTServer->ReadFromMessageHString(hMessage);
    HSTRING hROName = g_pLTServer->ReadFromMessageHString(hMessage);

	// BL 09/30/00 HACK to fix ammo save/load
	LTFLOAT fRestoreAmmoId = g_pLTServer->ReadFromMessageFloat(hMessage);
	uint32 nMusicIntensity = g_pLTServer->ReadFromMessageDWord(hMessage);

	// Save client-side info if loading a new level...

    HMESSAGEREAD hClientData = g_pLTServer->ReadFromMessageHMessageRead(hMessage);

    ObjectList* pKeepAliveList = g_pLTServer->CreateObjectList();


    char* pLGFileName = LTNULL;
    char* pSGFileName = LTNULL;
    char* pROFileName = LTNULL;

    if (hLGName) pLGFileName = g_pLTServer->GetStringData(hLGName);
    if (hSGName) pSGFileName = g_pLTServer->GetStringData(hSGName);
    if (hROName) pROFileName = g_pLTServer->GetStringData(hROName);

    LTBOOL bLoadWorldObjects     = LTFALSE;
    LTBOOL bSaveKeepAlives       = LTFALSE;
    LTBOOL bRestoreLevelObjects  = LTFALSE;


	// Set up the player's client data...

    void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (pPlayer)
	{
		pPlayer->SetClientSaveData(hClientData);
	}

	if (!hLGName)
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (m_nLastLGFlags == LOAD_NEW_GAME)
	{
        bLoadWorldObjects = LTTRUE;
	}
	else if (m_nLastLGFlags == LOAD_NEW_LEVEL)
	{
        bLoadWorldObjects   = LTTRUE;
        bSaveKeepAlives     = LTTRUE;
	}
	else if (m_nLastLGFlags == LOAD_RESTORE_GAME)
	{
        bRestoreLevelObjects = LTTRUE;
	}
	else // Invalid flags
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Invalid flags!");
		goto FREE_DATA;
	}


	// Validate the filenames...

	if (!pLGFileName || pLGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (bRestoreLevelObjects)
	{
		if (!pROFileName || pROFileName[0] == ' ')
		{
			ReportError(hSender, SERROR_LOADGAME);
            g_pLTServer->BPrint("Load Game Error: Invalid restore objects filename!");
			goto FREE_DATA;
		}
	}


	if (!pKeepAliveList)
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Couldn't create keep alive list!");
		goto FREE_DATA;
	}

	if (bSaveKeepAlives)
	{
		// Build keep alives list...

		// Let the player add necessary keep alives to the list...

        void *pData = g_pLTServer->GetClientUserData(hSender);
		CPlayerObj* pPlayer = (CPlayerObj*)pData;
		if (pPlayer)
		{
			pPlayer->BuildKeepAlives(pKeepAliveList);
		}


        dResult = g_pLTServer->SaveObjects(KEEPALIVE_FILENAME, pKeepAliveList, m_nLastLGFlags, 0);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
            g_pLTServer->BPrint("Load Game Error: Couldn't save keepalives");
			goto FREE_DATA;
		}
	}


	// Load the new level...

	flags = bLoadWorldObjects ? LOADWORLD_LOADWORLDOBJECTS : 0;
	flags |= LOADWORLD_NORELOADGEOMETRY;
    dResult = g_pLTServer->LoadWorld(pLGFileName, flags);

	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Couldn't Load world '%s'", pLGFileName);
		goto FREE_DATA;
	}


	if (bRestoreLevelObjects)
	{
        dResult = g_pLTServer->RestoreObjects(pROFileName, m_nLastLGFlags, RESTOREOBJECTS_RESTORETIME);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
            g_pLTServer->BPrint("Load Game Error: Couldn't restore objects '%s'", pROFileName);
			goto FREE_DATA;
		}
	}


	// Load the keep alives...

	if (bSaveKeepAlives && pKeepAliveList && pKeepAliveList->m_nInList > 0)
	{
        dResult = g_pLTServer->RestoreObjects(KEEPALIVE_FILENAME, m_nLastLGFlags, 0);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
            g_pLTServer->BPrint("Load Game Error: Couldn't restore keepalives");
			goto FREE_DATA;
		}
	}



	// Start the world...

    dResult = g_pLTServer->RunWorld();

	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
        g_pLTServer->BPrint("Load Game Error: Couldn't run world!");
	}


// Waste not, want not...

FREE_DATA:

	if (pKeepAliveList)
	{
        g_pLTServer->RelinquishList(pKeepAliveList);
	}

	if (hClientData)
	{
        g_pLTServer->EndHMessageRead(hClientData);
	}

    if (hLGName) g_pLTServer->FreeString(hLGName);
    if (hSGName) g_pLTServer->FreeString(hSGName);
    if (hROName) g_pLTServer->FreeString(hROName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleSaveGameMsg()
//
//	PURPOSE:	Handle saving a game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleSaveGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
    char* pSGFileName = LTNULL;
    HOBJECT hObj = LTNULL;
    ObjectList* pSaveList = g_pLTServer->CreateObjectList();

	// Read the message data...

    uint8   nSGFlags = g_pLTServer->ReadFromMessageByte(hMessage);
    HSTRING hSGName  = g_pLTServer->ReadFromMessageHString(hMessage);

	// BL 09/30/00 HACK to fix ammo save/load
	LTFLOAT fRestoreAmmoId = g_pLTServer->ReadFromMessageFloat(hMessage);
	uint32 nRestoreMusicIntensity = g_pLTServer->ReadFromMessageDWord(hMessage);

    HMESSAGEREAD hClientData = g_pLTServer->ReadFromMessageHMessageRead(hMessage);


	// Set up the player's client data...

    void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (pPlayer)
	{
		pPlayer->SetClientSaveData(hClientData);

		// BL 09/30/00 HACK to fix ammo save/load
		pPlayer->SetRestoreAmmoId(fRestoreAmmoId);
		g_pMusicMgr->SetRestoreMusicIntensity(nRestoreMusicIntensity);
	}

	if (!hSGName)
	{
		ReportError(hSender, SERROR_SAVEGAME);
        g_pLTServer->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}

    pSGFileName = g_pLTServer->GetStringData(hSGName);
	if (!pSGFileName || pSGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_SAVEGAME);
        g_pLTServer->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}


	// Save all the objects...

	if (!pSaveList)
	{
		ReportError(hSender, SERROR_SAVEGAME);
        g_pLTServer->BPrint("Save Game Error: Allocation error!");
		goto FREE_DATA;
	}


	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).

	if (!g_pWorldProperties)
	{
		ReportError(hSender, SERROR_SAVEGAME);
        g_pLTServer->BPrint("Save Game Error: No WorldProperties object!  Can not save game!");
		goto FREE_DATA;
	}


	// Add active objects to the list...

    hObj = g_pLTServer->GetNextObject(LTNULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
            g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Add inactive objects to the list...

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
            g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}


	// Make sure the WorldProperties object is saved FIRST, this way all the global
	// data will be available for the other objects when they get restored.
	// (ServerDE::AddObjectsToList() adds to the front of the list, so we
	// need to add it last ;)...

    g_pLTServer->AddObjectToList(pSaveList, g_pWorldProperties->m_hObject);



	if (pSaveList && pSaveList->m_nInList > 0)
	{
        LTRESULT dResult = g_pLTServer->SaveObjects(pSGFileName, pSaveList, LOAD_RESTORE_GAME,
												 SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEPORTALS);
		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_SAVEGAME);
            g_pLTServer->BPrint("Save Game Error: Couldn't save objects!");
		}
	}

// Waste not, want not...

FREE_DATA:

	if (hClientData)
	{
        g_pLTServer->EndHMessageRead(hClientData);
	}

    if (hSGName) g_pLTServer->FreeString(hSGName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ReportError()
//
//	PURPOSE:	Tell the client about a server error
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ReportError(HCLIENT hSender, uint8 nErrorType)
{
    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hSender, MID_SERVER_ERROR);
    g_pLTServer->WriteToMessageByte(hWrite, nErrorType);
    g_pLTServer->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheFiles()
//
//	PURPOSE:	Cache files that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheFiles()
{
	CacheModels();
	CacheTextures();
	CacheSprites();
	CacheSounds();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheModels()
//
//	PURPOSE:	Cache models that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheModels()
{
	for (int i=0; g_pCachedModels[i]; i++)
	{
        g_pLTServer->CacheFile(FT_MODEL, g_pCachedModels[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheTextures()
//
//	PURPOSE:	Cache textures that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheTextures()
{
	for (int i=0; g_pCachedTextures[i]; i++)
	{
        g_pLTServer->CacheFile(FT_TEXTURE, g_pCachedTextures[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheSprites()
//
//	PURPOSE:	Cache sprites that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheSprites()
{
	for (int i=0; g_pCachedSprite[i]; i++)
	{
        g_pLTServer->CacheFile(FT_SPRITE, g_pCachedSprite[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheSounds()
//
//	PURPOSE:	Cache sounds that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheSounds()
{
    int i;
    for (i=0; g_pCachedSoundLocal[i]; i++)
	{
        g_pLTServer->CacheFile(FT_SOUND, g_pCachedSoundLocal[i]);
	}

	for (i=0; g_pCachedSound3D[i]; i++)
	{
        g_pLTServer->CacheFile(FT_SOUND, g_pCachedSound3D[i]);
	}
}


void CGameServerShell::UpdateClientPingTimes()
{
	HMESSAGEWRITE hWrite;
	float ping;
    uint32 clientID;
	HCLIENT hClient;

    m_ClientPingSendCounter += g_pLTServer->GetFrameTime();
	if(m_ClientPingSendCounter > CLIENT_PING_UPDATE_RATE)
	{
        hWrite = g_pLTServer->StartMessage(LTNULL, MID_PINGTIMES);

            hClient = LTNULL;
            while(hClient = g_pLTServer->GetNextClient(hClient))
			{
                clientID = g_pLTServer->GetClientID(hClient);
                g_pLTServer->GetClientPing(hClient, ping);

                g_pLTServer->WriteToMessageWord(hWrite, (uint16)clientID);
                g_pLTServer->WriteToMessageWord(hWrite, (uint16)(ping * 1000.0f));
			}

        g_pLTServer->WriteToMessageWord(hWrite, 0xFFFF);
        g_pLTServer->EndMessage2(hWrite, MESSAGE_NAGGLE);

		m_ClientPingSendCounter = 0.0f;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Update
//
//	PURPOSE:	Update servier stuff periodically
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Check for first update...

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = LTFALSE;
		FirstUpdate();
	}

	g_pMusicMgr->Update();


	// Update the command mgr...

	m_CmdMgr.Update();


	// See if we should show our bounding box...

	if (g_CanShowDimsTrack.GetFloat())
	{
		if (g_ShowDimsTrack.GetFloat())
		{
			UpdateBoundingBoxes();
		}
		else
		{
			RemoveBoundingBoxes();
		}
	}


	// Did the server want to say something?

	char *pSay = m_SayTrack.GetStr("");
	if(pSay && pSay[0] != 0)
	{
		char fullMsg[512];

		sprintf(fullMsg, "HOST: %s", pSay);
        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage (NULL, MID_PLAYER_MESSAGE);
        g_pLTServer->WriteToMessageString (hMessage, fullMsg);
        g_pLTServer->EndMessage2 (hMessage, MESSAGE_NAGGLE);

		m_SayTrack.SetStr("");
	}


	// Check the time of day stuff (every half second).

	UpdateTimeOfDay(timeElapsed);

	// The rest is multiplayer only....
	if (GetGameType() == SINGLE)  return;

	// Update client ping times
	UpdateClientPingTimes();

	// Update GameSpy.
	UpdateGameSpyMgr();

	// Update game server info...
	UpdateGameServer();

	// Update multiplayer stuff...
	UpdateMultiplayer();

	// See if we should change worlds.
	CheckMultiSwitchWorlds();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CheckMultiSwitchWorlds
//
//	PURPOSE:	See if we should change multiplayer worlds
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CheckMultiSwitchWorlds()
{
	// See if its time...

	if (m_bStartNextMultiplayerLevel)
	{
		m_bStartNextMultiplayerLevel = LTFALSE;
		StartNextMultiplayerLevel();
		return;
	}

	// See if the SwitchWorld console variable is set, if so,
	// change to this world.

	char* pCmdName = "SwitchWorld";

	char* pCurLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	char* pNextLevelName = m_GameInfo.m_sLevels[(m_nCurLevel + 1) % m_GameInfo.m_byNumLevels];

	if (!m_hSwitchWorldVar)
	{
        g_pLTServer->SetGameConVar(pCmdName, "");
        m_hSwitchWorldVar = g_pLTServer->GetGameConVar(pCmdName);

		if (!m_hSwitchWorldVar)	return;
	}

	// Is it set to anything?
    char* pVal = g_pLTServer->GetVarValueString(m_hSwitchWorldVar);
	if (pVal && pVal[0] != 0)
	{
		// Switch to the requested world.
		if (SwitchToWorld(pVal, pNextLevelName) != LT_OK)
		{
			SwitchToWorld(pCurLevelName, pNextLevelName);
		}

		// Reset this.
        g_pLTServer->SetGameConVar(pCmdName, "");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Update
//
//	PURPOSE:	Do the first update
//
// ----------------------------------------------------------------------- //

void CGameServerShell::FirstUpdate()
{
	m_SayTrack.Init(g_pLTServer, "Say", "", 0.0f);
    m_ShowTimeTrack.Init(g_pLTServer, "ShowTime", LTNULL, 0.0f);
    m_WorldTimeTrack.Init(g_pLTServer, "WorldTime", "-1", 0.0f);
    m_WorldTimeSpeedTrack.Init(g_pLTServer, "WorldTimeSpeed", "-1", 0.0f);

	SetupGameInfo();

	if (GetGameType() != SINGLE)
	{
		// Set server options
		SetServerOptions();
		// Init GameSpy
		InitGameSpy();
		//Init the level name
		strncpy(m_sWorld, m_GameInfo.m_sLevels[m_nCurLevel], MAX_GEN_STRING);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateTimeOfDay
//
//	PURPOSE:	Update the time of day stuff
//
// ----------------------------------------------------------------------- //

void CGameServerShell::UpdateTimeOfDay(LTFLOAT timeElapsed)
{
	float todHours;
	float sunAngle;
    LTVector sunVec;
    unsigned char newSunVec[3];
	float testHours, t;
	TimeRamp *pRamp, *pNextRamp;
    uint32 i, iCurRamp;


	LTVector day, night, theColor;
    uint8 newColor[3];

	if (m_WorldTimeSpeedTrack.GetFloat() == -1)
	{
		newColor[0] = newColor[1] = newColor[2] = MAX_WORLDTIME_COLOR;
		newSunVec[0] = newSunVec[1] = newSunVec[2] = 0;
	}
	else
	{
		m_TODSeconds += timeElapsed * m_WorldTimeSpeedTrack.GetFloat();

		if(m_WorldTimeTrack.GetFloat() != -1.0f)
		{
			m_TODSeconds = TODHoursToSeconds(m_WorldTimeTrack.GetFloat());
			m_WorldTimeTrack.SetFloat(-1.0f);
		}

		todHours = TODSecondsToHours(m_TODSeconds);
		if(todHours > 24.0f)
		{
			todHours = 0.0f;
			m_TODSeconds = 0.0f;
		}

		// How often we update the time of day.
		static float updateRate = 0.0005f;

        m_TODCounter += timeElapsed;
        if(m_TODCounter > updateRate)
		{
			// Figure out how many time ramps we have.
			for(m_nTimeRamps=0; m_nTimeRamps < MAX_TIME_RAMPS; m_nTimeRamps++)
			{
				if(m_TimeRamps[m_nTimeRamps].m_Time == -1.0f)
					break;
			}

			// Figure out the shade from our ramp table.
			testHours = (float)fmod(todHours, 24.0f);
			for(i=0; i < m_nTimeRamps; i++)
			{
				pRamp = &m_TimeRamps[i];
				pNextRamp = &m_TimeRamps[(i+1) % m_nTimeRamps];

				if(testHours >= pRamp->m_Time && testHours <= pNextRamp->m_Time)
				{
					t = (testHours - pRamp->m_Time) / (pNextRamp->m_Time - pRamp->m_Time);
					theColor = pRamp->m_Color + (pNextRamp->m_Color - pRamp->m_Color) * t;
					iCurRamp = i;
				}
			}

			// Send trigger messages.
			for(i=m_iPrevRamp; i != iCurRamp; i = (i+1) % m_nTimeRamps)
			{
				pRamp = &m_TimeRamps[i];
				SendTriggerMsgToObjects(NULL, pRamp->m_hTarget, pRamp->m_hMessage);
			}

			m_iPrevRamp = iCurRamp;

            newColor[0] = (uint8)(theColor.x * (float)MAX_WORLDTIME_COLOR);
            newColor[1] = (uint8)(theColor.y * (float)MAX_WORLDTIME_COLOR);
            newColor[2] = (uint8)(theColor.z * (float)MAX_WORLDTIME_COLOR);

			// Figure out the sun direction.
			sunAngle = ((todHours - 6.0f) * MATH_PI) / 12.0f;
			sunVec.x = (float)cos(sunAngle);
			sunVec.y = (float)sin(sunAngle);
			//sunVec.z = 0.0f;
			sunVec.z = (float)sin(sunAngle);

			sunVec = -sunVec;

            newSunVec[0] = (uint8)(char)(sunVec.x * 127.0f);
            newSunVec[1] = (uint8)(char)(sunVec.y * 127.0f);
            newSunVec[2] = (uint8)(char)(sunVec.z * 127.0f);

            m_TODCounter = 0.0f;
		}
		else
		{
			newColor[0] = m_WorldTimeColor[0];
			newColor[1] = m_WorldTimeColor[1];
			newColor[2] = m_WorldTimeColor[2];

			newSunVec[0] = m_SunVec[0];
			newSunVec[1] = m_SunVec[1];
			newSunVec[2] = m_SunVec[2];
		}
	}

	if (m_ShowTimeTrack.GetFloat() != 0.0f)
	{
        g_pLTServer->CPrint("TOD: %.2f, Color: %d %d %d", TODSecondsToHours(m_TODSeconds), newColor[0], newColor[1], newColor[2]);
	}

	// Did it change?
	if (newColor[0] != m_WorldTimeColor[0] ||
		newColor[1] != m_WorldTimeColor[1] ||
		newColor[2] != m_WorldTimeColor[2] ||
		newSunVec[0] != m_SunVec[0] ||
		newSunVec[1] != m_SunVec[1] ||
		newSunVec[2] != m_SunVec[2])
	{
		m_WorldTimeColor[0] = newColor[0];
		m_WorldTimeColor[1] = newColor[1];
		m_WorldTimeColor[2] = newColor[2];

		m_SunVec[0] = newSunVec[0];
		m_SunVec[1] = newSunVec[1];
		m_SunVec[2] = newSunVec[2];

        SendTimeOfDay(LTNULL);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateGameServer
//
//	PURPOSE:	Updates a stand-alone server with game info if necessary
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::UpdateGameServer()
{
    // Check if we need to update...

	if (!m_bUpdateGameServ)
	{
        return(LTFALSE);
	}

    m_bUpdateGameServ = LTFALSE;


	// Make sure we are actually being hosted via GameServ...

	if (!m_bGameServHosted)
	{
        return(LTFALSE);
	}

	ServerAppShellUpdate( );

	// All done...
    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::AddClientToList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::AddClientToList(HCLIENT hClient)
{
	// Sanity checks...

    if (!hClient) return(LTFALSE);


	// Make sure this client isn't already in our list...

	if (IsClientInList(hClient))
	{
        return(LTTRUE);
	}


	// Add this client handle to our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
        if (m_aClients[i] == LTNULL)
		{
			m_aClients[i] = hClient;
            return(LTTRUE);
		}
	}


	// If we get here, there wasn't any space left in the array...

    return(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::RemoveClientFromList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::RemoveClientFromList(HCLIENT hClient)
{
	// Sanity checks...

    if (!hClient) return(LTFALSE);


	// Remove this client handle from our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
            m_aClients[i] = LTNULL;
            return(LTTRUE);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

    return(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::IsClientInList
//
//	PURPOSE:	Determines if the given client handle is in our list
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::IsClientInList(HCLIENT hClient)
{
	// Sanity checks...

    if (!hClient) return(LTFALSE);


	// Look for this client handle in our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
            return(LTTRUE);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

    return(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::GetPlayerFromClientList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

CPlayerObj*	CGameServerShell::GetPlayerFromClientList(HCLIENT hClient)
{
	// Sanity checks...

    if (!hClient) return(LTNULL);


	// Remove this client handle from our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);
			return(pPlayer);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

    return(LTNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetupGameInfo
//
//	PURPOSE:	Setup game info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetupGameInfo()
{
	NetGame* pGameInfo;
    uint32 dwLen = sizeof(NetGame);
    g_pLTServer->GetGameInfo((void**)&pGameInfo, &dwLen);

	if (pGameInfo)
	{
		memcpy(&m_GameInfo, pGameInfo, sizeof(NetGame));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetServerOptions
//
//	PURPOSE:	Set server options
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetServerOptions()
{
	int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
	if (nNumOptions > MAX_GAME_OPTIONS)
		nNumOptions = MAX_GAME_OPTIONS;

	for (int i = 0; i < nNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			pOpt->SetValue(m_GameInfo.m_fOptions[i]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateBoundingBoxes()
//
//	PURPOSE:	Update object bounding boxes
//
// ----------------------------------------------------------------------- //

void CGameServerShell::UpdateBoundingBoxes()
{
    HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hClass = g_pLTServer->GetClass("GameBase");

	// Active objects...

	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
            GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->UpdateBoundingBox();
			}
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Inactive objects...

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
            GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->UpdateBoundingBox();
			}
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::RemoveBoundingBoxes()
//
//	PURPOSE:	Remove object bounding boxes
//
// ----------------------------------------------------------------------- //

void CGameServerShell::RemoveBoundingBoxes()
{
    HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hClass = g_pLTServer->GetClass("GameBase");

	// Active objects...

	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
            GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->RemoveBoundingBox();
			}
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Inactive objects...

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
            GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->RemoveBoundingBox();
			}
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}
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
	if (GetGameType() == SINGLE) return;

//	if (GetGameType() == COOPERATIVE_ASSAULT && m_bShowMultiplayerSummary)
	if (m_bShowMultiplayerSummary)
	{
		LTBOOL bReady = LTTRUE;
		if (m_fSummaryEndTime > g_pLTServer->GetTime())
		{
			for (int i = 0; i < MAX_CLIENTS && bReady; i++)
			{
				CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
				if (pPlayer)
				{
					if (!pPlayer->IsReadyToExit())
					{
						bReady = LTFALSE;
						break;
					}
				}
			}
		}
		if (bReady)
		{
			m_bStartNextMultiplayerLevel = LTTRUE;
		}
		return;

	}

    uint8 byEnd = NGE_NEVER;
	if (GetGameType() == COOPERATIVE_ASSAULT)
	{
		byEnd = (uint8)g_NetCAGameEnd.GetFloat();
	}
	else
	{
		byEnd = (uint8)g_NetDMGameEnd.GetFloat();
	}


	m_eLevelEnd = LE_UNKNOWN;


    LTBOOL bStartLevel = LTFALSE;

	if (byEnd == NGE_TIME || byEnd == NGE_FRAGSANDTIME)
	{
        LTFLOAT fEndLevelTime = (g_NetEndTime.GetFloat() * 60.0f);
        LTFLOAT fTime = g_pLTServer->GetTime();

		if (fTime >= fEndLevelTime)
		{
			m_eLevelEnd = LE_TIMELIMIT;
            bStartLevel = LTTRUE;

		}
	}

	if (!bStartLevel && (byEnd == NGE_FRAGS || byEnd == NGE_FRAGSANDTIME))
	{
		if (GetGameType() == DEATHMATCH)
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
				if (pPlayer)
				{
					if (pPlayer->GetFragCount() >= (int)g_NetEndFrags.GetFloat())
					{
						m_eLevelEnd = LE_FRAGLIMIT;
						bStartLevel = LTTRUE;

						break;
					}
				}
			}
		}
		else
		{
			CTeam *pTeam = m_TeamMgr.GetFirstTeam();
			while (pTeam)
			{
				if (pTeam->GetScore() >= (int)g_NetEndScore.GetFloat())
				{
					m_eLevelEnd = LE_FRAGLIMIT;
					bStartLevel = LTTRUE;
					break;

				}
				pTeam = m_TeamMgr.GetNextTeam(pTeam);
			}
		}
	}

	if (!bStartLevel && g_NetUseSpawnLimit.GetFloat() > 0.0f)
	{
		LTBOOL bOneFound = LTFALSE;
		LTBOOL bOneAlive1 = LTFALSE;
		LTBOOL bOneAlive2 = LTFALSE;
		for (int i = 0; i < MAX_CLIENTS && !(bOneAlive1 && bOneAlive2); i++)
		{
			CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
			if (pPlayer)
			{

				if (pPlayer->GetState() == PS_GHOST)
				{
					bOneFound = LTTRUE;
				}
				else
				{
					if (pPlayer->GetTeamID() == 1)
						bOneAlive1 = LTTRUE;
					if (pPlayer->GetTeamID() == 2)
						bOneAlive2 = LTTRUE;
				}
			}

		}
		if (bOneFound)
		{
			if (bOneAlive1 && !bOneAlive2)
			{
				m_eLevelEnd = LE_TEAM1_WIN;
				bStartLevel = LTTRUE;

			}
			else if (!bOneAlive1 && bOneAlive2)
			{
				m_eLevelEnd = LE_TEAM2_WIN;
				bStartLevel = LTTRUE;
			}
			else if (!bOneAlive1 && !bOneAlive2)
			{
				m_eLevelEnd = LE_DRAW;
				bStartLevel = LTTRUE;
			}
		}

	}

	if (bStartLevel)
	{
//		if (GetGameType() == COOPERATIVE_ASSAULT)
			ShowMultiplayerSummary();
//		else
//			m_bStartNextMultiplayerLevel = LTTRUE;
	}
}

LTRESULT CGameServerShell::SwitchToWorld(char *pWorldName, char *pNextWorldName)
{
    LTRESULT dResult;

	// Tell all clients we're changing levels
    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage (LTNULL, MID_CHANGING_LEVELS);
	if (GetGameType() != SINGLE)
	{
		HSTRING hWorld = g_pLTServer->CreateString(pWorldName);
		g_pLTServer->WriteToMessageHString(hWrite, hWorld);
		g_pLTServer->FreeString(hWorld);
	}
    g_pLTServer->EndMessage (hWrite);



	// Clear player frags...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
		{
			pPlayer->SetFragCount(0);
			pPlayer->SetRespawnCount(0);
		}
	}


	// Create the transition team id list so we can properly restore the teams...
	m_TeamMgr.CreateTeamTransIDs();

	// Load the next level...
	if (pWorldName)
	{
        dResult = g_pLTServer->LoadWorld(pWorldName, LOADWORLD_LOADWORLDOBJECTS|LOADWORLD_RUNWORLD);
		if(dResult != LT_OK)
		{
			return dResult;
		}
	}
	else
	{
        g_pLTServer->BPrint("ERROR CAN'T START NEXT MULTIPLAYER LEVEL!");
		return LT_ERROR;
	}


	// Update the new world name.
	char sCurLevel[128];
	SAFE_STRCPY(sCurLevel, m_GameInfo.m_sLevels[m_nCurLevel]);
	strncpy(m_sWorld, sCurLevel, MAX_GEN_STRING);

	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::StartNextMultiplayerLevel
//
//	PURPOSE:	Start the next multiplayer level
//
// ----------------------------------------------------------------------- //

void CGameServerShell::StartNextMultiplayerLevel()
{
	m_bShowMultiplayerSummary = LTFALSE;

	if (++m_nCurLevel >= m_GameInfo.m_byNumLevels)
	{
		m_nCurLevel = 0;
	}

	char* pLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	int i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	char *pNextLevelName = m_GameInfo.m_sLevels[i];


	for (i = 0; i <= NUM_TEAMS; i++)
	{
		m_Objectives[i].Clear();
		m_CompletedObjectives[i].Clear();
	}

	SwitchToWorld(pLevelName, pNextLevelName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppMessageFn
//
//	PURPOSE:	Server app message function
//
// ----------------------------------------------------------------------- //

LTRESULT CGameServerShell::ServerAppMessageFn(char* sMsg, int nLen)
{
	// Sanity checks...

	if (!sMsg) return(LT_OK);

	istrstream memStream( sMsg, nLen );

	char nVal;
	memStream >> nVal;

	switch( nVal )
	{
		case SERVERSHELL_INIT:
			m_bGameServHosted = LTTRUE;
			break;
		case SERVERSHELL_NEXTWORLD:
			StartNextMultiplayerLevel( );
			break;
		case SERVERSHELL_SETWORLD:
			// Read in the world index.  Go back one, since StartNextMultiplayerLevel
			// will preincrement the curlevel.
			memStream >> m_nCurLevel;
			m_nCurLevel--;
			StartNextMultiplayerLevel( );
			break;
		// Invalid message.
		default:
			ASSERT( FALSE );
			break;
	}

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PauseGame
//
//	PURPOSE:	Pause/unpause the game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PauseGame(LTBOOL bPause)
{
    uint32 nFlags = g_pLTServer->GetServerFlags();

	if (bPause)
	{
		nFlags |= SS_PAUSED;
	}
	else
	{
		nFlags &= ~SS_PAUSED;
	}

    g_pLTServer->SetServerFlags (nFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleCmdMsg()
//
//	PURPOSE:	Handle console cmd messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleCmdMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
    HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hMessage);
	if (!hMsg) return;

    if (m_CmdMgr.Process(g_pLTServer->GetStringData(hMsg)))
	{
        g_pLTServer->CPrint("Sent Command '%s'", g_pLTServer->GetStringData(hMsg));
	}

	if (hMsg)
	{
        g_pLTServer->FreeString(hMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleTriggerMsg()
//
//	PURPOSE:	Handle console trigger messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleTriggerMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

		// Read the message data...

    HSTRING hstrObjName = g_pLTServer->ReadFromMessageHString(hMessage);
    HSTRING hstrMsg     = g_pLTServer->ReadFromMessageHString(hMessage);

    char* pName = LTNULL;
    char* pMsg  = LTNULL;

	if (hstrObjName)
	{
        pName = g_pLTServer->GetStringData(hstrObjName);
	}

	if (hstrMsg)
	{
        pMsg = g_pLTServer->GetStringData(hstrMsg);
	}

	// Special case if we're supposed to list objects of a certain type...

	if (_strnicmp(pMsg, "LIST", 4) == 0)
	{
        ILTCommon* pCommon = g_pLTServer->Common();
		if (!pCommon) return;

		ConParse parse;
		parse.Init(pMsg);

        LTBOOL bNoObjects = LTTRUE;

		if (pCommon->Parse(&parse) == LT_OK)
		{
			if (parse.m_nArgs > 1)
			{
                g_pLTServer->CPrint("Listing objects of type '%s'", parse.m_Args[1]);

                HCLASS  hClass = g_pLTServer->GetClass(parse.m_Args[1]);

				// Get the names of all the objects of the specified class...

                HOBJECT hObj = g_pLTServer->GetNextObject(LTNULL);
				while (hObj)
				{
                    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
					{
                        g_pLTServer->CPrint("%s (active)", g_pLTServer->GetObjectName(hObj));
                        bNoObjects = LTFALSE;
					}

                    hObj = g_pLTServer->GetNextObject(hObj);
				}

                hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
				while (hObj)
				{
                    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
					{
                        g_pLTServer->CPrint("%s (inactive)", g_pLTServer->GetObjectName(hObj));
                        bNoObjects = LTFALSE;
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
	else if (SendTriggerMsgToObjects(pPlayer, hstrObjName, hstrMsg))
	{
        g_pLTServer->CPrint("Sent '%s' Msg '%s'", pName ? pName : "Invalid Object!", pMsg ? pMsg : "Empty Message!!!");
	}
	else
	{
        g_pLTServer->CPrint("Failed to Send '%s' Msg '%s'!", pName ? pName : "Invalid Object!", pMsg ? pMsg : "Empty Message!!!");
	}

	if (hstrObjName)
	{
        g_pLTServer->FreeString(hstrObjName);
	}

	if (hstrMsg)
	{
        g_pLTServer->FreeString(hstrMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerExitLevel()
//
//	PURPOSE:	Handle exit level command
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerExitLevel(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	if (!hSender) return;

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

//	if (GetGameType() == COOPERATIVE_ASSAULT && m_bShowMultiplayerSummary)
	if (m_bShowMultiplayerSummary)
	{
		pPlayer->ReadyToExit(LTTRUE);
		return;
	}


    HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hClass = g_pLTServer->GetClass("ExitTrigger");

    HOBJECT hRemoveObj = LTNULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pPlayer, hObj, 0, "TRIGGER");
			return;
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}


    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pPlayer, hObj, 0, "TRIGGER");
			return;
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}


	// Okay, there was no exit trigger in the level...Force an exit...

	ExitLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ExitLevel()
//
//	PURPOSE:	Exit the current level
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ExitLevel(LTBOOL bSendMsgToClient)
{

	GameType eGameType = GetGameType();
	switch (eGameType)
	{
		case DEATHMATCH:
//			m_bStartNextMultiplayerLevel = LTTRUE;
//			break;
		case COOPERATIVE_ASSAULT:
			ShowMultiplayerSummary();
			break;

		// If single player, update the player summary...

		case SINGLE:
		{
            HOBJECT hPlayerObj = LTNULL;
			ObjArray <HOBJECT, 1> objArray;

            g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
			if (!objArray.NumObjects()) return;

			hPlayerObj = objArray.GetObject(0);
			if (!hPlayerObj || !IsPlayer(hPlayerObj)) return;

            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hPlayerObj);
			if (!pPlayer || pPlayer->IsDead()) return;

			// Tell the player to handle exiting...
			pPlayer->HandleExit();

			CPlayerSummaryMgr* pPSMgr = pPlayer->GetPlayerSummaryMgr();
			if (pPSMgr)
			{
				pPSMgr->HandleLevelEnd(pPlayer);
			}

			if (bSendMsgToClient)
			{
				HMESSAGEWRITE hMsg = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_EXITLEVEL);
				g_pLTServer->EndMessage(hMsg);
			}
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerSummary()
//
//	PURPOSE:	Handle the player summary request
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerSummary(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	if (!hSender) return;

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

	CPlayerSummaryMgr* pPlayerSummary = pPlayer->GetPlayerSummaryMgr();
	if (pPlayerSummary)
	{
		pPlayerSummary->SendDataToClient(hSender);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerChangeTeam()
//
//	PURPOSE:	Handle the player summary request
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerChangeTeam(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	if (!hSender || GetGameType() != COOPERATIVE_ASSAULT) return;

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

    uint8 byNewTeam = g_pLTServer->ReadFromMessageByte(hMessage);
    uint32 dwPlayerID = g_pLTServer->GetClientID(hSender);

	CTeam* pOldTeam = m_TeamMgr.GetTeamFromPlayerID(dwPlayerID);

    CTeam* pTeam = LTNULL;


	if (byNewTeam == TEAM_AUTO)
		pTeam = m_TeamMgr.GetTeamWithLeastPlayers(TRUE);
	else
		pTeam = m_TeamMgr.GetTeam(byNewTeam);

	if (pTeam && pTeam != pOldTeam)
	{
        if (m_TeamMgr.ChangePlayerTeam(dwPlayerID, pTeam, LTTRUE))
			RespawnPlayer(pPlayer, hSender);
		SendGameDataToClient(hSender);
        SendPlayerInfoMsgToClients(LTNULL, pPlayer, LTFALSE);

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleUpdateServerOptions()
//
//	PURPOSE:	Handle updates to the server options
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleUpdateServerOptions(HMESSAGEREAD hMessage)
{
	if (GetGameType() == SINGLE) return;
	int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
	if (nNumOptions > MAX_GAME_OPTIONS)
		nNumOptions = MAX_GAME_OPTIONS;
	for (int i = 0; i < nNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			LTFLOAT fVal = g_pLTServer->ReadFromMessageFloat(hMessage);
			pOpt->SetValue(fVal);
		}
	}
	SendServerOptionsToClient(LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Save
//
//	PURPOSE:	Save the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	m_charMgr.Save(hWrite);
	m_CmdMgr.Save(hWrite);

	{ // BL 09/28/00 need to save/load these because they don't
	  // get set by worldproperties in a load situation

		SAVE_DWORD(m_nTimeRamps);
		SAVE_DWORD(m_iPrevRamp);
		for ( uint32 iTimeRamp = 0 ; iTimeRamp < MAX_TIME_RAMPS ; iTimeRamp++ )
		{
			SAVE_FLOAT(m_TimeRamps[iTimeRamp].m_Time);
			SAVE_VECTOR(m_TimeRamps[iTimeRamp].m_Color);
			SAVE_HSTRING(m_TimeRamps[iTimeRamp].m_hTarget);
			SAVE_HSTRING(m_TimeRamps[iTimeRamp].m_hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Load
//
//	PURPOSE:	Load the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	_ASSERT(hRead);
	if (!hRead) return;

	m_charMgr.Load(hRead);
	m_CmdMgr.Load(hRead);

	{ // BL 09/28/00 need to save/load these because they don't
	  // get set by worldproperties in a load situation

		LOAD_DWORD(m_nTimeRamps);
		LOAD_DWORD(m_iPrevRamp);
		for ( uint32 iTimeRamp = 0 ; iTimeRamp < MAX_TIME_RAMPS ; iTimeRamp++ )
		{
			LOAD_FLOAT(m_TimeRamps[iTimeRamp].m_Time);
			LOAD_VECTOR(m_TimeRamps[iTimeRamp].m_Color);
			LOAD_HSTRING(m_TimeRamps[iTimeRamp].m_hTarget);
			LOAD_HSTRING(m_TimeRamps[iTimeRamp].m_hMessage);
		}
	}
}




TimeRamp* CGameServerShell::GetTimeRamp(uint32 i)
{
	if(i >= MAX_TIME_RAMPS)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return &m_TimeRamps[i];
}

void CGameServerShell::FillGameData(GAMEDATA *gameData)
{
	static CVarTrack cvRunSpeed;
	static CVarTrack cvRespawnScale;
	if (!cvRunSpeed.IsInitted())
	{
        cvRunSpeed.Init(g_pLTServer, "RunSpeed", LTNULL, 1.0f);
        cvRespawnScale.Init(g_pLTServer, "RespawnScale", LTNULL, 1.0f);
	}
	gameData->m_fRunSpeed = cvRunSpeed.GetFloat();
	gameData->m_fRespawnScale = cvRespawnScale.GetFloat();

	if (GetGameType() == COOPERATIVE_ASSAULT)
	{
	    gameData->m_dwEndFrags = (uint32)g_NetEndScore.GetFloat();
		gameData->m_byEnd = (uint8)g_NetCAGameEnd.GetFloat();
	}
	else
	{
		gameData->m_byEnd = (uint8)g_NetDMGameEnd.GetFloat();
	    gameData->m_dwEndFrags = (uint32)g_NetEndFrags.GetFloat();
	}
    gameData->m_dwEndTime = (uint32)g_NetEndTime.GetFloat();

	gameData->m_bUsePassword = m_GameInfo.m_bUsePassword;
	SAFE_STRCPY(gameData->m_szPassword,m_GameInfo.m_sPassword);

}

ObjectivesList* CGameServerShell::GetObjectives(uint8 byTeam)
{
    if (byTeam > NUM_TEAMS) return LTNULL;
	return &m_Objectives[byTeam];
}
ObjectivesList* CGameServerShell::GetCompletedObjectives(uint8 byTeam)
{
    if (byTeam > NUM_TEAMS) return LTNULL;
	return &m_CompletedObjectives[byTeam];
}

void CGameServerShell::ShowMultiplayerSummary()
{
	m_bShowMultiplayerSummary = LTTRUE;
	int endString = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
		{
			pPlayer->ReadyToExit(LTFALSE);
		}
	}
	if (GetGameType() == COOPERATIVE_ASSAULT && (m_eLevelEnd == LE_FRAGLIMIT || m_eLevelEnd == LE_TIMELIMIT))
	{
		uint32 team1Score = 0;
		uint32 team2Score = 0;
		CTeam* pTeam = m_TeamMgr.GetTeam(TEAM_1);
		if (pTeam) team1Score = pTeam->GetScore();
		pTeam = m_TeamMgr.GetTeam(TEAM_2);
		if (pTeam) team2Score = pTeam->GetScore();
		if (team1Score > team2Score)
			m_eLevelEnd = LE_TEAM1_WIN;
		else if (team2Score > team1Score)
			m_eLevelEnd = LE_TEAM2_WIN;
		else
			m_eLevelEnd = LE_DRAW;

	}
	m_fSummaryEndTime = g_pServerButeMgr->GetSummaryDelay() + g_pLTServer->GetTime();
	HMESSAGEWRITE hMsg = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_EXITLEVEL);
    g_pLTServer->WriteToMessageByte(hMsg, (uint8)m_eLevelEnd);
    g_pLTServer->WriteToMessageDWord(hMsg, (uint32)endString);
    g_pLTServer->EndMessage(hMsg);

}


// Game spy stuff...
LTBOOL CGameServerShell::InitGameSpy()
{
	// If we're not supposed to use game spy, just return...
	if (m_bDontUseGameSpy) return(LTFALSE);

	// Setup the display name...
	SAFE_STRCPY(m_sSpyGameHost, m_GameInfo.m_sSession);

	// Get our ip address and port...
	char sBuf[32];
	int  nBufSize = 30;
	WORD wPort = 0;
    g_pLTServer->GetTcpIpAddress(sBuf, nBufSize, wPort);

	// Init the game spy manager.
	char sVer[16];
	sprintf(sVer,"%d.%.3d",GAME_HANDSHAKE_VER_MAJOR,(GAME_HANDSHAKE_VER_MINOR-1));
	if (m_GameSpyMgr.Init(GAMESPY_GAME_NAME, sVer, GAMESPY_SECRET_KEY, 0, wPort, (wPort+166), GSMF_USEGAMEPORTFORHEARTBEAT))
	{
		m_GameSpyMgr.SetSendHandler(&m_SendHandler);
	}
	else
	{
		m_bDontUseGameSpy = LTTRUE;
		return(LTFALSE);
	}

	// All done.
	return(LTTRUE);
}

void CGameServerShell::TermGameSpy()
{
	m_GameSpyMgr.Term();
}

LTBOOL CGameServerShell::UpdateGameSpyMgr()
{
	// Sanity checks.
	if (m_bDontUseGameSpy) return(LTFALSE);

	// Update the GameSpy manager.
	m_GameSpyMgr.Update();

	// All done.
	return(LTTRUE);
}

LTRESULT CGameServerShell::ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort)
{
	if (!m_bDontUseGameSpy)
	{
		if(dataLen > 0 && sData[0] == '\\')
		{
			char sAddr[128];
			sprintf(sAddr, "%d.%d.%d.%d", senderAddr[0], senderAddr[1], senderAddr[2], senderAddr[3]);
			m_GameSpyMgr.OnQuery(sAddr, senderPort, sData, dataLen);
		}
	}

	return(LT_OK);
}

CPlayerObj* CGameServerShell::GetFirstNetPlayer()
{
	m_nGetPlayerIndex = 0;
	CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[m_nGetPlayerIndex]);
	while (!pPlayer && m_nGetPlayerIndex < MAX_CLIENTS-1)
	{
		m_nGetPlayerIndex++;
		pPlayer = GetPlayerFromClientList(m_aClients[m_nGetPlayerIndex]);
	}

	return(pPlayer);
}

CPlayerObj* CGameServerShell::GetNextNetPlayer()
{
	CPlayerObj* pPlayer = NULL;
	while (!pPlayer && m_nGetPlayerIndex < MAX_CLIENTS-1)
	{
		m_nGetPlayerIndex++;
		pPlayer = GetPlayerFromClientList(m_aClients[m_nGetPlayerIndex]);
	}

	return(pPlayer);
}

int	CGameServerShell::GetPlayerPing(CPlayerObj* pPlayer)
{
	if (!pPlayer) return(0);

	float ping = 0.0f;

	HCLIENT hClient = pPlayer->GetClient();
	if (!hClient) return(0);

	uint32 clientID = g_pLTServer->GetClientID(hClient);
	g_pLTServer->GetClientPing(hClient, ping);

	return((int)(ping*1000.0f));
}

char* CGameServerShell::GetHostName()
{
	return(m_sSpyGameHost);
}

char* CGameServerShell::GetCurLevel()
{
	return(m_sWorld);
}

char* CGameServerShell::GetGameSpyGameType()
{
	int nType = GetGameType();
	switch (nType)
	{
		case COOPERATIVE_ASSAULT:	return("H.A.R.M. vs. UNITY");
		default:					return("deathmatch");
	}
}

int	CGameServerShell::GetNumPlayers()
{
	CPlayerObj* pPlr   = GetFirstNetPlayer();
	int         nCount = 0;

	while (pPlr)
	{
		nCount++;
		pPlr = GetNextNetPlayer();
	}

	return(nCount);
}

int	CGameServerShell::GetMaxPlayers()
{
	int nMaxPlrs = (int)g_MaxPlayers.GetFloat();
	return(nMaxPlrs);
}

LTBOOL CGameServerShell::FillGameDataStruct(GAMEDATA* pGD)
{
	if (!pGD) return(LTFALSE);

	memset(pGD, 0, sizeof(GAMEDATA));
	FillGameData(pGD);

	return(LTTRUE);
}

void CGameServSendHandler::SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port)
{
	g_pLTServer->SendTo(pData, len, sAddr, port);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::IsPositionOccupied
//
//	PURPOSE:	Check for any other player object's at this position
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::IsPositionOccupied(LTVector & vPos, CPlayerObj* pPlayer)
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	int numObjects = objArray.NumObjects();

	if (!numObjects) return LTFALSE;

	HOBJECT hPlayerObj = LTNULL;
	if (pPlayer)
	{
		hPlayerObj = pPlayer->m_hObject;
	}

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);


		if (hObject != hPlayerObj)
		{
            LTVector vObjPos, vDims;
            g_pLTServer->GetObjectPos(hObject, &vObjPos);
            g_pLTServer->GetObjectDims(hObject, &vDims);

			// Increase the size of the dims to account for the players
			// dims overlapping...

			vDims *= 2.0f;

			if (vObjPos.x - vDims.x < vPos.x && vPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vPos.y && vPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vPos.z && vPos.z < vObjPos.z + vDims.z)
			{
				return LTTRUE;
			}
		}
	}
	return LTFALSE;
}

void CGameServerShell::ServerAppAddClient( HCLIENT hClient )
{
	char szBuf[1024];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write out the player information.
	memStream << (( BYTE )SERVERAPP_ADDCLIENT ) << endl;
	memStream << (( int )g_pLTServer->GetClientID( hClient )) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}

void CGameServerShell::ServerAppRemoveClient( HCLIENT hClient )
{
	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write out the player information.
	memStream << (( BYTE )SERVERAPP_REMOVECLIENT ) << endl;
	memStream << (( int )g_pLTServer->GetClientID( hClient )) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}

void CGameServerShell::ServerAppShellUpdate( )
{
	int i;
	char szBuf[1024];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	// Write the message id.
	memStream << (( BYTE )SERVERAPP_SHELLUPDATE ) << endl;

	// Add info for each player.
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if( !pPlayer )
			continue;

		HCLIENT hClient = pPlayer->GetClient();
		if( !hClient )
			continue;

		memStream << (( int )g_pLTServer->GetClientID( hClient )) << endl;
		memStream << pPlayer->GetNetName( ) << endl;
		memStream << (( int )pPlayer->GetFragCount( )) << endl;
	}

	// Signal end of player list.
	memStream << (( int )-1 ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}

void CGameServerShell::ServerAppPreStartWorld( )
{
	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	memStream << (( BYTE )SERVERAPP_PRELOADWORLD ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}


void CGameServerShell::ServerAppPostStartWorld( )
{
	char szBuf[32];
	ostrstream memStream( szBuf, sizeof( szBuf ));

	memStream << (( BYTE )SERVERAPP_POSTLOADWORLD ) << endl;
	memStream << (( int )m_nCurLevel ) << endl;

	// Send to the server app.
	g_pLTServer->SendToServerApp( szBuf, sizeof( szBuf ));
}


// ----------------------------------------------------------------------- //
// Sends a game data message to the specified client.
// ----------------------------------------------------------------------- //
void CGameServerShell::SendServerOptionsToClient(HCLIENT hClient)
{
	if (GetGameType() == SINGLE) return;

	HMESSAGEWRITE hWrite;
    hWrite = g_pLTServer->StartMessage(hClient, MID_UPDATE_OPTIONS);

    g_pLTServer->WriteToMessageString(hWrite, m_GameInfo.m_sSession);

	int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
	if (nNumOptions > MAX_GAME_OPTIONS)
		nNumOptions = MAX_GAME_OPTIONS;
	for (int i = 0; i < nNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			LTFLOAT fVal = pOpt->GetValue();
			g_pLTServer->WriteToMessageFloat(hWrite,fVal);
			
		}
	}

    g_pLTServer->EndMessage(hWrite);
}
