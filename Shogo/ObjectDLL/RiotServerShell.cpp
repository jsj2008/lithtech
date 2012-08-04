// ----------------------------------------------------------------------- //
//
// MODULE  : RiotServerShell.cpp
//
// PURPOSE : Riot's server shell - Implementation
//
// CREATED : 9/17/97
//
// ----------------------------------------------------------------------- //

#include "RiotServerShell.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "RiotMsgIDs.h"
#include "RiotCommandIDs.h"
#include "RiotObjectUtilities.h"
#include "DialogTrigger.h"
#include "CachedFiles.h"
#include "Trigger.h"
#include "Sparam.h"
#include "SSFXReg.h"
#include <stdio.h>

#define MAX_CLIENT_NAME_LENGTH		100
#define CLIENT_PING_UPDATE_RATE		3.0f
#define UPDATENAME_INTERVAL			10.0f

// Stuff to create a RiotServerShell.

SETUP_SERVERSHELL()


CRiotServerShell* g_pRiotServerShellDE = DNULL;



// The default server shell maker.
SShellMaker *g_pSShellMakerHead=DNULL;


ServerShellDE* CreateShogoServerShell(ServerDE *pServerDE)
{
	g_pServerDE = pServerDE;

	// Make sure we are using autodeactivation...

	pServerDE->RunGameConString("autodeactivate 1.0");

	CRiotServerShell *pShell = new CRiotServerShell;

	g_pRiotServerShellDE = pShell;

	return (ServerShellDE*)pShell;
}

void DeleteShogoServerShell(ServerShellDE *pInputShell)
{
	CRiotServerShell *pShell = (CRiotServerShell*)pInputShell;

	// g_pRiotServerShellDE = DNULL; 
	// (kls - 2/22/98 - CreateSeverShell() is called BEFORE
	// DeleteServerShell() is called so we CAN'T set this to NULL)

	delete pShell;
}


SShellMaker g_RiotSShellMaker(0, CreateShogoServerShell, DeleteShogoServerShell);


SShellMaker* GetSShellMaker()
{
	SShellMaker *pCur, *pBest;
	DDWORD bestPriority;

	pBest = DNULL;
	bestPriority = 0;	
	for(pCur=g_pSShellMakerHead; pCur; pCur=pCur->m_pNext)
	{
		if(pCur->m_Priority >= bestPriority)
		{
			pBest = pCur;
			bestPriority = pCur->m_Priority;
		}
	}

	return pBest;
}


ServerShellDE* CreateServerShell(ServerDE *pServerDE)
{
	SShellMaker *pBest;

	pBest = GetSShellMaker();	
	if(!pBest)
		return DNULL;

	return pBest->m_CreateFn(pServerDE);
}

void DeleteServerShell(ServerShellDE *pInputShell)
{
	SShellMaker *pBest;

	pBest = GetSShellMaker();	
	if(!pBest)
		return;

	pBest->m_DeleteFn(pInputShell);
}

DBOOL g_bInfiniteAmmo = DFALSE;
DBOOL g_bRobert = DFALSE;

CVarTrack g_RammingDamageTrack;




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
//	ROUTINE:	CRiotServerShell::CRiotServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CRiotServerShell::CRiotServerShell()
{
	memset(&m_GameInfo, 0, sizeof(NetGame));

	m_hstrStartPointName = g_pServerDE->CreateString("DEFAULT");
	m_nCurLevel			 = 0;
	
	ClearClientList();
	SetUpdateShogoServ();

	m_bShogoServHosted = DFALSE;
	m_WorldTimeColor[0] = m_WorldTimeColor[1] = m_WorldTimeColor[2] = MAX_WORLDTIME_COLOR;

	SetupGameInfo();

	m_nLastLGFlags  = LOAD_NEW_GAME;
	m_hSwitchWorldVar = DNULL;
	m_bFirstUpdate = DTRUE;
	
	m_TODSeconds = TODHoursToSeconds(12.0f);
	m_TODCounter = 0.0f;
	m_ClientPingSendCounter = 0.0f;
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::~CRiotServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CRiotServerShell::~CRiotServerShell()
{
	if (!g_pServerDE) return;

	if (m_hstrStartPointName)
	{
		g_pServerDE->FreeString(m_hstrStartPointName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnAddClient()
//
//	PURPOSE:	Called when a client connects to a server
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::OnAddClient(HCLIENT hClient)
{
	SSFXReg *pCur;
	HMESSAGEWRITE hWrite;
	ServerDE *pServerDE = g_pServerDE;

	// Send them the SFX registry.
	for(pCur=g_SSFXRegHead; pCur; pCur=pCur->m_pNext)
	{
		if(hWrite = pServerDE->StartMessage(hClient, MID_SFX_REG))
		{
			pServerDE->WriteToMessageString(hWrite, pCur->m_SFXName);
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)pCur->m_ID);
			pServerDE->EndMessage(hWrite);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnRemoveClient()
//
//	PURPOSE:	Called when a client disconnects from a server
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::OnRemoveClient(HCLIENT hClient)
{
	// MD 2/1/99: Clear its m_hClient so it doesn't pass bad pointers to engine.
	CPlayerObj *pPlayer;
	ServerDE *pServerDE = GetServerDE();

	if(pPlayer = (CPlayerObj*)pServerDE->GetClientUserData(hClient))
	{
		pPlayer->SetClient(DNULL);
	}

// Already does this in OnClientExitWorld, which gets called when a client exits.
/*
	if (GetGameType() != SINGLE)
	{
		// Send a message to all clients, letting them know this user is leaving the world

		DDWORD nClientID = g_pServerDE->GetClientID (hClient);
		HMESSAGEWRITE hWrite = g_pServerDE->StartMessage (DNULL, MID_PLAYER_REMOVED);
		g_pServerDE->WriteToMessageFloat (hWrite, (DFLOAT) nClientID);
		g_pServerDE->EndMessage (hWrite);
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

BaseClass* CRiotServerShell::OnClientEnterWorld(HCLIENT hClient, void *pClientData, DDWORD clientDataLen)
{
	if (!g_pServerDE || !hClient) return DNULL;

	BaseClass* pClass  = DNULL;
	DBOOL bFoundClient = DFALSE;

	char sClientName[MAX_CLIENT_NAME_LENGTH];
	char sClientRefName[MAX_CLIENT_NAME_LENGTH];
	sClientName[0] = sClientRefName[0] = '\0';


	// Send them the current time of day color.
	ServerDE *pServerDE = g_pServerDE;

	HMESSAGEWRITE hWrite;
	hWrite = pServerDE->StartMessage(hClient, MID_TIMEOFDAYCOLOR);
	pServerDE->WriteToMessageByte(hWrite, m_WorldTimeColor[0]);
	pServerDE->WriteToMessageByte(hWrite, m_WorldTimeColor[1]);
	pServerDE->WriteToMessageByte(hWrite, m_WorldTimeColor[2]);
	pServerDE->EndMessage(hWrite);


	g_pServerDE->GetClientName(hClient, sClientName, MAX_CLIENT_NAME_LENGTH-1);


	// Search through the client refs to see if any of them match our
	// client...

	HCLIENTREF hClientRef = g_pServerDE->GetNextClientRef(DNULL);
	while (hClientRef)
	{
		// See if this client reference is local or not...

		if (g_pServerDE->GetClientRefInfoFlags(hClientRef) & CIF_LOCAL)
		{
			bFoundClient = DTRUE;
		}


		// Determine if there is a reference to a client with the same
		// name...
	
		if (!bFoundClient && g_pServerDE->GetClientRefName(hClientRef, sClientRefName, MAX_CLIENT_NAME_LENGTH-1))
		{
			if (sClientName[0] && sClientRefName[0])
			{
				if (_stricmp(sClientName, sClientRefName) == 0)
				{
					bFoundClient = DTRUE;
				}
			}
		}


		// See if we found the right client...

		if (bFoundClient)
		{
			HOBJECT	hObject = g_pServerDE->GetClientRefObject(hClientRef);
			pClass = g_pServerDE->HandleToObject(hObject);

			if (pClass)
			{
				RespawnPlayer(pClass, hClient);
			}
			break;
		}

		hClientRef = g_pServerDE->GetNextClientRef(hClientRef);
	}


	// See if we need to create a player (no matches found)...

	if (!pClass)
	{
		pClass = CreatePlayer(hClient);
	}


	g_pServerDE->SetClientInfoFlags(hClient, 0);



	// Add this client to our local list...

	AddClientToList(hClient);
	SetUpdateShogoServ();


	// All done...

	return pClass;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnClientExitWorld()
//
//	PURPOSE:	remove a client
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::OnClientExitWorld(HCLIENT hClient)
{
	if (GetGameType() != SINGLE)
	{
		// Send a message to all clients, letting them know this user is leaving the world

		DDWORD nClientID = g_pServerDE->GetClientID (hClient);
		HMESSAGEWRITE hWrite = g_pServerDE->StartMessage (DNULL, MID_PLAYER_REMOVED);
		g_pServerDE->WriteToMessageFloat (hWrite, (DFLOAT) nClientID);
		g_pServerDE->EndMessage (hWrite);
	}


	// Remove this client from our local list...

	RemoveClientFromList(hClient);
	SetUpdateShogoServ();


	// Remove the player object...

	CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->GetClientUserData(hClient);
	if (pPlayer) 
	{
		// pPlayer->SetClient(DNULL);
		g_pServerDE->RemoveObject(pPlayer->m_hObject);
	}

	g_pServerDE->SetClientUserData(hClient, DNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::PreStartWorld()
//
//	PURPOSE:	Handle pre start world
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::PreStartWorld(DBOOL bSwitchingWorlds)
{		
	m_charMgr.PreStartWorld();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::PostStartWorld()
//
//	PURPOSE:	Handle post switch world
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::PostStartWorld()
{
	// g_pServerDE->BPrint("PostSwitchWorld() called");
	
	m_charMgr.PostStartWorld(m_nLastLGFlags);
}


void CRiotServerShell::SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer)
{
	HCLIENT hClient		= pPlayer->GetClient();
	DDWORD  nClientID   = g_pServerDE->GetClientID(hClient);
	HSTRING hClientName = g_pServerDE->CreateString(pPlayer->GetNetName());
	float r, g, b, a;

	g_pServerDE->GetObjectColor(pPlayer->m_hObject, &r, &g, &b, &a);

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(hClients, MID_PLAYER_ADDED);
	g_pServerDE->WriteToMessageHString(hWrite, hClientName);
	g_pServerDE->WriteToMessageFloat(hWrite, (DFLOAT) nClientID);
	g_pServerDE->WriteToMessageFloat(hWrite, (float)pPlayer->GetFragCount());
	g_pServerDE->WriteToMessageByte(hWrite, (DBYTE)(r * 255.0f));
	g_pServerDE->WriteToMessageByte(hWrite, (DBYTE)(g * 255.0f));
	g_pServerDE->WriteToMessageByte(hWrite, (DBYTE)(b * 255.0f));
	g_pServerDE->EndMessage2(hWrite, MESSAGE_NAGGLE);

	g_pServerDE->FreeString(hClientName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::OnMessage(HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	switch (messageID)
	{
		case MID_WEAPON_FIRE:
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponFireMessage(hMessage);
			}
		}
		break;

		case MID_WEAPON_SOUND:
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponSoundMessage(hMessage);
			}
		}
		break;

		case MID_WEAPON_STATE:
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleWeaponStateMessage(hMessage);
			}
		}
		break;

		case MID_WEAPON_CHANGE:
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				DBYTE nWeaponId = pServerDE->ReadFromMessageByte(hMessage);
				pPlayer->DoWeaponChange(nWeaponId);
			}
		}
		break;

		case MID_FRAG_SELF:
		{
			CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->GetClientUserData(hSender);
			if(pPlayer)
			{		
				LPBASECLASS pClass = pServerDE->HandleToObject(pPlayer->m_hObject);
				if (pClass)
				{
					DVector vDir(0,1,0);
					HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pClass, pPlayer->m_hObject, MID_DAMAGE);
					pServerDE->WriteToMessageVector(hMessage, &vDir);
					pServerDE->WriteToMessageFloat(hMessage, 100000.0f);
					pServerDE->WriteToMessageByte(hMessage, DT_KATO);
					pServerDE->WriteToMessageObject(hMessage, pPlayer->m_hObject);
					pServerDE->EndMessage(hMessage);
				}
			}
		}
		break;

		case MID_PLAYER_RESPAWN :
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer && pPlayer->GetState() == PS_DEAD)
			{
				pPlayer->Respawn (m_hstrStartPointName);
			}
		}
		break;

		case MID_PLAYER_MULTIPLAYER_INIT :
		{
			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				if (pPlayer->MultiplayerInit(hMessage))
				{
					// Send a message to all clients, letting them know a 
					// new client has joined the game...
					SendPlayerInfoMsgToClients(DNULL, pPlayer);

					// Tell the new Client about all the clients already 
					// on the server...

					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						CPlayerObj* pCurPlayer = GetPlayerFromClientList(m_aClients[i]);
						if (pCurPlayer && pCurPlayer != pPlayer)
						{
							SendPlayerInfoMsgToClients(pPlayer->GetClient(), pCurPlayer);
						}
					}

					pPlayer->Respawn(m_hstrStartPointName);
				}
			}
		}
		break;

		case MID_PLAYER_INITVARS :
		{
			DBOOL bRunLock = (DBOOL)pServerDE->ReadFromMessageByte(hMessage);

			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->SetRunLock (bRunLock);
			}
		}
		break;

		case MID_PLAYER_UPDATE :
			HandleUpdatePlayerMsg(hSender, hMessage);
		break;

		case MID_PLAYER_MESSAGE :
		{
			// retrieve the string they sent

			char* pString = pServerDE->ReadFromMessageString (hMessage);

			// So it shows up in ShogoSrv..
			if(pString && !(pServerDE->GetClientInfoFlags(hSender) & CIF_LOCAL))
			{
				pServerDE->CPrint(pString);
			}

			// now send the string to all clients

			HMESSAGEWRITE hMessage = pServerDE->StartMessage (NULL, MID_PLAYER_MESSAGE);
			pServerDE->WriteToMessageString (hMessage, pString);
			pServerDE->EndMessage2 (hMessage, MESSAGE_NAGGLE);
		}
		break;

		case MID_PLAYER_CHEAT :
		{

#ifdef _DEMO
			return;  // No cheats in demo mode
#endif

			if (GetGameType() != SINGLE) return;  // No cheats in multiplayer

			// get a pointer to the sender's player object

			void *pData = pServerDE->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (!pPlayer) return;
			
			// retrieve message data

			CheatCode nCheatCode = (CheatCode) pServerDE->ReadFromMessageByte (hMessage);
			DBYTE nData = pServerDE->ReadFromMessageByte (hMessage);

			// now deal with the specific cheat code

			HandleCheatCode(pPlayer, nCheatCode, nData);
		}
		break;

		case MID_DIALOG_CLOSE :
		{
			// retrieve message data

			DFLOAT fSelection = pServerDE->ReadFromMessageFloat (hMessage);
			DDWORD nByte1 = (DDWORD) pServerDE->ReadFromMessageByte (hMessage);
			DDWORD nByte2 = (DDWORD) pServerDE->ReadFromMessageByte (hMessage);
			DDWORD nByte3 = (DDWORD) pServerDE->ReadFromMessageByte (hMessage);
			DDWORD nByte4 = (DDWORD) pServerDE->ReadFromMessageByte (hMessage);
			
			DDWORD nDlgObject = (nByte1) | (nByte2 << 8) | (nByte3 << 16) | (nByte4 << 24);
			
			HOBJECT hDlgObj = (HOBJECT) nDlgObject;
			if (!hDlgObj) break;

			DialogTrigger* pDlg = (DialogTrigger*) pServerDE->HandleToObject (hDlgObj);
			if (!pDlg) break;

			pDlg->Trigger ((int) fSelection);
		}
		break;

		case MID_GAME_PAUSE:
		{
			PauseGame(DTRUE);
		}
		break;

		case MID_GAME_UNPAUSE:
		{
			PauseGame(DFALSE);
		}
		break;

		case MID_LOAD_GAME :
			HandleLoadGameMsg(hSender, hMessage);
		break;

		case MID_SAVE_GAME :
			HandleSaveGameMsg(hSender, hMessage);
		break;

		case MID_SINGLEPLAYER_START :
		{
			if (GetGameType() == SINGLE)
			{
				void *pData = pServerDE->GetClientUserData(hSender);
				CPlayerObj* pPlayer = (CPlayerObj*)pData;
				if (pPlayer)
				{
					pPlayer->StartLevel();
				}
			}
		}
		break;

		case MID_TRANSMISSIONENDED:
		{
			CPlayerObj* pPlayer = GetFirstPlayer( );
			if( pPlayer )
			{
				pPlayer->SetDialogActive( DFALSE );
			}
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleUpdatePlayerMsg()
//
//	PURPOSE:	Handle updating the player info
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleUpdatePlayerMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Update the player...

	void *pData = pServerDE->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;

	if (pPlayer)
	{
		if(pPlayer->ClientUpdate(hMessage))
		{
			pPlayer->HandlePlayerPositionMessage(hMessage); // Merged player position and update messages.
		}
	}
}
			

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleCheatCode()
//
//	PURPOSE:	Handle the various cheat codes
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleCheatCode(CPlayerObj* pPlayer, CheatCode nCheatCode, DBYTE nData)
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
			pPlayer->Respawn(m_hstrStartPointName);
		}
		break;

		case CHEAT_ONFOOT :
		{
			pPlayer->ToggleOnFootMode();
		}
		break;

		case CHEAT_VEHICLE :
		{
			pPlayer->ToggleVehicleMode();
		}
		break;

		case CHEAT_MECH :
		{
			pPlayer->IncMechaMode();
		}
		break;

		case CHEAT_FULL_WEAPONS :
		{
			pPlayer->FullWeaponCheat();
		}
		break;

		case CHEAT_POSWEAPON:
		case CHEAT_POSWEAPON_MUZZLE:
		case CHEAT_PLAYERMOVEMENT: 
		case CHEAT_PLAYERACCEL:
		case CHEAT_CAMERAOFFSET:
		{
			pPlayer->ToggleDebugCheat(nCheatCode);
		}
		break;

		case CHEAT_BIGGUNS:
		{
			HandleCheatBigGuns(nData);
		}
		break;

		case CHEAT_TRIGGERBOX:
		{
			HandleTriggerBoxCheat(nData);
		}
		break;

		case CHEAT_TEARS:
		{
			g_bInfiniteAmmo = !g_bInfiniteAmmo;

			if (g_bInfiniteAmmo)
			{
				pPlayer->FullWeaponCheat();
				pPlayer->FullAmmoCheat();
			}
		}
		break;

		case CHEAT_ROBERT:
		{
			g_bRobert = !g_bRobert;
		}
		break;

		case CHEAT_REMOVEAI:
		{
			HandleCheatRemoveAI(nData);
		}
		break;

		default:
			break;
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::OnCommandOn()
//
//	PURPOSE:	Handle commands
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::OnCommandOn(HCLIENT hClient, int command)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hClient) return;

	void *pData = pServerDE->GetClientUserData(hClient);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (!pPlayer) return;

	switch (command)
	{
		case COMMAND_ID_RUNLOCK		 : pPlayer->ToggleRunLock(); break;
		case COMMAND_ID_VEHICLETOGGLE: pPlayer->ToggleVehicleMode(); break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::SetStartPointName()
//
//	PURPOSE:	Set the name of the next start point.
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::SetStartPointName(HSTRING hString)
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
//	ROUTINE:	CRiotServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //

BaseClass* CRiotServerShell::CreatePlayer(HCLIENT hClient)
{
	if (!g_pServerDE) return DNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	ROT_INIT(theStruct.m_Rotation);
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

	HCLASS hClass = g_pServerDE->GetClass("CPlayerObj");

	BaseClass* pClass = NULL;
	if (hClass)
	{
		pClass = g_pServerDE->CreateObject(hClass, &theStruct);
		if (pClass)
		{
			RespawnPlayer(pClass, hClient);
		}
	}

	GameType eType = GetGameType();
	if (eType == SINGLE || eType == COOPERATIVE)
	{
		// Create story trigger...

		hClass = g_pServerDE->GetClass("CStoryTrigger");
		if (hClass)
		{
			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT(theStruct);
			theStruct.m_Flags |= FLAG_KEEPALIVE;

			g_pServerDE->CreateObject(hClass, &theStruct);
		}
	}

	return pClass;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::RespawnPlayer()
//
//	PURPOSE:	Respawn the player object
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::RespawnPlayer(BaseClass* pClass, HCLIENT hClient)
{
	if (!pClass || !hClient) return;

	// Player object meet client, client meet player object...

	CPlayerObj* pPlayer = (CPlayerObj*)pClass;
	pPlayer->SetClient(hClient);
	g_pServerDE->SetClientUserData(hClient, (void *)pClass);

	// If this is a multiplayer game wait until MULTIPLAYER_INIT message
	// to respawn the player...

	DBYTE nMessage = MID_PLAYER_MULTIPLAYER_INIT;

	if (GetGameType() == SINGLE)
	{
		pPlayer->Respawn(m_hstrStartPointName, m_nLastLGFlags);

		nMessage = MID_PLAYER_SINGLEPLAYER_INIT;
	}

	// Tell the client to init us!

	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(hClient, nMessage);
	g_pServerDE->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleCheatBigGuns()
//
//	PURPOSE:	Handle the big guns cheat
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleCheatBigGuns(DBYTE nData)
{
	if (!g_pServerDE) return;

	char buf[10];
	sprintf(buf, "%d", nData);
	g_pServerDE->SetGameConVar("BigGuns", buf);

	HOBJECT hObj   = g_pServerDE->GetNextObject(DNULL);
	HCLASS  hClass = g_pServerDE->GetClass("CBaseCharacter");

	// Update all the base character objects...

	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			CBaseCharacter* pChar = (CBaseCharacter*)g_pServerDE->HandleToObject(hObj);
			if (pChar)
			{
				pChar->HandleBigGunsCheat();
			}
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}

	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			CBaseCharacter* pChar = (CBaseCharacter*)g_pServerDE->HandleToObject(hObj);
			if (pChar)
			{
				pChar->HandleBigGunsCheat();
			}
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleCheatRemoveAI()
//
//	PURPOSE:	Handle the remove ai cheat
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleCheatRemoveAI(DBYTE nData)
{
	if (!g_pServerDE) return;

	HOBJECT hObj   = g_pServerDE->GetNextObject(DNULL);
	HCLASS  hClass = g_pServerDE->GetClass("BaseAI");

	// Remove all the ai objects...

	DBOOL bRemove = DFALSE;

	HOBJECT hRemoveObj = DNULL;
	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

		hObj = g_pServerDE->GetNextObject(hObj);

		if (hRemoveObj)
		{
			DVector vDir;
			VEC_INIT(vDir);
	
			LPBASECLASS pClass = g_pServerDE->HandleToObject(hRemoveObj);
			if (pClass)
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject(pClass, hRemoveObj, MID_DAMAGE);
				g_pServerDE->WriteToMessageVector(hMessage, &vDir);
				g_pServerDE->WriteToMessageFloat(hMessage, 100000.0f);
				g_pServerDE->WriteToMessageByte(hMessage, DT_KATO);
				g_pServerDE->WriteToMessageObject(hMessage, hRemoveObj);
				g_pServerDE->EndMessage(hMessage);
			}

			hRemoveObj = DNULL;
		}
	}


	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	hRemoveObj = DNULL;
	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);

		if (hRemoveObj)
		{
			DVector vDir;
			VEC_INIT(vDir);
	
			LPBASECLASS pClass = g_pServerDE->HandleToObject(hRemoveObj);
			if (pClass)
			{
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject(pClass, hRemoveObj, MID_DAMAGE);
				g_pServerDE->WriteToMessageVector(hMessage, &vDir);
				g_pServerDE->WriteToMessageFloat(hMessage, 100000.0f);
				g_pServerDE->WriteToMessageByte(hMessage, DT_KATO);
				g_pServerDE->WriteToMessageObject(hMessage, hRemoveObj);
				g_pServerDE->EndMessage(hMessage);
			}

			hRemoveObj = DNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleTriggerBoxCheat()
//
//	PURPOSE:	Handle the trigger box cheat
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleTriggerBoxCheat(DBYTE nData)
{
	if (!g_pServerDE) return;

	HOBJECT hObj   = g_pServerDE->GetNextObject(DNULL);
	HCLASS  hClass = g_pServerDE->GetClass("Trigger");

	// Update all the trigger objects...

	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			Trigger* pTrigger = (Trigger*)g_pServerDE->HandleToObject(hObj);
			if (pTrigger)
			{
				pTrigger->ToggleBoundingBoxes();
			}
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}


	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			Trigger* pTrigger = (Trigger*)g_pServerDE->HandleToObject(hObj);
			if (pTrigger)
			{
				pTrigger->ToggleBoundingBoxes();
			}
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleLoadGameMsg()
//
//	PURPOSE:	Handle loading a game
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleLoadGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DRESULT dResult = LT_OK;
	DDWORD flags;

	// Read the message data...

	m_nLastLGFlags	= pServerDE->ReadFromMessageByte(hMessage);
	m_eDifficulty	= (GameDifficulty) pServerDE->ReadFromMessageByte(hMessage);
	HSTRING	hLGName	= pServerDE->ReadFromMessageHString(hMessage);
	HSTRING hSGName	= pServerDE->ReadFromMessageHString(hMessage);
	HSTRING hROName	= pServerDE->ReadFromMessageHString(hMessage);

	// Save client-side info if loading a new level...

	HMESSAGEREAD hClientData = pServerDE->ReadFromMessageHMessageRead(hMessage);

	ObjectList* pKeepAliveList = pServerDE->CreateObjectList();


	char* pLGFileName = DNULL;
	char* pSGFileName = DNULL;
	char* pROFileName = DNULL;
	
	if (hLGName) pLGFileName = pServerDE->GetStringData(hLGName);
	if (hSGName) pSGFileName = pServerDE->GetStringData(hSGName);
	if (hROName) pROFileName = pServerDE->GetStringData(hROName);

	DBOOL bLoadWorldObjects		= DFALSE;
	DBOOL bSaveKeepAlives		= DFALSE;
	DBOOL bRestoreLevelObjects	= DFALSE;


	// Set up the player's client data...

	void *pData = pServerDE->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (pPlayer)
	{
		pPlayer->SetClientSaveData(hClientData);
	}

	if (!hLGName) 
	{
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (m_nLastLGFlags == LOAD_NEW_GAME)
	{
		bLoadWorldObjects = DTRUE;
	}
	else if (m_nLastLGFlags == LOAD_NEW_LEVEL)
	{
		bLoadWorldObjects	= DTRUE;
		bSaveKeepAlives		= DTRUE;
	}
	else if (m_nLastLGFlags == LOAD_RESTORE_GAME)
	{
		bRestoreLevelObjects = DTRUE;
	}
	else // Invalid flags
	{
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Invalid flags!");
		goto FREE_DATA;
	}


	// Validate the filenames...
	
	if (!pLGFileName || pLGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (bRestoreLevelObjects)
	{
		if (!pROFileName || pROFileName[0] == ' ')
		{
			ReportError(hSender, SERROR_LOADGAME);
			pServerDE->BPrint("Load Game Error: Invalid restore objects filename!");
			goto FREE_DATA;
		}
	}

	
	if (!pKeepAliveList)
	{	
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Couldn't create keep alive list!");
		goto FREE_DATA;
	}


	if (bSaveKeepAlives)
	{
		// Build keep alives list...

		// Find the story trigger and save it...

		HOBJECT hObj   = g_pServerDE->GetNextObject(DNULL);
		HCLASS  hClass = g_pServerDE->GetClass("CStoryTrigger");

		// Look in active objects list first...

		while (hObj)
		{
			if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
			{
				pServerDE->AddObjectToList(pKeepAliveList, hObj);
				break;
			}

			hObj = g_pServerDE->GetNextObject(hObj);
		}

		// Look in inactive objects list...

		if (!hObj)
		{

			hObj = pServerDE->GetNextInactiveObject(DNULL);
			while (hObj)
			{
				if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
				{
					pServerDE->AddObjectToList(pKeepAliveList, hObj);
					break;
				}

				hObj = g_pServerDE->GetNextInactiveObject(hObj);
			}
		}


		// Let the player add necessary keep alives to the list...

		void *pData = pServerDE->GetClientUserData(hSender);
		CPlayerObj* pPlayer = (CPlayerObj*)pData;
		if (pPlayer)
		{
			pPlayer->BuildKeepAlives(pKeepAliveList);
		}


		dResult = pServerDE->SaveObjects(KEEPALIVE_FILENAME, pKeepAliveList, m_nLastLGFlags, 0);
	
		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			pServerDE->BPrint("Load Game Error: Couldn't save keepalives");
			goto FREE_DATA;
		}
	}


	// Load the new level...

	flags = bLoadWorldObjects ? LOADWORLD_LOADWORLDOBJECTS : 0;
	flags |= LOADWORLD_NORELOADGEOMETRY;
	dResult = pServerDE->LoadWorld(pLGFileName, flags);
	
	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Couldn't Load world '%s'", pLGFileName);
		goto FREE_DATA;
	}


	if (bRestoreLevelObjects)
	{
		dResult = pServerDE->RestoreObjects(pROFileName, m_nLastLGFlags, RESTOREOBJECTS_RESTORETIME);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			pServerDE->BPrint("Load Game Error: Couldn't restore objects '%s'", pROFileName);
			goto FREE_DATA;
		}
	}


	// Load the keep alives...

	if (bSaveKeepAlives && pKeepAliveList && pKeepAliveList->m_nInList > 0)
	{
		dResult = pServerDE->RestoreObjects(KEEPALIVE_FILENAME, m_nLastLGFlags, 0);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			pServerDE->BPrint("Load Game Error: Couldn't restore keepalives");
			goto FREE_DATA;
		}
	}



	// Start the world...

	dResult = pServerDE->RunWorld();

	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
		pServerDE->BPrint("Load Game Error: Couldn't run world!");
	}


	// Make the client force the world to start...

	//if (m_nLastLGFlags == LOAD_NEW_GAME || m_nLastLGFlags == LOAD_NEW_LEVEL)
	//{
	//	PauseGame(DTRUE);
	//}


// Waste not, want not...

FREE_DATA:

	if (pKeepAliveList)
	{
		pServerDE->RelinquishList(pKeepAliveList);
	}

	if (hClientData)
	{
		pServerDE->EndHMessageRead(hClientData);
	}

	if (hLGName) pServerDE->FreeString(hLGName);
	if (hSGName) pServerDE->FreeString(hSGName);
	if (hROName) pServerDE->FreeString(hROName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::HandleSaveGameMsg()
//
//	PURPOSE:	Handle saving a game
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::HandleSaveGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pSGFileName = DNULL;
	HOBJECT	hObj = DNULL;
	ObjectList* pSaveList = pServerDE->CreateObjectList();

	// Read the message data...

	DBYTE	nSGFlags = pServerDE->ReadFromMessageByte(hMessage);
	HSTRING	hSGName	 = pServerDE->ReadFromMessageHString(hMessage);

	HMESSAGEREAD hClientData = pServerDE->ReadFromMessageHMessageRead(hMessage);


	// Set up the player's client data...

	void *pData = pServerDE->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (pPlayer)
	{
		pPlayer->SetClientSaveData(hClientData);
	}

	if (!hSGName) 
	{
		ReportError(hSender, SERROR_SAVEGAME);
		pServerDE->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}

	pSGFileName = pServerDE->GetStringData(hSGName);
	if (!pSGFileName || pSGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_SAVEGAME);
		pServerDE->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}


	// Save all the objects...

	if (!pSaveList) 
	{
		ReportError(hSender, SERROR_SAVEGAME);
		pServerDE->BPrint("Save Game Error: Allocation error!");
		goto FREE_DATA;
	}

	// Add active objects to the list...

	hObj = g_pServerDE->GetNextObject(DNULL);
	while (hObj)
	{
		pServerDE->AddObjectToList(pSaveList, hObj);
		hObj = g_pServerDE->GetNextObject(hObj);
	}

	// Add inactive objects to the list...

	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		pServerDE->AddObjectToList(pSaveList, hObj);
		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}


	if (pSaveList && pSaveList->m_nInList > 0)
	{
		DRESULT dResult = pServerDE->SaveObjects(pSGFileName, pSaveList, LOAD_RESTORE_GAME, 
												 SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEPORTALS);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_SAVEGAME);
			pServerDE->BPrint("Save Game Error: Couldn't save objects!");
		}
	}


// Waste not, want not...

FREE_DATA:

	if (hClientData)
	{
		pServerDE->EndHMessageRead(hClientData);
	}

	if (hSGName) pServerDE->FreeString(hSGName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::ReportError()
//
//	PURPOSE:	Tell the client about a server error
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::ReportError(HCLIENT hSender, DBYTE nErrorType)
{	
	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(hSender, MID_SERVER_ERROR);
	g_pServerDE->WriteToMessageByte(hWrite, nErrorType);
	g_pServerDE->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::CacheFiles()
//
//	PURPOSE:	Cache files that are used often
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::CacheFiles()
{
	CacheModels();
	CacheTextures();
	CacheSprites();
	CacheSounds();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::CacheModels()
//
//	PURPOSE:	Cache models that are used often
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::CacheModels()
{
	if (!g_pServerDE) return;

	for (int i=0; i < NUM_CACHED_MODELS; i++)
	{
		g_pServerDE->CacheFile(FT_MODEL, g_pCachedModels[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::CacheTextures()
//
//	PURPOSE:	Cache textures that are used often
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::CacheTextures()
{
	if (!g_pServerDE) return;

	for (int i=0; i < NUM_CACHED_TEXTURES; i++)
	{
		g_pServerDE->CacheFile(FT_TEXTURE, g_pCachedTextures[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::CacheSprites()
//
//	PURPOSE:	Cache sprites that are used often
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::CacheSprites()
{
	if (!g_pServerDE) return;

	for (int i=0; i < NUM_CACHED_SPRITES; i++)
	{
		g_pServerDE->CacheFile(FT_SPRITE, g_pCachedSprite[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::CacheSounds()
//
//	PURPOSE:	Cache sounds that are used often
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::CacheSounds()
{
	if (!g_pServerDE) return;

	for (int i=0; i < NUM_CACHED_SOUNDS_LOCAL; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSoundLocal[i]);
	}

	for (int i=0; i < NUM_CACHED_SOUNDS_AMBIENT; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSoundAmbient[i]);
	}

	for (int i=0; i < NUM_CACHED_SOUNDS_3D; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSound3D[i]);
	}
}


void CRiotServerShell::UpdateClientPingTimes()
{
	HMESSAGEWRITE hWrite;
	float ping;
	DDWORD clientID;
	HCLIENT hClient;
	
	ServerDE *pServerDE = g_pServerDE;
	if(!pServerDE)
		return;

	m_ClientPingSendCounter += pServerDE->GetFrameTime();
	if(m_ClientPingSendCounter > CLIENT_PING_UPDATE_RATE)
	{
		hWrite = pServerDE->StartMessage(DNULL, MID_PINGTIMES);
		
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

		m_ClientPingSendCounter = 0.0f;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::Update
//
//	PURPOSE:	Update servier stuff periodically
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::Update(DFLOAT timeElapsed)
{
	float todHours;

	// Sanity checks...
	ServerDE *pServerDE = g_pServerDE;

	if (!pServerDE || GetGameType() == SINGLE) return;



	// If it's the first update, setup time of day variables.
	if(m_bFirstUpdate)
	{
		m_SayTrack.Init(pServerDE, "Say", "", 0.0f);

		g_RammingDamageTrack.Init(pServerDE, "RammingDamage", DNULL, 1.0f);
		m_ShowTimeTrack.Init(pServerDE, "ShowTime", DNULL, 0.0f);
		m_WorldTimeTrack.Init(pServerDE, "WorldTime", "-1", 0.0f);
		m_WorldTimeSpeedTrack.Init(pServerDE, "WorldTimeSpeed", "-1", 0.0f);
		m_WorldColorDayTrack.Init(pServerDE, "WorldColorDay", "1 1 1", 0.0f);
		m_WorldColorNightTrack.Init(pServerDE, "WorldColorNight", ".3 .3 .3", 0.0f);
		m_bFirstUpdate = DFALSE;
	}


	// Did the server want to say something?
	char *pSay = m_SayTrack.GetStr("");
	if(pSay && pSay[0] != 0)
	{
		char fullMsg[512];

		sprintf(fullMsg, "HOST: %s", pSay);
		HMESSAGEWRITE hMessage = pServerDE->StartMessage (NULL, MID_PLAYER_MESSAGE);
		pServerDE->WriteToMessageString (hMessage, fullMsg);
		pServerDE->EndMessage2 (hMessage, MESSAGE_NAGGLE);
		
		m_SayTrack.SetStr("");
	}


	// Check the time of day stuff (every half second).
	DVector day, night, theColor;
	DBYTE newColor[3];
	float brightness;
	HMESSAGEWRITE hWrite;
	char *pStr;

	if(m_WorldTimeSpeedTrack.GetFloat() == -1)
	{
		newColor[0] = newColor[1] = newColor[2] = MAX_WORLDTIME_COLOR;
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

		m_TODCounter += timeElapsed;
		if(m_TODCounter > 0.5f)
		{
			if(todHours < 12.0f)
				brightness = todHours / 12.0f;
			else
				brightness = (12.0f - (todHours - 12.0f)) / 12.0f;

			pStr = m_WorldColorDayTrack.GetStr("1 1 1");
			sscanf(pStr, "%f %f %f", &day.x, &day.y, &day.z);

			pStr = m_WorldColorNightTrack.GetStr(".3 .3 .3");
			sscanf(pStr, "%f %f %f", &night.x, &night.y, &night.z);

			theColor = night + (day - night) * brightness;
			VEC_CLAMP(theColor, 0.0f, 1.0f);

			newColor[0] = (DBYTE)(theColor.x * (float)MAX_WORLDTIME_COLOR);
			newColor[1] = (DBYTE)(theColor.y * (float)MAX_WORLDTIME_COLOR);
			newColor[2] = (DBYTE)(theColor.z * (float)MAX_WORLDTIME_COLOR);
			
			m_TODCounter = 0.0f;
		}
		else
		{
			newColor[0] = m_WorldTimeColor[0];
			newColor[1] = m_WorldTimeColor[1];
			newColor[2] = m_WorldTimeColor[2];
		}
	}

	if(m_ShowTimeTrack.GetFloat() != 0.0f)
	{
		pServerDE->CPrint("TOD: %.2f, Color: %d %d %d", TODSecondsToHours(m_TODSeconds), newColor[0], newColor[1], newColor[2]);
	}

	// Did it change?
	if(newColor[0] != m_WorldTimeColor[0] || newColor[1] != m_WorldTimeColor[1] ||
		newColor[2] != m_WorldTimeColor[2])
	{
		hWrite = pServerDE->StartMessage(DNULL, MID_TIMEOFDAYCOLOR);
		pServerDE->WriteToMessageByte(hWrite, newColor[0]);
		pServerDE->WriteToMessageByte(hWrite, newColor[1]);
		pServerDE->WriteToMessageByte(hWrite, newColor[2]);
		pServerDE->EndMessage2(hWrite, MESSAGE_NAGGLE);

		m_WorldTimeColor[0] = newColor[0];
		m_WorldTimeColor[1] = newColor[1];
		m_WorldTimeColor[2] = newColor[2];
	}


	UpdateClientPingTimes();


	// Setup a static timer for session name updates...

	static DFLOAT timerUpdateName = UPDATENAME_INTERVAL;


	// Update our time and see if it's time to update the session name...

	if (timeElapsed < timerUpdateName)
	{
		timerUpdateName -= timeElapsed;
	}
	else
	{
		timerUpdateName = UPDATENAME_INTERVAL;
		UpdateSessionName();
	}


	// Update shogo server info...

	UpdateShogoServer();


	// Update multiplayer stuff...

	UpdateMultiplayer();

	// See if it wants us to switch worlds.
	CheckSwitchWorldCommand();
}


// See if the SwitchWorld console variable is set, if so, change to this world.
void CRiotServerShell::CheckSwitchWorldCommand()
{
	ServerDE *pServerDE;
	char *pCurLevelName, *pNextLevelName, *pCmdName, *pVal;

	pCmdName = "SwitchWorld";
	pServerDE = GetServerDE();	
	pCurLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	pNextLevelName = m_GameInfo.m_sLevels[(m_nCurLevel + 1) % m_GameInfo.m_byNumLevels];

	if(!m_hSwitchWorldVar)
	{
		pServerDE->SetGameConVar(pCmdName, "");
		m_hSwitchWorldVar = pServerDE->GetGameConVar(pCmdName);

		if(!m_hSwitchWorldVar)
			return;
	}
	
	// Is it set to anything?
	pVal = pServerDE->GetVarValueString(m_hSwitchWorldVar);
	if(pVal && pVal[0] != 0)
	{
		// Switch to the requested world.
		if(SwitchToWorld(pVal, pNextLevelName) != LT_OK)
			SwitchToWorld(pCurLevelName, pNextLevelName);

		// Reset this.
		pServerDE->SetGameConVar(pCmdName, "");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::UpdateSessionName
//
//	PURPOSE:	Updates the name of the session with current game info
//
// ----------------------------------------------------------------------- //

DBOOL CRiotServerShell::UpdateSessionName()
{
	// Check if this game is hosted via ShogoServ...

//	if (m_bShogoServHosted)
//	{
//		return(DFALSE);
//	}


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
	SAFE_STRCPY(sLevel, m_GameInfo.m_sLevels[m_nCurLevel]);


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
			count++;
			char sBase[32];

			sprintf(sBase, "%s%i", NST_PLRNAME_BASE, count);
			Sparam_Add(sSession, sBase, pPlayer->GetNetName());

			sprintf(sBase, "%s%i", NST_PLRFRAG_BASE, count);
			Sparam_Add(sSession, sBase, pPlayer->GetFragCount());
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
//	ROUTINE:	CRiotServerShell::UpdateShogoServer
//
//	PURPOSE:	Updates a stand-alone server with game info if necessary
//
// ----------------------------------------------------------------------- //

DBOOL CRiotServerShell::UpdateShogoServer()
{
	// Check if we need to update...

	if (!m_bUpdateShogoServ)
	{
		return(DFALSE);
	}

	m_bUpdateShogoServ = FALSE;


	// Make sure we are actually being hosted via ShogoServ...

	if (!m_bShogoServHosted)
	{
		return(DFALSE);
	}


	// Get the current base level name...

	char sCurLevel[128];
	SAFE_STRCPY(sCurLevel, m_GameInfo.m_sLevels[m_nCurLevel]);


	// Get the next base level name...

	char sNextLevel[128];
	int  i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	SAFE_STRCPY(sNextLevel, m_GameInfo.m_sLevels[i]);


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
			count++;
			char sBase[32];

			sprintf(sBase, "%s%i", NST_PLRNAME_BASE, count);
			Sparam_Add(sInfo, sBase, pPlayer->GetNetName());

			sprintf(sBase, "%s%i", NST_PLRFRAG_BASE, count);
			Sparam_Add(sInfo, sBase, pPlayer->GetFragCount());

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
//	ROUTINE:	CRiotServerShell::AddClientToList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

DBOOL CRiotServerShell::AddClientToList(HCLIENT hClient)
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
//	ROUTINE:	CRiotServerShell::RemoveClientFromList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

DBOOL CRiotServerShell::RemoveClientFromList(HCLIENT hClient)
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
//	ROUTINE:	CRiotServerShell::IsClientInList
//
//	PURPOSE:	Determines if the given client handle is in our list
//
// ----------------------------------------------------------------------- //

DBOOL CRiotServerShell::IsClientInList(HCLIENT hClient)
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
//	ROUTINE:	CRiotServerShell::GetPlayerFromClientList
//
//	PURPOSE:	Adds the given client handle to our local list
//
// ----------------------------------------------------------------------- //

CPlayerObj*	CRiotServerShell::GetPlayerFromClientList(HCLIENT hClient)
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
//	ROUTINE:	CRiotServerShell::SetupGameInfo
//
//	PURPOSE:	Setup game info
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::SetupGameInfo()
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
//	ROUTINE:	CRiotServerShell::UpdateMultiplayer
//
//	PURPOSE:	Determine if it is time to change levels
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::UpdateMultiplayer()
{
	if (!g_pServerDE || GetGameType() == SINGLE) return;
	
	if (m_GameInfo.m_byEnd == NGE_NEVER) return;

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
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
			if (pPlayer)
			{
				if (pPlayer->GetFragCount() >= (int)m_GameInfo.m_dwEndFrags)
				{
					bStartLevel = DTRUE;
					break;
				}
			}
		}
	}


	if (bStartLevel)
	{
		StartNextMultiplayerLevel();
	}
}


DRESULT CRiotServerShell::SwitchToWorld(char *pWorldName, char *pNextWorldName)
{
	ServerDE *pServerDE = GetServerDE();
	DRESULT dResult;

	// Tell the shogo server that we're changing the level...

	char sInfo[1024];
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGING);
	pServerDE->SendToServerApp(sInfo);


	// Tell all clients we're changing levels

	HMESSAGEWRITE hWrite = pServerDE->StartMessage (DNULL, MID_CHANGING_LEVELS);
	pServerDE->EndMessage (hWrite);

	
	// Clear player frags...

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayerObj* pPlayer = GetPlayerFromClientList(m_aClients[i]);
		if (pPlayer)
		{
			pPlayer->SetFragCount(0);
		}
	}


	// Load the next level...
	if (pWorldName)
	{
		dResult = pServerDE->LoadWorld(pWorldName, LOADWORLD_LOADWORLDOBJECTS|LOADWORLD_RUNWORLD);
		if(dResult != LT_OK)
		{
			// We must do this because Shogoserv needs to exit a critical section.
			sInfo[0] = 0;
			Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGESTOP);
			pServerDE->SendToServerApp(sInfo);
			return dResult;
		}
	}
	else
	{
		// We must do this because Shogoserv needs to exit a critical section.
		sInfo[0] = 0;
		Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGESTOP);
		pServerDE->SendToServerApp(sInfo);

		pServerDE->BPrint("ERROR CAN'T START NEXT MULTIPLAYER LEVEL!");
		return LT_ERROR;
	}


	// Tell the shogo server that we changed the level...

	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_LEVELCHANGED);

	char sCurLevel[128];
	SAFE_STRCPY(sCurLevel, m_GameInfo.m_sLevels[m_nCurLevel]);

	char sNextLevel[128];
	int i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	SAFE_STRCPY(sNextLevel, m_GameInfo.m_sLevels[i]);

	Sparam_Add(sInfo, NST_CURLEVEL, sCurLevel);
	Sparam_Add(sInfo, NST_NEXTLEVEL, sNextLevel);

	pServerDE->SendToServerApp(sInfo);
	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::StartNextMultiplayerLevel
//
//	PURPOSE:	Start the next multiplayer level
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::StartNextMultiplayerLevel()
{
	if (++m_nCurLevel >= m_GameInfo.m_byNumLevels)
	{
		m_nCurLevel = 0;
	}

	char* pLevelName = m_GameInfo.m_sLevels[m_nCurLevel];
	int i = m_nCurLevel + 1;
	if (i >= m_GameInfo.m_byNumLevels) i = 0;
	char *pNextLevelName = m_GameInfo.m_sLevels[i];

	SwitchToWorld(pLevelName, pNextLevelName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::ServerAppMessageFn
//
//	PURPOSE:	Server app message function
//
// ----------------------------------------------------------------------- //

DRESULT CRiotServerShell::ServerAppMessageFn(char* sMsg)
{
	// Sanity checks...

	if (!sMsg) return(LT_OK);


	// Check for "GAMEINIT" message...

	if (strcmp(sMsg, "GAMEINIT") == 0)
	{
		SetupGameInfo();
	}
	else if (strcmp(sMsg, "NEXTLEVEL") == 0)
	{
		StartNextMultiplayerLevel();
	}
	else if (strcmp(sMsg, "SERVHOST") == 0)
	{
		m_bShogoServHosted = DTRUE;
	}


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::SendShogoServConsoleMessage
//
//	PURPOSE:	Sends a string to be displayed in the ShogoServ console
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::SendShogoServConsoleMessage(char* sMsg)
{
	if (!sMsg) return;

	char sInfo[1024];
	sInfo[0] = '\0';

	Sparam_Add(sInfo, NST_GENERICMESSAGE, NGM_CONSOLEMSG);
	Sparam_Add(sInfo, NST_CONSOLEMESSAGE, sMsg);

	g_pServerDE->SendToServerApp(sInfo);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotServerShell::PauseGame
//
//	PURPOSE:	Pause/unpause the game
//
// ----------------------------------------------------------------------- //

void CRiotServerShell::PauseGame(DBOOL bPause)
{
	DDWORD nFlags = g_pServerDE->GetServerFlags();

	if (bPause)
	{
		nFlags |= SS_PAUSED;
	}
	else
	{
		nFlags &= ~SS_PAUSED;
	}
	
	g_pServerDE->SetServerFlags (nFlags);
}

