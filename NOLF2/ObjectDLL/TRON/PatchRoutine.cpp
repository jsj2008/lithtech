/****************************************************************************
;
;	MODULE:			PatchRoutine.cpp
;
;	PURPOSE:		Patch routine class for TRON
;
;	HISTORY:		02/12/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "PatchRoutine.h"
#include "TronPlayerObj.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"

BEGIN_CLASS(PatchRoutine)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_LOCALDIMS | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_LONGINTPROP(Health,0)
	ADD_LONGINTPROP(Energy,0)
END_CLASS_DEFAULT(PatchRoutine, GameBase, NULL, NULL)

// The drain rate in units per second
#define PATCH_ROUTINE_DRAIN_AMOUNT_PER_SECOND 40.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::PatchRoutine()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PatchRoutine::PatchRoutine() : GameBase()
{
	m_nTotalHealth = 0;
	m_nTotalEnergy = 0;
	m_nCurrentHealth = 0;
	m_nCurrentEnergy = 0;
	m_bActive = false;
	m_pPlayer = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::~PatchRoutine()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PatchRoutine::~PatchRoutine()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PatchRoutine::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			if((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

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
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
			return LT_OK;
		}

		default: break;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL PatchRoutine::ReadProp(ObjectCreateStruct *pData)
{
	// Sanity check
    if (!pData) return LTFALSE;

	// Read them in
	GenericProp genProp;
	if ( g_pLTServer->GetPropGeneric("Health", &genProp) == LT_OK )
	{
	    m_nTotalHealth = genProp.m_Long;
	}

	if ( g_pLTServer->GetPropGeneric("Energy", &genProp) == LT_OK )
	{
	    m_nTotalEnergy = genProp.m_Long;
	}

	// Sanity check
	if(m_nTotalEnergy && m_nTotalHealth)
	{
		// Bad level deisgner... BAD!!!
		g_pLTServer->CPrint("ERROR - A PatchRoutine object was found that had both energy and health specified. Bad level designer. BAD!!!\n");
		ASSERT(FALSE);
	}

	// And set
	Reset();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PatchRoutine::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_DWORD(m_nTotalHealth);
	SAVE_DWORD(m_nTotalEnergy);
	SAVE_DWORD(m_nCurrentHealth);
	SAVE_DWORD(m_nCurrentEnergy);

	// We specifically don't save the player because we don't support
	// saving/loading during a transfer
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PatchRoutine::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_DWORD(m_nTotalHealth);
	LOAD_DWORD(m_nTotalEnergy);
	LOAD_DWORD(m_nCurrentHealth);
	LOAD_DWORD(m_nCurrentEnergy);

	m_bActive = false;

	// We specifically don't save the player because we don't support
	// saving/loading during a transfer
	HOBJECT hPlayer = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //
bool PatchRoutine::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_StopActivate("STOPACTIVATE");
	
	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		HandleActivate(hSender);
	}
	else if (cMsg.GetArg(0) == s_cTok_StopActivate)
	{
		HandleStopActivate();
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::HandleActivate
//
//	PURPOSE:	Start activating
//
// ----------------------------------------------------------------------- //
void PatchRoutine::HandleActivate(HOBJECT hSender)
{
	// Sanity checks
	if(m_bActive)
	{
		// We should never get an activate message when we're already activated
		g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message when it was already being activated!\n");
		ASSERT(FALSE);
		return;
	}

	if(!IsPlayer(hSender))
	{
		g_pLTServer->CPrint("ERROR - PatchRoutine received an activate message from an object other than a player!\n");
		ASSERT(FALSE);
		return;
	}

	// Make sure we've got goodness left to give
	if(!m_nCurrentHealth && !m_nCurrentEnergy)
		return;

	// Make sure the player can take it
	CTronPlayerObj* pPlayer = (CTronPlayerObj*) g_pLTServer->HandleToObject(hSender);
	CDestructible* pDest = m_pPlayer->GetDestructible();
	ASSERT(pDest);
	
	if(m_nCurrentHealth)
	{
		// If the player is already at his max, we're done.
		if(pDest->GetHitPoints() >= pDest->GetMaxHitPoints())
			return;
	}
	else // m_nCurrentEnergy - assumed because we've checked this above
	{
		// If the player is already at his max, we're done.
		if(pDest->GetEnergy() >= pDest->GetMaxEnergy())
			return;
	}

	// We're all good
	m_pPlayer = pPlayer;
	m_bActive = true;
	
	// Let the updating BEGIN!!!!
	SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::HandleStopActivate
//
//	PURPOSE:	Stop activating
//
// ----------------------------------------------------------------------- //
void PatchRoutine::HandleStopActivate()
{
	// Stop updating us
	SetNextUpdate(0.0f);

	// And clear our player pointer
	m_pPlayer = NULL;

	m_bActive = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PatchRoutine::Update
//
//	PURPOSE:	Update function - drain some health/energy
//
// ----------------------------------------------------------------------- //
LTBOOL PatchRoutine::Update()
{
	// If we're updating, then we need to be giving goodies to the player
	ASSERT(m_bActive && m_pPlayer);

	// Get the player's damage aggregage
	CDestructible* pDest = m_pPlayer->GetDestructible();
	ASSERT(pDest);

	// Figure out how much energy to drain based on how much time has passed
	float fLastFrameDelta = g_pLTServer->GetFrameTime();
	int nAdjustedAmount = (int)(PATCH_ROUTINE_DRAIN_AMOUNT_PER_SECOND / fLastFrameDelta);

	int nMax;
	int nCur;
	int nCanTake;

	SetNextUpdate(0.001f);

	// Check if we're doing health
	if(m_nCurrentHealth)
	{
		// See how much the player can take
		nMax = (int)pDest->GetMaxHitPoints();
		nCur = (int)pDest->GetHitPoints();
		nCanTake = nMax - nCur;

		// Upper bound (by player)
		if(nCanTake > nAdjustedAmount)
			nCanTake = nAdjustedAmount;

		// Lower bound (by how much we have left)
		if(nCanTake > m_nCurrentHealth)
			nCanTake = m_nCurrentHealth;

		// And make it so
		if(nCanTake != 0)
		{
			m_nCurrentHealth -= nCanTake;
			pDest->SetHitPoints((float)(nCur + nCanTake));
		}
		else
		{
			// Player is already max'd out
			HandleStopActivate();
		}
	}
	else if(m_nCurrentEnergy)
	{
		// See how much the player can take
		nMax = (int)pDest->GetMaxEnergy();
		nCur = (int)pDest->GetEnergy();
		nCanTake = nMax - nCur;

		// Upper bound (by player)
		if(nCanTake > nAdjustedAmount)
			nCanTake = nAdjustedAmount;

		// Lower bound (by how much we have left)
		if(nCanTake > m_nCurrentEnergy)
			nCanTake = m_nCurrentEnergy;

		// And make it so
		if(nCanTake != 0)
		{
			m_nCurrentEnergy -= nCanTake;
			pDest->SetEnergy((float)(nCur + nCanTake));
		}
		else
		{
			// Player is already max'd out
			HandleStopActivate();
		}
	}
	else
	{
		// We've got nothing left to give to the world!
		HandleStopActivate();
	}

	return LTTRUE;
}