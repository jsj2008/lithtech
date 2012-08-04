#include "bdefs.h"

#include "s_client.h"
#include "ftserv.h"
#include "servermgr.h"
#include "s_net.h"
#include "dhashtable.h"
#include "sounddata.h"
#include "impl_common.h"
#include "s_object.h"
#include "soundtrack.h"
#include "serverevent.h"
#include "sysstreamsim.h"
#include "iltphysics.h"
#include "limits.h"
#include "ltmessage_server.h"
#include "netmgr.h"
#include "clienthack.h"

#include <queue>

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

//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);


struct UpdateInfo {
	Client			*m_pClient;
	CPacket_Write	m_cPacket;
	CPacket_Write	m_cUnguaranteed;
	uint32			m_nTargetUpdateSize;
	uint32			m_nUpdateTime;
};

SentList *g_pCurSentList;


uint32 g_Ticks_ClientVis;

extern int32 g_CV_STracePackets, g_CV_DelimitPackets;
extern int32 g_CV_MeasurePackets;
extern int32 g_bForceRemote;
extern int32 g_CV_ConnTroubleCount;
extern int32 g_CV_BandwidthTargetServer;

ILTStream* sm_FTOpenFn(FTServ *hServ, char *pFilename) 
{
	return server_filemgr->OpenFile(pFilename);
}

void sm_FTCloseFn(FTServ *hServ, ILTStream *pStream) 
{
	pStream->Release();
}

int sm_FTCantOpenFileFn(FTServ *hServ, char *pFilename) 
{
	return TODO_REMOVEFILE;
}



//------------------------------------------------------------------
// Client structure implementation
//------------------------------------------------------------------

Client::Client() :
	m_Link(LTLink_Init),
	m_pClientData(0),
	m_ClientDataLen(0),
	m_nDesiredReceiveBandwidth(0),
	m_AttachmentLink(LTLink_Init),
	m_pAttachmentParent(0),
	m_ViewPos(0.0f, 0.0f, 0.0f),
	m_Attachments(LTLink_Init),
	m_hFTServ(0),
	m_ObjInfos(0),
	m_iPrevSentList(0),
	m_pObject(0),
	m_pPluginUserData(0),
	m_ClientID(0),
	m_State(0),
	m_PuttingIntoWorldStage(0),
	m_ClientFlags(0),
	m_Name(0),
	m_ConnectionID(0),
	m_Events(LTLink_Init),
	m_hFileIDTable(0)
{
}

bool Client::Init( CBaseConn *pBaseConn, bool bIsLocal )
{
	m_ClientFlags |= CFLAG_WANTALLFILES | CFLAG_SENDGLOBALLIGHT;

	m_ClientFlags |= CFLAG_SENDCOBJROTATION;
	m_State = CLIENT_WAITINGTOENTERWORLD;
	LT_MEM_TRACK_ALLOC(m_Name = (char*)dalloc(1),LT_MEM_TYPE_MISC);
	m_Name[0] = 0;
	m_AttachmentLink.m_pData = this;

	if( bIsLocal ) 
	{
		m_ClientFlags |= CFLAG_LOCAL;
		g_pServerMgr->m_InternalFlags |= SFLAG_LOCAL;
	}

	uint32 flags = 0;
	if (bIsLocal) 
	{
		flags |= FTSFLAG_LOCAL;
	}

	FTSInitStruct initStruct;
	initStruct.m_OpenFn = sm_FTOpenFn;
	initStruct.m_CloseFn = sm_FTCloseFn;
	initStruct.m_CantOpenFileFn = sm_FTCantOpenFileFn;
	initStruct.m_pNetMgr = &g_pServerMgr->m_NetMgr;
	initStruct.m_ConnID = pBaseConn;
	m_hFTServ = fts_Init(&initStruct, flags);


	LT_MEM_TRACK_ALLOC(m_ObjInfos = (ObjInfo*)dalloc(sizeof(ObjInfo) * g_pServerMgr->m_nObjInfos),LT_MEM_TYPE_OBJECT);
	memset(m_ObjInfos, 0, sizeof(ObjInfo)*g_pServerMgr->m_nObjInfos);

	m_ConnectionID = pBaseConn;
	m_pObject = LTNULL;


	// Find a free ID.
	m_ClientID = 0;
	
	uint16 testID;
	for (testID=0; testID < 30000; testID++) 
	{
		bool bUnique = true;
		
		LTLink *pCur, *pListHead;
		pListHead = &g_pServerMgr->m_Clients.m_Head;
		for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) 
		{
			if (((Client*)pCur->m_pData)->m_ClientID == testID)
			{
				bUnique = false;
				break;
			}
		}

		if (bUnique) 
		{
			break;
		}
	}

	m_ClientID = testID;
 
	// Initialize the fileid info list.  Some information sent to the client doesn't change based on fileid.
	// This is used to reduce the amount of info sent to the client, but just comparing the new info to what
	// was sent last.  As an example, sound radii is typically the same for one particular file, so the server
	// only needs to send this info once.  Other info can be added to the FileIDInfo structure as needed.
	m_hFileIDTable = hs_CreateHashTable(500, HASH_2BYTENUMBER);
	
	// Add us to the list of clients.
	dl_AddHead(&g_pServerMgr->m_Clients, &m_Link, this);
	
	// Tell the server shell they're in.
	i_server_shell->OnAddClient((HCLIENT)this);

	// Don't let them send anything yet..
	pBaseConn->SetBandwidth(0);

	// MUST BE FIRST.  Send them the protocol version.
	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_NETPROTOCOLVERSION);
	cPacket.Writeuint32(LT_NET_PROTOCOL_VERSION);
	cPacket.Writeuint32(g_CV_BandwidthTargetServer);
	SendToClient(this, CPacket_Read(cPacket), false);

	// Send them their ID.
	cPacket.Writeuint8(SMSG_YOURID);
	cPacket.Writeuint16(m_ClientID);
	cPacket.Writeuint8((uint8)bIsLocal);
	SendToClient(this, CPacket_Read(cPacket), false);

	// Setup their file transfer.
	HHashIterator *hIterator = hs_GetFirstElement(server_filemgr->m_hFileTable);
	while (hIterator) 
	{
		HHashElement *hElement = hs_GetNextElement(hIterator);
		UsedFile *pUsedFile = (UsedFile*)hs_GetElementUserData(hElement);
 
		fts_AddFile(m_hFTServ, 
			pUsedFile->GetFilename(), pUsedFile->m_FileSize, pUsedFile->m_FileID, 
			(uint16)(pUsedFile->m_Flags | FFLAG_SENDWAIT));
	}
	fts_FlushAddedFiles(m_hFTServ); 
	
	// Now set them up to try to get into the world, after their file transfer
	// has completed.
	sm_SetClientState(this, CLIENT_WAITINGTOENTERWORLD);

	dsi_ConsolePrint("New client, id %d, for a total of %d.", m_ClientID, g_pServerMgr->m_Clients.m_nElements);

	g_pServerMgr->ResetFlowControl();

	return true;
}

Client::~Client( )
{ 
	// Free up everything.
	fts_Term(m_hFTServ);
	m_hFTServ = NULL; 

	LTLink *pCur;
	pCur = m_Events.m_Head.m_pNext;
	while( pCur != &m_Events.m_Head )
	{
		LTLink *pNext = pCur->m_pNext;
		CServerEvent *pEvent = (CServerEvent*)pCur->m_pData;
		pEvent->DecrementRefCount();
		pCur = pNext;
	}

	dfree(m_ObjInfos);
	dfree(m_SentLists[0].m_ObjectIDs);
	dfree(m_SentLists[1].m_ObjectIDs);

	dfree(m_Name);

	// Free the fileid info structures...
	HHashIterator *hIterator = hs_GetFirstElement(m_hFileIDTable);
	while( hIterator )
	{
		HHashElement *hElement = hs_GetNextElement(hIterator);
		if (hElement) 
		{
			sb_Free(&g_pServerMgr->m_FileIDInfoBank, hs_GetElementUserData(hElement));
		}
	}
	hs_DestroyHashTable(m_hFileIDTable);

	if (m_pClientData) 
	{
		delete[] m_pClientData;
		m_pClientData = NULL;
	}

	if( !m_Link.IsTiedOff( ))
	dl_RemoveAt(&g_pServerMgr->m_Clients, &m_Link);
}

//------------------------------------------------------------------
// ServerMgr client-related function implementation
//------------------------------------------------------------------

void sm_UpdateClientFileTransfer(Client *pClient) 
{
	fts_Update(pClient->m_hFTServ, g_pServerMgr->m_nTrueFrameTimeMS / 1000.0f);
}


bool sm_CanClientEnterWorld(Client *pClient) 
{
	if (g_pServerMgr->m_State != SERV_RUNNINGWORLD) 
		return false;

	if (!(pClient->m_ClientFlags & CFLAG_GOT_HELLO)) 
		return false;

 
	if (pClient->m_ClientFlags & CFLAG_LOCAL) 
	{
		return true;
	}
	else 
	{
		if (pClient->m_ClientFlags & CFLAG_WANTALLFILES) 
		{
			return (fts_GetNumNeededFiles(pClient->m_hFTServ) == 0) && 
				(fts_GetNumTotalFiles(pClient->m_hFTServ) == 0);
		}
		else 
		{
			return fts_GetNumNeededFiles(pClient->m_hFTServ) == 0;
		}
	}
}

LTRESULT sm_SendCacheListSection(Client *pClient, 
										uint32 nStartIndex, 
										uint8 nPacketID, 
										uint16 nFileType) 
{
	bool bPacketEmpty = true;

	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	cPacket.Writeuint8(nPacketID);

	for (uint32 i = nStartIndex; i < g_pServerMgr->m_CacheListSize; ++i) 
	{
		if (g_pServerMgr->m_CacheList[i].m_FileType == nFileType) 
		{
			bPacketEmpty = false;

			cPacket.Writeuint16((uint16)g_pServerMgr->m_CacheList[i].m_FileID);
		}
	}

	// Send it to the client if it's not empty
	if (!bPacketEmpty) 
	{
		SendToClient(pClient, CPacket_Read(cPacket), false);
	}

	return LT_OK;
}


// Sends the cached file list to the client
LTRESULT sm_SendCacheListToClient(Client *pClient, uint32 nStartIndex) 
{
	// Add models
	//sm_SendCacheListSection(pClient, nStartIndex, PRELOADTYPE_MODEL_CACHED, FT_MODEL);

	// Add sprites.
	sm_SendCacheListSection(pClient, nStartIndex, PRELOADTYPE_SPRITE, FT_SPRITE);

	// Add textures.
	sm_SendCacheListSection(pClient, nStartIndex, PRELOADTYPE_TEXTURE, FT_TEXTURE);

	// Add sounds.
	sm_SendCacheListSection(pClient, nStartIndex, PRELOADTYPE_SOUND, FT_SOUND);

	return LT_OK;
}


// Goes thru the current level and tells the client about everything it should preload.
LTRESULT sm_TellClientToPreloadStuff(Client *pClient) 
{
	// t.f
	g_pServerMgr->SendPreloadModelMsgToClient( pClient );
	
	// Send all the other files in the cache list
	sm_SendCacheListToClient(pClient, 0);

	// Send the sounds that weren't in the cache list
	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	cPacket.Writeuint8(PRELOADTYPE_SOUND);

	bool bPacketEmpty = true;
	
	LTLink *pCur = g_pServerMgr->m_SoundDataList.m_Head.m_pNext;
	while (pCur != &g_pServerMgr->m_SoundDataList.m_Head) 
	{
		CSoundData *pSoundData = (CSoundData *)pCur->m_pData;

		pCur = pCur->m_pNext;

		if (!pSoundData) continue;

		if (pSoundData->IsTouched()) 
		{
			if (pSoundData->GetFile()) 
			{
				bPacketEmpty = false;
				cPacket.Writeuint16((uint16)pSoundData->GetFile()->m_FileID);
			}
		}
	}

	if (!bPacketEmpty)
		SendToClient(pClient, CPacket_Read(cPacket), false);
	else
		cPacket.Reset();

	// Mark the end and send away.
	cPacket.Writeuint8(SMSG_PRELOADLIST);
	cPacket.Writeuint8(PRELOADTYPE_END);
	SendToClient(pClient, CPacket_Read(cPacket), false);

	return LT_OK;
}


LTRESULT sm_UpdatePuttingInWorld(Client *pClient)
{
	if (pClient->m_PuttingIntoWorldStage == PUTTINGINTOWORLD_LOADEDWORLD ||
		(pClient->m_ClientFlags & CFLAG_LOCAL))
	{
		// Tell them all the stuff they need to preload.
		sm_TellClientToPreloadStuff(pClient);


		// Tell them about the sky.
		sm_TellClientAboutSky(pClient);

 
		pClient->m_PuttingIntoWorldStage = PUTTINGINTOWORLD_PRELOADING;
	}

	if (pClient->m_PuttingIntoWorldStage == PUTTINGINTOWORLD_PRELOADED ||
		(pClient->m_ClientFlags & CFLAG_LOCAL))
	{
		// Call OnClientEnterWorld and setup the client's object.
		LPBASECLASS pObject = i_server_shell->OnClientEnterWorld((HCLIENT)pClient);

		if (!pObject) 
		{
			dsi_PrintToConsole("Error: ServerShell::OnClientEnterWorld returned LTNULL!");
			return LT_ERROR;
		}

		pClient->m_pObject = GetObjectServerObj(pObject);
		pClient->m_pObject->sd->m_pClient = pClient;

		ASSERT(g_pServerMgr->m_pWorldFile);
		
		// If they used a ClientRef's object, remove the ClientRef.
		LTLink *pCur, *pListHead;
		pListHead = &g_pServerMgr->m_ClientReferences.m_Head;
		for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) 
		{
			ClientRef *pRef = (ClientRef*)pCur->m_pData;
			
			if (pRef->m_ObjectID == pClient->m_pObject->m_ObjectID) 
			{
				pClient->m_pObject->m_InternalFlags &= ~IFLAG_HASCLIENTREF;
 
				dl_RemoveAt(&g_pServerMgr->m_ClientReferences, &pRef->m_Link);

				dfree(pRef);
				break;
			}
		}


		// Tell this client (and its attachments) to use the object ID if it
		// doesn't have an attachment parent.
		if (!pClient->m_pAttachmentParent) 
		{
			CPacket_Write cPacket;
			cPacket.Writeuint8(SMSG_CLIENTOBJECTID);
			cPacket.Writeuint16(pClient->m_pObject->m_ObjectID);
			SendToClient(pClient, CPacket_Read(cPacket), true);
		}

		// Reset their sent lists.
		pClient->m_SentLists[0].m_nObjectIDs = pClient->m_SentLists[1].m_nObjectIDs = 0;
	
		// Mark all the objects as new and find out what the client needs to be sent.
		pListHead = &g_pServerMgr->m_Objects.m_Head;
		for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) 
		{
			LTObject *pObj = (LTObject*)pCur->m_pData;
			
			ObjInfo *pInfo = &pClient->m_ObjInfos[pObj->m_ObjectID];
			pInfo->m_ChangeFlags = (uint16)sm_GetNewObjectChangeFlags(pObj);
		}


 
		// Mark all the soundtracks as new and find out what the client needs to be sent.
		for (pCur=g_pServerMgr->m_SoundTrackList.m_Head.m_pNext; pCur != &g_pServerMgr->m_SoundTrackList.m_Head; pCur = pCur->m_pNext) 
		{
			CSoundTrack *pSoundTrack = (CSoundTrack *)pCur->m_pData;
			
			ObjInfo *pInfo = &pClient->m_ObjInfos[GetLinkID(pSoundTrack->m_pIDLink)];
			pInfo->m_ChangeFlags = (uint16)sm_GetNewSoundTrackChangeFlags(pSoundTrack);
			pInfo->m_nSoundFlags = 0;
		}

		pClient->m_State = CLIENT_INWORLD;
	}

	return LT_OK;
}


LTRESULT sm_ConnectClientToWorld(Client *pClient) 
{
	if (g_pServerMgr->m_State != SERV_RUNNINGWORLD) 
	{
		return LT_OK;
	}

	ASSERT(pClient->m_State != CLIENT_INWORLD);

	// Send the client the console state...
	HHashIterator *hIterator = hs_GetFirstElement(console_state->State()->m_VarHash);
	while (hIterator)
	{
		HHashElement *hElement = hs_GetNextElement(hIterator);
		if (!hElement)
			continue;

		LTCommandVar *pCurVar = (LTCommandVar *)hs_GetElementUserData( hElement );

		CPacket_Write cPacket;
		cPacket.Writeuint8(SMSG_CONSOLEVAR);
		cPacket.WriteString(pCurVar->pVarName);
		cPacket.WriteString(pCurVar->pStringVal);
		SendToClient(pClient, CPacket_Read(cPacket), false);
	}

	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_LOADWORLD);
	cPacket.Writefloat(g_pServerMgr->m_GameTime);
	cPacket.Writeuint16((uint16)g_pServerMgr->m_pWorldFile->m_FileID);
	SendToClient(pClient, CPacket_Read(cPacket), false);

	// Alrighty.. wait till we get the acknowledgement back.
	pClient->m_State = CLIENT_PUTTINGINWORLD;
	pClient->m_PuttingIntoWorldStage = PUTTINGINTOWORLD_LOADINGWORLD;

	// Get them in right away if they're local.
	if (pClient->m_ClientFlags & CFLAG_LOCAL) 
	{
		sm_UpdatePuttingInWorld(pClient);
	}

	return LT_OK;
}

void sm_GetClientOutOfWorld(Client *pClient) 
{
	i_server_shell->OnClientExitWorld((HCLIENT)pClient);
	
	if (pClient->m_pObject) {
		pClient->m_pObject->sd->m_pClient = LTNULL;
	}

	pClient->m_pObject = LTNULL;

	// Send them an UNLOADWORLD packet.
	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_UNLOADWORLD);
	SendToClient(pClient, CPacket_Read(cPacket), false);

    // If they're local, call the function to get them out of the world
    // so the local client removes its objects.
    if (pClient->m_ClientFlags & CFLAG_LOCAL) {
        clienthack_UnloadWorld();
    }
}

// ----------------------------------------------------------------------- //
// Interface functions.
// ----------------------------------------------------------------------- //

Client* sm_OnNewConnection(CBaseConn *id, bool bIsLocal) 
{
	// Clear the local flag if remote is forced.
	if (g_bForceRemote) 
	{
		bIsLocal = false;
	}

	// Add the client to the internal structures.
	Client *pClient;
	LT_MEM_TRACK_ALLOC(( pClient = new Client( )),LT_MEM_TYPE_MISC);
	if( !pClient )
		return NULL;

	if( !pClient->Init( id, bIsLocal ))
	{
		delete pClient;
		pClient = NULL;
		return NULL;
	}

	return pClient;
}

void sm_OnBrokenConnection(CBaseConn *id) 
{
	Client *pClient = sm_FindClient(id);
	
	if (pClient) 
	{
		sm_RemoveClient(pClient);
	}
}

LTRESULT sm_AttachClient(Client *pParent, Client *pChild) 
{
	if (pParent == pChild) 
	{
		RETURN_ERROR(1, sm_AttachClient, LT_INVALIDPARAMS);
	}

	sm_DetachClient(pChild);

	pChild->m_pAttachmentParent = pParent;
	dl_Insert(&pParent->m_Attachments, &pChild->m_AttachmentLink);

	// Tell the client to use the new object ID.
	if (pParent->m_pObject) 
	{
		CPacket_Write cPacket;
		cPacket.Writeuint8(SMSG_CLIENTOBJECTID);
		cPacket.Writeuint16(pParent->m_pObject->m_ObjectID);
		SendToClient(pChild, CPacket_Read(cPacket), true);
	}

	return LT_OK;
}


LTRESULT sm_DetachClient(Client *pClient) 
{
	if (pClient->m_pAttachmentParent == 0)
		return LT_OK;

	dl_Remove(&pClient->m_AttachmentLink);
	pClient->m_pAttachmentParent = LTNULL;

	// Tell the client to use its normal object.
	if (pClient->m_pObject) 
	{
		CPacket_Write cPacket;
		cPacket.Writeuint8(SMSG_CLIENTOBJECTID);
		cPacket.Writeuint16(pClient->m_pObject->m_ObjectID);
		SendToClient(pClient, CPacket_Read(cPacket), true);
	}

	return LT_OK;
}


LTRESULT sm_DetachClientChildren(Client *pClient) 
{
	LTLink *pCur, *pNext;

	pCur = pClient->m_Attachments.m_pNext;
	while (pCur != &pClient->m_Attachments) 
	{
		pNext = pCur->m_pNext;
		sm_DetachClient((Client*)pCur->m_pData);
		pCur = pNext;
	}

	return LT_OK;
}


void sm_RemoveClient(Client *pClient) 
{
	dsi_ConsolePrint("Removing client, id %d, leaving %d.", 
		pClient->m_ClientID, g_pServerMgr->m_Clients.m_nElements-1);

	// Undo all attachments.
	sm_DetachClient(pClient);
	sm_DetachClientChildren(pClient);
 
	// Remove the client reference in any sound tracks...
	for (LTLink *pCur = g_pServerMgr->m_SoundTrackList.m_Head.m_pNext; 
		pCur != &g_pServerMgr->m_SoundTrackList.m_Head; 
		pCur = pCur->m_pNext) 
	{
		CSoundTrack *pSoundTrack = (CSoundTrack *)pCur->m_pData;
		
		ObjInfo *pInfo = &pClient->m_ObjInfos[GetLinkID(pSoundTrack->m_pIDLink)];
		if (!(pInfo->m_nSoundFlags & OBJINFOSOUNDF_CLIENTDONE)) 
		{
			pSoundTrack->Release(&pInfo->m_nSoundFlags);
		}
	}
	
	// If they had a local connection, clear the server's local flag.
	if (pClient->m_ClientFlags & CFLAG_LOCAL) 
	{
		g_pServerMgr->m_InternalFlags &= ~SFLAG_LOCAL;
	}

	// Get them out of the world..
	sm_SetClientState(pClient, CLIENT_CONNECTED);

	// Notify the shell.
	i_server_shell->OnRemoveClient((HCLIENT)pClient);

	// Remove the client.
	delete pClient;
	pClient = NULL;

	// The flow control needs re-balancing now...
	g_pServerMgr->ResetFlowControl();
}


 
bool sm_SetClientState(Client *pClient, int state) 
{
	if (pClient->m_State == state) 
		return true;
	
	if (state == CLIENT_INWORLD) 
	{
		if (sm_CanClientEnterWorld(pClient)) 
		{
			sm_ConnectClientToWorld(pClient);
			return true;
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		// If they're in the world, remove them from the world.
		if (pClient->m_State == CLIENT_INWORLD) 
		{
			sm_GetClientOutOfWorld(pClient);
		}
	}

	pClient->m_State = state;
	return true;
}


void sm_UpdateClientState(Client *pClient) 
{
	// Try to put them in the world.
	if (pClient->m_State == CLIENT_WAITINGTOENTERWORLD) 
	{
		sm_SetClientState(pClient, CLIENT_INWORLD);
	}
	else if (pClient->m_State == CLIENT_PUTTINGINWORLD) 
	{
		sm_UpdatePuttingInWorld(pClient);
	}

	// Update any files they're transferring.
	sm_UpdateClientFileTransfer(pClient);
}


// Prints all the packet data into the packet.trc file.
void sm_TracePacket(const CPacket_Read &cPacket) 
{
	if (g_CV_STracePackets == 0) 
		return;

	if (!g_pServerMgr->m_pTracePacketFile) 
	{
		g_pServerMgr->m_pTracePacketFile = streamsim_Open("packet.trc", "wb");
		if (!g_pServerMgr->m_pTracePacketFile) 
			return;
	}

	ILTStream *pStream = g_pServerMgr->m_pTracePacketFile;

	uint32 nPacketSize = cPacket.Size();
	*pStream << nPacketSize;
	CPacket_Read cDumpPacket(cPacket);
	cDumpPacket.SeekTo(0);
	while (!cDumpPacket.EOP())
	{
		uint32 nTemp = (uint32)cDumpPacket.Readuint32();
		*pStream << nTemp;
	}

	if (g_CV_DelimitPackets) 
	{
		char end[6] = "*END*";
		pStream->WriteString(end);
	}
}



// Returns true if it sent the packet.
// Set roomNeeded to FU_FORCEFLUSH to force it to send.
 
bool sm_FlushUpdate(UpdateInfo *pInfo, const CPacket_Read &cPacket, uint32 nPacketFlags) 
{
	if (cPacket.Size() <= 8)
		return false;

	sm_TracePacket(cPacket);

	SendToClient(pInfo->m_pClient, cPacket, false, nPacketFlags);

	return true;
}


void WriteUnguaranteedInfo(LTObject *pObject, CPacket_Write &cPacket) 
{
	uint32 flags = 0;

	if (pObject->sd->m_NetFlags & NETFLAG_POSUNGUARANTEED) 
	{
		flags |= UUF_POS;
	}

	if (pObject->sd->m_NetFlags & NETFLAG_ROTUNGUARANTEED) 
	{
		if (pObject->m_Flags & FLAG_YROTATION) 
		{
			flags |= UUF_YROTATION;
		}
		else 
		{
			flags |= UUF_ROT;
		}
	}

	if (pObject->sd->m_NetFlags & NETFLAG_ANIMUNGUARANTEED) 
	{
		// They shouldn't be able to set this flag unless it's a model.
		ASSERT(pObject->m_ObjectType == OT_MODEL);
		flags |= UUF_ANIMINFO;
	}

	ASSERT(flags); // You shouldn't get into this function without being unguaranteed-info friendly

 	cPacket.Writeuint16(pObject->m_ObjectID);
	cPacket.WriteBits(flags, UUF_FLAGCOUNT);

 	// Write position/rotation.
	if (flags & UUF_POS)
	{
		CLTMessage_Write_Server::WriteCompPos(cPacket, pObject->GetPos());
		bool bWriteVelocity = pObject->m_Velocity.MagSqr() > 0.00001f;
		cPacket.Writebool(bWriteVelocity);
		if (bWriteVelocity)
			CLTMessage_Write_Server::WriteCompLTVector(cPacket, pObject->m_Velocity);
	}

	if (flags & UUF_YROTATION) 
	{
		CLTMessage_Write_Server::WriteYRotation(cPacket, pObject->m_Rotation);
	}
	else if (flags & UUF_ROT) 
	{
		CLTMessage_Write_Server::WriteCompLTRotation(cPacket, pObject->m_Rotation);
	}

	// Write anim info.
	if (flags & UUF_ANIMINFO) 
	{
		WriteAnimInfo(ToModel(pObject), cPacket);
	}
}

void ExpandObjectIdList(SentList *pSentList)
{
	uint16* pNewIDs = NULL;
	int nNewSize = pSentList->m_nObjectIDs + 256;

	LT_MEM_TRACK_ALLOC(pNewIDs = (uint16*)dalloc(sizeof(uint16) * (nNewSize)),LT_MEM_TYPE_OBJECT);
	memcpy(pNewIDs, pSentList->m_ObjectIDs, sizeof(uint16) * pSentList->m_nObjectIDs);
	dfree(pSentList->m_ObjectIDs);
	pSentList->m_ObjectIDs = pNewIDs;
	pSentList->m_AllocatedSize = nNewSize;
}


void AddObjectIdToSentList(SentList* pSentList, uint16 id)
{
	// Mark the object as changed.
	if (pSentList->m_nObjectIDs >= pSentList->m_AllocatedSize)
	{
		ExpandObjectIdList( pSentList );
	}

	pSentList->m_ObjectIDs[pSentList->m_nObjectIDs] = id;
	pSentList->m_nObjectIDs++;

	//since all the object ID's are 16 bit we'll have other problems before this, but
	//we should assert just to make sure
	assert(pSentList->m_nObjectIDs < USHRT_MAX);
}

// Marks the object with CF_SENTINFO and sends any change info it has.
 
void sm_AddObjectChangeInfo(UpdateInfo *pInfo, LTObject *pObject, ObjInfo *pObjInfo)
{
	// Check if we already sent this.
	if (pObjInfo->m_ChangeFlags & CF_SENTINFO)
		return;

	// Update the send time
	pObjInfo->m_nLastSentG = pInfo->m_nUpdateTime;

	AddObjectIdToSentList(g_pCurSentList, pObject->m_ObjectID);

	// Setup the packet with update info.
	CPacket_Write cSubPacket;
	if( FillPacketFromInfo(pInfo->m_pClient, pObject, pObjInfo, cSubPacket))
	{
		if (!cSubPacket.Empty())
		{
			pInfo->m_cPacket.Writeuint32(cSubPacket.Size());
			pInfo->m_cPacket.WritePacket(CPacket_Read(cSubPacket));
		}
	}

	// Clear 'em.
	ASSERT(!(pObjInfo->m_ChangeFlags & CF_SENTINFO));
	pObjInfo->m_ChangeFlags = (uint16)((pObjInfo->m_ChangeFlags & CF_CLEARMASK) | CF_SENTINFO);
}

inline bool ShouldSendToClient(const Client *pClient, LTObject *pObject) 
{
	//we should always have an associated client and object
	assert(pClient && pObject);

	//flags that indicate that the client can have an interaction with the object
	static const uint32 k_nInteractionFlags = FLAG_VISIBLE | FLAG_SOLID | FLAG_RAYHIT;

	if (!(pObject->m_Flags & FLAG_FORCECLIENTUPDATE)) 
	{
		//if it is a normal object type, only inform the client about it if it has a special
		//effect message
		if((pObject->m_ObjectType == OT_NORMAL) && (pObject->sd->m_cSpecialEffectMsg.Size() == 0))
			return false;

		// See if this object can't be interacted with and hasn't changed
		if (!(pObject->m_Flags & k_nInteractionFlags) && 
			!(pClient->m_ObjInfos[pObject->m_ObjectID].m_ChangeFlags & CF_FLAGS))
			return false;
	}

	// This happens sometimes when an object removes another object in its
	// destructor.. not a big deal.
	if (!(pObject->m_InternalFlags & IFLAG_INWORLD)) 
	{
		return false;
	}
 
	// Don't tell them about null models.
	if ( (pObject->m_ObjectType == OT_MODEL) && 
		 (pObject->ToModel()->GetModelDB() == NULL) )
	{
		return false;
	}

	return true;
}

inline void UpdateSendToClientState(LTObject *pObject, UpdateInfo *pInfo) 
{
 	sm_AddObjectChangeInfo(pInfo, 
		pObject, &pInfo->m_pClient->m_ObjInfos[pObject->m_ObjectID]);

	for (Attachment *pAttachment=pObject->m_Attachments; pAttachment; pAttachment=pAttachment->m_pNext) 
	{
		LTObject *pAttachedObj = sm_FindObject(pAttachment->m_nChildID);
		if (!pAttachedObj) 
			continue;

		if (!ShouldSendToClient(pInfo->m_pClient, pAttachedObj)) 
			continue;

		sm_AddObjectChangeInfo(pInfo, pAttachedObj, &pInfo->m_pClient->m_ObjInfos[pAttachedObj->m_ObjectID]);
	}
}


void sm_SendSoundTracks(UpdateInfo *pInfo, CPacket_Write &cPacket)
{
	ObjInfo *pObjInfo;
	LTLink *pCur;
	CSoundTrack *pSoundTrack;
//  float fMaxDistSqr;

	for (pCur = g_pServerMgr->m_SoundTrackList.m_Head.m_pNext; pCur != &g_pServerMgr->m_SoundTrackList.m_Head; pCur = pCur->m_pNext)
	{
		pSoundTrack = (CSoundTrack *)pCur->m_pData;
		pObjInfo = &pInfo->m_pClient->m_ObjInfos[GetLinkID(pSoundTrack->m_pIDLink)];

		// Check if the sound was removed but not sending a end loop change.
		if (pSoundTrack->GetRemove() && !(pObjInfo->m_ChangeFlags & CF_SOUNDINFO))
			continue;

		// Check if the sound is done...
		if ((pSoundTrack->IsTrackTime() && pSoundTrack->GetTimeLeft() <= 0.0f))
			continue;
 
		// [RP] 1/11/02 - This range check introduces a bug where upon leaving the range
		// the sound gets removed and never re-added when coming back into the range.
		// The range check is simply a network optimization for the SoundTrack data.
		// Since the SoundTrack data isn't that big to begin with and sending all the sounds really
		// *shouldn't* be a problem... don't do the range check.
						
		// Check if sound is in range...
/*	  if (pSoundTrack->m_dwFlags & (PLAYSOUND_3D | PLAYSOUND_AMBIENT))
		{
			// Check if sound is within 2x its outer radius from the client or the viewer pos...
			fMaxDistSqr = 4.0f * pSoundTrack->m_fOuterRadius * pSoundTrack->m_fOuterRadius;
			if ((pSoundTrack->m_vPosition.DistSqr(pInfo->m_pClient->m_pObject->GetPos()) < fMaxDistSqr) || 
				(pSoundTrack->m_vPosition.DistSqr(pInfo->m_pClient->m_ViewPos) < fMaxDistSqr))
			{
			}
			else
			{
				continue;
			}
		}
*/
		pObjInfo = &pInfo->m_pClient->m_ObjInfos[GetLinkID(pSoundTrack->m_pIDLink)];

		uint16 nNewId = (uint16)GetLinkID(pSoundTrack->m_pIDLink);
		AddObjectIdToSentList( g_pCurSentList, nNewId );

		// Setup the packet with update info.  If the client already told us the sound is done, then don't
		// send it again...
		if (pObjInfo->m_ChangeFlags && !(pObjInfo->m_nSoundFlags & OBJINFOSOUNDF_CLIENTDONE))
		{
			FillSoundTrackPacketFromInfo(pSoundTrack, pObjInfo, pInfo->m_pClient, cPacket);
		}

		// Clear 'em.
		pObjInfo->m_ChangeFlags = CF_SENTINFO;
	}
}


void ClearSoundChangeFlags(UpdateInfo *pInfo)
{
	ObjInfo *pObjInfo;
	LTLink *pCur;
	CSoundTrack *pSoundTrack;

	for (pCur = g_pServerMgr->m_SoundTrackList.m_Head.m_pNext; pCur != &g_pServerMgr->m_SoundTrackList.m_Head; pCur = pCur->m_pNext) 
	{
		pSoundTrack = (CSoundTrack *)pCur->m_pData;
		pObjInfo = &pInfo->m_pClient->m_ObjInfos[GetLinkID(pSoundTrack->m_pIDLink)];
		pObjInfo->m_ChangeFlags = 0;
	}
}


void WriteEndUpdateInfo(Client *pClient, CPacket_Write &cPacket) 
{
	// Mark the end-update info.
	cPacket.Writeuint16(ID_TIMESTAMP);
	cPacket.WriteBits(0, UUF_FLAGCOUNT);
	cPacket.Writefloat(g_pServerMgr->m_GameTime);
}


bool IsClientInTrouble(Client *pClient) 
{
	if(pClient->m_ConnectionID)
	{
		if(pClient->m_ConnectionID->IsInTrouble())
		{
			return true;
		}
	}
	
	return false;
}


void WriteObjectRemoves(UpdateInfo *pInfo, SentList *pPrevList) 
{
	uint32 counter, nRemoves;
	LTRecord *pRecord;
	uint16 *pCurID;
	CSoundTrack *pSoundTrack;

	CPacket_Write cSubPacket;

	nRemoves = 0;
	counter = pPrevList->m_nObjectIDs;
	pCurID = pPrevList->m_ObjectIDs;
	while (counter--)
	{
		if (!(pInfo->m_pClient->m_ObjInfos[*pCurID].m_ChangeFlags & CF_SENTINFO))
		{
			// Write the UDPATESUB_OBJECTREMOVES if necessary.
			if (nRemoves == 0)
			{
				cSubPacket.Writeuint8(0);
				cSubPacket.Writeuint8(UPDATESUB_OBJECTREMOVES);
			}

			cSubPacket.Writeuint16(*pCurID);
			++nRemoves;

			// Find this record...
			pRecord = sm_FindRecord(*pCurID);
			if( pRecord && pRecord->m_pRecordData )
			{
				if (pRecord->m_nRecordType == RECORDTYPE_LTOBJECT)
				{
					pInfo->m_pClient->m_ObjInfos[*pCurID].m_ChangeFlags = 
						(uint16)sm_GetNewObjectChangeFlags((LTObject *)pRecord->m_pRecordData);
				}
				else
				{
					pSoundTrack = (CSoundTrack *)pRecord->m_pRecordData;


					// If the client hasn't already told us the sound is done, then remove the reference...
					if (!(pInfo->m_pClient->m_ObjInfos[ *pCurID ].m_nSoundFlags & OBJINFOSOUNDF_CLIENTDONE))
					{
						pSoundTrack->Release(&pInfo->m_pClient->m_ObjInfos[ *pCurID ].m_nSoundFlags);
					}

					 pInfo->m_pClient->m_ObjInfos[*pCurID].m_ChangeFlags = 
						(uint16)sm_GetNewSoundTrackChangeFlags(pSoundTrack);
				}
			}
		}

		++pCurID;
	}

	if (!cSubPacket.Empty())
	{
		pInfo->m_cPacket.Writeuint32(cSubPacket.Size());
		pInfo->m_cPacket.WritePacket(CPacket_Read(cSubPacket));
	}
}

struct CGuaranteedObjTrack
{
	bool operator<(const CGuaranteedObjTrack &sOther) const { return m_fPriority < sOther.m_fPriority; }
	LTObject *m_pObject;
	ObjInfo *m_pObjInfo;
	float m_fPriority;
};
typedef std::priority_queue<CGuaranteedObjTrack> TGuaranteedObjQueue;

void SendAllObjectsGuaranteed(ObjectMgr *pObjectMgr, UpdateInfo *pInfo) 
{
	//determine if we are dealing with a local client. 
	bool bLocalClient = !!(pInfo->m_pClient->m_ClientFlags & CFLAG_LOCAL);

	uint32 i;
	LTLink *pListHead, *pCur;
	LTObject *pObject;

	if(bLocalClient)
	{
		//we have a local client, so we can bypass queueing up, sorting, bandwidth checking
		//and other tasks and just send all objects
		for (i=0; i < NUM_OBJECTTYPES; i++)
		{
			pListHead = &pObjectMgr->m_ObjectLists[i].m_Head;
			for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
			{
				pObject = (LTObject*)pCur->m_pData;

				// Gotta check here too for objects not in the BSP.
				if (!ShouldSendToClient(pInfo->m_pClient, pObject)) 
					continue;

				// Don't send over the main world model
				if (pObject->IsMainWorldModel()) 
					continue;

				UpdateSendToClientState(pObject, pInfo);
			}
		}
	}
	else
	{
		// Do this in priority order...
		static TGuaranteedObjQueue aObjects;

		// Try not to use up the whole update...
		uint32 nUpdateSizeRemaining = pInfo->m_nTargetUpdateSize / 2;

		for (i=0; i < NUM_OBJECTTYPES; i++)
		{
			pListHead = &pObjectMgr->m_ObjectLists[i].m_Head;
			for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
			{
				pObject = (LTObject*)pCur->m_pData;

				// Gotta check here too for objects not in the BSP.
				if (!ShouldSendToClient(pInfo->m_pClient, pObject)) 
					continue;

				// Don't send over the main world model
				if (pObject->IsMainWorldModel()) 
					continue;

				CGuaranteedObjTrack cCurObj;
				cCurObj.m_pObject	= pObject;
				cCurObj.m_pObjInfo	= &pInfo->m_pClient->m_ObjInfos[pObject->m_ObjectID];
				cCurObj.m_fPriority = (float)(pInfo->m_nUpdateTime - cCurObj.m_pObjInfo->m_nLastSentG);

				aObjects.push(cCurObj);
			}
		}

		while (!aObjects.empty())
		{
			const CGuaranteedObjTrack &cCurObj = aObjects.top();

			UpdateSendToClientState(cCurObj.m_pObject, pInfo);

			// Jump out if we're sending too much...
			if (pInfo->m_cPacket.Size() >= nUpdateSizeRemaining)
			{
				break;
			}

			aObjects.pop();
		}

		// Act like we sent the left-overs so they don't look like they got removed...
		while (!aObjects.empty())
		{
			const CGuaranteedObjTrack &cCurObj = aObjects.top();

			cCurObj.m_pObjInfo->m_ChangeFlags |= CF_SENTINFO;
			AddObjectIdToSentList(g_pCurSentList, cCurObj.m_pObject->m_ObjectID);

			aObjects.pop();
		}
	}
}

//Handles writing out the unguaranteed data of an object as well as all of its attachments
static void WriteUnguaranteedDataWithAttachments(LTObject* pObject, CPacket_Write& cUnguaranteed, const uint32 k_nUnguaranteedMask)
{
	//write out the unguaranteed data for the object itself
	WriteUnguaranteedInfo(pObject, cUnguaranteed);

	//now do the same for all the attached objects
	for (Attachment *pAttachment = pObject->m_Attachments; pAttachment; pAttachment = pAttachment->m_pNext) 
	{
		LTObject *pAttachedObj = sm_FindObject(pAttachment->m_nChildID);
		if (!pAttachedObj) 
			continue;

		if ((pAttachedObj->sd->m_NetFlags & k_nUnguaranteedMask) == 0)
			continue;

		WriteUnguaranteedInfo(pAttachedObj, cUnguaranteed);
	}
}

//Handles updating the send time of the specified object and all of its attachments
static void UpdateSendTimeWithAttachments(LTObject* pObject, UpdateInfo *pInfo, const uint32 k_nUnguaranteedMask)
{
	// Update the send time
	pInfo->m_pClient->m_ObjInfos[pObject->m_ObjectID].m_nLastSentU = pInfo->m_nUpdateTime;

	// Update the send time of the attachments
	for (Attachment *pAttachment = pObject->m_Attachments; pAttachment; pAttachment = pAttachment->m_pNext) 
	{
		LTObject *pAttachedObj = sm_FindObject(pAttachment->m_nChildID);
		if (!pAttachedObj) 
			continue;

		if ((pAttachedObj->sd->m_NetFlags & k_nUnguaranteedMask) == 0)
			continue;

		pInfo->m_pClient->m_ObjInfos[pAttachedObj->m_ObjectID].m_nLastSentU = pInfo->m_nUpdateTime;
	}
}


struct CUnguaranteedObjTrack
{
	bool operator<(const CUnguaranteedObjTrack &sOther) const { return m_fPriority < sOther.m_fPriority; }
	LTObject *m_pObject;
	float m_fPriority;
};
typedef std::priority_queue<CUnguaranteedObjTrack> TUnguaranteedObjQueue;

void SendAllObjectsUnguaranteed(ObjectMgr *pObjectMgr, UpdateInfo *pInfo) 
{
	//determine if we are dealing with a local client. 
	bool bLocalClient = !!(pInfo->m_pClient->m_ClientFlags & CFLAG_LOCAL);

	//mask representing all the flags that would indicate an object needing to be included
	//in an unguaranteed packet
	const uint32 k_nUnguaranteedMask = (NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	if(bLocalClient)
	{
		//we are on a local client, we don't need to do queuing, weighting, or anything, everything
		//can just be sent down
		for (uint32 i = 0; i < NUM_OBJECTTYPES; i++)
		{
			LTLink *pListHead = &pObjectMgr->m_ObjectLists[i].m_Head;
			for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
			{
				LTObject *pObject = (LTObject*)pCur->m_pData;

				if ((pObject->sd->m_NetFlags & k_nUnguaranteedMask) == 0)
					continue;

				//write out all the unguaranteed data
				WriteUnguaranteedDataWithAttachments(pObject, pInfo->m_cUnguaranteed, k_nUnguaranteedMask);

				// Update the send time
				UpdateSendTimeWithAttachments(pObject, pInfo, k_nUnguaranteedMask);				
			}
		}
	}
	else
	{
		// Don't even bother if we're already over the limit
		if (pInfo->m_cPacket.Size() >= pInfo->m_nTargetUpdateSize)
		{
			return;
		}
		
		uint32 nUpdateSizeRemaining = pInfo->m_nTargetUpdateSize - pInfo->m_cPacket.Size();

		// Do this in priority order...
		static TUnguaranteedObjQueue aObjects;

		uint32 nUnguaranteedLength = 0;

		const float k_fDistPriorityScale = 1.0f / 128.0f;

		for (uint32 i = 0; i < NUM_OBJECTTYPES; i++)
		{
			LTLink *pListHead = &pObjectMgr->m_ObjectLists[i].m_Head;
			for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
			{
				LTObject *pObject = (LTObject*)pCur->m_pData;

				if ((pObject->sd->m_NetFlags & k_nUnguaranteedMask) == 0)
					continue;


				CUnguaranteedObjTrack cCurObj;
				cCurObj.m_pObject = pObject;

				//Determine the weight of this message based upon some rules
				ObjInfo* pObjInfo = &pInfo->m_pClient->m_ObjInfos[pObject->m_ObjectID];

				float fDistToClient = LTMAX(pObject->m_Pos.Dist(pInfo->m_pClient->m_ViewPos) * k_fDistPriorityScale, 1.0f);
				float fTime = (float)(pInfo->m_nUpdateTime - pObjInfo->m_nLastSentU) + 1.0f;
				float fSize = pObject->m_Dims.MagSqr();
				float fSpeed = (pObject->m_Velocity.Mag() * k_fDistPriorityScale) + 1.0f;
				cCurObj.m_fPriority = (fTime * fSize * fSpeed) / fDistToClient;

				aObjects.push(cCurObj);
			}
		}

		while (!aObjects.empty())
		{
			const CUnguaranteedObjTrack &cCurObj = aObjects.top();

			//write out all the unguaranteed data
			WriteUnguaranteedDataWithAttachments(cCurObj.m_pObject, pInfo->m_cUnguaranteed, k_nUnguaranteedMask);

			// Jump out if we're sending too much...
			if (pInfo->m_cUnguaranteed.Size() >= nUpdateSizeRemaining)
			{
				break;
			}
			else
				nUnguaranteedLength = pInfo->m_cUnguaranteed.Size();

			// Update the send time
			UpdateSendTimeWithAttachments(cCurObj.m_pObject, pInfo, k_nUnguaranteedMask);

			// Next!
			aObjects.pop();
		}

		// Forget the left-overs
		while (!aObjects.empty())
			aObjects.pop();

		// Strip the end of the packet
		if (nUnguaranteedLength != pInfo->m_cUnguaranteed.Size())
		{
			CPacket_Read cStripUnguaranteed(CPacket_Read(pInfo->m_cUnguaranteed), 0, nUnguaranteedLength);
			pInfo->m_cUnguaranteed.WritePacket(cStripUnguaranteed);
		}
	}
}


void sm_UpdateClientInWorld(Client *pClient)
{
	// If the client's queue is backed up, wait until it's ok.
	if (IsClientInTrouble(pClient))
	{
		return;
	}

	// If they're not in the world, they don't need to be updated...
	if (pClient->m_State != CLIENT_INWORLD)
		return;

	// Init the update info
	UpdateInfo updateInfo;
	updateInfo.m_cPacket.Writeuint8(SMSG_UPDATE);
	updateInfo.m_cUnguaranteed.Writeuint8(SMSG_UNGUARANTEEDUPDATE);
	updateInfo.m_pClient = pClient;

	// Keep track of time
	updateInfo.m_nUpdateTime = timeGetTime();
	uint32 nTimeSinceUpdate = updateInfo.m_nUpdateTime - pClient->m_nLastUpdateTime;
	pClient->m_nLastUpdateTime = updateInfo.m_nUpdateTime;

	// Get the available bandwidth
	// Note : We're more interested in multi-frame bandwidth usage.
	// Further note : We also over-estimate how much we will use.  Otherwise it will use less than it can.
	int32 nAvailableBandwidth = pClient->m_ConnectionID->GetAvailableBandwidth(nTimeSinceUpdate * 3.0f / 1000.0f) / 3;
	if (nAvailableBandwidth <= 0)
	{
		// Don't update them if we're choking
		return;
	}
	updateInfo.m_nTargetUpdateSize = (uint32)nAvailableBandwidth;

	SentList *pPrevList = &pClient->m_SentLists[pClient->m_iPrevSentList];
	SentList *pCurList = &pClient->m_SentLists[!pClient->m_iPrevSentList];
	g_pCurSentList = pCurList;

	// Clear the CF_SENTINFO flag on all the objects we sent info on earlier.
	for (uint32 i = 0; i < pPrevList->m_nObjectIDs; i++)
	{
		pClient->m_ObjInfos[pPrevList->m_ObjectIDs[i]].m_ChangeFlags &= ~CF_SENTINFO;
	}

	// Build the new SentInfo list.
	pCurList->m_nObjectIDs = 0;

#ifdef USE_LOCAL_STUFF
	if (!(pClient->m_ClientFlags & CFLAG_LOCAL))
#endif
	{
		// Activate everything they can see.
		{
			CountAdder cTicks_ClientVis(&g_Ticks_ClientVis);

			// Send all the alive objects to the client
			SendAllObjectsGuaranteed(&g_pServerMgr->m_ObjectMgr, &updateInfo);
		}
	}

	// Add all the event subpackets.
	LTLink *pCur = pClient->m_Events.m_Head.m_pNext;
	while (pCur != &pClient->m_Events.m_Head)
	{
		LTLink *pNext = pCur->m_pNext;
	 	CServerEvent *pEvent = (CServerEvent*)pCur->m_pData;
	
		WriteEventToPacket(pEvent, pClient, updateInfo.m_cPacket);

		dl_RemoveAt(&pClient->m_Events, pCur);
		pEvent->DecrementRefCount();

		pCur = pNext;
	}

 	// Send sound tracking data.
	sm_SendSoundTracks(&updateInfo, updateInfo.m_cPacket);
	
	// Write the list of objects to remove (objects we didn't send info on).
	WriteObjectRemoves(&updateInfo, pPrevList);

	// Clear out the change status on all the sound objects
	ClearSoundChangeFlags(&updateInfo);

	// Write unguaranteed stuff. 
	SendAllObjectsUnguaranteed(&g_pServerMgr->m_ObjectMgr, &updateInfo);

	// Mark the end of the unguaranteed info
	WriteEndUpdateInfo(pClient, updateInfo.m_cUnguaranteed);

	// Send them..
	sm_FlushUpdate(&updateInfo, CPacket_Read(updateInfo.m_cPacket), MESSAGE_GUARANTEED);
	sm_FlushUpdate(&updateInfo, CPacket_Read(updateInfo.m_cUnguaranteed), 0);

	pClient->m_iPrevSentList = !pClient->m_iPrevSentList; // Swap this..

	// Send the sky definition if need be.
	if (pClient->m_ClientFlags & CFLAG_SENDSKYDEF)
	{
		pClient->m_ClientFlags &= ~CFLAG_SENDSKYDEF;
		sm_TellClientAboutSky(pClient);
	}

	// Send the global light object if need be.
	if (pClient->m_ClientFlags & CFLAG_SENDGLOBALLIGHT)
	{
		pClient->m_ClientFlags &= ~CFLAG_SENDGLOBALLIGHT;
		sm_TellClientAboutGlobalLight(pClient);
	}
}


Client* sm_FindClient(CBaseConn *connID)
{
	// otherwise, search for the corresponding client in the client list
	LTLink* pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink* pCur=pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client* pClient = (Client*)pCur->m_pData;
		
		if (pClient->m_ConnectionID == connID)
		{
			return pClient;
		}
	}

	return LTNULL;
}


void sm_SetSendSkyDef()
{
	LTLink *pCur;

	for (pCur = g_pServerMgr->m_Clients.m_Head.m_pNext; pCur != &g_pServerMgr->m_Clients.m_Head; pCur=pCur->m_pNext)
	{
		((Client*)pCur->m_pData)->m_ClientFlags |= CFLAG_SENDSKYDEF;
	}
}


void sm_TellClientAboutSky(Client *pClient)
{
	CPacket_Write cPacket;

	cPacket.Writeuint8(SMSG_SKYDEF);
	cPacket.WriteType(g_pServerMgr->m_SkyDef);

	cPacket.Writeuint16(MAX_SKYOBJECTS);
	for (uint32 i = 0; i < MAX_SKYOBJECTS; i++)
	{
		cPacket.Writeuint16(g_pServerMgr->m_SkyObjects[i]);
	}

	SendToClient(pClient, CPacket_Read(cPacket), false);
}


void sm_TellClientAboutGlobalLight(Client *pClient)
{
	StaticSunLight *pSun = (StaticSunLight *)ilt_server->HandleToObject(g_pServerMgr->GetGlobalLightObject());

	// If we don't have a sun yet, we can't tell the client, now can we?
	// (They'll get told as soon as we get told...)
	if (!pSun)
		return;

	CPacket_Write cPacket; 

	cPacket.Writeuint8(SMSG_GLOBALLIGHT);
 
	LTRotation rRotation;

	ilt_server->GetObjectRotation(g_pServerMgr->GetGlobalLightObject(), &rRotation);

	cPacket.WriteLTVector(rRotation.Forward());

	//the brightness scale of the sunlight
	float fBrightness = pSun->m_BrightScale * pSun->m_ObjectBrightScale;

	cPacket.WriteLTVector(pSun->m_InnerColor * fBrightness);

	cPacket.Writefloat(pSun->m_fConvertToAmbient);

	SendToClient(pClient, CPacket_Read(cPacket), false);
}



LTRESULT sm_RemoveObjectFromSky(LTObject *pObject)
{
	uint32 i;

	// Is it even in the sky?
	if (~pObject->m_InternalFlags & IFLAG_INSKY)
	{
		return LT_OK;
	}

	pObject->m_InternalFlags &= ~IFLAG_INSKY;

	sm_SetSendSkyDef();
	for (i=0; i < MAX_SKYOBJECTS; i++)
	{
		if (g_pServerMgr->m_SkyObjects[i] == pObject->m_ObjectID)
		{
			g_pServerMgr->m_SkyObjects[i] = 0xFFFF;
			sm_SetSendSkyDef();
			return LT_OK;
		}
	}

	return LT_OK;
}


// Get the FileIDInfo for this file id.
FileIDInfo *sm_GetClientFileIDInfo(Client *pClient, uint16 wFileID)
{
	HHashElement *hElement;
	FileIDInfo *pFileIDInfo = LTNULL;

	// See if it's in the hash table.
	hElement = hs_FindElement(pClient->m_hFileIDTable, &wFileID, 2);
	if (hElement)
	{
		pFileIDInfo = (FileIDInfo *)hs_GetElementUserData(hElement);
		if (pFileIDInfo)
		{
			return pFileIDInfo;
		}
	}

	// Create a new fileidinfo...
	pFileIDInfo = (FileIDInfo *)sb_Allocate_z(&g_pServerMgr->m_FileIDInfoBank);
	if (!pFileIDInfo)
		return LTNULL;

	pFileIDInfo->m_nChangeFlags = FILEIDINFOF_NEWMASK;

	// Make a new one...
	hElement = hs_AddElement(pClient->m_hFileIDTable, &wFileID, 2);
	if (!hElement)
	{
		dfree(pFileIDInfo);
		return LTNULL;
	}

	// Have the hash table own the pointer...
	hs_SetElementUserData(hElement, (void *)pFileIDInfo);

	return pFileIDInfo;

}


