/****************************************************************************
;
;	MODULE:			CoreDump.cpp
;
;	PURPOSE:		Core Dump object class for TRON
;
;	HISTORY:		03/22/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "CoreDump.h"
#include "TronPlayerObj.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "InventoryButeMgr.h"

BEGIN_CLASS(CoreDump)
END_CLASS_DEFAULT(CoreDump, GameBase, NULL, NULL)

// The decay amount in units per second
#define CORE_DUMP_DECAY_AMOUNT_PER_SECOND 5.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::CoreDump()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
CoreDump::CoreDump() : GameBase()
{
	m_fTotalEnergy = 0.0f;
	m_fCurrentEnergy = 0.0f;
	m_bActive = false;
	m_pPlayer = NULL;
	memset(m_collFX,0,MAX_CORE_DUMP_FX*sizeof(HOBJECT));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::~CoreDump()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
CoreDump::~CoreDump()
{
	// Note: We don't remove all our FX here because that's done explicitly
	// before this object is removed. The only other way for this object
	// to be removed is by the engine when it removes ALL objects
	// so we don't need to remove our FX in that case.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
uint32 CoreDump::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);
			return ret;
		}

		case MID_SAVEOBJECT:
		{
			// NOTE: If you want to save the base class first,
			// just switch the following two lines of code.
            Save((ILTMessage_Write*)pData, (uint32)fData);
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			return ret;
		}

		case MID_LOADOBJECT:
		{
			// NOTE: If you want to load the base class first,
			// just switch the following two lines of code.
            Load((ILTMessage_Read*)pData, (uint32)fData);
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			return ret;
		}

		case MID_UPDATE:
		{
			// Let our function determine whether or not we call the base class
			if(Update())
				return LT_OK;

			return(GameBase::EngineMessageFn(messageID, pData, fData));
		}

		case MID_INITIALUPDATE:
		{
			// Initial update
			InitialUpdate();
			return LT_OK;
		}

		default: break;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CoreDump::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fTotalEnergy);
	SAVE_FLOAT(m_fCurrentEnergy);

	for(int i=0;i<MAX_CORE_DUMP_FX;i++)
	{
		SAVE_HOBJECT(m_collFX[i]);
	}

	// We specifically don't save/load the player because we don't support
	// saving/loading during a transfer
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CoreDump::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fTotalEnergy);
	LOAD_FLOAT(m_fCurrentEnergy);

	for(int i=0;i<MAX_CORE_DUMP_FX;i++)
	{
		LOAD_HOBJECT(m_collFX[i]);
	}

	m_bActive = false;

	// We specifically don't save/load the player because we don't support
	// saving/loading during a transfer
	HOBJECT hPlayer = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //
bool CoreDump::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_StopActivate("STOPACTIVATE");
	static CParsedMsg::CToken s_cTok_Complete("COMPLETE");
	
	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		HandleActivate(hSender);
	}
	else if (cMsg.GetArg(0) == s_cTok_StopActivate)
	{
		HandleStopActivate();
	}
	else if (cMsg.GetArg(0) == s_cTok_Complete)
	{
		HandleComplete(hSender);
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleActivate
//
//	PURPOSE:	Start activating
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleActivate(HOBJECT hSender)
{
	// Sanity checks
	if(m_bActive)
	{
		// We should never get an activate message when we're already activated
		g_pLTServer->CPrint("ERROR - CoreDump received an activate message when it was already being activated!\n");
		ASSERT(FALSE);
		return;
	}

	if(!IsPlayer(hSender))
	{
		g_pLTServer->CPrint("ERROR - CoreDump received an activate message from an object other than a player!\n");
		ASSERT(FALSE);
		return;
	}

	CTronPlayerObj* pPlayer = (CTronPlayerObj*) g_pLTServer->HandleToObject(hSender);

	// We're all good
	m_pPlayer = pPlayer;
	m_bActive = true;

	// Stop the updating temporarily
	SetNextUpdate(0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleStopActivate
//
//	PURPOSE:	Stop activating
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleStopActivate()
{
	// Clear our player pointer
	m_pPlayer = NULL;

	m_bActive = false;

	// Resume updating
	SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //
LTBOOL CoreDump::InitialUpdate()
{
	ObjectCreateStruct ocs;
	LTVector vPos;
	BaseClass* pObj;
	char szProps[1024];
	char szFX[128];

	// Get our position
	g_pLTServer->GetObjectPos(m_hObject,&vPos);

	// And create our clientFX at that position
	for(int i=0;i<MAX_CORE_DUMP_FX;i++)
	{
		// The string for the props of the SpecialFX object
		sprintf(szFX,"CoreDump_%d",i);
		sprintf(szProps,"FxName %s;Loop 1;SmoothShutdown 1",szFX);

		INIT_OBJECTCREATESTRUCT(ocs);
		ocs.m_Rotation.Init();
		ocs.m_Pos = vPos;

		HCLASS hClass = g_pLTServer->GetClass("SpecialFX");
		if(hClass)
		{
			// Here's where we actually create the object
			pObj = (BaseClass*)g_pLTServer->CreateObjectProps(hClass, &ocs, szProps);
			if(pObj)
			{
				// Store the HOBJECT in our array
				m_collFX[i] = pObj->m_hObject;
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
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	// Here we go! (sanity check just in case we got an active message before our first update)
	if(!m_bActive)
	{
		SetNextUpdate(0.001f);
	}
	else
	{
		SetNextUpdate(0.0f);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Update
//
//	PURPOSE:	Update function - drain some health/energy
//
// ----------------------------------------------------------------------- //
LTBOOL CoreDump::Update()
{
	// If we're updating, then we need to be decaying
	ASSERT(!m_bActive && !m_pPlayer);

	// If we've got -1.0f energy, we don't decay
	if(m_fTotalEnergy <= 0.0f)
	{
		SetNextUpdate(0.0f);
		return LTTRUE;
	}

	SetNextUpdate(0.001f);

	// Figure out how much to decay based on how much time has passed
	float fLastFrameDelta = g_pLTServer->GetFrameTime();
	float fEnergyLoss = CORE_DUMP_DECAY_AMOUNT_PER_SECOND * fLastFrameDelta;

	if(fEnergyLoss >= m_fCurrentEnergy)
	{
		// We're done
		Term();
		return LTTRUE;
	}
	else
	{
		// Take away some energy
		m_fCurrentEnergy -= fEnergyLoss;

		// See if we need to kill any clientFX
		float fPercentRemaining = m_fCurrentEnergy / m_fTotalEnergy;
		int nClientFX = (int)((fPercentRemaining * (float)MAX_CORE_DUMP_FX) + 1.0f);
		
		if(nClientFX >= MAX_CORE_DUMP_FX)
			return LTTRUE;

		char szFX[512];

		// Go through and delete them
		for(int i=nClientFX;i<MAX_CORE_DUMP_FX;i++)
		{
			if(m_collFX[i])
			{
				// If we delete one, let's spawn in a removal effect for it.
				sprintf(szFX,"CoreDumpRemove_%d",i);
				PlayClientFX(szFX,m_hObject);

				g_pLTServer->RemoveObject(m_collFX[i]);
				m_collFX[i] = NULL;
			}
		}
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::Term
//
//	PURPOSE:	When it's time for us to remove ourselves
//
// ----------------------------------------------------------------------- //
void CoreDump::Term()
{
	// Remove all our clientFX
	for(int i=0;i<MAX_CORE_DUMP_FX;i++)
	{
		if(m_collFX[i])
		{
			g_pLTServer->RemoveObject(m_collFX[i]);
			m_collFX[i] = NULL;
		}
	}

	// And remove ourselves
	g_pLTServer->RemoveObject(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CoreDump::HandleComplete
//
//	PURPOSE:	Transfer the inventory/energy to the player
//
// ----------------------------------------------------------------------- //
void CoreDump::HandleComplete(HOBJECT hSender)
{
	if(!IsPlayer(hSender))
	{
		g_pLTServer->CPrint("ERROR - CoreDump received a complete message from an object other than a player!\n");
		ASSERT(FALSE);
		return;
	}

	// Invoke the inventory commands
	int i;
	char *szCmd;
	GEN_INVENTORY_LIST::iterator iter;
	for(iter=m_lstInventory.begin();iter!=m_lstInventory.end();iter++)
	{
		// First get the command
		szCmd = g_pInventoryButeMgr->GetItemCommand(iter->nItemID);
		for(i=0;i<iter->nCount;i++)
		{
			// Now process it N times (once for each instance of the item we have)
			if(g_pCmdMgr->IsValidCmd(szCmd))
			{
				g_pCmdMgr->Process(szCmd,m_hObject,NULL);
			}
		}
	}

	// Now give the player some energy
	CTronPlayerObj* pPlayer = (CTronPlayerObj*) g_pLTServer->HandleToObject(hSender);
	
	// Get the player's damage aggregage
	CDestructible* pDest = m_pPlayer->GetDestructible();
	ASSERT(pDest);
	int nMax = (int)pDest->GetMaxEnergy();
	int nNew = (int)m_fCurrentEnergy + (int)pDest->GetEnergy();

	// Check
	if(nNew <= nMax)
	{
		// And set
		pDest->SetEnergy((float)nNew);
	}

	// And we're done.
	Term();
}