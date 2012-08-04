// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.h
//
// PURPOSE : Weapons container object - Definition
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONS_H__
#define __WEAPONS_H__

#include "iaggregate.h"
#include "ltbasedefs.h"
#include "WeaponMgr.h"

class BaseClass;
class ILTServer;
class CPlayerObj;
class CWeapon;
class CProjectile;

typedef ObjRefList ProjectileMsgList;

// Use weapon's default ammo id.  NOTE: This shares the same namespace as
// the WeaponMgr::m_AmmoList indexes: 0 - WeaponMgr::m_AmmoList->GetLength()).
// Also, 255 is reserved by the WeaponMgr...

#define AMMO_DEFAULT_ID			254

// Class Definition

// @class CWeapons Weapon container/management object
class CWeapons
{
	public :

		CWeapons();
		~CWeapons();

        LTBOOL Init(HOBJECT hCharacter, HOBJECT hWeaponModel = LTNULL);

        void ObtainWeapon(uint8 nWeapon, int nAmmoId = AMMO_DEFAULT_ID,
                          int nDefaultAmmo = -1, LTBOOL bNotifyClient=LTFALSE);
        void ObtainMod(uint8 nWeapon, uint8 nModId, bool bNotifyClient=false,
					   bool bDisplayMsg=true);

        LTBOOL ChangeWeapon(uint8 nNewWeapon);

		void DeselectCurWeapon();

		int	   AddAmmo(int nAmmoId, int nAmmo); //returns amount of ammo actually added
        LTBOOL SetAmmo(int nAmmoId, int nAmmo=-1);

		void DecrementAmmo(int nAmmoId);
		int  GetAmmoCount(int nAmmoId);
		int  GetWeaponAmmoCount(int nWeaponId);

		int  GetCurWeaponId() const { return m_nCurWeapon; }

		CWeapon* GetCurWeapon();
        CWeapon* GetWeapon(uint8 nWeaponId);

        LTBOOL IsValidIndex(uint8 nWeaponId);
        LTBOOL IsValidWeapon(uint8 nWeaponId);

        LTBOOL IsValidAmmoId(int nAmmoId);

		void Reset();

		// This is used solely by the CWeapon class so it can
		// shoot vector based projectiles without instantiating
		// a new object.
        CProjectile*  GetVecProjectile() { return m_pVecProjectile; }

        uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

        void Save(ILTMessage_Write *pMsg, uint8 nType);
        void Load(ILTMessage_Read *pMsg, uint8 nType);

	protected :

        LTBOOL AddWeapon(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
        LTBOOL AddAmmoBox(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
        LTBOOL AddMod(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
	
		LTBOOL AddWeapon(LPBASECLASS pObject, HOBJECT hSender, uint8 nWeaponId, 
			uint8 nAmmoId, int nAmmo, bool bForcePickup);

		bool   AddIsAmmoWeapon(uint8 nAmmoId);

		// Message handler functions
		void HandleProjectileMessage( HOBJECT, ILTMessage_Read* );


	private :  
		// Private Member functions

		// Removes all the pending objects from the current
		// "projectiles needing messages" list.
		bool CleanProjectileMessageList();


		// Member Variables

		LTObjRef		m_hCharacter;		// The character
		LTObjRef		m_hWeaponModel;	// The hand held weapon model
		int				m_nCurWeapon;   // Current weapon index

		CWeapon**		m_pWeapons;
		int*			m_pAmmo;

		// This is used solely by the CWeapon class so it can
		// shoot vector based projectiles without instantiating
		// a new object.
		CProjectile*	m_pVecProjectile; // Projectile class used with vector weapons

		void DeleteWeapons();
		void CreateAllWeapons();
        void CreateWeapon(uint8 nWeaponId, uint8 nAmmoId=AMMO_DEFAULT_ID);

		void HandlePotentialWeaponChange(CPlayerObj* pPlayer, uint8 nWeaponId, 
			uint8 nAmmoId, LTBOOL bHaveIt);

		LTBOOL IsBetterWeapon(CPlayerObj* pPlayer, uint8 nWeaponId);

		// list of projectiles that want messages
		ProjectileMsgList m_lProjectilesNeedingMessages;

		// Projectiles may try to remove themselves from
		// the update list while we are iterating through
		// it.  Keep track of them and remove them later.
		// NOTE: This may not be needed in the header, but
		// it does need to be accessable by all members.
		ProjectileMsgList m_lProjectilesToRemove;
};

inline LTBOOL CWeapons::IsValidAmmoId(int nAmmoId)
{
    if (!m_pAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId)) return LTFALSE;

    return LTTRUE;
}

inline CWeapon* CWeapons::GetCurWeapon()
{
    CWeapon* pRet = LTNULL;
	if (IsValidWeapon(m_nCurWeapon))
	{
		pRet = m_pWeapons[m_nCurWeapon];
	}
	return pRet;
}

inline LTBOOL CWeapons::IsValidIndex(uint8 nWeaponId)
{
	return g_pWeaponMgr->IsValidWeaponId(nWeaponId);
}

inline CWeapon* CWeapons::GetWeapon(uint8 nWeaponId)
{
    CWeapon* pRet = LTNULL;

	if (IsValidWeapon(nWeaponId))
	{
		pRet = m_pWeapons[nWeaponId];
	}

	return pRet;
}

extern CBankedList<CWeapon> s_bankCWeapon;

#endif //__WEAPONS_H__