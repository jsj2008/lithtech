// ----------------------------------------------------------------------- //
//
// MODULE  : GearItems.h
//
// PURPOSE : Gear items definition
//
// CREATED : 10/22/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GEAR_ITEMS_H__
#define __GEAR_ITEMS_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

class GearItem : public PickupItem
{
	public :

		GearItem();

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual void	ObjectTouch(HOBJECT hObject);
		virtual void	PickedUp(HMESSAGEREAD hRead);

        uint8           m_nGearId;

	private :

		void ReadProp(ObjectCreateStruct *pStruct);
		void PostPropRead(ObjectCreateStruct *pStruct);
        void InitialUpdate(ILTServer *pServer);
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
};

class CGearPlugin : public CWeaponMgrPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

#endif //  __GEAR_ITEMS_H__