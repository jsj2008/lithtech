// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.cpp
//
// PURPOSE : Weapons container object - Implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "Weapons.h"
#include "RiotWeapons.h"
#include "PlayerObj.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"
#include "BaseCharacter.h"
#include "RiotServerShell.h"

extern CRiotServerShell* g_pRiotServerShellDE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CWeapons()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CWeapons::CWeapons() : Aggregate()
{
	// Clear our array...

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		m_weapons[i] = DNULL;
	}

	m_nCurWeapon = -1;
	m_eArsenal   = AT_NONE;
	m_eModelSize = MS_NORMAL;
	m_hObject	 = DNULL;
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
	for (int i=0; i < GUN_MAX_NUMBER; i++)
	{
		if (m_weapons[i]) delete m_weapons[i];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Init()
//
//	PURPOSE:	Initialize weapons
//
// ----------------------------------------------------------------------- //

DBOOL CWeapons::Init(HOBJECT hObj, ModelSize eSize)
{
	m_hObject	 = hObj;
	m_eModelSize = eSize;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::SetArsenal()
//
//	PURPOSE:	Set up the available weapons
//
// ----------------------------------------------------------------------- //

void CWeapons::SetArsenal(ArsenalType eType)
{
	if (m_eArsenal == eType || !m_hObject) return;

	m_eArsenal = eType;

	switch (eType)
	{
		case AT_MECHA:  // Add the Mecha weapons..
		{
			CreateMechaWeapons();
		}
		break;

		case AT_ONFOOT:
		{
			CreateOnFootWeapons();
		}
		break;

		case AT_ALL_WEAPONS:
		{
			CreateMechaWeapons();
			CreateOnFootWeapons();
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CreateWeapon()
//
//	PURPOSE:	Create the specified weapon
//
// ----------------------------------------------------------------------- //

void CWeapons::CreateWeapon(DBYTE nWeaponId)
{
	if (!IsValidIndex(nWeaponId) || m_weapons[nWeaponId]) return;

	CWeapon* pWeapon = DNULL;
	DBOOL	 bMelee  = DFALSE;

	switch (nWeaponId)
	{
		case GUN_PULSERIFLE_ID :
			pWeapon = new CPulseRifle;
		break;

		case GUN_SPIDER_ID :
			pWeapon = new CSpider;
		break;

		case GUN_BULLGUT_ID :
			pWeapon = new CBullgut;
		break;

		case GUN_SNIPERRIFLE_ID :
			pWeapon = new CSniperRifle;
		break;

		case GUN_JUGGERNAUT_ID :
			pWeapon = new CJuggernaut;
		break;

		case GUN_SHREDDER_ID :
			pWeapon = new CShredder;
		break;

		case GUN_REDRIOT_ID :
			pWeapon = new CRedRiot;
		break;

		case GUN_ENERGYBATON_ID :
			pWeapon = new CEnergyBaton;
			bMelee = DTRUE;
		break;

		case GUN_ENERGYBLADE_ID :
			pWeapon = new CEnergyBlade;
			bMelee = DTRUE;
		break;

		case GUN_KATANA_ID :
			pWeapon = new CKatana;
			bMelee = DTRUE;
		break;

		case GUN_MONOKNIFE_ID :
			pWeapon = new CMonoKnife;
			bMelee = DTRUE;
		break;

		// On-foot mode weapons...

		case GUN_COLT45_ID :
			pWeapon = new CColt45;
		break;

		case GUN_SHOTGUN_ID	:
			pWeapon = new CShotgun;
		break;

		case GUN_ASSAULTRIFLE_ID :
			pWeapon = new CAssaultRifle;
		break;

		case GUN_ENERGYGRENADE_ID :
			pWeapon = new CEnergyGrenade;
		break;

		case GUN_KATOGRENADE_ID :
			pWeapon = new CKatoGrenade;
		break;

		case GUN_MAC10_ID :
			pWeapon = new CMac10;
		break;

		case GUN_TOW_ID	:
			pWeapon = new CTOW;
		break;

		case GUN_LASERCANNON_ID :
			pWeapon = new CLaserCannon;
		break;

		case GUN_SQUEAKYTOY_ID :
			pWeapon = new CSqueakyToy;
		break;

		case GUN_TANTO_ID :
			pWeapon = new CTanto;
			bMelee = DTRUE;
		break;

		default : break;
	}


	if (pWeapon)
	{
		pWeapon->Init(m_hObject, m_eModelSize);
		m_weapons[nWeaponId] = pWeapon;
	
		if (bMelee)
		{
			m_weapons[nWeaponId]->Aquire();
			m_weapons[nWeaponId]->AddAmmo(GetWeaponMaxAmmo(nWeaponId));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CreateMechaWeapons()
//
//	PURPOSE:	Set up the mecha weapon arsenal
//
// ----------------------------------------------------------------------- //

void CWeapons::CreateMechaWeapons()
{
	// Make sure we don't already have this arsenal...

	if (m_weapons[GUN_FIRSTMECH_ID]) return;

	for (int i=GUN_FIRSTMECH_ID; i <= GUN_LASTMECH_ID; i++)
	{
		CreateWeapon(i);
	}
} 


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::CreateOnFootWeapons()
//
//	PURPOSE:	Set up the on foot weapon arsenal
//
// ----------------------------------------------------------------------- //

void CWeapons::CreateOnFootWeapons()
{
	// Make sure we don't already have this arsenal...

	if (m_weapons[GUN_FIRSTONFOOT_ID]) return;

	
	for (int i=GUN_FIRSTONFOOT_ID; i <= GUN_LASTONFOOT_ID; i++)
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
		
DDWORD CWeapons::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DBYTE)lData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DBYTE)lData);
		}
		break;
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CWeapons::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_ADDWEAPON:
		{	
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return DFALSE;

			DBOOL	bHadIt	  = DTRUE;
			DBOOL   bPickedUp = DFALSE;
			DBYTE	nWeapon   = pServerDE->ReadFromMessageByte(hRead);
			DFLOAT	fAmmo	  = pServerDE->ReadFromMessageFloat(hRead);

			if (IsValidIndex(nWeapon))
			{
				if (m_weapons[nWeapon] && m_weapons[nWeapon]->Have())
				{
					if( m_weapons[nWeapon]->GetAmmoCount() < m_weapons[nWeapon]->GetMaxAmmo())
					{
						bPickedUp = DTRUE;
						m_weapons[nWeapon]->AddAmmo((int)fAmmo);
					}
				}
				else
				{
					bHadIt	  = DFALSE;
					bPickedUp = DTRUE;
					ObtainWeapon(nWeapon, (int)fAmmo);
				}
			}

			if (bPickedUp)
			{
				// Tell powerup it was picked up...

				HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
				pServerDE->WriteToMessageFloat(hWrite, -1.0f);
				pServerDE->EndMessage(hWrite);


				// Send the appropriate message to the client...

				HCLASS hClass = pServerDE->GetObjectClass(m_hObject);
				if (pServerDE->IsKindOf(hClass, pServerDE->GetClass("CPlayerObj")))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hObject);
					if (!pPlayer) return DFALSE;

					HCLIENT hClient = pPlayer->GetClient();
					HMESSAGEWRITE hMessage = pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
					pServerDE->WriteToMessageByte(hMessage, IC_WEAPON_PICKUP_ID);
					pServerDE->WriteToMessageByte(hMessage, nWeapon);
					pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)m_weapons[nWeapon]->GetAmmoCount());
					pServerDE->EndMessage(hMessage);

					
					// If there isn't currently a weapon selected, or we're
					// on our melee weapon, Select the new weapon..

					if (m_nCurWeapon < 0 || 
					   (GetCommandId(m_nCurWeapon) < GetCommandId(nWeapon) && !bHadIt) ||
						GetWeaponType((RiotWeaponId)m_nCurWeapon) == MELEE)
					{
						pPlayer->ChangeWeapon(GetCommandId(nWeapon), DTRUE);
					}
				}

			}

		}
		break;

		default : break;
	}

	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::ObtainWeapon()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// @parm the index of the weapon
//
// ----------------------------------------------------------------------- //

void CWeapons::ObtainWeapon(DBYTE nWeaponId, int nDefaultAmmo,
							DBOOL bNotifyClient)
{
	if (!IsValidIndex(nWeaponId)) return;


	// Create the weapon if need be...

	if (m_eArsenal == AT_AS_NEEDED)
	{
		if (!m_weapons[nWeaponId]) 
		{
			CreateWeapon(nWeaponId);
		}
	}


	// Make sure we actually have the thing...

	if (!m_weapons[nWeaponId]) return;
	

	// Give us the gun!

	m_weapons[nWeaponId]->Aquire();

	
	// Set the weapon's default ammo if appropriate...

	if (nDefaultAmmo >= 0)
	{
		m_weapons[nWeaponId]->SetAmmo(nDefaultAmmo);
	}

	
	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if (bNotifyClient)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (!pServerDE) return;

		// Send the appropriate message to the client...

		HCLASS hClass = pServerDE->GetObjectClass(m_hObject);
		if (pServerDE->IsKindOf(hClass, pServerDE->GetClass("CPlayerObj")))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hObject);
			if (!pPlayer) return;

			HCLIENT hClient = pPlayer->GetClient();
			HMESSAGEWRITE hMessage = pServerDE->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
			pServerDE->WriteToMessageByte(hMessage, IC_WEAPON_ID);
			pServerDE->WriteToMessageByte(hMessage, nWeaponId);
			pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)m_weapons[nWeaponId]->GetAmmoCount());
			pServerDE->EndMessage(hMessage);

			
			// If there isn't currently a weapon selected, or we're
			// on our melee weapon, Select the new weapon..

			if (m_nCurWeapon < 0 || GetWeaponType((RiotWeaponId)m_nCurWeapon) == MELEE)
			{
				pPlayer->ChangeWeapon(GetCommandId(nWeaponId));
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetWeaponModel()
//
//	PURPOSE:	Get the weapon model object associated with the weapon
//
// @parm the index of the weapon
//
// ----------------------------------------------------------------------- //

HOBJECT CWeapons::GetModelObject(DBYTE nWeaponId)
{
	if (IsValidIndex(nWeaponId))
	{
		if (m_weapons[nWeaponId]) return m_weapons[nWeaponId]->GetModelObject();
	}

	return DNULL;
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

DBOOL CWeapons::ChangeWeapon(DBYTE nNewWeapon)
{
	if (nNewWeapon == m_nCurWeapon) return DTRUE;

	if (!IsValidWeapon(nNewWeapon)) return DFALSE;

	// Set this as our current weapon...

	m_nCurWeapon = nNewWeapon;


	// Let the base character know that a weapon change occured...

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	HCLASS hClass = pServerDE->GetObjectClass(m_hObject);
	if (pServerDE->IsKindOf(hClass, pServerDE->GetClass("CBaseCharacter")))
	{
		CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject(m_hObject);
		if (pChar) 
		{
			pChar->HandleWeaponChange();
		}
	}

	// Select the weapon...

	m_weapons[m_nCurWeapon]->Select();

	return DTRUE;
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
		m_weapons[m_nCurWeapon]->Deselect();
	}

	m_nCurWeapon = -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::AddAmmo()
//
//	PURPOSE:	Add some ammo to a specific weapon
//
// @parm which weapon
// @parm how much ammo
//
// ----------------------------------------------------------------------- //

void CWeapons::AddAmmo(DBYTE nWeapon, int nAmmo)
{
	if (!IsValidWeapon(nWeapon)) return;

	m_weapons[nWeapon]->AddAmmo(nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetAmmoCount()
//
//	PURPOSE:	Return the ammo amount of the currently selected weapon
//
// @rdef the ammo count
//
// ----------------------------------------------------------------------- //

int CWeapons::GetAmmoCount()
{
	if (IsValidIndex(m_nCurWeapon) && m_weapons[m_nCurWeapon])
	{
		return m_weapons[m_nCurWeapon]->GetAmmoCount();
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::GetAmmoCount()
//
//	PURPOSE:	Return the ammo amount of the weapon
//
// @rdef the ammo count
//
// ----------------------------------------------------------------------- //

int CWeapons::GetAmmoCount(DBYTE nWeaponID)
{
	if (IsValidIndex(nWeaponID) && m_weapons[nWeaponID])
	{
		return m_weapons[nWeaponID]->GetAmmoCount();
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nCurWeapon);
	pServerDE->WriteToMessageByte(hWrite, m_eModelSize);
	pServerDE->WriteToMessageByte(hWrite, m_eArsenal);

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (m_weapons[i]) 
		{
			pServerDE->WriteToMessageByte(hWrite, DTRUE);
			m_weapons[i]->Save(hWrite, nType);
		}
		else
		{
			pServerDE->WriteToMessageByte(hWrite, DFALSE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapons::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapons::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	m_nCurWeapon	 = (int)pServerDE->ReadFromMessageFloat(hRead);
	m_eModelSize	 = (ModelSize)pServerDE->ReadFromMessageByte(hRead);
	m_eArsenal		 = (ArsenalType)pServerDE->ReadFromMessageByte(hRead);

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		DBOOL bLoad = pServerDE->ReadFromMessageByte(hRead);

		if (bLoad)
		{
			CreateWeapon(i);

			if (m_weapons[i]) 
			{
				m_weapons[i]->Load(hRead, nType);
			}
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
	for (int i=0; i < GUN_MAX_NUMBER; i++)
	{
		if (m_weapons[i] && GetWeaponType((RiotWeaponId)i) != MELEE)
		{
			m_weapons[i]->SetAmmo(0);
			m_weapons[i]->Drop();
		}
	}
}
