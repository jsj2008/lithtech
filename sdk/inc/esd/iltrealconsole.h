/****************************************************************************
;
;	MODULE:		ILTRealConsole (.H)
;
;	PURPOSE:	Real Console Interface class
;
;	HISTORY:	04/11/2000 [blg] This file was created
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#pragma once


// Includes...

#include "ltbasedefs.h"
#include "..\..\..\externalesd\realconsole\include\rngclib.h"

//-----------------------------------------------------------------------------
// Classes...
//-----------------------------------------------------------------------------

/*!
	ILTRealConsoleMgr interface.  Use this manager to handle messaging between
	the Real Networks console interface and the Lithtech engine.
*/
class ILTRealConsoleMgr
{
	// Member functions...

public:

/*!
\return an LTRESULT indicating success or failure

Initialize the console.  This function must be called first.

Used for: ESD.
*/
	virtual	LTRESULT		Init(GUID guidGameID) = 0;

/*!
\return an LTRESULT indicating success or failure

Terminate the console.  This function must be called last.

Used for: ESD.
*/
	virtual	LTRESULT		Term() = 0;

/*!
\return an LTRESULT indicating success or failure

Determines if the RealConsole manager is initialized.

Used for: ESD.
*/
	virtual	LTRESULT		IsInitialized(bool* pInited) = 0;

/*!
\return an LTRESULT indicating success or failure

Create an empty message object.

Used for: ESD.
*/
	virtual LTRESULT		CreateMessage(IRNGCMessage** ppMessage) = 0;

/*!
\return an LTRESULT indicating success or failure

Release a message that is finished being used.

Used for: ESD.
*/
	virtual LTRESULT		ReleaseMessage(IRNGCMessage* pMessage) = 0;

/*!
\return an LTRESULT indicating success or failure

Post a message to the console and return immediately.

Used for: ESD.
*/
	virtual LTRESULT		PostMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage) = 0;

/*!
\return an LTRESULT indicating success or failure

Send a message to the console and wait for response or timeout.

Used for: ESD.
*/
	virtual LTRESULT		SendMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessageToSend, IRNGCMessage **ppMessageResponse, uint32 nTimeout) = 0;

/*!
\return an LTRESULT indicating success or failure

Get the number of messages in the console message queue.

Used for: ESD.
*/
	virtual	LTRESULT		GetNumMessages(uint32* pNumMessages) = 0;

/*!
\return an LTRESULT indicating success or failure

Get the next message in the console message queue.

Used for: ESD.
*/
	virtual	LTRESULT		GetNextMessage(RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout) = 0;

/*!
\return an LTRESULT indicating success or failure

Get a specific message from the console message queue.

Used for: ESD.
*/
	virtual	LTRESULT		GetSpecificMessage(RNGC_MESSAGE_TYPE nRequestedType, IRNGCMessage** ppMessage, uint32 nTimeout) = 0;

/*!
\return an LTRESULT indicating success or failure

Get a message within a givin range from the console message queue.

Used for: ESD.
*/
	virtual	LTRESULT		GetMessageInRange(RNGC_MESSAGE_TYPE nFilterMin, RNGC_MESSAGE_TYPE nFilterMax, RNGC_MESSAGE_TYPE* pType, IRNGCMessage** ppMessage, uint32 nTimeout) = 0;

/*!
\return an LTRESULT indicating success or failure

Get a text description of a specific console message.

Used for: ESD.
*/
	virtual	LTRESULT		GetMessageDescription(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage, char* szBuffer, uint32* pSizeOfBuffer) = 0;

/*!
\return an LTRESULT indicating success or failure

Processes a system message.

Used for: ESD.
*/
	virtual	LTRESULT		ProcessSystemMessage(RNGC_MESSAGE_TYPE nType, IRNGCMessage* pMessage) = 0;

/*!
\return an LTRESULT indicating success or failure

Get a text description of a specific console error code.

Used for: ESD.
*/
	virtual	LTRESULT		GetErrorDescription(HRESULT hrError, char* szBuffer, uint32* pSizeOfBuffer) = 0;

/*!
\return an LTRESULT indicating success or failure

Get a text description for the last console error code.

Used for: ESD.
*/
	virtual	LTRESULT		GetLastErrorDescription(char* szBuffer, uint32* pSizeOfBuffer) = 0;

/*!
\return an LTRESULT indicating success or failure

Get the last console error code.

Used for: ESD.
*/
	virtual	LTRESULT		GetLastErrorResult(HRESULT* pErrorResult) = 0;
};

#endif // LITHTECH_ESD
