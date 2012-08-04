// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.h
//
// PURPOSE : AmmoBox object definition
//
// CREATED : 10/28/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AMMO_BOX_H__
#define __AMMO_BOX_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

#define AB_MAX_TYPES	10

class AmmoBox : public PickupItem
{
	public :

		AmmoBox();
		virtual void	PickedUp(HMESSAGEREAD hRead);

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32          ObjectMessageFn (HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void	ObjectTouch(HOBJECT hObject);

        uint8           m_nAmmoId[AB_MAX_TYPES];
		int				m_nAmmoCount[AB_MAX_TYPES];

		// The OriginalXXX are used only in multiplayer so the are not
		// saved/loaded...
        uint8           m_nOriginalAmmoId[AB_MAX_TYPES];
		int				m_nOriginalAmmoCount[AB_MAX_TYPES];

	private :

		void ReadProp(ObjectCreateStruct *pStruct);
        void InitialUpdate(ILTServer *pServer);
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void Leftovers(HMESSAGEREAD hRead);
};

class CAmmoBoxPlugin : public CWeaponMgrPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

#endif //  __AMMO_BOX_H__