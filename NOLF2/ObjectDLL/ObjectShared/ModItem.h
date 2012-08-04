// ----------------------------------------------------------------------- //
//
// MODULE  : ModItem.h
//
// PURPOSE : Mod items definition
//
// CREATED : 7/21/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MOD_ITEM_H__
#define __MOD_ITEM_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

LINKTO_MODULE( ModItem );

class ModItem : public PickupItem
{
	public :

		ModItem();

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		virtual void	ObjectTouch(HOBJECT hObject, bool bForcePickup=false);
		virtual void	PickedUp(ILTMessage_Read *pMsg);
		virtual void	Respawn( );
		
        uint8           m_nModId;

	private :

		bool ReadProp(ObjectCreateStruct *pStruct);
		bool PostPropRead(ObjectCreateStruct *pStruct);
        void InitialUpdate(ILTServer *pServer);
        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

class CModPlugin : public CWeaponMgrPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

		virtual LTRESULT PreHook_Dims(
				const char* szRezPath,
				const char* szPropValue,
				char* szModelFilenameBuf,
				int	  nModelFilenameBufLen,
				LTVector & vDims);

	protected:

		CPickupItemPlugin m_PickupItemPlugin;
};

#endif //  __MOD_ITEM_H__