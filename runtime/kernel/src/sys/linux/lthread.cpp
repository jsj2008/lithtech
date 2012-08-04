
#include "bdefs.h"
#include "lthread.h"


#define MyCS() (*((CRITICAL_SECTION*)&m_CS))

// How long the threads sleep between checking messages.
#define THREAD_SLEEP_INTERVAL 20

// Arbitrary.. hope we don't get a thread with this ID!
#define LCS_INVALIDTHREADID	0xFECDAEDC


static ObjectBank<LThreadMessage> g_LThreadMessageBank;
static LCriticalSection g_MessageBankCS; // Controls access to it.

// ------------------------------------------------------------------------------ //
// LCriticalSection
// ------------------------------------------------------------------------------ //

bool LCriticalSection::IsValid()
{
	return LTTRUE;
}

void LCriticalSection::Enter()
{
	// [dlj] removed because criticals can lock more than once per thread 
//	#ifdef _DEBUG
//		ASSERT(m_CurThreadID != pthread_self());
//	#endif

	super::Lock();
	
//	#ifdef _DEBUG
//		m_CurThreadID = pthread_self();
//	#endif
}

void LCriticalSection::Leave()
{
//	#ifdef _DEBUG
//		ASSERT(m_CurThreadID == pthread_self());
//	#endif

	super::Unlock();

//	#ifdef _DEBUG
//		m_CurThreadID = LCS_INVALIDTHREADID;
//	#endif
}


// ------------------------------------------------------------------------------ //
// LThreadMessage
// ------------------------------------------------------------------------------ //

LThreadMessage::LThreadMessage()
{
	m_ID = 0;
	m_pSender = LTNULL;
}


// ------------------------------------------------------------------------------ //
// LThreadQueue functions.
// ------------------------------------------------------------------------------ //

LThreadQueue::LThreadQueue() {}

LThreadQueue::~LThreadQueue() {}

LTRESULT LThreadQueue::Init()
{
	// This must be initialized for this to work.
	if(!g_MessageBankCS.IsValid() || !m_MessageCS.IsValid())
	{
		RETURN_ERROR(1, LThreadQueue::Init, LT_NOTINITIALIZED);
	}

	Clear();
	return LT_OK;
}


void LThreadQueue::Clear()
{
	GDeleteAndRemoveElementsOB(m_Messages, g_LThreadMessageBank);
}


LTRESULT LThreadQueue::PostMessage(LThreadMessage &msg)
{
	LThreadMessage *pToAdd;

	if(!g_MessageBankCS.IsValid())
	{
		RETURN_ERROR(1, LThreadQueue::PostMessage, LT_NOTINITIALIZED);
	}

	m_MessageCS.Enter();
	g_MessageBankCS.Enter();
		pToAdd = g_LThreadMessageBank.Allocate();
		if(pToAdd)
		{
			*pToAdd = msg;
			m_Messages.AddTail(pToAdd);
		}
	g_MessageBankCS.Leave();
	m_MessageCS.Leave();

	if(pToAdd)
	{	
		return LT_OK;
	}
	else
	{
		RETURN_ERROR(1, LThreadQueue::PostMessage, LT_OUTOFMEMORY);
	}
}


uint32 LThreadQueue::GetMessageCount()
{
	uint32 count;

	m_MessageCS.Enter();
		count = m_Messages.GetCount();
	m_MessageCS.Leave();
	return count;
}


LTRESULT LThreadQueue::GetMessage(LThreadMessage &msg, bool bWait)
{
	LThreadMessage *pMsg;


	if(GetMessageCount() == 0)
	{
		if(bWait)
		{
			// Wait for a message to come in.
			while(GetMessageCount() == 0)
			{
				// Sleep(2);
			}
		}
		else
		{
			return LT_NOTFOUND;
		}
	}

	if(!g_MessageBankCS.IsValid())
	{
		RETURN_ERROR(1, LThreadQueue::GetMessage, LT_NOTINITIALIZED);
	}

	m_MessageCS.Enter();
		pMsg = m_Messages.GetHead();
		msg = *pMsg;
		m_Messages.RemoveAt(pMsg);
	m_MessageCS.Leave();

	// Free the message.
	g_MessageBankCS.Enter();
		g_LThreadMessageBank.Free(pMsg);
	g_MessageBankCS.Leave();

	return LT_OK;
}


LTRESULT LThreadQueue::PeekMessage(LThreadMessage &msg)
{
	LThreadMessage *pMsg;

	if(m_Messages.GetCount() == 0)
	{
		return LT_NOTFOUND;
	}

	m_MessageCS.Enter();
		pMsg = m_Messages.GetHead();
		msg = *pMsg;
	m_MessageCS.Leave();

	return LT_OK;
}


bool LThreadQueue::PopMessage(LThreadMessage *pMsg)
{
	CSAccess cs(&m_MessageCS);
	LThreadMessage *pMessage;

	if (m_Messages.GetCount() >= 1)
	{
		pMessage = m_Messages.GetHead();
		m_Messages.RemoveAt(pMessage);
		
		g_MessageBankCS.Enter();
			g_LThreadMessageBank.Free(pMessage);
		g_MessageBankCS.Leave();

		return LTTRUE;
	}

	return LTFALSE;
}


// ------------------------------------------------------------------------------ //
// LThread functions.
// ------------------------------------------------------------------------------ //

LThread::LThread()
{
	m_ThreadID = 0;
	m_bRunning = FALSE;
	m_bTerminate = FALSE;
}


LThread::~LThread()
{
	// We shouldn't be running at this point.
	ASSERT(!m_bRunning);
}


ESTDLTResults LThread::Term()
{
	ASSERT(IsInThisThread());
	ASSERT(!m_bRunning);

	// If we're not running, then our thread handle is no longer valid.
	// MyHandle() = NULL;
	m_ThreadID = 0;

	// Clear our message queues.
	m_Incoming.Clear();
	m_Outgoing.Clear();

	return super::Term();
}


LTRESULT LThread::Start(const EThreadPriority& pri)
{
	ASSERT(!m_bRunning);

	if(m_Incoming.Init() != LT_OK || m_Outgoing.Init() != LT_OK)
	{
		m_Incoming.Clear();
		m_Outgoing.Clear();
		return LT_ERROR;
	}

	ESTDLTResults rslt = super::Run(pri);
	if (rslt != STDLT_OK)
	{
		RETURN_ERROR(1, LThread::Start, LT_ERROR);
	}
	
	m_bRunning = true;
	m_bTerminate = false;
	return LT_OK;
}


LTRESULT LThread::Pause()
{
	ASSERT(m_bRunning);
	ASSERT(!IsInThisThread());

	super::Sleep();
	return LT_OK;
}


LTRESULT LThread::Resume()
{
	ASSERT(m_bRunning);
	ASSERT(!IsInThisThread());

	super::Wakeup();
	return LT_OK;
}


LTRESULT LThread::Terminate(bool bWait)
{
//	if (IsThreadActive())
//	{
		m_bTerminate = true;

		if (bWait)
			WaitForTerm();
//	}

	return LT_OK;
}


ESTDLTResults LThread::ThreadInit()
{
	ESTDLTResults rslt = super::ThreadInit();
	if (rslt != STDLT_OK)
		return rslt;

	m_bRunning = true;
	return STDLT_OK;
}


void LThread::ThreadRun()
{
	LThreadMessage msg;

	while(1)
	{
		ASSERT(m_bRunning);

		// Is there a new message?
		if(m_Incoming.PeekMessage(msg) == LT_OK)
		{
			ProcessMessage(msg);

			// Pop off the one we just read.
			m_Incoming.PopMessage();
		}
		else
		{
			dsi_Sleep(THREAD_SLEEP_INTERVAL); // Sleep for a bit..
		}

		// Should we shutdown?
		if(m_bTerminate)
		{
			return;
		}
	}
}


ESTDLTResults LThread::ThreadTerm()
{
	m_bRunning = false;
	return super::ThreadTerm();
}


void LThread::ProcessMessage(LThreadMessage &msg)
{
}

