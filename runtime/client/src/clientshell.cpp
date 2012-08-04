// ------------------------------------------------------------------ //
//
//  FILE      : ClientShell.cpp
//
//  PURPOSE   : 
//
//  CREATED   : February 25 1997
//                           
//  COPYRIGHT : Microsoft 1997 All Rights Reserved
//
// ------------------------------------------------------------------ //

// Includes....
#include "bdefs.h"

#include "clientshell.h"
#include "de_world.h"
#include "setupobject.h"
#include "clientmgr.h"
#include "sprite.h"
#include "consolecommands.h"
#include "servermgr.h"
#include "predict.h"
#include "moveobject.h"
#include "server_interface.h"
#include "dhashtable.h"
#include "console.h"

#include "build_options.h"
#include "render.h"

#include "ltobjectcreate.h"

#include "client_ticks.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);


//IWorldBlindObjectData holder
#include "world_blind_object_data.h"
static IWorldBlindObjectData *g_iWorldBlindObjectData = LTNULL;
define_holder(IWorldBlindObjectData, g_iWorldBlindObjectData);



extern LTBOOL g_bUpdateServer;
extern int32 g_CV_MasterPaletteMode, g_bForceRemote;

CClientShell *g_pClientShell;





// ------------------------------------------------------------------ //
// Static functions..
// ------------------------------------------------------------------ //

static void AddAllObjectsToBSP(LTList *pList)
{
    LTLink *pCur, *pListHead;
    LTObject *pObj;
    WorldTree *pWorldTree;

    pWorldTree = world_bsp_client->ClientTree();

    pListHead = &pList->m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pObj = (LTObject*)pCur->m_pData;
    
        pWorldTree->InsertObject(pObj);
    }
}


static void RemoveAllObjectsFromWorldTree(LTList *pList)
{
    LTLink *pCur, *pListHead;
    LTObject *pObj;

    pListHead = &pList->m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pObj = (LTObject*)pCur->m_pData;
        pObj->RemoveFromWorldTree();
    }
}


static bool cs_ShouldUseTCPIP() {
    LTCommandVar *pVar;

    if (pVar = cc_FindConsoleVar(&g_ClientConsoleState, "tcpip"))
    {
        if (pVar->floatVal != 0.0f)
            return true;
    }

    return false;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Console stuff
//------------------------------------------------------------------
//------------------------------------------------------------------

CONCOLOR    g_ShellMsgColor = CONRGB(0, 255, 0);




////////////////////////////////////////////////////////
// ClientShell functions
////////////////////////////////////////////////////////

CClientShell::CClientShell()
{
    uint32 i;

    m_pLastWorld = NULL;
    m_bWorldOpened = false;
    m_GameTime = m_LastGameTime = m_GameFrameTime = 0.0f;

    m_HostID = INVALID_CONNID;

    m_pDriver = NULL;

    m_KillTag = 0;
    g_pServerMgr = NULL;
    m_ClientID = (uint16)-1;
    
    m_ClientObjectID = (uint16)-1;
    dl_TieOff(&m_MovingObjects);
    dl_TieOff(&m_RotatingObjects);
    m_pFrameClientObject = NULL;

    for (i=0; i < 16; i++)
    {
        m_ColorSignExtend[i] = (uint8)(((float)i * 255.0f) / 15.0f);
    }
}


CClientShell::~CClientShell()
{
    Term();
}


bool CClientShell::Init()
{
    // Setup net stuff.
    InitHandlers();
    g_pClientMgr->m_NetMgr.SetNetHandler(this);
    g_pClientShell = this;

    // Initialize the fileid info list.  Some information sent to the client doesn't change based on fileid.
    // This is used to reduce the amount of info sent to the client, by just comparing the new info to what
    // was sent last.  As an example, sound radii is typically the same for one particular file, so the server
    // only needs to send this info once.  Other info can be added to the FileIDInfo structure as needed.
    m_hFileIDTable = hs_CreateHashTable(100, HASH_2BYTENUMBER);

    return true;
}


// Clear the fileid's we took from the server for all of our client loaded models.
// The client can load files before the server, then the server assigns an id to
// it.  If the server goes away before the client is done with it, we have to
// clear the fileid we got from the server.
static
void Client_ResetModelFileIds( const Model & model , void *user_dat )
{
	const_cast<Model&>( model ).m_FileID = ( uint32 )-1;
}


void CClientShell::Term()
{
    HHashIterator *hIterator;
    HHashElement *hElement;

    // Let the client manager do its thing..
    g_pClientMgr->OnExitServer(this);

	if( g_pClientMgr->m_NetMgr.IsConnected() )
		SendGoodbye();

    g_pClientMgr->m_NetMgr.Disconnect(m_HostID, DISCONNECTREASON_VOLUNTARY_CLIENTSIDE);
    m_HostID = INVALID_CONNID;

    g_pClientMgr->m_NetMgr.SetNetHandler(NULL);

    // Uninit the server mugger if there is one.
    if (g_pServerMgr) {
        delete g_pServerMgr;
        g_pServerMgr = NULL;
    }

    // Uninit object stuff.
    RemoveAllObjects();

	// Clear out the sounds.
	GetClientILTSoundMgrImpl()->RemoveAllUnusedSoundInstances( );

    // Get rid of the world references because the (local) server will 
    // delete the world.
    NotifyWorldClosing();
    CloseWorlds();

	if (m_pDriver)
		g_pClientMgr->m_NetMgr.RemoveDriver(m_pDriver);
    m_pDriver = NULL;
    m_ClientObjectID = (uint16)-1;

    m_pFrameClientObject = NULL;

    // Free the fileid info structures...
    hIterator = hs_GetFirstElement(m_hFileIDTable);
    while (hIterator)
    {
        hElement = hs_GetNextElement(hIterator);
        if (hElement)
            sb_Free(&g_pClientMgr->m_FileIDInfoBank, hs_GetElementUserData(hElement));
    }
    hs_DestroyHashTable(m_hFileIDTable);
	m_hFileIDTable = NULL;

	// Clear the fileid's we took from the server for all of our client loaded models.
	g_ModelMgr.ForEach( Client_ResetModelFileIds, NULL );
	

    g_pClientShell = NULL;
}


LTRESULT CClientShell::StartupClient(CBaseDriver *pDriver)
{
    m_pDriver = pDriver;
    return LT_OK;
}


LTRESULT CClientShell::StartupLocal(StartGameRequest *pRequest,
    bool bHost, CBaseDriver *pServerDriver)
{
    bool bSelfConnect;
    LTRESULT dResult;
    CBaseDriver *pServerLocalDriver;
    char errorString[256];
    
    
    ASSERT(!m_pDriver);
    

    // This can be false if you just want to serv a game, so it should be a parameter.
    bSelfConnect = true;

    // Setup a server mugger.
    if ((dResult = CreateServerMgr()) != LT_OK)
        return dResult;


    // Give it the game info data.
    if (pRequest->m_pGameInfo && pRequest->m_GameInfoLen > 0)
    {
        g_pServerMgr->SetGameInfo(pRequest->m_pGameInfo, pRequest->m_GameInfoLen);
    }


    // Add the resources to it and listen locally by default.
    if (!g_pServerMgr->AddResources(g_pClientMgr->m_ResTrees, g_pClientMgr->m_nResTrees))
    {
        g_pServerMgr->GetErrorString(errorString, 256);
        RETURN_ERROR_PARAM(1, CClientShell::StartupLocal, LT_SERVERERROR, errorString);
    }

	// Load the object.lto file.
	if( !g_pServerMgr->LoadBinaries( ))
	{
		g_pServerMgr->GetErrorString(errorString, 256);
		RETURN_ERROR_PARAM(1, CClientShell::StartupLocal, LT_SERVERERROR, errorString);
	}

    if (!g_pServerMgr->Listen("local", "LithTech Session"))
    {
        g_pServerMgr->GetErrorString(errorString, sizeof(errorString));
        RETURN_ERROR_PARAM(1, CClientShell::StartupLocal, LT_SERVERERROR, errorString);
        //GetClientMgr()->ThrowClientException(EXC_SHUTDOWN_SHELL|EXC_MODAL_MESSAGE, Cli_ServerError, errorString);
    }

    m_bLocal = true;

    // Give the server the CBaseDriver we have setup.
    if (pServerDriver)
    {
        g_pServerMgr->TransferNetDriver(pServerDriver);
    }

    if (bSelfConnect)
    {
        // Setup a local driver.
        con_PrintString(g_ShellMsgColor, 1, "Connecting locally");
        m_pDriver = g_pClientMgr->m_NetMgr.AddDriver("local");
        
        // Connect locally.
        pServerLocalDriver = g_pServerMgr->GetLocalDriver();
        m_pDriver->LocalConnect(pServerLocalDriver);
        
        // Sort of a hack.. new connection notifications don't come in  (and thus 
        // m_HostID doesn't get set) until you do an update.
        g_pClientMgr->m_NetMgr.Update("Client: ", g_pClientMgr->m_CurTime);
    }

    return LT_OK;
}


LTRESULT CClientShell::CreateServerMgr()
{
    IFBREAKRETURNVAL(g_pServerMgr != NULL, LT_ERROR);

    LT_MEM_TRACK_ALLOC(g_pServerMgr = new CServerMgr,LT_MEM_TYPE_MISC);
    if (!g_pServerMgr || !g_pServerMgr->Init())
    {
        delete g_pServerMgr;
        g_pServerMgr = NULL;

        RETURN_ERROR(1, CClientShell::CreateServerMgr, LT_CANTCREATESERVER);
    }

    g_pServerMgr->m_NetMgr.SetAppGuid(g_pClientMgr->m_NetMgr.GetAppGuid());

    return LT_OK;
}


LTRESULT CClientShell::Update()
{
    long updateFlags;
    char errorString[256];
    LTCommandVar *pVar;
    LTObject *pClientObject;
    LTRESULT dResult;
    
    {
        CountAdder cntAdd(&g_Ticks_NetUpdate);
        g_pClientMgr->m_NetMgr.Update("Client: ", g_pClientMgr->m_CurTime);
    }

    {
		CountAdder cntAdd(&g_Ticks_ServerUpdate);

        // Update our local server manager if it's there.
        // (If the update fails an error will be returned.)
        if (g_pServerMgr && g_bUpdateServer)
        {
            updateFlags = 0;
            if (!dsi_IsClientActive() && m_ShellMode != STARTGAME_HOST && m_ShellMode != STARTGAME_HOSTTCP)
            {
                // Console variable 'alwaysfocused' overrides this.
                pVar = cc_FindConsoleVar(&g_ClientConsoleState, "alwaysfocused");
                if (!pVar ||
                    (pVar->floatVal != 1.0f))
                {
                    updateFlags |= UPDATEFLAG_NONACTIVE;
                }
            }

            if (!g_pServerMgr->Update(updateFlags, g_pClientMgr->m_nCurUpdateTimeMS))
            {
                g_pServerMgr->GetErrorString(errorString, 256);
                g_pClientMgr->SetupError(LT_SERVERERROR, errorString);
                RETURN_ERROR_PARAM(1, CClientShell::Update, LT_SERVERERROR|ERROR_DISCONNECT, errorString);
                //GetClientMgr()->ThrowClientException(EXC_SHUTDOWN_SHELL|EXC_MODAL_MESSAGE, Cli_ServerError, errorString);
            }
        }
    }


    m_GameFrameTime = 0.0f;

	{
        // Process all packets.
        CountAdder cntAdd2(&g_Ticks_NetUpdate);

        dResult = ProcessPackets();
    }

    if (dResult != LT_OK) {
      return dResult | ERROR_DISCONNECT;
    }
    
    // Update the game frame time.
    m_GameFrameTime = m_GameTime - m_LastGameTime;
    m_LastGameTime = m_GameTime;


    // Call PreUpdate and Update functions.
    if (dsi_IsClientActive())
    {
		// Give the predictive views some time!
		pd_Update(this);
    }

    {
		CountAdder cntAdd(&g_Ticks_ClientShell);

        i_client_shell->PreUpdate();

        i_client_shell->Update();
    }

    if (dsi_IsClientActive())
    {
        // Update all the client object structures from their script counterparts.
        g_pClientMgr->UpdateObjects();
    }

    {
		CountAdder cntAdd(&g_Ticks_ClientShell);

        i_client_shell->PostUpdate();
    }

    // Update client obect pointer...
    pClientObject = GetClientObject();
    if (pClientObject)
    {
        if (m_pFrameClientObject != pClientObject)
        {
            m_pFrameClientObject = pClientObject;
        }
    }
    return LT_OK;
}


void CClientShell::SendCommandToServer(char *pCommand)
{
	CPacket_Write cPacket;

	cPacket.Writeuint8(CMSG_COMMANDSTRING);
	cPacket.WriteString(pCommand);

    SendPacketToServer(CPacket_Read(cPacket));
}


void CClientShell::SendPacketToServer(const CPacket_Read &cPacket)
{
	g_pClientMgr->m_NetMgr.SendPacket(cPacket, m_HostID);
}



////////////////////////////////////////////////////////
// Main high-level functions
////////////////////////////////////////////////////////

void CClientShell::RemoveAllObjects()
{
    // Remove all the server objects.
	for (int i=0; i < NUM_OBJECTTYPES; i++)
    {
        g_pClientMgr->RemoveObjectsInList(&g_pClientMgr->m_ObjectMgr.m_ObjectLists[i], true);
    }
}


void CClientShell::NotifyWorldClosing()
{
    if (m_bWorldOpened)
    {
        g_pClientMgr->OnExitWorld(this);
    }
    
    m_bWorldOpened = false;
}



bool CClientShell::CreateVisContainerObjects() {
    uint32 i;
    const WorldData *pWorldData;
    ObjectCreateStruct ocs;
    InternalObjectSetup internalSetup;
    LTObject *pObject;
    LTRESULT dResult;

    internalSetup.m_pSetup = &ocs;

    for (i=0; i < world_bsp_client->NumWorldModels(); i++)
    {
        pWorldData = world_bsp_client->GetWorldModel(i);

        dResult = LT_OK;
        if (pWorldData->OriginalBSP()->m_WorldInfoFlags & (WIF_PHYSICSBSP | WIF_VISBSP))
        {
            // Ok, make an object for it.
            ocs.Clear();
            ocs.m_ObjectType = OT_WORLDMODEL;
            ocs.m_Flags = FLAG_SOLID | FLAG_RAYHIT | FLAG_VISIBLE;
            SAFE_STRCPY(ocs.m_Filename, pWorldData->m_pValidBsp->m_WorldName);
            ocs.m_Pos = pWorldData->OriginalBSP()->m_WorldTranslation;
            
            dResult = g_pClientMgr->AddObjectToClientWorld(OBJID_CLIENTCREATED,
                &internalSetup, &pObject, true, false);

            if (dResult != LT_OK)
            {
                return false;
            }
        }
    }

    return true;
}


bool CClientShell::BindWorlds()
{
    if (!r_IsRenderInitted())
        return true;

    if (!world_bsp_client->RenderContext())
    {
        world_bsp_client->RenderContext() = r_GetRenderStruct()->CreateContext();
        if (!world_bsp_client->RenderContext())
            return false;
    }

    return true;
}



void CClientShell::UnbindWorlds()
{
    if (world_bsp_client->RenderContext())
    {
        r_GetRenderStruct()->DeleteContext((HRENDERCONTEXT)world_bsp_client->RenderContext());
        world_bsp_client->RenderContext() = NULL;
    }
}


void clienthack_UnloadWorld()
{
    if (!g_pClientShell)
        return;
            
    g_pClientShell->m_ClientObjectID = (uint16)-1;
    cs_UnloadWorld(g_pClientShell);
}


// Get the FileIDInfo for this file id.
FileIDInfo *CClientShell::GetClientFileIDInfo(uint16 wFileID)
{
    HHashElement *hElement;
    FileIDInfo *pFileIDInfo = NULL;

    // See if it's in the hash table.
    hElement = hs_FindElement(m_hFileIDTable, &wFileID, 2);
    if (hElement)
    {
        pFileIDInfo = (FileIDInfo *)hs_GetElementUserData(hElement);
        if (pFileIDInfo)
            return pFileIDInfo;
    }

    // Create a new fileidinfo...
    pFileIDInfo = (FileIDInfo *)sb_Allocate_z(&g_pClientMgr->m_FileIDInfoBank);
    if (!pFileIDInfo)
        return NULL;

    // Make a new one...
    hElement = hs_AddElement(m_hFileIDTable, &wFileID, 2);
    if (!hElement)
    {
        dfree(pFileIDInfo);
        return NULL;
    }

    // Have the hash table own the pointer...
    hs_SetElementUserData(hElement, (void *)pFileIDInfo);

    return pFileIDInfo;

}


void WorldBsp::SetPolyTexturePointers() {
    uint32 i;
    Surface *pSurface;
    FileRef ref;

    for (i=0; i < m_nSurfaces; i++) {
        pSurface = &m_Surfaces[i];

        ref.m_FileType = FILE_ANYFILE;
        ref.m_pFilename = m_TextureNames[pSurface->m_iTexture];
    
        pSurface->m_pTexture = g_pClientMgr->AddSharedTexture(&ref);

        if (!pSurface->m_pTexture && (g_DebugLevel >= 1))
        {
            dsi_ConsolePrint("Unable to find world texture %s", ref.m_pFilename);
        }

        // Give it a default texture if it's missing its texture.
        if (!pSurface->m_pTexture)
        {
            ref.m_FileType = FILE_CLIENTFILE;
            ref.m_pFilename = "textures\\default_texture.dtx";
            pSurface->m_pTexture = g_pClientMgr->AddSharedTexture(&ref);
        }
    }
}

void cs_UnloadWorld(CClientShell *pShell)
{
	pShell->NotifyWorldClosing();
	pShell->RemoveAllObjects();
	pShell->CloseWorlds();
}

void CClientShell::CloseWorlds()
{
    int i;


    // Remove all objects from the BSP.
    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        RemoveAllObjectsFromWorldTree(&g_pClientMgr->m_ObjectMgr.m_ObjectLists[i]);
    }

    LTLink *pListHead, *pCur;
    LTObject *pObject;

    // Clear their standing-on status.
    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        pListHead = &g_pClientMgr->m_ObjectMgr.m_ObjectLists[i].m_Head;
        for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
            pObject = (LTObject*)pCur->m_pData;

            // Detach it from whatever it's standing on.
            DetachObjectStanding(pObject);
            DetachObjectsStandingOn(pObject);
        }
    }
    
    // Remove all world model objects.
    g_pClientMgr->RemoveObjectsInList(&g_pClientMgr->m_ObjectMgr.m_ObjectLists[OT_WORLDMODEL], false);

    g_pClientMgr->RemoveObjectsInList(&g_pClientMgr->m_ObjectMgr.m_ObjectLists[OT_CONTAINER], false);

    // Unbind all the worlds.
    UnbindWorlds();

    // Shut down the sounds.
    GetClientILTSoundMgrImpl()->StopAllSounds();
    
    // Close any open world files.
    world_bsp_client->Term();
}

LTRESULT CClientShell::DoLoadWorld(const CPacket_Read &cPacket, bool bLocal)
{
	CPacket_Read cLoadPacket(cPacket);
    int i;
    ILTStream *pStream;
    FileRef ref;
    FileIdentifier *pIdent;
    const char *pWorldName;
    ELoadWorldStatus loadStatus;


    // Cleanup...
    cs_UnloadWorld(this);

    
	// Tell the client shell to stop rendering for a sec..
	if (i_client_shell != NULL)
		i_client_shell->OnLockRenderer();

    // UnBind any textures we were using
    g_pClientMgr->UnbindSharedTextures(false);

    // Get the game time.
    m_GameTime = m_LastGameTime = cLoadPacket.Readfloat();
    m_GameFrameTime = 0.0f;
    
    m_ServerPeriodTrack = g_pClientMgr->m_CurTime;
    m_ServerPeriod = 1.0f / 30.0f;
    

    // Get the world file ID and world pointer.
    ref.m_FileType = FILE_SERVERFILE;
    ref.m_FileID = cLoadPacket.Readuint16();
    pWorldName = client_file_mgr->GetFilename(&ref);
    
    pIdent = client_file_mgr->GetFileIdentifier(&ref, TYPECODE_WORLD);

    // If we're local and it's the same world, don't reload all the textures.
    bool bFlushUnusedTextures = (pIdent != m_pLastWorld);

    // Get rid of unused Sprites before the world is loaded...
    if (bFlushUnusedTextures)
    {
        g_pClientMgr->TagAndFreeSprites();
    }

    //Init the prediction stuff with the server's time.
    pd_InitialServerUpdate(this, m_GameTime);

	// Load the client data
    pStream = client_file_mgr->OpenFile(&ref);
    if (!pStream)
    {
		// Tell the client shell it can render again..
		if (i_client_shell != NULL)
			i_client_shell->OnUnLockRenderer();

        RETURN_ERROR(1, CClientShell::DoLoadWorld, LT_MISSINGWORLDFILE);
    }

	//check if we have a local server.
    if (m_bLocal)
	{
        //inherit our client world from the server world.
        if (!world_bsp_client->InheritFromServer())
        {
			// Tell the client shell it can render again..
			if (i_client_shell != NULL)
				i_client_shell->OnUnLockRenderer();

            RETURN_ERROR_PARAM(1, ClientShell::DoLoadWorld, LT_ERROR, "Inherit failed");
        }

        loadStatus = world_bsp_client->LoadClientData(pStream);
    }
    else
    {
        // Notify the client shell and show the draw surface so they can put up a bitmap.
        if (i_client_shell != NULL) {
            i_client_shell->PreLoadWorld(pWorldName);
        }

        con_Printf(CONRGB(100,100,250), 1, "Entering world %s", client_file_mgr->GetFilename(&ref));
        
        loadStatus = world_bsp_client->Load(pStream);

		// Read the blind object data
		if (loadStatus == LoadWorld_Ok)
		{
			pStream->SeekTo( world_bsp_shared->GetBlindObjectDataPos() );
			loadStatus = g_iWorldBlindObjectData->Load(pStream);
		} 
    }

	// Release the stream, we're done loading
	pStream->Release();

    // Invalid file?
    if (loadStatus != LoadWorld_Ok)
    {
		// Tell the client shell it can render again..
		if (i_client_shell != NULL)
			i_client_shell->OnUnLockRenderer();

        g_pClientMgr->SetupError(LT_INVALIDWORLDFILE, pWorldName);
        RETURN_ERROR_PARAM(1, CClientShell::DoLoadWorld, LT_INVALIDWORLDFILE, pWorldName);
    }

    // Try to bind to the worlds.
    if (!BindWorlds()) {
		// Tell the client shell it can render again..
		if (i_client_shell != NULL)
			i_client_shell->OnUnLockRenderer();


        RETURN_ERROR(1, CClientShell::DoLoadWorld, LT_ERRORBINDINGWORLD);
    }


    // Get rid of unused shared textures.
    if (bFlushUnusedTextures)
    {
        g_pClientMgr->TagAndFreeTextures();
    }

    // Set poly texture pointers.
    world_bsp_client->SetWorldModelOriginalBSPPolyTexturePointers();

    // Bind textures so we're ready to go.
    g_pClientMgr->BindUnboundTextures();

	// Tell the client shell it can render again..
	if (i_client_shell != NULL)
		i_client_shell->OnUnLockRenderer();

    // Create objects for all the vis containers.
    if (!CreateVisContainerObjects())
    {
        CloseWorlds();
        RETURN_ERROR_PARAM(1, CClientShell::DoLoadWorld, LT_ERROR, "CreateVisContainerObjects failed");
    }
    
    // Add all objects back into the BSP.
    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        AddAllObjectsToBSP(&g_pClientMgr->m_ObjectMgr.m_ObjectLists[i]);
    }
    

    m_pLastWorld = pIdent;
          
    // Call the OnEnterWorld function.
    g_pClientMgr->OnEnterWorld(this);
    m_bWorldOpened = true;

    return LT_OK;
}

