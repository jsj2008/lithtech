// ----------------------------------------------------------------------- //
//
// MODULE  : TronTargetMgr.cpp
//
// PURPOSE : Implementation of class to handle tracking whjat the player is aimed at.
//			 Derived from TargetMgr.cpp
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "clientres.h"
#include "TronTargetMgr.h"
#include "TronPlayerMgr.h"
#include "GameClientShell.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "PickupItemFX.h"
#include "PlayerMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::CTronTargetMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronTargetMgr::CTronTargetMgr() : CTargetMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::~CTronTargetMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronTargetMgr::~CTronTargetMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::Update
//
//	PURPOSE:	See what the crosshair is pointed at this frame
//
// ----------------------------------------------------------------------- //
void CTronTargetMgr::Update()
{
	if (m_hLockedTarget && m_hTarget == m_hLockedTarget)
	{

		// are we transferring energy?
		if (g_pTronPlayerMgr->IsTransferringEnergy())
		{
			// don't change anything.
			SetTargetStringID(IDS_TRANSFERRING_ENERGY);
			return;
		}

		//are we disabling a GadgetTarget?
		if (g_pPlayerMgr->IsDisabling())
		{
			SetGadgetTarget( true );
			return;
		}

		//are we searching something?
		if (g_pPlayerMgr->IsSearching())
		{
			m_bSearchTarget = true;
			SetTargetStringID(IDS_TARGET_SEARCHING);
			return;
		}
	}

	//clear our targeting info
	m_hTarget = NULL;
	m_fTargetRange = 10000.0f;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	m_ActivationData.Init();

	m_bEnergyRequired = false;
	m_nEnergyRequired = 0;

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = true;

	//see what we've looking at
	float fDistAway = 10000.0f;
	CheckForIntersect(fDistAway);
	if (!m_hTarget) //nothing to see here
		return;
	m_fTargetRange = fDistAway;


	//if its a body's hitbox, check the body instead
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(m_hTarget);
	if (pBody)
	{
		m_hTarget = pBody->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}


	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	//special case handling for bodies
	if (pBody)
	{
		LTRotation	rCamRot;
		LTVector	vTargetPos, vCamPos, vDir, vCamR;

		HLOCALOBJ	hCamera = g_pPlayerMgr->GetCamera();

		g_pLTClient->GetObjectRotation( hCamera, &rCamRot );
		vCamR = rCamRot.Right();

		g_pLTClient->GetObjectPos( hCamera, &vCamPos );
		g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );
		

		vDir = vTargetPos - vCamPos;

		// See if we are looking at the target...

		vDir.Normalize();
		LTFLOAT	fCamAngle = vDir.Dot( vCamR );

		bool bCanSearch = !!(dwUserFlags & USRFLG_CAN_SEARCH);
		bool bCanMove = (pBody->CanBeCarried() && g_pPlayerMgr->CanDropBody());
		bool bCanRevive = pBody->CanBeRevived();

		if (bCanSearch && bCanMove)
		{
			if (fCamAngle > 0)
			{
				// we can search this body
				if (fDistAway <= kActivationDist)
				{
					m_bSearchTarget = true;
					m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
					SetTargetStringID(IDS_TARGET_SEARCH);
					return;
				}
			}
			else
			{
				// we can move this body
				if (fDistAway <= kActivationDist)
				{
					m_bMoveTarget = true;
					m_ActivationData.m_nType = MID_ACTIVATE_MOVE;
					SetTargetStringID(IDS_TARGET_MOVE);
					return;
				}
			}
		}
		else if (bCanSearch && fDistAway <= kActivationDist)
		{
			// we can search this body
			m_bSearchTarget = true;
			m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
			SetTargetStringID(IDS_TARGET_SEARCH);
			return;
		}
		else if (bCanMove && fDistAway <= kActivationDist)
		{
			// we can move this body
			m_bMoveTarget = true;
			m_ActivationData.m_nType = MID_ACTIVATE_MOVE;
			SetTargetStringID(IDS_TARGET_MOVE);
			return;
		}
		else if( bCanRevive && fDistAway <= kActivationDist )
		{
			// we can revive this body
			m_ActivationData.m_nType = MID_ACTIVATE_REVIVE;
			SetTargetStringID(IDS_TARGET_REVIVE);
			return;
		}
		else
		{
			SetTargetStringID(NULL);
			return;
		}

	}

	// is this a gadget target
	if (IsGadgetActivatable(m_hTarget) && (fDistAway <= kActivationDist))
	{
		// looks like we can use a gadget on it...
		SetGadgetTarget(false);
		return;
	}

	//are we aiming at a person?
	if (dwUserFlags & USRFLG_CHARACTER)
	{
		CCharacterFX* const pFX = (CCharacterFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, m_hTarget);

		//display debug info if we have any 
		if( pFX && pFX->GetInfoString() && *pFX->GetInfoString() )
		{
			SAFE_STRCPY(m_szDebugString,pFX->GetInfoString());			
		}
		else
		{
			m_szDebugString[0] = NULL;
		}

		// is this a person we can talk to?
		if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		{
			SetTargetStringID(IDS_TARGET_TALK);
			return;
		}

	
		WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(g_pPlayerStats->GetCurrentWeapon());
		if (pWeaponData)
		{
			// warn the player if we have a dangerous weapon and are pointing at a friend...
			if (pWeaponData->bLooksDangerous && pFX && pFX->m_cs.eCrosshairCharacterClass != BAD)
			{
				if (fDistAway <= (LTFLOAT) pWeaponData->nRange)
				{
					SetTargetStringID(IDS_TARGET_INNOCENT);
					return;
				}
			}


		}
	}
/*	Don't allow SEARCHING in Tron
	//is this a searchable object?
	if (dwUserFlags & USRFLG_CAN_SEARCH && (fDistAway <= kActivationDist))
	{
		m_bSearchTarget = true;
		m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
		SetTargetStringID(IDS_TARGET_SEARCH);
		return;

	}
*/
	//can we pick up or activate it?
	if (dwUserFlags & USRFLG_CAN_ACTIVATE && (fDistAway <= kActivationDist))
	{

		CPickupItemFX* const pFX = (CPickupItemFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_PICKUPITEM_ID, m_hTarget);
		if (pFX)
			SetTargetStringID(IDS_TARGET_TAKE);
		else
		{
			if (dwUserFlags & USRFLG_REQUIRES_ENERGY)
			{
				// This object requires energy! Send a message off to find out how much it needs
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(m_hTarget);
				cMsg.Writeuint32(MID_QUERY_TARGET_PROPERTIES);
				cMsg.Writeuint8(1);
				cMsg.Writeuint8(TARGET_PROP_ENERGY_REQUIRED);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				SetTargetStringID(IDS_TRANSFER_ENERGY);
			}
			else
			{
				SetTargetStringID(IDS_TARGET_USE);
			}
		}
		return;
	}


	//we can't do anything with out target
	SetTargetStringID(NULL);
	return;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronTargetMgr::HandleTargetPropertiesMessage
//
//	PURPOSE:	Special handler that processes additional information
//				about an object under the crosshair
//
// ----------------------------------------------------------------------- //
void CTronTargetMgr::HandleTargetPropertiesMessage(ILTMessage_Read *pMsg)
{
	// discard props if the crosshair is not over anything anymore
	if (!m_hTarget)
		return;

	HOBJECT hObj = pMsg->ReadObject();

	if (!hObj)
		return;

	// see if this is still a legitimate target object.
	if (hObj != m_hTarget)
		return;

	// if it is, then see what useful information we can grab from it
	uint8 nProps = pMsg->Readuint8();

	for (int i = 0; i < nProps; i++)
	{
		uint16 iThrowaway = pMsg->Readuint16(); // length of submessage was passed.  Ignore.

		uint8 iProp = pMsg->Readuint8();

		switch(iProp)
		{
		case TARGET_PROP_ENERGY_REQUIRED:
			{
				m_bEnergyRequired = true;
				m_nEnergyRequired = pMsg->Readuint32();
				// Note that we don't need to queue an update to the crosshair because it
				// automatically updates every frame
				break;
			}

		case TARGET_PROP_HEALTH:
			{
				break;
			}

		default:
			{
				break;
			}
		}
	}
}
/*
Andy
let the playermgr lock the target when a transfer starts
let the crosshair know when a a transfer starts

	IDS_REQUIRES_ENERGY				"Requires %d energy"
	IDS_INSUFFICIENT_ENERGY			"You do not have enough energy to complete this transfer."
	IDS_TRANSFER_ENERGY_SUCCESS		"Energy transfer successful."

*/