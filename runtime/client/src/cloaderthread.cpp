
#include "bdefs.h"
#include "cloaderthread.h"
#include "clientmgr.h"
#include "setupobject.h"


extern int32 g_CV_DebugLoaders;


CLoaderThread::CLoaderThread()
{

}


bool CLoaderThread::IsLoadingFile(FileIdentifier *pIdent)
{
	CSAccess cs(&m_Incoming.m_MessageCS);
	CSAccess cs2(&m_Outgoing.m_MessageCS);
	GPOS pos;

	ASSERT(!IsInThisThread());

	// Check all our messages.
	for(pos=m_Incoming.m_Messages; pos; )
	{
		if(m_Incoming.m_Messages.GetNext(pos)->m_Data[1].m_pData == pIdent)
			return LTTRUE;
	}

	for(pos=m_Outgoing.m_Messages; pos; )
	{
		if(m_Outgoing.m_Messages.GetNext(pos)->m_Data[1].m_pData == pIdent)
			return LTTRUE;
	}

	return LTFALSE;
}


LTRESULT CLoaderThread::WaitForFile(FileIdentifier *pIdent, void **ppObj)
{
	LThreadMessage msg;

	*ppObj = LTNULL;

	// Get messages until it's done.
	while(m_Outgoing.GetMessage(msg, LTTRUE) == LT_OK)
	{
		// Give it to the server..
		// t.f fix
		//if(g_pClientMgr)
			//g_pClientMgr->ProcessThreadMessage(msg);

		// See if it's our file.
		if(msg.m_Data[1].m_pData == pIdent)
		{
			if(msg.m_ID == CLT_LOADEDFILE)
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

	RETURN_ERROR(1, CLoaderThread::WaitForFile, LT_ERROR);
}


void CLoaderThread::ProcessMessage(LThreadMessage &msg)
{
	uint32 resType;
	FileIdentifier *pIdent;


	if(msg.m_ID == CLT_LOADFILE)
	{
		resType = msg.m_Data[0].m_dwData;
		pIdent = (FileIdentifier*)msg.m_Data[1].m_pData;

		switch(resType)
		{
			case FT_MODEL:
			{
				// t.f deprecate
				//LoadModel(pIdent);
			}
			break;
		}
	}
}


void CLoaderThread::LoadModel(FileIdentifier *pIdent)
{
	//Model *pModel;
	LThreadMessage msg;
	//LTRESULT dResult;

	if(g_CV_DebugLoaders)
	{
		dsi_ConsolePrint("CLoaderThread::LoadModel(%s)", pIdent->m_Filename);
	}
	assert(0); // we should never get here.
	return;
	//dResult = g_pClientMgr->LoadModelData(pIdent->m_Filename, pIdent, pModel);
	//if(dResult == LT_OK)
	//{
		//msg.m_ID = CLT_LOADEDFILE;
	//}
	//else
	//{
		//msg.m_ID = CLT_LOADERROR;
	//}
//
	//msg.m_Data[0].m_dwData = FT_MODEL;
	//msg.m_Data[1].m_pData = pIdent;
	//msg.m_Data[2].m_pData = pModel;
	m_Outgoing.PostMessage(msg);
}


