//------------------------------------------------------------------
//
//  FILE	  : ServerMgr.cpp
//
//  PURPOSE   :
//
//  CREATED   : November 8 1996
//
//  COPYRIGHT : LithTech Inc., 1996-2000
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "servermgr.h"
#include "sysdebugging.h"
#include "geomroutines.h"
#include "s_net.h"
#include "s_object.h"
#include "moveobject.h"
#include "animtracker.h"
#include "systimer.h"
#include "serverde_impl.h"
#include "interlink.h"
#include "sounddata.h"
#include "soundtrack.h"
#include "s_concommand.h"
#include "stringmgr.h"
#include "sysstreamsim.h"
#include "game_serialize.h"
#include "server_interface.h"
#include "server_extradata.h"
#include "syscounter.h"
#include "soundtrack.h"
#include "serverde_impl.h"
#include "smoveabstract.h"
#include "ltmessage_server.h"
#include "server_filemgr.h"
#include "s_client.h"
#include "serverevent.h"
#include "dhashtable.h"
#include "ftserv.h"
#include "soundtrack.h"
#include "ltobjectcreate.h"
#include <time.h>
#include "ltobjref.h"


// [KLS 4/19/02] - All the class-tick stuff is really just debugging info, so make sure we aren't
// wasting anytime doing anything in final builds...
//
#ifndef _FINAL
#define _PROCESS_CLASS_TICKS_
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//server console state
#include "server_consolestate.h"
#include "concommand.h"
static IServerConsoleState *console_state;
define_holder(IServerConsoleState, console_state);

//IWorld holder
#include "world_server_bsp.h"
#include "de_mainworld.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//IWorld holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);

//IWorldBlindObjectData holder
#include "world_blind_object_data.h"
static IWorldBlindObjectData *g_iWorldBlindObjectData = LTNULL;
define_holder(IWorldBlindObjectData, g_iWorldBlindObjectData);


extern int32 g_bForceRemote;
extern float g_ServerFPS;
#ifdef DE_SERVER_COMPILE
extern int32 g_LockServerFPS;
#endif // DE_SERVER_COMPILE
extern float g_ServerTimeScale;

uint32 g_ObjectMemory;

extern uint32 g_Ticks_MoveObject;
extern uint32 g_nMoveObjectCalls;
extern uint32 g_Ticks_Intersect, g_nIntersectCalls;
uint32 g_SphereFindTicks, g_SphereFindCount;

extern int32 g_CV_ShowClassTicks;

extern int32 g_CV_ShowGameTime;
extern int32 g_CV_ShowSphereFindTicks;

extern int32 g_CV_BandwidthTargetServer;

CServerMgr	  *g_pServerMgr = LTNULL;

#ifdef DE_SERVER_COMPILE
	ObjectBank<LTLink> g_DLinkBank;
#endif


// ----------------------------------------------------------------------- //
// The server's main message-handling function.
// ----------------------------------------------------------------------- //

void s_DisassociateClientsFromObjects() 
{
	LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext) 
	{
		Client *pClient = (Client*)pCur->m_pData;

		if (pClient->m_pObject) 
		{
			pClient->m_pObject->sd->m_pClient = LTNULL;
			pClient->m_pObject = LTNULL;
		}
	}
}



// Allocates a new ID for the object.
LTRESULT sm_AllocateID(LTLink **ppIDLink, uint16 objectID) 
{
	LTLink *pListHead, *pCur;

	if (objectID == INVALID_OBJECTID) 
	{
		// Look for an unblocked ID.
		bool bFoundOne = false;

		pListHead = &g_pServerMgr->m_FreeIDs;
		for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) 
		{
			*ppIDLink = pCur;

			if (!(GetLinkID(*ppIDLink) & IDFLAG_BLOCKEDID)) 
			{
				bFoundOne = true;
				break;
			}
		}

		// If if didn't find one, create a new object
		if (!bFoundOne) 
		{
			LTRESULT dResult;
			if ((dResult = sm_CreateNewID(ppIDLink)) != LT_OK) 
			{
				return dResult;
			}
		}
	}
	else 
	{
		// If objects up to this ID haven't been created yet, generate them.
		while (objectID >= g_pServerMgr->m_nAllocatedIDs) 
		{
			LTRESULT dResult;
			if ((dResult = sm_CreateNewID(ppIDLink)) != LT_OK) 
			{
				return dResult;
			}
		}

		*ppIDLink = sm_FindInFreeList(objectID);
		if (!*ppIDLink) 
		{
			RETURN_ERROR(1, sm_AllocateID, LT_ERROR);
		}
	}

	// Unblock the ID.
	SetLinkID(*ppIDLink, GetLinkID(*ppIDLink) & ~IDFLAG_BLOCKEDID);

	// Take it out of the free list and put it in the allocated list.
	dl_Remove(*ppIDLink);
	dl_Insert(&g_pServerMgr->m_IDs, *ppIDLink);

	// Resize the update infos if necessary.
	if (g_pServerMgr->m_nAllocatedIDs > g_pServerMgr->m_nObjInfos)
		g_pServerMgr->ResizeUpdateInfos(g_pServerMgr->m_nAllocatedIDs);

	// Clear its object info flags.
	uint32 nCurTime = timeGetTime();
	pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext) 
	{
		ObjInfo *pInfo = &((Client*)pCur->m_pData)->m_ObjInfos[GetLinkID(*ppIDLink)];
		pInfo->m_ChangeFlags = 0;
		pInfo->m_nSoundFlags = 0;
		pInfo->m_nLastSentG = nCurTime;
		pInfo->m_nLastSentU = nCurTime;
	}

	return LT_OK;
}


void sm_FreeID(LTLink *pIDLink) 
{
	// Remove from its current list.
	dl_Remove(pIDLink);

	// Put it in the free list.
	dl_Insert(&g_pServerMgr->m_FreeIDs, pIDLink);
}


void sm_FreeObjectScript(LTObject *pObject) 
{
	if (pObject->sd) 
	{
		if (pObject->sd->m_pObject) 
		{
			sm_FreeObjectOfClass(pObject->sd->m_pClass, pObject->sd->m_pObject);
			pObject->sd->m_pObject = LTNULL;
		}
	}
}


// Removes all objects (including inactive ones) from the world.
void sm_RemoveAllObjectsFromWorld(bool bRemoveStaticObjects) 
{
	// Remove any objects waiting to get removed.. it does this here so their
	// DLinks get freed correctly.
	sm_RemoveObjectsThatNeedToGetRemoved();

	typedef std::vector<LTObjRef> TObjRefList;
	TObjRefList aDeletingObjects;
	aDeletingObjects.reserve(g_pServerMgr->m_Objects.m_nElements);

	LTLink *pCur = g_pServerMgr->m_Objects.m_Head.m_pNext;
	while (pCur != &g_pServerMgr->m_Objects.m_Head) 
	{
		aDeletingObjects.push_back((LTObject*)pCur->m_pData);
		pCur = pCur->m_pNext;
	}

	// Remove all objects.
	TObjRefList::iterator iCurObj = aDeletingObjects.begin();
	for (; iCurObj != aDeletingObjects.end(); ++iCurObj)
	{
		LTObject *pObject = *iCurObj;
		if (!pObject)
			continue;
		if (bRemoveStaticObjects || ((pObject->sd->m_pClass->m_ClassFlags & CF_STATIC) == 0))
		{
			sm_RemoveObjectFromWorld(pObject->sd->m_pObject);
			// Some wacky objects put themselves back into the remove list when they get removed..
			// So don't delete those objects, just remove them.
			LTLink *pFilterCurObj = g_pServerMgr->m_RemovedObjectHead.m_pNext;
			while (pFilterCurObj != &g_pServerMgr->m_RemovedObjectHead)
			{
				LTLink *pNext = pFilterCurObj->m_pNext;
				if (pObject == (LTObject*)pFilterCurObj->m_pData)
				{
					dl_Remove(pFilterCurObj);
					g_DLinkBank.Free(pFilterCurObj);
				}
				pFilterCurObj = pNext;
			}
			// We might have put some objects on the delayed release list in the process, so clear that out again
			sm_RemoveObjectsThatNeedToGetRemoved();
		}
	}
}


// Frees all the elements of a list.
void sm_FreeList(LTList *pList) 
{
	LTLink *pListHead = &pList->m_Head;
	LTLink *pCur = pListHead->m_pNext;
	while (pCur != pListHead) 
	{
		LTLink *pNext = pCur->m_pNext;
		dfree(pCur->m_pData);
		pCur = pNext;
	}

	dl_TieOff(pListHead);
	pList->m_nElements = 0;
}


// Updates the client states (puts clients in the world if they
// were waiting to enter and can enter).
void sm_UpdateClientStates() 
{
	LTLink *pCur = g_pServerMgr->m_Clients.m_Head.m_pNext;
	while( pCur != &g_pServerMgr->m_Clients.m_Head ) 
	{
		LTLink* pNext = pCur->m_pNext;
		Client *pClient = (Client*)pCur->m_pData;
		if( pClient->m_ClientFlags & CFLAG_KICKED )
		{
			// Disconnect them via the netmgr.  Note: This will eventually cause the client to be
			// removed from the servermgr due to the disconnectnotify handling.
			g_pServerMgr->m_NetMgr.Disconnect(pClient->m_ConnectionID, DISCONNECTREASON_KICKED);
		}
		else
		{
		sm_UpdateClientState(pClient);
	}

		pCur = pNext;
	}
}


// Updates the client states for clients that are in the world.
 
void sm_UpdateClientsInWorld() 
{
	LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext) 
	{
		Client *pClient = (Client*)pCur->m_pData;
		sm_UpdateClientInWorld(pClient);
	}

	// Clear the send/drop counts
	g_pServerMgr->m_nSendPackets = 0;
	g_pServerMgr->m_nDroppedSendPackets = 0;
}

// ------------------------------------------------------------------------
// SendLoadRequest( model, client)
// tell the client to load this model. 
// this differenciates between cached and uncached files, so that the 
// client can know how to characterize the messsage.
// ------------------------------------------------------------------------
void SendLoadRequest(const Model*pModel, Client *pClient)
{
	CPacket_Write cPacket;

	cPacket.Writeuint8(SMSG_PRELOADLIST);

	if(pModel->m_Flags & MODELFLAG_CACHED)
	{
		cPacket.Writeuint8(PRELOADTYPE_MODEL_CACHED);
		DEBUG_MODEL_REZ(("model-rez: send cache-load req fileid(%d) %s   "  , pModel->m_FileID , pModel->GetFilename()));
	}
	else
	{
		cPacket.Writeuint8(PRELOADTYPE_MODEL);
		DEBUG_MODEL_REZ(("model-rez: send load req fileid(%d) %s   "  , pModel->m_FileID , pModel->GetFilename()));
	}
	
	// tell client the file-id
	cPacket.Writeuint16((uint16)pModel->m_FileID);

	::SendToClient(pClient, CPacket_Read(cPacket), false);
}


// ----------------------------------------------------------------------- //
//	  ROUTINE:		ServerMgr functions.
//	  PURPOSE:
// ----------------------------------------------------------------------- //

// This is implemented in servermain.cpp.
void _ServerStringWhine(const char *pString, void *pUser) 
{
	dsi_PrintToConsole("Unfreed (server) string: %s", pString);
}

 
CServerMgr::CServerMgr() :
	m_nSendPackets(0),
	m_nDroppedSendPackets(0),
	m_ChangedSoundTrackHead(0),
	m_nCurBandwidthTarget(g_CV_BandwidthTargetServer)
{ 
} // CServerMgr constructor

bool CServerMgr::Init()
{
	LT_MEM_TRACK_ALLOC(m_MoveAbstract = new SMoveAbstract(),LT_MEM_TYPE_MISC);

	if ((om_Init(&m_ObjectMgr, false) != LT_OK) ||
		!m_MoveAbstract )
	{
		return false;
	}

 
	world_bsp_server->ServerTree()->InitWorldTree(&m_ObjectMgr);

	//initialize the ILTServer implementation.
	CreateLTServerDE();


	m_NetMgr.Init("SERVER_PLAYER");
	m_NetMgr.SetNetHandler(this);

	dl_TieOff(&m_RemovedObjectHead);
	m_pServerAppHandler = LTNULL;
	m_pCurOCS = LTNULL;
	m_ErrorString[0] = 0;
	m_ObjectMap.SetCacheSize(100);
	m_CacheList = LTNULL;
	m_CacheListSize = m_CacheListAllocedSize = 0;

	memset(m_SkyObjects, 0xFF, sizeof(m_SkyObjects));

	m_hGlobalLightObject = 0;

	m_nObjInfos = UPDATEINFO_NUMSTART;
	m_nAllocatedIDs = 0;

	m_pGameInfo = LTNULL;
	m_GameInfoLen = 0;

	m_State = SERV_NOSTATE;
	m_ServerFlags = 0;
	m_InternalFlags = 0;

	g_ObjectMemory = 0;

	m_UpdatesSize = m_nUpdatesSent = 0;

	// Init lists and structure banks and stuff.
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_ObjectListBank, sizeof(ObjectList), 16, 4), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_ObjectLinkBank, sizeof(ObjectLink), 128, 64), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_InterLinkBank, sizeof(InterLink), 32, 32), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_ServerEventBank, sizeof(CServerEvent), 48, 32), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_ClientStructNodeBank, sizeof(ClientStructNode), 32, 16), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_SoundDataBank, sizeof(CSoundData), 16, 16), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_SoundTrackBank, sizeof(CSoundTrack), 16, 16), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&m_FileIDInfoBank, sizeof(FileIDInfo), 32, 32), LT_MEM_TYPE_MISC);

	m_SObjBank.SetCacheSize(64);

	dl_TieOff(&m_FreeIDs);
	dl_TieOff(&m_IDs);
	dl_InitList(&m_Objects);
	dl_InitList(&m_Clients);
	dl_InitList(&m_ClientReferences);
	dl_InitList(&m_SoundDataList);
	dl_InitList(&m_SoundTrackList);

	m_SkyDef.m_Min.Init(-10.0f, -10.0f, -10.0f);
	m_SkyDef.m_Max.Init(10.0f, 10.0f, 10.0f);
	m_SkyDef.m_ViewMin.Init();
	m_SkyDef.m_ViewMax.Init();

	m_bWorldLoaded = false;
	m_pWorldFile = LTNULL;
	m_pTracePacketFile = LTNULL;

	m_FrameTime = 0.0f;
	m_LastServerFPS = 0.0f;
	#ifdef DE_SERVER_COMPILE
	m_nTargetTimeSteps = 0;
	m_TargetTimeBase = 0.0f;
	#endif // DE_SERVER_COMPILE
	m_GameTime = 0.0f;
	m_nTrueFrameTimeMS = 0;
	m_nTrueLastTimeMS = 0;
	m_nTimeOffsetMS = 0;

	m_bNeedsFlowControlReset = true;

	m_hNameTable = hs_CreateHashTable(500, HASH_STRING_NOCASE);
	
	sm_InitConsoleCommands(console_state->State());
	cc_RunConfigFile(console_state->State(), "s_autoexec.cfg", CC_NOCOMMANDS, 0);

	server_filemgr->Init();

	m_ClassMgr.Init();
	m_StringHolder.SetAllocSize(300);


	m_pCollisionInfo = LTNULL;

	InitServerNetHandlers();

	SetFrameCode(0);

	return true;
}


void CServerMgr::Term()
{
	// Shutdown the world if it's running.
	DoEndWorld(false);

	//tell the server shell that the server is shutting down.
	if (i_server_shell != NULL) 
	{
		i_server_shell->OnServerTerm();
	}

	// Free game info.
	dfree(m_pGameInfo);
	m_pGameInfo   = LTNULL;
	m_GameInfoLen = 0;

	// Remove all the clients.
	LTLink *pListHead;
	pListHead = &m_Clients.m_Head;
	LTLink *pCur = pListHead->m_pNext;
	LTLink *pNext;
	while (pCur != pListHead)
	{
		pNext = pCur->m_pNext;
		sm_RemoveClient((Client*)pCur->m_pData);
		pCur = pNext;
	}
	dl_InitList(&m_Clients);

	m_NetMgr.Term();

	// All the objects better be cleared out.
	sm_RemoveAllObjectsFromWorld(true);

	// Delete the sound data and instances
	sm_RemoveAllSounds();

	ASSERT(m_Objects.m_Head.m_pNext == &m_Objects.m_Head);

	// Free all the IDs.
	for (pCur=m_FreeIDs.m_pNext; pCur != &m_FreeIDs; pCur=pNext)
	{
		pNext = pCur->m_pNext;
		g_DLinkBank.Free(pCur);
		--g_pServerMgr->m_nAllocatedIDs;
	}
	ASSERT(g_pServerMgr->m_nAllocatedIDs == 0);
	dl_TieOff(&m_FreeIDs);


	m_nAllocatedIDs = 0;
	m_ObjectMap.Term();

 
	// Destroy the world.
	world_bsp_server->Term();

	m_ClassMgr.Term();

	sm_TermConsoleCommands(console_state->State());

	// Whine about all the strings they didn't free.
	str_ShowAllStringsAllocated(_ServerStringWhine, LTNULL);

	// Get rid of the file trees.
	server_filemgr->Term();

	// Get rid of the struct banks.
	sb_Term(&m_ObjectListBank);
	sb_Term(&m_ObjectLinkBank);
	sb_Term(&m_InterLinkBank);
	sb_Term(&m_ServerEventBank);
	sb_Term(&m_ClientStructNodeBank);
	sb_Term(&m_SoundDataBank);
	sb_Term(&m_SoundTrackBank);
	sb_Term(&m_FileIDInfoBank);

	m_SObjBank.Term();

	hs_DestroyHashTable(m_hNameTable);
	m_hNameTable = LTNULL;

	// Free the sprite list.
	if (m_CacheList)
	{
		dfree(m_CacheList);
		m_CacheList = LTNULL;
	}
	m_CacheListSize = m_CacheListAllocedSize = 0;

	// Shut down the object manager.
	om_Term(&m_ObjectMgr);

	if (m_MoveAbstract)
	{
		delete m_MoveAbstract;
		m_MoveAbstract = LTNULL;
	}

	if (m_pTracePacketFile)
	{
		m_pTracePacketFile->Release();
		m_pTracePacketFile = LTNULL;
	}

	m_pServerAppHandler = LTNULL;
	g_pServerMgr = LTNULL;
}


bool CServerMgr::Listen(const char *pDriverInfo, const char *pListenInfo)
{
	CBaseDriver *pDriver = m_NetMgr.AddDriver(pDriverInfo);
	if (!pDriver)
	{
		sm_SetupError(LT_ERRORINITTINGNETDRIVER, pDriverInfo);
		return false;
	}

	dsi_ConsolePrint("Listening on driver: %s", pDriverInfo);
	return true;
}


bool CServerMgr::TransferNetDriver(CBaseDriver *pDriver)
{
	uint32 index = pDriver->m_pNetMgr->m_Drivers.FindElement(pDriver);
	if (index != BAD_INDEX)
		pDriver->m_pNetMgr->m_Drivers.Remove(index);

	m_NetMgr.m_Drivers.Append(pDriver);
	m_NetMgr.SetMainDriver(pDriver);
	pDriver->m_pNetMgr = &m_NetMgr;
	return true;
}


bool CServerMgr::AddResources(const char **pResources, uint32 nResources)
{
	if (nResources == 0)
	{
		sm_SetupError(LT_NOGAMERESOURCES);
		return false;
	}

	TreeType treeTypes[50];
	int32 nTreesLoaded;
	server_filemgr->AddResources(pResources, nResources, treeTypes, &nTreesLoaded);

	// Must at least have loaded one tree.
	if (nTreesLoaded == 0)
	{
		sm_SetupError(LT_CANTLOADGAMERESOURCES, pResources[0]);
		return false;
	}

	return true;
}

bool CServerMgr::LoadBinaries( )
{
	// Load all the server binaries.
	LTRESULT dResult = LoadServerBinaries(&m_ClassMgr);
	return dResult == LT_OK;
}



void CServerMgr::SetGameInfo(void *pData, uint32 dataLen)
{
	dfree(m_pGameInfo);
	LT_MEM_TRACK_ALLOC(m_pGameInfo = dalloc(dataLen),LT_MEM_TYPE_MISC);
	if (m_pGameInfo)
	{
		memcpy(m_pGameInfo, pData, dataLen);
		m_GameInfoLen = dataLen;
	}
}


int32 CServerMgr::GetNumClients()
{
	return m_Clients.m_nElements;
}


bool CServerMgr::GetPlayerName(int32 index, char *pName, int32 nMaxLen)
{
	pName[0] = 0;
	if (index >= (int32)m_Clients.m_nElements)
		return false;

	LTLink *pCur = m_Clients.m_Head.m_pNext;
	for (int32 i = 0; i < index; i++)
		pCur = pCur->m_pNext;

	Client *pStruct = (Client*)pCur->m_pData;

	InternalGetPlayerName(pStruct, pName, nMaxLen);
	return true;
}

bool CServerMgr::SetPlayerName(int32 index, const char *pName, int32 nMaxLen)
{
	if (!pName)
		return false;

	if (index >= (int32)m_Clients.m_nElements)
		return false;

	//
	//cycle to get the correct indexed client
	LTLink *pCur = m_Clients.m_Head.m_pNext;
	for (int32 i = 0; i < index; i++)
		pCur = pCur->m_pNext;
		
		
	Client *pStruct = (Client*)pCur->m_pData;

	InternalSetPlayerName(pStruct, pName, nMaxLen);

	return true;
}


bool CServerMgr::GetPlayerInfo(int32 index, ClientInfo* pCi)
{
	if (!pCi) 
		return false;

	if (index >= (int32)m_Clients.m_nElements)
		return false;

	LTLink *pCur = m_Clients.m_Head.m_pNext;
	for (int32 i = 0; i < index; i++)
		pCur = pCur->m_pNext;

	Client *pStruct = (Client*)pCur->m_pData;

	InternalGetPlayerName(pStruct, pCi->m_sName, MAX_CLIENTINFO_NAME);
	pCi->m_ClientID = pStruct->m_ClientID;

	pCi->m_Ping = -1.0f;
	if (pStruct->m_ConnectionID)
	{
		pCi->m_Ping = pStruct->m_ConnectionID->GetPing();
	}
	return true;
}

bool CServerMgr::BootPlayer(uint32 dwClientID)
{
	for (LTLink *pCur = m_Clients.m_Head.m_pNext; pCur != &m_Clients.m_Head; pCur = pCur->m_pNext)
	{
		Client *pClient = (Client*)pCur->m_pData;

		if (pClient->m_ClientID == dwClientID)
		{
			m_NetMgr.Disconnect(pClient->m_ConnectionID, DISCONNECTREASON_KICKED);
			return true;
		}
	}

	return(false);
}

Client* CServerMgr::GetClientFromId( uint32 dwClientID )
{
	LTLink *pCur;
	Client *pClient;

	// Search list of clients for client id.
	for(pCur=m_Clients.m_Head.m_pNext; pCur != &m_Clients.m_Head; pCur=pCur->m_pNext)
	{
		pClient = (Client*)pCur->m_pData;
		
		if(pClient->m_ClientID == dwClientID)
		{
			return pClient;
		}
	}

	return NULL;
}



// Clears the list of modified objects.
void sm_ClearChangedObjectList()
{
	LTLink *pCur, *pTemp;

	// Clear the change flags on the modified objects.
	pCur = g_pServerMgr->m_ChangedObjectHead.m_Head.m_pNext;
	while( pCur != &g_pServerMgr->m_ChangedObjectHead.m_Head)
	{
		(( LTObject* )( pCur->m_pData ))->sd->m_ChangeFlags = 0;
		pTemp = pCur->m_pNext;
		dl_RemoveAt( &g_pServerMgr->m_ChangedObjectHead, pCur );
		pCur = pTemp;
	}

	// Clear the 'changed object' list.
	dl_InitList( &g_pServerMgr->m_ChangedObjectHead );
}


// Clears the list of modified soundtracks.
void sm_ClearChangedSoundTrackList()
{
	// Clear the change flags on the modified objects.
	CSoundTrack *pCur = g_pServerMgr->m_ChangedSoundTrackHead;
	while (pCur)
	{
		pCur->m_wChangeFlags = 0;
		pCur = pCur->m_pChangedNext;
	}

	// Clear the 'changed object' list.
	g_pServerMgr->m_ChangedSoundTrackHead = LTNULL;
}



// Takes m_ChangeFlags from all the objects in the server's modified list and merges
// them with each client's object change flags and list.
void sm_MergeClientChangeLists()
{
	// For each client.
	LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink *pCurClient = pListHead->m_pNext; pCurClient != pListHead; pCurClient = pCurClient->m_pNext)
	{
		Client *pClient = (Client*)pCurClient->m_pData;

		if (pClient->m_State != CLIENT_INWORLD)
			continue;

		// For each object.
		LTLink *pCur = g_pServerMgr->m_ChangedObjectHead.m_Head.m_pNext;
		while (pCur != &g_pServerMgr->m_ChangedObjectHead.m_Head)
		{
			LTObject *pObj = (LTObject*)(pCur->m_pData);
			uint16 id = pObj->m_ObjectID;
			ObjInfo *pInfo = &pClient->m_ObjInfos[id];

			pInfo->m_ChangeFlags |= pObj->sd->m_ChangeFlags;
			ASSERT(pInfo->m_ChangeFlags);

			pCur = pCur->m_pNext;
		}

		// For each soundtrack.
		CSoundTrack *pSoundTrack = g_pServerMgr->m_ChangedSoundTrackHead;
		while (pSoundTrack)
		{
			uint16 id = (uint16)GetLinkID(pSoundTrack->m_pIDLink);
			ObjInfo *pInfo = &pClient->m_ObjInfos[id];

			pInfo->m_ChangeFlags |= pSoundTrack->m_wChangeFlags;
			ASSERT(pInfo->m_ChangeFlags);

			pSoundTrack = pSoundTrack->m_pChangedNext;
		}
	}

	sm_ClearChangedObjectList();
	sm_ClearChangedSoundTrackList();
}


void CServerMgr::PreUpdateObjects()
{
 
#ifdef _PROCESS_CLASS_TICKS_
 
	// Clear tick counts for each class.

	m_ClassMgr.ClearTickCounts();
 
#endif // _PROCESS_CLASS_TICKS_

 
 
	// Call the object init/update functions.

	LTLink* pHead = &m_Objects.m_Head;
	for (LTLink* pCur=pHead->m_pNext; pCur != pHead;)
	{
		LTObject* pObj = (LTObject*)pCur->m_pData;
		pCur=pCur->m_pNext;

		if (pObj->m_InternalFlags & IFLAG_INACTIVE_MASK)
		{

#ifdef _PROCESS_CLASS_TICKS_	

			if (g_CV_ShowClassTicks)
			{
				// Normally we don't care about these objects, but for
				// tracking add these to the list as well...
				CClassData *pClassData = (CClassData*)pObj->sd->m_pClass->m_pInternal[m_ClassMgr.m_ClassIndex];
				pClassData->UpdateTicks(pObj, 0 /*no time*/);
			}
			else
			{
				break;
			}

#else  // Normal behavoir

			// Since all inactive objects are at the end, just stop
			// at the first one.
			break;

#endif // _PROCESS_CLASS_TICKS_

		}
		else if (pObj->m_InternalFlags & IFLAG_INWORLD)
		{

#ifdef _PROCESS_CLASS_TICKS_

			// Start the counter for this object's update...

			CClassData *pClassData = (CClassData*)pObj->sd->m_pClass->m_pInternal[m_ClassMgr.m_ClassIndex];
			
			Counter cntTicks;
			cnt_StartCounter(cntTicks);

#endif // _PROCESS_CLASS_TICKS_

			
			// Do the object update...

			FullObjectUpdate(pObj);
	   	

#ifdef _PROCESS_CLASS_TICKS_

			// Update the class tick counters for this object...

			pClassData->UpdateTicks(pObj, cnt_EndCounter(cntTicks));

#endif // _PROCESS_CLASS_TICKS_

		}
	}


#ifdef _PROCESS_CLASS_TICKS_

	// Show the class tick counts.

	m_ClassMgr.ShowTickCounts();

#endif // _PROCESS_CLASS_TICKS_

	IncrementFrameCode();
}

// This is called after objects have been updating.  It updates client states,
// clears queues, etc.
void sm_FinishUpdateFrame()
{
	// Update client states (get clients into the world that were waiting).
	sm_UpdateClientStates();

	// Setup the client change flags based on the object change flags.
	sm_MergeClientChangeLists();

	// Remove the objects that got removed before updating in-world clients so they're not
	// in the BSP and don't get sent to the clients.
	while (g_pServerMgr->m_RemovedObjectHead.m_pNext != &g_pServerMgr->m_RemovedObjectHead)
	{
		sm_RemoveObjectsThatNeedToGetRemoved();
	}

	// Update the in-world clients.
	sm_UpdateClientsInWorld();
}

bool IsSoundTrackInRemoveList(CSoundTrack *pTest)
{
	CSoundTrack *pSoundTrack = g_pServerMgr->m_ChangedSoundTrackHead;
	while (pSoundTrack)
	{
		if (pSoundTrack == pTest)
			return true;

		pSoundTrack = pSoundTrack->m_pChangedNext;
	}

	return false;
}


void CServerMgr::UpdateSounds(float fDeltaTime)
{
	// Go through all the sound instances...
	LTLink *pNext;
	for (LTLink *pCur=m_SoundTrackList.m_Head.m_pNext; pCur != &m_SoundTrackList.m_Head; pCur = pNext)
	{
		CSoundTrack *pSoundTrack = (CSoundTrack*)pCur->m_pData;
		if (!pSoundTrack)
			return;

		pNext = pCur->m_pNext;

		// Handle removing sound...
		if (pSoundTrack->GetRemove())
		{
			if (pSoundTrack->m_nClientRefs == 0)
			{
				continue;
			}
		}

		pSoundTrack->Update(fDeltaTime);
	}
}


void CServerMgr::RemoveSounds()
{
	// Go through all the sound instances...
	LTLink *pNext;
	for (LTLink *pCur = m_SoundTrackList.m_Head.m_pNext; pCur != &m_SoundTrackList.m_Head; pCur = pNext)
	{
		CSoundTrack *pSoundTrack = (CSoundTrack*)pCur->m_pData;
		if (!pSoundTrack)
			return;

		pNext = pCur->m_pNext;

		// Handle removing sound...  (But only if it's not scheduled for a change..)
		if (pSoundTrack->GetRemove() && (pSoundTrack->m_wChangeFlags == 0))
		{
			if (pSoundTrack->m_nClientRefs == 0)
			{
				// Handle removing any remaining links to objects...
				if ((pSoundTrack->m_dwFlags & PLAYSOUND_ATTACHED) && pSoundTrack->GetObject())
				{
					LTObject *pObj = pSoundTrack->GetObject();
					if (pObj)
						DisconnectLinks(pObj, pSoundTrack);
				}

				ASSERT(!IsSoundTrackInRemoveList(pSoundTrack));

				dl_RemoveAt(&m_SoundTrackList, &pSoundTrack->m_Link);
				pSoundTrack->Term();
				sb_Free(&m_SoundTrackBank, pSoundTrack);
			}
		}
	}
}

bool CServerMgr::Update(int32 updateFlags, uint32 nCurTimeMS)
{
	#ifdef DE_SERVER_COMPILE
	static float s_serverSleepSecs = 0.0f;
	#endif // DE_SERVER_COMPILE

	int32 nOffsetTimeMS = (int32)nCurTimeMS + m_nTimeOffsetMS;

	float curTime = nOffsetTimeMS / 1000.0f;

	// Figure out the actual true frame time.
	m_nTrueFrameTimeMS = nOffsetTimeMS - m_nTrueLastTimeMS;
	m_nTrueLastTimeMS = nOffsetTimeMS;


	m_NetMgr.Update("Server: ", curTime);

	// Reset counters.
	g_Ticks_MoveObject = 0;
	g_nMoveObjectCalls = 0;
	g_Ticks_Intersect = g_nIntersectCalls = 0;
	g_SphereFindTicks = 0;
	g_SphereFindCount = 0;


	// Update the client flow control if necessary
	if ((uint32)g_CV_BandwidthTargetServer != m_nCurBandwidthTarget)
		m_bNeedsFlowControlReset = true;
	if (m_bNeedsFlowControlReset)
		InternalResetFlowControl();

	// Init the removed object list..
	// MD: Shouldn't ever clear it otherwise it won't removed some objects.
	//ASSERT(m_RemovedObjectHead.m_pNext == &m_RemovedObjectHead);
	//dl_TieOff(&m_RemovedObjectHead);

#ifdef DE_SERVER_COMPILE

	while (s_serverSleepSecs >= 0.0f )
	{
		float timeStart ;
		if (g_LockServerFPS)
			timeStart = time_GetTime();

		if (ProcessIncomingPackets() != LT_OK)
			return false;

		if (g_LockServerFPS)
		{
			float timeElapsed = (time_GetTime() - timeStart);
			if (timeElapsed == 0.0f)
				timeElapsed = 0.001f; // artificially force it to always take "at least 1 ms"
			s_serverSleepSecs -= timeElapsed;
			if (s_serverSleepSecs > 0.0f)
			{
				dsi_ServerSleep(1);
				timeElapsed = (time_GetTime() - timeStart);
				if (timeElapsed == 0.0f)
					timeElapsed = 0.001f; // always "at least 1 ms"
				s_serverSleepSecs -= timeElapsed;
			}
		}
		else
			break;
	}
	s_serverSleepSecs = 0.0f; // reset
#else
	
	if (ProcessIncomingPackets() != LT_OK)
		return false;

#endif // if DE_SERVER_COMPILE

	if (m_State == SERV_RUNNINGWORLD)
	{
		if (updateFlags & UPDATEFLAG_NONACTIVE || m_ServerFlags & SS_PAUSED)
		{
			// Alrighty... catchup the clock counters.  Don't catchup
			// the game time.. it just sits there.
			m_nTrueFrameTimeMS = 0;
			m_FrameTime = 0.0f;

			// Update the clients getting in...
			sm_UpdateClientStates(); // accesses m_TrueFrameTime

            // Update the server shell.
            if (i_server_shell != NULL) 
			{
                i_server_shell->Update( 0.0f );
            }

	 		#ifdef DE_SERVER_COMPILE
			if (g_LockServerFPS)
				dsi_ServerSleep((uint32)((1000 >> 1) / g_ServerFPS)); // half a cycle
  			#endif // DE_SERVER_COMPILE
		}
		else
		{
			// Do game time steps.
			if (g_ServerFPS != m_LastServerFPS)
			{
				#ifdef DE_SERVER_COMPILE
				m_TargetTimeBase = curTime; 
				m_nTargetTimeSteps = 0;
				#endif // DE_SERVER_COMPILE
			}
	
			m_FrameTime = ((LTCLAMP(m_nTrueFrameTimeMS / 1000.0f, 0.0f, 0.2f)) * g_ServerTimeScale);
			m_GameTime += m_FrameTime;

			
			// Update the sounds the server controls...
			// MAG - 2/14/02 - use true frame time
			// to keep client and server sound calcs in sync
			UpdateSounds(m_nTrueFrameTimeMS / 1000.0f);

			// Update the server shell.
			if (i_server_shell != NULL) {
				i_server_shell->Update(m_FrameTime);
			}

			// Update the objects.
			PreUpdateObjects();

			m_nTrueFrameTimeMS = 0; // Reset 

  			#ifdef DE_SERVER_COMPILE
			if (g_LockServerFPS)
			{
				float targetTime = m_TargetTimeBase + (float)m_nTargetTimeSteps / g_ServerFPS;
				float timeDiff = (targetTime - curTime);
				if (timeDiff > 0.0f)
				{
					if (timeDiff > (1.0f / (3.0f * g_ServerFPS))) // we're ahead of where we need to be and need to wait...
					{
 
						//DebugOut("diff=%.3f, targBase=%3.f, curTime=%.3f, targ=%.3f, catchup=%d\n", timeDiff, m_TargetTimeBase, curTime, targetTime, (int)m_CatchupSteps);
						s_serverSleepSecs = timeDiff;

					}
				}
			}
			m_nTargetTimeSteps += 1;
  			#endif // DE_SERVER_COMPILE

		}

		// Finish the frame.
		sm_FinishUpdateFrame();

		// This needs to get called after it updates the clients, becuase it may need
		// to end a looping sound before it removes it from the client.
		RemoveSounds();
	}
	else
	{
  		#ifdef DE_SERVER_COMPILE
		if (g_LockServerFPS)
			dsi_ServerSleep((uint32)((1000 >> 1) / g_ServerFPS)); // have a cycle
  		#endif // DE_SERVER_COMPILE
	}

	if (g_CV_ShowGameTime)
	{
		dsi_ConsolePrint("Game time: %.2f", m_GameTime);
	}

	if (g_CV_ShowSphereFindTicks)
	{
		dsi_ConsolePrint("ILTServer::FindObjectsTouchingSphere ticks: %d", g_SphereFindTicks);
		dsi_ConsolePrint("ILTServer::FindObjectsTouchingSphere count: %d", g_SphereFindCount);
	}

	return true;
}

void CServerMgr::GetErrorString(char *pStr, int32 maxLen)
{
	LTStrCpy(pStr, m_ErrorString, maxLen);
}


LPBASECLASS CServerMgr::EZCreateObject(CClassData *pClass, ObjectCreateStruct *pStruct)
{
	// Create and construct the object.
	LPBASECLASS pBaseClass = sm_AllocateObjectOfClass(pClass->m_pClass);

	if (!pStruct)
	{
		ASSERT((pClass->m_pClass->m_ClassFlags & CF_CLASSONLY) != 0);
		return pBaseClass;
	}

	// Point at the OCS
	ObjectCreateStruct *pOldOCS = g_pServerMgr->m_pCurOCS;
	m_pCurOCS = pStruct;

	// PostPropRead.
	pStruct->m_hClass = (HCLASS)pClass;
	uint32 nPreCreateResult = pBaseClass->OnPrecreate(pStruct, PRECREATE_NORMAL);

	// We're done with the OCS now
	m_pCurOCS = pOldOCS;

	LTRESULT nAddObjectResult;

	if (nPreCreateResult)
	{
		// Class-only objects are done, at this point.
		if ((pClass->m_pClass->m_ClassFlags & CF_CLASSONLY) != 0)
			return pBaseClass;

		LTObject *pObject;
		nAddObjectResult = sm_AddObjectToWorld(pBaseClass, pClass->m_pClass, pStruct,
			INVALID_OBJECTID, OBJECTCREATED_NORMAL, &pObject);
	}

	if ((nPreCreateResult) && (nAddObjectResult == LT_OK))
	{
		// Send the MID_ALLOBJECTSCREATED for consistancy with statically
		// created objects.
		//pBaseClass->EngineMessageFn(MID_ALLOBJECTSCREATED, LTNULL, 0);
		pBaseClass->OnAllObjectsCreated();

		return pBaseClass;
	}
	else
	{
		sm_FreeObjectOfClass(pClass->m_pClass, pBaseClass);
		return LTNULL;
	}
}



void CServerMgr::InternalResetFlowControl()
{
	ASSERT(m_bNeedsFlowControlReset);

	m_nCurBandwidthTarget = g_CV_BandwidthTargetServer;

	// Sort the clients by their desired bandwidth
	// Note : I'd prefer to leave the clients in their original order
	bool bHasLocalClient = false;

	LTLink *pListHead = &m_Clients.m_Head;

	LTLink *pCur;
	for (pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client *pCurClient = (Client*)pCur->m_pData;
		for (LTLink *pCompare = pCur->m_pNext; (pCompare != pListHead); pCompare = pCompare->m_pNext)
		{
			Client *pCompareClient = (Client *)pCompare->m_pData;
			if (pCompareClient->m_nDesiredReceiveBandwidth < pCurClient->m_nDesiredReceiveBandwidth)
			{
				// Swap the clients
				if (pCur->m_pNext == pCompare)
				{
					pCur->m_pPrev->m_pNext = pCompare;
					pCompare->m_pNext->m_pPrev = pCur;
					pCur->m_pNext = pCompare->m_pNext;
					pCompare->m_pNext = pCur;
					pCompare->m_pPrev = pCur->m_pPrev;
					pCur->m_pPrev = pCompare;
				}
				else
				{
					pCur->m_pNext->m_pPrev = pCompare;
					pCur->m_pPrev->m_pNext = pCompare;
					pCompare->m_pNext->m_pPrev = pCur;
					pCompare->m_pPrev->m_pNext = pCur;
					std::swap(pCur->m_pNext, pCompare->m_pNext);
					std::swap(pCur->m_pPrev, pCompare->m_pPrev);
				}

				pCurClient = pCompareClient;
				std::swap(pCompare, pCur);
			}
		}
		// Remember that we have a local client
		if (pCurClient->m_ClientFlags & CFLAG_LOCAL)
			bHasLocalClient = true;
	}

	// Partition out the amount remaining to the non-local clients
	uint32 nClientsToAdjust = m_Clients.m_nElements;
	if (bHasLocalClient)
		--nClientsToAdjust;
	uint32 nRemainingBandwidth = m_nCurBandwidthTarget;

	for (pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client *pCurClient = (Client*)pCur->m_pData;

		// Skip local clients
		if (pCurClient->m_ClientFlags & CFLAG_LOCAL)
			continue;

		uint32 nFairBandwidth = nRemainingBandwidth / nClientsToAdjust;
		uint32 nClientBandwidth = LTMIN(pCurClient->m_nDesiredReceiveBandwidth, nFairBandwidth);
		if (!nClientBandwidth)
			nClientBandwidth = nFairBandwidth;
		pCurClient->m_ConnectionID->SetBandwidth(nClientBandwidth);
		--nClientsToAdjust;

		ASSERT(nClientBandwidth <= nRemainingBandwidth);
		nRemainingBandwidth -= nClientBandwidth;
	}
	ASSERT(nClientsToAdjust == 0);

	m_bNeedsFlowControlReset = false;
}

void CServerMgr::SendFileIOMessage(uint8 fileType, uint8 msgID, uint16 fileID) 
{ 
	// Tell the clients to do the same.
	LTLink *pListHead = &m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client *pClient = (Client*)pCur->m_pData;

		// (local clients will just inherit the model)
		//if (!(pClient->m_ClientFlags & CFLAG_LOCAL))
		{
			CPacket_Write cPacket;
			cPacket.Writeuint8(msgID);
			cPacket.Writeuint8(fileType);
			cPacket.Writeuint16(fileID);
			::SendToClient(pClient, CPacket_Read(cPacket), false, MESSAGE_GUARANTEED);
		}
	}
}

// ------------------------------------------------------------------------
// ServerLoadChildModelCB
// ------------------------------------------------------------------------
LTRESULT ServerLoadChildModelCB(ModelLoadRequest *pRequest, Model **ppModel)
{
	const int kFilenameLen = _MAX_PATH + 1;
	char basePath[ kFilenameLen ], newFilename[ kFilenameLen ];
	char Filename [ kFilenameLen ];

	LTRESULT dResult;

	char *pBaseFilename = (char*)pRequest->m_pLoadFnUserData;

	*ppModel = LTNULL;
	
	// Add the filename to the previous path.
	CHelpers::ExtractNames(pBaseFilename, basePath, LTNULL, LTNULL, LTNULL);
	if(basePath[0] == 0)
		LTSNPrintF(Filename, kFilenameLen, "%s", pRequest->m_pFilename);
	else
		LTSNPrintF(Filename, kFilenameLen, "%s\\%s", basePath, pRequest->m_pFilename);

	CHelpers::FormatFilename( Filename, newFilename, kFilenameLen );

	dResult = g_pServerMgr->LoadModel(newFilename, *ppModel);

	return dResult ;
}

// ------------------------------------------------------------------------
// CacheModelFile( filename )
// if the file isn't already cached,
// try to load it, once loaded, tag the file as cached, add ref it.
// the tell the client what you have done... 
// ------------------------------------------------------------------------
LTRESULT CServerMgr::CacheModelFile(const char * pFilename)
{
	if (IsModelCached(pFilename))
	{
		DEBUG_MODEL_REZ(("model-rez: server cachemodelfile %s already cached", pFilename)); 
		return LT_OK;
	}

	Model *pModel ; // new model

	// load the file if it isn't cached
	if (LoadModel(pFilename, pModel) != LT_OK)
	{
		DEBUG_MODEL_REZ(("model-rez: server cachemodelfile could not load %s", pFilename )) ;
		return LT_ERROR ;
	}

	pModel->AddRef();
	pModel->m_Flags |= MODELFLAG_CACHED;

	// tell clients to cache this file.
	LTLink *pListHead =	&m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext) 
	{
		SendLoadRequest(pModel,(Client*)pCur->m_pData );
	}	

	return LT_OK;
}

// ------------------------------------------------------------------------
// UncacheModelFile( pModel )
// 
// ------------------------------------------------------------------------
LTRESULT CServerMgr::UncacheModelFile( Model *pModel )
{
	if (!pModel || ((pModel->m_Flags & MODELFLAG_CACHED ) == 0))
		return LT_ERROR;

	DEBUG_MODEL_REZ(("model-rez: server uncachemodelfile %s ", pModel->GetFilename() ) );

	pModel->m_Flags &= ~MODELFLAG_CACHED ;
	pModel->Release();
	// tell the client(s)
	SendFileIOMessage(FT_MODEL,SMSG_UNLOAD ,pModel->m_FileID);
	return LT_OK;
}

// ------------------------------------------------------------------------
// LoadModel( filename, return_model_pointer )
//
// returns LT_OK on sucessful load of model,
// Some other value when the model failed to load depending on nature of error.
// ------------------------------------------------------------------------
LTRESULT CServerMgr::LoadModel(const char* pFilename, 
							   Model*&	   pRetModel)		// return model value
								
{
	LTRESULT dResult = LT_ERROR ;
	Model *pModel = NULL;

	pModel = g_ModelMgr.Find( pFilename );

	 // create a new one from the file sys if none were found.
	if (pModel == NULL)
	{
		UsedFile *pUsedFile = LTNULL;
	
		if (server_filemgr->AddUsedFile(pFilename, 0, &pUsedFile) == 0)
		{
			RETURN_ERROR_PARAM(1, se_LoadModel, LT_MISSINGFILE, pFilename);
		}

		ModelLoadRequest request;

		request.m_pFile = server_filemgr->OpenFile3(pUsedFile);
		if (!request.m_pFile)
		{
			DEBUG_PRINT(1, ("Couldn't open model file %s", pFilename));
			RETURN_ERROR_PARAM(1, CServerMgr_LoadModel, LT_MISSINGFILE, pFilename);
		}

		// Create the new model
		LT_MEM_TRACK_ALLOC(pModel = new Model(),LT_MEM_TYPE_MISC);
		if (!pModel)
		{
			request.m_pFile->Release();
			RETURN_ERROR(1, CServerMgr_LoadModel, LT_OUTOFMEMORY);
		}
		
		// set up the parent model for loading its child.
		request.m_LoadChildFn = ServerLoadChildModelCB;
		request.m_pLoadFnUserData = (void*)(pFilename);

		dResult = pModel->Load(&request, pFilename);


		request.m_pFile->Release();

		// Make sure everything is dandy.
		if (dResult != LT_OK )
		{
			delete pModel;
			DEBUG_PRINT(1, ("Error %d loading model %s", dResult, pFilename));
			return dResult;
		}
		else if (!request.m_bTreesValid)
		{
			delete pModel;
			DEBUG_PRINT(1, ("Child model trees invalid in model %s", pFilename));
			RETURN_ERROR(1, CServerMgr_LoadModel, LT_INVALIDMODELFILE);
		}
		else if (!request.m_bAllChildrenLoaded)
		{
			delete pModel;
			DEBUG_PRINT(1, ("Missing one or more child models in model %s", pFilename));
			RETURN_ERROR(1, CServerMgr_LoadModel, LT_INVALIDMODELFILE);
		}

		// connect the model database to its file.
		pModel->m_FileID = pUsedFile->m_FileID;
		pUsedFile->m_Data = pModel ; // 	
			
		DEBUG_MODEL_REZ(("model-rez: server loadmodel [%d] %s ", pModel, pFilename ));
	}
	
	// We already have this model loaded... 
	if (pModel)
	{	
		UsedFile *pUsedFile ;
		// check if the model's file id is valid.
		// if it isn't then get a new file handle, and reset the id.
		if (server_filemgr->GetAddedUsedFile(pModel->GetFilename(),&pUsedFile) == false)
		{
			/* NOTE: 
			This case happens when the client loads a model and then the server loads the same
			model. pModel points to technically the same model, but since the client loaded it first
			its invalid.

			An other case is when an object is not deleted at level end, thus keeping the model around
			but invalidating the file id. 

			Either way the file id has to be fixed and this does it.
			*/
			uint16 old_fileid = pModel->m_FileID;
		
			// try to fix the error...
			if (server_filemgr->AddUsedFile(pModel->GetFilename(), 0, &pUsedFile) == 0)
			{
				RETURN_ERROR_PARAM(1, se_LoadModel, LT_MISSINGFILE, pFilename);
			}

			DEBUG_MODEL_REZ(("model-rez: server loadmodel warning model with inappropriate file id (old %d new %d) [%s] " , 
							 old_fileid, pModel->m_FileID,
							 pModel->GetFilename() ));

		}
			
		pModel->m_FileID = pUsedFile->m_FileID;
		pUsedFile->m_Data = pModel ; // 

		// set return values
		pRetModel = pModel;
		dResult = LT_OK ;	
	}
	else 
	{	
		DEBUG_MODEL_REZ(("model-rez: server loadmodel failed to load %s ", pFilename ));
	}
	
	
	return dResult ;
}


bool CServerMgr::IsModelCached(const char *pFilename)
{
	Model *pModel = g_ModelMgr.Find(pFilename);

	if (pModel)
	{
		if (pModel->m_Flags & MODELFLAG_CACHED)
			return true ;
	}
	
	return false ;
}

void CServerMgr::UncacheModels()
{
	g_ModelMgr.UncacheServerModels( );
}

void SendPreloadMsgToClientCB(const Model &model, void *user_data)
{
	Client *pClient = (Client *)user_data;

	// don't transmit out of range/invalid/clientonly models.
	if (model.m_FileID >= 0xffff) 
		return;

	// tell client if incomming file is cached. 
	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	if (model.m_Flags & MODELFLAG_CACHED)
	{
		cPacket.Writeuint8(PRELOADTYPE_MODEL_CACHED);
	}
	else
	{
		cPacket.Writeuint8(PRELOADTYPE_MODEL);
	}
		
	DEBUG_MODEL_REZ(("model-rez: send preload req fileid(%d) %s   "  , model.m_FileID , model.GetFilename()));

	// tell client the file-id
	cPacket.Writeuint16((uint16)model.m_FileID);
	::SendToClient(pClient, CPacket_Read(cPacket), false);
}


// ------------------------------------------------------------------------
// SendPreloadModelMsgToClient( a-client )
// 
// Tell the client to load model files. Some of these files are tagged as
// cached others are not.
// ------------------------------------------------------------------------
void CServerMgr::SendPreloadModelMsgToClient(Client *pClient)
{
	CPacket_Write cPacket;

	//// Mark the start of the list.
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	cPacket.Writeuint8(PRELOADTYPE_START);
	SendToClient(pClient, CPacket_Read(cPacket), false);
	
	g_ModelMgr.ForEach(SendPreloadMsgToClientCB, pClient) ;

	// send terminator
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	cPacket.Writeuint8(PRELOADTYPE_END);
	SendToClient(pClient, CPacket_Read(cPacket), false);
}

// ------------------------------------------------------------------------
// SendChangeChildModel( object, add_flag, file_id )
// ------------------------------------------------------------------------
void CServerMgr::SendChangeChildModel(uint16 parent_fid, uint16 child_fid)
{
	LTLink *pListHead = &m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{   
		Client *pClient = (Client*)pCur->m_pData;

		CPacket_Write cPacket;
	
		// pack it
		cPacket.Writeuint8(SMSG_CHANGE_CHILDMODEL);
		cPacket.Writeuint16(parent_fid);
		cPacket.Writeuint16(child_fid);

		// send it
		SendToClient(pClient, CPacket_Read(cPacket), false);
	}
}

// ----------------------------------------------------------------------- //
//	  ROUTINE:		Internal stuff.
//	  PURPOSE:
// ----------------------------------------------------------------------- //
void CServerMgr::ResizeUpdateInfos(uint32 nAllocatedIDs)
{
	LTLink *pListHead = &m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client *pClient = (Client*)pCur->m_pData;

		ObjInfo *pNew;
		LT_MEM_TRACK_ALLOC(pNew = (ObjInfo*)dalloc(sizeof(ObjInfo) * (nAllocatedIDs + UPDATEINFO_CACHESIZE)),LT_MEM_TYPE_MISC);

		memcpy(pNew, pClient->m_ObjInfos, sizeof(ObjInfo)*m_nObjInfos);
		memset(&pNew[m_nObjInfos], 0, sizeof(ObjInfo)*UPDATEINFO_CACHESIZE);	// Clear all the new object infos.

		dfree(pClient->m_ObjInfos);
		pClient->m_ObjInfos = pNew;
	}

	m_nObjInfos = nAllocatedIDs + UPDATEINFO_CACHESIZE;
}


// ----------------------------------------------------------------------- //
// Creates and initializes a SObjData.
// ----------------------------------------------------------------------- //

LTRESULT sm_CreateServerData(ObjectCreateStruct *pStruct,
							 ClassDef *pClass, 
							 LTObject *pObject, 
							 LPBASECLASS pBaseClass, 
							 SObjData **ppData)
{
	SObjData *pRet;

	*ppData = LTNULL;

    LT_MEM_TRACK_ALLOC(pRet = g_pServerMgr->m_SObjBank.Allocate(), LT_MEM_TYPE_MISC);
	if (!pRet)
	{
		RETURN_ERROR(1, sm_CreateServerData, LT_TRIEDTOREMOVECLIENTOBJECT);
	}

	// Init all the data.
	pRet->m_pObject = LTNULL;
	pRet->m_pSkin = LTNULL;
	pRet->m_pFile = LTNULL;
	pRet->m_pClass = pClass;
	pRet->m_pClient = LTNULL;
	pRet->m_NextUpdate = pStruct->m_NextUpdate;
	pRet->m_cSpecialEffectMsg.Clear();
	pRet->m_pIDLink = LTNULL;
	pRet->m_ChangeFlags = 0;
	dl_TieOff(&pRet->m_ChangedNode);
	pRet->m_NetFlags = 0;

	// Add its name to the hash table.
	if (pStruct->m_Name[0] == 0)
	{
		// Don't waste memory for most of the objects..
		pRet->m_hName = LTNULL;
	}
	else
	{
		pRet->m_hName = LTNULL;
		pRet->m_hName = hs_AddElement(g_pServerMgr->m_hNameTable, pStruct->m_Name, strlen(pStruct->m_Name)+1);
		hs_SetElementUserData(pRet->m_hName, pObject);
	}

	dl_TieOff(&pRet->m_Links);

	dl_AddHead(&g_pServerMgr->m_Objects, &pRet->m_ListNode, pObject);

	*ppData = pRet;
	return LT_OK;
}


LTRESULT sm_DestroyServerData(LTObject *pObject)
{
	ClientRef *pClientRef;

	// Free its script binding.
	sm_FreeObjectScript(pObject);

	// Get rid of its alpha list stuff.
	if (pObject->sd->m_hName)
	{
		hs_RemoveElement(g_pServerMgr->m_hNameTable, pObject->sd->m_hName);
	}

	// If it has a client ref, get rid of it.
	if (pObject->m_InternalFlags & IFLAG_HASCLIENTREF)
	{
		pClientRef = sm_FindClientRefFromObject(pObject);
		if (pClientRef)
		{
			dl_RemoveAt(&g_pServerMgr->m_ClientReferences, &pClientRef->m_Link);
			dfree(pClientRef);
		}
	}

    // Clear the change flags on the modified objects.
	if (IsObjectInChangedList(pObject))
		dl_RemoveAt(&g_pServerMgr->m_ChangedObjectHead, &pObject->sd->m_ChangedNode);

	// Break all links.
	BreakInterLinks(pObject, LINKTYPE_INTERLINK, true);
	BreakInterLinks(pObject, LINKTYPE_CONTAINER, false);
	BreakInterLinks(pObject, LINKTYPE_SOUND, false);

	dl_RemoveAt(&g_pServerMgr->m_Objects, &pObject->sd->m_ListNode);
	g_pServerMgr->m_SObjBank.Free(pObject->sd);
	return LT_OK;
}




// ----------------------------------------------------------------------- //
// Asks the server shell what files it wants to have loaded and caches out
// files that aren't needed.
// ----------------------------------------------------------------------- //
LTRESULT sm_StartCachingFiles()
{
	// Clear our 'wanted sprite' list.
	g_pServerMgr->m_CacheListSize = 0;

	// Clear all sounds as not needed.
	g_pServerMgr->UntouchAllSoundData();

	// Ask the shell what it wants to have loaded.
	g_pServerMgr->m_InternalFlags |= SFLAG_BUILDINGCACHELIST;

	return LT_OK;
}


LTRESULT sm_EndCachingFiles()
{
	i_server_shell->CacheFiles();
	g_pServerMgr->m_InternalFlags &= ~SFLAG_BUILDINGCACHELIST;

	sm_RemoveAllUnusedSoundData();

	return LT_OK;
}


LTRESULT sm_CacheEasyStuff()
{
	LTLink *pListHead, *pCur;
	LTObject *pObject;

	// Cache models.
	pListHead = &g_pServerMgr->m_ObjectMgr.m_ObjectLists[OT_MODEL].m_Head;
	for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
	{
		pObject = (LTObject*)pCur->m_pData;

		if (ToModel(pObject)->GetModelDB())
		{
			if (pObject->sd->m_pSkin)
				sm_CacheFile(FT_TEXTURE, server_filemgr->GetUsedFilename(pObject->sd->m_pSkin));
		}
	}

	// Cache sprites.
	pListHead = &g_pServerMgr->m_ObjectMgr.m_ObjectLists[OT_SPRITE].m_Head;
	for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
	{
		pObject = (LTObject*)pCur->m_pData;

		if (pObject->sd->m_pFile)
			sm_CacheFile(FT_SPRITE, server_filemgr->GetUsedFilename(pObject->sd->m_pFile));
	}

	return LT_OK;
}

bool CServerMgr::CreateVisContainerObjects() 
{
	// Hmmm... kinda messy..
	CClassData *pClass = m_ClassMgr.FindClassData("MainWorld");
	if (!pClass)
		return false;
 
	for (uint32 i=0; i < world_bsp_server->NumWorldModels(); i++)
	{
		const WorldData *pWorldData;
		pWorldData = world_bsp_server->GetWorldModel(i);

		if ((pWorldData->OriginalBSP()->m_WorldInfoFlags & (WIF_PHYSICSBSP | WIF_VISBSP)) == 0)
			continue;

		ObjectCreateStruct ocs;
		// Ok, make an object for it.
		ocs.Clear();
		ocs.m_ObjectType = OT_WORLDMODEL;
		ocs.m_Flags = FLAG_SOLID | FLAG_RAYHIT;
		SAFE_STRCPY(ocs.m_Filename, pWorldData->m_pValidBsp->m_WorldName);

		if (!EZCreateObject(pClass, &ocs))
		{
			return false;
		}
	}

	return true;
}


CBaseDriver* CServerMgr::GetLocalDriver()
{
	for (uint32 i = 0; i < m_NetMgr.m_Drivers; i++)
	{
		if (strcmp(m_NetMgr.m_Drivers[i]->m_Name, "local") == 0)
			return m_NetMgr.m_Drivers[i];
	}

	return LTNULL;
}


void CServerMgr::InternalGetPlayerName(Client *pStruct, char *pStr, int32 nMaxLen)
{
	LTStrCpy(pStr, pStruct->m_Name, nMaxLen);
}


void CServerMgr::InternalSetPlayerName(Client *pStruct, const char *pStr, int32 nMaxLen)
{
	//
	//free it if it's already allocated
	if(pStruct->m_Name)
	{
		dfree(pStruct->m_Name);
		pStruct->m_Name	= 0;
	}

	//
	//allocate anew.
	LT_MEM_TRACK_ALLOC(pStruct->m_Name	= (char*)dalloc(nMaxLen + 1),LT_MEM_TYPE_MISC);
	if(!pStruct->m_Name)
		return;				//m_Name will be NULL if failed

	LTStrCpy(pStruct->m_Name, pStr, nMaxLen);
}


//------------------------------------------------------------------------------------
//
// CServerMgr::FindSoundData
//
// Finds sound data object if already loaded.
//
//------------------------------------------------------------------------------------
CSoundData *CServerMgr::FindSoundData(UsedFile *pFile)
{
	for (LTLink *pCur = m_SoundDataList.m_Head.m_pNext; pCur != &m_SoundDataList.m_Head; pCur = pCur->m_pNext)
	{
		CSoundData *pTest = (CSoundData*)pCur->m_pData;

		if (pTest->GetFile() == pFile)
			return pTest;
	}

	return LTNULL;
}

//------------------------------------------------------------------------------------
//
// CServerMgr::GetSoundData
//
// Reads in sound data.
//
//------------------------------------------------------------------------------------
CSoundData *CServerMgr::GetSoundData(UsedFile *pFile)
{
	if (!pFile)
		return LTNULL;

	// Check if file data already read...
	CSoundData *pSoundData = FindSoundData(pFile);
	if (pSoundData)
		return pSoundData;

	const char *pszFileName = server_filemgr->GetUsedFilename(pFile);

	// Read in the file data...
	ILTStream *pStream = server_filemgr->OpenFile(pszFileName);
	if (!pStream)
		return LTNULL;

	pSoundData = (CSoundData *)sb_Allocate(&m_SoundDataBank);
	bool bResult = (pSoundData->Init(pFile, pStream, pFile->m_FileSize) != 0);
	// Close the file.
	pStream->Release();

	if (!bResult)
	{
		sb_Free(&m_SoundDataBank, pSoundData);
		return LTNULL;
	}

	// Add it to sound data list...
	dl_AddHead(&m_SoundDataList, &pSoundData->m_Link, pSoundData);

	return pSoundData;
}

// ------------------------------------------------------------------------------------ //
// C-style server functions.
// ------------------------------------------------------------------------------------ //

void BPrint(const char *pMsg, ...)
{
	if (!g_pServerMgr)
		return;

	static const uint32 knBufferSize = 512;
	char str[knBufferSize];
	va_list marker;

	va_start(marker, pMsg);
	LTVSNPrintF(str, knBufferSize, pMsg, marker);
	va_end(marker);

	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_MESSAGE);
	cPacket.Writeuint8(STC_BPRINT);
	cPacket.WriteString(str);
	SendServerMessage(CPacket_Read(cPacket));
}


LTRESULT sm_CreateNewID(LTLink **pID)
{
	if (g_pServerMgr->m_nAllocatedIDs + 1 > ID_MAX)
		RETURN_ERROR(1, sm_CreateNewID, LT_ERROR);

	*pID = LTNULL;
	LTLink *pRet;
	LT_MEM_TRACK_ALLOC(pRet = g_DLinkBank.Allocate(), LT_MEM_TYPE_MISC);

	pRet->m_pData = (void*)g_pServerMgr->m_nAllocatedIDs;
	++g_pServerMgr->m_nAllocatedIDs;

	dl_Insert(&g_pServerMgr->m_FreeIDs, pRet);

	// Make sure the object map has space.
	LTRecord dRecord;
	dRecord.m_nRecordType = 0;
	dRecord.m_pRecordData = LTNULL;
    LT_MEM_TRACK_ALLOC(g_pServerMgr->m_ObjectMap.Append(dRecord), LT_MEM_TYPE_MISC);

	*pID = pRet;
	return LT_OK;
}


LTRESULT sm_SetupError(LTRESULT theError, ...)
{
	va_list marker;

	va_start(marker, theError);
	LTRESULT dResult = dsi_SetupMessage(
		g_pServerMgr->m_ErrorString, sizeof(g_pServerMgr->m_ErrorString)-1, theError, marker);
	va_end(marker);

	g_pServerMgr->m_LastErrorCode = theError;
	return dResult;
}

void CServerMgr::UntouchAllSoundData()
{
	// Uninit sound stuff.
	LTLink *pCur = m_SoundDataList.m_Head.m_pNext;
	while (pCur != &m_SoundDataList.m_Head)
	{
		LTLink *pNext = pCur->m_pNext;
		CSoundData *pSoundData = (CSoundData *)pCur->m_pData;
		pSoundData->SetTouched(false);
		pCur = pNext;
	}
}



void sm_RemoveAllSounds()
{
	// Remove sound instances.
	sm_RemoveAllSoundInstances();

	// Uninit sound stuff.
	LTLink *pCur = g_pServerMgr->m_SoundDataList.m_Head.m_pNext;
	while (pCur != &g_pServerMgr->m_SoundDataList.m_Head)
	{
		LTLink *pNext = pCur->m_pNext;
		CSoundData *pSoundData = (CSoundData *)pCur->m_pData;
		sb_Free(&g_pServerMgr->m_SoundDataBank, pSoundData);
		pCur = pNext;
	}
	dl_InitList(&g_pServerMgr->m_SoundDataList);
}

void sm_RemoveAllUnusedSoundData()
{
	// Remove untouched sounddata
	LTLink *pCur = g_pServerMgr->m_SoundDataList.m_Head.m_pNext;
	while (pCur != &g_pServerMgr->m_SoundDataList.m_Head)
	{
		CSoundData *pSoundData = (CSoundData *)pCur->m_pData;
		pCur = pCur->m_pNext;
		if (pSoundData->IsTouched())
			continue;

		dl_RemoveAt(&g_pServerMgr->m_SoundDataList, &pSoundData->m_Link);
		pSoundData->Term();
		sb_Free(&g_pServerMgr->m_SoundDataBank, pSoundData);
	}
}


void sm_RemoveAllUnusedSoundTracks()
{
	// Remove any sound instances that aren't being held onto by handles
	LTLink *pCurInstance = g_pServerMgr->m_SoundTrackList.m_Head.m_pNext;
	while (pCurInstance != &g_pServerMgr->m_SoundTrackList.m_Head)
	{
		CSoundTrack *pSoundInstance = (CSoundTrack *)pCurInstance->m_pData;
		pCurInstance = pCurInstance->m_pNext;

		// Don't remove instances that have handles on them, unless told to...
		if (pSoundInstance->m_dwFlags & PLAYSOUND_GETHANDLE) {
			if (g_DebugLevel >= 3) {
				if (pSoundInstance->m_pFile) {
					dsi_ConsolePrint("Unfreed sound file (server) %s", server_filemgr->GetUsedFilename(pSoundInstance->m_pFile));
				}
			}
		}

		// Remove the instance...
		dl_RemoveAt(&g_pServerMgr->m_SoundTrackList, &pSoundInstance->m_Link);
		pSoundInstance->Term();
		sb_Free(&g_pServerMgr->m_SoundTrackBank, pSoundInstance);
	}
}

void sm_RemoveAllSoundInstances()
{
	LTLink *pCur = g_pServerMgr->m_SoundTrackList.m_Head.m_pNext;
	while (pCur != &g_pServerMgr->m_SoundTrackList.m_Head)
	{
		LTLink *pNext = pCur->m_pNext;
		CSoundTrack *pSoundTrack = (CSoundTrack *)pCur->m_pData;
		pSoundTrack->Term();
		sb_Free(&g_pServerMgr->m_SoundTrackBank, pSoundTrack);
		pCur = pNext;
	}
	dl_InitList(&g_pServerMgr->m_SoundTrackList);
}

// sm_SendToVisibleClients
//
// Sends packet to client objects in vislist based on pvPos.  If this is a single player local game, then
// the packet is sent regardless.
LTRESULT sm_SendToVisibleClients(const CPacket_Read &cPacket,
								 bool   bSendToAttachments, 
								 const LTVector &vPos, 
								 uint32   messageFlags)
{
	// Handle no clients...
	if (g_pServerMgr->m_Clients.m_nElements == 0) 
	{
		return LT_OK;
	}

	//we don't have any visibility anymore, so simply iterate through all clients and send it
	//to them
	LTLink* pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink* pCur=pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client* pClient = (Client*)pCur->m_pData;
		SendToClient(pClient, cPacket, true, messageFlags);
	}

	return LT_OK;
}

// ------------------------------------------------------------------------
// This only applies to non models now...
// ------------------------------------------------------------------------
void sm_CacheSingleFile(uint16 fileType, uint16 fileID) 
{
	// Don't send duplicates..
	for (uint32 i = 0; i < g_pServerMgr->m_CacheListSize; i++) 
	{
		if (g_pServerMgr->m_CacheList[i].m_FileID == fileID) 
		{
			return;
		}
	}

	// Expand the list if necessary
	if (g_pServerMgr->m_CacheListSize == g_pServerMgr->m_CacheListAllocedSize) 
	{
		uint32 newSize = g_pServerMgr->m_CacheListAllocedSize + 20;
		OtherFile *pNew;
		LT_MEM_TRACK_ALLOC(pNew = (OtherFile*)dalloc(sizeof(OtherFile) * newSize),LT_MEM_TYPE_MISC);
		memcpy(pNew, g_pServerMgr->m_CacheList, sizeof(OtherFile) * g_pServerMgr->m_CacheListAllocedSize);
		dfree(g_pServerMgr->m_CacheList);
		g_pServerMgr->m_CacheList = pNew;
		g_pServerMgr->m_CacheListAllocedSize = newSize;
	}

	// Update the list
	g_pServerMgr->m_CacheList[g_pServerMgr->m_CacheListSize].m_FileID = fileID;
	g_pServerMgr->m_CacheList[g_pServerMgr->m_CacheListSize].m_FileType = fileType;
	g_pServerMgr->m_CacheListSize++;

	// Send a single update packet if we're not building a cache list
	if ((g_pServerMgr->m_InternalFlags & SFLAG_BUILDINGCACHELIST) == 0) 
	{
		LTLink *pCur, *pListHead;

		pListHead = &g_pServerMgr->m_Clients.m_Head;
		for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) 
		{
			sm_SendCacheListToClient((Client*)pCur->m_pData, g_pServerMgr->m_CacheListSize - 1);
		}
	}
}

LTRESULT sm_CacheFile(uint32 fileType, const char *pFilename) 
{
	UsedFile *pFile;

	if (!pFilename || !*pFilename) 
	{
		RETURN_ERROR(1, ILTServer::CacheFile, LT_ERROR);
	}

	char pszFinalFilename[_MAX_PATH + 1];
	CHelpers::FormatFilename(pFilename, pszFinalFilename, _MAX_PATH + 1);

	switch(fileType)
	{
		case FT_MODEL:
		{
			if (g_pServerMgr->CacheModelFile(pszFinalFilename) != LT_OK )
			{
				RETURN_ERROR_PARAM(1, sm_CacheFile, LT_MISSINGFILE, pszFinalFilename);
			}	
		}
		break;


		case FT_SPRITE:
		case FT_TEXTURE:
		{
			if (server_filemgr->AddUsedFile(pszFinalFilename, 0, &pFile) == 0) 
			{
				RETURN_ERROR_PARAM(1, ILTServer::CacheFile, LT_NOTFOUND, pszFinalFilename);
			}
			else if (pFile) 
			{
				sm_CacheSingleFile((uint16)fileType, (uint16)pFile->m_FileID);
			}
		}
		break;

 
		case FT_SOUND:
		{
			if (server_filemgr->AddUsedFile(pszFinalFilename, 0, &pFile) == 0) 
			{
				RETURN_ERROR_PARAM(1, ILTServer::CacheFile, LT_NOTFOUND, pszFinalFilename);
			}
            // Load up the sound and add it to our cache list
            else
			{
				// Keep a copy of the data on the server side...
				CSoundData *pSoundData = g_pServerMgr->GetSoundData(pFile);
				if (pSoundData) 
				{
					pSoundData->SetTouched(true);
				}

				// See if we are batching up the cache list or sending them down individually
				if ((g_pServerMgr->m_InternalFlags & SFLAG_BUILDINGCACHELIST) == 0) 
				{
					sm_CacheSingleFile((uint16)fileType, (uint16)pFile->m_FileID);
				}
			}
		}
		break;
	}


	return LT_OK;
}



void CServerMgr::SetGlobalLightObject(HOBJECT hObject) 
{
	// Store the object
	m_hGlobalLightObject = hObject;

	// Flag all of the clients as needing this information
	LTLink *pCur;
	for (pCur = m_Clients.m_Head.m_pNext; pCur != &m_Clients.m_Head; pCur = pCur->m_pNext)
	{
		((Client*)pCur->m_pData)->m_ClientFlags |= CFLAG_SENDGLOBALLIGHT;
	}
}

LTRESULT CServerMgr::LoadWorld(ILTStream* pStream, const char *pWorldName)
{
	world_bsp_server->Term();

	// Load the world geometry.
	pStream->SeekTo(0);
 
	int32 loadStatus = world_bsp_server->Load(pStream);

	// Invalid file version
	if (loadStatus != 0)
	{
		world_bsp_server->Term();

		//the stream has already been released
		pStream = NULL;

		sm_SetupError(LT_INVALIDWORLDFILE, pWorldName);
		RETURN_ERROR_PARAM(1, CServerMgr::LoadWorld, LT_INVALIDWORLDFILE, pWorldName);
	}


	m_bWorldLoaded = true;
	return LT_OK;
}

// ----------------------------------------------------------------------- //
// Sets up a new object in the world.
// ----------------------------------------------------------------------- //
 
LTRESULT sm_AddObjectToWorld( LPBASECLASS pBaseClass, ClassDef *pClass,
	 ObjectCreateStruct *pStruct, uint16 objectID, float initialUpdateReason, LTObject **ppOut)
{
	*ppOut = LTNULL;

	if (!g_pServerMgr->m_bWorldLoaded)
	{
		RETURN_ERROR(1, sm_AddObjectToWorld, LT_NOTINWORLD);
	}

	// Create the object.
	LTObject *pObject;
	LTRESULT dResult = om_CreateObject(&g_pServerMgr->m_ObjectMgr, pStruct, &pObject);
	if (dResult != LT_OK)
		return dResult;

	pBaseClass->m_hObject = ServerObjToHandle(pObject);

	dResult = sm_CreateServerData(pStruct, pClass, pObject, pBaseClass, &pObject->sd);
	if (dResult != LT_OK)
	{
		om_DestroyObject(&g_pServerMgr->m_ObjectMgr, pObject);
		return dResult;
	}

	//tell this object that it is active
	pBaseClass->OnActivate();

	// Init extra data (like model, worldmodel, sprite, etc.)
	dResult = sm_InitExtraData(pObject, pStruct);
	if (dResult != LT_OK)
	{
		sm_DestroyServerData(pObject);
		om_DestroyObject(&g_pServerMgr->m_ObjectMgr, pObject);
		pBaseClass->m_hObject = NULL;
		return dResult;
	}

	// Get an id...
	LTLink *pIDLink;
	dResult = sm_AllocateID(&pIDLink, objectID);
	if (dResult != LT_OK)
	{
		sm_DestroyServerData(pObject);
		om_DestroyObject(&g_pServerMgr->m_ObjectMgr, pObject);
		pBaseClass->m_hObject = NULL;
		return dResult;
	}

	pObject->m_InternalFlags |= IFLAG_INWORLD | IFLAG_APPLYPHYSICS;

	// Assign the id...
	pObject->sd->m_pIDLink = pIDLink;
	pObject->m_ObjectID = (uint16)GetLinkID(pIDLink);
	g_pServerMgr->m_ObjectMap[pObject->m_ObjectID].m_nRecordType = RECORDTYPE_LTOBJECT;
	g_pServerMgr->m_ObjectMap[pObject->m_ObjectID].m_pRecordData = pObject;

	// This should be last so if an error returns, the caller needs to FreeObjectOfClass.
	pObject->sd->m_pObject = pBaseClass;

	// Transform it if it's a Worldmodel.
	if (HasWorldModel(pObject))
	{
		InitialWorldModelRotate(ToWorldModel(pObject));
	}

	// Add it to the bsp...
	sm_UpdateInBspStatus(pObject);

	// Call its initial update!
//	pBaseClass->EngineMessageFn(MID_OBJECTCREATED, pStruct, initialUpdateReason);
	pBaseClass->OnObjectCreated( initialUpdateReason );

	// Force collisions to set up some links (Fixes container links)
	// We need to make a copy of our position before calling in instead of passing
	// the address of pObject->m_Pos.  If we don't sometimes objects will be moved
	// when the level loads causing the game to be unplayable.  This is an attempt
	// to fix the MOVING OBJECTS BUG.
	LTVector vPos = pObject->m_Pos;
	FullMoveObject(pObject, &vPos, MO_SETCHANGEFLAG|MO_TELEPORT);

	// Set its initial change flags..
	SetObjectChangeFlags(pObject, sm_GetNewObjectChangeFlags(pObject));

	g_ObjectMemory += sizeof(LTObject) + sizeof(SObjData);
	*ppOut = pObject;

	return LT_OK;

}


// ----------------------------------------------------------------------- //
// Removes the object from the world.  Assumes that the object has
// its ID set already.
// ----------------------------------------------------------------------- //

LTRESULT sm_RemoveObjectFromWorld( LPBASECLASS pBaseClass )
{
	LTObject *pObject = BaseClassToServerObj(pBaseClass);
	if (!pObject)
	{
		RETURN_ERROR(1, sm_RemoveObjectFromWorld, LT_ERROR);
	}

	// Notify the object's reference list
	pObject->NotifyObjRefList_Delete();	

	// Make sure it's not a client object.
	if (pObject->sd)
	{
		pObject->m_InternalFlags |= IFLAG_OBJECTGOINGAWAY;

		// Send a detach message to the child attachments.
		Attachment *pAttachment = pObject->m_Attachments;
		while (pAttachment)
		{
			LTObject *pChild = sm_FindObject(pAttachment->m_nChildID);
			if (pChild)
			{
				pChild->sd->m_pObject->OnParentAttachmentRemoved();
			}

			pAttachment = pAttachment->m_pNext;
		}

		// Tell clients to get it out of the sky if it's in there.
		if (pObject->m_InternalFlags & IFLAG_INSKY)
		{
			sm_SetSendSkyDef();
			sm_RemoveObjectFromSky(pObject);
			pObject->m_InternalFlags &= ~IFLAG_INSKY;
		}

		if (pObject->sd->m_pClient)
		{
			RETURN_ERROR(1, sm_RemoveObjectFromWorld, LT_TRIEDTOREMOVECLIENTOBJECT);
		}


		// [kls 6/16/00] BreakInterLinks gets called in sm_DestroyServerData
		// below, however in the case where this object is attached to
		// another object, and the other object needs to remove the attachment,
		// we need to call this BEFORE g_pServerMgr->m_ObjectMap is cleared....

		BreakInterLinks(pObject, LINKTYPE_INTERLINK, true);


		if (pObject->sd->m_pIDLink)
		{
			sm_FreeID(pObject->sd->m_pIDLink);

			// however, still set m_pRecordData to NULL for any lookups that might happen
			g_pServerMgr->m_ObjectMap[pObject->m_ObjectID].m_nRecordType = 0;
			g_pServerMgr->m_ObjectMap[pObject->m_ObjectID].m_pRecordData = LTNULL;
		}

		// Detach it from whatever it's standing on.
		DetachObjectStanding(pObject);
		DetachObjectsStandingOn(pObject);

		sm_DestroyServerData(pObject);
		sm_TermExtraData(pObject);
		pObject->sd = LTNULL;
	}

	// Free the LTObject.
	g_ObjectMemory -= sizeof(LTObject) + sizeof(SObjData);
	return om_DestroyObject(&g_pServerMgr->m_ObjectMgr, pObject);
}

LTRESULT CServerMgr::DoStartWorld(const char *pWorldName, uint32 flags, uint32 nCurTimeMS)
{
	// Grab the starting time
	clock_t StartTime = clock();
   
	bool bSwitchingWorlds, bSameWorld;
	ILTStream *pStream;
	char worldName[_MAX_PATH + 1], *pStr;
	LTRESULT dResult;
	UsedFile *pWorldFile;
 

	// Seed our random numbers..
	float fSeed = time_GetTime();
	srand((uint32)fSeed);
	i_server_shell->SRand((uint32)fSeed);

 
	bSameWorld = false;
	if (flags & LOADWORLD_NORELOADGEOMETRY)
	{
		if (m_pWorldFile)
		{
			LTStrCpy(worldName, server_filemgr->GetUsedFilename(m_pWorldFile), sizeof(worldName));
			strupr(worldName);

			pStr = strstr(worldName, ".DAT");
			if (pStr)
				pStr[0] = 0;


			if (stricmp(pWorldName, worldName) == 0)
			{
				bSameWorld = true;
			}
		}
	}

	bSwitchingWorlds = (m_State == SERV_RUNNINGWORLD) ? true : false;
	i_server_shell->PreStartWorld(bSwitchingWorlds);

	// Close down the old stuff if any.
	if (bSwitchingWorlds)
	{
		sm_StartCachingFiles();

		DoEndWorld(bSameWorld);
	}

	// Reset sky.
	memset(m_SkyObjects, 0xFF, sizeof(m_SkyObjects));

	// Reset the game time.
	m_GameTime = 0.0f;

	// Reset the changed object list.
    dl_InitList(&m_ChangedObjectHead);

	// Reset the changed object list
	m_ChangedSoundTrackHead = LTNULL;

	// Tie this off in case they try to remove some objects.
	ASSERT(m_RemovedObjectHead.m_pNext == &m_RemovedObjectHead);
	dl_TieOff(&m_RemovedObjectHead);

 
	// See if the world exists.
	if (pWorldName[strlen(pWorldName)-4] != '.')
	{
		LTSNPrintF(worldName, sizeof(worldName), "%s.dat", pWorldName);
	}
	else 
	{
		LTSNPrintF(worldName, sizeof(worldName), "%s", pWorldName);
	}
 
	
	pStream = server_filemgr->OpenFile(worldName);

	if (!pStream)
	{
		sm_SetupError(LT_MISSINGWORLDFILE, pWorldName);
		RETURN_ERROR_PARAM(1, CServerMgr::DoStartWorld, LT_MISSINGWORLDFILE, pWorldName);
	}


 
	if (!pStream)
	{
		sm_SetupError(LT_MISSINGWORLDFILE, pWorldName);
		RETURN_ERROR_PARAM(1, CServerMgr::DoStartWorld, LT_MISSINGWORLDFILE, pWorldName);
	}
 
	if (server_filemgr->AddUsedFile(worldName, FFLAG_NEEDED, &pWorldFile) == 0)
	{
		sm_SetupError(LT_MISSINGWORLDFILE, pWorldName);
		RETURN_ERROR_PARAM(1, CServerMgr::DoStartWorld, LT_MISSINGWORLDFILE, pWorldName);
	}

	// Load everything up.
	if (bSameWorld && m_pWorldFile)
	{
		m_bWorldLoaded = true;
	}
	else
	{
		dsi_ConsolePrint("Loading world: %s", pWorldName);

		dResult = LoadWorld(pStream, pWorldName);
		if (dResult != LT_OK)
		{
 			if (pStream)
				pStream->Release();
			return dResult;
		}
	}

	m_pWorldFile = pWorldFile;

	// Create world models..
	CreateVisContainerObjects();

	// Read the blind object data
	pStream->SeekTo( world_bsp_shared->GetBlindObjectDataPos() );
	ELoadWorldStatus eResult = g_iWorldBlindObjectData->Load(pStream);
	if (eResult != LoadWorld_Ok)
	{
		if( pStream )
			pStream->Release();
		world_bsp_shared->Term();
		return LT_ERROR;
	}
 
	// Load the objects from the level.
	// Note: it treats everything below here like a regular Update() call and
	// even calls FinishUpdateFrame below so things work correctly (removed objects
	// actually get removed).
	LoadObjects(pStream, pWorldName, !!(flags & LOADWORLD_LOADWORLDOBJECTS), world_bsp_shared->GetObjectDataPos());

	// Let go of the stream if we're just re-loading, since the world already has one it's tracking
	if (pStream)
	{
		pStream->Release();
	}
 

	// Time check
	clock_t LoadWorldTime = clock();
	dsi_ConsolePrint("Loaded world: %s in %.2f seconds", pWorldName, ((float)(LoadWorldTime - StartTime)) / CLOCKS_PER_SEC);

	m_GameTime = 0.0f;

	m_nTimeOffsetMS = 0;
	m_nTrueLastTimeMS = nCurTimeMS;
	m_FrameTime = 0;

	if (flags & LOADWORLD_RUNWORLD)
	{
		DoRunWorld( );
	}
 
	return LT_OK;

}

LTRESULT CServerMgr::DoRunWorld()
{
	clock_t StartTime = clock();

	if (!world_bsp_server->IsLoaded()) 
	{
		RETURN_ERROR(1, CServerMgr::DoRunWorld, LT_NOTINWORLD);
	}

	// Cache stuff like skins.
	//sm_CacheEasyStuff();

	// Unload stuff we don't need.
	sm_EndCachingFiles();

	m_State = SERV_RUNNINGWORLD;
	m_LastServerFPS = -1.0; // force first-time initialization in Update

	// Call PostStartWorld if necessary.
	i_server_shell->PostStartWorld();

	// Notify each object that all the objects have been created...
	LTLink *pHead= &m_Objects.m_Head;
	LTLink *pCur = pHead->m_pNext;
	while (pCur != pHead)
	{
		LTObject *pObj = (LTObject*)pCur->m_pData;	
		pCur = pCur->m_pNext;

		pObj->sd->m_pObject->OnAllObjectsCreated();
	}

	// Finish the update frame.
	sm_FinishUpdateFrame();

	//inform the user how much time it took to run the world
	clock_t EndTime = clock();

	dsi_ConsolePrint("DoRunWorld completed in %.2f seconds", ((float)(EndTime - StartTime)) / CLOCKS_PER_SEC);

	return LT_OK;
}

void CServerMgr::DoEndWorld(bool bKeepGeometryAround)
{
	LTLink *pCur, *pHead;
	Client *pClient;

	// No longer consider world loaded.
	m_bWorldLoaded = false;

	// Get rid of any objects with client references laying around.  These are
	// the objects that got restored from a savegame but never had a client
	// take over them.
	sm_RemoveOldClientRefObjects();

	// Clear the client reference list (client references are only
	// valid while in the world/save file they came from).
	sm_ClearClientReferenceList();

	// All clients go into WAITINGTOENTERWORLD state.
	// Note: this must come before we actually delete the world so the local
	//	   client can get all of its objects out of the world.
	pHead = &m_Clients.m_Head;
	for (pCur = pHead->m_pNext; pCur != pHead; pCur = pCur->m_pNext)
	{
		pClient = (Client*)pCur->m_pData;
		sm_SetClientState(pClient, CLIENT_WAITINGTOENTERWORLD);
	}


	// Disassociate clients from their objects.
	s_DisassociateClientsFromObjects();

	// Remove all objects from the world.
	sm_RemoveAllObjectsFromWorld(false);

	// Delete the sound data and instances
	// This has to come after the removal of objects, cuz the objects have pointers to some sounds...
	sm_RemoveAllUnusedSoundTracks();
 
	if (!bKeepGeometryAround) 
	{
		world_bsp_server->Term();
	}

	UncacheModels();

	m_State = SERV_NOSTATE;

	dsi_ConsolePrint("World ended");
}
