// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronPlayerMgr.h"
#include "TronPlayerStats.h" // needed for energy transfer data
#include "HUDMgr.h"
#include "SubroutineMgr.h"

CTronPlayerMgr* g_pTronPlayerMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::CTronPlayerMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::CTronPlayerMgr() : CPlayerMgr()
{
	m_bTransferringEnergy = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::~CTronPlayerMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::~CTronPlayerMgr()
{
	g_pTronPlayerMgr = NULL;
}


LTBOOL CTronPlayerMgr::Init()
{
	g_pTronPlayerMgr = this;

	m_bTransferringEnergy = false;

	return CPlayerMgr::Init();
}

void CTronPlayerMgr::Term()
{
	m_bTransferringEnergy = false;
	g_pTronPlayerMgr = NULL;

	CPlayerMgr::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOn(int command)
{
	return CPlayerMgr::OnCommandOn(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOff(int command)
{
	
	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
            if (m_bTransferringEnergy)
			{
				// Clear the transfer flag
				m_bTransferringEnergy = false;

				HOBJECT hObj = m_pTargetMgr->GetLockedTarget();

				// release the lock on the target
				m_pTargetMgr->LockTarget(NULL);

				// Insufficient transfer.  Abort
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				// TODO put a message on the HUD
				return LTTRUE;
			}
		}
		break;

        default : 
			break;
	}

	return CPlayerMgr::OnCommandOff(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages specific to TRON
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnMessage(ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint8 messageID = pMsg->Readuint8();
	switch(messageID)
	{
		case MID_START_ENERGY_TRANSFER:		HandleMsgStartEnergyTransfer	(pMsg);			break;
		default:							return CPlayerMgr::OnMessage(messageID, pMsg);	break;
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Pause()
//
//	PURPOSE:	Note the pausing and unpausing of the game.
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Pause(LTBOOL bPause)
{
	m_bPause = bPause;
	if (!m_bPause)
	{
		m_fLastUpdateTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Update()
{
	CPlayerMgr::Update();

	if (m_bPause)	// if paused, that's all we do.
		return;

	if (m_bTransferringEnergy)
	{
		// advance any timers.
		float fNow = g_pLTClient->GetTime();
		float fDeltaTime = fNow - m_fLastUpdateTime;
		m_fLastUpdateTime = fNow;

		// Increment the amount of energy transferred based upon some fixed rate
		float fEnergyPerSecond = 5.0f;

		float fEnergy = fEnergyPerSecond * fDeltaTime;

		m_fEnergyTransferred += fEnergy;
		g_pHUDMgr->QueueUpdate(kHUDEnergyTrans);
		g_pHUDMgr->QueueUpdate(kHUDEnergy);
		if (m_fEnergyTransferred > m_fEnergyRequired)
		{
			// TRANSFER COMPLETE
			m_fEnergyTransferred = 0;
			m_bTransferringEnergy = false;
			HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
			m_pTargetMgr->LockTarget(NULL);
			int iEnergy = g_pTronPlayerStats->GetEnergy() - (int)m_fEnergyRequired;

			if (iEnergy < 0)
				iEnergy = 0;

			// Tell the playerstatsmgr about the new energy balance
			g_pTronPlayerStats->UpdateEnergy(iEnergy);

			// Notify the binary bit that the energy transfer is complete
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_OBJECT_MESSAGE);
			cMsg.WriteObject(hObj);
			cMsg.Writeuint32(MID_ENERGY_TRANSFER_COMPLETE);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			// TODO send a message to the HUD
		}
	}
}

void CTronPlayerMgr::InitTargetMgr()
{
	m_pTargetMgr = debug_new( CTronTargetMgr );				 
	ASSERT( 0 != m_pTargetMgr );
}

void CTronPlayerMgr::HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg)
{
	int iEnergyRequired = pMsg->Readuint32();
	int iEnergyAvailable = g_pTronPlayerStats->GetEnergy();

	HOBJECT hObj = m_pTargetMgr->GetTargetObject();

	if (!hObj) // this should be impossible
		return;

	// Exit out with a message to the screen that you don't have enough energy
	// On the exit, send back the FAILED message to the BinaryBit.
	if (iEnergyRequired > iEnergyAvailable)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		// TODO put a message on the HUD about insufficient energy
		return;
	}

	// Set the transfer flag
	m_bTransferringEnergy = true;

	m_fEnergyTransferred = 0.0f;
	m_fEnergyRequired = (float)iEnergyRequired;

	// lock targetmgr on this object until the player releases the activate key.
	m_pTargetMgr->LockTarget(hObj);

	m_fLastUpdateTime = g_pLTClient->GetTime();
}

float CTronPlayerMgr::GetPercentEnergyTransferred()
{
	if (!m_bTransferringEnergy)
		return 0.0f;

	if (m_fEnergyRequired < 1.0f)
		return 0.0f;

	return (m_fEnergyTransferred / m_fEnergyRequired);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetJumpVelocity
//
//	PURPOSE:	Gets the player's jump velocity which is dependent
//				on certain subroutines
//
// ----------------------------------------------------------------------- //
float CTronPlayerMgr::GetJumpVelocity(float fJumpVel, float fSuperJumpVel)
{
	// Check for existence of the jump enhancing subroutine
	if(g_pSubroutineMgr->IsUtilitySubroutineActive("YAmp"))
	{
		float fDelta = fSuperJumpVel - fJumpVel;
		
		// Check the state of it
		switch(g_pSubroutineMgr->GetActiveSubroutineVersion("YAmp"))
		{
			case VERSION_ALPHA:
			{
				return(fJumpVel + (fDelta * 0.33f));
			}
			case VERSION_BETA:
			{
				return(fJumpVel + (fDelta * 0.66f));
			}
			case VERSION_GOLD:
			{
				return(fSuperJumpVel);
			}
			break;
		}
	}

	return fJumpVel;
}