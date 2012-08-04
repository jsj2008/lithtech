#include "bdefs.h"

#define __SERVERAPI_EXPORT__

#include "server_interface.h"
#include "servermgr.h"
#include "s_concommand.h"
#include "s_client.h"
#include "impl_common.h"
#include "systimer.h"
#include "ltmessage.h"


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

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);



// ------------------------------------------------------------------- //
// Globals.
// ------------------------------------------------------------------- //
#if defined(_WIN32) || defined(__XBOX)
extern HINSTANCE *g_hResourceModule;
extern HINSTANCE g_hModuleInstanceHandle;
#endif

class CLTServerInterface : public ServerInterface
{
public:

        CLTServerInterface()
        {
            m_bUpdating = LTFALSE;
        }

        ~CLTServerInterface()
        {
        }

        virtual LTRESULT RunConsoleString(char *pStr);
        virtual LTRESULT GetConsoleVar(char *pName, HCONSOLEVAR *hVar, char *pDefaultVal);
        virtual LTRESULT GetVarValueFloat(HCONSOLEVAR hVar, float *val);
        virtual LTRESULT GetVarValueString(HCONSOLEVAR hVar, char *pStr, uint32 maxLen);
        virtual LTRESULT LoadConfigFile(char *pStr);
        virtual LTRESULT SaveConfigFile(char *pStr);
        virtual LTRESULT SetAppHandler(ServerAppHandler *pHandler);
		virtual LTRESULT SendToServerShell( ILTMessage_Read& msg );
        virtual bool	AddResources(const char **pResources, uint32 nResources);
		virtual bool	SetGameInfo( void *pGameInfo, uint32 nGameInfoLen );
		virtual bool	LoadBinaries( );
        virtual FileEntry*  GetFileList(char *pDirName);
        virtual void    FreeFileList(FileEntry *pList);
        virtual bool	StartWorld(StartGameRequest *pRequest);
        virtual int     GetNumClients();
        virtual bool	GetClientName(int index, char *pName, int maxChars);
        virtual bool	SetClientName(int index, char *pName, int maxChars);
        virtual bool	GetClientInfo(int index, ClientInfo* pInfo);
        virtual bool	BootClient(uint32 dwClientID);
		virtual bool	GetClientPing( uint32 nClientId, float &ping );
		virtual bool	GetClientAddr( uint32 nClientId, uint8 pAddr[4], uint16 *pPort);
        virtual int     GetErrorCode();
        virtual void    GetErrorString(char *pString, int maxLen);
        virtual bool	Update(long flags);

        virtual bool	InitNetworking(char *pDriver, uint32 dwFlags);
        virtual bool	GetServiceList(NetService *&pListHead);
        virtual bool	FreeServiceList(NetService *pListHead);
        virtual bool	SelectService(HNETSERVICE hNetService);
        virtual bool	UpdateSessionName(const char* sName);
        virtual bool	HostGame(NetHost* pHostInfo);
        virtual bool	GetTcpIpAddress(char* sAddress, uint32 dwBufferSize, unsigned short &hostPort);
        virtual LTRESULT    SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint32 port);

    LTBOOL              m_bUpdating;
};

CLTServerInterface g_ServerInterface;


class CSEntry
{
public:
            CSEntry(CLTServerInterface *pInterface)
            {
                while (pInterface->m_bUpdating)
#if defined(_WIN32) || defined(__XBOX)
                    Sleep(0);
#else
                    sleep(0);
#endif // _WIN32 || __XBOX
            }

            ~CSEntry()
            {
            }
#if defined(_WIN32) || defined(__XBOX)
    CRITICAL_SECTION *m_pSection;
#endif
};

#define CSENTRY CSEntry entry(CLTServerInterface *pInterface);


// ------------------------------------------------------------------- //
// ServerInterface implementation.
// ------------------------------------------------------------------- //

LTRESULT    CLTServerInterface::RunConsoleString(char *pStr)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::RunConsoleString, LT_NOTINITIALIZED);
    }

    return sm_HandleCommand(console_state->State(), pStr);
}

LTRESULT CLTServerInterface::GetConsoleVar(char *pName, HCONSOLEVAR *hVar, char *pDefaultVal)
{
    LTCommandVar *pVar;
    char temp[256];

	CSEntry entry(&g_ServerInterface);

    if (!hVar)
    {
        RETURN_ERROR(1, ServerInterface::GetConsoleVar, LT_INVALIDPARAMS);
    }

    *hVar = LTNULL;
    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::GetConsoleVar, LT_NOTINITIALIZED);
    }

    if (pVar = cc_FindConsoleVar(console_state->State(), pName))
    {
        *hVar = (HCONSOLEVAR)pVar;
        return LT_OK;
    }
    else if (pDefaultVal)
    {
        sprintf(temp, "%s \"%s\"", pName, pDefaultVal);
        cc_HandleCommand2(console_state->State(), temp, CC_NOCOMMANDS);
        if (pVar = cc_FindConsoleVar(console_state->State(), pName))
        {
            *hVar = (HCONSOLEVAR)pVar;
            return LT_FINISHED;
        }
        else
        {
            return LT_ERROR;
        }
    }
    else
    {
        return LT_NOTFOUND;
    }
}

LTRESULT CLTServerInterface::GetVarValueFloat(HCONSOLEVAR hVar, float *val)
{
    LTCommandVar *pVar;

	CSEntry entry(&g_ServerInterface);

    if (!hVar || !val)
    {
        RETURN_ERROR(1, ServerInterface::GetVarValueFloat, LT_INVALIDPARAMS);
    }

    pVar = (LTCommandVar*)hVar;
    *val = pVar->floatVal;
    return LT_OK;
}

LTRESULT CLTServerInterface::GetVarValueString(HCONSOLEVAR hVar, char *pStr, uint32 maxStringBytes)
{
    LTCommandVar *pVar;

	CSEntry entry(&g_ServerInterface);

    if (!hVar || !pStr || maxStringBytes == 0)
    {
        RETURN_ERROR(1, ServerInterface::GetVarValueString, LT_INVALIDPARAMS);
    }

    pVar = (LTCommandVar*)hVar;
    strncpy(pStr, pVar->pStringVal, maxStringBytes-1);
    pStr[maxStringBytes-1] = 0;
    return LT_OK;
}

LTRESULT CLTServerInterface::LoadConfigFile(char *pFilename)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::LoadConfigFile, LT_NOTINITIALIZED);
    }

    return cc_RunConfigFile(console_state->State(), pFilename, 0, VARFLAG_SAVE) ? LT_OK : LT_ERROR;
}

LTRESULT CLTServerInterface::SaveConfigFile(char *pFilename)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::SaveConfigFile, LT_NOTINITIALIZED);
    }

    return cc_SaveConfigFile(console_state->State(), pFilename) ? LT_OK : LT_ERROR;
}

LTRESULT CLTServerInterface::SetAppHandler(ServerAppHandler *pHandler)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::SetAppHandler, LT_NOTINITIALIZED);
    }

    g_pServerMgr->m_pServerAppHandler = pHandler;
    return LT_OK;
}

LTRESULT    CLTServerInterface::SendToServerShell( ILTMessage_Read& msg )
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        RETURN_ERROR(1, ServerInterface::SendToServerShell, LT_NOTINITIALIZED);
    }

    if (i_server_shell != NULL)
    {
        return i_server_shell->ServerAppMessageFn( msg );
    }
    else
    {
        return LT_NOTFOUND;
    }
}

bool CLTServerInterface::AddResources(const char **pResources, uint32 nResources)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return false;

    return g_pServerMgr->AddResources(pResources, nResources);
}

bool CLTServerInterface::SetGameInfo( void *pGameInfo, uint32 nGameInfoLen )
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return false;

	g_pServerMgr->SetGameInfo( pGameInfo, nGameInfoLen );

	return true;
}


bool CLTServerInterface::LoadBinaries( )
{
	CSEntry entry(&g_ServerInterface);

	if(!g_pServerMgr)
		return false;

	return g_pServerMgr->LoadBinaries( );
}

FileEntry*  CLTServerInterface::GetFileList(char *pDirName)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return server_filemgr->GetFileList(pDirName);
}

void CLTServerInterface::FreeFileList(FileEntry *pList)
{
	CSEntry entry(&g_ServerInterface);

    ic_FreeFileList(pList);
}

bool CLTServerInterface::StartWorld(StartGameRequest *pRequest)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    // This assumes AddResouces has been called.

    g_pServerMgr->SetGameInfo(pRequest->m_pGameInfo, pRequest->m_GameInfoLen);
    return g_pServerMgr->DoStartWorld(pRequest->m_WorldName, LOADWORLD_LOADWORLDOBJECTS|LOADWORLD_RUNWORLD, time_GetMSTime()) == LT_OK;
}

bool CLTServerInterface::HostGame(NetHost* pHostInfo)
{
    CBaseDriver* pDriver;

	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    if (!pHostInfo)
        return LTFALSE;

    pDriver = g_pServerMgr->m_NetMgr.GetMainDriver();
    if (!pDriver)
        return LTFALSE;

    if (pDriver->HostSession(pHostInfo) != LT_OK)
        return LTFALSE;

    return LTTRUE;
}

int CLTServerInterface::GetNumClients()
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return 0;

    return g_pServerMgr->GetNumClients();
}

bool CLTServerInterface::GetClientName(int index, char *pName, int maxChars)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return g_pServerMgr->GetPlayerName(index, pName, maxChars);
}

bool CLTServerInterface::SetClientName(int index, char *pName, int maxChars)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return g_pServerMgr->SetPlayerName(index, pName, maxChars);
}

bool CLTServerInterface::GetClientInfo(int index, ClientInfo* pInfo)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return g_pServerMgr->GetPlayerInfo(index, pInfo);
}

bool CLTServerInterface::BootClient(uint32 dwClientID)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return g_pServerMgr->BootPlayer(dwClientID);
}

bool CLTServerInterface::GetClientPing( uint32 nClientId, float &ping )
{
	ping = 0.0f;
	Client *pClient = g_pServerMgr->GetClientFromId( nClientId );
	if( pClient && pClient->m_ConnectionID )
	{
		ping = pClient->m_ConnectionID->GetPing( );
		return true;
	}
	else
	{
		DEBUG_PRINT(1, ( "ServerInterface::GetClientPing: Invalid client id." ));
		return false;
	}
}


bool CLTServerInterface::GetClientAddr( uint32 nClientId, uint8 pAddr[4], uint16 *pPort)
{
	Client *pClient = g_pServerMgr->GetClientFromId( nClientId );
	if( pClient && pClient->m_ConnectionID )
	{
		if( !pClient->m_ConnectionID->GetIPAddress(pAddr, pPort))
			return false;

		return true;
	}
	else
	{
		DEBUG_PRINT(1, ( "ServerInterface::GetClientAddr: Invalid client id." ));
		return false;
	}
}




bool CLTServerInterface::GetTcpIpAddress(char* sAddress, uint32 dwBufferSize, unsigned short &hostPort)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return LTFALSE;

    return g_pServerMgr->m_NetMgr.GetLocalIpAddress(sAddress, dwBufferSize, hostPort) == LT_OK;
}

int CLTServerInterface::GetErrorCode()
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
        return -1;

    return g_pServerMgr->GetErrorCode();
}

void CLTServerInterface::GetErrorString(char *pString, int maxLen)
{
	CSEntry entry(&g_ServerInterface);

    if (!g_pServerMgr)
    {
        pString[0] = 0;
        return;
    }

    g_pServerMgr->GetErrorString(pString, maxLen);
}

bool CLTServerInterface::Update(long flags)
{
    bool bRet;

    //CSEntry entry(&g_ServerInterface);

    ASSERT(!m_bUpdating);
    if (!g_pServerMgr)
        return LTFALSE;

    m_bUpdating = LTTRUE;
    bRet = g_pServerMgr->Update(flags, time_GetMSTime());
    m_bUpdating = LTFALSE;

    return bRet;
}

bool CLTServerInterface::InitNetworking(char *sDriver, uint32 dwFlags)
{
	CSEntry entry(&g_ServerInterface);

    // Sanity checks...
    if (!g_pServerMgr)
        return LTFALSE;

    // Make sure no drivers are loaded.
    g_pServerMgr->m_NetMgr.InitDrivers();

    // All done.
    return LTTRUE;
}

bool CLTServerInterface::GetServiceList(NetService*&pListHead)
{
	CSEntry entry(&g_ServerInterface);

    return g_pServerMgr->m_NetMgr.GetServiceList(pListHead) == LT_OK;
}

bool CLTServerInterface::FreeServiceList(NetService *pListHead)
{
	CSEntry entry(&g_ServerInterface);

    return g_pServerMgr->m_NetMgr.FreeServiceList(pListHead) == LT_OK;
}

bool CLTServerInterface::SelectService(HNETSERVICE hNetService)
{
	CSEntry entry(&g_ServerInterface);

    return g_pServerMgr->m_NetMgr.SelectService(hNetService) == LT_OK;
}

bool CLTServerInterface::UpdateSessionName(const char* sName)
{
	CSEntry entry(&g_ServerInterface);

    return g_pServerMgr->m_NetMgr.SetSessionName(sName) == LT_OK;
}

LTRESULT    CLTServerInterface::SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint32 port)
{
    uint32 i;
    CBaseDriver *pDriver;

	CSEntry entry(&g_ServerInterface);

    for (i=0; i < g_pServerMgr->m_NetMgr.m_Drivers; i++)
    {
        pDriver = g_pServerMgr->m_NetMgr.m_Drivers[i];

        if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
        {
			CLTMessage_Read *pReadMsg = reinterpret_cast<CLTMessage_Read *>(pMsg);
            return pDriver->SendTcpIp(pReadMsg->GetPacket(), sAddr, port);
        }
    }

    return LT_NOTINITIALIZED;
}

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef _WIN32

BOOL WINAPI DllEntryPoint(HINSTANCE hinstDLL, uint32 fdwReason, LPVOID lpvReserved)
{
    g_hModuleInstanceHandle = hinstDLL;
    return TRUE;
}

#endif


SERVERAPI SI_CREATESTATUS CreateServer(int version, LTGUID &appGuid, ServerInterface **ppServer)
{
    int status;

    *ppServer = LTNULL;

    // Make sure it isn't already active.
    if (g_pServerMgr != LTNULL)
    {
        return SI_ALREADYINSTANCED;
    }

    // Correct version?
    if (version != SI_VERSION)
    {
        return SI_INVALIDVERSION;
    }


    status = dsi_Init();
    if (status != 0)
    {
        if (status == 1)
            return SI_CANTLOADRESOURCEMODULE;
        else
            return SI_ERRORINITTING;
    }

    LT_MEM_TRACK_ALLOC(g_pServerMgr = new CServerMgr,LT_MEM_TYPE_MISC);
    if (!g_pServerMgr) {
        dsi_Term();
        return SI_ERRORINITTING;
    }

    if (g_pServerMgr->Init())
    {
        g_pServerMgr->m_NetMgr.m_guidApp = appGuid;
        *ppServer = &g_ServerInterface;
        return SI_OK;
    }
    else
    {
        delete g_pServerMgr;
        g_pServerMgr = LTNULL;

        dsi_Term();
        return SI_ERRORINITTING;
    }
}


SERVERAPI void DeleteServer()
{
    if (g_pServerMgr)
    {
        g_pServerMgr->Term();
        delete g_pServerMgr;
        g_pServerMgr = LTNULL;

        dsi_Term();
    }
}

#ifdef __cplusplus
}
#endif
