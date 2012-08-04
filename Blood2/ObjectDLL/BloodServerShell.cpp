// ----------------------------------------------------------------------- //
//
// MODULE  : BloodServerShell.cpp
//
// PURPOSE : Blood's server shell - Implementation
//
// CREATED : 9/17/97
//
// ----------------------------------------------------------------------- //

#include "BloodServerShell.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "PathMgr.h"
#include "ClientServerShared.h"
#include "FileCaching.h"
#include "ai_mgr.h"
#include "sparam.h"
#include "CameraObj.h"
#include "CVarTrack.h"
#include "CultistAI.h"

#include <windows.h>  // For DebugBreak
#include <winbase.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>


#define MAX_CLIENT_NAME_LENGTH		100
#define CLIENT_PING_UPDATE_RATE		7.5f
#define WAIT_TIME_FOR_LEVEL_SWITCH	7.5f

char g_szVarDoAutosave[]		= "GAME_DOAUTOSAVE";
char g_szVarRevisiting[]		= "GAME_REVISITING";
char g_szVarRestoring[]			= "GAME_RESTORING";
char g_szVarGotoStartpoint[]	= "GAME_GOTOSTARTPOINT";
char g_szVarWorldName[]			= "GAME_WORLDNAME";
char g_szVarDifficulty[]		= "GAME_DIFFICULTY";
char g_szVarGameType[]			= "GAME_GAMETYPE";
char g_szSavePath[]				= "Save\\Current\\";
char g_szAutoSaveFile[]			= "Auto.sav";
char g_szCurrentSaveFile[]		= "Current.sav";

DBOOL	g_bNextLevel = DFALSE;
DBOOL	g_bWaitToStartNextLevel = DFALSE;
DFLOAT	g_fWaitTimeForNextLevel = 0.0f;

CVarTrack	g_SayTrack;

extern	BOOL	g_bLevelChangeCharacter;
extern	int		g_nLevelChangeCharacter;


SETUP_SERVERSHELL()
DEFINE_CLASSES()

CBloodServerShell* g_pBloodServerShell = DNULL;

ServerShellDE* CreateServerShell(ServerDE *pServerDE)
{
	g_pServerDE = pServerDE;

	CBloodServerShell *pShell = new CBloodServerShell;

	g_pBloodServerShell = pShell;

	return (ServerShellDE*)pShell;
}


void DeleteServerShell(ServerShellDE *pInputShell)
{
	CBloodServerShell *pShell = (CBloodServerShell*)pInputShell;

	// Only set to NULL if we are deleting what we think is the current server shell..
	// gk
//	if (g_pBloodServerShell == pShell)
//		g_pBloodServerShell = DNULL;

	delete pShell;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::CBloodServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CBloodServerShell::CBloodServerShell()
{
	memset(&m_GameInfo, 0, sizeof(NetGame));

	m_hstrStartPointName = DNULL;

#ifdef _ADD_ON
	m_bAddonLevel = DFALSE;
#endif

	SetUpdateBlood2Serv();

	ClearClientList();

	SetupGameInfo();

	m_bBlood2ServHosted = DFALSE;
	m_nCurLevel			 = 0;

	if (!m_VoiceMgr.IsInited())
	{
		m_VoiceMgr.Init(g_pServerDE);
	}

	g_SayTrack.Term();

	// Reset the goto-start-point game con var...
	g_pServerDE->SetGameConVar(g_szVarGotoStartpoint, "1");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::~CBloodServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CBloodServerShell::~CBloodServerShell()
{
	if (!g_pServerDE) return;

	// Tell all clients that we are shutting down
	//HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(NULL, SMSG_SERVERSHUTDOWN);
	//g_pServerDE->EndMessage(hMessage);

	if (m_hstrStartPointName)
		g_pServerDE->FreeString(m_hstrStartPointName);

	m_VoiceMgr.Term();

	g_SayTrack.Term();
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnAddClient()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::OnAddClient(HCLIENT hClient)
{
}
*/	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnRemoveClient()
//
//	PURPOSE:	Remove a client
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::OnRemoveClient(HCLIENT hClient)
{
	CPlayerObj *pPlayer;
	ServerDE *pServerDE = GetServerDE();

	if(pPlayer = (CPlayerObj*)pServerDE->GetClientUserData(hClient))
	{
		pPlayer->SetClient(DNULL);
	}
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Handle client entering the world
//
// ----------------------------------------------------------------------- //

BaseClass* CBloodServerShell::OnClientEnterWorld(HCLIENT hClient, void* pClientData, DDWORD dwClientDataLen)
{
	if (!g_pServerDE || !hClient) return DNULL;

	BaseClass* pObject  = DNULL;
	DBOOL bFoundClient = DFALSE;

	char szClientName[MAX_CLIENT_NAME_LENGTH];
	char szClientRefName[MAX_CLIENT_NAME_LENGTH];
	szClientName[0] = szClientRefName[0] = '\0';

	g_pServerDE->GetClientName(hClient, szClientName, MAX_CLIENT_NAME_LENGTH-1);

	// Search through the client refs to see if any of them match..
	HCLIENTREF hClientRef = g_pServerDE->GetNextClientRef(DNULL);
	while (hClientRef)
	{
		// See if this client reference is local or not...
		if (g_pServerDE->GetClientRefInfoFlags(hClientRef) & CIF_LOCAL)
		{
			bFoundClient = DTRUE;
		}

		// Determine if there is a reference to a client with the same name...
		if (!bFoundClient && g_pServerDE->GetClientRefName(hClientRef, szClientRefName, MAX_CLIENT_NAME_LENGTH-1))
		{
			if (szClientName[0] && szClientRefName[0])
			{
				if (_mbsicmp((const unsigned char*)szClientName, (const unsigned char*)szClientRefName) == 0)
				{
					bFoundClient = DTRUE;
				}
			}
		}

		// See if we found the right client...
		if (bFoundClient)
		{
			HOBJECT	hObject = g_pServerDE->GetClientRefObject(hClientRef);
			pObject = g_pServerDE->HandleToObject(hObject);

			if (pObject)
				RespawnPlayer(pObject, hClient);
			break;
		}

		hClientRef = g_pServerDE->GetNextClientRef(hClientRef);
	}

	// Add this client to the appropriate team...
	DWORD dwTeamID = TEAM_1;

	if (IsMultiplayerTeamBasedGame())
	{
		if (pClientData && dwClientDataLen == sizeof(NetClientData))
		{
			NetClientData* pNcd = (NetClientData*)pClientData;

			dwTeamID = pNcd->m_dwTeam;
		}

		if (dwTeamID == TEAM_AUTO)
		{
			CTeam* pTeam = m_TeamMgr.GetTeamWithLeastPlayers(TRUE);
			if (pTeam)
			{
				dwTeamID = pTeam->GetID();
			}
			else
			{
				if (IsRandomChance(50)) dwTeamID = TEAM_2;
				else dwTeamID = TEAM_1;
			}
		}

		DDWORD dwTransTeamID = m_TeamMgr.GetTeamTransID(g_pServerDE->GetClientID(hClient));
		CTeam* pTeam         = m_TeamMgr.GetTeam(dwTransTeamID);

		if (dwTransTeamID != TM_ID_NULL && pTeam)
		{
			dwTeamID = dwTransTeamID;
		}
	}

	m_TeamMgr.AddPlayer(dwTeamID, g_pServerDE->GetClientID(hClient));


	// See if we need to create a player (no matches found)...
	if (!pObject)
	{
		pObject = CreatePlayer(hClient);
		RespawnPlayer(pObject, hClient);
	}

	// Add this client to our local list...
	AddClientToList(hClient);

	// All done...
	return pObject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnClientExitWorld()
//
//	PURPOSE:	Remove a client
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::OnClientExitWorld(HCLIENT hClient)
{
	// Tell all clients..
	RemovePlayerMessage(hClient);

	// Remove this client from the team...
	m_TeamMgr.RemovePlayer(g_pServerDE->GetClientID(hClient));
	
	// Remove this client from our local list...
	RemoveClientFromList(hClient);
	SetUpdateBlood2Serv();

	// Remove the player object...
	CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->GetClientUserData(hClient);
	if (pPlayer) 
	{
		pPlayer->DropFlag(DFALSE);
		g_pServerDE->RemoveObject(pPlayer->m_hObject);
	}

	g_pServerDE->SetClientUserData(hClient, DNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::OnMessage(HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage)
{
	if (!g_pServerDE) return;
	switch(messageID)
	{
		case CMSG_NEXTLEVEL:
		{
			g_bNextLevel = DTRUE;
			break;
		}
		case CMSG_WEAPON_FIRE:
		{
			void *pData = g_pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponFireMessage(hMessage);
			}
		}
		break;

		case CMSG_WEAPON_SOUND:
		{
			void *pData = g_pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponSoundMessage(hMessage);
			}
		}
		break;

		case CMSG_WEAPON_STATE:
		{
			void *pData = g_pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponStateMessage(hMessage);
			}
		}
		break;

		case CMSG_WEAPON_CHANGE:
		{
			void *pData = g_pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				DBYTE nWeaponId = g_pServerDE->ReadFromMessageByte(hMessage);
				pPlayer->DoWeaponChange(nWeaponId);
			}
		}
		break;

		// Got a load world message
		case CMSG_LOADWORLD:
			{
				DBOOL bResult;
				bResult = LoadWorld(hMessage);
				// Acknowledge the load, good or bad
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(NULL, SMSG_LOADWORLD_ACK);
				g_pServerDE->WriteToMessageByte(hMessage, (DBYTE)bResult);
				g_pServerDE->EndMessage(hMessage);
			}
			break;

		case CMSG_SAVEGAME:
			{
				HMESSAGEREAD hClientSaveData;
				CPlayerObj *pPlayerObj;
				char *pFilename = g_pServerDE->ReadFromMessageString(hMessage);
				DBOOL bSaveType = g_pServerDE->ReadFromMessageByte( hMessage );
				hClientSaveData = g_pServerDE->ReadFromMessageHMessageRead( hMessage );
	
				pPlayerObj = ( CPlayerObj * )g_pServerDE->GetClientUserData(hSender);
				if( pPlayerObj )
					pPlayerObj->SetClientSaveData( hClientSaveData );				
				

				if (bSaveType == SAVETYPE_CURRENT)
				{
					if (SaveGame(SAVETYPE_CURRENT, DTRUE, DTRUE))
					{	// Acknowledge a successful save
						HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(NULL, SMSG_SAVEGAME_ACK);
						g_pServerDE->EndMessage(hMessage);
					}
				}
				else	// Do autosave
				{
					if (SaveGame(SAVETYPE_AUTO, DTRUE, DTRUE))
					{
						HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(NULL, SMSG_SAVEGAME_ACK);
						g_pServerDE->EndMessage(hMessage);
					}
				}
			}
			break;

		case CMSG_DETACH_AI:
			{
				HOBJECT hObj = g_pServerDE->ReadFromMessageObject(hMessage);
			
				if(g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), g_pServerDE->GetClass("AI_Mgr")))
				{
					AI_Mgr* pAI = (AI_Mgr*)g_pServerDE->HandleToObject(hObj);
					pAI->DetachFromEnemy();
				}

				break;
			}

		case CMSG_ATTACH_ACK:
			{
				HOBJECT hObj = g_pServerDE->ReadFromMessageObject(hMessage);
			
				if(g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), g_pServerDE->GetClass("AI_Mgr")))
				{
					AI_Mgr* pAI = (AI_Mgr*)g_pServerDE->HandleToObject(hObj);
					pAI->ProceedToAttach();
				}

				break;
			}

		case CMSG_GAME_PAUSE:
		{
			DBOOL bPause = (DBOOL)g_pServerDE->ReadFromMessageByte(hMessage);

			DDWORD nFlags = g_pServerDE->GetServerFlags();
			if (bPause)
				nFlags |= SS_PAUSED;
			else
				nFlags &= ~SS_PAUSED;

			g_pServerDE->SetServerFlags(nFlags);
		}
		break;

		case CMSG_MULTIPLAYER_INIT:
		{
			CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(hSender);
			if (pObj)
			{
				pObj->OnMessage(messageID, hMessage);

				// Tell other players that he's here
				AddPlayerMessage(DNULL, hSender);

				// tell new player about all current players (excluding himself)
				AddPlayersMessage(hSender);
				SetUpdateBlood2Serv();
			}
		}
		break;

		case CMSG_SINGLEPLAYER_INIT:
		{
			CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(hSender);
			if (pObj)
			{
				pObj->OnMessage(messageID, hMessage);
			}
		}
		break;

		case CMSG_AUTOSAVE:
		{
#ifndef _DEMO
			// Autosave current situation, to be able to restart the world
			if (GetGameConVarValueFloat(g_szVarDoAutosave))
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(hSender, SMSG_DOAUTOSAVE);
				g_pServerDE->EndMessage(hMessage);

/*				if (SaveGame(SAVETYPE_AUTO, DTRUE, DTRUE))
				{
					HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(NULL, SMSG_SAVEGAME_ACK);
					g_pServerDE->EndMessage(hMessage);
				}
*/
			}
#endif

		}
		break;

		default:	// Let the player handle it
		{
			CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(hSender);
			if (pObj)
				pObj->OnMessage(messageID, hMessage);
		}
	}
}

			
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::OnCommandOn()
//
//	PURPOSE:	Handle commands
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::OnCommandOn(HCLIENT hClient, int command)
{
	CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(hClient);
	if (pObj)
		pObj->OnCommandOn(command);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::PreStartWorld()
//
//	PURPOSE:	Handle stuff that needs to happen before a world is loaded.
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::PreStartWorld(DBOOL bSwitchingWorlds)
{
	// Setup the teams for multiplayer
	m_TeamMgr.Term();
	m_TeamMgr.Init();

	m_TeamMgr.AddTeam(TEAM_1, "TEAM1");
	m_TeamMgr.AddTeam(TEAM_2, "TEAM2");

	// Setup the game info
	SetupGameInfo();

	g_bNextLevel = DFALSE;

	// Reset global camera pointer
	g_hActiveCamera = DNULL;
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::PostStartWorld()
//
//	PURPOSE:	Perform an autosave after the world is loaded
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::PostStartWorld()
{
	// Build the list of AI paths for this world...
	m_PathMgr.BuildPathList();

	// Init the voice manager if necessary...
	if (!m_VoiceMgr.IsInited())
	{
		m_VoiceMgr.Init(g_pServerDE);
	}

	m_VoiceMgr.StopAll();
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::ConsoleMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::ConsoleMessage(HCLIENT hClient, char *msg)
{
	HMESSAGEWRITE hMsg;
	
	hMsg = g_pServerDE->StartMessage(hClient, SMSG_CONSOLEMESSAGE);
	g_pServerDE->WriteToMessageString(hMsg, msg);
	g_pServerDE->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::AddPlayerMessage()
//
//	PURPOSE:	Sends an AddPlayer message to all clients when a new player
//				comes in.
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::AddPlayerMessage(HCLIENT hDestClient, HCLIENT hNewPlayerClient)
{
	if (!hNewPlayerClient || !g_pServerDE) return;
	HMESSAGEWRITE hMsg;

	CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(hNewPlayerClient);
	if (pObj)
	{
		HSTRING hstrName = pObj->GetPlayerName();

 		DDWORD dwClientID = g_pServerDE->GetClientID(hNewPlayerClient);
		// Send the message to hDestClient, if it's NULL, all clients get this message
		hMsg = g_pServerDE->StartMessage(hDestClient, SMSG_ADDPLAYER);
		g_pServerDE->WriteToMessageHString(hMsg, hstrName);
		g_pServerDE->WriteToMessageDWord(hMsg, dwClientID);
		g_pServerDE->WriteToMessageByte(hMsg, pObj->GetCharacter());
		g_pServerDE->WriteToMessageFloat(hMsg, (DFLOAT) pObj->GetFrags());
		g_pServerDE->WriteToMessageByte(hMsg, (DBYTE)pObj->GetTeamID());
		g_pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::AddPlayersMessage()
//
//	PURPOSE:	Iterates through the client list and sends a SMSG_ADDPLAYER
//				message for each to hClient.
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::AddPlayersMessage(HCLIENT hClient)
{
	if (!hClient || !g_pServerDE) return;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] && hClient != m_aClients[i])
		{
			AddPlayerMessage(hClient, m_aClients[i]);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::RemovePlayerMessage()
//
//	PURPOSE:	Sends an RemovePlayer message to all clients when a player
//				leaves.
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::RemovePlayerMessage(HCLIENT hClient)
{
	if (!hClient || !g_pServerDE) return;
	
	DDWORD dwClientID = g_pServerDE->GetClientID(hClient);

	// Send the message to all clients
	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(DNULL, SMSG_REMOVEPLAYER);
	g_pServerDE->WriteToMessageDWord(hMsg, dwClientID);
	// [blg] g_pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	g_pServerDE->EndMessage(hMsg);
}

void CBloodServerShell::FragStatus()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associate it with the client.
//
// ----------------------------------------------------------------------- //

BaseClass* CBloodServerShell::CreatePlayer(HCLIENT hClient)
{
	if (!g_pServerDE) return DNULL;

	ObjectCreateStruct ocStruct;

	INIT_OBJECTCREATESTRUCT(ocStruct);

	ROT_INIT(ocStruct.m_Rotation);
	VEC_INIT(ocStruct.m_Pos);
	ocStruct.m_Flags = 0;

	HCLASS hClass = g_pServerDE->GetClass("CPlayerObj");

	BaseClass* pClass = NULL;
	if (hClass)
	{
		pClass = g_pServerDE->CreateObject(hClass, &ocStruct);
		if (pClass)
		{
			CPlayerObj* pPlayer = (CPlayerObj*)pClass;
			pPlayer->SetClient(hClient);
			g_pServerDE->SetClientUserData(hClient, (void *)pClass);
			pPlayer->Respawn();
//			pPlayer->GoToStartPoint();
			pPlayer->UpdateTeamID();
		}
	}

	return pClass;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::RespawnPlayer()
//
//	PURPOSE:	Respawn the player object
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::RespawnPlayer(BaseClass* pBaseObject, HCLIENT hClient)
{
	if (!pBaseObject || !hClient) return;

	CPlayerObj* pPlayer = (CPlayerObj*)pBaseObject;
	pPlayer->SetClient(hClient);
	g_pServerDE->SetClientUserData(hClient, (void *)pPlayer);

	// If this is a multiplayer game wait until MULTIPLAYER_INIT message
	// to respawn the player...

	DBYTE nMessage = SMSG_PLAYERINIT_MULTIPLAYER;

	if (GetGameType() == GAMETYPE_SINGLE)
	{
		nMessage = SMSG_PLAYERINIT_SINGLEPLAYER;
	}

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(hClient, nMessage);

	// If this is a multiplayer game, send some game info too...
	if (nMessage == SMSG_PLAYERINIT_MULTIPLAYER)
	{
		g_pServerDE->WriteToMessageByte(hWrite, m_GameInfo.m_byType);
		g_pServerDE->WriteToMessageByte(hWrite, m_GameInfo.m_bFriendlyFire);
		g_pServerDE->WriteToMessageByte(hWrite, m_GameInfo.m_bNegTeamFrags);
		g_pServerDE->WriteToMessageByte(hWrite, m_GameInfo.m_bOnlyFlagScores);
		g_pServerDE->WriteToMessageByte(hWrite, m_GameInfo.m_bOnlyGoalScores);
	}


	// End the message...
	g_pServerDE->EndMessage2(hWrite, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

	
	// Either a new level or restoring a previously visited level, so go to the startpoint
	if (IsMultiplayerGame() || GetGameConVarValueFloat(g_szVarGotoStartpoint))
	{
		pPlayer->GoToStartPoint();
	}
	// Restoring a save, so don't change player and tell the client where to point the camera
	else
	{
		pPlayer->SendMessageToClient(SMSG_FORCEROTATION);
		pPlayer->SendMessageToClient(SMSG_EYELEVEL);
	}

	// Reset the goto-start-point game con var...
	g_pServerDE->SetGameConVar(g_szVarGotoStartpoint, "1");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::SetStartpointName()
//
//	PURPOSE:	Sets the name of the next startpoint.
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::SetStartPointName(HSTRING hString)
{
	if (!hString || !g_pServerDE) return;

	if (m_hstrStartPointName)
	{
		g_pServerDE->FreeString(m_hstrStartPointName);
	}

	m_hstrStartPointName = g_pServerDE->CopyString(hString);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::LoadWorld()
//
//	PURPOSE:	handles the LOADWORLD message
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::LoadWorld(HMESSAGEREAD hMessage)
{
	char szPath[MAX_PATH];
	char *pFilename		= g_pServerDE->ReadFromMessageString(hMessage);
	DBYTE nGameType		= g_pServerDE->ReadFromMessageByte(hMessage);
	DBYTE nLoadType		= g_pServerDE->ReadFromMessageByte(hMessage);
	DBYTE nDifficulty	= g_pServerDE->ReadFromMessageByte(hMessage);

	CultistAI::ResetStatics();	// fix a wierd clown animation bug

	if (!pFilename || _mbstrlen(pFilename) <= 0) return DFALSE;

	DRESULT res			= DE_OK;
	DBOOL bRestoring	= DFALSE;
	DBOOL bRestoringKeepAlive = DFALSE;
	DBOOL bRevisiting	= DFALSE;

	// Remove leading slashes..
	while(*pFilename == '\\' || *pFilename == '/')
		pFilename++;

	// Init the voice manager if necessary...
	if (!m_VoiceMgr.IsInited())
	{
		m_VoiceMgr.Init(g_pServerDE);
	}

	m_VoiceMgr.StopAll();

//	// Current world name
//	char *pOldWorld		= g_pServerDE->GetVarValueString(g_pServerDE->GetGameConVar(g_szVarWorldName));

	// Set console variables
	// Save the filename in a console var for later use.
	g_pServerDE->SetGameConVar(g_szVarWorldName, szPath);

#ifdef _ADDON
	char sUpr[256];
	strncpy(sUpr, pFilename, 255);
	strupr(sUpr);
	if (strstr(sUpr, "WORLDS_AO"))
	{
		strncpy(szPath, pFilename, MAX_PATH);
		m_bAddonLevel = DTRUE;
	}
	else
	{
		_mbscpy((unsigned char*)szPath, (const unsigned char*)"Worlds\\");
		_mbscat((unsigned char*)szPath, (const unsigned char*)pFilename);
		m_bAddonLevel = DFALSE;
	}
#else
	_mbscpy((unsigned char*)szPath, (const unsigned char*)"Worlds\\");
	_mbscat((unsigned char*)szPath, (const unsigned char*)pFilename);
#endif

	// Set console variables
	// Save the filename in a console var for later use.
	g_pServerDE->SetGameConVar(g_szVarWorldName, szPath);

	// If this is a brand-new game, set various stuff
	if (nLoadType == LOADTYPE_NEW_GAME)
	{
		// Set difficulty level
		SetGameInfo(nGameType, nDifficulty);
	}

	// Restoring an autosave game
	if (nLoadType == LOADTYPE_RESTOREAUTOSAVE)
	{
		pFilename = g_szAutoSaveFile;	// Restore from "auto.sav" file
		bRestoring = DTRUE;
	}
	// Restoring a saved game
	else if (nLoadType == LOADTYPE_RESTORESAVE)
	{
		bRestoring = DTRUE;
	}
	// New level, so we need to save the current world and the player data
	else if (nLoadType == LOADTYPE_NEW_LEVEL)
	{
		// Check if this is an add-on level that wants a player change
		DoLevelChangeCharacterCheck(pFilename);

		// Save the player data
		KeepAliveSave();
		bRestoringKeepAlive = DTRUE;

		// Save the current world sans players or console, in case we want to go back
/*		Not doing this, so I'll just comment it out for now.. (Greg 10/5)
		if (pOldWorld)
			SaveGame(pOldWorld, DFALSE, DFALSE);

		// See if there is a save file for this level
		char szSavePath[MAX_PATH];
		_mbscpy((unsigned char*)szSavePath, (const unsigned char*)g_szSavePath);
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)pFilename);
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)".sav");

		// See if there is a save file for this level
		if (int fhl = _open(szSavePath, _O_RDONLY) != -1)
		{
			_close(fhl);
			bRevisiting = DTRUE;
			bRestoring = DTRUE;
		}
*/
	}

	if (pFilename && _mbstrlen(pFilename))
	{
		DDWORD dwLoadFlags = 0;

		// Set load flags
		if (!bRestoring)
			dwLoadFlags |= LOADWORLD_LOADWORLDOBJECTS;
//		if (pOldWorld && _mbsicmp((const unsigned char*)pOldWorld, (const unsigned char*)szPath) == 0)	// loading same world as current, don't reload geometry
//		{
			dwLoadFlags |= LOADWORLD_NORELOADGEOMETRY;
//			g_pServerDE->DebugOut("Not loading geometry");
//		}

		dwLoadFlags |= LOADWORLD_NORELOADGEOMETRY;

		res = g_pServerDE->LoadWorld(szPath, dwLoadFlags);
		if (res != DE_OK)
		{
			g_pServerDE->BPrint("Error loading world %s: %d", pFilename, res);
			return DFALSE;
		}
	}
	// Restoring a game - Restore player position too.
	if (bRestoring)
	{
		if (nLoadType == LOADTYPE_RESTOREAUTOSAVE)
			RestoreGame(SAVETYPE_AUTO);
		else
			RestoreGame(SAVETYPE_CURRENT);
	}

	// Restoring saved player data
	if (bRestoringKeepAlive)
	{
		KeepAliveLoad();
	}

	// All types start at a startpoint if not restoring, or if restoring a previous visited level
	g_pServerDE->SetGameConVar(g_szVarGotoStartpoint, (!bRestoring || bRestoringKeepAlive) ? "1" : "0");

	// Set this if we are revisiting a world
	g_pServerDE->SetGameConVar(g_szVarRevisiting, bRevisiting ? "1" : "0");

	// Autosave if not restoring (new level), or if we are revisiting.
	// Set this flag so we know whether to autosave later
	g_pServerDE->SetGameConVar(g_szVarDoAutosave, (!bRestoring || bRevisiting) ? "1" : "0");

	// Now run the world
	res = g_pServerDE->RunWorld();
	if (res != DE_OK)
	{
		g_pServerDE->BPrint("Error running world %s: %d", pFilename, res);
		return DFALSE;
	}
 
	return DTRUE;
}

			
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::SaveGame()
//
//	PURPOSE:	Saves the state of the current world
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::SaveGame(DBYTE bySaveType, DBOOL bSavePlayers, DBOOL bSaveConsole)
{
	DBOOL bRet = DFALSE;

	if (!g_pServerDE) 
		return DFALSE;

	char szSavePath[MAX_PATH];
	
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)g_szSavePath);
	if (bySaveType == SAVETYPE_CURRENT)
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)g_szCurrentSaveFile);
	else if (bySaveType == SAVETYPE_AUTO)
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)g_szAutoSaveFile);
	else 
		return DFALSE;

	// Make a list of savable objects
	ObjectList* pObjectList = g_pServerDE->CreateObjectList();

	// Loop through active objects
	HOBJECT hObj = g_pServerDE->GetNextObject(DNULL);
	while (hObj)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(hObj);
		// Can this object be saved?
		if ((dwFlags & USRFLG_SAVEABLE) && (bSavePlayers || !IsPlayer(hObj)))
		{
			ObjectLink *ol = g_pServerDE->AddObjectToList(pObjectList, hObj);
			ol->m_hObject = hObj;
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}
	// And inactive objects
	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(hObj);
		// Can this object be saved?
		if ((dwFlags & USRFLG_SAVEABLE) && (bSavePlayers || !IsPlayer(hObj)))
		{
			ObjectLink *ol = g_pServerDE->AddObjectToList(pObjectList, hObj);
			ol->m_hObject = hObj;
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}

	DDWORD dwSaveFlags = 0;

	if (bSaveConsole)
		dwSaveFlags |= SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEPORTALS;

	if (g_pServerDE->SaveObjects(szSavePath, pObjectList, 0, dwSaveFlags) == DE_OK)
		bRet = DTRUE;

	g_pServerDE->RelinquishList(pObjectList);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::RestoreGame()
//
//	PURPOSE:	Restores a saved game
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::RestoreGame(DBYTE bySaveType)
{
	if (!g_pServerDE) 
		return DFALSE;

	char szSavePath[MAX_PATH];
	
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)g_szSavePath);
	if (bySaveType == SAVETYPE_CURRENT)
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)g_szCurrentSaveFile);
	else if (bySaveType == SAVETYPE_AUTO)
		_mbscat((unsigned char*)szSavePath, (const unsigned char*)g_szAutoSaveFile);
	else 
		return DFALSE;

	if (g_pServerDE->RestoreObjects(szSavePath, 0, RESTOREOBJECTS_RESTORETIME) != DE_OK)
		return DFALSE;

	return DTRUE;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::BuildSavePath()
//
//	PURPOSE:	Builds a path for a save file given a world file
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::BuildSavePath(char *pBuffer, char *pFilename)
{
	if (!pBuffer && !pFilename)
		return DFALSE;

	// Build the filename path..
	// Remove leading slashes..
	while(*pFilename == '\\' || *pFilename == '/')
		pFilename++;

	_mbscpy((unsigned char*)pBuffer, (const unsigned char*)g_szSavePath);

	// Keep track of the position that the filename will be appended to
	char *pEnd = (char *)(pBuffer + _mbstrlen(pBuffer));

	_mbscat((unsigned char*)pBuffer, (const unsigned char*)pFilename);

	// Replace slashes in the filename with underscores.
	char *pSlash;
	while (pSlash = _mbschr((const unsigned char*)pEnd, (unsigned int)'\\'))
		*pSlash = '_';

	// ..and tack on the extension.
	_mbscat((unsigned char*)pBuffer, (const unsigned char*)".sav");

	return DTRUE;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::KeepAliveSave()
//
//	PURPOSE:	Saves the state of player objects between levels
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::KeepAliveSave()
{
	DBOOL bRet = DFALSE;
	if (!g_pServerDE) return bRet;

	char szSavePath[MAX_PATH];

#ifndef _DEMO
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)g_szSavePath);
	_mbscat((unsigned char*)szSavePath, (const unsigned char*)"Keep.sav");
#else
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)"Keep.sav");
#endif

	// Make a list of savable objects
	ObjectList* pObjectList = g_pServerDE->CreateObjectList();

	HCLASS hPlayerTest = g_pServerDE->GetClass("CPlayerObj");

	HOBJECT hObj = g_pServerDE->GetNextObject(DNULL);

	while (hObj)
	{
		if (IsPlayer(hObj))
		{
			ObjectLink *ol = g_pServerDE->AddObjectToList(pObjectList, hObj);
			ol->m_hObject = hObj;
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}

	if (g_pServerDE->SaveObjects(szSavePath, pObjectList, 1, 0 ) == DE_OK)
		bRet = DTRUE;

	g_pServerDE->RelinquishList(pObjectList);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::KeepAliveLoad()
//
//	PURPOSE:	Restores player data saved when switching levels
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::KeepAliveLoad()
{
	if (!g_pServerDE) return DFALSE;

	char szSavePath[MAX_PATH];

#ifndef _DEMO
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)g_szSavePath);
	_mbscat((unsigned char*)szSavePath, (const unsigned char*)"Keep.sav");
#else
	_mbscpy((unsigned char*)szSavePath, (const unsigned char*)"Keep.sav");
#endif

	if (g_pServerDE->RestoreObjects(szSavePath, 1, 0) != DE_OK)
		return DFALSE;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::SetGameDifficulty()
//
//	PURPOSE:	Sets game info console vars for difficulty level
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::SetGameInfo(DBYTE nGameType, DBYTE nDifficulty)
{
	if (!g_pServerDE) return;

	char szValue[20];

	_itoa(nGameType, szValue, 10);
	g_pServerDE->SetGameConVar(g_szVarGameType, szValue);

	_itoa(nDifficulty, szValue, 10);
	g_pServerDE->SetGameConVar(g_szVarDifficulty, szValue);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::CacheFiles()
//
//	PURPOSE:	Cache files that are used often
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::CacheFiles()
{
	// Cache models...
	for (int i=0; i < NUM_CACHED_MODELS; i++)
	{
		g_pServerDE->CacheFile(FT_MODEL, g_pCachedModels[i]);
	}

	// Cache textures...

	for (i=0; i < NUM_CACHED_TEXTURES; i++)
	{
		g_pServerDE->CacheFile(FT_TEXTURE, g_pCachedTextures[i]);
	}

	// Cache sprites...

	for (i=0; i < NUM_CACHED_SPRITES; i++)
	{
		g_pServerDE->CacheFile(FT_SPRITE, g_pCachedSprite[i]);
	}

	// Cache sounds...

	for (i=0; i < NUM_CACHED_SOUNDS_LOCAL; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSoundLocal[i]);
	}

	for (i=0; i < NUM_CACHED_SOUNDS_AMBIENT; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSoundAmbient[i]);
	}

	for (i=0; i < NUM_CACHED_SOUNDS_3D; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSound3D[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::AddClientToList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::AddClientToList(HCLIENT hClient)
{
	// Sanity checks...

	if (!hClient) return(DFALSE);


	// Make sure this client isn't already in our list...

	if (IsClientInList(hClient))
	{
		return(DTRUE);
	}


	// Add this client handle to our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == DNULL)
		{
			m_aClients[i] = hClient;
			return(DTRUE);
		}
	}


	// If we get here, there wasn't any space left in the array...

	return(DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::RemoveClientFromList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::RemoveClientFromList(HCLIENT hClient)
{
	// Sanity checks...

	if (!hClient) return(DFALSE);


	// Remove this client handle from our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
			m_aClients[i] = DNULL;
			return(DTRUE);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

	return(DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::IsClientInList
//
//	PURPOSE:	Determines if the given client handle is in our list
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::IsClientInList(HCLIENT hClient)
{
	// Sanity checks...

	if (!hClient) return(DFALSE);


	// Look for this client handle in our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
			return(DTRUE);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

	return(DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::GetPlayerFromClientList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

CPlayerObj*	CBloodServerShell::GetPlayerFromClientList(HCLIENT hClient)
{
	// Sanity checks...

	if (!hClient) return(DNULL);
	if (!g_pServerDE) return(DNULL);


	// Remove this client handle from our array...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i] == hClient)
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->GetClientUserData(hClient);
			return(pPlayer);
		}
	}


	// If we get here, we didn't find the given client handle in the array...

	return(DNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::SetupGameInfo
//
//	PURPOSE:	Setup game info
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::SetupGameInfo()
{
	if (g_pServerDE)
	{
		NetGame* pGameInfo;
		DDWORD dwLen = sizeof(NetGame);
		g_pServerDE->GetGameInfo((void**)&pGameInfo, &dwLen);

		if (pGameInfo)
		{
			memcpy(&m_GameInfo, pGameInfo, sizeof(NetGame));
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::Update
//
//	PURPOSE:	Update servier stuff periodically
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::Update(DFLOAT timeElapsed)
{
	// Sanity checks...

	if (!g_pServerDE || GetGameType() == GAMETYPE_SINGLE) return;


	// Update the client ping times...

	UpdateClientPingTimes();


	// Check for a say message...

	if (!g_SayTrack.IsInitted())
	{
		g_SayTrack.Init(g_pServerDE, "Say", "", 0.0f);
	}
	else
	{
		char *sSay = g_SayTrack.GetStr("");

		if (sSay && sSay[0] != 0)
		{
			char sMsg[512];
			sprintf(sMsg, "HOST: %s", sSay);
			HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, SMSG_CONSOLEMESSAGE_ALL);
			g_pServerDE->WriteToMessageString(hMessage, sMsg);
			g_pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLE);
			
			g_SayTrack.SetStr("");
		}
	}


	// Setup a static timer for session name updates...

	static DFLOAT timerUpdateName = 5;


	// Update our time and see if it's time to update the session name...

	if (timeElapsed < timerUpdateName)
	{
		timerUpdateName -= timeElapsed;
	}
	else
	{
		timerUpdateName = 10;
		UpdateSessionName();
	}


	// Update shogo server info...

	UpdateBlood2Server();


	// Update multiplayer stuff...

	UpdateMultiplayer();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::UpdateSessionName
//
//	PURPOSE:	Updates the name of the session with current game info
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::UpdateSessionName()
{
	// Get the current session name...

	static char sSession[4096];

	DRESULT dr = g_pServerDE->GetSessionName(sSession, 4096);
	if (dr != LT_OK) return(DFALSE);


	// Extract the info we want to keep...

	char sName[NML_NAME];
	if (!Sparam_Get(sName, sSession, NST_GAMENAME)) return(DFALSE);
 
	char sHost[NML_HOST];
	if (!Sparam_Get(sHost, sSession, NST_GAMEHOST)) return(DFALSE);
 
	char sType[32];
	if (!Sparam_Get(sType, sSession, NST_GAMETYPE)) return(DFALSE);

	
	// Get the base level name...

	char sLevel[128];
	_mbscpy((unsigned char*)sLevel, (const unsigned char*)m_GameInfo.m_sLevels[m_nCurLevel]);


	// Clear the session string now that we have the info we want from it...

	sSession[0] = '\0';


	// Add the info we kept...

	Sparam_Add(sSession, NST_GAMENAME, sName);
	Sparam_Add(sSession, NST_GAMEHOST, sHost);
	Sparam_Add(sSession, NST_GAMELEVEL, sLevel);
	Sparam_Add(sSession, NST_GAMETYPE, sType);


	// Add info for each player...

	int count = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);

		if (pPlayer)
		{
			HSTRING hstrName = pPlayer->GetPlayerName();
			char* pName = hstrName ? g_pServerDE->GetStringData(hstrName) : "";

			count++;
			char sBase[32];

			sprintf(sBase, "%s%i", NST_PLRNAME_BASE, count);
			Sparam_Add(sSession, sBase, pName);

			sprintf(sBase, "%s%i", NST_PLRFRAG_BASE, count);
			Sparam_Add(sSession, sBase, pPlayer->GetFrags());
		}
	}

	Sparam_Add(sSession, NST_PLRCOUNT, count);


	// Update the session name...

	g_pServerDE->UpdateSessionName(sSession);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::UpdateBlood2Server
//
//	PURPOSE:	Updates a stand-alone server with game info if necessary
//
// ----------------------------------------------------------------------- //

DBOOL CBloodServerShell::UpdateBlood2Server()
{
	// Check if we need to update...

	if (!m_bUpdateBlood2Serv)
	{
		return(DFALSE);
	}

	m_bUpdateBlood2Serv = FALSE;


	// Make sure we are actually being hosted via ShogoServ...

	if (!m_bBlood2ServHosted)
	{
		return(DFALSE);
	}


	// Get the current base level name...

	char sCurLevel[128];
	_mbscpy((unsigned char*)sCurLevel, (const unsigned char*)m_GameInfo.m_sLevels[m_nCurLevel]);


	// Get the next base level name...

	char sNextLevel[128];
	int  i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	_mbscpy((unsigned char*)sNextLevel, (const unsigned char*)m_GameInfo.m_sLevels[i]);


	// Declare the string...

	static	char sInfo[4096];
	sInfo[0] = '\0';


	// Flag that this is a standard update message...

	Sparam_Add(sInfo, "GMSG", NGM_STANDARDUPDATE);


	// Add the levels...

	Sparam_Add(sInfo, NST_CURLEVEL, sCurLevel);
	Sparam_Add(sInfo, NST_NEXTLEVEL, sNextLevel);


	// Add info for each player...

	int count = 0;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
		{
			HSTRING hstrName = pPlayer->GetPlayerName();
			char* pName = hstrName ? g_pServerDE->GetStringData(hstrName) : "";

			count++;
			char sBase[32];

			sprintf(sBase, "%s%i", NST_PLRNAME_BASE, count);
			Sparam_Add(sInfo, sBase, pName);

			sprintf(sBase, "%s%i", NST_PLRFRAG_BASE, count);
			Sparam_Add(sInfo, sBase, pPlayer->GetFrags());

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				sprintf(sBase, "%s%i", NST_PLRID_BASE, count);
				Sparam_Add(sInfo, sBase, g_pServerDE->GetClientID(hClient));
			}
		}
	}

	Sparam_Add(sInfo, NST_PLRCOUNT, count);


	// Pass this info to the Shogo Server...

	g_pServerDE->SendToServerApp(sInfo);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::UpdateMultiplayer
//
//	PURPOSE:	Determine if it is time to change levels
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::UpdateMultiplayer()
{
	if (!g_pServerDE || GetGameType() == GAMETYPE_SINGLE) return;
	
	DBOOL bStartLevel = DFALSE;

	if (m_GameInfo.m_byEnd == NGE_TIME ||
		m_GameInfo.m_byEnd == NGE_FRAGSANDTIME)
	{
		DFLOAT fEndLevelTime = (m_GameInfo.m_dwEndTime * 60.0f);
		DFLOAT fTime = g_pServerDE->GetTime();

		if (fTime >= fEndLevelTime)
		{
			bStartLevel = DTRUE;
		}
	}

	if ( !bStartLevel && 
		 (m_GameInfo.m_byEnd == NGE_FRAGS || 
		  m_GameInfo.m_byEnd == NGE_FRAGSANDTIME) )
	{
		if (IsMultiplayerTeamBasedGame())
		{
			CTeam* pTeam = m_TeamMgr.GetFirstTeam();

			while (pTeam)
			{
				if (pTeam->GetFrags() >= (int)m_GameInfo.m_dwEndFrags)
				{
					bStartLevel = DTRUE;
					break;
				}

				pTeam = m_TeamMgr.GetNextTeam(pTeam);
			}
		}
		else
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
				if (pPlayer)
				{
					if (pPlayer->GetFrags() >= (int)m_GameInfo.m_dwEndFrags)
					{
						bStartLevel = DTRUE;
						break;
					}
				}
			}
		}
	}

	if (g_bWaitToStartNextLevel)
	{
		if(g_pServerDE->GetTime() - g_fWaitTimeForNextLevel > WAIT_TIME_FOR_LEVEL_SWITCH)
		{
			g_bWaitToStartNextLevel = DFALSE;
			StartNextMultiplayerLevelAck();
		}
		return;
	}

	if (g_bNextLevel)
	{
		bStartLevel = DTRUE;
	}

	if (bStartLevel)
	{
		StartNextMultiplayerLevel();
		g_bNextLevel = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::StartNextMultiplayerLevel
//
//	PURPOSE:	Start the next multiplayer level
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::StartNextMultiplayerLevel()
{
	// Tell the server that we're changing the level...
/*
	char sInfo[1024];
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGING);
	g_pServerDE->SendToServerApp(sInfo);
*/

	// Tell all clients we're changing levels...

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(DNULL, SMSG_MP_CHANGING_LEVELS);
	g_pServerDE->EndMessage(hWrite);

	g_bWaitToStartNextLevel = DTRUE;
	g_fWaitTimeForNextLevel = g_pServerDE->GetTime();


	// Clear player frags...
/*
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
		{
			pPlayer->SetFrags(0);
		}
	}


	// Stop voice mgr stuff...

	m_VoiceMgr.StopAll();


	// Create the transition team id list so we can properly restore the teams...

	m_TeamMgr.CreateTeamTransIDs();


	// Load the next level...
	
	if (++m_nCurLevel >= m_GameInfo.m_byNumLevels)
	{
		m_nCurLevel = 0;
	}

	char* pLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	if (pLevelName)
	{
		g_pServerDE->LoadWorld(pLevelName, LOADWORLD_LOADWORLDOBJECTS | LOADWORLD_RUNWORLD);
	}
	else
	{
		g_pServerDE->BPrint("ERROR CAN'T START NEXT MULTIPLAYER LEVEL!");
	}


	// Tell the shogo server that we changed the level...

	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGED);

	char sCurLevel[128];
	_mbscpy((unsigned char*)sCurLevel, (const unsigned char*)m_GameInfo.m_sLevels[m_nCurLevel]);

	char sNextLevel[128];
	i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	_mbscpy((unsigned char*)sNextLevel, (const unsigned char*)m_GameInfo.m_sLevels[i]);

	Sparam_Add(sInfo, NST_CURLEVEL, sCurLevel);
	Sparam_Add(sInfo, NST_NEXTLEVEL, sNextLevel);

	g_pServerDE->SendToServerApp(sInfo);*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::StartNextMultiplayerLevelAck
//
//	PURPOSE:	Start the next multiplayer level
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::StartNextMultiplayerLevelAck()
{
	// Tell the shogo server that we're changing the level...

	char sInfo[1024];
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGING);
	g_pServerDE->SendToServerApp(sInfo);


	// Clear player frags...
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
			pPlayer->SetFrags(0);
	}


	// Stop voice mgr stuff...
	m_VoiceMgr.StopAll();


	// Create the transition team id list so we can properly restore the teams...
	m_TeamMgr.CreateTeamTransIDs();


	// Load the next level...
	if (++m_nCurLevel >= m_GameInfo.m_byNumLevels)
		m_nCurLevel = 0;


	char* pLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	if (pLevelName)
		g_pServerDE->LoadWorld(pLevelName, LOADWORLD_LOADWORLDOBJECTS | LOADWORLD_RUNWORLD);
	else
		g_pServerDE->BPrint("ERROR CAN'T START NEXT MULTIPLAYER LEVEL!");


	// Tell the shogo server that we changed the level...
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGED);

	char sCurLevel[128];
	_mbscpy((unsigned char*)sCurLevel, (const unsigned char*)m_GameInfo.m_sLevels[m_nCurLevel]);

	char sNextLevel[128];
	i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	_mbscpy((unsigned char*)sNextLevel, (const unsigned char*)m_GameInfo.m_sLevels[i]);

	Sparam_Add(sInfo, NST_CURLEVEL, sCurLevel);
	Sparam_Add(sInfo, NST_NEXTLEVEL, sNextLevel);

	g_pServerDE->SendToServerApp(sInfo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::ServerAppMessageFn
//
//	PURPOSE:	Server app message function
//
// ----------------------------------------------------------------------- //

DRESULT CBloodServerShell::ServerAppMessageFn(char* sMsg)
{
	// Sanity checks...

	if (!sMsg) return(LT_OK);


	// Check for "GAMEINIT" message...

	if (_mbscmp((const unsigned char*)sMsg, (const unsigned char*)"GAMEINIT") == 0)
	{
		SetupGameInfo();
	}
	else if (_mbscmp((const unsigned char*)sMsg, (const unsigned char*)"NEXTLEVEL") == 0)
	{
		StartNextMultiplayerLevel();
	}
	else if (_mbscmp((const unsigned char*)sMsg, (const unsigned char*)"SERVHOST") == 0)
	{
		m_bBlood2ServHosted = DTRUE;
	}


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::SendBlood2ServConsoleMessage
//
//	PURPOSE:	Sends a string to be displayed in the ShogoServ console
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::SendBlood2ServConsoleMessage(char* sMsg)
{
	if (!sMsg || !m_bBlood2ServHosted) return;

	char sInfo[1024];
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_CONSOLEMSG);
	Sparam_Add(sInfo, NST_CONSOLEMESSAGE, sMsg);

	g_pServerDE->SendToServerApp(sInfo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::FindClient
//
//	PURPOSE:	Goes through the client list and checks for a player
//				represented by hObject. Returns the HCLIENT if it's still an 
//				active player.
//
// ----------------------------------------------------------------------- //

HCLIENT CBloodServerShell::FindClient(HOBJECT hObject)
{
	if (!g_pServerDE) return DFALSE;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_aClients[i])
		{
			CPlayerObj *pObj = (CPlayerObj*)g_pServerDE->GetClientUserData(m_aClients[i]);
			if (pObj && pObj->m_hObject == hObject)
				return m_aClients[i];
		}
	}
	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::UpdateClientPingTimes
//
//	PURPOSE:	Updates each client with all client ping times
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::UpdateClientPingTimes()
{
	HMESSAGEWRITE hWrite;
	float ping;
	DDWORD clientID;
	HCLIENT hClient;
	
	ServerDE *pServerDE = g_pServerDE;
	if(!pServerDE) return;

	static DFLOAT fPingUpdateCounter = 0.0f;

	fPingUpdateCounter += pServerDE->GetFrameTime();
	if(fPingUpdateCounter > CLIENT_PING_UPDATE_RATE)
	{
		hWrite = pServerDE->StartMessage(DNULL, SMSG_PINGTIMES);
		
			hClient = DNULL;
			while(hClient = pServerDE->GetNextClient(hClient))
			{
				clientID = pServerDE->GetClientID(hClient);
				pServerDE->GetClientPing(hClient, ping);

				pServerDE->WriteToMessageWord(hWrite, (D_WORD)clientID);
				pServerDE->WriteToMessageWord(hWrite, (D_WORD)(ping * 1000.0f));
			}

		pServerDE->WriteToMessageWord(hWrite, 0xFFFF);
		pServerDE->EndMessage2(hWrite, MESSAGE_NAGGLE);

		fPingUpdateCounter = 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodServerShell::DoLevelChangeCharacterCheck
//
//	PURPOSE:	Updates each client with all client ping times
//
// ----------------------------------------------------------------------- //

void CBloodServerShell::DoLevelChangeCharacterCheck(char* sLevel)
{
	g_bLevelChangeCharacter = DFALSE;

#ifndef _ADDON
	return;
#endif

	char sUpr[256];
	strncpy(sUpr, sLevel, 255);
	strupr(sUpr);

	if (strstr(sUpr, "_AO"))			// is this an add-on level?
	{
		if (strstr(sUpr, "_CC_C"))		// switch to caleb?
		{
			g_bLevelChangeCharacter = DTRUE;
			g_nLevelChangeCharacter = CHARACTER_CALEB;
		}
		else if (strstr(sUpr, "_CC_I"))	// switch to ishmael?
		{
			g_bLevelChangeCharacter = DTRUE;
			g_nLevelChangeCharacter = CHARACTER_ISHMAEL;
		}
		else if (strstr(sUpr, "_CC_O"))	// switch to ophelia?
		{
			g_bLevelChangeCharacter = DTRUE;
			g_nLevelChangeCharacter = CHARACTER_OPHELIA;
		}
		else if (strstr(sUpr, "_CC_G"))	// switch to gabby?
		{
			g_bLevelChangeCharacter = DTRUE;
			g_nLevelChangeCharacter = CHARACTER_GABREILLA;
		}

		// check for the last level...

		if (strstr(sUpr, "ENDBOSS_CC_C"))	// check for final caleb nightmare level
		{
			g_bLevelChangeCharacter = DFALSE;
		}
	}
}

