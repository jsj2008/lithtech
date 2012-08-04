
#include "bdefs.h"
#include "lthread.h"


// How long the threads sleep between checking messages.
#define THREAD_SLEEP_INTERVAL 20

// Arbitrary.. hope we don't get a thread with this ID!
#define LCS_INVALIDTHREADID	0xFECDAEDC


static ObjectBank<LThreadMessage> g_LThreadMessageBank;
static LCriticalSection g_MessageBankCS; // Controls access to it.

// ------------------------------------------------------------------------------ //
// Used to access the Windows objects from the system-independent ones in the header.
// ------------------------------------------------------------------------------ //

inline HANDLE& ToHandle(uint32 &nHandle)
{
	return *((HANDLE*)&nHandle);
}



// ------------------------------------------------------------------------------ //
// LCriticalSection
// ------------------------------------------------------------------------------ //

bool LCriticalSection::IsValid()
{
	return LTTRUE;
}

void LCriticalSection::Enter()
{
	super::Lock();
}

void LCriticalSection::Leave()
{
	super::Unlock();
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

LThreadQueue::LThreadQueue()
{
	// Create ourselves a message event
	ToHandle(m_hMsgEvent) = CreateEvent(NULL, TRUE, FALSE, NULL);
}


LThreadQueue::~LThreadQueue()
{
	Clear();
	// Delete the message event
	CloseHandle(ToHandle(m_hMsgEvent));
}


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
	// Pulse the message event in case someone's waiting on it
	PulseEvent(ToHandle(m_hMsgEvent));
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
		// Signal to ourself that we've got a message
		SetEvent(ToHandle(m_hMsgEvent));
		return LT_OK;
	}
	else
	{
		RETURN_ERROR(1, LThreadQueue::PostMessage, LT_OUTOFMEMORY);
	}
}


uint32 LThreadQueue::GetMessageCount()
{
	// Shortcut the CS entry if possible by checking the message event
	if (WaitForSingleObject(ToHandle(m_hMsgEvent), 0) == WAIT_TIMEOUT)
		return 0;

	uint32 count;

	m_MessageCS.Enter();
		count = m_Messages.GetCount();
	m_MessageCS.Leave();
	return count;
}


LTRESULT LThreadQueue::GetMessage(LThreadMessage &msg, bool bWait)
{
	// Do we have a message?
	if(WaitForSingleObject(ToHandle(m_hMsgEvent), (bWait) ? INFINITE : 0) == WAIT_TIMEOUT)
		return LT_NOTFOUND;

	// Get the message
	if (!PopMessage(&msg))
		return LT_NOTFOUND;

	return LT_OK;
}


LTRESULT LThreadQueue::PeekMessage(LThreadMessage &msg)
{
	LThreadMessage *pMsg;

	if(WaitForSingleObject(ToHandle(m_hMsgEvent), 0) == WAIT_TIMEOUT)
	{
		return LT_NOTFOUND;
	}

	// Protect our message access
	CSAccess cs(&m_MessageCS);

	// Jump out if the queue's actually empty
	if (m_Messages.IsEmpty())
		return LT_NOTFOUND;

	pMsg = m_Messages.GetHead();
	msg = *pMsg;

	return LT_OK;
}


bool LThreadQueue::PopMessage(LThreadMessage *pMsg)
{
	// Don't do anything if there's nothing in the queue
	if(WaitForSingleObject(ToHandle(m_hMsgEvent), 0) == WAIT_TIMEOUT)
		return LTFALSE;

	// Protect our message access
	CSAccess cs(&m_MessageCS);

	// Reset the message event and skip out if there isn't a message
	if (m_Messages.IsEmpty())
	{
		ResetEvent(ToHandle(m_hMsgEvent));
		return LTFALSE;
	}

	// Get the head message
	LThreadMessage *pHeadMessage = m_Messages.GetHead();
	// Copy it out if necessary
	if (pMsg)
		*pMsg = *pHeadMessage;
	// Remove it from the list
	m_Messages.RemoveAt(pHeadMessage);

	// Remove it from the message bank
	g_MessageBankCS.Enter();
		g_LThreadMessageBank.Free(pHeadMessage);
	g_MessageBankCS.Leave();

	// Reset the message event if the queue's empty
	if (m_Messages.IsEmpty())
		ResetEvent(ToHandle(m_hMsgEvent));

	return LTTRUE;
}


// ------------------------------------------------------------------------------ //
// LThread functions.
// ------------------------------------------------------------------------------ //

LThread::LThread()
{
}


LThread::~LThread()
{
}


ESTDLTResults LThread::Term()
{
	ASSERT(IsInThisThread());

	// Clear our message queues.
	m_Incoming.Clear();
	m_Outgoing.Clear();
	return super::Term();
}


LTRESULT LThread::Start(const EThreadPriority& pri)
{
	if(m_Incoming.Init() != LT_OK || m_Outgoing.Init() != LT_OK)
	{
		m_Incoming.Clear();
		m_Outgoing.Clear();
		return LT_ERROR;
	}

	// Create the stop event
	ToHandle(m_hStopEvent) = CreateEvent(NULL, TRUE, FALSE, NULL);

	ESTDLTResults rslt = super::Run(pri);
	if (rslt != STDLT_OK)
	{
		RETURN_ERROR(1, LThread::Start, LT_ERROR);
	}
	return LT_OK;
}


LTRESULT LThread::Pause()
{
	ASSERT(!IsInThisThread());
	
	super::Sleep();
	return LT_OK;
}


LTRESULT LThread::Resume()
{
	ASSERT(!IsInThisThread());

	super::Wakeup();
	return LT_OK;
}


LTRESULT LThread::Terminate(bool bWait)
{
	if(IsThreadActive())
	{
		// Signal the stop
		SetEvent(ToHandle(m_hStopEvent));

		// Wait for the thread to stop
		WaitForTerm();
		
		// Close our handles
		CloseHandle(ToHandle(m_hStopEvent));
		m_hStopEvent = NULL;
	}

	return LT_OK;
}


void LThread::ThreadRun()
{
	LThreadMessage msg;

	// The events we're going to wait for (Stop/Message)
	HANDLE aWaitEvents[2];
	aWaitEvents[0] = ToHandle(m_hStopEvent);
	aWaitEvents[1] = ToHandle(m_Incoming.GetMsgEvent());

	// Loop until we're told to stop
	while (WaitForMultipleObjects(2, aWaitEvents, FALSE, INFINITE) != WAIT_OBJECT_0)
	{
		// If we got here, we were signaled with a message

		// Is there a new message?
		if(m_Incoming.PeekMessage(msg) == LT_OK)
		{
			// Pop off the one we just read.
			m_Incoming.PopMessage();

			// Process it
			ProcessMessage(msg);
		}
		else
		{
			// Ok, maybe there really wasn't a message...
		}

	}
}

void LThread::ProcessMessage(LThreadMessage &msg)
{
}

bool LThread::ShouldTerminate()
{
	return WaitForSingleObject(ToHandle(m_hStopEvent), 0) == WAIT_TIMEOUT;
}
