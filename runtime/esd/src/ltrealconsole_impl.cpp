/****************************************************************************
;
;	MODULE:		LTRealConsole_impl (.CPP)
;
;	PURPOSE:	Real Console Manager class
;
;	HISTORY:	04/11/2000 [blg] This file was created
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD

// Includes...

#include "ltrealconsole_impl.h"
#include "rngcclientenginelib_i.c"
#include "irngameconsole_i.c"
#include "irngcclientengine_i.c"
#include "irngcmessage_i.c"

// Macros...

#define HANDLE_FAILURE_RESULT(hr, rt)	if (hr != S_OK) { SetLastError(hr); return(rt); } 
#define VERIFY_INIT						if (!m_bInited || !m_pConsole) { SetLastError(RNGC_NOCONSOLE); return(LT_NOTINITIALIZED); }
#define VERIFY_PARAM(p)					if (!p) { SetLastError(E_INVALIDARG); return(LT_INVALIDPARAMS); }


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::Init(GUID guidGameID)
{
	// Check if we are already initialized...

	if (m_bInited && m_pConsole)
	{
		HANDLE_FAILURE_RESULT(RNGC_IGNORE, LT_ALREADYINITIALIZED);
	}

	
	// Inialize the console...

	HRESULT hr = RNInitConsole(guidGameID, true, &m_pConsole);

	HANDLE_FAILURE_RESULT(hr, LT_ERROR);
	if (!m_pConsole) HANDLE_FAILURE_RESULT(RNGC_NOCONSOLE, LT_ERROR);


	// Flag that we are now initialized...

	m_bInited = LTTRUE;


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::Term()
{
	// If we not initialized, just return...

	if (!m_bInited)
	{
		return(LT_OK);
	}


	// Terminate the console...

	HRESULT hr = RNTermConsole(m_pConsole);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// Clear our members...

	Clear();


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::Clear
//
//	PURPOSE:	Clears all member variables
//
// ----------------------------------------------------------------------- //

void CLTRealConsoleMgr::Clear()
{
	// Clear our member variables...

	m_pConsole    = NULL;
	m_bInited     = FALSE;
	m_hrLastError = S_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::IsInitialized
//
//	PURPOSE:	Determines if the RealConsole manager is initialized
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::IsInitialized(bool* pInited)
{
	// Sanity checks...

	VERIFY_PARAM(pInited);


	// Set whether or not we are initialized...

	*pInited = m_bInited;


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::CreateMessage
//
//	PURPOSE:	Creates an empty message object
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::CreateMessage(IRNGCMessage** ppMessage)
{
	// Sanity checks...

	VERIFY_INIT;
	VERIFY_PARAM(ppMessage);


	// Create a new message object...

	HRESULT hr = RNCreateMessage(ppMessage);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::ReleaseMessage
//
//	PURPOSE:	Release a message that is finished being used
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::ReleaseMessage(IRNGCMessage* pMessage)
{
	// Sanity checks...

	VERIFY_PARAM(pMessage);


	// Release the message object...

	pMessage->Release();


	// All done...

	return(LT_OK);
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::PostMessage
//
//	PURPOSE:	Post a message to the console and return immediately
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::PostMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage)
{
	// Sanity checks...

	VERIFY_INIT;


	// Post the message...

	HRESULT hr = m_pConsole->PostMessage(nType, pMessage);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::SendMessage
//
//	PURPOSE:	Send a message to the console and wait for response or
//				timeout.
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::SendMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessageToSend, IRNGCMessage **ppMessageResponse, uint32 nTimeout)
{
	// Sanity checks...

	VERIFY_INIT;


	// Send the message...

	HRESULT hr = m_pConsole->SendMessage(nType, pMessageToSend, ppMessageResponse, nTimeout);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetNumMessages
//
//	PURPOSE:	Get the number of messages in the console message queue
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetNumMessages(uint32* pNumMessages)
{
	// Sanity checks...

	VERIFY_INIT;


	// Get the number of messages in the console's message queue...

	HRESULT hr = m_pConsole->GetNumMessages((long*)pNumMessages);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetNextMessage
//
//	PURPOSE:	Get the next message in the console message queue
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetNextMessage(RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout)
{
	// Sanity checks...

	VERIFY_INIT;


	// Get the next in the console's message queue...

	HRESULT hr = m_pConsole->GetNextMessage(pType, ppMessage, nTimeout);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetSpecificMessage
//
//	PURPOSE:	Get a specific message from the console message queue
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetSpecificMessage(RNGC_MESSAGE_TYPE nRequestedType, IRNGCMessage** ppMessage, uint32 nTimeout)
{
	// Sanity checks...

	VERIFY_INIT;


	// Get the requested message from the console's message queue...

	HRESULT hr = m_pConsole->GetSpecificMessage(nRequestedType, ppMessage, nTimeout);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetMessageInRange
//
//	PURPOSE:	Get a message within a givin range from the console
//				message queue.
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetMessageInRange(RNGC_MESSAGE_TYPE nFilterMin, RNGC_MESSAGE_TYPE nFilterMax, RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout)
{
	// Sanity checks...

	VERIFY_INIT;


	// Get a message within the given range from the console's message queue...

	HRESULT hr = m_pConsole->GetMessageInRange(nFilterMin, nFilterMax, pType, ppMessage, nTimeout);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetMessageDescription
//
//	PURPOSE:	Gets a text description for the given message type
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetMessageDescription(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage, char* szBuffer, uint32* pSizeOfBuffer)
{
	// Sanity checks...

	VERIFY_PARAM(szBuffer);
	VERIFY_PARAM(pSizeOfBuffer);


	// Get the console message description...

	HRESULT hr = RNGetMessageDescriptionA(m_pConsole, nType, pMessage, szBuffer, (unsigned long*)pSizeOfBuffer);
	if (hr == RNGC_BUFFERTOOSMALL) HANDLE_FAILURE_RESULT(hr, LT_BUFFERTOOSMALL);
	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetErrorDescription
//
//	PURPOSE:	Gets a text description for the given error code
//
// ----------------------------------------------------------------------- //

#define COPY_ERROR_DESC(hr, desc) case hr: { strncpy(szBuffer, desc, *pSizeOfBuffer); return(LT_OK); }

LTRESULT CLTRealConsoleMgr::GetErrorDescription(HRESULT hrError, char* szBuffer, uint32* pSizeOfBuffer)
{
	// Sanity checks...

	VERIFY_PARAM(szBuffer);
	VERIFY_PARAM(pSizeOfBuffer);


	// Get the console error description...

	HRESULT hr = RNGetErrorDescriptionA(m_pConsole, hrError, szBuffer, (unsigned long*)pSizeOfBuffer);
	if (hr == RNGC_BUFFERTOOSMALL) HANDLE_FAILURE_RESULT(hr, LT_BUFFERTOOSMALL);


	// If we didn't get an error description from the console, do our own...

	if (hr != S_OK)
	{
		switch (hr)
		{
			COPY_ERROR_DESC(RNGC_NOCONSOLE, "RNGC_NOCONSOLE");
			COPY_ERROR_DESC(RNGC_INVALID_IID, "RNGC_INVALID_IID");
			COPY_ERROR_DESC(RNGC_INVALID_LAUNCH, "RNGC_INVALID_LAUNCH");
			COPY_ERROR_DESC(RNGC_TOURNEY_OVER, "RNGC_TOURNEY_OVER");
			COPY_ERROR_DESC(RNGC_SERVER_UNREACHABLE, "RNGC_SERVER_UNREACHABLE");
			COPY_ERROR_DESC(RNGC_RESULTS_INVALID, "RNGC_RESULTS_INVALID");
			COPY_ERROR_DESC(RNGC_RESULTS_TOO_LATE, "RNGC_RESULTS_TOO_LATE");
			COPY_ERROR_DESC(RNGC_IGNORE, "RNGC_IGNORE");
			COPY_ERROR_DESC(RNGC_BUFFERTOOSMALL, "RNGC_BUFFERTOOSMALL");
			COPY_ERROR_DESC(RNGC_NOTFOUND, "RNGC_NOTFOUND");
			COPY_ERROR_DESC(RNGC_QUEUE_EMPTY, "RNGC_QUEUE_EMPTY");
			COPY_ERROR_DESC(RNGC_TIMED_OUT, "RNGC_TIMED_OUT");
			COPY_ERROR_DESC(RNGC_NOTINITIALIZED, "RNGC_NOTINITIALIZED");
			COPY_ERROR_DESC(S_OK, "S_OK");
		}
	}

	HANDLE_FAILURE_RESULT(hr, LT_ERROR);


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetLastErrorDescription
//
//	PURPOSE:	Gets a text description for the most recent error
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetLastErrorDescription(char* szBuffer, uint32* pSizeOfBuffer)
{
	// Let our main function do the real work...

	return(GetErrorDescription(m_hrLastError, szBuffer, pSizeOfBuffer));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::GetLastErrorResult
//
//	PURPOSE:	Gets the last error result code
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::GetLastErrorResult(HRESULT* pErrorResult)
{
	// Sanity checks...

	VERIFY_INIT;
	VERIFY_PARAM(pErrorResult);


	// Get the last error...

	*pErrorResult = m_hrLastError;


	// All done...

	return(LT_OK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTRealConsoleMgr::ProcessSystemMessage
//
//	PURPOSE:	Processes system messages
//
// ----------------------------------------------------------------------- //

LTRESULT CLTRealConsoleMgr::ProcessSystemMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage)
{
	// Check for system specific messages...

	switch (nType)
	{
		case RNGC_NOTIFY_ACTIVATED:
		{
			// TODO: Handle being activated
			return(LT_OK);
		}

		case RNGC_NOTIFY_DEACTIVATED:
		{
			// TODO: Handle being de-activated
			return(LT_OK);
		}

		case RNGC_DO_ACTIVATE:
		{
			// TODO: Activate
			return(LT_OK);
		}
	}


	// If we get here, it's not a system message...

	return(LT_ERROR);
}

#endif // LITHTECH_ESD
