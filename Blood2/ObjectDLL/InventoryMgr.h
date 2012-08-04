#ifndef __INVENTORYMGR_H__
#define __INVENTORYMGR_H__


#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"
#include "Weapon.h"
#include "InvItem.h"
#include "DLink.h"



// Defines....

// Return values from change weapon
#define CHWEAP_OK					0
#define CHWEAP_NOTENOUGHSTRENGTH	-1
#define CHWEAP_NOTAVAIL				-2
#define CHWEAP_CURRENT				-3
#define CHWEAP_NOTENOUGHMAGIC		-4
#define CHWEAP_NOAVAILSLOTS			-5
#define CHWEAP_ALREADYHAVE			-6
#define CHWEAP_WEAPONBUSY			-7
#define CHWEAP_NOMESSAGE			-8
#define CHWEAP_NOAMMO				-9


// inventory structure
class CInventoryMgr : public Aggregate
{
	public :

		CInventoryMgr();
		virtual ~CInventoryMgr();

		void		Init(HOBJECT hObject, HCLIENT hClient=NULL);
		void		Term();
		void		Update();
		
		// @cmember Handle object destruction
		virtual void HandleDestruction();

		// Weapon & ammo functions
		CViewWeaponModel* CreateViewModel(HATTACHMENT *hAttachment);
		int			ObtainWeapon(DDWORD nWeapon, int slot = -1);
		int			ChangeWeapon(DDWORD nWeapon);
		int			ChangeWeaponSlot(DBYTE nSlot);
		CWeapon*	FindBestWeapon(DFLOAT fTargetDist);
		int			SelectNextWeapon();
		int			SelectPrevWeapon();
		int			DropWeapon(DBYTE slot);
		int			DropCurrentWeapon();
		CWeapon*	GetWeapon(int slot) { return m_Weapons[slot]; }
		CWeapon*	GetCurrentWeapon() { return m_pCurWeapon; }
		DBOOL		HasWeapon(DDWORD nWeapon);
		void		ShowHandModels(DBOOL bShow);

		int			GetCurrentWeaponSlot() { return m_nCurWeaponSlot; }
		DDWORD		FireCurrentWeapon(DVector *firedPos, DRotation *rotP, DBOOL bAltFire, DBOOL bLefthand = DFALSE);
		void		UpdateCurrentWeaponFiring(DVector *firedPos, DVector *lFiredPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		void		SetNotFiringCurrentWeapon();
		void		ShowViewWeapons(DBOOL bVisible);
		void		SetCurrentWeaponSpecialData(DDWORD dwSpecial, DFLOAT fSpecial);

		void		SetFullAmmo();
		void		SetInfiniteAmmo(DBOOL bInfinite)	{ m_bInfiniteAmmo = bInfinite; SetFullAmmo(); }
		int			AddAmmo(DBYTE nAmmoType, DFLOAT fAmmoCount);
		DFLOAT		GetAmmoCount(DBYTE nAmmoType);
		void		SetAmmoCount(DBYTE nAmmoType, DFLOAT fAmmoCount);
		DFLOAT		GetCurrentWeaponAmmoCount();
		DFLOAT		GetCurrentWeaponAltFireAmmoCount();

		DBYTE		GetItemCharges( CInvItem *pItem );

		// Inventory/key/spell item functions
		int			AddItem(DBYTE nItemType, DBYTE nValue );
		int			SetActiveItem(DBYTE nItemSlot);

		// Select the next and previous items
		CInvItem	*GetPrevItem();
		CInvItem	*GetNextItem();
		void		SelectNextItem();
		void		SelectPrevItem();
		void		UpdateClient( );

		int			AddInventoryWeapon(DBYTE nWeapType, DBYTE nCount = 1);
		int			SelectInventoryWeapon(DDWORD nWeapon);
		DBOOL		IsItemActive(int nItem);
		CInvItem*	GetItem(int nItem);
		CInvItem*	GetCurrentItem() { return m_pCurItem ? (CInvItem*)m_pCurItem->m_pData : DNULL; }
		void		SelectItem(int nItemType );

		int			RemoveItem(DBYTE nItemType);
		int			RemoveItem( CInvItem *pItem );

		int			AddKey(HSTRING hstrItemName, HSTRING hstrDisplayName, HSTRING hstrIconFile, HSTRING hstrIconFile2, DBYTE byUseCount);
		int			QueryKey(HSTRING hstrItemName);


		// Owner attributes functions
		void		SetStrength(DBYTE nStrength);
		DBYTE       GetStrength()   { return m_nAttribStrength;}
		void		SetMagic(DBYTE nMagic);
		void		SetClient(HCLIENT hClient);

		void		AddDamageMultiplier(DFLOAT fFactor) { m_fDamageMultiplier *= fFactor; }
		void		RemoveDamageMultiplier(DFLOAT fFactor) { m_fDamageMultiplier /= fFactor; }
		DFLOAT		GetDamageMultiplier() { return m_fDamageMultiplier; }
		
		void		AddFireRateMultiplier(DFLOAT fFactor) { m_fFireRateMultiplier *= fFactor; }
		void		RemoveFireRateMultiplier(DFLOAT fFactor) { m_fFireRateMultiplier /= fFactor; }
		DFLOAT		GetFireRateMultiplier() { return m_fFireRateMultiplier; }

		void		SetGodMode(DBOOL bGodMode) { m_bGodMode = bGodMode; if (bGodMode) SetFullAmmo();}
		void		SendPickedUpMessage(HOBJECT hObject, int iRet);
		void		SendKeyQueryResponse(HOBJECT hObject, HSTRING hstrItemName, int iRet);
		void		SendClientItemMsg(DBYTE nMsg, DBYTE nItem);

	protected:

		DDWORD		EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		void		SendConsoleMessageToClient(char *msg);
		void		SetForceUpdateList(ForceUpdate* pFU);
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
	
	private: // Member Variables

		HOBJECT		m_hOwner;
		HCLIENT		m_hClient;

		DBYTE		m_nAttribStrength;
		DBYTE		m_nAttribMagic;
		DBOOL		m_bGodMode;
		DBOOL		m_bInfiniteAmmo;

		int			m_nCurWeaponSlot;
		DDWORD		m_nCurWeapon;	// The current weapon ID
		CViewWeaponModel*	m_pViewModel;	// View model
		CViewWeaponModel*	m_pLViewModel;	// Left view model
		HATTACHMENT	m_hViewModelAttach;
		HATTACHMENT m_hLViewModelAttach;
		HOBJECT		m_hMuzzleFlash;			// Muzzle flash light object
		CWeapon*	m_Weapons[SLOTCOUNT_TOTALWEAPONS]; // Array of carried weapons
		CWeapon*	m_LWeapons[SLOTCOUNT_TOTALWEAPONS]; // Array of left-hand weapons
		CWeapon*	m_pCurWeapon;
		DFLOAT		m_fAmmo[AMMO_MAXAMMOTYPES+1];	// All of our current ammo, DFLOAT to better set continuous use types

		DList		m_InvItemList;
		DLink*		m_pCurItem;
		DDWORD		m_WeapCount;
		DFLOAT		m_fManaRechargeTime;
		DFLOAT		m_fDamageMultiplier;
		DFLOAT		m_fFireRateMultiplier;
		DFLOAT		m_fMeleeMultiplier;

		HOBJECT		m_hLastPickupObject;
		int			m_nLastPickupResult;
		HOBJECT		m_hLastDroppedItem;
		DFLOAT		m_fLastDroppedTime;
        
        DBOOL       m_bDropEye;

		// Weapon switching
		DBOOL		m_bSwitchingWeapons;
		int			m_nNextWeaponSlot;

		// Update info...
		CInvItem *	m_pLastCurrentItem;
		CInvItem *	m_pLastPrevItem;
		CInvItem *	m_pLastNextItem;
		DBYTE		m_nLastCurrentItemCharge;
		DBYTE		m_nLastPrevItemCharge;
		DBYTE		m_nLastNextItemCharge;

		DBOOL		m_bShowItems;
};


// INLINE FUNCTIONS
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetAmmoCount()
//
//	PURPOSE:	Returns the ammount of the specified ammo
//
// ----------------------------------------------------------------------- //

inline DFLOAT CInventoryMgr::GetAmmoCount(DBYTE nAmmoType)
{
	if (nAmmoType <= AMMO_NONE || nAmmoType > AMMO_MAXAMMOTYPES)
		return 0;

    // If no ammo required then always return 1
    if (nAmmoType == AMMO_NONE) return 1;
    
	return m_fAmmo[nAmmoType];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetCurrentWeaponAmmoCount()
//
//	PURPOSE:	Returns the ammount of the current weapons ammo
//
// ----------------------------------------------------------------------- //

inline DFLOAT CInventoryMgr::GetCurrentWeaponAmmoCount()
{
	if (m_pCurWeapon)
		return GetAmmoCount(m_pCurWeapon->GetAmmoType(DFALSE));
	else
		return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetCurrentWeaponAltFireAmmoCount()
//
//	PURPOSE:	Returns the ammount of the current weapons ammo
//
// ----------------------------------------------------------------------- //

inline DFLOAT CInventoryMgr::GetCurrentWeaponAltFireAmmoCount()
{
	if (m_pCurWeapon)
		return GetAmmoCount(m_pCurWeapon->GetAmmoType(DTRUE));
	else
		return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::FireCurrentWeapon()
//
//	PURPOSE:	Fires the current weapon
//
// ----------------------------------------------------------------------- //

inline DDWORD CInventoryMgr::FireCurrentWeapon(DVector *firedPos, DRotation *rotP, DBOOL bAltFire, DBOOL bLeftHand)
{
	DDWORD dwRetval = 0;

	//SCHLEGZ 4/27/98 1:59:35 PM: GetAIWeaponPos does not work like you would think
	CServerDE* pServerDE = BaseClass::GetServerDE();

	CWeapon *pWeap = m_Weapons[m_nCurWeaponSlot];
	CWeapon *pLWeap = m_LWeapons[m_nCurWeaponSlot];

	if (!bLeftHand && pWeap)
	{
		pWeap->SetFirePosRot(firedPos, rotP, bAltFire);
		dwRetval = pWeap->Fire();
		pWeap->SetNotFiring();
	}
	else if (bLeftHand && pLWeap)
	{
		pLWeap->SetFirePosRot(firedPos, rotP, bAltFire);
		dwRetval = pLWeap->Fire();
		pLWeap->SetNotFiring();
		// ^ This pLWeap was 'pWeap'... I changed it cause it looks like a bug (Andy 1/11/99)
	}

	return dwRetval;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetCurrentWeaponSpecialData()
//
//	PURPOSE:	Sets the special firing values for the current weapon
//
// ----------------------------------------------------------------------- //

inline void CInventoryMgr::SetCurrentWeaponSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	CWeapon *pWeap = m_Weapons[m_nCurWeaponSlot];
	CWeapon *pLWeap = m_LWeapons[m_nCurWeaponSlot];

	if(pWeap)
		pWeap->SetSpecialData(dwSpecial, fSpecial);

	if(pLWeap)
		pLWeap->SetSpecialData(dwSpecial, fSpecial);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetNotFiring()
//
//	PURPOSE:	Tells current weapon it isn't firing anymore.
//
// ----------------------------------------------------------------------- //

inline void CInventoryMgr::SetNotFiringCurrentWeapon()
{
//	if (m_pCurWeapon)
//		m_pCurWeapon->SetNotFiring();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::HideViewWeapons()
//
//	PURPOSE:	Hides or shows the view weapons
//
// ----------------------------------------------------------------------- //

inline void CInventoryMgr::ShowViewWeapons(DBOOL bVisible)
{
	if (m_pViewModel && m_Weapons[m_nCurWeaponSlot])
	{
		m_pViewModel->SetVisible(bVisible);
	}
	if (m_pLViewModel && m_LWeapons[m_nCurWeaponSlot]) 
	{
		m_pLViewModel->SetVisible(bVisible);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SendConsoleMessageToClient()
//
//	PURPOSE:	Sends a message to the client
//
// ----------------------------------------------------------------------- //

inline void CInventoryMgr::SendConsoleMessageToClient(char *msg)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	HMESSAGEWRITE hMsg;
	
	if (!m_hClient)
		return;
 
	hMsg = pServerDE->StartMessage(m_hClient, SMSG_CONSOLEMESSAGE);
	pServerDE->WriteToMessageString(hMsg, msg);
	pServerDE->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::IsItemActive()
//
//	PURPOSE:	Returns the active state of the inventory item or spell.
//
// ----------------------------------------------------------------------- //

inline DBOOL CInventoryMgr::IsItemActive(int nItemType)
{
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType && pItem->IsActive())
		{
			return DTRUE;
		}
		pLink = pLink->m_pNext;
	}
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetCurrentItemIndex()
//
//	PURPOSE:	Returns the currently selected item
//
// ----------------------------------------------------------------------- //
/*
inline int CInventoryMgr::GetCurrentItemIndex()
{
	return m_nCurrentItemIndex;
}

inline void CInventoryMgr::SetCurrentItemIndex(int nIndex)
{
	m_nCurrentItemIndex=nIndex;
}
*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetItem()
//
//	PURPOSE:	Returns a pointer to the inventory item.
//
// ----------------------------------------------------------------------- //

inline CInvItem* CInventoryMgr::GetItem(int nItemType)
{
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType)
		{
			return pItem;
		}
		pLink = pLink->m_pNext;
	}
	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetItemIndex()
//
//	PURPOSE:	Returns a pointer to the inventory item at the specified index.
//
// ----------------------------------------------------------------------- //
/*
inline CInvItem* CInventoryMgr::GetItemIndex(int nIndex)
{
	assert(nIndex >= 0 && nIndex < SLOTCOUNT_ITEMS);
	return m_InvItems[nIndex];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetSlotItem()
//
//	PURPOSE:	Returns a pointer to the inventory item.
//
// ----------------------------------------------------------------------- //

inline CInvItem* CInventoryMgr::GetSlotItem(DBYTE nItemSlot)
{
	if(nItemSlot < 0 || nItemSlot >= SLOTCOUNT_ITEMS)
		return DNULL;

	if (m_InvItems[nItemSlot])
		return m_InvItems[nItemSlot];

	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetSlotSpell()
//
//	PURPOSE:	Returns a pointer to the inventory item.
//
// ----------------------------------------------------------------------- //

inline CInvItem* CInventoryMgr::GetSlotSpell(DBYTE nSpellSlot)
{
	if(nSpellSlot < 0 || nSpellSlot >= SLOTCOUNT_SPELLS)
		return DNULL;

	if (m_InvSpells[nSpellSlot])
		return m_InvSpells[nSpellSlot];

	return DNULL;
}
*/

#endif // __INVENTORYMGR_H__