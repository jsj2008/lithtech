#include "bdefs.h"

#include "sloaderthread.h"
#include "servermgr.h"
#include "server_extradata.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);





SLoaderThread::SLoaderThread()
{

}


LTRESULT SLoaderThread::WaitForFile(UsedFile *pFile, void **ppObj)
{
	LThreadMessage msg;

	*ppObj = LTNULL;

	// no longer valid.
	assert(0);
	// Get messages until it's done.
	while(m_Outgoing.GetMessage(msg, LTTRUE) == LT_OK)
	{
		// Give it to the server..
		//if(g_pServerMgr)
			//g_pServerMgr->ProcessThreadMessage(msg);

		// See if it's our file.
		if(msg.m_Data[1].m_pData == pFile)
		{
			if(msg.m_ID == SLT_LOADEDFILE)
			{
				*ppObj = msg.m_Data[2].m_pData;
				return LT_OK;
			}
			else
			{
				return LT_ERROR;
			}
		}
	}

	RETURN_ERROR(1, SLoaderThread::WaitForFile, LT_ERROR);
}


bool SLoaderThread::IsLoadingFile(UsedFile *pFile)
{
	CSAccess cs(&m_Incoming.m_MessageCS);
	CSAccess cs2(&m_Outgoing.m_MessageCS);
	GPOS pos;
	
	ASSERT(!IsInThisThread());
	if(!g_pServerMgr)
		return LTFALSE;

	// Check all our messages.
	for(pos=m_Incoming.m_Messages; pos; )
	{
		if(m_Incoming.m_Messages.GetNext(pos)->m_Data[1].m_pData == pFile)
			return LTTRUE;
	}

	for(pos=m_Outgoing.m_Messages; pos; )
	{
		if(m_Outgoing.m_Messages.GetNext(pos)->m_Data[1].m_pData == pFile)
			return LTTRUE;
	}
	
	return LTFALSE;
}


void SLoaderThread::ProcessMessage(LThreadMessage &msg)
{

	LThreadMessage result;
	// t.f fix
	
	if(!g_pServerMgr || msg.m_ID != SLT_LOADFILE)
		return;

	

	if(msg.m_Data[0].m_dwData == FT_MODEL)
	{
		assert(0);
		// we don't thread load models.
	}
}




