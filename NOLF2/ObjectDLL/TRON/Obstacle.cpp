/****************************************************************************
;
;	MODULE:			Obstacle.cpp
;
;	PURPOSE:		Base obstacle class for TRON
;
;	HISTORY:		12/19/2001 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/


#include "stdafx.h"
#include "Obstacle.h"
#include "PSets.h"
#include "PlayerObj.h"

BEGIN_CLASS(Obstacle)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_LOCALDIMS | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_PSETS_PROP(PF_GROUP1)
	ADD_STRINGPROP(GoodPSetsTrigger, "")
	ADD_STRINGPROP(BadPSetsTrigger, "")
	ADD_STRINGPROP(GoodGameTrigger, "")
	ADD_STRINGPROP(BadGameTrigger, "")
	ADD_STRINGPROP(FinishedMsg, "")
END_CLASS_DEFAULT(Obstacle, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::Obstacle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Obstacle::Obstacle() : GameBase()
{
	m_byPSets = 0;
	m_hstrGoodPSetsTrigger = NULL;
	m_hstrBadPSetsTrigger = NULL;
	m_hstrGoodGameTrigger = NULL;
	m_hstrBadGameTrigger = NULL;
	m_hstrFinishedMsg = NULL;
	m_bPlayingMiniGame = LTFALSE;
	m_byGameID = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::~Obstacle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Obstacle::~Obstacle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Obstacle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			
			break;
		}

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
			break;
		}

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

		default: break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 Obstacle::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	return GameBase::ObjectMessageFn(hSender, pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Obstacle::ReadProp(ObjectCreateStruct *pData)
{
	// Sanity check
    if (!pData) return LTFALSE;

	// Read the permission sets in
	LTBOOL bPSets[8];
	g_pLTServer->GetPropBool("Key1", &bPSets[0]);
	g_pLTServer->GetPropBool("Key2", &bPSets[1]);
	g_pLTServer->GetPropBool("Key3", &bPSets[2]);
	g_pLTServer->GetPropBool("Key4", &bPSets[3]);
	g_pLTServer->GetPropBool("Key5", &bPSets[4]);
	g_pLTServer->GetPropBool("Key6", &bPSets[5]);
	g_pLTServer->GetPropBool("Key7", &bPSets[6]);
	g_pLTServer->GetPropBool("Key8", &bPSets[7]);

	m_byPSets = MakePSetsByte(bPSets);

	const int nBufSize = 1024;
	char buf[nBufSize];
	
	buf[0] = '\0';
    g_pLTServer->GetPropString("GoodPSetsTrigger", buf, nBufSize);
    if (buf[0] && strlen(buf)) m_hstrGoodPSetsTrigger = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("BadPSetsTrigger", buf, nBufSize);
    if (buf[0] && strlen(buf)) m_hstrBadPSetsTrigger = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("GoodGameTrigger", buf, nBufSize);
    if (buf[0] && strlen(buf)) m_hstrGoodGameTrigger = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("BadGameTrigger", buf, nBufSize);
    if (buf[0] && strlen(buf)) m_hstrBadGameTrigger = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("FinishedMsg", buf, nBufSize);
    if (buf[0] && strlen(buf)) m_hstrFinishedMsg = g_pLTServer->CreateString(buf);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Obstacle::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// Permission sets
	SAVE_BYTE(m_byPSets);
	
	// Triggers
	SAVE_HSTRING(m_hstrGoodPSetsTrigger);
	SAVE_HSTRING(m_hstrBadPSetsTrigger);
	SAVE_HSTRING(m_hstrGoodGameTrigger);
	SAVE_HSTRING(m_hstrBadGameTrigger);
	SAVE_HSTRING(m_hstrFinishedMsg);

	// Other data
	SAVE_BOOL(m_bPlayingMiniGame);
	SAVE_BYTE(m_byGameID);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Obstacle::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// Permission sets
    LOAD_BYTE(m_byPSets);

	// Triggers
	LOAD_HSTRING(m_hstrGoodPSetsTrigger);
	LOAD_HSTRING(m_hstrBadPSetsTrigger);
	LOAD_HSTRING(m_hstrGoodGameTrigger);
	LOAD_HSTRING(m_hstrBadGameTrigger);
	LOAD_HSTRING(m_hstrFinishedMsg);

	// Other data
	LOAD_BOOL(m_bPlayingMiniGame);
	LOAD_BYTE(m_byGameID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Obstacle::StartMiniGame
//
//	PURPOSE:	Sends a message to the client to start the mini game
//
// ----------------------------------------------------------------------- //

void Obstacle::StartMiniGame()
{
	// Can't start a mini game if it's already going
	if(m_bPlayingMiniGame)
	{
		TRACE("Obstacle::StartMiniGame - WARNING - Tried to start a minigame while a minigame was already in progress.\n");
		return;
	}

	// Get the necessary info
	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if(!pPlayer)
	{
		TRACE("Obstacle::StartMiniGame - ERROR - Couldn't find the player object!\n");
		return;
	}

	// We're now playing
	m_bPlayingMiniGame = LTTRUE;

	uint32 dwPlayerSuccess = pPlayer->GetSuccessRating();
	uint8 byPlayerPSets = pPlayer->GetPSets();

	// Send the message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_BEGIN_MINIGAME);
	cMsg.Writeuint8(m_byGameID);
	cMsg.Writeuint32(dwPlayerSuccess);
	cMsg.Writeuint8(byPlayerPSets);
	cMsg.Writeuint8(GetPsetByte());
	cMsg.WriteObject(m_hObject);
	g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
}