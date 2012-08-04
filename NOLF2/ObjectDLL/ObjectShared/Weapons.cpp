// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.cpp
//
// PURPOSE : Weapons container object - Implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include <vector>
#include "stdafx.h"
#include "WeaponMgr.h"
#include "Weapons.h"
#include "PlayerObj.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "Character.h"
#include "ObjectMsgs.h"
#include "HHWeaponModel.h"
#include "AmmoBox.h"
#include "GameServerShell.h"
#include "WeaponItems.h"
#include "Weapons.h"
#include "Weapon.h"
#include "Projectile.h"

CBankedList<CWeapon> s_bankCWeapon;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CWeapons()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CWeapons::CWeapons() :
	m_nCurWeapon( -1 )
	, m_pWeapons( LTNULL )
	, m_pAmmo( LTNULL )
{
	m_pVecProjectile = debug_new( CProjectile );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::~CWeapons()
//
//	PURPOSE:	Destructor - deallocate weapons
//
// ----------------------------------------------------------------------- //

CWeapons::~CWeapons()
{
	DeleteWeapons();

	if (m_pAmmo)
	{
		debug_deletea(m_pAmmo);
		m_pAmmo = LTNULL;
	}

	if ( m_pVecProjectile )
	{
		debug_delete( m_pVecProjectile );
		m_pVecProjectile = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Init()
//
//	PURPOSE:	Initialize weapons
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::Init(HOBJECT hCharacter, HOBJECT hWeaponModel)
{
    LTBOOL bRet = LTFALSE;

	m_hCharacter = hCharacter;
	m_hWeaponModel = hWeaponModel;

	DeleteWeapons();

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
		ASSERT( LTNULL == m_pWeapons );
		m_pWeapons = debug_newa(CWeapon*, nNumWeapons);
		if (m_pWeapons)
		{
			memset(m_pWeapons, 0, sizeof(CWeapon*) * nNumWeapons);
		}
		else
		{
			return LTFALSE;
		}
	}

	int nNumAmmoIds = g_pWeaponMgr->GetNumAmmoIds();
	if (nNumAmmoIds > 0)
	{
		ASSERT( LTNULL == m_pAmmo );
		m_pAmmo = debug_newa(int, nNumAmmoIds);
		if (m_pAmmo)
		{
			memset(m_pAmmo, 0, sizeof(int) * nNumAmmoIds);
		}
		else
		{
            return LTFALSE;
		}
	}

	CreateAllWeapons();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CreateWeapon()
//
//	PURPOSE:	Create the specified weapon
//
// ----------------------------------------------------------------------- //

void CWeapons::CreateWeapon(uint8 nWeaponId, uint8 nAmmoId)
{
	if (!IsValidIndex(nWeaponId) || !m_pWeapons || m_pWeapons[nWeaponId]) return;

	if (nAmmoId == AMMO_DEFAULT_ID)
	{
		WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (!pWeaponData) return;

		nAmmoId = pWeaponData->nDefaultAmmoId;
	}

	CWeapon* pWeapon = s_bankCWeapon.New();
	if (pWeapon)
	{
		pWeapon->Init(this, m_hCharacter, nWeaponId, nAmmoId);
		m_pWeapons[nWeaponId] = pWeapon;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DeleteWeapons()
//
//	PURPOSE:	free the weapon arsenal
//
// ----------------------------------------------------------------------- //

void CWeapons::DeleteWeapons()
{
	if (m_pWeapons && g_pWeaponMgr)
	{
		for (int i=0; i < g_pWeaponMgr->GetNumWeapons(); i++)
		{
			if (m_pWeapons[i])
			{
				s_bankCWeapon.Delete(m_pWeapons[i]);
			}
		}

		debug_deletea(m_pWeapons);
		m_pWeapons = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CreateAllWeapons()
//
//	PURPOSE:	Set up the weapon arsenal
//
// ----------------------------------------------------------------------- //

void CWeapons::CreateAllWeapons()
{
	// Make sure we don't already have this arsenal...

	if (!m_pWeapons || m_pWeapons[0]) return;

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

	for (int i=0; i <= nNumWeapons; i++)
	{
		CreateWeapon(i);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CWeapons::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint8)lData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint8)lData);
		}
		break;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CWeapons::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_ADDWEAPON:
		{
			if (!AddWeapon(pObject, hSender, pMsg)) return 0;
		}
		break;

		case MID_AMMOBOX:
		{
			if (!AddAmmoBox(pObject, hSender, pMsg)) return 0;
		}
		break;

		case MID_ADDMOD:
		{
			if (!AddMod(pObject, hSender, pMsg)) return 0;
		}
		break;

		case MID_PROJECTILE:
		{
			HandleProjectileMessage( hSender, pMsg );
		}
		break;

		default : break;
	}

	// the fundamental class return 1 upon success
	// for no documented reason, so we will too
	return 1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddWeapon
//
//	PURPOSE:	Add a new weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddWeapon(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
    uint8   nWeaponId = pMsg->Readuint8();
    uint8   nAmmoId   = pMsg->Readuint8();
    int     nAmmo     = pMsg->Readint32();
	bool	bForce	  = pMsg->Readbool();

	// Make sure we always have the default ammo for the new weapon...

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);

	if (pWeapon && pWeapon->nDefaultAmmoId != nAmmoId &&
		IsValidAmmoId(pWeapon->nDefaultAmmoId))
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoId);
	
		// Only add the ammo if we don't have any of it...

		if (pAmmo && m_pAmmo[pWeapon->nDefaultAmmoId] == 0)
		{
			// Give the default ammo, and make sure the client is updated
			// if this is a player (and we actually added some)...

			if (AddAmmo(pWeapon->nDefaultAmmoId, pAmmo->nSelectionAmount))
			{
				if (IsPlayer(m_hCharacter))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
					if (pPlayer)
					{
						HCLIENT hClient = pPlayer->GetClient();
						if (hClient)
						{
							CAutoMessage cMsg;
							cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
							cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
							cMsg.Writeuint8(WMGR_INVALID_ID);
							cMsg.Writeuint8(pWeapon->nDefaultAmmoId);
							cMsg.Writefloat(static_cast<float>(GetAmmoCount(pWeapon->nDefaultAmmoId)));
							g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
						}
					}
				}
			}
		}
	}

	return AddWeapon(pObject, hSender, nWeaponId, nAmmoId, nAmmo, bForce);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddWeapon
//
//	PURPOSE:	Add a new weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddWeapon(LPBASECLASS pObject, HOBJECT hSender, uint8 nWeaponId,
						   uint8 nAmmoId, int nAmmo, bool bForcePickup)
{
 	if (!IsValidIndex(nWeaponId)) return LTFALSE;

	LTBOOL  bHaveIt   = LTTRUE;
    LTBOOL  bPickedUp = LTFALSE;

	if (m_pWeapons && m_pWeapons[nWeaponId] && m_pWeapons[nWeaponId]->Have())
	{
		bPickedUp = ( 0 != AddAmmo(nAmmoId, nAmmo) ) ;
	}
	else
	{
        bHaveIt   = LTFALSE;
        bPickedUp = LTTRUE;
		ObtainWeapon(nWeaponId, nAmmoId, nAmmo);
	}


	if (bPickedUp || bForcePickup)
	{
		// Tell weapon powerup it was picked up...
		if (pObject)
		{
			SendEmptyObjectMsg(MID_PICKEDUP, pObject->m_hObject, hSender);
		}

		// Send the appropriate message to the client...

		if (IsPlayer(m_hCharacter))
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
            if (!pPlayer) return LTFALSE;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
                cMsg.Writeuint8(nWeaponId);
                cMsg.Writeuint8(nAmmoId);
                cMsg.Writefloat((LTFLOAT)GetAmmoCount(nAmmoId));
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}

			if (bPickedUp)
			{
				HandlePotentialWeaponChange(pPlayer, nWeaponId, nAmmoId, bHaveIt);
			}
		}
	}
	else
	{
		if (IsPlayer(m_hCharacter))
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
            if (!pPlayer) return LTFALSE;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
				cMsg.Writeuint8(nWeaponId);
				cMsg.Writeuint8(WMGR_INVALID_ID);
				cMsg.Writefloat(0.0f);
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddAmmoBox
//
//	PURPOSE:	Collect ammo from a box
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddAmmoBox(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
    uint8   nAmmoIdsLeft = 0;

    uint8   numAmmoIds   = pMsg->Readuint8();
    uint8   nAmmoId[AB_MAX_TYPES];
	int		nAmmo[AB_MAX_TYPES];

	LTBOOL bTookAmmo = LTFALSE;

    uint8 i;
    for (i = 0; i < numAmmoIds; i++)
	{
        nAmmoId[i]  = pMsg->Readuint8();
        nAmmo[i]    = (int) pMsg->Readuint32();

		if (IsValidAmmoId(nAmmoId[i]))
		{
			int taken = AddAmmo(nAmmoId[i], nAmmo[i]);

			//for infinite ammotypes, take it all
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId[i]);
			if (pAmmo->nSelectionAmount >= 1000)
				taken = nAmmo[i];

			if (taken)
			{
				bTookAmmo = LTTRUE;
			}

			if (taken < nAmmo[i])
			{
				nAmmoIdsLeft++;
			}

			nAmmo[i] -= taken;
		}
	}

	if (nAmmoIdsLeft)
	{
		// Tell powerup what is left in the box...If we actually
		// took something...
		if (bTookAmmo)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_AMMOBOX);
			cMsg.Writeuint8(numAmmoIds);
			for (i = 0; i < numAmmoIds; i++)
			{
				cMsg.Writeuint8(nAmmoId[i]);
				cMsg.Writeint32(nAmmo[i]);
			}
			g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
		}
	}  
	else
	{
        // Tell ammo powerup it was picked up...
		SendEmptyObjectMsg(MID_PICKEDUP, pObject->m_hObject, hSender);
	}



	// Send the appropriate message to the client...
	if (IsPlayer(m_hCharacter))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
        if (!pPlayer) return LTFALSE;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			for (i = 0; i < numAmmoIds; i++)
			{
				if (IsValidAmmoId(nAmmoId[i]))
				{
					if (!AddIsAmmoWeapon(nAmmoId[i]))
					{
						// Normal method of picking up ammo...

						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
						cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
						cMsg.Writeuint8(WMGR_INVALID_ID);
						cMsg.Writeuint8(nAmmoId[i]);
						cMsg.Writefloat((LTFLOAT)GetAmmoCount(nAmmoId[i]));
						g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
					}
				}
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddIsAmmoWeapon
//
//	PURPOSE:	Add an IsAmmo type weapon if necessary...
//
// ----------------------------------------------------------------------- //

bool CWeapons::AddIsAmmoWeapon(uint8 nAmmoId)
{
	if (!IsValidAmmoId(nAmmoId)) return false;

	// If this ammo type is used by an IsAmmo weapon give us the weapon
	
	const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return false;

	if (pAmmo)
	{
		const WEAPON* pWeapon = g_pWeaponMgr->GetCorrespondingWeapon(pAmmo);
		if (pWeapon && pWeapon->bIsAmmo)
		{
			// Calling AddWeapon will make sure we actually have the weapon
			// and that the client is updated correctly.  However, make sure
			// we don't give us any more ammo, that was already taken care of...
			AddWeapon(LTNULL, LTNULL, pWeapon->nId, nAmmoId, 0, true);
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddMod
//
//	PURPOSE:	Add a weapon mod to the weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddMod(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	bool bRet = LTFALSE;

    uint8 nModId = pMsg->Readuint8();

	MOD const *pMod = g_pWeaponMgr->GetMod(nModId);
	if( pMod )
	{
		uint8 nWeaponId = pMod->GetWeaponId();
		
		bool bPickedUp = false;

		// [KLS 9/02/02] Don't call IsValidWeapon here because we want to allow
		// mods to be picked up for weapons we don't currently have yet...
	    if (m_pWeapons && IsValidIndex(nWeaponId) && m_pWeapons[nWeaponId])
		{
			// Check to see if we already have the mod...

			if (!m_pWeapons[nWeaponId]->HaveMod(nModId))
			{
				ObtainMod(nWeaponId, nModId);
				bPickedUp = true;
			}

			bRet = LTTRUE;
		}

		// Tell mod powerup if it was picked up...
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_PICKEDUP);
		cMsg.Writebool(bPickedUp);
		cMsg.Writebool(true);
		g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
	}

    return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddAmmo
//
//	PURPOSE:	Add ammo to a specific ammo type
//
// ----------------------------------------------------------------------- //

int CWeapons::AddAmmo(int nAmmoId, int nAmount)
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return 0;

	int nMaxAmmo = pAmmo->GetMaxAmount(m_hCharacter);

	int nMaxTaken = nMaxAmmo - m_pAmmo[nAmmoId];

	int taken = Min(nAmount,nMaxTaken);

	m_pAmmo[nAmmoId] += taken;

	if (m_pAmmo[nAmmoId] > nMaxAmmo)
	{
		m_pAmmo[nAmmoId] = nMaxAmmo;
	}
	else if (m_pAmmo[nAmmoId] < 0)
	{
		m_pAmmo[nAmmoId] = 0;
	}

	return taken;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DecrementAmmo
//
//	PURPOSE:	Decrement the specified ammo count
//
// ----------------------------------------------------------------------- //

void CWeapons::DecrementAmmo(int nAmmoId)
{
	if (!IsValidAmmoId(nAmmoId)) return;

	--m_pAmmo[nAmmoId];

	if (m_pAmmo[nAmmoId] < 0)
	{
		m_pAmmo[nAmmoId] = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::SetAmmo
//
//	PURPOSE:	Set the ammount of ammo for a specific ammo type
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::SetAmmo(int nAmmoId, int nAmount)
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
    if (!pAmmo) return LTFALSE;

	int nMaxAmmo = pAmmo->GetMaxAmount(m_hCharacter);

	// Set to max if less than 0...

	if (nAmount < 0)
	{
		nAmount = nMaxAmmo;
	}

	m_pAmmo[nAmmoId] = nAmount;

	if (m_pAmmo[nAmmoId] > nMaxAmmo)
	{
		m_pAmmo[nAmmoId] = nMaxAmmo;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObtainWeapon()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// ----------------------------------------------------------------------- //

void CWeapons::ObtainWeapon(uint8 nWeaponId, int nAmmoId,
                            int nDefaultAmmo, LTBOOL bNotifyClient)
{
	if (!m_pWeapons || !IsValidIndex(nWeaponId)) return;


	// If necessary set the ammo type based on the weapon's default
	// ammo...

	if (nAmmoId == AMMO_DEFAULT_ID)
	{
		WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (!pWeaponData) return;

		nAmmoId = pWeaponData->nDefaultAmmoId;
	}


	// Make sure we actually have the thing...

	if (!m_pWeapons[nWeaponId]) return;


	// Give us the weapon!

	m_pWeapons[nWeaponId]->Aquire();


	// Set the weapon's default ammo if appropriate...

	if (nDefaultAmmo >= 0)
	{
		SetAmmo(nAmmoId, nDefaultAmmo + GetAmmoCount(nAmmoId));
	}

	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeaponData) return;

	for (int m = 0; m < pWeaponData->nNumModIds; m++)
	{
		int nModId = pWeaponData->aModIds[m];

		MOD const *pModData = g_pWeaponMgr->GetMod(nModId);
		if (pModData)
		{
			if (pModData->bIntegrated )
			{
				m_pWeapons[nWeaponId]->AddMod( nModId );
			}
		}
	}



	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if (bNotifyClient)
	{
		// Send the appropriate message to the client...

		if (IsPlayer(m_hCharacter))
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
			if (!pPlayer) return;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_WEAPON_OBTAIN_ID);
                cMsg.Writeuint8(nWeaponId);
                cMsg.Writeuint8(nAmmoId);
                cMsg.Writefloat((LTFLOAT)GetAmmoCount(nAmmoId));
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}

			// If there isn't currently a weapon selected, select the new weapon..

			if (m_nCurWeapon < 0)
			{
				pPlayer->ChangeWeapon(g_pWeaponMgr->GetCommandId(nWeaponId));
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObtainMod()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// ----------------------------------------------------------------------- //

void CWeapons::ObtainMod(uint8 nWeaponId, uint8 nModId, bool bNotifyClient,
						 bool bDisplayMsg /*=true*/)
{
	if (!m_pWeapons || !IsValidIndex(nWeaponId) || !g_pWeaponMgr) return;


	// Make sure we actually have the thing...

	if (!m_pWeapons[nWeaponId]) return;

	MOD const *pModData = g_pWeaponMgr->GetMod(nModId);
	if( !pModData )
		return;

	m_pWeapons[nWeaponId]->AddMod(nModId);


	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if (bNotifyClient && IsPlayer(m_hCharacter))
	{
       CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
		if (!pPlayer) return;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_MOD_PICKUP_ID);
            cMsg.Writeuint8(true);
            cMsg.Writeuint8(nModId);
            cMsg.Writefloat(bDisplayMsg ? 1.0f : 0.0f);
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ChangeWeapon()
//
//	PURPOSE:	Change to a new weapon
//
// @parm the weapon to go to
// @rdef was the switch allowed?
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::ChangeWeapon(uint8 nNewWeapon)
{
    if (nNewWeapon == m_nCurWeapon) return LTTRUE;

    if (!m_pWeapons ||!IsValidWeapon(nNewWeapon)) return LTFALSE;

	// Set this as our current weapon...

	m_nCurWeapon = nNewWeapon;


	// Change our weapon model to the newly selected weapon

	if (IsCharacter(m_hCharacter))
	{
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hCharacter);
		if (pChar)
		{
			WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurWeapon);
            if (!pWeaponData) return LTFALSE;

 			// Associated our hand held weapon with our weapon...

			if (m_hWeaponModel)
			{
                CHHWeaponModel* pModel = (CHHWeaponModel*)g_pLTServer->HandleToObject(m_hWeaponModel);
				if (pModel)
				{
					pModel->Setup(m_pWeapons[m_nCurWeapon], pWeaponData);
				}
			}
		}
	}

	// Select the weapon...

	m_pWeapons[m_nCurWeapon]->Select();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::DeselectCurWeapon()
//
//	PURPOSE:	Deselect the current weapon
//
// ----------------------------------------------------------------------- //

void CWeapons::DeselectCurWeapon()
{
	if (IsValidWeapon(m_nCurWeapon))
	{
		m_pWeapons[m_nCurWeapon]->Deselect();
	}

	m_nCurWeapon = -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetWeaponAmmoCount()
//
//	PURPOSE:	Return the ammo amount of the specified selected weapon
//
// ----------------------------------------------------------------------- //

int CWeapons::GetWeaponAmmoCount(int nWeapon)
{
	if (!g_pWeaponMgr || !m_pWeapons || !m_pAmmo) return 0;

	if (IsValidIndex(nWeapon) && m_pWeapons[nWeapon])
	{
		return GetAmmoCount(m_pWeapons[nWeapon]->GetAmmoId());
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetAmmoCount()
//
//	PURPOSE:	Return the amount of the specified type of ammo
//
// ----------------------------------------------------------------------- //

int CWeapons::GetAmmoCount(int nAmmoId)
{
	if (!IsValidAmmoId(nAmmoId)) return 0;

	return m_pAmmo[nAmmoId];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Save(ILTMessage_Write *pMsg, uint8 nType)
{

	SAVE_HOBJECT(m_hCharacter);
	SAVE_HOBJECT(m_hWeaponModel);
	SAVE_INT(m_nCurWeapon);

	if (m_pWeapons)
	{
		int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

		for (int i=0; i < nNumWeapons; i++)
		{
			if (m_pWeapons[i])
			{
                SAVE_BOOL(LTTRUE);
				m_pWeapons[i]->Save(pMsg, nType);
			}
			else
			{
                SAVE_BOOL(LTFALSE);
			}
		}
	}

	int nNumAmmoIds = g_pWeaponMgr->GetNumAmmoIds();

	if (!m_pAmmo)
	{
		nNumAmmoIds = 0;
	}

	SAVE_BYTE(nNumAmmoIds);

	for (int i=0; i < nNumAmmoIds; i++)
	{
		SAVE_INT(m_pAmmo[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Load(ILTMessage_Read *pMsg, uint8 nType)
{

	LOAD_HOBJECT(m_hCharacter);
	LOAD_HOBJECT(m_hWeaponModel);

	// Create the m_pWeapons and m_pAmmo data members...

	Init(m_hCharacter, m_hWeaponModel);

	LOAD_INT(m_nCurWeapon);

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

    int i;
    for (i=0; i < nNumWeapons; i++)
	{
        LTBOOL bLoad;
		LOAD_BOOL(bLoad);

		if (bLoad)
		{
			CreateWeapon(i);

			if (m_pWeapons && m_pWeapons[i])
			{
				m_pWeapons[i]->SetParent(this);
				m_pWeapons[i]->Load(pMsg, nType);
			}
		}
	}

    uint8 nNumAmmoIds;
	LOAD_BYTE(nNumAmmoIds);

	for (i=0; i < nNumAmmoIds; i++)
	{
		if (m_pAmmo)
		{
			LOAD_INT(m_pAmmo[i]);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::IsValidWeapon()
//
//	PURPOSE:	Check if the current id is valid and we have it2
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::IsValidWeapon(uint8 nWeaponId)
{
	if(m_pWeapons && IsValidIndex(nWeaponId) && m_pWeapons[nWeaponId] &&
       m_pWeapons[nWeaponId]->Have()) return LTTRUE;

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Reset()
//
//	PURPOSE:	Reset all the weapons (i.e., we don't have any of them)
//
// ----------------------------------------------------------------------- //

void CWeapons::Reset()
{
	// Toss our current weapon since we won't have any after this function.
	DeselectCurWeapon( );

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

    int i;
    for (i=0; i < nNumWeapons; i++)
	{
		if (m_pWeapons && m_pWeapons[i])
		{
			m_pWeapons[i]->Drop();
		}
	}

	int nNumAmmoIds = g_pWeaponMgr->GetNumAmmoIds();

	for (i=0; i < nNumAmmoIds; i++)
	{
		if (m_pAmmo)
		{
			m_pAmmo[i] = 0;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::HandlePotentialWeaponChange()
//
//	PURPOSE:	Handle potentially changing weapons
//
// ----------------------------------------------------------------------- //

void CWeapons::HandlePotentialWeaponChange(CPlayerObj* pPlayer, uint8 nWeaponId, 
										   uint8 nAmmoId, LTBOOL bHaveIt)
{
	if (!pPlayer) return;

	LTBOOL bChangeWeapon   = LTFALSE;
	LTBOOL bIsBetterWeapon = IsBetterWeapon(pPlayer, nWeaponId);

	// If there isn't currently a weapon selected, or this weapon is
	// better than our currently selected weapon (and we didn't already
	// have it) select the new weapon...

	if (m_nCurWeapon < 0 || (!bHaveIt && bIsBetterWeapon))
	{
		bChangeWeapon = LTTRUE;
	}

	if (bChangeWeapon)
	{
		int nWeaponCoId = g_pWeaponMgr->GetCommandId(nWeaponId);
		pPlayer->ChangeWeapon(nWeaponCoId, LTTRUE, nAmmoId);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::IsBetterWeapon()
//
//	PURPOSE:	Determine if the passed in weapon type is "better" than the
//				weapon we are currently using
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::IsBetterWeapon(CPlayerObj* pPlayer, uint8 nWeaponId)
{
	return g_pWeaponMgr->IsBetterWeapon( nWeaponId, m_nCurWeapon );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeapons::HandleProjectileMessage()
//
//	PURPOSE:	Handle messages to/from projectiles
//
// ----------------------------------------------------------------------- //

void CWeapons::HandleProjectileMessage( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	// for catching errors
	LTRESULT ltResult;

	// get the projectile message's subtype
	uint8 nProjectileMsgType = pMsg->Readuint8();

	// if we need to resend the message
	switch ( nProjectileMsgType )
	{
		case MPROJ_RETURN:
		case MPROJ_UPDATE_POINT_TARGET:
		case MPROJ_UPDATE_OBJECT_TARGET:
		case MPROJ_UPDATE_CONTROL_LINE:
		{
			// this message is for the projectiles
			// we need to reset the reading position
			// because we read the subtype
			pMsg->SeekTo( 0 );

			//
			// pass the message to all the dependancies
			//

			// make a copy of the list in its current state to
			// avoid multiple access problems
			ProjectileMsgList lProjectilesNeedingMessages = m_lProjectilesNeedingMessages;

			// iteratate over all projectiles
			ProjectileMsgList::iterator iter;
			for ( iter = lProjectilesNeedingMessages.begin();
			      iter != lProjectilesNeedingMessages.end();
			      ++iter )
			{
				ltResult = g_pLTServer->SendToObject( pMsg, 
							hSender,
							*iter,
							MESSAGE_GUARANTEED
							);
				ASSERT( LT_OK == ltResult );
			}
		}
		break;

		case MPROJ_RETURNED:
		{
			// This message contains the information required to add the 
			// returned projectils ammo to the owner when caught

			uint8 nWeaponId = pMsg->Readuint8();

			uint8 nAmmoId = pMsg->Readuint8();
			
			uint8 nAmmoAmount = pMsg->Readuint8();
			
			AddAmmo( nAmmoId, nAmmoAmount );
		}
		break;

		case MPROJ_START_SENDING_MESSAGES:
		{
			// This message contains a list of projectiles that need
			//  message updates.
			
			// get the number of projectiles that need updates
			uint8 nNumProjectiles = pMsg->Readuint8();

			// keep track of each object specified in the message
			for ( int i = 0; i < nNumProjectiles; ++i )
			{
				// get the projectile's handle
				HOBJECT hProjectile = pMsg->ReadObject();

				// validate the handle
				ASSERT( INVALID_HOBJECT != hProjectile );

				// add it to the list of projectiles needing updates
				m_lProjectilesNeedingMessages.push_back( hProjectile );
			}
		}
		break;

		case MPROJ_STOP_SENDING_MESSAGES:
		{
			// This message contains a list of projectiles that don't need 
			// message updates.
			
			// This message tells us to stop sending messages to all/some
			// projecitles.  The first byte is the number of projectiles to
			// remove from consideration.  If this number is 0 it means
			// remove ALL projectiles.

			// get the number of projectiles that need updates
			uint8 nNumProjectiles = pMsg->Readuint8();

			if ( nNumProjectiles )
			{
				//
				// There are only a certian number of projectiles
				// that don't want updates, add them to the remove
				// list.
				//

				for ( int i = 0; i < nNumProjectiles; ++i )
				{
					HOBJECT hBrokenLink = pMsg->ReadObject();

					for ( ProjectileMsgList::iterator iter = m_lProjectilesNeedingMessages.begin();
							iter != m_lProjectilesNeedingMessages.end();
							++iter )
					{
						if ( hBrokenLink == *iter )
						{
							// remove the object from receiving updates
							m_lProjectilesNeedingMessages.erase( iter );
							break;
						}
					}
				}
			}
			else
			{
				// Remove ALL the projectiles
				m_lProjectilesNeedingMessages.clear();
			}
		}
		break;
	}
}
