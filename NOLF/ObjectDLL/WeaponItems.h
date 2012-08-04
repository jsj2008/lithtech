// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.h
//
// PURPOSE : Weapon items implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_ITEMS_H__
#define __WEAPON_ITEMS_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

class WeaponItem : public PickupItem
{
	public :

		WeaponItem();

        uint8 GetWeaponId() const { return m_nWeaponId; }
        uint8 GetAmmoId()	const { return m_nAmmoId; }
		virtual void	PickedUp(HMESSAGEREAD hRead);

		void SetIsLevelPowerup(LTBOOL b) { m_bIsLevelPowerup = b; }

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

        uint8           m_nWeaponId;
        uint8           m_nAmmoId;
		int				m_nAmmo;

		// NOTE:  m_bLevelPowerup was added for Update 1.002 and is NOT saved 
		// because it only changes the behavoir of multiplayer games.

		LTBOOL			m_bIsLevelPowerup;

		virtual void	ObjectTouch (HOBJECT hObject);

	private :

		void ReadProp(ObjectCreateStruct *pStruct);
		void PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
};

class CWeaponItemPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

	private:

		CWeaponMgrPlugin m_WeaponMgrPlugin;
};

#endif //  __WEAPON_ITEMS_H__