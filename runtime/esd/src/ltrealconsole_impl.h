/****************************************************************************
;
;	MODULE:		LTRealConsole_impl (.H)
;
;	PURPOSE:	Real Console Manager class
;
;	HISTORY:	04/11/2000 [blg] This file was created
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/
#ifdef LITHTECH_ESD
#pragma once


// Includes...

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC
#include "iltesd.h"
#include "..\..\..\externalesd\realconsole\include\rngclib.h"


// Classess...

class CLTRealConsoleMgr : public ILTRealConsoleMgr
{
	// Member functions...

public:

	// Constructor.
	CLTRealConsoleMgr() { Clear(); }

	// Destructor.
	virtual ~CLTRealConsoleMgr() { Term(); }

	// Initialize the console.  This function must be called first.
	virtual	LTRESULT		Init(GUID guidGameID);

	// Terminate the console.  This function must be called last.
	virtual	LTRESULT		Term();

	// Determines if the RealConsole manager is initialized.
	virtual	LTRESULT		IsInitialized(bool* pInited);

	// Create an empty message object.
	virtual LTRESULT		CreateMessage(IRNGCMessage** ppMessage);

	// Release a message that is finished being used.
	virtual LTRESULT		ReleaseMessage(IRNGCMessage* pMessage);

	// Post a message to the console and return immediately.
	virtual LTRESULT		PostMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage);

	// Send a message to the console and wait for response or timeout.
	virtual LTRESULT		SendMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessageToSend, IRNGCMessage **ppMessageResponse, uint32 nTimeout);

	// Get the number of messages in the console message queue.
	virtual	LTRESULT		GetNumMessages(uint32* pNumMessages);

	// Get the next message in the console message queue.
	virtual	LTRESULT		GetNextMessage(RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout);

	// Get a specific message from the console message queue.
	virtual	LTRESULT		GetSpecificMessage(RNGC_MESSAGE_TYPE nRequestedType, IRNGCMessage** ppMessage, uint32 nTimeout);

	// Get a message within a givin range from the console message queue.
	virtual	LTRESULT		GetMessageInRange(RNGC_MESSAGE_TYPE nFilterMin, RNGC_MESSAGE_TYPE nFilterMax, RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout);

	// Get a text description of a specific console message.
	virtual	LTRESULT		GetMessageDescription(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage, char* szBuffer, uint32* pSizeOfBuffer);

	// Processes a system message.
	virtual	LTRESULT		ProcessSystemMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage);

	// Get a text description of a specific console error code.
	virtual	LTRESULT		GetErrorDescription(HRESULT hrError, char* szBuffer, uint32* pSizeOfBuffer);

	// Get a text description for the last console error code.
	virtual	LTRESULT		GetLastErrorDescription(char* szBuffer, uint32* pSizeOfBuffer);

	// Get the last console error code.
	virtual	LTRESULT		GetLastErrorResult(HRESULT* pErrorResult);

private:
	void					Clear();
	void					SetLastError(HRESULT hrError) { m_hrLastError = hrError; }


	// Member variables...

private:
	bool					m_bInited;
	IRNGameConsole*			m_pConsole;
	HRESULT					m_hrLastError;
};
#endif // LITHTECH_ESD