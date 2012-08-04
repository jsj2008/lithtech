/****************************************************************************
;
;	MODULE:		LTRAudioQueue (.CPP)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltraudioqueue.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTAudioQueueElement member functions
//-----------------------------------------------------------------------------
CLTAudioQueueElement::CLTAudioQueueElement()
{
	m_pPtr = NULL;
	m_pNext = NULL;
}

//-----------------------------------------------------------------------------
CLTAudioQueueElement::~CLTAudioQueueElement()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioQueueElement::Init(void* pPtr)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueueElement::Init()");

	if (!pPtr)
		return LT_ERROR;

	m_pPtr = pPtr;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioQueueElement::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueueElement::Term()");

	return LT_OK;
}

//-----------------------------------------------------------------------------
// CLTAudioQueue member functions
//-----------------------------------------------------------------------------
CLTAudioQueue::CLTAudioQueue()
{
	m_pElementList = NULL;
}

//-----------------------------------------------------------------------------
CLTAudioQueue::~CLTAudioQueue()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioQueue::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueue::Init()");

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioQueue::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueue::Term()");

	// Clean up queue
	if (m_pElementList)
	{
		RMAAudioData* pMyData = NULL;
		Remove(&pMyData);
		while (pMyData)
		{
			pMyData->pData->Release();
			delete pMyData;
			pMyData = NULL;

			Remove(&pMyData);
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
void CLTAudioQueue::Add(void* pElement)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueue::Add()");

	ASSERT(pElement);

	CLTAudioQueueElement* pElem;
	LT_MEM_TRACK_ALLOC(pElem = new CLTAudioQueueElement,LT_MEM_TYPE_SOUND);
	ASSERT(pElem);
	pElem->Init(pElement);

	CLTAudioQueueElement*	pPtr = NULL;
	CLTAudioQueueElement**	ppPtr = NULL;

	for (ppPtr = &m_pElementList; NULL != (pPtr = *ppPtr); ppPtr = &pPtr->m_pNext);

	*ppPtr = pElem;
}

//-----------------------------------------------------------------------------
void CLTAudioQueue::Remove(RMAAudioData** ppPtr)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioQueue::Remove()");

	ASSERT(NULL == *ppPtr);

	if (!m_pElementList)
		return;

	CLTAudioQueueElement* pElem = m_pElementList;
	*ppPtr = (RMAAudioData*)pElem->m_pPtr;

	m_pElementList = m_pElementList->m_pNext;

	delete pElem;
}

#endif // LITHTECH_ESD