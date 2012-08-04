// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.cpp
//
// PURPOSE : Weapons container object - Implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

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


CBankedList<CWeapon> s_bankCWeapon;
CVarTrack g_WeaponsStay;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CWeapons()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CWeapons::CWeapons()
{
    m_pWeapons   = LTNULL;
    m_pAmmo      = LTNULL;
	m_nCurWeapon = -1;
    m_hCharacter     = LTNULL;
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

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmoTypes > 0)
	{
		m_pAmmo = debug_newa(int, nNumAmmoTypes);
		if (m_pAmmo)
		{
			memset(m_pAmmo, 0, sizeof(int) * nNumAmmoTypes);
		}
		else
		{
            return LTFALSE;
		}
	}

	CreateAllWeapons();

	if(!g_WeaponsStay.IsInitted())
	{
		g_WeaponsStay.Init(g_pLTServer, "NetWeaponsStay", NULL, 0.0f);
	}

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
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (!pWeaponData) return;

		nAmmoId = pWeaponData->nDefaultAmmoType;
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
            Save((HMESSAGEWRITE)pData, (uint8)lData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)lData);
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

uint32 CWeapons::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_ADDWEAPON:
		{
			if (!AddWeapon(pObject, hSender, hRead)) return 0;
		}
		break;

		case MID_AMMOBOX:
		{
			if (!AddAmmoBox(pObject, hSender, hRead)) return 0;
		}
		break;

		case MID_ADDMOD:
		{
			if (!AddMod(pObject, hSender, hRead)) return 0;
		}
		break;

		default : break;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddWeapon
//
//	PURPOSE:	Add a new weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddWeapon(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
    uint8   nWeaponId		= g_pLTServer->ReadFromMessageByte(hRead);
    uint8   nAmmoId			= g_pLTServer->ReadFromMessageByte(hRead);
    int     nAmmo			= (int) g_pLTServer->ReadFromMessageFloat(hRead);
	LTBOOL  bIsLevelPowerup = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

	// Make sure we always have the default ammo for the new weapon...

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);

	if (pWeapon && pWeapon->nDefaultAmmoType != nAmmoId &&
		IsValidAmmoId(pWeapon->nDefaultAmmoType))
	{
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoType);
	
		// Only add the ammo if we don't have any of it...

		if (pAmmo && m_pAmmo[pWeapon->nDefaultAmmoType] == 0)
		{
			// Give the default ammo, and make sure the client is updated
			// if this is a player (and we actually added some)...

			if (AddAmmo(pWeapon->nDefaultAmmoType, pAmmo->nSelectionAmount))
			{
				if (IsPlayer(m_hCharacter))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
					if (pPlayer)
					{
						HCLIENT hClient = pPlayer->GetClient();
						if (hClient)
						{
							HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
							g_pLTServer->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
							g_pLTServer->WriteToMessageByte(hMessage, WMGR_INVALID_ID);
							g_pLTServer->WriteToMessageByte(hMessage, pWeapon->nDefaultAmmoType);
							g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)GetAmmoCount(pWeapon->nDefaultAmmoType));
							g_pLTServer->EndMessage(hMessage);
						}
					}
				}
			}
		}
	}

	return AddWeapon(pObject, hSender, nWeaponId, nAmmoId, nAmmo, bIsLevelPowerup);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddWeapon
//
//	PURPOSE:	Add a new weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddWeapon(LPBASECLASS pObject, HOBJECT hSender, uint8 nWeaponId,
						   uint8 nAmmoId, int nAmmo, LTBOOL bIsLevelPowerup)
{
 	if (!IsValidIndex(nWeaponId)) return LTFALSE;

	LTBOOL  bHaveIt   = LTTRUE;
    LTBOOL  bPickedUp = LTFALSE;

	LTBOOL bWasOutOfAmmo = LTFALSE; 

	if (m_pWeapons && m_pWeapons[nWeaponId] && m_pWeapons[nWeaponId]->Have())
	{
		if (IsMultiplayerGame() && g_WeaponsStay.GetFloat() > 0.0f && bIsLevelPowerup)
		{
			// Add the ammo if we're out of it...

			if (IsValidAmmoId(nAmmoId) && m_pAmmo[nAmmoId] <= 0)
			{
				bPickedUp = AddAmmo(nAmmoId, nAmmo);
			}
			else
			{
				bPickedUp = LTFALSE;
			}
		}
		else
		{
			bPickedUp = AddAmmo(nAmmoId, nAmmo);
		}
	}
	else
	{
        bHaveIt   = LTFALSE;
        bPickedUp = LTTRUE;
		ObtainWeapon(nWeaponId, nAmmoId, nAmmo);
	}


	if (bPickedUp)
	{
		// Tell powerup it was picked up...

		HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);


		// Send the appropriate message to the client...

		if (IsPlayer(m_hCharacter))
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
            if (!pPlayer) return LTFALSE;

			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
                HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
                g_pLTServer->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
                g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
                g_pLTServer->WriteToMessageByte(hMessage, nAmmoId);
                g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)GetAmmoCount(nAmmoId));
                g_pLTServer->EndMessage(hMessage);
			}

			HandlePotentialWeaponChange(pPlayer, nWeaponId, nAmmoId, bHaveIt, bWasOutOfAmmo);
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

LTBOOL CWeapons::AddAmmoBox(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{

    uint8   nAmmoTypesLeft = 0;

    uint8   numAmmoTypes   = g_pLTServer->ReadFromMessageByte(hRead);
    uint8   nAmmoId[AB_MAX_TYPES];
	int		nAmmo[AB_MAX_TYPES];

	LTBOOL bTookAmmo[AB_MAX_TYPES];
	LTBOOL bTookAnyAmmo = LTFALSE;

    uint8 i;
    for (i = 0; i < numAmmoTypes; i++)
	{
		bTookAmmo[i] = LTFALSE;

        nAmmoId[i]  = g_pLTServer->ReadFromMessageByte(hRead);
        nAmmo[i]    = (int) g_pLTServer->ReadFromMessageFloat(hRead);

		if (IsValidAmmoId(nAmmoId[i]))
		{
			int taken = AddAmmo(nAmmoId[i], nAmmo[i]);
			if (taken)
			{
				bTookAmmo[i] = LTTRUE;
				bTookAnyAmmo = LTTRUE;
			}

			if (taken < nAmmo[i])
			{
				nAmmoTypesLeft++;
			}

			nAmmo[i] -= taken;
		}
	}

	if (nAmmoTypesLeft)
	{
		// Tell powerup what is left in the box...If we actually
		// took something...

		if (bTookAnyAmmo)
		{
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_AMMOBOX);
			g_pLTServer->WriteToMessageByte(hWrite, numAmmoTypes);
			for (i = 0; i < numAmmoTypes; i++)
			{
				g_pLTServer->WriteToMessageByte(hWrite, nAmmoId[i]);
				g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)nAmmo[i]);
			}
			g_pLTServer->EndMessage(hWrite);
		}
	}  
	else
	{
        // Tell powerup it was picked up...
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}



	// Send the appropriate message to the client...
	if (IsPlayer(m_hCharacter) && bTookAnyAmmo)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
        if (!pPlayer) return LTFALSE;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			for (i = 0; i < numAmmoTypes; i++)
			{
				if (bTookAmmo[i])
				{
                    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
                    g_pLTServer->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
                    g_pLTServer->WriteToMessageByte(hMessage, WMGR_INVALID_ID);
                    g_pLTServer->WriteToMessageByte(hMessage, nAmmoId[i]);
                    g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)GetAmmoCount(nAmmoId[i]));
                    g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddMod
//
//	PURPOSE:	Add a weapon mod to the weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeapons::AddMod(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{

    uint8 nModId = g_pLTServer->ReadFromMessageByte(hRead);

	MOD* pMod = g_pWeaponMgr->GetMod(nModId);
	if (pMod)
	{
		uint8 nWeaponId = pMod->GetWeaponId();

		// Check to see if we already have the mod...

		if (!m_pWeapons[nWeaponId]->HaveMod(nModId))
		{
			ObtainMod(nWeaponId, nModId);

			// Tell powerup it was picked up...

			HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
			g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
			g_pLTServer->EndMessage(hWrite);
		}
	}

    return LTTRUE;
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
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
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
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
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
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (!pWeaponData) return;

		nAmmoId = pWeaponData->nDefaultAmmoType;
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

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeaponData) return;

	for (int m = 0; m < pWeaponData->nNumModTypes; m++)
	{
		CModData mod;
		mod.m_nID = pWeaponData->aModTypes[m];

		MOD* pModData = g_pWeaponMgr->GetMod(mod.m_nID);
		if (pModData)
		{
			if (pModData->bIntegrated)
			{
				m_pWeapons[nWeaponId]->AddMod(&mod);
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
                HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
                g_pLTServer->WriteToMessageByte(hMessage, IC_WEAPON_OBTAIN_ID);
                g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
                g_pLTServer->WriteToMessageByte(hMessage, nAmmoId);
                g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)GetAmmoCount(nAmmoId));
                g_pLTServer->EndMessage(hMessage);
			}

			// If there isn't currently a weapon selected, or we're
			// on our melee weapon, Select the new weapon..

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

void CWeapons::ObtainMod(uint8 nWeaponId, uint8 nModId, LTBOOL bNotifyClient)
{
	if (!m_pWeapons || !IsValidIndex(nWeaponId) || !g_pWeaponMgr) return;


	// Make sure we actually have the thing...

	if (!m_pWeapons[nWeaponId]) return;


	CModData mod;
	mod.m_nID = nModId;
	m_pWeapons[nWeaponId]->AddMod(&mod);


	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if (bNotifyClient && IsPlayer(m_hCharacter))
	{
       CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hCharacter);
		if (!pPlayer) return;

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
            HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
            g_pLTServer->WriteToMessageByte(hMessage, IC_MOD_PICKUP_ID);
            g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
            g_pLTServer->WriteToMessageByte(hMessage, nModId);
            g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
            g_pLTServer->EndMessage(hMessage);
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
			WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nCurWeapon);
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

void CWeapons::Save(HMESSAGEWRITE hWrite, uint8 nType)
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
				m_pWeapons[i]->Save(hWrite, nType);
			}
			else
			{
                SAVE_BOOL(LTFALSE);
			}
		}
	}

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

	if (!m_pAmmo)
	{
		nNumAmmoTypes = 0;
	}

	SAVE_BYTE(nNumAmmoTypes);

	for (int i=0; i < nNumAmmoTypes; i++)
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

void CWeapons::Load(HMESSAGEREAD hRead, uint8 nType)
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
				m_pWeapons[i]->Load(hRead, nType);
			}
		}
	}

    uint8 nNumAmmoTypes;
	LOAD_BYTE(nNumAmmoTypes);

	for (i=0; i < nNumAmmoTypes; i++)
	{
		if (m_pAmmo)
		{
			LOAD_INT(m_pAmmo[i]);
		}
	}
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
	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

    int i;
    for (i=0; i < nNumWeapons; i++)
	{
		if (m_pWeapons && m_pWeapons[i])
		{
			m_pWeapons[i]->Drop();
		}
	}

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

	for (i=0; i < nNumAmmoTypes; i++)
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
										   uint8 nAmmoId, LTBOOL bHaveIt, LTBOOL
										   bWasOutOfAmmo)
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


	// If we are out of ammo, we should change to this weapon even if
	// we have it...

	if (bHaveIt && bWasOutOfAmmo)
	{
		// TO DO:  Calculate for WasOutOfAmmo needs work...basically, are all our
		// weapons out of ammo and we're running around with just the fisty cuffs...
		// ...if so we want to change to the weapon we just picked up...
		// bChangeWeapon = LTTRUE;
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
	int nCurWeaponCoId = g_pWeaponMgr->GetCommandId(m_nCurWeapon);
	int nWeaponCoId    = g_pWeaponMgr->GetCommandId(nWeaponId);

	// TO DO:  Update this to look at a player-specific weapon priority
	// list...

	return (LTBOOL)(nCurWeaponCoId < nWeaponCoId);
}
