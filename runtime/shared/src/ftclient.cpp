#include "bdefs.h"

#include "ftclient.h"
#include "packet.h"
#include "ftbase.h"
#include "netmgr.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);







// ----------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------- //

struct FTClient
{
    // The current file we're transferring.
    void            *m_pCurFile;
    uint16          m_CurFileID;
    
    // All the function pointers.
    FTCInitStruct   m_Init;

    void            *m_pUserData1;
};

// Only used by the clienthack stuff.
static FTClient *g_pFTClient=LTNULL;


// ----------------------------------------------------------------------- //
// Interface functions.
// ----------------------------------------------------------------------- //

FTClient *ftc_Init(FTCInitStruct *pStruct)
{
    FTClient *pClient;
    
    LT_MEM_TRACK_ALLOC(pClient = (FTClient*)dalloc(sizeof(FTClient)),LT_MEM_TYPE_MISC);
    if (pClient)
    {
        memset(pClient, 0, sizeof(FTClient));
        memcpy(&pClient->m_Init, pStruct, sizeof(FTCInitStruct));
        g_pFTClient = pClient;
    }
    
    return pClient; 
}


void ftc_Term(FTClient *pClient)
{
    if (!pClient)
        return;

    dfree(pClient);
    g_pFTClient = LTNULL;
}


void* ftc_GetUserData1(FTClient *pClient)
{
    if (!pClient)
        return LTNULL;

    return pClient->m_pUserData1;
}


void ftc_SetUserData1(FTClient *pClient, void *pUser)
{
    if (pClient)
        pClient->m_pUserData1 = pUser;
}


void ftc_Update(FTClient *hClient)
{
}


void ftc_ProcessPacket(FTClient *pClient, const CPacket_Read &cPacket)
{
    if (!pClient)
        return;

	CPacket_Read cPacket_Incoming(cPacket);

	cPacket_Incoming.SeekTo(0);
	
	if (cPacket_Incoming.Readuint8() != STC_FILEDESC)
		return;

	CPacket_Write cPacket_Response;
	bool bRespond = false;
	cPacket_Response.Writeuint8(CTS_FILESTATUS);

	while (!cPacket_Incoming.EOP())
	{
		uint32 nFileID = cPacket_Incoming.Readuint16();
		uint32 nFileSize = cPacket_Incoming.Readuint32();
		char aFileName[MAX_PATH];
		cPacket_Incoming.ReadString(aFileName, sizeof(aFileName));

        if (client_file_mgr->OnNewFile(pClient, aFileName, nFileSize, nFileID) == NF_HAVEFILE)
        {
            nFileID |= 0x8000;
        }

        cPacket_Response.Writeuint16((uint16)nFileID);
		bRespond = true;
    }

	if (bRespond)
    {
		pClient->m_Init.m_pNetMgr->SendPacket(CPacket_Read(cPacket_Response), pClient->m_Init.m_ConnID);
    }
}


