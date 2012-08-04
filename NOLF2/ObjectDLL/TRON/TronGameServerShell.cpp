// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// CREATED : 9/17/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronGameServerShell.h"
#include "GameStartPoint.h"
#include "TronPlayerObj.h"
#include "CoreDump.h"
#include "Body.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::CTronGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //
CTronGameServerShell::CTronGameServerShell() : CGameServerShell()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::~CTronGameServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //
CTronGameServerShell::~CTronGameServerShell()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnServerInititalized
//
//	PURPOSE:	What to do when the server is initialized
//
// ----------------------------------------------------------------------- //
LTRESULT CTronGameServerShell::OnServerInitialized()
{
	LTRESULT res = CGameServerShell::OnServerInitialized();
	
	m_LightCycleMgr.Init();

	return res;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //
CPlayerObj* CTronGameServerShell::CreatePlayer(HCLIENT hClient)
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

    HCLASS hClass = g_pLTServer->GetClass("CTronPlayerObj");

	GameStartPoint* pStartPoint = FindStartPoint(LTNULL);
	if (pStartPoint)
	{
		g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
	}

	CTronPlayerObj* pPlayer = NULL;
	if (hClass)
	{
        pPlayer = (CTronPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
	}

	return pPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::OnMessage
//
//	PURPOSE:	Message from the client
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint8 messageID = pMsg->Readuint8();

	switch (messageID)
	{
		case MID_COMPILE:	HandlePlayerCompile	(hSender, pMsg);	break;
		default: CGameServerShell::OnMessage(hSender,pMsg);
	}
}

void CTronGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Base class first
	CGameServerShell::Update(timeElapsed);

	// Now update the light cycle manager
	m_LightCycleMgr.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronGameServerShell::HandlePlayerCompile
//
//	PURPOSE:	Message from the client
//
// ----------------------------------------------------------------------- //
void CTronGameServerShell::HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg)
{
	// Handle the compile message
	CTronPlayerObj* pPlayer = (CTronPlayerObj*)g_pLTServer->GetClientUserData(hSender);

	if (pPlayer)
	{
		pPlayer->HandleCompile(pMsg);
	}
}

bool CTronGameServerShell::DropInventoryObject(Body* pBody)
{
	// Sanity check
	ASSERT(pBody);
	if(!pBody)
		return false;

	// Check the position of the object
	LTVector vPos;
	g_pLTServer->GetObjectPos(pBody->m_hObject,&vPos);

	// Now create a Core Dump object there
	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);
	ocs.m_Rotation.Init();
	ocs.m_Pos = vPos;

	CoreDump* pObj;
	HCLASS hClass = g_pLTServer->GetClass("CoreDump");
	if(hClass)
	{
		// Here's where we actually create the object
		pObj = (CoreDump*)g_pLTServer->CreateObject(hClass, &ocs);
		if(pObj)
		{
			// Fill out the inventory
			pObj->SetInventory(pBody->GetInventory());
			pObj->Init(pBody->GetEnergy());
			return true;
		}
		else
		{
			// Couldn't create the object
			ASSERT(FALSE);
		}
	}
	else
	{
		// Couldn't find the class
		ASSERT(FALSE);
	}

	return false;
}