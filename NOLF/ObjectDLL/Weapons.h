// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.h
//
// PURPOSE : Weapons container object - Definition
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONS_H__
#define __WEAPONS_H__

#include "iaggregate.h"
#include "ltbasedefs.h"
#include "Weapon.h"
#include "Projectile.h"
class BaseClass;
class ILTServer;
class CPlayerObj;

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
        void ObtainMod(uint8 nWeapon, uint8 nModId, LTBOOL bNotifyClient=LTFALSE);

        LTBOOL ChangeWeapon(uint8 nNewWeapon);

		void DeselectCurWeapon();

		int	  AddAmmo(int nAmmoId, int nAmmo); //returns amount of ammo actually added
        LTBOOL SetAmmo(int nAmmoId, int nAmmo=-1);

		void DecrementAmmo(int nAmmoId);
		int GetAmmoCount(int nAmmoId);
		int GetWeaponAmmoCount(int nWeaponId);

		int GetCurWeaponId() const { return m_nCurWeapon; }

		CWeapon* GetCurWeapon();
        CWeapon* GetWeapon(uint8 nWeaponId);

        LTBOOL IsValidIndex(uint8 nWeaponId);
        LTBOOL IsValidWeapon(uint8 nWeaponId);

        LTBOOL IsValidAmmoId(int nAmmoId);

		void Reset();

        CProjectile*  GetVecProjectile() { return &m_VecProjectile; }

        uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        void Save(HMESSAGEWRITE hWrite, uint8 nType);
        void Load(HMESSAGEREAD hRead, uint8 nType);

	protected :

        LTBOOL AddWeapon(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
        LTBOOL AddAmmoBox(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
        LTBOOL AddMod(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
	
		LTBOOL AddWeapon(LPBASECLASS pObject, HOBJECT hSender, uint8 nWeaponId, 
			uint8 nAmmoId, int nAmmo, LTBOOL bIsLevelPowerup);

	private :  // Member Variables

		HOBJECT			m_hCharacter;		// The character
		HOBJECT			m_hWeaponModel;	// The hand held weapon model
		int				m_nCurWeapon;   // Current weapon index

		CWeapon**		m_pWeapons;
		int*			m_pAmmo;

		CProjectile		m_VecProjectile; // Projectile class used with vector weapons

		void DeleteWeapons();
		void CreateAllWeapons();
        void CreateWeapon(uint8 nWeaponId, uint8 nAmmoId=AMMO_DEFAULT_ID);

		void HandlePotentialWeaponChange(CPlayerObj* pPlayer, uint8 nWeaponId, 
			uint8 nAmmoId, LTBOOL bHaveIt, LTBOOL bWasOutOfAmmo);
		LTBOOL IsBetterWeapon(CPlayerObj* pPlayer, uint8 nWeaponId);
};

inline LTBOOL CWeapons::IsValidAmmoId(int nAmmoId)
{
    if (!m_pAmmo || !g_pWeaponMgr->IsValidAmmoType(nAmmoId)) return LTFALSE;

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
	return g_pWeaponMgr->IsValidWeapon(nWeaponId);
}

inline LTBOOL CWeapons::IsValidWeapon(uint8 nWeaponId)
{
	if(m_pWeapons && IsValidIndex(nWeaponId) && m_pWeapons[nWeaponId] &&
       m_pWeapons[nWeaponId]->Have()) return LTTRUE;

    return LTFALSE;
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

inline void CWeapons::DeleteWeapons()
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
	}
}

#endif //__WEAPONS_H__