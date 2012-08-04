// ----------------------------------------------------------------------- //
//
// MODULE  : ModItem.h
//
// PURPOSE : Mod items definition
//
// CREATED : 7/21/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MOD_ITEM_H__
#define __MOD_ITEM_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

class ModItem : public PickupItem
{
	public :

		ModItem();

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual void	ObjectTouch(HOBJECT hObject);
		virtual void	PickedUp(HMESSAGEREAD hRead);

        uint8           m_nModId;

	private :

		void ReadProp(ObjectCreateStruct *pStruct);
		void PostPropRead(ObjectCreateStruct *pStruct);
        void InitialUpdate(ILTServer *pServer);
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
};

class CModPlugin : public CWeaponMgrPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

#endif //  __MOD_ITEM_H__