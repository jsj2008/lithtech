// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.h
//
// PURPOSE : Weapon items implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//			 07/12/02 - kls
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_ITEMS_H__
#define __WEAPON_ITEMS_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

LINKTO_MODULE( WeaponItems );


class WeaponItem : public PickupItem
{
	public :

		WeaponItem();

        uint8 GetWeaponId() const { return m_nWeaponId; }
        uint8 GetAmmoId()	const { return m_nAmmoId; }

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

        uint8           m_nWeaponId;
        uint8           m_nAmmoId;
		int				m_nAmmo;

		virtual void	ObjectTouch (HOBJECT hObject, bool bForcePickup=false);
		
		virtual void	PickedUp( ILTMessage_Read *pMsg );
		virtual void	Respawn( );

	private :

		bool ReadProp(ObjectCreateStruct *pStruct);
		bool PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

class CWeaponItemPlugin : public CPickupItemPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_Dims(
				const char* szRezPath,
				const char* szPropValue,
				char* szModelFilenameBuf,
				int	  nModelFilenameBufLen,
				LTVector & vDims);

	private:

		CWeaponMgrPlugin m_WeaponMgrPlugin;
};

#endif //  __WEAPON_ITEMS_H__