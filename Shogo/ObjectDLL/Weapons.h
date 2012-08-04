// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.h
//
// PURPOSE : Weapons container object - Definition
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONS_H__
#define __WEAPONS_H__

#include "cpp_aggregate_de.h"
#include "basedefs_de.h"
#include "WeaponDefs.h"
#include "Weapon.h"
#include "ModelFuncs.h"

class BaseClass;
class CServerDE;

// Class Definition

// @class CWeapons Weapon container/management object
class CWeapons : public Aggregate
{
	public :

		enum ArsenalType { AT_NONE=0, AT_ONFOOT, AT_MECHA, AT_ALL_WEAPONS,
						   AT_AS_NEEDED};

		CWeapons();
		~CWeapons();

		DBOOL Init(HOBJECT hObj, ModelSize eSize=MS_NORMAL);

		void SetArsenal(ArsenalType eType);

		void ObtainWeapon(DBYTE nWeapon, int nDefaultAmmo = -1, DBOOL bNotifyClient=DFALSE);	

		DBOOL ChangeWeapon(DBYTE nNewWeapon);

		void DeselectCurWeapon();

		HOBJECT GetModelObject(DBYTE nNewWeapon);

		void AddAmmo(DBYTE nWeaponID, int nAmmo);

		int GetAmmoCount();
		int GetAmmoCount(DBYTE nWeaponID);
		int GetCurWeaponId() const { return m_nCurWeapon; }

		CWeapon* GetCurWeapon();
		CWeapon* GetWeapon(DBYTE nWeaponId);

		DBOOL IsValidIndex(DBYTE nWeaponId);
		DBOOL IsValidWeapon(DBYTE nWeaponId);

		void Reset();

	protected :
	
		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);

	private :  // Member Variables

		HOBJECT			m_hObject;		// Just in case we need this...
		int				m_nCurWeapon;   // Current weapon index
		ArsenalType		m_eArsenal;		// OnFoot or Mech weapons
		ModelSize		m_eModelSize;	// Model size

		CWeapon*		m_weapons[GUN_MAX_NUMBER];

		void CreateMechaWeapons();
		void CreateOnFootWeapons();
		void CreateWeapon(DBYTE nWeaponId);
};

inline CWeapon* CWeapons::GetCurWeapon()
{ 
	CWeapon* pRet = DNULL;
	if (IsValidWeapon(m_nCurWeapon))
	{
		pRet = m_weapons[m_nCurWeapon];
	}
	return pRet;
}

inline DBOOL CWeapons::IsValidIndex(DBYTE nWeaponId)
{
	if(GUN_FIRST_ID <= nWeaponId && nWeaponId < GUN_MAX_NUMBER) return DTRUE;

	return DFALSE;
}

inline DBOOL CWeapons::IsValidWeapon(DBYTE nWeaponId)
{
	if(IsValidIndex(nWeaponId) && m_weapons[nWeaponId] && 
	   m_weapons[nWeaponId]->Have()) return DTRUE;

	return DFALSE;
}

inline CWeapon* CWeapons::GetWeapon(DBYTE nWeaponId)
{
	CWeapon* pRet = DNULL;

	if (IsValidWeapon(nWeaponId))
	{
		pRet = m_weapons[nWeaponId];
	}

	return pRet;
}

#endif //__WEAPONS_H__