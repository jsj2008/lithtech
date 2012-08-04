// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 11/5/01
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronGameClientShell.h"

#include "msgids.h"

#include "clientmultiplayermgr.h"
#include "iserverdir.h"

// Sample rate
int g_nSampleRate = 44100;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::CTronGameClientShell()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CTronGameClientShell::CTronGameClientShell() : CGameClientShell()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::~CTronGameClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //

CTronGameClientShell::~CTronGameClientShell()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnEngineInitialized
//
//	PURPOSE:	Wrapper for OnEngineInitialized so we can do something extra
//
// ----------------------------------------------------------------------- //

uint32 CTronGameClientShell::OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid)
{
	uint32 nResult = CGameClientShell::OnEngineInitialized(pMode, pAppGuid);
	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::ProcessPacket
//
//	PURPOSE:	Handling for connectionless networking packets
//
// ----------------------------------------------------------------------- //

LTRESULT CTronGameClientShell::ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort)
{
	// Gotta have some data...
	if (pMsg->Size() < 16)
		return LT_OK;

	// Skip the engine-side portion of the message header
	CLTMsgRef_Read cSubMsg(pMsg->SubMsg(8));

	// Remember what address this came from
	g_pClientMultiplayerMgr->SetCurMessageSource(senderAddr, senderPort);

	// Hand it off as if it were a message
	// Note : This has to go through the parent's OnMessage so we handle the message source properly
	CGameClientShell::OnMessage(cSubMsg);

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::OnMessage
//
//	PURPOSE:	Message from the server
//
// ----------------------------------------------------------------------- //
void CTronGameClientShell::OnMessage(ILTMessage_Read *pMsg)
{
	// Store the server's address as where this message came from
	uint8 aAddrBuffer[4];
	uint16 nPort;
	g_pLTClient->GetServerIPAddress(aAddrBuffer, &nPort);
	g_pClientMultiplayerMgr->SetCurMessageSource(aAddrBuffer, nPort);

	CGameClientShell::OnMessage(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CTronGameClientShell::PauseGame(bool bPause, bool bPauseSound)
{
	// start with the base "pause" function
	CGameClientShell::PauseGame(bPause, bPauseSound);

	g_pSubroutineMgr->Pause(bPause);	
	g_pTronPlayerMgr->Pause(bPause);
}
